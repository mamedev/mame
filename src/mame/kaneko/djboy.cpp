// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino

/*
DJ Boy (c)1989 Kaneko

Hardware has many similarities to Air Buster.

Self Test has two parts:
1) color test : press button#3 to advance past color pattern
2) i/o and sound test: use buttons 1,2,3 to select and play sound/music

- CPU0 manages sprites, which are also used to display text
        irq (0x10) - timing/watchdog
        irq (0x30) - processes sprites
        nmi: wakes up this CPU

- CPU1 manages the protection device, palette, and tilemap(s)
        nmi: resets this CPU
        irq: game update

- CPU2 manages sound chips
        irq: update music
        nmi: handle sound command

- The "BEAST" protection device has access to DIP switches and player inputs.


PCB Layout
----------

BS
|----------------------------------------------|
| 6264               BS15   6116               |
|                    BS-101 6116               |
| BS-005             780C-2                    |
| BS-004                           DSW1 DSW2   |
|                    6264              IO-JAMMA|
| 16MHz                     PAL1|----------|   |
| 12MHz                         |  BEAST   |  J|
|                               |----------|  A|
| BS-003              *                       M|
|            6116     BS-203  6295     YM2203 M|
| BS-000                           YM3014     A|
|          |-------|          6295  324  4558  |
| BS-001   |KANEKO |  780C-2        324        |
|          |PANDORA|                    VOL  JP|
| BS-002   |       |  BS-100  PAL2   324       |
|          |-------|           6264     VOL CN1|
| BS07     4464 4464  BS19     BS-200   LA4460 |
|          4464 4464  PAL3     D780C-2  LA4460 |
|----------------------------------------------|

Notes:
      D780C-2 - Z80 CPU. Clock 6.000MHz [12/2] (for all 3 Z80 CPUs)
      BEAST   - OKI MSM80C51F microcontroller with internal ROM. Clock 6.000MHz on pins 18 & 19
                chip is stamped 'KANEKO Beast (C)Intel '80 (C)KANEKO 1988'
      YM2203  - Yamaha YM2203, clock 3.000MHz [12/4]
      6295    - OKI M6295, clock 1.500MHz [12/8]. Sample rate (Hz) = 12000000 / 8 / 165
      PANDORA - Custom Kaneko graphics generator chip stamped 'PX79C480FP-3 PANDORA-CHIP' (QFP160)
      4464    - 64k x4 DRAM (DIP18)
      6116    - 2k x8 SRAM (DIP24)
      6264    - 8k x8 SRAM (DIP28)
      VSync   - 57.5Hz
      HSync   - 15.68kHz
      JP      - 3 pin jumper to set mono/stereo sound output
      CN1     - 4 pin connector for speakers when jumper is set for stereo sound output
      PAL1    - PAL16L8 stamped 'BS-501'
      PAL2    - PAL16L8 stamped 'BS-502'
      PAL3    - PAL16L8 stamped 'BS-500'
      IO-JAMMA- Custom Kaneko ceramic I/O input resistor pack stamped 'I/O JAMMA MC-8282837'
      LA4460  - Sanyo 12W Power Amplifier (SIL10)
      *       - Unpopulated DIP32 position
      ROMs    -
                BS15.6Y    27C512 EPROM (DIP28)   \ There is an alt. set of labels used for these ROMs with an 'S'
                BS07.1B    27C512 EPROM (DIP28)   | added to the name (i.e. 'BS15S'), but the actual ROM contents is identical
                BS19.4B    27C1001 EPROM (DIP32)  / to the regular set (both sets dumped / verified)
                BS-000.1H  4M mask ROM (DIP32) {sprite}
                BS-001.1F  4M mask ROM (DIP32) {sprite}
                BS-002.1D  4M mask ROM (DIP32) {sprite}
                BS-003.1K  4M mask ROM (DIP32) {sprite}
                BS-004.1S  4M mask ROM (DIP32) {tile}
                BS-005.1U  4M mask ROM (DIP32) {tile}
                BS-100.4D  1M mask ROM (DIP28) {Z80}
                BS-101.6W  1M mask ROM (DIP28) {Z80 data}
                BS-200.8C  1M mask ROM (DIP28) {Z80}
                BS-203.5J  2M mask ROM (DIP32) {OKI-M6295 samples}

      DIPs    - SW1
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |SCREEN NORMAL    OFF                        |
                |       FLIP      ON                         |
                |--------------------------------------------|
                |GAME   NORMAL        OFF                    |
                |MODE   TEST          ON                     |
                |--------------------------------------------|
                |COIN1  1C/1P                 OFF OFF        |
                |       1C/2P                 ON  OFF        |
                |       2C/1P                 OFF ON         |
                |       2C/3P                 ON  ON         |
                |                                            |
                |COIN2  1C/1P                         OFF OFF|
                |       1C/2P                         ON  OFF|
                |       2C/1P                         OFF ON |
                |       2C/3P                         ON  ON |
                |--------------------------------------------|
                |SW1 & SW4 NOT USED ALWAYS OFF               |
                |--------------------------------------------|

                SW2
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |DIFFICULTY                                  |
                |NORMAL       OFF OFF                        |
                |EASY         ON  OFF                        |
                |HARD         OFF ON                         |
                |HARDEST      ON  ON                         |
                |--------------------------------------------|
                |BONUS                                       |
                |10,30,50,70,90       OFF OFF                |
                |10,20,30,40,50,                             |
                |60,70,80,90          ON  OFF                |
                |20,50                OFF ON                 |
                |NONE                 ON  ON                 |
                |--------------------------------------------|
                |LIVES    5                   OFF OFF        |
                |         3                   ON  OFF        |
                |         7                   OFF ON         |
                |         9                   ON  ON         |
                |--------------------------------------------|
                |DEMO sound  YES                      OFF    |
                |            NO                       ON     |
                |--------------------------------------------|
                |SPEAKER     STEREO                       OFF|
                |OUTPUT      MONO                          ON|
                |--------------------------------------------|
*/

