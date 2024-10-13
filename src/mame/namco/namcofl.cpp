// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi
/*
    Namco System FL
    Driver by R. Belmont and ElSemi

    IMPORTANT (for Manual Calibration, MAME now loads these defaults instead)
    ---------
    To calibrate the steering, do the following:
    1) Delete the .nv file
    2) Start the game.  Speed Racer will guide you through the calibration
       (the "jump button" is mapped to Space).
       For Final Lap R, hold down service (9) and tap test (F2).  If you do
       not get an "initializing" message followed by the input test, keep doing
       it until you do.
    3) Exit MAME and restart the game, it's now calibrated.

TODO:
    speedrcr : Incorrect ROZ layer


PCB Layout
----------

SYSTEM-FL MAIN PCB 8636960100 (8636970100)
       |--------------------------------------------------------------------------------------------|
       |                                                                       |-----------------|  |
       |                                                                       |       4Z IC3 IC1| Z|
       |                21Y 20Y 19Y 18Y     16Y     14Y 13Y 12Y                |       4Y        | Y|
       |                    20X                                                |       4X        | X|
    |--|                                                                       |          IC4 IC2| W|
    |               22U 21U         18U     16U     14U 13U 12U                |    Sub Board    | U|
    |               22T             18T                                        |-----------------| T|
    |                                                                                               |
    |J          23S                                         12S                        4S    2S    S|
    |                                                                                               |
    |A          23R                                     13R                                        R|
    |                                                                                               |
    |M                  21P 20P 19P 18P                             10P    8P 7P    5P             P|
    |                       20N                                                               OSC1 N|
    |M                                                                                    OSC2      |
    |           23L     21L                             OSC3                              3L       L|
    |A                                              14K         11K                                K|
    |                                                                                               |
    |                           19J 18J 17J 16J                                                    J|
    |--|        23F     21F                                                                        F|
       |                                                                                            |
    |--|        23E                                     13E                                        E|
    |N          23D             19D                     13D                               3D       D|
    |A                                                                                              |
    |M          23C                                                                                C|
    |C  25B             21B                                                                        B|
    |O                                                                                              |
    |4              22A 21A     19A 18A     16A 15A 14A 13A 12A 11A 10A 9A 8A 7A 6A 5A    3A       A|
    |4                                                                                              |
    |--|                                                                                            |
       |25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1   |
       |--------------------------------------------------------------------------------------------|

RAM
---
4Y : CY7C199-25PC
4Z : CY7C199-25PC
5A : TC514256BZ-60
5P : LH528256N-70LL
6A : TC514256BZ-60
7A : TC514256BZ-60
7P : LH528256N-70LL
8A : TC514256BZ-60
8P : LH528256N-70LL
9A : TC514256BZ-60
10A: TC514256BZ-60
10P: LH528256N-70LL
11A: TC514256BZ-60
12A: TC514256BZ-60
12U: TC511632FL-70
12Y: TC511632FL-70
13U: TC511632FL-70
13Y: TC511632FL-70
14U: TC511632FL-70
14Y: TC511632FL-70
16U: TC511632FL-70
16Y: TC511632FL-70
19Y: M5M5178AFP-25
20Y: M5M5178AFP-25
21A: 65256BLFP-10T
21Y: M5M5178AFP-25
22A: 65256BLFP-10T
22T: LH528256N-70LL
22U: LH528256N-70LL
23E: M5M5256BFP-70LL
23F: M5M5256BFP-70LL

CUSTOM
------
4X : NAMCO C355 (sprite chip)
18Y: NAMCO 156
20X: NAMCO C116
21U: NAMCO 145
18T: NAMCO 123
23R: NAMCO C352
3D : NAMCO C380
4S : NAMCO 187
23L: NAMCO 75 (M37702 MCU)
11K: NAMCO 137
21B: NAMCO C345  9348EV  333791  VY06436A  NX25024K JAPAN
21F: NAMCO 195


ROMs - Main Board
-----------------
23S: mask ROM - SE1_VOI.23S (PCB LABEL 'VOICE'), mounted on a small plug-in PCB
     labelled MEMEXT 32M MROM PCB 8635909200 (8635909300). This chip is programmed in BYTE mode.
18U: MB834000 mask ROM - SE1_SSH.18U (PCB LABEL 'SSHAPE')
21P: MB838000 mask ROM - SE1_SCH0.21P (PCB LABEL 'SCHA0')
20P: MB838000 mask ROM - SE1_SCH1.20P (PCB LABEL 'SCHA1')
19P: MB838000 mask ROM - SE1_SCH2.19P (PCB LABEL 'SCHA2')
18P: MB838000 mask ROM - SE1_SCH3.18P (PCB LABEL 'SCHA3')
21L: M27C4002 EPROM - SE1_SPR.21L (PCB LABEL 'SPROG')
14K: MB834000 mask ROM - SE1_RSH.14K (PCB LABEL 'RSHAPE')
19J: MB838000 mask ROM - SE1_RCH0.19J (PCB LABEL 'RCHA0')
18J: MB838000 mask ROM - SE1_RCH1.18J (PCB LABEL 'RCHA1')
17J, 16J: RCH2, RCH3 but sockets not populated
19A: D27C4096 EPROM - SE2MPEA4.19A (PCB LABEL 'PROGE')
18A: D27C4096 EPROM - SE2MPOA4.18A (PCB LABEL 'PROGO')
16A: AM27C040 EPROM - SE1_DAT3.16A (PCB LABEL 'DATA3')
15A: AM27C040 EPROM - SE1_DAT2.15A (PCB LABEL 'DATA2')
14A: AM27C040 EPROM - SE1_DAT1.14A (PCB LABEL 'DATA1')
13A: AM27C040 EPROM - SE1_DAT0.13A (PCB LABEL 'DATA0')


ROMs - Sub Board
----------------
IC1: MB8316200 SOP44 mask ROM - SE1OBJ0L.IC1 (PCB LABEL 'OBJ0L')
IC2: MB8316200 SOP44 mask ROM - SE1OBJ0U.IC2 (PCB LABEL 'OBJ0U')
IC3: MB8316200 SOP44 mask ROM - SE1OBJ1L.IC3 (PCB LABEL 'OBJ1L')
IC4: MB8316200 SOP44 mask ROM - SE1OBJ1U.IC4 (PCB LABEL 'OBJ1U')


PALs
----
2S : PAL16L8BCN (PCB LABEL 'SYSFL-1')
3L : PAL16L8BCN (PCB LABEL 'SYSFL-2')
12S: PALCE16V8H (PCB LABEL 'SYSFL-3')
20N: PAL20L8BCN (PCB LABEL 'SYSFL-4')
19D: PALCE16V8H (PCB LABEL 'SYSFL-5')


OTHER
-----
OSC1: 80.000MHz
OSC2: 27.800MHz
OSC3: 48.384MHz
13D : KEYCUS2 (not populated)
3A  : Intel NG80960KA-20 (i960 CPU)
25B : Sharp PC9D10
23C : IR2C24AN
23D : IR2C24AN
13E : HN58C65 EEPROM

*/

