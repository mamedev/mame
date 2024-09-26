// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    http://www.computinghistory.org.uk/det/52264/HH-Tiger/

   SPECIFICATION
      Processors: Z80A (4MHz) plus 6809 I/O processor (2MHz) plus 7220
                  graphics processor.
             RAM: 64K with Z80, 2K with 6809, 96K with 7220, plus 0.5K CMOS
                  RAM with battery back-up. 62K available to CP/M.
             ROM: 2K with Z80, 16K with 6809.
     Text screen: 80 x 24 or teletext-type 40 x 24, 8 colours.
  Graphic screen: 512 x 512 pixels, 8 colours.
        Keyboard: 88 keys in unit with CPU, numeric/cursor pad. 10 function
                  keys.
         Storage: 2 x 1 MB (unformatted) floppies
      Interfaces: RS232, Centronics parallel, IEEE488, networking, light pen,
                  cassette, disk expansion (5.25" or 8"), optional UHF or
                  additional video monitor
Operating system: CP/M

    TODO:
    - nothing is known so emulation is very incomplete.

**********************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "machine/ram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "cpu/m6809/m6809.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/6522via.h"
#include "machine/mc6854.h"
#include "machine/mos6551.h"
#include "machine/tms9914.h"
#include "sound/spkrdev.h"
#include "video/upd7220.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"

// Debugging
#define VERBOSE 1
#include "logmacro.h"


namespace {

class hhtiger_state : public driver_device
{
public:
	hhtiger_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "z80")
		, m_rom_z80(*this, "rom_z80")
		, m_ram(*this, RAM_TAG)
		, m_dma(*this, "dma")
		, m_pio(*this, "pio")
		, m_subcpu(*this, "m6809")
		, m_rom_m6809(*this, "rom_m6809")
		, m_screen(*this, "screen")
		, m_gdc(*this, "upd7220")
		, m_palette(*this, "palette")
		, m_video_ram(*this, "video_ram")
		, m_nvram(*this, "nvram")
		, m_via(*this, "via%u", 0)
		, m_adlc(*this, "adlc")
		, m_acia(*this, "acia")
		, m_ieee(*this, IEEE488_TAG)
		, m_tms9914(*this, "hpib")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_irqs(*this, "irqs")
		, m_rom_mirror(true)
	{ }

	void hhtiger(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	uint8_t disable_rom_r();
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	UPD7220_DISPLAY_PIXELS_MEMBER(display_pixels);
	UPD7220_DRAW_TEXT_LINE_MEMBER(draw_text);

	/* handlers for logging only, can be removed when more is known */
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);
	void pio_pb_w(uint8_t data);
	void ardy_w(int state);
	void brdy_w(int state);

	uint8_t via_0_in_a();
	void via_0_out_a(uint8_t data);
	void via_0_out_b(uint8_t data);
	void via_0_out_ca2(int state);
	void via_0_out_cb2(int state);

	void via_1_out_a(uint8_t data);
	void via_1_out_b(uint8_t data);
	void via_1_out_ca2(int state);
	void via_1_out_cb2(int state);

