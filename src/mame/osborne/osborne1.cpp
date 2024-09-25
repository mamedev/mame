// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Vas Crabb
/***************************************************************************

    Osborne-1 driver file

The Osborne-1 memory is divided into 3 "banks".

Bank 1 simply consists of 64KB of RAM. The upper 4KB is used for the lower 8
bit of video RAM entries.

Bank 2 holds the BIOS ROM and I/O area. Only addresses 0000-3FFF are used
by bank 2 (4000-FFFF mirrors bank 1). Bank 2 is divided as follows:
3000-3FFF Nominally unused but acts as mirror of 2000-2FFF
2C00-2C03 Video PIA
2A00-2A01 Serial interface
2900-2903 488 PIA
2400-2400 SCREEN-PAC (if present)
2201-2280 Keyboard
2100-2103 Floppy
1000-1FFF Nominally unused but acts as read mirror of BIOS ROM
0000-0FFF BIOS ROM

The logic is actually quite sloppy, and will cause bus fighting under many
circumstances since it doesn't actually check all four bits, just that two
are in the desired state.

Bank 3 has the ninth bit needed to complete the full Video RAM. These bits
are stored at F000-FFFF. Only the highest bit is used.

On bootup bank 2 is active.

Banking is controlled by writes to I/O space.  Only two low address bits are
used, and the value on the data bus is completley ignored.
00 - Activate bank 2 (also triggered by CPU reset)
01 - Activate bank 1
02 - Set BIT 9 signal (map bank 3 into F000-FFFF)
03 - Clear BIT 9 signal (map bank 1/2 into F000-FFFF)

Selecting between bank 1 and bank 2 is also affected by M1 and IRQACK
conditions using a set of three flipflops.

There are three IRQ sources:
- IRQ0 = IRQ from the serial ACIA
- IRQ1 = IRQA from the video PIA
- IRQ2 = IRQA from the IEEE488 PIA

The serial speed configuration implements wiring changes recommended in the
Osborne 1 Technical Manual.  There's no way for software to read the
selected baud rates, so it will always call the low speed "300" and the high
speed "1200".  You as the user have to keep this in mind using the system.

Serial communications can be flaky when 600/2400 is selected.  This is not a
bug in MAME.  I've checked and double-checked the schematics to confirm it's
an original bug.  The division ratio from the master clock to the baud rates
in this mode is effectively 16*24*64 or 16*24*16 giving actual data rates of
650 baud or 2600 baud, about 8.3% too fast (16*26*64 and 16*26*16 would give
the correct rates).  MAME's bitbanger seems to be able to accept the ACIA
output at this rate, but the ACIA screws up when consuming data from MAME's
bitbanger.

Schematics specify a WD1793 floppy controller, but we're using the Fujitsu
equivalent MB8877 here.  Is it known that the original machines used one or
the other exclusively?  In any case MAME emulates them identically.

Installation of the SCREEN-PAC requires the CPU and character generator ROM
to be transplanted to the add-on board, and cables run to the sockets that
previously held these chips.  It contains additional RAM clocked at twice
the speed of the main system RAM.  Writes to video memory get sent to this
RAM as well as the main system RAM, so there are actually two live copies
of video RAM at all times.  The SCREEN-PAC supports switching between
normal and double horizontal resolution (52x24 or 104x24) at exactly 60Hz.

The Nuevo Video board also requires the CPU to be transplanted to it and has
a pair of RAMs holding a copy of video memory.  However it has its own
character generator ROM, so the mainboard's character generator ROM doesn't
need to be moved.  It doesn't behave like the SCREEN-PAC.  It uses a
Synertek SY6545-1 with its pixel clock derived from a 12.288MHz crystal
mapped at 0x04/0x05 in I/O space.  It runs at 640x240 (80x24) at just below
60Hz and doesn't allow resolution switching.  We don't know how contention
for video RAM is handled, or whether the CRTC can generate VBL interrupts.


TODO:

* Hook up the port direction control bits in the IEEE488 interface properly
  and test it with some emulated peripheral.  Also the BIOS can speak
  Centronics parallel over the same physical interface, so this should be
  tested, too.

* Complete emulation of the Nuevo Video board (interrupts, CRTC video RAM
  updates).  It would be nice to get a schematic for this.

***************************************************************************/

#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"

#include "cpu/z80/z80.h"

#include "imagedev/floppy.h"

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/wd_fdc.h"

#include "sound/spkrdev.h"

#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


