// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*

Little Robin, (c)1994 TCH

driver by:
  Pierpaolo Prazzoli
  David Haywood


Notes:

How big are the actual framebuffers?  Are both also double buffered?

Sound pitch is directly correlated with irqs, scanline timings and pixel clock,
so it's surely not 100% correct. Sound sample playbacks looks fine at current time tho.

------

LITTLE ROBIN  TCH S.A.
+---------------------------------------------+
|TDA2003                       U41_RAM  44C256|
|VR1                  U48      U40_RAM  44C256|
|  6.000MHz                    U36_RAM  44C256|
|                              U35_RAM  44C256|
|                  Bt478                      |
|J                            GAL22V10        |
|A                            GAL20V8         |
|M                                  TMS34010  |
|M            555N                   40.000MHz|
|A                    GAL22V10                |
|              84256A-70 84256A-70            |
|              TCH_1.U53 TCH_2.U29            |
|    16.000MHz                                |
|                                TCH_4.U32    |
| DSW1 DSW2      MC68000P12      TCH_3.U26    |
|---------------------------------------------+

     CPU: MC68000 series @ 16.000MHz/2 (known to use 10MHz or 12MHz parts)
     OSC: 40.000MHz, 16.000MHz & 6.000MHz
Graphics: TMS34010 (surface scratched, stamped B)
     RAM: 2 Fujitsu 84256A-70L 32K SRAM
          4 Samsung KM44C256CP-10 256K x 4bit CMOS DRAM
          U35, U36, U40 & U41 unknown ZIP style RAM
     DSW: 2 8-switch dipswitch banks
   Other: TDA2003 10Watt Amp
          ST 555N General Purpose Single Bipolar Timer
          Bt478KPJ35 Brooktree 80Mhz 265-Word Color Palette PS/2 RAMDAC
          VR1 volume resistor pot
          U48 is an unknown surface scratched socketed QFP68 part stamped A


Dip sw.1
--------
             | Coin 1 | Coin 2  |
              1  2  3   4  5  6   7  8   Coin  Play
---------------------------------------------------
 Coins        -  -  -   -  -  -           1     4
              +  -  -   +  -  -           1     3
              -  +  -   -  +  -           1     2
              +  +  -   +  +  -           1     1
              -  -  +   -  -  +           2     1
              +  -  +   +  -  +           3     1
              -  +  +   -  +  +           4     1
              +  +  +   +  +  +           5     1
 Player                           -  -    2
                                  +  -    3
                                  -  +    4
                                  +  +    5


Dip sw.2
--------          1  2  3  4  5  6  7  8
-----------------------------------------------------------
 Demo Sound       -                        Yes
                  +                        No
 Mode                -                     Test Mode
                     +                     Game Mode
 Difficulty             -  -  -            0 (Easy)
                        +  -  -            1
                        -  +  -            2 (Normal)
                        +  +  -            3
                        -  -  +            4
                        +  -  +            5
                        -  +  +            6
                        +  +  +            7 (Hardest)
 Bonus                           -  -      Every 150000
                                 +  -      Every 200000
                                 -  +      Every 300000
                                 +  +      No Bonus


*/

#include "emu.h"

#include "inder_vid.h"

#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "speaker.h"

namespace {

class littlerb_state : public driver_device
{
public:
	littlerb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_indervid(*this, "inder_vid"),
			m_sample_rom(*this, "samples"),
			m_ldac(*this, "ldac"),
			m_rdac(*this, "rdac"),
			m_soundframe(0)
	{
	}

	void littlerb(machine_config &config);

	void init_littlerb();

	DECLARE_CUSTOM_INPUT_MEMBER(frame_step_r);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<inder_vid_device> m_indervid;
	required_region_ptr<uint8_t> m_sample_rom;

	required_device<dac_byte_interface> m_ldac;
	required_device<dac_byte_interface> m_rdac;
	uint8_t m_sound_index_l = 0, m_sound_index_r = 0;
	uint16_t m_sound_pointer_l = 0, m_sound_pointer_r = 0;
	int m_soundframe;

	void l_sound_w(uint16_t data);
	void r_sound_w(uint16_t data);
	uint8_t sound_data_shift();

	TIMER_DEVICE_CALLBACK_MEMBER(sound_step_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_cb);

