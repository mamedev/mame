// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Robotron K1003

        20/11/2009 Skeleton driver.


'maincpu' (004B): unmapped i/o memory write to 0B = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (0322): unmapped i/o memory write to 0A = FF & FF
'maincpu' (0323): unmapped i/o memory write to 13 = FF & FF
'maincpu' (0325): unmapped i/o memory write to 11 = 00 & FF
'maincpu' (02F2): unmapped i/o memory write to 0A = 00 & FF
'maincpu' (02F2): unmapped i/o memory write to 0A = 00 & FF
'maincpu' (02F2): unmapped i/o memory write to 0A = 0C & FF
'maincpu' (02F2): unmapped i/o memory write to 0A = 0C & FF
'maincpu' (02F2): unmapped i/o memory write to 0A = 0C & FF

The writes to ports 0A, 11 & 13 are continuous.

- Need a keyboard

****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "k1003.lh"


class k1003_state : public driver_device
{
public:
	k1003_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
		{ }

	void k1003(machine_config &config);

private:
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(disp_1_w);
	DECLARE_WRITE8_MEMBER(disp_2_w);
	DECLARE_WRITE8_MEMBER(disp_w);

	void k1003_io(address_map &map);
	void k1003_mem(address_map &map);

	uint8_t m_disp_1;
	uint8_t m_disp_2;
	uint8_t bit_to_dec(uint8_t val);
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	output_finder<16> m_digits;
};


void k1003_state::k1003_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x17ff).ram();
	map(0x1800, 0x1fff).rom();
	map(0x2000, 0x27ff).rom();
	map(0x2800, 0x2fff).ram();
	map(0x3000, 0x3aff).rom();
}

READ8_MEMBER( k1003_state::port2_r )
{
	return 0x00;
}

READ8_MEMBER( k1003_state::key_r )
{
	return 0x00;
}


WRITE8_MEMBER( k1003_state::disp_1_w )
{
	m_disp_1 = data;
}

WRITE8_MEMBER( k1003_state::disp_2_w )
{
	m_disp_2 = data;
}

uint8_t k1003_state::bit_to_dec(uint8_t val)
{
	if (BIT(val,0)) return 0;
	if (BIT(val,1)) return 1;
	if (BIT(val,2)) return 2;
	if (BIT(val,3)) return 3;
	if (BIT(val,4)) return 4;
	if (BIT(val,5)) return 5;
	if (BIT(val,6)) return 6;
	if (BIT(val,7)) return 7;
	return 0;
}

WRITE8_MEMBER( k1003_state::disp_w )
{
	m_digits[bit_to_dec(data)*2] = m_disp_1;
	m_digits[bit_to_dec(data)*2+1] = m_disp_2;
}

void k1003_state::k1003_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).r(FUNC(k1003_state::key_r));
	map(0x02, 0x02).r(FUNC(k1003_state::port2_r));

	map(0x08, 0x08).w(FUNC(k1003_state::disp_w));
	map(0x09, 0x09).w(FUNC(k1003_state::disp_2_w));
	map(0x10, 0x10).w(FUNC(k1003_state::disp_1_w));
}

/* Input ports */
static INPUT_PORTS_START( k1003 )
INPUT_PORTS_END


void k1003_state::machine_reset()
{
}

