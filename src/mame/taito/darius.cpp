// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
// thanks-to:Richard Bush
/*******************************************************************************

Darius    (c) Taito 1986
======

David Graves, Jarek Burczynski

Sound panning and other enhancements: Hiromitsu Shioya

Sources:        MAME Rastan driver
            Raine source - invaluable for this driver -
            many thanks to Richard Bush and the Raine Team.

                *****

Rom Versions
------------

Darius appears to be a revision of Dariusj (as well as being
for a different sales area). It has continue facilities, missing
in Dariusj, and extra sprites which are used when continuing.
It also has 2 extra program roms.


Use of PC080SN
--------------

This game uses 3 x PC080SN for generating tilemaps. They must be
mapped somehow to a single memory block and set of scroll registers.
There is an additional text tilemap on top of this, to allow for
both background planes scrolling. This need is presumably what led
to the TC0100SCN tilemap chip, debuted in Ninja Warriors (c)1987.
(The TC0100SCN includes a separate text layer.)


ADPCM Z80
---------

This writes the rom area whenever an interrupt occurs. It has no ram
therefore no stack to store registers or return addresses: so the
standard Z80 writes to the stack become irrelevant.


Dumpers Notes
=============

Darius (Old JPN Ver.)
(c)1986 Taito

-----------------------
Sound Board
K1100228A
CPU     :Z80 x2
Sound   :YM2203C x2
OSC     :8.000MHz
-----------------------
A96_56.18
A96_57.33

-----------------------
M4300067A
K1100226A
CPU     :MC68000P8 x2
OSC     :16000.00KHz
-----------------------
A96_28.152
A96_29.185
A96_30.154
A96_31.187

A96_32.157
A96_33.190
A96_34.158
A96_35.191

A96_36.175
A96_37.196
A96_38.176
A96_39.197
A96_40.177
A96_41.198
A96_42.178
A96_43.199
A96_44.179
A96_45.200
A96_46.180
A96_47.201

-----------------------
K1100227A
OSC     :26686.00KHz
Other   :PC080SN x3
-----------------------
A96_48.103
A96_48.24
A96_48.63
A96_49.104
A96_49.25
A96_49.64
A96_50.105
A96_50.26
A96_50.65
A96_51.131
A96_51.47
A96_51.86
A96_52.132
A96_52.48
A96_52.87
A96_53.133
A96_53.49
A96_53.88

A96_54.142
A96_55.143

A96-24.163
A96-25.164
A96-26.165


TODO
====

When you add a coin there is temporary volume distortion of other
sounds.

*******************************************************************************/

#include "emu.h"
#include "taitosnd.h"

#include "pc080sn.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/flt_vol.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "darius.lh"


namespace {

#define VOL_MAX    (3 * 2 + 2)
#define PAN_MAX    (2 + 2 + 1)   /* FM 2port + PSG 2port + DA 1port */

class darius_state : public driver_device
{
public:
	darius_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_ram(*this, "fg_ram"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_cpub(*this, "cpub"),
		m_adpcm(*this, "adpcm"),
		m_pc080sn(*this, "pc080sn"),
		m_filter_l{{*this, "filter0.%ul", 0U}, {*this, "filter1.%ul", 0U}},
		m_filter_r{{*this, "filter0.%ur", 0U}, {*this, "filter1.%ur", 0U}},
		m_msm5205_l(*this, "msm5205.l"),
		m_msm5205_r(*this, "msm5205.r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void darius(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_fg_ram;
	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t *m_fg_tilemap = nullptr;

	/* misc */
	u16 m_cpua_ctrl = 0;
	u16 m_coin_word = 0;
	u8 m_adpcm_command = 0;
	u8 m_nmi_enable = 0;
	u32 m_def_vol[0x10]{};
	u8 m_vol[VOL_MAX]{};
	u8 m_pan[PAN_MAX]{};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<cpu_device> m_cpub;
	required_device<cpu_device> m_adpcm;
	required_device<pc080sn_device> m_pc080sn;

	required_device_array<filter_volume_device, 4> m_filter_l[2];
	required_device_array<filter_volume_device, 4> m_filter_r[2];
	required_device<filter_volume_device> m_msm5205_l;
	required_device<filter_volume_device> m_msm5205_r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void cpua_ctrl_w(u16 data);
	u16 coin_r();
	void coin_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void adpcm_command_w(u8 data);
	void fm0_pan_w(u8 data);
	void fm1_pan_w(u8 data);
	void psg0_pan_w(u8 data);
	void psg1_pan_w(u8 data);
	void da_pan_w(u8 data);
	u8 adpcm_command_r();
	u8 readport2();
	u8 readport3();
	void adpcm_nmi_disable(u8 data);
	void adpcm_nmi_enable(u8 data);
	void fg_layer_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write_portA0(u8 data);
	void write_portA1(u8 data);
	void write_portB0(u8 data);
	void write_portB1(u8 data);
	void adpcm_data_w(u8 data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 0); }
	u32 screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 1); }
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8 * 2); }
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs);
	u32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs);
	void parse_control(); // assumes Z80 sandwiched between 68Ks
	void update_fm0();
	void update_fm1();
	void update_psg0(int port);
	void update_psg1(int port);
	void update_da();
	void adpcm_int(int state);
	void darius_cpub_map(address_map &map) ATTR_COLD;
	void darius_map(address_map &map) ATTR_COLD;
	void darius_sound2_io_map(address_map &map) ATTR_COLD;
	void darius_sound2_map(address_map &map) ATTR_COLD;
	void darius_sound_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************
        VIDEO HARDWARE
