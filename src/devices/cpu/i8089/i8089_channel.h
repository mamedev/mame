// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

    I/O channel

***************************************************************************/

#ifndef MAME_CPU_I8080_I8089_CHANNEL_H
#define MAME_CPU_I8080_I8089_CHANNEL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class i8089_device;

class i8089_channel_device : public device_t
{
public:
	// construction/destruction
	i8089_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto sintr() { return m_write_sintr.bind(); }

	// set register
	void set_reg(int reg, uint32_t value, int tag = -1);

	int execute_run();
	void attention();

	// channel status
	bool executing();
	bool transferring();
	bool priority();
	int  chan_prio();
	bool chained();
	bool lock();
	void ca();

	void ext_w(int state);
	void drq_w(int state);

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
		uint32_t w; // 20-bit address
		bool t; // tag-bit
	}
	m_r[11];

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	// opcodes
	void add_rm(int r, int m, int o);
	void add_mr(int m, int r, int o);
	void addb_rm(int r, int m, int o);
	void addb_mr(int m, int r, int o);
	void addbi_ri(int r, int8_t i);
	void addbi_mi(int m, int8_t i, int o);
	void addi_ri(int r, int16_t i);
	void addi_mi(int m, int16_t i, int o);
	void and_rm(int r, int m, int o);
	void and_mr(int m, int r, int o);
	void andb_rm(int r, int m, int o);
	void andb_mr(int m, int r, int o);
	void andbi_ri(int r, int8_t i);
	void andbi_mi(int m, int8_t i, int o);
	void andi_ri(int r, int16_t i);
	void andi_mi(int m, int16_t i, int o);
	void call(int m, int16_t d, int o);
	void clr(int m, int b, int o);
	void dec_r(int r);
	void dec_m(int m, int o);
	void decb(int m, int o);
	void hlt();
	void inc_r(int r);
	void inc_m(int m, int o);
	void incb(int m, int o);
	void jbt(int m, int b, int16_t d, int o);
	void jmce(int m, int16_t d, int o);
	void jmcne(int m, int16_t d, int o);
	void jnbt(int m, int b, int16_t d, int o);
	void jnz_r(int r, int16_t d);
	void jnz_m(int m, int16_t d, int o);
	void jnzb(int m, int16_t d, int o);
	void jz_r(int r, int16_t d);
	void jz_m(int m, int16_t d, int o);
	void jzb(int m, int16_t d, int o);
	void lpd(int p, int m, int o);
	void lpdi(int p, int s, int o);
	void mov_mr(int m, int r, int o);
	void mov_rm(int r, int m, int o);
	void mov_mm(int m1, int m2, int o1, int o2);
	void movb_mr(int m, int r, int o);
	void movb_rm(int r, int m, int o);
	void movb_mm(int m1, int m2, int o1, int o2);
	void movbi_ri(int r, int8_t i);
	void movbi_mi(int m, int8_t i, int o);
	void movi_ri(int r, int16_t i);
	void movi_mi(int m, int16_t i, int o);
	void movp_mp(int m, int p, int o);
	void movp_pm(int p, int m, int o);
	void nop();
	void not_r(int r);
	void not_m(int m, int o);
	void not_rm(int r, int m, int o);
	void notb_m(int m, int o);
	void notb_rm(int r, int m, int o);
	void or_rm(int r, int m, int o);
	void or_mr(int m, int r, int o);
	void orb_rm(int r, int m, int o);
	void orb_mr(int m, int r, int o);
	void orbi_ri(int r, int8_t i);
	void orbi_mi(int m, int8_t i, int o);
	void ori_ri(int r, int16_t i);
	void ori_mi(int m, int16_t i, int o);
	void setb(int m, int b, int o);
	void do_sintr();
	void tsl(int m, int8_t i, int8_t d, int o);
	void wid(int s, int d);
	void xfer();
	void invalid(int opc);

	// instruction fetch
	int16_t displacement(int wb);
	uint32_t offset(int aa, int mm, int w);
	int8_t imm8();
	int16_t imm16();

	void examine_ccw(uint8_t ccw);

	devcb_write_line m_write_sintr;

	i8089_device *m_iop;

	int m_icount;

	// dma
	void terminate_dma(int offset);

	bool m_xfer_pending;
	uint16_t m_dma_value;
	int m_dma_state;
	bool m_drq, m_store_hi, m_load_hi;

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

	int m_prio;

	// priority
	enum
	{
		PRIO_DMA = 1,
		PRIO_DMA_TERM = 1,
		PRIO_PROG_CHAIN = 1,
		PRIO_CHAN_ATTN,
		PRIO_PROG,
		PRIO_IDLE
	};
};


// device type definition
DECLARE_DEVICE_TYPE(I8089_CHANNEL, i8089_channel_device)

#endif // MAME_CPU_I8080_I8089_CHANNEL_H
