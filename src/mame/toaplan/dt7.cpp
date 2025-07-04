// license:BSD-3-Clause
// copyright-holders:David Haywood

/* This is derived from toaplan2.cpp, but there are enough hardware
   differences to keep it separate

   TODO:
    - verify audio CPU opcodes (see toaplan_v25_tables.h)
    - correctly hook up interrupts (especially V25)
    - correctly hook up inputs (there's a steering wheel test? is the game switchable)
    - serial comms (needs support in V25 core?) for linked units
    - verify frequencies on chips
    - verify alt titles, some regions have 'Car Fighting' as a subtitle, region comes from EEPROM?
    - verify text layer palettes
    - service mode doesn't display properly
    - currently only coins up with service button
    - sound dies after one stage?
*/


#include "emu.h"

#include "toaplipt.h"
#include "toaplan_v25_tables.h"
#include "gp9001.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class dt7_state : public driver_device
{
public:
	dt7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001_%u", 0U)
		, m_oki(*this, "oki%u", 1U)
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode%u", 0U)
		, m_screen(*this, "screen%u", 0U)
		, m_palette(*this, "palette%u", 0U)
		, m_shared_ram(*this, "shared_ram")
		, m_tx_videoram(*this, "tx_videoram")
		, m_lineram(*this, "lineram")
		, m_eepromport(*this, "EEPROM")
		, m_sysport(*this, "SYS")
		, m_p1port(*this, "IN1")
		, m_p2port(*this, "IN2")
	{ }

public:
	void dt7(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void dt7_reset(int state);

	u32 screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tx_dt7_tile_info);

	void draw_tx_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int table);

	void dt7_68k_0_mem(address_map &map);
	void dt7_68k_1_mem(address_map &map);
	void dt7_v25_mem(address_map &map);
	void dt7_shared_mem(address_map &map);

	void dt7_irq(int state);
	void dt7_sndreset_coin_w(offs_t offset, u16 data, u16 mem_mask);

	u8 unmapped_v25_io1_r();
	u8 unmapped_v25_io2_r();

	u8 read_port_t();
	u8 read_port_2();
	void write_port_2(u8 data);

	u8 eeprom_r();
	void eeprom_w(u8 data);


	u8 dt7_shared_ram_hack_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);
	void shared_ram_audio_w(offs_t offset, u8 data);

	void screen_vblank(int state);

	u8 m_ioport_state;

	tilemap_t *m_tx_tilemap[2];    /* Tilemap for extra-text-layer */

	bitmap_ind8 m_custom_priority_bitmap;

	required_device<m68000_base_device> m_maincpu;
	required_device<m68000_base_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<gp9001vdp_device, 2> m_vdp;
	required_device_array<okim6295_device, 2> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_device_array<screen_device, 2> m_screen;
	required_device_array<palette_device, 2> m_palette;
	required_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	required_shared_ptr<u16> m_tx_videoram;
	required_shared_ptr<u16> m_lineram;
	required_ioport m_eepromport;
	required_ioport m_sysport;
	required_ioport m_p1port;
	required_ioport m_p2port;
};


void dt7_state::dt7_irq(int state)
{
	// only the first VDP gets IRQ acked, so does it trigger both CPUs, or does the main CPU then trigger sub?
	m_maincpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);

	// this reads the inputs (again what is the source?)
	// the audio CPU also has a 'serial' interrupt populated?
	// and the boards can be linked together

	// triggering this prevents our other timer? interrupt from working however?
	//m_audiocpu->set_input_line(NEC_INPUT_LINE_INTP0, state ? ASSERT_LINE : CLEAR_LINE);
}


// this is conditional on the unknown type of branch (see #define G_B0 in the table0
u8 dt7_state::read_port_t()
{
	logerror("%s: read port t\n", machine().describe_context());
	return machine().rand();
}

u8 dt7_state::eeprom_r()
{
	logerror("%s: eeprom_r\n", machine().describe_context());
	// if you allow eeprom hookup at the moment (remove the ram hack reads)
	// the game will init it the first time but then 2nd boot will be upside
	// down as Japan region, and hang after the region warning
	//return 0xff;
	return m_eepromport->read();
}

void dt7_state::eeprom_w(u8 data)
{
	logerror("%s: eeprom_w %02x\n", machine().describe_context(), data);
	m_eepromport->write(data);
}

u8 dt7_state::read_port_2()
{
	logerror("%s: read port 2\n", machine().describe_context());
	return m_ioport_state;
}