#include "emu.h"

#include "namco_c123tmap.h"
#include "namco_c116.h"
#include "namco_c169roz.h"
#include "namco_c355spr.h"
#include "namcomcu.h"

#include "cpu/i960/i960.h"
#include "sound/c352.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "finalapr.lh"

#include <algorithm>


namespace {

class namcofl_state : public driver_device
{
public:
	namcofl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_c116(*this, "c116"),
		m_screen(*this, "screen"),
		m_c123tmap(*this, "c123tmap"),
		m_c169roz(*this, "c169roz"),
		m_c355spr(*this, "c355spr"),
		m_mcu(*this, "mcu"),
		m_workram(*this, "workram"),
		m_shareram(*this, "shareram"),
		m_mainbank(*this, "mainbank"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_misc(*this, "MISC"),
		m_accel(*this, "ACCEL"),
		m_brake(*this, "BRAKE"),
		m_wheel(*this, "WHEEL")
	{ }

	void namcofl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i960_cpu_device> m_maincpu;
	required_device<namco_c116_device> m_c116;
	required_device<screen_device> m_screen;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device<namco_c169roz_device> m_c169roz;
	required_device<namco_c355spr_device> m_c355spr;
	required_device<m37710_cpu_device> m_mcu;

	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_shareram;
	memory_view m_mainbank;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	required_ioport m_misc;
	required_ioport m_accel;
	optional_ioport m_brake;
	required_ioport m_wheel;

	emu_timer *m_raster_interrupt_timer = nullptr;
	emu_timer *m_vblank_interrupt_timer = nullptr;
	emu_timer *m_network_interrupt_timer = nullptr;
	uint8_t m_mcu_port6 = 0;
	uint32_t m_sprbank = 0;

	uint32_t unk1_r();
	uint8_t network_r(offs_t offset);
	uint32_t sysreg_r();
	void sysreg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void c116_w(offs_t offset, uint8_t data);
	uint16_t mcu_shared_r(offs_t offset);
	void mcu_shared_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t port6_r();
	void port6_w(uint8_t data);
	uint8_t port7_r();
	uint8_t dac6_r();
	void spritebank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(network_interrupt_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);
	TIMER_CALLBACK_MEMBER(raster_interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);
	int objcode2tile(int code);
	void tilemap_cb(uint16_t code, int *tile, int *mask);
	void roz_cb(uint16_t code, int *tile, int *mask, int which);
	void namcoc75_am(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void namcofl_state::tilemap_cb(uint16_t code, int *tile, int *mask)
{
	*tile = code;
	*mask = code;
}

void namcofl_state::roz_cb(uint16_t code, int *tile, int *mask, int which)
{
	*tile = code;
	*mask = code;
}


uint32_t namcofl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// compute window for custom screen blanking
	rectangle clip;
	//004c 016b 0021 0101 004a 0060 (finalapr*)
	//004b 016b 0021 0101 0144 0047 (speedrcr)
	clip.min_x = m_c116->get_reg(0) - 0x4b;
	clip.max_x = m_c116->get_reg(1) - 0x4b - 1;
	clip.min_y = m_c116->get_reg(2) - 0x21;
	clip.max_y = m_c116->get_reg(3) - 0x21 - 1;
	// intersect with master clip rectangle
	clip &= cliprect;

	bitmap.fill(m_c116->black_pen(), cliprect );

	for (int pri = 0; pri < 16; pri++)
	{
		m_c169roz->draw(screen, bitmap, clip, pri);
		if ((pri & 1) == 0)
		{
			m_c123tmap->draw(screen, bitmap, clip, pri >> 1);
		}

		m_c355spr->draw(screen, bitmap, clip, pri);
	}

	return 0;
}

// NOTE : The two low bits toggle banks (code + 0x4000) for two
//        groups of sprites.  I am unsure how to differentiate those groups
//        at this time however.

void namcofl_state::spritebank_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sprbank);
}

int namcofl_state::objcode2tile(int code)
{
	if (BIT(code, 13))
		return (m_sprbank << 13) | (code & 0x1fff);

	return code;
}


uint32_t namcofl_state::unk1_r()
{
	return 0xffffffff;
}

uint8_t namcofl_state::network_r(offs_t offset)
{
	if (offset == 1)
		return 0x7d;

	return 0xff;
}

uint32_t namcofl_state::sysreg_r()
{
	return 0;
}

void namcofl_state::sysreg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if ((offset == 2) && ACCESSING_BITS_0_7)  // address space configuration
	{
		// 0: RAM at 00000000, ROM at 10000000
		// 1: ROM at 00000000, RAM at 10000000
		m_mainbank.select(data & 1);
	}
}

