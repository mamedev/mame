// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                         -= Funcube series Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    TMP68301* or ColdFire + H8/3007 + PIC12C508

Video  :    DX-101

Sound  :    OKI M9810

OSC    :    50.00000MHz
            14.74560MHz
            25.44700MHz (for Funcube 2 onward)

*   The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

-------------------------------------------------------------------------------------------
Ordered by Board        Year    Game                                    By
-------------------------------------------------------------------------------------------
P0-140B                 2000    Funcube                                 Namco
B0-006B                 2001-2  Funcube 2 - 5                           Namco
-------------------------------------------------------------------------------------------

TODO:

- Proper emulation of the Touchscreen device.
- Hacked to run, as they use a ColdFire CPU.
- Pay-out key causes "unknown error" after coin count reaches 0.

***************************************************************************/

#include "emu.h"

#include "funcube_touchscreen.h"

#include "cpu/m68000/mcf5206e.h"
#include "cpu/m68000/tmp68301.h"
#include "cpu/h8/h83006.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/okim9810.h"
#include "video/x1_020_dx_101.h"

#include "emupal.h"
#include "speaker.h"

#define LOG_DEBUG   (1U << 1)

#define LOG_ALL     (LOG_DEBUG)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGDEBUG(...)  LOGMASKED(LOG_DEBUG, __VA_ARGS__)


class funcube_state : public driver_device
{
public:
	funcube_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sub(*this, "sub"),
		m_video(*this, "video"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_oki(*this, "oki"),
		m_nvram(*this, "nvram", 0x180, ENDIANNESS_BIG),
		m_in_debug(*this, "DEBUG"),
		m_in_switch(*this, "SWITCH"),
		m_in_battery(*this, "BATTERY"),
		m_leds(*this, "led%u", 0U)
	{ }

	void funcube(machine_config &config) ATTR_COLD;
	void funcube2(machine_config &config) ATTR_COLD;

	void init_funcube() ATTR_COLD;
	void init_funcube2() ATTR_COLD;
	void init_funcube3() ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, uint8_t data) { m_nvram[offset] = data; }

	uint16_t debug_r();
	uint8_t coins_r();
	void leds_w(uint8_t data);
	uint8_t outputs_r();
	void outputs_w(uint8_t data);
	uint8_t battery_r();

	void funcube2_map(address_map &map) ATTR_COLD;
	void funcube_map(address_map &map) ATTR_COLD;
	void funcube_sub_map(address_map &map) ATTR_COLD;

	void funcube_debug_outputs();

	required_device<cpu_device> m_maincpu;
	required_device<h83007_device> m_sub;
	required_device<x1_020_dx_101_device> m_video;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_device<okim9810_device> m_oki;

	memory_share_creator<uint8_t> m_nvram;

	required_ioport m_in_debug;
	required_ioport m_in_switch;
	required_ioport m_in_battery;

	output_finder<7> m_leds;

	uint8_t m_outputs = 0, m_funcube_leds = 0;
	uint64_t m_coin_start_cycles = 0;
	uint8_t m_hopper_motor = 0;
};


/***************************************************************************


                                 Memory Maps


***************************************************************************/

uint16_t funcube_state::debug_r()
{
	uint16_t ret = m_in_debug->read();

	// This bits let you move the crosshair in the inputs / touch panel test with a joystick
	if (!(m_screen->frame_number() % 3))
		ret |= 0x3f;

	return ret;
}

void funcube_state::funcube_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x400002, 0x400003).r(FUNC(funcube_state::debug_r));
	map(0x400006, 0x400007).r("watchdog", FUNC(watchdog_timer_device::reset16_r)).nopw();

	map(0x500001, 0x500001).rw(m_oki, FUNC(okim9810_device::read_status), FUNC(okim9810_device::write_command));
	map(0x500003, 0x500003).w(m_oki, FUNC(okim9810_device::write_tmp_register));

	map(0x800000, 0x83ffff).rw(m_video, FUNC(x1_020_dx_101_device::spriteram_r), FUNC(x1_020_dx_101_device::spriteram_w));
	map(0x840000, 0x84ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  // Palette
	map(0x860000, 0x86003f).rw(m_video, FUNC(x1_020_dx_101_device::vregs_r), FUNC(x1_020_dx_101_device::vregs_w));

	map(0xc00000, 0xc002ff).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask32(0x00ff00ff);
}

