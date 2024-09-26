// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM)

    Driver by Ryan Holtz
*/

#include "emu.h"
#include "cpu/ssem/ssem.h"
#include "imagedev/snapquik.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include <sstream>


namespace {

class ssem_state : public driver_device
{
public:
	ssem_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_store(*this, "store"),
		m_screen(*this, "screen")
	{
	}

	void ssem(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(panel_check);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_ssem(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	inline uint32_t reverse(uint32_t v);
	std::string read_line(device_image_interface &image);

	void ssem_map(address_map &map) ATTR_COLD;

	template <typename Format, typename... Params>
	void glyph_print(bitmap_rgb32 &bitmap, int32_t x, int32_t y, Format &&fmt, Params &&...args);

	required_device<ssem_device> m_maincpu;
	required_shared_ptr<uint8_t> m_store;
	required_device<screen_device> m_screen;

	uint8_t m_store_line = 0;

	util::ovectorstream m_glyph_print_buf;
};



/****************************************************\
* General helper functions                           *
\****************************************************/

// The SSEM stores its data, visually, with the leftmost bit corresponding to the least significant bit.
// The de facto snapshot format for other SSEM simulators stores the data physically in that format as well.
// Therefore, in MESS, every 32-bit word has its bits reversed, too, and as a result the values must be
// un-reversed before being used.
inline uint32_t ssem_state::reverse(uint32_t v)
{
	// Taken from http://www.graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = ( v >> 16 ) | ( v << 16);

	return v;
}

/****************************************************\
* Address map                                        *
\****************************************************/

void ssem_state::ssem_map(address_map &map)
{
	map(0x00, 0x7f).ram().share("store");// Primary store
}

/****************************************************\
* Input ports and front panel handling               *
\****************************************************/

enum
{
	// Bit-edit buttons
	PANEL_BIT0,  PANEL_BIT1,  PANEL_BIT2,  PANEL_BIT3,  PANEL_BIT4,  PANEL_BIT5,  PANEL_BIT6,  PANEL_BIT7,
	PANEL_BIT8,  PANEL_BIT9,  PANEL_BIT10, PANEL_BIT11, PANEL_BIT12, PANEL_BIT13, PANEL_BIT14, PANEL_BIT15,
	PANEL_BIT16, PANEL_BIT17, PANEL_BIT18, PANEL_BIT19, PANEL_BIT20, PANEL_BIT21, PANEL_BIT22, PANEL_BIT23,
	PANEL_BIT24, PANEL_BIT25, PANEL_BIT26, PANEL_BIT27, PANEL_BIT28, PANEL_BIT29, PANEL_BIT30, PANEL_BIT31,

	// Page up, page down
	PANEL_PGUP, PANEL_PGDN,

	// Line up, line down
	PANEL_LNUP, PANEL_LNDN,

	// Halt
	PANEL_HALT
};

INPUT_CHANGED_MEMBER(ssem_state::panel_check)
{
	uint8_t edit0_state = ioport("EDIT0")->read();
	uint8_t edit1_state = ioport("EDIT1")->read();
	uint8_t edit2_state = ioport("EDIT2")->read();
	uint8_t edit3_state = ioport("EDIT3")->read();
	uint8_t misc_state = ioport("MISC")->read();

	switch( (int)param )
	{
		case PANEL_BIT0:
			if(edit0_state & 0x01) m_store[(m_store_line << 2) | 0] ^= 0x80;
			break;
		case PANEL_BIT1:
			if(edit0_state & 0x02) m_store[(m_store_line << 2) | 0] ^= 0x40;
			break;
		case PANEL_BIT2:
			if(edit0_state & 0x04) m_store[(m_store_line << 2) | 0] ^= 0x20;
			break;
		case PANEL_BIT3:
			if(edit0_state & 0x08) m_store[(m_store_line << 2) | 0] ^= 0x10;
			break;
		case PANEL_BIT4:
			if(edit0_state & 0x10) m_store[(m_store_line << 2) | 0] ^= 0x08;
			break;
		case PANEL_BIT5:
			if(edit0_state & 0x20) m_store[(m_store_line << 2) | 0] ^= 0x04;
			break;
		case PANEL_BIT6:
			if(edit0_state & 0x40) m_store[(m_store_line << 2) | 0] ^= 0x02;
			break;
		case PANEL_BIT7:
			if(edit0_state & 0x80) m_store[(m_store_line << 2) | 0] ^= 0x01;
			break;
		case PANEL_BIT8:
			if(edit1_state & 0x01) m_store[(m_store_line << 2) | 1] ^= 0x80;
			break;
		case PANEL_BIT9:
			if(edit1_state & 0x02) m_store[(m_store_line << 2) | 1] ^= 0x40;
			break;
		case PANEL_BIT10:
			if(edit1_state & 0x04) m_store[(m_store_line << 2) | 1] ^= 0x20;
			break;
		case PANEL_BIT11:
			if(edit1_state & 0x08) m_store[(m_store_line << 2) | 1] ^= 0x10;
			break;
		case PANEL_BIT12:
			if(edit1_state & 0x10) m_store[(m_store_line << 2) | 1] ^= 0x08;
			break;
		case PANEL_BIT13:
			if(edit1_state & 0x20) m_store[(m_store_line << 2) | 1] ^= 0x04;
			break;
		case PANEL_BIT14:
			if(edit1_state & 0x40) m_store[(m_store_line << 2) | 1] ^= 0x02;
			break;
		case PANEL_BIT15:
			if(edit1_state & 0x80) m_store[(m_store_line << 2) | 1] ^= 0x01;
			break;
		case PANEL_BIT16:
			if(edit2_state & 0x01) m_store[(m_store_line << 2) | 2] ^= 0x80;
			break;
		case PANEL_BIT17:
			if(edit2_state & 0x02) m_store[(m_store_line << 2) | 2] ^= 0x40;
			break;
		case PANEL_BIT18:
			if(edit2_state & 0x04) m_store[(m_store_line << 2) | 2] ^= 0x20;
			break;
		case PANEL_BIT19:
			if(edit2_state & 0x08) m_store[(m_store_line << 2) | 2] ^= 0x10;
			break;
		case PANEL_BIT20:
			if(edit2_state & 0x10) m_store[(m_store_line << 2) | 2] ^= 0x08;
			break;
		case PANEL_BIT21:
			if(edit2_state & 0x20) m_store[(m_store_line << 2) | 2] ^= 0x04;
			break;
		case PANEL_BIT22:
			if(edit2_state & 0x40) m_store[(m_store_line << 2) | 2] ^= 0x02;
			break;
		case PANEL_BIT23:
			if(edit2_state & 0x80) m_store[(m_store_line << 2) | 2] ^= 0x01;
			break;
		case PANEL_BIT24:
			if(edit3_state & 0x01) m_store[(m_store_line << 2) | 3] ^= 0x80;
			break;
		case PANEL_BIT25:
			if(edit3_state & 0x02) m_store[(m_store_line << 2) | 3] ^= 0x40;
			break;
		case PANEL_BIT26:
			if(edit3_state & 0x04) m_store[(m_store_line << 2) | 3] ^= 0x20;
			break;
		case PANEL_BIT27:
			if(edit3_state & 0x08) m_store[(m_store_line << 2) | 3] ^= 0x10;
			break;
		case PANEL_BIT28:
			if(edit3_state & 0x10) m_store[(m_store_line << 2) | 3] ^= 0x08;
			break;
		case PANEL_BIT29:
			if(edit3_state & 0x20) m_store[(m_store_line << 2) | 3] ^= 0x04;
			break;
		case PANEL_BIT30:
			if(edit3_state & 0x40) m_store[(m_store_line << 2) | 3] ^= 0x02;
			break;
		case PANEL_BIT31:
			if(edit3_state & 0x80) m_store[(m_store_line << 2) | 3] ^= 0x01;
			break;
		case PANEL_LNUP:
			if(misc_state & 0x01)
			{
				m_store_line--;
			}
			break;
		case PANEL_LNDN:
			if(misc_state & 0x02)
			{
				m_store_line++;
			}
			break;
		case PANEL_HALT:
			if(misc_state & 0x04)
			{
				m_maincpu->set_state_int(SSEM_HALT, 1 - m_maincpu->state_int(SSEM_HALT));
			}
			break;
	}
}

static INPUT_PORTS_START( ssem )
	PORT_START("EDIT0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 0")  PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 1")  PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 2")  PORT_CODE(KEYCODE_3) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT2)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 3")  PORT_CODE(KEYCODE_4) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT3)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 4")  PORT_CODE(KEYCODE_5) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 5")  PORT_CODE(KEYCODE_6) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT5)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 6")  PORT_CODE(KEYCODE_7) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT6)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 7")  PORT_CODE(KEYCODE_8) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT7)

	PORT_START("EDIT1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 8")  PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT8)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 9")  PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT9)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 10")    PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT10)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 11")    PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT11)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 12")    PORT_CODE(KEYCODE_T) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT12)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 13")    PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT13)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 14")    PORT_CODE(KEYCODE_U) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT14)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 15")    PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT15)

	PORT_START("EDIT2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 16")    PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT16)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 17")    PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT17)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 18")    PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT18)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 19")    PORT_CODE(KEYCODE_F) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT19)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 20")    PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT20)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 21")    PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT21)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 22")    PORT_CODE(KEYCODE_J) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT22)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 23")    PORT_CODE(KEYCODE_K) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT23)

	PORT_START("EDIT3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 24")    PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT24)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 25")    PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT25)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 26")    PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT26)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 27")    PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT27)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 28")    PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT28)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 29")    PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT29)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 30")    PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT30)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bit 31")    PORT_CODE(KEYCODE_COMMA) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_BIT31)

	PORT_START("MISC")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Up")   PORT_CODE(KEYCODE_UP)   PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_LNUP)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Down") PORT_CODE(KEYCODE_DOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_LNDN)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Halt") PORT_CHANGED_MEMBER(DEVICE_SELF, ssem_state, panel_check, PANEL_HALT)
		PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

