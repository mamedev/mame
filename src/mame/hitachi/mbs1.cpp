// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

MB-S1 (c) 1984 Hitachi

TODO:
- Cassette won't load, not even in Level 3 mode;
- Many features missing;

**************************************************************************************************/

#include "emu.h"

#include "bml3.h"
#include "mbs1_mmu.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mbs1_state : public bml3mk5_state
{
public:
	mbs1_state(const machine_config &mconfig, device_type type, const char *tag)
		: bml3mk5_state(mconfig, type, tag)
		, m_mmu(*this, "mmu")
		, m_vtram(*this, "vtram")
		, m_system_mode(*this, "SYSTEM_MODE")
	{ }

	void mbs1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

//  virtual void main_map(address_map &map) override ATTR_COLD;
	virtual void system_io(address_map &map) override ATTR_COLD;
	void s1_map(address_map &map) ATTR_COLD;
	void s1_mmu_map(address_map &map) ATTR_COLD;
	void s1_ext_io(address_map &map) ATTR_COLD;

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

private:
	required_device<mbs1_mmu_device> m_mmu;
	required_shared_ptr<u8> m_vtram;
	required_ioport m_system_mode;

	bool m_su_mode = 0;
	u8 m_system_clock = 0;
	bool m_system_type = false;
};

MC6845_UPDATE_ROW( mbs1_state::crtc_update_row )
{
	if (!m_system_type)
	{
		bml3mk5_state::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
		return;
	}
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 const bgcolor = 0; //m_hres_reg & 7;

	// TODO: WIDTH80, pinpoint bit used
	for (u8 x = 0; x < x_count; x += 2)
	{
		u16 mem = ((ma + x) >> 1) & 0x7ff;

		//u8 const attr = m_aram[mem];
		u8 const rawbits = m_vtram[mem];
		int const tile = rawbits & 0x7f;
		int const tile_bank = BIT(rawbits, 7);
		u8 const color = 7; //attr & 7;
		bool const reverse = (x == cursor_x); // ^BIT(attr, 3);

		u8 gfx_data = m_p_chargen[(tile<<4)|(ra<<1)|tile_bank];

		for (u8 xi = 0; xi < 8; xi++)
		{
			u8 const pen = BIT(reverse ? ~gfx_data : gfx_data, 7 - xi) ? color : bgcolor;
			int const res_x = x * 8 + xi * 2;
			bitmap.pix(y, res_x + 0) = palette[pen];
			bitmap.pix(y, res_x + 1) = palette[pen];
		}
	}

}


void mbs1_state::s1_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_mmu, FUNC(mbs1_mmu_device::read), FUNC(mbs1_mmu_device::write));
}

// TODO: sloppily enough, a memory_view can't see that is actually on higher base
// so using just map(0xf0000, 0xfffff).m(*this, FUNC(mbs1_state::main_map));
// will just punt with "A memory_view must be installed at its configuration address."
// We also eventually need to add some memory sharing here, for the work RAM so for now
// lets just repeat lv3 memory map.
void mbs1_state::s1_mmu_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x00000, 0x7ffff) extended RAM

//  map(0x84000, 0x8ffff) mirror of main work RAM +$4000
	map(0x84000, 0x8ffff).ram();
//  map(0xb0000, 0xb3fff) GVRAM
//  map(0xb4000, 0xbbfff) extra GVRAM banks
	map(0xbc000, 0xbc7ff).mirror(0x800).ram().share("vtram");
//  map(0xc0000, 0xc6fff) ROM for MPC-CM01 (card?)
//  map(0xc7000, 0xc7fff) EEPROM for MPC-CM01
	map(0xd0000, 0xd7fff).rom().region("dictionary", 0);
	map(0xe0000, 0xe7fff).rom().region("s1basic", 0);
//  map(0xe2000, 0xe2fff) view overlay for MPC-CM01
	map(0xe8000, 0xeffff).rom().region("s1boot", 0);
	map(0xefe00, 0xefeff).m(*this, FUNC(mbs1_state::s1_ext_io));
	map(0xeff00, 0xeffff).m(*this, FUNC(mbs1_state::system_io));

	map(0xf0000, 0xf03ff).ram();
	map(0xf0400, 0xf43ff).rw(FUNC(mbs1_state::vram_r), FUNC(mbs1_state::vram_w));
	map(0xf4400, 0xf7fff).ram();
	map(0xf8000, 0xf9fff).ram();
	map(0xfa000, 0xfbfff).view(m_banka);
	m_banka[0](0xfa000, 0xfbfff).readonly().share("rama");
	map(0xfa000, 0xfbfff).writeonly().share("rama");
	map(0xfc000, 0xfdfff).view(m_bankc);
	m_bankc[0](0xfc000, 0xfdfff).readonly().share("ramc");
	map(0xfc000, 0xfdfff).writeonly().share("ramc");
	map(0xfe000, 0xfefff).view(m_banke);
	m_banke[0](0xfe000, 0xfefff).readonly().share("rame");
	map(0xfe000, 0xfefff).writeonly().share("rame");
	map(0xff000, 0xffeff).view(m_bankf);
	m_bankf[0](0xff000, 0xffeff).readonly().share("ramf");
	map(0xff000, 0xffeff).writeonly().share("ramf");
	map(0xffff0, 0xfffff).view(m_bankg);
	m_bankg[0](0xffff0, 0xfffff).readonly().share("ramg");
	map(0xffff0, 0xfffff).writeonly().share("ramg");
	map(0xf8000, 0xfffff).view(m_rom_view);
	m_rom_view[0](0xfa000, 0xffeff).rom().region("maincpu", 0xa000);
	m_rom_view[0](0xffff0, 0xfffff).rom().region("maincpu", 0xfff0);
	map(0xfff00, 0xfffef).m(*this, FUNC(mbs1_state::system_io));
	map(0xfa000, 0xfa7ff).view(m_ig_view);
	m_ig_view[0](0xfa000, 0xfa7ff).writeonly().w(FUNC(mbs1_state::ig_ram_w));

}

