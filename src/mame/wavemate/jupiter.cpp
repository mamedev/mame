// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************

Wave Mate Jupiter


Jupiter 2
*********
Status: Preliminary
Doesn't show anything until a disk is loaded



Jupiter 3
*********
Status: Preliminary
Hangs if your input line starts with 'k'.



ToDo: (both)
- Connect all devices
- Everything!

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/keyboard.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define MCM6571AP_TAG   "vid125_6c"
#define S6820_TAG       "vid125_4a"
#define Z80_TAG         "cpu126_4c"
#define INS1771N1_TAG   "fdi027_4c"
#define MC6820P_TAG     "fdi027_4b"
#define MC6850P_TAG     "rsi068_6a"
#define MC6821P_TAG     "sdm058_4b"

class jupiter2_state : public driver_device
{
public:
	jupiter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MCM6571AP_TAG)
		, m_acia0(*this, "acia0")
		, m_acia1(*this, "acia1")
	{ }

	void jupiter2(machine_config &config);

	void init_jupiter2();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void jupiter2_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
};

class jupiter3_state : public driver_device
{
public:
	jupiter3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_p_videoram(*this, "videoram")
		, m_p_ram(*this, "ram")
		, m_p_chargen(*this, "chargen")
	{ }

	void jupiter3(machine_config &config);

	void init_jupiter3();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kbd_put(u8 data);
	uint8_t status_r();
	uint8_t key_r();
	uint8_t ff_r();

	void jupiter3_io(address_map &map) ATTR_COLD;
	void jupiter3_mem(address_map &map) ATTR_COLD;

	uint8_t m_term_data = 0U;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_ram;
	required_region_ptr<u8> m_p_chargen;
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( jupiter_m6800_mem )
//-------------------------------------------------

void jupiter2_state::jupiter2_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0xc000, 0xcfff).ram();  // Video RAM
	map(0xf000, 0xff00).rom().region(MCM6571AP_TAG, 0);
//  map(0xff58, 0xff5c) Cartridge Disk Controller PIA
//  map(0xff60, 0xff76) DMA Controller
//  map(0xff80, 0xff83) Floppy PIA
	map(0xff84, 0xff87).rw(INS1771N1_TAG, FUNC(wd_fdc_device_base::read), FUNC(wd_fdc_device_base::write));
//  map(0xff90, 0xff93) Hytype Parallel Printer PIA
//  map(0xffa0, 0xffa7) Persci Floppy Disk Controller
//  map(0xffb0, 0xffb3) Video PIA
	map(0xffc0, 0xffc1).rw(m_acia0, FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Serial Port 0 ACIA
	map(0xffc4, 0xffc5).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // Serial Port 1 ACIA
//  map(0xffc8, 0xffc9) Serial Port 2 ACIA
//  map(0xffcc, 0xffcd) Serial Port 3 ACIA
//  map(0xffd0, 0xffd1) Serial Port 4 ACIA / Cassette
//  map(0xffd4, 0xffd5) Serial Port 5 ACIA / EPROM Programmer (2704/2708)
//  map(0xffd8, 0xffd9) Serial Port 6 ACIA / Hardware Breakpoint Registers
//  map(0xffdc, 0xffdd) Serial Port 7 ACIA
	map(0xfff8, 0xffff).rom().region(MCM6571AP_TAG, 0x0ff8); // vectors
}



//-------------------------------------------------
//  ADDRESS_MAP( jupiter3_mem )
//-------------------------------------------------

void jupiter3_state::jupiter3_mem(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("ram");
	map(0xc000, 0xdfff).ram().share("videoram");
	map(0xe000, 0xefff).rom().region(Z80_TAG, 0);
	map(0xf000, 0xffff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( jupiter3_io )
//-------------------------------------------------

void jupiter3_state::jupiter3_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xa1, 0xa4).r(FUNC(jupiter3_state::ff_r));
	map(0xb0, 0xb0).r(FUNC(jupiter3_state::status_r));
	map(0xb2, 0xb2).r(FUNC(jupiter3_state::key_r));
}

