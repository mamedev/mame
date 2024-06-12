// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina, David Haywood
/***************************************************************************************

Heavy Unit
Kaneko/Taito 1988

Driver based on djboy.cpp / airbustr.cpp

This game runs on Kaneko hardware. The game is similar to R-Type.

PCB Layout
----------
M6100391A
M6100392A  880210204
KNA-001
|----------------------------------------------------|
|                                                    |
|           6116                          6116       |
|      15Mhz                              6116       |
|                                 PAL                |
|           B73_09.2P             B73_11.5P          |
|                                                    |
|                                                    |
|                                 Z80-1   DSW1 DSW2 J|
|                                                   A|
|     16MHz                                         M|
|                                                   M|
|       12MHz                       6264  MERMAID   A|
| B73_05.1H                                          |
| B73_04.1F B73_08.2F  6116              Z80-2       |
| B73_03.1D                       Z80-3  B73_12.7E   |
| B73_02.1C B73_07.2C  PANDORA    B73_10.5C  6116    |
| B73_01.1B B73_06.2B 4164 4164   6264 PAL  YM3014   |
|                     4164 4164   PAL       YM2203   |
|----------------------------------------------------|

Notes:
      Z80-1 clock  : 6.000MHz
      Z80-2 clock  : 6.000MHz
      Z80-3 clock  : 6.000MHz
      YM2203 clock : 3.000MHz
      VSync        : 58Hz
      HSync        : 15.59kHz
               \-\ : KANEKO 1988. DIP40 8751 MCU
      MERMAID    | : pin 18,19 = 6.000MHz (main clock)
                 | : pin 30 = 1.000MHz (prog/ale)
               /-/ : pin 22 = 111.48Hz (port 2 bit 1)

      PANDORA      : KANEKO PX79480FP-3 PANDORA-CHIP (C) KANEKO 1988


To Do:
------

- Fix cocktail mode
- Fix Flip Screen (currently displays an information screen and emulation hangs)


***************************************************************************************/

#include "emu.h"

#include "kan_pand.h"

#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

/*************************************
 *
 *  Driver state
 *
 *************************************/

class hvyunit_state : public driver_device
{
public:
	hvyunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mastercpu(*this, "master")
		, m_slavecpu(*this, "slave")
		, m_mermaid(*this, "mermaid")
		, m_soundcpu(*this, "soundcpu")
		, m_pandora(*this, "pandora")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_mermaidlatch(*this, "mermaidlatch")
		, m_slavelatch(*this, "slavelatch")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_masterbank(*this, "master_bank")
		, m_slavebank(*this, "slave_bank")
		, m_soundbank(*this, "sound_bank")
		, m_port_in(*this, "IN%u", 0U)
		, m_port_dsw(*this, "DSW%u", 1U)
	{
	}

	void hvyunit(machine_config &config);

private:
	void trigger_nmi_on_slave_cpu(uint8_t data);
	void master_bankswitch_w(uint8_t data);
	uint8_t mermaid_status_r();
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void slave_bankswitch_w(uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void slave_ack_w(uint8_t data);
	void sound_bankswitch_w(uint8_t data);
	void mermaid_p0_w(uint8_t data);
	uint8_t mermaid_p1_r();
	void mermaid_p1_w(uint8_t data);
	uint8_t mermaid_p2_r();
	void mermaid_p2_w(uint8_t data);
	uint8_t mermaid_p3_r();
	void mermaid_p3_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void master_io(address_map &map);
	void master_memory(address_map &map);
	void slave_io(address_map &map);
	void slave_memory(address_map &map);
	void sound_io(address_map &map);
	void sound_memory(address_map &map);

	// Devices
	required_device<cpu_device> m_mastercpu;
	required_device<cpu_device> m_slavecpu;
	required_device<i80c51_device> m_mermaid;
	required_device<cpu_device> m_soundcpu;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_mermaidlatch;
	required_device<generic_latch_8_device> m_slavelatch;

	// Memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	required_memory_bank m_masterbank;
	required_memory_bank m_slavebank;
	required_memory_bank m_soundbank;

	// Ports
	required_ioport_array<3> m_port_in;
	required_ioport_array<2> m_port_dsw;

	// Video
	tilemap_t       *m_bg_tilemap = nullptr;
	uint16_t          m_scrollx = 0;
	uint16_t          m_scrolly = 0;
	uint16_t          m_port0_data = 0;

	// Mermaid
	uint8_t           m_mermaid_p[4]{};

};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

void hvyunit_state::machine_start()
{
	m_masterbank->configure_entries(0, 8, memregion("master")->base(), 0x4000);
	m_slavebank->configure_entries(0, 4, memregion("slave")->base(), 0x4000);
	m_soundbank->configure_entries(0, 4, memregion("soundcpu")->base(), 0x4000);

	std::fill_n(&m_mermaid_p[0], 4, 0xff);
	save_item(NAME(m_mermaid_p));
}

void hvyunit_state::machine_reset()
{
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

TILE_GET_INFO_MEMBER(hvyunit_state::get_bg_tile_info)
{
	uint32_t const attr = m_colorram[tile_index];
	uint32_t const code = m_videoram[tile_index] + ((attr & 0x0f) << 8);
	uint32_t const color = (attr >> 4);

	tileinfo.set(0, code, color, 0);
}

void hvyunit_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hvyunit_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_port0_data));
}