*******************************************************************************/

TILE_GET_INFO_MEMBER(darius_state::get_fg_tile_info)
{
	u16 code = (m_fg_ram[tile_index + 0x2000] & 0x7ff);
	u16 attr = m_fg_ram[tile_index];

	tileinfo.set(1,
			code,
			(attr & 0x7f),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

void darius_state::video_start()
{
	m_gfxdecode->gfx(1)->set_granularity(16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darius_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_fg_tilemap->set_transparent_pen(0);
}

void darius_state::fg_layer_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_ram[offset]);
	if (offset < 0x4000)
		m_fg_tilemap->mark_tile_dirty((offset & 0x1fff));
}

/***************************************************************************/

void darius_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs)
{
	static const u32 primask[2] =
	{
		GFX_PMASK_2, // draw sprites with priority 0 which are under the mid layer
		0  // draw sprites with priority 1 which are over the mid layer
	};

	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		const u32 code = m_spriteram[offs + 2] & 0x1fff;

		if (code)
		{
			u16 data = m_spriteram[offs];
			const int sy = (256 - data) & 0x1ff;

			data = m_spriteram[offs + 1];
			const int sx = data & 0x3ff;

			data = m_spriteram[offs + 2];
			const bool flipx = ((data & 0x4000) >> 14);
			const bool flipy = ((data & 0x8000) >> 15);

			data = m_spriteram[offs + 3];
			const int priority = (data & 0x80) >> 7;  // 0 = low
			const u32 color = (data & 0x7f);

			int curx = sx - x_offs;
			int cury = sy + y_offs;

			if (curx > 900) curx -= 1024;
			if (cury > 400) cury -= 512;

			m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code, color,
					flipx, flipy,
					curx, cury,
					screen.priority(), primask[priority], 0);
		}
	}
}


u32 darius_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs)
{
	screen.priority().fill(0, cliprect);
	m_pc080sn->tilemap_update();

	// draw bottom layer(always active)
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1, -xoffs, 0);

	// draw middle layer
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 1, 0, 2, -xoffs, 0);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen, bitmap, cliprect, xoffs, -8);

	/* top(text) layer is in fixed position */
	m_fg_tilemap->set_scrollx(0, 0 + xoffs);
	m_fg_tilemap->set_scrolly(0, -8);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*******************************************************************************
        MISC. CONTROL
*******************************************************************************/

void darius_state::parse_control()   /* assumes Z80 sandwiched between 68Ks */
{
	/* bit 0 enables cpu B */
	/* however this fails when recovering from a save state
	   if cpu B is disabled !! */
	m_cpub->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}

void darius_state::cpua_ctrl_w(u16 data)
{
	if ((data & 0xff00) && ((data & 0xff) == 0))
		data = data >> 8;

	m_cpua_ctrl = data;

	parse_control();

	logerror("CPU #0 PC %06x: write %04x to cpu control\n", m_maincpu->pc(), data);
}


/*******************************************************************************
        GAME INPUTS
*******************************************************************************/

u16 darius_state::coin_r()
{
	return m_coin_word; /* bits 3&4 coin lockouts, must return zero */
}

void darius_state::coin_w(u16 data)
{
	/* coin control */
	/* bits 7,5,4,0 used on reset */
	/* bit 4 used whenever bg is blanked ? */
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x02);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x04);
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);
	m_coin_word = data;
}


/*******************************************************************************
        MEMORY STRUCTURES
*******************************************************************************/

void darius_state::darius_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x080000, 0x08ffff).ram();                                             /* main RAM */
	map(0x0a0000, 0x0a0001).w(FUNC(darius_state::cpua_ctrl_w));
	map(0x0b0000, 0x0b0001).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0xc00000, 0xc00001).nopr();
	map(0xc00001, 0xc00001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xc00003, 0xc00003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xc00008, 0xc00009).portr("P1");
	map(0xc0000a, 0xc0000b).portr("P2");
	map(0xc0000c, 0xc0000d).portr("SYSTEM");
	map(0xc0000e, 0xc0000f).r(FUNC(darius_state::coin_r));
	map(0xc00010, 0xc00011).portr("DSW");
	map(0xc00050, 0xc00051).noprw(); // unknown, written by both cpus - always 0?
	map(0xc00060, 0xc00061).w(FUNC(darius_state::coin_w));
	map(0xd00000, 0xd0ffff).rw(m_pc080sn, FUNC(pc080sn_device::word_r), FUNC(pc080sn_device::word_w));  /* tilemaps */
	map(0xd20000, 0xd20003).w(m_pc080sn, FUNC(pc080sn_device::yscroll_word_w));
	map(0xd40000, 0xd40003).w(m_pc080sn, FUNC(pc080sn_device::xscroll_word_w));
	map(0xd50000, 0xd50003).w(m_pc080sn, FUNC(pc080sn_device::ctrl_word_w));
	map(0xd80000, 0xd80fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");/* palette */
	map(0xe00100, 0xe00fff).ram().share("spriteram");
	map(0xe01000, 0xe02fff).ram().share("share2");
	map(0xe08000, 0xe0ffff).ram().w(FUNC(darius_state::fg_layer_w)).share("fg_ram");
	map(0xe10000, 0xe10fff).ram();                                             /* ??? */
}

