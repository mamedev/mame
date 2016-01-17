// license:BSD-3-Clause
// copyright-holders:Emmanuel Vadot

/****************************************** PICO emulation ****************************************/

/* todo, make this more independent of the Genesis emulation, it's really only the same CPU + VDP
   and doesn't need to be connected to the Genesis at all.

   sound is the 315-5641 / D77591, should be compatible with the 7759? but probably wants us to maintain
   an external buffer of at least 0x40 bytes and feed it on a timer in sync with the timer in the chip?

   currently no way to select (or display) the story book area of the games? (will require layout and
   external artwork)

*/


/*

Pico mainboard (PAL version)

+---+                              +-------------------------------------------------------------------------+
|   |                              |                                    +-+                                  |
|   |                              |                 +--+               |B|        +-+                       |
|   |                              |                 |A3|               +-+        | |                       |
|   +------------------------------+                 +--+                          |C|                     +-|
|                                                                                  | |                     | |
|    GCMK-C2X                                                                      +-+                     | --> PEN
|                                 +-----+     +--+                                                         +-|
|                                 |     |     |A3|            +--------+                                     |
|                                 | A1  |     +--+            |        |              +---------+            |
|          SEGA                   |     |                     |    A2  |              |HM53861J |            |
|     1994 837-10846              +-----+                     |        |              |         |            |
|      IAC MAIN PAL                                           +--------+              +---------+            |
|     MADE IN JAPAN                                                        +----+                            |
|           VA0          +----------+                                      |XTAL|                         +--+
|                        |   SEGA   |         +---------+                  |    |                         |  <-- VCC IN
|                        | 315-5640 |         |MC68HC000|                  |53.2|                         +--+
|                        | 9434 W51 |         |FN8-A    |  +----+          |00  |    +----------+            |
|                        |          |         |         |  |    |          +----+    |   SEGA   |            |
|                        |          |         |  2B89N  |  |    |                    | 315-5313A|            |
|                        |          |         |S0AH9425A|  | A4 |                    |   F1001  |         +--|
|                        +----------+         +---------+  |    |                    |          |         |  |
|                                                          |    |                    |9428 LAGG |         |  --> VIDEO OUT
|                                                          |    |                    |          |         |  |
|                                                          +----+                    +----------+         +--|
|                                                                                                            |
|                                                                     +----------------------------+         |
|                                                                     ||||||CARTRIDGE CONNECTOR|||||         |
|                                                                     +----------------------------+         |
|                                                                                                            |
+------------------------------------------------------------------------------------------------------------+

A1 = SEGA / 315-5641 / D77591 / 9442CA010
A2 = SEGA / 315-5769 U13 / 9451MD020
A3 = BA10324AF
A4 = MALAYSIA 9336 / 651632DFP-15 / 0000988S
B = 4K16 / HC00
C = MB3514 / 9325 M36


315-5640  - touchpad controller?
315-5313A - VDP
315-5641  - PCM chip


*/

/*
   Pico Implementation By ElBarto (Emmanuel Vadot, elbarto@megadrive.org)
   Still missing the PCM custom chip
   Some game will not boot due to this

 Pico Info from Notaz (http://notaz.gp2x.de/docs/picodoc.txt)

 addr   acc   description
-------+-----+------------
800001  byte  Version register.
              ?vv? ????, where v can be:
                00 - hardware is for Japan
                01 - European version
                10 - USA version
                11 - ?
800003  byte  Buttons, 0 for pressed, 1 for released:
                bit 0: UP (white)
                bit 1: DOWN (orange)
                bit 2: LEFT (blue)
                bit 3: RIGHT (green)
                bit 4: red button
                bit 5: unused?
                bit 6: unused?
                bit 7: pen button
800005  byte  Most significant byte of pen x coordinate.
800007  byte  Least significant byte of pen x coordinate.
800009  byte  Most significant byte of pen y coordinate.
80000b  byte  Least significant byte of pen y coordinate.
80000d  byte  Page register. One bit means one uncovered page sensor.
                00 - storyware closed
                01, 03, 07, 0f, 1f, 3f - pages 1-6
                either page 5 or page 6 is often unused.
800010  word  PCM data register.
        r/w   read returns free bytes left in PCM FIFO buffer
              writes write data to buffer.
800012  word  PCM control register.
        r/w   For writes, it has following possible meanings:
              ?p?? ???? ???? ?rrr
                p - set to enable playback?
                r - sample rate / PCM data type?
                  0: 8kHz 4bit ADPCM?
                  1-7: 16kHz variants?
              For reads, if bit 15 is cleared, it means PCM is 'busy' or
              something like that, as games sometimes wait for it to become 1.
800019  byte  Games write 'S'
80001b  byte  Games write 'E'
80001d  byte  Games write 'G'
80001f  byte  Games write 'A'

*/

