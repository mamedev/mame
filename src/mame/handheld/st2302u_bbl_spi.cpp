// license:BSD-3-Clause
// copyright-holders:David Haywood

// Handhelds based on the ST2205U or ST23XX architecture

// the BBL 380 - 180 in 1 features similar menus / presentation / games to the 'ORB Gaming Retro Arcade Pocket Handheld Games Console with 153 Games' (eg has Matchstick Man, Gang Tie III etc.)
// https://www.youtube.com/watch?v=NacY2WHd-CY

// these games were ported to unSP hardware at some point, generalplus_gpl162xx_lcdtype.cpp

// BIOS calls are made very frequently to the firmware (undumped for some sets).
// The most common call ($6058 in bbl380, $6074 in mc_cb203, $6062 in ragc153 & dphh8630, $6052 in pg118 & toumapet) seems to involve downloading a snippet of code from SPI and executing it from RAM at $0300.
// A variant of this call ($60d2 in bbl380, $60ed in mc_cb203, $60e3 in ragc153 & dphh8630, $60de in pg118 & toumapet) is invoked with jsr.
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

#include "emu.h"

#include "cpu/m6502/st2205u.h"
#include "machine/bl_handhelds_menucontrol.h"
#include "machine/generic_spi_flash.h"
#include "machine/xn297l.h"
#include "video/st7735_lcdc.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"


#define LOG_SPI (1U << 1)

//#define VERBOSE (LOG_SPI)
#include "logmacro.h"


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
		m_lcdc(*this, "lcdc"),
		m_genspi(*this, "spi"),
		m_radio(*this, "radio")
	{ }

	void bbl380(machine_config &config) ATTR_COLD;
	void bbl380_menuprot(machine_config &config) ATTR_COLD;
	void bbl380_menuprot_offset(machine_config &config) ATTR_COLD;
	void bbl380_24mhz(machine_config &config) ATTR_COLD;
	void bbl380_radio(machine_config &config) ATTR_COLD;
	void bbl380_radio_big(machine_config &config) ATTR_COLD;
	void bbl380_radio_qpet(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void bbl380_do_maincpu_config() ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bbl380_map(address_map &map) ATTR_COLD;

	required_device<st2205u_base_device> m_maincpu;
	required_device<screen_device> m_screen;

	void output_w(u8 data);
	void output2_w(offs_t, u8 data, u8 mem_mask);
	u8 flash_portc_r(offs_t, u8 mem_mask);
	void flash_portc_w(offs_t, u8 data, u8 mem_mask);
	u16 spi_exchange(u16 data, u8 bits);

	u8 m_output2val;
	bool m_radio_selected;
	bool m_flash_selected;

	required_region_ptr<u8> m_spirom;
	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_device<bl_handhelds_menucontrol_device> m_menucontrol;
	required_device<st7735_lcdc_device> m_lcdc;
	required_device<generic_spi_flash_device> m_genspi;
	optional_device<xn297l_device> m_radio;
};


void bbl380_state::output_w(u8 data)
{
	// Legacy sets need this transaction reset until their flash select is hooked up.
	// The pet hardware supplies the real flash select on PC3.
	if (!m_radio)
		m_genspi->reset();
}

void bbl380_state::output2_w(offs_t, u8 data, u8 mem_mask)
{
	if (m_radio)
	{
		int const cs = BIT(mem_mask, 7) ? BIT(data, 7) : 1;
		m_radio->cs_w(cs);
		m_radio_selected = !cs;
	}

	if ((data & 0x40) != (m_output2val & 0x40))
	{
		if (data & 0x40)
			m_menucontrol->reset_w(1);
	}

	m_menucontrol->data_w((data & 0x08) >> 3);
	m_menucontrol->clock_w((data & 0x04) >> 2);

	m_output2val = data;
}

u8 bbl380_state::flash_portc_r(offs_t, u8)
{
	return m_genspi->so_r() ? 0xff : 0xfd;
}

