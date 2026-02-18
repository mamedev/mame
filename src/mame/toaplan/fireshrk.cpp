// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench, Stephane Humbert
/***************************************************************************

        Toaplan Fire Shark hardware
        ------------------------------------
        MAME Driver by: Darren Olafson
        Technical info: Carl-Henrik Skarstedt  &  Magnus Danielsson
        Driver updates: Quench
        Video updates : SUZ
        Driven from toaplan/toaplan1.cpp


Hardware info by Guru

PCB Layout
----------
TOAPLAN Co., Ltd.
TP-O17
|---------------------------------------------------------|
|MB3730 YM3812 10MHz                     O17_01           |
| YM3014                    MN53007      O17_02      6264 |
|VOL  LM358 |---------------|            O17_03           |
|           |    68000      |            O17_04      6264 |
|  647180   |---------------|                             |
|            O17_12   O17_11                         6264 |
|            O17_10   O17_09    2018  2018                |
|J           6264       6264    2018  2018           6264 |
|A           O17_07   O17_05                              |
|M           O17_08   O17_06                         6264 |
|M                                              |------|  |
|A           DSW1    |------|                   |FCU-2 |  |
|                    |BCU-2 |   8464            |      |  |
|            DSW2    |      |   8464            |------|  |
|                    |------|   8464        PROM15        |
|            D65024             8464               6116   |
|                               8464               6116   |
|                     BCU       8464               6116   |
|            6116               8464              PROM14  |
|            6116               8464         28MHz        |
|---------------------------------------------------------|
Notes:
        68000 - Motorola MC68000P10 CPU. Clock input 10.000MHz
       647180 - Hitachi HD647180XOFS6 micro-controller with 16k internal OTP EPROM and 512 bytes internal RAM. Clock input 10.000MHz on pin 75
       YM3812 - Yamaha YM3812 FM operator type-L II (OPL II) LSI (DIP24). Clock input 3.500MHz [28/8]
       YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter (DIP8)
         2018 - Motorola MCM2018AN45 2kx8 SRAM (NDIP24)
         8464 - Fujitsu MB8464A-10L 8kx8 SRAM (NDIP28)
         6464 - Hyundai HY6264LP-10 8kx8 SRAM (DIP28)
         6116 - Hyundai HY6116AP-15 2kx8 SRAM (DIP24)
        BCU-2 - Custom graphics IC (QFP160)
        FCU-2 - Custom graphics IC (QFP136)
        LM358 - National Semiconductor LM358 Dual Operational Amplifier (DIP8)
       D65024 - NEC D65024GF035 uPD65000-series CMOS Gate Array (QFP100)
      MN53007 - Panasonic MN53007 CMOS Gate Array {732 gates} (DIP42)
    PROM14/15 - Philips/Signetics N82S123 Bipolar PROM (DIP16)
       MB3730 - Fujitsu MB3730 14W BTL Audio Power Amplifier
       DSW1/2 - 8-position DIP switch
          BCU - Unpopulated position for PGA177 IC
        HSYNC - 14.86496kHz
        VSYNC - 57.61308Hz


Supported games:

    ROM set     Toaplan
    name        board No        Game name
    --------------------------------------------------
    fireshrk    TP-O17      Fire Shark (World)           [1990]
    fireshrka   TP-O17      Fire Shark (World)           [1989]
    fireshrkd   TP-O17      Fire Shark (Korea)           [1990] (Easier set)
    fireshrkdh  TP-O17      Fire Shark (Korea)           [1990] (Harder set)
    samesame    TP-O17      Same! Same! Same! (Japan)    [1989] (1 Player version)
    samesame2   TP-O17      Same! Same! Same! (Japan)    [1989] (2 Player version)
    samesamecn  TP-O17      Jiao! Jiao! Jiao! (China)    [1990]


Notes:
    Fire Shark and clones have a hidden function for the
    service input. When invulnerability is enabled, pressing the
    service input makes the screen scroll faster.

***************************************************************************/

#include "emu.h"

