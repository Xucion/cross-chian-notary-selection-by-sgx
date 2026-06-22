#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_generate_or_load_keypair(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_generate_or_load_keypair_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_generate_or_load_keypair_t* ms = SGX_CAST(ms_generate_or_load_keypair_t*, pms);
	ms_generate_or_load_keypair_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_generate_or_load_keypair_t), ms, sizeof(ms_generate_or_load_keypair_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_pub_key = __in_ms.ms_pub_key;
	size_t _len_pub_key = 64;
	uint8_t* _in_pub_key = NULL;
	const uint8_t* _tmp_sealed_data = __in_ms.ms_sealed_data;
	uint32_t _tmp_sealed_size = __in_ms.ms_sealed_size;
	size_t _len_sealed_data = _tmp_sealed_size;
	uint8_t* _in_sealed_data = NULL;
	uint8_t* _tmp_sealed_out = __in_ms.ms_sealed_out;
	uint32_t _tmp_max_sealed_out_size = __in_ms.ms_max_sealed_out_size;
	size_t _len_sealed_out = _tmp_max_sealed_out_size;
	uint8_t* _in_sealed_out = NULL;
	uint32_t* _tmp_sealed_out_size = __in_ms.ms_sealed_out_size;
	size_t _len_sealed_out_size = sizeof(uint32_t);
	uint32_t* _in_sealed_out_size = NULL;
	sgx_status_t _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_pub_key, _len_pub_key);
	CHECK_UNIQUE_POINTER(_tmp_sealed_data, _len_sealed_data);
	CHECK_UNIQUE_POINTER(_tmp_sealed_out, _len_sealed_out);
	CHECK_UNIQUE_POINTER(_tmp_sealed_out_size, _len_sealed_out_size);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_pub_key != NULL && _len_pub_key != 0) {
		if ( _len_pub_key % sizeof(*_tmp_pub_key) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_pub_key = (uint8_t*)malloc(_len_pub_key)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_pub_key, 0, _len_pub_key);
	}
	if (_tmp_sealed_data != NULL && _len_sealed_data != 0) {
		if ( _len_sealed_data % sizeof(*_tmp_sealed_data) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_sealed_data = (uint8_t*)malloc(_len_sealed_data);
		if (_in_sealed_data == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_sealed_data, _len_sealed_data, _tmp_sealed_data, _len_sealed_data)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_sealed_out != NULL && _len_sealed_out != 0) {
		if ( _len_sealed_out % sizeof(*_tmp_sealed_out) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_sealed_out = (uint8_t*)malloc(_len_sealed_out);
		if (_in_sealed_out == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_sealed_out, _len_sealed_out, _tmp_sealed_out, _len_sealed_out)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_sealed_out_size != NULL && _len_sealed_out_size != 0) {
		if ( _len_sealed_out_size % sizeof(*_tmp_sealed_out_size) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_sealed_out_size = (uint32_t*)malloc(_len_sealed_out_size)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sealed_out_size, 0, _len_sealed_out_size);
	}
	_in_retval = generate_or_load_keypair(_in_pub_key, (const uint8_t*)_in_sealed_data, _tmp_sealed_size, _in_sealed_out, _tmp_max_sealed_out_size, _in_sealed_out_size);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_pub_key) {
		if (memcpy_verw_s(_tmp_pub_key, _len_pub_key, _in_pub_key, _len_pub_key)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_sealed_out) {
		if (memcpy_verw_s(_tmp_sealed_out, _len_sealed_out, _in_sealed_out, _len_sealed_out)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_sealed_out_size) {
		if (memcpy_verw_s(_tmp_sealed_out_size, _len_sealed_out_size, _in_sealed_out_size, _len_sealed_out_size)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_pub_key) free(_in_pub_key);
	if (_in_sealed_data) free(_in_sealed_data);
	if (_in_sealed_out) free(_in_sealed_out);
	if (_in_sealed_out_size) free(_in_sealed_out_size);
	return status;
}

static sgx_status_t SGX_CDECL sgx_decrypt_with_peer_pubkeys(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_decrypt_with_peer_pubkeys_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_decrypt_with_peer_pubkeys_t* ms = SGX_CAST(ms_decrypt_with_peer_pubkeys_t*, pms);
	ms_decrypt_with_peer_pubkeys_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_decrypt_with_peer_pubkeys_t), ms, sizeof(ms_decrypt_with_peer_pubkeys_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	uint8_t* _tmp_peer_pubkey_buf = __in_ms.ms_peer_pubkey_buf;
	size_t _tmp_total_len = __in_ms.ms_total_len;
	size_t _len_peer_pubkey_buf = _tmp_total_len;
	uint8_t* _in_peer_pubkey_buf = NULL;
	sgx_status_t _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_peer_pubkey_buf, _len_peer_pubkey_buf);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_peer_pubkey_buf != NULL && _len_peer_pubkey_buf != 0) {
		if ( _len_peer_pubkey_buf % sizeof(*_tmp_peer_pubkey_buf) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_peer_pubkey_buf = (uint8_t*)malloc(_len_peer_pubkey_buf);
		if (_in_peer_pubkey_buf == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_peer_pubkey_buf, _len_peer_pubkey_buf, _tmp_peer_pubkey_buf, _len_peer_pubkey_buf)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	_in_retval = decrypt_with_peer_pubkeys(_in_peer_pubkey_buf, _tmp_total_len, __in_ms.ms_num_keys);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	if (_in_peer_pubkey_buf) free(_in_peer_pubkey_buf);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_decrypt_batch(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_decrypt_batch_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_decrypt_batch_t* ms = SGX_CAST(ms_ecall_decrypt_batch_t*, pms);
	ms_ecall_decrypt_batch_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_decrypt_batch_t), ms, sizeof(ms_ecall_decrypt_batch_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	float* _tmp_input = __in_ms.ms_input;
	size_t _tmp_input_size = __in_ms.ms_input_size;
	size_t _len_input = _tmp_input_size * sizeof(float);
	float* _in_input = NULL;
	uint8_t* _tmp_flat_ciphertext = __in_ms.ms_flat_ciphertext;
	size_t _tmp_ct_flat_size = __in_ms.ms_ct_flat_size;
	size_t _len_flat_ciphertext = _tmp_ct_flat_size;
	uint8_t* _in_flat_ciphertext = NULL;
	uint8_t* _tmp_flat_iv = __in_ms.ms_flat_iv;
	size_t _tmp_iv_flat_size = __in_ms.ms_iv_flat_size;
	size_t _len_flat_iv = _tmp_iv_flat_size;
	uint8_t* _in_flat_iv = NULL;
	uint8_t* _tmp_flat_tag = __in_ms.ms_flat_tag;
	size_t _tmp_tag_flat_size = __in_ms.ms_tag_flat_size;
	size_t _len_flat_tag = _tmp_tag_flat_size;
	uint8_t* _in_flat_tag = NULL;
	uint8_t* _tmp_flat_aad = __in_ms.ms_flat_aad;
	size_t _tmp_aad_flat_size = __in_ms.ms_aad_flat_size;
	size_t _len_flat_aad = _tmp_aad_flat_size;
	uint8_t* _in_flat_aad = NULL;
	uint8_t* _tmp_ct_sizes = __in_ms.ms_ct_sizes;
	size_t _tmp_batch_size = __in_ms.ms_batch_size;
	size_t _len_ct_sizes = _tmp_batch_size;
	uint8_t* _in_ct_sizes = NULL;
	int* _tmp_notary_id = __in_ms.ms_notary_id;
	size_t _len_notary_id = sizeof(int);
	int* _in_notary_id = NULL;
	int* _tmp_trust_values = __in_ms.ms_trust_values;
	size_t _tmp_trust_values_size = __in_ms.ms_trust_values_size;
	size_t _len_trust_values = _tmp_trust_values_size * sizeof(int);
	int* _in_trust_values = NULL;
	int* _tmp_trust_states = __in_ms.ms_trust_states;
	size_t _len_trust_states = _tmp_trust_values_size * sizeof(int);
	int* _in_trust_states = NULL;
	sgx_status_t _in_retval;

	if (sizeof(*_tmp_input) != 0 &&
		(size_t)_tmp_input_size > (SIZE_MAX / sizeof(*_tmp_input))) {
		return SGX_ERROR_INVALID_PARAMETER;
	}

	if (sizeof(*_tmp_trust_values) != 0 &&
		(size_t)_tmp_trust_values_size > (SIZE_MAX / sizeof(*_tmp_trust_values))) {
		return SGX_ERROR_INVALID_PARAMETER;
	}

	if (sizeof(*_tmp_trust_states) != 0 &&
		(size_t)_tmp_trust_values_size > (SIZE_MAX / sizeof(*_tmp_trust_states))) {
		return SGX_ERROR_INVALID_PARAMETER;
	}

	CHECK_UNIQUE_POINTER(_tmp_input, _len_input);
	CHECK_UNIQUE_POINTER(_tmp_flat_ciphertext, _len_flat_ciphertext);
	CHECK_UNIQUE_POINTER(_tmp_flat_iv, _len_flat_iv);
	CHECK_UNIQUE_POINTER(_tmp_flat_tag, _len_flat_tag);
	CHECK_UNIQUE_POINTER(_tmp_flat_aad, _len_flat_aad);
	CHECK_UNIQUE_POINTER(_tmp_ct_sizes, _len_ct_sizes);
	CHECK_UNIQUE_POINTER(_tmp_notary_id, _len_notary_id);
	CHECK_UNIQUE_POINTER(_tmp_trust_values, _len_trust_values);
	CHECK_UNIQUE_POINTER(_tmp_trust_states, _len_trust_states);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_input != NULL && _len_input != 0) {
		if ( _len_input % sizeof(*_tmp_input) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_input = (float*)malloc(_len_input);
		if (_in_input == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_input, _len_input, _tmp_input, _len_input)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_flat_ciphertext != NULL && _len_flat_ciphertext != 0) {
		if ( _len_flat_ciphertext % sizeof(*_tmp_flat_ciphertext) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_flat_ciphertext = (uint8_t*)malloc(_len_flat_ciphertext);
		if (_in_flat_ciphertext == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_flat_ciphertext, _len_flat_ciphertext, _tmp_flat_ciphertext, _len_flat_ciphertext)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_flat_iv != NULL && _len_flat_iv != 0) {
		if ( _len_flat_iv % sizeof(*_tmp_flat_iv) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_flat_iv = (uint8_t*)malloc(_len_flat_iv);
		if (_in_flat_iv == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_flat_iv, _len_flat_iv, _tmp_flat_iv, _len_flat_iv)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_flat_tag != NULL && _len_flat_tag != 0) {
		if ( _len_flat_tag % sizeof(*_tmp_flat_tag) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_flat_tag = (uint8_t*)malloc(_len_flat_tag);
		if (_in_flat_tag == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_flat_tag, _len_flat_tag, _tmp_flat_tag, _len_flat_tag)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_flat_aad != NULL && _len_flat_aad != 0) {
		if ( _len_flat_aad % sizeof(*_tmp_flat_aad) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_flat_aad = (uint8_t*)malloc(_len_flat_aad);
		if (_in_flat_aad == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_flat_aad, _len_flat_aad, _tmp_flat_aad, _len_flat_aad)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_ct_sizes != NULL && _len_ct_sizes != 0) {
		if ( _len_ct_sizes % sizeof(*_tmp_ct_sizes) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_ct_sizes = (uint8_t*)malloc(_len_ct_sizes);
		if (_in_ct_sizes == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_ct_sizes, _len_ct_sizes, _tmp_ct_sizes, _len_ct_sizes)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_notary_id != NULL && _len_notary_id != 0) {
		if ( _len_notary_id % sizeof(*_tmp_notary_id) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_notary_id = (int*)malloc(_len_notary_id)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_notary_id, 0, _len_notary_id);
	}
	if (_tmp_trust_values != NULL && _len_trust_values != 0) {
		if ( _len_trust_values % sizeof(*_tmp_trust_values) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_trust_values = (int*)malloc(_len_trust_values);
		if (_in_trust_values == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_trust_values, _len_trust_values, _tmp_trust_values, _len_trust_values)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_trust_states != NULL && _len_trust_states != 0) {
		if ( _len_trust_states % sizeof(*_tmp_trust_states) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_trust_states = (int*)malloc(_len_trust_states);
		if (_in_trust_states == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_trust_states, _len_trust_states, _tmp_trust_states, _len_trust_states)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	_in_retval = ecall_decrypt_batch(_in_input, _tmp_input_size, _in_flat_ciphertext, _tmp_ct_flat_size, _in_flat_iv, _tmp_iv_flat_size, _in_flat_tag, _tmp_tag_flat_size, _in_flat_aad, _tmp_aad_flat_size, _in_ct_sizes, _tmp_batch_size, _in_notary_id, _in_trust_values, _tmp_trust_values_size, _in_trust_states);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_flat_ciphertext) {
		if (memcpy_verw_s(_tmp_flat_ciphertext, _len_flat_ciphertext, _in_flat_ciphertext, _len_flat_ciphertext)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_flat_iv) {
		if (memcpy_verw_s(_tmp_flat_iv, _len_flat_iv, _in_flat_iv, _len_flat_iv)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_flat_tag) {
		if (memcpy_verw_s(_tmp_flat_tag, _len_flat_tag, _in_flat_tag, _len_flat_tag)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_ct_sizes) {
		if (memcpy_verw_s(_tmp_ct_sizes, _len_ct_sizes, _in_ct_sizes, _len_ct_sizes)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_notary_id) {
		if (memcpy_verw_s(_tmp_notary_id, _len_notary_id, _in_notary_id, _len_notary_id)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_trust_values) {
		if (memcpy_verw_s(_tmp_trust_values, _len_trust_values, _in_trust_values, _len_trust_values)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	if (_in_trust_states) {
		if (memcpy_verw_s(_tmp_trust_states, _len_trust_states, _in_trust_states, _len_trust_states)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_input) free(_in_input);
	if (_in_flat_ciphertext) free(_in_flat_ciphertext);
	if (_in_flat_iv) free(_in_flat_iv);
	if (_in_flat_tag) free(_in_flat_tag);
	if (_in_flat_aad) free(_in_flat_aad);
	if (_in_ct_sizes) free(_in_ct_sizes);
	if (_in_notary_id) free(_in_notary_id);
	if (_in_trust_values) free(_in_trust_values);
	if (_in_trust_states) free(_in_trust_states);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[3];
} g_ecall_table = {
	3,
	{
		{(void*)(uintptr_t)sgx_generate_or_load_keypair, 0, 0},
		{(void*)(uintptr_t)sgx_decrypt_with_peer_pubkeys, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_decrypt_batch, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[6][3];
} g_dyn_entry_table = {
	6,
	{
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
		{0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_error(sgx_status_t err)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_print_error_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_error_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_error_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_error_t));
	ocalloc_size -= sizeof(ms_ocall_print_error_t);

	if (memcpy_verw_s(&ms->ms_err, sizeof(ms->ms_err), &err, sizeof(err))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_private_key(uint8_t* priv_bytes, uint32_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_priv_bytes = len;

	ms_ocall_print_private_key_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_private_key_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(priv_bytes, _len_priv_bytes);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (priv_bytes != NULL) ? _len_priv_bytes : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_private_key_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_private_key_t));
	ocalloc_size -= sizeof(ms_ocall_print_private_key_t);

	if (priv_bytes != NULL) {
		if (memcpy_verw_s(&ms->ms_priv_bytes, sizeof(uint8_t*), &__tmp, sizeof(uint8_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_priv_bytes % sizeof(*priv_bytes) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, priv_bytes, _len_priv_bytes)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_priv_bytes);
		ocalloc_size -= _len_priv_bytes;
	} else {
		ms->ms_priv_bytes = NULL;
	}

	if (memcpy_verw_s(&ms->ms_len, sizeof(ms->ms_len), &len, sizeof(len))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(1, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_dh_key(uint8_t* dh_bytes, uint32_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_dh_bytes = len;

	ms_ocall_print_dh_key_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_dh_key_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(dh_bytes, _len_dh_bytes);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (dh_bytes != NULL) ? _len_dh_bytes : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_dh_key_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_dh_key_t));
	ocalloc_size -= sizeof(ms_ocall_print_dh_key_t);

	if (dh_bytes != NULL) {
		if (memcpy_verw_s(&ms->ms_dh_bytes, sizeof(uint8_t*), &__tmp, sizeof(uint8_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_dh_bytes % sizeof(*dh_bytes) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, dh_bytes, _len_dh_bytes)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_dh_bytes);
		ocalloc_size -= _len_dh_bytes;
	} else {
		ms->ms_dh_bytes = NULL;
	}

	if (memcpy_verw_s(&ms->ms_len, sizeof(ms->ms_len), &len, sizeof(len))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(2, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_t));
	ocalloc_size -= sizeof(ms_ocall_print_t);

	if (str != NULL) {
		if (memcpy_verw_s(&ms->ms_str, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}

	status = sgx_ocall(3, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_buffer(const char* buf, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = len;

	ms_ocall_print_buffer_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_buffer_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_buffer_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_buffer_t));
	ocalloc_size -= sizeof(ms_ocall_print_buffer_t);

	if (buf != NULL) {
		if (memcpy_verw_s(&ms->ms_buf, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}

	if (memcpy_verw_s(&ms->ms_len, sizeof(ms->ms_len), &len, sizeof(len))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(4, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_float(float val)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_print_float_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_float_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_float_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_float_t));
	ocalloc_size -= sizeof(ms_ocall_print_float_t);

	if (memcpy_verw_s(&ms->ms_val, sizeof(ms->ms_val), &val, sizeof(val))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(5, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

