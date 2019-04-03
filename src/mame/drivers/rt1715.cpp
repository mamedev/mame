// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert, Sergey Svishchev
/***************************************************************************

        Robotron PC-1715

        10/06/2008 Preliminary driver.

    Notes:
    - keyboard connected to sio channel a
    - sio channel a clock output connected to ctc trigger 0
    - rt1715w: SCP3 boot crashes in z80dma (Unknown base register XX)

    Docs:
    - http://www.robotrontechnik.de/html/computer/pc1715w.htm
    - https://www.tiffe.de/Robotron/PC1715/ -- scanned PDFs
      https://www.tiffe.de/Robotron/PC1715/MANUAL_PC_1715.pdf
    - http://www.sax.de/~zander/pc1715/pc_bw.html -- typeset PDFs
      http://www.sax.de/~zander/pc1715/doku/pc_manu.pdf
    - http://xepb.org/robotron/

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"


class rt1715_state : public driver_device
{
public:
	rt1715_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_bankdev(*this, "bankdev%u", 0U)
		, m_sio0(*this, "sio0")
		, m_ctc0(*this, "ctc0")
		, m_fdc(*this, "i8272")
		, m_floppy(*this, "i8272:%u", 0U)
		, m_dma(*this, "z80dma")
		, m_ctc2(*this, "ctc2")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_crtc(*this, "i8275")
		, m_p_chargen(*this, "gfx")
		, m_p_videoram(*this, "videoram")
	{
	}

	void rt1715(machine_config &config);
	void rt1715w(machine_config &config);

private:
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_WRITE8_MEMBER(rt1715_floppy_enable);
	DECLARE_READ8_MEMBER(k7658_led1_r);
	DECLARE_READ8_MEMBER(k7658_led2_r);
	DECLARE_READ8_MEMBER(k7658_data_r);
	DECLARE_WRITE8_MEMBER(k7658_data_w);
	DECLARE_WRITE8_MEMBER(rt1715_rom_disable);
	DECLARE_WRITE8_MEMBER(rt1715w_set_bank);
	DECLARE_WRITE8_MEMBER(rt1715w_floppy_motor);
	DECLARE_WRITE8_MEMBER(rt1715w_krfd_w);
	void rt1715_palette(palette_device &palette) const;
	I8275_DRAW_CHARACTER_MEMBER(crtc_display_pixels);
	DECLARE_WRITE_LINE_MEMBER(crtc_drq_w);

	void k7658_io(address_map &map);
	void k7658_mem(address_map &map);
	void rt1715_base_io(address_map &map);
	void rt1715_io(address_map &map);
	void rt1715w_io(address_map &map);
	void rt1715_mem(address_map &map);
	void rt1715w_mem(address_map &map);
	void rt1715w_banked_mem(address_map &map);

	DECLARE_MACHINE_START(rt1715);
	DECLARE_MACHINE_RESET(rt1715);
	DECLARE_MACHINE_START(rt1715w);
	DECLARE_MACHINE_RESET(rt1715w);

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	optional_device_array<address_map_bank_device, 2> m_bankdev;
	required_device<z80sio_device> m_sio0;
	required_device<z80ctc_device> m_ctc0;
	optional_device<i8272a_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppy;
	optional_device<z80dma_device> m_dma;
	optional_device<z80ctc_device> m_ctc2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc;
	required_region_ptr<uint8_t> m_p_chargen;
	optional_shared_ptr<uint8_t> m_p_videoram;

	int m_led1_val;
	int m_led2_val;
	u8 m_krfd;
	uint16_t m_dma_adr;
};


/***************************************************************************
    FLOPPY
***************************************************************************/