uint8_t jupiter3_state::ff_r()
{
	return 0xfd;
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( jupiter )
//-------------------------------------------------

static INPUT_PORTS_START( jupiter )
INPUT_PORTS_END

uint8_t jupiter3_state::key_r()
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

uint8_t jupiter3_state::status_r()
{
	return (m_term_data) ? 0x80 : 0x00;
}

void jupiter3_state::kbd_put(u8 data)
{
	if (data)
		m_term_data = data ^ 0x80;
}


//**************************************************************************
//  VIDEO
//**************************************************************************

uint32_t jupiter3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 32; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 64; x++)
			{
				uint8_t gfx = 0;
				if (ra < 9)
				{
					uint8_t chr = m_p_videoram[x];
					gfx = m_p_chargen[(chr<<4) | ra ];
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}




//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static void jupiter_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( jupiter )
//-------------------------------------------------

void jupiter2_state::machine_start()
{
}


//-------------------------------------------------
//  MACHINE_START( jupiter3 )
//-------------------------------------------------

void jupiter3_state::machine_reset()
{
	uint8_t* ROM = memregion(Z80_TAG)->base();
	memcpy(m_p_ram, ROM, 0x1000);
	m_maincpu->set_pc(0xe000);

	m_term_data = 0;
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( jupiter )
//-------------------------------------------------

void jupiter2_state::jupiter2(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jupiter2_state::jupiter2_mem);

	// devices
	FD1771(config, INS1771N1_TAG, 1000000);
	FLOPPY_CONNECTOR(config, INS1771N1_TAG":0", jupiter_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, INS1771N1_TAG":1", jupiter_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	ACIA6850(config, m_acia0, XTAL(2'000'000)); // unknown frequency
	m_acia0->txd_handler().set("serial0", FUNC(rs232_port_device::write_txd));
	m_acia0->rts_handler().set("serial0", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial0(RS232_PORT(config, "serial0", default_rs232_devices, "terminal"));
	serial0.rxd_handler().set(m_acia0, FUNC(acia6850_device::write_rxd));
	serial0.cts_handler().set(m_acia0, FUNC(acia6850_device::write_cts));

	ACIA6850(config, m_acia1, XTAL(2'000'000)); // unknown frequency
	m_acia1->txd_handler().set("serial1", FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set("serial1", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, "terminal"));
	serial1.rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	serial1.cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( jupiter3 )
//-------------------------------------------------

void jupiter3_state::jupiter3(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jupiter3_state::jupiter3_mem);
	m_maincpu->set_addrmap(AS_IO, &jupiter3_state::jupiter3_io);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(jupiter3_state::screen_update));
	screen.set_size(512, 320);
	screen.set_visarea(0, 512-1, 0, 320-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// devices
	FD1771(config, INS1771N1_TAG, 1000000);
	FLOPPY_CONNECTOR(config, INS1771N1_TAG":0", jupiter_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, INS1771N1_TAG":1", jupiter_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(jupiter3_state::kbd_put));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( jupiter2 )
//-------------------------------------------------

ROM_START( jupiter2 )
	ROM_REGION( 0x1000, MCM6571AP_TAG, ROMREGION_INVERT ) // address and data lines are inverted
	ROM_LOAD( "idb v1.1 for 60k jii f000.1c", 0x0000, 0x0400, CRC(50893aae) SHA1(da0222c4cb6188f6cfc657fc33558d0a6a41cd1a) )
	ROM_LOAD( "idb v1.1 for 60k jii f400.6c", 0x0400, 0x0400, CRC(a435344a) SHA1(bc4f4143301b10ec762ecc0cb69e5a9d4c4bef7b) )
	ROM_LOAD( "idb v1.1 for 60k jii f800.1d", 0x0800, 0x0400, CRC(ab82df45) SHA1(be7ea5347ff0582401e26c2fa10e13463cbe57c6) )
	ROM_LOAD( "boot_v2.6_sn5d00000000000003_fc00.6d", 0x0c00, 0x0400, CRC(8f33e4ed) SHA1(fb206e5019c166583ff516de3608ae86d2636d2a) )
	ROM_LOAD( "jupiter ii boot rom v2.6 12_18_82 s_n 5d000...0015.6d", 0x0c00, 0x0400, CRC(f87cefdf) SHA1(229ea961e6036ec39e0ae33abc7f554bf9d8361b) )
ROM_END


//-------------------------------------------------
//  ROM( jupiter3 )
//-------------------------------------------------

ROM_START( jupiter3 )
	ROM_REGION( 0x1000, Z80_TAG, ROMREGION_INVERT ) // address and data lines are inverted
	ROM_LOAD( "jove 2.0 78_034 4v2d000 1.1c", 0x0000, 0x0400, CRC(be92a76c) SHA1(9c7d9b37c2bbf0c2e9465421e3e1bcf3dd9e66a6) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 2.6c", 0x0400, 0x0400, CRC(ee98dd32) SHA1(0513261c7c0d911225ea957ee67394871a36ada4) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 3.1d", 0x0800, 0x0400, CRC(51476b1d) SHA1(ab6f4eb244bcf9718aafdae67da086ec81f33fa6) )
	ROM_LOAD( "jove 2.0 78_034 4v2d000 4.6d", 0x0c00, 0x0400, CRC(16a9595d) SHA1(06150278650590497732e1f3f42356de56737921) )

	// character generator is missing, using one from c10 for now
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( jupiter )
//-------------------------------------------------

void jupiter2_state::init_jupiter2()
{
	uint8_t *rom = memregion(MCM6571AP_TAG)->base();
	uint8_t inverted[0x1000];

	memcpy(inverted, rom, 0x1000);

	for (offs_t addr = 0; addr < 0x400; addr++)
	{
		// invert address lines
		rom[0x3ff - addr] = inverted[addr];
		rom[0x7ff - addr] = inverted[addr + 0x400];
		rom[0xbff - addr] = inverted[addr + 0x800];
		rom[0xfff - addr] = inverted[addr + 0xc00];
	}
}


//-------------------------------------------------
//  DRIVER_INIT( jupiter3 )
//-------------------------------------------------

void jupiter3_state::init_jupiter3()
{
	uint8_t *rom = memregion(Z80_TAG)->base();
	uint8_t inverted[0x1000];

	memcpy(inverted, rom, 0x1000);

	for (offs_t addr = 0; addr < 0x400; addr++)
	{
		// invert address lines
		rom[0x3ff - addr] = inverted[addr];
		rom[0x7ff - addr] = inverted[addr + 0x400];
		rom[0xbff - addr] = inverted[addr + 0x800];
		rom[0xfff - addr] = inverted[addr + 0xc00];
	}
}

} // Anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS           INIT           COMPANY      FULLNAME       FLAGS
COMP( 1976, jupiter2, 0,      0,      jupiter2, jupiter, jupiter2_state, init_jupiter2, "Wave Mate", "Jupiter II",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1976, jupiter3, 0,      0,      jupiter3, jupiter, jupiter3_state, init_jupiter3, "Wave Mate", "Jupiter III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
