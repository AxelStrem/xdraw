//
// shared_com<I> : a smart shared pointer for COM interfaces
//

#pragma once
#include <type_traits>
#include <Unknwn.h>

template <class T> inline IID GetIID()
{
	return __uuidof(T);
}

#define SETUP_IID(interface_name) template<> inline IID GetIID< interface_name >() { return IID_##interface_name; }

template<class I> class remove_iunknown : public I
{
public:
	void AddRef(int) = delete;
	void Release(int) = delete;
	void QueryInterface() = delete;
};

template<class I> class shared_com
{
	I* mPtr = nullptr;

	void Release()
	{
		if (mPtr)
			mPtr->Release();
	}
public:
	typedef I* Iptr;

	~shared_com()
	{
		Release();
	}
	
    shared_com()  noexcept
    {}
	
    shared_com(nullptr_t)  noexcept
    {}

	template<class T>
	shared_com(T* ptr, typename std::enable_if<std::is_convertible<T*, I*>::value,int>::type tag = 0)  noexcept : mPtr(ptr)
	{
    
    }

    template<class T>
	shared_com(T* ptr, typename std::enable_if<std::conjunction<std::is_convertible<T*,IUnknown*>,
		                                                        std::negation<std::is_convertible<T*,I*>>>::value,
		                                       int>::type tag = 0)  noexcept
	{
		if (ptr->QueryInterface(GetIID<I>(), (void**)(&mPtr)) != S_OK)
			mPtr = nullptr;
		else
			ptr->Release();
	}

	template<class T>
	shared_com(shared_com<T>& source, typename std::enable_if<std::is_convertible<T*, I*>::value, int>::type tag = 0) : mPtr(source.get())
	{
		mPtr->AddRef();
	}

	template<class T> 
	shared_com(shared_com<T>& source, typename std::enable_if<std::conjunction<std::is_convertible<T*, IUnknown*>,
		                                                                       std::negation<std::is_convertible<T*, I*>>>::value,
		                                                      int>::type tag = 0)
	{
		if (source.get()->QueryInterface(GetIID<I>(), (void**)(&mPtr)) != S_OK)
			mPtr = nullptr;
	}

	template<class T>
	shared_com(shared_com<T>&& source)  noexcept : shared_com(source.get())
	{
		if(mPtr)
			source.get() = nullptr;
	}

	Iptr& get() noexcept
	{
		return mPtr;
	}

	Iptr* operator&() noexcept
	{
		return &mPtr;
	}

	remove_iunknown<I>* operator->() const noexcept
	{
		return static_cast<remove_iunknown<I>*>(mPtr);
	}

	operator bool() const noexcept
	{
		return mPtr != nullptr;
	}

	template<class T>
	shared_com& operator=(T&& source) noexcept
	{
		shared_com temp(std::forward<T&&>(source));
		std::swap(mPtr, temp.mPtr);
		return *this;
	}
};