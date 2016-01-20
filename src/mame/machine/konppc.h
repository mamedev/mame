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
	konppc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_num_boards(device_t &device, int num) { downcast<konppc_device &>(device).num_cgboards = num; }
	static void static_set_cbboard_type(device_t &device, int cgtype) { downcast<konppc_device &>(device).cgboard_type = cgtype; }

	void set_cgboard_id(int board_id);
	int get_cgboard_id(void);
	void set_cgboard_texture_bank(int board, const char *bank, UINT8 *rom);


	// read/write
	DECLARE_READ32_MEMBER( cgboard_dsp_comm_r_ppc );
	DECLARE_WRITE32_MEMBER( cgboard_dsp_comm_w_ppc );
	DECLARE_READ32_MEMBER( cgboard_dsp_shared_r_ppc );
	DECLARE_WRITE32_MEMBER( cgboard_dsp_shared_w_ppc );

	DECLARE_READ32_MEMBER( cgboard_0_comm_sharc_r );
	DECLARE_WRITE32_MEMBER( cgboard_0_comm_sharc_w );
	DECLARE_READ32_MEMBER( cgboard_0_shared_sharc_r );
	DECLARE_WRITE32_MEMBER( cgboard_0_shared_sharc_w );
	DECLARE_READ32_MEMBER( cgboard_1_comm_sharc_r );
	DECLARE_WRITE32_MEMBER( cgboard_1_comm_sharc_w );
	DECLARE_READ32_MEMBER( cgboard_1_shared_sharc_r );
	DECLARE_WRITE32_MEMBER( cgboard_1_shared_sharc_w );

	DECLARE_READ32_MEMBER(K033906_0_r);
	DECLARE_WRITE32_MEMBER(K033906_0_w);
	DECLARE_READ32_MEMBER(K033906_1_r);
	DECLARE_WRITE32_MEMBER(K033906_1_w);

	DECLARE_WRITE32_MEMBER(nwk_fifo_0_w);
	DECLARE_WRITE32_MEMBER(nwk_fifo_1_w);
	DECLARE_READ32_MEMBER(nwk_voodoo_0_r);
	DECLARE_READ32_MEMBER(nwk_voodoo_1_r);
	DECLARE_WRITE32_MEMBER(nwk_voodoo_0_w);
	DECLARE_WRITE32_MEMBER(nwk_voodoo_1_w);
protected:
	// device-level overrides
	virtual void device_start() override;

	UINT32 dsp_comm_sharc_r(int board, int offset);
	void dsp_comm_sharc_w(address_space &space, int board, int offset, UINT32 data);
	UINT32 dsp_shared_ram_r_sharc(int board, int offset);
	void dsp_shared_ram_w_sharc(int board, int offset, UINT32 data);

	UINT32 nwk_fifo_r(address_space &space, int board);
	void nwk_fifo_w(int board, UINT32 data);
private:
	// internal state
	UINT32 dsp_comm_ppc[MAX_CG_BOARDS][2];
	UINT32 dsp_comm_sharc[MAX_CG_BOARDS][2];
	UINT8 dsp_shared_ram_bank[MAX_CG_BOARDS];

	INT32 cgboard_id;
	INT32 cgboard_type;
	INT32 num_cgboards;

	std::unique_ptr<UINT32[]> dsp_shared_ram[MAX_CG_BOARDS];

	UINT32 dsp_state[MAX_CG_BOARDS];
	UINT32 nwk_device_sel[MAX_CG_BOARDS];
	const char *texture_bank[MAX_CG_BOARDS];

	int nwk_fifo_half_full_r;
	int nwk_fifo_half_full_w;
	int nwk_fifo_full;
	int nwk_fifo_mask;

	std::unique_ptr<UINT32[]> nwk_fifo[MAX_CG_BOARDS];
	INT32 nwk_fifo_read_ptr[MAX_CG_BOARDS];
	INT32 nwk_fifo_write_ptr[MAX_CG_BOARDS];

	std::unique_ptr<UINT32[]> nwk_ram[MAX_CG_BOARDS];
};


// device type definition
extern const device_type KONPPC;


void draw_7segment_led(bitmap_rgb32 &bitmap, int x, int y, UINT8 value);

#endif
