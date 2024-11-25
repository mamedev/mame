// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

PC/M by Miodrag Milanovic

http://www.li-pro.net/pcm.phtml  (in German)

2009-05-12 Preliminary driver.
2011-02-14 Added keyboard (from terminal).
2020-07-15 Added banking

Commands:
1 select memory bank 1
2 select memory bank 2
B
C start cp/m from the inbuilt CCP
D Debugger
Fx Format disk A or B
G  Jump to address
I List files on tape
L filename.typ  Load file from tape
R read from disk
S filename aaaa / bbbb save a file to tape
V verify
W write to disk
X
Z set tape baud (1200, 2400, 3600 (default), 4800)
filename   start running this .COM file

Therefore if you enter random input, it will lock up while trying to
load up a file of that name. Filenames on disk and tape are of the
standard 8.3 format. You must specify an extension.

Here is an example of starting the debugger, executing a command in
it, then exiting back to the monitor.

D
U
Q

In practice, the I and R commands produce an error, while all disk
commands are directed to tape. The F command lists the files on a
tape.

ToDo:
- Add NMI generator
- Find out if there really is any floppy-disk feature - the schematic
  has no mention of it. (it might be the ram drives)
- Add the 6 LEDs.
- Need software
- MNW until we can be sure it works as intended.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "k7659kb.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pcm_state : public driver_device
{
public:
	pcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_pio_s(*this, "pio_s")
		, m_pio_u(*this, "pio_u")
		, m_ctc_s(*this, "ctc_s")
		, m_ctc_u(*this, "ctc_u")
		, m_speaker(*this, "speaker")
		, m_cass(*this, "cassette")
		, m_p_chargen(*this, "chargen")
		, m_bank(*this, {"bankr", "bankw", "bank2", "bank3"})
	{ }

	void pcm(machine_config &config);

private:
	u8 port85_r();
	void port82_w(int state);
	void port85_w(u8 data);
	void port94_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	bool m_cone = 0;
	u8 m_port85 = 0U;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<z80pio_device> m_pio_s;
	required_device<z80pio_device> m_pio_u;
	required_device<z80ctc_device> m_ctc_s;
	required_device<z80ctc_device> m_ctc_u;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_region_ptr<u8> m_p_chargen;
	required_memory_bank_array<4> m_bank;
};


void  pcm_state::port82_w(int state)
{
	if (state)
	{
		m_cone ^= 1;
		m_speaker->level_w(m_cone);
	}
}


/* PIO connections as far as i could decipher

PortA is input and connects to the keyboard
PortB is mostly output and connects to a series of LEDs,
      but also carries the cassette control & data lines.

A0-A6 ascii codes from the keyboard
A7 strobe, high while a key is pressed
B0 power indicator LED
B1 Run/Stop LED
B2 Sound on/off LED
B3 n/c
B4 High=Save, Low=Load LED
B5 Motor On LED
B6 Save data
B7 Load data
There is also a HALT LED, connected directly to the processor.
*/


u8 pcm_state::port85_r()
{
	u8 data = m_port85 & 0x7f;

	if ((m_cass)->input() > 0.03)
		data |= 0x80;

	return data;
}

void pcm_state::port85_w(u8 data)
{
	m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_cass->output( BIT(data, 6) ? -1.0 : +1.0);
	m_port85 = data;
}

/* Banking:
d0,d1,d2 - select which 64k bank (first 3 are internal)
d3,d4,d5 - external
d6 - force c000-ffff of 64k bank
d7(low) - switch in the roms to 0000-1fff
Real hardware can disable all ram and crash the system. We don't emulate that. */

void pcm_state::port94_w(u8 data)
{
	u8 bank = BIT(data, 0, 3);
	if (bank < 3)
	{
		m_bank[0]->set_entry(bank);
		m_bank[1]->set_entry(bank);
		m_bank[2]->set_entry(bank);
		m_bank[3]->set_entry(bank);
	}

	if (BIT(data, 6))
		m_bank[3]->set_entry(0); // via D13 and D51.1

	if (!BIT(data, 7))
	{
		m_bank[0]->set_entry(3); // via D12 and D53.1
		m_bank[1]->set_entry(3);
	}
}

void pcm_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bankr").bankw("bankw");
	map(0x2000, 0xbfff).bankrw("bank2");
	map(0xc000, 0xffff).bankrw("bank3");
}

void pcm_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_ctc_s, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // system CTC
	map(0x84, 0x87).rw(m_pio_s, FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // system PIO
	map(0x88, 0x8B).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)); // SIO
	map(0x8C, 0x8F).rw(m_ctc_u, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // user CTC
	map(0x90, 0x93).rw(m_pio_u, FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // user PIO
	map(0x94, 0x97).w(FUNC(pcm_state::port94_w));
	//map(0x98, 0x9B) // NMI generator for debugging
	map(0x9C, 0x9F); // io ports available to the user
	// disk controller?
}

/* Input ports */
static INPUT_PORTS_START( pcm )
INPUT_PORTS_END

void pcm_state::machine_start()
{
	u8* r = m_ram->pointer();
	u8 *m = memregion("maincpu")->base();

	save_item(NAME(m_cone));
	save_item(NAME(m_port85));

	m_bank[0]->configure_entries(0, 3, r, 0x10000);
	m_bank[1]->configure_entries(0, 4, r, 0x10000);
	m_bank[2]->configure_entries(0, 3, r+0x2000, 0x10000);
	m_bank[3]->configure_entries(0, 3, r+0xc000, 0x10000);
	m_bank[0]->configure_entry(3, m);
}

void pcm_state::machine_reset()
{
	port94_w(0);   // setup initial state of banks
}

u32 pcm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0x400;

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 8; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				u8 const chr = m_ram->pointer()[x + 0xf800];

				u8 const gfx = m_p_chargen[(chr<<3) | ra];

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


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc_s" },     /* System ctc */
	{ "pio_s" },     /* System pio */
	{ "sio" },       /* sio */
	{ "pio_u" },     /* User pio */
	{ "ctc_u" },     /* User ctc */
	{ nullptr }
};


