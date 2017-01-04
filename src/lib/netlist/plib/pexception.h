// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PEXCEPTION_H_
#define PEXCEPTION_H_

#include <exception>

#include "pconfig.h"
#include "pstring.h"

namespace plib {
//============================================================
//  exception base
//============================================================

class pexception : public std::exception
{
public:
	pexception(const pstring text);
	pexception(const pexception &e) : std::exception(e) { m_text = e.m_text; }

	virtual ~pexception() noexcept {}

	const pstring &text() { return m_text; }

private:
	pstring m_text;
};

class file_e : public plib::pexception
{
public:
	explicit file_e(const pstring fmt, const pstring &filename);
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

/* FIXME: currently only a stub for later use. More use could be added by
 * using “-fnon-call-exceptions" and sigaction to enable c++ exception supported.
 */

class fpexception : public pexception
{
public:
	fpexception(const pstring &text);
};

static const unsigned FP_INEXACT = 0x0001;
static const unsigned FP_DIVBYZERO = 0x0002;
static const unsigned FP_UNDERFLOW = 0x0004;
static const unsigned FP_OVERFLOW = 0x0008;
static const unsigned FP_INVALID = 0x00010;
static const unsigned FP_ALL = 0x0001f;

/*
 * Catch SIGFPE on linux for debugging purposes.
 */

class fpsignalenabler
{
public:
	fpsignalenabler(unsigned fpexceptions);
	~fpsignalenabler();

	/* is the functionality supported ? */
	static bool supported();
	/* returns last global enable state */
	static bool global_enable(bool enable);

private:
	int m_last_enabled;

	static bool m_enable;
};


}

#endif /* PEXCEPTION_H_ */