/****************************************************\
* Video hardware                                     *
\****************************************************/

static const uint8_t char_glyphs[0x80][8] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0x38, 0x10, 0x00, 0x00, 0x00 },
	{ 0x38, 0x7c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x00 },
	{ 0xff, 0xff, 0xef, 0xc7, 0xef, 0xff, 0xff, 0xff },
	{ 0xc7, 0x83, 0x01, 0x01, 0x01, 0x83, 0xc7, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 },
	{ 0x66, 0x66, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00 },
	{ 0x18, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x18, 0x00 },
	{ 0xc6, 0xcc, 0x18, 0x30, 0x60, 0xc6, 0x86, 0x00 },
	{ 0x78, 0xcc, 0x78, 0x70, 0xce, 0xcc, 0x7e, 0x00 },
	{ 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00 },
	{ 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00 },
	{ 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00 },
	{ 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 },
	{ 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 },
	{ 0x02, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x00 },
	{ 0x3c, 0x66, 0x76, 0x7e, 0x6e, 0x66, 0x3c, 0x00 },
	{ 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00 },
	{ 0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00 },
	{ 0x3c, 0x66, 0x06, 0x0c, 0x06, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x66, 0x66, 0x7e, 0x06, 0x06, 0x06, 0x00 },
	{ 0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x7e, 0x66, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00 },
	{ 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 },
	{ 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 },
	{ 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00 },
	{ 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00 },
	{ 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00 },
	{ 0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00 },
	{ 0x3c, 0x66, 0x6e, 0x6e, 0x60, 0x66, 0x3c, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7c, 0x00 },
	{ 0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00 },
	{ 0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00 },
	{ 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00 },
	{ 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00 },
	{ 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00 },
	{ 0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x06 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00 },
	{ 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 },
	{ 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00 },
	{ 0xc6, 0xc6, 0xd6, 0xfe, 0xfe, 0xee, 0xc6, 0x00 },
	{ 0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00 },
	{ 0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00 },
	{ 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00 },
	{ 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00 },
	{ 0x80, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00 },
	{ 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00 },
	{ 0x10, 0x38, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f },
	{ 0x30, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7c, 0x00 },
	{ 0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00 },
	{ 0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00 },
	{ 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00 },
	{ 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00 },
	{ 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00 },
	{ 0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00 },
	{ 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x06 },
	{ 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x66, 0x00 },
	{ 0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00 },
	{ 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 },
	{ 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00 },
	{ 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00 },
	{ 0xc6, 0xc6, 0xd6, 0xfe, 0xfe, 0xee, 0xc6, 0x00 },
	{ 0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00 },
	{ 0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00 },
	{ 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00 },
	{ 0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00 },
	{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 },
	{ 0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00 },
	{ 0x00, 0x00, 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff },
};