void darius_state::darius_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x04ffff).ram();             /* local RAM */
	map(0xc00050, 0xc00051).noprw(); // unknown, written by both cpus - always 0?
	map(0xd80000, 0xd80fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xe00100, 0xe00fff).ram().share("spriteram");
	map(0xe01000, 0xe02fff).ram().share("share2");
	map(0xe08000, 0xe0ffff).ram().w(FUNC(darius_state::fg_layer_w)).share("fg_ram");
}


/*******************************************************************************
        SOUND
*******************************************************************************/

void darius_state::sound_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 3);
}

void darius_state::adpcm_command_w(u8 data)
{
	m_adpcm_command = data;
	/* logerror("#ADPCM command write =%2x\n",data); */
}

#if 0
void darius_state::display_value(u8 data)
{
	popmessage("d800=%x", data);
}
#endif


/*******************************************************************************
        Sound mixer/pan control
*******************************************************************************/

void darius_state::update_fm0()
{
	const int left  = (        m_pan[0]  * m_vol[6]) >> 8;
	const int right = ((0xff - m_pan[0]) * m_vol[6]) >> 8;

	if (m_filter_l[0][3] != nullptr)
		m_filter_l[0][3]->set_gain(left / 100.0);
	if (m_filter_r[0][3] != nullptr)
		m_filter_r[0][3]->set_gain(right / 100.0); /* FM #0 */
}

void darius_state::update_fm1()
{
	const int left  = (        m_pan[1]  * m_vol[7]) >> 8;
	const int right = ((0xff - m_pan[1]) * m_vol[7]) >> 8;

	if (m_filter_l[1][3] != nullptr)
		m_filter_l[1][3]->set_gain(left / 100.0);
	if (m_filter_r[1][3] != nullptr)
		m_filter_r[1][3]->set_gain(right / 100.0); /* FM #1 */
}

void darius_state::update_psg0(int port)
{
	filter_volume_device *lvol = nullptr, *rvol = nullptr;

	switch (port)
	{
		case 0: lvol = m_filter_l[0][0]; rvol = m_filter_r[0][0]; break;
		case 1: lvol = m_filter_l[0][1]; rvol = m_filter_r[0][1]; break;
		case 2: lvol = m_filter_l[0][2]; rvol = m_filter_r[0][2]; break;
		default: break;
	}

	const int left  = (        m_pan[2]  * m_vol[port]) >> 8;
	const int right = ((0xff - m_pan[2]) * m_vol[port]) >> 8;

	if (lvol != nullptr)
		lvol->set_gain(left / 100.0);
	if (rvol != nullptr)
		rvol->set_gain(right / 100.0);
}

void darius_state::update_psg1(int port)
{
	filter_volume_device *lvol = nullptr, *rvol = nullptr;

	switch (port)
	{
		case 0: lvol = m_filter_l[1][0]; rvol = m_filter_r[1][0]; break;
		case 1: lvol = m_filter_l[1][1]; rvol = m_filter_r[1][1]; break;
		case 2: lvol = m_filter_l[1][2]; rvol = m_filter_r[1][2]; break;
		default: break;
	}

	const int left  = (        m_pan[3]  * m_vol[port + 3]) >> 8;
	const int right = ((0xff - m_pan[3]) * m_vol[port + 3]) >> 8;

	if (lvol != nullptr)
		lvol->set_gain(left / 100.0);
	if (rvol != nullptr)
		rvol->set_gain(right / 100.0);
}

void darius_state::update_da()
{
	const int left  = m_def_vol[(m_pan[4] >> 0) & 0x0f];
	const int right = m_def_vol[(m_pan[4] >> 4) & 0x0f];

	if (m_msm5205_l != nullptr)
		m_msm5205_l->set_gain(left / 100.0);
	if (m_msm5205_r != nullptr)
		m_msm5205_r->set_gain(right / 100.0);
}

void darius_state::fm0_pan_w(u8 data)
{
	m_pan[0] = data & 0xff;  /* data 0x00:right 0xff:left */
	update_fm0();
}

void darius_state::fm1_pan_w(u8 data)
{
	m_pan[1] = data & 0xff;
	update_fm1();
}

void darius_state::psg0_pan_w(u8 data)
{
	m_pan[2] = data & 0xff;
	update_psg0(0);
	update_psg0(1);
	update_psg0(2);
}

void darius_state::psg1_pan_w(u8 data)
{
	m_pan[3] = data & 0xff;
	update_psg1(0);
	update_psg1(1);
	update_psg1(2);
}

void darius_state::da_pan_w(u8 data)
{
	m_pan[4] = data & 0xff;
	update_da();
}

