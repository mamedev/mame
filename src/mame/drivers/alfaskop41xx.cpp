// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/***************************************************************************

Alfaskop 41 series

This driver is a part of a revivel project for Alfaskop 41 series where
no known working system exists today because of the distributed nature.
All parts network boots over SS3 (SDLC) from a Floppy Disk unit and nothing
works unless there is a floppy in that unit. These floppies are rare and
many parts have been discarded because they are useless stand alone.

The goal is to emulate missing parts so a full system can be demonstrated again.

Links and credits
-----------------
Project home page: https://github.com/MattisLind/alfaskop_emu
Dalby Datormusem - http://www.datormuseum.se/peripherals/terminals/alfaskop
Bitsavers - http://bitsavers.org/pdf/ericsson/alfaskop/
Dansk Datahistorisk Forening - http://datamuseum.dk/

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"
#include "video/mc6845.h"
#include "screen.h"

//#include "bus/rs232/rs232.h"
//#include "machine/clock.h"

#define LOG_IO    (1U << 1)
#define LOG_NVRAM (1U << 2)
#define LOG_MIC   (1U << 3)
#define LOG_DIA   (1U << 4)

//#define VERBOSE (LOG_IO|LOG_NVRAM|LOG_MIC|LOG_DIA)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGNVRAM(...) LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGMIC(...)   LOGMASKED(LOG_MIC,   __VA_ARGS__)
#define LOGDIA(...)   LOGMASKED(LOG_DIA,   __VA_ARGS__)

class alfaskop4110_state : public driver_device
{
public:
	alfaskop4110_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbdacia(*this, "kbdacia")
		, m_micpia(*this, "micpia")
		, m_diapia(*this, "diapia")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_chargen(*this, "chargen")
	{ }

	void alfaskop4110(machine_config &config);
private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_kbdacia;
	required_device<pia6821_device> m_micpia;
	required_device<pia6821_device> m_diapia;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;

	/* Video controller */
	required_region_ptr<uint8_t> m_chargen;
	MC6845_UPDATE_ROW(crtc_update_row);
};

class alfaskop4120_state : public driver_device
{
public:
	alfaskop4120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_micpia(*this, "micpia")
		, m_fdapia(*this, "diapia")
	{ }

	void alfaskop4120(machine_config &config);
private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_micpia;
	required_device<pia6821_device> m_fdapia;
};

class alfaskop4101_state : public driver_device
{
public:
	alfaskop4101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_micpia(*this, "micpia")
	{ }

	void alfaskop4101(machine_config &config);
private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_micpia;
};

void alfaskop4110_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x7800, 0x7fff).ram().share(m_vram);
	map(0x8000, 0xefff).ram();

	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }),// TODO: Move to MRO board
				 NAME( [this](offs_t offset, uint8_t data) {    LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	map(0xf7d9, 0xf7d9).mirror(0x06).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("CRTC reg r %04x: %02x\n", offset, 0); return m_crtc->register_r(); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("CRTC reg w %04x: %02x\n", offset, data); m_crtc->register_w(data);}));
	map(0xf7d8, 0xf7d8).mirror(0x06).lw8(NAME([this](offs_t offset, uint8_t data) { LOGIO("CRTC adr w %04x: %02x\n", offset, data); m_crtc->address_w(data); }));
	map(0xf7d0, 0xf7d3).mirror(0x04).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("DIA pia_r %04x: %02x\n", offset, 0); return m_diapia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("DIA pia_w %04x: %02x\n", offset, data); m_diapia->write(offset & 3, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("MIC pia_r %04x: %02x\n", offset, 0); return m_micpia->read(offset & 3); }),
						  NAME( [this](offs_t offset, uint8_t data) { LOGIO("MIC pia_w %04x: %02x\n", offset, data); m_micpia->write(offset & 3, data); }));
	map(0xf7c0, 0xf7c1).mirror(0x02).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("KBD acia_r %04x: %02x\n", offset, 0); return m_kbdacia->read(offset & 1); }),
						  NAME( [this](offs_t offset, uint8_t data) { LOGIO("KBD acia_w %04x: %02x\n", offset, data); m_kbdacia->write(offset & 1, data); }));

		map(0xf7fc, 0xf7fc).mirror(0x00).lr8(NAME([this](offs_t offset) -> uint8_t { LOGIO("Address Switch 0-7\n"); return 0; }));

	map(0xf800, 0xffff).rom().region("roms", 0);
}

