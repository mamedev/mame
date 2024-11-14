// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert, Sergey Svishchev
/***************************************************************************

        Robotron PC-1715

        10/06/2008 Preliminary driver.

    Notes:
    - keyboard connected to sio channel a
    - sio channel a clock output connected to ctc trigger 0

    Docs:
    - http://www.robotrontechnik.de/html/computer/pc1715w.htm
    - https://www.tiffe.de/Robotron/PC1715/ -- scanned PDFs
      https://www.tiffe.de/Robotron/PC1715/MANUAL_PC_1715.pdf
    - http://www.sax.de/~zander/pc1715/pc_bw.html -- typeset PDFs
      http://www.sax.de/~zander/pc1715/doku/pc_manu.pdf
    - http://xepb.org/robotron/

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"


#define LOG_BANK    (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGBANK(format, ...)    LOGMASKED(LOG_BANK,   "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

class rt1715_state : public driver_device
{
public:
	rt1715_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_sio0(*this, "sio0")
		, m_ctc0(*this, "ctc0")
		, m_printer(*this, "printer")
		, m_rs232(*this, "rs232")
		, m_fdc(*this, "i8272")
		, m_floppy(*this, "i8272:%u", 0U)
		, m_dma(*this, "z80dma")
		, m_ctc2(*this, "ctc2")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_crtc(*this, "i8275")
		, m_p_chargen(*this, "gfx")
		, m_videoram(*this, "videoram")
		, m_p_cas(*this, "prom")
	{ }

	void rt1715(machine_config &config);
	void rt1715w(machine_config &config);

private:
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);
	void tc_w(int state);
	void rt1715_floppy_enable(uint8_t data);
	uint8_t k7658_led1_r();
	uint8_t k7658_led2_r();
	uint8_t k7658_data_r(offs_t offset);
	void k7658_data_w(uint8_t data);
	void rt1715_rom_disable(uint8_t data);
	void rt1715w_set_bank(uint8_t data);
	void rt1715w_floppy_motor(uint8_t data);
	void rt1715w_krfd_w(uint8_t data);
	void rt1715_palette(palette_device &palette) const;
	I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);
	void crtc_drq_w(int state);

	void k7658_io(address_map &map) ATTR_COLD;
	void k7658_mem(address_map &map) ATTR_COLD;
	void rt1715_base_io(address_map &map) ATTR_COLD;
	void rt1715_io(address_map &map) ATTR_COLD;
	void rt1715w_io(address_map &map) ATTR_COLD;
	void rt1715_mem(address_map &map) ATTR_COLD;
	void rt1715w_mem(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(rt1715);
	DECLARE_MACHINE_RESET(rt1715);
	DECLARE_MACHINE_START(rt1715w);
	DECLARE_MACHINE_RESET(rt1715w);

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<z80sio_device> m_sio0;
	required_device<z80ctc_device> m_ctc0;
	required_device<rs232_port_device> m_printer;
	required_device<rs232_port_device> m_rs232;
	optional_device<i8272a_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppy;
	optional_device<z80dma_device> m_dma;
	optional_device<z80ctc_device> m_ctc2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc;
	required_region_ptr<uint8_t> m_p_chargen;
	optional_device<ram_device> m_videoram;
	optional_region_ptr<uint8_t> m_p_cas;

	int m_led1_val = 0;
	int m_led2_val = 0;
	u8 m_krfd = 0U;
	uint16_t m_dma_adr = 0U;
	int m_r = 0, m_w = 0;
};


/***************************************************************************
    FLOPPY
***************************************************************************/

void rt1715_state::rt1715_floppy_enable(uint8_t data)
{
	LOG("%s: rt1715_floppy_enable %02x\n", machine().describe_context(), data);
}

void rt1715_state::rt1715w_floppy_motor(uint8_t data)
{
	LOG("%s: rt1715w_floppy_motor %02x\n", machine().describe_context(), data);

	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(data & 0x80 ? 1 : 0);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(data & 0x08 ? 1 : 0);
}