#include "emu.h"

#include "kan_pand.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mastercpu(*this, "mastercpu")
		, m_slavecpu(*this, "slavecpu")
		, m_soundcpu(*this, "soundcpu")
		, m_beast(*this, "beast")
		, m_pandora(*this, "pandora")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_slavelatch(*this, "slavelatch")
		, m_beastlatch(*this, "beastlatch")
		, m_videoram(*this, "videoram")
		, m_masterbank(*this, "master_bank%u", 0U)
		, m_slavebank(*this, "slave_bank")
		, m_soundbank(*this, "sound_bank")
		, m_port_in(*this, "IN%u", 0)
		, m_port_dsw(*this, "DSW%u", 1)
	{
	}

	void djboy(machine_config &config);

	void init_djboy();
	void init_djboyj();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	//static constexpr unsigned PROT_OUTPUT_BUFFER_SIZE = 8;

	uint8_t beast_status_r();
	void trigger_nmi_on_mastercpu(uint8_t data);
	void mastercpu_bankswitch_w(uint8_t data);
	void slavecpu_bankswitch_w(uint8_t data);
	void coin_count_w(uint8_t data);
	void soundcpu_bankswitch_w(uint8_t data);
	uint8_t beast_p0_r();
	void beast_p0_w(uint8_t data);
	uint8_t beast_p1_r();
	void beast_p1_w(uint8_t data);
	uint8_t beast_p2_r();
	void beast_p2_w(uint8_t data);
	uint8_t beast_p3_r();
	void beast_p3_w(uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void mastercpu_am(address_map &map);
	void mastercpu_port_am(address_map &map);
	void slavecpu_am(address_map &map);
	void slavecpu_port_am(address_map &map);
	void soundcpu_am(address_map &map);
	void soundcpu_port_am(address_map &map);

	// devices
	required_device<cpu_device> m_mastercpu;
	required_device<cpu_device> m_slavecpu;
	required_device<cpu_device> m_soundcpu;
	required_device<i80c51_device> m_beast;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_slavelatch;
	required_device<generic_latch_8_device> m_beastlatch;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;

	// ROM banking
	required_memory_bank_array<2> m_masterbank;
	required_memory_bank m_slavebank;
	required_memory_bank m_soundbank;

	required_ioport_array<3> m_port_in;
	required_ioport_array<2> m_port_dsw;

	uint8_t m_bankxor;

	// video-related
	tilemap_t *m_background = nullptr;
	uint8_t m_videoreg = 0;
	uint8_t m_scrollx = 0;
	uint8_t m_scrolly = 0;

	// Kaneko BEAST state
	uint8_t m_beast_p[4]{};
};


