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
#include "imagedev/floppy.h"
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
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "softlist_dev.h"
#include "kdt6.lh"


namespace {

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
		m_floppy(*this, "fdc:%u", 0U),
		m_beeper(*this, "beeper"),
		m_beep_timer(*this, "beep_timer"),
		m_centronics(*this, "centronics"),
		m_dip_s2(*this, "S2"),
		m_keyboard(*this, "kbd"),
		m_rs232b(*this, "rs232b"),
		m_drive_led(*this, "drive%u_led", 0U),
		m_sasi_dma(false),
		m_dma_map(0),
		m_status0(0), m_status1(0), m_status2(0),
		m_video_address(0)
	{ }

	void psi98(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t memory_r(offs_t offset);
	void memory_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t page0_r(offs_t offset);
	uint8_t page1_r(offs_t offset);
	uint8_t mapper_r(offs_t offset);
	void mapper_w(offs_t offset, uint8_t data);
	uint8_t sasi_ctrl_r();
	void sasi_ctrl_w(uint8_t data);
	void dma_map_w(uint8_t data);
	void status0_w(uint8_t data);
	uint8_t status1_r();
	void status1_w(uint8_t data);
	void status2_w(uint8_t data);

	uint8_t video_data_r();
	void video_data_w(uint8_t data);
	void video_data_inc_w(uint8_t data);
	void video_data_dec_w(uint8_t data);
	void video_address_latch_high_w(uint8_t data);
	void video_address_latch_low_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off);

	void fdc_tc_w(uint8_t data);
	void fdc_drq_w(int state);
	void drive0_led_cb(floppy_image_device *floppy, int state);
	void drive1_led_cb(floppy_image_device *floppy, int state);

	MC6845_UPDATE_ROW(crtc_update_row);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void pio_porta_w(uint8_t data);
	void keyboard_rx_w(int state);
	void rs232b_rx_w(int state);
	void siob_tx_w(int state);

	void psi98_io(address_map &map) ATTR_COLD;
	void psi98_mem(address_map &map) ATTR_COLD;

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
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<beep_device> m_beeper;
	required_device<timer_device> m_beep_timer;
	required_device<centronics_device> m_centronics;
	required_ioport m_dip_s2;
	required_device<psi_keyboard_bus_device> m_keyboard;
	required_device<rs232_port_device> m_rs232b;
	output_finder<2> m_drive_led;

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

void kdt6_state::psi98_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("page0_r").bankw("page0_w");
	map(0x1000, 0x1fff).bankr("page1_r").bankw("page1_w");
	map(0x2000, 0x2fff).bankr("page2_r").bankw("page2_w");
	map(0x3000, 0x3fff).bankr("page3_r").bankw("page3_w");
	map(0x4000, 0x4fff).bankr("page4_r").bankw("page4_w");
	map(0x5000, 0x5fff).bankr("page5_r").bankw("page5_w");
	map(0x6000, 0x6fff).bankr("page6_r").bankw("page6_w");
	map(0x7000, 0x7fff).bankr("page7_r").bankw("page7_w");
	map(0x8000, 0x8fff).bankr("page8_r").bankw("page8_w");
	map(0x9000, 0x9fff).bankr("page9_r").bankw("page9_w");
	map(0xa000, 0xafff).bankr("pagea_r").bankw("pagea_w");
	map(0xb000, 0xbfff).bankr("pageb_r").bankw("pageb_w");
	map(0xc000, 0xcfff).bankr("pagec_r").bankw("pagec_w");
	map(0xd000, 0xdfff).bankr("paged_r").bankw("paged_w");
	map(0xe000, 0xefff).bankr("pagee_r").bankw("pagee_w");
	map(0xf000, 0xffff).bankr("pagef_r").bankw("pagef_w");
}

