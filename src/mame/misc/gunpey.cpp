// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, David Haywood, Luca Elia, Morten Shearman Kirkegaard
/********************************************************************************************

    Gunpey (c) 2000 Banpresto

    Video is handled by an Axell AG-1 AX51101 sprite controller, see video/axell_ag1.cpp

    TODO:
    - Verify the display window / sheet positions against hardware

=============================================================================================

Gunpey
Banpresto, 2000

The hardware looks very Raizing/8ing -ish, especially the ROM stickers and PCB silk-screening
which are identical to those on Brave Blade and Battle Bakraid ;-)

PCB Layout
----------

VG-10
|-------------------------------------|
|        M6295  ROM5                  |
|        YMZ280B-F      ROM4   ROM3   |
|  YAC516      16.93MHz               |
|                       61256  ROM1   |
|                       61256  ROM2   |
|J                                    |
|A             PAL       XILINX       |
|M                       XC95108      |
|M             57.2424MHz             |
|A                            V30     |
|                                     |
|              |-------|              |
|              |AXELL  |              |
|       DSW1   |AG-1   |              |
|       DSW2   |AX51101|  T2316162    |
|              |-------|  T2316162    |
|-------------------------------------|

Notes:
      V30 clock: 14.3106MHz (= 57.2424 / 4)
      YMZ280B clock: 16.93MHz
      OKI M6295 clock: 2.11625MHz (= 16.93 / 8), sample rate = clock / 165
      VSync: 60Hz
      HSync: 15.79kHz

      ROMs:
           GP_ROM1.021 \
           GP_ROM2.022 / 27C040, Main program

           GP_ROM3.025 \
           GP_ROM4.525 / SOP44 32M MASK, Graphics

           GP_ROM5.622   SOP44 32M MASK, OKI samples

********************************************************************************************/

#include "emu.h"

#include "cpu/nec/nec.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "video/axell_ag1.h"

#include "screen.h"
#include "speaker.h"


namespace {

class gunpey_state : public driver_device
{
public:
	gunpey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ag1(*this, "ag1")
		, m_oki(*this, "oki")
		, m_inputs(*this, { "DSW1", "DSW2", "P1", "P2", "SYSTEM" })
	{ }

	void gunpey(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ax51101_device> m_ag1;
	required_device<okim6295_device> m_oki;

	required_ioport_array<5> m_inputs;

	u8 inputs_r(offs_t offset);
	void output_w(u8 data);

	void ag1_irq_w(int state);
	void ag1_irq_regs_w(offs_t offset, u8 data);
	void vblank_w(int state);
	void ag1_irq_retrigger();
	IRQ_CALLBACK_MEMBER(vector_r);

	void ag1_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};


IRQ_CALLBACK_MEMBER(gunpey_state::vector_r)
{
	return 0x200 / 4;
}

// HACK: with the AG-1 interrupt output driven to the V30 as a level the game
// stays on a black screen.  The V30 takes one interrupt per trigger instead,
// and the interrupt is triggered again on every write to the AG-1 interrupt
// mask / acknowledge registers and at the start of every frame while an
// enabled cause is still pending.
void gunpey_state::ag1_irq_w(int state)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

void gunpey_state::ag1_irq_retrigger()
{
	if (m_ag1->read(0x08) & m_ag1->read(0x09))
		m_maincpu->set_input_line(0, HOLD_LINE);
}

void gunpey_state::ag1_irq_regs_w(offs_t offset, u8 data)
{
	m_ag1->write(0x08 + offset, data);
	ag1_irq_retrigger();
}

void gunpey_state::vblank_w(int state)
{
	if (state)
		ag1_irq_retrigger();
}

u8 gunpey_state::inputs_r(offs_t offset)
{
	if (offset < 5)
		return m_inputs[offset]->read();

	return 0xff;
}

void gunpey_state::output_w(u8 data)
{
	//bit 0 is coin counter
//  popmessage("%02x",data);

	m_oki->set_rom_bank((data & 0x70) >> 4);
}

/***************************************************************************************/

void gunpey_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("wram");
//  map(0x50000, 0x500ff).ram();
//  map(0x50100, 0x502ff).noprw();
	map(0x80000, 0xfffff).rom();
}

void gunpey_state::io_map(address_map &map)
{
	map(0x7f40, 0x7f45).r(FUNC(gunpey_state::inputs_r));
	map(0x7f48, 0x7f48).w(FUNC(gunpey_state::output_w));
	map(0x7f80, 0x7f81).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x7f88, 0x7f88).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x7fc0, 0x7fff).rw(m_ag1, FUNC(ax51101_device::read), FUNC(ax51101_device::write));
	map(0x7fc8, 0x7fc9).w(FUNC(gunpey_state::ag1_irq_regs_w)); // HACK, see ag1_irq_w()
}

