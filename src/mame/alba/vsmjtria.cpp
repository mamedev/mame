// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Angelo Salese

/*
VS Mahjong Triangle (c) 1986 Dyna

Unmarked board with everything doubled up, to provide a versus experience with a single board
2 players can play separately or one against the other

CPU:    2x Z80
RAM:    6x M5M5117P
Sound:  2x AY-3-8910
I/O:    2x 8255, 4x 8-dip banks, 1x single dip switch (at position 11m)
OSC:    20MHz

Notes:
- Loosely based off alba/rmhaihai.cpp.
  Changes needed for merging both implementations seems too non-trivial to warrant a
  driver merge, basically just the video HW, the CRTC (448x224 clocked at 20 MHz?)
  and a few I/O bits looks similar, the odd screen size is also a thing in srmp2.cpp
  and probably other Seta HWs.
- CPU seems a bit too slow when deciding about what tile to discard,
  it also takes a bit too much to sync when a player takes a tile from the pond in
  a VS play. It's most likely btanb unless a gameplay video proves otherwise;
- Position 11m on PCB has a single slider switch.
  It is not read by the CPUs at all, but instead zaps the nearby NVRAM contents if active.

TODO:
- Upper byte of I/O access seems to be some extra comms state machine
  (i.e. it more or less mimic the commands that CPUs send between each other).
  Maybe just a debug port that is read thru external HW?
- Pinpoint how HW reads the three lead-filled marked as SW1 (position 1b).
  It's located next to the key matrix, maybe just ext debugging?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vsmjtria_state : public driver_device
{
public:
	vsmjtria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu%u", 0U)
		, m_gfxdecode(*this, "gfxdecode%u", 0U)
		, m_colorram(*this, "colorram%u", 0U)
		, m_videoram(*this, "videoram%u", 0U)
		, m_key{ { *this, "P1_KEY%u", 0U }, { *this, "P2_KEY%u", 0U } }
		, m_nvram(*this, "nvram%u", 0U)
		, m_nvram_data(*this, "nvram%u", 0U)
		, m_sw2(*this, "SW2")
	{ }

	void vsmjtria(machine_config &config);

	void init_vsmjtria();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device_array<cpu_device, 2> m_cpu;
	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_ioport_array<5> m_key[2];
	required_device_array<nvram_device, 2> m_nvram;
	required_shared_ptr_array<u8, 2> m_nvram_data;
	required_ioport m_sw2;

	tilemap_t *m_bg_tilemap[2];
	uint8_t m_keyboard_cmd[2];

	template <uint8_t Which> void nmi_set_w(uint8_t data);
	template <uint8_t Which> void nmi_ack_w(uint8_t data);
	template <uint8_t Which> void ctrl_w(uint8_t data);
	template <uint8_t Which> uint8_t keyboard_r();
	template <uint8_t Which> void keyboard_w(uint8_t data);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_prg_map(address_map &map);
	void main_io_map(address_map &map);
	void sub_prg_map(address_map &map);
	void sub_io_map(address_map &map);
};

/*
 * Video section
 */