void bbl380_state::flash_portc_w(offs_t, u8 data, u8 mem_mask)
{
	// The factory-test helper accesses the main flash using SPI mode 0 on
	// PC0=SCK, PC1=SO, PC2=SI and PC3=/CS rather than the SoC SPI controller.
	m_genspi->si_w(BIT(mem_mask, 2) ? BIT(data, 2) : 1);
	int const cs = BIT(mem_mask, 3) ? BIT(data, 3) : 1;
	m_genspi->cs_w(cs);
	m_flash_selected = !cs;
	m_genspi->sck_w(BIT(mem_mask, 0) ? BIT(data, 0) : 0);
}

u32 bbl380_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}

void bbl380_state::machine_start()
{
	// port related
	save_item(NAME(m_output2val));
	save_item(NAME(m_radio_selected));
	save_item(NAME(m_flash_selected));

	m_genspi->set_rom_ptr(memregion("spi")->base());
	m_genspi->set_rom_size(memregion("spi")->bytes());
}


void bbl380_state::machine_reset()
{
	m_output2val = 0;
	m_radio_selected = false;
	m_flash_selected = false;
	if (m_radio)
		m_radio->cs_w(1);

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0000, 0x0000, write8smo_delegate(*this, FUNC(bbl380_state::output_w))); // Port A output hack, SPI state needs resetting on every port write here or some gfx won't copy fully eg red squares on right of parachute, Soc implementation filters writes
}

u16 bbl380_state::spi_exchange(u16 data, u8 bits)
{
	u16 result = 0xffff;
	if (bits == 8 || bits == 16)
	{
		result = 0;
		for (int shift = bits - 8; shift >= 0; shift -= 8)
		{
			u8 const byte = data >> shift;
			u8 reply;
			if (m_radio_selected)
				reply = m_radio->transfer(byte);
			else if (!m_radio || m_flash_selected)
			{
				m_genspi->write(byte);
				reply = m_genspi->read();
			}
			else
				reply = 0xff;
			result = (result << 8) | reply;
		}
	}

	char const *const target = m_radio_selected
		? (m_flash_selected ? " (XN297L, flash also selected)" : " (XN297L)")
		: ((m_radio && !m_flash_selected) ? " (no device selected)" : "");
	LOGMASKED(LOG_SPI, "%s: %u-bit SPI transfer %04x returning %04x%s\n", machine().describe_context(), bits, data, result, target);
	return result;
}