	void z80_mem(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void m6809_mem(address_map &map) ATTR_COLD;
	void upd7220_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_memory_region m_rom_z80;
	required_device<ram_device> m_ram;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;

	required_device<mc6809_device> m_subcpu;
	required_memory_region m_rom_m6809;
	required_device<screen_device> m_screen;
	required_device<upd7220_device> m_gdc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_video_ram;
	required_device<nvram_device> m_nvram;
	required_device_array<via6522_device, 2> m_via;
	required_device<mc6854_device> m_adlc;
	required_device<mos6551_device> m_acia;
	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<input_merger_device> m_irqs;

	bool m_rom_mirror;
};


void hhtiger_state::machine_start()
{
	save_item(NAME(m_rom_mirror));
}


void hhtiger_state::machine_reset()
{
	m_rom_mirror = true;
}


static const gfx_layout hhtiger_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START(gfx_hhtiger)
	GFXDECODE_ENTRY("rom_m6809", 0x2000, hhtiger_charlayout, 0, 1)
GFXDECODE_END


UPD7220_DISPLAY_PIXELS_MEMBER(hhtiger_state::display_pixels)
{
	/* 96KB video RAM (32KB green + 32KB red + 32KB blue) */
	uint16_t const green = m_video_ram[(0x00000 + (address & 0x3fff))];
	uint16_t const red   = m_video_ram[(0x04000 + (address & 0x3fff))];
	uint16_t const blue  = m_video_ram[(0x08000 + (address & 0x3fff))];

	for (int xi = 0; xi<16; xi++)
	{
		int const r = BIT(red,   xi) ? 255 : 0;
		int const g = BIT(green, xi) ? 255 : 0;
		int const b = BIT(blue,  xi) ? 255 : 0;

		if (bitmap.cliprect().contains(x + xi, y))
			bitmap.pix(y, x + xi) = rgb_t(r, g, b);
	}
}

UPD7220_DRAW_TEXT_LINE_MEMBER(hhtiger_state::draw_text)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	for (int x = 0; x < pitch; x++)
	{
		uint8_t tile = m_video_ram[(((addr + x) * 2) & 0x1ffff) >> 1] & 0xff;
		uint8_t attr = m_video_ram[(((addr + x) * 2) & 0x1ffff) >> 1] >> 8;

		for (int yi = 0; yi < lr; yi++)
		{
			uint8_t tile_data = m_rom_m6809->base()[0x1000 | ((tile * 8 + yi) & 0x7ff)];

			if (cursor_on && cursor_addr == addr + x) //TODO
				tile_data ^= 0xff;

			for (int xi = 0; xi < 8; xi++)
			{
				int pen = (tile_data >> xi) & 1 ? (attr & 0x07) : 0;

				int const res_x = x * 8 + xi;
				int const res_y = y + yi;

				if (yi >= 8)
					pen = 0;

				if (!m_screen->visible_area().contains(res_x, res_y))
					bitmap.pix(res_y, res_x) = palette[pen];
			}
		}
	}
}


uint8_t hhtiger_state::disable_rom_r()
{
	if (!machine().side_effects_disabled())
		m_rom_mirror = false;

	return 0xff;
}

uint8_t hhtiger_state::read(offs_t offset)
{
	uint8_t data;

	if (m_rom_mirror)
	{
		data = m_rom_z80->base()[offset & 0x0fff];
	}
	else
	{
		switch (offset & 0xf800)
		{
		case 0xf800:
			data = m_rom_z80->base()[offset & 0x0fff];
			break;
		default:
			data = m_ram->pointer()[offset];
			break;
		}
	}

	return data;
}

void hhtiger_state::write(offs_t offset, uint8_t data)
{
	m_ram->pointer()[offset] = data;
}


uint8_t hhtiger_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void hhtiger_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

uint8_t hhtiger_state::io_read_byte(offs_t offset)
{
	address_space& io_space = m_maincpu->space(AS_IO);
	return io_space.read_byte(offset);
}

void hhtiger_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& io_space = m_maincpu->space(AS_IO);
	io_space.write_byte(offset, data);
}


//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

void hhtiger_state::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(hhtiger_state::read), FUNC(hhtiger_state::write));
}

void hhtiger_state::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x04, 0x07).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x10, 0x17).rw(m_tms9914, FUNC(tms9914_device::read), FUNC(tms9914_device::write)); // ??
	map(0x1e, 0x1e).r(FUNC(hhtiger_state::disable_rom_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( m6809_mem )
//-------------------------------------------------

void hhtiger_state::m6809_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0xb800, 0xb9ff).ram().share("nvram");
	map(0xbfa0, 0xbfa3).rw(m_gdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0xbfb0, 0xbfb3).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write)); // ??
	map(0xbfc0, 0xbfcf).m(m_via[1], FUNC(via6522_device::map));
	map(0xbfd0, 0xbfd3).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // ??
	map(0xbfe0, 0xbfef).m(m_via[0], FUNC(via6522_device::map));
	map(0xc000, 0xffff).rom().region("rom_m6809", 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( upd7220_map )
//-------------------------------------------------

void hhtiger_state::upd7220_map(address_map &map)
{
	map.global_mask(0x17fff);
	map(0x00000, 0x17fff).ram().share("video_ram");
}


static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio" },
	{ nullptr }
};