namespace {

static constexpr XTAL MAIN_CLOCK = 15.9744_MHz_XTAL;


class osborne1_state : public driver_device
{
public:
	osborne1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_rom_view(*this, "bank_0000"),
		m_main_ram(*this, "mainram"),
		m_attr_ram(*this, "attrram"),
		m_screen(*this, "screen"),
		m_maincpu(*this, "maincpu"),
		m_pia0(*this, "pia_0"),
		m_pia1(*this, "pia_1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_speaker(*this, "speaker"),
		m_acia(*this, "acia"),
		m_fdc(*this, "mb8877"),
		m_ieee(*this, IEEE488_TAG),
		m_floppy(*this, "mb8877:%u", 0U),
		m_keyb_row(*this, { "ROW0", "ROW1", "ROW3", "ROW4", "ROW5", "ROW2", "ROW6", "ROW7" }),
		m_btn_reset(*this, "RESET"),
		m_cnf(*this, "CNF"),
		m_vram_view(*this, "bank_fxxx"),
		m_p_chargen(*this, "chargen"),
		m_video_timer(nullptr),
		m_tilemap(nullptr),
		m_acia_rxc_txc_timer(nullptr)
	{
	}

	void osborne1(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_key);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void osborne1_base(machine_config &config);

	void osborne1_mem(address_map &map) ATTR_COLD;
	void osborne1_op(address_map &map) ATTR_COLD;
	void osborne1_io(address_map &map) ATTR_COLD;

	u8 bank2_peripherals_r(offs_t offset);
	void bank2_peripherals_w(offs_t offset, u8 data);
	void videoram_w(offs_t offset, u8 data);
	void attrram_w(offs_t offset, u8 data);
	u8 opcode_r(offs_t offset);
	void bankswitch_w(offs_t offset, u8 data);
	void irqack_w(int state);

	bool rom_mode() const { return 0 != m_rom_mode; }
	u8 scroll_x() const { return m_scroll_x; }

	template <int Width, unsigned Scale> void draw_rows(uint16_t col, bitmap_ind16 &bitmap, const rectangle &cliprect);

	memory_view                             m_rom_view;
	required_shared_ptr<u8>                 m_main_ram;
	required_shared_ptr<u8>                 m_attr_ram;
	required_device<screen_device>          m_screen;
	required_device<z80_device>             m_maincpu;
	required_device<pia6821_device>         m_pia0;
	required_device<pia6821_device>         m_pia1;

private:
	u8 ieee_pia_pb_r();
	void ieee_pia_pb_w(u8 data);
	void ieee_pia_irq_a_func(int state);

	void video_pia_port_a_w(u8 data);
	void video_pia_port_b_w(u8 data);
	void video_pia_out_cb2_dummy(int state);
	void video_pia_irq_a_func(int state);

	void serial_acia_irq_func(int state);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(video_callback);
	TIMER_CALLBACK_MEMBER(acia_rxc_txc_callback);

	TILE_GET_INFO_MEMBER(get_tile_info);

	bool set_rom_mode(u8 value);
	void update_irq();
	void update_acia_rxc_txc();

	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<speaker_sound_device>   m_speaker;
	required_device<acia6850_device>        m_acia;
	required_device<mb8877_device>          m_fdc;
	required_device<ieee488_device>         m_ieee;
	required_device_array<floppy_connector, 2> m_floppy;

	// user inputs
	required_ioport_array<8>    m_keyb_row;
	required_ioport             m_btn_reset;

	// fake inputs for hardware configuration and things that need rewiring
	required_ioport             m_cnf;

	// pieces of memory
	memory_view                 m_vram_view;
	required_region_ptr<u8>     m_p_chargen;

	// configuration (reloaded on reset)
	u8              m_acia_rxc_txc_div;
	u8              m_acia_rxc_txc_p_low;
	u8              m_acia_rxc_txc_p_high;

	// bank switch control bits
	u8              m_ub4a_q;
	u8              m_ub6a_q;
	u8              m_rom_mode;

	// onboard video state
	u8              m_scroll_x;
	u8              m_scroll_y;
	u8              m_beep_state;
	emu_timer       *m_video_timer;
	tilemap_t       *m_tilemap;

	// serial state
	u8              m_acia_irq_state;
	u8              m_acia_rxc_txc_state;
	emu_timer       *m_acia_rxc_txc_timer;

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_mem_cache;
};


class osborne1sp_state : public osborne1_state
{
public:
	osborne1sp_state(const machine_config &mconfig, device_type type, const char *tag) :
		osborne1_state(mconfig, type, tag)
	{
	}

