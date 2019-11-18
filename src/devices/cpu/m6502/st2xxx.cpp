// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2XXX LCD MCUs

    This extended SoC family combines a 65C02 CPU core (including the
    Rockwell bit opcodes) with a wide variety of on-chip peripherals.
    Features common to all besides internal RAM and ROM are parallel
    ports, internal timers, a multi-level interrupt controller, LCD
    controllers (of varying degrees of sophistication), R/C/slow XTAL
    clock generators, power management and PSG channels for speaker
    output. Each MCU also has numerous pins dedicated to LCD segment
    drivers, an external bus addressing several MB of off-chip
    memory using multiple chip select signals, or both.

    On all ST2XXX MCUs but the smallest single-chip ST20XX models,
    4000–7FFF (nominally program memory) and 8000–FFFF (nominally
    data memory) are bankswitched over all internal and external ROM,
    and interrupt vectors are read from 7Fxx rather than FFxx. The
    ST22XX and ST26XX series use a separate, auto-incrementing bank
    register for DMA reads from the 8000–FFFF area, and will also
    switch 4000–7FFF to a different bank during interrupt service if
    the IRREN bit in the SYS register is set.

**********************************************************************/

#include "emu.h"
#include "st2xxx.h"

#define LOG_IRQ (1 << 1U)
#define LOG_BT (1 << 2U)
#define LOG_LCDC (1 << 3U)
//#define VERBOSE (LOG_IRQ | LOG_BT | LOG_LCDC)
#include "logmacro.h"

st2xxx_device::st2xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map, int data_bits, bool has_banked_ram)
	: r65c02_device(mconfig, type, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, data_bits, 0)
	, m_in_port_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_out_port_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_prr_mask(data_bits <= 14 ? 0 : ((u16(1) << (data_bits - 14)) - 1) | (has_banked_ram ? 0x8000 : 0))
	, m_drr_mask(data_bits <= 15 ? 0 : ((u16(1) << (data_bits - 15)) - 1) | (has_banked_ram ? 0x8000 : 0))
	, m_pdata{0}
	, m_pctrl{0}
	, m_psel{0}
	, m_pfun{0}
	, m_pmcr(0)
	, m_bten(0)
	, m_btsr(0)
	, m_bt_mask(0)
	, m_bt_ireq(0)
	, m_pres_base(0)
	, m_pres_started(attotime::zero)
	, m_prs(0)
	, m_sys(0)
	, m_misc(0)
	, m_ireq(0)
	, m_iena(0)
	, m_lssa(0)
	, m_lvpw(0)
	, m_lxmax(0)
	, m_lymax(0)
	, m_lpan(0)
	, m_lctr(0)
	, m_lckr(0)
	, m_lfra(0)
	, m_lac(0)
	, m_lpwm(0)
	, m_lcd_ireq(0)
	, m_lcd_timer(nullptr)
	, m_bctr(0)
{
	program_config.m_internal_map = std::move(internal_map);
}

device_memory_interface::space_config_vector st2xxx_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void st2xxx_device::device_resolve_objects()
{
	for (auto &cb : m_in_port_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_out_port_cb)
		cb.resolve_safe();
}

TIMER_CALLBACK_MEMBER(st2xxx_device::bt_interrupt)
{
	// BTSR must be cleared each time the interrupt is serviced
	bool interrupt = (m_btsr == 0);

	m_btsr |= 1 << param;

	unsigned div = st2xxx_bt_divider(param);
	assert(div != 0);
	m_base_timer[param]->adjust(attotime::from_ticks(div, 32768), param);

	if (interrupt)
	{
		LOGMASKED(LOG_BT, "Interrupt caused by %.1f Hz base timer\n", 32768.0 / div);
		m_ireq |= m_bt_ireq;
		update_irq_state();
	}
}

void st2xxx_device::init_base_timer(u16 ireq)
{
	m_bt_ireq = ireq;

	for (int n = 0; n < 8; n++)
	{
		if (st2xxx_bt_divider(n) != 0)
		{
			m_bt_mask |= 1 << n;
			m_base_timer[n] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2xxx_device::bt_interrupt), this));
		}
	}

	assert(m_bt_mask != 0);
	assert(m_bt_ireq != 0);
}

TIMER_CALLBACK_MEMBER(st2xxx_device::lcd_interrupt)
{
	m_ireq |= m_lcd_ireq;
	update_irq_state();
}

void st2xxx_device::init_lcd_timer(u16 ireq)
{
	m_lcd_ireq = ireq;
	assert(m_lcd_ireq != 0);

	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2xxx_device::lcd_interrupt), this));
}

