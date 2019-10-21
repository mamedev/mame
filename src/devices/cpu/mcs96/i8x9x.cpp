// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#include "emu.h"
#include "i8x9x.h"
#include "i8x9xd.h"

i8x9x_device::i8x9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	mcs96_device(mconfig, type, tag, owner, clock, 8, address_map_constructor(FUNC(i8x9x_device::internal_regs), this)),
	m_ach_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}},
	m_hso_cb(*this),
	m_serial_tx_cb(*this),
	m_in_p0_cb(*this),
	m_out_p1_cb(*this), m_in_p1_cb(*this),
	m_out_p2_cb(*this), m_in_p2_cb(*this),
	base_timer2(0), ad_done(0), hsi_mode(0), hso_command(0), ad_command(0), hso_time(0), ad_result(0), pwm_control(0),
	ios0(0), ios1(0), ioc0(0), ioc1(0), sbuf(0), sp_con(0), sp_stat(0), serial_send_buf(0), serial_send_timer(0)
{
	for (auto &hso : hso_info)
	{
		hso.active = false;
		hso.command = 0;
		hso.time = 0;
	}
	hso_cam_hold.active = false;
	hso_cam_hold.command = 0;
	hso_cam_hold.time = 0;
}

std::unique_ptr<util::disasm_interface> i8x9x_device::create_disassembler()
{
	return std::make_unique<i8x9x_disassembler>();
}

void i8x9x_device::device_resolve_objects()
{
	for (auto &cb : m_ach_cb)
		cb.resolve();
	m_hso_cb.resolve_safe();
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

	state_add(I8X9X_HSI_MODE,    "HSI_MODE",    hsi_mode);
	state_add(I8X9X_HSO_TIME,    "HSO_TIME",    hso_time);
	state_add(I8X9X_HSO_COMMAND, "HSO_COMMAND", hso_command);
	state_add(I8X9X_AD_COMMAND,  "AD_COMMAND",  ad_command).mask(0xf);
	state_add(I8X9X_AD_RESULT,   "AD_RESULT",   ad_result);
	state_add(I8X9X_PWM_CONTROL, "PWM_CONTROL", pwm_control);
	state_add(I8X9X_SBUF_RX,     "SBUF_RX",     sbuf);
	state_add(I8X9X_SBUF_TX,     "SBUF_TX",     serial_send_buf);
	state_add(I8X9X_SP_CON,      "SP_CON",      sp_con).mask(0x1f);
	state_add(I8X9X_SP_STAT,     "SP_STAT",     sp_stat).mask(0xe0);
	state_add(I8X9X_BAUD_RATE,   "BAUD_RATE",   baud_reg);
	state_add(I8X9X_IOC0,        "IOC0",        ioc0).mask(0xfd);
	state_add(I8X9X_IOC1,        "IOC1",        ioc1);
	state_add(I8X9X_IOS0,        "IOS0",        ios0);
	state_add(I8X9X_IOS1,        "IOS1",        ios1);

	for(int i = 0; i < 8; i++)
	{
		save_item(NAME(hso_info[i].active), i);
		save_item(NAME(hso_info[i].command), i);
		save_item(NAME(hso_info[i].time), i);
	}
	save_item(NAME(hso_cam_hold.active));
	save_item(NAME(hso_cam_hold.command));
	save_item(NAME(hso_cam_hold.time));

	save_item(NAME(base_timer2));
	save_item(NAME(ad_done));
	save_item(NAME(hsi_mode));
	save_item(NAME(hso_command));
	save_item(NAME(ad_command));
	save_item(NAME(hso_time));
	save_item(NAME(ad_result));
	save_item(NAME(pwm_control));
	save_item(NAME(ios0));
	save_item(NAME(ios1));
	save_item(NAME(ioc0));
	save_item(NAME(ioc1));
	save_item(NAME(sbuf));
	save_item(NAME(sp_con));
	save_item(NAME(sp_stat));
	save_item(NAME(serial_send_buf));
	save_item(NAME(serial_send_timer));
	save_item(NAME(baud_reg));
	save_item(NAME(brh));
}

void i8x9x_device::device_reset()
{
	mcs96_device::device_reset();
	for (auto &hso : hso_info)
		hso.active = false;
	hso_cam_hold.active = false;
	hso_command = 0;
	hso_time = 0;
	timer2_reset(total_cycles());
	ios0 = ios1 = 0x00;
	ioc0 &= 0xaa;
	ioc1 = (ioc1 & 0xae) | 0x01;
	ad_result = 0;
	ad_done = 0;
	pwm_control = 0x00;
	sp_con &= 0x17;
	sp_stat &= 0x80;
	serial_send_timer = 0;
	brh = false;
	m_hso_cb(0);
}