	void osborne1sp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void osborne1sp_mem(address_map &map) ATTR_COLD;

	u8 bank2_peripherals_r(offs_t offset);
	void bank2_peripherals_w(offs_t offset, u8 data);

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// SCREEN-PAC registers
	u8              m_resolution = 0;
	u8              m_hc_left = 0;
};


class osborne1nv_state : public osborne1_state
{
public:
	osborne1nv_state(const machine_config &mconfig, device_type type, const char *tag) :
		osborne1_state(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_p_nuevo(*this, "nuevo")
	{
	}

	void osborne1nv(machine_config &config);

private:
	void osborne1nv_io(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr_changed);

	required_device<palette_device> m_palette;
	required_region_ptr<u8>         m_p_nuevo;
};


u8 osborne1_state::bank2_peripherals_r(offs_t offset)
{
	// Since each peripheral only checks two bits, many addresses will
	// result in multiple peripherals attempting to drive the bus.  This is
	// simulated by ANDing all the values together.
	u8 data = 0xff;
	if ((offset & 0x900) == 0x100) // Floppy
		data &= m_fdc->read(offset & 0x03);
	if ((offset & 0x900) == 0x900) // IEEE488 PIA
		data &= m_pia0->read(offset & 0x03);
	if ((offset & 0xa00) == 0x200) // Keyboard
	{
		for (unsigned b = 0; 8 > b; ++b)
		{
			if (BIT(offset, b))
				data &= m_keyb_row[b]->read();
		}
	}
	if ((offset & 0xa00) == 0xa00) // Serial
	{
		data &= m_acia->read(offset & 0x01);
	}
	if ((offset & 0xc00) == 0xc00) // Video PIA
		data &= m_pia1->read(offset & 0x03);
	return data;
}

void osborne1_state::bank2_peripherals_w(offs_t offset, u8 data)
{
	// Handle writes to the I/O area
	if ((offset & 0x900) == 0x100) // Floppy
		m_fdc->write(offset & 0x03, data);
	if ((offset & 0x900) == 0x900) // IEEE488 PIA
		m_pia0->write(offset & 0x03, data);
	if ((offset & 0xa00) == 0xa00) // Serial
		m_acia->write(offset & 0x01, data);
	if ((offset & 0xc00) == 0xc00) // Video PIA
		m_pia1->write(offset & 0x03, data);
}

u8 osborne1sp_state::bank2_peripherals_r(offs_t offset)
{
	u8 data = osborne1_state::bank2_peripherals_r(offset);

	if ((offset & 0xc00) == 0x400) // SCREEN-PAC
		data &= 0xfb;

	return data;
}

void osborne1sp_state::bank2_peripherals_w(offs_t offset, u8 data)
{
	osborne1_state::bank2_peripherals_w(offset, data);

	if ((offset & 0xc00) == 0x400) // SCREEN-PAC
	{
		m_resolution = BIT(data, 0);
		m_hc_left = BIT(~data, 1);
	}
}

void osborne1_state::videoram_w(offs_t offset, u8 data)
{
	m_main_ram[0xf000 | offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void osborne1_state::attrram_w(offs_t offset, u8 data)
{
	// Attribute RAM is only one bit wide - low seven bits are discarded and read back high
	m_attr_ram[offset] = data | 0x7f;
}

u8 osborne1_state::opcode_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		// Update the flipflops that control bank selection and NMI
		u8 const new_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
		if (!rom_mode())
		{
			set_rom_mode(m_ub4a_q ? 0 : 1);
			m_ub4a_q = m_ub6a_q;
		}
		m_ub6a_q = new_ub6a_q;
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);
	}

	// Now that's sorted out we can call the normal read handler
	return m_mem_cache.read_byte(offset);
}

void osborne1_state::bankswitch_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x00:
		if (set_rom_mode(1))
			m_ub4a_q = m_ub6a_q;
		break;
	case 0x01:
		m_ub4a_q = 1;
		m_ub6a_q = 1;
		set_rom_mode(0);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;
	case 0x02:
		m_vram_view.select(0);
		break;
	case 0x03:
		m_vram_view.disable();
		break;
	}
}

void osborne1_state::irqack_w(int state)
{
	// Update the flipflops that control bank selection and NMI
	if (!rom_mode())
		set_rom_mode(m_ub4a_q ? 0 : 1);
	m_ub4a_q = 0;
	m_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);
}


