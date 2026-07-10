// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Bashkiria-2M driver by Miodrag Milanovic

2008-03-28 Preliminary driver.

To get numbers, you have to hold down Shift.

B2M:
- Hit enter while the square block is showing - it will attempt to boot
  a disk.
- Or, just wait and a menu appears with choices S,L,W,R,G. It's all in
  Russian, and choosing any of them produces an error. (Doesn't work
  currently?)

B2MROM:
- At start you are in an empty ramdisk called A:
- You can use SAVE to create an empty file in the CP/M fashion, and you can
  TYPE it.
- You can go to Basic by typing BAS. This is corrupted and only a few
  commands are accepted: REM, LIST. The commands RUN and NEW cause a disk
  error and the machine freezes. SYSTEM quits back to A:
- There's nothing else on the romdisk.

So, in the end, nothing useful works.

Need a schematic so that the fdc could be repaired.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "formats/smx_dsk.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class b2m_state : public driver_device
{
public:
	b2m_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_pit(*this, "pit")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_fdc(*this, "fdc")
		, m_fd(*this, "fdc:%u", 0U)
		, m_pic(*this, "pic")
	{ }

	void b2mrom(machine_config &config) ATTR_COLD;
	void b2m(machine_config &config) ATTR_COLD;

private:
	uint8_t keyboard_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);
	uint8_t palette_r(offs_t offset);
	void localmachine_w(uint8_t data);
	uint8_t localmachine_r();

	void b2m_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	void pit_out1(int state);
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);
	uint8_t ppi1_portb_r();
	void ppi2_portc_w(uint8_t data);
	uint8_t fdc_status_hack_r();
	uint8_t romdisk_porta_r();
	void romdisk_portb_w(uint8_t data);
	void romdisk_portc_w(uint8_t data);
	void fdc_drq(int state);
	static void b2m_floppy_formats(format_registration &fr);

	void b2m_io(address_map &map) ATTR_COLD;
	void b2m_mem(address_map &map) ATTR_COLD;
	void b2m_rom_io(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void postload();
	void set_bank(int bank);

	uint8_t m_porta = 0U;
	uint8_t m_video_scroll = 0U;
	uint8_t m_portc = 0U;

	bool m_drq_interlock = false;

	uint8_t m_video_page = 0U;

	uint8_t m_romdisk_lsb = 0U;
	uint8_t m_romdisk_msb = 0U;

	uint8_t m_color[4]{};
	uint8_t m_localmachine = 0U;
	uint8_t m_vblank_state = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	/* devices */
	optional_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_fd;
	optional_device<pic8259_device> m_pic;
};


uint8_t b2m_state::keyboard_r(offs_t offset)
{
	uint8_t key = 0x00;
	if (offset < 0x100) {
		if ((offset & 0x01)!=0) { key |= ioport("LINE0")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE1")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE2")->read(); }
		if ((offset & 0x08)!=0) { key |= ioport("LINE3")->read(); }
		if ((offset & 0x10)!=0) { key |= ioport("LINE4")->read(); }
		if ((offset & 0x20)!=0) { key |= ioport("LINE5")->read(); }
		if ((offset & 0x40)!=0) { key |= ioport("LINE6")->read(); }
		if ((offset & 0x80)!=0) { key |= ioport("LINE7")->read(); }
	} else {
		if ((offset & 0x01)!=0) { key |= ioport("LINE8")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE9")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE10")->read(); }
	}
	return key;
}


void b2m_state::set_bank(int bank)
{
	uint8_t *rom;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	space.install_ram(0x0000, 0xffff, ram);

	rom = memregion("maincpu")->base();
	switch(bank) {
	case 0 :
	case 1 :
		space.unmap_write(0xe000, 0xffff);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	case 2 :
		space.unmap_write(0x2800, 0x2fff);
		space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
		space.install_ram(0x3000, 0x6fff, ram + 0x10000);
		space.unmap_write(0xe000, 0xffff);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	case 3 :
		space.unmap_write(0x2800, 0x2fff);
		space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
		space.install_ram(0x3000, 0x6fff, ram + 0x14000);
		space.unmap_write(0xe000, 0xffff);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	case 4 :
		space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
		space.install_ram(0x3000, 0x6fff, ram + 0x18000);
		space.unmap_write(0xe000, 0xffff);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	case 5 :
		space.unmap_write(0x2800, 0x2fff);
		space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
		space.install_ram(0x3000, 0x6fff, ram + 0x1c000);
		space.unmap_write(0xe000, 0xffff);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	case 6 :
		break;
	case 7 :
		space.unmap_write(0x0000, 0xffff);
		space.unmap_read(0x0000, 0xffff);

		space.install_rom(0x0000, 0x1fff, rom);
		space.install_rom(0x2800, 0x2fff, rom);
		space.install_rom(0x3000, 0x4fff, rom);
		space.install_rom(0x7000, 0x8fff, rom);
		space.install_rom(0xe000, 0xffff, rom);
		break;
	}
}