void rt1715_state::rt1715w_krfd_w(uint8_t data)
{
	LOG("%s: rt1715w_krfd_w %02x\n", machine().describe_context(), data);
	m_krfd = data;
}

void rt1715_state::tc_w(int state)
{
	m_fdc->tc_w(state & BIT(m_krfd, 7));
}

/***************************************************************************
    KEYBOARD
***************************************************************************/

/* si/so led */
uint8_t rt1715_state::k7658_led1_r()
{
	m_led1_val ^= 1;
	LOG("%s: k7658_led1_r %02x\n", machine().describe_context(), m_led1_val);
	return 0xff;
}

/* caps led */
uint8_t rt1715_state::k7658_led2_r()
{
	m_led2_val ^= 1;
	LOG("%s: k7658_led2_r %02x\n", machine().describe_context(), m_led2_val);
	return 0xff;
}

/* read key state */
uint8_t rt1715_state::k7658_data_r(offs_t offset)
{
	uint8_t result = 0xff;

	if (BIT(offset,  0)) result &= ioport("row_00")->read();
	if (BIT(offset,  1)) result &= ioport("row_10")->read();
	if (BIT(offset,  2)) result &= ioport("row_20")->read();
	if (BIT(offset,  3)) result &= ioport("row_30")->read();
	if (BIT(offset,  4)) result &= ioport("row_40")->read();
	if (BIT(offset,  5)) result &= ioport("row_50")->read();
	if (BIT(offset,  6)) result &= ioport("row_60")->read();
	if (BIT(offset,  7)) result &= ioport("row_70")->read();
	if (BIT(offset,  8)) result &= ioport("row_08")->read();
	if (BIT(offset,  9)) result &= ioport("row_18")->read();
	if (BIT(offset, 10)) result &= ioport("row_28")->read();
	if (BIT(offset, 11)) result &= ioport("row_38")->read();
	if (BIT(offset, 12)) result &= ioport("row_48")->read();

	return result;
}

/* serial output on D0 */
void rt1715_state::k7658_data_w(uint8_t data)
{
	LOG("%s: k7658_data_w %d\n", machine().describe_context(), BIT(data, 0));
	m_sio0->rxa_w(BIT(data, 0));
	m_sio0->rxca_w(0);
	m_sio0->rxca_w(1);
}


/***************************************************************************
    MEMORY HANDLING
***************************************************************************/

MACHINE_START_MEMBER(rt1715_state, rt1715)
{
	membank("bank2")->set_base(m_ram->pointer() + 0x0800);
	membank("bank3")->set_base(m_ram->pointer());
}

MACHINE_RESET_MEMBER(rt1715_state, rt1715)
{
	/* on reset, enable ROM */
	membank("bank1")->set_base(memregion("ipl")->base());
}

void rt1715_state::rt1715_rom_disable(uint8_t data)
{
	LOGBANK("%s: rt1715_set_bank %02x\n", machine().describe_context(), data);

	/* disable ROM, enable RAM */
	membank("bank1")->set_base(m_ram->pointer());
}

MACHINE_START_MEMBER(rt1715_state, rt1715w)
{
}

MACHINE_RESET_MEMBER(rt1715_state, rt1715w)
{
	m_dma->rdy_w(1);
	m_krfd = 0;
	m_dma_adr = 0;
	m_r = 0;
	m_w = 0;
}

/*
   BR (A62, A63)

   b2..0 = AB18..16

   0 - Hintergrundbank (Bildschirm, Zeichengeneratoren)
   1 - Systembank (gebanktes BIOS, BDOS)
   2 - Anwenderbank (TPA)
   3 - RAM-Disk
   4 - RAM-Disk
   5 - RAM-Disk
*/
void rt1715_state::rt1715w_set_bank(uint8_t data)
{
	int w = (data >> 4) & 7;
	int r = data & 7;

	LOGBANK("%s: rt1715w_set_bank target %x source %x%s\n", machine().describe_context(), w, r, r == w ? "" : " DIFF");

	m_r = r;
	m_w = w;
}