void st2xxx_device::save_common_registers()
{
	mi_st2xxx *intf = downcast<mi_st2xxx *>(mintf.get());

	save_item(NAME(m_pdata));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_psel));
	save_item(NAME(m_pfun));
	save_item(NAME(m_pmcr));
	if (m_prr_mask != 0)
	{
		if (BIT(st2xxx_sys_mask(), 1))
		{
			save_item(NAME(intf->irq_service));
			save_item(NAME(intf->irr_enable));
			save_item(NAME(intf->irr));
		}
		save_item(NAME(intf->prr));
	}
	if (m_drr_mask != 0)
	{
		save_item(NAME(intf->drr));
		if (st2xxx_has_dma())
			save_item(NAME(intf->dmr));
	}
	if (m_bt_mask != 0)
	{
		save_item(NAME(m_bten));
		save_item(NAME(m_btsr));
	}
	save_item(NAME(m_pres_base));
	save_item(NAME(m_pres_started));
	save_item(NAME(m_prs));
	save_item(NAME(m_sys));
	if (st2xxx_misc_mask() != 0)
		save_item(NAME(m_misc));
	save_item(NAME(m_ireq));
	save_item(NAME(m_iena));
	save_item(NAME(m_lssa));
	save_item(NAME(m_lvpw));
	save_item(NAME(m_lxmax));
	save_item(NAME(m_lymax));
	if (st2xxx_lpan_mask() != 0)
		save_item(NAME(m_lpan));
	save_item(NAME(m_lctr));
	save_item(NAME(m_lckr));
	save_item(NAME(m_lfra));
	save_item(NAME(m_lac));
	save_item(NAME(m_lpwm));
	if (st2xxx_bctr_mask() != 0)
	{
		save_item(NAME(m_bctr));
		save_item(NAME(m_brs));
		save_item(NAME(m_bdiv));
	}
}

void st2xxx_device::device_reset()
{
	m6502_device::device_reset();

	// reset port registers
	std::fill(std::begin(m_pdata), std::end(m_pdata), 0xff);
	std::fill(std::begin(m_pctrl), std::end(m_pctrl), 0);
	std::fill(std::begin(m_psel), std::end(m_psel), 0xff);
	std::fill(std::begin(m_pfun), std::end(m_pfun), 0);
	for (auto &cb : m_out_port_cb)
		cb(0xff);
	m_pmcr = 0x80;

	// reset bank registers
	mi_st2xxx &m = downcast<mi_st2xxx &>(*mintf);
	m.irr_enable = false;
	m.irr = 0;
	m.prr = 0;
	m.drr = 0;
	m.dmr = 0;

	// reset interrupt registers
	m_ireq = 0;
	m_iena = 0;
	update_irq_state();

	// reset base timer
	bten_w(0);
	m_btsr = 0;

	// reset prescaler
	prs_w(0x80);

	// reset miscellaneous registers
	m_sys = 0;
	m_misc = st2xxx_wdten_on_reset() ? 0x0c : 0;

	// reset LCDC registers
	m_lssa = 0;
	m_lvpw = 0;
	m_lxmax = 0;
	m_lymax = 0;
	m_lpan = 0;
	m_lctr = 0x80;
	m_lckr = 0;
	m_lfra = 0;
	m_lac = 0;
	m_lpwm = 0;
	m_lcd_timer->adjust(attotime::never);

	// reset UART and BRG
	m_bctr = 0;
}

u8 st2xxx_device::acknowledge_irq()
{
	// IREQH interrupts have priority over IREQL interrupts
	for (int pri = 0; pri < 16; pri++)
	{
		int level = pri ^ 8;
		if (BIT(m_ireq & m_iena, level))
		{
			LOGMASKED(LOG_IRQ, "%s interrupt acknowledged (PC = $%04X, vector = $%04X)\n",
				st2xxx_irq_name(level),
				PPC,
				0x7ff8 - (level << 1));
			m_ireq &= ~(1 << level);
			update_irq_state();
			return level;
		}
	}
	throw emu_fatalerror("ST2XXX: no IRQ to acknowledge!\n");
}

u8 st2xxx_device::pdata_r(offs_t offset)
{
	u8 pdata = m_pdata[offset];
	u8 pinmask = ~m_pctrl[offset] | (pdata & ~m_psel[offset]);
	if (pinmask != 0)
		pdata = (pdata & ~pinmask) | (m_in_port_cb[offset](0, pinmask) & pinmask);
	return pdata;
}