#include "emu.h"
#include "includes/md_cons.h"
#include "sound/315-5641.h"
#include "softlist.h"


#define PICO_PENX   1
#define PICO_PENY   2

class pico_base_state : public md_cons_state
{
public:
	pico_base_state(const machine_config &mconfig, device_type type, std::string tag)
		: md_cons_state(mconfig, type, tag),
		m_sega_315_5641_pcm(*this, "315_5641"),
		m_io_page(*this, "PAGE"),
		m_io_pad(*this, "PAD"),
		m_io_penx(*this, "PENX"),
		m_io_peny(*this, "PENY")
	{ }

	optional_device<sega_315_5641_pcm_device> m_sega_315_5641_pcm;

	required_ioport m_io_page;
	required_ioport m_io_pad;
	required_ioport m_io_penx;
	required_ioport m_io_peny;

	UINT8 m_page_register;

	UINT16 pico_read_penpos(int pen);
	DECLARE_READ16_MEMBER(pico_68k_io_read);
	DECLARE_WRITE16_MEMBER(pico_68k_io_write);
	DECLARE_WRITE_LINE_MEMBER(sound_cause_irq);

	DECLARE_DRIVER_INIT(pico);
	DECLARE_DRIVER_INIT(picou);
	DECLARE_DRIVER_INIT(picoj);
};

class pico_state : public pico_base_state
{
public:
	pico_state(const machine_config &mconfig, device_type type, std::string tag)
	: pico_base_state(mconfig, type, tag),
	m_picocart(*this, "picoslot") { }

	required_device<pico_cart_slot_device> m_picocart;
	DECLARE_MACHINE_START(pico);
};



UINT16 pico_base_state::pico_read_penpos(int pen)
{
	UINT16 penpos = 0;

	switch (pen)
	{
		case PICO_PENX:
			penpos = m_io_penx->read();
			penpos |= 0x6;
			penpos = penpos * 320 / 255;
			penpos += 0x3d;
			break;
		case PICO_PENY:
			penpos = m_io_peny->read();
			penpos |= 0x6;
			penpos = penpos * 251 / 255;
			penpos += 0x1fc;
			break;
	}

	return penpos;
}

