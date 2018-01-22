#include "catch.hpp"
#include "emucore.h"
#include "video/rgbutil.h"


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


TEST_CASE("check rgb", "[emu][video]")
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
	    static bilinear_filter(u32, u32, u32, u32, u8, u8)
	    bilinear_filter_rgbaint(u32, u32, u32, u32, u8, u8)
	*/

	auto random_i32_nolimit = []
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
	auto check_expected = [&] ()
	{
		const volatile s32 a = rgb.get_a32();
		const volatile s32 r = rgb.get_r32();
		const volatile s32 g = rgb.get_g32();
		const volatile s32 b = rgb.get_b32();
		REQUIRE(a == expected_a);
		(r == expected_r);
		REQUIRE(g == expected_g);
		REQUIRE(b == expected_b);
	};

	// check set/get
	SECTION("rgbaint_t::set(a, r, g, b)") 
	{
		expected_a = random_i32();
		expected_r = random_i32();
		expected_g = random_i32();
		expected_b = random_i32();
		rgb.set(expected_a, expected_r, expected_g, expected_b);
		
		check_expected();
	}

	// check construct/set
	SECTION("rgbaint_t::set(rgbaint_t)") 
	{	
		expected_a = random_i32();
		expected_r = random_i32();
		expected_g = random_i32();
		expected_b = random_i32();
		rgb.set(rgbaint_t(expected_a, expected_r, expected_g, expected_b));
		check_expected();
	}
	
	// check construct/assign
	SECTION("rgbaint_t assignment") 
	{	
		expected_a = random_i32();
		expected_r = random_i32();
		expected_g = random_i32();
		expected_b = random_i32();
		rgb = rgbaint_t(expected_a, expected_r, expected_g, expected_b);
		check_expected();
	}

	// check piecewise set
	SECTION("rgbaint_t::set_a") 
	{	
		rgb.set_a(expected_a = random_i32());
		check_expected();
	}
	
	SECTION("rgbaint_t::set_r") 
	{	
		rgb.set_r(expected_r = random_i32());
		check_expected();
	}

	SECTION("rgbaint_t::set_g") 
	{	
		rgb.set_g(expected_g = random_i32());
		check_expected();
	}
	
	SECTION("rgbaint_t::set_b") 
	{	
		rgb.set_b(expected_b = random_i32());
		check_expected();
	}
	
	// test merge_alpha
	SECTION("rgbaint_t::merge_alpha") 
	{	
		expected_a = rand();
		rgb.merge_alpha(rgbaint_t(expected_a, rand(), rand(), rand()));
		check_expected();
	}

	// test RGB addition (method)
	SECTION("rgbaint_t::add") 
	{	
		expected_a += actual_a = random_i32();
		expected_r += actual_r = random_i32();
		expected_g += actual_g = random_i32();
		expected_b += actual_b = random_i32();
		rgb.add(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	
	// test RGB addition (operator)
	SECTION("rgbaint_t::operator+=") 
	{	
		expected_a += actual_a = random_i32();
		expected_r += actual_r = random_i32();
		expected_g += actual_g = random_i32();
		expected_b += actual_b = random_i32();
		rgb += rgbaint_t(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test offset addition (method)
	SECTION("rgbaint_t::add_imm") 
	{	
		imm = random_i32();
		expected_a += imm;
		expected_r += imm;
		expected_g += imm;
		expected_b += imm;
		rgb.add_imm(imm);
		check_expected();
	}

	// test offset addition (operator)
	SECTION("rgbaint_t::operator+=") 
	{	
		imm = random_i32();
		expected_a += imm;
		expected_r += imm;
		expected_g += imm;
		expected_b += imm;
		rgb += imm;
		check_expected();
	}
	
	// test immediate RGB addition
	SECTION("rgbaint_t::add_imm_rgba") 
	{	
		expected_a += actual_a = random_i32();
		expected_r += actual_r = random_i32();
		expected_g += actual_g = random_i32();
		expected_b += actual_b = random_i32();
		rgb.add_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test RGB subtraction (method)
	SECTION("rgbaint_t::sub") 
	{	
		expected_a -= actual_a = random_i32();
		expected_r -= actual_r = random_i32();
		expected_g -= actual_g = random_i32();
		expected_b -= actual_b = random_i32();
		rgb.sub(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	
	// test RGB subtraction (operator)
	SECTION("rgbaint_t::operator-=") 
	{	
		expected_a -= actual_a = random_i32();
		expected_r -= actual_r = random_i32();
		expected_g -= actual_g = random_i32();
		expected_b -= actual_b = random_i32();
		rgb -= rgbaint_t(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test offset subtraction
	SECTION("rgbaint_t::sub_imm") 
	{	
		imm = random_i32();
		expected_a -= imm;
		expected_r -= imm;
		expected_g -= imm;
		expected_b -= imm;
		rgb.sub_imm(imm);
		check_expected();
	}
	
	// test immediate RGB subtraction
	SECTION("rgbaint_t::sub_imm_rgba") 
	{	
		expected_a -= actual_a = random_i32();
		expected_r -= actual_r = random_i32();
		expected_g -= actual_g = random_i32();
		expected_b -= actual_b = random_i32();
		rgb.sub_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test reversed RGB subtraction
	SECTION("rgbaint_t::subr") 
	{	
		expected_a = (actual_a = random_i32()) - expected_a;
		expected_r = (actual_r = random_i32()) - expected_r;
		expected_g = (actual_g = random_i32()) - expected_g;
		expected_b = (actual_b = random_i32()) - expected_b;
		rgb.subr(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	
	// test reversed offset subtraction
	SECTION("rgbaint_t::subr_imm") 
	{	
		imm = random_i32();
		expected_a = imm - expected_a;
		expected_r = imm - expected_r;
		expected_g = imm - expected_g;
		expected_b = imm - expected_b;
		rgb.subr_imm(imm);
		check_expected();
	}
	
	// test reversed immediate RGB subtraction
	SECTION("rgbaint_t::subr_imm_rgba") 
	{	
		expected_a = (actual_a = random_i32()) - expected_a;
		expected_r = (actual_r = random_i32()) - expected_r;
		expected_g = (actual_g = random_i32()) - expected_g;
		expected_b = (actual_b = random_i32()) - expected_b;
		rgb.subr_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}

	// test RGB multiplication (method)
	SECTION("rgbaint_t::mul") 
	{	
		expected_a *= actual_a = random_i32();
		expected_r *= actual_r = random_i32();
		expected_g *= actual_g = random_i32();
		expected_b *= actual_b = random_i32();
		rgb.mul(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	
	// test RGB multiplication (operator)
	SECTION("rgbaint_t::operator*=") 
	{	
		expected_a *= actual_a = random_i32();
		expected_r *= actual_r = random_i32();
		expected_g *= actual_g = random_i32();
		expected_b *= actual_b = random_i32();
		rgb *= rgbaint_t(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}

	// test factor multiplication (method)
	SECTION("rgbaint_t::mul_imm") 
	{	
		imm = random_i32();
		expected_a *= imm;
		expected_r *= imm;
		expected_g *= imm;
		expected_b *= imm;
		rgb.mul_imm(imm);
		check_expected();
	}

	// test factor multiplication (operator)
	SECTION("rgbaint_t::operator*=") 
	{	
		imm = random_i32();
		expected_a *= imm;
		expected_r *= imm;
		expected_g *= imm;
		expected_b *= imm;
		rgb *= imm;
		check_expected();
	}
	
	// test immediate RGB multiplication
	SECTION("rgbaint_t::mul_imm_rgba") 
	{	
		expected_a *= actual_a = random_i32();
		expected_r *= actual_r = random_i32();
		expected_g *= actual_g = random_i32();
		expected_b *= actual_b = random_i32();
		rgb.mul_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}

	// test RGB and not
	SECTION("rgbaint_t::andnot_reg") 
	{	
		expected_a &= ~(actual_a = random_i32());
		expected_r &= ~(actual_r = random_i32());
		expected_g &= ~(actual_g = random_i32());
		expected_b &= ~(actual_b = random_i32());
		rgb.andnot_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	
	// test RGB or
	SECTION("rgbaint_t::or_reg") 
	{	
		expected_a |= actual_a = random_i32();
		expected_r |= actual_r = random_i32();
		expected_g |= actual_g = random_i32();
		expected_b |= actual_b = random_i32();
		rgb.or_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}

	// test RGB and
	SECTION("rgbaint_t::and_reg") 
	{	
		expected_a &= actual_a = random_i32();
		expected_r &= actual_r = random_i32();
		expected_g &= actual_g = random_i32();
		expected_b &= actual_b = random_i32();
		rgb.and_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}

	// test RGB xor
	SECTION("rgbaint_t::xor_reg") 
	{	
		expected_a ^= actual_a = random_i32();
		expected_r ^= actual_r = random_i32();
		expected_g ^= actual_g = random_i32();
		expected_b ^= actual_b = random_i32();
		rgb.xor_reg(rgbaint_t(actual_a, actual_r, actual_g, actual_b));
		check_expected();
	}
	// test uniform or
	SECTION("rgbaint_t::or_imm") 
	{	
		imm = random_i32();
		expected_a |= imm;
		expected_r |= imm;
		expected_g |= imm;
		expected_b |= imm;
		rgb.or_imm(imm);
		check_expected();
	}
	
	// test uniform and
	SECTION("rgbaint_t::and_imm") 
	{	
		imm = random_i32();
		expected_a &= imm;
		expected_r &= imm;
		expected_g &= imm;
		expected_b &= imm;
		rgb.and_imm(imm);
		check_expected();
	}
	
	// test uniform xor
	SECTION("rgbaint_t::xor_imm") 
	{	
		imm = random_i32();
		expected_a ^= imm;
		expected_r ^= imm;
		expected_g ^= imm;
		expected_b ^= imm;
		rgb.xor_imm(imm);
		check_expected();
	}

	// test immediate RGB or
	SECTION("rgbaint_t::or_imm_rgba") 
	{	
		expected_a |= actual_a = random_i32();
		expected_r |= actual_r = random_i32();
		expected_g |= actual_g = random_i32();
		expected_b |= actual_b = random_i32();
		rgb.or_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test immediate RGB and
	SECTION("rgbaint_t::and_imm_rgba") 
	{	
		expected_a &= actual_a = random_i32();
		expected_r &= actual_r = random_i32();
		expected_g &= actual_g = random_i32();
		expected_b &= actual_b = random_i32();
		rgb.and_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}
	
	// test immediate RGB xor
	SECTION("rgbaint_t::xor_imm_rgba") 
	{	
		expected_a ^= actual_a = random_i32();
		expected_r ^= actual_r = random_i32();
		expected_g ^= actual_g = random_i32();
		expected_b ^= actual_b = random_i32();
		rgb.xor_imm_rgba(actual_a, actual_r, actual_g, actual_b);
		check_expected();
	}

	// test 8-bit get
	SECTION("8-bit get") 
	{	
		expected_a = s32(u32(expected_a) & 0x00ff);
		expected_r = s32(u32(expected_r) & 0x00ff);
		expected_g = s32(u32(expected_g) & 0x00ff);
		expected_b = s32(u32(expected_b) & 0x00ff);
		actual_a = s32(u32(rgb.get_a()));
		actual_r = s32(u32(rgb.get_r()));
		actual_g = s32(u32(rgb.get_g()));
		actual_b = s32(u32(rgb.get_b()));
		REQUIRE(actual_a == expected_a);
		REQUIRE(actual_r == expected_r);
		REQUIRE(actual_g == expected_g);
		REQUIRE(actual_b == expected_b);
	}

	// test set from packed RGBA
	SECTION("rgbaint_t::set(u32)") 
	{	
		imm = random_i32();
		expected_a = s32((u32(imm) >> 24) & 0x00ff);
		expected_r = s32((u32(imm) >> 16) & 0x00ff);
		expected_g = s32((u32(imm) >> 8) & 0x00ff);
		expected_b = s32((u32(imm) >> 0) & 0x00ff);
		rgb.set(u32(imm));
		check_expected();
	}
	
	// while we have a value loaded that we know doesn't exceed 8-bit range, check the non-clamping convert-to-rgba
	SECTION("non-clamping convert-to-rgba") 
	{	
		packed = rgb.to_rgba();
		REQUIRE(u32(imm) == u32(packed));
	}
	
	// test construct from packed RGBA and assign
	SECTION("rgbaint_t(u32)") 
	{	
		imm = random_i32();
		expected_a = s32((u32(imm) >> 24) & 0x00ff);
		expected_r = s32((u32(imm) >> 16) & 0x00ff);
		expected_g = s32((u32(imm) >> 8) & 0x00ff);
		expected_b = s32((u32(imm) >> 0) & 0x00ff);
		rgb = rgbaint_t(u32(imm));
		check_expected();
	}
	
	// while we have a value loaded that we know doesn't exceed 8-bit range, check the non-clamping convert-to-rgba
	SECTION("non-clamping convert-to-rgba") 
	{	
		packed = rgb.to_rgba();
		REQUIRE(u32(imm) == u32(packed));
	}
	
	// test set with rgb_t
	SECTION("rgbaint_t::set(rgba_t)") 
	{	
		packed = random_u32();
		expected_a = s32(u32(packed.a()));
		expected_r = s32(u32(packed.r()));
		expected_g = s32(u32(packed.g()));
		expected_b = s32(u32(packed.b()));
		rgb.set(packed);
		check_expected();
	}

	// test construct with rgb_t
	SECTION("construct rgb_t") 
	{	
		packed = random_u32();
		expected_a = s32(u32(packed.a()));
		expected_r = s32(u32(packed.r()));
		expected_g = s32(u32(packed.g()));
		expected_b = s32(u32(packed.b()));
		rgb = rgbaint_t(packed);
		check_expected();
	}
	
	// test clamping convert-to-rgba with hand-crafted values to catch edge cases
	SECTION("clamping convert-to-rgba with hand-crafted values to catch edge cases") 
	{	
		rgb.set(std::numeric_limits<s32>::min(), -1, 0, 1);
		packed = rgb.to_rgba_clamp();
		REQUIRE(u32(0x00000001) == u32(packed));

		rgb.set(254, 255, 256, std::numeric_limits<s32>::max());
		packed = rgb.to_rgba_clamp();
		REQUIRE(u32(0xfeffffff) == u32(packed));

		rgb.set(std::numeric_limits<s32>::max(), std::numeric_limits<s32>::min(), 256, -1);
		packed = rgb.to_rgba_clamp();
		REQUIRE(u32(0xff00ff00) == u32(packed));

		rgb.set(0, 255, 1, 254);
		packed = rgb.to_rgba_clamp();
		REQUIRE(u32(0x00ff01fe) == u32(packed));
	}
	
	// test in-place clamping with hand-crafted values to catch edge cases
	SECTION("rgbaint_t::clamp_to_uint8") 
	{	
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = 1;
		rgb.set(std::numeric_limits<s32>::min(), -1, 0, 1);
		rgb.clamp_to_uint8();
		check_expected();

		expected_a = 254;
		expected_r = 255;
		expected_g = 255;
		expected_b = 255;
		rgb.set(254, 255, 256, std::numeric_limits<s32>::max());
		rgb.clamp_to_uint8();
		check_expected();

		expected_a = 255;
		expected_r = 0;
		expected_g = 255;
		expected_b = 0;
		rgb.set(std::numeric_limits<s32>::max(), std::numeric_limits<s32>::min(), 256, -1);
		rgb.clamp_to_uint8();
		check_expected();

		expected_a = 0;
		expected_r = 255;
		expected_g = 1;
		expected_b = 254;
		rgb.set(0, 255, 1, 254);
		rgb.clamp_to_uint8();
		check_expected();
	}
	
	// test shift left
	SECTION("rgbaint_t::shl") 
	{	
		expected_a = (actual_a = random_i32()) << 19;
		expected_r = (actual_r = random_i32()) << 3;
		expected_g = (actual_g = random_i32()) << 21;
		expected_b = (actual_b = random_i32()) << 6;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shl(rgbaint_t(19, 3, 21, 6));
		check_expected();
	}
	
	// test shift left immediate
	SECTION("rgbaint_t::shl_imm") 
	{	
		expected_a = (actual_a = random_i32()) << 7;
		expected_r = (actual_r = random_i32()) << 7;
		expected_g = (actual_g = random_i32()) << 7;
		expected_b = (actual_b = random_i32()) << 7;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shl_imm(7);
		check_expected();
	}
	
	// test logical shift right
	SECTION("rgbaint_t::shr") 
	{	
		expected_a = s32(u32(actual_a = random_i32()) >> 8);
		expected_r = s32(u32(actual_r = random_i32()) >> 18);
		expected_g = s32(u32(actual_g = random_i32()) >> 26);
		expected_b = s32(u32(actual_b = random_i32()) >> 4);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shr(rgbaint_t(8, 18, 26, 4));
		check_expected();
	}
	
	// test logical shift right with opposite signs
	SECTION("rgbaint_t::shrwith opposite signs") 
	{	
		expected_a = s32(u32(actual_a = -actual_a) >> 21);
		expected_r = s32(u32(actual_r = -actual_r) >> 13);
		expected_g = s32(u32(actual_g = -actual_g) >> 11);
		expected_b = s32(u32(actual_b = -actual_b) >> 17);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shr(rgbaint_t(21, 13, 11, 17));
		check_expected();
	}
	
	// test logical shift right immediate
	SECTION("rgbaint_t::shr_imm") 
	{	
		expected_a = s32(u32(actual_a = random_i32()) >> 5);
		expected_r = s32(u32(actual_r = random_i32()) >> 5);
		expected_g = s32(u32(actual_g = random_i32()) >> 5);
		expected_b = s32(u32(actual_b = random_i32()) >> 5);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shr_imm(5);
		check_expected();
	}
	
	// test logical shift right immediate with opposite signs
	SECTION("rgbaint_t::shr_imm with opposite signs") 
	{	
		expected_a = s32(u32(actual_a = -actual_a) >> 15);
		expected_r = s32(u32(actual_r = -actual_r) >> 15);
		expected_g = s32(u32(actual_g = -actual_g) >> 15);
		expected_b = s32(u32(actual_b = -actual_b) >> 15);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.shr_imm(15);
		check_expected();
	}

	// test arithmetic shift right
	SECTION("rgbaint_t::sra") 
	{	
		expected_a = (actual_a = random_i32()) >> 16;
		expected_r = (actual_r = random_i32()) >> 20;
		expected_g = (actual_g = random_i32()) >> 14;
		expected_b = (actual_b = random_i32()) >> 2;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.sra(rgbaint_t(16, 20, 14, 2));
		check_expected();
	}

	// test arithmetic shift right with opposite signs
	SECTION("rgbaint_t::sra with opposite signs") 
	{	
		expected_a = (actual_a = -actual_a) >> 1;
		expected_r = (actual_r = -actual_r) >> 29;
		expected_g = (actual_g = -actual_g) >> 10;
		expected_b = (actual_b = -actual_b) >> 22;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.sra(rgbaint_t(1, 29, 10, 22));
		check_expected();
	}
	
	// test arithmetic shift right immediate (method)
	SECTION("rgbaint_t::sra_imm")
	{	
		expected_a = (actual_a = random_i32()) >> 12;
		expected_r = (actual_r = random_i32()) >> 12;
		expected_g = (actual_g = random_i32()) >> 12;
		expected_b = (actual_b = random_i32()) >> 12;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.sra_imm(12);
		check_expected();
	}
	
	// test arithmetic shift right immediate with opposite signs (method)
	SECTION("rgbaint_t::sra_imm with opposite signs") 
	{	
		expected_a = (actual_a = -actual_a) >> 9;
		expected_r = (actual_r = -actual_r) >> 9;
		expected_g = (actual_g = -actual_g) >> 9;
		expected_b = (actual_b = -actual_b) >> 9;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.sra_imm(9);
		check_expected();
	}

	// test arithmetic shift right immediate (operator)
	SECTION("rgbaint_t::operator>>=") 
	{	
		expected_a = (actual_a = random_i32()) >> 7;
		expected_r = (actual_r = random_i32()) >> 7;
		expected_g = (actual_g = random_i32()) >> 7;
		expected_b = (actual_b = random_i32()) >> 7;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb >>= 7;
		check_expected();
	}
	
	// test arithmetic shift right immediate with opposite signs (operator)
	SECTION("rgbaint_t::operator>>= with opposite signs") 
	{	
		expected_a = (actual_a = -actual_a) >> 11;
		expected_r = (actual_r = -actual_r) >> 11;
		expected_g = (actual_g = -actual_g) >> 11;
		expected_b = (actual_b = -actual_b) >> 11;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb >>= 11;
		check_expected();
	}

	// test RGB equality comparison
	SECTION("rgbaint_t::cmpeq RGB equality comparison") 
	{	
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
		check_expected();
		expected_a = 0;
		expected_r = ~s32(0);
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
		check_expected();
	}

	// test immediate equality comparison
	SECTION("rgbaint_t::cmpeq_imm immediate equality comparison") 
	{	
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
		check_expected();
		expected_a = (actual_a == actual_r) ? ~s32(0) : 0;
		expected_r = ~s32(0);
		expected_g = (actual_g == actual_r) ? ~s32(0) : 0;
		expected_b = (actual_b == actual_r) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(actual_r);
		check_expected();
		expected_a = (actual_a == actual_g) ? ~s32(0) : 0;
		expected_r = (actual_r == actual_g) ? ~s32(0) : 0;
		expected_g = ~s32(0);
		expected_b = (actual_b == actual_g) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(actual_g);
		check_expected();
		expected_a = (actual_a == actual_b) ? ~s32(0) : 0;
		expected_r = (actual_r == actual_b) ? ~s32(0) : 0;
		expected_g = (actual_g == actual_b) ? ~s32(0) : 0;
		expected_b = ~s32(0);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(actual_b);
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(std::numeric_limits<s32>::min());
		check_expected();
		expected_a = !actual_a ? ~s32(0) : 0;
		expected_r = !actual_r ? ~s32(0) : 0;
		expected_g = !actual_g ? ~s32(0) : 0;
		expected_b = !actual_b ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(0);
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm(std::numeric_limits<s32>::max());
		check_expected();
	}

	// test immediate RGB equality comparison
	SECTION("rgbaint_t::cmpeq_imm_rgba immediate RGB equality comparison") 
	{	
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
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = ~s32(0);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpeq_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
		check_expected();
	}

	// test RGB greater than comparison
	SECTION("rgbaint_t::cmpgt RGB greater than comparison") 
	{	
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
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = ~s32(0);
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
		check_expected();
	}
	
	// test immediate greater than comparison
	SECTION("rgbaint_t::cmpgt_imm immediate greater than comparison") 
	{	
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
		check_expected();
		expected_a = (actual_a > actual_r) ? ~s32(0) : 0;
		expected_r = 0;
		expected_g = (actual_g > actual_r) ? ~s32(0) : 0;
		expected_b = (actual_b > actual_r) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(actual_r);
		check_expected();
		expected_a = (actual_a > actual_g) ? ~s32(0) : 0;
		expected_r = (actual_r > actual_g) ? ~s32(0) : 0;
		expected_g =0;
		expected_b = (actual_b > actual_g) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(actual_g);
		check_expected();
		expected_a = (actual_a > actual_b) ? ~s32(0) : 0;
		expected_r = (actual_r > actual_b) ? ~s32(0) : 0;
		expected_g = (actual_g > actual_b) ? ~s32(0) : 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(actual_b);
		check_expected();
		expected_a = ~s32(0);
		expected_r = ~s32(0);
		expected_g = ~s32(0);
		expected_b = ~s32(0);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(std::numeric_limits<s32>::min());
		check_expected();
		expected_a = (actual_a > 0) ? ~s32(0) : 0;
		expected_r = (actual_r > 0) ? ~s32(0) : 0;
		expected_g = (actual_g > 0) ? ~s32(0) : 0;
		expected_b = (actual_b > 0) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(0);
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm(std::numeric_limits<s32>::max());
		check_expected();
	}

	// test immediate RGB greater than comparison
	SECTION("rgbaint_t::cmpgt_imm_rgba immediate RGB greater than comparison") 
	{	
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
		check_expected();
		expected_a = 0;
		expected_r = ~s32(0);
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmpgt_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
		check_expected();
	}
	// test RGB less than comparison
	SECTION("rgbaint_t::cmplt RGB less than comparison") 
	{	
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
		check_expected();
		expected_a = ~s32(0);
		expected_r = 0;
		expected_g = 0;
		expected_b = ~s32(0);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt(rgbaint_t(std::numeric_limits<s32>::max(), actual_r, actual_g - 1, actual_b + 1));
		check_expected();
	}

	// test immediate less than comparison
	SECTION("rgbaint_t::cmplt_imm immediate less than comparison") 
	{	
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
		check_expected();
		expected_a = (actual_a < actual_r) ? ~s32(0) : 0;
		expected_r = 0;
		expected_g = (actual_g < actual_r) ? ~s32(0) : 0;
		expected_b = (actual_b < actual_r) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(actual_r);
		check_expected();
		expected_a = (actual_a < actual_g) ? ~s32(0) : 0;
		expected_r = (actual_r < actual_g) ? ~s32(0) : 0;
		expected_g =0;
		expected_b = (actual_b < actual_g) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(actual_g);
		check_expected();
		expected_a = (actual_a < actual_b) ? ~s32(0) : 0;
		expected_r = (actual_r < actual_b) ? ~s32(0) : 0;
		expected_g = (actual_g < actual_b) ? ~s32(0) : 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(actual_b);
		check_expected();
		expected_a = 0;
		expected_r = 0;
		expected_g = 0;
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(std::numeric_limits<s32>::min());
		check_expected();
		expected_a = (actual_a < 0) ? ~s32(0) : 0;
		expected_r = (actual_r < 0) ? ~s32(0) : 0;
		expected_g = (actual_g < 0) ? ~s32(0) : 0;
		expected_b = (actual_b < 0) ? ~s32(0) : 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(0);
		check_expected();
		expected_a = ~s32(0);
		expected_r = ~s32(0);
		expected_g = ~s32(0);
		expected_b = ~s32(0);
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm(std::numeric_limits<s32>::max());
		check_expected();
	}
	
	// test immediate RGB less than comparison
	SECTION("rgbaint_t::cmplt_imm_rgba immediate RGB less than comparison") 
	{	
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
		check_expected();
		expected_a = ~s32(0);
		expected_r = 0;
		expected_g = ~s32(0);
		expected_b = 0;
		rgb.set(actual_a, actual_r, actual_g, actual_b);
		rgb.cmplt_imm_rgba(actual_a + 1, std::numeric_limits<s32>::min(), std::numeric_limits<s32>::max(), actual_b);
		check_expected();
	}
}
