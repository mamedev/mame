// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
#ifndef MAME_DATAEAST_DECOCASS_H
#define MAME_DATAEAST_DECOCASS_H

#pragma once

#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)

#include "decocass_tape.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "cpu/mcs48/mcs48.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define T1PROM 1
#define T1DIRECT 2
#define T1LATCH 4
#define T1LATCHINV 8

class decocass_state : public driver_device
{
public:
	decocass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_dongle_r(*this)
		, m_dongle_w(*this)
		, m_donglerom(*this, "dongle")
		, m_audiocpu(*this, "audiocpu")
		, m_watchdog(*this, "watchdog")
		, m_cassette(*this, "cassette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_rambase(*this, "rambase")
		, m_charram(*this, "charram")
		, m_fgvideoram(*this, "fgvideoram")
		, m_colorram(*this, "colorram")
		, m_tileram(*this, "tileram")
		, m_objectram(*this, "objectram")
		, m_paletteram(*this, "paletteram")
		, m_bank1(*this, "bank1")
	{
	}

	void decocass(machine_config &config);
	void decocrom(machine_config &config);

	void init_decocass();
	void init_decocrom();
	void init_cdsteljn();
	void init_nebula();

protected:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<upi41_cpu_device> m_mcu;

	/* dongles-related */
	read8sm_delegate    m_dongle_r; // TODO: why isn't this a virtual method?
	write8sm_delegate   m_dongle_w; // TODO: why isn't this a virtual method?

	optional_region_ptr<uint8_t> m_donglerom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	int32_t     m_firsttime = 0U;
	uint8_t     m_latch1 = 0U;

private:
	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<decocass_tape_device> m_cassette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_colorram;
	uint8_t *   m_bgvideoram = nullptr; /* shares bits D0-3 with tileram! */
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_objectram;
	required_shared_ptr<uint8_t> m_paletteram;
	optional_memory_bank         m_bank1;

	size_t    m_bgvideoram_size = 0U;

	/* video-related */
	tilemap_t   *m_fg_tilemap = nullptr;
	tilemap_t   *m_bg_tilemap_l = nullptr;
	tilemap_t   *m_bg_tilemap_r = nullptr;
	uint8_t     m_empty_tile[16*16]{};
	int32_t     m_watchdog_count = 0;
	int32_t     m_watchdog_flip = 0;
	int32_t     m_color_missiles = 0;
	int32_t     m_color_center_bot = 0;
	int32_t     m_mode_set = 0;
	int32_t     m_back_h_shift = 0;
	int32_t     m_back_vl_shift = 0;
	int32_t     m_back_vr_shift = 0;
	int32_t     m_part_h_shift = 0;
	int32_t     m_part_v_shift = 0;
	int32_t     m_center_h_shift_space = 0;
	int32_t     m_center_v_shift = 0;
	rectangle m_bg_tilemap_l_clip{};
	rectangle m_bg_tilemap_r_clip{};

	/* sound-related */
	uint8_t     m_sound_ack = 0U;  /* sound latches, ACK status bits and NMI timer */
	uint8_t     m_audio_nmi_enabled = 0U;
	uint8_t     m_audio_nmi_state = 0U;

	/* misc */
	uint8_t     m_decocass_reset = 0U;
	int32_t     m_de0091_enable = 0;  /* DE-0091xx daughter board enable */
	uint8_t     m_quadrature_decoder[4]{};  /* four inputs from the quadrature decoder (H1, V1, H2, V2) */
	int       m_showmsg = 0;        // for debugging purposes

	/* i8041 */
	uint8_t     m_i8041_p1 = 0U;
	uint8_t     m_i8041_p2 = 0U;
	int       m_i8041_p1_write_latch = 0;
	int       m_i8041_p1_read_latch = 0;
	int       m_i8041_p2_write_latch = 0;
	int       m_i8041_p2_read_latch = 0;

	/* DS Telejan */
	uint8_t     m_mux_data = 0U;

	TILEMAP_MAPPER_MEMBER(fgvideoram_scan_cols);
	TILEMAP_MAPPER_MEMBER(bgvideoram_scan_cols);
	TILE_GET_INFO_MEMBER(get_bg_l_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_r_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start() override ATTR_COLD;
	void decocass_palette(palette_device &palette) const;

	uint32_t screen_update_decocass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decocass_coin_counter_w(uint8_t data);
	void decocass_sound_command_w(uint8_t data);
	uint8_t decocass_sound_command_main_r();
	uint8_t decocass_sound_data_r();
	uint8_t decocass_sound_ack_r();
	void decocass_sound_data_w(uint8_t data);
	uint8_t decocass_sound_command_r();
	void decocass_sound_nmi_enable_w(uint8_t data);
	uint8_t decocass_sound_nmi_enable_r();
	uint8_t decocass_sound_data_ack_reset_r();
	void decocass_sound_data_ack_reset_w(uint8_t data);
	void decocass_nmi_reset_w(uint8_t data);
	void decocass_quadrature_decoder_reset_w(uint8_t data);
	void decocass_adc_w(uint8_t data);
	uint8_t decocass_input_r(offs_t offset);

	void decocass_reset_w(offs_t offset, uint8_t data);

	uint8_t decocass_e5xx_r(offs_t offset);
	void decocass_e5xx_w(offs_t offset, uint8_t data);
	void decocass_de0091_w(offs_t offset, uint8_t data);
	void decocass_e900_w(uint8_t data);


	void i8041_p1_w(uint8_t data);
	uint8_t i8041_p1_r();
	void i8041_p2_w(uint8_t data);
	uint8_t i8041_p2_r();

	void decocass_machine_state_save_init();

	void decocass_paletteram_w(offs_t offset, uint8_t data);
	void decocass_charram_w(offs_t offset, uint8_t data);
	void decocass_fgvideoram_w(offs_t offset, uint8_t data);
	void decocass_colorram_w(offs_t offset, uint8_t data);
	void decocass_bgvideoram_w(offs_t offset, uint8_t data);
	void decocass_tileram_w(offs_t offset, uint8_t data);
	void decocass_objectram_w(offs_t offset, uint8_t data);

	void decocass_watchdog_count_w(uint8_t data);
	void decocass_watchdog_flip_w(uint8_t data);
	void decocass_color_missiles_w(uint8_t data);
	void decocass_mode_set_w(uint8_t data);
	void decocass_color_center_bot_w(uint8_t data);
	void decocass_back_h_shift_w(uint8_t data);
	void decocass_back_vl_shift_w(uint8_t data);
	void decocass_back_vr_shift_w(uint8_t data);
	void decocass_part_h_shift_w(uint8_t data);
	void decocass_part_v_shift_w(uint8_t data);
	void decocass_center_h_shift_space_w(uint8_t data);
	void decocass_center_v_shift_w(uint8_t data);

	void decocass_video_state_save_init();

	void mirrorvideoram_w(offs_t offset, uint8_t data);
	void mirrorcolorram_w(offs_t offset, uint8_t data);
	uint8_t mirrorvideoram_r(offs_t offset);
	uint8_t mirrorcolorram_r(offs_t offset);
	uint8_t cdsteljn_input_r(offs_t offset);
	void cdsteljn_mux_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(decocass_audio_nmi_gen);
	void decocass_map(address_map &map) ATTR_COLD;
	void decocrom_map(address_map &map) ATTR_COLD;
	void decocass_sound_map(address_map &map) ATTR_COLD;

	void draw_edge(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, bool opaque);
	void draw_special_priority(bitmap_ind16 &bitmap, bitmap_ind8 &priority, const rectangle &cliprect);
	void draw_center(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mark_bg_tile_dirty(offs_t offset);
	void draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &priority, const rectangle &cliprect, int color,
					int sprite_y_adjust, int sprite_y_adjust_flip_screen,
					uint8_t *sprite_ram, int interleave);

	void draw_missiles(bitmap_ind16 &bitmap, bitmap_ind8 &priority, const rectangle &cliprect,
					int missile_y_adjust, int missile_y_adjust_flip_screen,
					uint8_t *missile_ram, int interleave);
protected:
	void decocass_fno( offs_t offset, uint8_t data );
};

class decocass_type1_state : public decocass_state
{
public:
	decocass_type1_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
		m_type1_map = nullptr;
	}

	void cprogolfj(machine_config &config);
	void cfboy0a1(machine_config &config);
	void cdsteljn(machine_config &config);
	void csuperas(machine_config &config);
	void clocknch(machine_config &config);
	void cterrani(machine_config &config);
	void chwy(machine_config &config);
	void ctisland3(machine_config &config);
	void cocean1a(machine_config &config);
	void cluckypo(machine_config &config);
	void cexplore(machine_config &config);
	void cmanhat(machine_config &config);
	void clocknchj(machine_config &config);
	void cprogolf(machine_config &config);
	void ctsttape(machine_config &config);
	void castfant(machine_config &config);
	void ctisland(machine_config &config);
	void cnebula(machine_config &config);

private:
	DECLARE_MACHINE_RESET(ctsttape);
	DECLARE_MACHINE_RESET(chwy);
	DECLARE_MACHINE_RESET(cdsteljn);
	DECLARE_MACHINE_RESET(cterrani);
	DECLARE_MACHINE_RESET(castfant);
	DECLARE_MACHINE_RESET(csuperas);
	DECLARE_MACHINE_RESET(cmanhat);
	DECLARE_MACHINE_RESET(clocknch);
	DECLARE_MACHINE_RESET(cprogolf);
	DECLARE_MACHINE_RESET(cprogolfj);
	DECLARE_MACHINE_RESET(cluckypo);
	DECLARE_MACHINE_RESET(ctisland);
	DECLARE_MACHINE_RESET(ctisland3);
	DECLARE_MACHINE_RESET(cexplore);
	DECLARE_MACHINE_RESET(cocean1a); /* 10 */
	DECLARE_MACHINE_RESET(cfboy0a1); /* 12 */
	DECLARE_MACHINE_RESET(clocknchj); /* 11 */
	DECLARE_MACHINE_RESET(cnebula);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_type1_r(offs_t offset);

	/* dongle type #1 */
	uint32_t    m_type1_inmap = 0U;
	uint32_t    m_type1_outmap = 0U;
	uint8_t* m_type1_map = 0U;
};


class decocass_type2_state : public decocass_state
{
public:
	decocass_type2_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_type2_r(offs_t offset);
	void decocass_type2_w(offs_t offset, uint8_t data);

