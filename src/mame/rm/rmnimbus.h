// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    rmnimbus.c
    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.
*/
#ifndef MAME_RM_RMNIMBUS_H
#define MAME_RM_RMNIMBUS_H

#pragma once

#include "cpu/i86/i186.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "bus/scsi/scsi.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/eepromser.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "bus/centronics/ctronics.h"
#include "emupal.h"
#include "screen.h"

#define MAINCPU_TAG "maincpu"
#define IOCPU_TAG   "iocpu"
#define Z80SIO_TAG  "z80sio"
#define FDC_TAG     "wd2793"
#define SCSIBUS_TAG "scsibus"
#define ER59256_TAG "er59256"
#define AY8910_TAG  "ay8910"
#define MONO_TAG    "mono"
#define MSM5205_TAG "msm5205"
#define VIA_TAG     "via6522"
#define CENTRONICS_TAG "centronics"

/* Mouse / Joystick */

#define JOYSTICK_TAG_BASE       "joystick"
#define JOYSTICK0_TAG           JOYSTICK_TAG_BASE "0"
#define JOYSTICK1_TAG           JOYSTICK_TAG_BASE "1"
#define MOUSE_BUTTON_TAG        "mousebtn"
#define MOUSEX_TAG              "mousex"
#define MOUSEY_TAG              "mousey"

/* Memory controller */
#define RAM_BANK00_TAG  "bank0"
#define RAM_BANK01_TAG  "bank1"
#define RAM_BANK02_TAG  "bank2"
#define RAM_BANK03_TAG  "bank3"
#define RAM_BANK04_TAG  "bank4"
#define RAM_BANK05_TAG  "bank5"
#define RAM_BANK06_TAG  "bank6"
#define RAM_BANK07_TAG  "bank7"

class rmnimbus_state : public driver_device
{
public:
	rmnimbus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_iocpu(*this, IOCPU_TAG),
		m_msm(*this, MSM5205_TAG),
		m_scsibus(*this, SCSIBUS_TAG),
		m_ram(*this, RAM_TAG),
		m_eeprom(*this, ER59256_TAG),
		m_via(*this, VIA_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_palette(*this, "palette"),
		m_scsi_data_out(*this, "scsi_data_out"),
		m_scsi_data_in(*this, "scsi_data_in"),
		m_scsi_ctrl_out(*this, "scsi_ctrl_out"),
		m_fdc(*this, FDC_TAG),
		m_z80sio(*this, Z80SIO_TAG),
		m_screen(*this, "screen"),
		m_io_config(*this, "config"),
		m_io_joysticks(*this, JOYSTICK_TAG_BASE "%u", 0),
		m_io_mouse_button(*this, MOUSE_BUTTON_TAG),
		m_io_mousex(*this, MOUSEX_TAG),
		m_io_mousey(*this, MOUSEY_TAG)
	{
	}

	static constexpr feature_type imperfect_features() { return feature::MOUSE; }

	void nimbus(machine_config &config);

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<i8031_device> m_iocpu;
	required_device<msm5205_device> m_msm;
	required_device<scsi_port_device> m_scsibus;
	required_device<ram_device> m_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<via6522_device> m_via;
	required_device<centronics_device> m_centronics;
	required_device<palette_device> m_palette;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<output_latch_device> m_scsi_ctrl_out;
	required_device<wd2793_device> m_fdc;
	required_device<z80sio_device> m_z80sio;
	required_device<screen_device> m_screen;
	required_ioport m_io_config;
	required_ioport_array<2> m_io_joysticks;
	required_ioport m_io_mouse_button;
	required_ioport m_io_mousex;
	required_ioport m_io_mousey;

	bitmap_ind16 m_video_mem;

	uint8_t m_mcu_reg080 = 0;
	uint8_t m_iou_reg092 = 0;
	uint8_t m_last_playmode = 0;
	uint8_t m_ay8910_a = 0;
	uint8_t m_ay8910_b = 0;
	uint16_t m_x = 0;
	uint16_t m_y = 0;
	uint16_t m_yline = 0;
	uint8_t m_colours = 0;
	uint8_t m_mode = 0;
	uint8_t m_upmode = 0;
	uint32_t m_debug_video = 0;
	uint8_t m_vector = 0;
	uint8_t m_eeprom_bits = 0;
	uint8_t m_eeprom_state = 0;

	uint8_t nimbus_mcu_r();
	void nimbus_mcu_w(uint8_t data);
	uint8_t scsi_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);
	uint8_t fdc_reg_r(offs_t offset);
	void fdc_reg_w(offs_t offset, uint8_t data);
	void fdc_ctl_w(uint8_t data);
	void nimbus_voice_w(offs_t offset, uint8_t data);
	uint8_t nimbus_pc8031_r(offs_t offset);
	void nimbus_pc8031_w(offs_t offset, uint8_t data);
	uint8_t nimbus_pc8031_iou_r(offs_t offset);
	void nimbus_pc8031_iou_w(offs_t offset, uint8_t data);
	uint8_t nimbus_pc8031_port1_r();
	void nimbus_pc8031_port1_w(uint8_t data);
	uint8_t nimbus_pc8031_port3_r();
	void nimbus_pc8031_port3_w(uint8_t data);
	uint8_t nimbus_iou_r(offs_t offset);
	void nimbus_iou_w(offs_t offset, uint8_t data);

