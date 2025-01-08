// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  Wild Poker
  TAB Austria.

  Preliminary driver by Roberto Fresca.


  Games running in this hardware:

  * Wild Poker (ver. D 1.01),         199?, TAB Austria.


  The HD63484 ACRTC support is incomplete,
  due to the preliminary emulation state.

*******************************************************************************

  Hardware Notes:
  ---------------

  CPU:
  - 1x MC68000P12        ; 68000 CPU @ 12 MHz, from Motorola.
  - 1x D8751H            ; 8751 MCU (3.6864 MHz?)

  Sound device:
  - 1x AY8930            ; Sound IC, from Microchip Technology.

  Video:
  - 1x HD63484CP8 @ 8MHz ; Advanced CRT Controller (ACRTC), from Hitachi Semiconductor.
  - 1x HD63485CP64       ; Hitachi - Graphic Memory Interface Controller (GMIC).
  - 2x HD63486CP32       ; Hitachi - Graphic Video Attribute Controller (GVAC).

  Other:
  - 1x MC68681           ; Motorola - Dual Asynchronous Receiver/Transmitter.
  - 4x XTALs....         ; 3.6864 / 12.000 / 26.000 / 24.000 MHz.

                                                                                          .--------.
  PCB Layout:                                                                           --+--------+--
  .---------------------------------------------------------------------------------------+        +-----------------------------------------------------.
  |                                                                                       |  DB9   |                                                     |
  |                        .--------.                    .--------.                       |        |                             .--------.              |
  |                        |::::::::|                    |::::::::|                       '--------'                             |74HCT32P|              |
  |                        '--------'                    '--------'                                                              '--------'              |
  |                        .--------.                    .---------. .----------.         .--------. .----------.  .----------.  .--------.              |
  |                        |LT1084CN|                    |SN75116N | | MM57410N |         |74HCT14P| |74HCT245P |  |74HCT245P |  |74HCT86P| .-------.    |
  |                        '--------'                    '---------' '----------'         '--------' '----------'  '----------'  '--------' |XTAL 3 |    |
  |                                                                                                                  .--------.   .-------. |       |    |
  |                                         .--------.    .---------.        .-..-.                                  |        |   |DM74S04| '-------'    |
  |                                         |  PC617 |    |74HCT14P |        | || |               .----------------. '--------'   '-------'              |
  |                                         '--------'    '---------'        '-''-'               |  inmos 8941-C  |                                     |
  |.--. .---------.  .---------.    .-----------. .--------.     .---------------.                |  IMS G176P-50  |           .----------. .----------. |
  ||..| |ULN2803A |  |74HCT533P|    |PC74HC245P | |74HCT125|     |    HYUNDAI    |                |                |           |HY53C464LS| |HY53C464LS| |
  ||..| '---------'  '---------'    '-----------' '--------'     |  HY6264LP-10  |                '----------------'           '----------' '----------' |
  ||..|                           .------------------------.     |  9040D KOREA  |  .-------------.        .-------------.     .----------. .----------. |
  ||..| .---------.  .---------.  |        AY8930 /P       |     '---------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..| |ULN2803A |  |74HCT533P|  |        9019CCA         |                        |             |        |             |     '----------' '----------' |
  ||..| '---------'  '---------'  |        TAIWAN          |     .---------------.  |    IE1 U    |        |    9117     |     .----------. .----------. |
  ||..|                           '------------------------'     |    HYUNDAI    |  | HD63484CP8  |        | HD63486CP32 |     |HY53C464LS| |HY53C464LS| |
  ||..| .---------.  .---------.                                 |  HY6264LP-10  |  |             |        |             |     '----------' '----------' |
  ||..| |ULN2803A |  |74HCT533P|       .--------. .--------.     |  9040D KOREA  |  |        Japan|        |        Japan|     .----------. .----------. |
  ||..| '---------'  '---------'  .---.|8      1| |8      1|     '---------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  |'--'                           |   ||  DSW1  | |  DSW2  |                        |             |        |             |     '----------' '----------' |
  |     .---------.  .---------.  '---''--------' '--------'                        '-------------'        '-------------'                               |
  |.--. |ULN2803A |  |74HCT533P|                                                                                                                         |
  ||..| '---------'  '---------'  .------------------------.                                                                   .----------. .----------. |
  ||..|                           |D8751H                  |                                                                   |HY53C464LS| |HY53C464LS| |
  ||..|                           |L0381103                |                        .-------------.        .-------------.     '----------' '----------' |
  ||..|  .--------.  .---------.  |          VD1.00        |  .------------------.  |             |        |             |     .----------. .----------. |
  ||..|  |MDP1603 |  |74HCT245P|  '------------------------'  |D27C020           |  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  |'--'  '--------'  '---------'  .-------.     .-------.     |                  |  |     9109    |        |    9117     |     '----------' '----------' |
  |                               |XTAL 1 |     |XTAL 2 |     |   VD / 1.01 / 3  |  | HD63485CP64 |        | HD63486CP32 |     .----------. .----------. |
  |.--.  .--------.  .---------.  |       |     |       |     '------------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..|  |MDP1603 |  |74HCT245P|  '-------'     '-------'                           |        Japan|        |        Japan|     '----------' '----------' |
  ||..|  '--------'  '---------'  .------------------------.                        |             |        |             |     .----------. .----------. |
  ||..|                           |        MC68681P        |                        |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..|  .--------.  .---------.  |         2C98R          |                        '-------------'        '-------------'     '----------' '----------' |
  ||..|  |MDP1603 |  |74HCT245P|  |        QQPQ9051        |                                                                                             |
  ||..|  '--------'  '---------'  '------------------------'  .------------------.                                             .--------.   .--------.   |
  ||..|                             .--------.    .--------.  |D27C020           |                                             |        |   |        |   |
  ||..|  .--------.  .---------.    |8      1|    |74HCT147|  |                  |                                             '--------'   '--------'   |
  ||..|  |MDP1603 |  |74HCT245P|    |  DSW3  |    '--------'  |   VD / 1.01 / 1  |                 .--------.  .--------.                                |
  ||..|  '--------'  '---------'    '--------'                '------------------'                 |74HCT138|  |74HCT74P|                                |
  |'--'                                                                     .----------. .-------. '--------'  '--------'                           .------.
  |                             .---------------------------------------.   | GAL16V8S | |74HCT74|                                                  |      |
  |      .-------.              |                                       |   '----------' '-------' .--------.  .--------.                           |      |
  |      |       |              |             MC68000P12                |  .--------.   .------.   |74HCT138|  |74HCT21P|                           |      |
  |      |Battery|              |               2C91E                   |  |74HCT04P|   |XTAL 4|   '--------'  '--------'                           |      |
  |      |       |              |              QZUZ9102                 |  '--------'   |      |   .--------.  .--------.                           |      |
  |      |       |              |                                       |  .--------.   '------'   |74HCT138|  |74HCT161|                           |      |
  |      '-------'              '---------------------------------------'  |74HCT14P|              '--------'  '--------'                           |      |
  |                                                                        '--------'  .-------.   .--------.  .--------.                           |      |
  |               .--.  .--.                                                           |74HCT08|   |74HCT21 |  |1      8|                           |      |
  |               |TL|  |TL|                                                           '-------'   '--------'  |  DSW4  |                           |      |
  |               '--'  '--'    ========================================                                       '--------'                           '------'
  |                            | |::::::::::::::::::::::::::::::::::::| |                                                                                |
  |                            | |::::::::::::::::::::::::::::::::::::| |                                                                                |
  |                             ========================================                                                                                 |
  '------------------------------------------------------------------------------------------------------------------------------------------------------'

  XTAL 1: 3.6864 MHz.
  XTAL 2: 12.000 MHz.
  XTAL 3: 26.000 MHz.
  XTAL 4: 24.000 MHz.

  TL: TL7705ACP


      DSW1:          DSW2:          DSW3:          DSW4:
   .--------.     .--------.     .--------.     .--------.
  1| oo oooo|8   1|oooooooo|8   1|oooooooo|8   1|  o     |8    ON
   |--------|     |--------|     |--------|     |--------|
   |o  o    |     |        |     |        |     |oo ooooo|     OFF
   '--------'     '--------'     '--------'     '--------'