READ16_MEMBER(pico_base_state::pico_68k_io_read )
{
	UINT8 retdata = 0;

	switch (offset)
	{
		case 0: /* Version register ?XX?????? where XX is 00 for japan, 01 for europe and 10 for USA*/
			retdata = m_version_hi_nibble;
			break;
		case 1:
			retdata = m_io_pad->read();
			break;

			/*
			Still notes from notaz for the pen :

			The pen can be used to 'draw' either on the drawing pad or on the storyware
			itself. Both storyware and drawing pad are mapped on single virtual plane, where
			coordinates range:

			x: 0x03c - 0x17c
			y: 0x1fc - 0x2f7 (drawing pad)
			  0x2f8 - 0x3f3 (storyware)
			*/
		case 2:
			retdata = pico_read_penpos(PICO_PENX) >> 8;
			break;
		case 3:
			retdata = pico_read_penpos(PICO_PENX) & 0x00ff;
			break;
		case 4:
			retdata = pico_read_penpos(PICO_PENY) >> 8;
			break;
		case 5:
			retdata = pico_read_penpos(PICO_PENY) & 0x00ff;
			break;
		case 6:
		/* Page register :
		   00 - storyware closed
		   01, 03, 07, 0f, 1f, 3f - pages 1-6
		   either page 5 or page 6 is often unused.
		*/
			{
				UINT8 tmp = m_io_page->read();
				if (tmp == 2 && m_page_register != 0x3f)
				{
					m_page_register <<= 1;
					m_page_register |= 1;
				}
				if (tmp == 1 && m_page_register != 0x00)
					m_page_register >>= 1;
				retdata = m_page_register;
				break;
			}



		case 8: // toy story 2 checks this for 0x3f (is that 'empty'?)
			/* Returns free bytes left in the PCM FIFO buffer */
			retdata = m_sega_315_5641_pcm->get_fifo_space();
			break;
		case 9:
		/*
		   For reads, if bit 15 is cleared, it means PCM is 'busy' or
		   something like that, as games sometimes wait for it to become 1.
		*/
			//  return (m_upd7759->busy_r()^1) << 15;
			// The BUSY bit stays 1 as long as some PCM sound is playing.
			// SMPS drivers check 800012 [byte] and clear the "prevent music PCM" byte when the READY bit gets set.
			// If this is done incorrectly, the voices in Sonic Gameworld (J) are muted by the music's PCM drums.
			return m_sega_315_5641_pcm->busy_r() << 15;


		case 7:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			logerror("pico_68k_io_read %d\n", offset);

	}

	return retdata | retdata << 8;
}


WRITE_LINE_MEMBER(pico_base_state::sound_cause_irq)
{
//  printf("sound irq\n");
	/* sega_315_5641_pcm callback */
	m_maincpu->set_input_line(3, HOLD_LINE);
}

WRITE16_MEMBER(pico_base_state::pico_68k_io_write )
{
//  printf("pico_68k_io_write %04x %04x %04x\n", offset*2, data, mem_mask);

	switch (offset)
	{
		case 0x10/2:
			if (mem_mask & 0xFF00)
				m_sega_315_5641_pcm->port_w(space, 0, (data >> 8) & 0xFF);
			if (mem_mask & 0x00FF)
				m_sega_315_5641_pcm->port_w(space, 0, (data >> 0) & 0xFF);
			break;
		case 0x12/2: // guess
			// Note about uPD7759 lines:
			//  reset line: 1 - normal, 1->0 - reset chip, 0 - playback disabled
			//  start line: 0->1 - start playback
			if (mem_mask & 0xFF00)
			{
				// I assume that:
				// value 8000 resets the FIFO? (always used with low reset line)
				// value 0800 maps to the uPD7759's reset line (0 = reset, 1 = normal)
				// value 4000 maps to the uPD7759's start line (0->1 = start)
				m_sega_315_5641_pcm->reset_w((data >> 8) & 0x08);
				m_sega_315_5641_pcm->start_w((data >> 8) & 0x40);
				if (data & 0x4000)
				{
					// Somewhere between "Reset Off" and the first sample data,
					// we need to send a few commands to make the sample stream work.
					// Doing that when rising the "start" line seems to work fine.
					m_sega_315_5641_pcm->port_w(space, 0, 0xFF);    // "Last Sample" value (must be >= 0x10)
					m_sega_315_5641_pcm->port_w(space, 0, 0x00);    // Dummy 1
					m_sega_315_5641_pcm->port_w(space, 0, 0x00);    // Addr MSB
					m_sega_315_5641_pcm->port_w(space, 0, 0x00);    // Addr LSB
				}
			}


			/*m_sega_315_5641_pcm->reset_w(0);
			m_sega_315_5641_pcm->start_w(0);
			m_sega_315_5641_pcm->reset_w(1);
			m_sega_315_5641_pcm->start_w(1);

			if (mem_mask&0x00ff) m_sega_315_5641_pcm->port_w(space,0,data&0xff);
			if (mem_mask&0xff00) m_sega_315_5641_pcm->port_w(space,0,(data>>8)&0xff);*/

			break;
	}
}

