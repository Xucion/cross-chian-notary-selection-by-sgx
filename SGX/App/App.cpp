#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "sgx_urts.h"
#include "Enclave_u.h"
#include "sgx_tseal.h"
#include "sgx_tcrypto.h"

#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <array>

// ===== 新增：MySQL 头文件 =====
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

#define ENCLAVE_PATH "enclave.signed.so"
#define SEALED_FILE "key.sealed"

// ===== MySQL 连接配置 =====
#define MYSQL_HOST "tcp://127.0.0.1:3306"
#define MYSQL_USER "sgx_user"
#define MYSQL_PASS "2628"
#define MYSQL_DB   "sgx_db"

using json = nlohmann::json;

// ===== 新增：获取 MySQL 连接 =====
sql::Connection* get_mysql_connection() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS);
    con->setSchema(MYSQL_DB);
    return con;
}

std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    if (hex.length() % 2 != 0) {
        throw std::runtime_error("Hex string has invalid length.");
    }
    for (size_t i = 0; i < hex.length(); i += 2) {
        uint8_t byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

void print_vector(const std::vector<uint8_t>& vec, const std::string& name) {
    std::cout << name << " (" << vec.size() << " bytes): ";
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(vec[i]);
        if (i < vec.size() - 1) std::cout << " ";
    }
    std::cout << std::dec << std::endl;
}

// ===== 辅助：将字节数组转十六进制字符串 =====
std::string bytes_to_hex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; i++) {
        oss << std::hex << std::uppercase
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    return oss.str();
}

// ===== 辅助：将 flat 数组指定段转十六进制字符串 =====
std::string flat_to_hex(const std::vector<uint8_t>& flat, size_t offset, size_t len) {
    return bytes_to_hex(flat.data() + offset, len);
}

int load_sealed_key(uint8_t* buf, uint32_t buf_size, uint32_t* actual_size) {
    FILE* f = fopen(SEALED_FILE, "rb");
    if (!f) return 0;
    *actual_size = fread(buf, 1, buf_size, f);
    fclose(f);
    return 1;
}

void save_sealed_key(const uint8_t* buf, uint32_t size) {
    if (size == 0) return;
    const char* tmp_file = "key.sealed.tmp";
    FILE* f = fopen(tmp_file, "wb");
    if (!f) return;
    size_t written = fwrite(buf, 1, size, f);
    if (written != size) { fclose(f); return; }
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    if (rename(tmp_file, SEALED_FILE) != 0) perror("rename");
}

void ocall_print_float(float val)  { printf("Max value is: %f\n", val); }
void ocall_print_error(sgx_status_t err) { printf("Enclave inner error: 0x%X\n", err); }
void ocall_print_private_key(uint8_t* priv_bytes, uint32_t len) {
    printf("[DEBUG] Private key:\n");
    for (uint32_t i = 0; i < len; i++) printf("%02X", priv_bytes[i]);
    printf("\n");
}
void ocall_print_dh_key(uint8_t* dh_bytes, uint32_t len) {
    printf("[DEBUG] DH:\n");
    for (uint32_t i = 0; i < len; i++) printf("%02X", dh_bytes[i]);
    printf("\n");
}
void ocall_print(const char* str) { std::cout << "[Enclave] " << str << std::endl; }
void ocall_print_buffer(const char* buf, size_t len) {
    std::cout << "[Enclave output] ";
    for (size_t i = 0; i < len; i++) std::cout << buf[i];
    std::cout << std::endl;
}

std::string join_with_pipe(const int* arr, size_t len) {
    std::string result;
    for (size_t i = 0; i < len; ++i) {
        result += std::to_string(arr[i]);
        if (i != len - 1) result += "|";
    }
    return result;
}

std::string join_with_pipe(const std::vector<int>& arr) {
    return join_with_pipe(arr.data(), arr.size());
}

