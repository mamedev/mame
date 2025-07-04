// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Paul Priest
/***************************************************************************

    validity.cpp

    Validity checks on internal data structures.

***************************************************************************/

#include "emu.h"
#include "validity.h"

#include "emuopts.h"
#include "main.h"
#include "romload.h"
#include "speaker.h"
#include "video/rgbutil.h"

#include "corestr.h"
#include "path.h"
#include "unicode.h"

#include <cctype>
#include <sstream>
#include <type_traits>
#include <typeinfo>


namespace {

//-------------------------------------------------
//  diamond_inheritance - forward declaration of a
//  class to force MSVC to use unknown inheritance
//  form of pointers to member functions
//-------------------------------------------------

class diamond_inheritance;


//-------------------------------------------------
//  test_delegate - a delegate that can return a
//  result in a register
//-------------------------------------------------

using test_delegate = delegate<char (void const *&)>;


//-------------------------------------------------
//  make_diamond_class_delegate - make a delegate
//  bound to an instance of an incomplete class
//  type
//-------------------------------------------------

test_delegate make_diamond_class_delegate(char (diamond_inheritance::*func)(void const *&), diamond_inheritance *obj)
{
	return test_delegate(func, obj);
}


//-------------------------------------------------
//  virtual_base - simple class that will be used
//  as the top vertex of the diamond
//-------------------------------------------------

struct virtual_base
{
	char get_base(void const *&p) { p = this; return 'x'; }
	int x;
};


//-------------------------------------------------
//  virtual_derived_a - first class derived from
//  virtual base
//-------------------------------------------------

struct virtual_derived_a : virtual virtual_base
{
	char get_derived_a(void const *&p) { p = this; return 'a'; }
	int a;
};


//-------------------------------------------------
//  virtual_derived_b - second class derived from
//  virtual base
//-------------------------------------------------

struct virtual_derived_b : virtual virtual_base
{
	char get_derived_b(void const *&p) { p = this; return 'b'; }
	int b;
};


//-------------------------------------------------
//  diamond_inheritance - actual definition of
//  class with diamond inheritance
//-------------------------------------------------

class diamond_inheritance : public virtual_derived_a, public virtual_derived_b
{
};


//-------------------------------------------------
//  pure_virtual_base - abstract class with a
//  vtable
//-------------------------------------------------

struct pure_virtual_base
{
	virtual ~pure_virtual_base() = default;
	virtual char operator()(void const *&p) const = 0;
};


//-------------------------------------------------
//  ioport_string_from_index - return an indexed
//  string from the I/O port system
//-------------------------------------------------

inline char const *ioport_string_from_index(u32 index)
{
	return ioport_configurer::string_from_token(reinterpret_cast<char const *>(uintptr_t(index)));
}


//-------------------------------------------------
//  random_u64
//  random_s64
//  random_u32
//  random_s32
//-------------------------------------------------
#undef rand
inline u32 random_u32() { return rand() ^ (rand() << 15); }
inline s32 random_i32() { return s32(random_u32()); }
inline u64 random_u64() { return u64(random_u32()) ^ (u64(random_u32()) << 30); }
inline s64 random_i64() { return s64(random_u64()); }


//-------------------------------------------------
//  validate_integer_semantics - validate that
//  integers behave as expected, particularly
//  with regards to overflow and shifting
//-------------------------------------------------

void validate_integer_semantics()
{
	// basic system checks
	if (~0 != -1) osd_printf_error("Machine must be two's complement\n");

	u8 a = 0xff;
	u8 b = a + 1;
	if (b > a) osd_printf_error("u8 must be 8 bits\n");

	// check size of core integer types
	if (sizeof(s8)  != 1) osd_printf_error("s8 must be 8 bits\n");
	if (sizeof(u8)  != 1) osd_printf_error("u8 must be 8 bits\n");
	if (sizeof(s16) != 2) osd_printf_error("s16 must be 16 bits\n");
	if (sizeof(u16) != 2) osd_printf_error("u16 must be 16 bits\n");
	if (sizeof(s32) != 4) osd_printf_error("s32 must be 32 bits\n");
	if (sizeof(u32) != 4) osd_printf_error("u32 must be 32 bits\n");
	if (sizeof(s64) != 8) osd_printf_error("s64 must be 64 bits\n");
	if (sizeof(u64) != 8) osd_printf_error("u64 must be 64 bits\n");

	// check signed right shift
	s8  a8 = -3;
	s16 a16 = -3;
	s32 a32 = -3;
	s64 a64 = -3;
	if (a8  >> 1 != -2) osd_printf_error("s8 right shift must be arithmetic\n");
	if (a16 >> 1 != -2) osd_printf_error("s16 right shift must be arithmetic\n");
	if (a32 >> 1 != -2) osd_printf_error("s32 right shift must be arithmetic\n");
	if (a64 >> 1 != -2) osd_printf_error("s64 right shift must be arithmetic\n");

	// TODO: check if this is actually working
	// check endianness definition
	u16 lsbtest = 0;
	*(u8 *)&lsbtest = 0xff;
#ifdef LSB_FIRST
	if (lsbtest == 0xff00) osd_printf_error("LSB_FIRST specified, but running on a big-endian machine\n");
#else
	if (lsbtest == 0x00ff) osd_printf_error("LSB_FIRST not specified, but running on a little-endian machine\n");
#endif
}


//-------------------------------------------------
//  validate_inlines - validate inline function
//  behaviors
//-------------------------------------------------

void validate_inlines()
{
	volatile u64 testu64a = random_u64();
	volatile s64 testi64a = random_i64();
	volatile u32 testu32a = random_u32();
	volatile u32 testu32b = random_u32();
	volatile s32 testi32a = random_i32();
	volatile s32 testi32b = random_i32();
	s32 resulti32, expectedi32;
	u32 resultu32, expectedu32;
	s64 resulti64, expectedi64;
	u64 resultu64, expectedu64;
	s32 remainder, expremainder;
	u32 uremainder, expuremainder, bigu32 = 0xffffffff;

	// use only non-zero, positive numbers
	if (testu64a == 0) testu64a++;
	if (testi64a == 0) testi64a++;
	else if (testi64a < 0) testi64a = -testi64a;
	if (testu32a == 0) testu32a++;
	if (testu32b == 0) testu32b++;
	if (testi32a == 0) testi32a++;
	else if (testi32a < 0) testi32a = -testi32a;
	if (testi32b == 0) testi32b++;
	else if (testi32b < 0) testi32b = -testi32b;

	resulti64 = mul_32x32(testi32a, testi32b);
	expectedi64 = s64(testi32a) * s64(testi32b);
	if (resulti64 != expectedi64)
		osd_printf_error("Error testing mul_32x32 (%08X x %08X) = %16X (expected %16X)\n", s32(testi32a), s32(testi32b), resulti64, expectedi64);

	resultu64 = mulu_32x32(testu32a, testu32b);
	expectedu64 = u64(testu32a) * u64(testu32b);
	if (resultu64 != expectedu64)
		osd_printf_error("Error testing mulu_32x32 (%08X x %08X) = %16X (expected %16X)\n", u32(testu32a), u32(testu32b), resultu64, expectedu64);

	resulti32 = mul_32x32_hi(testi32a, testi32b);
	expectedi32 = (s64(testi32a) * s64(testi32b)) >> 32;
	if (resulti32 != expectedi32)
		osd_printf_error("Error testing mul_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", s32(testi32a), s32(testi32b), resulti32, expectedi32);

	resultu32 = mulu_32x32_hi(testu32a, testu32b);
	expectedu32 = (s64(testu32a) * s64(testu32b)) >> 32;
	if (resultu32 != expectedu32)
		osd_printf_error("Error testing mulu_32x32_hi (%08X x %08X) = %08X (expected %08X)\n", u32(testu32a), u32(testu32b), resultu32, expectedu32);

	resulti32 = mul_32x32_shift(testi32a, testi32b, 7);
	expectedi32 = (s64(testi32a) * s64(testi32b)) >> 7;
	if (resulti32 != expectedi32)
		osd_printf_error("Error testing mul_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", s32(testi32a), s32(testi32b), resulti32, expectedi32);

	resultu32 = mulu_32x32_shift(testu32a, testu32b, 7);
	expectedu32 = (s64(testu32a) * s64(testu32b)) >> 7;
	if (resultu32 != expectedu32)
		osd_printf_error("Error testing mulu_32x32_shift (%08X x %08X) >> 7 = %08X (expected %08X)\n", u32(testu32a), u32(testu32b), resultu32, expectedu32);

	while (s64(testi32a) * s64(0x7fffffff) < testi64a)
		testi64a /= 2;
	while (u64(testu32a) * u64(bigu32) < testu64a)
		testu64a /= 2;

	resulti32 = div_64x32(testi64a, testi32a);
	expectedi32 = testi64a / s64(testi32a);
	if (resulti32 != expectedi32)
		osd_printf_error("Error testing div_64x32 (%16X / %08X) = %08X (expected %08X)\n", s64(testi64a), s32(testi32a), resulti32, expectedi32);

	resultu32 = divu_64x32(testu64a, testu32a);
	expectedu32 = testu64a / u64(testu32a);
	if (resultu32 != expectedu32)
		osd_printf_error("Error testing divu_64x32 (%16X / %08X) = %08X (expected %08X)\n", u64(testu64a), u32(testu32a), resultu32, expectedu32);

	resulti32 = div_64x32_rem(testi64a, testi32a, remainder);
	expectedi32 = testi64a / s64(testi32a);
	expremainder = testi64a % s64(testi32a);
	if (resulti32 != expectedi32 || remainder != expremainder)
		osd_printf_error("Error testing div_64x32_rem (%16X / %08X) = %08X,%08X (expected %08X,%08X)\n", s64(testi64a), s32(testi32a), resulti32, remainder, expectedi32, expremainder);

	resultu32 = divu_64x32_rem(testu64a, testu32a, uremainder);
	expectedu32 = testu64a / u64(testu32a);
	expuremainder = testu64a % u64(testu32a);
	if (resultu32 != expectedu32 || uremainder != expuremainder)
		osd_printf_error("Error testing divu_64x32_rem (%16X / %08X) = %08X,%08X (expected %08X,%08X)\n", u64(testu64a), u32(testu32a), resultu32, uremainder, expectedu32, expuremainder);

	resulti32 = mod_64x32(testi64a, testi32a);
	expectedi32 = testi64a % s64(testi32a);
	if (resulti32 != expectedi32)
		osd_printf_error("Error testing mod_64x32 (%16X / %08X) = %08X (expected %08X)\n", s64(testi64a), s32(testi32a), resulti32, expectedi32);

	resultu32 = modu_64x32(testu64a, testu32a);
	expectedu32 = testu64a % u64(testu32a);
	if (resultu32 != expectedu32)
		osd_printf_error("Error testing modu_64x32 (%16X / %08X) = %08X (expected %08X)\n", u64(testu64a), u32(testu32a), resultu32, expectedu32);

	while (s64(testi32a) * s64(0x7fffffff) < (s32(testi64a) << 3))
		testi64a /= 2;
	while (u64(testu32a) * u64(0xffffffff) < (u32(testu64a) << 3))
		testu64a /= 2;

	resulti32 = div_32x32_shift(s32(testi64a), testi32a, 3);
	expectedi32 = (s64(s32(testi64a)) << 3) / s64(testi32a);
	if (resulti32 != expectedi32)
		osd_printf_error("Error testing div_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", s32(testi64a), s32(testi32a), resulti32, expectedi32);

	resultu32 = divu_32x32_shift(u32(testu64a), testu32a, 3);
	expectedu32 = (u64(u32(testu64a)) << 3) / u64(testu32a);
	if (resultu32 != expectedu32)
		osd_printf_error("Error testing divu_32x32_shift (%08X << 3) / %08X = %08X (expected %08X)\n", u32(testu64a), u32(testu32a), resultu32, expectedu32);

	if (fabsf(recip_approx(100.0f) - 0.01f) > 0.0001f)
		osd_printf_error("Error testing recip_approx\n");

	for (int i = 0; i <= 32; i++)
	{
		u32 t = i < 32 ? (1 << (31 - i) | testu32a >> i) : 0;
		u8 resultu8 = count_leading_zeros_32(t);
		if (resultu8 != i)
			osd_printf_error("Error testing count_leading_zeros_32 %08x=%02x (expected %02x)\n", t, resultu8, i);

		t ^= 0xffffffff;
		resultu8 = count_leading_ones_32(t);
		if (resultu8 != i)
			osd_printf_error("Error testing count_leading_ones_32 %08x=%02x (expected %02x)\n", t, resultu8, i);
	}

	u32 expected32 = testu32a << 1 | testu32a >> 31;
	for (int i = -33; i <= 33; i++)
	{
		u32 resultu32r = rotr_32(testu32a, i);
		u32 resultu32l = rotl_32(testu32a, -i);

		if (resultu32r != expected32)
			osd_printf_error("Error testing rotr_32 %08x, %d=%08x (expected %08x)\n", u32(testu32a), i, resultu32r, expected32);
		if (resultu32l != expected32)
			osd_printf_error("Error testing rotl_32 %08x, %d=%08x (expected %08x)\n", u32(testu32a), -i, resultu32l, expected32);

		expected32 = expected32 >> 1 | expected32 << 31;
	}

	u64 expected64 = testu64a << 1 | testu64a >> 63;
	for (int i = -65; i <= 65; i++)
	{
		u64 resultu64r = rotr_64(testu64a, i);
		u64 resultu64l = rotl_64(testu64a, -i);

		if (resultu64r != expected64)
			osd_printf_error("Error testing rotr_64 %016x, %d=%016x (expected %016x)\n", u64(testu64a), i, resultu64r, expected64);
		if (resultu64l != expected64)
			osd_printf_error("Error testing rotl_64 %016x, %d=%016x (expected %016x)\n", u64(testu64a), -i, resultu64l, expected64);

		expected64 = expected64 >> 1 | expected64 << 63;
	}
}


//-------------------------------------------------
//  validate_rgb - validate optimised RGB utility
//  class
//-------------------------------------------------

void validate_rgb()
{
	/*
	    This performs cursory tests of most of the vector-optimised RGB
	    utilities, concentrating on the low-level maths.  It uses random
	    values most of the time for a quick go/no-go indication rather
	    than trying to exercise edge cases.  It doesn't matter too much
	    if the compiler optimises out some of the operations since it's
	    really intended to check for logic bugs in the vector code.  If
	    the compiler can work out that the code produces the expected
	    result, that's good enough.

	    The tests for bitwise logical operations are ordered to minimise
	    the chance of all-zero or all-one patterns producing a
	    misleading good result.

	    The following functions are not tested yet:
	    rgbaint_t()
	    clamp_and_clear(const u32)
	    sign_extend(const u32, const u32)
	    min(const s32)
	    max(const s32)
	    blend(const rgbaint_t&, u8)
	    scale_and_clamp(const rgbaint_t&)
	    scale_imm_and_clamp(const s32)
	    scale2_add_and_clamp(const rgbaint_t&, const rgbaint_t&, const rgbaint_t&)
	    scale_add_and_clamp(const rgbaint_t&, const rgbaint_t&);
	    scale_imm_add_and_clamp(const s32, const rgbaint_t&);
	*/

	auto random_i32_nolimit =
			[] ()
			{
				s32 result;
				do { result = random_i32(); } while ((result == std::numeric_limits<s32>::min()) || (result == std::numeric_limits<s32>::max()));
				return result;
			};

	volatile s32 expected_a, expected_r, expected_g, expected_b;
	volatile s32 actual_a, actual_r, actual_g, actual_b;
	volatile s32 imm;
	rgbaint_t rgb, other;
	rgb_t packed;
	auto check_expected =
			[&] (const char *desc)
			{
				const volatile s32 a = rgb.get_a32();
				const volatile s32 r = rgb.get_r32();
				const volatile s32 g = rgb.get_g32();
				const volatile s32 b = rgb.get_b32();
				if (a != expected_a) osd_printf_error("Error testing %s get_a32() = %d (expected %d)\n", desc, s32(a), s32(expected_a));
				if (r != expected_r) osd_printf_error("Error testing %s get_r32() = %d (expected %d)\n", desc, s32(r), s32(expected_r));
				if (g != expected_g) osd_printf_error("Error testing %s get_g32() = %d (expected %d)\n", desc, s32(g), s32(expected_g));
				if (b != expected_b) osd_printf_error("Error testing %s get_b32() = %d (expected %d)\n", desc, s32(b), s32(expected_b));
			};

	// check set/get
	expected_a = random_i32();
	expected_r = random_i32();
	expected_g = random_i32();
	expected_b = random_i32();
	rgb.set(expected_a, expected_r, expected_g, expected_b);
	check_expected("rgbaint_t::set(a, r, g, b)");

	// check construct/set
	expected_a = random_i32();
	expected_r = random_i32();
	expected_g = random_i32();
	expected_b = random_i32();
	rgb.set(rgbaint_t(expected_a, expected_r, expected_g, expected_b));
	check_expected("rgbaint_t::set(rgbaint_t)");

	packed = random_i32();
	expected_a = packed.a();
	expected_r = packed.r();
	expected_g = packed.g();
	expected_b = packed.b();
	rgb.set(packed);
	check_expected("rgbaint_t::set(const rgb_t& rgb)");

	// check construct/assign
	expected_a = random_i32();
	expected_r = random_i32();
	expected_g = random_i32();
	expected_b = random_i32();
	rgb = rgbaint_t(expected_a, expected_r, expected_g, expected_b);
	check_expected("rgbaint_t assignment");

	// check piecewise set
	rgb.set_a(expected_a = random_i32());
	check_expected("rgbaint_t::set_a");
	rgb.set_r(expected_r = random_i32());
	check_expected("rgbaint_t::set_r");
	rgb.set_g(expected_g = random_i32());
	check_expected("rgbaint_t::set_g");
	rgb.set_b(expected_b = random_i32());
	check_expected("rgbaint_t::set_b");

	// test merge_alpha
	expected_a = rand();
	rgb.merge_alpha(rgbaint_t(expected_a, rand(), rand(), rand()));
	check_expected("rgbaint_t::merge_alpha");

	// test RGB addition (method)
	expected_a += actual_a = random_i32();
	expected_r += actual_r = random_i32();
	expected_g += actual_g = random_i32();
	expected_b += actual_b = random_i32();
	rgb.add(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::add");

	// test RGB addition (operator)
	expected_a += actual_a = random_i32();
	expected_r += actual_r = random_i32();
	expected_g += actual_g = random_i32();
	expected_b += actual_b = random_i32();
	rgb += rgbaint_t(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::operator+=");

	// test offset addition (method)
	imm = random_i32();
	expected_a += imm;
	expected_r += imm;
	expected_g += imm;
	expected_b += imm;
	rgb.add_imm(imm);
	check_expected("rgbaint_t::add_imm");

	// test offset addition (operator)
	imm = random_i32();
	expected_a += imm;
	expected_r += imm;
	expected_g += imm;
	expected_b += imm;
	rgb += imm;
	check_expected("rgbaint_t::operator+=");

	// test immediate RGB addition
	expected_a += actual_a = random_i32();
	expected_r += actual_r = random_i32();
	expected_g += actual_g = random_i32();
	expected_b += actual_b = random_i32();
	rgb.add_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::add_imm_rgba");

	// test RGB subtraction (method)
	expected_a -= actual_a = random_i32();
	expected_r -= actual_r = random_i32();
	expected_g -= actual_g = random_i32();
	expected_b -= actual_b = random_i32();
	rgb.sub(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::sub");

	// test RGB subtraction (operator)
	expected_a -= actual_a = random_i32();
	expected_r -= actual_r = random_i32();
	expected_g -= actual_g = random_i32();
	expected_b -= actual_b = random_i32();
	rgb -= rgbaint_t(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::operator-=");

	// test offset subtraction
	imm = random_i32();
	expected_a -= imm;
	expected_r -= imm;
	expected_g -= imm;
	expected_b -= imm;
	rgb.sub_imm(imm);
	check_expected("rgbaint_t::sub_imm");

	// test immediate RGB subtraction
	expected_a -= actual_a = random_i32();
	expected_r -= actual_r = random_i32();
	expected_g -= actual_g = random_i32();
	expected_b -= actual_b = random_i32();
	rgb.sub_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::sub_imm_rgba");

	// test reversed RGB subtraction
	expected_a = (actual_a = random_i32()) - expected_a;
	expected_r = (actual_r = random_i32()) - expected_r;
	expected_g = (actual_g = random_i32()) - expected_g;
	expected_b = (actual_b = random_i32()) - expected_b;
	rgb.subr(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::subr");

	// test reversed offset subtraction
	imm = random_i32();
	expected_a = imm - expected_a;
	expected_r = imm - expected_r;
	expected_g = imm - expected_g;
	expected_b = imm - expected_b;
	rgb.subr_imm(imm);
	check_expected("rgbaint_t::subr_imm");

	// test reversed immediate RGB subtraction
	expected_a = (actual_a = random_i32()) - expected_a;
	expected_r = (actual_r = random_i32()) - expected_r;
	expected_g = (actual_g = random_i32()) - expected_g;
	expected_b = (actual_b = random_i32()) - expected_b;
	rgb.subr_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::subr_imm_rgba");

	// test RGB multiplication (method)
	expected_a *= actual_a = random_i32();
	expected_r *= actual_r = random_i32();
	expected_g *= actual_g = random_i32();
	expected_b *= actual_b = random_i32();
	rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::mul");

	// test RGB multiplication (operator)
	expected_a *= actual_a = random_i32();
	expected_r *= actual_r = random_i32();
	expected_g *= actual_g = random_i32();
	expected_b *= actual_b = random_i32();
	rgb *= rgbaint_t(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::operator*=");

	// test factor multiplication (method)
	imm = random_i32();
	expected_a *= imm;
	expected_r *= imm;
	expected_g *= imm;
	expected_b *= imm;
	rgb.mul_imm(imm);
	check_expected("rgbaint_t::mul_imm");

	// test factor multiplication (operator)
	imm = random_i32();
	expected_a *= imm;
	expected_r *= imm;
	expected_g *= imm;
	expected_b *= imm;
	rgb *= imm;
	check_expected("rgbaint_t::operator*=");

	// test immediate RGB multiplication
	expected_a *= actual_a = random_i32();
	expected_r *= actual_r = random_i32();
	expected_g *= actual_g = random_i32();
	expected_b *= actual_b = random_i32();
	rgb.mul_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::mul_imm_rgba");

	// test select alpha element multiplication
	expected_a *= actual_a = random_i32();
	expected_r *= actual_a;
	expected_g *= actual_a;
	expected_b *= actual_a;
	rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b).select_alpha32());
	check_expected("rgbaint_t::mul(select_alpha32)");

	// test select red element multiplication
	expected_a *= actual_r = random_i32();
	expected_r *= actual_r;
	expected_g *= actual_r;
	expected_b *= actual_r;
	rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b).select_red32());
	check_expected("rgbaint_t::mul(select_red32)");

	// test select green element multiplication
	expected_a *= actual_g = random_i32();
	expected_r *= actual_g;
	expected_g *= actual_g;
	expected_b *= actual_g;
	rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b).select_green32());
	check_expected("rgbaint_t::mul(select_green32)");

	// test select blue element multiplication
	expected_a *= actual_b = random_i32();
	expected_r *= actual_b;
	expected_g *= actual_b;
	expected_b *= actual_b;
	rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b).select_blue32());
	check_expected("rgbaint_t::mul(select_blue32)");

	// test RGB and not
	expected_a &= ~(actual_a = random_i32());
	expected_r &= ~(actual_r = random_i32());
	expected_g &= ~(actual_g = random_i32());
	expected_b &= ~(actual_b = random_i32());
	rgb.andnot_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::andnot_reg");

	// test RGB or
	expected_a |= actual_a = random_i32();
	expected_r |= actual_r = random_i32();
	expected_g |= actual_g = random_i32();
	expected_b |= actual_b = random_i32();
	rgb.or_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::or_reg");

	// test RGB and
	expected_a &= actual_a = random_i32();
	expected_r &= actual_r = random_i32();
	expected_g &= actual_g = random_i32();
	expected_b &= actual_b = random_i32();
	rgb.and_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::and_reg");

	// test RGB xor
	expected_a ^= actual_a = random_i32();
	expected_r ^= actual_r = random_i32();
	expected_g ^= actual_g = random_i32();
	expected_b ^= actual_b = random_i32();
	rgb.xor_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
	check_expected("rgbaint_t::xor_reg");

	// test uniform or
	imm = random_i32();
	expected_a |= imm;
	expected_r |= imm;
	expected_g |= imm;
	expected_b |= imm;
	rgb.or_imm(imm);
	check_expected("rgbaint_t::or_imm");

	// test uniform and
	imm = random_i32();
	expected_a &= imm;
	expected_r &= imm;
	expected_g &= imm;
	expected_b &= imm;
	rgb.and_imm(imm);
	check_expected("rgbaint_t::and_imm");

	// test uniform xor
	imm = random_i32();
	expected_a ^= imm;
	expected_r ^= imm;
	expected_g ^= imm;
	expected_b ^= imm;
	rgb.xor_imm(imm);
	check_expected("rgbaint_t::xor_imm");

	// test immediate RGB or
	expected_a |= actual_a = random_i32();
	expected_r |= actual_r = random_i32();
	expected_g |= actual_g = random_i32();
	expected_b |= actual_b = random_i32();
	rgb.or_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::or_imm_rgba");

	// test immediate RGB and
	expected_a &= actual_a = random_i32();
	expected_r &= actual_r = random_i32();
	expected_g &= actual_g = random_i32();
	expected_b &= actual_b = random_i32();
	rgb.and_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::and_imm_rgba");

	// test immediate RGB xor
	expected_a ^= actual_a = random_i32();
	expected_r ^= actual_r = random_i32();
	expected_g ^= actual_g = random_i32();
	expected_b ^= actual_b = random_i32();
	rgb.xor_imm_rgba(actual_a, actual_r, actual_g, actual_b);
	check_expected("rgbaint_t::xor_imm_rgba");

	// test 8-bit get
	expected_a = s32(u32(expected_a) & 0x00ff);
	expected_r = s32(u32(expected_r) & 0x00ff);
	expected_g = s32(u32(expected_g) & 0x00ff);
	expected_b = s32(u32(expected_b) & 0x00ff);
	actual_a = s32(u32(rgb.get_a()));
	actual_r = s32(u32(rgb.get_r()));
	actual_g = s32(u32(rgb.get_g()));
	actual_b = s32(u32(rgb.get_b()));
	if (actual_a != expected_a) osd_printf_error("Error testing rgbaint_t::get_a() = %d (expected %d)\n", s32(actual_a), s32(expected_a));
	if (actual_r != expected_r) osd_printf_error("Error testing rgbaint_t::get_r() = %d (expected %d)\n", s32(actual_r), s32(expected_r));
	if (actual_g != expected_g) osd_printf_error("Error testing rgbaint_t::get_g() = %d (expected %d)\n", s32(actual_g), s32(expected_g));
	if (actual_b != expected_b) osd_printf_error("Error testing rgbaint_t::get_b() = %d (expected %d)\n", s32(actual_b), s32(expected_b));

	// test set from packed RGBA
	imm = random_i32();
	expected_a = s32((u32(imm) >> 24) & 0x00ff);
	expected_r = s32((u32(imm) >> 16) & 0x00ff);
	expected_g = s32((u32(imm) >> 8) & 0x00ff);
	expected_b = s32((u32(imm) >> 0) & 0x00ff);
	rgb.set(u32(imm));
	check_expected("rgbaint_t::set(u32)");

	// while we have a value loaded that we know doesn't exceed 8-bit range, check the non-clamping convert-to-rgba
	packed = rgb.to_rgba();
	if (u32(imm) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba() = %08x (expected %08x)\n", u32(packed), u32(imm));

	// test construct from packed RGBA and assign
	imm = random_i32();
	expected_a = s32((u32(imm) >> 24) & 0x00ff);
	expected_r = s32((u32(imm) >> 16) & 0x00ff);
	expected_g = s32((u32(imm) >> 8) & 0x00ff);
	expected_b = s32((u32(imm) >> 0) & 0x00ff);
	rgb = rgbaint_t(u32(imm));
	check_expected("rgbaint_t(u32)");

	// while we have a value loaded that we know doesn't exceed 8-bit range, check the non-clamping convert-to-rgba
	packed = rgb.to_rgba();
	if (u32(imm) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba() = %08x (expected %08x)\n", u32(packed), u32(imm));

	// test set with rgb_t
	packed = random_u32();
	expected_a = s32(u32(packed.a()));
	expected_r = s32(u32(packed.r()));
	expected_g = s32(u32(packed.g()));
	expected_b = s32(u32(packed.b()));
	rgb.set(packed);
	check_expected("rgbaint_t::set(rgba_t)");

	// test construct with rgb_t
	packed = random_u32();
	expected_a = s32(u32(packed.a()));
	expected_r = s32(u32(packed.r()));
	expected_g = s32(u32(packed.g()));
	expected_b = s32(u32(packed.b()));
	rgb = rgbaint_t(packed);
	check_expected("rgbaint_t::set(rgba_t)");

	// test clamping convert-to-rgba with hand-crafted values to catch edge cases
	rgb.set(std::numeric_limits<s32>::min(), -1, 0, 1);
	packed = rgb.to_rgba_clamp();
	if (u32(0x00000001) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba_clamp() = %08x (expected 0x00000001)\n", u32(packed));
	rgb.set(254, 255, 256, std::numeric_limits<s32>::max());
	packed = rgb.to_rgba_clamp();
	if (u32(0xfeffffff) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba_clamp() = %08x (expected 0xfeffffff)\n", u32(packed));
	rgb.set(std::numeric_limits<s32>::max(), std::numeric_limits<s32>::min(), 256, -1);
	packed = rgb.to_rgba_clamp();
	if (u32(0xff00ff00) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba_clamp() = %08x (expected 0xff00ff00)\n", u32(packed));
	rgb.set(0, 255, 1, 254);
	packed = rgb.to_rgba_clamp();
	if (u32(0x00ff01fe) != u32(packed))
		osd_printf_error("Error testing rgbaint_t::to_rgba_clamp() = %08x (expected 0x00ff01fe)\n", u32(packed));

	// test in-place clamping with hand-crafted values to catch edge cases
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = 1;
	rgb.set(std::numeric_limits<s32>::min(), -1, 0, 1);
	rgb.clamp_to_uint8();
	check_expected("rgbaint_t::clamp_to_uint8");
	expected_a = 254;
	expected_r = 255;
	expected_g = 255;
	expected_b = 255;
	rgb.set(254, 255, 256, std::numeric_limits<s32>::max());
	rgb.clamp_to_uint8();
	check_expected("rgbaint_t::clamp_to_uint8");
	expected_a = 255;
	expected_r = 0;
	expected_g = 255;
	expected_b = 0;
	rgb.set(std::numeric_limits<s32>::max(), std::numeric_limits<s32>::min(), 256, -1);
	rgb.clamp_to_uint8();
	check_expected("rgbaint_t::clamp_to_uint8");
	expected_a = 0;
	expected_r = 255;
	expected_g = 1;
	expected_b = 254;
	rgb.set(0, 255, 1, 254);
	rgb.clamp_to_uint8();
	check_expected("rgbaint_t::clamp_to_uint8");

	// test shift left
	expected_a = (actual_a = random_i32()) << 19;
	expected_r = (actual_r = random_i32()) << 3;
	expected_g = (actual_g = random_i32()) << 21;
	expected_b = (actual_b = random_i32()) << 6;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shl(rgbaint_t(19, 3, 21, 6));
	check_expected("rgbaint_t::shl");

	// test shift left out of range
	expected_a = (actual_a = random_i32()) & 0;
	expected_r = (actual_r = random_i32()) & 0;
	expected_g = (actual_g = random_i32()) & 0;
	expected_b = (actual_b = random_i32()) & 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shl(rgbaint_t(-19, 32, -21, 38));
	check_expected("rgbaint_t::shl");

	// test shift left immediate
	expected_a = (actual_a = random_i32()) << 7;
	expected_r = (actual_r = random_i32()) << 7;
	expected_g = (actual_g = random_i32()) << 7;
	expected_b = (actual_b = random_i32()) << 7;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shl_imm(7);
	check_expected("rgbaint_t::shl_imm");

	// test shift left immediate out of range
	expected_a = (actual_a = random_i32()) & 0;
	expected_r = (actual_r = random_i32()) & 0;
	expected_g = (actual_g = random_i32()) & 0;
	expected_b = (actual_b = random_i32()) & 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shl_imm(32);
	check_expected("rgbaint_t::shl_imm");

	// test logical shift right
	expected_a = s32(u32(actual_a = random_i32()) >> 8);
	expected_r = s32(u32(actual_r = random_i32()) >> 18);
	expected_g = s32(u32(actual_g = random_i32()) >> 26);
	expected_b = s32(u32(actual_b = random_i32()) >> 4);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr(rgbaint_t(8, 18, 26, 4));
	check_expected("rgbaint_t::shr");

	// test logical shift right with opposite signs
	expected_a = s32(u32(actual_a = -actual_a) >> 21);
	expected_r = s32(u32(actual_r = -actual_r) >> 13);
	expected_g = s32(u32(actual_g = -actual_g) >> 11);
	expected_b = s32(u32(actual_b = -actual_b) >> 17);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr(rgbaint_t(21, 13, 11, 17));
	check_expected("rgbaint_t::shr");

	// test logical shift right out of range
	expected_a = (actual_a = random_i32()) & 0;
	expected_r = (actual_r = random_i32()) & 0;
	expected_g = (actual_g = random_i32()) & 0;
	expected_b = (actual_b = random_i32()) & 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr(rgbaint_t(40, -18, -26, 32));
	check_expected("rgbaint_t::shr");

	// test logical shift right immediate
	expected_a = s32(u32(actual_a = random_i32()) >> 5);
	expected_r = s32(u32(actual_r = random_i32()) >> 5);
	expected_g = s32(u32(actual_g = random_i32()) >> 5);
	expected_b = s32(u32(actual_b = random_i32()) >> 5);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr_imm(5);
	check_expected("rgbaint_t::shr_imm");

	// test logical shift right immediate with opposite signs
	expected_a = s32(u32(actual_a = -actual_a) >> 15);
	expected_r = s32(u32(actual_r = -actual_r) >> 15);
	expected_g = s32(u32(actual_g = -actual_g) >> 15);
	expected_b = s32(u32(actual_b = -actual_b) >> 15);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr_imm(15);
	check_expected("rgbaint_t::shr_imm");

	// test logical shift right immediate out of range
	expected_a = (actual_a = random_i32()) & 0;
	expected_r = (actual_r = random_i32()) & 0;
	expected_g = (actual_g = random_i32()) & 0;
	expected_b = (actual_b = random_i32()) & 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.shr_imm(35);
	check_expected("rgbaint_t::shr_imm");

	// test arithmetic shift right
	expected_a = (actual_a = random_i32()) >> 16;
	expected_r = (actual_r = random_i32()) >> 20;
	expected_g = (actual_g = random_i32()) >> 14;
	expected_b = (actual_b = random_i32()) >> 2;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra(rgbaint_t(16, 20, 14, 2));
	check_expected("rgbaint_t::sra");

	// test arithmetic shift right with opposite signs
	expected_a = (actual_a = -actual_a) >> 1;
	expected_r = (actual_r = -actual_r) >> 29;
	expected_g = (actual_g = -actual_g) >> 10;
	expected_b = (actual_b = -actual_b) >> 22;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra(rgbaint_t(1, 29, 10, 22));
	check_expected("rgbaint_t::sra");

	// test arithmetic shift right out of range
	expected_a = (actual_a = random_i32()) >> 31;
	expected_r = (actual_r = random_i32()) >> 31;
	expected_g = (actual_g = random_i32()) >> 31;
	expected_b = (actual_b = random_i32()) >> 31;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra(rgbaint_t(-16, -20, 46, 32));
	check_expected("rgbaint_t::sra");

	// test arithmetic shift right immediate (method)
	expected_a = (actual_a = random_i32()) >> 12;
	expected_r = (actual_r = random_i32()) >> 12;
	expected_g = (actual_g = random_i32()) >> 12;
	expected_b = (actual_b = random_i32()) >> 12;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra_imm(12);
	check_expected("rgbaint_t::sra_imm");

	// test arithmetic shift right immediate with opposite signs (method)
	expected_a = (actual_a = -actual_a) >> 9;
	expected_r = (actual_r = -actual_r) >> 9;
	expected_g = (actual_g = -actual_g) >> 9;
	expected_b = (actual_b = -actual_b) >> 9;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra_imm(9);
	check_expected("rgbaint_t::sra_imm");

	// test arithmetic shift right immediate out of range (method)
	expected_a = (actual_a = random_i32()) >> 31;
	expected_r = (actual_r = random_i32()) >> 31;
	expected_g = (actual_g = random_i32()) >> 31;
	expected_b = (actual_b = random_i32()) >> 31;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.sra_imm(38);
	check_expected("rgbaint_t::sra_imm");

	// test arithmetic shift right immediate (operator)
	expected_a = (actual_a = random_i32()) >> 7;
	expected_r = (actual_r = random_i32()) >> 7;
	expected_g = (actual_g = random_i32()) >> 7;
	expected_b = (actual_b = random_i32()) >> 7;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb >>= 7;
	check_expected("rgbaint_t::operator>>=");

	// test arithmetic shift right immediate with opposite signs (operator)
	expected_a = (actual_a = -actual_a) >> 11;
	expected_r = (actual_r = -actual_r) >> 11;
	expected_g = (actual_g = -actual_g) >> 11;
	expected_b = (actual_b = -actual_b) >> 11;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb >>= 11;
	check_expected("rgbaint_t::operator>>=");

	// test arithmetic shift right immediate out of range (operator)
	expected_a = (actual_a = random_i32()) >> 31;
	expected_r = (actual_r = random_i32()) >> 31;
	expected_g = (actual_g = random_i32()) >> 31;
	expected_b = (actual_b = random_i32()) >> 31;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb >>= 41;
	check_expected("rgbaint_t::operator>>=");

	// test RGB equality comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = ~s32(0);
	expected_r = 0;
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq(rgbaint_t(actual_a, actual_r - 1, actual_g + 1, std::numeric_limits<s32>::min()));
	check_expected("rgbaint_t::cmpeq");
	expected_a = 0;
	expected_r = ~s32(0);
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
	check_expected("rgbaint_t::cmpeq");

	// test immediate equality comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = ~s32(0);
	expected_r = (actual_r == actual_a) ? ~s32(0) : 0;
	expected_g = (actual_g == actual_a) ? ~s32(0) : 0;
	expected_b = (actual_b == actual_a) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(actual_a);
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = (actual_a == actual_r) ? ~s32(0) : 0;
	expected_r = ~s32(0);
	expected_g = (actual_g == actual_r) ? ~s32(0) : 0;
	expected_b = (actual_b == actual_r) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(actual_r);
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = (actual_a == actual_g) ? ~s32(0) : 0;
	expected_r = (actual_r == actual_g) ? ~s32(0) : 0;
	expected_g = ~s32(0);
	expected_b = (actual_b == actual_g) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(actual_g);
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = (actual_a == actual_b) ? ~s32(0) : 0;
	expected_r = (actual_r == actual_b) ? ~s32(0) : 0;
	expected_g = (actual_g == actual_b) ? ~s32(0) : 0;
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(actual_b);
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(std::numeric_limits<s32>::min());
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = !actual_a ? ~s32(0) : 0;
	expected_r = !actual_r ? ~s32(0) : 0;
	expected_g = !actual_g ? ~s32(0) : 0;
	expected_b = !actual_b ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(0);
	check_expected("rgbaint_t::cmpeq_imm");
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm(std::numeric_limits<s32>::max());
	check_expected("rgbaint_t::cmpeq_imm");

	// test immediate RGB equality comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = 0;
	expected_g = ~s32(0);
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm_rgba(std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_g, actual_b - 1);
	check_expected("rgbaint_t::cmpeq_imm_rgba");
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpeq_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
	check_expected("rgbaint_t::cmpeq_imm_rgba");

	// test RGB greater than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = ~s32(0);
	expected_g = 0;
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt(rgbaint_t(actual_a, actual_r - 1, actual_g + 1, std::numeric_limits<s32>::min()));
	check_expected("rgbaint_t::cmpgt");
	expected_a = 0;
	expected_r = 0;
	expected_g = ~s32(0);
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
	check_expected("rgbaint_t::cmpgt");

	// test immediate greater than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = (actual_r > actual_a) ? ~s32(0) : 0;
	expected_g = (actual_g > actual_a) ? ~s32(0) : 0;
	expected_b = (actual_b > actual_a) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(actual_a);
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = (actual_a > actual_r) ? ~s32(0) : 0;
	expected_r = 0;
	expected_g = (actual_g > actual_r) ? ~s32(0) : 0;
	expected_b = (actual_b > actual_r) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(actual_r);
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = (actual_a > actual_g) ? ~s32(0) : 0;
	expected_r = (actual_r > actual_g) ? ~s32(0) : 0;
	expected_g =0;
	expected_b = (actual_b > actual_g) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(actual_g);
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = (actual_a > actual_b) ? ~s32(0) : 0;
	expected_r = (actual_r > actual_b) ? ~s32(0) : 0;
	expected_g = (actual_g > actual_b) ? ~s32(0) : 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(actual_b);
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = ~s32(0);
	expected_r = ~s32(0);
	expected_g = ~s32(0);
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(std::numeric_limits<s32>::min());
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = (actual_a > 0) ? ~s32(0) : 0;
	expected_r = (actual_r > 0) ? ~s32(0) : 0;
	expected_g = (actual_g > 0) ? ~s32(0) : 0;
	expected_b = (actual_b > 0) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(0);
	check_expected("rgbaint_t::cmpgt_imm");
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm(std::numeric_limits<s32>::max());
	check_expected("rgbaint_t::cmpgt_imm");

	// test immediate RGB greater than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = ~s32(0);
	expected_r = 0;
	expected_g = 0;
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm_rgba(std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_g, actual_b - 1);
	check_expected("rgbaint_t::cmpgt_imm_rgba");
	expected_a = 0;
	expected_r = ~s32(0);
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmpgt_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
	check_expected("rgbaint_t::cmpgt_imm_rgba");

	// test RGB less than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = 0;
	expected_g = ~s32(0);
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt(rgbaint_t(actual_a, actual_r - 1, actual_g + 1, std::numeric_limits<s32>::min()));
	check_expected("rgbaint_t::cmplt");
	expected_a = ~s32(0);
	expected_r = 0;
	expected_g = 0;
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
	check_expected("rgbaint_t::cmplt");

	// test immediate less than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = (actual_r < actual_a) ? ~s32(0) : 0;
	expected_g = (actual_g < actual_a) ? ~s32(0) : 0;
	expected_b = (actual_b < actual_a) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(actual_a);
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = (actual_a < actual_r) ? ~s32(0) : 0;
	expected_r = 0;
	expected_g = (actual_g < actual_r) ? ~s32(0) : 0;
	expected_b = (actual_b < actual_r) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(actual_r);
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = (actual_a < actual_g) ? ~s32(0) : 0;
	expected_r = (actual_r < actual_g) ? ~s32(0) : 0;
	expected_g =0;
	expected_b = (actual_b < actual_g) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(actual_g);
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = (actual_a < actual_b) ? ~s32(0) : 0;
	expected_r = (actual_r < actual_b) ? ~s32(0) : 0;
	expected_g = (actual_g < actual_b) ? ~s32(0) : 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(actual_b);
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = 0;
	expected_r = 0;
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(std::numeric_limits<s32>::min());
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = (actual_a < 0) ? ~s32(0) : 0;
	expected_r = (actual_r < 0) ? ~s32(0) : 0;
	expected_g = (actual_g < 0) ? ~s32(0) : 0;
	expected_b = (actual_b < 0) ? ~s32(0) : 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(0);
	check_expected("rgbaint_t::cmplt_imm");
	expected_a = ~s32(0);
	expected_r = ~s32(0);
	expected_g = ~s32(0);
	expected_b = ~s32(0);
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm(std::numeric_limits<s32>::max());
	check_expected("rgbaint_t::cmplt_imm");

	// test immediate RGB less than comparison
	actual_a = random_i32_nolimit();
	actual_r = random_i32_nolimit();
	actual_g = random_i32_nolimit();
	actual_b = random_i32_nolimit();
	expected_a = 0;
	expected_r = ~s32(0);
	expected_g = 0;
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm_rgba(std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_g, actual_b - 1);
	check_expected("rgbaint_t::cmplt_imm_rgba");
	expected_a = ~s32(0);
	expected_r = 0;
	expected_g = ~s32(0);
	expected_b = 0;
	rgb.set(actual_a, actual_r, actual_g, actual_b);
	rgb.cmplt_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
	check_expected("rgbaint_t::cmplt_imm_rgba");

	// test bilinear_filter and bilinear_filter_rgbaint
	// SSE implementation carries more internal precision between the bilinear stages
#if defined(MAME_RGB_HIGH_PRECISION)
	const int first_shift = 1;
#else
	const int first_shift = 8;
#endif
	for (int index = 0; index < 1000; index++)
	{
		u8 u, v;
		rgbaint_t rgb_point[4];
		u32 top_row, bottom_row;

		for (int i = 0; i < 4; i++)
		{
			rgb_point[i].set(random_u32());
		}

		switch (index)
		{
			case 0: u = 0; v = 0; break;
			case 1: u = 255; v = 255; break;
			case 2: u = 0; v = 255; break;
			case 3: u = 255; v = 0; break;
			case 4: u = 128; v = 128; break;
			case 5: u = 63; v = 32; break;
			default:
				u = random_u32() & 0xff;
				v = random_u32() & 0xff;
				break;
		}

		top_row = (rgb_point[0].get_a() * (256 - u) + rgb_point[1].get_a() * u) >> first_shift;
		bottom_row = (rgb_point[2].get_a() * (256 - u) + rgb_point[3].get_a() * u) >> first_shift;
		expected_a = (top_row * (256 - v) + bottom_row * v) >> (16 - first_shift);

		top_row = (rgb_point[0].get_r() * (256 - u) + rgb_point[1].get_r() * u) >> first_shift;
		bottom_row = (rgb_point[2].get_r() * (256 - u) + rgb_point[3].get_r() * u) >> first_shift;
		expected_r = (top_row * (256 - v) + bottom_row * v) >> (16 - first_shift);

		top_row = (rgb_point[0].get_g() * (256 - u) + rgb_point[1].get_g() * u) >> first_shift;
		bottom_row = (rgb_point[2].get_g() * (256 - u) + rgb_point[3].get_g() * u) >> first_shift;
		expected_g = (top_row * (256 - v) + bottom_row * v) >> (16 - first_shift);

		top_row = (rgb_point[0].get_b() * (256 - u) + rgb_point[1].get_b() * u) >> first_shift;
		bottom_row = (rgb_point[2].get_b() * (256 - u) + rgb_point[3].get_b() * u) >> first_shift;
		expected_b = (top_row * (256 - v) + bottom_row * v) >> (16 - first_shift);

		imm = rgbaint_t::bilinear_filter(rgb_point[0].to_rgba(), rgb_point[1].to_rgba(), rgb_point[2].to_rgba(), rgb_point[3].to_rgba(), u, v);
		rgb.set(imm);
		check_expected("rgbaint_t::bilinear_filter");

		rgb.bilinear_filter_rgbaint(rgb_point[0].to_rgba(), rgb_point[1].to_rgba(), rgb_point[2].to_rgba(), rgb_point[3].to_rgba(), u, v);
		check_expected("rgbaint_t::bilinear_filter_rgbaint");
	}
}


//-------------------------------------------------
//  validate_delegates_mfp - test delegate member
//  function functionality
//-------------------------------------------------

void validate_delegates_mfp()
{
	struct base_a
	{
		virtual ~base_a() = default;
		char get_a(void const *&p) { p = this; return 'a'; }
		virtual char get_a_v(void const *&p) { p = this; return 'A'; }
		int a;
	};

	struct base_b
	{
		virtual ~base_b() = default;
		char get_b(void const *&p) { p = this; return 'b'; }
		virtual char get_b_v(void const *&p) { p = this; return 'B'; }
		int b;
	};

	struct multiple_inheritance : base_a, base_b
	{
	};

	struct overridden : base_a, base_b
	{
		virtual char get_a_v(void const *&p) override { p = this; return 'x'; }
		virtual char get_b_v(void const *&p) override { p = this; return 'y'; }
	};

	multiple_inheritance mi;
	overridden o;
	diamond_inheritance d;
	char ch;
	void const *addr;

	// test non-virtual member functions and "this" pointer adjustment
	test_delegate cb1(&multiple_inheritance::get_a, &mi);
	test_delegate cb2(&multiple_inheritance::get_b, &mi);

	addr = nullptr;
	ch = cb1(addr);
	if ('a' != ch)
		osd_printf_error("Error testing delegate non-virtual member function dispatch\n");
	if (static_cast<base_a *>(&mi) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_a *>(&mi)));

	addr = nullptr;
	ch = cb2(addr);
	if ('b' != ch)
		osd_printf_error("Error testing delegate non-virtual member function dispatch\n");
	if (static_cast<base_b *>(&mi) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_b *>(&mi)));

	// test that "this" pointer adjustment survives copy construction
	test_delegate cb3(cb1);
	test_delegate cb4(cb2);

	addr = nullptr;
	ch = cb3(addr);
	if ('a' != ch)
		osd_printf_error("Error testing copy constructed delegate non-virtual member function dispatch\n");
	if (static_cast<base_a *>(&mi) != addr)
		osd_printf_error("Error testing copy constructed delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_a *>(&mi)));

	addr = nullptr;
	ch = cb4(addr);
	if ('b' != ch)
		osd_printf_error("Error testing copy constructed delegate non-virtual member function dispatch\n");
	if (static_cast<base_b *>(&mi) != addr)
		osd_printf_error("Error testing copy constructed delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_b *>(&mi)));

	// test that "this" pointer adjustment survives assignment and doesn't suffer generational loss
	cb1 = cb4;
	cb2 = cb3;

	addr = nullptr;
	ch = cb1(addr);
	if ('b' != ch)
		osd_printf_error("Error testing assigned delegate non-virtual member function dispatch\n");
	if (static_cast<base_b *>(&mi) != addr)
		osd_printf_error("Error testing assigned delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_b *>(&mi)));

	addr = nullptr;
	ch = cb2(addr);
	if ('a' != ch)
		osd_printf_error("Error testing assigned delegate non-virtual member function dispatch\n");
	if (static_cast<base_a *>(&mi) != addr)
		osd_printf_error("Error testing assigned delegate this pointer adjustment %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_a *>(&mi)));

	// test virtual member functions and "this" pointer adjustment
	cb1 = test_delegate(&multiple_inheritance::get_a_v, &mi);
	cb2 = test_delegate(&multiple_inheritance::get_b_v, &mi);

	addr = nullptr;
	ch = cb1(addr);
	if ('A' != ch)
		osd_printf_error("Error testing delegate virtual member function dispatch\n");
	if (static_cast<base_a *>(&mi) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for virtual member function %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_a *>(&mi)));

	addr = nullptr;
	ch = cb2(addr);
	if ('B' != ch)
		osd_printf_error("Error testing delegate virtual member function dispatch\n");
	if (static_cast<base_b *>(&mi) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for virtual member function %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_b *>(&mi)));

	// test that virtual member functions survive copy construction
	test_delegate cb5(cb1);
	test_delegate cb6(cb2);

	addr = nullptr;
	ch = cb5(addr);
	if ('A' != ch)
		osd_printf_error("Error testing copy constructed delegate virtual member function dispatch\n");
	if (static_cast<base_a *>(&mi) != addr)
		osd_printf_error("Error testing copy constructed delegate this pointer adjustment for virtual member function %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_a *>(&mi)));

	addr = nullptr;
	ch = cb6(addr);
	if ('B' != ch)
		osd_printf_error("Error testing copy constructed delegate virtual member function dispatch\n");
	if (static_cast<base_b *>(&mi) != addr)
		osd_printf_error("Error testing copy constructed delegate this pointer adjustment for virtual member function %p -> %p (expected %p)\n", static_cast<void const *>(&mi), addr, static_cast<void const *>(static_cast<base_b *>(&mi)));

	// test virtual member function dispatch through base pointer
	cb1 = test_delegate(&base_a::get_a_v, static_cast<base_a *>(&o));
	cb2 = test_delegate(&base_b::get_b_v, static_cast<base_b *>(&o));

	addr = nullptr;
	ch = cb1(addr);
	if ('x' != ch)
		osd_printf_error("Error testing delegate virtual member function dispatch through base class pointer\n");
	if (&o != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for virtual member function through base class pointer %p -> %p (expected %p)\n", static_cast<void const *>(static_cast<base_a *>(&o)), addr, static_cast<void const *>(&o));

	addr = nullptr;
	ch = cb2(addr);
	if ('y' != ch)
		osd_printf_error("Error testing delegate virtual member function dispatch through base class pointer\n");
	if (&o != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for virtual member function through base class pointer %p -> %p (expected %p)\n", static_cast<void const *>(static_cast<base_b *>(&o)), addr, static_cast<void const *>(&o));

	// test creating delegates for a forward-declared class
	cb1 = make_diamond_class_delegate(&diamond_inheritance::get_derived_a, &d);
	cb2 = make_diamond_class_delegate(&diamond_inheritance::get_derived_b, &d);

	addr = nullptr;
	ch = cb1(addr);
	if ('a' != ch)
		osd_printf_error("Error testing delegate non-virtual member function dispatch for incomplete class\n");
	if (static_cast<virtual_derived_a *>(&d) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for incomplete class %p -> %p (expected %p)\n", static_cast<void const *>(&d), addr, static_cast<void const *>(static_cast<virtual_derived_b *>(&d)));

	addr = nullptr;
	ch = cb2(addr);
	if ('b' != ch)
		osd_printf_error("Error testing delegate non-virtual member function dispatch for incomplete class\n");
	if (static_cast<virtual_derived_b *>(&d) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for incomplete class %p -> %p (expected %p)\n", static_cast<void const *>(&d), addr, static_cast<void const *>(static_cast<virtual_derived_b *>(&d)));

#if defined(_MSC_VER) && !defined(__clang__)
	// test MSVC extension allowing casting member pointer types across virtual inheritance relationships
	cb1 = make_diamond_class_delegate(&diamond_inheritance::get_base, &d);

	addr = nullptr;
	ch = cb1(addr);
	if ('x' != ch)
		osd_printf_error("Error testing delegate non-virtual member function dispatch for incomplete class\n");
	if (static_cast<virtual_base *>(&d) != addr)
		osd_printf_error("Error testing delegate this pointer adjustment for incomplete class %p -> %p (expected %p)\n", static_cast<void const *>(&d), addr, static_cast<void const *>(static_cast<virtual_base *>(&d)));
#endif // defined(_MSC_VER) && !defined(__clang__)
}


//-------------------------------------------------
//  validate_delegates_latebind - test binding a
//  delegate to an object after the function is
//  set
//-------------------------------------------------

void validate_delegates_latebind()
{
	struct derived_a : pure_virtual_base, delegate_late_bind
	{
		virtual char operator()(void const *&p) const override { p = this; return 'a'; }
	};

	struct derived_b : pure_virtual_base, delegate_late_bind
	{
		virtual char operator()(void const *&p) const override { p = this; return 'b'; }
	};

	struct unrelated : delegate_late_bind
	{
	};

	char ch;
	void const *addr;
	derived_a a;
	derived_b b;
	unrelated u;

	// delegate with no target object
	test_delegate cb1(&pure_virtual_base::operator(), static_cast<pure_virtual_base *>(nullptr));

	// test late bind on construction
	test_delegate cb2(cb1, a);
	addr = nullptr;
	ch = cb2(addr);
	if (('a' != ch) || (&a != addr))
		osd_printf_error("Error testing delegate late bind on construction\n");

	// test explicit late bind
	cb1.late_bind(b);
	ch = cb1(addr);
	if (('b' != ch) || (&b != addr))
		osd_printf_error("Error testing delegate explicit late bind\n");

	// test late bind when object is set
	cb1.late_bind(a);
	ch = cb1(addr);
	if (('a' != ch) || (&a != addr))
		osd_printf_error("Error testing delegate explicit late bind when object is set\n");

	// test late bind on copy of delegate with target set
	test_delegate cb3(cb1, b);
	addr = nullptr;
	ch = cb3(addr);
	if (('b' != ch) || (&b != addr))
		osd_printf_error("Error testing delegate late bind on construction using template with object set\n");

	// test late bind exception
	ch = '-';
	try
	{
		cb1.late_bind(u);
	}
	catch (binding_type_exception const &e)
	{
		if ((e.target_type() != typeid(pure_virtual_base)) || (e.actual_type() != typeid(unrelated)))
		{
			osd_printf_error(
					"Error testing delegate late bind type error %s -> %s (expected %s -> %s)\n",
					e.actual_type().name(),
					e.target_type().name(),
					typeid(unrelated).name(),
					typeid(pure_virtual_base).name());
		}
		ch = '+';
	}
	if ('+' != ch)
		osd_printf_error("Error testing delegate late bind type error\n");

	// test syntax for creating delegate with alternate late bind base
	delegate<char (void const *&), pure_virtual_base> cb4(
			[] (auto &o, void const *&p) { p = &o; return 'l'; },
			static_cast<unrelated *>(nullptr));
	try { cb1.late_bind(a); }
	catch (binding_type_exception const &) { }
}


//-------------------------------------------------
//  validate_delegates_functoid - test delegate
//  functoid functionality
//-------------------------------------------------

void validate_delegates_functoid()
{
	using void_delegate = delegate<void (void const *&)>;
	struct const_only
	{
		char operator()(void const *&p) const { return 'C'; }
	};

	struct const_or_not
	{
		char operator()(void const *&p) { return 'n'; }
		char operator()(void const *&p) const { return 'c'; }
	};

	struct noncopyable
	{
		noncopyable() = default;
		noncopyable(noncopyable const &) = delete;
		noncopyable &operator=(noncopyable const &) = delete;

		char operator()(void const *&p) { p = this; return '*'; }
	};

	noncopyable n;
	char ch;
	void const *addr = nullptr;

	// test that const call operators are supported
	test_delegate cb1{ const_only() };
	if ('C' != cb1(addr))
		osd_printf_error("Error testing delegate functoid dispatch\n");

	// test that non-const call operators are preferred
	cb1 = test_delegate{ const_or_not() };
	if ('n' != cb1(addr))
		osd_printf_error("Error testing delegate functoid dispatch\n");

	// test that functoids are implicitly mutable
	cb1 = test_delegate{ [a = &addr, c = '0'] (void const *&p) mutable { p = a; return c++; } };

	addr = nullptr;
	ch = cb1(addr);
	if (('0' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 0)\n", ch);

	addr = nullptr;
	ch = cb1(addr);
	if (('1' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 1)\n", ch);

	// test that functoids survive copy construction
	test_delegate cb2(cb1);

	addr = nullptr;
	ch = cb2(addr);
	if (('2' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 2)\n", ch);

	addr = nullptr;
	ch = cb2(addr);
	if (('3' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 3)\n", ch);

	addr = nullptr;
	ch = cb1(addr);
	if (('2' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 2)\n", ch);

	// test that functoids survive assignment
	cb1 = cb2;

	addr = nullptr;
	ch = cb1(addr);
	if (('4' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 4)\n", ch);

	addr = nullptr;
	ch = cb1(addr);
	if (('5' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 5)\n", ch);

	addr = nullptr;
	ch = cb2(addr);
	if (('4' != ch) || (&addr != addr))
		osd_printf_error("Error testing delegate functoid %c (expected 4)\n", ch);

	// test that std::ref can be used with non-copyable functoids
	test_delegate cb3(std::ref(n));

	addr = nullptr;
	ch = cb3(addr);
	if (('*' != ch) || (&n != addr))
		osd_printf_error("Error testing delegate with functoid reference wrapper %p (expected %p)\n", addr, static_cast<void const *>(&n));

	// test that std::ref survives copy construction and assignment
	cb2 = cb3;
	test_delegate cb4(cb3);

	addr = nullptr;
	ch = cb2(addr);
	if (('*' != ch) || (&n != addr))
		osd_printf_error("Error testing delegate with functoid reference wrapper %p (expected %p)\n", addr, static_cast<void const *>(&n));

	addr = nullptr;
	ch = cb4(addr);
	if (('*' != ch) || (&n != addr))
		osd_printf_error("Error testing delegate with functoid reference wrapper %p (expected %p)\n", addr, static_cast<void const *>(&n));

	// test discarding return value for delegates returning void
	void_delegate void_cb1{ [&cb1] (void const *&p) { p = &cb1; return 123; } };
	void_delegate void_cb2{ std::ref(n) };

	addr = nullptr;
	void_cb1(addr);
	if (&cb1 != addr)
		osd_printf_error("Error testing delegate with functoid requiring adapter %p (expected %p)\n", addr, static_cast<void const *>(&cb1));

	addr = nullptr;
	void_cb2(addr);
	if (&n != addr)
		osd_printf_error("Error testing delegate with functoid requiring adapter %p (expected %p)\n", addr, static_cast<void const *>(&n));

	// test that adaptor is generated after assignment
	void_cb2 = void_cb1;

	addr = nullptr;
	void_cb2(addr);
	if (&cb1 != addr)
		osd_printf_error("Error testing delegate with functoid requiring adapter %p (expected %p)\n", addr, static_cast<void const *>(&cb1));
}

} // anonymous namespace



//-------------------------------------------------
//  get_defstr_index - return the index of the
//  string assuming it is one of the default
//  strings
//-------------------------------------------------

inline int validity_checker::get_defstr_index(const char *string, bool suppress_error)
{
	// check for strings that should be DEF_STR
	auto strindex = m_defstr_map.find(string);
	if (!suppress_error && strindex != m_defstr_map.end() && string != ioport_string_from_index(strindex->second))
		osd_printf_error("Must use DEF_STR( %s )\n", string);
	return (strindex != m_defstr_map.end()) ? strindex->second : 0;
}



//-------------------------------------------------
//  validate_tag - ensure that the given tag
//  meets the general requirements
//-------------------------------------------------

void validity_checker::validate_tag(const char *tag)
{
	// some common names that are now deprecated
	if (strcmp(tag, "main") == 0 || strcmp(tag, "audio") == 0 || strcmp(tag, "sound") == 0 || strcmp(tag, "left") == 0 || strcmp(tag, "right") == 0)
		osd_printf_error("Invalid generic tag '%s' used\n", tag);

	// scan for invalid characters
	static char const *const validchars = "abcdefghijklmnopqrstuvwxyz0123456789_.:^$";
	for (char const *p = tag; *p; ++p)
	{
		// only lower-case permitted
		if (*p != tolower(u8(*p)))
		{
			osd_printf_error("Tag '%s' contains upper-case characters\n", tag);
			break;
		}
		if (*p == ' ')
		{
			osd_printf_error("Tag '%s' contains spaces\n", tag);
			break;
		}
		if (!strchr(validchars, *p))
		{
			osd_printf_error("Tag '%s' contains invalid character '%c'\n",  tag, *p);
			break;
		}
	}

	// find the start of the final tag
	const char *begin = strrchr(tag, ':');
	if (begin == nullptr)
		begin = tag;
	else
		begin += 1;

	// 0-length = bad
	if (*begin == 0)
		osd_printf_error("Found 0-length tag\n");

	// too short/too long = bad
	if (strlen(begin) < MIN_TAG_LENGTH)
		osd_printf_error("Tag '%s' is too short (must be at least %d characters)\n", tag, MIN_TAG_LENGTH);
}



//**************************************************************************
//  VALIDATION FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  validity_checker - constructor
//-------------------------------------------------

validity_checker::validity_checker(emu_options &options, bool quick)
	: m_drivlist(options)
	, m_errors(0)
	, m_warnings(0)
	, m_print_verbose(options.verbose())
	, m_current_driver(nullptr)
	, m_current_device(nullptr)
	, m_current_ioport(nullptr)
	, m_checking_card(false)
	, m_quick(quick)
{
	// pre-populate the defstr map with all the default strings
	for (int strnum = 1; strnum < INPUT_STRING_COUNT; strnum++)
	{
		const char *string = ioport_string_from_index(strnum);
		if (string != nullptr)
			m_defstr_map.insert(std::make_pair(string, strnum));
	}
}

//-------------------------------------------------
//  validity_checker - destructor
//-------------------------------------------------

validity_checker::~validity_checker()
{
	validate_end();
}

//-------------------------------------------------
//  check_driver - check a single driver
//-------------------------------------------------

void validity_checker::check_driver(const game_driver &driver)
{
	// simply validate the one driver
	validate_begin();
	validate_one(driver);
	validate_end();
}


//-------------------------------------------------
//  check_shared_source - check all drivers that
//  share the same source file as the given driver
//-------------------------------------------------

void validity_checker::check_shared_source(const game_driver &driver)
{
	// initialize
	validate_begin();

	// then iterate over all drivers and check the ones that share the same source file
	m_drivlist.reset();
	while (m_drivlist.next())
		if (strcmp(driver.type.source(), m_drivlist.driver().type.source()) == 0)
			validate_one(m_drivlist.driver());

	// cleanup
	validate_end();
}


//-------------------------------------------------
//  check_all_matching - check all drivers whose
//  names match the given string
//-------------------------------------------------

bool validity_checker::check_all_matching(const char *string)
{
	// start by checking core stuff
	validate_begin();
	validate_integer_semantics();
	validate_inlines();
	validate_rgb();
	validate_delegates_mfp();
	validate_delegates_latebind();
	validate_delegates_functoid();

	// if we had warnings or errors, output
	if (m_errors > 0 || m_warnings > 0 || !m_verbose_text.empty())
	{
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "Core: %d errors, %d warnings\n", m_errors, m_warnings);
		if (m_errors > 0)
			output_indented_errors(m_error_text, "Errors");
		if (m_warnings > 0)
			output_indented_errors(m_warning_text, "Warnings");
		if (!m_verbose_text.empty())
			output_indented_errors(m_verbose_text, "Messages");
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "\n");
	}

	// then iterate over all drivers and check them
	m_drivlist.reset();
	bool validated_any = false;
	while (m_drivlist.next())
	{
		if (driver_list::matches(string, m_drivlist.driver().name))
		{
			validate_one(m_drivlist.driver());
			validated_any = true;
		}
	}

	// validate devices
	if (!string)
		validate_device_types();

	// cleanup
	validate_end();

	// if we failed to match anything, it
	if (string && !validated_any)
		throw emu_fatalerror(EMU_ERR_NO_SUCH_SYSTEM, "No matching systems found for '%s'", string);

	return !(m_errors > 0 || m_warnings > 0);
}


//-------------------------------------------------
//  validate_begin - prepare for validation by
//  taking over the output callbacks and resetting
//  our internal state
//-------------------------------------------------

void validity_checker::validate_begin()
{
	// take over error and warning outputs
	osd_output::push(this);

	// reset all our maps
	m_names_map.clear();
	m_descriptions_map.clear();
	m_roms_map.clear();
	m_defstr_map.clear();
	m_region_map.clear();
	m_ioport_set.clear();
	m_slotcard_set.clear();

	// reset internal state
	m_errors = 0;
	m_warnings = 0;
	m_already_checked.clear();
}


//-------------------------------------------------
//  validate_end - restore output callbacks and
//  clean up
//-------------------------------------------------

void validity_checker::validate_end()
{
	// restore the original output callbacks
	osd_output::pop(this);
}


//-------------------------------------------------
//  validate_drivers - master validity checker
//-------------------------------------------------

void validity_checker::validate_one(const game_driver &driver)
{
	// help verbose validation detect configuration-related crashes
	if (m_print_verbose)
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "Validating driver %s (%s)...\n", driver.name, core_filename_extract_base(driver.type.source()));

	// set the current driver
	m_current_driver = &driver;
	m_current_device = nullptr;
	m_current_ioport = nullptr;
	m_region_map.clear();
	m_ioport_set.clear();
	m_checking_card = false;

	// reset error/warning state
	int start_errors = m_errors;
	int start_warnings = m_warnings;
	m_error_text.clear();
	m_warning_text.clear();
	m_verbose_text.clear();

	// wrap in try/catch to catch fatalerrors
	try
	{
		machine_config config(driver, m_blank_options);
		validate_driver(config.root_device());
		validate_roms(config.root_device());
		validate_inputs(config.root_device());
		validate_devices(config);
	}
	catch (emu_fatalerror const &err)
	{
		osd_printf_error("Fatal error %s", err.what());
	}

	// if we had warnings or errors, output
	if (m_errors > start_errors || m_warnings > start_warnings || !m_verbose_text.empty())
	{
		if (!m_print_verbose)
			output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "Driver %s (file %s): ", driver.name, core_filename_extract_base(driver.type.source()));
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "%d errors, %d warnings\n", m_errors - start_errors, m_warnings - start_warnings);
		if (m_errors > start_errors)
			output_indented_errors(m_error_text, "Errors");
		if (m_warnings > start_warnings)
			output_indented_errors(m_warning_text, "Warnings");
		if (!m_verbose_text.empty())
			output_indented_errors(m_verbose_text, "Messages");
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "\n");
	}

	// reset the driver/device
	m_current_driver = nullptr;
	m_current_device = nullptr;
	m_current_ioport = nullptr;
	m_region_map.clear();
	m_ioport_set.clear();
	m_checking_card = false;
}


//-------------------------------------------------
//  validate_driver - validate basic driver
//  information
//-------------------------------------------------

void validity_checker::validate_driver(device_t &root)
{
	// check for duplicate names
	if (!m_names_map.insert(std::make_pair(m_current_driver->name, m_current_driver)).second)
	{
		const game_driver *match = m_names_map.find(m_current_driver->name)->second;
		osd_printf_error("Driver name is a duplicate of %s(%s)\n", core_filename_extract_base(match->type.source()), match->name);
	}

	// check for duplicate descriptions
	if (!m_descriptions_map.insert(std::make_pair(m_current_driver->type.fullname(), m_current_driver)).second)
	{
		const game_driver *match = m_descriptions_map.find(m_current_driver->type.fullname())->second;
		osd_printf_error("Driver description is a duplicate of %s(%s)\n", core_filename_extract_base(match->type.source()), match->name);
	}

	// determine if we are a clone
	bool is_clone = (strcmp(m_current_driver->parent, "0") != 0);
	int clone_of = driver_list::clone(*m_current_driver);
	if (clone_of != -1 && (driver_list::driver(clone_of).flags & machine_flags::IS_BIOS_ROOT))
		is_clone = false;

	// if we have at least 100 drivers, validate the clone
	// (100 is arbitrary, but tries to avoid tiny.mak dependencies)
	if (driver_list::total() > 100 && clone_of == -1 && is_clone)
		osd_printf_error("Driver is a clone of nonexistent driver %s\n", m_current_driver->parent);

	// look for recursive cloning
	if (clone_of != -1 && &driver_list::driver(clone_of) == m_current_driver)
		osd_printf_error("Driver is a clone of itself\n");

	// look for clones that are too deep
	if (clone_of != -1 && (clone_of = driver_list::non_bios_clone(clone_of)) != -1)
		osd_printf_error("Driver is a clone of a clone\n");

	// look for drivers specifying a parent ROM device type
	if (root.type().parent_rom_device_type())
		osd_printf_error("Driver has parent ROM device type '%s'\n", root.type().parent_rom_device_type()->shortname());

	// make sure the driver name is not too long
	if (!is_clone && strlen(m_current_driver->name) > 16)
		osd_printf_error("Parent driver name must be 16 characters or less\n");
	if (is_clone && strlen(m_current_driver->name) > 16)
		osd_printf_error("Clone driver name must be 16 characters or less\n");

	// make sure the driver name doesn't contain invalid characters
	for (const char *s = m_current_driver->name; *s != 0; s++)
		if (((*s < '0') || (*s > '9')) && ((*s < 'a') || (*s > 'z')) && (*s != '_'))
		{
			osd_printf_error("Driver name contains invalid characters\n");
			break;
		}

	// make sure the year is only digits, '?' or '+'
	for (const char *s = m_current_driver->year; *s != 0; s++)
		if (!isdigit(u8(*s)) && *s != '?' && *s != '+')
		{
			osd_printf_error("Driver has an invalid year '%s'\n", m_current_driver->year);
			break;
		}

	// normalize driver->compatible_with
	const char *compatible_with = m_current_driver->compatible_with;
	if (compatible_with != nullptr && strcmp(compatible_with, "0") == 0)
		compatible_with = nullptr;

	// check for this driver being compatible with a nonexistent driver
	if (compatible_with != nullptr && driver_list::find(m_current_driver->compatible_with) == -1)
		osd_printf_error("Driver is listed as compatible with nonexistent driver %s\n", m_current_driver->compatible_with);

	// check for clone_of and compatible_with being specified at the same time
	if (driver_list::clone(*m_current_driver) != -1 && compatible_with != nullptr)
		osd_printf_error("Driver cannot be both a clone and listed as compatible with another system\n");

	// find any recursive dependencies on the current driver
	for (int other_drv = driver_list::compatible_with(*m_current_driver); other_drv != -1; other_drv = driver_list::compatible_with(other_drv))
		if (m_current_driver == &driver_list::driver(other_drv))
		{
			osd_printf_error("Driver is recursively compatible with itself\n");
			break;
		}

	// make sure sound-less drivers are flagged
	device_t::feature_type const unemulated(m_current_driver->type.unemulated_features());
	device_t::feature_type const imperfect(m_current_driver->type.imperfect_features());
	if (!(m_current_driver->flags & (machine_flags::IS_BIOS_ROOT | machine_flags::NO_SOUND_HW)) && !(unemulated & device_t::feature::SOUND))
	{
		speaker_device_enumerator iter(root);
		if (!iter.first())
			osd_printf_error("Driver is missing MACHINE_NO_SOUND or MACHINE_NO_SOUND_HW flag\n");
	}

	// catch invalid flag combinations
	if (unemulated & ~device_t::feature::ALL)
		osd_printf_error("Driver has invalid unemulated feature flags (0x%08X)\n", util::underlying_value(unemulated & ~device_t::feature::ALL));
	if (imperfect & ~device_t::feature::ALL)
		osd_printf_error("Driver has invalid imperfect feature flags (0x%08X)\n", util::underlying_value(imperfect & ~device_t::feature::ALL));
	if (unemulated & imperfect)
		osd_printf_error("Driver cannot have features that are both unemulated and imperfect (0x%08X)\n", util::underlying_value(unemulated & imperfect));
	if ((m_current_driver->flags & machine_flags::NO_SOUND_HW) && ((unemulated | imperfect) & device_t::feature::SOUND))
		osd_printf_error("Machine without sound hardware cannot have unemulated/imperfect sound\n");

	// catch systems marked as supporting save states that contain devices that don't support save states
	if (!(m_current_driver->type.emulation_flags() & device_t::flags::SAVE_UNSUPPORTED))
	{
		std::set<std::add_pointer_t<device_type> > nosave;
		device_enumerator iter(root);
		std::string_view cardtag;
		for (auto &device : iter)
		{
			// ignore any children of a slot card
			if (!cardtag.empty())
			{
				std::string_view tag(device.tag());
				if ((tag.length() > cardtag.length()) && (tag.substr(0, cardtag.length()) == cardtag) && tag[cardtag.length()] == ':')
					continue;
				else
					cardtag = std::string_view();
			}

			// check to see if this is a slot card
			device_t *const parent(device.owner());
			if (parent)
			{
				device_slot_interface *slot;
				parent->interface(slot);
				if (slot && (slot->get_card_device() == &device))
				{
					cardtag = device.tag();
					continue;
				}
			}

			if (device.type().emulation_flags() & device_t::flags::SAVE_UNSUPPORTED)
				nosave.emplace(&device.type());
		}
		if (!nosave.empty())
		{
			std::ostringstream buf;
			for (auto const &devtype : nosave)
				util::stream_format(buf, "%s(%s) %s\n", core_filename_extract_base(devtype->source()), devtype->shortname(), devtype->fullname());
			osd_printf_error("Machine is marked as supporting save states but uses devices that lack save state support:\n%s", std::move(buf).str());
		}
	}
}


//-------------------------------------------------
//  validate_roms - validate ROM definitions
//-------------------------------------------------

void validity_checker::validate_roms(device_t &root)
{
	// iterate, starting with the driver's ROMs and continuing with device ROMs
	for (device_t &device : device_enumerator(root))
	{
		// track the current device
		m_current_device = &device;

		// scan the ROM entries for this device
		char const *last_region_name = "???";
		char const *last_name = "???";
		u32 current_length = 0;
		int items_since_region = 1;
		int last_bios = 0, max_bios = 0;
		std::unordered_map<std::string, int> bios_names;
		std::unordered_map<std::string, std::string> bios_descs;
		char const *defbios = nullptr;
		for (tiny_rom_entry const *romp = device.rom_region(); romp && !ROMENTRY_ISEND(romp); ++romp)
		{
			if (ROMENTRY_ISREGION(romp)) // if this is a region, make sure it's valid, and record the length
			{
				// if we haven't seen any items since the last region, print a warning
				if (items_since_region == 0)
					osd_printf_warning("Empty ROM region '%s' (warning)\n", last_region_name);

				// reset our region tracking states
				char const *const basetag = romp->name;
				items_since_region = (ROMREGION_ISERASE(romp) || ROMREGION_ISDISKDATA(romp)) ? 1 : 0;
				last_region_name = basetag;

				// check for a valid tag
				if (!basetag)
				{
					osd_printf_error("ROM_REGION tag with nullptr name\n");
					continue;
				}

				// validate the base tag
				validate_tag(basetag);

				// generate the full tag
				std::string const fulltag = device.subtag(romp->name);

				// attempt to add it to the map, reporting duplicates as errors
				current_length = ROMREGION_GETLENGTH(romp);
				if (!m_region_map.emplace(fulltag, current_length).second)
					osd_printf_error("Multiple ROM_REGIONs with the same tag '%s' defined\n", fulltag);
				if (current_length == 0)
					osd_printf_error("ROM region '%s' has zero length\n", fulltag);
			}
			else if (ROMENTRY_ISSYSTEM_BIOS(romp)) // If this is a system bios, make sure it is using the next available bios number
			{
				int const bios_flags = ROM_GETBIOSFLAGS(romp);
				char const *const biosname = romp->name;
				if (bios_flags != last_bios + 1)
					osd_printf_error("Non-sequential BIOS %s (specified as %d, expected to be %d)\n", biosname ? biosname : "UNNAMED", bios_flags - 1, last_bios);
				last_bios = bios_flags;

				// validate the name
				if (!biosname || biosname[0] == 0)
					osd_printf_error("BIOS %d is missing a name\n", bios_flags - 1);
				else
				{
					if (strlen(biosname) > 16)
						osd_printf_error("BIOS name %s exceeds maximum 16 characters\n", biosname);
					for (char const *s = biosname; *s; ++s)
					{
						if (((*s < '0') || (*s > '9')) && ((*s < 'a') || (*s > 'z')) && (*s != '.') && (*s != '_') && (*s != '-'))
						{
							osd_printf_error("BIOS name %s contains invalid characters\n", biosname);
							break;
						}
					}

					// check for duplicate names/descriptions
					auto const nameins = bios_names.emplace(biosname, bios_flags);
					if (!nameins.second)
						osd_printf_error("Duplicate BIOS name %s specified (%d and %d)\n", biosname, nameins.first->second, bios_flags - 1);
					if (!romp->hashdata || romp->hashdata[0] == 0)
						osd_printf_error("BIOS %s has empty description\n", biosname);
					else
					{
						auto const descins = bios_descs.emplace(romp->hashdata, biosname);
						if (!descins.second)
							osd_printf_error("BIOS %s has duplicate description '%s' (was %s)\n", biosname, romp->hashdata, descins.first->second);
					}
				}
			}
			else if (ROMENTRY_ISDEFAULT_BIOS(romp)) // if this is a default BIOS setting, remember it so it to check at the end
			{
				defbios = romp->name;
			}
			else if (ROMENTRY_ISFILE(romp)) // if this is a file, make sure it is properly formatted
			{
				// track the last filename we found
				last_name = romp->name;
				max_bios = std::max<int>(max_bios, ROM_GETBIOSFLAGS(romp));

				// validate the name
				if (strlen(last_name) > 127)
					osd_printf_error("ROM label %s exceeds maximum 127 characters\n", last_name);
				for (char const *s = last_name; *s; ++s)
				{
					if (((*s < '0') || (*s > '9')) && ((*s < 'a') || (*s > 'z')) && (*s != ' ') && (*s != '@') && (*s != '.') && (*s != ',') && (*s != '_') && (*s != '-') && (*s != '+') && (*s != '='))
					{
						osd_printf_error("ROM label %s contains invalid characters\n", last_name);
						break;
					}
				}

				// make sure the hash is valid
				util::hash_collection hashes;
				if (!hashes.from_internal_string(romp->hashdata))
					osd_printf_error("ROM '%s' has an invalid hash string '%s'\n", last_name, romp->hashdata);
			}

			// for any non-region ending entries, make sure they don't extend past the end
			if (!ROMENTRY_ISREGIONEND(romp) && current_length > 0)
			{
				items_since_region++;
				if (!ROMENTRY_ISIGNORE(romp) && (ROM_GETOFFSET(romp) + ROM_GETLENGTH(romp) > current_length))
					osd_printf_error("ROM '%s' extends past the defined memory region\n", last_name);
			}
		}

		// if we haven't seen any items since the last region, print a warning
		if (items_since_region == 0)
			osd_printf_warning("Empty ROM region '%s' (warning)\n", last_region_name);

		// check that default BIOS exists
		if (defbios && (bios_names.find(defbios) == bios_names.end()))
			osd_printf_error("Default BIOS '%s' not found\n", defbios);
		if (!device.get_default_bios_tag().empty() && (bios_names.find(device.get_default_bios_tag()) == bios_names.end()))
			osd_printf_error("Configured BIOS '%s' not found\n", device.get_default_bios_tag());

		// check that there aren't ROMs for a non-existent BIOS option
		if (max_bios > last_bios)
			osd_printf_error("BIOS %d set on file is higher than maximum system BIOS number %d\n", max_bios - 1, last_bios - 1);

		// final check for empty regions
		if (items_since_region == 0)
			osd_printf_warning("Empty ROM region '%s' (warning)\n", last_region_name);

		// reset the current device
		m_current_device = nullptr;
	}
}


//-------------------------------------------------
//  validate_analog_input_field - validate an
//  analog input field
//-------------------------------------------------

void validity_checker::validate_analog_input_field(const ioport_field &field)
{
	// analog ports must have a valid sensitivity
	if (field.sensitivity() == 0)
		osd_printf_error("Analog port with zero sensitivity\n");

	// check that the default falls in the bitmask range
	if (field.defvalue() & ~field.mask())
		osd_printf_error("Analog port with a default value (%X) out of the bitmask range (%X)\n", field.defvalue(), field.mask());

	// tests for positional devices
	if (field.type() == IPT_POSITIONAL || field.type() == IPT_POSITIONAL_V)
	{
		int shift;
		for (shift = 0; shift <= 31 && (~field.mask() & (1 << shift)) != 0; shift++) { }

		// convert the positional max value to be in the bitmask for testing
		//s32 analog_max = field.maxval();
		//analog_max = (analog_max - 1) << shift;

		// positional port size must fit in bits used
		if ((field.mask() >> shift) + 1 < field.maxval())
			osd_printf_error("Analog port with a positional port size bigger then the mask size\n");
	}

	// tests for absolute devices
	else if (field.type() > IPT_ANALOG_ABSOLUTE_FIRST && field.type() < IPT_ANALOG_ABSOLUTE_LAST)
	{
		// adjust for signed values
		s32 default_value = field.defvalue();
		s32 analog_min = field.minval();
		s32 analog_max = field.maxval();
		if (analog_min > analog_max)
		{
			analog_min = -analog_min;
			if (default_value > analog_max)
				default_value = -default_value;
		}

		// check that the default falls in the MINMAX range
		if (default_value < analog_min || default_value > analog_max)
			osd_printf_error("Analog port with a default value (%X) out of PORT_MINMAX range (%X-%X)\n", field.defvalue(), field.minval(), field.maxval());

		// check that the MINMAX falls in the bitmask range
		// we use the unadjusted min for testing
		if (field.minval() & ~field.mask() || analog_max & ~field.mask())
			osd_printf_error("Analog port with a PORT_MINMAX (%X-%X) value out of the bitmask range (%X)\n", field.minval(), field.maxval(), field.mask());

		// absolute analog ports do not use PORT_RESET
		if (field.analog_reset())
			osd_printf_error("Absolute analog port using PORT_RESET\n");

		// absolute analog ports do not use PORT_WRAPS
		if (field.analog_wraps())
			osd_printf_error("Absolute analog port using PORT_WRAPS\n");
	}

	// tests for non IPT_POSITIONAL relative devices
	else
	{
		// relative devices do not use PORT_MINMAX
		if (field.minval() != 0 || field.maxval() != field.mask())
			osd_printf_error("Relative port using PORT_MINMAX\n");

		// relative devices do not use a default value
		// the counter is at 0 on power up
		if (field.defvalue() != 0)
			osd_printf_error("Relative port using non-0 default value\n");

		// relative analog ports do not use PORT_WRAPS
		if (field.analog_wraps())
			osd_printf_error("Absolute analog port using PORT_WRAPS\n");
	}
}


//-------------------------------------------------
//  validate_dip_settings - validate a DIP switch
//  setting
//-------------------------------------------------

void validity_checker::validate_dip_settings(const ioport_field &field)
{
	char const *const demo_sounds = ioport_string_from_index(INPUT_STRING_Demo_Sounds);
	char const *const flipscreen = ioport_string_from_index(INPUT_STRING_Flip_Screen);
	char const *const name = field.specific_name() ? field.specific_name() : "UNNAMED";
	u8 coin_list[__input_string_coinage_end + 1 - __input_string_coinage_start] = { 0 };
	bool coin_error = false;

	// iterate through the settings
	for (auto setting = field.settings().begin(); field.settings().end() != setting; ++setting)
	{
		// note any coinage strings
		int strindex = get_defstr_index(setting->name());
		if (strindex >= __input_string_coinage_start && strindex <= __input_string_coinage_end)
			coin_list[strindex - __input_string_coinage_start] = 1;

		// make sure demo sounds default to on
		if (name == demo_sounds && strindex == INPUT_STRING_On && field.defvalue() != setting->value())
			osd_printf_error("Demo Sounds must default to On\n");

		// check for bad demo sounds options
		if (name == demo_sounds && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
			osd_printf_error("Demo Sounds option must be Off/On, not %s\n", setting->name());

		// check for bad flip screen options
		if (name == flipscreen && (strindex == INPUT_STRING_Yes || strindex == INPUT_STRING_No))
			osd_printf_error("Flip Screen option must be Off/On, not %s\n", setting->name());

		// if we have a neighbor, compare ourselves to him
		auto const nextsetting = std::next(setting);
		if (field.settings().end() != nextsetting)
		{
			// check for inverted off/on DIP switch order
			int next_strindex = get_defstr_index(nextsetting->name(), true);
			if (strindex == INPUT_STRING_On && next_strindex == INPUT_STRING_Off)
				osd_printf_error("%s option must have Off/On options in the order: Off, On\n", name);

			// check for inverted yes/no DIP switch order
			else if (strindex == INPUT_STRING_Yes && next_strindex == INPUT_STRING_No)
				osd_printf_error("%s option must have Yes/No options in the order: No, Yes\n", name);

			// check for inverted upright/cocktail DIP switch order
			else if (strindex == INPUT_STRING_Cocktail && next_strindex == INPUT_STRING_Upright)
				osd_printf_error("%s option must have Upright/Cocktail options in the order: Upright, Cocktail\n", name);

			// check for proper coin ordering
			else if (strindex >= __input_string_coinage_start && strindex <= __input_string_coinage_end && next_strindex >= __input_string_coinage_start && next_strindex <= __input_string_coinage_end &&
						strindex >= next_strindex && setting->condition() == nextsetting->condition())
			{
				osd_printf_error("%s option has unsorted coinage %s > %s\n", name, setting->name(), nextsetting->name());
				coin_error = true;
			}
		}
	}

	// if we have a coin error, demonstrate the correct way
	if (coin_error)
	{
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "   Note proper coin sort order should be:\n");
		for (int entry = 0; entry < std::size(coin_list); entry++)
			if (coin_list[entry])
				output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "      %s\n", ioport_string_from_index(__input_string_coinage_start + entry));
	}
}


//-------------------------------------------------
//  validate_condition - validate a condition
//  stored within an ioport field or setting
//-------------------------------------------------

void validity_checker::validate_condition(const ioport_condition &condition, device_t &device)
{
	if (condition.tag() == nullptr)
	{
		osd_printf_error("Condition referencing null ioport tag\n");
		return;
	}

	// resolve the tag, then find a matching port
	if (m_ioport_set.find(device.subtag(condition.tag())) == m_ioport_set.end())
		osd_printf_error("Condition referencing non-existent ioport tag '%s'\n", condition.tag());
}


//-------------------------------------------------
//  validate_inputs - validate input configuration
//-------------------------------------------------

void validity_checker::validate_inputs(device_t &root)
{
	// iterate over devices
	for (device_t &device : device_enumerator(root))
	{
		// see if this device has ports; if not continue
		if (device.input_ports() == nullptr)
			continue;

		// track the current device
		m_current_device = &device;

		// allocate the input ports
		ioport_list portlist;
		{
			// report any errors during construction
			std::ostringstream errorbuf;
			portlist.append(device, errorbuf);
			if (errorbuf.tellp())
				osd_printf_error("I/O port error during construction:\n%s\n", std::move(errorbuf).str());
		}

		// do a first pass over ports to add their names and find duplicates
		for (auto &port : portlist)
			if (!m_ioport_set.insert(port.second->tag()).second)
				osd_printf_error("Multiple I/O ports with the same tag '%s' defined\n", port.second->tag());

		// iterate over ports
		for (auto &port : portlist)
		{
			m_current_ioport = port.second->tag();

			// scan for invalid characters
			static char const *const validchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.:^$";
			for (char const *p = m_current_ioport; *p; ++p)
			{
				if (*p == ' ')
				{
					osd_printf_error("Tag '%s' contains spaces\n", m_current_ioport);
					break;
				}
				if (!strchr(validchars, *p))
				{
					osd_printf_error("Tag '%s' contains invalid character '%c'\n",  m_current_ioport, *p);
					break;
				}
			}

			// iterate through the fields on this port
			for (ioport_field const &field : port.second->fields())
			{
				// verify analog inputs
				if (field.is_analog())
					validate_analog_input_field(field);

				// checks based on field type
				if ((field.type() > IPT_UI_FIRST) && (field.type() < IPT_UI_LAST))
				{
					osd_printf_error("Field has invalid UI control type\n");
				}
				else if (field.type() == IPT_INVALID)
				{
					osd_printf_error("Field has an invalid type (0); use IPT_OTHER instead\n");
				}
				else if (field.type() == IPT_SPECIAL)
				{
					osd_printf_error("Field has an invalid type IPT_SPECIAL\n");
				}
				else if (field.type() == IPT_DIPSWITCH)
				{
					// DIP switch fields must have a specific name
					if (field.specific_name() == nullptr)
						osd_printf_error("DIP switch has no specific name\n");

					// verify the settings list
					validate_dip_settings(field);
				}
				else if (field.type() == IPT_CONFIG)
				{
					// config fields must have a specific name
					if (field.specific_name() == nullptr)
						osd_printf_error("Config switch has no specific name\n");
				}
				else if (field.type() == IPT_ADJUSTER)
				{
					// adjuster fields must have a specific name
					if (field.specific_name() == nullptr)
						osd_printf_error("Adjuster has no specific name\n");
				}

				// verify names
				char const *const name = field.specific_name();
				if (name != nullptr)
				{
					// check for empty string
					if (name[0] == 0)
						osd_printf_error("Field name is an empty string\n");

					// check for trailing spaces
					if (name[0] != 0 && name[strlen(name) - 1] == ' ')
						osd_printf_error("Field '%s' has trailing spaces\n", name);

					// check for invalid UTF-8
					if (!utf8_is_valid_string(name))
						osd_printf_error("Field '%s' has invalid characters\n", name);
				}

				// verify conditions on the field
				if (!field.condition().none())
					validate_condition(field.condition(), device);

				// verify conditions on the settings
				for (ioport_setting const &setting : field.settings())
					if (!setting.condition().none())
						validate_condition(setting.condition(), device);

				// verify natural keyboard codes
				for (int which = 0; which < 1 << (UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1); which++)
				{
					std::vector<char32_t> codes = field.keyboard_codes(which);
					for (char32_t code : codes)
					{
						if (!uchar_isvalid(code))
						{
							osd_printf_error("Field '%s' has non-character U+%04X in PORT_CHAR(%d)\n",
									name,
									uint32_t(code),
									int32_t(code));
						}
					}
				}
			}

			// done with this port
			m_current_ioport = nullptr;
		}

		// validate the default settings
		const input_device_default *def = device.input_ports_defaults();
		if (def != nullptr)
		{
			for ( ; def->tag; def++)
			{
				if (def->defvalue & ~def->mask)
					osd_printf_error("Default value 0x%x for field of port '%s' lies outside mask 0x%x\n", def->defvalue, def->mask);

				const auto it = portlist.find(device.subtag(def->tag));
				if (portlist.end() == it)
				{
					osd_printf_error("Default specified for nonexistent port '%s'\n", def->tag);
				}
				else
				{
					const auto &fields = it->second->fields();
					if (fields.end() == std::find_if(fields.begin(), fields.end(), [def] (const ioport_field &field) { return field.mask() == def->mask; }))
					{
						osd_printf_error(
								"Default value specified for field with mask 0x%x in port '%s' but no corresponding field found\n",
								def->mask,
								def->tag);
					}
				}
			}
		}

		// done with this device
		m_current_device = nullptr;
	}
}


//-------------------------------------------------
//  validate_devices - run per-device validity
//  checks
//-------------------------------------------------

void validity_checker::validate_devices(machine_config &config)
{
	std::unordered_set<std::string> device_map;

	for (device_t &device : device_enumerator(config.root_device()))
	{
		// track the current device
		m_current_device = &device;

		// validate auto-finders
		device.findit(this);

		// validate the device tag
		validate_tag(device.basetag());

		// look for duplicates
		bool duplicate = !device_map.insert(device.tag()).second;
		if (duplicate)
			osd_printf_error("Multiple devices with the same tag defined\n");

		// check for device-specific validity check
		device.validity_check(*this);

		// done with this device
		m_current_device = nullptr;

		// if it's a slot, iterate over possible cards (don't recurse, or you'll stack infinite tee connectors)
		device_slot_interface *const slot = dynamic_cast<device_slot_interface *>(&device);
		if (slot && !slot->fixed() && !duplicate)
		{
			for (auto &option : slot->option_list())
			{
				// the default option is already instantiated here, so don't try adding it again
				if (slot->default_option() != nullptr && option.first == slot->default_option())
					continue;

				// if we need to save time, instantiate and validate each slot card type at most once
				if (m_quick && !m_slotcard_set.insert(option.second->devtype().shortname()).second)
					continue;

				m_checking_card = true;
				device_t *card;
				{
					machine_config::token const tok(config.begin_configuration(slot->device()));
					card = config.device_add(option.second->name(), option.second->devtype(), option.second->clock());

					const char *const def_bios = option.second->default_bios();
					if (def_bios)
						card->set_default_bios_tag(def_bios);
					auto const &additions = option.second->machine_config();
					if (additions)
						additions(card);
					input_device_default const *input_def = option.second->input_device_defaults();
					if (input_def)
						card->set_input_default(input_def);
				}

				for (device_slot_interface &subslot : slot_interface_enumerator(*card))
				{
					if (subslot.fixed())
					{
						// TODO: make this self-contained so it can apply itself
						device_slot_interface::slot_option const *suboption = subslot.option(subslot.default_option());
						if (suboption)
						{
							machine_config::token const tok(config.begin_configuration(subslot.device()));
							device_t *const sub_card = config.device_add(suboption->name(), suboption->devtype(), suboption->clock());
							const char *const sub_bios = suboption->default_bios();
							if (sub_bios)
								sub_card->set_default_bios_tag(sub_bios);
							auto sub_additions = suboption->machine_config();
							if (sub_additions)
								sub_additions(sub_card);
						}
					}
				}

				for (device_t &card_dev : device_enumerator(*card))
					card_dev.config_complete();
				validate_roms(*card);
				validate_inputs(*card);

				for (device_t &card_dev : device_enumerator(*card))
				{
					m_current_device = &card_dev;
					card_dev.findit(this);
					validate_tag(card_dev.basetag());
					card_dev.validity_check(*this);
					m_current_device = nullptr;
				}

				machine_config::token const tok(config.begin_configuration(slot->device()));
				config.device_remove(option.second->name());
				m_checking_card = false;
			}
		}
	}
}


//-------------------------------------------------
//  validate_devices_types - check validity of
//  registered device types
//-------------------------------------------------

void validity_checker::validate_device_types()
{
	// reset error/warning state
	int start_errors = m_errors;
	int start_warnings = m_warnings;
	m_error_text.clear();
	m_warning_text.clear();
	m_verbose_text.clear();

	std::unordered_map<std::string, std::add_pointer_t<device_type> > device_name_map, device_shortname_map;
	machine_config config(GAME_NAME(___empty), m_drivlist.options());
	machine_config::token const tok(config.begin_configuration(config.root_device()));
	for (device_type type : registered_device_types)
	{
		device_t *const dev = config.device_add(type.shortname(), type, 0);

		char const *name((dev->shortname() && *dev->shortname()) ? dev->shortname() : type.type().name());
		std::string const description((dev->source() && *dev->source()) ? util::string_format("%s(%s)", core_filename_extract_base(dev->source()), name) : name);

		if (m_print_verbose)
			output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "Validating device %s...\n", description);

		// ensure shortname exists
		if (!dev->shortname() || !*dev->shortname())
		{
			osd_printf_error("Device %s does not have short name defined\n", description);
		}
		else
		{
			// make sure the device name is not too long
			if (strlen(dev->shortname()) > 32)
				osd_printf_error("Device short name must be 32 characters or less\n");

			// check for invalid characters in shortname
			for (char const *s = dev->shortname(); *s; ++s)
			{
				if (((*s < '0') || (*s > '9')) && ((*s < 'a') || (*s > 'z')) && (*s != '_'))
				{
					osd_printf_error("Device %s short name contains invalid characters\n", description);
					break;
				}
			}

			// check for name conflicts
			std::string tmpname(dev->shortname());
			game_driver_map::const_iterator const drvname(m_names_map.find(tmpname));
			auto const devname(device_shortname_map.emplace(std::move(tmpname), &type));
			if (m_names_map.end() != drvname)
			{
				game_driver const &dup(*drvname->second);
				osd_printf_error("Device %s short name is a duplicate of %s(%s)\n", description, core_filename_extract_base(dup.type.source()), dup.name);
			}
			else if (!devname.second)
			{
				device_t *const dup = config.device_add("_dup", *devname.first->second, 0);
				osd_printf_error("Device %s short name is a duplicate of %s(%s)\n", description, core_filename_extract_base(dup->source()), dup->shortname());
				config.device_remove("_dup");
			}
		}

		// ensure name exists
		if (!dev->name() || !*dev->name())
		{
			osd_printf_error("Device %s does not have name defined\n", description);
		}
		else
		{
			// check for description conflicts
			std::string tmpdesc(dev->name());
			game_driver_map::const_iterator const drvdesc(m_descriptions_map.find(tmpdesc));
			auto const devdesc(device_name_map.emplace(std::move(tmpdesc), &type));
			if (m_descriptions_map.end() != drvdesc)
			{
				game_driver const &dup(*drvdesc->second);
				osd_printf_error("Device %s name '%s' is a duplicate of %s(%s)\n", description, dev->name(), core_filename_extract_base(dup.type.source()), dup.name);
			}
			else if (!devdesc.second)
			{
				device_t *const dup = config.device_add("_dup", *devdesc.first->second, 0);
				osd_printf_error("Device %s name '%s' is a duplicate of %s(%s)\n", description, dev->name(), core_filename_extract_base(dup->source()), dup->shortname());
				config.device_remove("_dup");
			}
		}

		// ensure source exists
		if (!dev->source() || !*dev->source())
			osd_printf_error("Device %s does not have source defined\n", description);

		// check that reported type matches supplied type
		if (dev->type().type() != type.type())
			osd_printf_error("Device %s reports type '%s' (created with '%s')\n", description, dev->type().type().name(), type.type().name());

		// catch invalid flag combinations
		device_t::feature_type const unemulated(dev->type().unemulated_features());
		device_t::feature_type const imperfect(dev->type().imperfect_features());
		if (unemulated & ~device_t::feature::ALL)
			osd_printf_error("Device has invalid unemulated feature flags (0x%08X)\n", util::underlying_value(unemulated & ~device_t::feature::ALL));
		if (imperfect & ~device_t::feature::ALL)
			osd_printf_error("Device has invalid imperfect feature flags (0x%08X)\n", util::underlying_value(imperfect & ~device_t::feature::ALL));
		if (unemulated & imperfect)
			osd_printf_error("Device cannot have features that are both unemulated and imperfect (0x%08X)\n", util::underlying_value(unemulated & imperfect));

		// check that parents are only ever one generation deep
		auto const parent(dev->type().parent_rom_device_type());
		if (parent)
		{
			auto const grandparent(parent->parent_rom_device_type());
			if ((dev->type() == *parent) || !strcmp(parent->shortname(), name))
				osd_printf_error("Device has parent ROM set that identical to its type\n");
			if (grandparent)
				osd_printf_error("Device has parent ROM set '%s' which has parent ROM set '%s'\n", parent->shortname(), grandparent->shortname());
		}

		// give devices some of the same scrutiny that drivers get - necessary for cards not default for any slots
		validate_roms(*dev);
		validate_inputs(*dev);

		// reset the device
		m_current_device = nullptr;
		m_current_ioport = nullptr;
		m_region_map.clear();
		m_ioport_set.clear();

		// remove the device in preparation for re-using the machine configuration
		config.device_remove(type.shortname());
	}

	// if we had warnings or errors, output
	if (m_errors > start_errors || m_warnings > start_warnings || !m_verbose_text.empty())
	{
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "%d errors, %d warnings\n", m_errors - start_errors, m_warnings - start_warnings);
		if (m_errors > start_errors)
			output_indented_errors(m_error_text, "Errors");
		if (m_warnings > start_warnings)
			output_indented_errors(m_warning_text, "Warnings");
		if (!m_verbose_text.empty())
			output_indented_errors(m_verbose_text, "Messages");
		output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "\n");
	}
}


//-------------------------------------------------
//  build_output_prefix - create a prefix
//  indicating the current source file, driver,
//  and device
//-------------------------------------------------

void validity_checker::build_output_prefix(std::ostream &str) const
{
	// if we have a current (non-root) device, indicate that
	if (m_current_device && m_current_device->owner())
		util::stream_format(str, "%s device '%s': ", m_current_device->name(), m_current_device->tag() + 1);

	// if we have a current port, indicate that as well
	if (m_current_ioport)
		util::stream_format(str, "ioport '%s': ", m_current_ioport);
}


//-------------------------------------------------
//  error_output - error message output override
//-------------------------------------------------

void validity_checker::output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args)
{
	std::ostringstream output;
	switch (channel)
	{
	case OSD_OUTPUT_CHANNEL_ERROR:
		// count the error
		m_errors++;

		// output the source(driver) device 'tag'
		build_output_prefix(output);

		// generate the string
		util::stream_format(output, args);
		m_error_text.append(output.str());
		break;

	case OSD_OUTPUT_CHANNEL_WARNING:
		// count the error
		m_warnings++;

		// output the source(driver) device 'tag'
		build_output_prefix(output);

		// generate the string and output to the original target
		util::stream_format(output, args);
		m_warning_text.append(output.str());
		break;

	case OSD_OUTPUT_CHANNEL_VERBOSE:
		// if we're not verbose, skip it
		if (!m_print_verbose) break;

		// output the source(driver) device 'tag'
		build_output_prefix(output);

		// generate the string and output to the original target
		util::stream_format(output, args);
		m_verbose_text.append(output.str());
		break;

	default:
		chain_output(channel, args);
		break;
	}
}

//-------------------------------------------------
//  output_via_delegate - helper to output a
//  message via a varargs string, so the argptr
//  can be forwarded onto the given delegate
//-------------------------------------------------

template <typename Format, typename... Params>
void validity_checker::output_via_delegate(osd_output_channel channel, Format &&fmt, Params &&...args)
{
	// call through to the delegate with the proper parameters
	chain_output(channel, util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

//-------------------------------------------------
//  output_indented_errors - helper to output error
//  and warning messages with header and indents
//-------------------------------------------------
void validity_checker::output_indented_errors(std::string &text, const char *header)
{
	// remove trailing newline
	if (text[text.size()-1] == '\n')
		text.erase(text.size()-1, 1);
	strreplace(text, "\n", "\n   ");
	output_via_delegate(OSD_OUTPUT_CHANNEL_ERROR, "%s:\n   %s\n", header, text);
}
