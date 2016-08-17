// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8_port.h"

const device_type H8_PORT = &device_creator<h8_port_device>;

h8_port_device::h8_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_PORT, "H8 digital port", tag, owner, clock, "h8_digital_port", __FILE__),
	cpu(*this, DEVICE_SELF_OWNER), io(nullptr), address(0), default_ddr(0), ddr(0), pcr(0), odr(0), mask(0), dr(0), last_output(0)
{
}

void h8_port_device::set_info(int _address, UINT8 _default_ddr, UINT8 _mask)
{
	address = 2*_address;
	default_ddr = _default_ddr;
	mask = _mask;
}

WRITE8_MEMBER(h8_port_device::ddr_w)
{
	//  logerror("ddr_w %02x\n", data);
	ddr = data;
	update_output();
}

WRITE8_MEMBER(h8_port_device::dr_w)
{
	//  logerror("dr_w %02x\n", data);
	dr = data;
	update_output();
}

READ8_MEMBER(h8_port_device::dr_r)
{
	//  logerror("dr_r %02x\n", (dr | mask) & 0xff);
	return dr | mask;
}

READ8_MEMBER(h8_port_device::port_r)
{
	UINT8 res = mask | (dr & ddr);
	if((ddr & ~mask) != UINT8(~mask))
		res |= io->read_word(address) & ~ddr;

	//  logerror("port_r %02x (%02x %02x)\n", res, ddr & ~mask, UINT8(~mask));
	return res;
}

WRITE8_MEMBER(h8_port_device::pcr_w)
{
	logerror("pcr_w %02x\n", data);
	pcr = data;
}

READ8_MEMBER(h8_port_device::pcr_r)
{
	logerror("dr_r %02x\n", (pcr | mask) & 0xff);
	return pcr | mask;
}

WRITE8_MEMBER(h8_port_device::odr_w)
{
	logerror("odr_w %02x\n", data);
	odr = data;
}

READ8_MEMBER(h8_port_device::odr_r)
{
	logerror("odr_r %02x\n", (odr | mask) & 0xff);
	return odr | ~mask;
}

void h8_port_device::update_output()
{
	UINT8 res = dr & ddr & ~mask;
	if(res != last_output) {
		last_output = res;
		io->write_word(address, res);
	}
}

void h8_port_device::device_start()
{
	io = &cpu->space(AS_IO);
	save_item(NAME(ddr));
	save_item(NAME(dr));
	save_item(NAME(pcr));
	save_item(NAME(odr));
	save_item(NAME(last_output));
	last_output = 0;
}

void h8_port_device::device_reset()
{
	dr = 0;
	ddr = default_ddr;
	pcr = 0;
	odr = 0;
	update_output();
}
