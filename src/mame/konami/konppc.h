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

	void set_num_boards(int num) { num_cgboards = num; }
	void set_cbboard_type(int cgtype) { cgboard_type = cgtype; }

	void set_cgboard_id(int board_id);
	int get_cgboard_id(void);
	void set_cgboard_texture_bank(int board, const char *bank, uint8_t *rom);

	bool output_3d_enabled();

	// read/write
	uint32_t cgboard_dsp_comm_r_ppc(offs_t offset, uint32_t mem_mask = ~0);
	void cgboard_dsp_comm_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cgboard_dsp_shared_r_ppc(offs_t offset);
	void cgboard_dsp_shared_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t cgboard_0_comm_sharc_r(offs_t offset);
	void cgboard_0_comm_sharc_w(offs_t offset, uint32_t data);
	uint32_t cgboard_0_shared_sharc_r(offs_t offset);
	void cgboard_0_shared_sharc_w(offs_t offset, uint32_t data);
	uint32_t cgboard_1_comm_sharc_r(offs_t offset);
	void cgboard_1_comm_sharc_w(offs_t offset, uint32_t data);
	uint32_t cgboard_1_shared_sharc_r(offs_t offset);
	void cgboard_1_shared_sharc_w(offs_t offset, uint32_t data);

	uint32_t K033906_0_r(offs_t offset);
	void K033906_0_w(offs_t offset, uint32_t data);
	uint32_t K033906_1_r(offs_t offset);
	void K033906_1_w(offs_t offset, uint32_t data);

	void nwk_fifo_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void nwk_fifo_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t nwk_voodoo_0_r(offs_t offset);
	uint32_t nwk_voodoo_1_r(offs_t offset);
	void nwk_voodoo_0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void nwk_voodoo_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	uint32_t dsp_comm_sharc_r(int board, int offset);
	void dsp_comm_sharc_w(int board, int offset, uint32_t data);
	uint32_t dsp_shared_ram_r_sharc(int board, int offset);
	void dsp_shared_ram_w_sharc(int board, int offset, uint32_t data);

	uint32_t nwk_fifo_r(int board);
	void nwk_fifo_w(int board, uint32_t data);
private:
	// device finders
	optional_device_array<adsp21062_device, 2> m_dsp;
	optional_device_array<k033906_device, 2> m_k033906;
	optional_device_array<generic_voodoo_device, 2> m_voodoo;

	// internal state
	uint32_t dsp_comm_ppc[MAX_CG_BOARDS][2];
	uint32_t dsp_comm_sharc[MAX_CG_BOARDS][2];
	uint8_t dsp_shared_ram_bank[MAX_CG_BOARDS];

	int32_t cgboard_id = 0;
	int32_t cgboard_type;
	int32_t num_cgboards;

	std::unique_ptr<uint32_t[]> dsp_shared_ram[MAX_CG_BOARDS];

	uint32_t dsp_state[MAX_CG_BOARDS]{};
	uint32_t nwk_device_sel[MAX_CG_BOARDS]{};
	const char *texture_bank[MAX_CG_BOARDS]{};

	int nwk_fifo_half_full_r = 0;
	int nwk_fifo_half_full_w = 0;
	int nwk_fifo_full = 0;
	int nwk_fifo_mask = 0;

	bool enable_3d[MAX_CG_BOARDS]{};

	std::unique_ptr<uint32_t[]> nwk_fifo[MAX_CG_BOARDS];
	int32_t nwk_fifo_read_ptr[MAX_CG_BOARDS];
	int32_t nwk_fifo_write_ptr[MAX_CG_BOARDS];

	std::unique_ptr<uint32_t[]> nwk_ram[MAX_CG_BOARDS];
};


// device type definition
DECLARE_DEVICE_TYPE(KONPPC, konppc_device)


#endif // MAME_KONAMI_KONPPC_H
