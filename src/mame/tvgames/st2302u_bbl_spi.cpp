// license:BSD-3-Clause
// copyright-holders:David Haywood

// Handhelds based on the ST2205U or ST23XX architecture

// the BBL 380 - 180 in 1 features similar menus / presentation / games to the 'ORB Gaming Retro Arcade Pocket Handheld Games Console with 153 Games' (eg has Matchstick Man, Gang Tie III etc.)
// https://www.youtube.com/watch?v=NacY2WHd-CY

// these games were ported to unSP hardware at some point, generalplus_gpl162xx_lcdtype.cpp

// BIOS calls are made very frequently to the firmware (undumped for bbl380).
// The most common call ($6058 in bbl380, $6062 in ragc153 & dphh8630) seems to involve downloading a snippet of code from SPI and executing it from RAM at $0300.
// A variant of this call ($60d2 in bbl380, $60e3 in ragc153 & dphh8630) is invoked with jsr.
// For these calls, a 24-bit starting address is specified in $82:$81:$80, and the length in bytes is twice the number specified in $84:$83.
// There is a configurable XOR specified in $99 on ragc153 & dphh8630.
// $6003 performs a table lookup, depositing a sequence of data at $008e.
// $6000 is some sort of macro call with the X register as function selector
// (X = $24 should display the character in $0102 on screen).
// One other BIOS call ($6975 in bbl380, $69d2 in ragc153) has an unknown purpose.

/*
   Some sets contain games not indexed by the menu code, some of these games are broken / in a state of mid-reskinning, others seem to be functional

   Menu index list locations in ROM
   supreme 0x243e
*/

// TODO: convert to use generic_spi_flash.cpp (but there seems to be some buffering of writes / reads?)

#include "emu.h"

#include "cpu/m6502/st2205u.h"
#include "machine/bl_handhelds_menucontrol.h"
#include "bl_handhelds_lcdc.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class bbl380_state : public driver_device
{
public:
	bbl380_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_spirom(*this, "spi"),
		m_io_p1(*this, "IN0"),
		m_io_p2(*this, "IN1"),
		m_menucontrol(*this, "menucontrol"),
		m_lcdc(*this, "lcdc")
	{ }

	void bbl380(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bbl380_map(address_map &map) ATTR_COLD;

	required_device<st2205u_base_device> m_maincpu;
	required_device<screen_device> m_screen;

	void output_w(u8 data);
	void output2_w(u8 data);

	u8 m_output2val;

	enum spistate : u8
	{
		SPI_STATE_READY = 0,
		SPI_STATE_WAITING_HIGH_ADDR = 1,
		SPI_STATE_WAITING_MID_ADDR = 2,
		SPI_STATE_WAITING_LOW_ADDR = 3,
		SPI_STATE_WAITING_DUMMY1_ADDR = 4,
		SPI_STATE_WAITING_DUMMY2_ADDR = 5,
		SPI_STATE_READING = 6,
	};

	u8 m_spistate;
	u32 m_spiaddress;
	u8 m_delay;

	void spi_w(u8 data);
	u8 spi_r();

	required_region_ptr<u8> m_spirom;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_device<bl_handhelds_menucontrol_device> m_menucontrol;
	required_device<bl_handhelds_lcdc_device> m_lcdc;

	u8 ff_r() { return 0xff; }
};


void bbl380_state::output_w(u8 data)
{
	m_spistate = SPI_STATE_READY;
}

void bbl380_state::output2_w(u8 data)
{
	if ((data & 0x40) != (m_output2val & 0x40))
	{
		if (data & 0x40)
			m_menucontrol->reset_w(1);
	}

	m_menucontrol->data_w((data & 0x08) >> 3);
	m_menucontrol->clock_w((data & 0x04) >> 2);

	m_output2val = data;
}

u32 bbl380_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}

void bbl380_state::machine_start()
{
	// port related
	save_item(NAME(m_output2val));

	// SPI related
	save_item(NAME(m_spistate));
	save_item(NAME(m_spiaddress));
	save_item(NAME(m_delay));
}


void bbl380_state::machine_reset()
{
	m_output2val = 0;

	// TODO: handle these things in the core via callbacks etc. once correct behavior is agreed upon
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0010, 0x0011, read8smo_delegate(*this, FUNC(bbl380_state::spi_r)), write8smo_delegate(*this, FUNC(bbl380_state::spi_w))); // SPI related
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0014, 0x0014, read8smo_delegate(*this, FUNC(bbl380_state::ff_r))); // SPI related
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0000, 0x0000, write8smo_delegate(*this, FUNC(bbl380_state::output_w))); // Port A output hack, SPI state needs resetting on every port write here or some gfx won't copy fully eg red squares on right of parachute, Soc implementation filters writes
}


