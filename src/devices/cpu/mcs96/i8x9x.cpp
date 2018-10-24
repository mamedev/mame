// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#include "emu.h"
#include "i8x9x.h"
#include "i8x9xd.h"

i8x9x_device::i8x9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mcs96_device(mconfig, type, tag, owner, clock, 8, address_map_constructor(FUNC(i8x9x_device::internal_regs), this)),
	m_ach_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}},
	m_serial_tx_cb(*this),
	m_in_p0_cb(*this),
	m_out_p1_cb(*this), m_in_p1_cb(*this),
	m_out_p2_cb(*this), m_in_p2_cb(*this),
	base_timer2(0), ad_done(0), hso_command(0), ad_command(0), hso_time(0), ad_result(0),
	ios0(0), ios1(0), ioc0(0), ioc1(0), sbuf(0), sp_stat(0), serial_send_buf(0), serial_send_timer(0)
{
}

std::unique_ptr<util::disasm_interface> i8x9x_device::create_disassembler()
{
	return std::make_unique<i8x9x_disassembler>();
}

void i8x9x_device::device_resolve_objects()
{
	for (auto &cb : m_ach_cb)
		cb.resolve_safe(0);
	m_serial_tx_cb.resolve_safe();
	m_in_p0_cb.resolve_safe(0);
	m_out_p1_cb.resolve_safe();
	m_in_p1_cb.resolve_safe(0);
	m_out_p2_cb.resolve_safe();
	m_in_p2_cb.resolve_safe(0);
}

void i8x9x_device::device_start()
{
	mcs96_device::device_start();
	cycles_scaling = 3;
}

void i8x9x_device::device_reset()
{
	mcs96_device::device_reset();
	memset(hso_info, 0, sizeof(hso_info));
	memset(&hso_cam_hold, 0, sizeof(hso_cam_hold));
	hso_command = 0;
	hso_time = 0;
	base_timer2 = 0;
	ios0 = ios1 = ioc0 = ioc1 = 0x00;
	ad_result = 0;
	ad_done = 0;
	sp_stat = 0;
	serial_send_timer = 0;
}

void i8x9x_device::commit_hso_cam()
{
	for(int i=0; i<8; i++)
		if(!hso_info[i].active) {
			if(hso_command != 0x18 && hso_command != 0x19)
				logerror("%s: hso cam %02x %04x in slot %d (%04x)\n", tag(), hso_command, hso_time, i, PPC);
			hso_info[i].active = true;
			hso_info[i].command = hso_command;
			hso_info[i].time = hso_time;
			internal_update(total_cycles());
			return;
		}
	hso_cam_hold.active = true;
	hso_cam_hold.command = hso_command;
	hso_cam_hold.time = hso_time;
}

void i8x9x_device::ad_start(uint64_t current_time)
{
	ad_result = (m_ach_cb[ad_command & 7]() << 6) | 8 | (ad_command & 7);
	ad_done = current_time + 88;
	internal_update(current_time);
}

void i8x9x_device::serial_send(uint8_t data)
{
	serial_send_buf = data;
	serial_send_timer = total_cycles() + 9600;
}

void i8x9x_device::serial_send_done()
{
	serial_send_timer = 0;
	m_serial_tx_cb(serial_send_buf);
	pending_irq |= IRQ_SERIAL;
	sp_stat |= 0x20;
	check_irq();
}

void i8x9x_device::internal_regs(address_map &map)
{
	map(0x00, 0x01).lr16("r0", []() -> u16 { return 0; }).nopw();
	map(0x02, 0x03).r(FUNC(i8x9x_device::ad_result_r)); // 8-bit access
	map(0x02, 0x02).w(FUNC(i8x9x_device::ad_command_w));
	map(0x03, 0x03).w(FUNC(i8x9x_device::hsi_mode_w));
	map(0x04, 0x05).rw(FUNC(i8x9x_device::hsi_time_r), FUNC(i8x9x_device::hso_time_w)); // 16-bit access
	map(0x06, 0x06).rw(FUNC(i8x9x_device::hsi_status_r), FUNC(i8x9x_device::hso_command_w));
	map(0x08, 0x08).rw(FUNC(i8x9x_device::int_mask_r), FUNC(i8x9x_device::int_mask_w));
	map(0x09, 0x09).rw(FUNC(i8x9x_device::int_pending_r), FUNC(i8x9x_device::int_pending_w));
	map(0x0a, 0x0b).r(FUNC(i8x9x_device::timer1_r)); // 16-bit access
	map(0x0c, 0x0d).r(FUNC(i8x9x_device::timer2_r)); // 16-bit access
	map(0x0a, 0x0a).w(FUNC(i8x9x_device::watchdog_w));
	map(0x0e, 0x0e).rw(FUNC(i8x9x_device::port0_r), FUNC(i8x9x_device::baud_rate_w));
	map(0x0f, 0x0f).rw(FUNC(i8x9x_device::port1_r), FUNC(i8x9x_device::port1_w));
	map(0x10, 0x10).rw(FUNC(i8x9x_device::port2_r), FUNC(i8x9x_device::port2_w));
	map(0x11, 0x11).rw(FUNC(i8x9x_device::sp_stat_r), FUNC(i8x9x_device::sp_con_w));
	map(0x15, 0x15).rw(FUNC(i8x9x_device::ios0_r), FUNC(i8x9x_device::ioc0_w));
	map(0x16, 0x16).rw(FUNC(i8x9x_device::ios1_r), FUNC(i8x9x_device::ioc1_w));
	map(0x17, 0x17).w(FUNC(i8x9x_device::pwm_control_w));
	map(0x18, 0xff).ram().share("register_file");
}