*******************************************************************************

  *** Game Notes ***

  Nothing yet...


*******************************************************************************

  ---------------------------------
  ***  Memory Map (preliminary) ***
  ---------------------------------

  00000 - 7FFFF  ; ROM space.


*******************************************************************************

  DRIVER UPDATES:

  [2012-06-11]

  - Initial release.
  - Pre-defined Xtals.
  - Added ASCII PCB layout.
  - Started a preliminary memory map.
  - Added technical notes.


  TODO:

  - Improve memory map.
  - ACRTC support.
  - GFX decode.
  - Sound support.
  - A lot!.


*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/ds2401.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/hd63484.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MAIN_CLOCK  XTAL(12'000'000)
#define AY_CLOCK    MAIN_CLOCK / 8
#define SEC_CLOCK   XTAL(3'686'400)
#define AUX1_CLOCK  XTAL(26'000'000)
#define AUX2_CLOCK  XTAL(24'000'000)


class wildpkr_state : public driver_device
{
public:
	wildpkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_id(*this, "id"),
		m_dac(*this, "dac"),
		m_dac_clock(*this, "dacclock"),
		m_nvram(*this, "nvram")
	{ }

	void wildpkr(machine_config &config);
	void tabpkr(machine_config &config);

	void init_wildpkr();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	optional_device<ds2401_device> m_id;
	optional_device<dac_byte_interface> m_dac;
	optional_device<clock_device> m_dac_clock;