void djboy_state::scrollx_w(uint8_t data)
{
	m_scrollx = data;
}

void djboy_state::scrolly_w(uint8_t data)
{
	m_scrolly = data;
}

TILE_GET_INFO_MEMBER(djboy_state::get_bg_tile_info)
{
	uint8_t const attr = m_videoram[tile_index + 0x800];
	int code = m_videoram[tile_index] + (attr & 0xf) * 256;
	int const color = attr >> 4;

	if (color & 8)
		code |= 0x1000;

	tileinfo.set(0, code, color, 0);    // no flip
}

void djboy_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset & 0x7ff);
}

void djboy_state::video_start()
{
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(djboy_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
}

uint32_t djboy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/**
	 * xx------ msb x
	 * --x----- msb y
	 * ---x---- flipscreen?
	 * ----xxxx ROM bank
	 */

	int const scrollx = m_scrollx | ((m_videoreg & 0xc0) << 2);
	m_background->set_scrollx(0, scrollx - 0x391);

	int const scrolly = m_scrolly | ((m_videoreg & 0x20) << 3);
	m_background->set_scrolly(0, scrolly);

	m_background->draw(screen, bitmap, cliprect, 0, 0);
	m_pandora->update(bitmap, cliprect);

	return 0;
}

void djboy_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_pandora->eof();
	}
}


// KANEKO BEAST state

uint8_t djboy_state::beast_status_r()
{
	return (m_slavelatch->pending_r() ? 0x0 : 0x4) | (m_beastlatch->pending_r() ? 0x8 : 0x0);
}

/******************************************************************************/

void djboy_state::trigger_nmi_on_mastercpu(uint8_t data)
{
	m_mastercpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void djboy_state::mastercpu_bankswitch_w(uint8_t data)
{
	data ^= m_bankxor;
	m_masterbank[0]->set_entry(data);
	m_masterbank[1]->set_entry(0); // unsure if / how this area is banked
}

/******************************************************************************/

/**
 * xx------ msb scrollx
 * --x----- msb scrolly
 * ---x---- screen flip
 * ----xxxx bank
 */
void djboy_state::slavecpu_bankswitch_w(uint8_t data)
{
	m_videoreg = data;

	if ((data & 0xc) != 4)
		m_slavebank->set_entry((data & 0xf));
}

void djboy_state::coin_count_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

/******************************************************************************/

void djboy_state::soundcpu_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data);  // shall we check data<0x07?
}

/******************************************************************************/

void djboy_state::mastercpu_am(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xafff).bankr(m_masterbank[1]);
	map(0xb000, 0xbfff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_r), FUNC(kaneko_pandora_device::spriteram_w));
	map(0xc000, 0xdfff).bankr(m_masterbank[0]);
	map(0xe000, 0xffff).ram().share("master_slave");
}

void djboy_state::mastercpu_port_am(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(djboy_state::mastercpu_bankswitch_w));
}

/******************************************************************************/

void djboy_state::slavecpu_am(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_slavebank);
	map(0xc000, 0xcfff).ram().w(FUNC(djboy_state::videoram_w)).share(m_videoram);
	map(0xd000, 0xd3ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xd400, 0xd8ff).ram();
	map(0xe000, 0xffff).ram().share("master_slave");
}

