/***************************************************************************

 h8priv.h : Private constants and other definitions for the H8/3002 emulator.

****************************************************************************/

#pragma once

#ifndef __H8PRIV_H__
#define __H8PRIV_H__

#define H8_MAX_PORTS (16)  // number of I/O ports defined architecturally (1-9 and A-G = 16)

typedef struct
{
	UINT32 tgr, irq, out;
} H8S2XXX_TPU_ITEM;

typedef struct
{
	emu_timer *timer;
	int cycles_per_tick;
	UINT64 timer_cycles;
} H8S2XXX_TPU;

typedef struct
{
	emu_timer *timer;
	UINT32 bitrate;
} H8S2XXX_SCI;

typedef struct
{
	emu_timer *timer;
	int cycles_per_tick;
	UINT64 timer_cycles;
} H8S2XXX_TMR;

struct h83xx_state
{
	// main CPU stuff
	UINT32 h8err;
	UINT32 regs[8];
	UINT32 pc, ppc;

	UINT32 irq_req[3];
	INT32 cyccnt;

	UINT8  ccr, exr;
	UINT8  h8nflag, h8vflag, h8cflag, h8zflag, h8iflag, h8hflag;
	UINT8  h8uflag, h8uiflag;
	UINT8  incheckirqs;

    bool   has_exr;

	device_irq_acknowledge_callback irq_cb;
	legacy_cpu_device *device;

	address_space *program;
	direct_read_data *direct;
	address_space *io;

	// onboard peripherals stuff
	UINT8 per_regs[0x1C0];

	UINT16 h8TCNT[5];
	UINT8 h8TSTR;

	UINT8 STCR, TCR[2], TCSR[2], TCORA[2], TCORB[2], TCNT[2];
	UINT16 FRC;

	emu_timer *timer[5];
	emu_timer *frctimer;

	H8S2XXX_TMR tmr[2];
	H8S2XXX_TPU tpu[6];
	H8S2XXX_SCI sci[3];

    UINT8 ddrs[H8_MAX_PORTS], drs[H8_MAX_PORTS], pcrs[H8_MAX_PORTS], odrs[H8_MAX_PORTS];

	int mode_8bit;
    bool has_h8speriphs;
};

// defined in h8speriph.c
extern void h8s_tmr_init(h83xx_state *h8);
extern void h8s_tpu_init(h83xx_state *h8);
extern void h8s_sci_init(h83xx_state *h8);
extern void h8s_periph_reset(h83xx_state *h8);
extern void h8s_dtce_check(h83xx_state *h8, int vecnum);

INLINE h83xx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == H83002 ||
		   device->type() == H83007 ||
		   device->type() == H83044 ||
		   device->type() == H83334 ||
		   device->type() == H8S2241 ||
		   device->type() == H8S2246 ||
		   device->type() == H8S2323 ||
		   device->type() == H8S2394);
	return (h83xx_state *)downcast<legacy_cpu_device *>(device)->token();
}

UINT8 h8_register_read8(h83xx_state *h8, UINT32 address);
UINT8 h8_3007_register_read8(h83xx_state *h8, UINT32 address);
UINT8 h8_3007_register1_read8(h83xx_state *h8, UINT32 address);
void h8_register_write8(h83xx_state *h8, UINT32 address, UINT8 val);
void h8_3007_register_write8(h83xx_state *h8, UINT32 address, UINT8 val);
void h8_3007_register1_write8(h83xx_state *h8, UINT32 address, UINT8 val);

void h8_itu_init(h83xx_state *h8);
void h8_3007_itu_init(h83xx_state *h8);
void h8_itu_reset(h83xx_state *h8);
UINT8 h8_itu_read8(h83xx_state *h8, UINT8 reg);
UINT8 h8_3007_itu_read8(h83xx_state *h8, UINT8 reg);
void h8_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val);
void h8_3007_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val);

UINT8 h8s2241_per_regs_read_8(h83xx_state *h8, int offset);
UINT8 h8s2246_per_regs_read_8(h83xx_state *h8, int offset);
UINT8 h8s2323_per_regs_read_8(h83xx_state *h8, int offset);
UINT8 h8s2394_per_regs_read_8(h83xx_state *h8, int offset);

UINT16 h8s2241_per_regs_read_16(h83xx_state *h8, int offset);
UINT16 h8s2246_per_regs_read_16(h83xx_state *h8, int offset);
UINT16 h8s2323_per_regs_read_16(h83xx_state *h8, int offset);
UINT16 h8s2394_per_regs_read_16(h83xx_state *h8, int offset);

void h8s2241_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data);
void h8s2246_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data);
void h8s2323_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data);
void h8s2394_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data);

void h8s2241_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data);
void h8s2246_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data);
void h8s2323_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data);
void h8s2394_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data);

#endif /* __H8PRIV_H__ */
