// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Sandro Ronco
/***************************************************************************

        Videoton TVC 32/64 driver

        12/05/2009 Skeleton driver.

        TODO:
        - UPM crashes when formatting a floppy
        - overscan and mid-frame changes

****************************************************************************/

#include "emu.h"
#include "audio/tvc.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "video/mc6845.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/tvc/tvc.h"
#include "bus/tvc/hbf.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/tvc_cas.h"

#define TVC_RAM_BANK    1
#define TVC_ROM_BANK    2

#define CENTRONICS_TAG  "centronics"


class tvc_state : public driver_device
{
public:
	tvc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_sound(*this, "custom")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_expansions(*this, "exp%u", 1)
		, m_palette(*this, "palette")
		, m_keyboard(*this, "LINE.%u", 0)
	{ }

	void tvc(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<tvc_sound_device> m_sound;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<centronics_device> m_centronics;
	required_device_array<tvcexp_slot_device, 4> m_expansions;
	required_device<palette_device> m_palette;
	required_ioport_array<16> m_keyboard;

	memory_region *m_bios_rom;
	memory_region *m_cart_rom;
	memory_region *m_ext;
	memory_region *m_vram;

	uint8_t       m_video_mode;
	uint8_t       m_keyline;
	uint8_t       m_active_slot;
	uint8_t       m_int_flipflop;
	uint8_t       m_col[4];
	uint8_t       m_bank_type[4];
	uint8_t       m_bank;
	uint8_t       m_vram_bank;
	uint8_t       m_cassette_ff;
	uint8_t       m_centronics_ff;

	void set_mem_page(uint8_t data);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(vram_bank_w);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(int_state_r);
	DECLARE_WRITE8_MEMBER(flipflop_w);
	DECLARE_WRITE8_MEMBER(border_color_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(cassette_w);
	DECLARE_READ8_MEMBER(_5b_r);
	DECLARE_WRITE_LINE_MEMBER(int_ff_set);
	DECLARE_WRITE_LINE_MEMBER(centronics_ack);

	// expansions
	DECLARE_WRITE8_MEMBER(expansion_w);
	DECLARE_READ8_MEMBER(expansion_r);
	DECLARE_READ8_MEMBER(exp_id_r);
	DECLARE_WRITE8_MEMBER(expint_ack_w);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	MC6845_UPDATE_ROW(crtc_update_row);

	void tvc_palette(palette_device &palette) const;

	void tvc_io(address_map &map);
	void tvc_mem(address_map &map);
};



#define TVC_INSTALL_ROM_BANK(_bank,_tag,_start,_end) \
	if (m_bank_type[_bank] != TVC_ROM_BANK) \
	{ \
		space.install_read_bank(_start, _end, _tag); \
		space.unmap_write(_start, _end); \
		m_bank_type[_bank] = TVC_ROM_BANK; \
	}
#define TVC_INSTALL_RAM_BANK(_bank,_tag,_start,_end) \
	if (m_bank_type[_bank] != TVC_RAM_BANK) \
	{ \
		space.install_readwrite_bank(_start, _end, _tag); \
		m_bank_type[_bank] = TVC_RAM_BANK; \
	}

void tvc_state::set_mem_page(uint8_t data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	switch (data & 0x18)
	{
		case 0x00 : // system ROM selected
			TVC_INSTALL_ROM_BANK(0, "bank1", 0x0000, 0x3fff);
			membank("bank1")->set_base(m_bios_rom->base());
			break;
		case 0x08 : // Cart ROM selected
			if (m_cart_rom)
			{
				TVC_INSTALL_ROM_BANK(0, "bank1", 0x0000, 0x3fff);
				membank("bank1")->set_base(m_cart_rom->base());
			}
			break;
		case 0x10 : // RAM selected
			TVC_INSTALL_RAM_BANK(0, "bank1", 0x0000, 0x3fff);
			membank("bank1")->set_base(m_ram->pointer());
			break;
		case 0x18 : // Video RAM
			if (m_vram->bytes() > 0x4000)
			{
				// TVC 64+ only
				TVC_INSTALL_RAM_BANK(0, "bank1", 0x0000, 0x3fff);
				membank("bank1")->set_base(m_vram->base() + ((m_vram_bank & 0x03)<<14));
			}
			else
			{
				space.unmap_readwrite(0x0000, 0x3fff);
				m_bank_type[0] = -1;
			}
			break;
	}

	if ((data & 0x20)==0)       // Video RAM
	{
		TVC_INSTALL_RAM_BANK(2, "bank3", 0x8000, 0xbfff);
		membank("bank3")->set_base(m_vram->base() + ((m_vram_bank & 0x0c)<<12));
	}
	else                        // System RAM page 3
	{
		if (m_ram->size() > 0x8000)
		{
			TVC_INSTALL_RAM_BANK(2, "bank3", 0x8000, 0xbfff);
			membank("bank3")->set_base(m_ram->pointer() + 0x8000);
		}
		else
		{
			space.unmap_readwrite(0x8000, 0xbfff);
			m_bank_type[2] = -1;
		}
	}

	switch(data & 0xc0)
	{
		case 0x00 : // Cart ROM selected
			if (m_cart_rom)
			{
				TVC_INSTALL_ROM_BANK(3, "bank4", 0xc000, 0xffff);
				membank("bank4")->set_base(m_cart_rom->base());
			}
			break;
		case 0x40 : // System ROM selected
			TVC_INSTALL_ROM_BANK(3, "bank4", 0xc000, 0xffff);
			membank("bank4")->set_base(m_bios_rom->base());
			break;
		case 0x80 : // RAM selected
			if (m_ram->size() > 0x8000)
			{
				TVC_INSTALL_RAM_BANK(3, "bank4", 0xc000, 0xffff);
				membank("bank4")->set_base(m_ram->pointer() + 0xc000);
			}
			else
			{
				space.unmap_readwrite(0xc000, 0xffff);
				m_bank_type[3] = -1;
			}
			break;
		case 0xc0 : // External ROM selected
			TVC_INSTALL_ROM_BANK(3, "bank4", 0xc000, 0xffff);
			membank("bank4")->set_base(m_ext->base());
			space.install_readwrite_handler (0xc000, 0xdfff, read8_delegate(FUNC(tvc_state::expansion_r), this), write8_delegate(FUNC(tvc_state::expansion_w), this), 0);
			m_bank_type[3] = -1;
			break;
	}
}


WRITE8_MEMBER(tvc_state::expansion_w)
{
	m_expansions[m_active_slot & 3]->write(space, offset, data);
}


READ8_MEMBER(tvc_state::expansion_r)
{
	return m_expansions[m_active_slot & 3]->read(space, offset);
}

WRITE8_MEMBER(tvc_state::bank_w)
{
	m_bank = data;
	set_mem_page(data);
}

WRITE8_MEMBER(tvc_state::vram_bank_w)
{
	// TVC 64+ only
	if (m_vram->bytes() > 0x4000)
	{
		// bit 4-5 - screen video RAM
		// bit 2-3 - video RAM active in bank 3
		// bit 0-1 - video RAM active in bank 1

		m_vram_bank = data;
		set_mem_page(m_bank);
	}
}

WRITE8_MEMBER(tvc_state::palette_w)
{
	//  0 I 0 G | 0 R 0 B
	//  0 0 0 0 | I G R B
	int i = ((data&0x40)>>3) | ((data&0x10)>>2) | ((data&0x04)>>1) | (data&0x01);

	m_col[offset] = i;
}

WRITE8_MEMBER(tvc_state::keyboard_w)
{
	// bit 6-7 - expansion select
	// bit 0-3 - keyboard scan

	m_keyline = data & 0x0f;
	m_active_slot = (data>>6) & 0x03;
}

READ8_MEMBER(tvc_state::keyboard_r)
{
	return m_keyboard[m_keyline & 0x0f]->read();
}

READ8_MEMBER(tvc_state::int_state_r)
{
	/*
	    x--- ----   centronics ACK flipflop
	    -x-- ----   colour
	    --x- ----   cassette input
	    ---x ----   vblank or tone interrupt
	    ---- xxxx   expansions interrupt (active low)
	*/

	double level = m_cassette->input();

	uint8_t expint = (m_expansions[0]->int_r()<<0) | (m_expansions[1]->int_r()<<1) |
					(m_expansions[2]->int_r()<<2) | (m_expansions[3]->int_r()<<3);

	return 0x40 | (m_int_flipflop << 4) | (level > 0.01 ? 0x20 : 0x00) | (m_centronics_ff << 7) | (expint & 0x0f);
}

WRITE8_MEMBER(tvc_state::flipflop_w)
{
	// every write here clears the vblank flipflop
	m_int_flipflop = 1;
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

READ8_MEMBER(tvc_state::exp_id_r)
{
	// expansion slots ID
	return  (m_expansions[0]->id_r()<<0) | (m_expansions[1]->id_r()<<2) |
			(m_expansions[2]->id_r()<<4) | (m_expansions[3]->id_r()<<6);
}

WRITE8_MEMBER(tvc_state::expint_ack_w)
{
	m_expansions[offset & 3]->int_ack();
}

WRITE8_MEMBER(tvc_state::border_color_w)
{
	// x-x- x-x-    border color (I G R B)
}


WRITE8_MEMBER(tvc_state::sound_w)
{
	switch(offset)
	{
		case 1:
			// bit 6-7 - cassette motors
			m_cassette->change_state(BIT(data, 6) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
			//m_cassette2->change_state(BIT(data, 7) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
			m_cassette->output(m_cassette_ff ? +1 : -1);
			break;
		case 2:
			// bit 0-1 - video mode
			// bit 7   - centronics STROBE
			m_video_mode = data & 0x03;
			m_centronics->write_strobe(BIT(data, 7));
			if (!BIT(data, 7))
				m_centronics_ff = 0;
			break;
	}

	// sound ports
	m_sound->write(space, offset, data);
}

READ8_MEMBER(tvc_state::_5b_r)
{
	m_sound->reset_divider();
	return 0xff;
}

WRITE8_MEMBER(tvc_state::cassette_w)
{
	// writig here cause the toggle of the cassette flipflop
	m_cassette_ff = !m_cassette_ff;
	m_cassette->output(m_cassette_ff ? +1 : -1);
}

void tvc_state::tvc_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void tvc_state::tvc_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(tvc_state::border_color_w));
	map(0x01, 0x01).w("cent_data_out", FUNC(output_latch_device::bus_w));
	map(0x02, 0x02).w(FUNC(tvc_state::bank_w));
	map(0x03, 0x03).w(FUNC(tvc_state::keyboard_w));
	map(0x04, 0x06).w(FUNC(tvc_state::sound_w));
	map(0x07, 0x07).w(FUNC(tvc_state::flipflop_w));
	map(0x0f, 0x0f).w(FUNC(tvc_state::vram_bank_w));
	map(0x10, 0x1f).rw("exp1", FUNC(tvcexp_slot_device::io_read), FUNC(tvcexp_slot_device::io_write));
	map(0x20, 0x2f).rw("exp2", FUNC(tvcexp_slot_device::io_read), FUNC(tvcexp_slot_device::io_write));
	map(0x30, 0x3f).rw("exp3", FUNC(tvcexp_slot_device::io_read), FUNC(tvcexp_slot_device::io_write));
	map(0x40, 0x4f).rw("exp4", FUNC(tvcexp_slot_device::io_read), FUNC(tvcexp_slot_device::io_write));
	map(0x50, 0x50).w(FUNC(tvc_state::cassette_w));
	map(0x58, 0x58).r(FUNC(tvc_state::keyboard_r));
	map(0x59, 0x59).r(FUNC(tvc_state::int_state_r));
	map(0x5a, 0x5a).r(FUNC(tvc_state::exp_id_r));
	map(0x5b, 0x5b).r(FUNC(tvc_state::_5b_r));
	map(0x58, 0x5b).w(FUNC(tvc_state::expint_ack_w));
	map(0x60, 0x63).w(FUNC(tvc_state::palette_w));
	map(0x70, 0x70).w("crtc", FUNC(mc6845_device::address_w));
	map(0x71, 0x71).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

/* Input ports */
static INPUT_PORTS_START( tvc )
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)         PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)         PORT_CHAR('3')  PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)         PORT_CHAR('2')  PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('/')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\x8d") PORT_CODE(KEYCODE_1_PAD)  PORT_CHAR(0x00cd)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)         PORT_CHAR('1')  PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)         PORT_CHAR('4')  PORT_CHAR('!')

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)         PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xbc") PORT_CODE(KEYCODE_2_PAD)  PORT_CHAR(0x00fc)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)      PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xb3") PORT_CODE(KEYCODE_3_PAD)  PORT_CHAR(0x00f3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xb6") PORT_CODE(KEYCODE_4_PAD)  PORT_CHAR(0x00f6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)         PORT_CHAR('7')  PORT_CHAR('=')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)         PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)         PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';')  PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)         PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_HOME)      PORT_CHAR('@')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)         PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)         PORT_CHAR('r')  PORT_CHAR('R')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)         PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)         PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc5\x91") PORT_CODE(KEYCODE_5_PAD)  PORT_CHAR(0x0151)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xba") PORT_CODE(KEYCODE_6_PAD)  PORT_CHAR(0x00fa)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)         PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)         PORT_CHAR('u')  PORT_CHAR('U')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)         PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)         PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)         PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')    PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)         PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_END)       PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)         PORT_CHAR('f')  PORT_CHAR('F')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)         PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)         PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xa1") PORT_CODE(KEYCODE_7_PAD)  PORT_CHAR(0x00e1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc5\xb1") PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(0x0171)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xa9") PORT_CODE(KEYCODE_9_PAD)  PORT_CHAR(0x00e9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)         PORT_CHAR('j')  PORT_CHAR('J')

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)         PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)         PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)         PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)         PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)         PORT_CHAR('v')  PORT_CHAR('V')

	PORT_START("LINE.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT)    PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')  PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)     PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)         PORT_CHAR('m')  PORT_CHAR('M')

	PORT_START("LINE.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insert")  PORT_CODE(KEYCODE_INSERT)  PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up")      PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down")    PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fire")    PORT_CODE(KEYCODE_PGUP)    //PORT_CHAR(UCHAR_MAMEKEY())
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Acc")     PORT_CODE(KEYCODE_PGDN)    //PORT_CHAR(UCHAR_MAMEKEY())
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right")   PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left")    PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( tvc64pru )
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)         PORT_CHAR('4')  PORT_CHAR(0x0436)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)         PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)         PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DL") PORT_CODE(KEYCODE_HOME)     // delete line
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)         PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DC") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)         PORT_CHAR('3')  PORT_CHAR('#')

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("IL") PORT_CODE(KEYCODE_END)      // insert line
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)         PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)         PORT_CHAR('8')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('&')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)         PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)         PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)         PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)         PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)         PORT_CHAR('k')  PORT_CHAR('K')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)         PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)         PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)         PORT_CHAR('g')  PORT_CHAR('G')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)         PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)         PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)         PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)         PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)         PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)         PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)         PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')    PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)         PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)         PORT_CHAR('o')  PORT_CHAR('O')

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)         PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)         PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)         PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)         PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)         PORT_CHAR('m')  PORT_CHAR('M')

	PORT_START("LINE.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT)    PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR('@')  PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)     PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)         PORT_CHAR('x')  PORT_CHAR('X')

	PORT_START("LINE.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insert")     PORT_CODE(KEYCODE_INSERT)   PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Up")     PORT_CODE(KEYCODE_8_PAD)    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Down")   PORT_CODE(KEYCODE_2_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Fire")   PORT_CODE(KEYCODE_PGUP)     //PORT_CHAR(UCHAR_MAMEKEY())
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Acc")    PORT_CODE(KEYCODE_PGDN)     //PORT_CHAR(UCHAR_MAMEKEY())
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Right")  PORT_CODE(KEYCODE_6_PAD)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Joy Left")   PORT_CODE(KEYCODE_4_PAD)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.9")
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*")  PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_START("LINE.10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void tvc_state::machine_start()
{
	for (int i = 0; i < 4; i++)
		m_col[i] = i;

	m_int_flipflop = 0;

	m_bios_rom = memregion("sys");
	m_ext = memregion("ext");
	m_vram = memregion("vram");

	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
}

