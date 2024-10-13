// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Roberto Fresca

/******************************************************************************

  Night Mare / Clean Octopus.

  E.F.O. S.A. Video IV System.
  Barcelona, Spain.

  This is a multi board system based on 2x RCA CDP1802 COSMAC (main & sound CPU),
  and two graphics devices (VDP) similar to TMS9928 (Texas Instruments), marked
  as EFO 90501. Also has an On-Screen settings instead of regular DIP switches,
  saving the config into a SDA2006 NV EEPROM. A big design for 1982.

  The game was designed by EFO/Playmatic in 1982, then dissappeared and remained
  lost till now.

  The name of the game for the local market was "Night Mare" (misspelled in
  some cases as "Nigth Mare"), but for the international market they changed the
  name to "Clean Octopus".


--------------------------------------------------------------------------------

  Main board:

  1x CPU RCA 1802 @ 3.60398 MHz.

  2x EFO 90501 (remarked TMS9928 VDP)

  16x TMS4116 (16384 x 1 DRAM) (for VDPs).
  2x MM2114N (1024 x 4 SRAM) (1024 KB for working RAM).
  1x SDA2006 (non volatile 512-bit EEPROM) (for settings).

  3x TMS2564 EPROMs.

  3x CDP1852 (I/O).

  1x Xtal @ 10.816 MHz.


  Sound board:

  1x CPU RCA 1802 @ 2.95008 MHz.

  1x EFO 90503 (seems a remarked TMS5220, speech synthesizer IC).

  1x CDP1824 (32 x 8) RAM.

  2x CDP1852 (I/O).

  1x Xtal @ 2.9500 MHz.

  Sound ROM is missing.
  Overall the board is pretty much dead, no interruptions observed, no video sync output.

  PCBs layout:

  .---------------------------------------------------.----------------------------------------------------------------------.
  |             SOUND-3   | O O O O |                 |          .-------.   .-------.        | O O O O O O O O O O O |  .---|
  |                       '---------'                 |     IC32 |TMS4116|   |TMS4116| IC18   '-----------------------'  | O |
  |     [ FUSE ]                                      |          '-------'   '-------'            J3                     | O |
  |                                               .---|          .-------.   .-------.                MALE CONNECTORS    | O |
  |     [ FUSE ]       S O U N D               GND| O |     IC33 |TMS4116|   |TMS4116| IC19                            J | O |
  |                                            SPK| O |          '-------'   '-------'                                 1 | O |
  |                    B O A R D                  '---|          .-------.   .-------.         .-------------.           | O |
  |                                                   |     IC34 |TMS4116|   |TMS4116| IC20    | CDP 1852 CE | IC8       | O |
  |                                   IC10            |          '-------'   '-------'         |     RCA     |           | O |
  |                               .----------.        |          .-------.   .-------.         '-------------'           >---|
  |                               |CD40106BE |        |     IC35 |TMS4116|   |TMS4116| IC21                              | O |
  |     IC2              IC6      '----------'        |          '-------'   '-------'         .-------------.           | O |
  | .----------.     .----------.                     |          .-------.   .-------.         | CDP 1852 CE | IC9     J | O |
  | |CDP1859CE |     |CDP1824CE |                     |     IC36 |TMS4116|   |TMS4116| IC22    |     RCA     |         2 | O |
  | '----------'     '----------'       IC9           |          '-------'   '-------'         '-------------'           | O |
  |     IC1      .--------------. .--------------.    |          .-------.   .-------.                                   | O |
  | .----------. |   MISSING    | |  EFO 90503   |    |     IC37 |TMS4116|   |TMS4116| IC23    .-------------.           | O |
  | |CDP1859CE | |    EPROM     | |              |    |          '-------'   '-------'         | CDP 1852 CE | IC10      '---|
  | '----------' '--------------' '--------------'   O=O         .-------.   .-------.         |     RCA     |               |
  |                    IC5               IC8         O=O    IC38 |TMS4116|   |TMS4116| IC24    '-------------'               |
  |                                .-------------.   O=O         '-------'   '-------'                                       |
  |       [2.9500]                 | CDP 1852 CE |   O=O J       .-------.   .-------.       .-----IC11------.               |
  |       [ XTAL ]                 |     RCA     |   O=O 5  IC39 |TMS4116|   |TMS4116| IC25  |   TMS 2564    |  .---IC1---.  |
  |                                '-------------'   O=O         '-------'   '-------'       |   NM1-1A1     |  |CDP1859CE|  |
  |                  IC6                 IC7         O=O                                     '---------------'  '---------'  |
  |         .-------------------.  .-------------.   O=O         .-------.   .-------.                          .---IC2---.  |
  |         |   CDP 1802 ACE    |  | CDP 1852 CE |   O=O    IC40 |74LS04N|   |74LS04N| IC26  .-----IC12------.  |CD4042BE |  |
  |         |        RCA        |  |     RCA     |   O=O         '-------'   '-------'       |   TMS 2564    |  '---------'  |
  |         '-------------------'  '-------------'    |        .---------. .---------.       |   NM1-1B1     |  .---IC3---.  |
  |                                                   |   IC41 |74LS373N | |74LS373N | IC27  '---------------'  |CD4011UBE|  |
  '---------------------------------------------------|        '---------' '---------'                          '---------'  |
                                                      |          .-------------------.       .-----IC13------.  .---IC4---.  |
                                                      |     IC28 |     EFO 90501     |       |   TMS 2564    |  |CD4071BE |  |
  .---------------------------------------------------|          |                   |       |   NM1-1C1     |  '---------'  |
  |                                                   |          '-------------------'       '---------------'  .---IC5---.  |
  |               P.S.U. - V. IV                      |          .-------------------.                          |CD4001UBE|  |
  |                                                   |     IC29 |     EFO 90501     |                          '---------'  |
  |                                                   |          |                   |       .---------.        .---IC6---.  |
  |                 P O W E R                         |          '-------------------'       | MM2114N | IC15   |CDP1853CE|  |
  |                                                   |                        [10.816]      '---------'        '---------'  |
  |                 S U P P L Y                      O=O +12                   [ XTAL ]      .---------.        .---IC7---.  |
  |                                                  O=O GND                                 | MM2114N | IC16   | SDA2006 |  |
  |                 B O A R D                        O=O -5   J               .---------.    '---------'        '---------'  |
  |                                                  O=O +5   6          IC30 | 74LS04N |                                    |
  |                                                  O=O +15                  '---------'                                    |
  |                                                  O=O +33                                 .-------------------.           |
  |                                                  O=O CLK                  .---------.    |   CDP 1802 ACE    | IC17      |
  |                                                  O=O +C              IC31 |74LS107N |    |        RCA        |           |
  |                                                   |       J4              '---------'    '-------------------'           |
  |                                                   |  .-----------.                                                       |
  |                                                   |  | O O O O O |             VIDEO IV                                  |
  '---------------------------------------------------'----------------------------------------------------------------------'


  PINOUTS
  -------

  Main Board:


  J1:  Pin marked 0 ---> IC9 CDP1852CE, pin 3.       J2:  Pin marked +C --> Vcc for external use.
  J1:  Pin marked 1 ---> IC9 CDP1852CE, pin 5.       J2:  Pin marked +C --> Vcc for external use.
  J1:  Pin marked 2 ---> IC9 CDP1852CE, pin 7.       J2:  Pin marked 1 ---> IC10 CDP1852CE, pin 6.
  J1:  Pin marked 3 ---> IC9 CDP1852CE, pin 9.       J2:  Pin marked 2 ---> IC10 CDP1852CE, pin 8.
  J1:  Pin marked 4 ---> IC9 CDP1852CE, pin 16.      J2:  Pin marked 3 ---> IC10 CDP1852CE, pin 10.
  J1:  Pin marked 5 ---> IC9 CDP1852CE, pin 18.      J2:  Pin marked 4 ---> IC10 CDP1852CE, pin 17.
  J1:  Pin marked 6 ---> IC9 CDP1852CE, pin 20.      J2:  Pin marked 5 ---> IC10 CDP1852CE, pin 14.
  J1:  Pin marked 7 ---> IC9 CDP1852CE, pin 22.


  J3:  Pin marked 0 ---> IC8 CDP1852CE, pin 3.       J4:  Pin marked SY --> Video Sync.
  J3:  Pin marked 1 ---> IC8 CDP1852CE, pin 5.       J4:  Pin marked R ---> Red.
  J3:  Pin marked 2 ---> IC8 CDP1852CE, pin 7.       J4:  Pin marked G ---> Green.
  J3:  Pin marked 3 ---> IC8 CDP1852CE, pin 9.       J4:  Pin marked B ---> Blue.
  J3:  Pin marked 4 ---> IC8 CDP1852CE, pin 16.      J4:  Pin marked GND -> GND.
  J3:  Pin marked 5 ---> IC8 CDP1852CE, pin 18.
  J3:  Pin marked 6 ---> IC8 CDP1852CE, pin 20.
  J3:  Pin marked 7 ---> IC8 CDP1852CE, pin 22.      J5:  Pin marked GND --> GND.
  J3:  Pin marked D ---> IC7 CDP1802ACE, pin 21.     J5:  Pins marked 1-8 -> CPU data bus.
  J3:  Pin marked GND -> GND    .                    J5:  Pin marked 9 ----> Mainboard IC5 CD4001, pin 11.
  J3:  Pin marked T ---> IC7 CDP1802ACE, pin 22.


  IC7, SDA2006 512-bit NV EEPROM:

                        .----v----.
                   -5V -|01     18|- N/C
                  +12V -|02  S  17|- N/C
                  +33V -|03  D  16|- IC17 CD1802, pin 04
                   GND -|04  A  15|- IC17 CD1802, pin 24
                   N/C -|05  2  14|- IC17 CD1802, pin 23
   IC17 CD1802, pin 03 -|06  0  13|- GND
                   N/C -|07  0  12|- IC10 CDP1852, pin 19
  IC10 CDP1852, pin 21 -|08  6  11|- GND
                   GND -|09     10|- +15V
                        '---------'

  01: Ubb (Substrate vias).                         10) CS2 (Chip select inputs: 12 bits CW).
  02: Udd (Supply voltage).                         11) CS2 (Chip select inputs: 12 bits CW).
  03: Uph (Programming voltage).                    12) Φ (Clock input).
  04: STWL (Lenght of Control Word: 8/12 bits).     13) L (Programming signal output) (load).
  05: N/C.                                          14) Dq (Data output).
  06: /RES (Reset input).                           15) INV (Invert input signals).
  07: N/C.                                          16) REC (Data input control) (receive).
  08: Di (Data input).                              17) Uss (Ground).
  09: CS3 (Chip select inputs: 8 or 12 bits CW).    18) Upi (Write voltage).


  Since STWL is connected to GND, the control word is set to 8-bit lenght.



  Sound Board:
  (also used on some Pinball machines)

  IC9, EFO 90503 (seems a remarked TMS5220, speech synthesizer IC).

               .-----v-----.
           D0 -|01       28|- /RS
              -|02       27|- /WS
              -|03   E   26|- D1
     .--- VBB -|04   F   25|-
  (*)|    VCC -|05   O   24|- D2
     '--- OSC -|06       23|-
              -|07   9   22|- D3
          SPK -|08   0   21|-
              -|09   5   20|-
              -|10   0   19|- D4
          GND -|11   3   18|- /RCI
           D5 -|12       17|- /INT
           D6 -|13       16|-
           D7 -|14       15|-
               '-----------'

  (*) Pin 4 is wired to a pot + resistor, and then connected to pin 6.
      This is surely to sync the 160 kHz needed for the device.


  TODO:

  - Soft reset doesn't work.
  - Verify video mixing (Press F2 to enter service mode, then press 1 + 2 to continue
    to settings screen. There's diagnostic color pattern at the top of screen)
  - Add sound hardware (ROM is missing)
  - Quitting MAME while in service mode settings screen will invalidate settings

******************************************************************************/