u8 osborne1_state::ieee_pia_pb_r()
{
	/*
	    bit     description

	    0
	    1
	    2
	    3       EOI
	    4
	    5       DAV
	    6       NDAC
	    7       NRFD
	*/
	u8 data = 0;

	data |= m_ieee->eoi_r() << 3;
	data |= m_ieee->dav_r() << 5;
	data |= m_ieee->ndac_r() << 6;
	data |= m_ieee->nrfd_r() << 7;

	return data;
}

void osborne1_state::ieee_pia_pb_w(u8 data)
{
	/*
	    bit     description

	    0       0 = DATAn as output, 1 = DATAn as input
	    1       0 = NDAC/NRFD as output, 1 = NDAC/NRFD as input; also gates SRQ
	    2       0 = EOI/DAV as output, 1 = EOI/DAV as input
	    3       EOI
	    4       ATN
	    5       DAV
	    6       NDAC
	    7       NRFD
	*/
	m_ieee->host_eoi_w(BIT(data, 3));
	m_ieee->host_atn_w(BIT(data, 4));
	m_ieee->host_dav_w(BIT(data, 5));
	m_ieee->host_ndac_w(BIT(data, 6));
	m_ieee->host_nrfd_w(BIT(data, 7));
}

void osborne1_state::ieee_pia_irq_a_func(int state)
{
	update_irq();
}


void osborne1_state::video_pia_port_a_w(u8 data)
{
	m_scroll_x = data >> 1;

	m_fdc->dden_w(BIT(data, 0));
}

void osborne1_state::video_pia_port_b_w(u8 data)
{
	m_speaker->level_w((BIT(data, 5) && m_beep_state) ? 1 : 0);

	if (BIT(data, 6))
	{
		m_fdc->set_floppy(m_floppy[0]->get_device());
		m_floppy[0]->get_device()->mon_w(0);
	}
	else if (BIT(data, 7))
	{
		m_fdc->set_floppy(m_floppy[1]->get_device());
		m_floppy[1]->get_device()->mon_w(0);
	}
	else
	{
		m_fdc->set_floppy(nullptr);
	}
}

void osborne1_state::video_pia_out_cb2_dummy(int state)
{
}

void osborne1_state::video_pia_irq_a_func(int state)
{
	update_irq();
}


void osborne1_state::serial_acia_irq_func(int state)
{
	m_acia_irq_state = state;
	update_irq();
}


INPUT_CHANGED_MEMBER( osborne1_state::reset_key )
{
	// This key affects NMI
	if (!m_ub6a_q)
		m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


void osborne1_state::machine_start()
{
	m_video_timer = timer_alloc(FUNC(osborne1_state::video_callback), this);
	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(osborne1_state::get_tile_info)), TILEMAP_SCAN_ROWS,
			8, 10, 128, 32);

	m_acia_rxc_txc_timer = timer_alloc(FUNC(osborne1_state::acia_rxc_txc_callback), this);

	m_maincpu->space(AS_PROGRAM).cache(m_mem_cache);

	save_item(NAME(m_acia_rxc_txc_div));
	save_item(NAME(m_acia_rxc_txc_p_low));
	save_item(NAME(m_acia_rxc_txc_p_high));

	save_item(NAME(m_ub4a_q));
	save_item(NAME(m_ub6a_q));
	save_item(NAME(m_rom_mode));

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_beep_state));

	save_item(NAME(m_acia_irq_state));
	save_item(NAME(m_acia_rxc_txc_state));
}

void osborne1_state::machine_reset()
{
	// Refresh configuration
	switch (m_cnf->read() & 0x03)
	{
	case 0x00:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 23;
		m_acia_rxc_txc_p_high   = 29;
		break;
	case 0x01:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 9;
		m_acia_rxc_txc_p_high   = 15;
		break;
	case 0x02:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 5;
		m_acia_rxc_txc_p_high   = 8;
		break;
	case 0x03:
		m_acia_rxc_txc_div      = 8;
		m_acia_rxc_txc_p_low    = 5;
		m_acia_rxc_txc_p_high   = 8;
		break;
	}

	// Initialise memory configuration
	m_rom_mode = 0;
	set_rom_mode(1);
	m_vram_view.disable();

	// Reset serial state
	m_acia_irq_state = 0;
	m_acia_rxc_txc_state = 0;
	update_acia_rxc_txc();

	// The low bits of attribute RAM are not physically present and hence always read high
	for (u8 &b : m_attr_ram)
		b |= 0x7f;
}

void osborne1_state::video_start()
{
	m_video_timer->adjust(m_screen->time_until_pos(1, 0));
}