void funcube_state::funcube2_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00200000, 0x0020ffff).ram();

	map(0x00500002, 0x00500003).r(FUNC(funcube_state::debug_r));
	map(0x00500004, 0x00500007).r("watchdog", FUNC(watchdog_timer_device::reset32_r)).nopw();

	map(0x00600001, 0x00600001).rw(m_oki, FUNC(okim9810_device::read_status), FUNC(okim9810_device::write_command));
	map(0x00600003, 0x00600003).w(m_oki, FUNC(okim9810_device::write_tmp_register));

	map(0x00800000, 0x0083ffff).rw(m_video, FUNC(x1_020_dx_101_device::spriteram_r), FUNC(x1_020_dx_101_device::spriteram_w));
	map(0x00840000, 0x0084ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x00860000, 0x0086003f).rw(m_video, FUNC(x1_020_dx_101_device::vregs_r), FUNC(x1_020_dx_101_device::vregs_w));

	map(0x00c00000, 0x00c002ff).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask32(0x00ff00ff);

//  map(0xf0000000, 0xf00001ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_r), FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_w)); // technically this can be moved with MBAR
	map(0xffffe000, 0xffffffff).ram();    // SRAM
}

// Sub CPU

void funcube_state::funcube_sub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x20017f).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask16(0xffff);
}


// Simulate coin drop through two sensors

static constexpr XTAL FUNCUBE_SUB_CPU_CLOCK = XTAL(14'745'600);

uint8_t funcube_state::coins_r()
{
	uint8_t ret = m_in_switch->read();
	uint8_t coin_bit0 = 1; // active low
	uint8_t coin_bit1 = 1;

	const uint8_t hopper_bit = (m_hopper_motor && !(m_screen->frame_number() % 20)) ? 1 : 0;

	const uint64_t coin_total_cycles = FUNCUBE_SUB_CPU_CLOCK.value() / (1000/10);

	if (m_coin_start_cycles)
	{
		const uint64_t elapsed = m_sub->total_cycles() - m_coin_start_cycles;

		if (elapsed < coin_total_cycles/2)
			coin_bit0 = 0;
		else if (elapsed < coin_total_cycles)
			coin_bit1 = 0;
		else
		{
			if (!machine().side_effects_disabled())
				m_coin_start_cycles = 0;
		}
	}
	else
	{
		if (!machine().side_effects_disabled())
		{
			if (!(ret & 1))
				m_coin_start_cycles = m_sub->total_cycles();
		}
	}

	return (ret & ~7) | (hopper_bit << 2) | (coin_bit1 << 1) | coin_bit0;
}

void funcube_state::funcube_debug_outputs()
{
	//LOGDEBUG("LED: %02x OUT: %02x\n", m_funcube_leds, m_outputs);
}

void funcube_state::leds_w(uint8_t data)
{
	m_funcube_leds = data;

	m_leds[0] = BIT(~data, 0); // win lamp (red)
	m_leds[1] = BIT(~data, 1); // win lamp (green)

	// Set in a moving pattern: 0111 -> 1011 -> 1101 -> 1110
	m_leds[2] = BIT(~data, 4);
	m_leds[3] = BIT(~data, 5);
	m_leds[4] = BIT(~data, 6);
	m_leds[5] = BIT(~data, 7);

	funcube_debug_outputs();
}

uint8_t funcube_state::outputs_r()
{
	// Bits 1,2,3 read
	return m_outputs;
}

void funcube_state::outputs_w(uint8_t data)
{
	m_outputs = data;

	// Bits 0,1,3 written

	// Bit 0: hopper motor
	m_hopper_motor = BIT(~data, 0);

	// Bit 1: high on pay out

	// Bit 3: low after coining up, blinks on pay out
	m_leds[6] = BIT(~data, 3);

	funcube_debug_outputs();
}

uint8_t funcube_state::battery_r()
{
	return m_in_battery->read() ? 0x40 : 0x00;
}


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( funcube )
	PORT_START("SWITCH")    // c00030.l
	PORT_BIT(     0x01, IP_ACTIVE_LOW,  IPT_COIN1    ) PORT_IMPULSE(1)  // coin solenoid 1
	PORT_BIT(     0x02, IP_ACTIVE_HIGH, IPT_CUSTOM  )                  // coin solenoid 2
	PORT_BIT(     0x04, IP_ACTIVE_HIGH, IPT_CUSTOM  )                  // hopper sensor
	PORT_BIT(     0x08, IP_ACTIVE_LOW,  IPT_BUTTON2  )                  // game select
	PORT_BIT(     0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT(     0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME( "Reset Key" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW   )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW   )

	PORT_START("BATTERY")
	PORT_DIPNAME( 0x10, 0x10, "Battery" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )

	PORT_START("DEBUG")
	// 500002.w
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)

	// 500000.w
	PORT_DIPNAME(    0x00010000, 0x00000000, "Debug 0" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00020000, 0x00000000, "Debug 1" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00040000, 0x00000000, "Debug 2" )    // Touch-Screen
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00040000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00080000, 0x00000000, "Debug 3" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00100000, 0x00000000, "Debug 4" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00200000, 0x00000000, "Debug 5" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00200000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00400000, 0x00000000, "Debug 6" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00400000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00800000, 0x00000000, "Debug 7" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

