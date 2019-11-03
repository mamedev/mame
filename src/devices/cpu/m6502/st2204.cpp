// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2204 8-Bit Integrated Microcontroller

    Functional blocks:
    * Interrupt controller (11 levels excluding BRK and RESET)
    * GPIO (6 ports, 8 bits each)
    * External bus (up to 7 CS outputs, 48M maximum addressable)
    * Timers/event counters with clocking outputs (2 plus base timer)
    * Programmable sound generator (2 channels plus DAC)
    * LCD controller (320x240 B/W or 240x160 4-gray)
    * Serial peripheral interface
    * UART (built-in BRG; RS-232 and IrDA modes)
    * Direct memory access
    * Power down modes (WAI-0, WAI-1, STP)
    * Watchdog timer
    * Low voltage detector
    * 512K ROM (may be disabled)
    * 10K RAM

    Emulation is largely based on documentation for the ST2202, which
    has similar though somewhat lesser capabilities.

**********************************************************************/

#include "emu.h"
#include "st2204.h"

DEFINE_DEVICE_TYPE(ST2204, st2204_device, "st2204", "Sitronix ST2204 Integrated Microcontroller")

st2204_device::st2204_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: st2xxx_device(mconfig, ST2204, tag, owner, clock,
					address_map_constructor(FUNC(st2204_device::int_map), this),
					26, // logical; only 23 address lines are brought out
					0x0f7f)
	, m_bten(0)
	, m_btsr(0)
	, m_base_timer{0}
{
}

template<int N>
TIMER_CALLBACK_MEMBER(st2204_device::bt_interrupt)
{
	m_btsr |= 1 << N;
	m_ireq |= 0x020;
	update_irq_state();
}

void st2204_device::device_start()
{
	std::unique_ptr<mi_st2204> intf = std::make_unique<mi_st2204>();
	intf->data = &space(AS_DATA);
	intf->dcache = space(AS_DATA).cache<0, 0, ENDIANNESS_LITTLE>();
	intf->irr_enable = false;
	intf->irr = 0;
	intf->prr = 0;
	intf->drr = 0;
	intf->irq_service = false;

	m_base_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2204_device::bt_interrupt<0>), this));
	m_base_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2204_device::bt_interrupt<1>), this));
	m_base_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2204_device::bt_interrupt<2>), this));
	m_base_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2204_device::bt_interrupt<3>), this));
	m_base_timer[4] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st2204_device::bt_interrupt<4>), this));

	save_item(NAME(m_pdata));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_psel));
	save_item(NAME(m_pfun));
	save_item(NAME(m_bten));
	save_item(NAME(m_btsr));
	save_item(NAME(m_sys));
	save_item(NAME(m_pmcr));
	save_item(NAME(m_ireq));
	save_item(NAME(m_iena));
	save_item(NAME(m_lssa));
	save_item(NAME(m_lvpw));
	save_item(NAME(m_lxmax));
	save_item(NAME(m_lymax));
	save_item(NAME(intf->irr_enable));
	save_item(NAME(intf->irr));
	save_item(NAME(intf->prr));
	save_item(NAME(intf->drr));
	save_item(NAME(intf->irq_service));

	mintf = std::move(intf);
	init();

	state_add(ST_IRR, "IRR", downcast<mi_st2204 &>(*mintf).irr).mask(0xff);
	state_add(ST_PRR, "PRR", downcast<mi_st2204 &>(*mintf).prr).mask(0xfff);
	state_add(ST_DRR, "DRR", downcast<mi_st2204 &>(*mintf).drr).mask(0x7ff);
	state_add<u8>(ST_IREQ, "IREQ", [this]() { return m_ireq; }, [this](u16 data) { m_ireq = data; update_irq_state(); }).mask(m_ireq_mask);
	state_add<u8>(ST_IENA, "IENA", [this]() { return m_iena; }, [this](u16 data) { m_iena = data; update_irq_state(); }).mask(m_ireq_mask);
	for (int i = 0; i < 5; i++)
	{
		state_add(ST_PDA + i, string_format("PD%c", 'A' + i).c_str(), m_pdata[i]);
		state_add(ST_PCA + i, string_format("PC%c", 'A' + i).c_str(), m_pctrl[i]);
		if (i == 2)
			state_add(ST_PSA + i, string_format("PS%c", 'A' + i).c_str(), m_psel[i]);
		if (i == 2 || i == 3)
			state_add(ST_PFC + i - 2, string_format("PF%c", 'A' + i).c_str(), m_pfun[i - 2]);
	}
	state_add(ST_PDL, "PDL", m_pdata[6]);
	state_add(ST_PCL, "PCL", m_pctrl[6]);
	state_add(ST_PMCR, "PMCR", m_pmcr);
	state_add<u8>(ST_BTEN, "BTEN", [this]() { return m_bten; }, [this](u8 data) { bten_w(data); }).mask(0x1f);
	state_add(ST_BTSR, "BTSR", m_btsr).mask(0x1f);
	state_add<u8>(ST_SYS, "SYS", [this]() { return m_sys; }, [this](u8 data) { sys_w(data); });
	state_add(ST_LSSA, "LSSA", m_lssa);
	state_add(ST_LVPW, "LVPW", m_lvpw);
	state_add(ST_LXMAX, "LXMAX", m_lxmax);
	state_add(ST_LYMAX, "LYMAX", m_lymax);
}

