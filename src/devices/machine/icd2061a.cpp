// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    IC Designs 2061A Dual Programmable Graphics Clock Generator
                    ____   ____
     SEL0/CLK -> 1 |    \_/    | 16 <- /PWRDWN
    SEL1/DATA -> 2 |           | 15 <- INTCLK
         AVDD -> 3 |           | 14 <- INIT1
      /OUTDIS -> 4 | ICD2061A  | 13 <- VDD
          GND -> 5 |           | 12 <- INIT0
       XTALIN -> 6 |           | 11 <- FEATCLK
      XTALOUT <- 7 |           | 10 -> /ERROUT
      MCLKOUT <- 8 |___________| 9  -> VCLKOUT


    TODO:
    - Not handled: MCLKOUT_HIGH_Z, VCLKOUT_HIGH_Z, VCLKOUT_FORCED_HIGH

***************************************************************************/

#include "emu.h"
#include "icd2061a.h"

#define LOG_PINS (1 << 1)
#define LOG_STATE (1 << 2)
#define LOG_TODO (1 << 3)

// #define VERBOSE (LOG_GENERAL | LOG_PINS | LOG_STATE | LOG_TODO)
#define VERBOSE (LOG_TODO)

#include "logmacro.h"

#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)


DEFINE_DEVICE_TYPE(ICD2061A, icd2061a_device, "icd2061a", "IC Designs 2061A Dual Programmable Graphics Clock Generator")


icd2061a_device::icd2061a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ICD2061A, tag, owner, clock)
	, m_vclkout_changed_cb(*this)
	, m_mclkout_changed_cb(*this)
	, m_errout_cb(*this)
	, m_init0(0), m_init1(0)
	, m_outdis(1), m_pwrdwn(1)
	, m_intclk(0)
{
}

void icd2061a_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_unlock_step));
	save_item(NAME(m_cur_bit));
	save_item(NAME(m_data));
	save_item(NAME(m_data_prev));
	save_item(NAME(m_clk));
	save_item(NAME(m_cmd));

	save_item(NAME(m_init0));
	save_item(NAME(m_init1));
	save_item(NAME(m_sel0));
	save_item(NAME(m_sel1));
	save_item(NAME(m_outdis));
	save_item(NAME(m_pwrdwn));
	save_item(NAME(m_intclk));
	save_item(NAME(m_vclkout_select));
	save_item(NAME(m_mclkout_select));

	save_item(NAME(m_regs));
	save_item(NAME(m_reg_clocks));
	save_item(NAME(m_prescale));
	save_item(NAME(m_powerdown_mode));
	save_item(NAME(m_muxref_vclkout_source));
	save_item(NAME(m_timeout_interval));
	save_item(NAME(m_muxref_adjust));

	save_item(NAME(m_featclock));
	save_item(NAME(m_vclkout_clock));
	save_item(NAME(m_mclkout_clock));

	m_watchdog_timer = timer_alloc(FUNC(icd2061a_device::watchdog_callback), this);
	m_watchdog_timer->adjust(attotime::never);

	m_update_timer = timer_alloc(FUNC(icd2061a_device::update_clock_callback), this);
	m_update_timer->adjust(attotime::never);

	m_state = CLOCKGEN_UNLOCK;
	m_unlock_step = 0;
	m_cur_bit = 0;
	m_data = m_data_prev = 1;
	m_clk = 1;
	m_cmd = 0;

	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_prescale), std::end(m_prescale), 2);
	m_powerdown_mode = 0;
	m_muxref_vclkout_source = 0;
	m_timeout_interval = 5;
	m_muxref_adjust = 1;
	m_powerdown_divisor = 8;
	m_sel0 = m_sel1 = 0;
	m_vclkout_select = m_mclkout_select = -1;
	m_vclkout_clock = m_mclkout_clock = 0;

	// Are these values derived from the XTALIN at all?
	// The manual only gives the frequencies as these values,
	// not p/q/m values that could be used to derive it from XTALIN
	if (m_init0 == 0 && m_init1 == 0) {
		m_reg_clocks[MREG] = 32'500'000;
		m_reg_clocks[REG0] = 25'175'000;
		m_reg_clocks[REG1] = m_reg_clocks[REG2] = 28'322'000;
	} else if (m_init0 == 0 && m_init1 == 1) {
		m_reg_clocks[MREG] = 40'000'000;
		m_reg_clocks[REG0] = 25'175'000;
		m_reg_clocks[REG1] = m_reg_clocks[REG2] = 28'322'000;
	} else if (m_init0 == 1 && m_init1 == 0) {
		m_reg_clocks[MREG] = 50'350'000;
		m_reg_clocks[REG0] = 40'000'000;
		m_reg_clocks[REG1] = m_reg_clocks[REG2] = 28'322'000;
	} else if (m_init0 == 1 && m_init1 == 1) {
		m_reg_clocks[MREG] = 56'644'000;
		m_reg_clocks[REG0] = 40'000'000;
		m_reg_clocks[REG1] = m_reg_clocks[REG2] = 50'350'000;
	}

	m_errout_cb(1); // set no error start
	update_clock_callback(0);
}