void bbl380_state::bbl380_map(address_map &map)
{
	map(0x0002000, 0x0003fff).rom().region("maincpu", 0);
	map(0x1800000, 0x1800000).w(m_lcdc, FUNC(st7735_lcdc_device::lcdc_command_w));
	map(0x1804000, 0x1804000).rw(m_lcdc, FUNC(st7735_lcdc_device::lcdc_data_r), FUNC(st7735_lcdc_device::lcdc_data_w));
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
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(bbl380_prot)
	PORT_INCLUDE(bbl380)

	PORT_MODIFY("IN1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", FUNC(bl_handhelds_menucontrol_device::data_r))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("menucontrol", FUNC(bl_handhelds_menucontrol_device::status_r))
INPUT_PORTS_END

static INPUT_PORTS_START(bbl380_pet)
	PORT_INCLUDE(bbl380)

	PORT_MODIFY("IN0") // 3 buttons only, no side buttons?
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Select")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Enter")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Back/Menu")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x02, 0x00, "Battery Charging")
	PORT_CONFSETTING(0x00, "Not Charging")
	PORT_CONFSETTING(0x02, "Charging")
	PORT_BIT(0xec, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // going low causes the software to enter a wireless receive path; source unknown
INPUT_PORTS_END

static INPUT_PORTS_START(bbl380_pet560)
	PORT_INCLUDE(bbl380)

	PORT_MODIFY("IN0") // has side buttons
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select") // increase selection value, left in games
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Back/Menu") // right in games, back in menu
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  // for slider menu only
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // ^
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("IN1") // no battery charging state on this one?
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(bbl380_pet568)
	PORT_INCLUDE(bbl380)

	PORT_MODIFY("IN0") // has side buttons
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  // for slider menu only
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // ^
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Select") // left in games
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Back/Menu") // right in games
	PORT_CONFNAME(0x20, 0x00, "Battery Charging")
	PORT_CONFSETTING(0x00, "Not Charging")
	PORT_CONFSETTING(0x20, "Charging")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) 
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("IN1")
	PORT_BIT(0xef, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void bbl380_state::bbl380_do_maincpu_config()
{
	m_maincpu->set_addrmap(AS_DATA, &bbl380_state::bbl380_map);
	m_maincpu->in_pa_callback().set_ioport("IN0");
	m_maincpu->in_pb_callback().set_ioport("IN1");
	m_maincpu->out_pa_callback().set(FUNC(bbl380_state::output_w));
	m_maincpu->set_spi_exchange_callback(FUNC(bbl380_state::spi_exchange));

	m_maincpu->add_route(st2205u_base_device::PSG_OUTPUT_PWM, "mono", 1.00);
}

void bbl380_state::bbl380(machine_config &config)
{
	ST2302U(config, m_maincpu, 32'000'000);  // 32MHz clock correct for music tempo. SoC type not confirmed
	SPEAKER(config, "mono").front_center();

	bbl380_do_maincpu_config();

	SCREEN(config, m_screen).set_lcd(); // TFT color LCD
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128);
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(bbl380_state::screen_update));

	BL_HANDHELDS_MENUCONTROL(config, m_menucontrol);
	ST7735(config, m_lcdc);

	GENERIC_SPI_FLASH(config, m_genspi);

	// LCD controller seems to be either Sitronix ST7735R or (if RDDID bytes match) Ilitek ILI9163C
	// (SoC's built-in LCDC is unused or nonexistent?)
	// Several other LCDC models are identified by ragc153 and dphh8630
}

void bbl380_state::bbl380_menuprot(machine_config &config)
{
	bbl380(config);
	m_maincpu->out_pb_callback().set(FUNC(bbl380_state::output2_w));
}

void bbl380_state::bbl380_menuprot_offset(machine_config &config)
{
	bbl380_menuprot(config);
	m_screen->set_size(161, 132);
	m_screen->set_visarea(1, 161 - 1, 2, 130 - 1);
}

void bbl380_state::bbl380_24mhz(machine_config &config)
{
	bbl380(config);
	ST2302U(config.replace(), m_maincpu, 24'000'000); // 24MHz clock correct for music tempo. SoC type not confirmed
	bbl380_do_maincpu_config();
}

void bbl380_state::bbl380_radio(machine_config &config)
{
	bbl380_24mhz(config);
	m_maincpu->out_pb_callback().set(FUNC(bbl380_state::output2_w));
	m_maincpu->in_pc_callback().set(FUNC(bbl380_state::flash_portc_r));
	m_maincpu->out_pc_callback().set(FUNC(bbl380_state::flash_portc_w));
	XN297L(config, m_radio, 16_MHz_XTAL);

	m_screen->set_size(161, 132);
	m_screen->set_visarea(1, 129 - 1, 0, 128 - 1);
}

void bbl380_state::bbl380_radio_big(machine_config &config)
{
	bbl380_radio(config);

	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 160 - 1, 2, 130 - 1);
}

void bbl380_state::bbl380_radio_qpet(machine_config &config)
{
	bbl380_radio(config);

	m_screen->set_size(160, 132);
	m_screen->set_visarea(3, 131 - 1, 0, 128 - 1);
}


