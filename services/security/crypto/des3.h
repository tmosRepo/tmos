/*
 * des3.h
 *
 *  Created on: Aug 29, 2017
 *     License: see LICENSE file
 */

#ifndef SERVICES_SECURITY_CRYPTO_DES3_H_
#define SERVICES_SECURITY_CRYPTO_DES3_H_


#include <tmos.h>
#include <des.h>


#define DES3_BLOCK_SIZE 8

struct des3_cipher_algo_t: cipher_algo_t
{
	uint32_t ks1[32];
	uint32_t ks2[32];
	uint32_t ks3[32];

	des3_cipher_algo_t(const cipher_info_t* info) :
			cipher_algo_t(info)
	{
	};

	RES_CODE init_key(const uint8_t* key, size_t key_size) override;
	RES_CODE get_kcv(const uint8_t* key, size_t key_size, uint8_t* output);
	void encrypt(const void* input, void* output, size_t blocks) override;
	void decrypt(const void* input, void* output, size_t blocks) override;
};

cipher_algo_t* new_cipher_des3();





#endif /* SERVICES_SECURITY_CRYPTO_DES3_H_ */