void kdt6_state::psi98_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x04, 0x07).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x10, 0x13).rw("ctc2", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x14, 0x14).r(m_fdc, FUNC(upd765a_device::msr_r));
	map(0x15, 0x15).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x18, 0x18).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x19, 0x19).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1c, 0x1c).w(FUNC(kdt6_state::status0_w));
	map(0x1d, 0x1d).r(m_keyboard, FUNC(psi_keyboard_bus_device::key_data_r));
	map(0x1e, 0x1e).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x1f, 0x1f).w(FUNC(kdt6_state::fdc_tc_w));
	map(0x20, 0x2f).rw(FUNC(kdt6_state::mapper_r), FUNC(kdt6_state::mapper_w));
	map(0x30, 0x30).rw(FUNC(kdt6_state::video_data_r), FUNC(kdt6_state::video_data_w));
	map(0x31, 0x31).w(FUNC(kdt6_state::video_data_inc_w));
	map(0x36, 0x36).w(FUNC(kdt6_state::video_data_dec_w));
	map(0x37, 0x37).w(FUNC(kdt6_state::video_data_inc_w));
	map(0x38, 0x38).w(FUNC(kdt6_state::status1_w));
	map(0x39, 0x39).r(FUNC(kdt6_state::status1_r));
	map(0x3a, 0x3a).w(FUNC(kdt6_state::status2_w));
	map(0x3b, 0x3b).rw(FUNC(kdt6_state::sasi_ctrl_r), FUNC(kdt6_state::sasi_ctrl_w));
	map(0x3c, 0x3c).w(FUNC(kdt6_state::dma_map_w));
#if 0
	map(0x3d, 0x3d) WATCHDOG
	map(0x3e, 0x3e) WATCHDOG TRIGGER
	map(0x3f, 0x3f) SASI DATA
#endif
	map(0x40, 0x40).w(FUNC(kdt6_state::video_address_latch_high_w));
	map(0x41, 0x41).w(FUNC(kdt6_state::video_address_latch_low_w));
}


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

static void kdt6_floppies(device_slot_interface &device)
{
	device.option_add("fd55f", TEAC_FD_55F);
}

void kdt6_state::fdc_tc_w(uint8_t data)
{
	m_fdc->tc_w(1);
	m_fdc->tc_w(0);
}

void kdt6_state::fdc_drq_w(int state)
{
	if (!m_sasi_dma && BIT(m_status0, 4) == 0)
		m_dma->rdy_w(state);
}

void kdt6_state::drive0_led_cb(floppy_image_device *floppy, int state)
{
	m_drive_led[0] = state;
}

void kdt6_state::drive1_led_cb(floppy_image_device *floppy, int state)
{
	m_drive_led[1] = state;
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void kdt6_state::video_address_latch_high_w(uint8_t data)
{
	m_video_address &= 0x00ff;
	m_video_address |= (data << 8);
}

void kdt6_state::video_address_latch_low_w(uint8_t data)
{
	m_video_address &= 0xff00;
	m_video_address |= (data << 0);
}

uint8_t kdt6_state::video_data_r()
{
	return m_vram[m_video_address];
}

void kdt6_state::video_data_w(uint8_t data)
{
	m_vram[m_video_address] = ((m_status1 & 0x0c) << 6) | data;
}

void kdt6_state::video_data_inc_w(uint8_t data)
{
	m_vram[m_video_address++] = ((m_status1 & 0x0c) << 6) | data;
}

void kdt6_state::video_data_dec_w(uint8_t data)
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
	pen_t const *const pen = m_palette->pens();

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
				bitmap.pix(y, x + i*8) = pen[blink ? 0 : (color ^ inverse)];
			}
		}
		else
		{
			// gfx mode
			uint8_t data = m_vram[(ma << 4) | (ra << 6) | i];

			// draw 8 pixels of the cell
			for (int x = 0; x < 8; x++)
				bitmap.pix(y, x + i*8) = pen[BIT(data, 7 - x)];
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

void kdt6_state::pio_porta_w(uint8_t data)
{
	m_centronics->write_strobe(BIT(data, 0));
	m_centronics->write_init(BIT(data, 1));
}

void kdt6_state::keyboard_rx_w(int state)
{
	if (machine().phase() >= machine_phase::RESET)
	{
		if ((m_dip_s2->read() & 0x01) == 0x01)
			m_sio->rxb_w(state);
	}
}

void kdt6_state::rs232b_rx_w(int state)
{
	if (machine().phase() >= machine_phase::RESET)
	{
		if ((m_dip_s2->read() & 0x01) == 0x00)
			m_sio->rxb_w(state);
	}
}

void kdt6_state::siob_tx_w(int state)
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

uint8_t kdt6_state::memory_r(offs_t offset)
{
	return m_ram[m_dma_map << 16 | offset];
}

void kdt6_state::memory_w(offs_t offset, uint8_t data)
{
	m_ram[m_dma_map << 16 | offset] = data;
}

uint8_t kdt6_state::io_r(offs_t offset)
{
	return m_cpu->space(AS_IO).read_byte(offset);
}

void kdt6_state::io_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_IO).write_byte(offset, data);
}