void djboy_state::slavecpu_port_am(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(djboy_state::slavecpu_bankswitch_w));
	map(0x02, 0x02).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x04, 0x04).r(m_slavelatch, FUNC(generic_latch_8_device::read)).w(m_beastlatch, FUNC(generic_latch_8_device::write));
	map(0x06, 0x06).w(FUNC(djboy_state::scrolly_w));
	map(0x08, 0x08).w(FUNC(djboy_state::scrollx_w));
	map(0x0a, 0x0a).w(FUNC(djboy_state::trigger_nmi_on_mastercpu));
	map(0x0c, 0x0c).r(FUNC(djboy_state::beast_status_r));
	map(0x0e, 0x0e).w(FUNC(djboy_state::coin_count_w));
}

/******************************************************************************/

void djboy_state::soundcpu_am(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xc000, 0xdfff).ram();
}

void djboy_state::soundcpu_port_am(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(djboy_state::soundcpu_bankswitch_w));
	map(0x02, 0x03).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x06, 0x06).rw("oki_l", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x07, 0x07).rw("oki_r", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

/******************************************************************************/

uint8_t djboy_state::beast_p0_r()
{
	// ?
	return 0;
}

void djboy_state::beast_p0_w(uint8_t data)
{
	if (!BIT(m_beast_p[0], 1) && BIT(data, 1))
	{
		m_slavelatch->write(m_beast_p[1]);
	}

	if (BIT(~data, 0))
		m_beastlatch->acknowledge_w();

	m_beast_p[0] = data;
}

uint8_t djboy_state::beast_p1_r()
{
	if (BIT(~m_beast_p[0], 0))
		return m_beastlatch->read();
	else
		return 0; // ?
}

void djboy_state::beast_p1_w(uint8_t data)
{
	m_beast_p[1] = data;
}

uint8_t djboy_state::beast_p2_r()
{
	switch ((m_beast_p[0] >> 2) & 3)
	{
		case 0: return m_port_in[1]->read();
		case 1: return m_port_in[2]->read();
		case 2: return m_port_in[0]->read();
		default: return 0xff;
	}
}

void djboy_state::beast_p2_w(uint8_t data)
{
	m_beast_p[2] = data;
}

uint8_t djboy_state::beast_p3_r()
{
	uint8_t dsw = 0;
	uint8_t const dsw1 = ~m_port_dsw[0]->read();
	uint8_t const dsw2 = ~m_port_dsw[1]->read();

	switch ((m_beast_p[0] >> 5) & 3)
	{
		case 0: dsw = (BIT(dsw2, 4) << 3) | (BIT(dsw2, 0) << 2) | (BIT(dsw1, 4) << 1) | BIT(dsw1, 0); break;
		case 1: dsw = (BIT(dsw2, 5) << 3) | (BIT(dsw2, 1) << 2) | (BIT(dsw1, 5) << 1) | BIT(dsw1, 1); break;
		case 2: dsw = (BIT(dsw2, 6) << 3) | (BIT(dsw2, 2) << 2) | (BIT(dsw1, 6) << 1) | BIT(dsw1, 2); break;
		case 3: dsw = (BIT(dsw2, 7) << 3) | (BIT(dsw2, 3) << 2) | (BIT(dsw1, 7) << 1) | BIT(dsw1, 3); break;
	}
	return (dsw << 4) | (m_beastlatch->pending_r() ? 0x0 : 0x4) | (m_slavelatch->pending_r() ? 0x8 : 0x0);
}

void djboy_state::beast_p3_w(uint8_t data)
{
	m_beast_p[3] = data;
	m_slavecpu->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
}
// Program / data maps are defined in the 8051 core

/******************************************************************************/

static INPUT_PORTS_START( djboy )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // labeled "TEST" in self test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // punch
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // kick
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // jump
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") // Manual states "CAUTION  !! .... Don't use ."
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_HIGH, "SW1:3" )
//  PORT_DIPNAME( 0x04, 0x00, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW1:3")
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus Levels (in thousands)" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "10,30,50,70,90" )
	PORT_DIPSETTING(    0x04, "10,20,30,40,50,60,70,80,90" )
	PORT_DIPSETTING(    0x08, "20,50" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "9" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Stereo Sound" )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_djboy )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x000, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_djboy_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x100, 16 )
GFXDECODE_END

/******************************************************************************/

// Main Z80 uses IM2
TIMER_DEVICE_CALLBACK_MEMBER(djboy_state::scanline)
{
	int scanline = param;

	if (scanline == 240) // vblank-out irq
		m_mastercpu->set_input_line_and_vector(0, HOLD_LINE, 0xfd); // Z80

	// Pandora "sprite end dma" irq? TODO: timing is clearly off, attract mode relies on this
	if (scanline == 64)
		m_mastercpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void djboy_state::machine_start()
{
	uint8_t *master = memregion("mastercpu")->base();
	uint8_t *slave = memregion("slavecpu")->base();
	uint8_t *sound = memregion("soundcpu")->base();

	m_masterbank[0]->configure_entries(0, 32, &master[0x00000], 0x2000);
	m_slavebank->configure_entries(0, 4, &slave[0x00000], 0x4000);
	m_slavebank->configure_entries(8, 8, &slave[0x10000], 0x4000);
	m_soundbank->configure_entries(0, 8, &sound[0x00000], 0x4000);
	m_masterbank[1]->configure_entry(0, &master[0x08000]); // unsure if / how this area is banked

	save_item(NAME(m_videoreg));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	// Kaneko BEAST
	save_item(NAME(m_beast_p));
}

void djboy_state::machine_reset()
{
	m_videoreg = 0;
	m_scrollx = 0;
	m_scrolly = 0;
}

void djboy_state::djboy(machine_config &config)
{
	Z80(config, m_mastercpu, 12_MHz_XTAL / 2); // 6.000MHz, verified
	m_mastercpu->set_addrmap(AS_PROGRAM, &djboy_state::mastercpu_am);
	m_mastercpu->set_addrmap(AS_IO, &djboy_state::mastercpu_port_am);
	TIMER(config, "scantimer").configure_scanline(FUNC(djboy_state::scanline), "screen", 0, 1);

	Z80(config, m_slavecpu, 12_MHz_XTAL / 2); // 6.000MHz, verified
	m_slavecpu->set_addrmap(AS_PROGRAM, &djboy_state::slavecpu_am);
	m_slavecpu->set_addrmap(AS_IO, &djboy_state::slavecpu_port_am);
	m_slavecpu->set_vblank_int("screen", FUNC(djboy_state::irq0_line_hold));

	Z80(config, m_soundcpu, 12_MHz_XTAL / 2); // 6.000MHz, verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &djboy_state::soundcpu_am);
	m_soundcpu->set_addrmap(AS_IO, &djboy_state::soundcpu_port_am);

	I80C51(config, m_beast, 12_MHz_XTAL / 2); // 6.000MHz, verified
	m_beast->port_in_cb<0>().set(FUNC(djboy_state::beast_p0_r));
	m_beast->port_out_cb<0>().set(FUNC(djboy_state::beast_p0_w));
	m_beast->port_in_cb<1>().set(FUNC(djboy_state::beast_p1_r));
	m_beast->port_out_cb<1>().set(FUNC(djboy_state::beast_p1_w));
	m_beast->port_in_cb<2>().set(FUNC(djboy_state::beast_p2_r));
	m_beast->port_out_cb<2>().set(FUNC(djboy_state::beast_p2_w));
	m_beast->port_in_cb<3>().set(FUNC(djboy_state::beast_p3_r));
	m_beast->port_out_cb<3>().set(FUNC(djboy_state::beast_p3_w));

	config.set_maximum_quantum(attotime::from_hz(6000));

	GENERIC_LATCH_8(config, m_slavelatch);

	GENERIC_LATCH_8(config, m_beastlatch);
	m_beastlatch->data_pending_callback().set_inputline(m_beast, INPUT_LINE_IRQ0);
	m_beastlatch->set_separate_acknowledge(true);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.5);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(djboy_state::screen_update));
	screen.screen_vblank().set(FUNC(djboy_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_djboy);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x200);
	m_palette->set_endianness(ENDIANNESS_BIG);

	KANEKO_PANDORA(config, m_pandora, 0, m_palette, gfx_djboy_spr);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12_MHz_XTAL / 4)); // 3.000MHz, verified
	ymsnd.irq_handler().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.40);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.40);

	okim6295_device &oki_l(OKIM6295(config, "oki_l", 12_MHz_XTAL / 8, okim6295_device::PIN7_LOW)); // 1.500MHz, verified
	oki_l.set_device_rom_tag("oki");
	oki_l.add_route(ALL_OUTPUTS, "lspeaker", 0.50);

	okim6295_device &oki_r(OKIM6295(config, "oki_r", 12_MHz_XTAL / 8, okim6295_device::PIN7_LOW)); // 1.500MHz, verified
	oki_r.set_device_rom_tag("oki");
	oki_r.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}