	/* dongle type #2: status of the latches */
	int32_t     m_type2_d2_latch = 0; /* latched 8041-STATUS D2 value */
	int32_t     m_type2_xx_latch = 0; /* latched value (D7-4 == 0xc0) ? 1 : 0 */
	int32_t     m_type2_promaddr = 0; /* latched PROM address A0-A7 */
};


class decocass_type3_state : public decocass_state
{
public:
	decocass_type3_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}


	void csdtenis(machine_config &config);
	void cburnrub(machine_config &config);
	void cppicf(machine_config &config);
	void cgraplop2(machine_config &config);
	void cfghtice(machine_config &config);
	void cpsoccer(machine_config &config);
	void cnightst(machine_config &config);
	void cprobowl(machine_config &config);
	void cskater(machine_config &config);
	void cbtime(machine_config &config);
	void cgraplop(machine_config &config);
	void clapapa(machine_config &config);
	void cfishing(machine_config &config);
	void czeroize(machine_config &config);

private:
	DECLARE_MACHINE_RESET(cfishing);
	DECLARE_MACHINE_RESET(cbtime);
	DECLARE_MACHINE_RESET(cburnrub);
	DECLARE_MACHINE_RESET(cgraplop);
	DECLARE_MACHINE_RESET(cgraplop2);
	DECLARE_MACHINE_RESET(clapapa);
	DECLARE_MACHINE_RESET(cskater);
	DECLARE_MACHINE_RESET(cprobowl);
	DECLARE_MACHINE_RESET(cnightst);
	DECLARE_MACHINE_RESET(cpsoccer);
	DECLARE_MACHINE_RESET(csdtenis);
	DECLARE_MACHINE_RESET(czeroize);
	DECLARE_MACHINE_RESET(cppicf);
	DECLARE_MACHINE_RESET(cfghtice);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_type3_r(offs_t offset);
	void decocass_type3_w(offs_t offset, uint8_t data);