void osborne1sp_state::machine_start()
{
	osborne1_state::machine_start();

	save_item(NAME(m_resolution));
	save_item(NAME(m_hc_left));
}

void osborne1sp_state::machine_reset()
{
	osborne1_state::machine_reset();

	// Reset video hardware
	m_resolution = 0;
	m_hc_left = 1;
}

template <int Width, unsigned Scale>
inline void osborne1_state::draw_rows(uint16_t col, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; cliprect.max_y >= y; ++y)
	{
		// Vertical scroll is latched at the start of the visible area
		if (0 == y)
			m_scroll_y = m_pia1->b_output() & 0x1f;

		// Draw a line of the display
		u8 const ra = y % 10;
		uint16_t *p = &bitmap.pix(y);
		uint16_t const row = ((m_scroll_y + (y / 10)) << 7) & 0x0f80;

		for (uint16_t x = 0; Width > x; ++x)
		{
			uint16_t const offs = row | ((col + x) & 0x7f);
			u8 const chr = m_main_ram[0xf000 | offs];
			u8 const clr = BIT(m_attr_ram[offs], 7) ? 2 : 1;

			bool const underline = BIT(chr, 7) && (ra == 9);
			u8 const gfx = underline ? 0xff : m_p_chargen[(ra << 7) | (chr & 0x7f)];

			// Display a scanline of a character
			for (unsigned b = 0; 8 > b; ++b)
				p = std::fill_n(p, Scale, BIT(gfx, 7 - b) ? clr : 0);
		}
	}
}

// The derivation of the initial column is not obvious.  The 7-bit
// column counter is preloaded near the beginning of the horizontal
// blank period.  The initial column is offset by the number of
// character clock periods in the horizontal blank period minus one
// because it latches the value before it's displayed.  Using the
// standard video display, there are 12 character clock periods in
// the horizontal blank period, so subtracting 1 gives 0x0B.  Using
// the SCREEN-PAC's high-resolution mode, the character clock is
// twice the frequency giving 24 character clock periods in the
// horizontal blanking period, so subtracting 1 gives 0x17.  Using
// the standard video display, the column counter is preloaded with
// the high 7 bits of the value from PIA1 PORTB.  The SCREEN-PAC
// takes the two high bits of this value, but sets the low five bits
// to a fixed value of 1 or 9 depending on the value of the HC-LEFT
// signal (set by bit 1 of the value written to 0x2400).  Of course
// it depends on the value wrapping around to zero when it counts
// past 0x7F.

uint32_t osborne1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_rows<52, 1>(scroll_x() + 0x0b, bitmap, cliprect);

	return 0;
}

uint32_t osborne1sp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_resolution)
		draw_rows<104, 1>((scroll_x() & 0x60) + (m_hc_left ? 0x09 : 0x01) + 0x17, bitmap, cliprect);
	else
		draw_rows<52, 2>(scroll_x() + 0x0b, bitmap, cliprect);

	return 0;
}


TIMER_CALLBACK_MEMBER(osborne1_state::video_callback)
{
	int const y = m_screen->vpos();
	u8 const ra = y % 10;

	// The beeper is gated so it's active four out of every ten scanlines
	m_beep_state = (ra & 0x04) ? 1 : 0;
	m_speaker->level_w((m_beep_state && BIT(m_pia1->b_output(), 5)) ? 1 : 0);

	int const next((y - ra) + ((ra < 4) ? 4 : (ra < 8) ? 8 : 14));
	m_video_timer->adjust(m_screen->time_until_pos((m_screen->height() > next) ? next : 0, 0));
}

TIMER_CALLBACK_MEMBER(osborne1_state::acia_rxc_txc_callback)
{
	m_acia_rxc_txc_state = m_acia_rxc_txc_state ? 0 : 1;
	update_acia_rxc_txc();
}


TILE_GET_INFO_MEMBER(osborne1_state::get_tile_info)
{
	// The gfxdecode and tilemap aren't actually used for drawing, they just look nice in the graphics viewer
	tileinfo.set(0, m_main_ram[0xf000 | tile_index] & 0x7f, 0, 0);
}


bool osborne1_state::set_rom_mode(u8 value)
{
	if (value != m_rom_mode)
	{
		m_rom_mode = value;
		if (m_rom_mode)
			m_rom_view.select(0);
		else
			m_rom_view.disable();
		return true;
	}
	else
	{
		return false;
	}
}

