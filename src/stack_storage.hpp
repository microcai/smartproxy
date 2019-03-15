
#pragma once

struct stack_storage_base
{
	virtual void* heap() const = 0;
};

template<typename ElementType, int NumberOfElements>
struct stack_storage : public stack_storage_base
{
	mutable char preallocatebuf[NumberOfElements * sizeof(ElementType)];

	void* heap() const override
	{
		return preallocatebuf;
	}
};

template<typename ElementType>
struct stack_allocator
{
	typedef ElementType value_type;

	template< class U > struct rebind { typedef stack_allocator<U> other; };

	stack_allocator(stack_storage_base& storage) : stor(storage) {};

	template <class U> constexpr stack_allocator(const stack_allocator<U>& other) noexcept
		: stor(other.stor)
	{
	}

	ElementType* allocate(int n){
		return reinterpret_cast<ElementType*>(stor.heap());
	}

	void deallocate(void * ptr,int n){}

	stack_storage_base& stor;
};
