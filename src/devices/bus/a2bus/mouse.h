// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    mouse.h

    Implemention of the Apple II Mouse Card

*********************************************************************/

#ifndef __A2BUS_MOUSE__
#define __A2BUS_MOUSE__

#include "emu.h"
#include "a2bus.h"
#include "machine/6821pia.h"
#include "cpu/m6805/m6805.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mouse_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mouse_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a2bus_mouse_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_READ8_MEMBER(pia_in_a);
	DECLARE_READ8_MEMBER(pia_in_b);
	DECLARE_WRITE8_MEMBER(pia_out_a);
	DECLARE_WRITE8_MEMBER(pia_out_b);
	DECLARE_WRITE_LINE_MEMBER(pia_irqa_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irqb_w);

	DECLARE_READ8_MEMBER(mcu_port_a_r);
	DECLARE_READ8_MEMBER(mcu_port_b_r);
	DECLARE_READ8_MEMBER(mcu_port_c_r);
	DECLARE_WRITE8_MEMBER(mcu_port_a_w);
	DECLARE_WRITE8_MEMBER(mcu_port_b_w);
	DECLARE_WRITE8_MEMBER(mcu_port_c_w);
	DECLARE_WRITE8_MEMBER(mcu_ddr_a_w);
	DECLARE_WRITE8_MEMBER(mcu_ddr_b_w);
	DECLARE_WRITE8_MEMBER(mcu_ddr_c_w);
	DECLARE_READ8_MEMBER(mcu_timer_r);
	DECLARE_WRITE8_MEMBER(mcu_timer_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;

	required_device<pia6821_device> m_pia;
	required_device<m68705_device> m_mcu;
	required_ioport m_mouseb, m_mousex, m_mousey;

private:
	UINT8 *m_rom;
	bool m_started;
	int m_rom_bank;
	UINT8 m_ddr_a;
	UINT8 m_ddr_b;
	UINT8 m_ddr_c;
	UINT8 m_port_a_out;
	UINT8 m_port_b_out;
	UINT8 m_port_c_out;
	UINT8 m_port_a_in;
	UINT8 m_port_b_in;
	UINT8 m_port_c_in;
	UINT8 m_timer_cnt;
	UINT8 m_timer_ctl;
	UINT8 m_mask_option;
	int last_mx, last_my, count_x, count_y;
	emu_timer *m_timer, *m_read_timer;
};

// device type definition
extern const device_type A2BUS_MOUSE;

#endif /* __A2BUS_MOUSE__ */