	optional_shared_ptr<u16> m_nvram;

	u16 m_clock_rate = 0U;

	void wildpkr_palette(palette_device &palette) const;
	u8 unknown_read8();
	void unknown_write8(u8 data);
	void nvram_w(offs_t offset, u16 data);
	u16 id_serial_r();
	void id_serial_w(u16 data);
	void out0_w(u16 data);
	void out1_w(u16 data);
	void dac_w(u8 data);
	void clock_start_w(u16 data);
	void clock_rate_w(u16 data);
	void unknown_trigger_w(u16 data);
	u16 tabpkr_irq_ack(offs_t offset);
	void cpu_space_map(address_map &map) ATTR_COLD;
	void hd63484_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void tabpkr_map(address_map &map) ATTR_COLD;
	void wildpkr_map(address_map &map) ATTR_COLD;
};


/*************************
*     Video Hardware     *
*************************/

void wildpkr_state::video_start()
{
}

void wildpkr_state::wildpkr_palette(palette_device &palette) const
{
}


/*************************
*      ACRTC Access      *
*************************/


/*************************
*      Misc Handlers     *
*************************/

u8 wildpkr_state::unknown_read8()
{
	return 0xff;
}

void wildpkr_state::unknown_write8(u8 data)
{
}

void wildpkr_state::nvram_w(offs_t offset, u16 data)
{
	m_nvram[offset] = data | 0xff00;
}

u16 wildpkr_state::id_serial_r()
{
	return m_id->read();
}

void wildpkr_state::id_serial_w(u16 data)
{
	m_id->write(data & 1);
}

void wildpkr_state::out0_w(u16 data)
{
}

void wildpkr_state::out1_w(u16 data)
{
}

void wildpkr_state::dac_w(u8 data)
{
	m_dac->write(data);
}

void wildpkr_state::clock_start_w(u16 data)
{
	if (data != 0 && m_clock_rate != 0)
		m_dac_clock->set_clock_scale(1.0 / m_clock_rate);
	else
		m_dac_clock->set_clock_scale(0.0);
}

void wildpkr_state::clock_rate_w(u16 data)
{
	m_clock_rate = data;
}