// the AG-1 fetches its sheet and cell tables from the V30 work RAM
void gunpey_state::ag1_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("wram");
}


/***************************************************************************************/

static INPUT_PORTS_START( gunpey )
	PORT_START("DSW1")  // IN0 - 7f40
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty (vs. mode)" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x00, "Matches (vs. mode)?" )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  // IN1 - 7f41
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")    // IN2 - 7f42
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")    // IN3 - 7f43
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // IN4 - 7f44
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )   // TEST!!
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************************/

void gunpey_state::gunpey(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, 57.2424_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gunpey_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gunpey_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(gunpey_state::vector_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(57.2424_MHz_XTAL / 8, 442, 0, 320, 264, 0, 240); // just to get ~60 Hz
	screen.set_screen_update(m_ag1, FUNC(ax51101_device::screen_update));
	screen.screen_vblank().set(FUNC(gunpey_state::vblank_w));

	AX51101(config, m_ag1, 57.2424_MHz_XTAL);
	m_ag1->set_screen("screen");
	m_ag1->set_cgrom_tag("blit_rom");
	m_ag1->set_addrmap(0, &gunpey_state::ag1_map);
	m_ag1->irq_callback<0>().set(FUNC(gunpey_state::ag1_irq_w));

	SPEAKER(config, "speaker", 2).front();

	OKIM6295(config, m_oki, 16.9344_MHz_XTAL / 8, okim6295_device::PIN7_LOW);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.125, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.125, 1);

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.add_route(0, "speaker", 0.25, 0);
	ymz.add_route(1, "speaker", 0.25, 1);
}

/***************************************************************************************/

ROM_START( gunpey )
	ROM_REGION( 0x100000, "maincpu", 0 ) // V30 code
	ROM_LOAD16_BYTE( "gp_rom1.021",  0x00000, 0x80000, CRC(07a589a7) SHA1(06c4140ffd5f74b3d3ddfc424f43fcd08d903490) ) // Gunpey(AC) -Release-
	ROM_LOAD16_BYTE( "gp_rom2.022",  0x00001, 0x80000, CRC(f66bc4cf) SHA1(54931d878d228c535b9e2bf22a0a3e41756f0fe5) ) // Jan 17 2000 19:10:22 (J)

	ROM_REGION( 0x400000, "blit_rom", 0 )
	ROM_LOAD( "gp_rom3.025",  0x00000, 0x400000,  CRC(f2d1f9f0) SHA1(0d20301fd33892074508b9d127456eae80cc3a1c) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "gp_rom4.525",  0x000000, 0x400000, CRC(78dd1521) SHA1(91d2046c60e3db348f29f776def02e3ef889f2c1) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "gp_rom5.622",  0x000000, 0x400000,  CRC(f79903e0) SHA1(4fd50b4138e64a48ec1504eb8cd172a229e0e965)) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 2000, gunpey, 0, gunpey, gunpey, gunpey_state, empty_init, ROT0, "Bandai / Banpresto", "Gunpey (Japan)", MACHINE_SUPPORTS_SAVE )