void icd2061a_device::set_featclock(const uint32_t clock)
{
	m_featclock = clock;
}

TIMER_CALLBACK_MEMBER( icd2061a_device::watchdog_callback )
{
	// If the timeout is hit then rearm the locked mode and accept the last values as sel0/sel1
	LOG("watchdog timed out, setting sel0 = %d, sel1 = %d\n", m_clk, m_data);

	if (m_sel0 != m_clk || m_sel1 != m_data) {
		m_sel0 = m_clk;
		m_sel1 = m_data;

		update_clock();
	}

	m_state = CLOCKGEN_UNLOCK;
	m_unlock_step = 0;
	m_unlock_step = 0;
	m_cur_bit = 0;
	m_cmd = 0;
}

TIMER_CALLBACK_MEMBER( icd2061a_device::update_clock_callback )
{
	// mclkout
	if (m_outdis == 0)
		m_mclkout_select = MCLKOUT_HIGH_Z;
	else if (m_outdis == 1 && m_pwrdwn == 1)
		m_mclkout_select = MCLKOUT_MREG;
	else if (m_outdis == 1 && m_pwrdwn == 0)
		m_mclkout_select = MCLKOUT_PWRDWN;

	if (m_mclkout_select == MCLKOUT_PWRDWN) {
		if (m_powerdown_mode == 1 || m_powerdown_divisor == 0)
			m_reg_clocks[MREG] = 0;
		else
			m_reg_clocks[MREG] = clock() / ((17 - m_powerdown_divisor) * 2); // 1 = divisor of 32, 15 = divisor of 4
	} else if (m_mclkout_select == MCLKOUT_MREG) {
		const int a = BIT(m_regs[MREG], 21, 2); // register addr
		const int p = BIT(m_regs[MREG], 10, 7) + 3; // p counter value
		const int m = BIT(m_regs[MREG], 7, 3); // post-vco divisor
		const int q = BIT(m_regs[MREG], 0, 7) + 2; // q counter value
		m_reg_clocks[MREG] = (clock() * m_prescale[a] * (p / double(q))) / (1 << m);
	} else {
		LOGTODO("unimplemented mclkout selected %d\n", m_mclkout_select);
	}

	if (m_reg_clocks[MREG] != m_mclkout_clock) {
		m_mclkout_changed_cb(m_reg_clocks[MREG]);
		m_mclkout_clock = m_reg_clocks[MREG];
	}

	// vclkout
	if (m_outdis == 0)
		m_vclkout_select = VCLKOUT_HIGH_Z;
	else if (m_outdis == 1 && m_pwrdwn == 0)
		m_vclkout_select = VCLKOUT_FORCED_HIGH;
	else if (m_outdis == 1 && m_pwrdwn == 1 && m_sel1 == 0 && m_sel0 == 0)
		m_vclkout_select = VCLKOUT_REG0;
	else if (m_outdis == 1 && m_pwrdwn == 1 && m_sel1 == 0 && m_sel0 == 1)
		m_vclkout_select = VCLKOUT_REG1;
	else if (m_outdis == 1 && m_pwrdwn == 1 && m_intclk == 0 && m_sel1 == 1 && m_sel0 == 0)
		m_vclkout_select = VCLKOUT_FEATCLK;
	else if (m_outdis == 1 && m_pwrdwn == 1 && m_sel1 == 1 && (m_intclk == 1 || m_sel0 == 1))
		m_vclkout_select = VCLKOUT_REG2;

	uint32_t vclkout_clock = m_vclkout_clock;
	if (m_vclkout_select == VCLKOUT_FEATCLK) {
		vclkout_clock = m_featclock;
	} else if (m_vclkout_select >= VCLKOUT_REG0 && m_vclkout_select <= VCLKOUT_REG2) {
		const int a = BIT(m_regs[m_vclkout_select], 21, 2); // register addr
		const int p = BIT(m_regs[m_vclkout_select], 10, 7) + 3; // p counter value
		const int m = BIT(m_regs[m_vclkout_select], 7, 3); // post-vco divisor
		const int q = BIT(m_regs[m_vclkout_select], 0, 7) + 2; // q counter value
		vclkout_clock = m_reg_clocks[m_vclkout_select] = (clock() * m_prescale[a] * (p / double(q))) / (1 << m);
	} else {
		LOGTODO("unimplemented vclkout selected %d\n", m_vclkout_select);
	}

	if (vclkout_clock != m_vclkout_clock) {
		m_vclkout_clock = vclkout_clock;
		m_vclkout_changed_cb(vclkout_clock);
	}
}