WRITE8_MEMBER(rt1715_state::rt1715_floppy_enable)
{
	logerror("%s: rt1715_floppy_enable %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(rt1715_state::rt1715w_floppy_motor)
{
	logerror("%s: rt1715w_floppy_motor %02x\n", machine().describe_context(), data);

	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(data & 0x80 ? 1 : 0);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(data & 0x08 ? 1 : 0);
}

WRITE8_MEMBER(rt1715_state::rt1715w_krfd_w)
{
	logerror("%s: rt1715w_krfd_w %02x\n", machine().describe_context(), data);
	m_krfd = data;
}

WRITE_LINE_MEMBER(rt1715_state::tc_w)
{
	m_fdc->tc_w(state & BIT(m_krfd, 7));
}

/***************************************************************************
    KEYBOARD
***************************************************************************/

/* si/so led */
READ8_MEMBER(rt1715_state::k7658_led1_r)
{
	m_led1_val ^= 1;
	logerror("%s: k7658_led1_r %02x\n", machine().describe_context(), m_led1_val);
	return 0xff;
}

/* caps led */
READ8_MEMBER(rt1715_state::k7658_led2_r)
{
	m_led2_val ^= 1;
	logerror("%s: k7658_led2_r %02x\n", machine().describe_context(), m_led2_val);
	return 0xff;
}

/* read key state */
READ8_MEMBER(rt1715_state::k7658_data_r)
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
WRITE8_MEMBER(rt1715_state::k7658_data_w)
{
	logerror("%s: k7658_data_w %02x\n", machine().describe_context(), BIT(data, 0));
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

WRITE8_MEMBER(rt1715_state::rt1715_rom_disable)
{
	logerror("%s: rt1715_set_bank %02x\n", machine().describe_context(), data);

	/* disable ROM, enable RAM */
	membank("bank1")->set_base(m_ram->pointer());
}

MACHINE_START_MEMBER(rt1715_state, rt1715w)
{
	membank("bank2")->set_base(m_ram->pointer() + 0x4000);
	membank("bank3")->set_base(m_ram->pointer());
}

MACHINE_RESET_MEMBER(rt1715_state, rt1715w)
{
	m_bankdev[0]->set_bank(0);
	m_bankdev[1]->set_bank(0);

	m_dma->rdy_w(1);
	m_krfd = 0;
	m_dma_adr = 0;
}

/*
   b2..0 = AB18..16

   0 - Hintergrundbank (Bildschirm, Zeichengeneratoren)
   1 - Systembank (gebanktes BIOS, BDOS)
   2 - Anwenderbank (TPA)
   3 - RAM-Disk
   4 - RAM-Disk
   5 - RAM-Disk
*/
WRITE8_MEMBER(rt1715_state::rt1715w_set_bank)
{
	int r = data >> 4;
	int w = data & 15;

	logerror("%s: rt1715w_set_bank target %x source %x%s\n", machine().describe_context(), r, w, r == w ? "" : " DIFF");

	m_bankdev[0]->set_bank(r);
	m_bankdev[1]->set_bank(w);
}

READ8_MEMBER(rt1715_state::memory_read_byte)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(rt1715_state::memory_write_byte)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

READ8_MEMBER(rt1715_state::io_read_byte)
{
	address_space &prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(rt1715_state::io_write_byte)
{
	address_space &prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

WRITE_LINE_MEMBER(rt1715_state::busreq_w)
{
	// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

WRITE_LINE_MEMBER(rt1715_state::crtc_drq_w)
{
	if (state)
	{
		address_space &mem = m_maincpu->space(AS_PROGRAM);
		m_crtc->dack_w(mem, 0, m_p_videoram[m_dma_adr++]);
		m_dma_adr %= (80 * 24);
	}
}

I8275_DRAW_CHARACTER_MEMBER(rt1715_state::crtc_display_pixels)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 gfx = (lten) ? 0xff : 0;

	if (!vsp)
		gfx = m_p_chargen[linecount << 7 | charcode];

	if (rvv)
		gfx ^= 0xff;

	for (u8 i=0; i<8; i++)
		bitmap.pix32(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
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
	map(0x0c, 0x0f).rw(m_sio0, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
//  map(0x10, 0x13).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
//  map(0x14, 0x17).rw(m_sio1, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
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
	map(0x0000, 0xffff).r(m_bankdev[0], FUNC(address_map_bank_device::read8)).w(m_bankdev[1], FUNC(address_map_bank_device::write8));
}

void rt1715_state::rt1715w_banked_mem(address_map &map)
{
	map(0x00000, 0x007ff).rom().region("ipl", 0);
	map(0x02000, 0x02fff).ram().region("gfx", 0);
	map(0x03000, 0x03fff).ram().share("videoram");
	map(0x04000, 0x0ffff).bankrw("bank2");
	map(0x10000, 0x4ffff).bankrw("bank3");
}

// rt1715w -- decoders A13, A14, page C
void rt1715_state::rt1715w_io(address_map &map)
{
	rt1715_base_io(map);

	map(0x00, 0x00).rw(m_dma, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w)); // A2
	map(0x04, 0x07).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // A4
//  map(0x1a, 0x1b) // chargen write protection
	map(0x1c, 0x1d).m(m_fdc, FUNC(i8272a_device::map));
	map(0x20, 0x23).w(FUNC(rt1715_state::rt1715w_krfd_w)); // KRFD -- FD-Steuerregister (A45)
	map(0x24, 0x27).w(FUNC(rt1715_state::rt1715w_set_bank)); // BR (A62, A63)
	map(0x28, 0x2b).w(FUNC(rt1715_state::rt1715w_floppy_motor)); // MOS
	map(0x34, 0x37).portr("S8"); // KON -- Konfigurations-schalter FD (config switch -- A114, DIP S8)
//  map(0x38, 0x3b) // SR (RST1) -- Ru:cksetzen von Flip-Flops im FD
//  map(0x3c, 0x3f) // RST (RST2) -- Ru:cksetzen von Flip-Flops in V.24 (Pru:ftechnik)
	// used via DMA only
	map(0x40, 0x40).r(m_fdc, FUNC(i8272a_device::msr_r));
	map(0x41, 0x41).rw(m_fdc, FUNC(i8272a_device::dma_r), FUNC(i8272a_device::dma_w));
}

void rt1715_state::k7658_mem(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(rt1715_state::k7658_data_w));
	map(0x0000, 0x07ff).mirror(0xf800).rom();
}

void rt1715_state::k7658_io(address_map &map)
{
	map(0x2000, 0x2000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led1_r));
	map(0x4000, 0x4000).mirror(0x8000).r(FUNC(rt1715_state::k7658_led2_r));
	map(0x8000, 0x9fff).r(FUNC(rt1715_state::k7658_data_r));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( rt1715w )
	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "UNK0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( k7658 )
	PORT_START("row_00")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_20")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_30")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_40")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_50")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_60")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_70")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_08")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_18")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_28")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_38")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_48")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
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
	{ "ctc0" },
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