	/* dongle type #3: status and patches */
	int32_t     m_type3_ctrs = 0;     /* 12 bit counter stage */
	int32_t     m_type3_d0_latch = 0; /* latched 8041-D0 value */
	int32_t     m_type3_pal_19 = 0;       /* latched 1 for PAL input pin-19 */
	int32_t     m_type3_swap = 0;
};



class decocass_type4_state : public decocass_state
{
public:
	decocass_type4_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

private:

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_type4_r(offs_t offset);
	void decocass_type4_w(offs_t offset, uint8_t data);

	/* dongle type #4: status */
	int32_t     m_type4_ctrs = 0;     /* latched PROM address (E5x0 LSB, E5x1 MSB) */
	int32_t     m_type4_latch = 0;        /* latched enable PROM (1100xxxx written to E5x1) */
};


class decocass_type5_state : public decocass_state
{
public:
	decocass_type5_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

private:

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_type5_r(offs_t offset);
	void decocass_type5_w(offs_t offset, uint8_t data);

	/* dongle type #5: status */
	int32_t     m_type5_latch = 0;        /* latched enable PROM (1100xxxx written to E5x1) */
};


class decocass_nodong_state : public decocass_state
{
public:
	decocass_nodong_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

private:

	//virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_nodong_r(offs_t offset);
};


class decocass_widel_state : public decocass_state
{
public:
	decocass_widel_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

private:

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t decocass_widel_r(offs_t offset);
	void decocass_widel_w(offs_t offset, uint8_t data);

	/* dongle type widel: status */
	int32_t     m_widel_ctrs = 0;     /* latched PROM address (E5x0 LSB, E5x1 MSB) */
	int32_t     m_widel_latch = 0;        /* latched enable PROM (1100xxxx written to E5x1) */
};

class decocass_darksoft_state : public decocass_state
{
public:
	decocass_darksoft_state(const machine_config &mconfig, device_type type, const char *tag)
		: decocass_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t decocass_darksoft_r(offs_t offset);
	void decocass_darksoft_w(offs_t offset, uint8_t data);

	uint32_t m_address;
};

#endif // MAME_DATAEAST_DECOCASS_H