void icd2061a_device::update_clock()
{
	// Set muxed clock during transition period
	m_vclkout_changed_cb(m_muxref_vclkout_source ? m_mclkout_clock : clock());

	m_watchdog_timer->adjust(attotime::never);
	m_update_timer->adjust(attotime::from_msec(m_timeout_interval));
}

void icd2061a_device::data_w(int state)
{
	LOGMASKED(LOG_PINS, "data_w %d\n", state);

	m_watchdog_timer->adjust(attotime::from_msec(m_timeout_interval));

	m_data = state;
}

void icd2061a_device::clk_w(int state)
{
	LOGMASKED(LOG_PINS, "clk_w %d\n", state);

	m_watchdog_timer->adjust(attotime::from_msec(m_timeout_interval));

	if (!m_clk && state) {
		if (m_state == CLOCKGEN_UNLOCK && m_data == 1) {
			// Any number of 1s can be read until the final transition with data low
			m_errout_cb(1); // clear any previous errors since we're in a good state now

			m_unlock_step++;
			LOGMASKED(LOG_STATE, "unlock count = %d\n", m_unlock_step);
		} else if (m_state == CLOCKGEN_UNLOCK && m_data == 0 && m_unlock_step >= 5) {
			// Found last part of unlock sequence, move on to start bit
			m_state = CLOCKGEN_START_BIT;
			m_unlock_step = 0;
			LOGMASKED(LOG_STATE, "found unlock end, state = CLOCKGEN_START_BIT\n");
		} else if (m_state == CLOCKGEN_START_BIT && m_data == 0) {
			// Found start bit transition, move on to data
			m_state = CLOCKGEN_DATA;
			m_cur_bit = 0;
			m_cmd = 0;
			LOGMASKED(LOG_STATE, "found start bit, state = CLOCKGEN_DATA\n");
		} else if (m_state == CLOCKGEN_DATA && m_data_prev != m_data && m_cur_bit < 24) {
			// Data uses modified Manchester encoding so the data bit read on each edge must be different
			// Must read exactly 24 bits of data here
			m_cmd |= m_data << m_cur_bit;
			LOGMASKED(LOG_STATE, "data %d %06x\n", m_cur_bit, m_cmd);
			m_cur_bit++;
		} else if (m_state == CLOCKGEN_DATA && m_data == 1 && m_cur_bit == 24) {
			// Found end bit transition, accept data and then rearm lock
			const int idx = BIT(m_cmd, 21, 3);

			if (idx == 4) {
				const int p = BIT(m_cmd, 17, 4);
				LOG("PWRDWN register %06x p[%d]\n", m_cmd, p);
				m_powerdown_divisor = p;
			} else if (idx == 6) {
				const int c = BIT(m_cmd, 15, 6);
				const int ps = BIT(m_cmd, 12, 3);

				m_powerdown_mode = BIT(c, 5);
				m_muxref_vclkout_source = BIT(c, 4);
				m_timeout_interval = 5 * (1 << BIT(c, 3));
				m_muxref_adjust = BIT(c, 1);

				m_prescale[0] = 2 << BIT(ps, 0);
				m_prescale[1] = 2 << BIT(ps, 1);
				m_prescale[2] = 2 << BIT(ps, 2);

				LOG("CNTL program %06x c[%d] ps[%d]\n", m_cmd, c, ps);
			} else if (idx <= 3) {
				const int a = BIT(m_cmd, 21, 2); // register addr
				const int i = BIT(m_cmd, 17, 4); // index, used to make sure clock is in expected range
				const int p = BIT(m_cmd, 10, 7) + 3; // p counter value
				const int m = BIT(m_cmd, 7, 3); // post-vco divisor
				const int q = BIT(m_cmd, 0, 7) + 2; // q counter value
				const double outclock = (clock() * m_prescale[a] * (p / double(q))) / 1000000.0;
				const double outclock_scaled = outclock / (1 << m);

				m_regs[idx] = m_cmd;

				LOG("VCO program %06x a[%d] i[%d] p[%d] m[%d] q[%d] prescale[%d] clock[%lf] clock_scaled[%lf]\n", m_cmd, a, i, p, m, q, m_prescale[a], outclock, outclock_scaled);
			} else {
				LOG("Unknown register selected: %06x %d\n", m_cmd, idx);
			}

			m_state = CLOCKGEN_UNLOCK;
			m_unlock_step = 0;

			LOGMASKED(LOG_STATE, "accepted\n");

			update_clock();
		} else {
			// Error state, rearm lock
			m_state = CLOCKGEN_UNLOCK;
			m_unlock_step = 0;

			m_errout_cb(0); // notify of error

			LOGMASKED(LOG_STATE, "error\n");
		}
	} else {
		m_data_prev = m_data;
	}

	m_clk = state;
}
