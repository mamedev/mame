// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************************

    The Aussie Byte II Single-Board Computer, created by SME Systems, Melbourne, Australia.
    Also known as the Knight 2000 Microcomputer.

    Status:
    Boots up from floppy.
    Output to serial terminal and to 6545 are working. Serial keyboard works.

    Video
    Graphics not working properly.
    Mode 0 - lores (wide, chunky)
    Mode 1 - external, *should* be ok
    Mode 2 - thin graphics, not explained well enough to code
    Mode 3 - alphanumeric, works

    Developed in conjunction with members of the MSPP. Written in July, 2015.

    TODO:
    - CRT8002 attributes controller
    - Graphics
    - Hard drive controllers and drives
    - Test Centronics printer
    - PIO connections

    Note of MAME restrictions:
    - Votrax doesn't sound anything like the real thing
    - WD1001/WD1002 device is not emulated
    - CRT8002 device is not emulated

**************************************************************************************************/

/***********************************************************

    Includes

************************************************************/
#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "sound/votrax.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/***********************************************************

    Class

************************************************************/
namespace {

class aussiebyte_state : public driver_device
{
public:
	aussiebyte_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_bankr0(*this, "bankr0")
		, m_bankw0(*this, "bankw0")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_p_chargen(*this, "chargen")
		, m_p_mram(*this, "mram", 0x40000, ENDIANNESS_LITTLE)
		, m_p_videoram(*this, "vram", 0x10000, ENDIANNESS_LITTLE)
		, m_p_attribram(*this, "aram", 0x800, ENDIANNESS_LITTLE)
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_centronics(*this, "centronics")
		, m_rs232(*this, "rs232")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_crtc(*this, "crtc")
		, m_speaker(*this, "speaker")
		, m_votrax(*this, "votrax")
		, m_rtc(*this, "rtc")
	{ }

	void aussiebyte(machine_config &config) ATTR_COLD;

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, u8 data);
	u8 io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, u8 data);
	void port15_w(u8 data);
	void port16_w(u8 data);
	void port17_w(u8 data);
	void port18_w(u8 data);
	u8 port19_r();
	void port1a_w(u8 data);
	void port1b_w(u8 data);
	void port1c_w(u8 data);
	void port20_w(u8 data);
	u8 port28_r();
	u8 port33_r();
	void port34_w(u8 data);
	void port35_w(u8 data);
	u8 port36_r();
	u8 port37_r();
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void sio1_rdya_w(int state);
	void sio1_rdyb_w(int state);
	void sio2_rdya_w(int state);
	void sio2_rdyb_w(int state);
	void address_w(u8 data);
	void register_w(u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 crt8002(u8 ac_ra, u8 ac_chr, u8 ac_attr, u16 ac_cnt, bool ac_curs);
	bool m_port15 = false; // rom switched in (0), out (1)
	u8 m_port17 = 0U;
	u8 m_port17_rdy = 0U;
	u8 m_port19 = 0U;
	u8 m_port1a = 0U; // bank to switch to when write to port 15 happens
	u8 m_port28 = 0U;
	u8 m_port34 = 0U;
	u8 m_port35 = 0U; // byte to be written to vram or aram
	u8 m_video_index = 0U;
	u16 m_cnt = 0U;
	u16 m_alpha_address = 0U;
	u16 m_graph_address = 0U;
	bool m_centronics_busy = false;
	std::unique_ptr<u8[]> m_vram; // video ram, 64k dynamic
	std::unique_ptr<u8[]> m_aram; // attribute ram, 2k static
	std::unique_ptr<u8[]> m_ram;  // main ram, 256k dynamic
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_memory_bank m_bankr0, m_bankw0, m_bank1, m_bank2;
	required_region_ptr<u8> m_p_chargen;
	memory_share_creator<u8> m_p_mram;
	memory_share_creator<u8> m_p_videoram;
	memory_share_creator<u8> m_p_attribram;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<votrax_sc01_device> m_votrax;
	required_device<msm5832_device> m_rtc;
};


