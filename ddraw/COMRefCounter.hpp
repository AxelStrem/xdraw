#pragma once
#include "shared_com.hpp"

//
// COMRefCounter template implements IUnknown methods
// 

template<class COMInterface> bool check_single_iid(REFIID riid)
{
	return (GetIID<COMInterface>() == riid);
}

template<class... COMInterface> struct iid_checker
{
	static bool check(REFIID riid)
	{
		return false;
	}
};

template<class Head, class ... COMInterfaces> struct iid_checker<Head, COMInterfaces...>
{
	static bool check(REFIID riid)
	{
		return check_single_iid<Head>(riid) || iid_checker<COMInterfaces...>::check(riid);
	}
};

template<class COMInterface, class ... COMImplements> class COMRefCounter : public COMInterface
{
	volatile ULONG m_cRef = 1;
public:
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR * ppvObj)
	{
		// Always set out parameter to NULL, validating it first.
		if (!ppvObj)
			return E_INVALIDARG;
		*ppvObj = NULL;
		//riid = {69C11C3E-B46B-11D1-AD7A-00C04FC29B4E}
		if(iid_checker<COMInterface, IUnknown, COMImplements...>::check(riid))
		{
			// Increment the reference count and return the pointer.
			*ppvObj = (LPVOID)this;
			AddRef();
			LOG("QS\r\n");
			return NOERROR;
		}

		LOG("QF\r\n");

		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef) ()
	{
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	STDMETHOD_(ULONG, Release) ()
	{
		// Decrement the object's internal counter.
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		if (0 == m_cRef)
		{
			delete this;
		}
		return ulRefCount;
	}

	virtual ~COMRefCounter() {}
};