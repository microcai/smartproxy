#include <string.h>
#include <algorithm>
#include <openssl/pem.h>
#include "easyssl.hpp"

std::string RSA_public_encrypt(RSA* rsa, const std::string& from)
{
    std::string result;
    const int keysize = RSA_size(rsa);
    std::vector<unsigned char> block(keysize);
    const int chunksize = keysize  - RSA_PKCS1_PADDING_SIZE;
    const auto inputlen = static_cast<int>(from.length());

    for(int i = 0 ; i < inputlen; i+= chunksize)
    {
        auto resultsize = RSA_public_encrypt(std::min(chunksize, inputlen - i), (uint8_t*) &from[i],  &block[0], (RSA*) rsa, RSA_PKCS1_PADDING);
        result.append((char*)block.data(), resultsize);
    }
    return result;
}

std::string RSA_private_decrypt(RSA* rsa, const std::string& from)
{
    std::string result;
    const int keysize = RSA_size(rsa);
    std::vector<unsigned char> block(keysize);

    for(int i = 0 ; i < from.length(); i+= keysize)
    {
        auto resultsize = RSA_private_decrypt(std::min(keysize, static_cast<int>(from.length() - i)),
			(uint8_t*) &from[i],  &block[0], rsa, RSA_PKCS1_PADDING);
		if (resultsize > 0)
		{
			result.append((char*)block.data(), resultsize);
		}
		else
		{
			return "";
		}

    }
    return result;
}

std::string RSA_private_encrypt(RSA* rsa, const std::string& from)
{
    std::string result;
    const int keysize = RSA_size(rsa);
    std::vector<unsigned char> block(keysize);
    const int chunksize = keysize  - RSA_PKCS1_PADDING_SIZE;
    const auto inputlen = static_cast<int>(from.length());

    for(int i = 0 ; i < from.length(); i+= chunksize)
    {
        int flen = std::min<int>(chunksize, inputlen - i);

        std::fill(block.begin(),block.end(), 0);

        auto resultsize = RSA_private_encrypt(
                              flen,
                              (uint8_t*) &from[i],
                              &block[0],
                              rsa,
                              RSA_PKCS1_PADDING
                          );
        result.append((char*)block.data(), resultsize);
    }
    return result;
}

std::string RSA_public_decrypt(RSA* rsa, const std::string& from)
{
    std::string result;
    const int keysize = RSA_size(rsa);
    std::vector<unsigned char> block(keysize);

    const auto inputlen = static_cast<int>(from.length());

    for(int i = 0 ; i < from.length(); i+= keysize)
    {
        int flen = std::min(keysize, inputlen - i);

        auto resultsize = RSA_public_decrypt(
                              flen,
                              (uint8_t*) &from[i],
                              &block[0],
                              rsa,
                              RSA_PKCS1_PADDING
                          );
        result.append((char*)block.data(), resultsize);
    }
    return result;
}