void st2xxx_device::pdata_w(offs_t offset, u8 data)
{
	// Set output state (CMOS or open drain) or activate/deactive pullups for input pins
	if (data != m_pdata[offset])
	{
		m_pdata[offset] = data;
		m_out_port_cb[offset](0, data, m_pctrl[offset]);
	}
}

u8 st2xxx_device::pctrl_r(offs_t offset)
{
	return m_pctrl[offset];
}

void st2xxx_device::pctrl_w(offs_t offset, u8 data)
{
	if (data != m_pctrl[offset])
	{
		m_pctrl[offset] = data;
		m_out_port_cb[offset](0, m_pdata[offset], data);
	}
}

u8 st2xxx_device::pfc_r()
{
	return m_pfun[0];
}

void st2xxx_device::pfc_w(u8 data)
{
	m_pfun[0] = data;
}

u8 st2xxx_device::pfd_r()
{
	return m_pfun[1];
}

void st2xxx_device::pfd_w(u8 data)
{
	m_pfun[1] = data;
}

u8 st2xxx_device::pl_r()
{
	return pdata_r(6);
}

void st2xxx_device::pl_w(u8 data)
{
	pdata_w(6, data);
}

u8 st2xxx_device::psc_r()
{
	return m_psel[2];
}

void st2xxx_device::psc_w(u8 data)
{
	m_psel[2] = data;
}

u8 st2xxx_device::pse_r()
{
	return m_psel[4];
}

void st2xxx_device::pse_w(u8 data)
{
	m_psel[4] = data;
}

u8 st2xxx_device::pcl_r()
{
	return pctrl_r(6);
}

void st2xxx_device::pcl_w(u8 data)
{
	pctrl_w(6, data);
}

u8 st2xxx_device::pmcr_r()
{
	return m_pmcr;
}

void st2xxx_device::pmcr_w(u8 data)
{
	m_pmcr = data & st2xxx_pmcr_mask();
}

u8 st2xxx_device::bten_r()
{
	return m_bten;
}

void st2xxx_device::bten_w(u8 data)
{
	data &= m_bt_mask;

	for (int n = 0; n < 8; n++)
	{
		if (BIT(data, n) && !BIT(m_bten, n))
		{
			unsigned div = st2xxx_bt_divider(n);
			assert(div != 0);
			assert(m_base_timer[n] != nullptr);
			m_base_timer[n]->adjust(attotime::from_ticks(div, 32768), n);
			LOGMASKED(LOG_BT, "Base timer %d enabled at %.1f Hz (PC = $%04X)\n", n, 32768.0 / div, PPC);
		}
		else if (!BIT(data, n) && BIT(m_bten, n))
		{
			m_base_timer[n]->adjust(attotime::never);
			LOGMASKED(LOG_BT, "Base timer %d disabled (PC = $%04X)\n", n, PPC);
		}
	}

	m_bten = data;
}

u8 st2xxx_device::btsr_r()
{
	return m_btsr;
}

void st2xxx_device::btclr_w(u8 data)
{
	// Write 1 to clear each individual bit
	m_btsr &= ~data;
}

void st2xxx_device::btclr_all_w(u8 data)
{
	// Only bit 7 has any effect
	if (BIT(data, 7))
		m_btsr = 0;
}

u32 st2xxx_device::tclk_pres_div(u8 mode) const
{
	assert(mode < 8);
	if (mode == 0)
		return 0x10000;
	else if (mode < 4)
		return 0x20000 >> (mode * 2);
	else if (mode == 4)
		return 0x100;
	else
		return 0x8000 >> (mode * 2);
}

u16 st2xxx_device::pres_count() const
{
	return (m_pres_base + ((m_prs & 0x60) == 0x40 ? attotime_to_cycles(machine().time() - m_pres_started) : 0));
}

u8 st2xxx_device::prs_r()
{
	return pres_count() & 0xff;
}

void st2xxx_device::prs_w(u8 data)
{
	data &= st2xxx_prs_mask();

	// Bit 7 produces prescaler reset pulse
	if (BIT(data, 7))
	{
		st2xxx_tclk_stop();
		m_pres_base = 0;
		if ((m_prs & 0x60) == 0x40)
		{
			m_pres_started = machine().time();
			st2xxx_tclk_start();
		}
		data &= 0x7f;
	}

	// Bit 6 enables prescaler; bit 5 selects clock source
	if ((data & 0x60) == 0x40 && (m_prs & 0x60) != 0x40)
	{
		m_pres_started = machine().time();
		st2xxx_tclk_start();
	}
	else if ((data & 0x60) == 0x40 && (m_prs & 0x60) != 0x40)
	{
		st2xxx_tclk_stop();
		m_pres_base += attotime_to_cycles(machine().time() - m_pres_started);
	}

	m_prs = data;
}

