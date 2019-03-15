
#pragma once

#include <string>
#include <boost/archive/iterators/transform_width.hpp>

#include "openssl/aes.h"

namespace crypto_util
{
	std::string base64_encode(std::string);

	std::string md5sum(const std::string& sign_data);

	std::string md5sum_raw(const std::string& sign_data);

	std::string sha1_sum(const std::string& in);

	std::string sha256_sum(const std::string& in);

	template<typename STRING>
	inline std::string sign_json(STRING&& json, STRING&& time, STRING&& security_token)
	{
		return md5sum(json+time+security_token);
	}

	template<int length = 20>
	inline std::string base32_encode(unsigned char in[length])
	{
		static const auto base32_chars = "ABCDEFGHJKNPQRSTUVWXYZ0123456789";

		std::string ret;
		ret.reserve(length * 8 /5);

		boost::archive::iterators::transform_width<const char*, 5, 8> in_it(in);
		boost::archive::iterators::transform_width<const char*, 5, 8> in_it_end(in + length);

		for (; in_it != in_it_end; ++ in_it)
		{
			ret.push_back(base32_chars[*in_it]);
		}

		return ret;
	}

	std::string sha1_sum_base32(const std::string& in);

	enum { SHA1WithRSA, SHA256WithRSA, MD5WithRSA };
	enum class encoding
	{
		base64,
		hex,
		raw,
	};

	std::string rsa_sign(const std::string& content, const std::string& key, int type = SHA1WithRSA);
	bool rsa_verify(const std::string& content, const std::string& sign, const std::string& key, int type = SHA1WithRSA);

	std::string rsa_pub_enc(const std::string& content, const std::string& key);
	std::string rsa_priv_dec(const std::string& content, const std::string& key);

	std::string hmac_md5_sign(const std::string& content, const std::string& key);
	std::string hmac_sha1_sign(const std::string& content, const std::string& key, encoding output_encoding = encoding::base64);
	std::string hmac_sha512_sign(const std::string& content, const std::string& key);

	// 对 content 进行hash操作，生成一个 16 个字符长的，由纯数字构成的 hash.
	std::string simple_num_hash(const std::string& content, int l = 16);

	std::string aes_cbc_encrypt(std::string d, std::string key, std::string iv);
	std::string aes_cbc_decrypt(std::string in, std::string key, std::string iv);

	std::string aes_ecb_encrypt(std::string d, const AES_KEY& key);
	std::string aes_ecb_decrypt(std::string in, const AES_KEY& key);

	std::vector<unsigned char> aes_ecb_encrypt(std::vector<unsigned char>, const AES_KEY& key);
	std::vector<unsigned char> aes_ecb_decrypt(std::vector<unsigned char>, const AES_KEY& key);

	std::string aes_ecb_encrypt(std::string d, std::string key, encoding output_encoding = encoding::base64);
	std::string aes_ecb_decrypt(std::string in, std::string key, encoding input_encoding = encoding::base64);

	std::string encrypt_des_cbc(const std::string& data, const std::string& key, const std::string& iv);
	std::string decrypt_des_cbc(const std::string& data, const std::string& key, const std::string& iv);
}