void wildpkr_state::unknown_trigger_w(u16 data)
{
}

/*************************
*      Memory Map        *
*************************/

void wildpkr_state::wildpkr_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x113fff).ram();
	map(0x800000, 0x800003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800080, 0x80009f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x800180, 0x800180).r(FUNC(wildpkr_state::unknown_read8));
	map(0x800181, 0x800181).w(FUNC(wildpkr_state::unknown_write8));
	map(0x800200, 0x800200).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x800202, 0x800202).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x800204, 0x800204).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x800280, 0x800280).w("aysnd", FUNC(ay8930_device::data_w));
	map(0x800282, 0x800282).w("aysnd", FUNC(ay8930_device::address_w));
	map(0x800285, 0x800285).r("aysnd", FUNC(ay8930_device::data_r)); // (odd!)
	map(0x800286, 0x800289).nopw();
}

void wildpkr_state::tabpkr_map(address_map &map)
{
	map(0x000000, 0x2fffff).rom();
	map(0x300000, 0x303fff).ram();
	map(0x400000, 0x400fff).ram().w(FUNC(wildpkr_state::nvram_w)).share("nvram");
	map(0x500000, 0x500003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x500021, 0x500021).rw("ramdac", FUNC(ramdac_device::index_r), FUNC(ramdac_device::index_w));
	map(0x500023, 0x500023).rw("ramdac", FUNC(ramdac_device::pal_r), FUNC(ramdac_device::pal_w));
	map(0x500025, 0x500025).rw("ramdac", FUNC(ramdac_device::mask_r), FUNC(ramdac_device::mask_w));
	map(0x500040, 0x50005f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x500060, 0x500061).rw(FUNC(wildpkr_state::id_serial_r), FUNC(wildpkr_state::id_serial_w));
	map(0x600000, 0x600001).portr("IN0").w(FUNC(wildpkr_state::out0_w));
	map(0x600002, 0x600003).portr("IN1").w(FUNC(wildpkr_state::out1_w));
	map(0x600004, 0x600005).portr("IN2");
	map(0x600004, 0x600004).w(FUNC(wildpkr_state::dac_w));
	map(0x700000, 0x700001).w(FUNC(wildpkr_state::clock_start_w));
	map(0x700002, 0x700003).w(FUNC(wildpkr_state::clock_rate_w));
	map(0x700004, 0x700007).w(FUNC(wildpkr_state::unknown_trigger_w));
	map(0x70000a, 0x70000b).nopw(); // only writes 0 at POST
}

void wildpkr_state::hd63484_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

/* Unknown R/W:


*/


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( wildpkr )
INPUT_PORTS_END