	void main(address_map &map);
};

void littlerb_state::machine_start()
{
	save_item(NAME(m_sound_index_l));
	save_item(NAME(m_sound_index_r));
	save_item(NAME(m_sound_pointer_l));
	save_item(NAME(m_sound_pointer_r));
	save_item(NAME(m_soundframe));
}

// could be slightly different (timing wise, directly related to the irqs), but certainly they smoked some bad pot for this messy way ...
uint8_t littlerb_state::sound_data_shift()
{
	return ((m_soundframe % 16) == 0) ? 8 : 0;
}

// l is SFX, r is BGM (they don't seem to share the same data ROM)
void littlerb_state::l_sound_w(uint16_t data)
{
	m_sound_index_l = (data >> sound_data_shift()) & 0xff;
	m_sound_pointer_l = 0;
	//popmessage("%04x %04x",m_sound_index_l,m_sound_index_r);
}

void littlerb_state::r_sound_w(uint16_t data)
{
	m_sound_index_r = (data >> sound_data_shift()) & 0xff;
	m_sound_pointer_r = 0;
	//popmessage("%04x %04x",m_sound_index_l,m_sound_index_r);
}

void littlerb_state::main(address_map &map)
{
	map(0x000008, 0x000017).nopw();
	map(0x000020, 0x00002f).nopw();
	map(0x000070, 0x000073).nopw();
	map(0x060004, 0x060007).nopw();
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x203fff).ram(); // main ram?

	map(0x700000, 0x700007).rw("inder_vid:tms", FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));

	map(0x740000, 0x740001).w(FUNC(littlerb_state::l_sound_w));
	map(0x760000, 0x760001).w(FUNC(littlerb_state::r_sound_w));
	map(0x780000, 0x780001).nopw(); // generic outputs
	map(0x7c0000, 0x7c0001).portr("DSW");
	map(0x7e0000, 0x7e0001).portr("P1");
	map(0x7e0002, 0x7e0003).portr("P2");
}

// guess according to DASM code and checking the gameplay speed, could be different
CUSTOM_INPUT_MEMBER(littlerb_state::frame_step_r)
{
	uint32_t ret = m_soundframe;

	return (ret) & 7;
}

static INPUT_PORTS_START( littlerb )
	PORT_START("DSW")   // 16bit
	PORT_DIPNAME( 0x0003, 0x0001, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x001c, 0x0004, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00e0, 0x0020, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(      0x0600, "Every 150,000" )
	PORT_DIPSETTING(      0x0200, "Every 200,000" )
	PORT_DIPSETTING(      0x0400, "Every 300,000" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x3800, 0x2800, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x3800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Medium_Easy ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x4000, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("P1")    // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x1000, 0x1000, "???"  )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xe000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(littlerb_state, frame_step_r)

	PORT_START("P2")    // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(littlerb_state::sound_cb)
{
	m_ldac->write(m_sample_rom[m_sound_pointer_l | (m_sound_index_l << 10) | 0x40000]);
	m_rdac->write(m_sample_rom[m_sound_pointer_r | (m_sound_index_r << 10) | 0x00000]);
	m_sound_pointer_l++;
	m_sound_pointer_l&=0x3ff;
	m_sound_pointer_r++;
	m_sound_pointer_r&=0x3ff;
}

TIMER_DEVICE_CALLBACK_MEMBER(littlerb_state::sound_step_cb)
{
	m_soundframe++;
}

void littlerb_state::littlerb(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(16'000'000)/2); // 10MHz rated part, near 16Mhz XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &littlerb_state::main);

	INDER_VIDEO(config, m_indervid, 0); // XTAL(40'000'000)

	// TODO: not accurate - driven by XTAL(6'000'000)?
	TIMER(config, "step_timer").configure_periodic(FUNC(littlerb_state::sound_step_cb), attotime::from_hz(7500/150));
	TIMER(config, "sound_timer").configure_periodic(FUNC(littlerb_state::sound_cb), attotime::from_hz(7500));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_8BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5); // unknown DAC
	DAC_8BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5); // unknown DAC
}

ROM_START( littlerb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tch_1.u53", 0x00001, 0x80000, CRC(172fbc13) SHA1(cd165ca0d0546e2634cf182dc98004cbfb02cf9f) )
	ROM_LOAD16_BYTE( "tch_2.u29", 0x00000, 0x80000, CRC(b2fb1d61) SHA1(9a9d7176c241928d07af651e5f7f21d4f019701d) )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "tch_3.u26", 0x40000, 0x40000, CRC(f193c5b6) SHA1(95548a40e2b5064c558b36cabbf507d23678b1b2) )
	ROM_LOAD( "tch_4.u32", 0x00000, 0x40000, CRC(d6b81583) SHA1(b7a63d18a41ccac4d3db9211de0b0cdbc914317a) )
ROM_END

void littlerb_state::init_littlerb()
{
	/* various scenes flicker to the point of graphics being invisible (eg. the map screen at the very start of a game)
	   unless you overclock the TMS34010 to 120%, possible timing bug in the core? this is a hack */
	m_indervid->subdevice<cpu_device>("tms")->set_clock_scale(1.2f);
}

} // anonymous namespace

GAME( 1994, littlerb, 0, littlerb, littlerb, littlerb_state, init_littlerb, ROT0, "TCH", "Little Robin", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