void osborne1_state::update_irq()
{
	if (m_pia0->irq_a_state())
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xf0); // Z80
	else if (m_pia1->irq_a_state())
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xf8); // Z80
	else if (m_acia_irq_state)
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xfc); // Z80
	else
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, CLEAR_LINE, 0xfe); // Z80
}

void osborne1_state::update_acia_rxc_txc()
{
	m_acia->write_rxc(m_acia_rxc_txc_state);
	m_acia->write_txc(m_acia_rxc_txc_state);
	attoseconds_t const dividend = (ATTOSECONDS_PER_SECOND / 100) * (m_acia_rxc_txc_state ? m_acia_rxc_txc_p_high : m_acia_rxc_txc_p_low);
	attoseconds_t const divisor = (15974400 / 100) / m_acia_rxc_txc_div;
	m_acia_rxc_txc_timer->adjust(attotime(0, dividend / divisor));
}


MC6845_UPDATE_ROW(osborne1nv_state::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint16_t const base = (ma >> 1) & 0xf80;
	uint32_t *p = &bitmap.pix(y);
	for (u8 x = 0; x < x_count; ++x)
	{
		uint16_t const offset = base | ((ma + x) & 0x7f);
		u8 const chr = m_main_ram[0xf000 | offset];
		u8 const clr = BIT(m_attr_ram[offset], 7) ? 2 : 1;

		u8 const gfx = ((chr & 0x80) && (ra == 9)) ? 0xff : m_p_nuevo[(ra << 7) | (chr & 0x7f)];

		for (unsigned bit = 0; 8 > bit; ++bit)
			*p++ = palette[BIT(gfx, 7 - bit) ? clr : 0];
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(osborne1nv_state::crtc_update_addr_changed)
{
}


void osborne1_state::osborne1_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_main_ram);
	map(0xf000, 0xffff).w(FUNC(osborne1_state::videoram_w));

	map(0x0000, 0x3fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x0fff).mirror(0x1000).rom().region("maincpu", 0).unmapw();
	m_rom_view[0](0x2000, 0x2fff).mirror(0x1000).rw(FUNC(osborne1_state::bank2_peripherals_r), FUNC(osborne1_state::bank2_peripherals_w));

	map(0xf000, 0xffff).view(m_vram_view);
	m_vram_view[0](0xf000, 0xffff).ram().w(FUNC(osborne1_state::attrram_w)).share(m_attr_ram);
}

void osborne1sp_state::osborne1sp_mem(address_map &map)
{
	osborne1_mem(map);

	m_rom_view[0](0x2000, 0x2fff).mirror(0x1000).rw(FUNC(osborne1sp_state::bank2_peripherals_r), FUNC(osborne1sp_state::bank2_peripherals_w));
}


void osborne1_state::osborne1_op(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(osborne1_state::opcode_r));
}


void osborne1_state::osborne1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x03);

	map(0x00, 0x03).w(FUNC(osborne1_state::bankswitch_w));
}

void osborne1nv_state::osborne1nv_io(address_map &map)
{
	osborne1_io(map);

	map.global_mask(0xff); // FIXME: complete guess

	map(0x04, 0x04).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x05, 0x05).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	// seems to be something at 0x06 as well, but no idea what - BIOS writes 0x07 on boot
}


static INPUT_PORTS_START( osborne1 )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)                             PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                            PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)    PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("Return")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)   PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                                   PORT_CHAR('\t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)                                   PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)        PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)        PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)        PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)        PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)        PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)        PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)        PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                                     PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                                     PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)     PORT_CODE(KEYCODE_DEL_PAD)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                                 PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)        PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)     PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                                     PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                                     PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                                     PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                                     PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                                     PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                                     PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                                     PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                                     PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                                     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                                     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                                     PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                                     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                                     PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                                     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                                     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                                     PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                                 PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                                     PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                                     PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                                     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                                     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                                     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                                     PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                                     PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                                PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                                     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                             PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                                 PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                                 PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                                 PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                                 PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE PORT_NAME("Alpha Lock")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_NAME("RESET") PORT_CHANGED_MEMBER(DEVICE_SELF, osborne1_state, reset_key, 0)
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CNF")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x03, 0x00, "Serial Speed")
	PORT_CONFSETTING(0x00, "300/1200")
	PORT_CONFSETTING(0x01, "600/2400")
	PORT_CONFSETTING(0x02, "1200/4800")
	PORT_CONFSETTING(0x03, "2400/9600")
INPUT_PORTS_END

INPUT_PORTS_START( osborne1nv )
	PORT_INCLUDE(osborne1)

	PORT_MODIFY("CNF")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


/*
 * The Osborne-1 supports the following disc formats:
 * - Osborne single density: 40 tracks, 10 sectors per track, 256-byte sectors (100 KByte)
 * - Osborne double density: 40 tracks, 5 sectors per track, 1024-byte sectors (200 KByte)
 * - IBM Personal Computer: 40 tracks, 8 sectors per track, 512-byte sectors (160 KByte)
 * - Xerox 820 Computer: 40 tracks, 18 sectors per track, 128-byte sectors (90 KByte)
 * - DEC 1820 double density: 40 tracks, 9 sectors per track, 512-byte sectors (180 KByte)
 *
 */

static void osborne1_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD); // Siemens FDD 100-5, custom Osborne electronics
	device.option_add("525ssdd", FLOPPY_525_SSDD); // MPI 52(?), custom Osborne electronics
}


