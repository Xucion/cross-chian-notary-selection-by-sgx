#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

sgx_status_t generate_or_load_keypair(uint8_t* pub_key, const uint8_t* sealed_data, uint32_t sealed_size, uint8_t* sealed_out, uint32_t max_sealed_out_size, uint32_t* sealed_out_size);
sgx_status_t decrypt_with_peer_pubkeys(uint8_t* peer_pubkey_buf, size_t total_len, size_t num_keys);
sgx_status_t ecall_decrypt_batch(float* input, size_t input_size, uint8_t* flat_ciphertext, size_t ct_flat_size, uint8_t* flat_iv, size_t iv_flat_size, uint8_t* flat_tag, size_t tag_flat_size, uint8_t* flat_aad, size_t aad_flat_size, uint8_t* ct_sizes, size_t batch_size, int* notary_id, int* trust_values, size_t trust_values_size, int* trust_states);

sgx_status_t SGX_CDECL ocall_print_error(sgx_status_t err);
sgx_status_t SGX_CDECL ocall_print_private_key(uint8_t* priv_bytes, uint32_t len);
sgx_status_t SGX_CDECL ocall_print_dh_key(uint8_t* dh_bytes, uint32_t len);
sgx_status_t SGX_CDECL ocall_print(const char* str);
sgx_status_t SGX_CDECL ocall_print_buffer(const char* buf, size_t len);
sgx_status_t SGX_CDECL ocall_print_float(float val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