// FIXME: remove this trampoline once the IRQ is moved into the actual device
void namcofl_state::c116_w(offs_t offset, uint8_t data)
{
	m_c116->write(offset, data);

	if ((offset & 0x180e) == 0x180a)
	{
		uint16_t triggerscanline=m_c116->get_reg(5)-(32+1);
		m_raster_interrupt_timer->adjust(m_screen->time_until_pos(triggerscanline));
	}
}

void namcofl_state::main_map(address_map &map)
{
	map(0x00000000, 0x1fffffff).view(m_mainbank);
	m_mainbank[0](0x00000000, 0x000fffff).flags(i960_cpu_device::BURST).ram().share(m_workram);
	m_mainbank[0](0x10000000, 0x100fffff).flags(i960_cpu_device::BURST).rom().region("maincpu", 0);
	m_mainbank[1](0x00000000, 0x000fffff).flags(i960_cpu_device::BURST).rom().region("maincpu", 0);
	m_mainbank[1](0x10000000, 0x100fffff).flags(i960_cpu_device::BURST).ram().share(m_workram);

	map(0x20000000, 0x201fffff).flags(i960_cpu_device::BURST).rom().region("data", 0);
	map(0x30000000, 0x30001fff).flags(i960_cpu_device::BURST).ram().share("nvram");
	map(0x30100000, 0x30100003).flags(i960_cpu_device::BURST).w(FUNC(namcofl_state::spritebank_w));
	map(0x30284000, 0x3028bfff).flags(i960_cpu_device::BURST).ram().share(m_shareram);
	map(0x30300000, 0x30303fff).flags(i960_cpu_device::BURST).ram(); // COMRAM
	map(0x30380000, 0x303800ff).flags(i960_cpu_device::BURST).r(FUNC(namcofl_state::network_r)).umask32(0x00ff00ff); // network registers
	map(0x30400000, 0x30407fff).flags(i960_cpu_device::BURST).r(m_c116, FUNC(namco_c116_device::read)).w(FUNC(namcofl_state::c116_w));
	map(0x30800000, 0x3080ffff).flags(i960_cpu_device::BURST).rw(m_c123tmap, FUNC(namco_c123tmap_device::videoram16_r), FUNC(namco_c123tmap_device::videoram16_w));
	map(0x30a00000, 0x30a0003f).flags(i960_cpu_device::BURST).rw(m_c123tmap, FUNC(namco_c123tmap_device::control16_r), FUNC(namco_c123tmap_device::control16_w));
	map(0x30c00000, 0x30c1ffff).flags(i960_cpu_device::BURST).rw(m_c169roz, FUNC(namco_c169roz_device::videoram_r), FUNC(namco_c169roz_device::videoram_w));
	map(0x30d00000, 0x30d0001f).flags(i960_cpu_device::BURST).rw(m_c169roz, FUNC(namco_c169roz_device::control_r), FUNC(namco_c169roz_device::control_w));
	map(0x30e00000, 0x30e1ffff).flags(i960_cpu_device::BURST).rw(m_c355spr, FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w)).share("objram");
	map(0x30f00000, 0x30f0000f).flags(i960_cpu_device::BURST).ram(); // NebulaM2 code says this is int enable at 0000, int request at 0004, but doesn't do much about it
	map(0x40000000, 0x4000005f).flags(i960_cpu_device::BURST).rw(FUNC(namcofl_state::sysreg_r), FUNC(namcofl_state::sysreg_w));
	map(0xfffffffc, 0xffffffff).flags(i960_cpu_device::BURST).r(FUNC(namcofl_state::unk1_r));
}