void tvc_state::machine_reset()
{
	memset(m_ram->pointer(), 0, m_ram->size());
	set_mem_page(0);
	m_video_mode = 0;
	m_cassette_ff = 1;
	m_centronics_ff = 1;
	m_bank = 0;
	m_vram_bank = 0;
	memset(m_bank_type, 0, sizeof(m_bank_type));
	m_active_slot = 0;

	// Bank 2 is always RAM
	membank("bank2")->set_base(m_ram->pointer() + 0x4000);
}

MC6845_UPDATE_ROW( tvc_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint8_t *vram = m_vram->base() + ((m_vram_bank & 0x30)<<10);
	uint16_t offset = ((ma*4 + ra*0x40) & 0x3fff);
	int i;

	switch(m_video_mode) {
		case 0 :
				//  2 colors mode
				for ( i = 0; i < x_count; i++ )
				{
					uint8_t data = vram[offset + i];
					*p++ = palette[m_col[BIT(data,7)]];
					*p++ = palette[m_col[BIT(data,6)]];
					*p++ = palette[m_col[BIT(data,5)]];
					*p++ = palette[m_col[BIT(data,4)]];
					*p++ = palette[m_col[BIT(data,3)]];
					*p++ = palette[m_col[BIT(data,2)]];
					*p++ = palette[m_col[BIT(data,1)]];
					*p++ = palette[m_col[BIT(data,0)]];
				}
				break;
		case 1 :
				// 4 colors mode
				// a0 b0 c0 d0 a1 b1 c1 d1
				for ( i = 0; i < x_count; i++ )
				{
					uint8_t data = vram[offset + i];
					*p++ = palette[m_col[BIT(data,3)*2 + BIT(data,7)]];
					*p++ = palette[m_col[BIT(data,3)*2 + BIT(data,7)]];
					*p++ = palette[m_col[BIT(data,2)*2 + BIT(data,6)]];
					*p++ = palette[m_col[BIT(data,2)*2 + BIT(data,6)]];
					*p++ = palette[m_col[BIT(data,1)*2 + BIT(data,5)]];
					*p++ = palette[m_col[BIT(data,1)*2 + BIT(data,5)]];
					*p++ = palette[m_col[BIT(data,0)*2 + BIT(data,4)]];
					*p++ = palette[m_col[BIT(data,0)*2 + BIT(data,4)]];
				}
				break;
		default:
				// 16 colors mode
				// IIGG RRBB
				for ( i = 0; i < x_count; i++ )
				{
					uint8_t data = vram[offset + i];
					uint8_t col0 = ((data & 0x80)>>4) | ((data & 0x20)>>3) | ((data & 0x08)>>2) | ((data & 0x02)>>1);
					uint8_t col1 = ((data & 0x40)>>3) | ((data & 0x10)>>2) | ((data & 0x04)>>1) | (data & 0x01);
					*p++ = palette[col0];
					*p++ = palette[col0];
					*p++ = palette[col0];
					*p++ = palette[col0];
					*p++ = palette[col1];
					*p++ = palette[col1];
					*p++ = palette[col1];
					*p++ = palette[col1];
				}
				break;

	}
}