static INPUT_PORTS_START(hhtiger)
INPUT_PORTS_END


uint8_t hhtiger_state::pio_pa_r()
{
	uint8_t data = 0xff;

	LOG("pio_pa_r %02X\n", data);
	return data;
}

void hhtiger_state::pio_pa_w(uint8_t data)
{
	LOG("pio_pa_w %02X\n", data);
	m_via[0]->write_pa(data);
}

void hhtiger_state::pio_pb_w(uint8_t data)
{
	LOG("pio_pb_w %02X\n", data);
	m_via[0]->write_pb(data);
}

void hhtiger_state::ardy_w(int state)
{
	LOG("ardy_w %d\n", state);
	m_via[0]->write_ca1(state);
}

void hhtiger_state::brdy_w(int state)
{
	LOG("brdy_w %d\n", state);
	m_via[0]->write_cb1(state);
}


uint8_t hhtiger_state::via_0_in_a()
{
	uint8_t data = 0xff;

	LOG("via0_in_a %02X\n", data);
	data = m_pio->port_a_read();
	return data;
}

void hhtiger_state::via_0_out_a(uint8_t data)
{
	LOG("via0_out_a %02X\n", data);
	m_pio->port_a_write(data);
}

void hhtiger_state::via_0_out_b(uint8_t data)
{
	LOG("via0_out_b %02X\n", data);
}

void hhtiger_state::via_0_out_ca2(int state)
{
	LOG("via0_out_ca2 %d\n", state);
	m_pio->strobe_a(state);
}

void hhtiger_state::via_0_out_cb2(int state)
{
	LOG("via0_out_cb2 %d\n", state);
}


void hhtiger_state::via_1_out_a(uint8_t data)
{
	LOG("via1_out_a %02X\n", data);
}

void hhtiger_state::via_1_out_b(uint8_t data)
{
	LOG("via1_out_b %02X\n", data);
}

void hhtiger_state::via_1_out_ca2(int state)
{
	LOG("via1_out_ca2 %d\n", state);
}

void hhtiger_state::via_1_out_cb2(int state)
{
	LOG("via1_out_cb2 %d\n", state);
}


static void hhtiger_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}