void st2204_device::device_reset()
{
	st2xxx_device::device_reset();

	mi_st2204 &m = downcast<mi_st2204 &>(*mintf);
	m.irr_enable = false;
	m.irr = 0;
	m.prr = 0;
	m.drr = 0;

	bten_w(0);
	m_btsr = 0;
}

u8 st2204_device::mi_st2204::pread(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	return data->read_byte(u32(bank ^ 1) << 14 | (adr & 0x3fff));
}

u8 st2204_device::mi_st2204::preadc(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	return dcache->read_byte(u32(bank ^ 1) << 14 | (adr & 0x3fff));
}

void st2204_device::mi_st2204::pwrite(u16 adr, u8 val)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	data->write_byte(u32(bank ^ 1) << 14 | (adr & 0x3fff), val);
}

u8 st2204_device::mi_st2204::dread(u16 adr)
{
	return data->read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

u8 st2204_device::mi_st2204::dreadc(u16 adr)
{
	return dcache->read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

void st2204_device::mi_st2204::dwrite(u16 adr, u8 val)
{
	data->write_byte(u32(drr) << 15 | (adr & 0x7fff), val);
}

u8 st2204_device::mi_st2204::read(u16 adr)
{
	return program->read_byte(adr);
}

u8 st2204_device::mi_st2204::read_sync(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : cache->read_byte(adr);
}

u8 st2204_device::mi_st2204::read_arg(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : cache->read_byte(adr);
}

u8 st2204_device::mi_st2204::read_vector(u16 adr)
{
	return pread(adr);
}

void st2204_device::mi_st2204::write(u16 adr, u8 val)
{
	program->write_byte(adr, val);
}

u8 st2204_device::pmcr_r()
{
	return m_pmcr;
}

void st2204_device::pmcr_w(u8 data)
{
	m_pmcr = data;
}

u8 st2204_device::bten_r()
{
	return m_bten;
}

void st2204_device::bten_w(u8 data)
{
	for (int n = 0; n < 5; n++)
	{
		if (BIT(data, n) && !BIT(m_bten, n))
		{
			// 2 Hz, 8 Hz, 64 Hz, 256 Hz, 2048 Hz
			attotime period = attotime::from_hz(2 << ((n & 1) * 2 + (n >> 1) * 5));
			m_base_timer[n]->adjust(period, 0, period);
		}
		else if (!BIT(data, n) && BIT(m_bten, n))
			m_base_timer[n]->adjust(attotime::never);
	}

	m_bten = data & 0x1f;
}

u8 st2204_device::btsr_r()
{
	return m_btsr;
}

void st2204_device::btsr_w(u8 data)
{
	// Only bit 7 has any effect
	if (BIT(data, 7))
		m_btsr = 0;
}

u8 st2204_device::sys_r()
{
	return m_sys | 0x01;
}

void st2204_device::sys_w(u8 data)
{
	m_sys = data;
	downcast<mi_st2204 &>(*mintf).irr_enable = BIT(data, 1);
}

u8 st2204_device::irr_r()
{
	return downcast<mi_st2204 &>(*mintf).irr;
}

void st2204_device::irr_w(u8 data)
{
	downcast<mi_st2204 &>(*mintf).irr = data;
}

u8 st2204_device::prrl_r()
{
	return downcast<mi_st2204 &>(*mintf).prr & 0xff;
}

void st2204_device::prrl_w(u8 data)
{
	u16 &prr = downcast<mi_st2204 &>(*mintf).prr;
	prr = data | (prr & 0x0f00);
}

u8 st2204_device::prrh_r()
{
	return downcast<mi_st2204 &>(*mintf).prr >> 8;
}

void st2204_device::prrh_w(u8 data)
{
	u16 &prr = downcast<mi_st2204 &>(*mintf).prr;
	prr = (data & 0x0f) << 16 | (prr & 0x00ff);
}

u8 st2204_device::drrl_r()
{
	return downcast<mi_st2204 &>(*mintf).drr & 0xff;
}

void st2204_device::drrl_w(u8 data)
{
	u16 &drr = downcast<mi_st2204 &>(*mintf).drr;
	drr = data | (drr & 0x0700);
}

u8 st2204_device::drrh_r()
{
	return downcast<mi_st2204 &>(*mintf).drr >> 8;
}

void st2204_device::drrh_w(u8 data)
{
	u16 &drr = downcast<mi_st2204 &>(*mintf).drr;
	drr = (data & 0x07) << 16 | (drr & 0x00ff);
}

u8 st2204_device::pmem_r(offs_t offset)
{
	return downcast<mi_st2204 &>(*mintf).pread(offset);
}

void st2204_device::pmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2204 &>(*mintf).pwrite(offset, data);
}

u8 st2204_device::dmem_r(offs_t offset)
{
	return downcast<mi_st2204 &>(*mintf).dread(offset);
}

void st2204_device::dmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2204 &>(*mintf).dwrite(offset, data);
}

