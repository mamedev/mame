// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Root Counter emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#ifndef MAME_CPU_PSX_RCNT_H
#define MAME_CPU_PSX_RCNT_H

#pragma once


DECLARE_DEVICE_TYPE(PSX_RCNT, psxrcnt_device)

#define PSX_RC_STOP ( 0x01 )
#define PSX_RC_RESET ( 0x04 ) /* guess */
#define PSX_RC_COUNTTARGET ( 0x08 )
#define PSX_RC_IRQTARGET ( 0x10 )
#define PSX_RC_IRQOVERFLOW ( 0x20 )
#define PSX_RC_REPEAT ( 0x40 )
#define PSX_RC_CLC ( 0x100 )
#define PSX_RC_DIV ( 0x200 )
#define PSX_RC_REACHEDTARGET ( 0x800 )
#define PSX_RC_REACHEDFFFF ( 0x1000 )

class psxrcnt_device : public device_t
{
public:
	psxrcnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq0() { return m_irq0_handler.bind(); }
	auto irq1() { return m_irq1_handler.bind(); }
	auto irq2() { return m_irq2_handler.bind(); }

	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_CALLBACK_MEMBER( timer_update );

private:
	struct psx_root
	{
		emu_timer *timer;
		uint16_t n_count;
		uint16_t n_mode;
		uint16_t n_target;
		uint64_t n_start;
	};

	psx_root root_counter[ 3 ];

	uint64_t gettotalcycles( void );
	int root_divider( int n_counter );
	uint16_t root_current( int n_counter );
	int root_target( int n_counter );
	void root_timer_adjust( int n_counter );

	devcb_write_line m_irq0_handler;
	devcb_write_line m_irq1_handler;
	devcb_write_line m_irq2_handler;
};

#endif // MAME_CPU_PSX_RCNT_H