void alfaskop4120_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }), // TODO: Move to MRO board
				 NAME([this](offs_t offset, uint8_t data) { LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	map(0xf740, 0xf743).mirror(0x0c).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("FDA pia_r %04x: %02x\n", offset, 0); return m_fdapia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("FDA pia_w %04x: %02x\n", offset, data); m_fdapia->write(offset & 3, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("MIC pia_r %04x: %02x\n", offset, 0); return m_micpia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("MIC pia_w %04x: %02x\n", offset, data); m_micpia->write(offset & 3, data); }));
	map(0xf800, 0xffff).rom().region("roms", 0);
}

void alfaskop4101_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf600, 0xf6ff).lrw8(NAME([this](offs_t offset) -> uint8_t { LOGNVRAM("nvram_r %04x: %02x\n", offset, 0); return (uint8_t) 0; }),
				 NAME([this](offs_t offset, uint8_t data) { LOGNVRAM("nvram_w %04x: %02x\n", offset, data); }));
	map(0xf7c4, 0xf7c7).mirror(0x00).lrw8(NAME([this](offs_t offset) -> uint8_t    { LOGIO("MIC pia_r %04x: %02x\n", offset, 0); return m_micpia->read(offset & 3); }),
						  NAME([this](offs_t offset, uint8_t data) { LOGIO("MIC pia_w %04x: %02x\n", offset, data); m_micpia->write(offset & 3, data); }));
	map(0xf800, 0xffff).rom().region("roms", 0);
}

/* Input ports */
static INPUT_PORTS_START( alfaskop4110 )
INPUT_PORTS_END

static INPUT_PORTS_START( alfaskop4120 )
INPUT_PORTS_END

static INPUT_PORTS_START( alfaskop4101 )
INPUT_PORTS_END

/* Simplified chargen, no attributes or special formats/features yet  */
MC6845_UPDATE_ROW( alfaskop4110_state::crtc_update_row )
{
	offs_t base = ma + 0x4000;
	u32 *px = &bitmap.pix32(y);

	for (int i = 0; i < x_count; i++)
	{
		u8 chr = m_vram[(base + i) & 0x07ff] & 0x7f;
		rgb_t bg = rgb_t::white();
		rgb_t fg = rgb_t::black();

		u8 dots = m_chargen[chr * 16 + ra];

		for (int n = 8; n > 0; n--, dots <<= 1)
			*px++ = BIT(dots, 7) ? fg : bg;
	}
}