template <typename Format, typename... Params>
void ssem_state::glyph_print(bitmap_rgb32 &bitmap, int32_t x, int32_t y, Format &&fmt, Params &&...args)
{
	const rectangle &visarea = m_screen->visible_area();

	m_glyph_print_buf.clear();
	m_glyph_print_buf.seekp(0, util::ovectorstream::beg);
	util::stream_format(m_glyph_print_buf, std::forward<Format>(fmt), std::forward<Params>(args)...);
	m_glyph_print_buf.put('\0');

	for(char const *buf = &m_glyph_print_buf.vec()[0]; *buf; buf++)
	{
		uint8_t cur = uint8_t(*buf);
		if(cur < 0x80)
		{
			int32_t line = 0;
			for(line = 0; line < 8; line++)
			{
				uint32_t *const d = &bitmap.pix(y + line);
				for(uint32_t bit = 0; bit < 8; bit++)
				{
					if(char_glyphs[cur][line] & (1 << (7 - bit)))
					{
						d[x+bit] = 0xffffffff;
					}
					else
					{
						d[x+bit] = 0;
					}
				}
			}
		}

		x += 8;
		if(x >= visarea.max_x)
		{
			x = 0;
			y += 8;
			if(y >= visarea.max_y)
			{
				y = 0;
			}
		}
	}
}