void funcube_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_coin_start_cycles));
	save_item(NAME(m_hopper_motor));
	save_item(NAME(m_outputs));
	save_item(NAME(m_funcube_leds));
}

void funcube_state::machine_reset()
{
	m_coin_start_cycles = 0;
	m_hopper_motor = 0;
	m_outputs = 0;
	m_funcube_leds = 0;
}

// original on EVA board, no Coldfire
void funcube_state::funcube(machine_config &config)
{
	// TODO: check exact type and clock
	TMP68301(config, m_maincpu, XTAL(50'000'000)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &funcube_state::funcube_map);

	H83007(config, m_sub, FUNCUBE_SUB_CPU_CLOCK);
	m_sub->set_addrmap(AS_PROGRAM, &funcube_state::funcube_sub_map);
	m_sub->read_port4().set(FUNC(funcube_state::battery_r));
	m_sub->read_port7().set(FUNC(funcube_state::coins_r));
	m_sub->read_porta().set(FUNC(funcube_state::outputs_r));
	m_sub->write_porta().set(FUNC(funcube_state::outputs_w));
	m_sub->write_portb().set(FUNC(funcube_state::leds_w));

	FUNCUBE_TOUCHSCREEN(config, "touchscreen", 200).tx_cb().set(m_sub, FUNC(h8_device::sci_rx_w<1>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_size(0x200, 0x200);
	m_screen->set_visarea(0x0+1, 0x140-1+1, 0x00, 0xf0-1);
	m_screen->set_screen_update(m_video, FUNC(x1_020_dx_101_device::screen_update));
	m_screen->screen_vblank().set(m_video, FUNC(x1_020_dx_101_device::screen_vblank));
	m_screen->screen_vblank().append_inputline(m_maincpu, 0);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000+0xf0);    // extra 0xf0 because we might draw 256-color object with 16-color granularity

	X1_020_DX_101(config, m_video, XTAL(50'000'000));
	m_video->set_palette(m_palette);
	m_video->set_screen(m_screen);
	m_video->raster_irq_callback().set_inputline(m_maincpu, 1, HOLD_LINE);
	m_video->flip_screen_callback().set(FUNC(funcube_state::flip_screen_set));
	m_video->flip_screen_x_callback().set(FUNC(funcube_state::flip_screen_x_set));
	m_video->flip_screen_y_callback().set(FUNC(funcube_state::flip_screen_y_set));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	OKIM9810(config, m_oki, XTAL(4'096'000));
	m_oki->add_route(0, "speaker", 0.80, 0);
	m_oki->add_route(1, "speaker", 0.80, 1);
}


void funcube_state::funcube2(machine_config &config)
{
	funcube(config);
	MCF5206E(config.replace(), m_maincpu, XTAL(25'447'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &funcube_state::funcube2_map);
	downcast<mcf5206e_device &>(*m_maincpu).gpio_r_cb().set_ioport("BATTERY");

	m_sub->read_port4().set([]() -> u8 { return 0; }); // unused

	// video hardware
	m_screen->set_visarea(0x0, 0x140-1, 0x00, 0xf0-1);
	m_screen->screen_vblank().set(m_video, FUNC(x1_020_dx_101_device::screen_vblank));
	m_screen->screen_vblank().append_inputline(m_maincpu, 1);

	m_video->raster_irq_callback().set_nop(); // used?
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

FUNCUBE
EVA2E PCB

It's the same PCB as Namco Stars (P0-140B). It's a lot more complicated to dump
than the others because there are several surface mounted flash ROMs spread across
multiple daughterboards instead of simple socketed 32M DIP42 mask roms all on one PCB.

***************************************************************************/

ROM_START( funcube )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fcu1_prg0-f.u08", 0x00000, 0x80000, CRC(57f4f340) SHA1(436fc66409b254aba68ae33fc994bc270ce803a6) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fcu_0_iopr-0b.1b", 0x00000, 0x20000, CRC(87e3690f) SHA1(1b9dc573de31543884678df2dba2d6a74d6a2496) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u12", 0x000000, 0x200000, CRC(908b6baf) SHA1(cb5aa8c9b16abb17d8cc16d0d3b2f690a48ee503) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u13", 0x000001, 0x200000, CRC(8c31ca21) SHA1(e497ab1d7d30b41928a0c3db1ea7c3420376ad8c) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u14", 0x000002, 0x200000, CRC(4298d599) SHA1(d245206bc78de5f17da85ae6063b662cf9cf67aa) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u15", 0x000003, 0x200000, CRC(0669c78e) SHA1(0158fc4f90efa12d795b97873b8646c352864c69) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fcu1_snd-0a.u40", 0x000000, 0x200000, CRC(448539bc) SHA1(9e53bd5e29d1a88bf634e58bfeebccd3a1c2d866) )
ROM_END

/***************************************************************************

              FUNCUBE (BET) Series PCB for chapters 2 through 5

PCB Number: B0-006B (also known as EVA3_A system and is non-JAMMA)
+------------------------------------------------+
|+--+ S +---+ +---+                CN5           |
||  | W |   | |   |                          CN6?|
||  | 4 | U | | U |                              |
||  |   | 4 | | 4 |     +---+                CN2?|
||  |   | 2 | | 3 |     |DX |                    |
||  |   |   | |   |     |102|                    |
||C |   |   | |   |     +---+                    |
||N |   +---+ +---+                              |
||4 |                                            |
||  |      +----------+   M1                     |
||  |  M3  |          |                        C |
||  |      |   NEC    |   M1                   N |
||  |  M3  |  DX-101  |                        3 |
||  |      |          |                          |
||  |      |          |   50MHz                  |
|+--+      +----------+                          |
| PIC  25.447MHz         +-----------+           |
|  CN7                   |    U47    |           |
|                        +-----------+           |
|          +-----------+  +---+ +---+       D    |
|          |     U3    |  |OKI| |DX |       S    |
|    M2    +-----------+  |   | |102|       W    |
|                         +---+ +---+       1    |
|                 ispLSI2032                     |
|    M1                      +---+               |
|          +----------+      |IDT|           +--+|
|          |          |  C   |   |           |  ||
| C        | ColdFire |  N   +---+           |  ||
| N  M2    | XCF5206E |  8                   |  ||
| 1        |          |        +---+         |C ||
|          |          |        |H8 |         |N ||
|    M1    +----------+        +---+      D  |9 ||
|                         14.7456MHz      S  |  ||
|                            +-----------+W  |  ||
|            SW1      BAT1   |    U49    |2  +--+|
|                            +-----------+       |
+------------------------------------------------+

   CPU: ColdFire XCF5206EFT54 (160 Pin PQFP)
        Hitachi H8/3007 (64130007F20) used for touch screen I/O
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x2)
 Sound: OKI MSM9810B 8-Channel Mixing ADPCM Type Voice Synthesis LSI
   OSC: 50MHz, 25.447MHz & 14.7456MHz
 Other: Lattice ispLSI2032 - stamped "EVA3A"
        BAT1 - CR2032 3Volt

ColdFire XCF5206EFT54:
  68K/ColdFire V2 core family
  8K internal SRAM
  54MHz (max) Bus Frequency
  32bit External Bus Width
  2 UART Serial Interfaces
  2 Timer Channels

PIC - PIC12C508 MCU used for security
       Labeled FC21A for Funcube 2
       Labeled FC41A for Funcube 4

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q
IDT - IDT 7130 64-pin TQFP High-speed 1K x 8 Dual-Port Static RAM

CN1 - Unused 64 pin double row connecter
CN2?  2x2 connecter
CN3 - Unused 50 pin double row connecter
CN4 - 96 pin triple row connecter
CN5 - 2x3 pin connecter
CN6?  3x3 connecter
CN7 - Unused 20 pin connecter
CN8 - 8 pin single row connecter
CN9 - 40 pin double row connecter

DSW1 - 8 position dipswitch
DSW2 - 2 position dipswitch
SW1  - Pushbutton
SW4  - Single position slider switch

U3  - Is a 27C4002 EPROM
U49 - Is a 27C1001 EPROM
U42, U43 & U47 are mask ROMs read as 27C322

The same H8/3007 code "FC21 IOPR-0" at U49 is used for FUNCUBE 2,3,4 & 5

"15.5846kHz hsync, vsync between 60-61Hz (can't split the csync)"

***************************************************************************/

ROM_START( funcube2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc21_prg-0b.u3", 0x00000, 0x80000, CRC(add1c8a6) SHA1(bf91518da659098a4bad4e756533525fcc910570) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc21a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD32_WORD( "fc21_obj-0.u43", 0x000000, 0x400000, CRC(08cfe6d9) SHA1(d10f362dcde01f7a9855d8f76af3084b5dd1573a) )
	ROM_LOAD32_WORD( "fc21_obj-1.u42", 0x000002, 0x400000, CRC(4c1fbc20) SHA1(ff83691c19ce3600b31c494eaec26d2ac79e0028) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc21_voi0.u47", 0x000000, 0x200000, CRC(4a49370a) SHA1(ac10e2c25626965b49475767ef5a0ec3ba9a2d01) )
ROM_END

ROM_START( funcube3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc31_prg-0a.u4", 0x00000, 0x80000, CRC(ed7d70dd) SHA1(4ebfca9e60ab5e8de22821f0475abf515c83ce53) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x400, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc31a.u57", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD32_WORD( "fc31_obj-0.u43", 0x000000, 0x400000, CRC(08c5eb6f) SHA1(016d8f3067db487ccd47188142743897c9722b1f) )
	ROM_LOAD32_WORD( "fc31_obj-1.u42", 0x000002, 0x400000, CRC(4dadc76e) SHA1(cf82296b38dc22a618fd178816316af05f2459b3) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc31_snd-0.u47", 0x000000, 0x200000, CRC(36b03769) SHA1(20e583359421e0933c781a487fe5f7220052a6d4) )
ROM_END

ROM_START( funcube4 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc41_prg-0.u3", 0x00000, 0x80000, CRC(ef870874) SHA1(dcb8dc3f780ca135df55e4b4f3c95620597ad28f) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc41a", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD32_WORD( "fc41_obj-0.u43", 0x000000, 0x400000, CRC(9ff029d5) SHA1(e057f4929aa745ecaf9d4ff7e39974c82e440146) )
	ROM_LOAD32_WORD( "fc41_obj-1.u42", 0x000002, 0x400000, CRC(5ab7b087) SHA1(c600158b2358cdf947357170044dda2deacd4f37) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc41_snd0.u47", 0x000000, 0x200000, CRC(e6f7d2bc) SHA1(638c73d439eaaff8097cb0aa2684f9f7111bcade) )
ROM_END

ROM_START( funcube5 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc51_prg-0.u4", 0x00000, 0x80000, CRC(4e34c2d8) SHA1(1ace4f6edab291e69e5c36b15193fba62f4a6773) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc51a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD32_WORD( "fc51_obj-0.u43", 0x000000, 0x400000, CRC(116624b3) SHA1(c0b3dbe0ea4a0808222616c3ef77b2d1194a970a) )
	ROM_LOAD32_WORD( "fc51_obj-1.u42", 0x000002, 0x400000, CRC(35c6ec61) SHA1(424c9b66a2cdd5217d8a577d0179d1228112ee5b) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc51_snd-0.u47", 0x000000, 0x200000, CRC(2a504fe1) SHA1(911ad650bf48aa78d9cb3c64284aa526ceb519ba) )
ROM_END

void funcube_state::init_funcube()
{
//  uint16_t *main_cpu = (uint16_t *) memregion("maincpu")->base();
//
//  main_cpu[0x064/2] = 0x0000;
//  main_cpu[0x066/2] = 0x042a; // PIC protection?

}

void funcube_state::init_funcube2()
{
	uint32_t *main_cpu = (uint32_t *) memregion("maincpu")->base();

	main_cpu[0xa5c/4] = 0x4e713e3c;       // PIC protection?
	main_cpu[0xa74/4] = 0x4e713e3c;
	main_cpu[0xa8c/4] = 0x4e7141f9;

}

void funcube_state::init_funcube3()
{
	uint32_t *main_cpu = (uint32_t *) memregion("maincpu")->base();

	main_cpu[0x008bc/4] = 0x4a804e71;
	main_cpu[0x19f0c/4] = 0x4e714e71;
	main_cpu[0x19fb8/4] = 0x4e714e71;

}

// title logo is clipped
GAME( 2000, funcube,  0, funcube,  funcube, funcube_state, init_funcube,  ROT0, "Namco", "Funcube (v1.5)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 2001, funcube2, 0, funcube2, funcube, funcube_state, init_funcube2, ROT0, "Namco", "Funcube 2 (v1.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 2001, funcube3, 0, funcube2, funcube, funcube_state, init_funcube3, ROT0, "Namco", "Funcube 3 (v1.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 2001, funcube4, 0, funcube2, funcube, funcube_state, init_funcube2, ROT0, "Namco", "Funcube 4 (v1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 2002, funcube5, 0, funcube2, funcube, funcube_state, init_funcube2, ROT0, "Namco", "Funcube 5 (v1.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