void namcofl_state::mcu_shared_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// HACK!  Many games data ROM routines redirect the vector from the sound command read to an RTS.
	// This needs more investigation.  nebulray and vshoot do NOT do this.
	// Timers A2 and A3 are set up in "external input counter" mode, this may be related.
#if 0
	if ((offset == 0x647c/2) && (data != 0))
	{
		data = 0xd2f6;
	}
#endif

	if (offset & 1)
		m_shareram[offset >> 1] = (m_shareram[offset >> 1] & ~(uint32_t(mem_mask) << 16)) | ((data & mem_mask) << 16);
	else
		m_shareram[offset >> 1] = (m_shareram[offset >> 1] & ~uint32_t(mem_mask)) | (data & mem_mask);

	// C75 BIOS has a very short window on the CPU sync signal, so immediately let the i960 at it
	if ((offset == 0x6000/2) && (data & 0x80))
	{
		m_mcu->yield();
	}
}

uint16_t namcofl_state::mcu_shared_r(offs_t offset)
{
	if (offset & 1)
		return m_shareram[offset >> 1] >> 16;
	else
		return m_shareram[offset >> 1];
}

uint8_t namcofl_state::port6_r()
{
	return m_mcu_port6;
}

void namcofl_state::port6_w(uint8_t data)
{
	m_mcu_port6 = data;
}

uint8_t namcofl_state::port7_r()
{
	switch (m_mcu_port6 & 0xf0)
	{
		case 0x00:
			return m_in0->read();

		case 0x20:
			return m_misc->read();

		case 0x40:
			return m_in1->read();

		case 0x60:
			return m_in2->read();

		default:
			break;
	}

	return 0xff;
}

uint8_t namcofl_state::dac6_r()
{
	return m_brake.read_safe(0xff);
}

