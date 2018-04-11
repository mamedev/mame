// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair



#include "emu.h"
#include "scc2698b.h"

DEFINE_DEVICE_TYPE(SCC2698B, scc2698b_device, "scc2698b", "SCC2698B Octal UART")



scc2698b_device::scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCC2698B, tag, owner, clock)
{
}



void scc2698b_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{

}


void scc2698b_device::device_start()
{

}


void scc2698b_device::device_reset()
{

}



uint8_t scc2698b_device::read_reg(int offset)
{
	return 0;
}


WRITE8_MEMBER(scc2698b_device::reg_w )
{
	write_reg(offset & 3, data);
}



void scc2698b_device::write_reg(int offset, uint8_t data)
{

}
