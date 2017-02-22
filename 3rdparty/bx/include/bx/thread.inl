/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_THREAD_H_HEADER_GUARD
#	error "Must be included from bx/thread.h!"
#endif // BX_THREAD_H_HEADER_GUARD

#if BX_CONFIG_SUPPORTS_THREADING

namespace bx
{
#if BX_PLATFORM_WINDOWS
	inline TlsData::TlsData()
	{
		m_id = TlsAlloc();
		BX_CHECK(TLS_OUT_OF_INDEXES != m_id, "Failed to allocated TLS index (err: 0x%08x).", GetLastError() );
	}

	inline TlsData::~TlsData()
	{
		BOOL result = TlsFree(m_id);
		BX_CHECK(0 != result, "Failed to free TLS index (err: 0x%08x).", GetLastError() ); BX_UNUSED(result);
	}

	inline void* TlsData::get() const
	{
		return TlsGetValue(m_id);
	}

	inline void TlsData::set(void* _ptr)
	{
		TlsSetValue(m_id, _ptr);
	}

#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)

	inline TlsData::TlsData()
	{
		int result = pthread_key_create(&m_id, NULL);
		BX_CHECK(0 == result, "pthread_key_create failed %d.", result); BX_UNUSED(result);
	}

	inline TlsData::~TlsData()
	{
		int result = pthread_key_delete(m_id);
		BX_CHECK(0 == result, "pthread_key_delete failed %d.", result); BX_UNUSED(result);
	}

	inline void* TlsData::get() const
	{
		return pthread_getspecific(m_id);
	}

	inline void TlsData::set(void* _ptr)
	{
		int result = pthread_setspecific(m_id, _ptr);
		BX_CHECK(0 == result, "pthread_setspecific failed %d.", result); BX_UNUSED(result);
	}
#endif // BX_PLATFORM_*

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING
