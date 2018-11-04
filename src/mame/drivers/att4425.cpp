// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    AT&T model 4425 text terminal with mosaic graphics character set
    Also known as Teletype model 56D.

    Skeleton driver.  This terminal is supported by netpbm -- pbmto4425(1)

    To do:
    - keyboard
    - everything else

****************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"

#include "screen.h"


#define SCREEN_TAG          "screen"
#define Z80_TAG             "maincpu"
#define Z80CTC_TAG          "z80ctc"
#define Z80SIO_TAG          "z80sio"
#define RS232_A_TAG         "sioa"
#define RS232_B_TAG         "siob"
#define I8251_TAG           "i8251"

class att4425_state : public driver_device
{
public:
	att4425_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_i8251(*this, I8251_TAG)
		, m_sio(*this, Z80SIO_TAG)
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_screen(*this, SCREEN_TAG)
		{ }

	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_READ8_MEMBER(port14_r);
	DECLARE_READ8_MEMBER(port15_r);

	DECLARE_WRITE_LINE_MEMBER(write_line_clock);
	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void att4425(machine_config &config);
	void att4425_io(address_map &map);
	void att4425_mem(address_map &map);
private:
	virtual void machine_start() override;
	virtual void video_start() override;

	required_device<z80_device> m_maincpu;
	required_device<i8251_device> m_i8251;
	required_device<z80sio_device> m_sio;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<screen_device> m_screen;
};

/* Memory Maps */

WRITE8_MEMBER(att4425_state::port10_w)
{
	logerror("Writing %02X to port 10\n", data);
}

WRITE8_MEMBER(att4425_state::port14_w)
{
	logerror("Writing %02X to port 14\n", data);
}

READ8_MEMBER(att4425_state::port14_r)
{
	// only complement of bit 0 used?
	return 0;
}

READ8_MEMBER(att4425_state::port15_r)
{
	// status of something (at least bits 2 and 3 used)
	return 0;
}

ADDRESS_MAP_START(att4425_state::att4425_mem)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("videoram") // c000..f7af?
ADDRESS_MAP_END

ADDRESS_MAP_START(att4425_state::att4425_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(port10_w)
	AM_RANGE(0x14, 0x14) AM_READWRITE(port14_r, port14_w)
	AM_RANGE(0x15, 0x15) AM_READ(port15_r)
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE(Z80SIO_TAG, z80sio_device, ba_cd_r, ba_cd_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( att4425 )
INPUT_PORTS_END

/* Video */

void att4425_state::video_start()
{
}

uint32_t att4425_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y, ra, gfx, fg, bg, chr, attr;
	uint16_t sy = 0, ma, x, ca;

	fg = 2;
	bg = 0;

	for (y = 0; y < 27; y++)
	{
		ma = 0x7e9c + 4 * (81 - 27 + y);
		ma = (m_p_videoram[ma] << 8) + m_p_videoram[ma + 1] - 0x8000;

		for (ra = 0; ra < 13; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 160; x += 2)
			{
				chr = m_p_videoram[x + 1];
				attr = m_p_videoram[x];
				ca = (chr << 4) & 0x7f0;

				// font 2
				if (attr & 0x01)
					ca += 0x0800;

				// underline
				if (attr & 0x10 && ra == 12)
					gfx = 0xff;
				else
					gfx = m_p_chargen[ca | ra] ^ 255;

				// dim
				if (attr & 0x02)
					fg = 1;
				else
					fg = 2;

				// conceal
				if (attr & 0x04)
					fg = bg;

				// reverse video
				if (attr & 0x20)
					gfx ^= 255;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
				*p++ = bg;
			}
		}
		ma += 160;
	}
	return 0;
}

static const gfx_layout att4425_charlayout =
{
	8, 13,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, },
	16*8
};

static GFXDECODE_START( att4425 )
	GFXDECODE_ENTRY( "chargen", 0x0000, att4425_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Initialization */

void att4425_state::machine_start()
{
}

/* Machine Driver */

WRITE_LINE_MEMBER(att4425_state::write_line_clock)
{
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
	m_sio->rxtxcb_w(state);
}

WRITE_LINE_MEMBER(att4425_state::write_keyboard_clock)
{
	m_i8251->write_txc(state);
	m_i8251->write_rxc(state);
}

static const z80_daisy_config att4425_daisy_chain[] =
{
	// XXX order is unknown
	{ Z80SIO_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

MACHINE_CONFIG_START(att4425_state::att4425)
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL(32'000'000)/8) // XXX
	MCFG_CPU_PROGRAM_MAP(att4425_mem)
	MCFG_CPU_IO_MAP(att4425_io)
	MCFG_Z80_DAISY_CHAIN(att4425_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(att4425_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_SIZE(720, 351)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 351-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", att4425)
	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")

	// ch.3 -- timer?
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL(32'000'000)) // XXX
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
#ifdef notdef
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE(Z80SIO_TAG, z80sio_device, txca_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxtxcb_w))
#endif

	MCFG_DEVICE_ADD(Z80SIO_TAG, Z80SIO, 4800) // XXX
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))

	// host
	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, ctsa_w))

	// aux printer?
	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, "printer")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxb_w))

	// XXX
	MCFG_DEVICE_ADD("line_clock", CLOCK, 9600*64)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(att4425_state, write_line_clock))

	MCFG_DEVICE_ADD(I8251_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_cts))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_dsr))

	// XXX
	MCFG_DEVICE_ADD("keyboard_clock", CLOCK, 4800*64)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(att4425_state, write_keyboard_clock))

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_DEFAULT_VALUE(0)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( att4425 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "455773-1.bin", 0x0000, 0x2000, CRC(d216515b) SHA1(6e098c35f8fe6be4f28a577a43145c92972041b1) )
	ROM_LOAD( "455774-1.bin", 0x2000, 0x2000, CRC(636c069a) SHA1(c00648eae44c574b983de6a0ba6cf74a7f07b098) )
	ROM_LOAD( "456305-1.bin", 0x4000, 0x2000, CRC(43cbf638) SHA1(939569e65957370ab8e60d4f90179373b72b9573) )
	ROM_LOAD( "456306-1.bin", 0x6000, 0x2000, CRC(e4f2b0f1) SHA1(c80c2b7219b313b4924834b0a9d1d42536d1ae63) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "char.bin", 0x0000, 0x2000, CRC(cca962cc) SHA1(201d97b954f782ceae8d17a08fb9a1c4d5ae7a58) )
ROM_END

/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT    STATE          INIT  COMPANY      FULLNAME           FLAGS
COMP( 1983, att4425,  0,      0,      att4425,   att4425, att4425_state, 0,    "AT&T", "AT&T Teletype 4425", MACHINE_IS_SKELETON )