uint32_t ssem_state::screen_update_ssem(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t line = 0;
	uint32_t accum = m_maincpu->state_int(SSEM_A);
	uint32_t bit = 0;
	uint32_t word = 0;

	for(line = 0; line < 32; line++)
	{
		word = (m_store[(line << 2) | 0] << 24) |
				(m_store[(line << 2) | 1] << 16) |
				(m_store[(line << 2) | 2] <<  8) |
				(m_store[(line << 2) | 3] <<  0);
		for(bit = 0; bit < 32; bit++)
		{
			if(word & (1 << (31 - bit)))
			{
				glyph_print(bitmap, bit << 3, line << 3, "%c", line == m_store_line ? 4 : 2);
			}
			else
			{
				glyph_print(bitmap, bit << 3, line << 3, "%c", line == m_store_line ? 3 : 1);
			}
		}
	}

	for(bit = 0; bit < 32; bit++)
	{
		if(accum & (1 << bit))
		{
			glyph_print(bitmap, bit << 3, 264, "%c", 2);
		}
		else
		{
			glyph_print(bitmap, bit << 3, 264, "%c", 1);
		}
	}

	word = reverse((m_store[(m_store_line << 2) | 0] << 24) |
					(m_store[(m_store_line << 2) | 1] << 16) |
					(m_store[(m_store_line << 2) | 2] <<  8) |
					(m_store[(m_store_line << 2) | 3] <<  0));
	glyph_print(bitmap, 0, 272, "LINE:%02u  VALUE:%08x  HALT:%d", m_store_line, word, m_maincpu->state_int(SSEM_HALT));
	return 0;
}

/****************************************************\
* Image helper functions                             *
\****************************************************/

std::string ssem_state::read_line(device_image_interface &image)
{
	std::ostringstream stream;
	for (u8 i = 0; i < 100; i++)
	{
		char c = 0;
		if (image.fread(&c, 1) != 1)
			break;
		// convert opcode to lower case
		if (c >= 'A' && c <= 'Z')
			c += 32;
		if (c >= 32)
			stream << c;
		else
		{
			image.fread(&c, 1); // skip LF
			break;
		}
	}
	return std::move(stream).str();
}


