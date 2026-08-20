// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco.h

    TRS-80 Radio Shack Color Computer Family

***************************************************************************/

#ifndef MAME_TRS_COCO_H
#define MAME_TRS_COCO_H

#include "emu.h"
#pragma once

#include "coco_vhd.h"

#include "bus/coco/coco_dwsock.h"
#include "bus/coco/cococart.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "machine/mc14529.h"
#include "machine/ram.h"
#include "sound/dac.h"

#include "screen.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

INPUT_PORTS_EXTERN( coco_analog_control );
INPUT_PORTS_EXTERN( coco_joystick );
INPUT_PORTS_EXTERN( coco_beckerport );

void coco_cart(device_slot_interface &device);

// constants
#define JOYSTICK_DELTA          10
#define JOYSTICK_SENSITIVITY    100

// devices
#define MAINCPU_TAG                 "maincpu"
#define RS232_TAG                   "rs232"

// inputs
#define CTRL_SEL_RIGHT              "ctrl_sel_right"
#define CTRL_SEL_LEFT               "ctrl_sel_left"
#define BECKERPORT_TAG              "beckerport"
#define JOYSTICK_RX_TAG             "joystick_rx"
#define JOYSTICK_RY_TAG             "joystick_ry"
#define JOYSTICK_LX_TAG             "joystick_lx"
#define JOYSTICK_LY_TAG             "joystick_ly"
#define JOYSTICK_BUTTONS_TAG        "joystick_buttons"
// #define RAT_MOUSE_RX_TAG            "rat_mouse_rx"
// #define RAT_MOUSE_RY_TAG            "rat_mouse_ry"
// #define RAT_MOUSE_LX_TAG            "rat_mouse_lx"
// #define RAT_MOUSE_LY_TAG            "rat_mouse_ly"
#define RAT_MOUSE_BUTTONS_TAG       "rat_mouse_buttons"
#define DIECOM_LIGHTGUN_RX_TAG      "dclg_rx"
#define DIECOM_LIGHTGUN_RY_TAG      "dclg_ry"
#define DIECOM_LIGHTGUN_BUTTONS_TAG "dclg_triggers"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class coco_joy_handler;

class coco_state : public driver_device, public device_cococart_host_interface
{
public:
	enum {
		JOY_DEVICE_STANDARD    = 0,
		JOY_DEVICE_TANDY_HIRES = 1,
		JOY_DEVICE_CM3_HIRES   = 2,
		JOY_DEVICE_RAT_MOUSE   = 3,
		JOY_DEVICE_DIECOM_LG   = 4,
		JOY_DEVICE_UNCONNECTED = 5,
	};

	coco_state(const machine_config &mconfig, device_type type, const char *tag);

	// IO
	virtual void ff20_write(offs_t offset, uint8_t data);
	virtual uint8_t ff40_read(offs_t offset);
	virtual void ff40_write(offs_t offset, uint8_t data);
	uint8_t ff60_read(offs_t offset);
	void ff60_write(offs_t offset, uint8_t data);

	// PIA0
	void pia0_pa_w(uint8_t value);
	void pia0_pb_w(uint8_t value);
	void pia0_pa7_w(uint8_t value);

	// PIA1
	uint8_t pia1_pa_r();
	uint8_t pia1_pb_r();
	virtual void pia1_pa_w(uint8_t data);
	virtual void pia1_pb_w(uint8_t data);

	// joystick handling
	DECLARE_INPUT_CHANGED_MEMBER(joystick_changed);
	DECLARE_INPUT_CHANGED_MEMBER(joystick_button_changed);
	DECLARE_INPUT_CHANGED_MEMBER(joystick_mode_changed);
	std::unique_ptr<coco_joy_handler> m_joy_handlers[2];
	void write_joystick_mux(int slot, uint8_t val);
	void adjust_host_joy_timer(int target_slot, attotime duration);
	screen_device *get_screen() { return m_screen; }
	mc14529_device *get_mux() {return m_mux; }

	// floating bus & "space"
	uint8_t floating_bus_r()   { return floating_bus_read(); }
	uint8_t floating_space_read(offs_t offset);
	void floating_space_write(offs_t offset, uint8_t data);

	// cartridge stuff
	void cart_w(int state) { cart_w((bool) state); }
	virtual address_space &cartridge_space() override;
	virtual void add_sound_route(device_sound_interface &sound_device, int output_index, double gain) override;
	virtual void set_sound_gain(device_sound_interface &sound_device, int output_index, double gain) override;

	// disassembly override
	static offs_t os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	void coco_sound(machine_config &config);
	void coco_floating(machine_config &config);

	void coco_floating_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	emu_timer *m_joy_timer;
 	TIMER_CALLBACK_MEMBER(joy_timer_callback);

