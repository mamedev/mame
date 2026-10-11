// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#include "emu.h"
#include "i8x9x.h"
#include "i8x9xd.h"

#include <bit>

i8x9x_device::i8x9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int data_width) :
	mcs96_device(mconfig, type, tag, owner, clock, data_width, address_map_constructor(FUNC(i8x9x_device::internal_regs), this)),
	m_ach_cb(*this, 0),
	m_hso_cb(*this),
	m_serial_tx_cb(*this),
	m_txd_cb(*this),
	m_in_p0_cb(*this, 0),
	m_out_p1_cb(*this), m_in_p1_cb(*this, 0xff),
	m_out_p2_cb(*this), m_in_p2_cb(*this, 0xc2),
	m_p1_driven_mask(0),
	base_timer2(0), ad_done(0), hsi_mode(0), hsi_status(0), hso_command(0), ad_command(0), hso_active(0), hso_time(0), ad_result(0), pwm_control(0),
	port1(0), port2(0),
	ios0(0), ios1(0), ioc0(0), ioc1(0), extint(false),
	sbuf(0), sp_con(0), sp_stat(0), serial_send_buf(0), baud_reg(0), brh(false),
	tx_next(0), rx_next(0),
	tx_shift(0), tx_bits(0), tx_busy(false), tx_pending(false), tx_pending_buf(0), tx_ti_due(false),
	txd_serial(1), txd_pin(1), rxd_pin(1),
	rx_active(false), rx_bit(0), rx_shift(0)
{
	for (auto &hso : hso_info)
	{
		hso.command = 0;
		hso.time = 0;
	}
	hso_cam_hold.command = 0;
	hso_cam_hold.time = 0;
}

std::unique_ptr<util::disasm_interface> i8x9x_device::create_disassembler()
{
	return std::make_unique<i8x9x_disassembler>();
}

void i8x9x_device::device_start()
{
	mcs96_device::device_start();
	cycles_scaling = 3;

	state_add(I8X9X_HSI_MODE,    "HSI_MODE",    hsi_mode);
	state_add<u8>(I8X9X_HSI_STATUS, "HSI_STATUS",
					[this]() -> u8 { return hsi_status; },
					[this](u8 data) { hsi_status = (data & 0x55) | (hsi_status & 0xaa); });
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
	if (i8x9x_has_p1())
		state_add<u8>(I8X9X_PORT1, "PORT1",     [this]() -> u8 { return port1; }, [this](u8 data) { port1_w(data); });
	state_add<u8>(I8X9X_PORT2,   "PORT2",       [this]() -> u8 { return port2; }, [this](u8 data) { port2_w(data); }).mask(i8x9x_p2_mask());
	state_add(I8X9X_IOC0,        "IOC0",        ioc0).mask(0xfd);
	state_add(I8X9X_IOC1,        "IOC1",        ioc1);
	state_add<u8>(I8X9X_IOS0,    "IOS0",        [this]() -> u8 { return ios0; }, [this](u8 data) { ios0_w(data); });
	state_add(I8X9X_IOS1,        "IOS1",        ios1);

	save_item(STRUCT_MEMBER(hso_info, command));
	save_item(STRUCT_MEMBER(hso_info, time));
	save_item(STRUCT_MEMBER(hso_info, fire_at));
	save_item(NAME(hso_cam_hold.command));
	save_item(NAME(hso_cam_hold.time));

	save_item(NAME(base_timer2));
	save_item(NAME(ad_done));
	save_item(NAME(hsi_mode));
	save_item(NAME(hsi_status));
	save_item(NAME(hso_command));
	save_item(NAME(ad_command));
	save_item(NAME(hso_active));
	save_item(NAME(hso_time));
	save_item(NAME(ad_result));
	save_item(NAME(port1));
	save_item(NAME(port2));
	save_item(NAME(pwm_control));
	save_item(NAME(ios0));
	save_item(NAME(ios1));
	save_item(NAME(ioc0));
	save_item(NAME(ioc1));
	save_item(NAME(extint));
	save_item(NAME(sbuf));
	save_item(NAME(sp_con));
	save_item(NAME(sp_stat));
	save_item(NAME(serial_send_buf));
	save_item(NAME(baud_reg));
	save_item(NAME(brh));

	save_item(NAME(tx_next));
	save_item(NAME(rx_next));
	save_item(NAME(tx_shift));
	save_item(NAME(tx_bits));
	save_item(NAME(tx_busy));
	save_item(NAME(tx_pending));
	save_item(NAME(tx_pending_buf));
	save_item(NAME(tx_ti_due));
	save_item(NAME(txd_serial));
	save_item(NAME(txd_pin));
	save_item(NAME(rxd_pin));
	save_item(NAME(rx_active));
	save_item(NAME(rx_bit));
	save_item(NAME(rx_shift));
}