uint32_t hvyunit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	constexpr int SX_POS = 96;
	constexpr int SY_POS = 0;

	m_bg_tilemap->set_scrollx(0, ((m_port0_data & 0x40) << 2) + m_scrollx + SX_POS); // TODO
	m_bg_tilemap->set_scrolly(0, ((m_port0_data & 0x80) << 1) + m_scrolly + SY_POS); // TODO
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_pandora->update(bitmap, cliprect);

	return 0;
}

void hvyunit_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_pandora->eof();
	}
}


/*************************************
 *
 *  Master CPU handlers
 *
 *************************************/

void hvyunit_state::trigger_nmi_on_slave_cpu(uint8_t data)
{
	m_slavecpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void hvyunit_state::master_bankswitch_w(uint8_t data)
{
	m_masterbank->set_entry(data & 7);
}

uint8_t hvyunit_state::mermaid_status_r()
{
	return (!m_slavelatch->pending_r() << 2) | (m_mermaidlatch->pending_r() << 3);
}


/*************************************
 *
 *  Slave CPU handlers
 *
 *************************************/

void hvyunit_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void hvyunit_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void hvyunit_state::slave_bankswitch_w(uint8_t data)
{
	m_port0_data = data;
	m_slavebank->set_entry(data & 3);
}

void hvyunit_state::scrollx_w(uint8_t data)
{
	m_scrollx = data;
}

void hvyunit_state::scrolly_w(uint8_t data)
{
	m_scrolly = data;
}

void hvyunit_state::slave_ack_w(uint8_t data)
{
	m_slavecpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


/*************************************
 *
 *  Sound CPU handlers
 *
 *************************************/

void hvyunit_state::sound_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 3);
}


/*************************************
 *
 *  Protection MCU handlers
 *
 *************************************/

void hvyunit_state::mermaid_p0_w(uint8_t data)
{
	if (!BIT(m_mermaid_p[0], 1) && BIT(data, 1))
		m_slavelatch->write(m_mermaid_p[1]);

	if (BIT(data, 0) == 0)
		m_mermaid_p[1] = m_mermaidlatch->read();

	if (!BIT(m_mermaid_p[0], 4) && BIT(data, 4))
	{
		machine().bookkeeping().coin_counter_w(0, BIT(m_mermaid_p[2], 0));
		machine().bookkeeping().coin_counter_w(1, BIT(m_mermaid_p[2], 1));
		machine().bookkeeping().coin_lockout_w(0, !BIT(m_mermaid_p[2], 2));
		machine().bookkeeping().coin_lockout_w(1, !BIT(m_mermaid_p[2], 3));
	}

	m_mermaid_p[0] = data;
}

uint8_t hvyunit_state::mermaid_p1_r()
{
	return m_mermaid_p[1];
}

void hvyunit_state::mermaid_p1_w(uint8_t data)
{
	m_mermaid_p[1] = data;
}

uint8_t hvyunit_state::mermaid_p2_r()
{
	switch ((m_mermaid_p[0] >> 2) & 3)
	{
		case 0: return m_port_in[1]->read();
		case 1: return m_port_in[2]->read();
		case 2: return m_port_in[0]->read();
		default: return 0xff;
	}
}

void hvyunit_state::mermaid_p2_w(uint8_t data)
{
	m_mermaid_p[2] = data;
}

