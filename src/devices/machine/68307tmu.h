// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "cpu/m68000/m68000.h"

class m68307cpu_device;


#define m68307TIMER_TMR (0x0)
#define m68307TIMER_TRR (0x1)
#define m68307TIMER_TCR (0x2)
#define m68307TIMER_TCN (0x3)
#define m68307TIMER_TER (0x4)
#define m68307TIMER_WRR (0x5)
#define m68307TIMER_WCR (0x6)
#define m68307TIMER_XXX (0x7)

struct m68307_single_timer
{
	uint16_t regs[0x8];
	bool enabled;
	emu_timer *mametimer;
};


class m68307_timer
{
	public:
	m68307_single_timer singletimer[2];

	emu_timer *wd_mametimer;
	m68307cpu_device *parent;

	void write_tmr(uint16_t data, uint16_t mem_mask, int which);
	void write_trr(uint16_t data, uint16_t mem_mask, int which);
	void write_ter(uint16_t data, uint16_t mem_mask, int which);
	uint16_t read_tcn(uint16_t mem_mask, int which);

	void init(m68307cpu_device *device);
	void reset(void);

	void timer0_callback(void *ptr, int32_t param);
	void timer1_callback(void *ptr, int32_t param);
	void wd_timer_callback(void *ptr, int32_t param);
};