void i8x9x_device::device_reset()
{
	mcs96_device::device_reset();
	hso_active = 0;
	hso_command = 0;
	hso_time = 0;
	timer2_reset(total_cycles());
	port1 = 0xff;
	port2 = 0xc1 & i8x9x_p2_mask(); // P2.5 is cleared
	ios0 = ios1 = 0x00;
	ioc0 &= 0xaa;
	ioc1 = (ioc1 & 0xae) | 0x01;
	ad_result = 0;
	ad_done = 0;
	pwm_control = 0x00;
	sp_con &= 0x17;
	sp_stat &= 0x80;
	brh = false;
	m_out_p1_cb(0xff);
	m_out_p2_cb(0xc1);
	m_hso_cb(0);
	serial_tx_stop();
	serial_rx_stop();
}

void i8x9x_device::commit_hso_cam()
{
	for(int i=0; i<8; i++)
		if(!BIT(hso_active, i)) {
			//logerror("hso cam %02x %04x in slot %d (%04x)\n", hso_command, hso_time, i, PPC);
			hso_active |= 1 << i;
			if(hso_active == 0xff)
				ios0 |= 0x40;
			hso_info[i].command = hso_command;
			hso_info[i].time = hso_time;
			hso_info[i].fire_at = timer_time_until(BIT(hso_command, 6) ? 2 : 1,
					total_cycles(), hso_time);
			internal_update(total_cycles());
			return;
		}
	ios0 |= 0xc0;
	hso_cam_hold.command = hso_command;
	hso_cam_hold.time = hso_time;
}

void i8x9x_device::ad_start(u64 current_time)
{
	ad_result = 8 | (ad_command & 7);
	if (!BIT(i8x9x_p0_mask(), ad_command & 7))
		logerror("Analog input on ACH%d does not exist on this device\n", ad_command & 7);
	else if (m_ach_cb[ad_command & 7].isunset())
		logerror("Analog input on ACH%d not configured\n", ad_command & 7);
	else
		ad_result |= m_ach_cb[ad_command & 7]() << 6;
	ad_done = current_time + 88;
	internal_update(current_time);
}

u32 i8x9x_device::serial_bit_clocks() const
{
	// BAUD_RATE bit 15 selects XTAL1 as the clock source. The alternative,
	// T2CLK, is an external pin that isn't emulated; fall back to the
	// 9600 clocks per 10-bit frame this core used before bit-level serial.
	if (!BIT(baud_reg, 15))
		return 960;

	u32 const divisor = (baud_reg & 0x7fff) + 1;
	return ((sp_con & 3) == 0 ? 4 : 64) * divisor;
}

u8 i8x9x_device::serial_data_bits() const
{
	// Modes 0 and 1 have 8 data bits, modes 2 and 3 have 9
	return (sp_con & 2) ? 9 : 8;
}

void i8x9x_device::serial_send(u8 data, u64 current_time)
{
	if (tx_busy)
	{
		// "New data placed in SBUF (tx) is held and will not be transmitted
		// until the end of the stop bit has been sent."
		tx_pending = true;
		tx_pending_buf = data;
		return;
	}

	serial_send_buf = data;

	u8 const mode = sp_con & 3;
	bool const pen = BIT(sp_con, 2);
	u16 frame = data;
	if (mode == 1 && pen)
		frame = (data & 0x7f) | ((std::popcount<u32>(data & 0x7f) & 1) << 7);
	else if (mode >= 2)
		frame |= ((mode == 3 && pen) ? (std::popcount<u32>(data) & 1) : BIT(sp_con, 4)) << 8;

	if (mode == 0)
	{
		// Synchronous mode shifts out on RXD with TXD as the clock; only the
		// timing is emulated
		tx_shift = 0xffff;
		tx_bits = 8;
	}
	else
	{
		// Start bit, data bits (LSB first) and stop bit
		u8 const bits = serial_data_bits();
		tx_shift = (frame << 1) | (1 << (bits + 1));
		tx_bits = bits + 2;
	}
	tx_busy = true;
	tx_event(current_time);
}