void i8x9x_device::ad_command_w(u8 data)
{
	ad_command = data;
	if (ad_command & 8)
		ad_start(total_cycles());
}

u8 i8x9x_device::ad_result_r(offs_t offset)
{
	return ad_result >> (offset ? 8 : 0);
}

void i8x9x_device::hsi_mode_w(u8 data)
{
	logerror("hsi_mode %02x (%04x)\n", data, PPC);
}

void i8x9x_device::hso_time_w(u16 data)
{
	hso_time = data;
	commit_hso_cam();
}

u16 i8x9x_device::hsi_time_r()
{
	if (!machine().side_effects_disabled())
		logerror("read hsi time (%04x)\n", PPC);
	return 0x0000;
}

void i8x9x_device::hso_command_w(u8 data)
{
	hso_command = data;
}

u8 i8x9x_device::hsi_status_r()
{
	if (!machine().side_effects_disabled())
		logerror("read hsi status (%04x)\n", PPC);
	return 0x00;
}

void i8x9x_device::sbuf_w(u8 data)
{
	logerror("sbuf %02x (%04x)\n", data, PPC);
	serial_send(data);
}

u8 i8x9x_device::sbuf_r()
{
	if (!machine().side_effects_disabled())
		logerror("read sbuf %02x (%04x)\n", sbuf, PPC);
	return sbuf;
}

void i8x9x_device::int_mask_w(u8 data)
{
	PSW = (PSW & 0xff00) | data;
	check_irq();
}

u8 i8x9x_device::int_mask_r()
{
	return PSW;
}

void i8x9x_device::int_pending_w(u8 data)
{
	pending_irq = data;
	logerror("int_pending %02x (%04x)\n", data, PPC);
}

u8 i8x9x_device::int_pending_r()
{
	if (!machine().side_effects_disabled())
		logerror("read int pending (%04x)\n", PPC);
	return pending_irq;
}

void i8x9x_device::watchdog_w(u8 data)
{
	logerror("watchdog %02x (%04x)\n", data, PPC);
}

u16 i8x9x_device::timer1_r()
{
	u16 data = timer_value(1, total_cycles());
	if (!machine().side_effects_disabled())
		logerror("read timer1 %04x (%04x)\n", data, PPC);
	return data;
}

u16 i8x9x_device::timer2_r()
{
	u16 data = timer_value(2, total_cycles());
	if (!machine().side_effects_disabled())
		logerror("read timer2 %04x (%04x)\n", data, PPC);
	return data;
}

void i8x9x_device::baud_rate_w(u8 data)
{
	logerror("baud rate %02x (%04x)\n", data, PPC);
}

u8 i8x9x_device::port0_r()
{
	static int last = -1;
	if (!machine().side_effects_disabled() && m_in_p0_cb() != last)
	{
		last = m_in_p0_cb();
		logerror("read p0 %02x\n", last);
	}
	return m_in_p0_cb();
}

void i8x9x_device::port1_w(u8 data)
{
	logerror("io port 1 %02x (%04x)\n", data, PPC);
	m_out_p1_cb(data);
}

u8 i8x9x_device::port1_r()
{
	return m_in_p1_cb();
}

void i8x9x_device::port2_w(u8 data)
{
	logerror("io port 2 %02x (%04x)\n", data, PPC);
	m_out_p2_cb(data);
}

u8 i8x9x_device::port2_r()
{
	return m_in_p2_cb();
}

void i8x9x_device::sp_con_w(u8 data)
{
	logerror("sp con %02x (%04x)\n", data, PPC);
}