ROM_START( djboy )
	ROM_REGION( 0x40000, "mastercpu", 0 )
	ROM_LOAD( "djboy.4b",  0x00000, 0x20000, CRC(354531ec) SHA1(4722376601ca2d8fb79622fef35ab5b5b084555d) ) // verified on 2 PCBs, neither PCB had a label on the ROM
	ROM_LOAD( "bs100.4d",  0x20000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x30000, "slavecpu", 0 )
	ROM_LOAD( "djboy.5y",  0x00000, 0x10000, CRC(91eb189a) SHA1(a6a2662369cd6c851ae45a49654f6150fa8cf42e) ) // mask ROM without label
	ROM_LOAD( "bs101.6w",  0x10000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "bs200.8c",  0x00000, 0x20000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )

	ROM_REGION( 0x1000, "beast", 0 ) // MSM80C51F microcontroller
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) // Internal ROM image

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs06.1b",  0x1f0000, 0x10000, CRC(22c8aa08) SHA1(5521c9d73b4ee82a2de1992d6edc7ef62788ad72) ) // replaces last 0x200 tiles - verified correct for the World set

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboyu )
	ROM_REGION( 0x40000, "mastercpu", 0 )
	ROM_LOAD( "bs64.4b",   0x00000, 0x20000, CRC(b77aacc7) SHA1(78100d4695738a702f13807526eb1bcac759cce3) )
	ROM_LOAD( "bs100.4d",  0x20000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x30000, "slavecpu", 0 )
	ROM_LOAD( "bs65.5y",   0x00000, 0x10000, CRC(0f1456eb) SHA1(62ed48c0d71c1fabbb3f6ada60381f57f692cef8) )
	ROM_LOAD( "bs101.6w",  0x10000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "bs200.8c",  0x00000, 0x20000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )

	ROM_REGION( 0x1000, "beast", 0 ) // MSM80C51F microcontroller
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) // Internal ROM image

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07s.1b", 0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) ) // replaces last 0x200 tiles - found labeled as both BS07 and BS07S, same data - verified

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboyua )
	ROM_REGION( 0x40000, "mastercpu", 0 )
	ROM_LOAD( "bs19s.4b",  0x00000, 0x20000, CRC(17ce9f6c) SHA1(a0c1832b05dc46991e8949067ca0278f5498835f) ) // found labeled as both BS19 and BS19S, same data - verified
	ROM_LOAD( "bs100.4d",  0x20000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x30000, "slavecpu", 0 )
	ROM_LOAD( "bs15s.5y",  0x00000, 0x10000, CRC(e6f966b2) SHA1(f9df16035a8b09d87eb70315b216892e25d99b03) ) // found labeled as both BS15 and BS15S, same data - verified
	ROM_LOAD( "bs101.6w",  0x10000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "bs200.8c",  0x00000, 0x20000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )

	ROM_REGION( 0x1000, "beast", 0 ) // MSM80C51F microcontroller
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) // Internal ROM image

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07s.1b", 0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) ) // replaces last 0x200 tiles - found labeled as both BS07 and BS07S, same data - verified

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboyj )
	ROM_REGION( 0x40000, "mastercpu", 0 )
	ROM_LOAD( "bs12.4b",   0x00000, 0x20000, CRC(0971523e) SHA1(f90cd02cedf8632f4b651de7ea75dc8c0e682f6e) )
	ROM_LOAD( "bs100.4d",  0x20000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x30000, "slavecpu", 0 )
	ROM_LOAD( "bs13.5y",   0x00000, 0x10000, CRC(5c3f2f96) SHA1(bb7ee028a2d8d3c76a78a29fba60bcc36e9399f5) )
	ROM_LOAD( "bs101.6w",  0x10000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "bs200.8c",  0x00000, 0x20000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )

	ROM_REGION( 0x1000, "beast", 0 ) // MSM80C51F microcontroller
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) // Internal ROM image

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs06.1b",  0x1f0000, 0x10000, CRC(22c8aa08) SHA1(5521c9d73b4ee82a2de1992d6edc7ef62788ad72) ) // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "bs-204.5j", 0x000000, 0x40000, CRC(510244f0) SHA1(afb502d46d268ad9cd209ae1da72c50e4e785626) )
ROM_END