#if 0
	/* keyboard */
	z80_device &keyboard(Z80(config, "keyboard", 683000));
	keyboard.set_addrmap(AS_PROGRAM, &rt1715_state::k7658_mem);
	keyboard.set_addrmap(AS_IO, &rt1715_state::k7658_io);
#endif

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update("i8275", FUNC(i8275_device::screen_update));
	m_screen->set_raw(13.824_MHz_XTAL, 864, 0, 624, 320, 0, 300); // ?

	GFXDECODE(config, "gfxdecode", "palette", gfx_rt1715);
	PALETTE(config, "palette", FUNC(rt1715_state::rt1715_palette), 3);

	I8275(config, m_crtc, 13.824_MHz_XTAL / 8);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(rt1715_state::crtc_display_pixels), this);
	m_crtc->set_screen(m_screen);

	/* keyboard */
	Z80SIO(config, m_sio0, 9.832_MHz_XTAL / 4);

	Z80CTC(config, m_ctc0, 9.832_MHz_XTAL / 4);
	m_ctc0->zc_callback<0>().set(m_sio0, FUNC(z80sio_device::txca_w));
	m_ctc0->zc_callback<2>().set(m_sio0, FUNC(z80sio_device::rxtxcb_w));

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

	ADDRESS_MAP_BANK(config, "bankdev0").set_map(&rt1715_state::rt1715w_banked_mem).set_options(ENDIANNESS_BIG, 8, 19, 0x10000);
	ADDRESS_MAP_BANK(config, "bankdev1").set_map(&rt1715_state::rt1715w_banked_mem).set_options(ENDIANNESS_BIG, 8, 19, 0x10000);

	MCFG_MACHINE_START_OVERRIDE(rt1715_state, rt1715w)
	MCFG_MACHINE_RESET_OVERRIDE(rt1715_state, rt1715w)

	config.device_remove("a71");
	config.device_remove("a72");

	m_crtc->drq_wr_callback().set(FUNC(rt1715_state::crtc_drq_w));

	// operates in polled mode
	I8272A(config, m_fdc, 8'000'000 / 4, false);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w)).invert();
	FLOPPY_CONNECTOR(config, "i8272:0", rt1715w_floppies, "525qd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "i8272:1", rt1715w_floppies, "525qd", floppy_image_device::default_floppy_formats);

	Z80DMA(config, m_dma, 15.9744_MHz_XTAL / 4);
	m_dma->out_busreq_callback().set(FUNC(rt1715_state::busreq_w));
	m_dma->out_int_callback().set(FUNC(rt1715_state::tc_w));
	m_dma->in_mreq_callback().set(FUNC(rt1715_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(rt1715_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(rt1715_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(rt1715_state::io_write_byte));

	Z80CTC(config, m_ctc2, 15.9744_MHz_XTAL / 4);

	m_ram->set_default_size("256K");
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


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY     FULLNAME                             FLAGS
COMP( 1986, rt1715,   0,      0,      rt1715,  k7658, rt1715_state, empty_init, "Robotron", "Robotron PC-1715",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715lc, rt1715, 0,      rt1715,  k7658, rt1715_state, empty_init, "Robotron", "Robotron PC-1715 (latin/cyrillic)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, rt1715w,  rt1715, 0,      rt1715w, rt1715w, rt1715_state, empty_init, "Robotron", "Robotron PC-1715W",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