void b2m_state::pit_out1(int state)
{
	m_speaker->level_w(state);
}

void b2m_state::ppi1_porta_w(uint8_t data)
{
	m_porta = data;
}

void b2m_state::ppi1_portb_w(uint8_t data)
{
	m_video_scroll = data;
}

void b2m_state::ppi1_portc_w(uint8_t data)
{
	m_portc = data;
	set_bank(m_portc & 7);
	m_video_page = BIT(m_portc, 7);
}

uint8_t b2m_state::ppi1_portb_r()
{
	return m_video_scroll;
}

void b2m_state::fdc_drq(int state)
{
	/* Clears HALT state of CPU when data is ready to read */
	if (state && m_drq_interlock)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}


void b2m_state::ppi2_portc_w(uint8_t data)
{
	uint8_t drive = BIT(~data, 1);
	uint8_t side  = BIT(~data, 0);

	floppy_image_device *floppy = nullptr;
	if (m_fd[drive].found())
		floppy = m_fd[drive]->get_device();

	if (floppy)
		floppy->mon_w(0);
	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(side);

	/*
	    When bit 5 is set CPU is in HALT state and stay there until
	    DRQ is triggered from floppy side
	*/
	m_drq_interlock = BIT(data, 5);
}

uint8_t b2m_state::fdc_status_hack_r()
{
	if (!machine().side_effects_disabled() && m_drq_interlock && !m_fdc->drq_r())
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	return m_fdc->status_r();
}

uint8_t b2m_state::romdisk_porta_r()
{
	uint8_t *romdisk = memregion("maincpu")->base() + 0x2000;
	return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
}

void b2m_state::romdisk_portb_w(uint8_t data)
{
	m_romdisk_lsb = data;
}

void b2m_state::romdisk_portc_w(uint8_t data)
{
	m_romdisk_msb = data & 0x7f;
}

void b2m_state::palette_w(offs_t offset, uint8_t data)
{
	uint8_t b = BIT(~data, 6, 2) * 0x55;
	uint8_t g = BIT(~data, 4, 2) * 0x55;
	uint8_t r = BIT(~data, 2, 2) * 0x55;

	uint8_t bw = BIT(~data, 0, 2) * 0x55;

	m_color[offset & 3] = data;

	if (ioport("MONITOR")->read()==1)
		m_palette->set_pen_color(offset, r, g, b);
	else
		m_palette->set_pen_color(offset, bw, bw, bw);
}

uint8_t b2m_state::palette_r(offs_t offset)
{
	return m_color[offset];
}

void b2m_state::localmachine_w(uint8_t data)
{
	m_localmachine = data;
}

uint8_t b2m_state::localmachine_r()
{
	return m_localmachine;
}

void b2m_state::postload()
{
	set_bank(m_portc & 7);
}

void b2m_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_porta));
	save_item(NAME(m_video_scroll));
	save_item(NAME(m_portc));
	save_item(NAME(m_video_page));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_romdisk_msb));
	save_item(NAME(m_color));
	save_item(NAME(m_localmachine));
	save_item(NAME(m_vblank_state));

	machine().save().register_postload(save_prepost_delegate(FUNC(b2m_state::postload), this));
}

INTERRUPT_GEN_MEMBER(b2m_state::vblank_interrupt)
{
	m_vblank_state++;
	if (m_vblank_state>1) m_vblank_state=0;
	m_pic->ir0_w(m_vblank_state);
}

void b2m_state::machine_reset()
{
	m_vblank_state = 0;
	set_bank(7);

	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

uint32_t b2m_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const ram = m_ram->pointer();

	for (int x = 0; x < 48; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			u16 const t = x*256 + ((y + m_video_scroll) & 0xff);
			u8 code1, code2;
			if (m_video_page==0)
			{
				code1 = ram[0x11000 + t];
				code2 = ram[0x15000 + t];
			}
			else
			{
				code1 = ram[0x19000 + t];
				code2 = ram[0x1d000 + t];
			}
			for (int b = 7; b >= 0; b--)
			{
				u8 const col = (BIT(code2, b)<<1) + BIT(code1, b);
				bitmap.pix(y, x*8+b) =  col;
			}
		}
	}

	return 0;
}

void b2m_state::b2m_palette(palette_device &palette) const
{
	static constexpr rgb_t b2m_pens[4] = {
		{ 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00 }, // 3
	};

	palette.set_pen_colors(0, b2m_pens);
}


/* Address maps */
void b2m_state::b2m_mem(address_map &map)
{
	// Dynamic, set with b2m_set_bank
}

void b2m_state::b2m_io(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x03).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x04, 0x07).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0c, 0x0c).rw(FUNC(b2m_state::localmachine_r), FUNC(b2m_state::localmachine_w));
	map(0x10, 0x13).rw(FUNC(b2m_state::palette_r), FUNC(b2m_state::palette_w));
	map(0x14, 0x15).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x18, 0x19).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x1c, 0x1f).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x1c, 0x1c).r(FUNC(b2m_state::fdc_status_hack_r));
}

