/*
 * des3.cpp
 *
 *  Created on: Aug 29, 2017
 *     License: see LICENSE file
 */

#include <tmos.h>
#include <des3.h>

#if DES3_SUPPORT

const cipher_info_t des3_cipher_info =
{
   "3DES",
   DES3_BLOCK_SIZE,
   CIPHER_TYPE_BLOCK
};


cipher_algo_t* new_cipher_des3()
{
	cipher_algo_t* algo;

	algo = new des3_cipher_algo_t(&des3_cipher_info);
	return algo;
}

RES_CODE des3_cipher_algo_t::init_key(const uint8_t* key, size_t key_size)
{
	switch(key_size)
	{
	case 8:
	      //This option provides backward compatibility with DES, because the
	      //first and second DES operations cancel out
	      des_init_key(ks1, key);
	      des_init_key(ks2, key);
	      des_init_key(ks3, key);
		break;

	case 16:
	      //If the key length is 128 bits including parity, the first 8 bytes of the
	      //encoding represent the key used for the two outer DES operations, and
	      //the second 8 bytes represent the key used for the inner DES operation
	      des_init_key(ks1, key);
	      des_init_key(ks2, key + 8);
	      des_init_key(ks3, key);
		break;

	case 24:
	      //If the key length is 192 bits including parity, then three independent DES
	      //keys are represented, in the order in which they are used for encryption
	      des_init_key(ks1, key);
	      des_init_key(ks2, key + 8);
	      des_init_key(ks3, key + 16);
		break;

	default:
		return RES_TLS_INVALID_KEY_LENGTH;
	}

	return RES_OK;
}

/**
 * KCV: Key Checksum Value. it is a simple checksum calculated on a plaintext
 * key to check its validity after it's encrypted. to calculate it, use the key
 * to encrypt an empty message (all bytes set to 0) and get the first 3 bytes of
 * the result - that's the KCV
 * @param key: array containing the plaintext key whose KCV we want
 * @param key_size: length of the key in bytes
 * @param output: where to store the result. must be at least 3 free bytes
 * @return RES_OK if successful
 */
RES_CODE des3_cipher_algo_t::get_kcv(const uint8_t* key, size_t key_size, uint8_t* output)
{
	RES_CODE res;
	const uint8_t zeros[] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint32_t* ks1_bkp;
	uint32_t* ks2_bkp;
	uint32_t* ks3_bkp;
	uint8_t* result;

	ks1_bkp = new uint32_t[32];
	if(!ks1_bkp)
		return RES_OUT_OF_MEMORY;

	ks2_bkp = new uint32_t[32];
	if(!ks2_bkp)
		return RES_OUT_OF_MEMORY;

	ks3_bkp = new uint32_t[32];
	if(!ks3_bkp)
		return RES_OUT_OF_MEMORY;

	result = new uint8_t[8];
	if(!result)
		return RES_OUT_OF_MEMORY;

	//backup the keys
	memcpy(ks1_bkp, ks1, sizeof(ks1));
	memcpy(ks2_bkp, ks2, sizeof(ks2));
	memcpy(ks3_bkp, ks3, sizeof(ks3));

	//temporarily assign a new key
	res = init_key(key, key_size);

	if(res == RES_OK)
	{
		//encrypt 8 bytes of zeroes using the new key
		encrypt(zeros, result, 1);

		//and get the first 3 bytes of the result. that's the KCV
		memcpy(output, result, 3);
	}

	//restore the keys
	memcpy(ks1, ks1_bkp, sizeof(ks1));
	memcpy(ks2, ks2_bkp, sizeof(ks2));
	memcpy(ks3, ks3_bkp, sizeof(ks3));

	//delete the backups
	delete[] ks1_bkp;
	delete[] ks2_bkp;
	delete[] ks3_bkp;
	delete[] result;

	return res;
}

void des3_cipher_algo_t::encrypt(const void* input, void* output, size_t blocks)
{
	while(blocks--)
	{
		//The first pass is a DES encryption
		des_crypt_block(ks1, 2, (const uint32_t*)input, (uint32_t*)output);
		//The second pass is a DES decryption of the first ciphertext result
		des_crypt_block(ks2+30, -2, (uint32_t*)output, (uint32_t*)output);
		//The third pass is a DES encryption of the second pass result
		des_crypt_block(ks3, 2, (uint32_t*)output, (uint32_t*)output);

		input = (const uint8_t*)input + DES3_BLOCK_SIZE;
		output = (uint8_t*)output + DES3_BLOCK_SIZE;
	}
}
void des3_cipher_algo_t::decrypt(const void* input, void* output, size_t blocks)
{
	while(blocks--)
	{
		//The first pass is a DES decryption
		des_crypt_block(ks3+30, -2, (const uint32_t*)input, (uint32_t*)output);
		//The second pass is a DES encryption of the first pass result
		des_crypt_block(ks2, 2, (uint32_t*)output, (uint32_t*)output);
		//The third pass is a DES decryption of the second ciphertext result
		des_crypt_block(ks1+30, -2, (uint32_t*)output, (uint32_t*)output);

		input = (const uint8_t*)input + DES3_BLOCK_SIZE;
		output = (uint8_t*)output + DES3_BLOCK_SIZE;
	}
}

#endif // DES3_SUPPORT