uint8_t rt1715_state::memory_read_byte(offs_t offset)
{
	uint8_t data = 0;

	switch (m_r)
	{
	case 0:
		switch (offset >> 12)
		{
		case 0:
			data = memregion("ipl")->base()[offset & 0x7ff];
			break;

		case 1:
			break;

		case 2:
			data = m_p_chargen[offset & 0xfff];
			break;

		case 3:
			data = m_videoram->pointer()[offset & 0xfff];
			break;

		default:
			data = m_ram->pointer()[offset];
			break;
		}
		LOGBANK("mem r %04x bank %d == %02x\n", offset, m_r, data);
		break;

	default:
		uint8_t cas_addr = 128 + (m_r << 4) + ((offset >> 12) & 15);
		uint8_t cas_data = m_p_cas[cas_addr] ^ 15;
		switch (cas_data)
		{
		case 1:
			data = m_ram->pointer()[offset];
			break;

		case 2:
			data = m_ram->pointer()[offset + 0x10000];
			break;

		case 4:
			data = m_ram->pointer()[offset + 0x20000];
			break;

		case 8:
			data = m_ram->pointer()[offset + 0x30000];
			break;

		default:
			break;
		}
		LOGBANK("mem r %04x bank %d cas %d(%02x) == %02x\n", offset, m_r, cas_data, cas_addr, data);
		break;
	}
	return data;
}

void rt1715_state::memory_write_byte(offs_t offset, uint8_t data)
{
	switch (m_w)
	{
	case 0:
		switch (offset >> 12)
		{
		case 0:
		case 1:
			break;

		case 2:
			m_p_chargen[offset & 0xfff] = data;
			break;

		case 3:
			m_videoram->pointer()[offset & 0xfff] = data;
			break;

		default:
			m_ram->pointer()[offset] = data;
			break;
		}
		LOGBANK("mem w %04x bank %d <- %02x\n", offset, m_w, data);
		break;

	default:
		uint8_t cas_addr = 128 + (m_w << 4) + ((offset >> 12) & 15);
		uint8_t cas_data = m_p_cas[cas_addr] ^ 15;
		switch (cas_data)
		{
		case 1:
			m_ram->pointer()[offset] = data;
			break;

		case 2:
			m_ram->pointer()[offset + 0x10000] = data;
			break;

		case 4:
			m_ram->pointer()[offset + 0x20000] = data;
			break;

		case 8:
			m_ram->pointer()[offset + 0x30000] = data;
			break;

		default:
			break;
		}
		LOGBANK("mem w %04x bank %d cas %d(%02x) <- %02x\n", offset, m_w, cas_data, cas_addr, data);
		break;
	}
}

