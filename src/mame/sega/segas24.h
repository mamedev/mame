// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
 * Sega System 24
 *
 */
#ifndef MAME_SEGA_SEGAS24_H
#define MAME_SEGA_SEGAS24_H

#pragma once

#include "machine/timer.h"
#include "segaic24.h"
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
	void dcclubj(machine_config &config);
	void mahmajn(machine_config &config);
	void sgmastj(machine_config &config);
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

	uint8_t m_fdc_track_side = 0;
	uint8_t m_fdc_mode = 0;
	int m_fdc_status = 0;
	int m_fdc_track = 0;
	int m_fdc_sector = 0;
	int m_fdc_data = 0;
	int m_fdc_phys_track = 0;
	bool m_fdc_irq = false;
	bool m_fdc_drq = false;
	int m_fdc_span = 0;
	int m_fdc_index_count = 0;
	uint8_t *m_fdc_pt = nullptr;
	int m_track_size = 0;
	int m_cur_input_line = 0;
	uint8_t m_curbank = 0;
	uint8_t m_mlatch = 0;
	const uint8_t *m_mlatch_table = nullptr;

	uint16_t m_irq_tdata = 0, m_irq_tval = 0;
	uint8_t m_irq_tmode = 0, m_irq_allow0 = 0, m_irq_allow1 = 0;
	bool m_irq_timer_pend0 = false;
	bool m_irq_timer_pend1 = false;
	bool m_irq_yms = false;
	bool m_irq_vblank = false;
	bool m_irq_sprite = false;
	attotime m_irq_synctime, m_irq_vsynctime;
	required_device<timer_device> m_irq_timer;
	required_device<timer_device> m_irq_timer_clear;
	//timer_device *m_irq_frc;
	required_device<timer_device> m_frc_cnt_timer;
	uint8_t m_frc_mode = 0;

	bool m_cnt1 = false;

	required_device<segas24_tile_device> m_vtile;
	required_device<segas24_sprite_device> m_vsprite;
	required_device<segas24_mixer_device> m_vmixer;

	void irq_ym(int state);
	uint16_t paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t irq_r(offs_t offset);
	void irq_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t fdc_status_r();
	void fdc_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t curbank_r();
	void curbank_w(uint8_t data);
	uint8_t frc_mode_r();
	void frc_mode_w(uint8_t data);
	uint8_t frc_r();
	void frc_w(uint8_t data);
	uint8_t mlatch_r();
	void mlatch_w(uint8_t data);
	uint16_t iod_r(offs_t offset);
	void iod_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t dcclub_p1_r();
	uint8_t dcclub_p3_r();
	uint8_t mahmajn_input_line_r();
	uint8_t mahmajn_inputs_r();

	void mahmajn_mux_w(uint8_t data);
	void hotrod_lamps_w(uint8_t data);

	void fdc_init();
	void reset_reset();
	void reset_bank();
	void irq_init();
	void irq_timer_sync();
	void irq_timer_start(int old_tmode);
	void cnt1(int state);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer_clear_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_frc_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_vbl);

	// game specific
	TIMER_CALLBACK_MEMBER(gground_hack_timer_callback);
	emu_timer *m_gground_hack_timer = nullptr;
	required_ioport m_p1;
	required_ioport m_p2;
	required_ioport m_p3;
	optional_ioport m_paddle;
	optional_ioport_array<8> m_mj_inputs;

	void common_map(address_map &map) ATTR_COLD;
	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void hotrod_common_map(address_map &map) ATTR_COLD;
	void hotrod_cpu1_map(address_map &map) ATTR_COLD;
	void hotrod_cpu2_map(address_map &map) ATTR_COLD;
	void rombd_common_map(address_map &map) ATTR_COLD;
	void rombd_cpu1_map(address_map &map) ATTR_COLD;
	void rombd_cpu2_map(address_map &map) ATTR_COLD;
	void roughrac_common_map(address_map &map) ATTR_COLD;
	void roughrac_cpu1_map(address_map &map) ATTR_COLD;
	void roughrac_cpu2_map(address_map &map) ATTR_COLD;
	void dcclubj_cpu1_map(address_map &map) ATTR_COLD;
	void dcclubj_cpu2_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SEGA_SEGAS24_H
