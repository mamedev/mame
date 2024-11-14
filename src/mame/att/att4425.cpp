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

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

#include "emupal.h"
#include "screen.h"


namespace {

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

	void att4425(machine_config &config);

private:
	void port10_w(uint8_t data);
	void port14_w(uint8_t data);
	uint8_t port14_r();
	uint8_t port15_r();

	void write_line_clock(int state);
	void write_keyboard_clock(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	void att4425_io(address_map &map) ATTR_COLD;
	void att4425_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<i8251_device> m_i8251;
	required_device<z80sio_device> m_sio;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<screen_device> m_screen;
};

/* Memory Maps */

void att4425_state::port10_w(uint8_t data)
{
	logerror("Writing %02X to port 10\n", data);
}

void att4425_state::port14_w(uint8_t data)
{
	logerror("Writing %02X to port 14\n", data);
}

uint8_t att4425_state::port14_r()
{
	// only complement of bit 0 used?
	return 0;
}

uint8_t att4425_state::port15_r()
{
	// status of something (at least bits 2 and 3 used)
	return 0;
}

void att4425_state::att4425_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z80_TAG, 0);
	map(0x8000, 0xffff).ram().share("videoram"); // c000..f7af?
}

void att4425_state::att4425_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw(m_i8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x10).w(FUNC(att4425_state::port10_w));
	map(0x14, 0x14).rw(FUNC(att4425_state::port14_r), FUNC(att4425_state::port14_w));
	map(0x15, 0x15).r(FUNC(att4425_state::port15_r));
	map(0x18, 0x1b).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

/* Input Ports */

static INPUT_PORTS_START( att4425 )
INPUT_PORTS_END

/* Video */

uint32_t att4425_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy = 0;

	uint8_t fg = 2;
	uint8_t bg = 0;

	for (uint8_t y = 0; y < 27; y++)
	{
		uint16_t ma = 0x7e9c + 4 * (81 - 27 + y);
		ma = (m_p_videoram[ma] << 8) + m_p_videoram[ma + 1] - 0x8000;

		for (uint8_t ra = 0; ra < 13; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 160; x += 2)
			{
				uint8_t const chr = m_p_videoram[x + 1];
				uint8_t const attr = m_p_videoram[x];
				uint16_t ca = (chr << 4) & 0x7f0;
				uint8_t gfx;

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
				for (int i = 7; i >= 0; i--)
					*p++ = BIT(gfx, i) ? fg : bg;

				*p++ = bg;
			}
		}
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

static GFXDECODE_START( gfx_att4425 )
	GFXDECODE_ENTRY( "chargen", 0x0000, att4425_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Initialization */

void att4425_state::machine_start()
{
}

/* Machine Driver */

void att4425_state::write_line_clock(int state)
{
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
	m_sio->rxtxcb_w(state);
}

void att4425_state::write_keyboard_clock(int state)
{
	m_i8251->write_txc(state);
	m_i8251->write_rxc(state);
}

static const z80_daisy_config att4425_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

void att4425_state::att4425(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(32'000'000) / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &att4425_state::att4425_mem);
	m_maincpu->set_addrmap(AS_IO, &att4425_state::att4425_io);
	m_maincpu->set_daisy_config(att4425_daisy_chain);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(att4425_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->set_size(720, 351);
	m_screen->set_visarea(0, 720 - 1, 0, 351 - 1);
	GFXDECODE(config, "gfxdecode", "palette", gfx_att4425);
	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	// ch.3 -- timer?
	z80ctc_device &ctc(Z80CTC(config, Z80CTC_TAG, XTAL(32'000'000)));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
#ifdef notdef
	ctc.zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<0>().append(m_sio, FUNC(z80sio_device::txca_w));
	ctc.zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
#endif

	Z80SIO(config, m_sio, 4800);
	m_sio->out_int_callback().set_inputline(Z80_TAG, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));

	// host
	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "null_modem"));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	// aux printer?
	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, "printer"));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));

	clock_device &line_clock(CLOCK(config, "line_clock", 9600 * 64));
	line_clock.signal_handler().set(FUNC(att4425_state::write_line_clock));

	I8251(config, m_i8251, 0);
	m_i8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_i8251->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_i8251->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_i8251, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_i8251, FUNC(i8251_device::write_dsr));

	clock_device &keyboard_clock(CLOCK(config, "keyboard_clock", 4800 * 64));
	keyboard_clock.signal_handler().set(FUNC(att4425_state::write_keyboard_clock));

	RAM(config, RAM_TAG).set_default_size("32K").set_default_value(0);
}

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

} // anonymous namespace


/* System Drivers */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME              FLAGS
COMP( 1983, att4425, 0,      0,      att4425, att4425, att4425_state, empty_init, "AT&T",  "AT&T Teletype 4425", MACHINE_IS_SKELETON )
