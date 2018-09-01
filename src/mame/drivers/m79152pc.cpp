// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mera-Elzab 79152pc

        This is terminal

        29/12/2011 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8212.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"


class m79152pc_state : public driver_device
{
public:
	m79152pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_p_attributes(*this, "attributes")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_uart(*this, "uart")
	{ }

	void m79152pc(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(latch_full_w);
	DECLARE_READ_LINE_MEMBER(mcu_t0_r);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void mcu_map(address_map &map);
	void mcu_io_map(address_map &map);

	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_attributes;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<z80sio_device> m_uart;

	bool m_latch_full;
};

WRITE_LINE_MEMBER(m79152pc_state::latch_full_w)
{
	m_latch_full = state == ASSERT_LINE;
}

READ_LINE_MEMBER(m79152pc_state::mcu_t0_r)
{
	return m_latch_full ? 0 : 1;
}

void m79152pc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);;
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8fff).ram().share("videoram");
	map(0x9000, 0x9fff).ram().share("attributes");
}

void m79152pc_state::io_map(address_map &map)
{
	//ADDRESS_MAP_UNMAP_HIGH
	map.global_mask(0xff);
	map(0x40, 0x43).rw(m_uart, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x44, 0x47).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x48, 0x4b).w("pit", FUNC(pit8253_device::write));
	map(0x4c, 0x4c).w("mculatch", FUNC(i8212_device::strobe));
	map(0x54, 0x57).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void m79152pc_state::mcu_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("mcu", 0);
}

void m79152pc_state::mcu_io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff).r("mculatch", FUNC(i8212_device::read));
}

/* Input ports */
static INPUT_PORTS_START( m79152pc )
INPUT_PORTS_END


uint32_t m79152pc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// Attributes are unknown so are not implemented
	uint8_t y,ra,chr,gfx; //,attr;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = m_p_videoram[x];
				//attr = m_p_attributes[x];
				gfx = m_p_chargen[((chr<<4) | ra) + 4 ];

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
		ma+=80;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout m79152pc_charlayout =
{
	8, 12,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_m79152pc )
	GFXDECODE_ENTRY( "chargen", 0x0000, m79152pc_charlayout, 0, 1 )
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "uart" },
	{ nullptr }
};

MACHINE_CONFIG_START(m79152pc_state::m79152pc)
	/* basic machine hardware */
	Z80(config, m_maincpu, 4'000'000); // UA880D
	m_maincpu->set_addrmap(AS_PROGRAM, &m79152pc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m79152pc_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	mcs48_cpu_device &mcu(I8035(config, "mcu", 6'000'000)); // NEC D8035HLC
	mcu.set_addrmap(AS_PROGRAM, &m79152pc_state::mcu_map);
	mcu.set_addrmap(AS_IO, &m79152pc_state::mcu_io_map);
	mcu.t0_in_cb().set(FUNC(m79152pc_state::mcu_t0_r));

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 300-1)
	MCFG_SCREEN_UPDATE_DRIVER(m79152pc_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_m79152pc)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	clock_device &baudclk(CLOCK(config, "baudclk", 921600));
	baudclk.signal_handler().set("ctc", FUNC(z80ctc_device::trg2));
	baudclk.signal_handler().append("pit", FUNC(pit8253_device::write_clk1));
	baudclk.signal_handler().append("pit", FUNC(pit8253_device::write_clk2));

	pit8253_device &pit(PIT8253(config, "pit", 0)); // КР580ВИ53
	pit.out_handler<1>().set(m_uart, FUNC(z80sio_device::txcb_w));
	pit.out_handler<2>().set(m_uart, FUNC(z80sio_device::rxcb_w));

	i8212_device &mculatch(I8212(config, "mculatch")); // CEMI UCY74S412
	mculatch.md_rd_callback().set_constant(0);
	mculatch.int_wr_callback().set(m_uart, FUNC(z80sio_device::ctsb_w)).invert();
	mculatch.int_wr_callback().append(FUNC(m79152pc_state::latch_full_w));

	I8255A(config, "ppi"); // NEC D8255AD-2

	z80ctc_device &ctc(Z80CTC(config, "ctc", 4'000'000));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<2>().set(m_uart, FUNC(z80sio_device::txca_w));
	ctc.zc_callback<2>().append(m_uart, FUNC(z80sio_device::rxca_w));

	Z80SIO(config, m_uart, 4'000'000); // UB8560D
	m_uart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_uart->out_txda_callback().set("keyboard", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtra_callback().set("keyboard", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rtsa_callback().set("keyboard", FUNC(rs232_port_device::write_rts));
	m_uart->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtrb_callback().set("modem", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rtsb_callback().set("modem", FUNC(rs232_port_device::write_rts));

	MCFG_DEVICE_ADD("keyboard", RS232_PORT, default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(WRITELINE(m_uart, z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE(m_uart, z80sio_device, ctsa_w))
	MCFG_DEVICE_ADD("modem", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(m_uart, z80sio_device, rxb_w))
	//MCFG_RS232_CTS_HANDLER(WRITELINE(m_uart, z80sio_device, ctsb_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( m79152pc )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "left.bin", 0x0000, 0x4000, CRC(8cd677fc) SHA1(7ad28f3ba984383f24a36639ca27fc1eb5a5d002))

	ROM_REGION( 0x1000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "right.bin", 0x0000, 0x1000, CRC(93f83fdc) SHA1(e8121b3d175c46c02828f43ec071a7d9c62e7c26)) // chargen

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "char.bin",  0x0000, 0x0800, CRC(da3792a5) SHA1(b4a4f0d61d8082b7909a346a5b01494c53cf8d05))
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY       FULLNAME         FLAGS
COMP( ????, m79152pc, 0,      0,      m79152pc, m79152pc, m79152pc_state, empty_init, "Mera-Elzab", "MERA 79152 PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