uint8_t rt1715_state::io_read_byte(offs_t offset)
{
	address_space &prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void rt1715_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

void rt1715_state::crtc_drq_w(int state)
{
	if (state)
	{
		m_crtc->dack_w(m_videoram->pointer()[m_dma_adr++]);
		m_dma_adr %= (80 * 24);
	}
}

I8275_DRAW_CHARACTER_MEMBER(rt1715_state::crtc_display_pixels)
{
	using namespace i8275_attributes;

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 gfx = BIT(attrcode, LTEN) ? 0xff : 0;

	if (!BIT(attrcode, VSP))
		gfx = m_p_chargen[(BIT(attrcode, GPA0) ? 0x800 : 0) | (linecount << 7) | charcode];

	if (BIT(attrcode, RVV))
		gfx ^= 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for (u8 i=0; i<8; i++)
		bitmap.pix(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
}

/* F4 Character Displayer */
static const gfx_layout rt1715_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*128, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( gfx_rt1715 )
	GFXDECODE_ENTRY("gfx", 0x0000, rt1715_charlayout, 0, 1)
	GFXDECODE_ENTRY("gfx", 0x0800, rt1715_charlayout, 0, 1)
GFXDECODE_END


/***************************************************************************
    PALETTE
***************************************************************************/

void rt1715_state::rt1715_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_pen_color(1, rgb_t(0x00, 0x7f, 0x00)); // low intensity
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // high intensity
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void rt1715_state::rt1715_mem(address_map &map)
{
	map(0x0000, 0x07ff).bankr("bank1").bankw("bank3");
	map(0x0800, 0xffff).bankrw("bank2");
}

void rt1715_state::rt1715_base_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x0b).rw(m_ctc0, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw(m_sio0, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x10, 0x17).noprw();
//  map(0x10, 0x13).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
//  map(0x14, 0x17).rw(m_sio1, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x18, 0x19).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
//  map(0x2c, 0x2f) // LT107CS -- serial DSR?
//  map(0x30, 0x33) // LT111CS -- serial SEL? (data rate selector)
}

void rt1715_state::rt1715_io(address_map &map)
{
	rt1715_base_io(map);

	map(0x00, 0x03).rw("a71", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // floppy data
	map(0x04, 0x07).rw("a72", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // floppy control/status
	map(0x20, 0x20).w(FUNC(rt1715_state::rt1715_floppy_enable));
//  map(0x24, 0x27).w(FUNC(rt1715_state::rt1715_rom_enable)); // MEMCS0
	map(0x28, 0x2b).w(FUNC(rt1715_state::rt1715_rom_disable)); // MEMCS1
//  map(0x34, 0x37) // BWSCS (read: memory start address, write: switch chargen)
}

void rt1715_state::rt1715w_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(rt1715_state::memory_read_byte), FUNC(rt1715_state::memory_write_byte));
}

// rt1715w -- decoders A13, A14, page C
void rt1715_state::rt1715w_io(address_map &map)
{
	rt1715_base_io(map);

	map(0x00, 0x00).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write)); // A2
	map(0x04, 0x07).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // A4
//  map(0x1a, 0x1b) // chargen write protection
	map(0x1c, 0x1d).m(m_fdc, FUNC(i8272a_device::map));
	map(0x20, 0x23).w(FUNC(rt1715_state::rt1715w_krfd_w)); // KRFD -- FD-Steuerregister (A45)
	map(0x24, 0x27).w(FUNC(rt1715_state::rt1715w_set_bank)); // BR (A62, A63)
	map(0x28, 0x2b).w(FUNC(rt1715_state::rt1715w_floppy_motor)); // MOS
	map(0x34, 0x37).portr("S8"); // KON -- Konfigurations-schalter FD (config switch -- A114, DIP S8)
//  map(0x38, 0x3b) // SR (RST1) -- Ru:cksetzen von Flip-Flops im FD
//  map(0x3c, 0x3f) // RST (RST2) -- Ru:cksetzen von Flip-Flops in V.24 (Pru:ftechnik)
	// these two ports are accessed only via DMA
	map(0x40, 0x40).r(m_fdc, FUNC(i8272a_device::msr_r));
	map(0x41, 0x41).rw(m_fdc, FUNC(i8272a_device::dma_r), FUNC(i8272a_device::dma_w));
}

void rt1715_state::k7658_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0xf800).rom();
}

