#include "Enclave.h"
#include "Enclave_t.h"
#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include "sgx_utils.h"
#include "sgx_tseal.h"
#include <string.h>
#include <vector>
#include <string>
#include <array>
#include <cfloat> // 用于 FLT_MAX
#include <cstdio> // 用于 snprintf
#include <algorithm> // 用于 std::min, std::max

// ============================================================
// 配置区域
// ============================================================
// Support generated batch data plus subsequently registered notaries.
#define MAX_ENTRIES 5000 

// ============================================================
// 全局变量
// ============================================================
static sgx_ec256_private_t g_private_key;
static bool g_key_initialized = false;
static sgx_ec256_public_t g_public_key;  

// 静态分配足够大的密钥数组
static sgx_aes_gcm_128bit_key_t aes_key[MAX_ENTRIES]; 
size_t g_aes_key_count = 0;

// ============================================================
// 辅助函数
// ============================================================

static inline bool float_to_fixed_string(float v, std::string &out, int precision=3) {
    char buf[64];
    int ret = std::snprintf(buf, sizeof(buf), "%.*f", precision, v);
    if (ret < 0 || (size_t)ret >= sizeof(buf)) return false;
    out = buf;
    return true;
}

bool try_unseal_key(const uint8_t* sealed_data, uint32_t sealed_size) {
    if (sealed_size == 0) return false;

    uint32_t data_size = sgx_get_encrypt_txt_len((const sgx_sealed_data_t*)sealed_data);
    if (data_size != sizeof(sgx_ec256_private_t)) return false;

    uint8_t buf[sizeof(sgx_ec256_private_t)];
    sgx_status_t ret = sgx_unseal_data((const sgx_sealed_data_t*)sealed_data, NULL, 0, buf, &data_size);
    if (ret != SGX_SUCCESS) return false;

    memcpy(&g_private_key, buf, sizeof(g_private_key));
    
    ret = sgx_ecc256_calculate_pub_from_priv(&g_private_key, &g_public_key);
    if (ret != SGX_SUCCESS) return false;

    g_key_initialized = true;
    return true;
}