// internal OTPROM BIOS, dumped from dgun2953 PCB, 6000-7fff range
#define INTERNAL_ROM_TYPE1 \
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF) \
	ROM_LOAD("st2x_internal_type1.bin", 0x0000, 0x2000, CRC(f4dc1fc2) SHA1(bbc11539c48eb612ebae50da45e03b6fde440941))

// internal OTPROM BIOS, dumped from retro150a PCB, 6000-7fff range
#define INTERNAL_ROM_TYPE2 \
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF) \
	ROM_LOAD("st2x_internal_type2.bin", 0x0000, 0x2000, CRC(32d96794) SHA1(9d7e3e284f1656d8b2f7dae754cab1f82b3a1d61))


// sets with unknown version of internal ROM

ROM_START(bbl380)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("bbl380_st2205u.bin", 0x000000, 0x004000, NO_DUMP) // internal OTPROM BIOS (addresses are different from other sets)

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("bbl 380 180 in 1.bin", 0x000000, 0x400000, BAD_DUMP CRC(146c88da) SHA1(7f18526a6d8cf991f86febce3418d35aac9f49ad))
	// 0x0022XX, 0x0026XX, 0x002AXX, 0x002CXX, 0x002DXX, 0x0031XX, 0x0036XX, etc. should not be FF fill
ROM_END

ROM_START(mc_cb203)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cb230_st2205u.bin", 0x000000, 0x004000, NO_DUMP) // internal OTPROM BIOS (addresses are different from other sets, including bbl380)

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("s25fl032.bin", 0x000000, 0x400000, CRC(33c4e67b) SHA1(5787db4c8ce4c2569a5f9e9054cbb1944c1b3092))
ROM_END

// sets with 1nd version of internal ROM

ROM_START(rhhc152)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("152_mk25q32amg_ef4016.bin", 0x000000, 0x400000, CRC(5f553895) SHA1(cd21c6ff225e0455531f6b1d9f1c66a284948516))
ROM_END

ROM_START(ragc153)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32ams.bin", 0x000000, 0x400000, CRC(de328d73) SHA1(d17b97e9057be4add68b9f5a26e04c9f0a139673)) // first 0x100 bytes would read as 0xff at regular speed, but give valid looking consistent data at a slower rate
ROM_END

ROM_START(dphh8630)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x200000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("bg25q16.bin", 0x000000, 0x200000, CRC(277850d5) SHA1(740087842e1e63bf99b4ca9c1b2053361f267269))
ROM_END

ROM_START(dgun2953)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("dg160_25x32v_ef3016.bin", 0x000000, 0x400000, CRC(2e993bac) SHA1(4b310e326a47df1980aeef38aa9a59018d7fe76f))
ROM_END

ROM_START(arcade10)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q40.bin", 0x000000, 0x080000, CRC(62784666) SHA1(ba1a4abed0a41b2fb3868543306243e68ea6b2e1))
ROM_END

ROM_START(supreme)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32.bin", 0x000000, 0x400000, CRC(93072a3d) SHA1(9f8770839032922e64d5ddd8864441357623c45f))
ROM_END

ROM_START(throwbck)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32egig.bin", 0x000000, 0x400000, CRC(959eb09d) SHA1(901738e6b6c8fdfe4ed9b268ba3ddd1444551442))
ROM_END

ROM_START(rocoball)
	INTERNAL_ROM_TYPE1

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("spi.bin", 0x000000, 0x800000, CRC(59894e3a) SHA1(e05c40de0c52cd8aa972f70e86c77c32cd6b93cc) )
ROM_END

// sets with 2nd version of internal ROM

ROM_START(dphh8633)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25lq032.u2", 0x000000, 0x400000, CRC(45b8609a) SHA1(d03615a68465a1a365ba07db0b352424680d62d0) )
ROM_END

ROM_START(dphh8661)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25d32h.u2", 0x000000, 0x400000, CRC(b91c1dfc) SHA1(97557d10174c74d40aba780398cb2de3974b2f24) )
ROM_END