uint8_t hvyunit_state::mermaid_p3_r()
{
	uint8_t dsw = 0;
	uint8_t const dsw1 = m_port_dsw[0]->read();
	uint8_t const dsw2 = m_port_dsw[1]->read();

	switch ((m_mermaid_p[0] >> 5) & 3)
	{
		case 0: dsw = (BIT(dsw2, 4) << 3) | (BIT(dsw2, 0) << 2) | (BIT(dsw1, 4) << 1) | BIT(dsw1, 0); break;
		case 1: dsw = (BIT(dsw2, 5) << 3) | (BIT(dsw2, 1) << 2) | (BIT(dsw1, 5) << 1) | BIT(dsw1, 1); break;
		case 2: dsw = (BIT(dsw2, 6) << 3) | (BIT(dsw2, 2) << 2) | (BIT(dsw1, 6) << 1) | BIT(dsw1, 2); break;
		case 3: dsw = (BIT(dsw2, 7) << 3) | (BIT(dsw2, 3) << 2) | (BIT(dsw1, 7) << 1) | BIT(dsw1, 3); break;
	}

	return (dsw << 4) | (m_slavelatch->pending_r() << 3) | (!m_mermaidlatch->pending_r() << 2);
}

void hvyunit_state::mermaid_p3_w(uint8_t data)
{
	m_mermaid_p[3] = data;
	m_slavecpu->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void hvyunit_state::master_memory(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_masterbank);
	map(0xc000, 0xcfff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_r), FUNC(kaneko_pandora_device::spriteram_w));
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xffff).ram().share("sharedram");
}

void hvyunit_state::master_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(hvyunit_state::master_bankswitch_w));
	map(0x01, 0x01).w(FUNC(hvyunit_state::master_bankswitch_w)); // correct?
	map(0x02, 0x02).w(FUNC(hvyunit_state::trigger_nmi_on_slave_cpu));
}


void hvyunit_state::slave_memory(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_slavebank);
	map(0xc000, 0xc3ff).ram().w(FUNC(hvyunit_state::videoram_w)).share(m_videoram);
	map(0xc400, 0xc7ff).ram().w(FUNC(hvyunit_state::colorram_w)).share(m_colorram);
	map(0xd000, 0xd1ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd200, 0xd7ff).ram();
	map(0xd800, 0xd9ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xda00, 0xdfff).ram();
	map(0xe000, 0xffff).ram().share("sharedram");
}

void hvyunit_state::slave_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(hvyunit_state::slave_bankswitch_w));
	map(0x02, 0x02).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x04, 0x04).r(m_slavelatch, FUNC(generic_latch_8_device::read));
	map(0x04, 0x04).w(m_mermaidlatch, FUNC(generic_latch_8_device::write));
	map(0x06, 0x06).w(FUNC(hvyunit_state::scrolly_w));
	map(0x08, 0x08).w(FUNC(hvyunit_state::scrollx_w));
	map(0x0c, 0x0c).r(FUNC(hvyunit_state::mermaid_status_r));
	map(0x0e, 0x0e).w(FUNC(hvyunit_state::slave_ack_w));

//  map(0x22, 0x22).r(FUNC(hvyunit_state::hu_scrolly_hi_reset)); //22/a2 taken from ram $f065
//  map(0xa2, 0xa2).r(FUNC(hvyunit_state::hu_scrolly_hi_set));
}


void hvyunit_state::sound_memory(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xc000, 0xc7ff).ram();
}

