#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_generate_or_load_keypair_t {
	sgx_status_t ms_retval;
	uint8_t* ms_pub_key;
	const uint8_t* ms_sealed_data;
	uint32_t ms_sealed_size;
	uint8_t* ms_sealed_out;
	uint32_t ms_max_sealed_out_size;
	uint32_t* ms_sealed_out_size;
} ms_generate_or_load_keypair_t;

typedef struct ms_decrypt_with_peer_pubkeys_t {
	sgx_status_t ms_retval;
	uint8_t* ms_peer_pubkey_buf;
	size_t ms_total_len;
	size_t ms_num_keys;
} ms_decrypt_with_peer_pubkeys_t;

typedef struct ms_ecall_decrypt_batch_t {
	sgx_status_t ms_retval;
	float* ms_input;
	size_t ms_input_size;
	uint8_t* ms_flat_ciphertext;
	size_t ms_ct_flat_size;
	uint8_t* ms_flat_iv;
	size_t ms_iv_flat_size;
	uint8_t* ms_flat_tag;
	size_t ms_tag_flat_size;
	uint8_t* ms_flat_aad;
	size_t ms_aad_flat_size;
	uint8_t* ms_ct_sizes;
	size_t ms_batch_size;
	int* ms_notary_id;
	int* ms_trust_values;
	size_t ms_trust_values_size;
	int* ms_trust_states;
} ms_ecall_decrypt_batch_t;

typedef struct ms_ocall_print_error_t {
	sgx_status_t ms_err;
} ms_ocall_print_error_t;

typedef struct ms_ocall_print_private_key_t {
	uint8_t* ms_priv_bytes;
	uint32_t ms_len;
} ms_ocall_print_private_key_t;

typedef struct ms_ocall_print_dh_key_t {
	uint8_t* ms_dh_bytes;
	uint32_t ms_len;
} ms_ocall_print_dh_key_t;

typedef struct ms_ocall_print_t {
	const char* ms_str;
} ms_ocall_print_t;

typedef struct ms_ocall_print_buffer_t {
	const char* ms_buf;
	size_t ms_len;
} ms_ocall_print_buffer_t;

typedef struct ms_ocall_print_float_t {
	float ms_val;
} ms_ocall_print_float_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_error(void* pms)
{
	ms_ocall_print_error_t* ms = SGX_CAST(ms_ocall_print_error_t*, pms);
	ocall_print_error(ms->ms_err);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_private_key(void* pms)
{
	ms_ocall_print_private_key_t* ms = SGX_CAST(ms_ocall_print_private_key_t*, pms);
	ocall_print_private_key(ms->ms_priv_bytes, ms->ms_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_dh_key(void* pms)
{
	ms_ocall_print_dh_key_t* ms = SGX_CAST(ms_ocall_print_dh_key_t*, pms);
	ocall_print_dh_key(ms->ms_dh_bytes, ms->ms_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print(void* pms)
{
	ms_ocall_print_t* ms = SGX_CAST(ms_ocall_print_t*, pms);
	ocall_print(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_buffer(void* pms)
{
	ms_ocall_print_buffer_t* ms = SGX_CAST(ms_ocall_print_buffer_t*, pms);
	ocall_print_buffer(ms->ms_buf, ms->ms_len);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_float(void* pms)
{
	ms_ocall_print_float_t* ms = SGX_CAST(ms_ocall_print_float_t*, pms);
	ocall_print_float(ms->ms_val);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[6];
} ocall_table_Enclave = {
	6,
	{
		(void*)Enclave_ocall_print_error,
		(void*)Enclave_ocall_print_private_key,
		(void*)Enclave_ocall_print_dh_key,
		(void*)Enclave_ocall_print,
		(void*)Enclave_ocall_print_buffer,
		(void*)Enclave_ocall_print_float,
	}
};
sgx_status_t generate_or_load_keypair(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* pub_key, const uint8_t* sealed_data, uint32_t sealed_size, uint8_t* sealed_out, uint32_t max_sealed_out_size, uint32_t* sealed_out_size)
{
	sgx_status_t status;
	ms_generate_or_load_keypair_t ms;
	ms.ms_pub_key = pub_key;
	ms.ms_sealed_data = sealed_data;
	ms.ms_sealed_size = sealed_size;
	ms.ms_sealed_out = sealed_out;
	ms.ms_max_sealed_out_size = max_sealed_out_size;
	ms.ms_sealed_out_size = sealed_out_size;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t decrypt_with_peer_pubkeys(sgx_enclave_id_t eid, sgx_status_t* retval, uint8_t* peer_pubkey_buf, size_t total_len, size_t num_keys)
{
	sgx_status_t status;
	ms_decrypt_with_peer_pubkeys_t ms;
	ms.ms_peer_pubkey_buf = peer_pubkey_buf;
	ms.ms_total_len = total_len;
	ms.ms_num_keys = num_keys;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_decrypt_batch(sgx_enclave_id_t eid, sgx_status_t* retval, float* input, size_t input_size, uint8_t* flat_ciphertext, size_t ct_flat_size, uint8_t* flat_iv, size_t iv_flat_size, uint8_t* flat_tag, size_t tag_flat_size, uint8_t* flat_aad, size_t aad_flat_size, uint8_t* ct_sizes, size_t batch_size, int* notary_id, int* trust_values, size_t trust_values_size, int* trust_states)
{
	sgx_status_t status;
	ms_ecall_decrypt_batch_t ms;
	ms.ms_input = input;
	ms.ms_input_size = input_size;
	ms.ms_flat_ciphertext = flat_ciphertext;
	ms.ms_ct_flat_size = ct_flat_size;
	ms.ms_flat_iv = flat_iv;
	ms.ms_iv_flat_size = iv_flat_size;
	ms.ms_flat_tag = flat_tag;
	ms.ms_tag_flat_size = tag_flat_size;
	ms.ms_flat_aad = flat_aad;
	ms.ms_aad_flat_size = aad_flat_size;
	ms.ms_ct_sizes = ct_sizes;
	ms.ms_batch_size = batch_size;
	ms.ms_notary_id = notary_id;
	ms.ms_trust_values = trust_values;
	ms.ms_trust_values_size = trust_values_size;
	ms.ms_trust_states = trust_states;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

