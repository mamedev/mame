// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    h83217.cpp

    H8/3217 family emulation

    TODO:
    - H8/3212 and H8/3202 have less internal modules
    - PWM timer module
    - Host Interface module
    - optional I2C bus module
    - keyboard matrix interrupt
    - TCONR @ 0xff9f (timer connection)
    - SEDGR @ 0xffa8 (edge sense)
    - WSCR @ 0xffc2 (waitstate control)
    - finish STCR emulation
    - finish SYSCR emulation

***************************************************************************/

#include "emu.h"
#include "h83217.h"

DEFINE_DEVICE_TYPE(H83217, h83217_device, "h83217", "Hitachi H8/3217")
DEFINE_DEVICE_TYPE(H83216, h83216_device, "h83216", "Hitachi H8/3216")
DEFINE_DEVICE_TYPE(H83214, h83214_device, "h83214", "Hitachi H8/3214")
DEFINE_DEVICE_TYPE(H83212, h83212_device, "h83212", "Hitachi H8/3212")
DEFINE_DEVICE_TYPE(H83202, h83202_device, "h83202", "Hitachi H8/3202")


h83217_device::h83217_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83217_device::map), this)),
	m_intc(*this, "intc"),
	m_port(*this, "port%u", 1),
	m_timer8(*this, "timer8_%u", 0),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_watchdog(*this, "watchdog"),
	m_ram_view(*this, "ram_view"),
	m_rom_size(rom_size),
	m_ram_size(ram_size),
	m_md(3)
{
}

h83217_device::h83217_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83217_device(mconfig, H83217, tag, owner, clock, 0xf000, 0x800)
{
}

h83216_device::h83216_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83217_device(mconfig, H83216, tag, owner, clock, 0xc000, 0x800)
{
}

h83214_device::h83214_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83217_device(mconfig, H83214, tag, owner, clock, 0x8000, 0x400)
{
}

h83212_device::h83212_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83217_device(mconfig, H83212, tag, owner, clock, 0x4000, 0x200)
{
}

h83202_device::h83202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83217_device(mconfig, H83202, tag, owner, clock, 0x4000, 0x200)
{
}

void h83217_device::map(address_map &map)
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

	map(0xff9a, 0xff9a).rw(m_timer8[2], FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xff9b, 0xff9b).rw(m_timer8[2], FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xff9c, 0xff9d).rw(m_timer8[2], FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xff9e, 0xff9e).rw(m_timer8[2], FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xff9f, 0xff9f).lw8(NAME([this](u8 data) { logerror("tconr = %02x\n", data); }));

	map(0xffaa, 0xffab).rw(m_watchdog, FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));

	map(0xffac, 0xffac).rw(m_port[0], FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffad, 0xffad).rw(m_port[1], FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffae, 0xffae).rw(m_port[2], FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
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

	map(0xffc2, 0xffc2).lw8(NAME([this](u8 data) { logerror("wscr = %02x\n", data); }));
	map(0xffc3, 0xffc3).rw(FUNC(h83217_device::stcr_r), FUNC(h83217_device::stcr_w));
	map(0xffc4, 0xffc4).rw(FUNC(h83217_device::syscr_r), FUNC(h83217_device::syscr_w));
	map(0xffc5, 0xffc5).r(FUNC(h83217_device::mdcr_r));
	map(0xffc6, 0xffc6).lr8(NAME([this]() { return m_intc->iscr_r() | ~0x47; }));
	map(0xffc6, 0xffc6).lw8(NAME([this](u8 data) { m_intc->iscr_w(data & 0x47); }));
	map(0xffc7, 0xffc7).lr8(NAME([this]() { return m_intc->ier_r() | ~0x47; }));
	map(0xffc7, 0xffc7).lw8(NAME([this](u8 data) { m_intc->ier_w(data & 0x47); }));

	map(0xffc8, 0xffc8).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw(m_timer8[0], FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd0, 0xffd0).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw(m_timer8[1], FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

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

void h83217_device::device_add_mconfig(machine_config &config)
{
	H8_INTC(config, m_intc, *this);
	H8_PORT(config, m_port[0], *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port[1], *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port[2], *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port[3], *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port[4], *this, h8_device::PORT_5, 0x00, 0xc0);
	H8_PORT(config, m_port[5], *this, h8_device::PORT_6, 0x00, 0x80);
	H8_PORT(config, m_port[6], *this, h8_device::PORT_7, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, m_timer8[0], *this, m_intc, 23, 24, 25, 8, 2, 64, 32, 1024, 256);
	H8_TIMER8_CHANNEL(config, m_timer8[1], *this, m_intc, 26, 27, 28, 8, 2, 64, 128, 1024, 2048);
	H8_TIMER8_CHANNEL(config, m_timer8[2], *this, m_intc, 47, 48, 49, 1, 1, 2, 2, 512, 512);
	H8_TIMER16(config, m_timer16, *this, 1, 0xff);
	H8325_TIMER16_CHANNEL(config, m_timer16_0, *this, m_intc, 19);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 29, 30, 31, 32);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 33, 34, 35, 36);
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 44, h8_watchdog_device::B);
}

void h83217_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h83217_device::irq_setup()
{
	m_CCR |= F_I;
}

void h83217_device::update_irq_filter()
{
	if(m_CCR & F_I)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void h83217_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h83217_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer8[0]->internal_update(current_time));
	add_event(event_time, m_timer8[1]->internal_update(current_time));
	add_event(event_time, m_timer8[2]->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83217_device::notify_standby(int state)
{
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);

	for (auto & timer8 : m_timer8)
		timer8->notify_standby(state);

	m_timer16_0->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h83217_device::device_start()
{
	h8_device::device_start();

	m_stcr = 0;
	m_syscr = 0;

	save_item(NAME(m_md));
	save_item(NAME(m_stcr));
	save_item(NAME(m_syscr));
}

void h83217_device::device_reset()
{
	h8_device::device_reset();

	m_stcr = 0x00;
	m_syscr = 0x09;
	m_ram_view.select(0);
}

u8 h83217_device::stcr_r()
{
	return m_stcr;
}

void h83217_device::stcr_w(u8 data)
{
	logerror("stcr = %02x\n", data);

	// ICKS0/1
	m_timer8[0]->set_extra_clock_bit(BIT(data, 0));
	m_timer8[1]->set_extra_clock_bit(BIT(data, 1));

	m_stcr = data;
}

u8 h83217_device::syscr_r()
{
	return m_syscr;
}

void h83217_device::syscr_w(u8 data)
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

	m_syscr = (m_syscr & 0x08) | (data & 0xf7);
}

u8 h83217_device::mdcr_r()
{
	if(!machine().side_effects_disabled())
		logerror("mdcr_r\n");
	return (m_md & 0x03) | 0xe4;
}