void i8x9x_device::commit_hso_cam()
{
	for(int i=0; i<8; i++)
		if(!hso_info[i].active) {
			//logerror("hso cam %02x %04x in slot %d (%04x)\n", hso_command, hso_time, i, PPC);
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

void i8x9x_device::ad_start(u64 current_time)
{
	ad_result = 8 | (ad_command & 7);
	if (m_ach_cb[ad_command & 7].isnull())
		logerror("Analog input on ACH%d not configured\n", ad_command & 7);
	else
		ad_result |= m_ach_cb[ad_command & 7]() << 6;
	ad_done = current_time + 88;
	internal_update(current_time);
}

void i8x9x_device::serial_send(u8 data)
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
	map(0x07, 0x07).rw(FUNC(i8x9x_device::sbuf_r), FUNC(i8x9x_device::sbuf_w));
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
	ad_command = data & 0xf;
	if (ad_command & 8)
		ad_start(total_cycles());
}

u8 i8x9x_device::ad_result_r(offs_t offset)
{
	return ad_result >> (offset ? 8 : 0);
}

void i8x9x_device::hsi_mode_w(u8 data)
{
	hsi_mode = data;
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
	if (brh)
		baud_reg = (baud_reg & 0x00ff) | u16(data) << 8;
	else
		baud_reg = (baud_reg & 0xff00) | data;
	if (!machine().side_effects_disabled())
		brh = !brh;
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
	sp_con = data & 0x1f;
}

u8 i8x9x_device::sp_stat_r()
{
	u8 res = sp_stat;
	if (!machine().side_effects_disabled())
	{
		sp_stat &= 0x80;
		logerror("read sp stat %02x (%04x)\n", res, PPC);
	}
	return res;
}

void i8x9x_device::ioc0_w(u8 data)
{
	ioc0 = data & 0xfd;
	if (BIT(data, 1))
		timer2_reset(total_cycles());
}

u8 i8x9x_device::ios0_r()
{
	return ios0;
}

void i8x9x_device::ioc1_w(u8 data)
{
	ioc1 = data;
}

u8 i8x9x_device::ios1_r()
{
	u8 res = ios1;
	if (!machine().side_effects_disabled())
		ios1 = ios1 & 0xc0;
	return res;
}

void i8x9x_device::pwm_control_w(u8 data)
{
	pwm_control = data;
}

void i8x9x_device::do_exec_partial()
{
}

void i8x9x_device::serial_w(u8 val)
{
	sbuf = val;
	sp_stat |= 0x40;
	pending_irq |= IRQ_SERIAL;
	check_irq();
}

u16 i8x9x_device::timer_value(int timer, u64 current_time) const
{
	if(timer == 2)
		current_time -= base_timer2;
	return current_time >> 3;
}

u64 i8x9x_device::timer_time_until(int timer, u64 current_time, u16 timer_value) const
{
	u64 timer_base = timer == 2 ? base_timer2 : 0;
	u64 delta = (current_time - timer_base) >> 3;
	u32 tdelta = u16(timer_value - delta);
	if(!tdelta)
		tdelta = 0x10000;
	return timer_base + ((delta + tdelta) << 3);
}

void i8x9x_device::timer2_reset(u64 current_time)
{
	base_timer2 = current_time;
}

void i8x9x_device::trigger_cam(int id, u64 current_time)
{
	hso_cam_entry &cam = hso_info[id];
	cam.active = false;
	switch(cam.command & 0x0f) {
	case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5:
		set_hso(1 << (cam.command & 7), BIT(cam.command, 5));
		break;

	case 0x6:
		set_hso(0x03, BIT(cam.command, 5));
		break;

	case 0x7:
		set_hso(0x0c, BIT(cam.command, 5));
		break;

	case 0x8: case 0x9: case 0xa: case 0xb:
		ios1 |= 1 << (cam.command & 3);
		break;

	case 0xe:
		timer2_reset(current_time);
		break;

	case 0xf:
		ad_start(current_time);
		break;

	default:
		logerror("HSO action %x undefined\n", cam.command & 0x0f);
		break;
	}

	if(BIT(cam.command, 4))
	{
		pending_irq |= BIT(cam.command, 3) ? IRQ_SOFT : IRQ_HSO;
		check_irq();
	}
}

void i8x9x_device::set_hso(u8 mask, bool state)
{
	if(state)
		ios0 |= mask;
	else
		ios0 &= ~mask;
	m_hso_cb(0, ios0 & 0x3f, mask);
}

void i8x9x_device::internal_update(u64 current_time)
{
	u16 current_timer1 = timer_value(1, current_time);
	u16 current_timer2 = timer_value(2, current_time);

	for(int i=0; i<8; i++)
		if(hso_info[i].active) {
			u8 cmd = hso_info[i].command;
			u16 t = hso_info[i].time;
			if(((cmd & 0x40) && t == current_timer2) ||
				(!(cmd & 0x40) && t == current_timer1)) {
				//logerror("hso cam %02x %04x in slot %d triggered\n", cmd, t, i);
				trigger_cam(i, current_time);
			}
		}

	if(ad_done && current_time >= ad_done) {
		// A/D conversion complete
		ad_done = 0;
		ad_result &= ~8;
		pending_irq |= IRQ_AD;
		check_irq();
	}

	if(current_time == serial_send_timer)
		serial_send_done();

	u64 event_time = 0;
	for(int i=0; i<8; i++) {
		if(!hso_info[i].active && hso_cam_hold.active) {
			hso_info[i] = hso_cam_hold;
			hso_cam_hold.active = false;
			logerror("%s: hso cam %02x %04x in slot %d from hold\n", tag(), hso_cam_hold.command, hso_cam_hold.time, i);
		}
		if(hso_info[i].active) {
			u64 new_time = timer_time_until(hso_info[i].command & 0x40 ? 2 : 1, current_time, hso_info[i].time);
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

c8095_device::c8095_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, C8095, tag, owner, clock)
{
}

p8098_device::p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, P8098, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(C8095, c8095_device, "c8095", "Intel C8095")
DEFINE_DEVICE_TYPE(P8098, p8098_device, "p8098", "Intel P8098")

#include "cpu/mcs96/i8x9x.hxx"
