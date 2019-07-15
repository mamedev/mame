// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************

    PC-88VA (c) 1987 NEC

********************************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_PC88VA_H
#define MAME_INCLUDES_PC88VA_H

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
//#include "machine/upd71071.h"
#include "machine/upd765.h"
#include "machine/bankdev.h"
#include "sound/2203intf.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/xdf_dsk.h"

// TODO: for the time being, just disable FDC CPU, it's for PC-8801 compatibility mode anyway.
//       the whole FDC device should be converted (it's also used by PC-9801)
#define TEST_SUBFDC 0



class pc88va_state : public driver_device
{
public:
	enum
	{
		TIMER_PC8801FD_UPD765_TC_TO_ZERO,
		TIMER_T3_MOUSE_CALLBACK,
		TIMER_PC88VA_FDC_TIMER,
		TIMER_PC88VA_FDC_MOTOR_START_0,
		TIMER_PC88VA_FDC_MOTOR_START_1
	};

	pc88va_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_fdc(*this, "upd765"),
		m_fdd(*this, "upd765:%u", 0U),
		m_dmac(*this, "dmac"),
		m_pic1(*this, "pic8259_master"),
		m_pic2(*this, "pic8259_slave"),
		m_palram(*this, "palram"),
		m_sysbank(*this, "sysbank"),
		m_tvram(*this, "tvram"),
		m_gvram(*this, "gvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void pc88va(machine_config &config);

protected:
	struct tsp_t
	{
		uint16_t tvram_vreg_offset;
		uint16_t attr_offset;
		uint16_t spr_offset;
		uint8_t disp_on;
		uint8_t spr_on;
		uint8_t pitch;
		uint8_t line_height;
		uint8_t h_line_pos;
		uint8_t blink;
		uint16_t cur_pos_x,cur_pos_y;
		uint8_t curn;
		uint8_t curn_blink;
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_fdd;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_shared_ptr<uint16_t> m_palram;
	required_device<address_map_bank_device> m_sysbank;
	required_shared_ptr<uint16_t> m_tvram;
	required_shared_ptr<uint16_t> m_gvram;
	uint8_t *m_kanjiram;
	uint16_t m_bank_reg;
	uint16_t m_screen_ctrl_reg;
	uint8_t m_timer3_io_reg;
	emu_timer *m_t3_mouse_timer;
	tsp_t m_tsp;
	uint16_t m_video_pri_reg[2];
	uint8_t m_backupram_wp;
	uint8_t m_cmd;
	uint8_t m_buf_size;
	uint8_t m_buf_index;
	uint8_t m_buf_ram[16];
	uint8_t m_portc_test;
	uint8_t m_fdc_motor_status[2];

	/* floppy state */
	uint8_t m_i8255_0_pc;
	uint8_t m_i8255_1_pc;
	uint8_t m_fdc_mode;
	uint8_t m_fdc_irq_opcode;
	DECLARE_READ16_MEMBER(sys_mem_r);
	DECLARE_WRITE16_MEMBER(sys_mem_w);
	DECLARE_READ8_MEMBER(idp_status_r);
	DECLARE_WRITE8_MEMBER(idp_command_w);
	DECLARE_WRITE8_MEMBER(idp_param_w);
	DECLARE_WRITE16_MEMBER(palette_ram_w);
	DECLARE_READ16_MEMBER(sys_port4_r);
	DECLARE_READ16_MEMBER(bios_bank_r);
	DECLARE_WRITE16_MEMBER(bios_bank_w);
	DECLARE_READ8_MEMBER(rom_bank_r);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE16_MEMBER(backupram_wp_1_w);
	DECLARE_WRITE16_MEMBER(backupram_wp_0_w);
	DECLARE_READ8_MEMBER(kanji_ram_r);
	DECLARE_WRITE8_MEMBER(kanji_ram_w);
	DECLARE_READ8_MEMBER(hdd_status_r);
	#if TEST_SUBFDC
	DECLARE_READ8_MEMBER(upd765_tc_r);
	DECLARE_WRITE8_MEMBER(upd765_mc_w);
	#else
	DECLARE_READ8_MEMBER(no_subfdc_r);
	#endif
	DECLARE_READ8_MEMBER(pc88va_fdc_r);
	DECLARE_WRITE8_MEMBER(pc88va_fdc_w);
	DECLARE_READ16_MEMBER(sysop_r);
	DECLARE_READ16_MEMBER(screen_ctrl_r);
	DECLARE_WRITE16_MEMBER(screen_ctrl_w);
	DECLARE_WRITE8_MEMBER(timer3_ctrl_reg_w);
	DECLARE_WRITE16_MEMBER(video_pri_w);
	DECLARE_READ8_MEMBER(backupram_dsw_r);
	DECLARE_WRITE8_MEMBER(sys_port1_w);
	uint32_t screen_update_pc88va(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pc88va_vrtc_irq);
	DECLARE_READ8_MEMBER(cpu_8255_c_r);
	DECLARE_WRITE8_MEMBER(cpu_8255_c_w);
	DECLARE_READ8_MEMBER(fdc_8255_c_r);
	DECLARE_WRITE8_MEMBER(fdc_8255_c_w);
	DECLARE_READ8_MEMBER(r232_ctrl_porta_r);
	DECLARE_READ8_MEMBER(r232_ctrl_portb_r);
	DECLARE_READ8_MEMBER(r232_ctrl_portc_r);
	DECLARE_WRITE8_MEMBER(r232_ctrl_porta_w);
	DECLARE_WRITE8_MEMBER(r232_ctrl_portb_w);
	DECLARE_WRITE8_MEMBER(r232_ctrl_portc_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(pc88va_pit_out0_changed);
//  DECLARE_WRITE_LINE_MEMBER(pc88va_upd765_interrupt);
	uint8_t m_fdc_ctrl_2;
	TIMER_CALLBACK_MEMBER(pc8801fd_upd765_tc_to_zero);
	TIMER_CALLBACK_MEMBER(t3_mouse_callback);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_timer);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_motor_start_0);
	TIMER_CALLBACK_MEMBER(pc88va_fdc_motor_start_1);
//  uint16_t m_fdc_dma_r();
//  void m_fdc_dma_w(uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(pc88va_hlda_w);
	DECLARE_WRITE_LINE_MEMBER(pc88va_tc_w);
	DECLARE_READ8_MEMBER(fdc_dma_r);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);
	DECLARE_READ8_MEMBER(dma_memr_cb);
	DECLARE_WRITE8_MEMBER(dma_memw_cb);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void pc88va_fdc_update_ready(floppy_image_device *, int);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t calc_kanji_rom_addr(uint8_t jis1,uint8_t jis2,int x,int y);
	void draw_text(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tsp_sprite_enable(uint32_t spr_offset, uint16_t sw_bit);
	void execute_sync_cmd();
	void execute_dspon_cmd();
	void execute_dspdef_cmd();
	void execute_curdef_cmd();
	void execute_actscr_cmd();
	void execute_curs_cmd();
	void execute_emul_cmd();
	void execute_spron_cmd();
	void execute_sprsw_cmd();

	void pc88va_map(address_map &map);
	void pc88va_io_map(address_map &map);
	void sysbank_map(address_map &map);

	void pc88va_z80_io_map(address_map &map);
	void pc88va_z80_map(address_map &map);
protected:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#endif