static ADDRESS_MAP_START( pico_mem, AS_PROGRAM, 16, pico_base_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM

	AM_RANGE(0x800000, 0x80001f) AM_READWRITE(pico_68k_io_read, pico_68k_io_write)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)
	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000)
ADDRESS_MAP_END


static INPUT_PORTS_START( pico )
	PORT_START("PAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Red Button")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Pen Button")

	PORT_START("PAGE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Increment Page")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Decrement Page")

	PORT_START("PENX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("PEN X")

	PORT_START("PENY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0,255 ) PORT_PLAYER(1) PORT_NAME("PEN Y")
INPUT_PORTS_END


static SLOT_INTERFACE_START(pico_cart)
	SLOT_INTERFACE_INTERNAL("rom",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_sram",  MD_ROM_SRAM)   // not sure these are needed...
	SLOT_INTERFACE_INTERNAL("rom_sramsafe",  MD_ROM_SRAM)   // not sure these are needed...
SLOT_INTERFACE_END

MACHINE_START_MEMBER(pico_state,pico)
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7fffff, read16_delegate(FUNC(base_md_cart_slot_device::read),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa13000, 0xa130ff, read16_delegate(FUNC(base_md_cart_slot_device::read_a13),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write_a13),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa15000, 0xa150ff, read16_delegate(FUNC(base_md_cart_slot_device::read_a15),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write_a15),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa14000, 0xa14003, write16_delegate(FUNC(base_md_cart_slot_device::write_tmss_bank),(base_md_cart_slot_device*)m_picocart));

	m_vdp->stop_timers();
}

static MACHINE_CONFIG_START( pico, pico_state )
	MCFG_FRAGMENT_ADD( md_ntsc )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pico_mem)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")
	MCFG_DEVICE_REMOVE("ymsnd")

	MCFG_MACHINE_START_OVERRIDE( pico_state, pico )
	MCFG_MACHINE_RESET_OVERRIDE( pico_base_state, ms_megadriv )

	MCFG_PICO_CARTRIDGE_ADD("picoslot", pico_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","pico")

	MCFG_SOUND_ADD("315_5641", SEGA_315_5641_PCM, UPD7759_STANDARD_CLOCK*2)
	MCFG_UPD7759_DRQ_CALLBACK(WRITELINE(pico_state,sound_cause_irq))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.16)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( picopal, pico_state )
	MCFG_FRAGMENT_ADD( md_pal )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pico_mem)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")
	MCFG_DEVICE_REMOVE("ymsnd")

	MCFG_MACHINE_START_OVERRIDE( pico_state, pico )
	MCFG_MACHINE_RESET_OVERRIDE( pico_base_state, ms_megadriv )

	MCFG_PICO_CARTRIDGE_ADD("picoslot", pico_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","pico")

	MCFG_SOUND_ADD("315_5641", SEGA_315_5641_PCM, UPD7759_STANDARD_CLOCK*2)
	MCFG_UPD7759_DRQ_CALLBACK(WRITELINE(pico_state,sound_cause_irq))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.16)
MACHINE_CONFIG_END



ROM_START( pico )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START( picou )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

ROM_START( picoj )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END


DRIVER_INIT_MEMBER(pico_base_state, pico)
{
	DRIVER_INIT_CALL(megadrie);
	DRIVER_INIT_CALL(mess_md_common);

	m_version_hi_nibble = 0x60; // Export PAL
}

DRIVER_INIT_MEMBER(pico_base_state, picou)
{
	DRIVER_INIT_CALL(megadriv);
	DRIVER_INIT_CALL(mess_md_common);

	m_version_hi_nibble = 0x40; // Export NTSC
}