void hvyunit_state::sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(hvyunit_state::sound_bankswitch_w));
	map(0x02, 0x03).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( hvyunit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Service_Mode ) )     PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, ( "Coin Mode" ) )         PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, ( "Mode 1" ) )
	PORT_DIPSETTING(    0x00, ( "Mode 2" ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x08, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bonus" )             PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( hvyunitj )
	PORT_INCLUDE(hvyunit)

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW1:4")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_hvyunit )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x000, 16 ) // background tiles
GFXDECODE_END

static GFXDECODE_START( gfx_hvyunit_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x100, 16 ) // sprite bank
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

// Main Z80 uses IM2
TIMER_DEVICE_CALLBACK_MEMBER(hvyunit_state::scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_mastercpu->set_input_line_and_vector(0, HOLD_LINE, 0xfd); // Z80

	// Pandora "sprite end dma" irq? TODO: timing is likely off
	if(scanline == 64)
		m_mastercpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void hvyunit_state::hvyunit(machine_config &config)
{
	Z80(config, m_mastercpu, XTAL(12'000'000)/2); // 6MHz verified on PCB
	m_mastercpu->set_addrmap(AS_PROGRAM, &hvyunit_state::master_memory);
	m_mastercpu->set_addrmap(AS_IO, &hvyunit_state::master_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(hvyunit_state::scanline), "screen", 0, 1);

	Z80(config, m_slavecpu, XTAL(12'000'000)/2); // 6MHz verified on PCB
	m_slavecpu->set_addrmap(AS_PROGRAM, &hvyunit_state::slave_memory);
	m_slavecpu->set_addrmap(AS_IO, &hvyunit_state::slave_io);
	m_slavecpu->set_vblank_int("screen", FUNC(hvyunit_state::irq0_line_assert));

	Z80(config, m_soundcpu, XTAL(12'000'000)/2); // 6MHz verified on PCB
	m_soundcpu->set_addrmap(AS_PROGRAM, &hvyunit_state::sound_memory);
	m_soundcpu->set_addrmap(AS_IO, &hvyunit_state::sound_io);

	I80C51(config, m_mermaid, XTAL(12'000'000)/2); // 6MHz verified on PCB
	m_mermaid->port_out_cb<0>().set(FUNC(hvyunit_state::mermaid_p0_w));
	m_mermaid->port_in_cb<1>().set(FUNC(hvyunit_state::mermaid_p1_r));
	m_mermaid->port_out_cb<1>().set(FUNC(hvyunit_state::mermaid_p1_w));
	m_mermaid->port_in_cb<2>().set(FUNC(hvyunit_state::mermaid_p2_r));
	m_mermaid->port_out_cb<2>().set(FUNC(hvyunit_state::mermaid_p2_w));
	m_mermaid->port_in_cb<3>().set(FUNC(hvyunit_state::mermaid_p3_r));
	m_mermaid->port_out_cb<3>().set(FUNC(hvyunit_state::mermaid_p3_w));

	GENERIC_LATCH_8(config, m_mermaidlatch);
	m_mermaidlatch->data_pending_callback().set_inputline(m_mermaid, INPUT_LINE_IRQ0);

	GENERIC_LATCH_8(config, m_slavelatch);

	config.set_maximum_quantum(attotime::from_hz(6000));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 240-1);
	screen.set_screen_update(FUNC(hvyunit_state::screen_update));
	screen.screen_vblank().set(FUNC(hvyunit_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_hvyunit);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x800);

	KANEKO_PANDORA(config, m_pandora, 0, m_palette, gfx_hvyunit_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000)/4)); // 3MHz verified on PCB
	ymsnd.irq_handler().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.80);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// There is likely a World version using the newer (B73_25 - B73_28) graphics ROMs with a program ROM labeled B73_29