/***********************************************************

    Address Maps

************************************************************/

void aussiebyte_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bankr0").bankw("bankw0");
	map(0x4000, 0x7fff).bankrw("bank1");
	map(0x8000, 0xbfff).bankrw("bank2");
	map(0xc000, 0xffff).ram();
}

void aussiebyte_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x04, 0x07).rw(m_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).noprw(); // winchester interface
	map(0x10, 0x13).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write));
	map(0x14, 0x14).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x15, 0x15).w(FUNC(aussiebyte_state::port15_w)); // boot rom disable
	map(0x16, 0x16).w(FUNC(aussiebyte_state::port16_w)); // fdd select
	map(0x17, 0x17).w(FUNC(aussiebyte_state::port17_w)); // DMA mux
	map(0x18, 0x18).w(FUNC(aussiebyte_state::port18_w)); // fdc select
	map(0x19, 0x19).r(FUNC(aussiebyte_state::port19_r)); // info port
	map(0x1a, 0x1a).w(FUNC(aussiebyte_state::port1a_w)); // membank
	map(0x1b, 0x1b).w(FUNC(aussiebyte_state::port1b_w)); // winchester control
	map(0x1c, 0x1f).w(FUNC(aussiebyte_state::port1c_w)); // gpebh select
	map(0x20, 0x23).rw(m_pio2, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x24, 0x27).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x28, 0x28).r(FUNC(aussiebyte_state::port28_r)).w(m_votrax, FUNC(votrax_sc01_device::write));
	map(0x2c, 0x2c).w(m_votrax, FUNC(votrax_sc01_device::inflection_w));
	map(0x30, 0x30).w(FUNC(aussiebyte_state::address_w));
	map(0x31, 0x31).r(m_crtc, FUNC(mc6845_device::status_r));
	map(0x32, 0x32).w(FUNC(aussiebyte_state::register_w));
	map(0x33, 0x33).r(FUNC(aussiebyte_state::port33_r));
	map(0x34, 0x34).w(FUNC(aussiebyte_state::port34_w)); // video control
	map(0x35, 0x35).w(FUNC(aussiebyte_state::port35_w)); // data to vram and aram
	map(0x36, 0x36).r(FUNC(aussiebyte_state::port36_r)); // data from vram and aram
	map(0x37, 0x37).r(FUNC(aussiebyte_state::port37_r)); // read dispen flag
	map(0x40, 0x4f).rw(FUNC(aussiebyte_state::rtc_r), FUNC(aussiebyte_state::rtc_w));
}

/***********************************************************

    Keyboard

************************************************************/
static INPUT_PORTS_START( aussiebyte )
INPUT_PORTS_END

/***********************************************************

    I/O Ports

************************************************************/
void aussiebyte_state::port15_w(u8 data)
{
	m_bankr0->set_entry(m_port15); // point at ram
	m_port15 = true;
}

/* FDD select
0 Drive Select bit O
1 Drive Select bit 1
2 Drive Select bit 2
3 Drive Select bit 3
  - These bits connect to a 74LS145 binary to BCD converter.
  - Drives 0 to 3 are 5.25 inch, 4 to 7 are 8 inch, 9 and 0 are not used.
  - Currently we only support drive 0.
4 Side Select to Disk Drives.
5 Disable 5.25 inch floppy spindle motors.
6 Unused.
7 Enable write precompensation on WD2797 controller. */
void aussiebyte_state::port16_w(u8 data)
{
	floppy_image_device *m_floppy = nullptr;
	if ((data & 15) == 0)
		m_floppy = m_floppy0->get_device();
	else if ((data & 15) == 1)
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(data, 5));
		m_floppy->ss_w(BIT(data, 4));
	}
}

/* DMA select
0 - FDC
1 - SIO Ch A
2 - SIO Ch B
3 - Winchester bus
4 - SIO Ch C
5 - SIO Ch D
6 - Ext ready 1
7 - Ext ready 2 */
void aussiebyte_state::port17_w(u8 data)
{
	m_port17 = data & 7;
	m_dma->rdy_w(BIT(m_port17_rdy, data));
}