#include "emu.h"
#include "video/tms9928a.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/cdp1852.h"
#include "machine/sda2006.h"


namespace {

#define MASTER_CLOCK    XTAL(10'816'000)
#define SOUND_CLOCK     XTAL( 2'950'000)

class nightmare_state : public driver_device
{
public:
	nightmare_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "cdp1802")
		, m_soundcpu(*this,"cdp1802_sound")
		, m_vdc(*this, "vdc")
		, m_vdc2(*this, "vdc2")
		, m_eeprom(*this,"eeprom")
	{ }

	void nightmare(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_reset);

	int clear_r();
	int ef1_r();
	int ef2_r();
	void q_w(int state);
	void ic10_w(uint8_t data);
	void unkout_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	uint32_t screen_update_nightmare(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cosmac_device> m_maincpu;
	required_device<cosmac_device> m_soundcpu;
	required_device<tms9928a_device> m_vdc;
	required_device<tms9928a_device> m_vdc2;
	required_device<sda2006_device> m_eeprom;

	// cpu state
	int m_reset = 0;
	emu_timer *m_reset_timer = nullptr;
};

TIMER_CALLBACK_MEMBER(nightmare_state::clear_reset)
{
	m_reset = 1;
}

void nightmare_state::machine_start()
{
	save_item(NAME(m_reset));

	m_reset_timer = timer_alloc(FUNC(nightmare_state::clear_reset), this);
}

/* Machine Reset */

void nightmare_state::machine_reset()
{
	m_reset = 0;
	m_reset_timer->adjust(attotime::from_msec(200));
}

/* CDP1802 Interface */

int nightmare_state::clear_r()
{
	return m_reset;
}

void nightmare_state::q_w(int state)
{
	m_eeprom->write_clock(state);
}

int nightmare_state::ef1_r()
{
	//EEPROM Inv ???

	return 0;
}


int nightmare_state::ef2_r()
{
	//EEPROM Dq data read;
	return m_eeprom->read_data();
}


void nightmare_state::ic10_w(uint8_t data)
{
  /*
    7 - EEPROM Di
    6 - EEPROM Clock
    5 - J2
    4 - J2
    3 - J2
    2 - J2
    1 - J2
    0 - ?
  */

	m_eeprom->write_data((data&0x80) ?1:0);
	m_eeprom->write_enable((data&0x40) ?1:0);
}


void nightmare_state::unkout_w(uint8_t data)
{
  // J3
}

void nightmare_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x83ff).ram();
}

