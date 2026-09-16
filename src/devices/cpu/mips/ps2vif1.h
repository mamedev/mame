// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 VU1 interface device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_CPU_MIPS_PS2VIF1_H
#define MAME_CPU_MIPS_PS2VIF1_H

#pragma once

class ps2_vif1_device;

#include "video/ps2gs.h"
#include "ps2vu.h"

class ps2_vif1_device : public device_t, public device_execute_interface
{
public:
	template <typename T, typename U>
	ps2_vif1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gs_tag, U &&vu1_tag)
		: ps2_vif1_device(mconfig, tag, owner, clock)
	{
		m_gs.set_tag(std::forward<T>(gs_tag));
		m_vu1.set_tag(std::forward<U>(vu1_tag));
	}

	ps2_vif1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_vif1_device() override;

	uint64_t mmio_r(offs_t offset);
	void mmio_w(offs_t offset, uint64_t data);

	uint32_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint32_t data);

	void dma_write(const uint64_t hi, const uint64_t lo);
	void tag_write(uint32_t *data);
	bool fifo_available(uint32_t count) const { return (BUFFER_SIZE - m_end) >= count; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	uint32_t calculate_unpack_count();

	void decode_vifcode();

	void transfer_vifcode_data();
	void transfer_mpg();
	void transfer_unpack();

	void fifo_push(uint32_t value);
	uint32_t fifo_pop();
	uint32_t fifo_depth() const { return m_end - m_curr; }

	enum : uint32_t
	{
		STAT_MODE_MASK   = (3 << 0),
		STAT_MODE_IDLE   = 0,
		STAT_MODE_WAIT   = 1,
		STAT_MODE_DECODE = 2,
		STAT_MODE_DATA   = 3,

		STAT_E_WAIT      = (1 << 2),
		STAT_GS_WAIT     = (1 << 3),
		STAT_MARK        = (1 << 6),
		STAT_DBUF        = (1 << 7),
		STAT_STALL_STOP  = (1 << 8),
		STAT_STALL_FBRK  = (1 << 9),
		STAT_STALL_INT   = (1 << 10),
		STAT_INT         = (1 << 11),
		STAT_BAD_TAG     = (1 << 12),
		STAT_BAD_CODE    = (1 << 13),
		STAT_FDR_TO_HOST = (1 << 23)
	};

	enum : uint8_t
	{
		CMD_INT         = 0x80,
		CMD_UNPACK_MASK = 0x10
	};

	enum : uint8_t
	{
		FMT_S32     = 0x00,
		FMT_S16     = 0x01,
		FMT_S8      = 0x02,
		//FMT_UNK0  = 0x03,
		FMT_V2_32   = 0x04,
		FMT_V2_16   = 0x05,
		FMT_V2_8    = 0x06,
		//FMT_UNK1  = 0x07,
		FMT_V3_32   = 0x08,
		FMT_V3_16   = 0x09,
		FMT_V3_8    = 0x0a,
		//FMT_UNK2  = 0x0b,
		FMT_V4_32   = 0x0c,
		FMT_V4_16   = 0x0d,
		FMT_V4_8    = 0x0e,
		FMT_V4_5    = 0x0f,
	};

	required_device<ps2_gs_device> m_gs;
	required_device<sonyvu1_device> m_vu1;

	int m_icount;

	uint32_t m_buffer[0x40];
	uint32_t m_curr;
	uint32_t m_end;

	uint32_t m_status;
	uint32_t m_control;
	uint32_t m_err;
	uint32_t m_mark;
	uint32_t m_cycle;
	uint32_t m_mode;
	uint32_t m_num;
	uint32_t m_mask;
	uint32_t m_code;
	uint32_t m_itops;
	uint32_t m_base;
	uint32_t m_offset;
	uint32_t m_tops;
	uint32_t m_itop;
	uint32_t m_top;

	uint32_t m_row_fill[4];
	uint32_t m_col_fill[4];

	uint32_t m_data_needed;
	uint32_t m_data_index;
	uint8_t m_command;
	uint8_t m_alignment;

	uint32_t m_mpg_count;
	uint32_t m_mpg_addr;
	uint64_t m_mpg_insn;

	uint32_t m_unpack_count;
	uint32_t m_unpack_addr;
	uint32_t m_unpack_last;
	uint32_t m_unpack_bits_remaining;
	bool m_unpack_signed;
	bool m_unpack_add_tops;
	uint8_t m_unpack_format;

	static const size_t BUFFER_SIZE;
	static const uint32_t FORMAT_SIZE[0x10];
};

DECLARE_DEVICE_TYPE(SONYPS2_VIF1, ps2_vif1_device)

#endif // MAME_CPU_MIPS_PS2VIF1_H