/**** Mixer Control ****/

void darius_state::write_portA0(u8 data)
{
	// volume control FM #0 PSG #0 A
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );
	//popmessage(" A0 %02x A1 %02x B0 %02x B1 %02x", port[0], port[1], port[2], port[3] );

	m_vol[0] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[6] = m_def_vol[(data >> 0) & 0x0f];
	update_fm0();
	update_psg0(0);
}

void darius_state::write_portA1(u8 data)
{
	// volume control FM #1 PSG #1 A
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[3] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[7] = m_def_vol[(data >> 0) & 0x0f];
	update_fm1();
	update_psg1(0);
}

void darius_state::write_portB0(u8 data)
{
	// volume control PSG #0 B/C
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[1] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[2] = m_def_vol[(data >> 0) & 0x0f];
	update_psg0(1);
	update_psg0(2);
}

void darius_state::write_portB1(u8 data)
{
	// volume control PSG #1 B/C
	//popmessage(" pan %02x %02x %02x %02x %02x", m_pan[0], m_pan[1], m_pan[2], m_pan[3], m_pan[4] );

	m_vol[4] = m_def_vol[(data >> 4) & 0x0f];
	m_vol[5] = m_def_vol[(data >> 0) & 0x0f];
	update_psg1(1);
	update_psg1(2);
}


/*******************************************************************************
        Sound memory structures / ADPCM
*******************************************************************************/

void darius_state::darius_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xb000, 0xb000).nopr().w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xb001, 0xb001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xc000, 0xc000).w(FUNC(darius_state::fm0_pan_w));
	map(0xc400, 0xc400).w(FUNC(darius_state::fm1_pan_w));
	map(0xc800, 0xc800).w(FUNC(darius_state::psg0_pan_w));
	map(0xcc00, 0xcc00).w(FUNC(darius_state::psg1_pan_w));
	map(0xd000, 0xd000).w(FUNC(darius_state::da_pan_w));
	map(0xd400, 0xd400).w(FUNC(darius_state::adpcm_command_w));  /* ADPCM command for second Z80 to read from port 0x00 */
//  map(0xd800, 0xd800).w(FUNC(darius_state::(display_value));    /* ??? */
	map(0xdc00, 0xdc00).w(FUNC(darius_state::sound_bankswitch_w));
}

void darius_state::darius_sound2_map(address_map &map)
{
	map(0x0000, 0xffff).rom().nopw();
	/* yes, no RAM */
}


void darius_state::adpcm_int(int state)
{
	if (m_nmi_enable)
		m_adpcm->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

u8 darius_state::adpcm_command_r()
{
	/* logerror("%s read port 0: %02x\n", machine().describe_context(), adpcm_command ); */
	return m_adpcm_command;
}

u8 darius_state::readport2()
{
	return 0;
}

u8 darius_state::readport3()
{
	return 0;
}

void darius_state::adpcm_nmi_disable(u8 data)
{
	m_nmi_enable = 0;
	/* logerror("%s write port 0: NMI DISABLE\n", machine().describe_context(), data ); */
}

void darius_state::adpcm_nmi_enable(u8 data)
{
	m_nmi_enable = 1;
	/* logerror("%s write port 1: NMI ENABLE\n", machine().describe_context() ); */
}

void darius_state::adpcm_data_w(u8 data)
{
	m_msm->data_w(data);
	m_msm->reset_w(!(data & 0x20));    /* my best guess, but it could be output enable as well */
}

void darius_state::darius_sound2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(darius_state::adpcm_command_r), FUNC(darius_state::adpcm_nmi_disable));
	map(0x01, 0x01).w(FUNC(darius_state::adpcm_nmi_enable));
	map(0x02, 0x02).r(FUNC(darius_state::readport2)).w(FUNC(darius_state::adpcm_data_w));  /* readport2 ??? */
	map(0x03, 0x03).r(FUNC(darius_state::readport3)); /* ??? */
}


/*******************************************************************************
        INPUT PORTS, DIPs
*******************************************************************************/

#define TAITO_COINAGE_JAPAN_16 \
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6") \
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) ) \
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8") \
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )


static INPUT_PORTS_START( darius )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Autofire" )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "every 600k" )
	PORT_DIPSETTING(      0x0c00, "600k only" )
	PORT_DIPSETTING(      0x0400, "800k only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dariuse )
	PORT_INCLUDE( darius )

	PORT_MODIFY("DSW")
	TAITO_COINAGE_JAPAN_16
INPUT_PORTS_END

static INPUT_PORTS_START( dariusj ) // No Continue for this version
	PORT_INCLUDE( dariuse )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dariusu ) // US version uses the Japan coinage settings
	PORT_INCLUDE( dariuse )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "Discount" )
	PORT_DIPSETTING(      0x0000, "Same as Start" )
INPUT_PORTS_END


/*******************************************************************************
        GFX DECODING
*******************************************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,8) },       /* pixel bits separated */
	{ STEP8(0,1), STEP8(8*4*8,1) },
	{ STEP8(0,8*4), STEP8(8*4*8*2,8*4) },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout textlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	2,  /* 2 bits per pixel */
	{ STEP2(0,8) },   /* pixel bits separated */
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_darius )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout,           0, 128 )  /* sprites */
	GFXDECODE_ENTRY( "text",    0, textlayout,           0, 128 )  /* top layer scr tiles */
