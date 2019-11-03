// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2205U 8-Bit Integrated Microcontroller

**********************************************************************/

#ifndef MAME_CPU_M6502_ST2205U_H
#define MAME_CPU_M6502_ST2205U_H

#pragma once

#include "st2xxx.h"

class st2205u_device : public st2xxx_device
{
public:
	enum {
		ST_BTEN = ST_LYMAX + 1,
		ST_BTREQ,
		ST_BTC,
		ST_BRR,
		ST_LVCTR
	};

	st2205u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	class mi_st2205u : public mi_st2xxx {
	public:
		virtual u8 read(u16 adr) override;
		virtual u8 read_sync(u16 adr) override;
		virtual u8 read_arg(u16 adr) override;
		virtual u8 read_vector(u16 adr) override;
		virtual void write(u16 adr, u8 val) override;

		u8 pread(u16 adr);
		u8 preadc(u16 adr);
		void pwrite(u16 adr, u8 val);
		u8 dread(u16 adr);
		u8 dreadc(u16 adr);
		void dwrite(u16 adr, u8 val);
		u8 bread(u16 adr);
		u8 breadc(u16 adr);
		void bwrite(u16 adr, u8 val);

		bool irr_enable;
		u16 irr;
		u16 prr;
		u16 drr;
		u16 brr;

		std::unique_ptr<u8[]> ram;
	};

	template<int N> TIMER_CALLBACK_MEMBER(bt_interrupt);

	u8 irrl_r();
	void irrl_w(u8 data);
	u8 irrh_r();
	void irrh_w(u8 data);
	u8 prrl_r();
	void prrl_w(u8 data);
	u8 prrh_r();
	void prrh_w(u8 data);
	u8 drrl_r();
	void drrl_w(u8 data);
	u8 drrh_r();
	void drrh_w(u8 data);
	u8 brrl_r();
	void brrl_w(u8 data);
	u8 brrh_r();
	void brrh_w(u8 data);
	u8 sys_r();
	void sys_w(u8 data);
	u8 pmcr_r();
	void pmcr_w(u8 data);
	u8 bten_r();
	void bten_w(u8 data);
	u8 btreq_r();
	void btclr_w(u8 data);
	u8 btc_r();
	void btc_w(u8 data);
	u8 lvctr_r();
	void lvctr_w(u8 data);

	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	u8 pmem_r(offs_t offset);
	void pmem_w(offs_t offset, u8 data);
	u8 dmem_r(offs_t offset);
	void dmem_w(offs_t offset, u8 data);
	u8 bmem_r(offs_t offset);
	void bmem_w(offs_t offset, u8 data);

	void int_map(address_map &map);

	u8 m_bten;
	u8 m_btreq;
	u8 m_btc;
	emu_timer *m_base_timer[8];
	u8 m_lvctr;
};

DECLARE_DEVICE_TYPE(ST2205U, st2205u_device)

#endif // MAME_MACHINE_M6502_ST2205_H
