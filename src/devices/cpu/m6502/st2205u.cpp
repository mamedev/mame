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
    * USB 2.0 (separate control and bulk transfer endpoint buffers)
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

    ST2302U lacks the LCDC, UART and NAND interface and includes only
    8K mask ROM, 2K SRAM and 20 GPIO pins, but appears to be broadly
    similar in other aspects. (The differences, if any, between
    ST2302U and ST2312U are unclear, but ST2301U and ST2331U omit the
    PSG.) It also supports a SYSCLK of up to 24 MHz.

**********************************************************************/

#include "emu.h"
#include "st2205u.h"

#define LOG_DAC (1U << 1)
#define LOG_DMA (1U << 2)
//#define VERBOSE (LOG_DMA)
#include "logmacro.h"

#define LOGDAC(...) LOGMASKED(LOG_DAC, __VA_ARGS__)
#define LOGDMA(...) LOGMASKED(LOG_DMA, __VA_ARGS__)


DEFINE_DEVICE_TYPE(ST2205U, st2205u_device, "st2205u", "Sitronix ST2205U Integrated Microcontroller")
DEFINE_DEVICE_TYPE(ST2302U, st2302u_device, "st2302u", "Sitronix ST2302U Integrated Microcontroller")

st2205u_base_device::st2205u_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map, int data_bits, bool has_banked_ram)
	: st2xxx_device(mconfig, type, tag, owner, clock, internal_map, data_bits, has_banked_ram)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_btc(0)
	, m_tc_12bit{0}
	, m_count_12bit{0}
	, m_timer_12bit{nullptr}
	, m_t4c(0)
	, m_tien(0)
	, m_dac_fifo{{0}}
	, m_fifo_filled{0}
	, m_fifo_pos{0}
	, m_psgc(0)
	, m_psgm(0)
	, m_psg_on(0)
	, m_psg_vol{0}
	, m_psg_volm{0}
	, m_mul(0)
	, m_usbcon(0)
	, m_usbien(0)
	, m_dptr{0}
	, m_dbkr{0}
	, m_dcnt{0}
	, m_dctr(0)
	, m_dmod{0}
	, m_rctr(0)
	, m_lvctr(0)
{
}

st2205u_device::st2205u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: st2205u_base_device(mconfig, ST2205U, tag, owner, clock,
					address_map_constructor(FUNC(st2205u_device::int_map), this),
					26, // logical; only 23 address lines are brought out
					true)
	, m_lbuf(0)
	, m_lpal_index(0)
	, m_gray_levels{0}
{
}

st2302u_device::st2302u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: st2205u_base_device(mconfig, ST2302U, tag, owner, clock,
					address_map_constructor(FUNC(st2302u_device::int_map), this),
					26, // ???
					false)
{
}

void st2205u_base_device::sound_stream_update(sound_stream &stream)
{
	int samples = stream.samples();
	int outpos = 0;
	while (samples-- != 0)
	{
		for (int channel = 0; channel < 4; channel++)
		{
			s16 adpcm_contribution = m_adpcm_level[channel];
			stream.add_int(channel, outpos, adpcm_contribution * 0x10, 32768);

			auto psg_contribution = std::sin((double)m_psg_freqcntr[channel]/4096.0f);
			stream.add_int(channel, outpos, psg_contribution * m_psg_amplitude[channel]*0x80,32768);
		}

		outpos++;
	}
}

void st2205u_base_device::base_init(std::unique_ptr<mi_st2xxx> &&intf)
{
	m_stream = stream_alloc(0, 4, 48000);

	m_timer_12bit[0] = timer_alloc(FUNC(st2205u_device::t0_interrupt), this);
	m_timer_12bit[1] = timer_alloc(FUNC(st2205u_device::t1_interrupt), this);
	m_timer_12bit[2] = timer_alloc(FUNC(st2205u_device::t2_interrupt), this);
	m_timer_12bit[3] = timer_alloc(FUNC(st2205u_device::t3_interrupt), this);

	init_base_timer(0x0040);
	init_lcd_timer(0x0080);

	save_item(NAME(m_btc));
	save_item(NAME(m_tc_12bit));
	save_item(NAME(m_count_12bit));
	save_item(NAME(m_t4c));
	save_item(NAME(m_tien));
	save_item(NAME(m_dac_fifo));
	save_item(NAME(m_fifo_filled));
	save_item(NAME(m_fifo_pos));
	save_item(NAME(m_psgc));
	save_item(NAME(m_psgm));
	save_item(NAME(m_psg_on));
	save_item(NAME(m_psg_vol));
	save_item(NAME(m_psg_volm));
	save_item(NAME(m_mul));
	save_item(NAME(m_usbcon));
	save_item(NAME(m_usbien));
	save_item(NAME(m_dptr));
	save_item(NAME(m_dbkr));
	save_item(NAME(m_dcnt));
	save_item(NAME(m_dctr));
	save_item(NAME(m_dmod));
	save_item(NAME(m_rctr));
	save_item(NAME(m_lvctr));

	save_item(NAME(m_adpcm_level));
	save_item(NAME(m_psg_amplitude));
	save_item(NAME(m_psg_freqcntr));

	mintf = std::move(intf);
	save_common_registers();
	init();
}