GFXDECODE_END

static GFXDECODE_START( gfx_darius_tmap )
	GFXDECODE_ENTRY( "pc080sn", 0, gfx_8x8x4_packed_msb, 0, 128 )  /* scr tiles */
GFXDECODE_END


/*******************************************************************************
        MACHINE DRIVERS
*******************************************************************************/

void darius_state::device_post_load()
{
	parse_control();
}

void darius_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	for (int i = 0; i < 0x10; i++)
	{
		//logerror( "calc %d = %d\n", i, (int)(100.0f / (float)pow(10.0f, (32.0f - (i * (32.0f / (float)(0xf)))) / 20.0f)) );
		m_def_vol[i] = (int)(100.0f / (float)pow(10.0f, (32.0f - (i * (32.0f / (float)(0xf)))) / 20.0f));
	}

	save_item(NAME(m_cpua_ctrl));
	save_item(NAME(m_coin_word));

	save_item(NAME(m_adpcm_command));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_vol));
	save_item(NAME(m_pan));
}


void darius_state::machine_reset()
{
	m_audiobank->set_entry(0);

	m_cpua_ctrl = 0xff;
	m_coin_word = 0;
	m_adpcm_command = 0;
	m_nmi_enable = 0;

	machine().sound().system_mute(false);  /* mixer enabled */

	for (auto & elem : m_vol)
		elem = 0x00;    /* min volume */

	for (auto & elem : m_pan)
		elem = 0x80;    /* center */

}

