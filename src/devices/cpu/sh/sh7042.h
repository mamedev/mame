// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH704x, sh2 variant

#ifndef MAME_CPU_SH_SH7042_H
#define MAME_CPU_SH_SH7042_H

#pragma once

#include "sh2.h"
#include "sh_intc.h"
#include "sh_adc.h"
#include "sh_cmt.h"
#include "sh_port.h"

class sh7042_device : public sh2_device
{
public:
	sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template<int Port> auto read_adc() { return m_read_adc[Port].bind(); }
	template<int Sci> void sci_rx_w(int state) { do_sci_w(Sci, state); }
	template<int Sci> auto write_sci_tx() { return m_sci_tx[Sci].bind(); }

	auto read_porta()  { return m_read_port32 [0].bind(); }
	auto write_porta() { return m_write_port32[0].bind(); }
	auto read_portb()  { return m_read_port16 [0].bind(); }
	auto write_portb() { return m_write_port16[0].bind(); }
	auto read_portc()  { return m_read_port16 [1].bind(); }
	auto write_portc() { return m_write_port16[1].bind(); }
	auto read_portd()  { return m_read_port32 [1].bind(); }
	auto write_portd() { return m_write_port32[1].bind(); }
	auto read_porte()  { return m_read_port16 [2].bind(); }
	auto write_porte() { return m_write_port16[2].bind(); }
	auto read_portf()  { return m_read_port16 [3].bind(); }

	void internal_update();
	u16 do_read_adc(int port) { return m_read_adc[port](); }
	u16 do_read_port16(int port) { return m_read_port16[port](); }
	void do_write_port16(int port, u16 data, u16 ddr) { m_write_port16[port](0, data, ddr); }
	u32 do_read_port32(int port) { return m_read_port32[port](); }
	void do_write_port32(int port, u32 data, u32 ddr) { m_write_port32[port](0, data, ddr); }

	u64 current_cycles() { return machine().time().as_ticks(clock()); }

	void set_internal_interrupt(int level, u32 vector);

protected:
	const char *m_port16_names;
	const char *m_port32_names;
	enum {
		CSR_ADF  = 0x80,
		CSR_ADIE = 0x40,
		CSR_ADST = 0x20,
		CSR_CKS  = 0x10,
		CSR_GRP  = 0x08,
		CSR_CHAN = 0x07
	};

	enum {
		CR_PWR   = 0x40,
		CR_TRGS  = 0x30,
		CR_SCAN  = 0x08,
		CR_SIM   = 0x04,
		CR_BUF   = 0x03
	};

	sh7042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<sh_intc_device> m_intc;
	required_device<sh_adc_device> m_adc;
	required_device<sh_cmt_device> m_cmt;
	required_device<sh_port32_device> m_porta;
	required_device<sh_port16_device> m_portb;
	required_device<sh_port16_device> m_portc;
	required_device<sh_port32_device> m_portd;
	required_device<sh_port16_device> m_porte;
	required_device<sh_port16_device> m_portf;

	devcb_read16::array<8> m_read_adc;
	devcb_write_line::array<2> m_sci_tx;
	devcb_read16::array<4> m_read_port16;
	devcb_write16::array<4> m_write_port16;
	devcb_read32::array<2> m_read_port32;
	devcb_write32::array<2> m_write_port32;

	emu_timer *m_event_timer;

	u16 m_pcf_ah;
	u32 m_pcf_al;
	u32 m_pcf_b;
	u16 m_pcf_c;
	u32 m_pcf_dh;
	u16 m_pcf_dl;
	u32 m_pcf_e;
	u16 m_pcf_if;

	void map(address_map &map);

	u16 adc_default(int adc);
	u16 port16_default_r(int port);
	void port16_default_w(int port, u16 data);
	u32 port32_default_r(int port);
	void port32_default_w(int port, u32 data);

	void add_event(u64 &event_time, u64 new_event);
	void recompute_timer(u64 event_time);
	TIMER_CALLBACK_MEMBER(event_timer_tick);
	void internal_update(u64 current_time);

	void do_sci_w(int sci, int state);

	u16 pcf_ah_r();
	void pcf_ah_w(offs_t, u16 data, u16 mem_mask);
	u32 pcf_al_r();
	void pcf_al_w(offs_t, u32 data, u32 mem_mask);
	u32 pcf_b_r();
	void pcf_b_w(offs_t, u32 data, u32 mem_mask);
	u16 pcf_c_r();
	void pcf_c_w(offs_t, u16 data, u16 mem_mask);
	u32 pcf_dh_r();
	void pcf_dh_w(offs_t, u32 data, u32 mem_mask);
	u16 pcf_dl_r();
	void pcf_dl_w(offs_t, u16 data, u16 mem_mask);
	u32 pcf_e_r();
	void pcf_e_w(offs_t, u32 data, u32 mem_mask);
	u16 pcf_if_r();
	void pcf_if_w(offs_t, u16 data, u16 mem_mask);
};

class sh7043_device : public sh7042_device
{
public:
	sh7043_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SH7042, sh7042_device)
DECLARE_DEVICE_TYPE(SH7043, sh7043_device)

#endif // MAME_CPU_SH_SH7042_H
