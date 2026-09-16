// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 GS Interface (GIF) device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_VIDEO_PS2GIF_H
#define MAME_VIDEO_PS2GIF_H

#pragma once

class ps2_gif_device;

#include "ps2gs.h"
#include "cpu/mips/ps2vu.h"

class ps2_gif_device : public device_t, public device_execute_interface
{
public:
	template <typename T, typename U>
	ps2_gif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gs_tag, U &&vu1_tag)
		: ps2_gif_device(mconfig, tag, owner, clock)
	{
		m_gs.set_tag(std::forward<T>(gs_tag));
		m_vu1.set_tag(std::forward<U>(vu1_tag));
	}

	ps2_gif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_gif_device() override;

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

	void kick_path1(uint32_t address);
	void write_path1(uint64_t hi, uint64_t lo);
	void write_path3(uint64_t hi, uint64_t lo);
	void set_path3_mask(bool masked);
	bool path1_available() const { return !m_current_path1_tag.valid(); }
	bool path3_available() const { return m_path3_available; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	void fetch_path1(uint64_t &hi, uint64_t &lo);

	void gif_reset();

	class tag_t
	{
	public:
		tag_t() : m_nloop(0), m_end(false), m_prim_enable(false), m_prim_output(false), m_prim(0), m_format(FMT_DISABLE), m_reg_count(0), m_curr_reg(0)
		{
			memset(m_regs, 0, 16);
			memset(m_words, 0, sizeof(uint32_t) * 4);
		}
		tag_t(uint64_t lo, uint64_t hi);

		enum format_t : uint8_t
		{
			FMT_PACKED = 0,
			FMT_REGLIST,
			FMT_IMAGE,
			FMT_DISABLE
		};

		bool valid() const { return m_nloop != 0; }
		uint16_t nloop() const { return m_nloop; }
		bool end() const { return m_end; }
		bool is_prim() const { return m_prim_enable; }
		bool is_prim_output() const { return m_prim_output; }
		void set_prim_output() { m_prim_output = true; }
		uint16_t prim() const { return m_prim; }
		format_t format() const { return m_format; }
		uint32_t word(offs_t index) const { return m_words[index]; }
		uint32_t reg() const { return m_regs[m_curr_reg]; }

		void loop() { m_nloop--; }
		void next_reg();

	protected:
		uint16_t m_nloop;
		bool m_end;
		bool m_prim_enable;
		bool m_prim_output;
		uint16_t m_prim;
		format_t m_format;
		uint8_t m_reg_count;
		uint8_t m_curr_reg;
		uint8_t m_regs[16];
		uint32_t m_words[4];
	};

	void process_tag(tag_t &tag, uint64_t hi, uint64_t lo);

	enum : uint32_t
	{
		CTRL_RST = (1 << 0),
		CTRL_PSE = (1 << 3)
	};

	required_device<ps2_gs_device> m_gs;
	required_device<sonyvu1_device> m_vu1;

	int m_icount;

	uint32_t m_ctrl;
	uint32_t m_mode;
	uint32_t m_stat;
	tag_t m_current_path1_tag;
	tag_t m_current_path3_tag;
	tag_t m_last_tag;
	uint32_t m_cnt;
	uint32_t m_p3cnt;
	uint32_t m_p3tag;
	uint64_t m_next_path3_hi;
	uint64_t m_next_path3_lo;
	bool m_path3_available;
	bool m_p3mask;
	uint32_t m_vu_mem_address;
};

DECLARE_DEVICE_TYPE(SONYPS2_GIF, ps2_gif_device)

#endif // MAME_VIDEO_PS2GIF_H
