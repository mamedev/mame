// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco.h

    TRS-80 Radio Shack Color Computer Family

***************************************************************************/

#pragma once

#ifndef __COCO__
#define __COCO__


#include "emu.h"
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "machine/6821pia.h"
#include "bus/coco/cococart.h"
#include "machine/coco_vhd.h"
#include "bus/coco/coco_dwsock.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/wave.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

INPUT_PORTS_EXTERN( coco_analog_control );
INPUT_PORTS_EXTERN( coco_cart_autostart );
INPUT_PORTS_EXTERN( coco_rtc );
INPUT_PORTS_EXTERN( coco_beckerport );

SLOT_INTERFACE_EXTERN( coco_cart );

// constants
#define JOYSTICK_DELTA          10
#define JOYSTICK_SENSITIVITY    100

// devices
#define MAINCPU_TAG                 "maincpu"
#define PIA0_TAG                    "pia0"
#define PIA1_TAG                    "pia1"
#define SAM_TAG                     "sam"
#define VDG_TAG                     "vdg"
#define SCREEN_TAG                  "screen"
#define DAC_TAG                     "dac"
#define CARTRIDGE_TAG               "ext"
#define RS232_TAG                   "rs232"
#define DWSOCK_TAG                  "dwsock"
#define VHD0_TAG                    "vhd0"
#define VHD1_TAG                    "vhd1"

// inputs
#define CTRL_SEL_TAG                "ctrl_sel"
#define HIRES_INTF_TAG              "hires_intf"
#define CART_AUTOSTART_TAG          "cart_autostart"
#define BECKERPORT_TAG              "beckerport"
#define JOYSTICK_RX_TAG             "joystick_rx"
#define JOYSTICK_RY_TAG             "joystick_ry"
#define JOYSTICK_LX_TAG             "joystick_lx"
#define JOYSTICK_LY_TAG             "joystick_ly"
#define JOYSTICK_BUTTONS_TAG        "joystick_buttons"
#define RAT_MOUSE_RX_TAG            "rat_mouse_rx"
#define RAT_MOUSE_RY_TAG            "rat_mouse_ry"
#define RAT_MOUSE_LX_TAG            "rat_mouse_lx"
#define RAT_MOUSE_LY_TAG            "rat_mouse_ly"
#define RAT_MOUSE_BUTTONS_TAG       "rat_mouse_buttons"
#define DIECOM_LIGHTGUN_RX_TAG      "dclg_rx"
#define DIECOM_LIGHTGUN_RY_TAG      "dclg_ry"
#define DIECOM_LIGHTGUN_LX_TAG      "dclg_lx"
#define DIECOM_LIGHTGUN_LY_TAG      "dclg_ly"
#define DIECOM_LIGHTGUN_BUTTONS_TAG "dclg_triggers"

