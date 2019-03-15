
#pragma once

#include <array>
#include <bitset>

#include <boost/asio.hpp>

struct streambufstorage
{
	streambufstorage(){ storage_status.reset(); }
	static constexpr unsigned chunk_size = 1024;
	using basic_chunk = char[chunk_size];

	std::array<basic_chunk, 8> storage;
	std::bitset<8> storage_status;

	void* alloc(std::size_t s)
	{
		for (int i=0; i < storage_status.size(); i++)
			if (storage_status[i] == false)
			{
				storage_status.set(i);
				return reinterpret_cast<void*>(storage[i]);
			}
		throw std::bad_alloc{};
	}

	void dealloc(void* ptr, std::size_t s)
	{
		for (int i=0; i < storage_status.size(); i++)
			if (reinterpret_cast<void*>(storage[i]) == ptr)
			{
				storage_status.reset(i);
			}
	}

	static constexpr std::size_t max_size() {
		return sizeof(streambufstorage::storage);
	}
};

template<class T>
struct streambufalloc
{
	streambufstorage& storage;
public:
    using value_type = T;
    using is_always_equal = std::false_type;
    using pointer = T*;
    using reference = T&;
    using const_pointer = T const*;
    using const_reference = T const&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<class U>
    struct rebind
    {
        using other = streambufalloc<U>;
    };

    explicit
    streambufalloc(streambufstorage& storage)
        : storage(storage)
    {
    }

    streambufalloc(streambufalloc const& other)
        : storage(other.storage)
    {
    }

    template<class U>
    streambufalloc(streambufalloc<U> const& other)
        : storage(other.storage)
    {
    }

    ~streambufalloc()
    {
    }

    value_type*
    allocate(size_type n)
    {
        return static_cast<value_type*>(
            storage.alloc(n * sizeof(T)));
    }

    void
    deallocate(value_type* ptr, size_type s)
    {
        storage.dealloc(ptr, s);
    }

    template<class U>
    friend
    bool
    operator==(
        streambufalloc const& lhs,
        streambufalloc<U> const& rhs)
    {
        return &lhs.storage == &rhs.storage;
    }

    template<class U>
    friend
    bool
    operator!=(
        streambufalloc const& lhs,
        streambufalloc<U> const& rhs)
    {
        return ! (lhs == rhs);
    }
};