void bbl380_state::spi_w(u8 data)
{
	switch (m_spistate)
	{
	case SPI_STATE_READY:
	{
		if (data == 0x03)
		{
			m_spistate = SPI_STATE_WAITING_HIGH_ADDR;
		}
		else
		{
			logerror("%s: invalid state request %02x\n", machine().describe_context(), data);
		}
		break;
	}

	case SPI_STATE_WAITING_HIGH_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xff00ffff) | data << 16;
		m_spistate = SPI_STATE_WAITING_MID_ADDR;
		break;
	}

	case SPI_STATE_WAITING_MID_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffff00ff) | data << 8;
		m_spistate = SPI_STATE_WAITING_LOW_ADDR;
		break;
	}

	case SPI_STATE_WAITING_LOW_ADDR:
	{
		m_spiaddress = (m_spiaddress & 0xffffff00) | data;
		m_spistate = SPI_STATE_READING;
		m_delay = 2;
		break;
	}

	case SPI_STATE_READING:
	{
		// writes when in read mode clock in data?
		m_delay = 1;
		break;
	}

	case SPI_STATE_WAITING_DUMMY1_ADDR:
	{
		m_spistate = SPI_STATE_WAITING_DUMMY2_ADDR;
		break;
	}

	case SPI_STATE_WAITING_DUMMY2_ADDR:
	{
		//  m_spistate = SPI_STATE_READY;
		break;
	}

	}
}

u8 bbl380_state::spi_r()
{
	switch (m_spistate)
	{
	case SPI_STATE_READING:
	{
		if (m_delay > 0)
		{
			m_delay--;
			return 0x00;
		}
		else
		{
			u8 dat = m_spirom[m_spiaddress & 0x3fffff];
			//logerror("%s: reading SPI %02x from SPI Address %08x\n", machine().describe_context(), dat, m_spiaddress);
			m_spiaddress++;
			return dat;
		}
	}

	default:
	{
		//logerror("%s: reading FIFO in unknown state\n", machine().describe_context() );
		return 0x00;
	}
	}

	return 0x00;
}


void bbl380_state::bbl380_map(address_map &map)
{
	map(0x0000000, 0x03fffff).rom().region("maincpu", 0);
	map(0x1800000, 0x1800000).w(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_command_w));
	map(0x1804000, 0x1804000).rw(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_data_r), FUNC(bl_handhelds_lcdc_device::lcdc_data_w));
}

static INPUT_PORTS_START(bbl380)
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // maybe ON/OFF
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("SOUND")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT(0x06, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", bl_handhelds_menucontrol_device, data_r)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", bl_handhelds_menucontrol_device, status_r)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void bbl380_state::bbl380(machine_config &config)
{
	ST2302U(config, m_maincpu, 32000000); // unknown clock; type not confirmed
	m_maincpu->set_addrmap(AS_DATA, &bbl380_state::bbl380_map);
	m_maincpu->in_pa_callback().set_ioport("IN0");
	m_maincpu->in_pb_callback().set_ioport("IN1");


	m_maincpu->out_pa_callback().set(FUNC(bbl380_state::output_w));
	m_maincpu->out_pb_callback().set(FUNC(bbl380_state::output2_w));
	// TODO, hook these up properly
	//m_maincpu->spi_in_callback().set(FUNC(bbl380_state::spi_r));
	//m_maincpu->spi_out_callback().set(FUNC(bbl380_state::spi_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(0, "mono", 1.00);
	m_maincpu->add_route(1, "mono", 1.00);
	m_maincpu->add_route(2, "mono", 1.00);
	m_maincpu->add_route(3, "mono", 1.00);


	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // TFT color LCD
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128);
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(bbl380_state::screen_update));

	BL_HANDHELDS_MENUCONTROL(config, m_menucontrol, 0);
	BL_HANDHELDS_LCDC(config, m_lcdc, 0);

	// LCD controller seems to be either Sitronix ST7735R or (if RDDID bytes match) Ilitek ILI9163C
	// (SoC's built-in LCDC is unused or nonexistent?)
	// Several other LCDC models are identified by ragc153 and dphh8630
}

ROM_START(bbl380)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bbl380_st2205u.bin", 0x000000, 0x004000, NO_DUMP) // internal OTPROM BIOS (addresses are different from other sets)

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("bbl 380 180 in 1.bin", 0x000000, 0x400000, CRC(146c88da) SHA1(7f18526a6d8cf991f86febce3418d35aac9f49ad) BAD_DUMP)
	// 0x0022XX, 0x0026XX, 0x002AXX, 0x002CXX, 0x002DXX, 0x0031XX, 0x0036XX, etc. should not be FF fill
