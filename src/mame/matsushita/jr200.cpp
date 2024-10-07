// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Zandona'
/***************************************************************************

JR-200 (c) 1982 National / Panasonic

BIOS will jump to D800 if it contains 7E. If not, it will jump to A000
 if it contains CE. Otherwise, the Monitor is entered. From the monitor,
 GA000 for cold start Basic, or GE79D for warm start Basic.

To get to the monitor from Basic type MON. Commands must be in UPPERcase.
Dnnnn : Display block of hex (D for next block)
Gnnnn : Go to address
Mnnnn : Modify memory at address

Cassette (if it worked):
- POKE 43,0 for 2400 baud, or POKE 43,1 for 600 baud (affects SAVE only);
- SAVE "x" (1-16 chars) to save. LOAD to load (it autodetects the speed).
- 600 baud uses Kansas City format, each byte has a start bit, 8 data bits
  (LSB first) and 3 stop bits. 1 = 2 cycles @1200Hz; 0 = 4 cycles @2400Hz
- 2400 baud has the same byte format, but each bit is represented in this
  way: 1 = half cycle @1200Hz; 0 = 1 cycle @2400Hz.
- Since it is not working, programs can be entered via the Paste facility.

TODO:
- Timings are basically screwed, it takes too much to load the POST but
  then the cursor blink is too fast
- keyboard MCU irq and data polling simulation should be inside a timer
  callback
- MN1544 4-bit CPU core and ROM dump
--- Character Generator ROM
--- Keyboard
--- Joysticks
--- Cassette baud switch
--- 128 bytes of RAM
- MN1271 device to be emulated
--- 4x 8-bit I/O ports
--- 3-bit port
--- timers
--- sound
--- cassette
--- serial / RS-232
--- interface between the 2 CPUs
- JR200 keyboard includes Kana characters, Kana On, Kana off, GRAPH
- JR200U keyboard omits Kana, but has GRAPH ON and GRAPH OFF instead.
- Keyboard matrix and decoding incomplete.
- The BREAK key on the unit is actually a soft reset.
- The Chargen interfaces to the MN1271, so that the text character
  definitions can be copied to D000-D7FF ram (via 0150-016F). The user can
  modify these characters with pokes. Since we have no information, some
  guesswork has been used to feed in the required info.
- Sound command never ends (SOUND 1000,50 should output 1000 Hz for 1 sec)
- Keyclick not working (POKE 0,64 should activate it)
- Cassette not implemented (needs MN1271 to work)
- Kana character 0xAC ('`) displays wrongly; chargen and manual conflict.

****************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


namespace {

class jr200_state : public driver_device
{
public:
	jr200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram")
		, m_cram(*this, "cram")
		, m_mn1271_ram(*this, "mn1271_ram")
		, m_maincpu(*this, "maincpu")
		, m_beeper(*this, "beeper")
		, m_pcg1(*this, "pcg1")
		, m_pcg2(*this, "pcg2")
		, m_gfx_rom(*this, "gfx_rom")
		, m_gfx_ram(*this, "gfx_ram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void jr200(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(nmi_button);

private:
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_shared_ptr<uint8_t> m_mn1271_ram;
	uint8_t m_border_col = 0;
	uint8_t m_old_keydata = 0;
	uint8_t m_freq_reg[2]{};
	u16 m_autorepeat = 0;
	u8 m_port_ctr = 0;
	int m_port_cnt = 0;
	emu_timer *m_timer_d = nullptr;
	uint8_t mcu_keyb_r();
	void unknown_port_w(u8);
	void jr200_beep_w(uint8_t data);
	void jr200_beep_freq_w(offs_t offset, uint8_t data);
	void jr200_border_col_w(uint8_t data);
	uint8_t mn1271_io_r(offs_t offset);
	void mn1271_io_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_jr200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_d_callback);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_shared_ptr<uint8_t> m_pcg1;
	required_shared_ptr<uint8_t> m_pcg2;
	required_region_ptr<u8> m_gfx_rom;
	required_shared_ptr<uint8_t> m_gfx_ram;
	required_ioport_array<11> m_io_keyboard;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



/* TODO: double check this */
static const uint8_t jr200_keycodes[6][7][8] =
{
	/* unshifted */
	{
	{ 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 },
	{ 0x39, 0x30, 0x2d, 0x5e, 0x08, 0x7f, 0x13, 0x5c },
	{ 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69 },
	{ 0x6f, 0x70, 0x40, 0x5b, 0x0d, 0x61, 0x73, 0x64 },
	{ 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x3a },
	{ 0x5d, 0x7a, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d },
	{ 0x2c, 0x2e, 0x2f, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	},

	/* shifted */
	{
	{ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28 },
	{ 0x29, 0x20, 0x3d, 0x5f, 0x08, 0x7f, 0x13, 0x7c },
	{ 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49 },
	{ 0x4f, 0x50, 0x60, 0x7b, 0x0d, 0x41, 0x53, 0x44 },
	{ 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x2b, 0x2a },
	{ 0x7d, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d },
	{ 0x3c, 0x3e, 0x3f, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	},

	/* graph on */
	{
	{ 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88 },
	{ 0x89, 0x30, 0xed, 0x8c, 0x08, 0x7f, 0x13, 0x8e },
	{ 0x98, 0x9b, 0x99, 0xec, 0xeb, 0x9a, 0xe9, 0x90 },
	{ 0x8d, 0xe0, 0xea, 0x5b, 0x0d, 0x91, 0x92, 0x93 },
	{ 0x94, 0x95, 0x96, 0x97, 0xef, 0xf0, 0x3b, 0x3a },
	{ 0x5d, 0xfa, 0xf8, 0xe3, 0xf6, 0xe2, 0xee, 0x8a },
	{ 0x2c, 0x2e, 0x2f, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	},

	/* graph on shifted*/
	{
	{ 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88 },
	{ 0x89, 0x30, 0xed, 0x8c, 0x08, 0x7f, 0x13, 0x8e },
	{ 0x9e, 0xff, 0x9f, 0x9c, 0x8f, 0x9a, 0xe9, 0xfe },
	{ 0x9d, 0xfc, 0xea, 0x5b, 0x0d, 0xf1, 0xf7, 0xe5 },
	{ 0xf2, 0xf4, 0xf9, 0xf5, 0xfb, 0xfd, 0x3b, 0x3a },
	{ 0x5d, 0xe1, 0xf3, 0xe4, 0xe6, 0xe7, 0xe8, 0x8b },
	{ 0x2c, 0x2e, 0x2f, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	},
	/* kana on */
	{
	{ 0xc7, 0xcc, 0xb1, 0xb3, 0xaa, 0xab, 0xac, 0xad },
	{ 0xae, 0xdc, 0xce, 0xcd, 0x08, 0x7f, 0x13, 0xb0 },
	{ 0xc0, 0xc3, 0xb2, 0xbd, 0xb6, 0xdd, 0xc5, 0xc6 },
	{ 0xd7, 0xbe, 0xde, 0xdf, 0x0d, 0xc1, 0xc4, 0xbc },
	{ 0xca, 0xb7, 0xb8, 0xcf, 0xc9, 0xd8, 0xda, 0xb9 },
	{ 0xd1, 0xaf, 0xbb, 0xbf, 0xcb, 0xba, 0xd0, 0xd3 },
	{ 0xc8, 0xd9, 0xd2, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	},

	/* kana on shifted*/
	{
	{ 0xc7, 0xcc, 0xb1, 0xb3, 0xaa, 0xab, 0xac, 0xad },
	{ 0xae, 0xa6, 0xce, 0xcd, 0x08, 0x7f, 0x13, 0xb0 },
	{ 0xc0, 0xc3, 0xb2, 0xbd, 0xb6, 0xdd, 0xc5, 0xc6 },
	{ 0xd7, 0xbe, 0xde, 0xa2, 0x0d, 0xc1, 0xc4, 0xbc },
	{ 0xca, 0xb7, 0xb8, 0xcf, 0xc9, 0xd8, 0xda, 0xb9 },
	{ 0xa3, 0xaf, 0xbb, 0xbf, 0xcb, 0xba, 0xd0, 0xd3 },
	{ 0xa4, 0xa1, 0xa5, 0x20, 0x1e, 0x1d, 0x1f, 0x1c },
	}
};


uint32_t jr200_state::screen_update_jr200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_border_col, cliprect);

	for (int y = 0; y < 24; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			uint8_t tile = m_vram[x + y*32];
			uint8_t attr = m_cram[x + y*32];

			for(int yi=0;yi<8;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					int pen = 0;
					if(attr & 0x80) //bitmap mode
					{
						/*
						    this mode draws 4 x 4 dot blocks, by combining lower 6 bits of tile and attribute vram

						    tile def
						    00xx x--- up-right
						    00-- -xxx up-left
						    attr def
						    10xx x--- down-right
						    10-- -xxx down-left
						*/
						u8 step = ((xi & 4) ? 3 : 0) + ((yi & 4) ? 6 : 0);

						pen = ((((attr & 0x3f) << 6) | (tile & 0x3f)) >> (step)) & 0x07;
					}
					else // tile mode
					{
						pen = BIT(m_gfx_ram[(tile*8)+yi], 7-xi) ? (attr & 0x7) : BIT(attr, 3, 3);
						if (BIT(attr, 6))
						{
							if ((tile >= 0x20) && (tile < 0x40))
							{
								tile &= 0x1f;
								pen = BIT(m_pcg1[(tile*8)+yi], 7-xi) ? (attr & 0x7) : BIT(attr, 3, 3);
							}
							else
							if ((tile >= 0x40) && (tile < 0x60))
							{
								tile &= 0x1f;
								pen = BIT(m_pcg2[(tile*8)+yi], 7-xi) ? (attr & 0x7) : BIT(attr, 3, 3);
							}
						}
					}

					bitmap.pix(y*8+yi+16, x*8+xi+16) = m_palette->pen(pen);
				}
			}
		}
	}

	return 0;
}


/*

I/O Device

*/

uint8_t jr200_state::mcu_keyb_r()
{
	if (m_port_ctr == 1)
		return m_gfx_rom[m_port_cnt];
	if (m_port_ctr == 2)
		return m_io_keyboard[10]->read();

	u8 modifiers = m_io_keyboard[7]->read();
	u8 table = 0, keydata = 0;
	u8 ret = 0;
	// KANA
	if (modifiers & 0x40)
		table = 4;
	// GRAPH
	if (modifiers & 0x10)
		table = 2;
	// SHIFT
	if (modifiers & 0x06)
		table ++;

	/* scan keyboard */
	for (u8 row = 0; row < 7; row++)
	{
		uint8_t data = m_io_keyboard[row]->read();

		for (u8 col = 0; col < 8; col++)
		{
			if (!BIT(data, col))
			{
				/* latch key data */
				keydata = jr200_keycodes[table][row][col];
				// LOCK
				if ((modifiers & 0x01) && ((keydata & 0xdf) > 0x40) && ((keydata & 0xdf) < 0x5b))
					keydata ^= 0x20;
				// CTRL
				if ((modifiers & 0x08) && (keydata > 0x40) && (keydata < 0x80))
					keydata &= 0x1f;
			}
		}
	}

	// Autorepeat handler
	// This might need to be done away with if games don't like it (should any be found)
	if (keydata && (m_old_keydata == keydata))
	{
		m_autorepeat++;
		if (m_autorepeat == 2) // initial keypress
			ret = keydata;
		else
		if (m_autorepeat == 0x330) // pause
		{
			ret = keydata;
			m_autorepeat = 0x2e0; // repeat speed (pause - this)
		}
	}
	else
	if (m_old_keydata != keydata)
	{
		// new key or none
		m_old_keydata = keydata;
		m_autorepeat = 0;
	}

	return ret;
}

void jr200_state::jr200_beep_w(uint8_t data)
{
	/* writing 0x0e enables the beeper, writing anything else disables it */
	m_beeper->set_state(((data & 0xf) == 0x0e) ? 1 : 0);
}

void jr200_state::jr200_beep_freq_w(offs_t offset, uint8_t data)
{
	m_freq_reg[offset] = data;

	u32 beep_freq = ((m_freq_reg[0]<<8) | (m_freq_reg[1] & 0xff)) + 1;

	if (beep_freq)
		m_beeper->set_clock(84000 / beep_freq);
	else
		m_beeper->set_clock(0);
}

void jr200_state::jr200_border_col_w(uint8_t data)
{
	m_border_col = data;
}


TIMER_CALLBACK_MEMBER(jr200_state::timer_d_callback)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

// get data from chargen for bios to copy to D000-D7FF.
// After that, one more copy for the cassette dipswitch
void jr200_state::unknown_port_w(u8 data)
{
	if ((m_port_ctr == 0) && (data == 0x31))
	{
		m_port_ctr++;
		m_port_cnt = -1;
	}
	else
	if ((m_port_ctr == 1) && (data == 0x73))
	{
		m_port_cnt++;
		if (m_port_cnt == 0x800)
			m_port_ctr++;
	}
	else
	if ((m_port_ctr == 2) && (data == 0x73))
		m_port_ctr++;
}

uint8_t jr200_state::mn1271_io_r(offs_t offset)
{
	uint8_t retVal = m_mn1271_ram[offset];

	switch(offset+0xc800)
	{
		case 0xc801: retVal= mcu_keyb_r(); break;
		case 0xc803: retVal= (m_mn1271_ram[0x03] & 0xcf) | 0x30;  break;//---x ---- printer status ready (ACTIVE HIGH)
		case 0xc807: retVal= (m_mn1271_ram[0x07] & 0x80) | 0x60; break;
		case 0xc80a: retVal= (m_mn1271_ram[0x0a] & 0xfe); break;
		case 0xc80c: retVal= (m_mn1271_ram[0x0c] & 0xdf) | 0x20; break;
		case 0xc80e: retVal= 0; break;
		case 0xc810: retVal= 0; break;
		case 0xc816: retVal= 0x4e; break;
		case 0xc81c: retVal= (m_mn1271_ram[0x1c] & 0xfe) | 1;  break;//bit 0 needs to be high otherwise system refuses to boot
		case 0xc81d: retVal= (m_port_ctr == 3) ? (m_mn1271_ram[0x1d] & 0xed) : 1; break;
	}
	//logerror("mn1271_io_r [%04x] = %02x\n",offset+0xc800,retVal);
	return retVal;
}

void jr200_state::mn1271_io_w(offs_t offset, uint8_t data)
{
	m_mn1271_ram[offset] = data;//printf("%X=%X ",offset,data);
	switch(offset+0xc800)
	{
		case 0xc803: unknown_port_w(data); break;
		case 0xc805: break; //LPT printer port W
		case 0xc816: if (data!=0) {
					m_timer_d->adjust(attotime::zero, 0, attotime::from_hz(XTAL(14'318'181)) * (m_mn1271_ram[0x17]*0x100 + m_mn1271_ram[0x18]));
				} else {
					m_timer_d->adjust(attotime::zero, 0,  attotime::zero);
				}
				break;
		case 0xc819: jr200_beep_w(data); break;
		case 0xc81a:
		case 0xc81b: jr200_beep_freq_w(offset-0x1a,data); break;
	}
}

void jr200_state::mem_map(address_map &map)
{
/*
    0000-3fff RAM
    4000-4fff RAM ( 4k expansion)
    4000-7fff RAM (16k expansion)
    4000-bfff RAM (32k expansion)
*/
	// 32K RAM
	map(0x0000, 0x7fff).ram();
	// BASIC ROM
	map(0xa000, 0xbfff).rom();
	// PCG 1
	map(0xc000, 0xc0ff).ram().share("pcg1");
	// Videoram
	map(0xc100, 0xc3ff).ram().share("vram");
	// PCG 2
	map(0xc400, 0xc4ff).ram().share("pcg2");
	// Attribute RAM
	map(0xc500, 0xc7ff).ram().share("cram");
	// MN1271 PIA/timer
	map(0xc800, 0xc81f).mirror(0x01e0).rw(FUNC(jr200_state::mn1271_io_r), FUNC(jr200_state::mn1271_io_w)).share("mn1271_ram");
	// YLHSD61K201F (or HD61K201F) CRTC
	map(0xca00, 0xcbff).w(FUNC(jr200_state::jr200_border_col_w));
	// External I/O area
	map(0xcc00, 0xcfff);
	// RAM-based normal characters
	map(0xd000, 0xd7ff).ram().share("gfx_ram");
	// System extension (presumably via the External Bus Connector - there's no "cart" slot)
	map(0xd800, 0xdfff).rom();
	// BIOS
	map(0xe000, 0xffff).rom();
}

/* Input ports */
static INPUT_PORTS_START( jr200 )
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(0x27)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUBOUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("YEN") PORT_CODE(KEYCODE_HOME) PORT_CHAR(0x5c) PORT_CHAR(0x7c)

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR(0x60)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X7")
	// This key does not exist, it's for our convenience only
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	// on JR200U there's GRAPH ON and GRAPH OFF keys
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_RCONTROL)
	// Kana is on JR200 only
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KANA OFF") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KANA ON") PORT_CODE(KEYCODE_RALT)
	// This key does a soft reset, so needs a proper NMI handler
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_PGDN) PORT_CHANGED_MEMBER(DEVICE_SELF, jr200_state, nmi_button, 0)

	PORT_START("X8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // reserved for Joystick 1

	PORT_START("X9")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // reserved for Joystick 2

	PORT_START("X10")
	PORT_DIPNAME( 0x01, 0x00, "Cassette Baud")  // mounted on the underside of the unit
	PORT_DIPSETTING(    0x00, "2400")
	PORT_DIPSETTING(    0x01, "600")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(jr200_state::nmi_button)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_jr200 )
	GFXDECODE_RAM( "gfx_ram", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_RAM( "pcg1", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_RAM( "pcg2", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

void jr200_state::machine_start()
{
	m_timer_d = timer_alloc(FUNC(jr200_state::timer_d_callback), this);
	save_item(NAME(m_border_col));
	save_item(NAME(m_old_keydata));
	save_item(NAME(m_freq_reg));
	save_item(NAME(m_autorepeat));
	save_item(NAME(m_port_ctr));
	save_item(NAME(m_port_cnt));
}

void jr200_state::machine_reset()
{
	m_autorepeat = 0;
	m_old_keydata = 0;
	m_port_ctr = 0;
	m_port_cnt = 0;
	memset(m_mn1271_ram,0,0x20);
}


void jr200_state::jr200(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(14'318'181) / 4); /* MN1800A, ? MHz assumption that it is same as JR-100*/
	m_maincpu->set_addrmap(AS_PROGRAM, &jr200_state::mem_map);

//  MN1544(config, "mn1544", ?);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(16 + 256 + 16, 16 + 192 + 16); /* border size not accurate */
	screen.set_visarea(0, 16 + 256 + 16 - 1, 0, 16 + 192 + 16 - 1);
	screen.set_screen_update(FUNC(jr200_state::screen_update_jr200));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jr200);
	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	SPEAKER(config, "mono").front_center();

	// All sounds are produced by the MN1271

	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS,"mono",0.50);
}