void namcofl_state::namcoc75_am(address_map &map)
{
	map(0x002000, 0x002fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
	map(0x004000, 0x00bfff).ram().rw(FUNC(namcofl_state::mcu_shared_r), FUNC(namcofl_state::mcu_shared_w));
	map(0x200000, 0x27ffff).rom().region("c75data", 0);
}


static INPUT_PORTS_START( speedrcr )
	PORT_START("MISC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) // enters pages in test menu

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no gear button on this
	PORT_DIPNAME( 0x20, 0x20, "Freeze Screen" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // buttons are named because their use in navigating test mode isn't clear
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Weapon 3 / Test Menu Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Weapon 1 / Test Menu Modify")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Weapon 2 / Test Menu Up")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Start / Jump") // jump / start button

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	// no brake

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
INPUT_PORTS_END


static INPUT_PORTS_START( finalapr )
	PORT_START("MISC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE // gear
	PORT_DIPNAME( 0x20, 0x20, "Freeze Screen" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // freezes the game

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(6)

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(6)

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(6)
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(namcofl_state::network_interrupt_callback)
{
	m_maincpu->set_input_line(I960_IRQ0, ASSERT_LINE);
	m_network_interrupt_timer->adjust(m_screen->frame_period());
}


TIMER_CALLBACK_MEMBER(namcofl_state::vblank_interrupt_callback)
{
	m_maincpu->set_input_line(I960_IRQ2, ASSERT_LINE);
	m_vblank_interrupt_timer->adjust(m_screen->frame_period());
}


TIMER_CALLBACK_MEMBER(namcofl_state::raster_interrupt_callback)
{
	m_screen->update_partial(m_screen->vpos());
	m_maincpu->set_input_line(I960_IRQ1, ASSERT_LINE);
	m_raster_interrupt_timer->adjust(m_screen->frame_period());
}

TIMER_DEVICE_CALLBACK_MEMBER(namcofl_state::mcu_irq0_cb)
{
	m_mcu->set_input_line(M37710_LINE_IRQ0, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(namcofl_state::mcu_irq2_cb)
{
	m_mcu->set_input_line(M37710_LINE_IRQ2, HOLD_LINE);
}


void namcofl_state::machine_start()
{
	m_raster_interrupt_timer = timer_alloc(FUNC(namcofl_state::raster_interrupt_callback), this);
	m_network_interrupt_timer = timer_alloc(FUNC(namcofl_state::network_interrupt_callback), this);
	m_vblank_interrupt_timer =  timer_alloc(FUNC(namcofl_state::vblank_interrupt_callback), this);

	m_mainbank.select(1);

	save_item(NAME(m_mcu_port6));
	save_item(NAME(m_sprbank));
}


void namcofl_state::machine_reset()
{
	m_network_interrupt_timer->adjust(m_screen->time_until_pos(m_screen->visible_area().max_y + 3));
	m_vblank_interrupt_timer->adjust(m_screen->time_until_pos(m_screen->visible_area().max_y + 1));

	std::fill_n(&m_workram[0], m_workram.bytes() / 4, 0);
	m_mainbank.select(1);
}


void namcofl_state::namcofl(machine_config &config)
{
	I960(config, m_maincpu, 80_MHz_XTAL / 4); // i80960KA-20 == 20 MHz part
	m_maincpu->set_addrmap(AS_PROGRAM, &namcofl_state::main_map);

	NAMCO_C75(config, m_mcu, 48.384_MHz_XTAL / 3);
	m_mcu->set_addrmap(AS_PROGRAM, &namcofl_state::namcoc75_am);
	m_mcu->p6_in_cb().set(FUNC(namcofl_state::port6_r));
	m_mcu->p6_out_cb().set(FUNC(namcofl_state::port6_w));
	m_mcu->p7_in_cb().set(FUNC(namcofl_state::port7_r));
	m_mcu->an7_cb().set_ioport("ACCEL");
	m_mcu->an6_cb().set(FUNC(namcofl_state::dac6_r));
	m_mcu->an5_cb().set_ioport("WHEEL");
	m_mcu->an4_cb().set_constant(0xff);
	m_mcu->an3_cb().set_constant(0xff);
	m_mcu->an2_cb().set_constant(0xff);
	m_mcu->an1_cb().set_constant(0xff);
	m_mcu->an0_cb().set_constant(0xff);
	// TODO: IRQ generation for these
	TIMER(config, "mcu_irq0").configure_periodic(FUNC(namcofl_state::mcu_irq0_cb), attotime::from_hz(60));
	TIMER(config, "mcu_irq2").configure_periodic(FUNC(namcofl_state::mcu_irq2_cb), attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(48.384_MHz_XTAL / 8, 384, 0, 288, 264, 0, 224); // same as namconb1.cpp?
	m_screen->set_screen_update(FUNC(namcofl_state::screen_update));
	m_screen->screen_vblank().set(m_c355spr, FUNC(namco_c355spr_device::vblank));
	m_screen->set_palette(m_c116);

	NAMCO_C169ROZ(config, m_c169roz, 0);
	m_c169roz->set_palette(m_c116);
	m_c169roz->set_is_namcofl(true);
	m_c169roz->set_ram_words(0x20000 / 2);
	m_c169roz->set_tile_callback(namco_c169roz_device::c169_tilemap_delegate(&namcofl_state::roz_cb, this));
	m_c169roz->set_color_base(0x1800);

	NAMCO_C355SPR(config, m_c355spr, 0);
	m_c355spr->set_screen(m_screen);
	m_c355spr->set_palette(m_c116);
	m_c355spr->set_scroll_offsets(0, 0);
	m_c355spr->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate(&namcofl_state::objcode2tile, this));
	m_c355spr->set_palxor(0x0);
	m_c355spr->set_color_base(0);
	m_c355spr->set_buffer(1);

	NAMCO_C123TMAP(config, m_c123tmap, 0);
	m_c123tmap->set_palette(m_c116);
	m_c123tmap->set_tile_callback(namco_c123tmap_device::c123_tilemap_delegate(&namcofl_state::tilemap_cb, this));
	m_c123tmap->set_color_base(0x1000);

	NAMCO_C116(config, m_c116, 0);
	m_c116->enable_shadows();

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	c352_device &c352(C352(config, "c352", 48.384_MHz_XTAL / 2, 288));
	c352.add_route(0, "lspeaker", 1.00);
	c352.add_route(1, "rspeaker", 1.00);
	//c352.add_route(2, "lspeaker", 1.00); // Second DAC not present.
	//c352.add_route(3, "rspeaker", 1.00);
}

ROM_START( speedrcr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("se2_mp_ea4.19a",   0x000000, 0x080000, CRC(95ab3fd7) SHA1(273a536f8512f3c55260ac1b78533bc35b8390ed) )
	ROM_LOAD32_WORD("se2_mp_oa4.18a",   0x000002, 0x080000, CRC(5b5ef1eb) SHA1(3e9e4abb1a32269baef772079de825dfe1ea230c) )

	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD32_BYTE("se1_dat0.13a",   0x000000, 0x080000, CRC(cc5d6ff5) SHA1(6fad40a1fac75bc64d3b7a7562cf7ce2a3abd36a) )
	ROM_LOAD32_BYTE("se1_dat1.14a",   0x000001, 0x080000, CRC(ddc8b306) SHA1(f169d521b800c108deffdef9fc6b0058621ee909) )
	ROM_LOAD32_BYTE("se1_dat2.15a",   0x000002, 0x080000, CRC(2a29abbb) SHA1(945419ed61e9a656a340214a63a01818396fbe98) )
	ROM_LOAD32_BYTE("se1_dat3.16a",   0x000003, 0x080000, CRC(49849aff) SHA1(b7c7eea1d56304e40e996ee998c971313ff03614) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 )
	ROM_LOAD("se1_spr.21l",   0x000000,  0x80000, CRC(850a27ac) SHA1(7d5db840ec67659a1f2e69a62cdb03ce6ee0b47b) )

	ROM_REGION( 0x200000, "c169roz", 0 ) // "RCHAR" (roz characters)
	ROM_LOAD("se1_rch0.19j",   0x000000, 0x100000, CRC(a0827288) SHA1(13691ef4d402a6dc91851de4f82cfbdf96d417cb) )
	ROM_LOAD("se1_rch1.18j",   0x100000, 0x100000, CRC(af7609ad) SHA1(b16041f0eb47d7566011d9d762a3083411dc422e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("se1_sch0.21p",   0x000000, 0x100000, CRC(7b5cfad0) SHA1(5a0355e37eb191bc0cf8b6b7c3d0274560b9bbd5) )
	ROM_LOAD("se1_sch1.20p",   0x100000, 0x100000, CRC(5086e0d3) SHA1(0aa7d11f4f9a75117e69cc77f1b73a68d9007aef) )
	ROM_LOAD("se1_sch2.19p",   0x200000, 0x100000, CRC(e59a731e) SHA1(3fed72e9bb485d4d689ab51490360c4c6f1dc5cb) )
	ROM_LOAD("se1_sch3.18p",   0x300000, 0x100000, CRC(f817027a) SHA1(71745476f496c60d89c8563b3e46bc85eebc79ce) )

	ROM_REGION( 0x800000, "c355spr", 0 )  // OBJ
	ROM_LOAD32_WORD("se1obj0l.ic1", 0x000000, 0x200000, CRC(17585218) SHA1(3332afa9bd194ac37b8d6f352507c523a0f2e2b3) )
	ROM_LOAD32_WORD("se1obj0u.ic2", 0x000002, 0x200000, CRC(d14b1236) SHA1(e5447732ef3acec88fb7a00e0deca3e71a40ae65) )
	ROM_LOAD32_WORD("se1obj1l.ic3", 0x400000, 0x200000, CRC(c4809fd5) SHA1(e0b80fccc17c83fb9d08f7f1cf2cd2f0f3a510b4) )
	ROM_LOAD32_WORD("se1obj1u.ic4", 0x400002, 0x200000, CRC(0beefa56) SHA1(012fb7b330dbf851ab2217da0a0e7136ddc3d23f) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("se1_rsh.14k",    0x000000, 0x080000, CRC(f6408a1f) SHA1(3a299719090de3915331fc1ddbe0f41834da063a) )

	ROM_REGION( 0x80000, "c123tmap:mask", 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("se1_ssh.18u",    0x000000, 0x080000, CRC(cb534142) SHA1(935a377c72b3a815ed46dfdb6ea6734d312da373) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD("se1_voi.23s",   0x000000, 0x400000, CRC(0cfa2d8a) SHA1(e029b39432cf35071eec8da27df8beeccd458eba) )

	ROM_REGION( 0x000005, "pals", 0)
	ROM_LOAD( "sysfl-1.bin",  0x000000, 0x000001, NO_DUMP ) // PAL16L8BCN at 2S
	ROM_LOAD( "sysfl-2.bin",  0x000000, 0x000001, NO_DUMP ) // PAL16L8BCN at 3L
	ROM_LOAD( "sysfl-3.bin",  0x000000, 0x000001, NO_DUMP ) // PALCE16V8H-15PC/4 at 12S
	ROM_LOAD( "sysfl-4.bin",  0x000000, 0x000001, NO_DUMP ) // PAL20L8BCNS at 20N
	ROM_LOAD( "sysfl-5.bin",  0x000000, 0x000001, NO_DUMP ) // PALCE16V8H-15PC/4 at 19D

	ROM_REGION( 0x2000, "nvram", 0 ) // default settings, including calibration
	ROM_LOAD("speedrcr.nv",   0x000000, 0x2000, CRC(58b41c70) SHA1(c30ea7f4951ce208781deafef8d99bdb4902e5b8) )
ROM_END


ROM_START( finalapr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr2_mp_eb.19a",   0x000000, 0x080000, CRC(8bfe615f) SHA1(7b867eb261268a83177f1f873689f77d1b6c47ca) )
	ROM_LOAD32_WORD("flr2_mp_ob.18a",   0x000002, 0x080000, CRC(91c14e4f) SHA1(934a86daaef0e3e2c2b3066f4677ccb3aaab6eaf) )

	ROM_REGION32_LE( 0x200000, "data", ROMREGION_ERASEFF )

	ROM_REGION16_LE( 0x80000, "c75data", 0 )
	ROM_LOAD("flr1_spr.21l", 0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, "c169roz", 0 ) // "RCHAR" (roz characters)
	ROM_LOAD("flr1_rch0.19j", 0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1_rch1.18j", 0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1_sch0.21p", 0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1_sch1.20p", 0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1_sch2.19p", 0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1_sch3.18p", 0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, "c355spr", 0 )  // OBJ
	ROM_LOAD32_WORD("flr1_obj0l.ic1", 0x000000, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD32_WORD("flr1_obj0u.ic2", 0x000002, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD32_WORD("flr1_obj1l.ic3", 0x400000, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD32_WORD("flr1_obj1u.ic4", 0x400002, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1_rsh.14k", 0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, "c123tmap:mask", 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1_ssh.18u", 0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD("flr1_voi.23s", 0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )

	ROM_REGION( 0x2000, "nvram", 0 ) // default settings, including calibration
	ROM_LOAD("finalapr.nv",   0x000000, 0x2000, CRC(d51d65fe) SHA1(8a0a523cb6ba2880951e41ca04db23584f0a108c) )
ROM_END

ROM_START( finalapr1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr2_mp_e.19a",   0x000000, 0x080000, CRC(cc8961ae) SHA1(08ce4d27a723101370d1c536b26256ce0d8a1b6c) )
	ROM_LOAD32_WORD("flr2_mp_o.18a",   0x000002, 0x080000, CRC(8118f465) SHA1(c4b79878a82fd36b5707e92aa893f69c2b942d57) )

	ROM_REGION32_LE( 0x200000, "data", ROMREGION_ERASEFF )

	ROM_REGION16_LE( 0x80000, "c75data", 0 )
	ROM_LOAD("flr1_spr.21l", 0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, "c169roz", 0 ) // "RCHAR" (roz characters)
	ROM_LOAD("flr1_rch0.19j", 0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1_rch1.18j", 0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1_sch0.21p", 0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1_sch1.20p", 0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1_sch2.19p", 0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1_sch3.18p", 0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, "c355spr", 0 )  // OBJ
	ROM_LOAD32_WORD("flr1_obj0l.ic1", 0x000000, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD32_WORD("flr1_obj0u.ic2", 0x000002, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD32_WORD("flr1_obj1l.ic3", 0x400000, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD32_WORD("flr1_obj1u.ic4", 0x400002, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1_rsh.14k", 0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, "c123tmap:mask", 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1_ssh.18u", 0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD("flr1_voi.23s", 0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )

	ROM_REGION( 0x2000, "nvram", 0 ) // default settings, including calibration
	ROM_LOAD("finalapr.nv",   0x000000, 0x2000, CRC(d51d65fe) SHA1(8a0a523cb6ba2880951e41ca04db23584f0a108c) )
ROM_END

ROM_START( finalaprj )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr1_mp_ec.19a", 0x000000, 0x080000, CRC(52735494) SHA1(db9873cb39bcfdd3dbe2e5079249fecac2c46df9) )
	ROM_LOAD32_WORD("flr1_mp_oc.18a", 0x000002, 0x080000, CRC(b11fe577) SHA1(70b51a1e66a3bb92f027aad7ba0f358c0e139b3c) )

	ROM_REGION32_LE( 0x200000, "data", ROMREGION_ERASEFF )

	ROM_REGION16_LE( 0x80000, "c75data", 0 )
	ROM_LOAD("flr1_spr.21l", 0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, "c169roz", 0 ) // "RCHAR" (roz characters)
	ROM_LOAD("flr1_rch0.19j", 0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1_rch1.18j", 0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1_sch0.21p", 0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1_sch1.20p", 0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1_sch2.19p", 0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1_sch3.18p", 0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, "c355spr", 0 )  // OBJ
	ROM_LOAD32_WORD("flr1_obj0l.ic1", 0x000000, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD32_WORD("flr1_obj0u.ic2", 0x000002, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD32_WORD("flr1_obj1l.ic3", 0x400000, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD32_WORD("flr1_obj1u.ic4", 0x400002, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1_rsh.14k", 0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, "c123tmap:mask", 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1_ssh.18u", 0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD("flr1_voi.23s", 0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )

	ROM_REGION( 0x2000, "nvram", 0 ) // default settings, including calibration
	ROM_LOAD("finalapr.nv",   0x000000, 0x2000, CRC(d51d65fe) SHA1(8a0a523cb6ba2880951e41ca04db23584f0a108c) )
ROM_END

ROM_START( finalaprj1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr1_mp_eb.19a", 0x000000, 0x080000, CRC(1a77bcc0) SHA1(4090917afcd0346ea78e6e307879a980cf196204) )
	ROM_LOAD32_WORD("flr1_mp_ob.18a", 0x000002, 0x080000, CRC(5f64eb2b) SHA1(0011ceeedefcf16c333c7ab28f334dd228eac4cf) )

	ROM_REGION32_LE( 0x200000, "data", ROMREGION_ERASEFF )

	ROM_REGION16_LE( 0x80000, "c75data", 0 )
	ROM_LOAD("flr1_spr.21l", 0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, "c169roz", 0 ) // "RCHAR" (roz characters)
	ROM_LOAD("flr1_rch0.19j", 0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1_rch1.18j", 0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1_sch0.21p", 0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1_sch1.20p", 0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1_sch2.19p", 0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1_sch3.18p", 0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, "c355spr", 0 )  // OBJ
	ROM_LOAD32_WORD("flr1_obj0l.ic1", 0x000000, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD32_WORD("flr1_obj0u.ic2", 0x000002, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD32_WORD("flr1_obj1l.ic3", 0x400000, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD32_WORD("flr1_obj1u.ic4", 0x400002, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1_rsh.14k", 0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, "c123tmap:mask", 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1_ssh.18u", 0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD("flr1_voi.23s", 0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )

	ROM_REGION( 0x2000, "nvram", 0 ) // default settings, including calibration
	ROM_LOAD("finalapr.nv",   0x000000, 0x2000, CRC(d51d65fe) SHA1(8a0a523cb6ba2880951e41ca04db23584f0a108c) )
ROM_END

} // anonymous namespace


GAME(  1995, speedrcr,   0,        namcofl, speedrcr, namcofl_state, empty_init, ROT0, "Namco", "Speed Racer",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )

// Final Lap R was released 02/94, a 1993 copyright date is displayed on the title screen
GAMEL( 1994, finalapr,   0,        namcofl, finalapr, namcofl_state, empty_init, ROT0, "Namco", "Final Lap R (Rev. B)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finalapr )
GAMEL( 1994, finalapr1,  finalapr, namcofl, finalapr, namcofl_state, empty_init, ROT0, "Namco", "Final Lap R",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finalapr )
GAMEL( 1994, finalaprj,  finalapr, namcofl, finalapr, namcofl_state, empty_init, ROT0, "Namco", "Final Lap R (Japan Rev. C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finalapr )
GAMEL( 1994, finalaprj1, finalapr, namcofl, finalapr, namcofl_state, empty_init, ROT0, "Namco", "Final Lap R (Japan Rev. B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finalapr )