ROM_START( djboyja )
	ROM_REGION( 0x40000, "mastercpu", 0 )
	ROM_LOAD( "djboyja.4b", 0x00000, 0x20000, CRC(f7ac20ca) SHA1(1bf66bfaa6d98f4cff50bc4d15cebfb7f4b09ae5) ) // mask ROM without label
	ROM_LOAD( "bs100.4d",   0x20000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x30000, "slavecpu", 0 )
	ROM_LOAD( "bs13.5y",   0x00000, 0x10000, CRC(5c3f2f96) SHA1(bb7ee028a2d8d3c76a78a29fba60bcc36e9399f5) )
	ROM_LOAD( "bs101.6w",  0x10000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD( "bs200.8c",  0x00000, 0x20000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )

	ROM_REGION( 0x1000, "beast", 0 ) // MSM80C51F microcontroller
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) // Internal ROM image

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs06.1b",  0x1f0000, 0x10000, CRC(22c8aa08) SHA1(5521c9d73b4ee82a2de1992d6edc7ef62788ad72) ) // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "bs24_l.5j", 0x000000, 0x20000, CRC(b0e0a452) SHA1(ff2d7ea750d43b1a05c55ad01296866af9aea444) ) // same data as bs-204.5j but split in halves
	ROM_LOAD( "bs24_h.5l", 0x020000, 0x20000, CRC(d24988ad) SHA1(22c2b9823e5320cf4e659d3e3b54991617b1e6b1) )
