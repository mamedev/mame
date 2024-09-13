// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    h8325.cpp

    H8/325 family emulation

    TODO:
    - HCSR @ 0xfffe (port 3 handshake)
    - FNCR @ 0xffff (16-bit timer noise canceler)

***************************************************************************/

#include "emu.h"
#include "h8325.h"

DEFINE_DEVICE_TYPE(H83257, h83257_device, "h83257", "Hitachi H8/3257")
DEFINE_DEVICE_TYPE(H83256, h83256_device, "h83256", "Hitachi H8/3256")
DEFINE_DEVICE_TYPE(H8325, h8325_device, "h8325", "Hitachi H8/325")
DEFINE_DEVICE_TYPE(H8324, h8324_device, "h8324", "Hitachi H8/324")
DEFINE_DEVICE_TYPE(H8323, h8323_device, "h8323", "Hitachi H8/323")
DEFINE_DEVICE_TYPE(H8322, h8322_device, "h8322", "Hitachi H8/322")


h8325_device::h8325_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8325_device::map), this)),
	m_intc(*this, "intc"),
	m_port(*this, "port%u", 1),
	m_timer8(*this, "timer8_%u", 0),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_ram_view(*this, "ram_view"),
	m_rom_size(rom_size),
	m_ram_size(ram_size),
	m_md(3)
{
}

h83257_device::h83257_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H83257, tag, owner, clock, 0xf000, 0x800)
{
}

h83256_device::h83256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H83256, tag, owner, clock, 0xc000, 0x800)
{
}

h8325_device::h8325_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H8325, tag, owner, clock, 0x8000, 0x400)
{
}

h8324_device::h8324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H8324, tag, owner, clock, 0x6000, 0x400)
{
}

h8323_device::h8323_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H8323, tag, owner, clock, 0x4000, 0x200)
{
}

h8322_device::h8322_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8325_device(mconfig, H8322, tag, owner, clock, 0x2000, 0x100)
{
}

void h8325_device::map(address_map &map)
{
	if(m_md >= 2)
		map(0x0000, m_rom_size - 1).rom();

	map(0xff80 - m_ram_size, 0xff7f).view(m_ram_view);
	m_ram_view[0](0xff80 - m_ram_size, 0xff7f).ram().share(m_internal_ram);

	map(0xff90, 0xff90).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tcr_r), FUNC(h8325_timer16_channel_device::tcr_w));
	map(0xff91, 0xff91).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tsr_r), FUNC(h8325_timer16_channel_device::tsr_w));
	map(0xff92, 0xff93).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::tcnt_r), FUNC(h8325_timer16_channel_device::tcnt_w));
	map(0xff94, 0xff95).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::ocra_r), FUNC(h8325_timer16_channel_device::ocra_w));
	map(0xff96, 0xff97).rw(m_timer16_0, FUNC(h8325_timer16_channel_device::ocrb_r), FUNC(h8325_timer16_channel_device::ocrb_w));
	map(0xff98, 0xff99).r(m_timer16_0, FUNC(h8325_timer16_channel_device::icr_r));

	map(0xffb0, 0xffb0).rw(m_port[0], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb1, 0xffb1).rw(m_port[1], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb2, 0xffb2).rw(m_port[0], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb3, 0xffb3).rw(m_port[1], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb4, 0xffb4).rw(m_port[2], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb5, 0xffb5).rw(m_port[3], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb6, 0xffb6).rw(m_port[2], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb7, 0xffb7).rw(m_port[3], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb8, 0xffb8).rw(m_port[4], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb9, 0xffb9).rw(m_port[5], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffba, 0xffba).rw(m_port[4], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbb, 0xffbb).rw(m_port[5], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbc, 0xffbc).rw(m_port[6], FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffbe, 0xffbe).rw(m_port[6], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(0xffc4, 0xffc4).rw(FUNC(h8325_device::syscr_r), FUNC(h8325_device::syscr_w));
	map(0xffc5, 0xffc5).r(FUNC(h8325_device::mdcr_r));
	map(0xffc6, 0xffc6).lr8(NAME([this]() { return m_intc->iscr_r() | ~0x77; }));
	map(0xffc6, 0xffc6).lw8(NAME([this](u8 data) { m_intc->iscr_w(data & 0x77); }));
	map(0xffc7, 0xffc7).lr8(NAME([this]() { return m_intc->ier_r() | ~0x07; }));
	map(0xffc7, 0xffc7).lw8(NAME([this](u8 data) { m_intc->ier_w(data & 0x07); }));

	map(0xffc8, 0xffc8).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd0, 0xffd0).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(0xffd8, 0xffd8).lr8(NAME([this]() { return m_sci[0]->smr_r() | 0x04; }));
	map(0xffd8, 0xffd8).lw8(NAME([this](u8 data) { m_sci[0]->smr_w(data & ~0x04); }));
	map(0xffd9, 0xffd9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).lr8(NAME([this]() { return m_sci[0]->scr_r() | 0x0c; }));
	map(0xffda, 0xffda).lw8(NAME([this](u8 data) { m_sci[0]->scr_w(data & ~0x0c); }));
	map(0xffdb, 0xffdb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).lr8(NAME([this]() { return m_sci[0]->ssr_r() | 0x07; }));
	map(0xffdc, 0xffdc).lw8(NAME([this](u8 data) { m_sci[0]->ssr_w(data & ~0x07); }));
	map(0xffdd, 0xffdd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));

	map(0xffe0, 0xffe0).lr8(NAME([this]() { return m_sci[1]->smr_r() | 0x04; }));
	map(0xffe0, 0xffe0).lw8(NAME([this](u8 data) { m_sci[1]->smr_w(data & ~0x04); }));
	map(0xffe1, 0xffe1).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffe2, 0xffe2).lr8(NAME([this]() { return m_sci[1]->scr_r() | 0x0c; }));
	map(0xffe2, 0xffe2).lw8(NAME([this](u8 data) { m_sci[1]->scr_w(data & ~0x0c); }));
	map(0xffe3, 0xffe3).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffe4, 0xffe4).lr8(NAME([this]() { return m_sci[1]->ssr_r() | 0x07; }));
	map(0xffe4, 0xffe4).lw8(NAME([this](u8 data) { m_sci[1]->ssr_w(data & ~0x07); }));
	map(0xffe5, 0xffe5).r(m_sci[1], FUNC(h8_sci_device::rdr_r));
}

