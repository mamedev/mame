// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2204 8-Bit Integrated Microcontroller

**********************************************************************/

#ifndef MAME_CPU_M6502_ST2204_H
#define MAME_CPU_M6502_ST2204_H

#pragma once

#include "st2xxx.h"

class st2204_device : public st2xxx_device
{
public:
	enum {
		ST_BTEN = ST_LYMAX + 1,
		ST_BTSR,
		ST_DMS,
		ST_DMD,
		ST_DMR
	};

	st2204_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	class mi_st2204 : public mi_st2xxx {
	public:
		virtual u8 read(u16 adr) override;
		virtual u8 read_sync(u16 adr) override;
		virtual u8 read_arg(u16 adr) override;
		u8 read_dma(u16 adr);
		virtual u8 read_vector(u16 adr) override;
		virtual void write(u16 adr, u8 val) override;

		u8 pread(u16 adr);
		u8 preadc(u16 adr);
		void pwrite(u16 adr, u8 val);
		u8 dread(u16 adr);
		u8 dreadc(u16 adr);
		void dwrite(u16 adr, u8 val);

		bool irr_enable;
		u8 irr;
		u16 prr;
		u16 drr;
		u16 dmr;
	};

	template<int N> TIMER_CALLBACK_MEMBER(bt_interrupt);

	u8 sys_r();
	void sys_w(u8 data);
	u8 irr_r();
	void irr_w(u8 data);
	u8 prrl_r();
	void prrl_w(u8 data);
	u8 prrh_r();
	void prrh_w(u8 data);
	u8 drrl_r();
	void drrl_w(u8 data);
	u8 drrh_r();
	void drrh_w(u8 data);
	u8 pmcr_r();
	void pmcr_w(u8 data);
	u8 bten_r();
	void bten_w(u8 data);
	u8 btsr_r();
	void btsr_w(u8 data);
	u8 dmsl_r();
	void dmsl_w(u8 data);
	u8 dmsh_r();
	void dmsh_w(u8 data);
	u8 dmdl_r();
	void dmdl_w(u8 data);
	u8 dmdh_r();
	void dmdh_w(u8 data);
	void dcntl_w(u8 data);
	void dcnth_w(u8 data);
	u8 dmrl_r();
	void dmrl_w(u8 data);
	u8 dmrh_r();
	void dmrh_w(u8 data);

	u8 pmem_r(offs_t offset);
	void pmem_w(offs_t offset, u8 data);
	u8 dmem_r(offs_t offset);
	void dmem_w(offs_t offset, u8 data);

	void int_map(address_map &map);

	u8 m_bten;
	u8 m_btsr;
	emu_timer *m_base_timer[5];

	u16 m_dms;
	u16 m_dmd;
	u8 m_dcnth;
};

DECLARE_DEVICE_TYPE(ST2204, st2204_device)

#endif // MAME_MACHINE_M6502_ST2204_H
