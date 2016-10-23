// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_port.h

    H8 8 bits digital port


***************************************************************************/

#ifndef __H8_PORT_H__
#define __H8_PORT_H__

#include "h8.h"

#define MCFG_H8_PORT_ADD( _tag, address, ddr, mask )    \
	MCFG_DEVICE_ADD( _tag, H8_PORT, 0 ) \
	downcast<h8_port_device *>(device)->set_info(address, ddr, mask);

class h8_port_device : public device_t {
public:
	h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_info(int address, uint8_t default_ddr, uint8_t mask);

	void ddr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pcr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pcr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void odr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t odr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	required_device<h8_device> cpu;
	address_space *io;

	int address;
	uint8_t default_ddr, ddr, pcr, odr;
	uint8_t mask;
	uint8_t dr;
	uint8_t last_output;

	virtual void device_start() override;
	virtual void device_reset() override;
	void update_output();
};

extern const device_type H8_PORT;

#endif
