// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Zentec's 8085-based Zephyr/ZMS-series terminals.

This represents the second generation of display terminals by Zentec Corporation. The
first, which included the ZMS-50, ZMS-70 & ZMS-90, had an 8080 CPU and different video
timings.

U.S. Patent No. 4,203,107 looks like the best available description of the Zephyr video
hardware, since no schematics or manuals have been found.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
#include "machine/ay31015.h"
#include "screen.h"

class zms8085_state : public driver_device
{
public:
	zms8085_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_rs232(*this, "rs232")
		, m_screen(*this, "screen")
		, m_mainram(*this, "mainram")
		, m_p_chargen(*this, "chargen")
	{ }

	void zephyr(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(special_r);
	DECLARE_READ8_MEMBER(uart_status_r);

	virtual void machine_start() override;

	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ay51013_device> m_uart;
	required_device<rs232_port_device> m_rs232;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_mainram;
	required_region_ptr<u8> m_p_chargen;
};


u32 zms8085_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 pos = m_mainram[0x17] | (u16(m_mainram[0x18]) << 8);

	for (unsigned y = 0; y < 250; y++)
	{
		for (unsigned ch = 0; ch < 80; ch++)
		{
			unsigned x = ch * 10;
			u8 code = m_mainram[(ch + pos) & 0xfff];
			u8 data = m_p_chargen[(code & 0x7f) * 16 + y % 10];
			for (int bit = 7; bit >= 0; bit--)
				bitmap.pix(y, x++) = BIT(data, bit) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x++) = rgb_t::black();
			bitmap.pix(y, x++) = rgb_t::black();
			bitmap.pix(y, x++) = rgb_t::black();
		}
		if ((y % 10) == 9)
			pos += 80;
	}
	return 0;
}


READ8_MEMBER(zms8085_state::special_r)
{
	return 0x40;
}

READ8_MEMBER(zms8085_state::uart_status_r)
{
	return m_uart->tbmt_r() | (m_uart->dav_r() << 1);
}


void zms8085_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0).nopw();
	map(0x1000, 0x1fff).ram().share("mainram");
}

void zms8085_state::io_map(address_map &map)
{
	map(0x61, 0x61).r(FUNC(zms8085_state::special_r)).w(m_uart, FUNC(ay51013_device::transmit));
	map(0x62, 0x62).r(m_uart, FUNC(ay51013_device::receive));
	map(0x63, 0x63).r(FUNC(zms8085_state::uart_status_r));
	map(0x68, 0x68).nopw();
}


static INPUT_PORTS_START( zephyr )
INPUT_PORTS_END


void zms8085_state::machine_start()
{
	m_uart->write_swe(0);

	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_np(1);
	m_uart->write_eps(1);
	m_uart->write_tsb(0);
	m_uart->write_cs(1);
}

MACHINE_CONFIG_START(zms8085_state::zephyr)
	MCFG_DEVICE_ADD("maincpu", I8085A, XTAL(15'582'000) / 2) // divider not verified
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(15'582'000), 980, 0, 800, 265, 0, 250)
	MCFG_SCREEN_UPDATE_DRIVER(zms8085_state, screen_update)

	AY51013(config, m_uart); // SMC COM2017
	m_uart->set_tx_clock(153600); // should actually be configurable somehow
	m_uart->set_rx_clock(153600);
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->write_dav_callback().set_inputline("maincpu", I8085_RST65_LINE);
	m_uart->set_auto_rdav(true);

	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);
MACHINE_CONFIG_END

/**************************************************************************************************************

Zentec Zephyr (Model 00-441-01).
Chips: COM2017, i8085A, 2x unreadable (40-pin AMI DIP), Beeper
Crystal: 15.582000

***************************************************************************************************************/

ROM_START( zephyr )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_DEFAULT_BIOS("test") // for now
	ROM_SYSTEM_BIOS(0, "main", "Main program" )
	ROMX_LOAD( "23-067-03b.bin",  0x0000, 0x0800, CRC(29cfa003) SHA1(9de7a8402173a2c448e54ee433ba3050db7b70bb), ROM_BIOS(0) )
	ROMX_LOAD( "23-067-004b.bin", 0x0800, 0x0800, CRC(37741104) SHA1(52b9998e0a8d4949e0dc7c3349b3681e13345061), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "test", "Test program" )
	ROMX_LOAD( "23-006-32c.bin",  0x0000, 0x0800, CRC(0a3a5447) SHA1(a8c25730a1d7e5b9c86e0d504afc923e931f9025), ROM_BIOS(1) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "23-066-02a.bin",  0x0000, 0x0800, CRC(d5650b6c) SHA1(e6333e59018d9904f12abb270db4ba28aeff1995) )
ROM_END

COMP( 1979, zephyr, 0, 0, zephyr, zephyr, zms8085_state, empty_init, "Zentec", "Zephyr (00-441-01)", MACHINE_IS_SKELETON )
