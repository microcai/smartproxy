
#pragma once

#include <array>
#include <cstring>

class secure_string
{
public:
	template<int len>
	secure_string(const char password[len]) {
		static_assert(len < 1000, "string too long");

		std::memset(&secure_store[0], 0, sizeof(secure_store));
		std::memmove(&secure_store[0], password, len);
	}

	secure_string(const char* len)
	{
		std::memset(&secure_store[0], 0, sizeof(secure_store));
		std::strcpy(&secure_store[0], len);
	}

	~secure_string(){
		std::memset(&secure_store[0], 0, sizeof(secure_store));
	}

	secure_string(secure_string&& moved)
	{
		std::memmove(&secure_store[0], &(moved.secure_store[0]), sizeof(secure_store));
	}

	secure_string(const secure_string& copy_from)
	{
		std::memmove(&secure_store[0], &(copy_from.secure_store[0]), sizeof(secure_store));
	}

	secure_string& operator = (const secure_string& copy_from)
	{
		std::memset(&secure_store[0], 0, sizeof(secure_store));
		std::memmove(&secure_store[0], &(copy_from.secure_store[0]), sizeof(secure_store));
		return * this;
	}

	secure_string& operator = (secure_string&& copy_from)
	{
		std::memset(&secure_store[0], 0, sizeof(secure_store));
		std::memmove(&secure_store[0], &(copy_from.secure_store[0]), sizeof(secure_store));
		return * this;
	}

	int size() {
		return strnlen(secure_store.data(), sizeof(secure_store));
	}

	const char* c_str() {
		return secure_store.data();
	}


private:
	std::array<char, 1000> secure_store;
};