DRIVER_INIT_MEMBER(pico_base_state, picoj)
{
	DRIVER_INIT_CALL(megadrij);
	DRIVER_INIT_CALL(mess_md_common);

	m_version_hi_nibble = 0x00; // JPN NTSC
}


CONS( 1994, pico,       0,         0,      picopal,         pico, pico_base_state,   pico,    "Sega",   "Pico (Europe, PAL)", MACHINE_NOT_WORKING)
CONS( 1994, picou,      pico,      0,      pico,            pico, pico_base_state,   picou,   "Sega",   "Pico (USA, NTSC)", MACHINE_NOT_WORKING)
CONS( 1993, picoj,      pico,      0,      pico,            pico, pico_base_state,   picoj,   "Sega",   "Pico (Japan, NTSC)", MACHINE_NOT_WORKING)

/*

This looks a lot like a Pico with extra sound hardware...
 YMZ263B is the basis of a Sound Blaster Clone
 YMF262-M is the OPL3

YAMAHA - MIXT BOOK PLAYER COPERA
MMG-1


            +-------------+
            |             |
            |------+      |
            |      |      |
 MIDI OUT <--      |      |
            |      |      |
            |------+      |
            |             |              +------------------------------------------------------------------------------------------------------+
            |------+      |              |                                    +--+                                                              |
            |      |      |              |                                    |YM|                                                              |
  MIDI IN <--      |      |              |                                    |7 |                                                              |
            |      |      |              |                                    |12|                             +---------+                      |
            |------+ +--+ |       +------+                                    |8B|                             | YAMAHA  |                      |
            |        |||| |       |                                           +--+                             | YMZ263B |                      |
            +---------||--+       |                              +--------+                                    |         |                      |
                      ||          |                              |YM7 128B|                                    +---------+    +---+             |
                      ||          |                              +--------+                                                   |YMF|             |
            +---------||----------+                                                                                           |262|             |
            |         ||                                                                                                      |-M |             |
            |         ||                                                                                                      +---+             |
            |         ||                                                                                                                        |
            |         ||                                                           SEGA                                                         |
            |------+  ||                                                 1993 837-9845 COPERA                                                   |
            |      |  ||                                                      MADE IN JAPAN                                                  +--|
            |      |  ||                                                           VA0                                                       |  --> PEN
  CONTROL <--      |  ||                                                                                                                     |  |
            |      |  ||                                                                                                                     +--|
            |      |  ||                                                                                                                        |
            |      |  ||                                                                                                                        |
            |------+  ||                                                                                         +----+                         |
            |         ||                                                                                         |XTAL|  +------------+         |
            |         ||                                                                                         |    |  | OKI JAPAN  |      +----+
            |         ||                                          +----------+                                   |53.6|  | M54C864-80 |      |    |
            |        ||||                                         |   SEGA   |                                   |93  |  +------------+      |    |
            |        +--+                                         | 315-5639 |                                   | Mhz|                      |    |
            |                                                     |       U11|                                   +----+  +-------------+     |  E |
            |                                                     |9341PD025 |                                           |             |     |  X |
            |--+                                                  +----------+                                           |     SEGA    |     |  T |
      MIC <--  |                                                                     +----+  +----+                      |  315-5313A  |     |  E |
            |  |                                                                     |TC51|  |TC51|                      |   FC1001    |     |  N |
            |--+                                                                     |832A|  |832A|                      |  9331 AASG  |     |  D |
            |                                                                        |FL-1|  |FL-1|                      |             |     |  E |
            |                                                                        |0   |  |0   |                      |             |     |  D |
            |                                                                        |    |  |    |                      |             |     |    |
            |--+                                                                     +----+  +----+                      +-------------+     |    |
          +--  |                                                                                                                             |  C |
          | |--+                                                                                                                             |  O |
S-AUDIO <-| |                              +-----------+           +-------+                                                                 |  N |
          | |--+                           |   SEGA    |           | SEGA  |          +-----------+                                          |  N |
          +--  |                           | 315-5640  |           |315-564|          |   3D4 UA  |                                          |  E |
            |--+                           | 9333 W26  |           |1      |          |HD68HC000CP|                                          |  C |
            |                              |           |           |D77591 |          |8          |                                          |  T |
            |--+                           |           |           +-------+          |           |                                          |  O |
  S-VIDEO <--  |                           |           |                              |           |                                          |  R |
            |--+                           |           |                              |           |                                          |    |
            |                              +-----------+                              |      JAPAN|                                          |    |
            |                                                                         +-----------+                                          |    |
            |--+                                                                                                                             |    |
       VCC --> |                                                                      +----------------------------------------------+       |    |
            |--+                                                                      |                  CARTRIDGE                   |       +----+
            |                                                                         |                  CONNECTOR                   |          |
            |                                                                         +----------------------------------------------+          |
            +-----------------------------------------------------------------------------------------------------------------------------------+

*/