/* FDC params
2 EXC: WD2797 clock frequency. H = 5.25"; L = 8"
3 WIEN: WD2797 Double density select. */
void aussiebyte_state::port18_w(u8 data)
{
	m_fdc->set_unscaled_clock(BIT(data, 2) ? 1e6 : 2e6);
	m_fdc->dden_w(BIT(data, 3));
}

u8 aussiebyte_state::port19_r()
{
	return m_port19;
}

// Memory banking
void aussiebyte_state::port1a_w(u8 data)
{
	data &= 7;
	switch (data)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			m_port1a = data*3+1;
			if (m_port15)
				m_bankr0->set_entry(data*3+1);
			m_bankw0->set_entry(data*3+1);
			m_bank1->set_entry(data*3+2);
			m_bank2->set_entry(data*3+3);
			break;
		case 5:
			m_port1a = 1;
			if (m_port15)
				m_bankr0->set_entry(1);
			m_bankw0->set_entry(1);
			m_bank1->set_entry(2);
			m_bank2->set_entry(13);
			break;
		case 6:
			m_port1a = 14;
			if (m_port15)
				m_bankr0->set_entry(14);
			m_bankw0->set_entry(14);
			m_bank1->set_entry(15);
			//m_bank2->set_entry(0); // open bus
			break;
		case 7:
			m_port1a = 1;
			if (m_port15)
				m_bankr0->set_entry(1);
			m_bankw0->set_entry(1);
			m_bank1->set_entry(4);
			m_bank2->set_entry(13);
			break;
	}
}

// Winchester control
void aussiebyte_state::port1b_w(u8 data)
{
}

// GPEHB control
void aussiebyte_state::port1c_w(u8 data)
{
}

void aussiebyte_state::port20_w(u8 data)
{
	m_speaker->level_w(BIT(data, 7));
	m_rtc->cs_w(BIT(data, 0));
	m_rtc->hold_w(BIT(data, 0));
}

u8 aussiebyte_state::port28_r()
{
	return m_port28;
}


// dummy read port, forces requested action to happen
u8 aussiebyte_state::port33_r()
{
	return 0xff;
}

/*
Video control - needs to be fully understood
d0, d1, d2, d3 - can replace RA0-3 in graphics mode
d4 - GS - unknown
d5 - /SRRD - controls write of data to either vram or aram (1=vram, 0=aram)
d6 - /VWR - 0 = enable write vdata to vram, read from aram to vdata ; 1 = enable write to aram from vdata
d7 - OE on port 35
*/
void aussiebyte_state::port34_w(u8 data)
{
	m_port34 = data;
}

void aussiebyte_state::port35_w(u8 data)
{
	m_port35 = data;
}

u8 aussiebyte_state::port36_r()
{
	if (BIT(m_port34, 5))
	{
		if (BIT(m_aram[m_alpha_address & 0x7ff], 7))
			return m_vram[m_alpha_address];
		else
			return m_vram[m_graph_address];
	}
	else
		return m_aram[m_alpha_address & 0x7ff];
}

u8 aussiebyte_state::port37_r()
{
	return m_crtc->de_r() ? 0xff : 0xfe;
}


/***********************************************************

    Video

************************************************************/
MC6845_ON_UPDATE_ADDR_CHANGED( aussiebyte_state::crtc_update_addr )
{
/* not sure what goes in here - parameters passed are device, address, strobe */
//  m_video_address = address;// & 0x7ff;
}

void aussiebyte_state::address_w(u8 data)
{
	m_crtc->address_w(data);

	m_video_index = data & 0x1f;

	if (m_video_index == 31)
	{
		m_alpha_address++;
		m_alpha_address &= 0x3fff;
		m_graph_address = (m_alpha_address << 4) | (m_port34 & 15);

		if (BIT(m_port34, 5))
		{
			if (BIT(m_aram[m_alpha_address & 0x7ff], 7))
				m_vram[m_alpha_address] = m_port35;
			else
				m_vram[m_graph_address] = m_port35;
		}
		else
			m_aram[m_alpha_address & 0x7ff] = m_port35;
	}
}