void darius_state::darius(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000)/2);  /* 8 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &darius_state::darius_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(darius_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(8'000'000)/2); /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &darius_state::darius_sound_map);

	M68000(config, m_cpub, XTAL(16'000'000)/2); /* 8 MHz */
	m_cpub->set_addrmap(AS_PROGRAM, &darius_state::darius_cpub_map);
	m_cpub->set_vblank_int("lscreen", FUNC(darius_state::irq4_line_hold));

	Z80(config, m_adpcm, XTAL(8'000'000)/2); /* 4 MHz */  /* ADPCM player using MSM5205 */
	m_adpcm->set_addrmap(AS_PROGRAM, &darius_state::darius_sound2_map);
	m_adpcm->set_addrmap(AS_IO, &darius_state::darius_sound2_io_map);

	config.set_maximum_quantum(attotime::from_hz(600));   /* 10 CPU slices per frame ? */

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_darius);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	config.set_default_layout(layout_darius);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(36*8, 32*8);
	lscreen.set_visarea(0*8, 36*8-1, 1*8, 29*8-1);
	lscreen.set_screen_update(FUNC(darius_state::screen_update_left));
	lscreen.set_palette(m_palette);

	screen_device &mscreen(SCREEN(config, "mscreen", SCREEN_TYPE_RASTER));
	mscreen.set_refresh_hz(60);
	mscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	mscreen.set_size(36*8, 32*8);
	mscreen.set_visarea(0*8, 36*8-1, 1*8, 29*8-1);
	mscreen.set_screen_update(FUNC(darius_state::screen_update_middle));
	mscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(36*8, 32*8);
	rscreen.set_visarea(0*8, 36*8-1, 1*8, 29*8-1);
	rscreen.set_screen_update(FUNC(darius_state::screen_update_right));
	rscreen.set_palette(m_palette);

	PC080SN(config, m_pc080sn, 0, m_palette, gfx_darius_tmap);
	m_pc080sn->set_offsets(-16, 8);
	m_pc080sn->set_yinvert(0);
	m_pc080sn->set_dblwidth(1);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(8'000'000)/2)); /* 4 MHz */
	ym1.irq_handler().set_inputline(m_audiocpu, 0); /* assumes Z80 sandwiched between 68Ks */
	ym1.port_a_write_callback().set(FUNC(darius_state::write_portA0));
	ym1.port_b_write_callback().set(FUNC(darius_state::write_portB0));
	ym1.add_route(0, "filter0.0l", 0.08);
	ym1.add_route(0, "filter0.0r", 0.08);
	ym1.add_route(1, "filter0.1l", 0.08);
	ym1.add_route(1, "filter0.1r", 0.08);
	ym1.add_route(2, "filter0.2l", 0.08);
	ym1.add_route(2, "filter0.2r", 0.08);
	ym1.add_route(3, "filter0.3l", 0.60);
	ym1.add_route(3, "filter0.3r", 0.60);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(8'000'000)/2)); /* 4 MHz */
	ym2.port_a_write_callback().set(FUNC(darius_state::write_portA1));
	ym2.port_b_write_callback().set(FUNC(darius_state::write_portB1));
	ym2.add_route(0, "filter1.0l", 0.08);
	ym2.add_route(0, "filter1.0r", 0.08);
	ym2.add_route(1, "filter1.1l", 0.08);
	ym2.add_route(1, "filter1.1r", 0.08);
	ym2.add_route(2, "filter1.2l", 0.08);
	ym2.add_route(2, "filter1.2r", 0.08);
	ym2.add_route(3, "filter1.3l", 0.60);
	ym2.add_route(3, "filter1.3r", 0.60);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(darius_state::adpcm_int));   /* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);      /* 8KHz   */
	m_msm->add_route(ALL_OUTPUTS, "msm5205.l", 1.0);
	m_msm->add_route(ALL_OUTPUTS, "msm5205.r", 1.0);

	for (int chip = 0; chip < 2; chip++)
	{
		for (int out = 0; out < 4; out++)
		{
			FILTER_VOLUME(config, m_filter_l[chip][out]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
			FILTER_VOLUME(config, m_filter_r[chip][out]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
		}
	}

	FILTER_VOLUME(config, m_msm5205_l).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_VOLUME(config, m_msm5205_r).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/*******************************************************************************
        ROM DEFINITIONS
*******************************************************************************/

ROM_START( darius )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_59-1.186", 0x00000, 0x10000, CRC(11aab4eb) SHA1(92f795e96a940e8d94abbf429ba4ac119992b991) )
	ROM_LOAD16_BYTE( "a96_58-1.152", 0x00001, 0x10000, CRC(5f71e697) SHA1(bf959cf82e8e8ba950ab40d9c008ad5de01385aa) )
	ROM_LOAD16_BYTE( "a96_61-2.187", 0x20000, 0x10000, CRC(4736aa9b) SHA1(05e549d96a053e6b3bc34359267adcd73f98dd4a) )
	ROM_LOAD16_BYTE( "a96_66-2.153", 0x20001, 0x10000, CRC(4ede5f56) SHA1(88c06aef4b0a3e29fa30c24a57f2d3a05fc9f021) )
	ROM_LOAD16_BYTE( "a96_31.188",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )    /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "pc080sn", 0 )
	/* There are THREE of each SCR gfx rom on the actual board, making a complete set for every PC080SN tilemap chip */
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00003, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00000, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40003, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40000, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_62.175", 0x80003, 0x10000, CRC(9179862c) SHA1(be94c7d213a34baf82f974ee1092aba44b072623) )
	ROM_LOAD32_BYTE( "a96_63.196", 0x80001, 0x10000, CRC(fa19cfff) SHA1(58a3ae3270ebe5a162cd62df06da7199843707cf) )
	ROM_LOAD32_BYTE( "a96_64.176", 0x80002, 0x10000, CRC(814c676f) SHA1(a6a64e65a3c163ecfede14b48ea70c20050248c3) )
	ROM_LOAD32_BYTE( "a96_65.197", 0x80000, 0x10000, CRC(14eee326) SHA1(41760fada2a5e34ee6c9250af927baf650d9cfc4) )

	ROM_REGION( 0x8000, "text", 0 )         /* 8x8 SCR tiles */
	/* There's only one of each of these on a real board */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END


ROM_START( dariusu )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_59-1.186", 0x00000, 0x10000, CRC(11aab4eb) SHA1(92f795e96a940e8d94abbf429ba4ac119992b991) )
	ROM_LOAD16_BYTE( "a96_58-1.152", 0x00001, 0x10000, CRC(5f71e697) SHA1(bf959cf82e8e8ba950ab40d9c008ad5de01385aa) )
	ROM_LOAD16_BYTE( "a96_61-2.187", 0x20000, 0x10000, CRC(4736aa9b) SHA1(05e549d96a053e6b3bc34359267adcd73f98dd4a) )
	ROM_LOAD16_BYTE( "a96_60-2.153", 0x20001, 0x10000, CRC(9bf58617) SHA1(09b52b6aa522a61813b2e581b7e039a1fb6d6411) )
	ROM_LOAD16_BYTE( "a96_31.188",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )    /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "pc080sn", 0 )
	/* There are THREE of each SCR gfx rom on the actual board, making a complete set for every PC080SN tilemap chip */
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00003, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00000, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40003, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40000, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_62.175", 0x80003, 0x10000, CRC(9179862c) SHA1(be94c7d213a34baf82f974ee1092aba44b072623) )
	ROM_LOAD32_BYTE( "a96_63.196", 0x80001, 0x10000, CRC(fa19cfff) SHA1(58a3ae3270ebe5a162cd62df06da7199843707cf) )
	ROM_LOAD32_BYTE( "a96_64.176", 0x80002, 0x10000, CRC(814c676f) SHA1(a6a64e65a3c163ecfede14b48ea70c20050248c3) )
	ROM_LOAD32_BYTE( "a96_65.197", 0x80000, 0x10000, CRC(14eee326) SHA1(41760fada2a5e34ee6c9250af927baf650d9cfc4) )

	ROM_REGION( 0x8000, "text", 0 )         /* 8x8 SCR tiles */
	/* There's only one of each of these on a real board */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END


