// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PEXCEPTION_H_
#define PEXCEPTION_H_

///
/// \file: pexception.h
///

#include "pstring.h"
#include "ptypes.h"

#include <exception>

#define passert_always(expr)							\
  ((expr) ? static_cast<void>(0) : plib::passert_fail (#expr, __FILE__, __LINE__, nullptr))

#define passert_always_msg(expr, msg)							\
  ((expr) ? static_cast<void>(0) : plib::passert_fail (#expr, __FILE__, __LINE__, msg))

namespace plib {

	/// \brief Terminate the program.
	///
	/// \note could be enhanced by setting a termination handler
	///
	[[noreturn]] void terminate(const char *msg) noexcept;

	[[noreturn]] void passert_fail(const char *assertion,
		const char *file, int lineno, const char *msg) noexcept;

	/// \brief throw an exception.
	///
	/// throws an exception E. The purpose is to clearly identify exception
	/// throwing in the code
	///
	/// \tparam E Type of exception to be thrown
	///
	template<typename E, typename... Args>
	[[noreturn]] static inline void pthrow(Args&&... args) noexcept(false)
	{
		throw E(std::forward<Args>(args)...);
	}

	//============================================================
	//  exception base
	//============================================================

	class pexception : public std::exception
	{
	public:
		explicit pexception(const pstring &text);

		const pstring &text() const noexcept { return m_text; }
		const char* what() const noexcept override { return m_text.c_str(); }

	private:
		pstring m_text;
	};

	class file_e : public plib::pexception
	{
	public:
		file_e(const pstring &fmt, const pstring &filename);
	};

	class file_open_e : public file_e
	{
	public:
		explicit file_open_e(const pstring &filename);
	};

	class file_read_e : public file_e
	{
	public:
		explicit file_read_e(const pstring &filename);
	};

	class file_write_e : public file_e
	{
	public:
		explicit file_write_e(const pstring &filename);
	};

	class null_argument_e : public plib::pexception
	{
	public:
		explicit null_argument_e(const pstring &argument);
	};

	class out_of_mem_e : public plib::pexception
	{
	public:
		explicit out_of_mem_e(const pstring &location);
	};

	// FIXME: currently only a stub for later use. More use could be added by
	// using “-fnon-call-exceptions" and sigaction to enable c++ exception supported.
	//

	class fpexception_e : public pexception
	{
	public:
		explicit fpexception_e(const pstring &text);
	};

	static constexpr unsigned FP_INEXACT = 0x0001;
	static constexpr unsigned FP_DIVBYZERO = 0x0002;
	static constexpr unsigned FP_UNDERFLOW = 0x0004;
	static constexpr unsigned FP_OVERFLOW = 0x0008;
	static constexpr unsigned FP_INVALID = 0x00010;
	static constexpr unsigned FP_ALL = 0x0001f;

	/// \brief Catch SIGFPE on linux for debugging purposes.
	///
	class fpsignalenabler
	{
	public:
		explicit fpsignalenabler(unsigned fpexceptions);

		COPYASSIGNMOVE(fpsignalenabler, delete)

		~fpsignalenabler();

		/// \brief is the functionality supported.
		///
		/// \return current status
		///
		static bool supported();
		/// \brief get/set global enable status
		///
		/// \param enable new status
		/// \return returns last global enable state
		///
		static bool global_enable(bool enable);

	private:
		int m_last_enabled;

		static bool m_enable;
	};


} // namespace plib

#endif // PEXCEPTION_H_