void nightmare_state::io_map(address_map &map)
{
	map(1, 1).r("ic8", FUNC(cdp1852_device::read)).w(FUNC(nightmare_state::unkout_w));
	map(2, 2).r("ic9", FUNC(cdp1852_device::read)).w("ic10", FUNC(cdp1852_device::write));

	map(4, 5).rw(m_vdc, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(6, 7).rw(m_vdc2, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
}

void nightmare_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
}

void nightmare_state::sound_io_map(address_map &map)
{
}

uint32_t nightmare_state::screen_update_nightmare(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// combine two buffers (additive?)
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t const *const bitmap1 = &m_vdc2->get_bitmap().pix(y);
		uint32_t const *const bitmap2 = &m_vdc->get_bitmap().pix(y);
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint32_t p1 = bitmap1[x];
			uint32_t p2 = bitmap2[x];
			uint32_t result = 0;

			for (int shift=0; shift<32;shift+=8)
			{
				uint32_t const data = ((p2>>shift)&0xff)+((p1>>shift)&0xff);
				result|=((data>0xff)?0xff:data)<<shift;
			}
			dst[x]=result;
		}
	}

	return 0;
}

static INPUT_PORTS_START( nightmare )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("EF")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_WRITE_LINE_DEVICE_MEMBER("cdp1802", cosmac_device, ef3_w) //ic17 - cpu
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER("cdp1802", cosmac_device, ef4_w)
INPUT_PORTS_END