ROM_START(retro150)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25d32sh.u2", 0x000000, 0x400000, CRC(294290aa) SHA1(078892b2bb10e347ed07273bafed486e0f52c909) )
ROM_END

ROM_START(retro150a)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("by25q32ess.bin", 0x000000, 0x400000, CRC(ef9e8091) SHA1(5b924d5fd4419956d49379a695b87435df7a1155) )
ROM_END

ROM_START(pg118)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25vq32.bin", 0x000000, 0x400000, CRC(e99f1621) SHA1(f907c36a1a884d892331b7de294a8fd58f7bf9d5) )
ROM_END

ROM_START(ppg118)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("25q32.u2", 0x000000, 0x400000, CRC(c96a30b8) SHA1(da2c41e57b852f3a6644a7cbd0d3740e1b0555dc) )
ROM_END

ROM_START(table108)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("mx25q32.bin", 0x000000, 0x400000, CRC(3fcad3d6) SHA1(9e7b1ba2be174c9ef41eaa06f1537dbe14a695e0) )
ROM_END

ROM_START(toumapet)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25d32sh.bin", 0x000000, 0x400000, CRC(25498f00) SHA1(c5c410e29f540d7f1fd4bbb333467f8a3eaccc15) )
ROM_END

ROM_START(touma560)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("py25q64ha.bin", 0x000000, 0x800000, CRC(7974bf3c) SHA1(8467f869f86b51a86eb115e1408b30daa8f148e7) )
ROM_END

ROM_START(touma568)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x800000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25q64sh.bin", 0x000000, 0x800000, CRC(32f6d834) SHA1(c0dc5b4792a6d86822a666a0f8de7380d1905505) )
ROM_END