// relative to $efe00
void mbs1_state::s1_ext_io(address_map &map)
{
	map(0x00, 0x0f).rw(m_mmu, FUNC(mbs1_mmu_device::bank_r), FUNC(mbs1_mmu_device::bank_w));
//  map(0x10, 0x10) FUSE (w/o)
//  map(0x11, 0x11) OS-9 address segment
//  map(0x18, 0x18) TRAP (r/o)
//  map(0x19, 0x19) BUS control
//  map(0x1a, 0x1b) Pro-control / Acc-control for MPC-68008
//  map(0x20, 0x20) BMSK color (w/o)
//  map(0x21, 0x21) Active graphic plane
//  map(0x23, 0x23) Display page
//  map(0x24, 0x24) SCRN mode
//  map(0x25, 0x27) BRG graphic color
//  map(0x28, 0x2f) palette
//  map(0x40, 0x41) PIA 1 (DE-9)
//  map(0x42, 0x43) PIA 2 (Printer, mirror of 0xffc2-c3)
}

void mbs1_state::system_io(address_map &map)
{
	bml3mk5_state::system_io(map);
	map(0xeffeb, 0xeffeb).lrw8(
		NAME([this]() {
			return (m_system_type) | (m_system_clock << 1) | (m_su_mode << 2);
		}),
		NAME([this] (u8 data) {
			logerror("$effeb system clock write %02x\n", data);
			m_system_clock = BIT(data, 1);
			m_maincpu->set_unscaled_clock(CPU_EXT_CLOCK * (m_system_clock + 1));
		})
	);
}

// TODO: duplicated from base bml3, needs keyboard converted into slot device
static INPUT_PORTS_START( mbs1 )
	PORT_START("DIPSW")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	// TODO: seems to apply for Level 3 mode only
	// May have been removed and hardwired to '1' here.
	PORT_DIPNAME( 0x04, 0x04, "40-/80-column")
	PORT_DIPSETTING(0x00, "40 chars/line")
	PORT_DIPSETTING(0x04, "80 chars/line")
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM_MODE")
	PORT_DIPNAME( 0x01, 0x01, "System Mode")
	PORT_DIPSETTING(0x00, "Level 3 mode")
	PORT_DIPSETTING(0x01, "S1 mode")

	PORT_START("X0") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("? PAD")
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X2")  // ???
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Lock") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana Shift")  PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 PAD") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 PAD") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('^')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 PAD") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete PAD") //backspace
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(u8"¥") PORT_CODE(KEYCODE_TAB)

	PORT_START("X1") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 PAD") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". PAD")  PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) //or cls?
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 PAD") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 PAD") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 PAD") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- PAD") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("X2") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ PAD") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 PAD") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 PAD") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 PAD") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0xffe00000,IP_ACTIVE_HIGH,IPT_UNKNOWN)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHANGED_MEMBER(DEVICE_SELF, bml3_state, nmi_button, 0)
INPUT_PORTS_END

void mbs1_state::machine_start()
{
	bml3mk5_state::machine_start();
	save_item(NAME(m_system_clock));
	save_item(NAME(m_su_mode));
}


void mbs1_state::machine_reset()
{
	bml3mk5_state::machine_reset();
	m_system_type = bool(BIT(m_system_mode->read(), 0));
	m_mmu->init_banks(m_system_type, m_su_mode);
}


void mbs1_state::mbs1(machine_config &config)
{
	bml3mk5_state::bml3mk5(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbs1_state::s1_map);

	MBS1_MMU(config, m_mmu, 0);
	m_mmu->set_map(&mbs1_state::s1_mmu_map);
}

ROM_START( mbs1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("rom1.rom", 0xa000, 0x5f00, CRC(b23b0530) SHA1(cb9a8352d2ba0fc0085e4219cd07c91c8447ab75) )
	ROM_LOAD("rom2.rom", 0xfff0, 0x0010, CRC(63b9adb2) SHA1(62b950671b533d18bca36eae362f2bc26638b587) )

	ROM_REGION( 0x8000, "s1boot", ROMREGION_ERASEFF )
	ROM_LOAD("s1rom2.rom", 0x0000, 0x7e00, CRC(e266108c) SHA1(18af24fa8f123960562fbe1c58e209d06d62c205) )
	ROM_LOAD("s1romi.rom", 0x7ff0, 0x0010, CRC(81cc0e4d) SHA1(7e52264e0fdb5cb9a3489c70842f4d12146060ba) )

	ROM_REGION( 0x8000, "s1basic", ROMREGION_ERASEFF )
	ROM_LOAD("s1bas1.rom", 0x0000, 0x8000, CRC(efb80721) SHA1(81c926f9fa658012cd04b30176aad800db46594e) )

	ROM_REGION( 0x8000, "dictionary", ROMREGION_ERASEFF )
	ROM_LOAD("s1dic.rom", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("font.rom", 0x0000, 0x1000, CRC(d1f27c5a) SHA1(a3abbdea9f6656bd795fd35ee806a54d7be35de0) )
ROM_END

} // anonymous namespace


COMP( 1984, mbs1,    0,     0,      mbs1,    mbs1,  mbs1_state,    empty_init, "Hitachi", "MB-S1", MACHINE_NOT_WORKING )
