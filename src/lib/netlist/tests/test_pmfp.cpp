// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_pmfp.cpp
///
/// tests for plib::pmfp
///

#include "plib/ptests.h"

#include "plib/pexception.h"
#include "plib/ppmf.h"

#include <utility>
#include <memory>

///     `plib::late_pmfp<plib::pmfp<void, pstring>> a(&nld_7493::printer);`
///     // Store the a object somewhere
///
///     // After full construction ...
///
/// ```
///     auto dele = a(this);
///     dele(pstring("Hello World!"));
/// ```

class test_late_pmfp : public plib::testing::Test
{
protected:
	class Ta
	{
	public:
		void ap(int &r) { r = ax; }
		int ax;
	};

};

PTEST_F(test_late_pmfp, late_pmfp)
{
#if !PPMF_USE_MAME_DELEGATES
	plib::late_pmfp<plib::pmfp<void (int &)>> late(&Ta::ap);

	Ta a;
	a.ax = 1;
	auto mfp(late(&a));
	int r(0);
	mfp(r);
	PEXPECT_TRUE(r == 1);
#endif
}

PTEST(test_compile, compile)
{
#if !PPMF_USE_MAME_DELEGATES
	plib::pmfp_base<plib::ppmf_type::PMF, void(int)> mfp_PPMF_TYPE_PMF;
	plib::pmfp_base<plib::ppmf_type::INTERNAL_ITANIUM, void(int)> mfp_PPMF_TYPE_INTERNAL_ITANIUM;
	plib::pmfp_base<plib::ppmf_type::INTERNAL_ARM, void(int)> mfp_PPMF_TYPE_INTERNAL_ARM;
	plib::pmfp_base<plib::ppmf_type::INTERNAL_MSC, void(int)> mfp_PPMF_TYPE_INTERNAL_MSC;
	PEXPECT_TRUE(mfp_PPMF_TYPE_PMF.isnull());
	PEXPECT_TRUE(mfp_PPMF_TYPE_INTERNAL_ITANIUM.isnull());
	PEXPECT_TRUE(mfp_PPMF_TYPE_INTERNAL_ARM.isnull());
	PEXPECT_TRUE(mfp_PPMF_TYPE_INTERNAL_MSC.isnull());
#endif

#if defined(__GNUC__) && !defined(__clang__) && !defined(__NVCC__)
	plib::pmfp_base<plib::ppmf_type::GNUC_PMF_CONV, void(int)> mfp_PPMF_TYPE_GNUC_PMF_CONV;
	PEXPECT_TRUE(mfp_PPMF_TYPE_GNUC_PMF_CONV.isnull());
#else
	PEXPECT_NE("ppmf_type::GNUC_PMF_CONV not supported on this build", "");
#endif
#if defined(__EMSCRIPTEN__)
	PEXPECT_EQ(plib::ppmf_internal_selector::value,plib::ppmf_type::INTERNAL_ARM);
#endif
}
