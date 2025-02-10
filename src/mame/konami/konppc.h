// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_KONAMI_KONPPC_H
#define MAME_KONAMI_KONPPC_H

#pragma once

#include "cpu/sharc/sharc.h"
#include "machine/k033906.h"
#include "video/voodoo.h"

class konppc_device :  public device_t
{
public:
	static constexpr int CGBOARD_TYPE_ZR107   = 0;
	static constexpr int CGBOARD_TYPE_GTICLUB = 1;
	static constexpr int CGBOARD_TYPE_NWKTR   = 2;
	static constexpr int CGBOARD_TYPE_HORNET  = 3;
	static constexpr int CGBOARD_TYPE_HANGPLT = 4;

	static constexpr int MAX_CG_BOARDS        = 2;

	// construction/destruction
	konppc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configurations
	template <typename T> void set_dsp_tag(int which, T &&tag) { m_dsp[which].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_k033906_tag(int which, T &&tag) { m_k033906[which].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_voodoo_tag(int which, T &&tag) { m_voodoo[which].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_texture_bank_tag(int which, T &&tag) { m_texture_bank[which].set_tag(std::forward<T>(tag)); }

	void set_num_boards(int num) { m_num_cgboards = num; }
	void set_cgboard_type(int cgtype) { m_cgboard_type = cgtype; }

	void set_cgboard_id(int board_id);
	int get_cgboard_id(void);

	bool output_3d_enabled();

	// read/write
	uint32_t cgboard_dsp_comm_r_ppc(offs_t offset, uint32_t mem_mask = ~0);
	void cgboard_dsp_comm_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cgboard_dsp_shared_r_ppc(offs_t offset);
	void cgboard_dsp_shared_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	template <unsigned Board> uint32_t cgboard_comm_sharc_r(offs_t offset) { return dsp_comm_sharc_r(Board, offset); }
	template <unsigned Board> void cgboard_comm_sharc_w(offs_t offset, uint32_t data) { dsp_comm_sharc_w(Board, offset, data); }
	template <unsigned Board> uint32_t cgboard_shared_sharc_r(offs_t offset) { return dsp_shared_ram_r_sharc(Board, offset); }
	template <unsigned Board> void cgboard_shared_sharc_w(offs_t offset, uint32_t data) { dsp_shared_ram_w_sharc(Board, offset, data); }

	template <unsigned Board> uint32_t cgboard_k033906_r(offs_t offset) { return k033906_r(Board, offset); }
	template <unsigned Board> void cgboard_k033906_w(offs_t offset, uint32_t data) { k033906_w(Board, offset, data); }

	template <unsigned Board> void nwk_voodoo_fifo_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { voodoo_fifo_w(Board, offset, data, mem_mask); }
	template <unsigned Board> uint32_t nwk_voodoo_r(offs_t offset) { return voodoo_r(Board, offset); }
	template <unsigned Board> void nwk_voodoo_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { voodoo_w(Board, offset, data, mem_mask); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// device finders
	optional_device_array<adsp21062_device, 2> m_dsp;
	optional_device_array<k033906_device, 2> m_k033906;
	optional_device_array<generic_voodoo_device, 2> m_voodoo;
	optional_memory_bank_array<2> m_texture_bank;

	// internal state
	uint32_t m_dsp_comm_ppc[MAX_CG_BOARDS][2];
	uint32_t m_dsp_comm_sharc[MAX_CG_BOARDS][2];
	uint8_t m_dsp_shared_ram_bank[MAX_CG_BOARDS];

	int32_t m_cgboard_id = 0;
	int32_t m_cgboard_type;
	int32_t m_num_cgboards;

	std::unique_ptr<uint32_t[]> m_dsp_shared_ram[MAX_CG_BOARDS];

	uint32_t m_dsp_state[MAX_CG_BOARDS]{};
	uint32_t m_nwk_device_sel[MAX_CG_BOARDS]{};

	int m_nwk_fifo_half_full_r = 0;
	int m_nwk_fifo_half_full_w = 0;
	int m_nwk_fifo_full = 0;
	int m_nwk_fifo_mask = 0;

	bool m_enable_3d[MAX_CG_BOARDS]{};

	std::unique_ptr<uint32_t[]> m_nwk_fifo[MAX_CG_BOARDS];
	int32_t m_nwk_fifo_read_ptr[MAX_CG_BOARDS];
	int32_t m_nwk_fifo_write_ptr[MAX_CG_BOARDS];

	std::unique_ptr<uint32_t[]> m_nwk_ram[MAX_CG_BOARDS];

	uint32_t dsp_comm_sharc_r(int board, int offset);
	void dsp_comm_sharc_w(int board, int offset, uint32_t data);
	uint32_t dsp_shared_ram_r_sharc(int board, int offset);
	void dsp_shared_ram_w_sharc(int board, int offset, uint32_t data);

	uint32_t nwk_fifo_r(int board);
	void nwk_fifo_w(int board, uint32_t data);

	uint32_t k033906_r(int board, offs_t offset);
	void k033906_w(int board, offs_t offset, uint32_t data);

	uint32_t voodoo_r(int board, offs_t offset);
	void voodoo_w(int board, offs_t offset, uint32_t data, uint32_t mem_mask);
	void voodoo_fifo_w(int board, offs_t offset, uint32_t data, uint32_t mem_mask);
};


// device type definition
DECLARE_DEVICE_TYPE(KONPPC, konppc_device)


#endif // MAME_KONAMI_KONPPC_H
