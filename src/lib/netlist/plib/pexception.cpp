// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "pexception.h"
#include "pfmtlog.h"

#include <cfenv>
#include <cfloat>
#include <iostream>

#if (defined(__x86_64__) || defined(__i386__)) && defined(__linux__)
#define HAS_FEENABLE_EXCEPT     (1)
#else
#define HAS_FEENABLE_EXCEPT     (0)
#endif

namespace plib {

	//============================================================
	// terminate
	//============================================================

	void terminate(const char *msg) noexcept
	{
		try
		{
			std::cerr << msg << "\n";
		}
		catch (...)
		{
			/* ignore */
		}
		std::terminate();
	}

	void passert_fail(const char *assertion, const char *file, int lineno, const char *msg) noexcept
	{
		try
		{
			std::cerr << file << ":" << lineno << ": ";
			if (msg != nullptr)
				std::cerr << msg << "\n";
			else
				std::cerr << "Assertion '" << assertion << "' failed.\n";
		}
		catch (...)
		{
			/* ignore */
		}
		std::terminate();
	}

	//============================================================
	//  Exceptions
	//============================================================

	pexception::pexception(const pstring &text)
	: m_text(text)
	{
	}


	file_e::file_e(const pstring &fmt, const pstring &filename)
		: pexception(pfmt(fmt)(filename))
	{
	}


	file_open_e::file_open_e(const pstring &filename)
		: file_e("File open failed: {}", filename)
	{
	}


	file_read_e::file_read_e(const pstring &filename)
		: file_e("File read failed: {}", filename)
	{
	}


	file_write_e::file_write_e(const pstring &filename)
		: file_e("File write failed: {}", filename)
	{
	}


	null_argument_e::null_argument_e(const pstring &argument)
		: pexception(pfmt("Null argument passed: {}")(argument))
	{
	}


	out_of_mem_e::out_of_mem_e(const pstring &location)
		: pexception(pfmt("Out of memory: {}")(location))
	{
	}


	fp_exception_e::fp_exception_e(const pstring &text)
		: pexception(pfmt("Exception error: {}")(text))
	{
	}


	bool fp_signal_enabler::m_enable = false; // NOLINT

	//FIXME: mingw needs to be compiled with `-fnon-call-exceptions`

	fp_signal_enabler::fp_signal_enabler(unsigned fpexceptions)
	{
	#if HAS_FEENABLE_EXCEPT
		if (m_enable)
		{
			int b = 0;
			if (fpexceptions & plib::FP_INEXACT) b = b | FE_INEXACT;
			if (fpexceptions & plib::FP_DIVBYZERO) b = b | FE_DIVBYZERO;
			if (fpexceptions & plib::FP_UNDERFLOW) b = b | FE_UNDERFLOW;
			if (fpexceptions & plib::FP_OVERFLOW) b = b | FE_OVERFLOW;
			if (fpexceptions & plib::FP_INVALID) b = b | FE_INVALID;
			if ((b & m_last_enabled) != b)
				m_last_enabled = feenableexcept(b);
		}
	#elif defined(_WIN32) && defined(_EM_INEXACT)
		if (m_enable)
		{
			int b = _EM_DENORMAL | _EM_INEXACT | _EM_ZERODIVIDE | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_INVALID;
			if (fpexceptions & plib::FP_INEXACT) b &= ~_EM_INEXACT;
			if (fpexceptions & plib::FP_DIVBYZERO) b &= ~_EM_ZERODIVIDE;
			if (fpexceptions & plib::FP_UNDERFLOW) b &= ~_EM_UNDERFLOW;
			if (fpexceptions & plib::FP_OVERFLOW) b &= ~_EM_OVERFLOW;
			if (fpexceptions & plib::FP_INVALID) b &= ~_EM_INVALID;
			m_last_enabled = _controlfp(0, 0);
			_controlfp(b, _MCW_EM );
		}
	#else
		m_last_enabled = 0;
	#endif
	}

	fp_signal_enabler::~fp_signal_enabler()
	{
	#if HAS_FEENABLE_EXCEPT
		if (m_enable)
		{
			fedisableexcept(FE_ALL_EXCEPT);  // Enable all floating point exceptions but FE_INEXACT
			feenableexcept(m_last_enabled);  // Enable all floating point exceptions but FE_INEXACT
		}
	#elif defined(_WIN32) && defined(_EM_INEXACT)
		if (m_enable)
		{
			_controlfp(m_last_enabled, _MCW_EM);
		}
	#endif
	}

	bool fp_signal_enabler::supported()
	{
	#if HAS_FEENABLE_EXCEPT
		return true;
	#elif defined(_WIN32) && defined(_EM_INEXACT)
		return true;
	#else
		return false;
	#endif
	}

	bool fp_signal_enabler::global_enable(bool enable)
	{
		bool old = m_enable;
		m_enable = enable;
		return old;
	}


} // namespace plib