void tvc_state::tvc_palette(palette_device &palette) const
{
	static constexpr rgb_t tvc_pens[16] =
	{
		{ 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x7f },
		{ 0x7f, 0x00, 0x00 },
		{ 0x7f, 0x00, 0x7f },
		{ 0x00, 0x7f, 0x00 },
		{ 0x00, 0x7f, 0x7f },
		{ 0x7f, 0x7f, 0x00 },
		{ 0x7f, 0x7f, 0x7f },
		{ 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0xff },
		{ 0xff, 0x00, 0x00 },
		{ 0xff, 0x00, 0xff },
		{ 0x00, 0xff, 0x00 },
		{ 0x00, 0xff, 0xff },
		{ 0xff, 0xff, 0x00 },
		{ 0xff, 0xff, 0xff }
	};

	palette.set_pen_colors(0, tvc_pens);
}

WRITE_LINE_MEMBER(tvc_state::int_ff_set)
{
	if (state)
	{
		m_int_flipflop = 0;
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(tvc_state::centronics_ack)
{
	if (state)
		m_centronics_ff = 1;
}

QUICKLOAD_LOAD_MEMBER(tvc_state::quickload_cb)
{
	uint8_t first_byte;

	image.fread(&first_byte, 1);
	if (first_byte == 0x11)
	{
		image.fseek(0x90, SEEK_SET);
		image.fread(m_ram->pointer() + 0x19ef, image.length() - 0x90);
		return image_init_result::PASS;
	}
	else
	{
		return image_init_result::FAIL;
	}
}


void tvc_exp(device_slot_interface &device)
{
	device.option_add("hbf", TVC_HBF);      // Videoton HBF floppy interface
}


void tvc_state::tvc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3125000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tvc_state::tvc_mem);
	m_maincpu->set_addrmap(AS_IO, &tvc_state::tvc_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 240);
	screen.set_visarea(0, 512 - 1, 0, 240 - 1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(tvc_state::tvc_palette), 16);

	mc6845_device &crtc(MC6845(config, "crtc", 3125000/2)); // clk taken from schematics
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8); /*?*/
	crtc.set_update_row_callback(FUNC(tvc_state::crtc_update_row), this);
	crtc.out_cur_callback().set(FUNC(tvc_state::int_ff_set));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("32K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TVC_SOUND(config, m_sound, 0);
	m_sound->sndint_wr_callback().set(FUNC(tvc_state::int_ff_set));
	m_sound->add_route(ALL_OUTPUTS, "mono", 0.75);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(tvc_state::centronics_ack));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "tvc_cart", "bin,rom,crt");

	/* expansion interface */
	TVCEXP_SLOT(config, m_expansions[0], tvc_exp , nullptr);
	m_expansions[0]->out_irq_callback().set_inputline(m_maincpu, 0);
	m_expansions[0]->out_nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	TVCEXP_SLOT(config, m_expansions[1], tvc_exp , nullptr);
	m_expansions[1]->out_irq_callback().set_inputline(m_maincpu, 0);
	m_expansions[1]->out_nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	TVCEXP_SLOT(config, m_expansions[2], tvc_exp , nullptr);
	m_expansions[2]->out_irq_callback().set_inputline(m_maincpu, 0);
	m_expansions[2]->out_nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	TVCEXP_SLOT(config, m_expansions[3], tvc_exp , nullptr);
	m_expansions[3]->out_irq_callback().set_inputline(m_maincpu, 0);
	m_expansions[3]->out_nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(tvc64_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("tvc_cass");
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* quickload */
	QUICKLOAD(config, "quickload", "cas", attotime::from_seconds(6)).set_load_callback(FUNC(tvc_state::quickload_cb), this);

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("tvc_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("tvc_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("tvc_flop");
}

/* ROM definition */
ROM_START( tvc64 )
	ROM_REGION( 0x4000, "sys", ROMREGION_ERASEFF )
	ROM_LOAD( "tvc12_d4.64k", 0x0000, 0x2000, CRC(834ca9be) SHA1(c333318c1c6185aae2d3dfb86d55e3a4a3071a73))
	ROM_LOAD( "tvc12_d3.64k", 0x2000, 0x2000, CRC(71753d02) SHA1(d9a1905cf55c532b3380c83158fb5254ee503829))

	ROM_REGION( 0x4000, "ext", ROMREGION_ERASEFF )
	ROM_LOAD( "tvc12_d7.64k", 0x2000, 0x2000, CRC(1cbbeac6) SHA1(54b29c9ca9942f04620fbf3edab3b8e3cd21c194))

	ROM_REGION( 0x4000, "vram", ROMREGION_ERASE )
ROM_END

ROM_START( tvc64p )
	ROM_REGION( 0x4000, "sys", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v22", "v2.2")
	ROMX_LOAD( "tvc22_d6.64k", 0x0000, 0x2000, CRC(05ac3a34) SHA1(bdc7eda5fd53f806dca8c4929ee498e8e59eb787), ROM_BIOS(0) )
	ROMX_LOAD( "tvc22_d4.64k", 0x2000, 0x2000, CRC(ba6ad589) SHA1(e5c8a6db506836a327d901387a8dc8c681a272db), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v21", "v2.1")
	ROMX_LOAD( "tvc21_d6.64k", 0x0000, 0x2000, CRC(f197ffce) SHA1(7b27a91504dd864170451949ada5f938d6532cae), ROM_BIOS(1) )
	ROMX_LOAD( "tvc21_d4.64k", 0x2000, 0x2000, CRC(b054c0b2) SHA1(c8ca8d5a4d092604de01e2cafc2a2dabe94e6380), ROM_BIOS(1) )

	ROM_REGION( 0x4000, "ext", ROMREGION_ERASEFF )
	ROM_LOAD( "tvc22_d7.64k", 0x2000, 0x2000, CRC(05e1c3a8) SHA1(abf119cf947ea32defd08b29a8a25d75f6bd4987))

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE )
ROM_END

ROM_START( tvc64pru )
	ROM_REGION( 0x4000, "sys", ROMREGION_ERASEFF )
	ROM_LOAD( "tvcru_d6.bin", 0x0000, 0x2000, CRC(1e0fa0b8) SHA1(9bebb6c8f03f9641bd35c9fd45ffc13a48e5c572))
	ROM_LOAD( "tvcru_d4.bin", 0x2000, 0x2000, CRC(bac5dd4f) SHA1(665a1b8c80b6ad82090803621f0c73ef9243c7d4))

	ROM_REGION( 0x4000, "ext", ROMREGION_ERASEFF )
	ROM_LOAD( "tvcru_d7.bin", 0x2000, 0x2000, CRC(70cde756) SHA1(c49662af9f6653347ead641e85777c3463cc161b))

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT     CLASS      INIT        COMPANY       FULLNAME             FLAGS
COMP( 1985, tvc64,    0,      0,      tvc,     tvc,      tvc_state, empty_init, "Videoton",   "TVC 64",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1985, tvc64p,   tvc64,  0,      tvc,     tvc,      tvc_state, empty_init, "Videoton",   "TVC 64+",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1985, tvc64pru, tvc64,  0,      tvc,     tvc64pru, tvc_state, empty_init, "Videoton",   "TVC 64+ (Russian)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