#include "toaplan_bcu.h"
#include "toaplan_fcu.h"
#include "toaplan_video_controller.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z180/hd647180x.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class fireshrk_state : public driver_device
{
public:
	fireshrk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_fcu(*this, "fcu"),
		m_bcu(*this, "bcu"),
		m_vctrl(*this, "vctrl"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette_%u", 0U),
		m_paletteram(*this, "paletteram_%u", 0U, 0x800U, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram", 0x800, ENDIANNESS_BIG),
		m_spritesizeram(*this, "spritesizeram", 0x80, ENDIANNESS_BIG),
		m_tjump_io(*this, "TJUMP")
	{ }

	void fireshrk(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym3812_device> m_ymsnd;
	required_device<toaplan_fcu_device> m_fcu;
	required_device<toaplan_bcu_device> m_bcu;
	required_device<toaplan_video_controller_device> m_vctrl;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;
	required_device_array<palette_device, 2> m_palette;

	memory_share_array_creator<u16, 2> m_paletteram;

	memory_share_creator<u16> m_spriteram;
	memory_share_creator<u16> m_spritesizeram;

	required_ioport m_tjump_io;

	void coin_w(u8 data);

	u8 cmdavailable_r();
	u8 port_6_word_r();

	void pri_cb(u8 priority, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void reset_callback(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
    Sprite Handlers
***************************************************************************/

void fireshrk_state::pri_cb(u8 priority, u32 &pri_mask)
{
	// ~0 for behind tilemap in same priority
	pri_mask = (~u32(0)) << priority;
}


/***************************************************************************
    Draw the game screen in the given bitmap.
***************************************************************************/

u32 fireshrk_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// first draw everything, including "disabled" tiles and priority 0
	m_bcu->draw_background(screen, bitmap, cliprect, 0, 0);

	// then draw the higher priority layers in order
	for (int priority = 1; priority < 16; priority++)
	{
		m_bcu->draw_tilemap(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), priority, 0);
	}

	m_fcu->draw_sprites(screen, bitmap, cliprect);
	return 0;
}

/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

void fireshrk_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (m_vctrl->intenable())
			m_maincpu->set_input_line(4, HOLD_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE); // Frame done
	}
}

u8 fireshrk_state::port_6_word_r()
{
	/* Bit 0x80 is secondary CPU (HD647180) ready signal */
	return (m_soundlatch->pending_r() ? 0 : 0x80) | m_tjump_io->read();
}

void fireshrk_state::coin_w(u8 data)
{
	// Upper 4 bits are junk (normally 1110 or 0000, which are artifacts of sound command processing)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}

void fireshrk_state::reset_callback(int state)
{
	m_ymsnd->reset();
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_soundlatch->acknowledge_w();
}

/***************************** 68000 Memory Map *****************************/

void fireshrk_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x07ffff).rom();
	map(0x080000, 0x080003).w(m_bcu, FUNC(toaplan_bcu_device::tile_offsets_w));
	map(0x080006, 0x080006).w(m_fcu, FUNC(toaplan_fcu_device::flipscreen_w));
	map(0x0c0000, 0x0c3fff).ram();         /* Frame done at $c1ada */
	map(0x100000, 0x107fff).m(m_vctrl, FUNC(toaplan_video_controller_device::host_map));
	map(0x140000, 0x140001).portr("P1");
	map(0x140002, 0x140003).portr("P2");
	map(0x140004, 0x140005).portr("DSWA");
	map(0x140006, 0x140007).portr("DSWB");
	map(0x140008, 0x140009).portr("SYSTEM");
	map(0x14000b, 0x14000b).r(FUNC(fireshrk_state::port_6_word_r));    /* Territory, and MCU ready */
	map(0x14000d, 0x14000d).w(FUNC(fireshrk_state::coin_w));  /* Coin counter/lockout */
//  map(0x14000e, 0x14000f).nopr(); // irq ack?
	map(0x14000f, 0x14000f).w(m_soundlatch, FUNC(generic_latch_8_device::write));   /* Commands sent to HD647180 */
	map(0x180000, 0x18001f).m(m_bcu, FUNC(toaplan_bcu_device::host_map));
	map(0x1c0000, 0x1c0007).m(m_fcu, FUNC(toaplan_fcu_device::host_map));
}

u8 fireshrk_state::cmdavailable_r()
{
	return m_soundlatch->pending_r() ? 1 : 0;
}

void fireshrk_state::sound_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x63, 0x63).nopr(); // read port D
	map(0x80, 0x81).rw(m_ymsnd, FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xa0, 0xa0).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xb0, 0xb0).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