// it seems to attempt to read inputs (including the tilt switch?) here on startup
// strangely all the EEPROM access code (which is otherwise very similar to FixEight
// also has accesses to this port added, maybe something is sitting in the middle?
void dt7_state::write_port_2(u8 data)
{
	if ((m_ioport_state & 0x01) != (data & 0x01))
	{
		if (data & 0x01)
			logerror("%s: bit 0x01 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x01 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x02) != (data & 0x02))
	{
		if (data & 0x02)
			logerror("%s: bit 0x02 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x02 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x04) != (data & 0x04))
	{
		if (data & 0x04)
			logerror("%s: bit 0x04 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x04 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x08) != (data & 0x08))
	{
		if (data & 0x08)
			logerror("%s: bit 0x08 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x08 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x10) != (data & 0x10))
	{
		if (data & 0x10)
			logerror("%s: bit 0x10 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x10 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x20) != (data & 0x20))
	{
		if (data & 0x20)
			logerror("%s: bit 0x20 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x20 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x40) != (data & 0x40))
	{
		if (data & 0x40)
			logerror("%s: bit 0x40 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x40 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x80) != (data & 0x80))
	{
		if (data & 0x80)
			logerror("%s: bit 0x80 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x80 high to low\n", machine().describe_context());
	}

	m_ioport_state = data;
}


// hacks because the sound CPU isn't running properly
u8 dt7_state::dt7_shared_ram_hack_r(offs_t offset)
{
	u16 ret = m_shared_ram[offset];

	int pc = m_maincpu->pc();

	if (pc == 0x7d84) { return 0xff; } // status?

	u32 addr = (offset * 2) + 0x610000;

	if (addr == 0x061f00c) { return m_sysport->read(); }
	if (addr == 0x061d000) { return 0x00; } // settings (from EEPROM?) including flipscreen
	if (addr == 0x061d002) { return 0x00; } // settings (from EEPROM?) dipswitch?
	if (addr == 0x061d004) { return 0x00; } // settings (from EEPROM?) region
	if (addr == 0x061f004) { return m_p1port->read(); } // P1 inputs
	if (addr == 0x061f006) { return m_p2port->read(); } // P2 inputs
	//if (addr == 0x061f00e) { return machine().rand(); } // P2 coin / start

	logerror("%08x: dt7_shared_ram_hack_r address %08x ret %02x\n", pc, addr, ret);

	return ret;
}

void dt7_state::shared_ram_w(offs_t offset, u8 data)
{
	m_shared_ram[offset] = data;
}

void dt7_state::shared_ram_audio_w(offs_t offset, u8 data)
{
	// just a helper function to try and debug the sound CPU a bit more easily
	//int pc = m_audiocpu->pc();
	//if (offset == 0xf004 / 2)
	//  logerror("%08x: shared_ram_audio_w address %08x data %02x\n", pc, offset, data);
	shared_ram_w(offset, data);
}

void dt7_state::dt7_sndreset_coin_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x8000) ? CLEAR_LINE : ASSERT_LINE);
	logerror("%s: dt7_sndreset_coin_w %04x %04x\n", machine().describe_context(), data, mem_mask);
	// coin counters in lower byte?
}


void dt7_state::dt7_68k_0_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();

	map(0x380000, 0x380001).r(m_vdp[0], FUNC(gp9001vdp_device::vdpcount_r));

	map(0x200000, 0x200fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette0");
	map(0x280000, 0x280fff).ram().w(m_palette[1], FUNC(palette_device::write16)).share("palette1");

	map(0x300000, 0x300001).w(FUNC(dt7_state::dt7_sndreset_coin_w));

	map(0x400000, 0x40000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x480000, 0x48000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));

	dt7_shared_mem(map);

	map(0x610000, 0x61ffff).rw(FUNC(dt7_state::dt7_shared_ram_hack_r), FUNC(dt7_state::shared_ram_w)).umask16(0x00ff);
//  map(0x620000, 0x62ffff).rw(FUNC(dt7_state::dt7_shared_ram_hack_r), FUNC(dt7_state::shared_ram_w)).umask16(0x00ff);
}

void dt7_state::dt7_shared_mem(address_map &map)
{
	map(0x500000, 0x50ffff).ram().share("shared_ram2");
	// is this really in the middle of shared RAM, or is there a DMA to get it out?
	map(0x509000, 0x50afff).ram().w(FUNC(dt7_state::tx_videoram_dt7_w)).share("tx_videoram");
	map(0x50f000, 0x50ffff).ram().share("lineram");

}
void dt7_state::dt7_68k_1_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().mirror(0x080000); // mirror needed or road doesn't draw
	dt7_shared_mem(map);
}


u8 dt7_state::unmapped_v25_io1_r()
{
	logerror("%s: 0x58008 unknown read\n", machine().describe_context());
	return machine().rand();
}

u8 dt7_state::unmapped_v25_io2_r()
{
	logerror("%s: 0x5800a unknown read\n", machine().describe_context());
	return machine().rand();
}

void dt7_state::machine_start()
{
	save_item(NAME(m_ioport_state));
}

void dt7_state::machine_reset()
{
	m_ioport_state = 0x00;
}

void dt7_state::dt7_v25_mem(address_map &map)
{
	// exact mirroring unknown, don't cover up where the inputs/sound maps
	// is it meant to mirror in all these locations, or is there a different issue in play?
	map(0x00000, 0x07fff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0x20000, 0x27fff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0x28000, 0x2ffff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0x60000, 0x67fff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0x68000, 0x6ffff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0x70000, 0x77fff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");
	map(0xf8000, 0xfffff).ram().w(FUNC(dt7_state::shared_ram_audio_w)).share("shared_ram");

	map(0x58000, 0x58001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58002, 0x58002).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58004, 0x58005).rw("ymsnd2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58006, 0x58006).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58008, 0x58008).r(FUNC(dt7_state::unmapped_v25_io1_r));
	map(0x5800a, 0x5800a).r(FUNC(dt7_state::unmapped_v25_io2_r));
}

void dt7_state::dt7_reset(int state)
{
	//if (m_audiocpu != nullptr)
	//  m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// needed when exiting service menu
	m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

static GFXDECODE_START( gfx_textrom_double )
	GFXDECODE_ENTRY( "text_0", 0, gfx_8x8x4_packed_msb, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_textrom_double_1 )
	GFXDECODE_ENTRY( "text_1", 0, gfx_8x8x4_packed_msb, 0, 128 )
GFXDECODE_END

void dt7_state::dt7(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dt7_state::dt7_68k_0_mem);
	m_maincpu->reset_cb().set(FUNC(dt7_state::dt7_reset));

	M68000(config, m_subcpu, 32_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &dt7_state::dt7_68k_1_mem);

	v25_device &audiocpu(V25(config, m_audiocpu, 32_MHz_XTAL/2));
	audiocpu.set_addrmap(AS_PROGRAM, &dt7_state::dt7_v25_mem);
	audiocpu.set_decryption_table(toaplan_v25_tables::dt7_decryption_table);
	audiocpu.pt_in_cb().set(FUNC(dt7_state::read_port_t));
	audiocpu.p2_in_cb().set(FUNC(dt7_state::read_port_2));
	audiocpu.p2_out_cb().set(FUNC(dt7_state::write_port_2));
	audiocpu.p1_in_cb().set(FUNC(dt7_state::eeprom_r));
	audiocpu.p1_out_cb().set(FUNC(dt7_state::eeprom_w));

	// eeprom type confirmed, and gets inited after first boot, but then game won't boot again?
	EEPROM_93C66_16BIT(config, m_eeprom);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[0]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[0]->set_screen_update(FUNC(dt7_state::screen_update_dt7_1));
	m_screen[0]->screen_vblank().set(FUNC(dt7_state::screen_vblank));
	m_screen[0]->set_palette(m_palette[0]);

	SCREEN(config, m_screen[1], SCREEN_TYPE_RASTER);
	m_screen[1]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[1]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[1]->set_screen_update(FUNC(dt7_state::screen_update_dt7_2));
	//m_screen[1]->screen_vblank().set(FUNC(dt7_state::screen_vblank));
	m_screen[1]->set_palette(m_palette[1]);

	PALETTE(config, m_palette[0]).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	PALETTE(config, m_palette[1]).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette[0]);
	m_vdp[0]->vint_out_cb().set(FUNC(dt7_state::dt7_irq));
	m_vdp[0]->set_screen(m_screen[0]);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette[1]);
	m_vdp[1]->set_screen(m_screen[1]);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_textrom_double);

	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_textrom_double_1);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "speaker", 0.5, 0);

	OKIM6295(config, m_oki[0], 27_MHz_XTAL / 24, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);

	YM2151(config, "ymsnd2", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	OKIM6295(config, m_oki[1], 27_MHz_XTAL/24, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


static INPUT_PORTS_START( dt7 )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

TILE_GET_INFO_MEMBER(dt7_state::get_tx_dt7_tile_info)
{
	const u16 attrib = m_tx_videoram[tile_index];
	const u32 tile_number = attrib & 0x3ff;

	u32 color = (attrib & 0xf800) >> 11;

	color |= 0x60;

	tileinfo.set(0,
			tile_number,
			color,
			0);
}

void dt7_state::video_start()
{
	/* our current VDP implementation needs this bitmap to work with */
	m_screen[0]->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp[0]->custom_priority_bitmap = &m_custom_priority_bitmap;
	m_vdp[1]->custom_priority_bitmap = &m_custom_priority_bitmap;

	// a different part of this tilemap is displayed on each screen
	// each screen has a different palette and uses a ROM in a different location on the PCB
	m_tx_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(dt7_state::get_tx_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(dt7_state::get_tx_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tx_tilemap[0]->set_transparent_pen(0);
	m_tx_tilemap[1]->set_transparent_pen(0);
}

void dt7_state::tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap[0]->mark_tile_dirty(offset);
	m_tx_tilemap[1]->mark_tile_dirty(offset);
}


void dt7_state::draw_tx_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int table)
{
	// there seems to be RAM for another tx tilemap
	// but there were 2 empty sockets / sockets with blank tx ROMs, so
	// it's likely only one of them is used

	// see comments in other toaplan drivers, this is likely per-line
	int flipx = m_lineram[(table * 2)] & 0x8000;
	m_tx_tilemap[table / 2]->set_flip(flipx ? 0 : TILEMAP_FLIPX);

	rectangle clip = cliprect;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;

		u16 scroll1 = m_lineram[((y * 8) + (table * 2) + 0) & 0x7ff]; // lineselect
		//u16 scroll2 = m_lineram[((y * 8) + (table * 2) + 1) & 0x7ff]; // xscroll

		scroll1 &= 0x7fff; // 0x8000 is per-line flip
		scroll1 -= 0x0900; // are all these scroll bits?

		m_tx_tilemap[table / 2]->set_scrolly(0, scroll1 - y);
		m_tx_tilemap[table / 2]->draw(screen, bitmap, clip, 0);
	}
}

u32 dt7_state::screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[0]->render_vdp(bitmap, cliprect);
	draw_tx_tilemap(screen, bitmap, cliprect, 0);
	return 0;
}


u32 dt7_state::screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[1]->render_vdp(bitmap, cliprect);
	draw_tx_tilemap(screen, bitmap, cliprect, 2);
	return 0;
}

void dt7_state::screen_vblank(int state)
{
	m_vdp[0]->screen_eof();
	m_vdp[1]->screen_eof();
}



ROM_START( dt7 )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "main.11", 0x000000, 0x080000, CRC(01646c22) SHA1(4b87f00dc99e1206b3b9eaee425fc05e1a033bee) )

	ROM_REGION( 0x080000, "subcpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "2.21", 0x000000, 0x080000, CRC(a08e25ed) SHA1(db10c64ce305477442b35e7624052aae9fb6e412) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */
	/* Note, same markings as other games found in toaplan2.cpp, but table is different! */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "3a.49", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) )
	ROM_LOAD( "3b.50", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) )
	ROM_LOAD( "3c.51", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) )
	ROM_LOAD( "3d.52", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) )
	ROM_LOAD( "4a.30", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) )
	ROM_LOAD( "4b.31", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) )
	ROM_LOAD( "4c.32", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) )
	ROM_LOAD( "4d.33", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) )

	ROM_REGION( 0x08000, "text_0", 0 )
	ROM_LOAD( "7text.115", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) )
	// some dumps contain an empty '1M' ROM located next to each 'text' ROM on the PCB?

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD( "3a.68", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) )
	ROM_LOAD( "3b.69", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) )
	ROM_LOAD( "3c.70", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) )
	ROM_LOAD( "3d.71", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) )
	ROM_LOAD( "4a.87", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) )
	ROM_LOAD( "4b.88", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) )
	ROM_LOAD( "4c.89", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) )
	ROM_LOAD( "4d.90", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) )

	ROM_REGION( 0x08000, "text_1", 0 )
	ROM_LOAD( "7text.152", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) )
	// some dumps contain an empty '1M' ROM located next to each 'text' ROM on the PCB?

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "7adpcm.37", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) )

	ROM_REGION( 0x40000, "oki2", 0 )     /* ADPCM Samples */
	ROM_LOAD( "7adpcm.43", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) )
ROM_END

} // anonymous namespace

// The region comes from the EEPROM? so will need clones like FixEight
GAME( 1993, dt7,         0,        dt7,          dt7,        dt7_state,empty_init, ROT270, "Toaplan",         "DT7 (prototype)",              MACHINE_NOT_WORKING ) // flyer shows "Survival Battle Dynamic Trial 7"