void rt1715_state::k7658_io(address_map &map)
{
	map(0x0000, 0x1fff).w(FUNC(rt1715_state::k7658_data_w)).nopr();
	map(0x2000, 0x2000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led1_r));
	map(0x4000, 0x4000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led2_r));
	map(0x8000, 0x9fff).r(FUNC(rt1715_state::k7658_data_r));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( k7658 )
	PORT_START("row_00")
	// D04 A54 E04 B54 B04 C04 D54 C54
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad S *1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad S *3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad CE") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))

	PORT_START("row_10")
	// D03 A53 E03 B53 B03 C03 D53 C53
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

	PORT_START("row_20")
	// D02 A52 E02 B52 B02 C02 D52 C52
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	PORT_START("row_30")
	// D11 --- E11 A15 A10 C11 --- A16
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad New-Line") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("row_40")
	// D10 --- E10 --- B10 C10 E52 E51
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("row_50")
	// D12 --- E12 B16 B01 C12 D16 C16
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')

	PORT_START("row_60")
	// D07 A17 E07 B17 B07 C07 D17 C17
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A17 8E")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("rechter Rand") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("row_70")
	// D01 A51 E01 B51 B00 C01 D51 C51
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("row_08")
	// D00 B99 E00 --- B11 C00 E15 E16
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SI/SO") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_START("row_18")
	// D08 A56 E08 B56 B08 C08 D56 C56
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad SQ (F14)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad PS (F13)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))

	PORT_START("row_28")
	// D09 E53 E09 E54 B09 C09 E55 E56
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("row_38")
	// D05 A55 E05 B55 B05 C05 D55 C55
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("row_48")
	// D06 A05 E06 B15 B06 C06 D15 C15
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("linker Rand")
INPUT_PORTS_END

static INPUT_PORTS_START( rt1715w )
	PORT_INCLUDE(k7658)
	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Floppy drive type" )
	PORT_DIPSETTING(    0x01, "5.25\"-FD" )
	PORT_DIPSETTING(    0x00, "8\"-FD" )
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/* verify priority -- p. 14 of PC-1715-Servicemanual.pdf */
static const z80_daisy_config rt1715_daisy_chain[] =
{
	{ "a71" },
	{ "a72" },
	{ "ctc0" },
	{ "sio0" },
	{ nullptr }
};

static const z80_daisy_config rt1715w_daisy_chain[] =
{
	{ "sio0" },
	{ nullptr }
};

static void rt1715w_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void rt1715_state::rt1715(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 9.832_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rt1715_state::rt1715_mem);
	m_maincpu->set_addrmap(AS_IO, &rt1715_state::rt1715_io);
	m_maincpu->set_daisy_config(rt1715_daisy_chain);

	MCFG_MACHINE_START_OVERRIDE(rt1715_state, rt1715)
	MCFG_MACHINE_RESET_OVERRIDE(rt1715_state, rt1715)

	/* keyboard */
	z80_device &keyboard(Z80(config, "keyboard", 683000));
	keyboard.set_addrmap(AS_PROGRAM, &rt1715_state::k7658_mem);
	keyboard.set_addrmap(AS_IO, &rt1715_state::k7658_io);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update("i8275", FUNC(i8275_device::screen_update));
	m_screen->set_raw(13.824_MHz_XTAL, 864, 0, 624, 320, 0, 300); // ?

	GFXDECODE(config, "gfxdecode", "palette", gfx_rt1715);
	PALETTE(config, "palette", FUNC(rt1715_state::rt1715_palette), 3);

	I8275(config, m_crtc, 13.824_MHz_XTAL / 8);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(rt1715_state::crtc_display_pixels));
	m_crtc->set_screen(m_screen);

	Z80SIO(config, m_sio0, 9.832_MHz_XTAL / 4);
	m_sio0->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio0->out_txda_callback().set(m_printer, FUNC(rs232_port_device::write_txd));
	m_sio0->out_txdb_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_sio0->out_dtrb_callback().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_sio0->out_rtsb_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));

	Z80CTC(config, m_ctc0, 15.9744_MHz_XTAL / 4);
	m_ctc0->zc_callback<0>().set(m_sio0, FUNC(z80sio_device::txca_w));
	m_ctc0->zc_callback<2>().set(m_sio0, FUNC(z80sio_device::rxtxcb_w));

	// X4 connector -- printer
	RS232_PORT(config, m_printer, default_rs232_devices, "printer");
	m_printer->cts_handler().set(m_sio0, FUNC(z80sio_device::ctsa_w));

	// X5 connector -- V24 port
	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_sio0, FUNC(z80sio_device::rxb_w));
	m_rs232->cts_handler().set(m_sio0, FUNC(z80sio_device::ctsb_w));
	m_rs232->dsr_handler().set(m_sio0, FUNC(z80sio_device::syncb_w));

	/* floppy */
	Z80PIO(config, "a71", 9.832_MHz_XTAL / 4);
	Z80PIO(config, "a72", 9.832_MHz_XTAL / 4);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_default_value(0x00);
}

