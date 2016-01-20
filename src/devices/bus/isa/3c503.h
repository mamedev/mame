// license:BSD-3-Clause
// copyright-holders:Carl
/* 3com Etherlink II 3c503 */

#ifndef __3C503_H__
#define __3C503_H__

#include "emu.h"
#include "isa.h"
#include "machine/dp8390.h"

class el2_3c503_device: public device_t,
					public device_isa8_card_interface
{
public:
	el2_3c503_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;

	void el2_3c503_irq_w(int state);
	DECLARE_READ8_MEMBER(el2_3c503_mem_read);
	DECLARE_WRITE8_MEMBER(el2_3c503_mem_write);
	DECLARE_READ8_MEMBER(el2_3c503_loport_r);
	DECLARE_WRITE8_MEMBER(el2_3c503_loport_w);
	DECLARE_READ8_MEMBER(el2_3c503_hiport_r);
	DECLARE_WRITE8_MEMBER(el2_3c503_hiport_w);
	void eop_w(int state) override;
	UINT8 dack_r(int line) override;
	void dack_w(int line, UINT8 data) override;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	required_device<dp8390d_device> m_dp8390;
	UINT8 m_board_ram[8*1024];
	UINT8 m_rom[8*1024];
	UINT8 m_prom[32];
	UINT8 m_irq_state;

	UINT8 el2_3c503_mem_read(offs_t offset);
	void el2_3c503_mem_write(offs_t offset, UINT8 data);

	void set_irq(int state);
	void set_drq(int state);

	struct {
		UINT8 pstr;
		UINT8 pspr;
		UINT8 dqtr;
		UINT8 bcfr;
		UINT8 pcfr;
		UINT8 gacfr;
		UINT8 ctrl;
		UINT8 streg;
		UINT8 idcfr;
		UINT16 da;
		UINT32 vptr;
		UINT8 rfmsb;
		UINT8 rflsb;
	} m_regs;
};

extern const device_type EL2_3C503;

#endif