/* ROM definition */
ROM_START( jr200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1.bin",   0xa000, 0x2000, CRC(bfed707b) SHA1(551823e7ca63f459eb46eb4c7a3e1e169fba2ca2))
	ROM_LOAD( "rom2.bin",   0xe000, 0x2000, CRC(a1cb5027) SHA1(5da98d4ce9cba8096d98e6f2de60baa1673406d0))

	ROM_REGION( 0x10000, "mn1544", ROMREGION_ERASEFF )
	ROM_LOAD( "mn1544.bin",  0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x0800, "gfx_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "char.rom", 0x0000, 0x0800, CRC(cb641624) SHA1(6fe890757ebc65bbde67227f9c7c490d8edd84f2) )
ROM_END

ROM_START( jr200u )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom",  0xa000, 0x2000, CRC(cc53eb52) SHA1(910927b98a8338ba072173d79613422a8cb796da) )
	ROM_LOAD( "jr200u.bin", 0xe000, 0x2000, CRC(37ca3080) SHA1(17d3fdedb4de521da7b10417407fa2b61f01a77a) )

	ROM_REGION( 0x10000, "mn1544", ROMREGION_ERASEFF )
	ROM_LOAD( "mn1544.bin",  0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x0800, "gfx_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "char.rom", 0x0000, 0x0800, CRC(cb641624) SHA1(6fe890757ebc65bbde67227f9c7c490d8edd84f2) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME   FLAGS
COMP( 1982, jr200,  0,      0,      jr200,   jr200, jr200_state, empty_init, "National",  "JR-200",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1982, jr200u, jr200,  0,      jr200,   jr200, jr200_state, empty_init, "Panasonic", "JR-200U", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
