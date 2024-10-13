// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Howie Cohen, Frank Palazzolo, Ernesto Corvi, Aaron Giles
/*************************************************************************

    Sega Z80-3D system

*************************************************************************/
#ifndef MAME_SEGA_TURBO_H
#define MAME_SEGA_TURBO_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

// sprites are scaled in the analog domain; to give a better rendition of this, we scale in the X direction by this factor

#define TURBO_X_SCALE       2


class turbo_base_state : public driver_device
{
public:
	turbo_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i8255(*this, "i8255%u", 0U)
		, m_spriteroms(*this, "sprites")
		, m_proms(*this, "proms")
		, m_videoram(*this, "videoram")
		, m_sprite_position(*this, "spritepos")
		, m_samples(*this, "samples")
		, m_discrete(*this, "discrete")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_pedal(*this, "PEDAL")
		, m_dsw(*this, "DSW%u", 1U)
		, m_digits(*this, "digit%u", 0U)
		, m_lamp(*this, "lamp")
	{ }

	ioport_value pedal_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// device / memory pointers
	required_device<z80_device> m_maincpu;
	optional_device_array<i8255_device, 4> m_i8255;

	required_region_ptr<uint8_t> m_spriteroms;
	required_region_ptr<uint8_t> m_proms;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_sprite_position;

	required_device<samples_device> m_samples;
	optional_device<discrete_device> m_discrete;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	optional_ioport m_pedal;
	optional_ioport_array<3> m_dsw;
	output_finder<32> m_digits;
	output_finder<> m_lamp;

	// machine state
	uint8_t m_i8279_scanlines = 0;

	// sound state
	uint8_t m_sound_state[3]{};

	// video state
	tilemap_t *m_fg_tilemap = nullptr;

	struct sprite_info
	{
		uint16_t ve = 0;        // VE0-15 signals for this row
		uint8_t lst = 0;        // LST0-7 signals for this row
		uint32_t latched[8]{};  // latched pixel data
		uint8_t plb[8]{};       // latched PLB state
		uint32_t offset[8]{};   // current offset for this row
		uint32_t frac[8]{};     // leftover fraction
		uint32_t step[8]{};     // stepping value
	};

	sprite_info m_sprite_info;

	void scanlines_w(uint8_t data);
	void digit_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	inline uint32_t sprite_xscale(uint8_t dacinput, double vr1, double vr2, double cext);
};

class buckrog_state : public turbo_base_state
{
public:
	buckrog_state(const machine_config &mconfig, device_type type, const char *tag)
		: turbo_base_state(mconfig, type, tag)
		, m_subcpu(*this, "subcpu")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_spriteram(*this, "spriteram")
		, m_bitmap_ram(*this, "bitmap_ram", 0xe000, ENDIANNESS_LITTLE)
		, m_bgcolorrom(*this, "bgcolor")
	{ }

	void buckrog(machine_config &config);
	void buckroge(machine_config &config);
	void buckrogu(machine_config &config);
	void buckrog_samples(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_subcpu;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_shared_ptr<uint8_t> m_spriteram;
	memory_share_creator<uint8_t> m_bitmap_ram;
	required_region_ptr<uint8_t> m_bgcolorrom;

	uint8_t m_fchg;
	uint8_t m_mov;
	uint8_t m_obch;
	uint8_t m_command;
	uint8_t m_myship;
	uint8_t m_last_sound_a;

	uint8_t subcpu_command_r();
	uint8_t port_2_r();
	uint8_t port_3_r();
	void bitmap_w(offs_t offset, uint8_t data);
	void ppi0a_w(uint8_t data);
	void ppi0b_w(uint8_t data);
	void ppi0c_w(uint8_t data);
	void ppi1c_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void i8255_0_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(delayed_i8255_w);
	void sound_a_w(uint8_t data);
	void sound_b_w(uint8_t data);
	void prepare_sprites(uint8_t y);
	uint32_t get_sprite_bits(uint8_t *plb);
	void update_samples();

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sub_prg_map(address_map &map) ATTR_COLD;
	void sub_portmap(address_map &map) ATTR_COLD;
};

class subroc3d_state : public turbo_base_state
{
public:
	subroc3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: turbo_base_state(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_shutter(*this, "shutter")
	{ }

	void subroc3d(machine_config &config);
	void subroc3d_samples(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_spriteram;
	output_finder<> m_shutter;

	uint8_t m_col = 0;
	uint8_t m_ply = 0;
	uint8_t m_flip = 0;
	uint8_t m_mdis = 0;
	uint8_t m_mdir = 0;
	uint8_t m_tdis = 0;
	uint8_t m_tdir = 0;
	uint8_t m_fdis = 0;
	uint8_t m_fdir = 0;
	uint8_t m_hdis = 0;
	uint8_t m_hdir = 0;

	void ppi0a_w(uint8_t data);
	void ppi0b_w(uint8_t data);
	void ppi0c_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_a_w(uint8_t data);
	void sound_b_w(uint8_t data);
	void sound_c_w(uint8_t data);
	void prepare_sprites(uint8_t y);
	uint32_t get_sprite_bits(uint8_t *plb);
	inline void update_volume(int leftchan, uint8_t dis, uint8_t dir);

	void prg_map(address_map &map) ATTR_COLD;
};

class turbo_state : public turbo_base_state
{
public:
	turbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: turbo_base_state(mconfig, type, tag)
		, m_roadroms(*this, "road")
		, m_spriteram(*this, "spriteram", 0x80, ENDIANNESS_LITTLE)
		, m_vr(*this, "VR%u", 1U)
		, m_dial(*this, "DIAL")
		, m_tachometer(*this, "tachometer")
		, m_speed(*this, "speed")
	{ }

	void turbo(machine_config &config);
	void turbo_samples(machine_config &config);

	void init_turbo_enc();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_roadroms;
	memory_share_creator<uint8_t> m_spriteram;
	required_ioport_array<2> m_vr;
	required_ioport m_dial;
	output_finder<> m_tachometer;
	output_finder<> m_speed;

	uint8_t m_osel;
	uint8_t m_bsel;
	uint8_t m_opa;
	uint8_t m_opb;
	uint8_t m_opc;
	uint8_t m_ipa;
	uint8_t m_ipb;
	uint8_t m_ipc;
	uint8_t m_fbpla;
	uint8_t m_fbcol;
	uint8_t m_collision;
	uint8_t m_last_analog;
	uint8_t m_accel;

	uint8_t collision_r();
	void collision_clear_w(uint8_t data);
	void analog_reset_w(uint8_t data);
	void coin_meter_1_w(int state);
	void coin_meter_2_w(int state);
	void start_lamp_w(int state);
	void ppi0a_w(uint8_t data);
	void ppi0b_w(uint8_t data);
	void ppi0c_w(uint8_t data);
	void ppi1a_w(uint8_t data);
	void ppi1b_w(uint8_t data);
	void ppi1c_w(uint8_t data);
	void ppi3c_w(uint8_t data);
	uint8_t analog_r();
	uint8_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_a_w(uint8_t data);
	void sound_b_w(uint8_t data);
	void sound_c_w(uint8_t data);
	void prepare_sprites(uint8_t y);
	uint32_t get_sprite_bits(uint8_t road);
	void rom_decode();
	void update_samples();
	TIMER_CALLBACK_MEMBER(update_sound_a);

	void prg_map(address_map &map) ATTR_COLD;
};


#endif // MAME_SEGA_TURBO_H