ROM_END


void djboy_state::init_djboy()
{
	m_bankxor = 0x00;
}

void djboy_state::init_djboyj()
{
	m_bankxor = 0x1f;
}

} // anonymous namespace


//    YEAR, NAME,    PARENT, MACHINE, INPUT, STATE,       INIT,          MNTR, COMPANY,                           FULLNAME,                FLAGS
GAME( 1989, djboy,   0,      djboy,   djboy, djboy_state, init_djboy,    ROT0, "Kaneko",                          "DJ Boy (World)",        MACHINE_SUPPORTS_SAVE )
GAME( 1990, djboyu,  djboy,  djboy,   djboy, djboy_state, init_djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (US, set 1)",    MACHINE_SUPPORTS_SAVE ) // Sammy & Williams logos in FG ROM
GAME( 1990, djboyua, djboy,  djboy,   djboy, djboy_state, init_djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (US, set 2)",    MACHINE_SUPPORTS_SAVE ) // Sammy & Williams logos in FG ROM
GAME( 1989, djboyj,  djboy,  djboy,   djboy, djboy_state, init_djboyj,   ROT0, "Kaneko (Sega license)",           "DJ Boy (Japan, set 1)", MACHINE_SUPPORTS_SAVE ) // Sega logo in FG ROM
GAME( 1989, djboyja, djboy,  djboy,   djboy, djboy_state, init_djboyj,   ROT0, "Kaneko (Sega license)",           "DJ Boy (Japan, set 2)", MACHINE_SUPPORTS_SAVE ) // Sega logo in FG ROM