void aussiebyte_state::register_w(u8 data)
{
	m_crtc->register_w(data);
	u16 temp = m_alpha_address;

	// Get transparent address
	if (m_video_index == 18)
		m_alpha_address = (data << 8 ) | (temp & 0xff);
	else if (m_video_index == 19)
		m_alpha_address = data | (temp & 0xff00);
}

u8 aussiebyte_state::crt8002(u8 ac_ra, u8 ac_chr, u8 ac_attr, u16 ac_cnt, bool ac_curs)
{
	u8 gfx = 0;
	switch (ac_attr & 3)
	{
		case 0: // lores gfx
			switch (ac_ra)
			{
				case 0:
				case 1:
				case 2:
					gfx = (BIT(ac_chr, 7) ? 0xf8 : 0) | (BIT(ac_chr, 3) ? 7 : 0);
					break;
				case 3:
				case 4:
				case 5:
					gfx = (BIT(ac_chr, 6) ? 0xf8 : 0) | (BIT(ac_chr, 2) ? 7 : 0);
					break;
				case 6:
				case 7:
				case 8:
					gfx = (BIT(ac_chr, 5) ? 0xf8 : 0) | (BIT(ac_chr, 1) ? 7 : 0);
					break;
				default:
					gfx = (BIT(ac_chr, 4) ? 0xf8 : 0) | (BIT(ac_chr, 0) ? 7 : 0);
					break;
			}
			break;
		case 1: // external mode
			gfx = bitswap<8>(ac_chr, 0,1,2,3,4,5,6,7);
			break;
		case 2: // thin gfx
			break;
		case 3: // alpha
			gfx = m_p_chargen[((ac_chr & 0x7f)<<4) | ac_ra];
			break;
	}

	if (BIT(ac_attr, 3) & (ac_ra == 11)) // underline
		gfx = 0xff;
	if (BIT(ac_attr, 2) & ((ac_ra == 5) | (ac_ra == 6))) // strike-through
		gfx = 0xff;
	if (BIT(ac_attr, 6) & BIT(ac_cnt, 13)) // flash
		gfx = 0;
	if (BIT(ac_attr, 5)) // blank
		gfx = 0;
	if (ac_curs && BIT(ac_cnt, 14)) // cursor
		gfx ^= 0xff;
	if (BIT(ac_attr, 4)) // reverse video
		gfx ^= 0xff;
	return gfx;
}

MC6845_UPDATE_ROW( aussiebyte_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	ra &= 15;
	m_cnt++;

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = ma + x;
		u8 attr = m_aram[mem & 0x7ff];
		u8 chr;
		if (BIT(attr, 7))
			chr = m_vram[mem & 0x3fff]; // alpha
		else
			chr = m_vram[(mem << 4) | ra]; // gfx

		u8 gfx = crt8002(ra, chr, attr, m_cnt, (x==cursor_x));

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}


/***********************************************************

    RTC

************************************************************/
u8 aussiebyte_state::rtc_r(offs_t offset)
{
	m_rtc->read_w(1);
	m_rtc->address_w(offset);
	u8 data = m_rtc->data_r();
	m_rtc->read_w(0);
	return data;
}

void aussiebyte_state::rtc_w(offs_t offset, u8 data)
{
	m_rtc->address_w(offset);
	m_rtc->data_w(data);
	m_rtc->write_w(1);
	m_rtc->write_w(0);
}

/***********************************************************

    DMA

************************************************************/
u8 aussiebyte_state::memory_read_byte(offs_t offset)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void aussiebyte_state::memory_write_byte(offs_t offset, u8 data)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

u8 aussiebyte_state::io_read_byte(offs_t offset)
{
	address_space &io_space = m_maincpu->space(AS_IO);
	return io_space.read_byte(offset);
}

void aussiebyte_state::io_write_byte(offs_t offset, u8 data)
{
	address_space &io_space = m_maincpu->space(AS_IO);
	io_space.write_byte(offset, data);
}