ROM_START( dariusj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_29-1.185", 0x00000, 0x10000, CRC(75486f62) SHA1(818b095f2c6cc5764161c3e14ba70fe1c4b2f724) )
	ROM_LOAD16_BYTE( "a96_28-1.152", 0x00001, 0x10000, CRC(fb34d400) SHA1(b14517384f5eadca8b73833bcd81374614b928d4) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_31.187",   0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154",   0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33-1.190", 0x00000, 0x10000, CRC(ff186048) SHA1(becb00d2cc69a6d4e839086bd3d902f4e6a99aa6) )
	ROM_LOAD16_BYTE( "a96_32-1.157", 0x00001, 0x10000, CRC(d9719de8) SHA1(9e907cfb5cbe6abebccfbd065d02e7a71c5aa494) )
	ROM_LOAD16_BYTE( "a96_35-1.191", 0x20000, 0x10000, CRC(b3280193) SHA1(f4bad066c16682f9267752c50a31ef64b312f11e) )
	ROM_LOAD16_BYTE( "a96_34-1.158", 0x20001, 0x10000, CRC(ca3b2573) SHA1(4da0d8536e546ea46b2374318e25c30305f4c977) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00003, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00000, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40003, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40000, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80003, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80000, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "text", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END

ROM_START( dariuso )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_29.185", 0x00000, 0x10000, CRC(f775162b) SHA1(a17e570c2ba4daf0a3526b45c324c822faac0c8d) )
	ROM_LOAD16_BYTE( "a96_28.152", 0x00001, 0x10000, CRC(4721d667) SHA1(fa9a109054a818f836452215204ce91f2b166ddb) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_31.187", 0x40000, 0x10000, CRC(e9bb5d89) SHA1(a5d08129c32b97e2cce84496945766fd32b6506e) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_30.154", 0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_33.190", 0x00000, 0x10000, CRC(d2f340d2) SHA1(d9175bf4dda5707afb3c57d3b6affe0305084c71) )
	ROM_LOAD16_BYTE( "a96_32.157", 0x00001, 0x10000, CRC(044c9848) SHA1(5293e9e83fd38d0d14e4f3b3a342d88e27ee44d6) )
	ROM_LOAD16_BYTE( "a96_35.191", 0x20000, 0x10000, CRC(b8ed718b) SHA1(8951f9c3c971c5621ec98b63fb27d44f30304c70) )
	ROM_LOAD16_BYTE( "a96_34.158", 0x20001, 0x10000, CRC(7556a660) SHA1(eaa82f3e1f827616ff25e22673d6d2ee54f0ad4c) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00003, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00000, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40003, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40000, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80003, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80000, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "text", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END

ROM_START( dariuse )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_FILL( 0x00000, 0x60000, 0xff )
	ROM_LOAD16_BYTE( "a96_68.185", 0x00000, 0x10000, CRC(ed721127) SHA1(8127f4a9b26b5fb83a381235eef0577d60d1cfd7) )
	ROM_LOAD16_BYTE( "a96_67.152", 0x00001, 0x10000, CRC(b99aea8c) SHA1(859ada7c472ab2ac308faa775066e79ed1f4ad71) )
	/* middle area is empty */
	ROM_LOAD16_BYTE( "a96_70.187", 0x40000, 0x10000, CRC(54590b31) SHA1(2b89846f14a5cb19b58ab4999bc5ae11671bbb5a) )   /* 2 data roms */
	ROM_LOAD16_BYTE( "a96_69.154", 0x40001, 0x10000, CRC(9eb5e127) SHA1(50e2fe5ec7f79ecf1fb5107298da13ef5ab37162) )   // == a96_30.154

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "a96_57.33", 0x00000, 0x10000, CRC(33ceb730) SHA1(05070ea503ac57ff8445145d6f97115f7aad90a5) )

	ROM_REGION( 0x80000, "cpub", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "a96_72.190", 0x00000, 0x10000, CRC(248ca2cc) SHA1(43b29146d8e2c62dd1fb7dc842fd441a360f2453) )
	ROM_LOAD16_BYTE( "a96_71.157", 0x00001, 0x10000, CRC(65dd0403) SHA1(8036c35ce5df0727cccb9ece3bfac9577160d4fd) )
	ROM_LOAD16_BYTE( "a96_74.191", 0x20000, 0x10000, CRC(0ea31f60) SHA1(c9e7eaf8bf3abbef944b7de407d5d5ddaac93e31) )
	ROM_LOAD16_BYTE( "a96_73.158", 0x20001, 0x10000, CRC(27036a4d) SHA1(426dccb8f559d39460c97bfd4354c74a59af172e) )

	ROM_REGION( 0x10000, "adpcm", 0 )   /* second Z80 driving the ADPCM chip */
	ROM_LOAD( "a96_56.18", 0x00000, 0x10000, CRC(292ef55c) SHA1(67bfe3693e43daece06d4795645d54cd66419e5b) )        /* Z80 prog + ADPCM samples */

	ROM_REGION( 0x60000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "a96_48.24",  0x00000, 0x10000, CRC(39c9b3aa) SHA1(43a91d916c5a09207dfa37413feb5025636f37ae) )   /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_49.25",  0x20000, 0x10000, CRC(37a7d88a) SHA1(cede0d810d74ec460dcc4b391bb1acd5a669a7b4) )
	ROM_LOAD16_BYTE( "a96_50.26",  0x40000, 0x10000, CRC(75d738e4) SHA1(634606da46136ab605f5477af5639a20e39b44c4) )
	ROM_LOAD16_BYTE( "a96_51.47",  0x00001, 0x10000, CRC(1bf8f0d3) SHA1(7f36e69336260958282eb663fe71b56410f0ee42) )
	ROM_LOAD16_BYTE( "a96_52.48",  0x20001, 0x10000, CRC(2d9b2128) SHA1(9b72936fbd9dca6ef8302ac6c40a1cec019cebb5) )
	ROM_LOAD16_BYTE( "a96_53.49",  0x40001, 0x10000, CRC(0173484c) SHA1(41d70039bda0965afe89251696ceaec7b7f40c24) )

	ROM_REGION( 0xc0000, "sprites", 0 )
	ROM_LOAD32_BYTE( "a96_44.179", 0x00003, 0x10000, CRC(bbc18878) SHA1(7732ab2a3002f8b500615377dab42ac75451cb3b) )   /* 16x16 sprites */
	ROM_LOAD32_BYTE( "a96_45.200", 0x00001, 0x10000, CRC(616cdd8b) SHA1(74e0c483a68d984a689ea1381ed3a9da2f8a410a) )
	ROM_LOAD32_BYTE( "a96_46.180", 0x00002, 0x10000, CRC(fec35418) SHA1(f0f401c3634e91b81cb8484b7b03f350d382e889) )
	ROM_LOAD32_BYTE( "a96_47.201", 0x00000, 0x10000, CRC(8df9286a) SHA1(4a197e4c38d1750cc316b8710f4a0fef4316be14) )

	ROM_LOAD32_BYTE( "a96_40.177", 0x40003, 0x10000, CRC(b699a51e) SHA1(5fd751dd44618743dc8a3df04cf0a987753a868b) )
	ROM_LOAD32_BYTE( "a96_41.198", 0x40001, 0x10000, CRC(97128a3a) SHA1(257ddd1ba71e6beeaf18e0c5d7006d1d2b6a5edf) )
	ROM_LOAD32_BYTE( "a96_42.178", 0x40002, 0x10000, CRC(7f55ee0f) SHA1(d9ba7b8fbf59308a08613d67e92da6829f6b6db3) )
	ROM_LOAD32_BYTE( "a96_43.199", 0x40000, 0x10000, CRC(c7cad469) SHA1(dbd37aa10f12e4950f8ec6bcd7d150fa55e64742) )

	ROM_LOAD32_BYTE( "a96_36.175", 0x80003, 0x10000, CRC(af598141) SHA1(f3b888bcbd4560cca48187055cbe4107e2b392a6) )
	ROM_LOAD32_BYTE( "a96_37.196", 0x80001, 0x10000, CRC(b48137c8) SHA1(03e98a93f4fa19dfe77da244c002abc84b936a22) )
	ROM_LOAD32_BYTE( "a96_38.176", 0x80002, 0x10000, CRC(e4f3e3a7) SHA1(0baa8a672516bcc4f17f40f429ac3d227de16625) )
	ROM_LOAD32_BYTE( "a96_39.197", 0x80000, 0x10000, CRC(ea30920f) SHA1(91d47b10886d6c243bc676435e300cb3b5fcca33) )

	ROM_REGION( 0x8000, "text", 0 )         /* 8x8 SCR tiles */
	ROM_LOAD16_BYTE( "a96_54.143", 0x0000, 0x4000, CRC(51c02ae2) SHA1(27d2a6c649d047da1f22758569cb36531e3bf8bc) )
	ROM_LOAD16_BYTE( "a96_55.144", 0x0001, 0x4000, CRC(771e4d98) SHA1(0e8ce5d569775883f4bc777b9bd49eb23ba7b42e) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD16_BYTE( "a96-24.163", 0x0000, 0x0400, CRC(0fa8be7f) SHA1(079686b5d65b4b966591090d8c0e13e66dc5beca) ) /* proms, currently unused */
	ROM_LOAD16_BYTE( "a96-25.164", 0x0400, 0x0400, CRC(265508a6) SHA1(f8ee1c658b33ae76d8a457a4042d9b4b58247823) )
	ROM_LOAD16_BYTE( "a96-26.165", 0x0800, 0x0400, CRC(4891b9c0) SHA1(1f550a9a4ad3ca379f88f5865ed1b281c7b87f31) )
ROM_END

} // anonymous namespace


/*******************************************************************************
        DRIVERS
*******************************************************************************/

GAME( 1986, darius,  0,       darius,  darius,  darius_state, empty_init, ROT0, "Taito Corporation Japan",   "Darius (World, rev 2)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariusu, darius,  darius,  dariusu, darius_state, empty_init, ROT0, "Taito America Corporation", "Darius (US, rev 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariusj, darius,  darius,  dariusj, darius_state, empty_init, ROT0, "Taito Corporation",         "Darius (Japan, rev 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariuso, darius,  darius,  dariusj, darius_state, empty_init, ROT0, "Taito Corporation",         "Darius (Japan)",               MACHINE_SUPPORTS_SAVE )
GAME( 1986, dariuse, darius,  darius,  dariuse, darius_state, empty_init, ROT0, "Taito Corporation",         "Darius Extra Version (Japan)", MACHINE_SUPPORTS_SAVE )