MACHINE_CONFIG_EXTERN( coco_sound );



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coco_state : public driver_device
{
public:
	coco_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_0;
	required_device<pia6821_device> m_pia_1;
	required_device<dac_device> m_dac;
	required_device<wave_device> m_wave;
	required_device<cococart_slot_device> m_cococart;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	optional_device<rs232_port_device> m_rs232;
	optional_device<coco_vhd_image_device> m_vhd_0;
	optional_device<coco_vhd_image_device> m_vhd_1;
	optional_device<beckerport_device> m_beckerport;
	optional_ioport                    m_beckerportconfig;

	// driver update handlers
	DECLARE_INPUT_CHANGED_MEMBER(keyboard_changed);
	DECLARE_INPUT_CHANGED_MEMBER(joystick_mode_changed);

	// IO
	virtual DECLARE_READ8_MEMBER( ff00_read );
	virtual DECLARE_WRITE8_MEMBER( ff00_write );
	virtual DECLARE_READ8_MEMBER( ff20_read );
	virtual DECLARE_WRITE8_MEMBER( ff20_write );
	virtual DECLARE_READ8_MEMBER( ff40_read );
	virtual DECLARE_WRITE8_MEMBER( ff40_write );
	DECLARE_READ8_MEMBER( ff60_read );
	DECLARE_WRITE8_MEMBER( ff60_write );

	// PIA0
	DECLARE_WRITE8_MEMBER( pia0_pa_w );
	DECLARE_WRITE8_MEMBER( pia0_pb_w );
	DECLARE_WRITE_LINE_MEMBER( pia0_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( pia0_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( pia0_irq_a );
	DECLARE_WRITE_LINE_MEMBER( pia0_irq_b );

	// PIA1
	DECLARE_READ8_MEMBER( pia1_pa_r );
	DECLARE_READ8_MEMBER( pia1_pb_r );
	DECLARE_WRITE8_MEMBER( pia1_pa_w );
	DECLARE_WRITE8_MEMBER( pia1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_firq_a );
	DECLARE_WRITE_LINE_MEMBER( pia1_firq_b );

	// floating bus
	DECLARE_READ8_MEMBER( floating_bus_read )   { return floating_bus_read(); }

	DECLARE_WRITE_LINE_MEMBER( cart_w ) { cart_w((bool) state); }
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// interrupts
	virtual bool firq_get_line(void);
	virtual bool irq_get_line(void);
	void recalculate_irq(void);
	void recalculate_firq(void);

	// changed handlers
	virtual void pia1_pa_changed(UINT8 data);
	virtual void pia1_pb_changed(UINT8 data);

	// miscellaneous
	virtual void update_keyboard_input(UINT8 value, UINT8 z);
	virtual void cart_w(bool state);
	virtual void update_cart_base(UINT8 *cart_base) = 0;

private:
	// timer constants
	static const device_timer_id TIMER_HIRES_JOYSTICK_X = 0;
	static const device_timer_id TIMER_HIRES_JOYSTICK_Y = 1;
	static const device_timer_id TIMER_DIECOM_LIGHTGUN = 2;

	enum soundmux_status_t
	{
		SOUNDMUX_SEL1 = 1,
		SOUNDMUX_SEL2 = 2,
		SOUNDMUX_ENABLE = 4
	};

	enum joystick_type_t
	{
		JOYSTICK_NONE = 0x00,
		JOYSTICK_NORMAL = 0x01,
		JOYSTICK_RAT_MOUSE = 0x02,
		JOYSTICK_DIECOM_LIGHT_GUN = 0x03
	};

	enum hires_type_t
	{
		HIRES_NONE = 0x00,
		HIRES_RIGHT = 0x01,
		HIRES_RIGHT_COCOMAX3 = 0x02,
		HIRES_LEFT = 0x03,
		HIRES_LEFT_COCOMAX3 = 0x04
	};

	struct analog_input_t
	{
		ioport_port *m_input[2][2];
		ioport_port *m_buttons;

		UINT8 input(int joystick, int axis) const { return m_input[joystick][axis] ? m_input[joystick][axis]->read() : 0x00; }
		UINT8 buttons(void) const { return m_buttons ? m_buttons->read() : 0x00; }
	};

	void analog_port_start(analog_input_t *analog, const char *rx_tag, const char *ry_tag, const char *lx_tag, const char *ly_tag, const char *buttons_tag);

	// wrappers for configuration
	joystick_type_t joystick_type(int index);
	hires_type_t hires_interface_type(void);
	bool is_joystick_hires(int joystick_index);

	soundmux_status_t soundmux_status(void);
	void update_sound(void);
	bool joyin(void);
	void poll_joystick(bool *joyin, UINT8 *buttons);
	void poll_keyboard(void);
	void poll_hires_joystick(void);
	void update_cassout(int cassout);
	void update_prinout(bool prinout);
	void diecom_lightgun_clock(void);

	// thin wrappers for PIA output
	UINT8 dac_output(void)  { return m_dac_output; }    // PA drives the DAC
	bool sel1(void)         { return m_pia_0->ca2_output() ? true : false; }
	bool sel2(void)         { return m_pia_0->cb2_output() ? true : false; }
	bool snden(void)        { return m_pia_1->cb2_output() ? true : false; }

	// VHD selection
	coco_vhd_image_device *current_vhd(void);

	// floating bus
	UINT8 floating_bus_read(void);

	// disassembly override
	static offs_t dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);

	// input ports
	ioport_port *m_keyboard[7];
	ioport_port *m_joystick_type_control;
	ioport_port *m_joystick_hires_control;
	analog_input_t m_joystick;
	analog_input_t m_rat_mouse;
	analog_input_t m_diecom_lightgun;

	// DAC output
	UINT8 m_dac_output;

	// remember the last audio sample level from the analog sources (DAC, cart, cassette) so that we don't
	// introduce step changes when the audio output is enabled/disabled via PIA1 CB2
	UINT8 m_analog_audio_level;

	// hires interface
	emu_timer *m_hiresjoy_transition_timer[2];
	bool m_hiresjoy_ca;

	// diecom lightgun
	emu_timer *m_diecom_lightgun_timer;
	bool m_dclg_previous_bit;
	UINT8 m_dclg_output_h;
	UINT8 m_dclg_output_v;
	UINT8 m_dclg_state;
	UINT16 m_dclg_timer;

	// VHD selection
	UINT8 m_vhd_select;

	// safety to prevent stack overflow when reading floating bus
	bool m_in_floating_bus_read;
};

#endif // __COCO__