void i8x9x_device::serial_tx_done()
{
	sp_con &= ~0x10; // TB8 is cleared after each transmission
	m_serial_tx_cb(serial_send_buf);
	pending_irq |= IRQ_SERIAL;
	sp_stat |= 0x20;
	check_irq();
}

void i8x9x_device::serial_tx_stop()
{
	tx_next = 0;
	tx_busy = false;
	tx_pending = false;
	tx_ti_due = false;
	tx_bits = 0;
	txd_serial = 1;
	update_txd();
}

// The serial state, including the next deadline, is updated before the
// callbacks in serial_tx_done() and update_txd() run. A callback can re-enter
// internal_update (for instance TXD looped back to RXD), and must not see
// this event as still due.
void i8x9x_device::tx_event(u64 current_time)
{
	u32 const bit_clocks = serial_bit_clocks();

	// TI is set in the middle of the last data bit
	if (tx_ti_due)
	{
		tx_ti_due = false;
		tx_next = current_time + bit_clocks / 2;
		serial_tx_done();
		return;
	}

	if (!tx_bits)
	{
		// The whole frame has been shifted out
		tx_next = 0;
		tx_busy = false;
		if (tx_pending)
		{
			tx_pending = false;
			serial_send(tx_pending_buf, current_time);
		}
		return;
	}

	bool const async = (sp_con & 3) != 0;
	int const bit = tx_shift & 1;
	tx_shift >>= 1;
	tx_bits--;

	// After the last data bit only the stop bit (modes 1-3) or nothing
	// (mode 0) remains
	if (tx_bits == (async ? 1 : 0))
	{
		tx_ti_due = true;
		tx_next = current_time + bit_clocks / 2;
	}
	else
	{
		tx_next = current_time + bit_clocks;
	}

	if (async)
	{
		txd_serial = bit;
		update_txd();
	}
}

void i8x9x_device::update_txd()
{
	// IOC1 bit 5 selects TXD rather than P2.0 on the shared pin
	int const state = BIT(ioc1, 5) ? txd_serial : BIT(port2, 0);
	if (state != txd_pin)
	{
		txd_pin = state;
		m_txd_cb(state);
	}
}

void i8x9x_device::rxd_w(int state)
{
	state = state ? 1 : 0;
	if (state == rxd_pin)
		return;
	rxd_pin = state;

	// A falling edge on an idle line starts a frame when reception is
	// enabled (REN) in one of the asynchronous modes
	if (!state && !rx_active && BIT(sp_con, 3) && (sp_con & 3))
	{
		// Outside its timeslice the CPU may have run a little past the time
		// of the edge; count from the edge itself
		u64 edge_time = total_cycles();
		if (!executing() && local_time() > machine().time())
			edge_time -= attotime_to_cycles(local_time() - machine().time());

		rx_active = true;
		rx_bit = 0;
		rx_shift = 0;
		rx_next = edge_time + serial_bit_clocks() / 2;
		if (executing())
			internal_update(total_cycles());
	}
}

void i8x9x_device::serial_rx_stop()
{
	rx_active = false;
	rx_next = 0;
}

void i8x9x_device::rx_event(u64 current_time)
{
	u8 const bits = serial_data_bits();
	if (rx_bit == 0)
	{
		// The start bit must still be low at mid-bit
		if (rxd_pin)
		{
			serial_rx_stop();
			return;
		}
	}
	else
	{
		rx_shift |= rxd_pin << (rx_bit - 1);
	}

	// RI is set once the last data bit has been sampled, without waiting
	// for the stop bit. Its validity isn't checked: the 8X9X has no
	// framing error flag.
	if (rx_bit == bits)
	{
		serial_rx_stop();

		u8 const mode = sp_con & 3;
		bool const pen = BIT(sp_con, 2);

		// Mode 2 only accepts frames with the ninth bit set
		if (mode == 2 && !BIT(rx_shift, 8))
			return;

		sbuf = rx_shift & 0xff;
		if (pen && mode != 2)
		{
			// SP_STAT bit 7 is RPE, set on an even parity error
			bool const error = std::popcount<u32>(rx_shift & ((mode == 3) ? 0x1ff : 0xff)) & 1;
			sp_stat = (sp_stat & 0x7f) | (error ? 0x80 : 0x00);
		}
		else if (mode >= 2)
		{
			// SP_STAT bit 7 is RB8, the received ninth bit
			sp_stat = (sp_stat & 0x7f) | (BIT(rx_shift, 8) << 7);
		}
		sp_stat |= 0x40;
		pending_irq |= IRQ_SERIAL;
		check_irq();
		return;
	}

	rx_bit++;
	rx_next = current_time + serial_bit_clocks();
}