	uint8_t nimbus_rompack_r(offs_t offset);
	void nimbus_rompack_w(offs_t offset, uint8_t data);
	void nimbus_sound_ay8910_porta_w(uint8_t data);
	void nimbus_sound_ay8910_portb_w(uint8_t data);
	uint8_t nimbus_joystick_r();
	void nimbus_joystick_select(offs_t offset, uint8_t data);
	uint8_t nimbus_mouse_js_r();
	void nimbus_mouse_js_w(uint8_t data);
	uint16_t nimbus_video_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void nimbus_video_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;
	uint32_t screen_update_nimbus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sio_interrupt(int state);
	void nimbus_fdc_intrq_w(int state);
	void nimbus_fdc_drq_w(int state);
	int nimbus_fdc_enmf_r();

	void nimbus_via_write_portb(uint8_t data);
	void write_scsi_bsy(int state);
	void write_scsi_cd(int state);
	void write_scsi_io(int state);
	void write_scsi_msg(int state);
	void write_scsi_req(int state);
	void nimbus_msm5205_vck(int state);
	void write_scsi_iena(int state);

	uint8_t get_pixel(uint16_t x, uint16_t y);
	uint16_t read_pixel_line(uint16_t x, uint16_t y, uint8_t pixels, uint8_t bpp);
	uint16_t read_pixel_data(uint16_t x, uint16_t y);
	void set_pixel(uint16_t x, uint16_t y, uint8_t colour);
	void set_pixel40(uint16_t x, uint16_t y, uint8_t colour);
	void write_pixel_line(uint16_t x, uint16_t y, uint16_t, uint8_t pixels, uint8_t bpp);
	void move_pixel_line(uint16_t x, uint16_t y, uint8_t width);
	void write_pixel_data(uint16_t x, uint16_t y, uint16_t data);
	void change_palette(uint8_t bank, uint16_t colours);
	void external_int(uint8_t vector, bool state);
	uint8_t cascade_callback();
	void nimbus_bank_memory();
	void memory_reset();
	void fdc_reset();
	uint8_t fdc_driveno(uint8_t drivesel);
	void hdc_reset();
	void hdc_post_rw();
	void hdc_drq(bool state);
	void pc8031_reset();
	//void ipc_dumpregs();
	void iou_reset();
	void rmni_sound_reset();
	void mouse_js_reset();
	void check_scsi_irq();
	void set_scsi_drqlat(bool   clock, bool clear);

	int m_scsi_iena = 0;
	int m_scsi_msg = 0;
	int m_scsi_bsy = 0;
	int m_scsi_io = 0;
	int m_scsi_cd = 0;
	int m_scsi_req = 0;
	int m_scsi_reqlat = 0;

	// Static data related to Floppy and SCSI hard disks
	struct
	{
		uint8_t reg400 = 0;
	} m_nimbus_drives;

	/* 8031 Peripheral controller */
	struct
	{
		uint8_t ipc_in = 0;
		uint8_t ipc_out = 0;
		uint8_t status_in = 0;
		uint8_t status_out = 0;
	} m_ipc_interface;

	/* Mouse */
	struct
	{
		uint16_t xpos_loc = 0;
		uint16_t ypos_loc = 0;
		uint16_t xmin_loc = 0;
		uint16_t ymin_loc = 0;
		uint16_t xmax_loc = 0;
		uint16_t ymax_loc = 0;

		uint8_t m_mouse_x = 0;
		uint8_t m_mouse_y = 0;

		uint8_t m_mouse_pcx = 0;
		uint8_t m_mouse_pcy = 0;

		uint8_t m_intstate_x = 0;
		uint8_t m_intstate_y = 0;

		uint8_t m_reg0a4 = 0;

		emu_timer *m_mouse_timer = nullptr;
	} m_nimbus_mouse;

	uint8_t m_selected_js_idx = 0;

	bool m_voice_enabled = false;

	void nimbus_io(address_map &map) ATTR_COLD;
	void nimbus_iocpu_io(address_map &map) ATTR_COLD;
	void nimbus_iocpu_mem(address_map &map) ATTR_COLD;
	void nimbus_mem(address_map &map) ATTR_COLD;

	void decode_dssi_none(uint16_t ds, uint16_t si);
	void decode_dssi_generic(uint16_t ds, uint16_t si);
	void decode_dssi_f_fill_area(uint16_t ds, uint16_t si);
	void decode_dssi_f_plot_character_string(uint16_t ds, uint16_t si);
	void decode_dssi_f_set_new_clt(uint16_t ds, uint16_t si);
	void decode_dssi_f_plonk_char(uint16_t ds, uint16_t si);
	void decode_dssi_f_rw_sectors(uint16_t ds, uint16_t si);

	void debug_command(const std::vector<std::string_view> &params);
	void video_debug(const std::vector<std::string_view> &params);
	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	TIMER_CALLBACK_MEMBER(do_mouse);
	void do_mouse_real(int8_t xdiff, int8_t ydiff);
	void do_mouse_hle(int8_t xdiff, int8_t ydiff);
};

#endif // MAME_RM_RMNIMBUS_H
