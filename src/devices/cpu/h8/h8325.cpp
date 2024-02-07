// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8325.cpp

    H8/325 family emulation

    TODO:
    - serial controllers are slightly different

***************************************************************************/

#include "emu.h"
#include "h8325.h"

DEFINE_DEVICE_TYPE(H83257, h83257_device, "h83257", "Hitachi H8/3257")
DEFINE_DEVICE_TYPE(H83256, h83256_device, "h83256", "Hitachi H8/3256")
DEFINE_DEVICE_TYPE(H8325, h8325_device, "h8325", "Hitachi H8/325")
DEFINE_DEVICE_TYPE(H8324, h8324_device, "h8324", "Hitachi H8/324")
DEFINE_DEVICE_TYPE(H8323, h8323_device, "h8323", "Hitachi H8/323")
DEFINE_DEVICE_TYPE(H8322, h8322_device, "h8322", "Hitachi H8/322")


h8325_device::h8325_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8325_device::map), this)),
	m_intc(*this, "intc"),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_port5(*this, "port5"),
	m_port6(*this, "port6"),
	m_port7(*this, "port7"),
	m_timer8_0(*this, "timer8_0"),
	m_timer8_1(*this, "timer8_1"),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_syscr(0),
	m_ram_start(start)
{
}

h83257_device::h83257_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H83257, tag, owner, clock, 0xf780)
{
}

h83256_device::h83256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H83256, tag, owner, clock, 0xf780)
{
}

h8325_device::h8325_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H8325, tag, owner, clock, 0xfb80)
{
}

h8324_device::h8324_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H8324, tag, owner, clock, 0xfb80)
{
}

h8323_device::h8323_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H8323, tag, owner, clock, 0xfd80)
{
}

h8322_device::h8322_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8325_device(mconfig, H8322, tag, owner, clock, 0xfe80)
{
}

void h8325_device::map(address_map &map)
{
	map(m_ram_start, 0xff7f).ram();

	map(0xff90, 0xff90).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tcr_r), FUNC(h8325_timer16_channel_device::tcr_w));
	map(0xff91, 0xff91).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tsr_r), FUNC(h8325_timer16_channel_device::tsr_w));
	map(0xff92, 0xff93).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tcnt_r), FUNC(h8325_timer16_channel_device::tcnt_w));
	map(0xff94, 0xff95).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::ocra_r), FUNC(h8325_timer16_channel_device::ocra_w));
	map(0xff96, 0xff97).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::ocrb_r), FUNC(h8325_timer16_channel_device::ocrb_w));
	map(0xff98, 0xff99).r(m_timer16_0, FUNC(h8325_timer16_channel_device::icr_r));

	map(0xffb0, 0xffb0).w(m_port1, FUNC(h8_port_device::ddr_w));
	map(0xffb1, 0xffb1).w(m_port2, FUNC(h8_port_device::ddr_w));
	map(0xffb2, 0xffb2).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb3, 0xffb3).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb4, 0xffb4).w(m_port3, FUNC(h8_port_device::ddr_w));
	map(0xffb5, 0xffb5).w(m_port4, FUNC(h8_port_device::ddr_w));
	map(0xffb6, 0xffb6).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb7, 0xffb7).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb8, 0xffb8).w(m_port5, FUNC(h8_port_device::ddr_w));
	map(0xffb9, 0xffb9).w(m_port6, FUNC(h8_port_device::ddr_w));
	map(0xffba, 0xffba).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbb, 0xffbb).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbc, 0xffbc).w(m_port7, FUNC(h8_port_device::ddr_w));
	map(0xffbe, 0xffbe).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(0xffc4, 0xffc4).rw(FUNC(h8325_device::syscr_r), FUNC(h8325_device::syscr_w));
	map(0xffc5, 0xffc5).r(FUNC(h8325_device::mdcr_r));
	map(0xffc6, 0xffc6).rw(m_intc, FUNC(h8_intc_device::iscr_r), FUNC(h8_intc_device::iscr_w));
	map(0xffc7, 0xffc7).rw(m_intc, FUNC(h8_intc_device::ier_r), FUNC(h8_intc_device::ier_w));
	map(0xffc8, 0xffc8).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd0, 0xffd0).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(0xffd8, 0xffd8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffd9, 0xffd9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffdb, 0xffdb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffdd, 0xffdd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(0xffe0, 0xffe0).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffe1, 0xffe1).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffe2, 0xffe2).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffe3, 0xffe3).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffe4, 0xffe4).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffe5, 0xffe5).r(m_sci[1], FUNC(h8_sci_device::rdr_r));
}

void h8325_device::device_add_mconfig(machine_config &config)
{
	H8325_INTC(config, m_intc, *this);
	H8_PORT(config, m_port1, *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port2, *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port3, *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0x00, 0xc0);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x00, 0x80);
	H8_PORT(config, m_port7, *this, h8_device::PORT_7, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, m_timer8_0, *this, m_intc, 12, 13, 14, 8, 8, 64, 64, 1024, 1024);
	H8_TIMER8_CHANNEL(config, m_timer8_1, *this, m_intc, 15, 16, 17, 8, 8, 64, 64, 1024, 1024);
	H8_TIMER16(config, m_timer16, *this, 1, 0xff);
	H8325_TIMER16_CHANNEL(config, m_timer16_0, *this, m_intc, 8);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 18, 19, 20, 20);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 21, 22, 23, 23);
}

void h8325_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h8325_device::irq_setup()
{
	m_CCR |= F_I;
}

void h8325_device::update_irq_filter()
{
	if(m_CCR & F_I)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void h8325_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h8325_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer8_0->internal_update(current_time));
	add_event(event_time, m_timer8_1->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8325_device::device_start()
{
	h8_device::device_start();
	save_item(NAME(m_syscr));
}

void h8325_device::device_reset()
{
	h8_device::device_reset();
	m_syscr = 0x01;
}

uint8_t h8325_device::syscr_r()
{
	return m_syscr | 0x0a;
}

void h8325_device::syscr_w(uint8_t data)
{
	logerror("syscr = %02x\n", data);
	m_syscr = data;
}

uint8_t h8325_device::mdcr_r()
{
	if(!machine().side_effects_disabled())
		logerror("mdcr_r\n");
	return 0xe7;
}