void i8x9x_device::internal_regs(address_map &map)
{
	map(0x00, 0x01).lr16([] () -> u16 { return 0; }, "r0").nopw();
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
	return hsi_status;
}

void i8x9x_device::sbuf_w(u8 data)
{
	logerror("sbuf %02x (%04x)\n", data, PPC);
	serial_send(data, total_cycles());
	internal_update(total_cycles());
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
	if (0 && !machine().side_effects_disabled())
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
	{
		baud_reg = (baud_reg & 0x00ff) | u16(data) << 8;
		if (!BIT(baud_reg, 15) && !machine().side_effects_disabled())
			logerror("Serial baud rate uses unemulated T2CLK source (%04x)\n", baud_reg);
	}
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
	return m_in_p0_cb() & i8x9x_p0_mask();
}

void i8x9x_device::port1_w(u8 data)
{
	if (!i8x9x_has_p1())
	{
		logerror("%s: Write %02x to nonexistent port 1\n", machine().describe_context(), data);
		return;
	}

	port1 = data;
	m_out_p1_cb(data);
}

u8 i8x9x_device::port1_r()
{
	if (!i8x9x_has_p1())
		return 0xff;

	return m_in_p1_cb() & (port1 | m_p1_driven_mask);
}

void i8x9x_device::port2_w(u8 data)
{
	data &= 0xe1 & i8x9x_p2_mask();
	port2 = data;
	m_out_p2_cb(data);
	update_txd();
}

u8 i8x9x_device::port2_r()
{
	// P2.0 and P2.5 are for output only (but can be read back despite what Intel claims?)
	// P2.1 is also the RXD input, which rxd_w drives
	return (m_in_p2_cb() | 0x25 | ~i8x9x_p2_mask()) & (port2 | (extint ? 0x1e : 0x1a)) & (rxd_pin ? 0xff : 0xfd);
}

