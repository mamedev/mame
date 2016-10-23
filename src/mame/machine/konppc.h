// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef _KONPPC_H
#define _KONPPC_H

#define CGBOARD_TYPE_ZR107      0
#define CGBOARD_TYPE_GTICLUB    1
#define CGBOARD_TYPE_NWKTR      2
#define CGBOARD_TYPE_HORNET     3
#define CGBOARD_TYPE_HANGPLT    4

#define MAX_CG_BOARDS   2

#define MCFG_KONPPC_CGBOARD_NUMBER(_num) \
	konppc_device::static_set_num_boards(*device, _num);

#define MCFG_KONPPC_CGBOARD_TYPE(_cgtype) \
	konppc_device::static_set_cbboard_type(*device, _cgtype);

class konppc_device :  public device_t
{
public:
	// construction/destruction
	konppc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void static_set_num_boards(device_t &device, int num) { downcast<konppc_device &>(device).num_cgboards = num; }
	static void static_set_cbboard_type(device_t &device, int cgtype) { downcast<konppc_device &>(device).cgboard_type = cgtype; }

	void set_cgboard_id(int board_id);
	int get_cgboard_id(void);
	void set_cgboard_texture_bank(int board, const char *bank, uint8_t *rom);


	// read/write
	uint32_t cgboard_dsp_comm_r_ppc(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_dsp_comm_w_ppc(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cgboard_dsp_shared_r_ppc(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_dsp_shared_w_ppc(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t cgboard_0_comm_sharc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_0_comm_sharc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cgboard_0_shared_sharc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_0_shared_sharc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cgboard_1_comm_sharc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_1_comm_sharc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cgboard_1_shared_sharc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cgboard_1_shared_sharc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t K033906_0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void K033906_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t K033906_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void K033906_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void nwk_fifo_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void nwk_fifo_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t nwk_voodoo_0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t nwk_voodoo_1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void nwk_voodoo_0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void nwk_voodoo_1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
protected:
	// device-level overrides
	virtual void device_start() override;

	uint32_t dsp_comm_sharc_r(int board, int offset);
	void dsp_comm_sharc_w(address_space &space, int board, int offset, uint32_t data);
	uint32_t dsp_shared_ram_r_sharc(int board, int offset);
	void dsp_shared_ram_w_sharc(int board, int offset, uint32_t data);

	uint32_t nwk_fifo_r(address_space &space, int board);
	void nwk_fifo_w(int board, uint32_t data);
private:
	// internal state
	uint32_t dsp_comm_ppc[MAX_CG_BOARDS][2];
	uint32_t dsp_comm_sharc[MAX_CG_BOARDS][2];
	uint8_t dsp_shared_ram_bank[MAX_CG_BOARDS];

	int32_t cgboard_id;
	int32_t cgboard_type;
	int32_t num_cgboards;

	std::unique_ptr<uint32_t[]> dsp_shared_ram[MAX_CG_BOARDS];

	uint32_t dsp_state[MAX_CG_BOARDS];
	uint32_t nwk_device_sel[MAX_CG_BOARDS];
	const char *texture_bank[MAX_CG_BOARDS];

	int nwk_fifo_half_full_r;
	int nwk_fifo_half_full_w;
	int nwk_fifo_full;
	int nwk_fifo_mask;

	std::unique_ptr<uint32_t[]> nwk_fifo[MAX_CG_BOARDS];
	int32_t nwk_fifo_read_ptr[MAX_CG_BOARDS];
	int32_t nwk_fifo_write_ptr[MAX_CG_BOARDS];

	std::unique_ptr<uint32_t[]> nwk_ram[MAX_CG_BOARDS];
};


// device type definition
extern const device_type KONPPC;


void draw_7segment_led(bitmap_rgb32 &bitmap, int x, int y, uint8_t value);

#endif
