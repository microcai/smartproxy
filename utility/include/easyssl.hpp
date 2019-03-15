#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include <openssl/x509.h>
#include <openssl/rsa.h>

/*
 * 顾名思义，这个是简单 RSA , c++ 封装，专门对付 openssl 烂接口烂源码烂文档这种弱智库的
 */

std::string RSA_public_encrypt(RSA * rsa, const std::string & from);

std::string RSA_private_decrypt(RSA * rsa, const std::string & from);

std::string RSA_private_encrypt(RSA * rsa, const std::string & from);

std::string RSA_public_decrypt(RSA * rsa, const std::string & from);

