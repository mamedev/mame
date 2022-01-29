// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_pstring.cpp
///
/// tests for pstring
///

#include "plib/ptests.h"

#include "plib/pexception.h"
#include "plib/pstring.h"

PTEST(pstring, conversion)
{
	PEXPECT_EQ( putf8string("Общая ком"), putf8string(putf16string(putf8string("Общая ком"))));
	PEXPECT_EQ( putf8string("Общая ком"), putf8string(putf16string("Общая ком")));
}
