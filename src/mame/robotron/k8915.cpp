// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Robotron K8915

2010-08-30

When it says DIAGNOSTIC RAZ P, press enter.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"


namespace {

class k8915_state : public driver_device
{
public:
	k8915_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void k8915(machine_config &config);

private:
	void k8915_a8_w(u8 data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_framecnt = 0U;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


void k8915_state::k8915_a8_w(u8 data)
{
// seems to switch ram and rom around.
	m_bank1->set_entry((data == 0x87) ? 0 : 1);
}

void k8915_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram().share("mainram").bankr("bank1");
	map(0x1000, 0x17ff).ram().share("videoram");
	map(0x1800, 0xffff).ram();
}

void k8915_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x50, 0x53).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x58, 0x5b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xa8, 0xa8).w(FUNC(k8915_state::k8915_a8_w));
}

/* Input ports */
static INPUT_PORTS_START( k8915 )
INPUT_PORTS_END

void k8915_state::machine_reset()
{
	m_bank1->set_entry(1);
}

void k8915_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
	save_item(NAME(m_framecnt));
}

u32 k8915_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;

	m_framecnt++;

	for (u8 y = 0; y < 25; y++)
	{
		for (u8 ra = 0; ra < 10; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 80; x++)
			{
				u8 gfx = 0;

				if (ra < 9)
				{
					u8 chr = m_p_videoram[x];

					/* Take care of flashing characters */
					if ((chr & 0x80) && (m_framecnt & 0x08))
						chr = 0x20;

					chr &= 0x7f;

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
		ma+=80;
	}
	return 0;
}


void k8915_state::k8915(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'915'200) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &k8915_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &k8915_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(k8915_state::screen_update));
	screen.set_size(640, 250);
	screen.set_visarea(0, 639, 0, 249);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	z80ctc_device& ctc(Z80CTC(config, "ctc", XTAL(4'915'200) / 2));
	ctc.set_clk<2>(XTAL(4'915'200) / 2);
	ctc.zc_callback<2>().set("sio", FUNC(z80sio_device::rxtxcb_w));

	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'915'200) / 2));
	sio.out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232.dcd_handler().set("sio", FUNC(z80sio_device::dcdb_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));
}


/* ROM definition */
ROM_START( k8915 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "k8915.bin", 0x0000, 0x1000, CRC(ca70385f) SHA1(a34c14adae9be821678aed7f9e33932ee1f3e61c))

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1982, k8915,  0,      0,      k8915,   k8915, k8915_state, empty_init, "Robotron", "K8915",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