ROM_END

ROM_START(mc_cb203)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bbl380_st2205u.bin", 0x000000, 0x004000, NO_DUMP) // internal OTPROM BIOS (addresses are different from other sets, including bbl380)

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("s25fl032.bin", 0x000000, 0x400000, CRC(33c4e67b) SHA1(5787db4c8ce4c2569a5f9e9054cbb1944c1b3092))
ROM_END

ROM_START(rhhc152)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("152_mk25q32amg_ef4016.bin", 0x000000, 0x400000, CRC(5f553895) SHA1(cd21c6ff225e0455531f6b1d9f1c66a284948516))
ROM_END

ROM_START(ragc153)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32ams.bin", 0x000000, 0x400000, CRC(de328d73) SHA1(d17b97e9057be4add68b9f5a26e04c9f0a139673)) // first 0x100 bytes would read as 0xff at regular speed, but give valid looking consistent data at a slower rate
ROM_END

ROM_START(dphh8630)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("bg25q16.bin", 0x000000, 0x200000, CRC(277850d5) SHA1(740087842e1e63bf99b4ca9c1b2053361f267269))
ROM_END

ROM_START(dgun2953)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("dg160_25x32v_ef3016.bin", 0x000000, 0x400000, CRC(2e993bac) SHA1(4b310e326a47df1980aeef38aa9a59018d7fe76f))
ROM_END

ROM_START(arcade10)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q40.bin", 0x000000, 0x080000, CRC(62784666) SHA1(ba1a4abed0a41b2fb3868543306243e68ea6b2e1))
ROM_END

ROM_START(supreme)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32.bin", 0x000000, 0x400000, CRC(93072a3d) SHA1(9f8770839032922e64d5ddd8864441357623c45f))
ROM_END

ROM_START(pg118)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25vq32.bin", 0x000000, 0x400000, CRC(e99f1621) SHA1(f907c36a1a884d892331b7de294a8fd58f7bf9d5) )
ROM_END

ROM_START(toumapet)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal.bin", 0x002000, 0x002000, BAD_DUMP CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941)) // internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25d32sh.bin", 0x000000, 0x400000, CRC(25498f00) SHA1(c5c410e29f540d7f1fd4bbb333467f8a3eaccc15) )
ROM_END


} // anonymous namespace


// older releases (primarily for Asian market?)

CONS( 201?, bbl380,        0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "BaoBaoLong", "BBL380 - 180 in 1", MACHINE_NOT_WORKING )

CONS( 201?, mc_cb203,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Coolboy", "Coolboy RS-17 - 203 in 1", MACHINE_NOT_WORKING )

// newer releases (more heavily censored, for export markets?) internal ROM was changed for these

CONS( 201?, dphh8630,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 230-in-1 - Model 8630 / Model 8633", MACHINE_NOT_WORKING ) // sometimes sold as PCP.  Model 8630/8633 are same ROM, different case

CONS( 201?, rhhc152,       0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Orb", "Retro Handheld Console 152-in-1", MACHINE_IMPERFECT_SOUND ) // looks like a mini GameBoy - 'Over 150 games' on box

CONS( 201?, ragc153,       0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Orb", "Retro Arcade Game Controller 153-in-1", MACHINE_IMPERFECT_SOUND ) // looks like a Game & Watch

CONS( 201?, dgun2953,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "dreamGEAR", "My Arcade Gamer Mini 160-in-1 (DGUN-2953)", MACHINE_IMPERFECT_SOUND )

CONS( 201?, arcade10,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Fizz Creations", "Mini Arcade Console (Arcade 10-in-1)", MACHINE_IMPERFECT_SOUND )

CONS( 201?, supreme,       0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Fizz Creations", "Arcade Classics Mini Handheld Arcade (Supreme 150)", MACHINE_IMPERFECT_SOUND )

// these are for the Japanese market, the ROM is the same between the Pocket Game and Game Computer but the form factor is different.
// they have the 0xE4 XOR on the SPI data like many of the above units, but don't currently boot - need to verify if the internal ROM part should be the same or not
CONS( 2019, pg118,         0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Pocket Game / Game Computer", "Pocket Game 118-in-1 / Game Computer 118-in-1", MACHINE_NOT_WORKING )
// also has the 0xE4 XOR, also doesn't currently boot
CONS( 2021, toumapet,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Shenzhen Shiji New Technology", "Tou ma Pet", MACHINE_NOT_WORKING )