static INPUT_PORTS_START( tabpkr )
	PORT_START("IN0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


/*************************
*     Machine Start      *
*************************/

void wildpkr_state::machine_start()
{
/*
  ACRTC memory:

  00000-3ffff = RAM
  40000-7ffff = ROM
  80000-bffff = unused
  c0000-fffff = unused
*/
	m_clock_rate = 0;
}


void wildpkr_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void wildpkr_state::cpu_space_map(address_map &map)
{
	map(0xfffff2, 0xffffff).r(FUNC(wildpkr_state::tabpkr_irq_ack));
}

u16 wildpkr_state::tabpkr_irq_ack(offs_t offset)
{
	m_maincpu->set_input_line(offset+1, CLEAR_LINE);

	if (offset+1 == 2)
		return m_duart->get_irq_vector();
	else
		return m68000_device::autovector(offset+1);
}


/*************************
*    Machine Drivers     *
*************************/

void wildpkr_state::wildpkr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &wildpkr_state::wildpkr_map);
	//m_maincpu->set_vblank_int("screen", FUNC(wildpkr_state::irq2_line_hold)); // guess

	MC68681(config, m_duart, SEC_CLOCK);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 280);
	screen.set_visarea(0, 384-1, 0, 280-1);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");

	HD63484(config, "acrtc", 0).set_addrmap(0, &wildpkr_state::hd63484_map);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &wildpkr_state::ramdac_map);

	PALETTE(config, "palette", FUNC(wildpkr_state::wildpkr_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8930(config, "aysnd", AY_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.50);
}


void wildpkr_state::tabpkr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wildpkr_state::tabpkr_map);
	m_maincpu->set_periodic_int(FUNC(wildpkr_state::irq3_line_assert), attotime::from_hz(60*256));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &wildpkr_state::cpu_space_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // DS1220Y

	MC68681(config, m_duart, 3686400);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	DS2401(config, m_id, 0);

	CLOCK(config, m_dac_clock, 1500000); // base rate derived from program code
	m_dac_clock->signal_handler().set_inputline(m_maincpu, M68K_IRQ_5, ASSERT_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 280);
	screen.set_visarea(0, 384-1, 0, 280-1);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);

	HD63484(config, "acrtc", 0).set_addrmap(0, &wildpkr_state::hd63484_map);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &wildpkr_state::ramdac_map);

	PALETTE(config, "palette", FUNC(wildpkr_state::wildpkr_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AD557(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( wildpkr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vd_1.01_3.bin", 0x000000, 0x40000, CRC(d19d5609) SHA1(87eedb7daaa8ac33c0a73e4e849b9a0f76152261) )
	ROM_LOAD16_BYTE( "vd_1.01_1.bin", 0x000001, 0x40000, CRC(f10644ab) SHA1(5872fe41b8c7fec5e83011abdf82a85f064b734f) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "d8751h",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal6v8s.bin",  0x0000, 0x0117, CRC(389c63a7) SHA1(4ebb26a001ed14a9e96dd268ed1c7f298f0c086b) )
ROM_END

/* seems to be different hardware, but same basic video chips, keep here or move?

cpu 68000-16
Xtal 24Mhaz
cpu ram 2x 6264

Audio DAC AD557JN

video area
insg176p-66 ramdac?
hd63487cp  Memory interface and video attribute controller
hd63484cp8 advanced CRT controller
4x km44c258cz-10 rams

*/

ROM_START( tabpkr ) // Royal Poker V 1.85 Oct 29 1996 12:20:07
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop1851.bin", 0x000001, 0x80000, CRC(fbe13fa8) SHA1(7c19b6b4d9a9935b6feb70b6261bafc6d9afb59f) )
	ROM_LOAD16_BYTE( "rop1853.bin", 0x000000, 0x80000, CRC(e0c312b4) SHA1(57c64c82f723067b7b2f9bf3fdaf5aedeb4f9dc3) )
	// are these missing, or just unpopulated but checked anyway?
	/* reads 0x100000 - 0x1fffff ? - 2x sockets for same type of roms as above */
	/* reads 0x200000 - 0x2fffff ? - 1x socket for larger ROM? */

	ROM_REGION(8, "id", 0)
	ROM_LOAD("ds2401.bin", 0, 8, NO_DUMP)
	// Dummy data to appease POST
	ROM_FILL(0, 1, 0x66)
	ROM_FILL(1, 1, 0xfa)
	ROM_FILL(2, 1, 0xce)
	ROM_FILL(3, 1, 0xde)
	ROM_FILL(4, 1, 0xad)
	ROM_FILL(5, 1, 0xbe)
	ROM_FILL(6, 1, 0xef)
	ROM_FILL(7, 1, 0x01)
ROM_END



/*************************
*      Driver Init       *
*************************/

void wildpkr_state::init_wildpkr()
{
}

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT    STATE           INIT           ROT   COMPANY        FULLNAME                    FLAGS
GAME( 199?, wildpkr,   0,        wildpkr,  wildpkr, wildpkr_state,  init_wildpkr,  ROT0, "TAB Austria", "Wild Poker (ver. D 1.01)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1996, tabpkr,    0,        tabpkr,   tabpkr,  wildpkr_state,  init_wildpkr,  ROT0, "TAB Austria", "Royal Poker V 1.85",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