void st2204_device::int_map(address_map &map)
{
	map(0x0000, 0x0004).rw(FUNC(st2204_device::pdata_r), FUNC(st2204_device::pdata_w));
	map(0x0005, 0x0005).rw(FUNC(st2204_device::psc_r), FUNC(st2204_device::psc_w));
	map(0x0008, 0x000c).rw(FUNC(st2204_device::pdata_r), FUNC(st2204_device::pdata_w));
	map(0x000d, 0x000d).rw(FUNC(st2204_device::pfc_r), FUNC(st2204_device::pfc_w));
	map(0x000e, 0x000e).rw(FUNC(st2204_device::pfd_r), FUNC(st2204_device::pfd_w));
	map(0x000f, 0x000f).rw(FUNC(st2204_device::pmcr_r), FUNC(st2204_device::pmcr_w));
	map(0x0020, 0x0020).rw(FUNC(st2204_device::bten_r), FUNC(st2204_device::bten_w));
	map(0x0021, 0x0021).rw(FUNC(st2204_device::btsr_r), FUNC(st2204_device::btsr_w));
	map(0x0030, 0x0030).rw(FUNC(st2204_device::sys_r), FUNC(st2204_device::sys_w));
	map(0x0031, 0x0031).rw(FUNC(st2204_device::irr_r), FUNC(st2204_device::irr_w));
	map(0x0032, 0x0032).rw(FUNC(st2204_device::prrl_r), FUNC(st2204_device::prrl_w));
	map(0x0033, 0x0033).rw(FUNC(st2204_device::prrh_r), FUNC(st2204_device::prrh_w));
	map(0x0034, 0x0034).rw(FUNC(st2204_device::drrl_r), FUNC(st2204_device::drrl_w));
	map(0x0035, 0x0035).rw(FUNC(st2204_device::drrh_r), FUNC(st2204_device::drrh_w));
	map(0x003c, 0x003c).rw(FUNC(st2204_device::ireql_r), FUNC(st2204_device::ireql_w));
	map(0x003d, 0x003d).rw(FUNC(st2204_device::ireqh_r), FUNC(st2204_device::ireqh_w));
	map(0x003e, 0x003e).rw(FUNC(st2204_device::ienal_r), FUNC(st2204_device::ienal_w));
	map(0x003f, 0x003f).rw(FUNC(st2204_device::ienah_r), FUNC(st2204_device::ienah_w));
	map(0x0040, 0x0040).w(FUNC(st2204_device::lssal_w));
	map(0x0041, 0x0041).w(FUNC(st2204_device::lssah_w));
	map(0x0042, 0x0042).w(FUNC(st2204_device::lvpw_w));
	map(0x0043, 0x0043).rw(FUNC(st2204_device::lxmax_r), FUNC(st2204_device::lxmax_w));
	map(0x0044, 0x0044).rw(FUNC(st2204_device::lymax_r), FUNC(st2204_device::lymax_w));
	map(0x004c, 0x004c).rw(FUNC(st2204_device::pl_r), FUNC(st2204_device::pl_w));
	map(0x004e, 0x004e).w(FUNC(st2204_device::pcl_w));
	map(0x0080, 0x287f).ram();
	map(0x4000, 0x7fff).rw(FUNC(st2204_device::pmem_r), FUNC(st2204_device::pmem_w));
	map(0x8000, 0xffff).rw(FUNC(st2204_device::dmem_r), FUNC(st2204_device::dmem_w));
}