void i8x9x_device::sp_con_w(u8 data)
{
	u8 const old_mode = sp_con & 3;
	sp_con = data & 0x1f;

	// Changing modes resets the serial port, aborting any transmission or
	// reception in progress; clearing REN stops a reception in progress
	if ((sp_con & 3) != old_mode)
	{
		serial_tx_stop();
		serial_rx_stop();
	}
	else if (!BIT(sp_con, 3) && rx_active)
	{
		serial_rx_stop();
	}
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

void i8x9x_device::ios0_w(u8 data)
{
	u8 mask = (data ^ ios0) & 0x3f;
	ios0 = (data & 0x3f) | (ios0 & 0xc0);
	if (mask != 0)
		m_hso_cb(0, data & 0x3f, mask);
}

void i8x9x_device::ioc1_w(u8 data)
{
	ioc1 = data;
	update_txd();
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

// Timer 1 increments once every eight state times, and a state time is three
// oscillator periods on this family, so the divisor is 8 * 3 = 24.
static constexpr u32 TIMER_DIVISOR = 24;

u16 i8x9x_device::timer_value(int timer, u64 current_time) const
{
	if(timer == 2)
		current_time -= base_timer2;
	return u16(current_time / TIMER_DIVISOR);
}

u64 i8x9x_device::timer_time_until(int timer, u64 current_time, u16 timer_value) const
{
	u64 timer_base = timer == 2 ? base_timer2 : 0;
	u64 delta = (current_time - timer_base) / TIMER_DIVISOR;
	u32 tdelta = u16(timer_value - delta);
	if(!tdelta)
		tdelta = 0x10000;
	return timer_base + ((delta + tdelta) * TIMER_DIVISOR);
}

void i8x9x_device::timer2_reset(u64 current_time)
{
	base_timer2 = current_time;
	// hso_info[].time holds a timer value, not a deadline, so timer2-relative
	// entries need fire_at recomputed against the new base.
	for(int i=0; i<8; i++)
		if(BIT(hso_active, i) && BIT(hso_info[i].command, 6))
			hso_info[i].fire_at = timer_time_until(2, current_time, hso_info[i].time);
}

void i8x9x_device::set_hsi_state(int pin, bool state)
{
	if(pin == 0 && !BIT(hsi_status, 1) && state) {
		if(BIT(ioc1, 1)) {
			pending_irq |= IRQ_HSI0;
			check_irq();
		}
		if((ioc0 & 0x28) == 0x28)
			timer2_reset(total_cycles());
	}

	if(state)
		hsi_status |= 2 << (pin * 2);
	else
		hsi_status &= ~(2 << (pin * 2));
}

void i8x9x_device::trigger_cam(int id, u64 current_time)
{
	hso_cam_entry &cam = hso_info[id];
	if(hso_active == 0xff && !BIT(ios0, 7))
		ios0 &= 0xbf;
	hso_active &= ~(1 << id);
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
	// Fire on "the deadline has been reached or passed" rather than on an
	// exact match against the timer value sampled right now.
	for(int i=0; i<8; i++)
		if(BIT(hso_active, i) && current_time >= hso_info[i].fire_at) {
			//logerror("hso cam %02x %04x in slot %d triggered\n",
			//      hso_info[i].command, hso_info[i].time, i);
			trigger_cam(i, current_time);
		}

	if(ad_done && current_time >= ad_done) {
		// A/D conversion complete
		ad_done = 0;
		ad_result &= ~8;
		pending_irq |= IRQ_AD;
		check_irq();
	}

	// Serial port bit events. Each one schedules the next relative to its
	// own deadline, so bit timing doesn't drift when an update runs late.
	while(tx_next && current_time >= tx_next)
		tx_event(tx_next);
	while(rx_next && current_time >= rx_next)
		rx_event(rx_next);

	u64 event_time = 0;
	for(int i=0; i<8; i++) {
		if(!BIT(hso_active, i) && BIT(ios0, 7)) {
			hso_info[i] = hso_cam_hold;
			// The holding register carries a timer value, so the deadline is
			// only fixed once the entry actually reaches the CAM, i.e. here.
			hso_info[i].fire_at = timer_time_until(BIT(hso_cam_hold.command, 6) ? 2 : 1,
					current_time, hso_cam_hold.time);
			hso_active |= 1 << i;
			ios0 &= 0x7f;
			if(hso_active == 0xff)
				ios0 |= 0x40;
			logerror("hso cam %02x %04x in slot %d from hold\n", hso_cam_hold.command, hso_cam_hold.time, i);
		}
		if(BIT(hso_active, i)) {
			u64 new_time = timer_time_until(hso_info[i].command & 0x40 ? 2 : 1, current_time, hso_info[i].time);
			if(!event_time || new_time < event_time)
				event_time = new_time;
		}
	}

	if(ad_done && (!event_time || ad_done < event_time))
		event_time = ad_done;

	if(tx_next && (!event_time || tx_next < event_time))
		event_time = tx_next;

	if(rx_next && (!event_time || rx_next < event_time))
		event_time = rx_next;

	recompute_bcount(event_time);
}

void i8x9x_device::execute_set_input(int linenum, int state)
{
	switch(linenum) {
	case EXTINT_LINE:
		if(!extint && state && !BIT(ioc1, 1)) {
			pending_irq |= IRQ_EXTINT;
			check_irq();
		}
		extint = state;
		break;

	case HSI0_LINE:
		set_hsi_state(0, state);
		break;

	case HSI1_LINE:
		set_hsi_state(1, state);
		break;

	case HSI2_LINE:
		set_hsi_state(2, state);
		break;

	case HSI3_LINE:
		set_hsi_state(3, state);
		break;
	}
}

c8095_90_device::c8095_90_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, C8095_90, tag, owner, clock, 16)
{
}

n8097bh_device::n8097bh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, N8097BH, tag, owner, clock, 16)
{
}

p8098_device::p8098_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, P8098, tag, owner, clock, 8)
{
}

p8798_device::p8798_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	i8x9x_device(mconfig, P8798, tag, owner, clock, 8)
{
}

DEFINE_DEVICE_TYPE(C8095_90, c8095_90_device, "c8095_90", "Intel C8095-90")
DEFINE_DEVICE_TYPE(N8097BH, n8097bh_device, "n8097bh", "Intel N8097BH")
DEFINE_DEVICE_TYPE(P8098, p8098_device, "p8098", "Intel P8098")
DEFINE_DEVICE_TYPE(P8798, p8798_device, "p8798", "Intel P8798")

#include "cpu/mcs96/i8x9x.hxx"
