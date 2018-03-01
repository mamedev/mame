// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Kontron KDT6

    This is the base board for various machines, it needs to be combined
    with an I/O board and a bus board. This gives us
    - KDT6 + 9xx/IOC + 9xx/BUS: PSI908/PSI9C
    - KDT6 + 98/IOC + 98/BUS: PSI98

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/upd765.h"
#include "machine/upd1990a.h"
#include "machine/clock.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"
#include "bus/centronics/ctronics.h"
#include "bus/psi_kbd/psi_kbd.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "speaker.h"
#include "softlist.h"
#include "kdt6.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kdt6_state : public driver_device
{
public:
	kdt6_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_cpu(*this, "maincpu"),
	m_dma(*this, "dma"),
	m_sio(*this, "sio"),
	m_crtc(*this, "crtc"),
	m_page_r(*this, "page%x_r", 0), m_page_w(*this, "page%x_w", 0),
	m_boot(*this, "boot"),
	m_palette(*this, "palette"),
	m_gfx(*this, "gfx"),
	m_rtc(*this, "rtc"),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_beeper(*this, "beeper"),
	m_beep_timer(*this, "beep_timer"),
	m_centronics(*this, "centronics"),
	m_dip_s2(*this, "S2"),
	m_keyboard(*this, "kbd"),
	m_rs232b(*this, "rs232b"),
	m_sasi_dma(false),
	m_dma_map(0),
	m_status0(0), m_status1(0), m_status2(0),
	m_video_address(0)
	{ }

	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_READ8_MEMBER(memory_r);
	DECLARE_WRITE8_MEMBER(memory_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(page0_r);
	DECLARE_READ8_MEMBER(page1_r);
	DECLARE_READ8_MEMBER(mapper_r);
	DECLARE_WRITE8_MEMBER(mapper_w);
	DECLARE_READ8_MEMBER(sasi_ctrl_r);
	DECLARE_WRITE8_MEMBER(sasi_ctrl_w);
	DECLARE_WRITE8_MEMBER(dma_map_w);
	DECLARE_WRITE8_MEMBER(status0_w);
	DECLARE_READ8_MEMBER(status1_r);
	DECLARE_WRITE8_MEMBER(status1_w);
	DECLARE_WRITE8_MEMBER(status2_w);

	DECLARE_READ8_MEMBER(video_data_r);
	DECLARE_WRITE8_MEMBER(video_data_w);
	DECLARE_WRITE8_MEMBER(video_data_inc_w);
	DECLARE_WRITE8_MEMBER(video_data_dec_w);
	DECLARE_WRITE8_MEMBER(video_address_latch_high_w);
	DECLARE_WRITE8_MEMBER(video_address_latch_low_w);

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	DECLARE_WRITE8_MEMBER(fdc_tc_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	void drive0_led_cb(floppy_image_device *floppy, int state);
	void drive1_led_cb(floppy_image_device *floppy, int state);

	MC6845_UPDATE_ROW(crtc_update_row);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(pio_porta_w);
	DECLARE_WRITE_LINE_MEMBER(keyboard_rx_w);
	DECLARE_WRITE_LINE_MEMBER(rs232b_rx_w);
	DECLARE_WRITE_LINE_MEMBER(siob_tx_w);

	void psi98(machine_config &config);
	void psi98_io(address_map &map);
	void psi98_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_cpu;
	required_device<z80dma_device> m_dma;
	required_device<z80sio_device> m_sio;
	required_device<mc6845_device> m_crtc;
	required_memory_bank_array<16> m_page_r, m_page_w;
	required_memory_region m_boot;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_gfx;
	required_device<upd1990a_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device<centronics_device> m_centronics;
	required_ioport m_dip_s2;
	required_device<psi_keyboard_bus_device> m_keyboard;
	required_device<rs232_port_device> m_rs232b;

	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint16_t[]> m_vram; // 10-bit
	std::unique_ptr<uint8_t[]> m_dummy_r, m_dummy_w;

	bool m_sasi_dma;
	uint8_t m_dma_map;
	uint8_t m_status0;
	uint8_t m_status1;
	uint8_t m_status2;
	uint16_t m_mapper[16]; // 12-bit
	uint16_t m_video_address;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

ADDRESS_MAP_START(kdt6_state::psi98_mem)
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("page0_r") AM_WRITE_BANK("page0_w")
	AM_RANGE(0x1000, 0x1fff) AM_READ_BANK("page1_r") AM_WRITE_BANK("page1_w")
	AM_RANGE(0x2000, 0x2fff) AM_READ_BANK("page2_r") AM_WRITE_BANK("page2_w")
	AM_RANGE(0x3000, 0x3fff) AM_READ_BANK("page3_r") AM_WRITE_BANK("page3_w")
	AM_RANGE(0x4000, 0x4fff) AM_READ_BANK("page4_r") AM_WRITE_BANK("page4_w")
	AM_RANGE(0x5000, 0x5fff) AM_READ_BANK("page5_r") AM_WRITE_BANK("page5_w")
	AM_RANGE(0x6000, 0x6fff) AM_READ_BANK("page6_r") AM_WRITE_BANK("page6_w")
	AM_RANGE(0x7000, 0x7fff) AM_READ_BANK("page7_r") AM_WRITE_BANK("page7_w")
	AM_RANGE(0x8000, 0x8fff) AM_READ_BANK("page8_r") AM_WRITE_BANK("page8_w")
	AM_RANGE(0x9000, 0x9fff) AM_READ_BANK("page9_r") AM_WRITE_BANK("page9_w")
	AM_RANGE(0xa000, 0xafff) AM_READ_BANK("pagea_r") AM_WRITE_BANK("pagea_w")
	AM_RANGE(0xb000, 0xbfff) AM_READ_BANK("pageb_r") AM_WRITE_BANK("pageb_w")
	AM_RANGE(0xc000, 0xcfff) AM_READ_BANK("pagec_r") AM_WRITE_BANK("pagec_w")
	AM_RANGE(0xd000, 0xdfff) AM_READ_BANK("paged_r") AM_WRITE_BANK("paged_w")
	AM_RANGE(0xe000, 0xefff) AM_READ_BANK("pagee_r") AM_WRITE_BANK("pagee_w")
	AM_RANGE(0xf000, 0xffff) AM_READ_BANK("pagef_r") AM_WRITE_BANK("pagef_w")
ADDRESS_MAP_END

ADDRESS_MAP_START(kdt6_state::psi98_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("dma", z80dma_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("sio", z80sio_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("pio", z80pio_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ctc2", z80ctc_device, read, write)
	AM_RANGE(0x14, 0x14) AM_DEVREAD("fdc", upd765a_device, msr_r)
	AM_RANGE(0x15, 0x15) AM_DEVREADWRITE("fdc", upd765a_device, fifo_r, fifo_w)
	AM_RANGE(0x18, 0x18) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x19, 0x19) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x1c, 0x1c) AM_WRITE(status0_w)
	AM_RANGE(0x1d, 0x1d) AM_DEVREAD("kbd", psi_keyboard_bus_device, key_data_r)
	AM_RANGE(0x1e, 0x1e) AM_DEVREADWRITE("fdc", upd765a_device, mdma_r, mdma_w)
	AM_RANGE(0x1f, 0x1f) AM_WRITE(fdc_tc_w)
	AM_RANGE(0x20, 0x2f) AM_READWRITE(mapper_r, mapper_w)
	AM_RANGE(0x30, 0x30) AM_READWRITE(video_data_r, video_data_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(video_data_inc_w)
	AM_RANGE(0x36, 0x36) AM_WRITE(video_data_dec_w)
	AM_RANGE(0x37, 0x37) AM_WRITE(video_data_inc_w)
	AM_RANGE(0x38, 0x38) AM_WRITE(status1_w)
	AM_RANGE(0x39, 0x39) AM_READ(status1_r)
	AM_RANGE(0x3a, 0x3a) AM_WRITE(status2_w)
	AM_RANGE(0x3b, 0x3b) AM_READWRITE(sasi_ctrl_r, sasi_ctrl_w)
	AM_RANGE(0x3c, 0x3c) AM_WRITE(dma_map_w)
#if 0
	AM_RANGE(0x3d, 0x3d) WATCHDOG
	AM_RANGE(0x3e, 0x3e) WATCHDOG TRIGGER
	AM_RANGE(0x3f, 0x3f) SASI DATA
#endif
	AM_RANGE(0x40, 0x40) AM_WRITE(video_address_latch_high_w)
	AM_RANGE(0x41, 0x41) AM_WRITE(video_address_latch_low_w)
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( psi98 )
	PORT_START("S2")
	PORT_DIPNAME(0x01, 0x01, "SIO B") PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(0x00, "RS232")
	PORT_DIPSETTING(0x01, "Keyboard")
INPUT_PORTS_END


//**************************************************************************
//  FLOPPY
//**************************************************************************

static SLOT_INTERFACE_START( kdt6_floppies )
	SLOT_INTERFACE("fd55f", TEAC_FD_55F)
SLOT_INTERFACE_END

WRITE8_MEMBER( kdt6_state::fdc_tc_w )
{
	m_fdc->tc_w(1);
	m_fdc->tc_w(0);
}

WRITE_LINE_MEMBER( kdt6_state::fdc_drq_w )
{
	if (!m_sasi_dma && BIT(m_status0, 4) == 0)
		m_dma->rdy_w(state);
}

void kdt6_state::drive0_led_cb(floppy_image_device *floppy, int state)
{
	machine().output().set_value("drive0_led", state);
}

void kdt6_state::drive1_led_cb(floppy_image_device *floppy, int state)
{
	machine().output().set_value("drive1_led", state);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

WRITE8_MEMBER( kdt6_state::video_address_latch_high_w )
{
	m_video_address &= 0x00ff;
	m_video_address |= (data << 8);
}

WRITE8_MEMBER( kdt6_state::video_address_latch_low_w )
{
	m_video_address &= 0xff00;
	m_video_address |= (data << 0);
}

READ8_MEMBER( kdt6_state::video_data_r )
{
	return m_vram[m_video_address];
}

WRITE8_MEMBER( kdt6_state::video_data_w )
{
	m_vram[m_video_address] = ((m_status1 & 0x0c) << 6) | data;
}

WRITE8_MEMBER( kdt6_state::video_data_inc_w )
{
	m_vram[m_video_address++] = ((m_status1 & 0x0c) << 6) | data;
}

WRITE8_MEMBER( kdt6_state::video_data_dec_w )
{
	m_vram[m_video_address--] = ((m_status1 & 0x0c) << 6) | data;
}

uint32_t kdt6_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

MC6845_UPDATE_ROW( kdt6_state::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		if (BIT(m_status1, 6))
		{
			// text mode
			uint16_t code = m_vram[((m_status1 & 0x03) << 14) | (ma + i)];
			uint8_t data = m_gfx[((code & 0xff) << 4) | ra];

			int inverse = BIT(code, 8) | BIT(m_status1, 5) | ((i == cursor_x) ? 1 : 0);
			int blink = BIT(code, 9) & m_crtc->cursor_r();

			// draw 8 pixels of the character
			for (int x = 0; x < 8; x++)
			{
				int color = BIT(data, 7 - x);
				bitmap.pix32(y, x + i*8) = pen[blink ? 0 : (color ^ inverse)];
			}
		}
		else
		{
			// gfx mode
			uint8_t data = m_vram[(ma << 4) | (ra << 6) | i];

			// draw 8 pixels of the cell
			for (int x = 0; x < 8; x++)
				bitmap.pix32(y, x + i*8) = pen[BIT(data, 7 - x)];
		}
	}
}


//**************************************************************************
//  SOUND
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER( kdt6_state::beeper_off )
{
	m_beeper->set_state(0);
}


//**************************************************************************
//  EXTERNAL I/O
//**************************************************************************

WRITE8_MEMBER( kdt6_state::pio_porta_w )
{
	m_centronics->write_strobe(BIT(data, 0));
	m_centronics->write_init(BIT(data, 1));
}

WRITE_LINE_MEMBER( kdt6_state::keyboard_rx_w )
{
	if (machine().phase() >= machine_phase::RESET)
	{
		if ((m_dip_s2->read() & 0x01) == 0x01)
			m_sio->rxb_w(state);
	}
}

WRITE_LINE_MEMBER( kdt6_state::rs232b_rx_w )
{
	if (machine().phase() >= machine_phase::RESET)
	{
		if ((m_dip_s2->read() & 0x01) == 0x00)
			m_sio->rxb_w(state);
	}
}

WRITE_LINE_MEMBER( kdt6_state::siob_tx_w )
{
	if (machine().phase() >= machine_phase::RESET)
	{
		if ((m_dip_s2->read() & 0x01) == 0x01)
			m_keyboard->tx_w(state);
		else
			m_rs232b->write_txd(state);
	}
}


//**************************************************************************
//  MACHINE
//**************************************************************************

WRITE_LINE_MEMBER( kdt6_state::busreq_w )
{
	m_cpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state);
}

READ8_MEMBER( kdt6_state::memory_r )
{
	return m_ram[m_dma_map << 16 | offset];
}

WRITE8_MEMBER( kdt6_state::memory_w )
{
	m_ram[m_dma_map << 16 | offset] = data;
}

READ8_MEMBER( kdt6_state::io_r )
{
	return m_cpu->space(AS_IO).read_byte(offset);
}

WRITE8_MEMBER( kdt6_state::io_w )
{
	m_cpu->space(AS_IO).write_byte(offset, data);
}

READ8_MEMBER( kdt6_state::page0_r )
{
	if (BIT(m_status0, 5) == 0)
		return m_boot->as_u8(offset);

	return reinterpret_cast<uint8_t *>(m_page_r[0]->base())[offset];
}

READ8_MEMBER( kdt6_state::page1_r )
{
	if (BIT(m_status0, 5) == 0)
		return m_boot->as_u8(0x1000 + offset);

	return reinterpret_cast<uint8_t *>(m_page_r[1]->base())[offset];
}

READ8_MEMBER( kdt6_state::sasi_ctrl_r )
{
	uint8_t data = 0;

	// 7-------  sasi select
	// -6------  sasi reset
	// --5-----  sasi input/output
	// ---4----  sasi control/data
	// ----3---  sasi message
	// -----2--  sasi request
	// ------1-  sasi busy
	// -------0  data output upd1990

	data |= m_rtc->data_out_r() << 0;
	data |= 0xfe;

	return data;
}

WRITE8_MEMBER( kdt6_state::sasi_ctrl_w )
{
	logerror("sasi_ctrl_w: %02x\n", data);
	m_sasi_dma = bool(BIT(data, 2) == 0);
}

WRITE8_MEMBER( kdt6_state::dma_map_w )
{
	m_dma_map = data;
}

READ8_MEMBER( kdt6_state::mapper_r )
{
	return m_mapper[offset];
}

WRITE8_MEMBER( kdt6_state::mapper_w )
{
	m_mapper[offset] = ((m_status2 & 0x0f) << 8) | data;

	if (BIT(m_status1, 7) == 0)
	{
		offs_t addr = bitswap(m_mapper[offset], 8, 9, 10, 11, 7, 6, 5, 4, 0, 1, 2, 3) << 12;

		m_page_r[offset]->set_base(addr < 0x40000 ? &m_ram[addr] : &m_dummy_r[0]);
		m_page_w[offset]->set_base(addr < 0x40000 ? &m_ram[addr] : &m_dummy_w[0]);

		if (0)
			logerror("map_page: %x -> %06x\n", offset, addr);
	}
}

WRITE8_MEMBER( kdt6_state::status0_w )
{
	logerror("status0_w: %02x\n", data);

	// 7-------  floppy motor
	// -6------  floppy drive type
	// --5-----  prom off
	// ---4----  dma trigger (floppy/sio)
	// ----3---  character generator a12
	// -----2--  sound
	// ------1-  system frequency (0 = half)
	// -------0  watchdog enable

	m_cpu->set_unscaled_clock((XTAL(16'000'000) / 4) * (BIT(data, 1) ? 1 : 0.5));

	if ((BIT(m_status0, 2) ^ BIT(data, 2)) && BIT(data, 2))
	{
		m_beeper->set_state(1);
		m_beep_timer->adjust(attotime::from_msec(250)); // timing unknown
	}

	if (m_floppy0->get_device())
		m_floppy0->get_device()->mon_w(BIT(data, 7) ? 0 : 1);
	if (m_floppy1->get_device())
		m_floppy1->get_device()->mon_w(BIT(data, 7) ? 0 : 1);

	m_status0 = data;
}

READ8_MEMBER( kdt6_state::status1_r )
{
	// 7-------  disable memory mapper (1 = disable)
	// -6------  display mode (1 = text, 0 = gfx)
	// --5-----  video invert
	// ---4----  select 3 video memory bank
	// ----3---  video bit 9
	// -----2--  video bit 8
	// ------1-  video scroll address bit 15
	// -------0  video scroll address bit 14

	return m_status1;
}

WRITE8_MEMBER( kdt6_state::status1_w )
{
	logerror("status1_w: %02x\n", data);

	// memory mapper disabled?
	if (BIT(data, 7))
	{
		for (unsigned i = 0; i < 16; i++)
		{
			m_page_r[i]->set_base(&m_ram[i * 0x1000]);
			m_page_w[i]->set_base(&m_ram[i * 0x1000]);
		}
	}

	// note that enabling the memory mapper doesn't mean that the pages are automatically mapped in
	// it still needs a write to the mapper register to activate

	m_status1 = data;
}

WRITE8_MEMBER( kdt6_state::status2_w )
{
	if (0)
		logerror("status2_w: %02x\n", data);

	// 7-------  rtc chip select
	// -6------  rtc output enable
	// --5-----  rtc strobe
	// ---4----  rtc clock
	// ----321-  rtc control 2-0
	// -------0  rtc data
	// ----3210  memory mapper bit 0-3

	m_rtc->cs_w(BIT(data, 7));
	m_rtc->oe_w(BIT(data, 6));
	m_rtc->stb_w(BIT(data, 5));
	m_rtc->clk_w(BIT(data, 4));
	m_rtc->c2_w(BIT(data, 3));
	m_rtc->c1_w(BIT(data, 2));
	m_rtc->c0_w(BIT(data, 1));
	m_rtc->data_in_w(BIT(data, 0));

	m_status2 = data & 0x0f;
}

void kdt6_state::machine_start()
{
	// 256 kb ram, 64 kb vram (and two dummy regions for invalid pages)
	m_ram = std::make_unique<uint8_t[]>(0x40000);
	m_vram = std::make_unique<uint16_t[]>(0x10000);
	m_dummy_r = std::make_unique<uint8_t[]>(0x1000);
	m_dummy_w = std::make_unique<uint8_t[]>(0x1000);

	// override the region 0x0000 to 0x1fff here to enable prom reading
	m_cpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8_delegate(FUNC(kdt6_state::page0_r), this));
	m_cpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(kdt6_state::page1_r), this));

	m_fdc->set_rate(250000);

	if (m_floppy0->get_device())
		m_floppy0->get_device()->setup_led_cb(floppy_image_device::led_cb(&kdt6_state::drive0_led_cb, this));
	if (m_floppy1->get_device())
		m_floppy1->get_device()->setup_led_cb(floppy_image_device::led_cb(&kdt6_state::drive1_led_cb, this));

	// register for save states
	save_pointer(NAME(m_ram.get()), 0x40000);
	save_pointer(NAME(m_vram.get()), 0x10000);
	save_item(NAME(m_sasi_dma));
	save_item(NAME(m_dma_map));
	save_item(NAME(m_status0));
	save_item(NAME(m_status1));
	save_item(NAME(m_status2));
	save_pointer(NAME(m_mapper), 16);
	save_item(NAME(m_video_address));
}

void kdt6_state::machine_reset()
{
	// status0 is forced to 0 on reset
	status0_w(m_cpu->space(AS_IO), 0, 0);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "ctc1" },
	{ "sio" },
	{ "ctc2" },
	{ "pio" },
	{ nullptr }
};

MACHINE_CONFIG_START(kdt6_state::psi98)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(16'000'000) / 4)
	MCFG_CPU_PROGRAM_MAP(psi98_mem)
	MCFG_CPU_IO_MAP(psi98_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain_intf)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS(XTAL(13'516'800), 824, 48, 688, 274, 0, 250)
	MCFG_SCREEN_UPDATE_DRIVER(kdt6_state, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_DEFAULT_LAYOUT(layout_kdt6)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL(13'516'800) / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(kdt6_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("ctc2", z80ctc_device, trg2))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 1000) // frequency unknown
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_DRIVER_ADD("beep_timer", kdt6_state, beeper_off)

	MCFG_DEVICE_ADD("dma", Z80DMA, XTAL(16'000'000) / 4)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(kdt6_state, busreq_w))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(kdt6_state, memory_r))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(kdt6_state, memory_w))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(kdt6_state, io_r))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(kdt6_state, io_w))

	// jumper J3 allows selection of 16MHz / 8 instead
	MCFG_CLOCK_ADD("uart_clk", XTAL(9'830'400) / 8)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc1", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc1", z80ctc_device, trg2))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL(16'000'000) / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("sio", z80sio_device, rxtxcb_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("sio", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", z80sio_device, txca_w))

	MCFG_DEVICE_ADD("ctc2", Z80CTC, XTAL(16'000'000) / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("ctc2", z80ctc_device, trg3))

	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(16'000'000) / 4)
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE(kdt6_state, siob_tx_w))
	MCFG_Z80SIO_OUT_DTRB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio", z80sio_device, dcda_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("sio", z80sio_device, synca_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsa_w))  MCFG_DEVCB_XOR(1)

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(kdt6_state, rs232b_rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio", z80sio_device, dcdb_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("sio", z80sio_device, syncb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsb_w))  MCFG_DEVCB_XOR(1)

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL(16'000'000) / 4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(kdt6_state, pio_porta_w))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("cent_data_in", input_buffer_device, read))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("pio", z80pio_device, pa2_w))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("pio", z80pio_device, pa3_w))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("pio", z80pio_device, pa4_w))
	MCFG_CENTRONICS_SELECT_HANDLER(DEVWRITELINE("pio", z80pio_device, pa5_w))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_UPD1990A_ADD("rtc", XTAL(32'768), NOOP, NOOP)

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("ctc1", z80ctc_device, trg0))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(kdt6_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", kdt6_floppies, "fd55f", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", kdt6_floppies, "fd55f", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("floppy_list", "psi98")

	MCFG_PSI_KEYBOARD_INTERFACE_ADD("kbd", "hle")
	MCFG_PSI_KEYBOARD_RX_HANDLER(WRITELINE(kdt6_state, keyboard_rx_w))
	MCFG_PSI_KEYBOARD_KEY_STROBE_HANDLER(DEVWRITELINE("ctc2", z80ctc_device, trg1))

	// 6 ECB slots
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( psi98 )
	ROM_REGION(0x2000, "boot", 0)
	ROM_LOAD("td_62b60_1.prom1", 0x0000, 0x1000, CRC(dbc01eeb) SHA1(3ecab997623836599a5e0b2a05d0f743d93edf00)) // SUM16: 0a84 (file: 29ca)
	ROM_LOAD("boot_v61_a.prom2", 0x1000, 0x1000, CRC(6dd9a5e3) SHA1(b812947009d83e9c6119e731ba7d40fe3597d42c)) // SUM16: d4ec (file: b1fb)

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("ch6_d-e.bin", 0x0000, 0x1000, CRC(e7bca335) SHA1(454ca3b8b8ac66464870e4bd5497050038d771c8))      // SUM16: 39b1 (file: 5f55)

	// PALs
	// 1-FF5B  12L6  memory address decoder
	// 2-0F61  12L8  i/o address decoder
	// 3-C7BF  10H8  interrupt priority controller
	// 4-EC5E  16L8  bus controller
	// 5-1126  16H2  fdc write precompensation
	// 6-1BA7  10L8  video memory access controller
	// 7-C1ED  16L8  video memory timing controller
	// 8-CD9F  12H6  fdc timing generator
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT  CLASS       INIT  COMPANY    FULLNAME  FLAGS
COMP( 1984, psi98,  0,      0,       psi98,   psi98, kdt6_state, 0,    "Kontron", "PSI98",  0 )