/***********************************************************

    DMA selector

************************************************************/
void aussiebyte_state::sio1_rdya_w(int state)
{
	m_port17_rdy = (m_port17_rdy & 0xfd) | (u8)(state << 1);
	if (m_port17 == 1)
		m_dma->rdy_w(state);
}

void aussiebyte_state::sio1_rdyb_w(int state)
{
	m_port17_rdy = (m_port17_rdy & 0xfb) | (u8)(state << 2);
	if (m_port17 == 2)
		m_dma->rdy_w(state);
}

void aussiebyte_state::sio2_rdya_w(int state)
{
	m_port17_rdy = (m_port17_rdy & 0xef) | (u8)(state << 4);
	if (m_port17 == 4)
		m_dma->rdy_w(state);
}

void aussiebyte_state::sio2_rdyb_w(int state)
{
	m_port17_rdy = (m_port17_rdy & 0xdf) | (u8)(state << 5);
	if (m_port17 == 5)
		m_dma->rdy_w(state);
}


/***********************************************************

    Video

************************************************************/

/* F4 Character Displayer */
static const gfx_layout crt8002_charlayout =
{
	8, 12,                   /* 7 x 11 characters */
	128,                  /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_crt8002 )
	GFXDECODE_ENTRY( "chargen", 0x0000, crt8002_charlayout, 0, 1 )
GFXDECODE_END

/***************************************************************

    Daisy Chain

****************************************************************/

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio2" },
	{ "sio1" },
	{ "sio2" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};


/***********************************************************

    Floppy Disk

************************************************************/

void aussiebyte_state::fdc_intrq_w(int state)
{
	u8 data = (m_port19 & 0xbf) | (state ? 0x40 : 0);
	m_port19 = data;
}

void aussiebyte_state::fdc_drq_w(int state)
{
	u8 data = (m_port19 & 0x7f) | (state ? 0x80 : 0);
	m_port19 = data;
	state ^= 1; // inverter on pin38 of fdc
	m_port17_rdy = (m_port17_rdy & 0xfe) | (u8)state;
	if (m_port17 == 0)
		m_dma->rdy_w(state);
}