void h8325_device::device_add_mconfig(machine_config &config)
{
	H8325_INTC(config, m_intc, *this);
	H8_PORT(config, m_port[0], *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port[1], *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port[2], *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port[3], *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port[4], *this, h8_device::PORT_5, 0x00, 0xc0);
	H8_PORT(config, m_port[5], *this, h8_device::PORT_6, 0x00, 0x80);
	H8_PORT(config, m_port[6], *this, h8_device::PORT_7, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, m_timer8[0], *this, m_intc, 12, 13, 14, 8, 8, 64, 64, 1024, 1024);
	H8_TIMER8_CHANNEL(config, m_timer8[1], *this, m_intc, 15, 16, 17, 8, 8, 64, 64, 1024, 1024);
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

void h8325_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer8[0]->internal_update(current_time));
	add_event(event_time, m_timer8[1]->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8325_device::notify_standby(int state)
{
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_timer8[0]->notify_standby(state);
	m_timer8[1]->notify_standby(state);
	m_timer16_0->notify_standby(state);
}

void h8325_device::device_start()
{
	h8_device::device_start();

	m_mds = 0;
	m_syscr = 0;

	save_item(NAME(m_md));
	save_item(NAME(m_mds));
	save_item(NAME(m_syscr));
}

void h8325_device::device_reset()
{
	h8_device::device_reset();

	m_syscr = 0x01;
	m_ram_view.select(0);

	// MD pins are latched at reset
	m_mds = m_md;
}

u8 h8325_device::syscr_r()
{
	return m_syscr | 0x0a;
}

void h8325_device::syscr_w(u8 data)
{
	logerror("syscr = %02x\n", data);

	// RAME
	if(data & 1)
		m_ram_view.select(0);
	else
		m_ram_view.disable();

	// NMIEG
	m_intc->set_nmi_edge(BIT(data, 2));

	// SSBY
	m_standby_pending = bool(data & 0x80);

	m_syscr = data;
}

u8 h8325_device::mdcr_r()
{
	if(!machine().side_effects_disabled())
		logerror("mdcr_r\n");
	return (m_mds & 0x03) | 0xe4;
}
