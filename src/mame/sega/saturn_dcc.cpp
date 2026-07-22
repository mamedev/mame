// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sega Saturn DCC dual CPU i/f

Essentially does two things
- handles dual CPU MINIT/SINIT signals;
- handles Slave CPU h/vblank irq vectors;

TODO:
- find the actual part #;
- MINIT/SINIT writes must be 16-bit and checked against who's accessing it, TBD otherwise;
- should probably handle sync barriers from here as well;
- complete irq acknowledge support for slave CPU;

Known games to be very tight on interleaving, acts bad with -drc:
(not necessarily DCC fault)
- blastwnd (before FMV, wants an INTERLEAVE_DURATION of zero somehow)
- choroqpk (car selection)
  060311E4: MOV.L R14,@-SP ;R14 = 0x60ffba0 / R15 = 0x60ffba0
  060311E6: MOV SP,R14 ;R14 = 0x60ffba0 / R15 = 0x60ffb9c / [0x60ffb9c] <- 0x60ffba0
  060311E8: MOV.L @SP+,R14 ;R14 = 0x60ffb9c / R15 = 0x60ffb9c / [0x60ffb9c] -> R14
  060311EA: RTS ;R14 = 0x60ffba0 / R15 = 0x60ffba0
  060311EC: NOP
  06031734: MULS.W R9, R8 ;R14 = 0x60ffba0 / R15 = 0x60ffba0 / EA = 0x60311E4
  on DRC this becomes:
  R14 0x6031b78 (cause of the crash later on), R15 = 0x60ffba4 and EA = 0
- falcom1 Dragon Slayer I (during game loading, slave pitfalls and brings everything else to death)
- shinreij (selecting options menu, wants specifically a zero in INTERLEAVE_DURATION
  but that breaks initial game loading)
- stv:smleague/finlarch
- stv:twcup98 (https://mametesters.org/view.php?id=9491)

**************************************************************************************************/

#include "emu.h"
#include "saturn_dcc.h"

DEFINE_DEVICE_TYPE(SATURN_DCC, saturn_dcc_device, "saturn_dcc", "Sega Saturn DCC Dual CPU interface")

saturn_dcc_device::saturn_dcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_DCC, tag, owner, clock)
	, m_master_cpu(*this, finder_base::DUMMY_TAG)
	, m_slave_cpu(*this, finder_base::DUMMY_TAG)
{
}

void saturn_dcc_device::device_validity_check(validity_checker &valid) const
{
	if (!this->clock())
		osd_printf_error("%s: clock needs to be set\n", this->tag());
}


void saturn_dcc_device::device_start()
{

}

void saturn_dcc_device::device_reset()
{

}

void saturn_dcc_device::minit_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask != 0x0000ffff && mem_mask != 0xffff0000)
		popmessage("saturn_dcc.cpp: minit_w non 16-bit access %08x", mem_mask);

	// printf("MINIT write = %08x %08x\n", data, mem_mask);
	machine().scheduler().add_quantum(attotime::from_hz(this->clock() / INTERLEAVE_DIV), attotime::from_usec(INTERLEAVE_DURATION));
	machine().scheduler().synchronize(
		timer_expired_delegate(FUNC(saturn_dcc_device::handle_frt_cb), this), 1);
}

void saturn_dcc_device::sinit_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask != 0x0000ffff && mem_mask != 0xffff0000)
		popmessage("saturn_dcc.cpp: sinit_w non 16-bit access %08x", mem_mask);
	// printf("SINIT write = %08x %08x\n", data, mem_mask);
	machine().scheduler().add_quantum(attotime::from_hz(this->clock() / INTERLEAVE_DIV), attotime::from_usec(INTERLEAVE_DURATION));
	machine().scheduler().synchronize(
		timer_expired_delegate(FUNC(saturn_dcc_device::handle_frt_cb), this), 0);
}

void saturn_dcc_device::handle_frt_cb(s32 param)
{
	if (param)
		m_slave_cpu->pulse_frt_input();
	else
		m_master_cpu->pulse_frt_input();
	//printf("%d\n", param);
}

IRQ_CALLBACK_MEMBER(saturn_dcc_device::irq_ack_cb)
{
	m_slave_cpu->set_input_line(irqline, CLEAR_LINE);
	// TODO: can also be 0x42 and 0xff (?)
	return (irqline == 6) ? 0x43 : 0x41;
}