uint8_t kdt6_state::page0_r(offs_t offset)
{
	if (BIT(m_status0, 5) == 0)
		return m_boot->as_u8(offset);

	return reinterpret_cast<uint8_t *>(m_page_r[0]->base())[offset];
}

uint8_t kdt6_state::page1_r(offs_t offset)
{
	if (BIT(m_status0, 5) == 0)
		return m_boot->as_u8(0x1000 + offset);

	return reinterpret_cast<uint8_t *>(m_page_r[1]->base())[offset];
}

uint8_t kdt6_state::sasi_ctrl_r()
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

void kdt6_state::sasi_ctrl_w(uint8_t data)
{
	logerror("sasi_ctrl_w: %02x\n", data);
	m_sasi_dma = bool(BIT(data, 2) == 0);
}

void kdt6_state::dma_map_w(uint8_t data)
{
	m_dma_map = data;
}

uint8_t kdt6_state::mapper_r(offs_t offset)
{
	return m_mapper[offset];
}

void kdt6_state::mapper_w(offs_t offset, uint8_t data)
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

void kdt6_state::status0_w(uint8_t data)
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

	for (auto &floppy : m_floppy)
		if (floppy->get_device())
			floppy->get_device()->mon_w(BIT(data, 7) ? 0 : 1);

	m_status0 = data;
}

uint8_t kdt6_state::status1_r()
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

void kdt6_state::status1_w(uint8_t data)
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

void kdt6_state::status2_w(uint8_t data)
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
	m_drive_led.resolve();

	// 256 kb ram, 64 kb vram (and two dummy regions for invalid pages)
	m_ram = std::make_unique<uint8_t[]>(0x40000);
	m_vram = std::make_unique<uint16_t[]>(0x10000);
	m_dummy_r = std::make_unique<uint8_t[]>(0x1000);
	m_dummy_w = std::make_unique<uint8_t[]>(0x1000);

	// override the region 0x0000 to 0x1fff here to enable prom reading
	m_cpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8sm_delegate(*this, FUNC(kdt6_state::page0_r)));
	m_cpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(kdt6_state::page1_r)));

	m_fdc->set_rate(250000);

	if (m_floppy[0]->get_device())
		m_floppy[0]->get_device()->setup_led_cb(floppy_image_device::led_cb(&kdt6_state::drive0_led_cb, this));
	if (m_floppy[1]->get_device())
		m_floppy[1]->get_device()->setup_led_cb(floppy_image_device::led_cb(&kdt6_state::drive1_led_cb, this));

	// register for save states
	save_pointer(NAME(m_ram), 0x40000);
	save_pointer(NAME(m_vram), 0x10000);
	save_item(NAME(m_sasi_dma));
	save_item(NAME(m_dma_map));
	save_item(NAME(m_status0));
	save_item(NAME(m_status1));
	save_item(NAME(m_status2));
	save_item(NAME(m_mapper));
	save_item(NAME(m_video_address));
}

