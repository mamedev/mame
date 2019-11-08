// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2205U 8-Bit Integrated Microcontroller

    Functional blocks:
    * Interrupt controller (15 levels excluding BRK and RESET)
    * GPIO (7 ports, 8 bits each)
    * External bus (up to 7 CS outputs, 48M maximum addressable)
    * Timers/event counters with clocking outputs (4 plus base timer)
    * Programmable sound generator (4 channels with FIFOs, plus PWM
      or ADPCM DAC and 16x8 signed multiplicator)
    * LCD controller (640x400 B/W, 400x320 4-gray, 160xRGBx120 16-gray)
    * Serial peripheral interface
    * UART (built-in BRG; RS-232 and IrDA modes)
    * USB 1.1 (separate control and bulk transfer endpoint buffers)
    * Direct memory access (2 channels, optional XOR/OR/AND logic)
    * NAND/AND Flash memory interface (includes ECC generator)
    * Power down modes (WAI-0, WAI-1, STP)
    * Watchdog timer
    * Real time clock (seconds, minutes, hours with alarm interrupts)
    * Low voltage detector with reset
    * 16K OTP ROM (may be disabled)
    * 32K SRAM

    One important difference between the ST2205U and almost every
    other ST2XXX MCU is that PRR[0] and IRR[0] are *not* inverted
    relative to A14.

**********************************************************************/

#include "emu.h"
#include "st2205u.h"

DEFINE_DEVICE_TYPE(ST2205U, st2205u_device, "st2205", "Sitronix ST2205U Integrated Microcontroller")

st2205u_device::st2205u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: st2xxx_device(mconfig, ST2205U, tag, owner, clock,
					address_map_constructor(FUNC(st2205u_device::int_map), this),
					26, // logical; only 23 address lines are brought out
					true)
	, m_btc(0)
	, m_tc_12bit{0}
	, m_t4c(0)
	, m_tien(0)
	, m_lvctr(0)
{
}