void hhtiger_state::hhtiger(machine_config &config)
{
	/* main cpu z80a */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &hhtiger_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &hhtiger_state::z80_io);
	m_maincpu->set_daisy_config(daisy_chain_intf);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->in_mreq_callback().set(FUNC(hhtiger_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(hhtiger_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(hhtiger_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(hhtiger_state::io_write_byte));

	Z80PIO(config, m_pio, 16_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(hhtiger_state::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(hhtiger_state::pio_pa_w));
	m_pio->out_ardy_callback().set(FUNC(hhtiger_state::ardy_w));
	m_pio->out_pb_callback().set(FUNC(hhtiger_state::pio_pb_w));
	m_pio->out_brdy_callback().set(FUNC(hhtiger_state::brdy_w));

	RAM(config, m_ram).set_default_size("64K");

	/* unknown fdc - floppy drives are housed with monitor so maybe fdc is external */
	FLOPPY_CONNECTOR(config, "0", hhtiger_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "1", hhtiger_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "2", hhtiger_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* unknown sound hardware - maybe connected to square-wave timer output from via */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* io cpu 6809 */
	MC6809(config, m_subcpu, 16_MHz_XTAL / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &hhtiger_state::m6809_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs);
	m_irqs->output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(16_MHz_XTAL, 520, 0, 320, 308, 0, 240);
	m_screen->set_screen_update("upd7220", FUNC(upd7220_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_hhtiger);
	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	UPD7220(config, m_gdc, 16_MHz_XTAL / 16);
	m_gdc->set_addrmap(0, &hhtiger_state::upd7220_map);
	m_gdc->set_display_pixels(FUNC(hhtiger_state::display_pixels));
	m_gdc->set_draw_text(FUNC(hhtiger_state::draw_text));
	m_gdc->set_screen(m_screen);

	MOS6522(config, m_via[0], 16_MHz_XTAL / 16);
	m_via[0]->readpa_handler().set(FUNC(hhtiger_state::via_0_in_a));
	m_via[0]->writepa_handler().set(FUNC(hhtiger_state::via_0_out_a));
	m_via[0]->writepb_handler().set(FUNC(hhtiger_state::via_0_out_b));
	m_via[0]->ca2_handler().set(FUNC(hhtiger_state::via_0_out_ca2));
	m_via[0]->cb2_handler().set(FUNC(hhtiger_state::via_0_out_cb2));
	m_via[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	MOS6522(config, m_via[1], 16_MHz_XTAL / 16);
	m_via[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_via[1]->writepa_handler().set(FUNC(hhtiger_state::via_1_out_a));
	m_via[1]->writepb_handler().set(FUNC(hhtiger_state::via_1_out_b));
	m_via[1]->ca2_handler().set(FUNC(hhtiger_state::via_1_out_ca2));
	m_via[1]->cb2_handler().set(FUNC(hhtiger_state::via_1_out_cb2));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via[1], FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	MC6854(config, m_adlc);
	m_adlc->out_irq_cb().set("irqs", FUNC(input_merger_device::in_w<2>));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set("irqs", FUNC(input_merger_device::in_w<3>));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	TMS9914(config, m_tms9914, 16_MHz_XTAL / 4);
	m_tms9914->int_write_cb().set("irqs", FUNC(input_merger_device::in_w<4>));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	IEEE488(config, m_ieee);
	m_ieee->eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	m_ieee->dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	m_ieee->nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	m_ieee->ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	m_ieee->ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	m_ieee->srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	m_ieee->atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	m_ieee->ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_dev", 0, cbm_ieee488_devices, nullptr);
}


ROM_START(hhtiger)
	ROM_REGION(0x1000, "rom_z80", 0)
	ROM_DEFAULT_BIOS("rel13")
	ROM_SYSTEM_BIOS(0, "rel12", "Rel1.2")
	ROMX_LOAD("rel1.2-0ea0.ic79", 0x0000, 0x1000, CRC(2f81e48c) SHA1(a4a1d7fde9f92abd6d8f8a1c24e35d713a5cbcb2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rel13", "Rel1.3")
	ROMX_LOAD("rel1.3-0ea0.ic79", 0x0000, 0x1000, CRC(2f81e48c) SHA1(a4a1d7fde9f92abd6d8f8a1c24e35d713a5cbcb2), ROM_BIOS(1))
	/* Rel1.4 also known to exist */
	ROM_REGION(0x4000, "rom_m6809", 0)
	ROMX_LOAD("rel1.2-cfd3.ic16", 0x0000, 0x2000, CRC(1ae4d2e0) SHA1(c379c5f1be24835ae4b4bd7bed35800faa3a9af6), ROM_BIOS(0))
	ROMX_LOAD("rel1.2-8fd2.ic15", 0x2000, 0x2000, CRC(0ef23968) SHA1(d46ce3b965ce51d0c90a10121661199b51e33d8b), ROM_BIOS(0))
	ROMX_LOAD("rel1.3-789a.ic16", 0x0000, 0x2000, CRC(7fa6fd33) SHA1(0b2768c170ca7077ef5164bfa13d9bf033528115), ROM_BIOS(1))
	ROMX_LOAD("rel1.3-77c1.ic15", 0x2000, 0x2000, CRC(dd2f15d5) SHA1(139a2b97cb8c27a50e3bfa3f42a9572203e453e0), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME     FLAGS */
COMP( 1983, hhtiger, 0,      0,       hhtiger, hhtiger, hhtiger_state, empty_init, "H/H Microcomputers", "H/H Tiger", MACHINE_IS_SKELETON )