MACHINE_CONFIG_START(k1003_state::k1003)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",I8008, 800000)
	MCFG_DEVICE_PROGRAM_MAP(k1003_mem)
	MCFG_DEVICE_IO_MAP(k1003_io)

	/* video hardware */
	config.set_default_layout(layout_k1003);
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( k1003 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "k1003.01", 0x0000, 0x0100, CRC(9342f67d) SHA1(75d33cad89cf47e8e691a6ddbb86a8c11f454434))
	ROM_LOAD( "k1003.02", 0x0100, 0x0100, CRC(a6846b2b) SHA1(a38b15ae0ac3f216e49aef4618363ea0d262fe52))
	ROM_LOAD( "k1003.03", 0x0200, 0x0100, CRC(3fddd922) SHA1(9c9f28ad8a611d8a0911d2935ffa262976a0272d))
	ROM_LOAD( "k1003.04", 0x0300, 0x0100, CRC(ec3edbe9) SHA1(d58064db7f2d085760088da1899f44f7a5b02923))
	ROM_LOAD( "k1003.05", 0x0400, 0x0100, CRC(93836b34) SHA1(7397c9748c1adb347270529464f1e10d3ac92879))
	ROM_LOAD( "k1003.06", 0x0500, 0x0100, CRC(79e39c8f) SHA1(bffc24a47867834d749d32307dca1053231a1b62))
	ROM_LOAD( "k1003.07", 0x0600, 0x0100, CRC(1f7279e0) SHA1(1e625adf606f87a55b6e8827fc9764d8a777903a))
	ROM_LOAD( "k1003.08", 0x0700, 0x0100, CRC(4b950957) SHA1(268c7dbf52a85bdf4eb5ebbe6d01aae45e853a72))

	ROM_LOAD( "k1003.09", 0x1800, 0x0100, CRC(f3ec866f) SHA1(0b274be7290c9d469205136851c48595cd4f15e2))
	ROM_LOAD( "k1003.10", 0x1900, 0x0100, CRC(c4af2cf7) SHA1(1815030d08072542fa56c4063b6de1d64b146887))
	ROM_LOAD( "k1003.11", 0x1a00, 0x0100, CRC(473ef6db) SHA1(45372e09babf6fa08875e53d43e90d53f9cc0ec1))
	ROM_LOAD( "k1003.12", 0x1b00, 0x0100, CRC(8af505d4) SHA1(21302f2bb660ddaf5e4526335ec457479f1673d5))
	ROM_LOAD( "k1003.13", 0x1c00, 0x0100, CRC(753166da) SHA1(90b91bac845f5d0ecd3af4d67174d351e478b632))
	ROM_LOAD( "k1003.14", 0x1d00, 0x0100, CRC(a885a676) SHA1(e899ea5b97734360421d2696e0a75d65fa8a031c))
	ROM_LOAD( "k1003.15", 0x1e00, 0x0100, CRC(db63b0cd) SHA1(726d80bb34862301df773b8587acccbeedfcc38d))
	ROM_LOAD( "k1003.16", 0x1f00, 0x0100, CRC(9457f1bd) SHA1(aae0a7c0a63d8213a57850383aaf92927577be7a))

	// Math pack
	ROM_LOAD( "026.bin", 0x2000, 0x0100, CRC(d678e80c) SHA1(bdf696e9704c286ed0ad5ffbdae206580a277c38))
	ROM_LOAD( "027.bin", 0x2100, 0x0100, CRC(dbe2ca8e) SHA1(2060bbb6b6dee87c98ddcf84a49069547749ae9b))
	ROM_LOAD( "028.bin", 0x2200, 0x0100, CRC(2cd742ed) SHA1(ec3f1ba548c64b0fe538af365a383f2341c81988))
	ROM_LOAD( "029.bin", 0x2300, 0x0100, CRC(12165b43) SHA1(7bb2c97893a07196cf245c8ac3a64c0ee49bb75e))
	ROM_LOAD( "030.bin", 0x2400, 0x0100, CRC(545dd7e0) SHA1(ab9f2e8cd4d3d4ba8accf74caec20de2e3094a66))
	ROM_LOAD( "031.bin", 0x2500, 0x0100, CRC(315d27d2) SHA1(26bb39d50781eed8d8be4ad6e1f13bc2b4672ce2))
	ROM_LOAD( "032.bin", 0x2600, 0x0100, CRC(d03f7cc7) SHA1(5a1c0614eed6dfe21d0d1776f409c1c2209a74d1))
	ROM_LOAD( "033.bin", 0x2700, 0x0100, CRC(efaeb541) SHA1(decdbbd3c4084dc18b34577f78ddf7044341764a))

	ROM_LOAD( "k1003.17", 0x3000, 0x0100, CRC(9031390b) SHA1(6f99a9f643b19770a373242edb0df8f342bbc230))
	ROM_LOAD( "k1003.18", 0x3100, 0x0100, CRC(38435ffe) SHA1(7db78d304fe8a8f71c067babcdcf3c06da908ad3))
	ROM_LOAD( "k1003.19", 0x3200, 0x0100, CRC(3cfddbda) SHA1(7e9d5c6126d0f08fcb7d88d0cc26eb24467f7321))
	ROM_LOAD( "k1003.20", 0x3300, 0x0100, CRC(08707172) SHA1(fef18e407ebec13d34c2bd6925ffae533139551e))
	ROM_LOAD( "k1003.21", 0x3400, 0x0100, CRC(4038b284) SHA1(f6ee7fd8fb06a73a7d1ed71ca7022f1c76c207bd))
	ROM_LOAD( "k1003.22", 0x3500, 0x0100, CRC(04691d40) SHA1(72f96994811f435adecc74585f3dfaa89d0f192a))
	ROM_LOAD( "k1003.23", 0x3600, 0x0100, CRC(a2f7170c) SHA1(2cd9a64019c2cd7f6f6146d1886de6a1aa5ccc88))
	ROM_LOAD( "k1003.24", 0x3700, 0x0100, CRC(c0935c12) SHA1(f3fc7c3fa97b2bcb9ec522002ee760a226eb329a))
	ROM_LOAD( "k1003.25", 0x3800, 0x0100, CRC(a827aec0) SHA1(a8f582a9a6b31581d8a174a0bc2f4588d6b53400))
	ROM_LOAD( "k1003.26", 0x3900, 0x0100, CRC(fc949804) SHA1(088b63b7f8704efb6867be899b81d64a294af6be))
	ROM_LOAD( "k1003.27", 0x3a00, 0x0100, CRC(ddcdd065) SHA1(e29c6f2dd1e4da125d150e28cf51f8c558ec9ee5))
	// 0x3b00 - missing on board - returning 0xff

	ROM_REGION( 0x1000, "user1", 0 )
	// k1003-extension
	ROM_LOAD( "040.bin",  0x0000, 0x0100, CRC(06865678) SHA1(91a4f0a32e93d315d4f78a732472c08a380205fc) )
	ROM_LOAD( "041.bin",  0x0000, 0x0100, CRC(dcd776b3) SHA1(cf8082d31be9bea1e9672d0b92006f14719ba7a6) )
	ROM_LOAD( "042.bin",  0x0000, 0x0100, CRC(e74aca0d) SHA1(f515b52862f0f31551ec008bf6f27c5fdf78c32a) )
	ROM_LOAD( "043.bin",  0x0000, 0x0100, CRC(770820e5) SHA1(c34b6a7bde43758c7c3c7041ce1dee06456fe5c4) )
	ROM_LOAD( "044.bin",  0x0000, 0x0100, CRC(82bd3f5a) SHA1(c4290507de8d295c0b7f04ac2179365b8f73d7c7) )
	ROM_LOAD( "045.bin",  0x0000, 0x0100, CRC(66c85afb) SHA1(937828a6aee46cbcad86bb1776deb7f9b1dc69ff) )
	// k1003-statistics
	ROM_LOAD( "435.bin",  0x0000, 0x0100, CRC(31203b34) SHA1(5638c45e23b279c5c8960d5d7f2b16acb2e47ed2) )
	ROM_LOAD( "436.bin",  0x0000, 0x0100, CRC(dd2d4eb5) SHA1(c436419ef71cdf4dfc4d118301749f746f9cd6e5) )
	ROM_LOAD( "437.bin",  0x0000, 0x0100, CRC(00433159) SHA1(69fab3b875c285e6abcab43b0ba6ef916d0a1484) )
	ROM_LOAD( "439.bin",  0x0000, 0x0100, CRC(3f5be050) SHA1(f2730078a8346e8670b632048f358a47ff57941e) )
	ROM_LOAD( "440.bin",  0x0000, 0x0100, CRC(856685fa) SHA1(dbd33194a4eb9ed037f8129bf8b9d4628aab8151) )
	ROM_LOAD( "441.bin",  0x0000, 0x0100, CRC(257df6a3) SHA1(65c0429b0e352434b7b661e63cacfe74ac1d1ac9) )
	ROM_LOAD( "442.bin",  0x0000, 0x0100, CRC(d037e0bb) SHA1(5ae8ad62673bd732a05232645c523206024f9afb) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1978, k1003, 0,      0,      k1003,   k1003, k1003_state, empty_init, "Robotron", "K1003",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
