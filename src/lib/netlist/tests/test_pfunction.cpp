// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_pfunction.cpp
///
/// tests for pfunction
///

#include "plib/ptests.h"

#include "plib/pexception.h"
#include "plib/pfunction.h"

#define PFUNCEXPECT(formula, val) \
	PEXPECT_EQ(val, plib::pfunction<double>(formula)())

PTEST(pfunction, operators)
{
	PFUNCEXPECT("1==1", 1.0);
	PFUNCEXPECT("1 *0 == 2-1-1", 1.0);
	PFUNCEXPECT("0!=1", 1.0);
	PFUNCEXPECT("0<1",  1.0);
	PFUNCEXPECT("1>0",  1.0);
	PFUNCEXPECT("0<=1", 1.0);
	PFUNCEXPECT("1>=0", 1.0);
	PFUNCEXPECT("1<=1", 1.0);
	PFUNCEXPECT("1>=1", 1.0);
	PEXPECT_EQ(1.0, plib::pfunction<double>("0!=a", {"a"})({1.0}));
}

PTEST(pfunction, func_if)
{
	PFUNCEXPECT("if(1>0, 2, 0)", 2.0);
	PFUNCEXPECT("if(0>1, 2, 3)", 3.0);
	PFUNCEXPECT("if(sin(1)>0, 2, 3)", 2.0); // fail
	PEXPECT_EQ( 1.0,   plib::pfunction<double>("if(A2>2.5, 0-A1, (0.07-(0.005*A1))*if(A0>2.5,1,0-1))", {"A0","A1","A2"})({1.0,-1.0,3.0}));
	PEXPECT_EQ(-0.065, plib::pfunction<double>("if(A2>2.5, 0-A1, (0.07-(0.005*A1))*if(A0>2.5,1,0-1))", {"A0","A1","A2"})({1.0,1.0,1.0}));
	PEXPECT_EQ( 0.065, plib::pfunction<double>("if(A2>2.5, 0-A1, (0.07-(0.005*A1))*if(A0>2.5,1,0-1))", {"A0","A1","A2"})({3.0,1.0,1.0}));
	PEXPECT_TRUE(1.0 == plib::pfunction<double>("0!=a", {"a"})({1.0}));
}

PTEST(pfunction, unary_minus)
{
	PFUNCEXPECT("-1>-2", 1.0);
	PFUNCEXPECT("(-3)*(-4)", 12.0);
	PFUNCEXPECT("(-3)*-4", 12.0);
	PFUNCEXPECT("-3*-4", 12.0);
	PEXPECT_EQ( -3.0, plib::pfunction<double>("-A0", {"A0"})({3.0}));
	PFUNCEXPECT("3*-trunc(3.2)", -9.0);
	PFUNCEXPECT("3*-(3*2)", -18.0);
	PFUNCEXPECT("3*-(2*1)^2", -12.0);
	PEXPECT_NO_THROW(plib::pfunction<double>("(-3)")()); // fail
}
PTEST(pfunction, expect_throw)
{
	PEXPECT_THROW(plib::pfunction<double>("(3, 4)")(), plib::pexception);
	PEXPECT_THROW(plib::pfunction<double>("((3)")(), plib::pexception);
	PEXPECT_THROW(plib::pfunction<double>("(3))")(), plib::pexception);
}
