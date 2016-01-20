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
	h8_port_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void set_info(int address, UINT8 default_ddr, UINT8 mask);

	DECLARE_WRITE8_MEMBER(ddr_w);
	DECLARE_WRITE8_MEMBER(dr_w);
	DECLARE_READ8_MEMBER(dr_r);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(pcr_w);
	DECLARE_READ8_MEMBER(pcr_r);
	DECLARE_WRITE8_MEMBER(odr_w);
	DECLARE_READ8_MEMBER(odr_r);

protected:
	required_device<h8_device> cpu;
	address_space *io;

	int address;
	UINT8 default_ddr, ddr, pcr, odr;
	UINT8 mask;
	UINT8 dr;
	UINT8 last_output;

	virtual void device_start() override;
	virtual void device_reset() override;
	void update_output();
};

extern const device_type H8_PORT;

#endif