void alfaskop4110_state::alfaskop4110(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4110_state::mem_map);

	MC6845(config, m_crtc, XTAL(19'170'000) / 9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(alfaskop4110_state::crtc_update_row));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(19'170'000, 80 * 8, 0, 80 * 8, 400, 0, 400);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PIA6821(config, m_micpia, 0); // Main board PIA
	m_micpia->readcb1_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("MIC PIA: CB1_r\n"); return 0;});
	m_micpia->cb2_handler().set([this](offs_t offset, uint8_t data) { LOGMIC("MIC PIA: CB2_w\n"); });
	m_micpia->writepa_handler().set([this](offs_t offset, uint8_t data) { LOGMIC("MIC PIA: PA_w\n"); });
	m_micpia->writepb_handler().set([this](offs_t offset, uint8_t data) { LOGMIC("MIC PIA: PB_w\n"); });
	m_micpia->readpa_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("MIC PIA: PA_r\n"); return 0;});
	m_micpia->readpb_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("MIC PIA: PB_r\n"); return 0;});
	m_micpia->readca1_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("MIC PIA: CA1_r\n"); return 0;});
	m_micpia->readca2_handler().set([this](offs_t offset) -> uint8_t { LOGMIC("MIC PIA: CA2_r\n"); return 0;});

	PIA6821(config, m_diapia, 0); // Display PIA, controls how the CRTC accesses memory etc
	m_diapia->readcb1_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: CB1_r\n"); return 0;});
	m_diapia->cb2_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: CB2_w\n"); });
	m_diapia->writepa_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: PA_w\n"); });
	m_diapia->writepb_handler().set([this](offs_t offset, uint8_t data) { LOGDIA("DIA PIA: PB_w\n"); });
	m_diapia->readpa_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: PA_r\n"); return 0;});
	m_diapia->readpb_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: PB_r\n"); return 0;});
	m_diapia->readca1_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: CA1_r\n"); return 0;});
	m_diapia->readca2_handler().set([this](offs_t offset) -> uint8_t { LOGDIA("DIA PIA: CA2_r\n"); return 0;});

	ACIA6850(config, m_kbdacia, 0);
	//CLOCK(config, "acia_clock", ACIA_CLOCK).signal_handler().set(FUNC(alfaskop4110_state::write_acia_clock));
}

void alfaskop4120_state::alfaskop4120(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4120_state::mem_map);

	PIA6821(config, m_micpia, 0); // Main board PIA
	PIA6821(config, m_fdapia, 0); // Floppy Disk PIA
}

void alfaskop4101_state::alfaskop4101(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(19'170'000) / 18); // Verified from service manual
	m_maincpu->set_addrmap(AS_PROGRAM, &alfaskop4101_state::mem_map);

	PIA6821(config, m_micpia, 0); // Main board PIA
}

/* ROM definitions */
ROM_START( alfaskop4110 ) // Display Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "e3405870205201.bin", 0x0000, 0x0800, CRC(23f20f7f) SHA1(6ed008e309473ab966c6b0d42a4f87c76a7b1d6e))
	ROM_REGION( 0x800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "e3405972067500.bin", 0x0000, 0x0400, CRC(fb12b549) SHA1(53783f62c5e51320a53e053fbcf8b3701d8a805f))
	ROM_LOAD( "e3405972067600.bin", 0x0400, 0x0400, CRC(c7069d65) SHA1(587efcbee036d4c0c5b936cc5d7b1f97b6fe6dba))
ROM_END

ROM_START( alfaskop4120 ) // Flexible Disk Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "alfaskop4120.bin", 0x0000, 0x0800, NO_DUMP)
ROM_END

ROM_START( alfaskop4101 ) // Communication Processor Unit
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "alfaskop4101.bin", 0x0000, 0x0800, NO_DUMP)
ROM_END

/* Driver(S) */

// Only 4101 may exist as a driver in the end making the 4110 and 4120 as slots devices on the SS3 bus, time will tell

//    YEAR  NAME          PARENT  COMPAT  MACHINE       INPUT         CLASS               INIT        COMPANY      FULLNAME       FLAGS
COMP( 1984, alfaskop4110, 0,      0,      alfaskop4110, alfaskop4110, alfaskop4110_state, empty_init, "Ericsson",  "Alfaskop Display Unit 4110", MACHINE_IS_SKELETON)
COMP( 1984, alfaskop4120, 0,      0,      alfaskop4120, alfaskop4120, alfaskop4120_state, empty_init, "Ericsson",  "Alfaskop Flexible Disk Unit 4120", MACHINE_IS_SKELETON)
COMP( 1984, alfaskop4101, 0,      0,      alfaskop4101, alfaskop4101, alfaskop4101_state, empty_init, "Ericsson",  "Alfaskop Communication Processor 4101", MACHINE_IS_SKELETON)
