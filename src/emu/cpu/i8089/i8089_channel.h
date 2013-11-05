/***************************************************************************

    Intel 8089 I/O Processor

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    I/O channel

***************************************************************************/

#pragma once

#ifndef __I8089_CHANNEL_H__
#define __I8089_CHANNEL_H__

#include "emu.h"
#include "i8089.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8089_CHANNEL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, I8089_CHANNEL, 0)

#define MCFG_I8089_CHANNEL_SINTR(_sintr) \
	downcast<i8089_channel *>(device)->set_sintr_callback(DEVCB2_##_sintr);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class i8089_device;

class i8089_channel : public device_t
{
public:
	// construction/destruction
	i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _sintr> void set_sintr_callback(_sintr sintr) { m_write_sintr.set_callback(sintr); }

	// set register
	void set_reg(int reg, int value, int tag = -1);

	int execute_run();
	void attention();

	// channel status
	bool executing();
	bool transferring();
	bool priority();
	bool chained();
	bool lock();

	DECLARE_WRITE_LINE_MEMBER( ext_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

	// register
	enum
	{
		GA,  // 20-bit general purpose address a
		GB,  // 20-bit general purpose address b
		GC,  // 20-bit general purpose address c
		BC,  // byte count
		TP,  // 20-bit task pointer
		IX,  // byte count
		CC,  // mask compare
		MC,  // channel control

		// internal use register
		CP,  // 20-bit control block pointer
		PP,  // 20-bit parameter pointer
		PSW, // program status word

		NUM_REGS
	};

	struct
	{
		int w; // 20-bit address
		int t; // tag-bit
	}
	m_r[11];

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:

	// opcodes
	void add_rm(int r, int m, int o = 0);
	void add_mr(int m, int r, int o = 0);
	void addb_rm(int r, int m, int o = 0);
	void addb_mr(int m, int r, int o = 0);
	void addbi_ri(int r, int i);
	void addbi_mi(int m, int i, int o = 0);
	void addi_ri(int r, int i);
	void addi_mi(int m, int i, int o = 0);
	void and_rm(int r, int m, int o = 0);
	void and_mr(int m, int r, int o = 0);
	void andb_rm(int r, int m, int o = 0);
	void andb_mr(int m, int r, int o = 0);
	void andbi_ri(int r, int i);
	void andbi_mi(int m, int i, int o = 0);
	void andi_ri(int r, int i);
	void andi_mi(int m, int i, int o = 0);
	void call(int m, int d, int o = 0);
	void clr(int m, int b, int o = 0);
	void dec_r(int r);
	void dec_m(int m, int o = 0);
	void decb(int m, int o = 0);
	void hlt();
	void inc_r(int r);
	void inc_m(int m, int o = 0);
	void incb(int m, int o = 0);
	void jbt(int m, int b, int d, int o = 0);
	void jmce(int m, int d, int o = 0);
	void jmcne(int m, int d, int o = 0);
	void jmp(int d);
	void jnbt(int m, int b, int d, int o = 0);
	void jnz_r(int r, int d);
	void jnz_m(int m, int d, int o = 0);
	void jnzb(int m, int d, int o = 0);
	void jz_r(int r, int d);
	void jz_m(int m, int d, int o = 0);
	void jzb(int m, int d, int o = 0);
	void lcall(int m, int d, int o = 0);
	void ljbt(int m, int b, int d, int o = 0);
	void ljmce(int m, int d, int o = 0);
	void ljmcne(int m, int d, int o = 0);
	void ljmp(int d);
	void ljnbt(int m, int b, int d, int o = 0);
	void ljnz_r(int r, int d);
	void ljnz_m(int m, int d, int o = 0);
	void ljnzb(int m, int d, int o = 0);
	void ljz_r(int r, int d);
	void ljz_m(int m, int d, int o = 0);
	void ljzb(int m, int d, int o = 0);
	void lpd(int p, int m, int o = 0);
	void lpdi(int p, int i, int o = 0);
	void mov_mr(int m, int r, int o = 0);
	void mov_rm(int r, int m, int o = 0);
	void mov_mm(int m1, int m2, int o1 = 0, int o2 = 0);
	void movb_mr(int m, int r, int o = 0);
	void movb_rm(int r, int m, int o = 0);
	void movb_mm(int m1, int m2, int o1 = 0, int o2 = 0);
	void movbi_ri(int r, int i);
	void movbi_mi(int m, int i, int o = 0);
	void movi_ri(int r, int i);
	void movi_mi(int m, int i, int o = 0);
	void movp_mp(int m, int p, int o = 0);
	void movp_pm(int p, int m, int o = 0);
	void nop();
	void not_r(int r);
	void not_m(int m, int o = 0);
	void not_rm(int r, int m, int o = 0);
	void notb_m(int m, int o = 0);
	void notb_rm(int r, int m, int o = 0);
	void or_rm(int r, int m, int o = 0);
	void or_mr(int m, int r, int o = 0);
	void orb_rm(int r, int m, int o = 0);
	void orb_mr(int m, int r, int o = 0);
	void orbi_ri(int r, int i);
	void orbi_mi(int m, int i, int o = 0);
	void ori_ri(int r, int i);
	void ori_mi(int m, int i, int o = 0);
	void setb(int m, int b, int o = 0);
	void sintr();
	void tsl(int m, int i, int d, int o = 0);
	void wid(int s, int d);
	void xfer();
	void invalid(int opc);

	// instruction fetch
	UINT16 displacement(int wb);
	UINT8 offset(int aa);
	UINT8 imm8();
	UINT16 imm16();

	void examine_ccw(UINT8 ccw);

	devcb2_write_line m_write_sintr;

	i8089_device *m_iop;

	int m_icount;

	// dma
	void terminate_dma(int offset);

	bool m_xfer_pending;
	UINT16 m_dma_value;
	int m_dma_state;

	// dma state
	enum
	{
		DMA_IDLE,
		DMA_WAIT_FOR_SOURCE_DRQ,
		DMA_FETCH,
		DMA_TRANSLATE,
		DMA_WAIT_FOR_DEST_DRQ,
		DMA_STORE,
		DMA_STORE_BYTE_HIGH,
		DMA_COMPARE,
		DMA_TERMINATE
	};
};


// device type definition
extern const device_type I8089_CHANNEL;


#endif  /* __I8089_CHANNEL_H__ */