void b2m_state::b2m_rom_io(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x03).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x04, 0x07).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0c, 0x0c).rw(FUNC(b2m_state::localmachine_r), FUNC(b2m_state::localmachine_w));
	map(0x10, 0x13).rw(FUNC(b2m_state::palette_r), FUNC(b2m_state::palette_w));
	map(0x14, 0x15).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x18, 0x19).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
}


/* Input ports */
static INPUT_PORTS_START( b2m )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('{')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('*') PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('&') PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('^') PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('%') PORT_CHAR('5')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('$') PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('#') PORT_CHAR('3')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('@') PORT_CHAR('2')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('_') PORT_CHAR('-')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('\'') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("`") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('!') PORT_CHAR('1')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('|') PORT_CHAR('\\')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR(')') PORT_CHAR('0')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('(') PORT_CHAR('9')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Rshift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LAlt") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LCtrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("???") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LShift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PgdN") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("end") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('?') PORT_CHAR('/')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\"') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)    PORT_CHAR('>') PORT_CHAR('.')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PgUp") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("LINE10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("MONITOR")
	PORT_CONFNAME(0x01, 0x01, "Monitor")
	PORT_CONFSETTING(   0x01, "Color")
	PORT_CONFSETTING(   0x00, "B/W")
INPUT_PORTS_END

void b2m_state::b2m_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SMX_FORMAT);
}

static void b2m_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


/* Machine driver */
void b2m_state::b2m(machine_config &config)
{
	/* basic machine hardware */
	i8080_cpu_device &maincpu(I8080(config, m_maincpu, 2000000));
	maincpu.set_addrmap(AS_PROGRAM, &b2m_state::b2m_mem);
	maincpu.set_addrmap(AS_IO, &b2m_state::b2m_io);
	maincpu.set_vblank_int("screen", FUNC(b2m_state::vblank_interrupt));
	maincpu.in_inta_func().set("pic", FUNC(pic8259_device::acknowledge));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(384, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(b2m_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(b2m_state::b2m_palette), 4);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(0);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_pit->set_clk<1>(2000000);
	m_pit->out_handler<1>().set(FUNC(b2m_state::pit_out1));
	m_pit->set_clk<2>(2000000);
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8253_device::write_clk0));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(b2m_state::ppi1_porta_w));
	ppi1.in_pb_callback().set(FUNC(b2m_state::ppi1_portb_r));
	ppi1.out_pb_callback().set(FUNC(b2m_state::ppi1_portb_w));
	ppi1.out_pc_callback().set(FUNC(b2m_state::ppi1_portc_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.out_pc_callback().set(FUNC(b2m_state::ppi2_portc_w));

	i8255_device &ppi3(I8255(config, "ppi3"));
	ppi3.in_pa_callback().set(FUNC(b2m_state::romdisk_porta_r));
	ppi3.out_pb_callback().set(FUNC(b2m_state::romdisk_portb_w));
	ppi3.out_pc_callback().set(FUNC(b2m_state::romdisk_portc_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	/* sound */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* uart */
	I8251(config, "uart");

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->drq_wr_callback().set(FUNC(b2m_state::fdc_drq));

	FLOPPY_CONNECTOR(config, m_fd[0], b2m_floppies, "525qd", b2m_state::b2m_floppy_formats);
	FLOPPY_CONNECTOR(config, m_fd[1], b2m_floppies, "525qd", b2m_state::b2m_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("b2m");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_default_value(0x00);
}

void b2m_state::b2mrom(machine_config &config)
{
	b2m(config);
	m_maincpu->set_addrmap(AS_IO, &b2m_state::b2m_rom_io);
}

/* ROM definition */

ROM_START( b2m )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "b2m.rom", 0x0000, 0x2000, CRC(3f3214d6) SHA1(dd93e7fbabf14d1aed6777fe1ccfe0a3ca8fcaf2) )
ROM_END

ROM_START( b2mrom )
	ROM_REGION( 0x80c0, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios2.rom",  0x0000, 0x2000, CRC(c22a98b7) SHA1(7de91e653bf4b191ded62cf21532578268e4a2c1) )
	ROM_LOAD( "ramdos.sys", 0x2000, 0x60c0, CRC(91ed6df0) SHA1(4fd040f2647a6b7930c330c75560a035027d0606) )
ROM_END

} // anonymous namespace

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME                 FLAGS */
COMP( 1989, b2m,    0,      0,      b2m,     b2m,   b2m_state, empty_init, "BNPO",  "Bashkiria-2M",          MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP( 1989, b2mrom, b2m,    0,      b2mrom,  b2m,   b2m_state, empty_init, "BNPO",  "Bashkiria-2M ROM-disk", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