void rt1715_state::rt1715w(machine_config &config)
{
	rt1715(config);

	m_maincpu->set_clock(15.9744_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rt1715_state::rt1715w_mem);
	m_maincpu->set_addrmap(AS_IO, &rt1715_state::rt1715w_io);
	m_maincpu->set_daisy_config(rt1715w_daisy_chain);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	MCFG_MACHINE_START_OVERRIDE(rt1715_state, rt1715w)
	MCFG_MACHINE_RESET_OVERRIDE(rt1715_state, rt1715w)

	config.device_remove("a71");
	config.device_remove("a72");

	m_crtc->drq_wr_callback().set(FUNC(rt1715_state::crtc_drq_w));

	// operates in polled mode
	I8272A(config, m_fdc, 8'000'000 / 4, false);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w)).invert();
	FLOPPY_CONNECTOR(config, "i8272:0", rt1715w_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "i8272:1", rt1715w_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);

	Z80DMA(config, m_dma, 15.9744_MHz_XTAL / 4);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set(FUNC(rt1715_state::tc_w));
	m_dma->in_mreq_callback().set(FUNC(rt1715_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(rt1715_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(rt1715_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(rt1715_state::io_write_byte));

	Z80CTC(config, m_ctc2, 15.9744_MHz_XTAL / 4);

	m_ram->set_default_size("256K");
	RAM(config, m_videoram).set_default_size("4K").set_default_value(0x00);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( rt1715 )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s619.a25.2", 0x0000, 0x0800, CRC(98647763) SHA1(93fba51ed26392ec3eff1037886576fa12443fe5))
	ROM_LOAD("s602.a25.1", 0x0800, 0x0800, NO_DUMP) // CCITT fd67

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715lc )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s643.a25.2", 0x0000, 0x0800, CRC(ea37f0e6) SHA1(357760974d944b9782734504b9820771e7e37645))
	ROM_LOAD("s605.a25.1", 0x0800, 0x0800, CRC(38062024) SHA1(798f62d4adeb7098b7dcbfe6caf28302853ee97d))

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s642.ic8", 0x0000, 0x0800, NO_DUMP) // CCITT 962e

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715w )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s550.bin", 0x0000, 0x0800, CRC(0a96c754) SHA1(4d9ad5b877353d91ba355044d2847e1d621e2b01))

	// loaded from floppy on startup
	ROM_REGION(0x1000, "gfx", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0100, "prom", 0)
	ROM_LOAD("287.bin", 0x0000, 0x0100, CRC(8508360c) SHA1(d262a8c3cf2d284c67f23b853e0d59ae5cc1d4c8)) // /CAS decoder prom, 74S287
ROM_END

} // anonymous namespace


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY     FULLNAME                             FLAGS
COMP( 1986, rt1715,   0,      0,      rt1715,  k7658,   rt1715_state, empty_init, "Robotron", "Robotron PC-1715",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715lc, rt1715, 0,      rt1715,  k7658,   rt1715_state, empty_init, "Robotron", "Robotron PC-1715 (latin/cyrillic)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715w,  rt1715, 0,      rt1715w, rt1715w, rt1715_state, empty_init, "Robotron", "Robotron PC-1715W",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