/****************************************************\
* Image loading                                      *
\****************************************************/

QUICKLOAD_LOAD_MEMBER(ssem_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	std::string buffer;
	u32 num_lines = 0;

	std::string image_line = read_line(image);
	if (image_line.empty())
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid image file: no data in line 1");

	sscanf(image_line.c_str(), "%d", &num_lines);  //num_lines = std::stoul(image_line);

	if (num_lines < 1)
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid image file: no data to process");

	for (u32 i = 0; i < num_lines; i++)
	{
		u32 line = 0, word = 0;
		image_line = read_line(image);
		u32 length = image_line.length();

		if (length < 8)
		{
			return std::make_pair(
					image_error::INVALIDIMAGE,
					util::string_format("Bad data (%s) in line %d", image_line, i + 2));
		}

		// Isolate and convert 4-digit decimal address
		buffer = image_line.substr(0, 4);
		sscanf(buffer.c_str(), "%04u", &line);

		if (image.is_filetype("snp"))
		{
			if (length < 37)
			{
				return std::make_pair(
						image_error::INVALIDIMAGE,
						util::string_format("Bad data (%s) in line %d", image_line, i + 2));
			}

			// Parse a line such as: 0000:00000110101001000100000100000100
			for (u8 b = 0; b < 32; b++)
				if (image_line[5 + b] == '1')
					word |= 1 << (31 - b);
		}
		else
		if (image.is_filetype("asm"))
		{
			int32_t value = 0;
			uint32_t unsigned_value = 0;

			// Isolate the value
			if (length > 8)
			{
				buffer = image_line.substr(9);
				sscanf(buffer.c_str(), "%d", &value);
				unsigned_value = reverse((uint32_t)value);
			}

			// Isolate the opcode
			buffer = image_line.substr(5, 3);

			if (buffer == "num")
				word = unsigned_value;
			else if (buffer == "jmp")
				word = 0x00000000 | unsigned_value ;
			else if (buffer == "jrp")
				word = 0x00040000 | unsigned_value;
			else if (buffer == "ldn")
				word = 0x00020000 | unsigned_value;
			else if (buffer == "sto")
				word = 0x00060000 | unsigned_value;
			else if (buffer == "sub")
				word = 0x00010000 | unsigned_value;
			else if (buffer == "cmp")
				word = 0x00030000 | unsigned_value;
			else if (buffer == "stp")
				word = 0x00070000 | unsigned_value;
			else
				logerror("Unknown opcode (%s) in line %d\n",buffer,i+2);
		}

		space.write_byte((line << 2) + 0, BIT(word, 24, 8));
		space.write_byte((line << 2) + 1, BIT(word, 16, 8));
		space.write_byte((line << 2) + 2, BIT(word, 8,  8));
		space.write_byte((line << 2) + 3, BIT(word, 0,  8));
	}

	return std::make_pair(std::error_condition(), std::string());
}

/****************************************************\
* Machine definition                                 *
\****************************************************/

void ssem_state::machine_start()
{
	save_item(NAME(m_store_line));
	m_glyph_print_buf.reserve(1024);
}

void ssem_state::machine_reset()
{
	m_store_line = 0;
}

void ssem_state::ssem(machine_config &config)
{
	/* basic machine hardware */
	SSEMCPU(config, m_maincpu, 700);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssem_state::ssem_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 280);
	m_screen->set_visarea(0, 255, 0, 279);
	m_screen->set_screen_update(FUNC(ssem_state::screen_update_ssem));
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "snp,asm", attotime::from_seconds(1)));
	quickload.set_load_callback(FUNC(ssem_state::quickload_cb));
	quickload.set_interface("ssemquik");
	SOFTWARE_LIST(config, "quik_list").set_original("ssem_quik");
}


ROM_START( ssem )
	ROM_REGION( 0x80, "maincpu", ROMREGION_ERASE00 )  /* Main Store */
ROM_END

} // anonymous namespace


//   YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                  FULLNAME
COMP(1948, ssem, 0,      0,      ssem,    ssem,  ssem_state, empty_init, "Manchester University", "Small-Scale Experimental Machine (SSEM), 'Baby'", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
