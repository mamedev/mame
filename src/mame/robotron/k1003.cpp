// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*******************************************************************************************************

Robotron K1003

2009-11-20 Skeleton driver.

The last of the K-1000 series of electronic programmable desktop calculators.

K-1001 : Enter via keyboard, results are on the LED display, much like any other advanced calculator
         or adding machine.

K-1002 : Has an additional magnetic stripe reader/writer. You can save your programs to the flexible
         card and insert it in another session. Each card holds about 200 bytes. Further, the unit has
         a cartridge slot where preset mathematical packages could be inserted (such as Statistics module).

K-1003 : All of the above, plus a thermal printer. Maximum ram is 4k.


TODO:
- Need schematic, unable to locate one
- keyboard to be worked out
- Cartslots
- Printer
- Magnetic cards

******************************************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "k1003.lh"


namespace {

class k1003_state : public driver_device
{
public:
	k1003_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		{ }

	void k1003(machine_config &config);

private:
	uint8_t port2_r();
	uint8_t key_r();
	void disp_1_w(uint8_t data);
	void disp_2_w(uint8_t data);
	void seg_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_disp_1 = 0U;
	uint8_t m_disp_2 = 0U;
	u8 m_digit = 0U;
	[[maybe_unused]] uint8_t bit_to_dec(uint8_t val);
	void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<8> m_io_keyboard;
	output_finder<16> m_digits;
};


void k1003_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x17ff).ram();
	map(0x1800, 0x1fff).rom();
	map(0x2000, 0x27ff).rom();
	map(0x2800, 0x2fff).ram();
	map(0x3000, 0x3aff).rom();
}

// if non-zero, jump to cart?
uint8_t k1003_state::port2_r()
{
	return 0x00;
}

uint8_t k1003_state::key_r()
{//printf("%X ",m_digit);
	u8 data = 0;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_digit, i))
			data |= m_io_keyboard[i]->read();

	return data;
}


void k1003_state::disp_1_w(uint8_t data)
{
	m_disp_1 = data;
}

void k1003_state::disp_2_w(uint8_t data)
{
	m_disp_2 = data;
}

uint8_t k1003_state::bit_to_dec(uint8_t val) // not used atm
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

void k1003_state::seg_w(uint8_t data)
{
	data = bitswap<8>(data, 0,1,2,3,4,5,6,7);

	for (u8 i = 0; i < 8; i++)
	{
		m_digits[i] = (m_digits[i] & ~data) | (BIT(m_disp_1, i) ? data : 0);
		m_digits[i+8] = (m_digits[i+8] & ~data) | (BIT(m_disp_2, i) ? data : 0);
	}
}

void k1003_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1f);
	map(0x00, 0x00).r(FUNC(k1003_state::key_r));
	map(0x02, 0x02).r(FUNC(k1003_state::port2_r));

	map(0x08, 0x08).w(FUNC(k1003_state::seg_w));
	map(0x09, 0x09).w(FUNC(k1003_state::disp_2_w));
	map(0x0a, 0x0a).w(FUNC(k1003_state::disp_1_w));
	map(0x10, 0x10).lw8(NAME([this] (u8 data) { m_digit = ~data; }));
	map(0x11, 0x13).nopw();   // stop error.log rapidly filling up.
}

/* Input ports */
static INPUT_PORTS_START( k1003 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~")  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
INPUT_PORTS_END


void k1003_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_disp_1));
	save_item(NAME(m_disp_2));
	save_item(NAME(m_digit));
}

void k1003_state::k1003(machine_config &config)
{
	/* basic machine hardware */
	I8008(config, m_maincpu, 800000);
	m_maincpu->set_addrmap(AS_PROGRAM, &k1003_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &k1003_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_k1003);
}


/* ROM definition */
ROM_START( k1003 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
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
	ROM_LOAD( "026.bin",  0x2000, 0x0100, CRC(d678e80c) SHA1(bdf696e9704c286ed0ad5ffbdae206580a277c38))
	ROM_LOAD( "027.bin",  0x2100, 0x0100, CRC(dbe2ca8e) SHA1(2060bbb6b6dee87c98ddcf84a49069547749ae9b))
	ROM_LOAD( "028.bin",  0x2200, 0x0100, CRC(2cd742ed) SHA1(ec3f1ba548c64b0fe538af365a383f2341c81988))
	ROM_LOAD( "029.bin",  0x2300, 0x0100, CRC(12165b43) SHA1(7bb2c97893a07196cf245c8ac3a64c0ee49bb75e))
	ROM_LOAD( "030.bin",  0x2400, 0x0100, CRC(545dd7e0) SHA1(ab9f2e8cd4d3d4ba8accf74caec20de2e3094a66))
	ROM_LOAD( "031.bin",  0x2500, 0x0100, CRC(315d27d2) SHA1(26bb39d50781eed8d8be4ad6e1f13bc2b4672ce2))
	ROM_LOAD( "032.bin",  0x2600, 0x0100, CRC(d03f7cc7) SHA1(5a1c0614eed6dfe21d0d1776f409c1c2209a74d1))
	ROM_LOAD( "033.bin",  0x2700, 0x0100, CRC(efaeb541) SHA1(decdbbd3c4084dc18b34577f78ddf7044341764a))

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
	ROM_LOAD( "041.bin",  0x0100, 0x0100, CRC(dcd776b3) SHA1(cf8082d31be9bea1e9672d0b92006f14719ba7a6) )
	ROM_LOAD( "042.bin",  0x0200, 0x0100, CRC(e74aca0d) SHA1(f515b52862f0f31551ec008bf6f27c5fdf78c32a) )
	ROM_LOAD( "043.bin",  0x0300, 0x0100, CRC(770820e5) SHA1(c34b6a7bde43758c7c3c7041ce1dee06456fe5c4) )
	ROM_LOAD( "044.bin",  0x0400, 0x0100, CRC(82bd3f5a) SHA1(c4290507de8d295c0b7f04ac2179365b8f73d7c7) )
	ROM_LOAD( "045.bin",  0x0500, 0x0100, CRC(66c85afb) SHA1(937828a6aee46cbcad86bb1776deb7f9b1dc69ff) )
	// k1003-statistics
	ROM_LOAD( "435.bin",  0x0800, 0x0100, CRC(31203b34) SHA1(5638c45e23b279c5c8960d5d7f2b16acb2e47ed2) )
	ROM_LOAD( "436.bin",  0x0900, 0x0100, CRC(dd2d4eb5) SHA1(c436419ef71cdf4dfc4d118301749f746f9cd6e5) )
	ROM_LOAD( "437.bin",  0x0a00, 0x0100, CRC(00433159) SHA1(69fab3b875c285e6abcab43b0ba6ef916d0a1484) )
	ROM_LOAD( "439.bin",  0x0b00, 0x0100, CRC(3f5be050) SHA1(f2730078a8346e8670b632048f358a47ff57941e) )
	ROM_LOAD( "440.bin",  0x0c00, 0x0100, CRC(856685fa) SHA1(dbd33194a4eb9ed037f8129bf8b9d4628aab8151) )
	ROM_LOAD( "441.bin",  0x0d00, 0x0100, CRC(257df6a3) SHA1(65c0429b0e352434b7b661e63cacfe74ac1d1ac9) )
	ROM_LOAD( "442.bin",  0x0e00, 0x0100, CRC(d037e0bb) SHA1(5ae8ad62673bd732a05232645c523206024f9afb) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1978, k1003, 0,      0,      k1003,   k1003, k1003_state, empty_init, "Robotron", "K1003",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