/* F4 Character Displayer */
static const gfx_layout osborne1_charlayout =
{
	8, 10,              // 8 x 10 characters
	128,                // 128 characters
	1,                  // 1 bits per pixel
	{ 0 },              // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*128*8, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8 },
	8                   // every char takes 16 x 1 bytes
};

static GFXDECODE_START( gfx_osborne1 )
	GFXDECODE_ENTRY("chargen", 0x0000, osborne1_charlayout, 0, 1)
GFXDECODE_END


void osborne1_state::osborne1_base(machine_config &config)
{
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &osborne1_state::osborne1_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &osborne1_state::osborne1_op);
	m_maincpu->set_addrmap(AS_IO, &osborne1_state::osborne1_io);
	m_maincpu->irqack_cb().set(FUNC(osborne1_state::irqack_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());

	GFXDECODE(config, m_gfxdecode, "palette", gfx_osborne1);
	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 1.00);

	PIA6821(config, m_pia0);
	m_pia0->readpa_handler().set(m_ieee, FUNC(ieee488_device::dio_r));
	m_pia0->readpb_handler().set(FUNC(osborne1_state::ieee_pia_pb_r));
	m_pia0->writepa_handler().set(m_ieee, FUNC(ieee488_device::host_dio_w));
	m_pia0->writepb_handler().set(FUNC(osborne1_state::ieee_pia_pb_w));
	m_pia0->ca2_handler().set(m_ieee, FUNC(ieee488_device::host_ifc_w));
	m_pia0->cb2_handler().set(m_ieee, FUNC(ieee488_device::host_ren_w));
	m_pia0->irqa_handler().set(FUNC(osborne1_state::ieee_pia_irq_a_func));

	IEEE488(config, m_ieee, 0);
	m_ieee->srq_callback().set(m_pia0, FUNC(pia6821_device::ca2_w));

	PIA6821(config, m_pia1);
	m_pia1->writepa_handler().set(FUNC(osborne1_state::video_pia_port_a_w));
	m_pia1->writepb_handler().set(FUNC(osborne1_state::video_pia_port_b_w));
	m_pia1->cb2_handler().set(FUNC(osborne1_state::video_pia_out_cb2_dummy));
	m_pia1->irqa_handler().set(FUNC(osborne1_state::video_pia_irq_a_func));

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(FUNC(osborne1_state::serial_acia_irq_func));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.ri_handler().set(m_pia1, FUNC(pia6821_device::ca2_w));

	MB8877(config, m_fdc, MAIN_CLOCK/16);
	m_fdc->set_force_ready(true);
	FLOPPY_CONNECTOR(config, m_floppy[0], osborne1_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], osborne1_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("osborne1");
}

void osborne1_state::osborne1(machine_config &config)
{
	osborne1_base(config);

	m_screen->set_screen_update(FUNC(osborne1_state::screen_update));
	m_screen->set_raw(MAIN_CLOCK / 2, 512, 0, 52 * 8, 260, 0, 24 * 10);
	m_screen->screen_vblank().set(m_pia1, FUNC(pia6821_device::ca1_w));
	m_screen->set_palette("palette");
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE);
}

void osborne1sp_state::osborne1sp(machine_config &config)
{
	osborne1_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &osborne1sp_state::osborne1sp_mem);

	m_screen->set_screen_update(FUNC(osborne1sp_state::screen_update));
	m_screen->set_raw(MAIN_CLOCK, 1024, 0, 104 * 8, 260, 0, 24 * 10);
	m_screen->screen_vblank().set(m_pia1, FUNC(pia6821_device::ca1_w));
	m_screen->set_palette("palette");
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE);
}

