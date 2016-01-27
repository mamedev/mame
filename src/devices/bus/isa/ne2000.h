// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __NE2000_H__
#define __NE2000_H__

#include "emu.h"
#include "isa.h"
#include "machine/dp8390.h"

class ne2000_device: public device_t,
						public device_isa16_card_interface
{
public:
	ne2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void ne2000_irq_w(int state);
	DECLARE_READ8_MEMBER(ne2000_mem_read);
	DECLARE_WRITE8_MEMBER(ne2000_mem_write);
	DECLARE_READ16_MEMBER(ne2000_port_r);
	DECLARE_WRITE16_MEMBER(ne2000_port_w);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	required_device<dp8390d_device> m_dp8390;
	UINT8 m_irq;
	UINT8 m_board_ram[16*1024];
	UINT8 m_prom[16];
};

extern const device_type NE2000;

#endif
