// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    K1801VM1 on-chip timer

**********************************************************************/

#ifndef MAME_USSR_VM1TIMER_H
#define MAME_USSR_VM1TIMER_H

#pragma once

#include "machine/pdp11.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class k1801vm1_timer_device : public device_t
{
public:
	// construction/destruction
	k1801vm1_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void init_w();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr uint16_t TMRCSR_SP      = 0001;
	static constexpr uint16_t TMRCSR_CAP     = 0002;
	static constexpr uint16_t TMRCSR_MON     = 0004;
	static constexpr uint16_t TMRCSR_OS      = 0010;
	static constexpr uint16_t TMRCSR_RUN     = 0020;
	static constexpr uint16_t TMRCSR_D16     = 0040;
	static constexpr uint16_t TMRCSR_D4      = 0100;
	static constexpr uint16_t TMRCSR_FL      = 0200;
	static constexpr uint16_t TMRCSR_WR      = 0377;

	uint16_t m_csr;
	uint16_t m_counter;
	uint16_t m_limit;

	emu_timer *m_timer;
	emu_timer *m_reload;

	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(timer_reload);
};


// device type definition
DECLARE_DEVICE_TYPE(K1801VM1_TIMER, k1801vm1_timer_device)

#endif // MAME_USSR_VM1TIMER_H
