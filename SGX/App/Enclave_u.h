#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_ERROR_DEFINED__
#define OCALL_PRINT_ERROR_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_error, (sgx_status_t err));
#endif
#ifndef OCALL_PRINT_PRIVATE_KEY_DEFINED__
#define OCALL_PRINT_PRIVATE_KEY_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_private_key, (uint8_t* priv_bytes, uint32_t len));
#endif
#ifndef OCALL_PRINT_DH_KEY_DEFINED__
#define OCALL_PRINT_DH_KEY_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_dh_key, (uint8_t* dh_bytes, uint32_t len));
#endif
#ifndef OCALL_PRINT_DEFINED__
#define OCALL_PRINT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print, (const char* str));
#endif
#ifndef OCALL_PRINT_BUFFER_DEFINED__
#define OCALL_PRINT_BUFFER_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_buffer, (const char* buf, size_t len));
#endif
#ifndef OCALL_PRINT_FLOAT_DEFINED__
#define OCALL_PRINT_FLOAT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_float, (float val));
#endif

sgx_status_t generate_or_load_keypair(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* pub_key, const uint8_t* sealed_data, uint32_t sealed_size, uint8_t* sealed_out, uint32_t max_sealed_out_size, uint32_t* sealed_out_size);
sgx_status_t decrypt_with_peer_pubkeys(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* peer_pubkey_buf, size_t total_len, size_t num_keys);
sgx_status_t ecall_decrypt_batch(sgx_enclave_id_t eid, sgx_status_t* retval, float* input, size_t input_size, uint8_t* flat_ciphertext, size_t ct_flat_size, uint8_t* flat_iv, size_t iv_flat_size, uint8_t* flat_tag, size_t tag_flat_size, uint8_t* flat_aad, size_t aad_flat_size, uint8_t* ct_sizes, size_t batch_size, int* notary_id, int* trust_values, size_t trust_values_size, int* trust_states);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