void osborne1nv_state::osborne1nv(machine_config &config)
{
	osborne1_base(config);

	m_maincpu->set_addrmap(AS_IO, &osborne1nv_state::osborne1nv_io);

	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	m_screen->set_raw(12.288_MHz_XTAL, (96 + 1) * 8, 0, 80 * 8, ((25 + 1) * 10) + 4, 0, 24 * 10);
	m_screen->screen_vblank().set(m_pia1, FUNC(pia6821_device::ca1_w)); // FIXME: AFAICT the PIA gets this from the (vestigial) onboard video circuitry

	sy6545_1_device &crtc(SY6545_1(config, "crtc", 12.288_MHz_XTAL / 8));
	crtc.set_screen(m_screen);
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(osborne1nv_state::crtc_update_row));
	crtc.set_on_update_addr_change_callback(FUNC(osborne1nv_state::crtc_update_addr_changed));
}


ROM_START( osborne1 )
	ROM_DEFAULT_BIOS("ver144")
	ROM_SYSTEM_BIOS( 0, "vera",   "BIOS version A" )
	ROM_SYSTEM_BIOS( 1, "ver12",  "BIOS version 1.2" )
	ROM_SYSTEM_BIOS( 2, "ver121", "BIOS version 1.2.1" )
	ROM_SYSTEM_BIOS( 3, "ver13",  "BIOS version 1.3" )
	ROM_SYSTEM_BIOS( 4, "ver14",  "BIOS version 1.4" )
	ROM_SYSTEM_BIOS( 5, "ver143", "BIOS version 1.43" )
	ROM_SYSTEM_BIOS( 6, "ver144", "BIOS version 1.44" )

	ROM_REGION( 0x1000, "maincpu", 0 )
	ROMX_LOAD( "osba.bin",               0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(0) )
	ROMX_LOAD( "osb12.bin",              0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(1) )
	ROMX_LOAD( "osb121.bin",             0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(2) )
	ROMX_LOAD( "osb13.bin",              0x0000, 0x1000, NO_DUMP,                                                      ROM_BIOS(3) )
	ROMX_LOAD( "rev1.40.ud11",           0x0000, 0x1000, CRC(3d966335) SHA1(0c60b97a3154a75868efc6370d26995eadc7d927), ROM_BIOS(4) )
	ROMX_LOAD( "rev1.43.ud11",           0x0000, 0x1000, CRC(91a48e3c) SHA1(c37b83f278d21e6e92d80f9c057b11f7f22d88d4), ROM_BIOS(5) )
	ROMX_LOAD( "3a10082-00rev-e.ud11",   0x0000, 0x1000, CRC(c0596b14) SHA1(ee6a9cc9be3ddc5949d3379351c1d58a175ce9ac), ROM_BIOS(6) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(0) ) // this is CHRROM from v1.4 BIOS MB
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(1) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(2) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(3) )
	ROMX_LOAD( "char.ua15",      0x0000, 0x800, CRC(5297c109) SHA1(e1a59d87edd66e6c226102cb0688e9cb74dbb594), ROM_BIOS(4) )
	ROMX_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801), ROM_BIOS(5) )
	ROMX_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801), ROM_BIOS(6) )
ROM_END

#define rom_osborne1sp rom_osborne1

ROM_START( osborne1nv )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "monrom-rev1.51-12.ud11", 0x0000, 0x1000, CRC(298da402) SHA1(7fedd070936ccfe98f96d6e0ac71689666da79cb) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "7a3007-00.ud15", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801) )

	ROM_REGION( 0x0800, "nuevo", 0 )
	ROM_LOAD( "character_generator_6-29-84.14", 0x0000, 0x800, CRC(6c1eab0d) SHA1(b04459d377a70abc9155a5486003cb795342c801) )
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY          FULLNAME                     FLAGS
COMP( 1981, osborne1,   0,        0,      osborne1,   osborne1,   osborne1_state,   empty_init, "Osborne",       "Osborne-1",                 MACHINE_SUPPORTS_SAVE )
COMP( 1983, osborne1sp, osborne1, 0,      osborne1sp, osborne1,   osborne1sp_state, empty_init, "Osborne",       "Osborne-1 with SCREEN-PAC", MACHINE_SUPPORTS_SAVE )
COMP( 1984, osborne1nv, osborne1, 0,      osborne1nv, osborne1nv, osborne1nv_state, empty_init, "Osborne/Nuevo", "Osborne-1 (Nuevo Video)",   MACHINE_SUPPORTS_SAVE )