void st2205u_device::device_start()
{
	std::unique_ptr<mi_st2205u> intf = std::make_unique<mi_st2205u>();
	intf->data = &space(AS_DATA);
	intf->dcache = space(AS_DATA).cache<0, 0, ENDIANNESS_LITTLE>();
	intf->irr_enable = false;
	intf->irr = 0;
	intf->prr = 0;
	intf->drr = 0;
	intf->brr = 0;
	intf->irq_service = false;
	intf->ram = make_unique_clear<u8[]>(0x8000);

	init_base_timer(0x0040);
	init_lcd_timer(0x0080);

	save_item(NAME(m_btc));
	save_item(NAME(m_tc_12bit));
	save_item(NAME(m_t4c));
	save_item(NAME(m_tien));
	save_item(NAME(m_lvctr));
	save_item(NAME(intf->brr));
	save_pointer(NAME(intf->ram), 0x8000);

	mintf = std::move(intf);
	save_common_registers();
	init();

	state_add(ST_IRR, "IRR", downcast<mi_st2205u &>(*mintf).irr).mask(0x87ff);
	state_add(ST_PRR, "PRR", downcast<mi_st2205u &>(*mintf).prr).mask(0x87ff);
	state_add(ST_DRR, "DRR", downcast<mi_st2205u &>(*mintf).drr).mask(0x8fff);
	state_add(ST_BRR, "BRR", downcast<mi_st2205u &>(*mintf).brr).mask(0x9fff);
	state_add<u16>(ST_IREQ, "IREQ", [this]() { return m_ireq; }, [this](u16 data) { m_ireq = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
	state_add<u16>(ST_IENA, "IENA", [this]() { return m_iena; }, [this](u16 data) { m_iena = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
	for (int i = 0; i < 6; i++)
	{
		state_add(ST_PAOUT + i, string_format("P%cOUT", 'A' + i).c_str(), m_pdata[i]);
		state_add(ST_PCA + i, string_format("PC%c", 'A' + i).c_str(), m_pctrl[i]);
		if (i == 2 || i == 4)
			state_add(ST_PSA + i, string_format("PS%c", 'A' + i).c_str(), m_psel[i]);
		if (i == 2 || i == 3)
			state_add(ST_PFC + i - 2, string_format("PF%c", 'A' + i).c_str(), m_pfun[i - 2]).mask(i == 2 ? 0xfe : 0xff);
	}
	state_add(ST_PLOUT, "PLOUT", m_pdata[6]);
	state_add(ST_PCL, "PCL", m_pctrl[6]);
	state_add(ST_PMCR, "PMCR", m_pmcr);
	state_add(ST_MISC, "MISC", m_misc).mask(st2xxx_misc_mask());
	state_add<u8>(ST_SYS, "SYS", [this]() { return m_sys; }, [this](u8 data) { sys_w(data); }).mask(0xfe);
	state_add<u8>(ST_PRS, "PRS", [this]() { return m_prs; }, [this](u8 data) { prs_w(data); }).mask(0x40);
	state_add<u8>(ST_BTEN, "BTEN", [this]() { return m_bten; }, [this](u8 data) { bten_w(data); });
	state_add(ST_BTSR, "BTREQ", m_btsr);
	state_add(ST_BTC, "BTC", m_btc);
	for (int i = 0; i < 4; i++)
		state_add(ST_T0C + i, string_format("T%dC", i).c_str(), m_tc_12bit[i]);
	state_add(ST_T4C, "T4C", m_t4c);
	state_add(ST_TIEN, "TIEN", m_tien);
	state_add(ST_LSSA, "LSSA", m_lssa);
	state_add(ST_LVPW, "LVPW", m_lvpw);
	state_add(ST_LXMAX, "LXMAX", m_lxmax);
	state_add(ST_LYMAX, "LYMAX", m_lymax);
	state_add(ST_LPAN, "LPAN", m_lpan).mask(st2xxx_lpan_mask());
	state_add(ST_LCTR, "LCTR", m_lctr).mask(st2xxx_lctr_mask());
	state_add(ST_LCKR, "LCKR", m_lckr).mask(st2xxx_lckr_mask());
	state_add(ST_LFRA, "LFRA", m_lfra).mask(0x3f);
	state_add(ST_LAC, "LAC", m_lac).mask(0x1f);
	state_add(ST_LPWM, "LPWM", m_lpwm).mask(st2xxx_lpwm_mask());
	state_add(ST_BCTR, "BCTR", m_bctr).mask(0xb7);
	state_add(ST_BRS, "BRS", m_brs);
	state_add(ST_BDIV, "BDIV", m_bdiv);
	state_add(ST_LVCTR, "LVCTR", m_lvctr).mask(0x0f);
}

void st2205u_device::device_reset()
{
	st2xxx_device::device_reset();

	downcast<mi_st2205u &>(*mintf).brr = 0;

	m_btc = 0;

	std::fill(std::begin(m_tc_12bit), std::end(m_tc_12bit), 0);
	m_t4c = 0;
	m_tien = 0;

	m_lvctr = 0;
}

const char *st2205u_device::st2xxx_irq_name(int i) const
{
	switch (i)
	{
	case 0: return "PE0/1/2 edge";
	case 1: return "Timer 0";
	case 2: return "Timer 1";
	case 3: return "Timer 2";
	case 4: return "Timer 3";
	case 5: return "PA transition";
	case 6: return "Base timer";
	case 7: return "LCD buffer";
	case 8: return "SPI TX empty";
	case 9: return "SPI RX ready";
	case 10: return "UART TX";
	case 11: return "UART RX";
	case 12: return "USB";
	case 14: return "PCM";
	case 15: return "RTC";
	default: return "Reserved";
	}
}

u8 st2205u_device::mi_st2205u::pread(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	if (BIT(bank, 15))
		return ram[0x4000 | (adr & 0x3fff)];
	else
		return data->read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

u8 st2205u_device::mi_st2205u::preadc(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	if (BIT(bank, 15))
		return ram[0x4000 | (adr & 0x3fff)];
	else
		return dcache->read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

void st2205u_device::mi_st2205u::pwrite(u16 adr, u8 val)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	if (BIT(bank, 15))
		ram[0x4000 | (adr & 0x3fff)] = val;
	else
		data->write_byte(u32(bank) << 14 | (adr & 0x3fff), val);
}

u8 st2205u_device::mi_st2205u::dread(u16 adr)
{
	if (BIT(drr, 15))
		return ram[adr & 0x7fff];
	else
		return data->read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

u8 st2205u_device::mi_st2205u::dreadc(u16 adr)
{
	if (BIT(drr, 15))
		return ram[adr & 0x7fff];
	else
		return dcache->read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

void st2205u_device::mi_st2205u::dwrite(u16 adr, u8 val)
{
	if (BIT(drr, 15))
		ram[adr & 0x7fff] = val;
	else
		data->write_byte(u32(drr) << 15 | (adr & 0x7fff), val);
}

u8 st2205u_device::mi_st2205u::bread(u16 adr)
{
	if (BIT(brr, 15))
		return ram[0x2000 | (adr & 0x1fff)];
	else
		return data->read_byte(u32(brr) << 13 | (adr & 0x1fff));
}

u8 st2205u_device::mi_st2205u::breadc(u16 adr)
{
	if (BIT(brr, 15))
		return ram[0x2000 | (adr & 0x1fff)];
	else
		return dcache->read_byte(u32(brr) << 13 | (adr & 0x1fff));
}

void st2205u_device::mi_st2205u::bwrite(u16 adr, u8 val)
{
	if (BIT(brr, 15))
		ram[0x2000 | (adr & 0x1fff)] = val;
	else
		data->write_byte(u32(brr) << 13 | (adr & 0x1fff), val);
}

u8 st2205u_device::mi_st2205u::read(u16 adr)
{
	return program->read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_sync(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : BIT(adr, 13) ? breadc(adr) : cache->read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_arg(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : BIT(adr, 13) ? breadc(adr) : cache->read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_vector(u16 adr)
{
	return pread(adr);
}

void st2205u_device::mi_st2205u::write(u16 adr, u8 val)
{
	program->write_byte(adr, val);
}

u8 st2205u_device::brrl_r()
{
	return downcast<mi_st2205u &>(*mintf).brr & 0xff;
}

void st2205u_device::brrl_w(u8 data)
{
	u16 &brr = downcast<mi_st2205u &>(*mintf).brr;
	brr = data | (brr & 0x9f00);
}

u8 st2205u_device::brrh_r()
{
	return downcast<mi_st2205u &>(*mintf).brr >> 8;
}

void st2205u_device::brrh_w(u8 data)
{
	u16 &brr = downcast<mi_st2205u &>(*mintf).brr;
	brr = (data & 0x9f) << 8 | (brr & 0x00ff);
}

unsigned st2205u_device::st2xxx_bt_divider(int n) const
{
	// 2 Hz
	if (n == 0)
		return 16384;

	// 32 Hz, 64 Hz, 128 Hz, 256 Hz, 512 Hz
	if (n <= 5)
		return 2048 >> n;

	// 2048 Hz
	if (n == 6)
		return 16;

	// 8192 Hz / BTC
	assert(n == 7);
	return 4 * (m_btc != 0 ? m_btc : 256);
}

u8 st2205u_device::btc_r()
{
	return m_btc;
}

void st2205u_device::btc_w(u8 data)
{
	m_btc = data;
}

u8 st2205u_device::tc_12bit_r(offs_t offset)
{
	return (m_tc_12bit[offset >> 1] >> (BIT(offset, 0) ? 8 : 0)) & 0x00ff;
}

void st2205u_device::tc_12bit_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_tc_12bit[offset >> 1] = (m_tc_12bit[offset >> 1] & 0x00ff) | u16(data) << 8;
	else
		m_tc_12bit[offset >> 1] = (m_tc_12bit[offset >> 1] & 0xff00) | data;
}

u8 st2205u_device::t4c_r()
{
	return m_t4c;
}

void st2205u_device::t4c_w(u8 data)
{
	m_t4c = data;
}

u8 st2205u_device::tien_r()
{
	return m_tien;
}

void st2205u_device::tien_w(u8 data)
{
	m_tien = data;
}

void st2205u_device::st2xxx_tclk_start()
{
}

void st2205u_device::st2xxx_tclk_stop()
{
}

unsigned st2205u_device::st2xxx_lfr_clocks() const
{
	unsigned lcdcks = ((m_lxmax * 2 + m_lfra * 4) + 5) * (m_lymax ? m_lymax : 256) * ((m_lctr & 0x03) == 0 ? 2 : 4);

	if ((m_lckr & 0x30) == 0x00 || (m_lckr & 0x30) == 0x30)
		return lcdcks * std::max(((m_lckr & 0x0c) >> 2) * 8, 4);
	else
		return lcdcks * std::max((m_lckr & 0x0f) * 2, 1);
}

u8 st2205u_device::lvctr_r()
{
	return m_lvctr | 0x01;
}

void st2205u_device::lvctr_w(u8 data)
{
	m_lvctr = data & 0x0f;
}

u8 st2205u_device::ram_r(offs_t offset)
{
	return downcast<mi_st2205u &>(*mintf).ram[0x0080 + offset];
}

void st2205u_device::ram_w(offs_t offset, u8 data)
{
	downcast<mi_st2205u &>(*mintf).ram[0x0080 + offset] = data;
}

u8 st2205u_device::pmem_r(offs_t offset)
{
	return downcast<mi_st2205u &>(*mintf).pread(offset);
}

void st2205u_device::pmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2205u &>(*mintf).pwrite(offset, data);
}

u8 st2205u_device::dmem_r(offs_t offset)
{
	return downcast<mi_st2205u &>(*mintf).dread(offset);
}

void st2205u_device::dmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2205u &>(*mintf).dwrite(offset, data);
}

u8 st2205u_device::bmem_r(offs_t offset)
{
	return downcast<mi_st2205u &>(*mintf).bread(offset);
}

void st2205u_device::bmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2205u &>(*mintf).bwrite(offset, data);
}

void st2205u_device::int_map(address_map &map)
{
	map(0x0000, 0x0005).rw(FUNC(st2205u_device::pdata_r), FUNC(st2205u_device::pdata_w));
	map(0x0006, 0x0006).rw(FUNC(st2205u_device::psc_r), FUNC(st2205u_device::psc_w));
	map(0x0007, 0x0007).rw(FUNC(st2205u_device::pse_r), FUNC(st2205u_device::pse_w));
	map(0x0008, 0x000d).rw(FUNC(st2205u_device::pctrl_r), FUNC(st2205u_device::pctrl_w));
	map(0x000e, 0x000e).rw(FUNC(st2205u_device::pfc_r), FUNC(st2205u_device::pfc_w));
	map(0x000f, 0x000f).rw(FUNC(st2205u_device::pfd_r), FUNC(st2205u_device::pfd_w));
	map(0x0020, 0x0027).rw(FUNC(st2205u_device::tc_12bit_r), FUNC(st2205u_device::tc_12bit_w));
	map(0x0028, 0x0028).rw(FUNC(st2205u_device::tien_r), FUNC(st2205u_device::tien_w));
	map(0x0029, 0x0029).rw(FUNC(st2205u_device::prs_r), FUNC(st2205u_device::prs_w));
	map(0x002a, 0x002a).rw(FUNC(st2205u_device::bten_r), FUNC(st2205u_device::bten_w));
	map(0x002b, 0x002b).rw(FUNC(st2205u_device::btsr_r), FUNC(st2205u_device::btclr_w));
	map(0x002c, 0x002c).rw(FUNC(st2205u_device::btc_r), FUNC(st2205u_device::btc_w));
	map(0x002d, 0x002d).rw(FUNC(st2205u_device::t4c_r), FUNC(st2205u_device::t4c_w));
	map(0x0030, 0x0030).rw(FUNC(st2205u_device::irrl_r), FUNC(st2205u_device::irrl_w));
	map(0x0031, 0x0031).rw(FUNC(st2205u_device::irrh_r), FUNC(st2205u_device::irrh_w));
	map(0x0032, 0x0032).rw(FUNC(st2205u_device::prrl_r), FUNC(st2205u_device::prrl_w));
	map(0x0033, 0x0033).rw(FUNC(st2205u_device::prrh_r), FUNC(st2205u_device::prrh_w));
	map(0x0034, 0x0034).rw(FUNC(st2205u_device::drrl_r), FUNC(st2205u_device::drrl_w));
	map(0x0035, 0x0035).rw(FUNC(st2205u_device::drrh_r), FUNC(st2205u_device::drrh_w));
	map(0x0036, 0x0036).rw(FUNC(st2205u_device::brrl_r), FUNC(st2205u_device::brrl_w));
	map(0x0037, 0x0037).rw(FUNC(st2205u_device::brrh_r), FUNC(st2205u_device::brrh_w));
	map(0x0038, 0x0038).rw(FUNC(st2205u_device::misc_r), FUNC(st2205u_device::misc_w));
	map(0x0039, 0x0039).rw(FUNC(st2205u_device::sys_r), FUNC(st2205u_device::sys_w));
	map(0x003a, 0x003a).rw(FUNC(st2205u_device::pmcr_r), FUNC(st2205u_device::pmcr_w));
	map(0x003c, 0x003c).rw(FUNC(st2205u_device::ireql_r), FUNC(st2205u_device::ireql_w));
	map(0x003d, 0x003d).rw(FUNC(st2205u_device::ireqh_r), FUNC(st2205u_device::ireqh_w));
	map(0x003e, 0x003e).rw(FUNC(st2205u_device::ienal_r), FUNC(st2205u_device::ienal_w));
	map(0x003f, 0x003f).rw(FUNC(st2205u_device::ienah_r), FUNC(st2205u_device::ienah_w));
	map(0x0040, 0x0040).w(FUNC(st2205u_device::lssal_w));
	map(0x0041, 0x0041).w(FUNC(st2205u_device::lssah_w));
	map(0x0042, 0x0042).w(FUNC(st2205u_device::lvpw_w));
	map(0x0043, 0x0043).rw(FUNC(st2205u_device::lxmax_r), FUNC(st2205u_device::lxmax_w));
	map(0x0044, 0x0044).rw(FUNC(st2205u_device::lymax_r), FUNC(st2205u_device::lymax_w));
	map(0x0045, 0x0045).rw(FUNC(st2205u_device::lpan_r), FUNC(st2205u_device::lpan_w));
	map(0x0047, 0x0047).rw(FUNC(st2205u_device::lctr_r), FUNC(st2205u_device::lctr_w));
	map(0x0048, 0x0048).w(FUNC(st2205u_device::lckr_w));
	map(0x0049, 0x0049).w(FUNC(st2205u_device::lfra_w));
	map(0x004a, 0x004a).rw(FUNC(st2205u_device::lac_r), FUNC(st2205u_device::lac_w));
	map(0x004b, 0x004b).rw(FUNC(st2205u_device::lpwm_r), FUNC(st2205u_device::lpwm_w));
	map(0x004e, 0x004e).rw(FUNC(st2205u_device::pl_r), FUNC(st2205u_device::pl_w));
	map(0x004f, 0x004f).rw(FUNC(st2205u_device::pcl_r), FUNC(st2205u_device::pcl_w));
	map(0x0057, 0x0057).rw(FUNC(st2205u_device::lvctr_r), FUNC(st2205u_device::lvctr_w));
	map(0x005a, 0x005a).rw(FUNC(st2205u_device::dmrl_r), FUNC(st2205u_device::dmrl_w));
	map(0x005b, 0x005b).rw(FUNC(st2205u_device::dmrh_r), FUNC(st2205u_device::dmrh_w));
	map(0x0063, 0x0063).rw(FUNC(st2205u_device::bctr_r), FUNC(st2205u_device::bctr_w));
	map(0x0066, 0x0066).rw(FUNC(st2205u_device::brs_r), FUNC(st2205u_device::brs_w));
	map(0x0067, 0x0067).rw(FUNC(st2205u_device::bdiv_r), FUNC(st2205u_device::bdiv_w));
	map(0x0080, 0x1fff).rw(FUNC(st2205u_device::ram_r), FUNC(st2205u_device::ram_w)); // assumed to be shared with banked RAM
	map(0x2000, 0x3fff).rw(FUNC(st2205u_device::bmem_r), FUNC(st2205u_device::bmem_w));
	map(0x4000, 0x7fff).rw(FUNC(st2205u_device::pmem_r), FUNC(st2205u_device::pmem_w));
	map(0x8000, 0xffff).rw(FUNC(st2205u_device::dmem_r), FUNC(st2205u_device::dmem_w));
}