void kdt6_state::machine_reset()
{
	// status0 is forced to 0 on reset
	status0_w(0);
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

void kdt6_state::psi98(machine_config &config)
{
	Z80(config, m_cpu, XTAL(16'000'000) / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &kdt6_state::psi98_mem);
	m_cpu->set_addrmap(AS_IO, &kdt6_state::psi98_io);
	m_cpu->set_daisy_config(daisy_chain_intf);
	m_cpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(XTAL(13'516'800), 824, 48, 688, 274, 0, 250);
	screen.set_screen_update(FUNC(kdt6_state::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);
	config.set_default_layout(layout_kdt6);

	MC6845(config, m_crtc, XTAL(13'516'800) / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(kdt6_state::crtc_update_row));
	m_crtc->out_vsync_callback().set("ctc2", FUNC(z80ctc_device::trg2));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1000); // frequency unknown
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.50);
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	TIMER(config, m_beep_timer).configure_generic(FUNC(kdt6_state::beeper_off));

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_busreq_callback().set_inputline(m_cpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set(FUNC(kdt6_state::memory_r));
	m_dma->out_mreq_callback().set(FUNC(kdt6_state::memory_w));
	m_dma->in_iorq_callback().set(FUNC(kdt6_state::io_r));
	m_dma->out_iorq_callback().set(FUNC(kdt6_state::io_w));

	// jumper J3 allows selection of 16MHz / 8 instead
	clock_device &uart_clk(CLOCK(config, "uart_clk", XTAL(9'830'400) / 8));
	uart_clk.signal_handler().set("ctc1", FUNC(z80ctc_device::trg1));
	uart_clk.signal_handler().append("ctc1", FUNC(z80ctc_device::trg2));

	z80ctc_device &ctc1(Z80CTC(config, "ctc1", 16_MHz_XTAL / 4));
	ctc1.intr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	ctc1.zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	ctc1.zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxca_w));
	ctc1.zc_callback<2>().append(m_sio, FUNC(z80sio_device::txca_w));

	z80ctc_device &ctc2(Z80CTC(config, "ctc2", 16_MHz_XTAL / 4));
	ctc2.intr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	ctc2.zc_callback<0>().set("speaker", FUNC(speaker_sound_device::level_w));
	ctc2.zc_callback<2>().set("ctc2", FUNC(z80ctc_device::trg3));

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4);
	m_sio->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(FUNC(kdt6_state::siob_tx_w));
	m_sio->out_dtrb_callback().set(m_rs232b, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_rs232b, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	rs232a.dsr_handler().set(m_sio, FUNC(z80sio_device::synca_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w)).invert();

	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);
	m_rs232b->rxd_handler().set(FUNC(kdt6_state::rs232b_rx_w));
	m_rs232b->dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	m_rs232b->dsr_handler().set(m_sio, FUNC(z80sio_device::syncb_w));
	m_rs232b->cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w)).invert();

	z80pio_device &pio(Z80PIO(config, "pio", 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	pio.out_pa_callback().set(FUNC(kdt6_state::pio_porta_w));
	pio.in_pb_callback().set("cent_data_in", FUNC(input_buffer_device::read));
	pio.out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->fault_handler().set("pio", FUNC(z80pio_device::pa2_w));
	m_centronics->perror_handler().set("pio", FUNC(z80pio_device::pa3_w));
	m_centronics->busy_handler().set("pio", FUNC(z80pio_device::pa4_w));
	m_centronics->select_handler().set("pio", FUNC(z80pio_device::pa5_w));

	INPUT_BUFFER(config, "cent_data_in");
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	UPD1990A(config, m_rtc);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set("ctc1", FUNC(z80ctc_device::trg0));
	m_fdc->drq_wr_callback().set(FUNC(kdt6_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", kdt6_floppies, "fd55f", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", kdt6_floppies, "fd55f", floppy_image_device::default_mfm_floppy_formats);

	SOFTWARE_LIST(config, "floppy_list").set_original("psi98");

	PSI_KEYBOARD_INTERFACE(config, m_keyboard, "hle");
	m_keyboard->rx().set(FUNC(kdt6_state::keyboard_rx_w));
	m_keyboard->key_strobe().set("ctc2", FUNC(z80ctc_device::trg1));

	// 6 ECB slots
}


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

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY    FULLNAME  FLAGS
COMP( 1984, psi98, 0,      0,      psi98,   psi98, kdt6_state, empty_init, "Kontron", "PSI98",  0 )
