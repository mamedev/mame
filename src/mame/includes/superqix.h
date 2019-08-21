// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Nicola Salmoria, Tomasz Slanina
#ifndef MAME_INCLUDES_SUPERQIX_H
#define MAME_INCLUDES_SUPERQIX_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "cpu/m6805/m68705.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "emupal.h"
#include "tilemap.h"

class superqix_state_base : public driver_device
{
public:
	superqix_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_mcu(*this, "mcu")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_bitmapram(*this, "bitmapram")
		, m_bitmapram2(*this, "bitmapram2")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_ay1(*this, "ay1")
	{ }

	void init_perestro();
	void init_sqix();
	void init_sqixr0();
	void init_pbillian();
	void init_hotsmash();

	TILE_GET_INFO_MEMBER(sqix_get_bg_tile_info);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_mcu;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_bitmapram;
	optional_shared_ptr<uint8_t> m_bitmapram2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_ay1;

	// commmon 68705/8751/HLE
	uint8_t m_from_mcu;     // byte latch for 68705/8751->z80 comms
	uint8_t m_from_z80;     // byte latch for z80->68705/8751 comms
	bool m_z80_has_written; // z80 has written to latch flag
	bool m_mcu_has_written; // 68705/8751 has written to latch flag

	//general machine stuff
	bool m_invert_coin_lockout;
	bool m_invert_p2_spinner;
	int m_gfxbank;
	bool m_show_bitmap;
	bool m_nmi_mask;

	std::unique_ptr<bitmap_ind16> m_fg_bitmap[2];
	tilemap_t *m_bg_tilemap;

	DECLARE_READ8_MEMBER(nmi_ack_r);
	DECLARE_WRITE8_MEMBER(superqix_videoram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram2_w);
	DECLARE_WRITE8_MEMBER(superqix_0410_w);

	DECLARE_VIDEO_START(superqix);
	static rgb_t BBGGRRII(uint32_t raw);
	uint32_t screen_update_superqix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void superqix_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_map(address_map &map);

	virtual void machine_init_common();
};


class superqix_state : public superqix_state_base
{
public:
	superqix_state(const machine_config &mconfig, device_type type, const char *tag)
		: superqix_state_base(mconfig, type, tag)
		, m_ay2(*this, "ay2")
	{
	}

	void sqix(machine_config &config);
	void sqix_8031(machine_config &config);
	void sqix_nomcu(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(fromz80_semaphore_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(frommcu_semaphore_input_r);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<ay8910_device>    m_ay2;

	// 8031 and/or 8751 MCU related
	uint8_t m_bl_port1;
	uint8_t m_bl_fake_port2;
	uint8_t m_port2_raw;
	uint8_t m_bl_port3_out;

	DECLARE_READ8_MEMBER(z80_semaphore_assert_r);
	DECLARE_WRITE8_MEMBER(bootleg_mcu_port1_w);
	DECLARE_WRITE8_MEMBER(mcu_port2_w);
	DECLARE_WRITE8_MEMBER(mcu_port3_w);
	DECLARE_READ8_MEMBER(mcu_port3_r);
	DECLARE_READ8_MEMBER(bootleg_mcu_port3_r);
	DECLARE_WRITE8_MEMBER(bootleg_mcu_port3_w);
	DECLARE_WRITE8_MEMBER(z80_ay1_sync_address_w);
	DECLARE_READ8_MEMBER(z80_ay2_iob_r);
	DECLARE_WRITE8_MEMBER(z80_ay2_iob_w);
	DECLARE_WRITE8_MEMBER(bootleg_flipscreen_w);
	DECLARE_READ8_MEMBER(bootleg_in0_r);
	INTERRUPT_GEN_MEMBER(sqix_timer_irq);
	DECLARE_MACHINE_START(superqix);
	DECLARE_MACHINE_RESET(superqix);

	void sqix_port_map(address_map &map);
	void sqix_8031_map(address_map &map);

	virtual void machine_init_common() override;

	TIMER_CALLBACK_MEMBER(z80_semaphore_assert_cb);
	TIMER_CALLBACK_MEMBER(mcu_port2_w_cb);
	TIMER_CALLBACK_MEMBER(mcu_port3_w_cb);
	TIMER_CALLBACK_MEMBER(z80_ay1_sync_address_w_cb);
	TIMER_CALLBACK_MEMBER(z80_ay2_iob_w_cb);
	TIMER_CALLBACK_MEMBER(bootleg_mcu_port1_w_cb);
};


class hotsmash_state : public superqix_state_base
{
public:
	hotsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: superqix_state_base(mconfig, type, tag)
		, m_dsw(*this, "DSW%u", 1)
		, m_dials(*this, "DIAL%u", 1)
		, m_plungers(*this, "PLUNGER%u", 1)
		, m_launchbtns(*this, "LAUNCH%u", 1)
		, m_samples(*this, "samples")
		, m_samples_region(*this, "samples")
		, m_samplebuf()
		, m_portb_out(0xff)
		, m_portc_out(0xff)
		, m_dial_oldpos{ 0, 0 }
		, m_dial_sign{ 0, 0 }
	{
	}

	void pbillian(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(pbillian_semaphore_input_r);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	DECLARE_READ8_MEMBER(hotsmash_68705_porta_r);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portb_w);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portc_w);
	DECLARE_WRITE8_MEMBER(hotsmash_z80_mcu_w);
	DECLARE_READ8_MEMBER(hotsmash_z80_mcu_r);

	DECLARE_WRITE8_MEMBER(pbillian_sample_trigger_w);
	DECLARE_WRITE8_MEMBER(pbillian_0410_w);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	SAMPLES_START_CB_MEMBER(pbillian_sh_start);

	TILE_GET_INFO_MEMBER(pb_get_bg_tile_info);

	DECLARE_MACHINE_START(pbillian);
	DECLARE_VIDEO_START(pbillian);

	u32 screen_update_pbillian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pbillian_port_map(address_map &map);

	virtual void machine_init_common() override;

	int read_inputs(int player);

	void pbillian_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_ioport_array<2>         m_dsw;
	required_ioport_array<2>         m_dials;
	optional_ioport_array<2>         m_plungers;
	optional_ioport_array<2>         m_launchbtns;
	optional_device<samples_device>  m_samples;
	optional_region_ptr<u8>          m_samples_region;

	std::unique_ptr<s16[]>           m_samplebuf;

	// 68705 related
	u8  m_porta_in;
	u8  m_portb_out;
	u8  m_portc_out;

	// spinner quadrature stuff
	int m_dial_oldpos[2];
	int m_dial_sign[2];
};

#endif // MAME_INCLUDES_SUPERQIX_H
