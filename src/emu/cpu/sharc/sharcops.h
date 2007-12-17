typedef struct
{
	UINT32 op_mask;
	UINT32 op_bits;
	void (* handler)(void);
} SHARC_OP;

static SHARC_OP sharc_opcode_table[] =
{
	//  |0 0 1|
	{	0xe000,		0x2000,		sharcop_compute_dreg_dm_dreg_pm						},

	//  |0 0 0|0 0 0 0 1|
	{	0xff00,		0x0100,		sharcop_compute										},

	//  |0 1 0|0|
	{	0xf000,		0x4000,		sharcop_compute_ureg_dmpm_premod					},

	//  |0 1 0|1|
	{	0xf000,		0x5000,		sharcop_compute_ureg_dmpm_postmod					},

	//  |0 1 1|0|x x x|0|0|
	{	0xf180,		0x6000,		sharcop_compute_dm_to_dreg_immmod					},

	//  |0 1 1|0|x x x|0|1|
	{	0xf180,		0x6080,		sharcop_compute_dreg_to_dm_immmod					},

	//  |0 1 1|0|x x x|1|0|
	{	0xf180,		0x6100,		sharcop_compute_pm_to_dreg_immmod					},

	//  |0 1 1|0|x x x|1|1|
	{	0xf180,		0x6180,		sharcop_compute_dreg_to_pm_immmod					},

	//  |0 1 1|1|
	{	0xf000,		0x7000,		sharcop_compute_ureg_to_ureg						},

	//  |1 0 0|0|
	{	0xf000,		0x8000,		sharcop_imm_shift_dreg_dmpm							},

	//  |0 0 0|0 0 0 1 0|
	{	0xff00,		0x0200,		sharcop_imm_shift									},

	//  |0 0 0|0 0 1 0 0|
	{	0xff00,		0x0400,		sharcop_compute_modify								},

	//  |0 0 0|0 0 1 1 0|0|
	{	0xff80,		0x0600,		sharcop_direct_jump									},

	//  |0 0 0|0 0 1 1 0|1|
	{	0xff80,		0x0680,		sharcop_direct_call									},

	//  |0 0 0|0 0 1 1 1|0|
	{	0xff80,		0x0700,		sharcop_relative_jump								},

	//  |0 0 0|0 0 1 1 1|1|
	{	0xff80,		0x0780,		sharcop_relative_call								},

	//  |0 0 0|0 1 0 0 0|0|
	{	0xff80,		0x0800,		sharcop_indirect_jump								},

	//  |0 0 0|0 1 0 0 0|1|
	{	0xff80,		0x0880,		sharcop_indirect_call								},

	//  |0 0 0|0 1 0 0 1|0|
	{	0xff80,		0x0900,		sharcop_relative_jump_compute						},

	//  |0 0 0|0 1 0 0 1|1|
	{	0xff80,		0x0980,		sharcop_relative_call_compute						},

	//  |1 1 0|
	{	0xe000,		0xc000,		sharcop_indirect_jump_compute_dreg_dm				},

	//  |1 1 1|
	{	0xe000,		0xe000,		sharcop_relative_jump_compute_dreg_dm				},

	//  |0 0 0|0 1 0 1 0|
	{	0xff00,		0x0a00,		sharcop_rts											},

	//  |0 0 0|0 1 0 1 1|
	{	0xff00,		0x0b00,		sharcop_rti											},

	//  |0 0 0|0 1 1 0 0|
	{	0xff00,		0x0c00,		sharcop_do_until_counter_imm						},

	//  |0 0 0|0 1 1 0 1|
	{	0xff00,		0x0d00,		sharcop_do_until_counter_ureg						},

	//  |0 0 0|0 1 1 1 0|
	{	0xff00,		0x0e00,		sharcop_do_until									},

	//  |0 0 0|1 0 0|0|0|
	{	0xff00,		0x1000,		sharcop_dm_to_ureg_direct							},

	//  |0 0 0|1 0 0|0|1|
	{	0xff00,		0x1100,		sharcop_ureg_to_dm_direct							},

	//  |0 0 0|1 0 0|1|0|
	{	0xff00,		0x1200,		sharcop_pm_to_ureg_direct							},

	//  |0 0 0|1 0 0|1|1|
	{	0xff00,		0x1300,		sharcop_ureg_to_pm_direct							},

	//  |1 0 1|0|x x x|0|
	{	0xf100,		0xa000,		sharcop_dm_to_ureg_indirect							},

	//  |1 0 1|0|x x x|1|
	{	0xf100,		0xa100,		sharcop_ureg_to_dm_indirect							},

	//  |1 0 1|1|x x x|0|
	{	0xf100,		0xb000,		sharcop_pm_to_ureg_indirect							},

	//  |1 0 1|1|x x x|1|
	{	0xf100,		0xb100,		sharcop_ureg_to_pm_indirect							},

	//  |1 0 0|1|
	{	0xf000,		0x9000,		sharcop_imm_to_dmpm									},

	//  |0 0 0|0 1 1 1 1|
	{	0xff00,		0x0f00,		sharcop_imm_to_ureg									},

	//  |0 0 0|1 0 1 0 0|
	{	0xff00,		0x1400,		sharcop_sysreg_bitop								},

	//  |0 0 0|1 0 1 1 0|0|
	{	0xff80,		0x1600,		sharcop_modify										},

	//  |0 0 0|1 0 1 1 0|1|
	{	0xff80,		0x1680,		sharcop_bit_reverse									},

	//  |0 0 0|1 0 1 1 1|
	{	0xff00,		0x1700,		sharcop_push_pop_stacks								},

	//  |0 0 0|0 0 0 0 0|0|
	{	0xff80,		0x0000,		sharcop_nop											},

	//  |0 0 0|0 0 0 0 0|1|
	{	0xff80,		0x0080,		sharcop_idle										},
};