void nightmare_state::nightmare(machine_config &config)
{
	/* main cpu */
	CDP1802(config, m_maincpu, MASTER_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &nightmare_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &nightmare_state::io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(nightmare_state::clear_r));
	m_maincpu->q_cb().set(FUNC(nightmare_state::q_w));
	m_maincpu->ef1_cb().set(FUNC(nightmare_state::ef1_r));
	m_maincpu->ef2_cb().set(FUNC(nightmare_state::ef2_r));
	m_maincpu->tpb_cb().set("ic10", FUNC(cdp1852_device::clock_w));

	/* sound cpu */
	CDP1802(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &nightmare_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &nightmare_state::sound_io_map);
	m_soundcpu->set_disable();

	/* i/o hardware */
	cdp1852_device &ic8(CDP1852(config, "ic8"));
	ic8.mode_cb().set_constant(0);
	ic8.di_cb().set_ioport("IN0");

	cdp1852_device &ic9(CDP1852(config, "ic9"));
	ic9.mode_cb().set_constant(0);
	ic9.di_cb().set_ioport("IN1");

	cdp1852_device &ic10(CDP1852(config, "ic10"));
	ic10.mode_cb().set_constant(1);
	ic10.do_cb().set(FUNC(nightmare_state::ic10_w));

	SDA2006(config, m_eeprom);

	/* video hardware */
	EFO90501( config, m_vdc, MASTER_CLOCK );
	m_vdc->set_screen("screen");
	m_vdc->set_vram_size(0x4000);

	EFO90501( config, m_vdc2, MASTER_CLOCK );
	m_vdc2->set_screen("screen");
	m_vdc2->set_vram_size(0x4000);
	m_vdc2->int_callback().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nightmare_state::screen_update_nightmare));
}


ROM_START( nightmare )
	ROM_REGION( 0x6000, "cdp1802", 0 )
	ROM_LOAD( "nm1-ia1.bin", 0x0000, 0x2000, CRC(5d648f62) SHA1(028a47d4b1b4910d0d4e00f81d4e94a5478834d3) )
	ROM_LOAD( "nm1-ib1.bin", 0x2000, 0x2000, CRC(c10695f7) SHA1(929467fe7529782e8181d3caae3a67bb0a8d8753) )
	ROM_LOAD( "nm1-ic1.bin", 0x4000, 0x2000, CRC(a3117246) SHA1(ca9601401f7ab34200c969e41ffae50bee0aca4d) )

	ROM_REGION( 0x10000, "cdp1802_sound", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x4000, NO_DUMP )

	ROM_REGION( 0x40, "eeprom", 0 )
	ROM_LOAD( "eeprom", 0x00, 0x40, CRC(7824e1f8) SHA1(2ccac62b4e8abcb2b3d66fa4025947fea184664e) )
ROM_END

} // anonymous namespace


GAME( 1982, nightmare, 0,        nightmare, nightmare,   nightmare_state,   empty_init, ROT90, "E.F.O.", "Night Mare (Spain)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
