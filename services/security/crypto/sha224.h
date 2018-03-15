/*
 * sha224.h
 *
 *  Created on: Jul 17, 2017
 *     License: see LICENSE file
 */

#ifndef SERVICES_SECURITY_CRYPTO_SHA224_H_
#define SERVICES_SECURITY_CRYPTO_SHA224_H_

#include <tmos.h>
#include <sha256.h>

#define SHA224_BLOCK_SIZE 64
#define SHA224_DIGEST_SIZE 28

extern const hash_info_t SHA224_HASH_ALGO[1];


struct sha224_algo_t: public sha256_algo_t
{

    sha224_algo_t() : sha256_algo_t(SHA224_HASH_ALGO)
	{
		Reset();
	}

	virtual void Reset();
};

hash_algo_t* new_sha224_hash();




#endif /* SERVICES_SECURITY_CRYPTO_SHA224_H_ */