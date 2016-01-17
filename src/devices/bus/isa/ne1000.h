// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __NE1000_H__
#define __NE1000_H__

// NE1000 is 8bit has 8KB ram; NE2000 is 16bit has 16KB ram

#include "emu.h"
#include "isa.h"
#include "machine/dp8390.h"

class ne1000_device: public device_t,
						public device_isa8_card_interface
{
public:
	ne1000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void ne1000_irq_w(int state);
	DECLARE_READ8_MEMBER(ne1000_mem_read);
	DECLARE_WRITE8_MEMBER(ne1000_mem_write);
	DECLARE_READ8_MEMBER(ne1000_port_r);
	DECLARE_WRITE8_MEMBER(ne1000_port_w);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	required_device<dp8390d_device> m_dp8390;
	UINT8 m_irq;
	UINT8 m_board_ram[8*1024];
	UINT8 m_prom[16];
};

extern const device_type NE1000;

#endif