void vsmjtria_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(vsmjtria_state::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(vsmjtria_state::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

template <uint8_t Which>
void vsmjtria_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void vsmjtria_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(vsmjtria_state::get_bg_tile_info)
{
	int attr = m_colorram[Which][tile_index];
	int code = m_videoram[Which][tile_index] + ((attr & 0x07) << 8) + ((attr & 0x80) << 4);
	int color = (attr >> 3);

	tileinfo.set(0, code, color, 0);
}

template <uint8_t Which>
uint32_t vsmjtria_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap[Which]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/*
 * I/O
 */

template <uint8_t Which>
void vsmjtria_state::nmi_set_w(uint8_t data)
{
	m_cpu[Which]->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

template <uint8_t Which>
void vsmjtria_state::nmi_ack_w(uint8_t data)
{
	m_cpu[Which]->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

// TODO: flip screen and coin counter are actually never triggered, are they really connected for this HW?
// (ctrl_w fn comes from rmhaihai.cpp assumption of being identical, it's most likely not present here)
template <uint8_t Which>
void vsmjtria_state::ctrl_w(uint8_t data)
{
//  flip_screen_set(data & 0x01);
	m_bg_tilemap[Which]->set_flip((data & 1) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	// (data & 0x02) is switched on and off in service mode

	machine().bookkeeping().coin_lockout_w(Which, ~data & 0x04);
	machine().bookkeeping().coin_counter_w(Which, data & 0x08);
}

template <uint8_t Which>
uint8_t vsmjtria_state::keyboard_r()
{
	uint8_t data = 0x00;

	for (int key_n = 0; key_n < 5; key_n ++ )
	{
		if (BIT(m_keyboard_cmd[Which], key_n))
			data |= m_key[Which][key_n]->read();
	}

	return data;
}

template <uint8_t Which>
void vsmjtria_state::keyboard_w(uint8_t data)
{
	// bits 6-5 are always set when accessed, unknown purpose
	// 0x60 (at boot only, not read back), then 0x61 -> 0x62 -> 0x64 -> 0x68 -> 0x70 and back to 0x61
	m_keyboard_cmd[Which] = data;
}

void vsmjtria_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram().share("nvram0");
	map(0xa800, 0xafff).ram().w(FUNC(vsmjtria_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xb000, 0xb7ff).ram().w(FUNC(vsmjtria_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xc000, 0xdfff).rom().region("cpu0", 0xa000);
	map(0xe000, 0xffff).rom();
}

void vsmjtria_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).r("ay0", FUNC(ay8910_device::data_r));
	map(0x20, 0x21).w("ay0", FUNC(ay8910_device::address_data_w));
	map(0x60, 0x60).w(FUNC(vsmjtria_state::ctrl_w<0>));
	map(0xa0, 0xa0).w(FUNC(vsmjtria_state::nmi_ack_w<0>));
	map(0xc0, 0xc0).w(FUNC(vsmjtria_state::nmi_set_w<1>));
	map(0xe0, 0xe0).r("latch1", FUNC(generic_latch_8_device::read)).w("latch0", FUNC(generic_latch_8_device::write));
}

void vsmjtria_state::sub_prg_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram().share("nvram1");
	map(0xa800, 0xafff).ram().w(FUNC(vsmjtria_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xb000, 0xb7ff).ram().w(FUNC(vsmjtria_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xc000, 0xdfff).rom().region("cpu1", 0xa000);
	map(0xe000, 0xffff).rom();
}

void vsmjtria_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).r("ay1", FUNC(ay8910_device::data_r));
	map(0x20, 0x21).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x60, 0x60).w(FUNC(vsmjtria_state::ctrl_w<1>));
	map(0xa0, 0xa0).w(FUNC(vsmjtria_state::nmi_ack_w<1>));
	map(0xc0, 0xc0).w(FUNC(vsmjtria_state::nmi_set_w<0>));
	map(0xe0, 0xe0).r("latch0", FUNC(generic_latch_8_device::read)).w("latch1", FUNC(generic_latch_8_device::write));
}


static INPUT_PORTS_START( vsmjtria )
	// TODO: deduplicate all inputs here
	// either use a C-style macro or device-ify
	PORT_START("P1_COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNKNOWN ) // stored at A38F along with bit 0 - seems to be vestigial, bit 2 tested at 02D1, but result discarded
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // masked out after being read

	PORT_START("P2_COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNKNOWN ) // stored at A38F along with bit 0 - seems to be vestigial, bit 2 tested at 02D1, but result discarded
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // masked out after being read

	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Single Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_I )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1) PORT_NAME("P1 VS Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Single Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_K )  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("P2 VS Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")  // DSW1 and 2 are for main CPU
	PORT_DIPNAME( 0x01, 0x01, "Test Mode (P1)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Hide Girls" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW3 and 4 are for sub CPU
	PORT_DIPNAME( 0x01, 0x01, "Test Mode (P2)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Hide Girls" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// single slider switch, not read by CPU
	// grounds NVRAM contents, simulated in machine_reset fn
	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "Clear NVRAM at boot" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static GFXDECODE_START( gfx_vsmjtria_main )
	GFXDECODE_ENTRY( "maingfx", 0, charlayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_vsmjtria_sub )
	GFXDECODE_ENTRY( "subgfx", 0, charlayout, 0, 32 )
GFXDECODE_END

void vsmjtria_state::machine_start()
{
	m_keyboard_cmd[0] = m_keyboard_cmd[1] = 0;

	save_item(NAME(m_keyboard_cmd));
}

void vsmjtria_state::machine_reset()
{
	if (m_sw2->read() & 1)
	{
		// Note: there's no direct setter in nvram_device that directly flushes contents for this case scenario
		for (auto &nvram : m_nvram_data)
			std::fill_n(&nvram[0], nvram.length(), 0);

		logerror("machine_reset: flush NVRAM contents\n");
	}
}

void vsmjtria_state::vsmjtria(machine_config &config)
{
	config.set_perfect_quantum("cpu0"); // crude way to make comms work - does it have FIFOs rather than latches?

	// Both CPUs are LH0080A -> Z80A, nominally 4 MHz
	// Clock also controls sound tempo, which at /4 sounds too fast already.
	Z80(config, m_cpu[0], 20_MHz_XTAL / 5); // divider guessed
	m_cpu[0]->set_addrmap(AS_PROGRAM, &vsmjtria_state::main_prg_map);
	m_cpu[0]->set_addrmap(AS_IO, &vsmjtria_state::main_io_map);
	m_cpu[0]->set_vblank_int("lscreen", FUNC(vsmjtria_state::irq0_line_hold));

	Z80(config, m_cpu[1], 20_MHz_XTAL / 5); // divider guessed
	m_cpu[1]->set_addrmap(AS_PROGRAM, &vsmjtria_state::sub_prg_map);
	m_cpu[1]->set_addrmap(AS_IO, &vsmjtria_state::sub_io_map);
	m_cpu[1]->set_vblank_int("rscreen", FUNC(vsmjtria_state::irq0_line_hold));

	NVRAM(config, m_nvram[0], nvram_device::DEFAULT_ALL_0);
	NVRAM(config, m_nvram[1], nvram_device::DEFAULT_ALL_0);

	GENERIC_LATCH_8(config, "latch0");
	GENERIC_LATCH_8(config, "latch1");

	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.in_pa_callback().set([this]() { logerror("%s ppi0 pa read\n", machine().describe_context()); return 0; });
	ppi0.in_pb_callback().set(FUNC(vsmjtria_state::keyboard_r<0>));
	ppi0.in_pc_callback().set_ioport("P1_COIN");
	ppi0.out_pa_callback().set(FUNC(vsmjtria_state::keyboard_w<0>));
	ppi0.out_pb_callback().set([this](uint8_t data) { logerror("%s ppi0 pb write: %02X\n", machine().describe_context(), data); });
	ppi0.out_pc_callback().set([this](uint8_t data) { logerror("%s ppi0 pc write: %02X\n", machine().describe_context(), data); });

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set([this]() { logerror("%s ppi1 pa read\n", machine().describe_context()); return 0; });
	ppi1.in_pb_callback().set(FUNC(vsmjtria_state::keyboard_r<1>));
	ppi1.in_pc_callback().set_ioport("P2_COIN");
	ppi1.out_pa_callback().set(FUNC(vsmjtria_state::keyboard_w<1>));
	ppi1.out_pb_callback().set([this](uint8_t data) { logerror("%s ppi1 pb write: %02X\n", machine().describe_context(), data); });
	ppi1.out_pc_callback().set([this](uint8_t data) { logerror("%s ppi1 pc write: %02X\n", machine().describe_context(), data); });

	PALETTE(config, "palette0", palette_device::RGB_444_PROMS, "mainproms", 256);
	PALETTE(config, "palette1", palette_device::RGB_444_PROMS, "subproms", 256);

	GFXDECODE(config, m_gfxdecode[0], "palette0", gfx_vsmjtria_main);
	GFXDECODE(config, m_gfxdecode[1], "palette1", gfx_vsmjtria_sub);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(64*8, 32*8);
	lscreen.set_visarea(4*8, 60*8-1, 2*8, 30*8-1);
	lscreen.set_screen_update(FUNC(vsmjtria_state::screen_update<0>));
	lscreen.set_palette("palette0");

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(64*8, 32*8);
	rscreen.set_visarea(4*8, 60*8-1, 2*8, 30*8-1);
	rscreen.set_screen_update(FUNC(vsmjtria_state::screen_update<1>));
	rscreen.set_palette("palette1");

	SPEAKER(config, "speaker0").front_left(); // P1 side

	ay8910_device &ay0(AY8910(config, "ay0", 20_MHz_XTAL / 16)); // divider guessed
	ay0.port_a_read_callback().set_ioport("DSW2");
	ay0.port_b_read_callback().set_ioport("DSW1");
	ay0.add_route(ALL_OUTPUTS, "speaker0", 0.33);

	SPEAKER(config, "speaker1").front_right(); // P2 side

	ay8910_device &ay1(AY8910(config, "ay1", 20_MHz_XTAL / 16)); // divider guessed
	ay1.port_a_read_callback().set_ioport("DSW4");
	ay1.port_b_read_callback().set_ioport("DSW3");
	ay1.add_route(ALL_OUTPUTS, "speaker1", 0.33);
}

ROM_START( vsmjtria )
	ROM_REGION( 0x10000, "cpu0", 0 ) // only 5.n8 and 6.n12 differ, and only for 2 bytes
	ROM_LOAD( "dyna 5.n8",  0x0000, 0x8000, CRC(526e3e6e) SHA1(db7b116f285761b040bb3b5cb3e00f5bf78606a9) )
	ROM_LOAD( "dyna 41.n7", 0x8000, 0x8000, CRC(0c150add) SHA1(856de25fe43b656ea7c3066f838de6581df896c9) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "dyna 6.n12",  0x0000, 0x8000, CRC(f1b09d7a) SHA1(89b653ce332f3b1c1c9f7b82e3636dfb329ebd93) )
	ROM_LOAD( "dyna 41.n13", 0x8000, 0x8000, CRC(0c150add) SHA1(856de25fe43b656ea7c3066f838de6581df896c9) )

	ROM_REGION( 0x20000, "maingfx", 0 ) // main and sub GFX are identical
	ROM_LOAD( "dyna 1.h4", 0x00000, 0x08000, CRC(3a16a4f8) SHA1(3a3cb6559be96a3070f53d192cc240e3619a749a) )
	ROM_LOAD( "dyna 2.h5", 0x08000, 0x08000, CRC(a6c80d40) SHA1(f28efec537f7d5825d9336569fcbec38928eaba2) )
	ROM_LOAD( "dyna 3.h7", 0x10000, 0x08000, CRC(30a22f1b) SHA1(52aea2dde753556df0a8d96b5a297ddd34d917cf) )
	// 0x18000-0x1ffff empty space filled by the init function

	ROM_REGION( 0x20000, "subgfx", 0 )
	ROM_LOAD( "dyna 1.h16", 0x00000, 0x08000, CRC(3a16a4f8) SHA1(3a3cb6559be96a3070f53d192cc240e3619a749a) )
	ROM_LOAD( "dyna 2.h15", 0x08000, 0x08000, CRC(a6c80d40) SHA1(f28efec537f7d5825d9336569fcbec38928eaba2) )
	ROM_LOAD( "dyna 3.h13", 0x10000, 0x08000, CRC(30a22f1b) SHA1(52aea2dde753556df0a8d96b5a297ddd34d917cf) )
	// 0x18000-0x1ffff empty space filled by the init function

	ROM_REGION( 0x300, "mainproms", 0 ) // main and sub PROMs are identical
	ROM_LOAD( "82s129a_bkr.e5", 0x000, 0x100, CRC(7a6f5c49) SHA1(d0699e6c2c5760b62aba563e039f84f811d1c6a7) )
	ROM_LOAD( "82s129a_bkg.e4", 0x100, 0x100, CRC(bafdf4e2) SHA1(ddaf98c6d04c9b199f0006ef3f8758ca6b0385ae) )
	ROM_LOAD( "82s129a_bkb.e6", 0x200, 0x100, CRC(6dc54155) SHA1(c95d2b1ada45bc8b439b3562ebef451e3e27cb42) )

	ROM_REGION( 0x300, "subproms", 0 )
	ROM_LOAD( "82s129a_bkr.e15", 0x000, 0x100, CRC(7a6f5c49) SHA1(d0699e6c2c5760b62aba563e039f84f811d1c6a7) )
	ROM_LOAD( "82s129a_bkg.e16", 0x100, 0x100, CRC(bafdf4e2) SHA1(ddaf98c6d04c9b199f0006ef3f8758ca6b0385ae) )
	ROM_LOAD( "82s129a_bkb.e14", 0x200, 0x100, CRC(6dc54155) SHA1(c95d2b1ada45bc8b439b3562ebef451e3e27cb42) )

	ROM_REGION( 0x200, "mainplds", 0 ) // main and sub PLDs are identical, read as PLS153
	ROM_LOAD( "ck2605_tp1.d1", 0x000, 0x0eb, CRC(98da4486) SHA1(a5a300978e3e5ccb68e1768a50f3e9d9459ee81d) )
	ROM_LOAD( "ck2605_tp2.l3", 0x100, 0x0eb, CRC(89b69b50) SHA1(c5d95488540e10092f9935427a6f680959c413d9) )

	ROM_REGION( 0x200, "subplds", 0 ) // main and sub PLDs are identical, read as PLS153
	ROM_LOAD( "ck2605_tp1.d19", 0x000, 0x0eb, CRC(98da4486) SHA1(a5a300978e3e5ccb68e1768a50f3e9d9459ee81d) )
	ROM_LOAD( "ck2605_tp2.l17", 0x100, 0x0eb, CRC(89b69b50) SHA1(c5d95488540e10092f9935427a6f680959c413d9) )
ROM_END


static void unpack_high_bit(int size, uint8_t *rom)
{
	// unpack the high bit of gfx
	for (int b = size - 0x4000; b >= 0; b -= 0x4000)
	{
		if (b) memcpy(rom + b, rom + b/2, 0x2000);

		for (int a = 0; a < 0x2000;a++)
		{
			rom[a + b + 0x2000] = rom[a + b] >> 4;
		}
	}
}
void vsmjtria_state::init_vsmjtria()
{
	unpack_high_bit(memregion("maingfx")->bytes() / 2, memregion("maingfx")->base() + 0x10000);
	unpack_high_bit(memregion("subgfx")->bytes() / 2, memregion("subgfx")->base() + 0x10000);
}

} // Anonymous namespace


GAME( 1986, vsmjtria, 0, vsmjtria, vsmjtria, vsmjtria_state, init_vsmjtria, ROT0, "Dyna", "VS Mahjong Triangle", MACHINE_SUPPORTS_SAVE ) // puts Home Data in RAM?
