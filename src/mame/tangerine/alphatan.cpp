// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tantel AlphaTantel


    Hardware:
    ---------
    CPU:    R6504

    RAM:    M6810
            M5101 - battery backed
            2114 x 2 - video RAM

    Video: SAA5020 VTC
           SAA5050 VCG
           SAA5070 VIOP
           TEA1002

    TODO:
    - implement SAA5070 (datasheet required)
    - keyboard matrix (probably via SAA5070)
    - cassette interface (via SAA5070)
    - program mode switch

**********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6504.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "video/tea1002.h"
#include "video/saa5050.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"

#include "emupal.h"
#include "screen.h"


namespace {

class alphatan_state : public driver_device
{
public:
	alphatan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_nvram(*this, "nvram")
		, m_screen(*this, "screen")
		, m_tea1002(*this, "encoder")
		, m_palette(*this, "palette")
		, m_trom(*this, "saa5050")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cassette(*this, "cassette")
		, m_config(*this, "CONFIG")
		{ }

	void alphatan(machine_config &config);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_device<tea1002_device> m_tea1002;
	required_device<palette_device> m_palette;
	required_device<saa5050_device> m_trom;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_config;

	void alphatan_map(address_map &map) ATTR_COLD;

	uint8_t saa5070_r(offs_t offset);
	void saa5070_w(offs_t offset, uint8_t data);
	void write400(offs_t offset, uint8_t data);
	void write600(offs_t offset, uint8_t data);

	void palette_init(palette_device &palette) const;
};

void alphatan_state::alphatan_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram().mirror(0x0180); // MCM6810
	map(0x0200, 0x027f).ram().share("nvram"); // MCM5101
	map(0x0300, 0x0300).rw(FUNC(alphatan_state::saa5070_r), FUNC(alphatan_state::saa5070_w)); // write 0x20, 0x14, 0x0c, 0x18, 0x40, 0x90, 0x1f, 0xd4, 0x0f
	map(0x0400, 0x0400).w(FUNC(alphatan_state::write400)); // write 0x03, 0x06, 0x08, 0x00, 0x04, 0x07, 0x01, 0x05
	map(0x0500, 0x0500).w(m_cent_data_out, FUNC(output_latch_device::write));
	map(0x0600, 0x0600).w(FUNC(alphatan_state::write600)); // unknown, writes only 0x01, 0x02, or 0x03
	map(0x0700, 0x0700).portr("CONFIG");
	map(0x0800, 0x0fff).ram().share("videoram"); // 2x2114
	map(0x1000, 0x1fff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( alphatan )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x10, 0x10, "Banner")
	PORT_CONFSETTING(0x00, "Granada")
	PORT_CONFSETTING(0x10, "Tantel")
INPUT_PORTS_END


uint8_t alphatan_state::saa5070_r(offs_t offset)
{
	uint8_t data = 0x00;

	//logerror("saa5070_r: %02x\n", data);

	return data;
}

void alphatan_state::saa5070_w(offs_t offset, uint8_t data)
{
	//logerror("saa5070_w: %02x\n", data);
}

void alphatan_state::write400(offs_t offset, uint8_t data)
{
	//logerror("write400: %02x\n", data);
}

void alphatan_state::write600(offs_t offset, uint8_t data)
{
	//logerror("write600: %02x\n", data);
}


void alphatan_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, m_tea1002->color(i));
}

uint32_t alphatan_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	m_trom->dew_w(1);
	m_trom->dew_w(0);

	for (int y = 0; y < 24 * 20; y++)
	{
		int sy = y / 20;
		int x = 0;

		m_trom->lose_w(1);
		m_trom->lose_w(0);

		int ssy = m_trom->tlc_r() ? sy : sy - 1;
		offs_t video_ram_addr = ssy * 64;

		for (int sx = 0; sx < 40; sx++)
		{
			uint8_t code = m_videoram[video_ram_addr++];

			m_trom->write(code & 0x7f);

			m_trom->f1_w(1);
			m_trom->f1_w(0);

			for (int bit = 0; bit < 12; bit++)
			{
				m_trom->tr6_w(1);
				m_trom->tr6_w(0);

				int color = m_trom->get_rgb();

				if (BIT(code, 7)) color ^= 0x07;

				bitmap.pix(y, x++) = palette[color];
			}
		}
	}

	return 0;
}


void alphatan_state::machine_start()
{
}


void alphatan_state::alphatan(machine_config &config)
{
	M6504(config, m_maincpu, 8.86_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &alphatan_state::alphatan_map);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//m_screen->set_raw(6_MHz_XTAL, 768, 0, 480, 625, 0, 480);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(768, 480);
	m_screen->set_visarea(0, 480-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(alphatan_state::screen_update));

	TEA1002(config, m_tea1002, 8.86_MHz_XTAL);
	PALETTE(config, m_palette, FUNC(alphatan_state::palette_init), 8, 8);

	SAA5050(config, m_trom, 6_MHz_XTAL);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
}


ROM_START(alphatan)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("tantel2732.bin", 0x0000, 0x1000, CRC(875c1ce9) SHA1(76e874cbc5d1341b09f57424181f24bdb31da210))
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE     INPUT      CLASS           INIT        COMPANY    FULLNAME        FLAGS
COMP( 1981, alphatan,  0,      0,      alphatan,   alphatan,  alphatan_state, empty_init, "Tantel",  "AlphaTantel",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