std::vector<std::string> split_fields(const std::string& s, char delimiter = '|') {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

// ============================================================
// ECALLs
// ============================================================

extern "C" sgx_status_t generate_or_load_keypair(uint8_t* pub_key, const uint8_t* sealed_data, uint32_t sealed_size, uint8_t* sealed_out, 
    uint32_t max_sealed_out_size, uint32_t* sealed_out_size)
{
    if (!g_key_initialized && try_unseal_key(sealed_data, sealed_size)) {
        memcpy(pub_key, &g_public_key.gx, 32);
        memcpy(pub_key + 32, &g_public_key.gy, 32);
        return SGX_SUCCESS;
    }
    
    sgx_ecc_state_handle_t ecc;
    sgx_ec256_public_t public_key;
    sgx_status_t ret;

    ret = sgx_ecc256_open_context(&ecc);
    if (ret != SGX_SUCCESS) return ret;

    ret = sgx_ecc256_create_key_pair(&g_private_key, &public_key, ecc);
    sgx_ecc256_close_context(ecc);
    if (ret != SGX_SUCCESS) return ret;

    memcpy(pub_key, &public_key.gx, 32);
    memcpy(pub_key + 32, &public_key.gy, 32);
    g_key_initialized = true;

    uint32_t required_size = sgx_calc_sealed_data_size(0, sizeof(g_private_key));
    if (max_sealed_out_size < required_size) {
        return SGX_ERROR_INVALID_PARAMETER;
    }

    ret = sgx_seal_data(0, NULL, sizeof(g_private_key), (const uint8_t*)&g_private_key, required_size, (sgx_sealed_data_t*)sealed_out);              
    if (ret != SGX_SUCCESS) {
        ocall_print_error(ret);
        return ret;
    }
    *sealed_out_size = required_size;
    return SGX_SUCCESS;
}

extern "C" sgx_status_t decrypt_with_peer_pubkeys(uint8_t* peer_pubkey_buf, size_t total_len, size_t num_keys)
{
    if (!g_key_initialized) return SGX_ERROR_UNEXPECTED;
    if (num_keys > MAX_ENTRIES) return SGX_ERROR_INVALID_PARAMETER;
    
    sgx_status_t ret;
    sgx_ec256_dh_shared_t shared_key;
    sgx_ec256_public_t peer_pub;
    sgx_ecc_state_handle_t ecc;
    
    ret = sgx_ecc256_open_context(&ecc);
    if (ret != SGX_SUCCESS) return ret;
    
    g_aes_key_count = 0;

    for (size_t i = 0; i < num_keys; ++i) {
        const uint8_t* pubkey_ptr = peer_pubkey_buf + i * 64;

        memcpy(peer_pub.gx, pubkey_ptr, 32);
        memcpy(peer_pub.gy, pubkey_ptr + 32, 32);

        ret = sgx_ecc256_compute_shared_dhkey(&g_private_key, &peer_pub, &shared_key, ecc);
        if (ret != SGX_SUCCESS) {
            sgx_ecc256_close_context(ecc);
            return ret;
        }
        
        memcpy(aes_key[i], &shared_key, 16);
        g_aes_key_count++;
    }
    sgx_ecc256_close_context(ecc);
    return SGX_SUCCESS;
}

sgx_status_t ecall_decrypt_batch(
    float* input,
    size_t input_size,
    uint8_t* ciphertexts, size_t ct_total_size, 
    uint8_t* ivs, size_t iv_size, 
    uint8_t* tags, size_t tag_size, 
    uint8_t* aads, size_t aad_size, 
    uint8_t* ct_sizes, 
    size_t num_entries,
    int* notary_id,
    int* trust_values, size_t trust_values_size,
    int* trust_states
) 
{
    if (num_entries > MAX_ENTRIES) {
        ocall_print("Error: num_entries exceeds MAX_ENTRIES");
        return SGX_ERROR_INVALID_PARAMETER;
    }
    if (input == nullptr || input_size != 3) {
        ocall_print("Error: input must contain exactly three asset deltas.");
        return SGX_ERROR_INVALID_PARAMETER;
    }

    // ==========================================
    // 1. 初始化查找变量 (逻辑修改点)
    // ==========================================
    int best_index = -1;
    
    // 我们不再找 min_value，而是找 max_trust
    int max_trust = -1; 
    
    std::array<float, 3> input_delta = { input[0], input[1], input[2] };
    std::array<float, 3> selected_assets = { 0.0f, 0.0f, 0.0f };
    int trust_bonus_base = 0;
    
    // 缓存最佳结果的字符串和位置
    std::string best_plain_str; 
    size_t best_ct_offset = 0;
    size_t best_ct_len = 0;

    size_t current_ct_offset = 0;

    // ==========================================
    // 2. 遍历解密并比较
    // ==========================================
    for (size_t i = 0; i < num_entries; ++i) {
        if (i >= g_aes_key_count) return SGX_ERROR_UNEXPECTED;

        size_t current_len = ct_sizes[i];
        
        // 准备解密容器
        std::vector<uint8_t> plaintext(current_len);
        
        uint8_t* ct_ptr = ciphertexts + current_ct_offset;
        uint8_t* iv_ptr = ivs + i * 12;
        uint8_t* tag_ptr = tags + i * 16;
        uint8_t* aad_ptr = aads + i * 18;

        sgx_status_t ret = sgx_rijndael128GCM_decrypt(
            &aes_key[i],
            ct_ptr, (uint32_t)current_len,
            plaintext.data(),
            iv_ptr, 12,
            aad_ptr, 18,
            (const sgx_aes_gcm_128bit_tag_t*)tag_ptr
        );

        if (ret == SGX_SUCCESS) {
            // 条件1：该节点尚未被占用
            if (trust_states[i] == 0) {
                std::string plain_str(reinterpret_cast<char*>(plaintext.data()), plaintext.size());
                auto fields = split_fields(plain_str);
                
                if (fields.size() >= 3) {
                    try {
                        std::array<float, 3> current_assets = {
                            std::stof(fields[0]),
                            std::stof(fields[1]),
                            std::stof(fields[2])
                        };
                        
                        // 获取当前节点的信誉值 (确保不越界)
                        int current_trust = 0;
                        if (i < trust_values_size) {
                            current_trust = trust_values[i];
                        }

                        bool has_enough_assets = true;
                        int current_bonus_base = 0;
                        for (size_t asset_idx = 0; asset_idx < current_assets.size(); ++asset_idx) {
                            float updated_asset = current_assets[asset_idx] + input_delta[asset_idx];
                            if (updated_asset < 0.0f) {
                                has_enough_assets = false;
                                break;
                            }
                            if (input_delta[asset_idx] < 0.0f) {
                                current_bonus_base += static_cast<int>(-input_delta[asset_idx]);
                            }
                        }

                        // ==========================================
                        // 核心比较逻辑修改
                        // ==========================================
                        // 1. 三项资产在应用变更后都不能小于 0
                        // 2. 在满足1的前提下，选择信誉值更高的 (current_trust > max_trust)
                        if (has_enough_assets) {
                            if (current_trust > max_trust) {
                                max_trust = current_trust;
                                best_index = (int)i;
                                selected_assets = current_assets;
                                trust_bonus_base = current_bonus_base;
                                
                                // 缓存数据
                                best_plain_str = plain_str;
                                best_ct_offset = current_ct_offset;
                                best_ct_len = current_len;
                            }
                            // 如果信誉值相同 (current_trust == max_trust)，
                            // 这里不做操作，保留先找到的那一个。
                            // 如果你想在信誉相同时，选资产更少的，可以在这里加 else if
                        }

                    } catch (...) {}
                }
            }
        } 
        current_ct_offset += current_len;
    } 

    // ==========================================
    // 3. 结果处理
    // ==========================================
    
    if (best_index == -1) {
        *notary_id = -1;
        ocall_print("No suitable notary found (insufficient funds or all occupied).");
        return SGX_SUCCESS; 
    }

    // 更新状态
    trust_states[best_index] = 1; 
    if ((size_t)best_index < trust_values_size) {
        trust_values[best_index] += trust_bonus_base / 10;
    }
    *notary_id = best_index;

    // ==========================================
    // 4. 修改明文并重新加密 (只针对 best_index)
    // ==========================================
    
    auto j_fields = split_fields(best_plain_str);
    if (j_fields.size() < 3) return SGX_ERROR_UNEXPECTED;

    for (size_t asset_idx = 0; asset_idx < selected_assets.size(); ++asset_idx) {
        std::string new_val_str;
        float new_val_num = selected_assets[asset_idx] + input_delta[asset_idx];
        if (!float_to_fixed_string(new_val_num, new_val_str)) {
            return SGX_ERROR_UNEXPECTED;
        }
        j_fields[asset_idx] = new_val_str;
    }

    // 重新拼接
    std::string new_plain_str;
    for (size_t k = 0; k < j_fields.size(); ++k) {
        new_plain_str += j_fields[k];
        if (k < j_fields.size() - 1) new_plain_str += "|";
    }

    size_t new_plain_len = new_plain_str.size();
    std::vector<uint8_t> new_ct(new_plain_len);
    
    uint8_t new_iv[12];
    sgx_status_t rr = sgx_read_rand(new_iv, 12);
    if (rr != SGX_SUCCESS) return rr;

    uint8_t new_tag[16];
    uint8_t* target_aad = aads + best_index * 18;

    sgx_status_t er = sgx_rijndael128GCM_encrypt(
        &aes_key[best_index],
        reinterpret_cast<const uint8_t*>(new_plain_str.data()), 
        (uint32_t)new_plain_len,
        new_ct.data(),
        new_iv, 12,
        target_aad, 18,
        (sgx_aes_gcm_128bit_tag_t*)new_tag
    );

    if (er != SGX_SUCCESS) return er;
    ocall_print(new_plain_str.c_str());

    // ==========================================
    // 5. 写回密文数组 (处理长度变化)
    // ==========================================
    
    ptrdiff_t shrink = (ptrdiff_t)best_ct_len - (ptrdiff_t)new_plain_len;

    if (shrink < 0) {
        ocall_print("Error: New ciphertext larger than old.");
        return SGX_ERROR_UNEXPECTED;
    }

    std::memcpy(ciphertexts + best_ct_offset, new_ct.data(), new_plain_len);

    if (shrink > 0) {
        size_t tail_start = best_ct_offset + best_ct_len;
        size_t tail_len = ct_total_size - tail_start;
        std::memmove(
            ciphertexts + best_ct_offset + new_plain_len,
            ciphertexts + tail_start,
            tail_len
        );
    }

    ct_sizes[best_index] = (uint8_t)new_plain_len;
    std::memcpy(ivs + best_index * 12, new_iv, 12);
    std::memcpy(tags + best_index * 16, new_tag, 16);

    return SGX_SUCCESS;
}