static void aussiebyte_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(aussiebyte_state::quickload_cb)
{
	// RAM must be banked in
	m_port15 = true; // disable boot rom
	m_port1a = 4;
	m_bankr0->set_entry(m_port1a); // enable correct program bank
	m_bankw0->set_entry(m_port1a);

	address_space &prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	u16 quickload_size = image.length();
	for (u16 i = 0; i < quickload_size; i++)
	{
		u8 data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}

/***********************************************************

    Machine Driver

************************************************************/
void aussiebyte_state::machine_reset()
{
	m_port15 = false;
	m_port17 = 0;
	m_port17_rdy = 0;
	m_port1a = 1;
	m_alpha_address = 0;
	m_graph_address = 0;
	m_bankr0->set_entry(16); // point at rom
	m_bankw0->set_entry(1); // always write to ram
	m_bank1->set_entry(2);
	m_bank2->set_entry(3);
	m_maincpu->reset();
}

void aussiebyte_state::machine_start()
{
	m_vram = std::make_unique<u8[]>(0x10000);
	m_aram = std::make_unique<u8[]>(0x800);
	m_ram = make_unique_clear<u8[]>(0x40000);
	save_pointer(NAME(m_vram), 0x10000);
	save_pointer(NAME(m_aram), 0x800);
	save_pointer(NAME(m_ram),  0x40000);
	save_item(NAME(m_port15));
	save_item(NAME(m_port17));
	save_item(NAME(m_port17_rdy));
	save_item(NAME(m_port19));
	save_item(NAME(m_port1a));
	save_item(NAME(m_port28));
	save_item(NAME(m_port34));
	save_item(NAME(m_port35));
	save_item(NAME(m_video_index));
	save_item(NAME(m_cnt));
	save_item(NAME(m_alpha_address));
	save_item(NAME(m_graph_address));
	save_item(NAME(m_centronics_busy));

	// Main ram is divided into 16k blocks (0-15). The boot rom is block number 16.
	// For convenience, bank 0 is permanently assigned to C000-FFFF

	m_bankr0->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bankw0->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bank1->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bank2->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bankr0->configure_entry(16, memregion("roms")->base());

	m_cnt = 0;
}


void aussiebyte_state::aussiebyte(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &aussiebyte_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aussiebyte_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 952, 0, 640, 336, 0, 288);
	screen.set_screen_update("crtc", FUNC(sy6545_1_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_crt8002);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	VOTRAX_SC01A(config, m_votrax, 720000); // 720kHz? needs verify
	m_votrax->ar_callback().set([this] (bool state) { m_port28 = state ? 0 : 1; });
	m_votrax->add_route(ALL_OUTPUTS, "mono", 1.00);

	/* devices */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->busy_handler().set([this] (bool state) { m_centronics_busy = state; });
	INPUT_BUFFER(config, "cent_data_in");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(4.9152_MHz_XTAL / 4);
	m_ctc->set_clk<1>(4.9152_MHz_XTAL / 4);
	m_ctc->set_clk<2>(4.9152_MHz_XTAL / 4);
	m_ctc->zc_callback<0>().set("sio1", FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append("sio1", FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set("sio1", FUNC(z80sio_device::rxtxcb_w));
	m_ctc->zc_callback<1>().append("sio2", FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<1>().append("sio2", FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set("ctc", FUNC(z80ctc_device::trg3));
	m_ctc->zc_callback<2>().append("sio2", FUNC(z80sio_device::rxtxcb_w));

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSREQ);
	// BAO, not used
	m_dma->in_mreq_callback().set(FUNC(aussiebyte_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(aussiebyte_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(aussiebyte_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(aussiebyte_state::io_write_byte));

	Z80PIO(config, m_pio1, 16_MHz_XTAL / 4);
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio1->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio1->in_pb_callback().set("cent_data_in", FUNC(input_buffer_device::read));
	m_pio1->out_ardy_callback().set(m_centronics, FUNC(centronics_device::write_strobe)).invert();

	Z80PIO(config, m_pio2, 16_MHz_XTAL / 4);
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio2->out_pa_callback().set(FUNC(aussiebyte_state::port20_w));

	z80sio_device& sio1(Z80SIO(config, "sio1", 16_MHz_XTAL / 4));
	sio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio1.out_wrdya_callback().set(FUNC(aussiebyte_state::sio1_rdya_w));
	sio1.out_wrdyb_callback().set(FUNC(aussiebyte_state::sio1_rdyb_w));

	z80sio_device& sio2(Z80SIO(config, "sio2", 16_MHz_XTAL / 4));
	sio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio2.out_wrdya_callback().set(FUNC(aussiebyte_state::sio2_rdya_w));
	sio2.out_wrdyb_callback().set(FUNC(aussiebyte_state::sio2_rdyb_w));
	sio2.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard");
	m_rs232->rxd_handler().set("sio2", FUNC(z80sio_device::rxa_w));

	WD2797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(aussiebyte_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(aussiebyte_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", aussiebyte_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", aussiebyte_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* devices */
	SY6545_1(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(aussiebyte_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(aussiebyte_state::crtc_update_addr));

	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	/* quickload */
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(aussiebyte_state::quickload_cb));

	SOFTWARE_LIST(config, "flop_list").set_original("aussiebyte");
}


/***********************************************************

    Game driver

************************************************************/


ROM_START(aussieby)
	ROM_REGION(0x4000, "roms", 0) // Size of bank 16
	ROM_LOAD( "knight_boot_0000.u27", 0x0000, 0x1000, CRC(1f200437) SHA1(80d1d208088b325c16a6824e2da605fb2b00c2ce) )

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD( "8002.bin", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690) )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY        FULLNAME          FLAGS
COMP( 1984, aussieby, 0,      0,      aussiebyte, aussiebyte, aussiebyte_state, empty_init, "SME Systems", "Aussie Byte II", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