u8 i8x9x_device::sp_stat_r()
{
	uint8_t res = sp_stat;
	if (!machine().side_effects_disabled())
	{
		sp_stat &= 0x80;
		logerror("read sp stat %02x (%04x)\n", res, PPC);
	}
	return res;
}

void i8x9x_device::ioc0_w(u8 data)
{
	logerror("ioc0 %02x (%04x)\n", data, PPC);
}

u8 i8x9x_device::ios0_r()
{
	if (!machine().side_effects_disabled())
		logerror("read ios 0 %02x (%04x)\n", ios0, PPC);
	return ios0;
}

void i8x9x_device::ioc1_w(u8 data)
{
	logerror("ioc1 %02x (%04x)\n", data, PPC);
}

u8 i8x9x_device::ios1_r()
{
	uint8_t res = ios1;
	if (!machine().side_effects_disabled())
		ios1 = ios1 & 0xc0;
	return res;
}

void i8x9x_device::pwm_control_w(u8 data)
{
	logerror("pwm control %02x (%04x)\n", data, PPC);
}

void i8x9x_device::do_exec_partial()
{
}

void i8x9x_device::serial_w(uint8_t val)
{
	sbuf = val;
	sp_stat |= 0x40;
	pending_irq |= IRQ_SERIAL;
	check_irq();
}

uint16_t i8x9x_device::timer_value(int timer, uint64_t current_time) const
{
	if(timer == 2)
		current_time -= base_timer2;
	return current_time >> 3;
}

uint64_t i8x9x_device::timer_time_until(int timer, uint64_t current_time, uint16_t timer_value) const
{
	uint64_t timer_base = timer == 2 ? base_timer2 : 0;
	uint64_t delta = (current_time - timer_base) >> 3;
	uint32_t tdelta = uint16_t(timer_value - delta);
	if(!tdelta)
		tdelta = 0x10000;
	return timer_base + ((delta + tdelta) << 3);
}

void i8x9x_device::trigger_cam(int id, uint64_t current_time)
{
	hso_cam_entry &cam = hso_info[id];
	cam.active = false;
	switch(cam.command & 0x0f) {
	case 0x8: case 0x9: case 0xa: case 0xb:
		ios1 |= 1 << (cam.command & 3);
		pending_irq |= IRQ_SOFT;
		check_irq();
		break;

	default:
		logerror("%s: Action %x unimplemented\n", tag(), cam.command & 0x0f);
		break;
	}
}

void i8x9x_device::internal_update(uint64_t current_time)
{
	uint16_t current_timer1 = timer_value(1, current_time);
	uint16_t current_timer2 = timer_value(2, current_time);

	for(int i=0; i<8; i++)
		if(hso_info[i].active) {
			uint8_t cmd = hso_info[i].command;
			uint16_t t = hso_info[i].time;
			if(((cmd & 0x40) && t == current_timer2) ||
				(!(cmd & 0x40) && t == current_timer1)) {
				if(cmd != 0x18 && cmd != 0x19)
					logerror("%s: hso cam %02x %04x in slot %d triggered\n",
								tag(), cmd, t, i);
				trigger_cam(i, current_time);
			}
		}

	if(current_time == ad_done) {
		ad_done = 0;
		ad_result &= ~8;
	}

	if(current_time == serial_send_timer)
		serial_send_done();

	uint64_t event_time = 0;
	for(int i=0; i<8; i++) {
		if(!hso_info[i].active && hso_cam_hold.active) {
			hso_info[i] = hso_cam_hold;
			hso_cam_hold.active = false;
			logerror("%s: hso cam %02x %04x in slot %d from hold\n", tag(), hso_cam_hold.command, hso_cam_hold.time, i);
		}
		if(hso_info[i].active) {
			uint64_t new_time = timer_time_until(hso_info[i].command & 0x40 ? 2 : 1, current_time, hso_info[i].time);
			if(!event_time || new_time < event_time)
				event_time = new_time;
		}
	}

	if(ad_done && ad_done < event_time)
		event_time = ad_done;

	if(serial_send_timer && serial_send_timer < event_time)
		event_time = serial_send_timer;

	recompute_bcount(event_time);
}

c8095_device::c8095_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8x9x_device(mconfig, C8095, tag, owner, clock)
{
}

p8098_device::p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8x9x_device(mconfig, P8098, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(C8095, c8095_device, "c8095", "Intel C8095")
DEFINE_DEVICE_TYPE(P8098, p8098_device, "p8098", "Intel P8098")

#include "cpu/mcs96/i8x9x.hxx"