class copera_state : public pico_base_state
{
public:
	copera_state(const machine_config &mconfig, device_type type, std::string tag)
	: pico_base_state(mconfig, type, tag),
	m_picocart(*this, "coperaslot") { }

	required_device<copera_cart_slot_device> m_picocart;
	DECLARE_MACHINE_START(copera);
};



static ADDRESS_MAP_START( copera_mem, AS_PROGRAM, 16, copera_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM

	AM_RANGE(0x800000, 0x80001f) AM_READWRITE(pico_68k_io_read, pico_68k_io_write)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000)
ADDRESS_MAP_END



static SLOT_INTERFACE_START(copera_cart)
	SLOT_INTERFACE_INTERNAL("rom",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_sram",  MD_ROM_SRAM)   // not sure these are needed...
	SLOT_INTERFACE_INTERNAL("rom_sramsafe",  MD_ROM_SRAM)   // not sure these are needed...
SLOT_INTERFACE_END

MACHINE_START_MEMBER(copera_state,copera)
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x000000, 0x7fffff, read16_delegate(FUNC(base_md_cart_slot_device::read),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa13000, 0xa130ff, read16_delegate(FUNC(base_md_cart_slot_device::read_a13),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write_a13),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa15000, 0xa150ff, read16_delegate(FUNC(base_md_cart_slot_device::read_a15),(base_md_cart_slot_device*)m_picocart), write16_delegate(FUNC(base_md_cart_slot_device::write_a15),(base_md_cart_slot_device*)m_picocart));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa14000, 0xa14003, write16_delegate(FUNC(base_md_cart_slot_device::write_tmss_bank),(base_md_cart_slot_device*)m_picocart));

	m_sega_315_5641_pcm->reset_w(0);
	m_sega_315_5641_pcm->start_w(0);
	m_sega_315_5641_pcm->reset_w(1);
	m_sega_315_5641_pcm->start_w(1);

}

static MACHINE_CONFIG_START( copera, copera_state )
	MCFG_FRAGMENT_ADD( md_ntsc )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(copera_mem)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")
	MCFG_DEVICE_REMOVE("ymsnd")

	MCFG_MACHINE_START_OVERRIDE( copera_state, copera )
	MCFG_MACHINE_RESET_OVERRIDE( pico_base_state, ms_megadriv )

	MCFG_COPERA_CARTRIDGE_ADD("coperaslot", copera_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","copera")

	MCFG_SOUND_ADD("315_5641", SEGA_315_5641_PCM, UPD7759_STANDARD_CLOCK)
	MCFG_UPD7759_DRQ_CALLBACK(WRITELINE(copera_state,sound_cause_irq))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.16)
MACHINE_CONFIG_END



ROM_START( copera )
	ROM_REGION(MD_CPU_REGION_SIZE, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASEFF)
ROM_END

CONS( 1993, copera,       0,         0,      copera,         pico, pico_base_state,   picoj,    "Yamaha / Sega",   "Yamaha Mixt Book Player Copera", MACHINE_NOT_WORKING)