	// accessors
	pia6821_device &pia_0() { return *m_pia_0; }
	pia6821_device &pia_1() { return *m_pia_1; }
	cococart_slot_device &cococart() { return *m_cococart; }
	ram_device &ram() { return *m_ram; }

	// miscellaneous
	virtual void cart_w(bool state);
	virtual void update_cart_base(uint8_t *cart_base) { }

	// PIA0 PA input mirror
	uint8_t m_pia0_pa_buffer;
	uint8_t m_pia0_pb_buffer;

	// VHD selection
	coco_vhd_image_device *current_vhd();

	// floating bus
	uint8_t floating_bus_read();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_0;
	required_device<pia6821_device> m_pia_1;
	required_device<mc14529_device> m_mux;
	required_device<dac_byte_interface> m_dac;
	required_device<dac_1bit_device> m_sbs;
	optional_device<screen_device> m_screen;
	required_device<cococart_slot_device> m_cococart;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<address_map_bank_device> m_floating;
	optional_device<rs232_port_device> m_rs232;
	optional_device<coco_vhd_image_device> m_vhd_0;
	optional_device<coco_vhd_image_device> m_vhd_1;
	optional_device<beckerport_device> m_beckerport;
	optional_ioport m_beckerportconfig;
	required_device<input_merger_device> m_irqs;
	required_device<input_merger_device> m_firqs;
	required_ioport_array<4> m_joystick_ports;

	// input ports
	void update_input_port(int port, uint8_t selection);
	virtual const std::type_info& get_type_info_for_selection(uint8_t selection);
	virtual std::unique_ptr<coco_joy_handler> make_joy_handler(uint8_t selection, int port);
	required_ioport_array<7> m_keyboard;
	optional_ioport m_joystick_type_right;
	optional_ioport m_joystick_type_left;

	// keyboard handling
	void refresh_keyboard_matrix();
	virtual void on_keyboard_state_changed(bool any_pressed) { } // Empty default for CoCo 1/2

	// DAC output
	uint8_t m_dac_output;

	// VHD selection
	uint8_t m_vhd_select = 0U;

	// safety to prevent stack overflow when reading floating bus
	bool m_in_floating_bus_read = false;

private:
	int current_joystick_value(uint8_t mux_value);
};


//**************************************************************************
//  coco_joy_handler - classes for things that plug into the joystick port
//                     and sometimes casette / serial
//**************************************************************************

class coco_joy_handler
{
protected:
	coco_state &m_host;
	int m_base_slot;
	ioport_port *m_buttons;

public:
	coco_joy_handler(coco_state &host, int base_slot, ioport_port *buttons);

	virtual ~coco_joy_handler() = default;
	virtual void joy_changed(int axis, int joy_val) {}
	virtual bool evaluate_comparator(int dac, int joy_val);
	virtual uint8_t button_status();
	virtual void hires_trigger(uint8_t state, attotime current_time, int axis, int joy_val) {};
	virtual void saturated(s32 target_slot) {}
	virtual void lightgun_clock(int clock) {}
};

class coco_joy_disconnected : public coco_joy_handler
{
public:
	using coco_joy_handler::coco_joy_handler;
};

class coco_joy_standard : public coco_joy_handler
{
public:
	using coco_joy_handler::coco_joy_handler;
	virtual void joy_changed(int axis, int joy_val) override;
};

class coco_tandy_hires_joy : public coco_joy_handler
{
public:
	coco_tandy_hires_joy(coco_state &host, int base_slot, ioport_port *buttons);

	virtual void hires_trigger(uint8_t state, attotime current_time, int axis, int joy_val) override;
	virtual void saturated(s32 target_slot) override;
	virtual bool evaluate_comparator(int dac, int joy_val) override;

protected:
	double m_multiplier;
	double m_offset;
	bool m_was_low;
	attotime m_charge_start_time;
};

class coco_cm3_hires_joy : public coco_tandy_hires_joy
{
public:
	coco_cm3_hires_joy(coco_state &host, int base_slot, ioport_port *buttons);
};

class coco_diecom_light_gun : public coco_joy_handler
{
public:
	coco_diecom_light_gun(coco_state &host, int base_slot, ioport_port *buttons, ioport_port *h_port, ioport_port *v_port);

	virtual void lightgun_clock(int clock) override;
	virtual void saturated(s32 target_slot) override;

protected:
	static const int dclg_table[];

	ioport_port *m_h_port;
	ioport_port *m_v_port;
	uint8_t m_output_v;
	uint8_t m_output_h;
	int m_previous_bit;
	uint32_t m_adaptor_state;
	uint32_t m_horizontal_clock_count;
};

#endif // MAME_TRS_COCO_H