/*****************************************************************************
    Input Port definitions
*****************************************************************************/

static INPUT_PORTS_START( fireshrk )
	PORT_START("VBLANK")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")      /* DSW A */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:!1")    // No upright/cocktail DIPSW in fireshrk
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )                    PORT_DIPLOCATION("SW1:!3")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")      /* DSW B */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x04, "50K, every 150K" )
	PORT_DIPSETTING(    0x00, "70K, every 200K" )
	PORT_DIPSETTING(    0x08, "100K" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )           PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("TJUMP")     /* Territory Jumper Block */
	PORT_DIPNAME( 0x01, 0x00, "Show Territory Notice" )     PORT_DIPLOCATION("JMPR:!1")   // When NO is selected, the region reverts to Europe
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )                                             // regardless of which region is selected
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Region ) )           PORT_DIPLOCATION("JMPR:!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x04, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x00, "USA (Romstar)" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( fireshrka ) /* No "Romstar" license */
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Region ) )           PORT_DIPLOCATION("JMPR:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( samesame )
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )                                                     \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DSWA")     /* DSW A */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
/* settings listed in service mode, but not actually used ???
    PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
    PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
*/

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("JMPR:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( samesame2 )
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
/* settings listed in service mode, but not actually used
        PORT_DIPNAME( 0x03, 0x00, DEF_STR( Region ) )
        PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )
        PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
*/
	PORT_DIPNAME( 0x03, 0x00, "Show Territory Notice" )  PORT_DIPLOCATION("JMPR:!1,!2")
	PORT_DIPSETTING(    0x03, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("JMPR:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("JMPR:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( jiaojiao )
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("DSWA")     /* DSW A */
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) ) PORT_CONDITION("TJUMP", 0x01, EQUALS, 0x00)

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x01, 0x00, "Coinage Style" )       PORT_DIPLOCATION("JMPR:!1")
	PORT_DIPSETTING(    0x01, "Fire Shark" )
	PORT_DIPSETTING(    0x00, "Same! Same! Same!" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )    PORT_DIPLOCATION("JMPR:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )    PORT_DIPLOCATION("JMPR:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )    PORT_DIPLOCATION("JMPR:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,            /* 8x8 */
	RGN_FRAC(1,2),  /* 16384/32768 tiles */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8            /* every tile takes 16 consecutive bytes */
};


static GFXDECODE_START( gfx_bcu )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 0, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_fcu )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0, 64 )
GFXDECODE_END


void fireshrk_state::machine_reset()
{
	machine().bookkeeping().coin_lockout_global_w(0);
}


