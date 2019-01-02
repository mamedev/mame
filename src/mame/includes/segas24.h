// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
 * Sega System 24
 *
 */
#ifndef MAME_INCLUDES_SEGAS24_H
#define MAME_INCLUDES_SEGAS24_H

#pragma once

#include "machine/timer.h"
#include "video/segaic24.h"
#include "emupal.h"
#include "screen.h"

class segas24_state : public driver_device
{
public:
	segas24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_romboard(*this, "romboard")
		, m_floppy(*this, "floppy")
		, m_rombank(*this, "rombank%u", 1U)
		, m_irq_timer(*this, "irq_timer")
		, m_irq_timer_clear(*this, "irq_timer_clear")
		, m_frc_cnt_timer(*this, "frc_timer")
		, m_vtile(*this, "tile")
		, m_vsprite(*this, "sprite")
		, m_vmixer(*this, "mixer")
		, m_gground_hack_timer(nullptr)
		, m_p1(*this, "P1")
		, m_p2(*this, "P2")
		, m_p3(*this, "P3")
		, m_paddle(*this, "PADDLE")
		, m_mj_inputs(*this, {"MJ0", "MJ1", "MJ2", "MJ3", "MJ4", "MJ5", "P1", "P2"})
	{
	}

	void init_crkdown();
	void init_quizmeku();
	void init_qrouka();
	void init_roughrac();
	void init_qgh();
	void init_gground();
	void init_mahmajn2();
	void init_sspiritj();
	void init_mahmajn();
	void init_hotrod();
	void init_sspirits();
	void init_dcclub();
	void init_bnzabros();
	void init_dcclubfd();
	void init_qsww();
	void init_sgmast();

	void dcclub(machine_config &config);
	void mahmajn(machine_config &config);
	void system24_floppy(machine_config &config);
	void system24_floppy_dcclub(machine_config &config);
	void system24_floppy_fd1094(machine_config &config);
	void system24_floppy_fd_upd(machine_config &config);
	void system24_floppy_hotrod(machine_config &config);
	void system24_floppy_rom(machine_config &config);
	void system24_rom(machine_config &config);
	void system24(machine_config &config);

protected:
	virtual void device_post_load() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;
	optional_memory_region m_romboard;
	optional_region_ptr<uint8_t> m_floppy;

	optional_memory_bank_array<2> m_rombank;

	static const uint8_t  s_mahmajn_mlt[8];
	static const uint8_t s_mahmajn2_mlt[8];
	static const uint8_t      s_qgh_mlt[8];
	static const uint8_t s_bnzabros_mlt[8];
	static const uint8_t   s_qrouka_mlt[8];
	static const uint8_t s_quizmeku_mlt[8];
	static const uint8_t   s_dcclub_mlt[8];

	uint8_t m_fdc_track_side;
	uint8_t m_fdc_mode;
	int m_fdc_status;
	int m_fdc_track;
	int m_fdc_sector;
	int m_fdc_data;
	int m_fdc_phys_track;
	bool m_fdc_irq;
	bool m_fdc_drq;
	int m_fdc_span;
	int m_fdc_index_count;
	uint8_t *m_fdc_pt;
	int m_track_size;
	int m_cur_input_line;
	uint8_t m_curbank;
	uint8_t m_mlatch;
	const uint8_t *m_mlatch_table;

	uint16_t m_irq_tdata, m_irq_tval;
	uint8_t m_irq_tmode, m_irq_allow0, m_irq_allow1;
	bool m_irq_timer_pend0;
	bool m_irq_timer_pend1;
	bool m_irq_yms;
	bool m_irq_vblank;
	bool m_irq_sprite;
	attotime m_irq_synctime, m_irq_vsynctime;
	required_device<timer_device> m_irq_timer;
	required_device<timer_device> m_irq_timer_clear;
	//timer_device *m_irq_frc;
	required_device<timer_device> m_frc_cnt_timer;
	uint8_t m_frc_mode;

	bool m_cnt1;

	required_device<segas24_tile_device> m_vtile;
	required_device<segas24_sprite_device> m_vsprite;
	required_device<segas24_mixer_device> m_vmixer;

	DECLARE_WRITE_LINE_MEMBER(irq_ym);
	DECLARE_READ16_MEMBER(  paletteram_r );
	DECLARE_WRITE16_MEMBER( paletteram_w );
	DECLARE_READ16_MEMBER(  irq_r );
	DECLARE_WRITE16_MEMBER( irq_w );
	DECLARE_READ16_MEMBER(  fdc_r );
	DECLARE_WRITE16_MEMBER( fdc_w );
	DECLARE_READ16_MEMBER(  fdc_status_r );
	DECLARE_WRITE16_MEMBER( fdc_ctrl_w );
	DECLARE_READ8_MEMBER(  curbank_r );
	DECLARE_WRITE8_MEMBER( curbank_w );
	DECLARE_READ8_MEMBER(  frc_mode_r );
	DECLARE_WRITE8_MEMBER( frc_mode_w );
	DECLARE_READ8_MEMBER(  frc_r );
	DECLARE_WRITE8_MEMBER( frc_w );
	DECLARE_READ8_MEMBER(  mlatch_r );
	DECLARE_WRITE8_MEMBER( mlatch_w );
	DECLARE_READ16_MEMBER(  iod_r );
	DECLARE_WRITE16_MEMBER( iod_w );

	READ8_MEMBER(dcclub_p1_r);
	READ8_MEMBER(dcclub_p3_r);
	READ8_MEMBER(mahmajn_input_line_r);
	READ8_MEMBER(mahmajn_inputs_r);

	WRITE8_MEMBER(mahmajn_mux_w);
	WRITE8_MEMBER(hotrod_lamps_w);

	void fdc_init();
	void reset_reset();
	void reset_bank();
	void irq_init();
	void irq_timer_sync();
	void irq_timer_start(int old_tmode);
	WRITE_LINE_MEMBER(cnt1);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_clear_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_frc_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_vbl);

	// game specific
	TIMER_CALLBACK_MEMBER(gground_hack_timer_callback);
	emu_timer *m_gground_hack_timer;
	required_ioport m_p1;
	required_ioport m_p2;
	required_ioport m_p3;
	optional_ioport m_paddle;
	optional_ioport_array<8> m_mj_inputs;

	void common_map(address_map &map);
	void cpu1_map(address_map &map);
	void cpu2_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void hotrod_common_map(address_map &map);
	void hotrod_cpu1_map(address_map &map);
	void hotrod_cpu2_map(address_map &map);
	void rombd_common_map(address_map &map);
	void rombd_cpu1_map(address_map &map);
	void rombd_cpu2_map(address_map &map);
	void roughrac_common_map(address_map &map);
	void roughrac_cpu1_map(address_map &map);
	void roughrac_cpu2_map(address_map &map);
};

#endif // MAME_INCLUDES_SEGAS24_H