ROM_START( hvyunit )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_10.5c",  0x00000, 0x20000, CRC(ca52210f) SHA1(346951962aa5bbad641117dbd66f035dddc7c0bf) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_11.5p",  0x00000, 0x10000, CRC(cb451695) SHA1(116fd59f96a54c22fae65eea9ee5e58cb9ce5074) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	//                      0x190000, 0x010000  no data
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	//                      0x1b0000, 0x010000  no data
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitj )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_30.5c",  0x00000, 0x20000, CRC(600af545) SHA1(c52b9be2bae28848ad0818c296f000a1bda4fa4f) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_28.2c",  0x100000, 0x020000, CRC(a02e08d6) SHA1(72764d4e8474aaac0674fd1c20278a706da7ade2) ) // == b73_22.2c, despite the different label
	ROM_LOAD( "b73_27.2b",  0x120000, 0x020000, CRC(8708f97c) SHA1(ccddc7f2fa53c5e35345c2db0520f515c512b723) ) // == b73_21.2b, despite the different label
	ROM_LOAD( "b73_25.0b",  0x140000, 0x020000, CRC(2f13f81e) SHA1(9d9c1869bf582a0bc0581cdf5b65237124b9e456) ) // == b73_15.0b, despite the different label
	ROM_LOAD( "b73_26.0c",  0x160000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) ) // == b73_16.0c, despite the different label

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitja )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_24.5c",  0x00000, 0x20000, CRC(60122f5a) SHA1(f9abccc8c4f65c613f901c7baebe02881ea8bf60) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "sprites", 0 ) // The data in first half of b73_15.0b actually differs slightly to the other sets, a 0x22 fill is replaced by 0xff on empty tiles
	ROM_LOAD( "b73_22.2c",  0x100000, 0x020000, CRC(a02e08d6) SHA1(72764d4e8474aaac0674fd1c20278a706da7ade2) ) // == b73_28.2c, despite the different label - M5M27C101P mask ROM
	ROM_LOAD( "b73_21.2b",  0x120000, 0x020000, CRC(8708f97c) SHA1(ccddc7f2fa53c5e35345c2db0520f515c512b723) ) // == b73_27.2b, despite the different label - M5M27C101P mask ROM
	ROM_LOAD( "b73_15.0b",  0x140000, 0x020000, CRC(2f13f81e) SHA1(9d9c1869bf582a0bc0581cdf5b65237124b9e456) ) // == b73_25.0b, despite the different label - M5M27C101P mask ROM
	ROM_LOAD( "b73_16.0c",  0x160000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) ) // == b73_26.0c, despite the different label - HN27512P mask ROM
	// The data below is moved from 0x000000-0x080000 to 0x180000-0x200000 compared to other sets
	ROM_LOAD( "b73_17.0d",  0x180000, 0x020000, CRC(a8ec5309) SHA1(55418711fd9ca38f1c41c8c7dd7984920702c5e9) ) // == 1/4 b73_08.2f - M5M27C101P mask ROM
	ROM_LOAD( "b73_18.0f",  0x1a0000, 0x020000, CRC(dc955a69) SHA1(f476f449c1a6b1d0212e16827c121713451c1918) ) // == 2/4 b73_08.2f - M5M27C101P mask ROM
	ROM_LOAD( "b73_19.0h",  0x1c0000, 0x020000, CRC(2fb1b3e3) SHA1(f25e8a432721a772b62eff52a0b97e89f56d79af) ) // == 3/4 b73_08.2f - M5M27C101P mask ROM
	ROM_LOAD( "b73_20.0k",  0x1e0000, 0x020000, CRC(0662d0dd) SHA1(323b3f1d8fc034e22e8ac8dcc17b080ecaeaf3ed) ) // == 4/4 b73_08.2f - M5M27C101P mask ROM

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "b73_23.2p",  0x000000, 0x080000, NO_DUMP )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitjo )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_13.5c",  0x00000, 0x20000, CRC(e2874601) SHA1(7f7f3287113b8622eb365d04135d2d9c35d70554) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	//                      0x190000, 0x010000  no data
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	//                      0x1b0000, 0x010000  no data
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitu )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_34.5c",  0x00000, 0x20000, CRC(05c30a90) SHA1(97cc0ded2896e0945d790247c284e5058c28c735) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_35.6p",  0x00000, 0x10000, CRC(aed1669d) SHA1(d0539261d6128fa2d58b529e8383b6d1f3ccac77) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_28.2c",  0x100000, 0x020000, CRC(a02e08d6) SHA1(72764d4e8474aaac0674fd1c20278a706da7ade2) ) // == b73_22.2c, despite the different label
	ROM_LOAD( "b73_27.2b",  0x120000, 0x020000, CRC(8708f97c) SHA1(ccddc7f2fa53c5e35345c2db0520f515c512b723) ) // == b73_21.2b, despite the different label
	ROM_LOAD( "b73_25.0b",  0x140000, 0x020000, CRC(2f13f81e) SHA1(9d9c1869bf582a0bc0581cdf5b65237124b9e456) ) // == b73_15.0b, despite the different label
	ROM_LOAD( "b73_26.0c",  0x160000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) ) // == b73_16.0c, despite the different label

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, hvyunit,   0,       hvyunit, hvyunit,  hvyunit_state, empty_init, ROT0, "Kaneko / Taito", "Heavy Unit (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, hvyunitj,  hvyunit, hvyunit, hvyunitj, hvyunit_state, empty_init, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, newer)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, hvyunitja, hvyunit, hvyunit, hvyunitj, hvyunit_state, empty_init, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, alternate ROM format)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, hvyunitjo, hvyunit, hvyunit, hvyunitj, hvyunit_state, empty_init, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, older)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, hvyunitu,  hvyunit, hvyunit, hvyunitj, hvyunit_state, empty_init, ROT0, "Kaneko / Taito", "Heavy Unit -U.S.A. Version- (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