void fireshrk_state::fireshrk(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &fireshrk_state::main_map);
	m_maincpu->reset_cb().set(FUNC(fireshrk_state::reset_callback));

	hd647180x_device &audiocpu(HD647180X(config, m_audiocpu, XTAL(10'000'000))); // HD647180XOFS6 CPU
	// 16k byte ROM and 512 byte RAM are internal
	audiocpu.set_addrmap(AS_IO, &fireshrk_state::sound_io_map);
	audiocpu.in_pd_callback().set(FUNC(fireshrk_state::cmdavailable_r));

	GENERIC_LATCH_8(config, m_soundlatch).set_separate_acknowledge(true);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// Parameter uploaded to CRTC
	m_screen->set_raw(XTAL(28'000'000) / 4, (224+1)*2, 0, 320, (134+1)*2, 0, 240);
	m_screen->set_screen_update(FUNC(fireshrk_state::screen_update));
	m_screen->screen_vblank().set(m_fcu, FUNC(toaplan_fcu_device::vblank));
	m_screen->screen_vblank().append(FUNC(fireshrk_state::screen_vblank));

	PALETTE(config, m_palette[0]).set_format(palette_device::xBGR_555, 0x400);
	PALETTE(config, m_palette[1]).set_format(palette_device::xBGR_555, 0x400);

	TOAPLAN_BCU(config, m_bcu, XTAL(28'000'000), m_palette[0], gfx_bcu);
	m_bcu->set_offset(-0x1ef, -0x101, -0x11, -0xff);

	TOAPLAN_FCU(config, m_fcu, XTAL(28'000'000), m_palette[1], gfx_fcu);
	m_fcu->set_spriteram_tag(m_spriteram);
	m_fcu->set_spritesizeram_tag(m_spritesizeram);
	m_fcu->set_pri_callback(FUNC(fireshrk_state::pri_cb));
	m_fcu->frame_done_cb().set(m_screen, FUNC(screen_device::vblank));

	TOAPLAN_VIDEO_CONTROLLER(config, m_vctrl, XTAL(28'000'000));
	m_vctrl->set_screen(m_screen);
	m_vctrl->set_palette_tag(0, m_palette[0]);
	m_vctrl->set_palette_tag(1, m_palette[1]);
	m_vctrl->set_paletteram_tag(0, m_paletteram[0]);
	m_vctrl->set_paletteram_tag(1, m_paletteram[1]);
	m_vctrl->set_byte_per_color(0, 2);
	m_vctrl->set_byte_per_color(1, 2);
	m_vctrl->vblank_callback().set_ioport("VBLANK");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM3812(config, m_ymsnd, XTAL(28'000'000) / 8);
	m_ymsnd->irq_handler().set_inputline(m_audiocpu, 0);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( fireshrk )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "09.8j",       0x000000, 0x08000, CRC(f0c70e6f) SHA1(037690448786d61aa116b24b638430c577ea78e2) )
	ROM_LOAD16_BYTE( "10.8l",       0x000001, 0x08000, CRC(9d253d77) SHA1(0414d1f475abb9ccfd7daa11c2f400a14f25db09) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( fireshrka )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09ii.8j", 0x000000, 0x08000, CRC(b60541ee) SHA1(e4fb752073c99a83939ebc45307777b94519f01c) )
	ROM_LOAD16_BYTE( "o17_10ii.8l", 0x000001, 0x08000, CRC(96f5045e) SHA1(16cf2f4d55570cf0489a426d6e841d2968f9423a) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( fireshrkd )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09dyn.8j",0x000000, 0x10000, CRC(e25eee27) SHA1(1ff3f838123180a0b6672c9beee6c0f0092a0f94) ) // Identical halves
	ROM_LOAD16_BYTE( "o17_10dyn.8l",0x000001, 0x10000, CRC(c4c58cf6) SHA1(5867ecf66cd6c16cfcc54a581d3f4a8b666fd839) ) // Identical halves
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( fireshrkdh )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09dyh.8j",0x000000, 0x10000, CRC(7b4c14dd) SHA1(d40dcf223f16c0f507aeb282d1524dbf1349c536) ) /* Identical halves */
	ROM_LOAD16_BYTE( "o17_10dyh.8l",0x000001, 0x10000, CRC(a3f159f9) SHA1(afc9630ca38da730f7cf4954d1333954e8d75787) ) /* Identical halves */
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( samesame )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09.8j",  0x000000, 0x08000, CRC(3f69e437) SHA1(f2a40fd42cb5ecb2e514b72e7550aa479a9f9ad6) )
	ROM_LOAD16_BYTE( "o17_10.8l",  0x000001, 0x08000, CRC(4e723e0a) SHA1(e06394d50addeda1045c02c646964afbc6005a82) )
	ROM_LOAD16_BYTE( "o17_11.7j",  0x040000, 0x20000, CRC(be07d101) SHA1(1eda14ba24532b565d6ad57490b73ff312f98b53) )
	ROM_LOAD16_BYTE( "o17_12.7l",  0x040001, 0x20000, CRC(ef698811) SHA1(4c729704eba0bf469599c79009327e4fa5dc540b) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( samesame2 )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09x.8j",  0x000000, 0x08000, CRC(3472e03e) SHA1(a0f12622a1963bfac2d5f357afbfb5d7db2cd8df) )
	ROM_LOAD16_BYTE( "o17_10x.8l",  0x000001, 0x08000, CRC(a3ac49b5) SHA1(c5adf026b9129b64acee5a079e102377a8488220) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( samesamecn )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17-09-h2.8j",0x000000, 0x08000, CRC(fc8c2420) SHA1(cf8333d3749213f2007467d3a80bd36ff7b4ce92) ) // The actual label is stamped with the letter "H" and separate "2"
	ROM_LOAD16_BYTE( "o17-10-h2.8l",0x000001, 0x08000, CRC(cc0ffbeb) SHA1(1cf85f68b4e368294069053ba8f5710d6c557ede) ) // The actual label is stamped with the letter "H" and separate "2"
	ROM_LOAD16_BYTE( "o17-11-2.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) ) // The actual label is stamped with the number "2"
	ROM_LOAD16_BYTE( "o17-12-2.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) ) // The actual label is stamped with the number "2"

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

ROM_START( samesamenh ) /* this hack has been used on various PCBs */
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "o17_09_nv.8j", 0x000000, 0x08000, CRC(f60af2f9) SHA1(ce41efd5ca4f4adc8bf1976f61a8a8d357fb234a) )
	ROM_LOAD16_BYTE( "o17_10_nv.8l", 0x000001, 0x08000, CRC(023bcb95) SHA1(69a051fb223e6cacaf1cda8bf5430933d24fb8a7) )
	ROM_LOAD16_BYTE( "o17_11.7j",    0x040000, 0x20000, CRC(be07d101) SHA1(1eda14ba24532b565d6ad57490b73ff312f98b53) )
	ROM_LOAD16_BYTE( "o17_12.7l",    0x040001, 0x20000, CRC(ef698811) SHA1(4c729704eba0bf469599c79009327e4fa5dc540b) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // HD647180 (Z180) with internal ROM
	ROM_LOAD( "hd647180_tp-017.8m",  0x00000, 0x08000, CRC(43523032) SHA1(1b94003a00e7bf6bdf1b1b946f42ff5d04629949) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) // ???
ROM_END

} // Anonymous namespace


//    YEAR  NAME        PARENT    MACHINE   INPUT      CLASS           INIT        ROT     COMPANY                                       FULLNAME                                            FLAGS
GAME( 1990, fireshrk,   0,        fireshrk, fireshrk,  fireshrk_state, empty_init, ROT270, "Toaplan",                                    "Fire Shark",                                       0 )
GAME( 1989, fireshrka,  fireshrk, fireshrk, fireshrka, fireshrk_state, empty_init, ROT270, "Toaplan",                                    "Fire Shark (earlier)",                             0 )
GAME( 1990, fireshrkd,  fireshrk, fireshrk, samesame2, fireshrk_state, empty_init, ROT270, "Toaplan (Dooyong license)",                  "Fire Shark (Korea, set 1, easier)",                0 )
GAME( 1990, fireshrkdh, fireshrk, fireshrk, samesame2, fireshrk_state, empty_init, ROT270, "Toaplan (Dooyong license)",                  "Fire Shark (Korea, set 2, harder)",                0 )
GAME( 1989, samesame,   fireshrk, fireshrk, samesame,  fireshrk_state, empty_init, ROT270, "Toaplan",                                    "Same! Same! Same! (Japan, 1P set)",                0 )
GAME( 1989, samesame2,  fireshrk, fireshrk, samesame2, fireshrk_state, empty_init, ROT270, "Toaplan",                                    "Same! Same! Same! (Japan, 2P set)",                0 )
GAME( 1990, samesamecn, fireshrk, fireshrk, jiaojiao,  fireshrk_state, empty_init, ROT270, "Toaplan (Hong Kong Honest Trading license)", "Jiao! Jiao! Jiao! (China)",                        0 )
GAME( 2015, samesamenh, fireshrk, fireshrk, samesame,  fireshrk_state, empty_init, ROT270, "hack (trap15)",                              "Same! Same! Same! (Japan, 1P set, NEW VER! hack)", 0 )