int main(int argc, char* argv[]) {

    sgx_enclave_id_t eid;
    sgx_launch_token_t token = {0};
    int updated = 0;
    sgx_status_t ret;

    ret = sgx_create_enclave(ENCLAVE_PATH, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
    if (ret != SGX_SUCCESS) { printf("[-] Failed to create enclave\n"); return -1; }

    uint8_t pub_key[64] = {0};
    uint8_t sealed_data[2048] = {0};
    uint32_t sealed_size = 0;
    load_sealed_key(sealed_data, sizeof(sealed_data), &sealed_size);

    uint8_t sealed_out[2048] = {0};
    uint32_t max_sealed_out_size = sizeof(sealed_out);
    uint32_t sealed_out_size = max_sealed_out_size;

    sgx_status_t ecall_ret;
    ret = generate_or_load_keypair(
        eid, &ecall_ret,
        pub_key,
        sealed_data, sealed_size,
        sealed_out, max_sealed_out_size,
        &sealed_out_size
    );
    if (ret != SGX_SUCCESS || ecall_ret != SGX_SUCCESS) {
        printf("[-] Enclave failed: sgx_ecall ret=0x%X, ecall_ret=0x%X\n", ret, ecall_ret);
        return -1;
    }
    save_sealed_key(sealed_out, sealed_out_size);

    printf("[+] ECC Public Key:\nX = ");
    for (int i = 0; i < 32; i++) printf("%02X", pub_key[i]);
    printf("\nY = ");
    for (int i = 32; i < 64; i++) printf("%02X", pub_key[i]);
    printf("\n");

    if (argc > 1) {
        std::string arg_str = argv[1];

        // ===== 管理员指令：_has_banned / _has_Unlocked =====
        if (arg_str.find("_has_banned") != std::string::npos ||
            arg_str.find("_has_Unlocked") != std::string::npos) {

            bool is_banned = (arg_str.find("_has_banned") != std::string::npos);
            size_t split_pos = arg_str.find("_has_");

            try {
                int target_idx = std::stoi(arg_str.substr(0, split_pos));

                sql::Connection* con = get_mysql_connection();

                if (is_banned) {
                    // ===== 原：删除文件中第 target_idx 行 =====
                    // ===== 新：按行号找到对应 id 然后 DELETE =====
                    // 用 LIMIT + OFFSET 定位第 target_idx 条（0-based）
                    std::unique_ptr<sql::PreparedStatement> pstmt(
                        con->prepareStatement(
                            "DELETE FROM notary_data WHERE id = ("
                            "  SELECT id FROM ("
                            "    SELECT id FROM notary_data ORDER BY id LIMIT 1 OFFSET ?"
                            "  ) AS tmp"
                            ")"
                        )
                    );
                    pstmt->setInt(1, target_idx);
                    int affected = pstmt->executeUpdate();
                    if (affected > 0)
                        std::cout << "[Admin] Row at index " << target_idx << " deleted." << std::endl;
                    else
                        std::cerr << "Error: Index " << target_idx << " out of range." << std::endl;
                } else {
                    // ===== 原：找到第 target_idx 行，将最后一列（state）改为 0 =====
                    // ===== 新：UPDATE state=0 WHERE id 是第 target_idx 条 =====
                    std::unique_ptr<sql::PreparedStatement> pstmt(
                        con->prepareStatement(
                            "UPDATE notary_data SET state = 0 WHERE id = ("
                            "  SELECT id FROM ("
                            "    SELECT id FROM notary_data ORDER BY id LIMIT 1 OFFSET ?"
                            "  ) AS tmp"
                            ")"
                        )
                    );
                    pstmt->setInt(1, target_idx);
                    int affected = pstmt->executeUpdate();
                    if (affected > 0)
                        std::cout << "[Admin] Row at index " << target_idx << " state set to 0." << std::endl;
                    else
                        std::cerr << "Error: Index " << target_idx << " out of range." << std::endl;
                }

                delete con;
                printf("Max value is: ok");
            } catch (const sql::SQLException& e) {
                std::cerr << "MySQL error: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error processing admin command: " << e.what() << std::endl;
            }

            sgx_destroy_enclave(eid);

            return 0;
        }

        if (strlen(argv[1]) < 70) {
            // ===== 读取参数，解析 tokens =====
            std::vector<std::string> tokens;
            std::stringstream ss(argv[1]);
            std::string token;
            while (ss >> token) tokens.push_back(token);
            if (tokens.size() < 2) {
                std::cerr << "参数格式错误：至少需要数量和一组资产变更，例如 \"2 10|5|-50 0|30|-60\"" << std::endl;
                sgx_destroy_enclave(eid);
                return 1;
            }

            std::cout << "[+] 接收到输入参数数量: " << tokens[0] << std::endl;
            for (size_t i = 1; i < tokens.size(); ++i) {
                std::cout << "[+] 第 " << i << " 组资产变更: " << tokens[i] << std::endl;
            }

            // ===== 原：ifstream 读取 raw_data.txt 逐行解析 =====
            // ===== 新：SELECT * FROM notary_data =====
            std::vector<std::vector<uint8_t>> ciphertexts, ivs, tags, aads, pubs;
            std::vector<int> trust_values, trust_states;
            std::vector<int> row_ids; // 新增：保存每行的 MySQL id，用于后续 UPDATE

            try {
                sql::Connection* con = get_mysql_connection();
                std::unique_ptr<sql::Statement> stmt(con->createStatement());
                std::unique_ptr<sql::ResultSet> res(
                    stmt->executeQuery("SELECT id, ciphertext, iv, tag, aad, pub_key, trust, state FROM notary_data ORDER BY id")
                );

                int line_number = 0;
                while (res->next()) {
                    line_number++;
                    try {
                        row_ids.push_back(res->getInt("id"));
                        ciphertexts.push_back(hex_to_bytes(res->getString("ciphertext")));
                        ivs.push_back(hex_to_bytes(res->getString("iv")));
                        tags.push_back(hex_to_bytes(res->getString("tag")));
                        aads.push_back(hex_to_bytes(res->getString("aad")));
                        pubs.push_back(hex_to_bytes(res->getString("pub_key")));
                        trust_values.push_back(res->getInt("trust"));
                        trust_states.push_back(res->getInt("state"));

                        std::cout << "=== Row " << line_number << " (id=" << row_ids.back() << ") ===" << std::endl;
                        print_vector(ciphertexts.back(), "Ciphertext");
                        print_vector(ivs.back(), "IV");
                        print_vector(tags.back(), "Tag");
                        print_vector(aads.back(), "AAD");
                        print_vector(pubs.back(), "Public Key");
                        std::cout << "Trust Value: " << trust_values.back() << std::endl;
                        std::cout << "Trust state: " << trust_states.back() << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Hex conversion error at row " << line_number << ": " << e.what() << std::endl;
                    }
                }
                delete con;
            } catch (const sql::SQLException& e) {
                std::cerr << "MySQL SELECT error: " << e.what() << std::endl;
                sgx_destroy_enclave(eid);
                return 1;
            }

            // ===== Enclave 调用部分（不变）=====
            std::vector<uint8_t> flat_keys;
            for (const auto& pk : pubs)
                flat_keys.insert(flat_keys.end(), pk.begin(), pk.end());

            sgx_status_t ecall_ret2;
            ret = decrypt_with_peer_pubkeys(eid, &ecall_ret2,
                flat_keys.data(), flat_keys.size(), pubs.size());

            std::vector<uint8_t> flat_ciphertext, flat_iv, flat_tag, flat_aad, flat_pub;
            std::vector<uint8_t> ct_sizes_u8;
            for (size_t i = 0; i < pubs.size(); ++i) {
                flat_ciphertext.insert(flat_ciphertext.end(), ciphertexts[i].begin(), ciphertexts[i].end());
                flat_iv.insert(flat_iv.end(), ivs[i].begin(), ivs[i].end());
                flat_tag.insert(flat_tag.end(), tags[i].begin(), tags[i].end());
                flat_aad.insert(flat_aad.end(), aads[i].begin(), aads[i].end());
                flat_pub.insert(flat_pub.end(), pubs[i].begin(), pubs[i].end());
                ct_sizes_u8.push_back(static_cast<uint8_t>(ciphertexts[i].size()));
            }

            int requested_notary_count = 0;
            try {
                requested_notary_count = std::stoi(tokens[0]);
            } catch (const std::exception&) {
                std::cerr << "参数格式错误：第一个字段必须是公证人数量。" << std::endl;
                sgx_destroy_enclave(eid);
                return 1;
            }

            if (requested_notary_count <= 0) {
                std::cerr << "参数格式错误：公证人数量必须大于 0。" << std::endl;
                sgx_destroy_enclave(eid);
                return 1;
            }

            if ((int)tokens.size() - 1 != requested_notary_count) {
                std::cerr << "参数格式错误：公证人数量与资产变更组数不一致。" << std::endl;
                sgx_destroy_enclave(eid);
                return 1;
            }

            std::vector<std::array<float, 3>> asset_deltas;
            asset_deltas.reserve(tokens.size() - 1);
            for (size_t i = 1; i < tokens.size(); ++i) {
                std::array<float, 3> delta = {0.0f, 0.0f, 0.0f};
                std::stringstream ss2(tokens[i]);
                std::string token2;
                size_t field_idx = 0;
                while (std::getline(ss2, token2, '|')) {
                    if (field_idx >= delta.size()) {
                        std::cerr << "参数格式错误：" << tokens[i] << " 不是 a1|a2|a3 三段格式。" << std::endl;
                        sgx_destroy_enclave(eid);
                        return 1;
                    }
                    try {
                        delta[field_idx] = std::stof(token2);
                    } catch (const std::exception&) {
                        std::cerr << "无法转换: " << token2 << std::endl;
                        sgx_destroy_enclave(eid);
                        return 1;
                    }
                    field_idx++;
                }

                if (field_idx != delta.size()) {
                    std::cerr << "参数格式错误：" << tokens[i] << " 不是 a1|a2|a3 三段格式。" << std::endl;
                    sgx_destroy_enclave(eid);
                    return 1;
                }
                asset_deltas.push_back(delta);
            }

            std::vector<int> notary_ids(asset_deltas.size(), -1);
            sgx_status_t ecall_ret3;
            for (size_t i = 0; i < asset_deltas.size(); i++) {
                ret = ecall_decrypt_batch(
                    eid, &ecall_ret3,
                    asset_deltas[i].data(), asset_deltas[i].size(),
                    flat_ciphertext.data(), flat_ciphertext.size(),
                    flat_iv.data(), flat_iv.size(),
                    flat_tag.data(), flat_tag.size(),
                    flat_aad.data(), flat_aad.size(),
                    ct_sizes_u8.data(), ct_sizes_u8.size(),
                    &notary_ids[i],
                    trust_values.data(), trust_values.size(),
                    trust_states.data()
                );
                if (ret != SGX_SUCCESS || ecall_ret3 != SGX_SUCCESS) {
                    std::cerr << "ecall_decrypt_batch failed: ret=0x" << std::hex << ret
                              << ", ecall_ret=0x" << ecall_ret3 << std::dec << std::endl;
                    sgx_destroy_enclave(eid);
                    return 1;
                }
            }

            for (size_t i = 0; i < notary_ids.size(); i++)
                std::cout << "notary_id is " << notary_ids[i] << std::endl;
            std::string s = join_with_pipe(notary_ids);
            std::cout << "Max value is:" << s << std::endl;

            // ===== 原：ofstream trunc 重写整个文件 =====
            // ===== 新：UPDATE notary_data SET trust=?, state=? WHERE id=? =====
            try {
                sql::Connection* con = get_mysql_connection();
                std::unique_ptr<sql::PreparedStatement> pstmt(
                    con->prepareStatement(
                        "UPDATE notary_data SET ciphertext=?, iv=?, tag=?, trust=?, state=? WHERE id=?"
                    )
                );

                size_t ct_offset = 0;
                for (size_t i = 0; i < ct_sizes_u8.size(); i++) {
                    size_t ct_len = ct_sizes_u8[i];  // enclave 可能已修改长度

                    // 从 flat 数组中按偏移提取第 i 条的密文/iv/tag
                    std::string new_ciphertext_hex = bytes_to_hex(flat_ciphertext.data() + ct_offset, ct_len);
                    std::string new_iv_hex         = bytes_to_hex(flat_iv.data()  + i * 12, 12);
                    std::string new_tag_hex        = bytes_to_hex(flat_tag.data() + i * 16, 16);

                    pstmt->setString(1, new_ciphertext_hex);
                    pstmt->setString(2, new_iv_hex);
                    pstmt->setString(3, new_tag_hex);
                    pstmt->setInt   (4, trust_values[i]);
                    pstmt->setInt   (5, trust_states[i]);
                    pstmt->setInt   (6, row_ids[i]);
                    pstmt->executeUpdate();

                    ct_offset += ct_len;
                }
                delete con;
                printf("写入完成\n");
            } catch (const sql::SQLException& e) {
                std::cerr << "MySQL UPDATE error: " << e.what() << std::endl;
            }
        }
        else if (strlen(argv[1]) >= 70) {
            // 假设输入格式是: 密文 IV TAG AAD 公钥(或其他) ...
            std::vector<std::string> tokens;
            std::stringstream ss(argv[1]);
            std::string token;
            while (ss >> token) tokens.push_back(token);

            if (tokens.size() >= 5) { // 确保参数够用
                try {
                    sql::Connection* con = get_mysql_connection();
                    std::unique_ptr<sql::PreparedStatement> pstmt(
                        con->prepareStatement(
                            "INSERT INTO notary_data (ciphertext, iv, tag, aad, pub_key, trust, state) VALUES (?, ?, ?, ?, ?, 20, 0)"
                        )
                    );
                    pstmt->setString(1, tokens[0]); // ciphertext
                    pstmt->setString(2, tokens[1]); // iv
                    pstmt->setString(3, tokens[2]); // tag
                    pstmt->setString(4, tokens[3]); // aad
                    pstmt->setString(5, tokens[4]); // pub_key
                    
                    pstmt->executeUpdate();
                    std::cout << "[+] 数据已成功插入数据库！" << std::endl;
                    delete con;
                    printf("Max value is: ok");
                } catch (const sql::SQLException& e) {
                    std::cerr << "MySQL INSERT error: " << e.what() << std::endl;
                }
            } else {
                std::cerr << "参数数量不足以插入数据库！" << std::endl;
                sgx_destroy_enclave(eid);
                return -1; // 参数不足，返回非 0
            }
        }
    } else {
        std::cout << "参数有误或无参数数据:\n";
    }

    sgx_destroy_enclave(eid);
    return 0;
}