void st2205u_device::device_start()
{
	std::unique_ptr<mi_st2205u> intf = std::make_unique<mi_st2205u>();
	space(AS_DATA).specific(intf->data);
	space(AS_DATA).cache(intf->dcache);
	intf->irr_enable = false;
	intf->irr = 0;
	intf->prr = 0;
	intf->drr = 0;
	intf->brr = 0;
	intf->irq_service = false;
	intf->ram = make_unique_clear<u8[]>(0x8000);

	save_item(NAME(m_lbuf));
	save_item(NAME(m_lpal_index));
	save_item(NAME(m_gray_levels));
	save_item(NAME(intf->brr));
	save_pointer(NAME(intf->ram), 0x8000);

	base_init(std::move(intf));

	state_add(ST_IRR, "IRR", downcast<mi_st2205u &>(*mintf).irr).mask(0x8fff);
	state_add(ST_PRR, "PRR", downcast<mi_st2205u &>(*mintf).prr).mask(0x8fff);
	state_add(ST_DRR, "DRR", downcast<mi_st2205u &>(*mintf).drr).mask(0x87ff);
	state_add(ST_BRR, "BRR", downcast<mi_st2205u &>(*mintf).brr).mask(0x9fff);
	state_add(ST_IREQ, "IREQ", m_ireq, [this](u16 data) { m_ireq = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
	state_add(ST_IENA, "IENA", m_iena, [this](u16 data) { m_iena = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
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
	state_add(ST_SYS, "SYS", m_sys, [this](u8 data) { sys_w(data); }).mask(0xfe);
	state_add(ST_PRS, "PRS", m_prs, [this](u8 data) { prs_w(data); }).mask(0x40);
	state_add(ST_BTEN, "BTEN", m_bten, [this](u8 data) { bten_w(data); });
	state_add(ST_BTSR, "BTREQ", m_btsr);
	state_add(ST_BTC, "BTC", m_btc);
	for (int i = 0; i < 4; i++)
		state_add(ST_T0C + i, string_format("T%dC", i).c_str(), m_tc_12bit[i]);
	state_add(ST_T4C, "T4C", m_t4c);
	state_add(ST_TIEN, "TIEN", m_tien);
	for (int i = 0; i < 4; i++)
		state_add(ST_FIFOS0 + i, string_format("FIFOS%d", i).c_str(), m_fifo_filled[i]).mask(0x1f);
	state_add(ST_PSGC, "PSGC", m_psgc);
	state_add(ST_PSGM, "PSGM", m_psgm);
	for (int i = 0; i < 4; i++)
		state_add(ST_VOL0 + i, string_format("VOL%d", i).c_str(), m_psg_vol[i]).mask(0xbf);
	state_add(ST_VOLM0, "VOLM0", m_psg_volm[0]).mask(0x3f);
	state_add(ST_VOLM1, "VOLM1", m_psg_volm[1]).mask(0x7f);
	state_add(ST_MUL, "MUL", m_mul);
	state_add(ST_LSSA, "LSSA", m_lssa);
	state_add(ST_LVPW, "LVPW", m_lvpw);
	state_add(ST_LXMAX, "LXMAX", m_lxmax);
	state_add(ST_LYMAX, "LYMAX", m_lymax);
	state_add(ST_LPAN, "LPAN", m_lpan).mask(st2xxx_lpan_mask());
	state_add(ST_LBUF, "LBUF", m_lbuf);
	state_add(ST_LCTR, "LCTR", m_lctr).mask(st2xxx_lctr_mask());
	state_add(ST_LCKR, "LCKR", m_lckr).mask(st2xxx_lckr_mask());
	state_add(ST_LFRA, "LFRA", m_lfra).mask(0x3f);
	state_add(ST_LAC, "LAC", m_lac).mask(0x1f);
	state_add(ST_LPWM, "LPWM", m_lpwm).mask(st2xxx_lpwm_mask());
	state_add(ST_SCTR, "SCTR", m_sctr);
	state_add(ST_SCKR, "SCKR", m_sckr).mask(0x7f);
	state_add(ST_SSR, "SSR", m_ssr).mask(0x77);
	state_add(ST_SMOD, "SMOD", m_smod).mask(0x0f);
	state_add(ST_UCTR, "UCTR", m_uctr).mask(st2xxx_uctr_mask());
	state_add(ST_USR, "USR", m_usr).mask(0x7f);
	state_add(ST_IRCTR, "IRCTR", m_irctr).mask(0xc7);
	state_add(ST_BCTR, "BCTR", m_bctr).mask(0xb7);
	state_add(ST_BRS, "BRS", m_brs);
	state_add(ST_BDIV, "BDIV", m_bdiv);
	state_add(ST_USBCON, "USBCON", m_usbcon).mask(0xfc);
	state_add(ST_USBIEN, "USBIEN", m_usbien).mask(0xbf);
	for (int i = 0; i < 2; i++)
	{
		state_add(ST_DMS0 + i, string_format("DMS%d", i).c_str(), m_dptr[i * 2]).mask(0x7fff);
		state_add(ST_DMD0 + i, string_format("DMD%d", i).c_str(), m_dptr[i * 2 + 1]).mask(0x7fff);
		state_add(ST_DBKS0 + i, string_format("DBKS%d", i).c_str(), m_dbkr[i * 2]).mask(0x87ff);
		state_add(ST_DBKD0 + i, string_format("DBKD%d", i).c_str(), m_dbkr[i * 2 + 1]).mask(0x87ff);
		state_add(ST_DCNT0 + i, string_format("DCNT%d", i).c_str(), m_dcnt[i]).mask(0x7fff);
		state_add(ST_DMOD0 + i, string_format("DMOD%d", i).c_str(), m_dmod[i]).mask(0x3f);
	}
	state_add(ST_DCTR, "DCTR", m_dctr).mask(0x03);
	state_add(ST_RCTR, "RCTR", m_rctr).mask(0xef);
	state_add(ST_LVCTR, "LVCTR", m_lvctr).mask(0x0f);
}

void st2302u_device::device_start()
{
	std::unique_ptr<mi_st2302u> intf = std::make_unique<mi_st2302u>();
	space(AS_DATA).specific(intf->data);
	space(AS_DATA).cache(intf->dcache);
	intf->irr_enable = false;
	intf->irr = 0;
	intf->prr = 0;
	intf->drr = 0;
	intf->irq_service = false;

	base_init(std::move(intf));

	state_add(ST_IRR, "IRR", downcast<mi_st2302u &>(*mintf).irr).mask(0x0fff);
	state_add(ST_PRR, "PRR", downcast<mi_st2302u &>(*mintf).prr).mask(0x0fff);
	state_add(ST_DRR, "DRR", downcast<mi_st2302u &>(*mintf).drr).mask(0x07ff);
	state_add(ST_IREQ, "IREQ", m_ireq, [this](u16 data) { m_ireq = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
	state_add(ST_IENA, "IENA", m_iena, [this](u16 data) { m_iena = data; update_irq_state(); }).mask(st2xxx_ireq_mask());
	for (int i = 0; i < 6; i++)
	{
		state_add(ST_PAOUT + i, string_format("P%cOUT", 'A' + i).c_str(), m_pdata[i]);
		state_add(ST_PCA + i, string_format("PC%c", 'A' + i).c_str(), m_pctrl[i]);
		if (i == 2 || i == 4)
			state_add(ST_PSA + i, string_format("PS%c", 'A' + i).c_str(), m_psel[i]);
		if (i == 2 || i == 3)
			state_add(ST_PFC + i - 2, string_format("PF%c", 'A' + i).c_str(), m_pfun[i - 2]).mask(i == 2 ? 0xfe : 0xff);
	}
	state_add(ST_PMCR, "PMCR", m_pmcr);
	state_add(ST_MISC, "MISC", m_misc).mask(st2xxx_misc_mask());
	state_add(ST_SYS, "SYS", m_sys, [this](u8 data) { sys_w(data); }).mask(0xfe);
	state_add(ST_PRS, "PRS", m_prs, [this](u8 data) { prs_w(data); }).mask(0x40);
	state_add(ST_BTEN, "BTEN", m_bten, [this](u8 data) { bten_w(data); });
	state_add(ST_BTSR, "BTREQ", m_btsr);
	state_add(ST_BTC, "BTC", m_btc);
	for (int i = 0; i < 4; i++)
		state_add(ST_T0C + i, string_format("T%dC", i).c_str(), m_tc_12bit[i]);
	state_add(ST_T4C, "T4C", m_t4c);
	state_add(ST_TIEN, "TIEN", m_tien);
	for (int i = 0; i < 4; i++)
		state_add(ST_FIFOS0 + i, string_format("FIFOS%d", i).c_str(), m_fifo_filled[i]).mask(0x1f);
	state_add(ST_PSGC, "PSGC", m_psgc);
	state_add(ST_PSGM, "PSGM", m_psgm);
	for (int i = 0; i < 4; i++)
		state_add(ST_VOL0 + i, string_format("VOL%d", i).c_str(), m_psg_vol[i]).mask(0xbf);
	state_add(ST_VOLM0, "VOLM0", m_psg_volm[0]).mask(0x3f);
	state_add(ST_VOLM1, "VOLM1", m_psg_volm[1]).mask(0x7f);
	state_add(ST_MUL, "MUL", m_mul);
	state_add(ST_SCTR, "SCTR", m_sctr);
	state_add(ST_SCKR, "SCKR", m_sckr).mask(0x7f);
	state_add(ST_SSR, "SSR", m_ssr).mask(0x77);
	state_add(ST_SMOD, "SMOD", m_smod).mask(0x0f);
	for (int i = 0; i < 2; i++)
	{
		state_add(ST_DMS0 + i, string_format("DMS%d", i).c_str(), m_dptr[i * 2]).mask(0x7fff);
		state_add(ST_DMD0 + i, string_format("DMD%d", i).c_str(), m_dptr[i * 2 + 1]).mask(0x7fff);
		state_add(ST_DBKS0 + i, string_format("DBKS%d", i).c_str(), m_dbkr[i * 2]).mask(0x87ff);
		state_add(ST_DBKD0 + i, string_format("DBKD%d", i).c_str(), m_dbkr[i * 2 + 1]).mask(0x87ff);
		state_add(ST_DCNT0 + i, string_format("DCNT%d", i).c_str(), m_dcnt[i]).mask(0x7fff);
		state_add(ST_DMOD0 + i, string_format("DMOD%d", i).c_str(), m_dmod[i]).mask(0x3f);
	}
	state_add(ST_DCTR, "DCTR", m_dctr).mask(0x03);
	state_add(ST_RCTR, "RCTR", m_rctr).mask(0xef);
	state_add(ST_LVCTR, "LVCTR", m_lvctr).mask(0x0f);
}

void st2205u_base_device::device_reset()
{
	st2xxx_device::device_reset();

	m_btc = 0;

	std::fill(std::begin(m_tc_12bit), std::end(m_tc_12bit), 0);
	std::fill(std::begin(m_count_12bit), std::end(m_count_12bit), 0);
	for (auto &timer : m_timer_12bit)
		timer->adjust(attotime::never);
	m_t4c = 0;
	m_tien = 0;

	std::fill(std::begin(m_fifo_filled), std::end(m_fifo_filled), 0);
	std::fill(std::begin(m_fifo_pos), std::end(m_fifo_pos), 0);
	m_psgc = 0;
	m_psgm = 0;
	m_psg_on = 0;
	std::fill(std::begin(m_psg_vol), std::end(m_psg_vol), 0);
	std::fill(std::begin(m_psg_volm), std::end(m_psg_volm), 0);
	m_mul = 0;

	m_usbcon = 0;
	m_usbien = 0x20;

	std::fill(std::begin(m_dptr), std::end(m_dptr), 0);
	std::fill(std::begin(m_dbkr), std::end(m_dbkr), 0);
	std::fill(std::begin(m_dcnt), std::end(m_dcnt), 0);
	m_dctr = 0;
	std::fill(std::begin(m_dmod), std::end(m_dmod), 0);

	m_rctr = 0;

	m_lvctr = 0;

	std::fill(std::begin(m_adpcm_level), std::end(m_adpcm_level), 0);
	std::fill(std::begin(m_psg_amplitude), std::end(m_psg_amplitude), 0);
	std::fill(std::begin(m_psg_freqcntr), std::end(m_psg_freqcntr), 0);
}

void st2205u_device::device_reset()
{
	st2205u_base_device::device_reset();

	downcast<mi_st2205u &>(*mintf).brr = 0;

	m_lbuf = 0;
	m_lpal_index = 0;
	std::fill(std::begin(m_gray_levels), std::end(m_gray_levels), 0);
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

const char *st2302u_device::st2xxx_irq_name(int i) const
{
	switch (i)
	{
	case 0: return "P?0/1/2/3 edge";
	case 1: return "Timer 0";
	case 2: return "Timer 1";
	case 3: return "Timer 2";
	case 4: return "Timer 3";
	case 5: return "PA transition";
	case 6: return "Base timer";
	case 8: return "SPI TX empty";
	case 9: return "SPI RX ready";
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
		return data.read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

u8 st2205u_device::mi_st2205u::preadc(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	if (BIT(bank, 15))
		return ram[0x4000 | (adr & 0x3fff)];
	else
		return dcache.read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

void st2205u_device::mi_st2205u::pwrite(u16 adr, u8 val)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	if (BIT(bank, 15))
		ram[0x4000 | (adr & 0x3fff)] = val;
	else
		data.write_byte(u32(bank) << 14 | (adr & 0x3fff), val);
}

u8 st2205u_device::mi_st2205u::dread(u16 adr)
{
	if (BIT(drr, 15))
		return ram[adr & 0x7fff];
	else
		return data.read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

u8 st2205u_device::mi_st2205u::dreadc(u16 adr)
{
	if (BIT(drr, 15))
		return ram[adr & 0x7fff];
	else
		return dcache.read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

void st2205u_device::mi_st2205u::dwrite(u16 adr, u8 val)
{
	if (BIT(drr, 15))
		ram[adr & 0x7fff] = val;
	else
		data.write_byte(u32(drr) << 15 | (adr & 0x7fff), val);
}

u8 st2205u_device::mi_st2205u::bread(u16 adr)
{
	if (BIT(brr, 15))
		return ram[0x2000 | (adr & 0x1fff)];
	else
		return data.read_byte(u32(brr) << 13 | (adr & 0x1fff));
}

u8 st2205u_device::mi_st2205u::breadc(u16 adr)
{
	if (BIT(brr, 15))
		return ram[0x2000 | (adr & 0x1fff)];
	else
		return dcache.read_byte(u32(brr) << 13 | (adr & 0x1fff));
}

void st2205u_device::mi_st2205u::bwrite(u16 adr, u8 val)
{
	if (BIT(brr, 15))
		ram[0x2000 | (adr & 0x1fff)] = val;
	else
		data.write_byte(u32(brr) << 13 | (adr & 0x1fff), val);
}

u8 st2302u_device::mi_st2302u::pread(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	return data.read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

u8 st2302u_device::mi_st2302u::preadc(u16 adr)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	return dcache.read_byte(u32(bank) << 14 | (adr & 0x3fff));
}

void st2302u_device::mi_st2302u::pwrite(u16 adr, u8 val)
{
	u16 bank = irq_service && irr_enable ? irr : prr;
	data.write_byte(u32(bank) << 14 | (adr & 0x3fff), val);
}

u8 st2302u_device::mi_st2302u::dread(u16 adr)
{
	return data.read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

u8 st2302u_device::mi_st2302u::dreadc(u16 adr)
{
	return dcache.read_byte(u32(drr) << 15 | (adr & 0x7fff));
}

void st2302u_device::mi_st2302u::dwrite(u16 adr, u8 val)
{
	data.write_byte(u32(drr) << 15 | (adr & 0x7fff), val);
}

u8 st2205u_device::mi_st2205u::read(u16 adr)
{
	return program.read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_sync(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : BIT(adr, 13) ? breadc(adr) : cprogram.read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_arg(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : BIT(adr, 13) ? breadc(adr) : cprogram.read_byte(adr);
}

u8 st2205u_device::mi_st2205u::read_vector(u16 adr)
{
	return pread(adr);
}

void st2205u_device::mi_st2205u::write(u16 adr, u8 val)
{
	program.write_byte(adr, val);
}

u8 st2302u_device::mi_st2302u::read(u16 adr)
{
	return program.read_byte(adr);
}

u8 st2302u_device::mi_st2302u::read_sync(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : cprogram.read_byte(adr);
}

u8 st2302u_device::mi_st2302u::read_arg(u16 adr)
{
	return BIT(adr, 15) ? dreadc(adr) : BIT(adr, 14) ? preadc(adr) : cprogram.read_byte(adr);
}

u8 st2302u_device::mi_st2302u::read_vector(u16 adr)
{
	return pread(adr);
}

void st2302u_device::mi_st2302u::write(u16 adr, u8 val)
{
	program.write_byte(adr, val);
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

unsigned st2205u_base_device::st2xxx_bt_divider(int n) const
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

u8 st2205u_base_device::btc_r()
{
	return m_btc;
}

void st2205u_base_device::btc_w(u8 data)
{
	m_btc = data;
}

u8 st2205u_base_device::psg_r(offs_t offset)
{
	u8 index = m_fifo_pos[offset >> 1];
	if (BIT(offset, 0))
	{
		bool fwra = m_fifo_filled[offset >> 1] < 8;
		return (m_dac_fifo[offset >> 1][index] & 0x100) >> 1 | (fwra ? 0x60 : 0x40) | m_fifo_filled[offset >> 1];
	}
	else
		return m_dac_fifo[offset >> 1][index] & 0xff;
}

void st2205u_base_device::psg_w(offs_t offset, u8 data)
{
	if (m_fifo_filled[offset >> 1] < 16)
	{
		u8 index = (m_fifo_pos[offset >> 1] + m_fifo_filled[offset >> 1]++) & 15;
		m_dac_fifo[offset >> 1][index] = data | (BIT(offset, 0) ? 0x100 : 0);
	}
}

u8 st2205u_base_device::psgc_r()
{
	return m_psgc;
}

void st2205u_base_device::psgc_w(u8 data)
{
	m_psgc = data;
	m_psg_on &= (data & 0xf0) >> 4;
}

u8 st2205u_base_device::psgm_r()
{
	return m_psgm;
}

void st2205u_base_device::psgm_w(u8 data)
{
	m_psgm = data;
}

u8 st2205u_base_device::vol_r(offs_t offset)
{
	return m_psg_vol[offset] | 0x40;
}

void st2205u_base_device::vol_w(offs_t offset, u8 data)
{
	m_psg_vol[offset] = data & 0xbf;
}

u8 st2205u_base_device::volm_r(offs_t offset)
{
	return m_psg_volm[offset] | (offset == 1 ? 0x80 : 0xc0);
}

void st2205u_base_device::volm_w(offs_t offset, u8 data)
{
	m_psg_volm[offset] = data & (offset == 1 ? 0x7f : 0x3f);
}

void st2205u_base_device::st2xxx_tclk_start()
{
	for (int t = 0; t < 4; t++)
		if (BIT(m_tien, t) && (m_tc_12bit[t] & 0x7000) < 0x6000)
			timer_start_from_tclk(t);
}

void st2205u_base_device::st2xxx_tclk_stop()
{
	for (int t = 0; t < 4; t++)
	{
		if (BIT(m_tien, t) && (m_tc_12bit[t] & 0x7000) < 0x6000)
		{
			m_count_12bit[t] = timer_12bit_count(t);
			m_timer_12bit[t]->adjust(attotime::never);
		}
	}
}

u32 st2205u_base_device::tclk_pres_div(u8 mode) const
{
	assert(mode < 6);

	// dphh8630 game 17 "Gang Nam Style" uses mode 0 for ADPCM music and if a 32Mhz clock is used, requires a divider of 1
	// alternatively the divider can remain as 2 if the code in timer_12bit_process processes the FIFO every call instead
	// of toggling it with m_psg_on, which is correct?
	const int divtable[8] = { 1, 4, 8, 32, 1024, 4096, 4096, 4096 };

	return divtable[mode];
}

TIMER_CALLBACK_MEMBER(st2205u_base_device::t0_interrupt)
{
	timer_12bit_process(0);
}

TIMER_CALLBACK_MEMBER(st2205u_base_device::t1_interrupt)
{
	timer_12bit_process(1);
}

TIMER_CALLBACK_MEMBER(st2205u_base_device::t2_interrupt)
{
	timer_12bit_process(2);
}

TIMER_CALLBACK_MEMBER(st2205u_base_device::t3_interrupt)
{
	timer_12bit_process(3);
}

void st2205u_base_device::push_adpcm_value(int channel, u16 psg_data)
{
	// the ADPCM often ends up off-center before samples are played
	// is the FIFO hookup causing non-ADPCM data to be processed as ADPCM
	// if mode changes in m_psgm aren't in sync with the FIFO output?

	m_stream->update();

	if (BIT(psg_data, 8))
		m_adpcm_level[channel] -= psg_data & 0xff;
	else
		m_adpcm_level[channel] += psg_data & 0xff;

	LOGDAC("Playing ADPCM sample %c%02X on channel %d (new level is %04x)\n", BIT(psg_data, 8) ? '-' : '+', psg_data & 0xff, channel, m_adpcm_level[channel]);
}

void st2205u_base_device::reset_adpcm_value(int channel)
{
	m_stream->update();

	m_adpcm_level[channel] = 0;
}

void st2205u_base_device::timer_12bit_process(int t)
{
	if (BIT(m_psgc, t + 4))
	{
		if (BIT(m_psg_on, t))
			m_psg_on &= ~(1 << t);
		else if (m_fifo_filled[t] != 0)
		{
			m_psg_on |= 1 << t;

			u16 psg_data = m_dac_fifo[t][m_fifo_pos[t]];
			if (BIT(m_psgm, 2 * t + 1))
			{
				push_adpcm_value(t, psg_data);
			}
			else
			{
				reset_adpcm_value(t);
				LOGDAC("Playing %s sample %02X on channel %d\n", BIT(m_psgm, 2 * t) ? "tone" : "DAC", psg_data & 0xff, t);

				m_psg_amplitude[t] = psg_data & 0xff; // amplitude is controller by the data writes
				m_psg_freqcntr[t] += 0x80; // the frequency is determined by the timer speed (there must be a better way to do this?)
			}

			--m_fifo_filled[t];
			m_fifo_pos[t] = (m_fifo_pos[t] + 1) & 15;
		}
	}
	// TODO: PCM has its own FIFO

	// Timer interrupt is triggered when FIFO has more than 8 empty bytes
	if (m_fifo_filled[t] < 8)
	{
		m_ireq |= 0x0002 << t;
		update_irq_state();
	}

	// Bit 7 of TnCH allows auto-reload
	m_count_12bit[t] = BIT(m_tc_12bit[t], 15) ? m_tc_12bit[t] & 0x0fff : 0;

	u8 tck = (m_tc_12bit[t] & 0x7000) >> 12;
	if (tck < 6)
		m_timer_12bit[t]->adjust(cycles_to_attotime((0x1000 - m_count_12bit[t]) * tclk_pres_div(tck)));
	else if (tck == 7 && BIT(t, 0))
		m_timer_12bit[t]->adjust(attotime::from_ticks(0x1000 - m_count_12bit[t], 32768));

	// TODO: BGRCK & INTX sources
}

u16 st2205u_base_device::timer_12bit_count(int t) const
{
	u16 count = m_count_12bit[t];
	if (BIT(m_tien, t))
	{
		u8 tck = (m_tc_12bit[t] & 0x7000) >> 12;
		if (BIT(m_prs, 6) && tck < 6)
			count = 0x0fff - (attotime_to_cycles(m_timer_12bit[t]->remaining()) / tclk_pres_div(tck));
		else if (tck == 7 && BIT(t, 0))
			count = 0x0fff - m_timer_12bit[t]->remaining().as_ticks(32768);
	}

	return count & 0x0fff;
}

void st2205u_base_device::timer_start_from_tclk(int t)
{
	u32 div = tclk_pres_div((m_tc_12bit[t] & 0x7000) >> 12);
	m_timer_12bit[t]->adjust(cycles_to_attotime((0x0fff - m_count_12bit[t]) * div + div - (pres_count() & (div - 1))));
}

void st2205u_base_device::timer_start_from_oscx(int t)
{
	m_timer_12bit[t]->adjust(attotime::from_ticks(0x1000 - m_count_12bit[t], 32768));
}

u8 st2205u_base_device::tc_12bit_r(offs_t offset)
{
	if (BIT(offset, 0))
		return (timer_12bit_count(offset >> 1) | (m_tc_12bit[offset >> 1] & 0xf000)) >> 8;
	else
		return timer_12bit_count(offset >> 1) & 0x00ff;
}

void st2205u_base_device::tc_12bit_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_tc_12bit[offset >> 1] = (m_tc_12bit[offset >> 1] & 0x00ff) | u16(data) << 8;
	else
		m_tc_12bit[offset >> 1] = (m_tc_12bit[offset >> 1] & 0xff00) | data;
}

u8 st2205u_base_device::t4c_r()
{
	return m_t4c;
}

void st2205u_base_device::t4c_w(u8 data)
{
	m_t4c = data;
}

u8 st2205u_base_device::tien_r()
{
	return m_tien;
}

void st2205u_base_device::tien_w(u8 data)
{
	for (int t = 0; t < 4; t++)
	{
		if (BIT(m_tien, t) && !BIT(data, t))
		{
			m_count_12bit[t] = timer_12bit_count(t);
			m_timer_12bit[t]->adjust(attotime::never);
		}
		else if (!BIT(m_tien, t) && BIT(data, t))
		{
			m_count_12bit[t] = m_tc_12bit[t] & 0x0fff;

			u8 tck = (m_tc_12bit[t] & 0x7000) >> 12;
			if (BIT(m_prs, 6) && tck < 6)
				timer_start_from_tclk(t);
			else if (tck == 7 && BIT(t, 0))
				timer_start_from_oscx(t);
		}
	}

	m_tien = data;
}

unsigned st2205u_device::st2xxx_lfr_clocks() const
{
	unsigned lcdcks = ((m_lxmax * 2 + m_lfra * 4) + 5) * (m_lymax ? m_lymax : 256) * ((m_lctr & 0x03) == 0 ? 2 : 4);

	if ((m_lckr & 0x30) == 0x00 || (m_lckr & 0x30) == 0x30)
		return lcdcks * std::max(((m_lckr & 0x0c) >> 2) * 8, 4);
	else
		return lcdcks * std::max((m_lckr & 0x0f) * 2, 1);
}

u8 st2205u_device::lbuf_r()
{
	return m_lbuf;
}

void st2205u_device::lbuf_w(u8 data)
{
	m_lbuf = data;
}

void st2205u_device::lpal_w(u8 data)
{
	u8 index = m_lpal_index++;
	m_lpal_index &= 0x0f;
	if ((m_lctr & 0x0c) == 0x04)
		index = (m_lpwm & 0xc0) >> 4 | (index & 0x03);

	m_gray_levels[index] = data & 0x1f;
}

u8 st2205u_base_device::usbcon_r()
{
	return m_usbcon | 0x03;
}

void st2205u_base_device::usbcon_w(u8 data)
{
	m_usbcon = data & 0xfc;
}

u8 st2205u_base_device::usbien_r()
{
	return m_usbien | 0x40;
}

void st2205u_base_device::usbien_w(u8 data)
{
	m_usbien = data & 0xbf;
}

u8 st2205u_base_device::dptrl_r()
{
	return m_dptr[m_dctr] & 0x00ff;
}

void st2205u_base_device::dptrl_w(u8 data)
{
	m_dptr[m_dctr] = (m_dptr[m_dctr] & 0x7f00) | data;
}

u8 st2205u_base_device::dptrh_r()
{
	return (m_dptr[m_dctr] >> 8) | 0x80;
}

void st2205u_base_device::dptrh_w(u8 data)
{
	m_dptr[m_dctr] = u16(data & 0x7f) << 8 | (m_dptr[m_dctr] & 0x00ff);
}

u8 st2205u_base_device::dbkrl_r()
{
	return m_dbkr[m_dctr] & 0x00ff;
}

void st2205u_base_device::dbkrl_w(u8 data)
{
	m_dbkr[m_dctr] = (m_dbkr[m_dctr] & 0x8700) | data;
}

u8 st2205u_base_device::dbkrh_r()
{
	return (m_dbkr[m_dctr] >> 8) | 0x78;
}

void st2205u_base_device::dbkrh_w(u8 data)
{
	m_dbkr[m_dctr] = u16(data & 0x87) << 8 | (m_dbkr[m_dctr] & 0x00ff);
}

u8 st2205u_base_device::dcntl_r()
{
	return m_dcnt[m_dctr >> 1] & 0x00ff;
}

void st2205u_base_device::dcntl_w(u8 data)
{
	m_dcnt[m_dctr >> 1] = (m_dcnt[m_dctr >> 1] & 0x7f00) | data;
}

u8 st2205u_base_device::dcnth_r()
{
	return (m_dcnt[m_dctr >> 1] >> 8) | 0x80;
}

void st2205u_base_device::dcnth_w(u8 data)
{
	m_dcnt[m_dctr >> 1] = (data & 0x7f) << 8 | (m_dcnt[m_dctr >> 1] & 0x00ff);

	// start DMA
	u16 srcp = m_dptr[m_dctr & 2];
	u16 srcb = m_dbkr[m_dctr & 2];
	u16 dstp = m_dptr[m_dctr | 1];
	u16 dstb = m_dbkr[m_dctr | 1];
	u16 count = m_dcnt[m_dctr >> 1] + 1;
	const u8 mode = m_dmod[m_dctr >> 1];

	LOGDMA("%s: DMA%d $%X bytes in mode $%02X from $%X (%s) to $%X (%s)\n",
			machine().describe_context(),
			m_dctr >> 1, count, mode,
			srcp | (srcb & 0x7ff) << 15, BIT(srcb, 15) ? "RAM" : "ROM",
			dstp | (dstb & 0x7ff) << 15, BIT(dstb, 15) ? "RAM" : "ROM");

	// FIXME: DMA should be performed in the execution loop and consume bus cycles, not happen instantly
	while (count-- != 0)
	{
		uint8_t data;
		if (BIT(srcb, 15))
			data = mintf->cprogram.read_byte(srcp); // FIXME: 0080-7FFF should be all RAM on ST2205U
		else
			data = downcast<mi_st2xxx &>(*mintf).dcache.read_byte(srcp | u32(srcb << 15));
		if (!BIT(mode, 1))
		{
			if (srcp++ == 0x7fff)
			{
				srcp = 0;
				srcb++;
			}
		}

		// TODO: XOR/OR/AND logic for DMA0 three-cycle modes (different on ST23XX?)
		if (BIT(dstb, 15))
			mintf->cprogram.write_byte(dstp, data); // FIXME: 0080-7FFF should be all RAM on ST2205U
		else
			downcast<mi_st2xxx &>(*mintf).dcache.write_byte(dstp | u32(dstb << 15), data);
		if (!BIT(mode, 3))
		{
			if (dstp++ == 0x7fff)
			{
				dstp = 0;
				dstb++;
			}
		}
	}

	// update pointers for continue mode
	if ((mode & 0x03) == 0x00)
	{
		m_dptr[m_dctr & 2] = srcp;
		m_dbkr[m_dctr & 2] = srcb & 0x87ff;
	}
	if ((mode & 0x0c) == 0x00)
	{
		m_dptr[m_dctr | 1] = dstp;
		m_dbkr[m_dctr | 1] = dstb & 0x87ff;
	}
}

u8 st2205u_base_device::dctr_r()
{
	return m_dctr | 0xfc;
}

void st2205u_base_device::dctr_w(u8 data)
{
	m_dctr = data & 0x03;
}

u8 st2205u_base_device::dmod_r()
{
	return m_dmod[m_dctr >> 1] | 0xc0;
}

void st2205u_base_device::dmod_w(u8 data)
{
	m_dmod[m_dctr >> 1] = data & 0x3f;
}

u8 st2205u_base_device::rctr_r()
{
	return (m_rctr & 0xe0) | 0x10;
}

void st2205u_base_device::rctr_w(u8 data)
{
	m_rctr = data & 0xef;
}

u8 st2205u_base_device::lvctr_r()
{
	return m_lvctr | 0x01;
}

void st2205u_base_device::lvctr_w(u8 data)
{
	m_lvctr = data & 0x0f;
}

u8 st2205u_base_device::mull_r()
{
	return m_mul & 0x00ff;
}

void st2205u_base_device::mull_w(u8 data)
{
	// TODO: result loaded 6 cycles after multiplier is written
	m_mul = (s32(s16(m_mul)) * s8(data)) >> 8;
}

u8 st2205u_base_device::mulh_r()
{
	return m_mul >> 8;
}

void st2205u_base_device::mulh_w(u8 data)
{
	// write low byte of multiplicand, then high byte
	m_mul = u16(data) << 8 | m_mul >> 8;
}

void st2302u_device::unk18_w(u8 data)
{
	logerror("%s: Writing %02X to unknown register $18\n", machine().describe_context(), data);
}

void st2302u_device::unk6d_w(u8 data)
{
	// $6D is PCMH on ST2205U, but probably not here
	logerror("%s: Writing %02X to unknown register $6D\n", machine().describe_context(), data);
}

void st2302u_device::unk6e_w(u8 data)
{
	// $6E is MULL on ST2205U, but definitely not here
	logerror("%s: Writing %02X to unknown register $6E\n", machine().describe_context(), data);
}

u8 st2302u_device::unk7b_r()
{
	// code usually waits for bit 3 to set itself after writing #$01 to this address
	return 0x08;
}

void st2302u_device::unk7b_w(u8 data)
{
	logerror("%s: Writing %02X to unknown register $7B\n", machine().describe_context(), data);
}

void st2302u_device::unk7c_w(u8 data)
{
	logerror("%s: Writing %02X to unknown register $7C\n", machine().describe_context(), data);
}

void st2302u_device::unk7d_w(u8 data)
{
	// usually written with #$05 after the $7B wait is over
	logerror("%s: Writing %02X to unknown register $7D\n", machine().describe_context(), data);
}

void st2302u_device::unk7e_w(u8 data)
{
	// written in succession with $7F, sometimes several times
	logerror("%s: Writing %02X to unknown register $7E\n", machine().describe_context(), data);
}

void st2302u_device::unk7f_w(u8 data)
{
	// written in succession with $7E, sometimes several times
	logerror("%s: Writing %02X to unknown register $7F\n", machine().describe_context(), data);
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

u8 st2302u_device::pmem_r(offs_t offset)
{
	return downcast<mi_st2302u &>(*mintf).pread(offset);
}

void st2302u_device::pmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2302u &>(*mintf).pwrite(offset, data);
}

u8 st2302u_device::dmem_r(offs_t offset)
{
	return downcast<mi_st2302u &>(*mintf).dread(offset);
}

void st2302u_device::dmem_w(offs_t offset, u8 data)
{
	downcast<mi_st2302u &>(*mintf).dwrite(offset, data);
}

void st2205u_base_device::base_map(address_map &map)
{
	map(0x0020, 0x0027).rw(FUNC(st2205u_base_device::tc_12bit_r), FUNC(st2205u_base_device::tc_12bit_w));
	map(0x0028, 0x0028).rw(FUNC(st2205u_base_device::tien_r), FUNC(st2205u_base_device::tien_w));
	map(0x0029, 0x0029).rw(FUNC(st2205u_base_device::prs_r), FUNC(st2205u_base_device::prs_w));
	map(0x002a, 0x002a).rw(FUNC(st2205u_base_device::bten_r), FUNC(st2205u_base_device::bten_w));
	map(0x002b, 0x002b).rw(FUNC(st2205u_base_device::btsr_r), FUNC(st2205u_base_device::btclr_w));
	map(0x002c, 0x002c).rw(FUNC(st2205u_base_device::btc_r), FUNC(st2205u_base_device::btc_w));
	map(0x002d, 0x002d).rw(FUNC(st2205u_base_device::t4c_r), FUNC(st2205u_base_device::t4c_w));
	map(0x002e, 0x002e).rw(FUNC(st2205u_base_device::rctr_r), FUNC(st2205u_base_device::rctr_w));
	map(0x0030, 0x0030).rw(FUNC(st2205u_base_device::irrl_r), FUNC(st2205u_base_device::irrl_w));
	map(0x0031, 0x0031).rw(FUNC(st2205u_base_device::irrh_r), FUNC(st2205u_base_device::irrh_w));
	map(0x0032, 0x0032).rw(FUNC(st2205u_base_device::prrl_r), FUNC(st2205u_base_device::prrl_w));
	map(0x0033, 0x0033).rw(FUNC(st2205u_base_device::prrh_r), FUNC(st2205u_base_device::prrh_w));
	map(0x0034, 0x0034).rw(FUNC(st2205u_base_device::drrl_r), FUNC(st2205u_base_device::drrl_w));
	map(0x0035, 0x0035).rw(FUNC(st2205u_base_device::drrh_r), FUNC(st2205u_base_device::drrh_w));
	map(0x0038, 0x0038).rw(FUNC(st2205u_base_device::misc_r), FUNC(st2205u_base_device::misc_w));
	map(0x0039, 0x0039).rw(FUNC(st2205u_base_device::sys_r), FUNC(st2205u_base_device::sys_w));
	map(0x003c, 0x003c).rw(FUNC(st2205u_base_device::ireql_r), FUNC(st2205u_base_device::ireql_w));
	map(0x003d, 0x003d).rw(FUNC(st2205u_base_device::ireqh_r), FUNC(st2205u_base_device::ireqh_w));
	map(0x003e, 0x003e).rw(FUNC(st2205u_base_device::ienal_r), FUNC(st2205u_base_device::ienal_w));
	map(0x003f, 0x003f).rw(FUNC(st2205u_base_device::ienah_r), FUNC(st2205u_base_device::ienah_w));
	map(0x0058, 0x0058).rw(FUNC(st2205u_base_device::dptrl_r), FUNC(st2205u_base_device::dptrl_w));
	map(0x0059, 0x0059).rw(FUNC(st2205u_base_device::dptrh_r), FUNC(st2205u_base_device::dptrh_w));
	map(0x005a, 0x005a).rw(FUNC(st2205u_base_device::dbkrl_r), FUNC(st2205u_base_device::dbkrl_w));
	map(0x005b, 0x005b).rw(FUNC(st2205u_base_device::dbkrh_r), FUNC(st2205u_base_device::dbkrh_w));
	map(0x005c, 0x005c).rw(FUNC(st2205u_base_device::dcntl_r), FUNC(st2205u_base_device::dcntl_w));
	map(0x005d, 0x005d).rw(FUNC(st2205u_base_device::dcnth_r), FUNC(st2205u_base_device::dcnth_w));
	map(0x005e, 0x005e).rw(FUNC(st2205u_base_device::dctr_r), FUNC(st2205u_base_device::dctr_w));
	map(0x005f, 0x005f).rw(FUNC(st2205u_base_device::dmod_r), FUNC(st2205u_base_device::dmod_w));
	map(0x0070, 0x0070).rw(FUNC(st2205u_base_device::usbcon_r), FUNC(st2205u_base_device::usbcon_w));
	map(0x0071, 0x0071).rw(FUNC(st2205u_base_device::usbien_r), FUNC(st2205u_base_device::usbien_w));
}

void st2205u_device::int_map(address_map &map)
{
	base_map(map);
	map(0x0000, 0x0005).rw(FUNC(st2205u_device::pdata_r), FUNC(st2205u_device::pdata_w));
	map(0x0006, 0x0006).rw(FUNC(st2205u_device::psc_r), FUNC(st2205u_device::psc_w));
	map(0x0007, 0x0007).rw(FUNC(st2205u_device::pse_r), FUNC(st2205u_device::pse_w));
	map(0x0008, 0x000d).rw(FUNC(st2205u_device::pctrl_r), FUNC(st2205u_device::pctrl_w));
	map(0x000e, 0x000e).rw(FUNC(st2205u_device::pfc_r), FUNC(st2205u_device::pfc_w));
	map(0x000f, 0x000f).rw(FUNC(st2205u_device::pfd_r), FUNC(st2205u_device::pfd_w));
	map(0x0010, 0x0017).rw(FUNC(st2205u_device::psg_r), FUNC(st2205u_device::psg_w));
	map(0x0018, 0x001b).rw(FUNC(st2205u_device::vol_r), FUNC(st2205u_device::vol_w));
	map(0x001c, 0x001d).rw(FUNC(st2205u_device::volm_r), FUNC(st2205u_device::volm_w));
	map(0x001e, 0x001e).rw(FUNC(st2205u_device::psgc_r), FUNC(st2205u_device::psgc_w));
	map(0x001f, 0x001f).rw(FUNC(st2205u_device::psgm_r), FUNC(st2205u_device::psgm_w));
	map(0x0036, 0x0036).rw(FUNC(st2205u_device::brrl_r), FUNC(st2205u_device::brrl_w));
	map(0x0037, 0x0037).rw(FUNC(st2205u_device::brrh_r), FUNC(st2205u_device::brrh_w));
	map(0x003a, 0x003a).rw(FUNC(st2205u_device::pmcr_r), FUNC(st2205u_device::pmcr_w));
	map(0x0040, 0x0040).w(FUNC(st2205u_device::lssal_w));
	map(0x0041, 0x0041).w(FUNC(st2205u_device::lssah_w));
	map(0x0042, 0x0042).w(FUNC(st2205u_device::lvpw_w));
	map(0x0043, 0x0043).rw(FUNC(st2205u_device::lxmax_r), FUNC(st2205u_device::lxmax_w));
	map(0x0044, 0x0044).rw(FUNC(st2205u_device::lymax_r), FUNC(st2205u_device::lymax_w));
	map(0x0045, 0x0045).rw(FUNC(st2205u_device::lpan_r), FUNC(st2205u_device::lpan_w));
	map(0x0046, 0x0046).rw(FUNC(st2205u_device::lbuf_r), FUNC(st2205u_device::lbuf_w));
	map(0x0047, 0x0047).rw(FUNC(st2205u_device::lctr_r), FUNC(st2205u_device::lctr_w));
	map(0x0048, 0x0048).w(FUNC(st2205u_device::lckr_w));
	map(0x0049, 0x0049).w(FUNC(st2205u_device::lfra_w));
	map(0x004a, 0x004a).rw(FUNC(st2205u_device::lac_r), FUNC(st2205u_device::lac_w));
	map(0x004b, 0x004b).rw(FUNC(st2205u_device::lpwm_r), FUNC(st2205u_device::lpwm_w));
	map(0x004c, 0x004c).w(FUNC(st2205u_device::lpal_w));
	map(0x004e, 0x004e).rw(FUNC(st2205u_device::pl_r), FUNC(st2205u_device::pl_w));
	map(0x004f, 0x004f).rw(FUNC(st2205u_device::pcl_r), FUNC(st2205u_device::pcl_w));
	map(0x0052, 0x0052).rw(FUNC(st2205u_device::sctr_r), FUNC(st2205u_device::sctr_w));
	map(0x0053, 0x0053).rw(FUNC(st2205u_device::sckr_r), FUNC(st2205u_device::sckr_w));
	map(0x0054, 0x0054).rw(FUNC(st2205u_device::ssr_r), FUNC(st2205u_device::ssr_w));
	map(0x0055, 0x0055).rw(FUNC(st2205u_device::smod_r), FUNC(st2205u_device::smod_w));
	map(0x0057, 0x0057).rw(FUNC(st2205u_device::lvctr_r), FUNC(st2205u_device::lvctr_w));
	map(0x0060, 0x0060).rw(FUNC(st2205u_device::uctr_r), FUNC(st2205u_device::uctr_w));
	map(0x0061, 0x0061).rw(FUNC(st2205u_device::usr_r), FUNC(st2205u_device::usr_clr_w));
	map(0x0062, 0x0062).rw(FUNC(st2205u_device::irctr_r), FUNC(st2205u_device::irctr_w));
	map(0x0063, 0x0063).rw(FUNC(st2205u_device::bctr_r), FUNC(st2205u_device::bctr_w));
	map(0x0064, 0x0064).rw(FUNC(st2205u_device::udata_r), FUNC(st2205u_device::udata_w));
	map(0x0066, 0x0066).rw(FUNC(st2205u_device::brs_r), FUNC(st2205u_device::brs_w));
	map(0x0067, 0x0067).rw(FUNC(st2205u_device::bdiv_r), FUNC(st2205u_device::bdiv_w));
	map(0x006e, 0x006e).rw(FUNC(st2205u_device::mull_r), FUNC(st2205u_device::mull_w));
	map(0x006f, 0x006f).rw(FUNC(st2205u_device::mulh_r), FUNC(st2205u_device::mulh_w));
	map(0x0080, 0x1fff).rw(FUNC(st2205u_device::ram_r), FUNC(st2205u_device::ram_w)); // assumed to be shared with banked RAM
	map(0x2000, 0x3fff).rw(FUNC(st2205u_device::bmem_r), FUNC(st2205u_device::bmem_w));
	map(0x4000, 0x7fff).rw(FUNC(st2205u_device::pmem_r), FUNC(st2205u_device::pmem_w));
	map(0x8000, 0xffff).rw(FUNC(st2205u_device::dmem_r), FUNC(st2205u_device::dmem_w));
}

void st2302u_device::int_map(address_map &map)
{
	base_map(map);
	map(0x0000, 0x0005).rw(FUNC(st2302u_device::pdata_r), FUNC(st2302u_device::pdata_w));
	map(0x0006, 0x0006).rw(FUNC(st2302u_device::psc_r), FUNC(st2302u_device::psc_w));
	map(0x0008, 0x000d).rw(FUNC(st2302u_device::pctrl_r), FUNC(st2302u_device::pctrl_w));
	map(0x000e, 0x000e).rw(FUNC(st2302u_device::pfc_r), FUNC(st2302u_device::pfc_w));
	map(0x000f, 0x000f).rw(FUNC(st2302u_device::pfd_r), FUNC(st2302u_device::pfd_w));
	map(0x0012, 0x0012).rw(FUNC(st2302u_device::sctr_r), FUNC(st2302u_device::sctr_w));
	map(0x0013, 0x0013).rw(FUNC(st2302u_device::sckr_r), FUNC(st2302u_device::sckr_w));
	map(0x0014, 0x0014).rw(FUNC(st2302u_device::ssr_r), FUNC(st2302u_device::ssr_w));
	map(0x0015, 0x0015).rw(FUNC(st2302u_device::smod_r), FUNC(st2302u_device::smod_w));
	map(0x0016, 0x0016).rw(FUNC(st2302u_device::mull_r), FUNC(st2302u_device::mull_w));
	map(0x0017, 0x0017).rw(FUNC(st2302u_device::mulh_r), FUNC(st2302u_device::mulh_w));
	map(0x0018, 0x0018).w(FUNC(st2302u_device::unk18_w));
	map(0x0040, 0x0047).rw(FUNC(st2302u_device::psg_r), FUNC(st2302u_device::psg_w));
	map(0x0048, 0x004b).rw(FUNC(st2302u_device::vol_r), FUNC(st2302u_device::vol_w));
	map(0x004c, 0x004d).rw(FUNC(st2302u_device::volm_r), FUNC(st2302u_device::volm_w));
	map(0x004e, 0x004e).rw(FUNC(st2302u_device::psgc_r), FUNC(st2302u_device::psgc_w));
	map(0x004f, 0x004f).rw(FUNC(st2302u_device::psgm_r), FUNC(st2302u_device::psgm_w));
	map(0x006d, 0x006d).w(FUNC(st2302u_device::unk6d_w));
	map(0x006e, 0x006e).w(FUNC(st2302u_device::unk6e_w));
	map(0x007b, 0x007b).rw(FUNC(st2302u_device::unk7b_r), FUNC(st2302u_device::unk7b_w));
	map(0x007c, 0x007c).w(FUNC(st2302u_device::unk7c_w));
	map(0x007d, 0x007d).w(FUNC(st2302u_device::unk7d_w));
	map(0x007e, 0x007e).w(FUNC(st2302u_device::unk7e_w));
	map(0x007f, 0x007f).w(FUNC(st2302u_device::unk7f_w));
	map(0x0080, 0x07ff).ram();
	map(0x4000, 0x7fff).rw(FUNC(st2302u_device::pmem_r), FUNC(st2302u_device::pmem_w));
	map(0x8000, 0xffff).rw(FUNC(st2302u_device::dmem_r), FUNC(st2302u_device::dmem_w));
}