ROM_START(qpet)
	INTERNAL_ROM_TYPE2

	ROM_REGION(0x200000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("t25s16.bin", 0x000000, 0x200000, CRC(78a9c285) SHA1(73b0ebe1c88af79fae3357ab3cb4920d685a14f4) )
ROM_END


ROM_START(tchib158)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("st2x_internal_tchib158.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION(0x400000, "spi", ROMREGION_ERASEFF)
	ROM_LOAD("p25d32sh.u2", 0x000000, 0x400000, CRC(274a25ff) SHA1(4c0560ee6cb2d31edd4afdf99adf04ce8d69c6bf) )
ROM_END

} // anonymous namespace


// older releases (primarily for Asian market?)

CONS( 201?, bbl380,        0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "BaoBaoLong", "BBL380 - 180 in 1", MACHINE_NOT_WORKING )

CONS( 201?, mc_cb203,      0,       0,      bbl380,   bbl380, bbl380_state, empty_init, "Coolboy", "Coolboy RS-17 - 203 in 1", MACHINE_NOT_WORKING )

// newer releases (more heavily censored, for export markets?) internal ROM was changed for these

CONS( 201?, dphh8630,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 230-in-1 - Model 8630", MACHINE_IMPERFECT_SOUND ) // sometimes sold as PCP

CONS( 201?, rhhc152,       0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Orb", "Retro Handheld Console 152-in-1", MACHINE_IMPERFECT_SOUND ) // looks like a mini GameBoy - 'Over 150 games' on box

CONS( 201?, ragc153,       0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Orb", "Retro Arcade Game Controller 153-in-1", MACHINE_IMPERFECT_SOUND ) // looks like a Game & Watch

CONS( 201?, dgun2953,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "dreamGEAR", "My Arcade Gamer Mini 160-in-1 (DGUN-2953)", MACHINE_IMPERFECT_SOUND )

CONS( 201?, arcade10,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Fizz Creations", "Mini Arcade Console (Arcade 10-in-1)", MACHINE_IMPERFECT_SOUND )

CONS( 201?, supreme,       0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Fizz Creations", "Arcade Classics Mini Handheld Arcade (Supreme 150)", MACHINE_IMPERFECT_SOUND )

CONS( 201?, throwbck,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Westminster", "Throwback Pocket Video Game Console 150+ 8-Bit Games", MACHINE_IMPERFECT_SOUND )

// might not be using the menu protection device, but accesses something there instead
// could contain corrupt save data
CONS( 2020, rocoball,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "<unknown>", "Roco Battle Ball", MACHINE_NOT_WORKING )

// releases with different internal ROM

// for the UK market, runs at a slightly slower clock
CONS( 201?, retro150,      0,       0,      bbl380_24mhz,   bbl380, bbl380_state, empty_init, "Red5", "Retro Arcade Game Controller (150-in-1) (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 201?, retro150a,     retro150,0,      bbl380_24mhz,   bbl380, bbl380_state, empty_init, "Red5", "Retro Arcade Game Controller (150-in-1) (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// these are for the Japanese market, the ROM is the same between the Pocket Game and Game Computer but the form factor is different.
// pg118 and table108 have a screen offset issue
CONS( 2019, pg118,         0,       0,      bbl380_menuprot_offset,   bbl380_prot, bbl380_state, empty_init, "Pocket Game / Game Computer", "Pocket Game 118-in-1 / Game Computer 118-in-1", MACHINE_NOT_WORKING )
CONS( 201?, table108,      0,       0,      bbl380_menuprot_offset,   bbl380_prot, bbl380_state, empty_init, "<unknown>", "Table Game Classic 108-in-1 (KTFC-001B)", MACHINE_NOT_WORKING )

CONS( 201?, ppg118,        0,       0,      bbl380_24mhz,   bbl380_prot, bbl380_state, empty_init, "<unknown>", "PPG Play Portable Game 118 Games (HH-0046)", MACHINE_NOT_WORKING )

// it is unclear if dphh8633 refers to the case style, rather than the software, as the dphh8630 set was also noted as previously being found in an 8633 unit
// 49. Crazy Dancer 62. Dancer Trace, and 69. Dancer Attack don't work properly, despite
// working on other units in MAME and on real hardware with the same ROM as dphh8661, why?
CONS( 201?, dphh8633,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 268-in-1 - Model 8633", MACHINE_NOT_WORKING )
CONS( 2016, dphh8661,      0,       0,      bbl380_24mhz,      bbl380_prot, bbl380_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 268-in-1 - Model 8661", MACHINE_NOT_WORKING ) // from PCP? (logo on back of console) 2016 date on PCB

// The OK-550 PCB has an XN297LBW wireless transceiver.
CONS( 2021, toumapet,      0,       0,      bbl380_radio,     bbl380_pet,    bbl380_state, empty_init, "Shenzhen Shiji New Technology", "Tou ma Pet (OK-550)", MACHINE_NOT_WORKING | ROT90 )
// OK-558 is Tou ma pet Watch
CONS( 2021, touma560,      0,       0,      bbl380_radio_big, bbl380_pet560, bbl380_state, empty_init, "Shenzhen Shiji New Technology", "Tou ma Pet (OK-560)", MACHINE_NOT_WORKING | ROT90 )
CONS( 2021, touma568,      0,       0,      bbl380_radio_big, bbl380_pet568, bbl380_state, empty_init, "Shenzhen Shiji New Technology", "Tou ma Pet (OK-568)", MACHINE_NOT_WORKING | ROT90 )

CONS( 2020, qpet,          0,       0,      bbl380_radio_qpet, bbl380_pet, bbl380_state, empty_init, "M&D", "Q Pet (2nd version)", MACHINE_NOT_WORKING| ROT90 ) // firmware uses the same transceiver protocol

// yet another internal ROM? (doesn't seem to boot with the ones we have)
CONS( 2022, tchib158,      0,       0,      bbl380_menuprot,   bbl380_prot, bbl380_state, empty_init, "Tchibo GmbH", "Tchibo 158-in-1 Retro Game", MACHINE_NOT_WORKING )