/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_pcm )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void pcm_state::pcm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000) /4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pcm_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pcm_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(pcm_state::screen_update));
	screen.set_size(64*8, 16*8);
	screen.set_visarea(0, 64*8-1, 0, 16*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_pcm);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Sound */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	K7659_KEYBOARD(config, K7659_KEYBOARD_TAG, 0);
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	Z80PIO(config, m_pio_u, XTAL(10'000'000)/4);
	m_pio_u->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio_s, XTAL(10'000'000)/4);
	m_pio_s->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio_s->in_pa_callback().set(K7659_KEYBOARD_TAG, FUNC(k7659_keyboard_device::read));
	m_pio_s->in_pb_callback().set(FUNC(pcm_state::port85_r));
	m_pio_s->out_pb_callback().set(FUNC(pcm_state::port85_w));

	Z80SIO(config, "sio", XTAL(10'000'000) /4);

	Z80CTC(config, m_ctc_u, 10_MHz_XTAL / 4);
	m_ctc_u->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc_s, 10_MHz_XTAL / 4);
	m_ctc_s->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc_s->zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	m_ctc_s->zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));
	m_ctc_s->zc_callback<1>().set("sio", FUNC(z80sio_device::rxtxcb_w));
	m_ctc_s->zc_callback<2>().set(FUNC(pcm_state::port82_w));  // speaker

	// internal ram: bank0 for cp/m; banks 1 and 2 are ram-drives; last 2k is dummy for rom-write
	RAM(config, RAM_TAG).set_default_size("194K");
}

/* ROM definition */
ROM_START( pcm )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v202", "Version 2.02" )
	ROMX_LOAD( "bios_v202.d14", 0x0000, 0x0800, CRC(27c24892) SHA1(a97bf9ef075de91330dc0c7cfd3bb6c7a88bb585), ROM_BIOS(0))
	ROMX_LOAD( "bios_v202.d15", 0x0800, 0x0800, CRC(e9cedc70) SHA1(913c526283d9289d0cb2157985bb48193df7aa16), ROM_BIOS(0))
	ROMX_LOAD( "bios_v202.d16", 0x1000, 0x0800, CRC(abe12001) SHA1(d8f0db6b141736d7715d588384fa49ab386bcc55), ROM_BIOS(0))
	ROMX_LOAD( "bios_v202.d17", 0x1800, 0x0800, CRC(2d48d1cc) SHA1(36a825140124dbe10d267fdf28b3eacec6f6d556), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v210", "Version 2.10" )
	ROMX_LOAD( "bios_v210.d14", 0x0000, 0x0800, CRC(45923112) SHA1(dde922533ebd0f6ac06d25b9786830ee3c7178b9), ROM_BIOS(1))
	ROMX_LOAD( "bios_v210.d15", 0x0800, 0x0800, CRC(e9cedc70) SHA1(913c526283d9289d0cb2157985bb48193df7aa16), ROM_BIOS(1))
	ROMX_LOAD( "bios_v210.d16", 0x1000, 0x0800, CRC(ee9ed77b) SHA1(12ea18e3e280f2a0657ff11c7bcdd658d325235c), ROM_BIOS(1))
	ROMX_LOAD( "bios_v210.d17", 0x1800, 0x0800, CRC(2d48d1cc) SHA1(36a825140124dbe10d267fdf28b3eacec6f6d556), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v330", "Version 3.30" )
	ROMX_LOAD( "bios_v330.d14", 0x0000, 0x0800, CRC(9bbfee10) SHA1(895002f2f4c711278f1e2d0e2a987e2d31472e4f), ROM_BIOS(2))
	ROMX_LOAD( "bios_v330.d15", 0x0800, 0x0800, CRC(4f8d5b40) SHA1(440b0be4cf45a5d450f9eb7684ceb809450585dc), ROM_BIOS(2))
	ROMX_LOAD( "bios_v330.d16", 0x1000, 0x0800, CRC(93fd0d91) SHA1(c8f1bbb63eca3c93560622581ecbb588716aeb91), ROM_BIOS(2))
	ROMX_LOAD( "bios_v330.d17", 0x1800, 0x0800, CRC(d8c7ce33) SHA1(9030d9a73ef1c12a31ac2cb9a593fb2a5097f24d), ROM_BIOS(2))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "charrom.d113", 0x0000, 0x0800, CRC(5684b3c3) SHA1(418054aa70a0fd120611e32059eb2051d3b82b5a))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY          FULLNAME  FLAGS */
COMP( 1988, pcm,  0,      0,      pcm,     pcm,   pcm_state, empty_init, "Mugler/Mathes", "PC/M",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