u8 st2xxx_device::sys_r()
{
	return m_sys | 0x01;
}

void st2xxx_device::sys_w(u8 data)
{
	u8 mask = st2xxx_sys_mask();
	m_sys = data & mask;
	if (BIT(mask, 1))
		downcast<mi_st2xxx &>(*mintf).irr_enable = BIT(data, 1);
}

u8 st2xxx_device::misc_r()
{
	return m_misc;
}

void st2xxx_device::misc_w(u8 data)
{
	m_misc = data & st2xxx_misc_mask();
}

u8 st2xxx_device::irrl_r()
{
	return downcast<mi_st2xxx &>(*mintf).irr & 0xff;
}

void st2xxx_device::irrl_w(u8 data)
{
	u16 &irr = downcast<mi_st2xxx &>(*mintf).irr;
	irr = (data & m_prr_mask) | (irr & 0xff00);
}

u8 st2xxx_device::irrh_r()
{
	return downcast<mi_st2xxx &>(*mintf).irr >> 8;
}

void st2xxx_device::irrh_w(u8 data)
{
	u16 &irr = downcast<mi_st2xxx &>(*mintf).irr;
	irr = ((u16(data) << 8) & m_prr_mask) | (irr & 0x00ff);
}

u8 st2xxx_device::prrl_r()
{
	return downcast<mi_st2xxx &>(*mintf).prr & 0xff;
}

void st2xxx_device::prrl_w(u8 data)
{
	u16 &prr = downcast<mi_st2xxx &>(*mintf).prr;
	prr = (data & m_prr_mask) | (prr & 0xff00);
}

u8 st2xxx_device::prrh_r()
{
	return downcast<mi_st2xxx &>(*mintf).prr >> 8;
}

void st2xxx_device::prrh_w(u8 data)
{
	u16 &prr = downcast<mi_st2xxx &>(*mintf).prr;
	prr = ((u16(data) << 8) & m_prr_mask) | (prr & 0x00ff);
}

u8 st2xxx_device::drrl_r()
{
	return downcast<mi_st2xxx &>(*mintf).drr & 0xff;
}

void st2xxx_device::drrl_w(u8 data)
{
	u16 &drr = downcast<mi_st2xxx &>(*mintf).drr;
	drr = (data & m_drr_mask) | (drr & 0xff00);
}

u8 st2xxx_device::drrh_r()
{
	return downcast<mi_st2xxx &>(*mintf).drr >> 8;
}

void st2xxx_device::drrh_w(u8 data)
{
	u16 &drr = downcast<mi_st2xxx &>(*mintf).drr;
	drr = ((u16(data) << 8) & m_drr_mask) | (drr & 0x00ff);
}

u8 st2xxx_device::dmrl_r()
{
	return downcast<mi_st2xxx &>(*mintf).dmr & 0xff;
}

void st2xxx_device::dmrl_w(u8 data)
{
	u16 &dmr = downcast<mi_st2xxx &>(*mintf).dmr;
	dmr = (data & m_drr_mask) | (dmr & 0xff00);
}

u8 st2xxx_device::dmrh_r()
{
	return downcast<mi_st2xxx &>(*mintf).dmr >> 8;
}

void st2xxx_device::dmrh_w(u8 data)
{
	u16 &dmr = downcast<mi_st2xxx &>(*mintf).dmr;
	dmr = ((u16(data) << 8) & m_drr_mask) | (dmr & 0x00ff);
}

u8 st2xxx_device::ireql_r()
{
	return m_ireq & 0x00ff;
}

void st2xxx_device::ireql_w(u8 data)
{
	if ((m_ireq & ~data & 0x00ff) != 0)
	{
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(data, i) && BIT(m_ireq, i))
				LOGMASKED(LOG_IRQ, "%s interrupt cleared (PC = $%04X)\n", st2xxx_irq_name(i), PPC);
		}
		m_ireq &= data | 0xff00;
		update_irq_state();
	}
}

u8 st2xxx_device::ireqh_r()
{
	return m_ireq >> 8;
}

void st2xxx_device::ireqh_w(u8 data)
{
	if ((m_ireq & ~(u16(data) << 8) & 0xff00) != 0)
	{
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(data, i) && BIT(m_ireq, i + 8))
				LOGMASKED(LOG_IRQ, "%s interrupt cleared (PC = $%04X)\n", st2xxx_irq_name(i + 8), PPC);
		}
		m_ireq &= u16(data) << 8 | 0x00ff;
		update_irq_state();
	}
}

u8 st2xxx_device::ienal_r()
{
	return m_iena & 0x00ff;
}

void st2xxx_device::ienal_w(u8 data)
{
	data &= st2xxx_ireq_mask();
	if (data != (m_iena & 0x00ff))
	{
		for (int i = 0; i < 8; i++)
		{
			if (BIT(data, i) != BIT(m_iena, i))
				LOGMASKED(LOG_IRQ, "%s interrupt %sabled (PC = $%04X)\n",
					st2xxx_irq_name(i),
					BIT(data, i) ? "en" : "dis",
					PPC);
		}
		m_iena = (m_iena & 0xff00) | data;
		update_irq_state();
	}
}

u8 st2xxx_device::ienah_r()
{
	return m_iena >> 8;
}

void st2xxx_device::ienah_w(u8 data)
{
	data &= st2xxx_ireq_mask() >> 8;
	if (data != (m_iena >> 8))
	{
		for (int i = 0; i < 8; i++)
		{
			if (BIT(data, i) != BIT(m_iena, i + 8))
				LOGMASKED(LOG_IRQ, "%s interrupt %sabled (PC = $%04X)\n",
					st2xxx_irq_name(i + 8),
					BIT(data, i) ? "en" : "dis",
					PPC);
		}
		m_iena = (m_iena & 0x00ff) | (u16(data) << 8);
		update_irq_state();
	}
}

void st2xxx_device::lssal_w(u8 data)
{
	m_lssa = (m_lssa & 0xff00) | data;
}

void st2xxx_device::lssah_w(u8 data)
{
	m_lssa = (m_lssa & 0x00ff) | (u16(data) << 8);
}

void st2xxx_device::lvpw_w(u8 data)
{
	m_lvpw = data;
}

u8 st2xxx_device::lxmax_r()
{
	return m_lxmax;
}

void st2xxx_device::lxmax_w(u8 data)
{
	m_lxmax = data;
	lfr_recalculate_period();
}

u8 st2xxx_device::lymax_r()
{
	return m_lymax;
}

void st2xxx_device::lymax_w(u8 data)
{
	m_lymax = data;
	lfr_recalculate_period();
}

u8 st2xxx_device::lpan_r()
{
	return m_lpan;
}

void st2xxx_device::lpan_w(u8 data)
{
	m_lpan = data & st2xxx_lpan_mask();
}

u8 st2xxx_device::lctr_r()
{
	return m_lctr;
}

void st2xxx_device::lctr_w(u8 data)
{
	data &= st2xxx_lctr_mask();
	u8 old_lctr = std::exchange(m_lctr, data);

	if ((old_lctr & 0xbf) != (m_lctr & 0xbf))
		lfr_recalculate_period();
}

void st2xxx_device::lckr_w(u8 data)
{
	m_lckr = data & st2xxx_lckr_mask();
	lfr_recalculate_period();
}

void st2xxx_device::lfra_w(u8 data)
{
	m_lfra = data & 0x3f;
	lfr_recalculate_period();
}

void st2xxx_device::lfr_recalculate_period()
{
	if (!BIT(m_lctr, 7))
	{
		unsigned clocks = st2xxx_lfr_clocks();
		assert(clocks != 0);
		attotime period = cycles_to_attotime(clocks);
		LOGMASKED(LOG_LCDC, "LCD frame rate = %f Hz (PC = $%04X)\n", period.as_hz(), PPC);
		m_lcd_timer->adjust(period, 0, period);
	}
	else
		m_lcd_timer->adjust(attotime::never);
}

u8 st2xxx_device::lac_r()
{
	return m_lac;
}

void st2xxx_device::lac_w(u8 data)
{
	m_lac = data & 0x1f;
}

u8 st2xxx_device::lpwm_r()
{
	return m_lpwm;
}

void st2xxx_device::lpwm_w(u8 data)
{
	m_lpwm = data & st2xxx_lpwm_mask();
}

u8 st2xxx_device::bctr_r()
{
	return m_bctr;
}

void st2xxx_device::bctr_w(u8 data)
{
	m_bctr = data & st2xxx_bctr_mask();
}

u8 st2xxx_device::brs_r()
{
	return m_brs;
}

void st2xxx_device::brs_w(u8 data)
{
	m_brs = data;
}

u8 st2xxx_device::bdiv_r()
{
	return m_bdiv;
}

void st2xxx_device::bdiv_w(u8 data)
{
	m_bdiv = data;
}

#include "cpu/m6502/st2xxx.hxx"
