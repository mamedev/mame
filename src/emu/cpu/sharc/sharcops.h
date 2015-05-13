// license:BSD-3-Clause
// copyright-holders:Ville Linde

const adsp21062_device::SHARC_OP adsp21062_device::s_sharc_opcode_table[] =
{
	//  |0 0 1|
	{   0xe000,     0x2000,     &adsp21062_device::sharcop_compute_dreg_dm_dreg_pm                     },

	//  |0 0 0|0 0 0 0 1|
	{   0xff00,     0x0100,     &adsp21062_device::sharcop_compute                                     },

	//  |0 1 0|0|
	{   0xf000,     0x4000,     &adsp21062_device::sharcop_compute_ureg_dmpm_premod                    },

	//  |0 1 0|1|
	{   0xf000,     0x5000,     &adsp21062_device::sharcop_compute_ureg_dmpm_postmod                   },

	//  |0 1 1|0|x x x|0|0|
	{   0xf180,     0x6000,     &adsp21062_device::sharcop_compute_dm_to_dreg_immmod                   },

	//  |0 1 1|0|x x x|0|1|
	{   0xf180,     0x6080,     &adsp21062_device::sharcop_compute_dreg_to_dm_immmod                   },

	//  |0 1 1|0|x x x|1|0|
	{   0xf180,     0x6100,     &adsp21062_device::sharcop_compute_pm_to_dreg_immmod                   },

	//  |0 1 1|0|x x x|1|1|
	{   0xf180,     0x6180,     &adsp21062_device::sharcop_compute_dreg_to_pm_immmod                   },

	//  |0 1 1|1|
	{   0xf000,     0x7000,     &adsp21062_device::sharcop_compute_ureg_to_ureg                        },

	//  |1 0 0|0|
	{   0xf000,     0x8000,     &adsp21062_device::sharcop_imm_shift_dreg_dmpm                         },

	//  |0 0 0|0 0 0 1 0|
	{   0xff00,     0x0200,     &adsp21062_device::sharcop_imm_shift                                   },

	//  |0 0 0|0 0 1 0 0|
	{   0xff00,     0x0400,     &adsp21062_device::sharcop_compute_modify                              },

	//  |0 0 0|0 0 1 1 0|0|
	{   0xff80,     0x0600,     &adsp21062_device::sharcop_direct_jump                                 },

	//  |0 0 0|0 0 1 1 0|1|
	{   0xff80,     0x0680,     &adsp21062_device::sharcop_direct_call                                 },

	//  |0 0 0|0 0 1 1 1|0|
	{   0xff80,     0x0700,     &adsp21062_device::sharcop_relative_jump                               },

	//  |0 0 0|0 0 1 1 1|1|
	{   0xff80,     0x0780,     &adsp21062_device::sharcop_relative_call                               },

	//  |0 0 0|0 1 0 0 0|0|
	{   0xff80,     0x0800,     &adsp21062_device::sharcop_indirect_jump                               },

	//  |0 0 0|0 1 0 0 0|1|
	{   0xff80,     0x0880,     &adsp21062_device::sharcop_indirect_call                               },

	//  |0 0 0|0 1 0 0 1|0|
	{   0xff80,     0x0900,     &adsp21062_device::sharcop_relative_jump_compute                       },

	//  |0 0 0|0 1 0 0 1|1|
	{   0xff80,     0x0980,     &adsp21062_device::sharcop_relative_call_compute                       },

	//  |1 1 0|
	{   0xe000,     0xc000,     &adsp21062_device::sharcop_indirect_jump_compute_dreg_dm               },

	//  |1 1 1|
	{   0xe000,     0xe000,     &adsp21062_device::sharcop_relative_jump_compute_dreg_dm               },

	//  |0 0 0|0 1 0 1 0|
	{   0xff00,     0x0a00,     &adsp21062_device::sharcop_rts                                         },

	//  |0 0 0|0 1 0 1 1|
	{   0xff00,     0x0b00,     &adsp21062_device::sharcop_rti                                         },

	//  |0 0 0|0 1 1 0 0|
	{   0xff00,     0x0c00,     &adsp21062_device::sharcop_do_until_counter_imm                        },

	//  |0 0 0|0 1 1 0 1|
	{   0xff00,     0x0d00,     &adsp21062_device::sharcop_do_until_counter_ureg                       },

	//  |0 0 0|0 1 1 1 0|
	{   0xff00,     0x0e00,     &adsp21062_device::sharcop_do_until                                    },

	//  |0 0 0|1 0 0|0|0|
	{   0xff00,     0x1000,     &adsp21062_device::sharcop_dm_to_ureg_direct                           },

	//  |0 0 0|1 0 0|0|1|
	{   0xff00,     0x1100,     &adsp21062_device::sharcop_ureg_to_dm_direct                           },

	//  |0 0 0|1 0 0|1|0|
	{   0xff00,     0x1200,     &adsp21062_device::sharcop_pm_to_ureg_direct                           },

	//  |0 0 0|1 0 0|1|1|
	{   0xff00,     0x1300,     &adsp21062_device::sharcop_ureg_to_pm_direct                           },

	//  |1 0 1|0|x x x|0|
	{   0xf100,     0xa000,     &adsp21062_device::sharcop_dm_to_ureg_indirect                         },

	//  |1 0 1|0|x x x|1|
	{   0xf100,     0xa100,     &adsp21062_device::sharcop_ureg_to_dm_indirect                         },

	//  |1 0 1|1|x x x|0|
	{   0xf100,     0xb000,     &adsp21062_device::sharcop_pm_to_ureg_indirect                         },

	//  |1 0 1|1|x x x|1|
	{   0xf100,     0xb100,     &adsp21062_device::sharcop_ureg_to_pm_indirect                         },

	//  |1 0 0|1|
	{   0xf000,     0x9000,     &adsp21062_device::sharcop_imm_to_dmpm                                 },

	//  |0 0 0|0 1 1 1 1|
	{   0xff00,     0x0f00,     &adsp21062_device::sharcop_imm_to_ureg                                 },

	//  |0 0 0|1 0 1 0 0|
	{   0xff00,     0x1400,     &adsp21062_device::sharcop_sysreg_bitop                                },

	//  |0 0 0|1 0 1 1 0|0|
	{   0xff80,     0x1600,     &adsp21062_device::sharcop_modify                                      },

	//  |0 0 0|1 0 1 1 0|1|
	{   0xff80,     0x1680,     &adsp21062_device::sharcop_bit_reverse                                 },

	//  |0 0 0|1 0 1 1 1|
	{   0xff00,     0x1700,     &adsp21062_device::sharcop_push_pop_stacks                             },

	//  |0 0 0|0 0 0 0 0|0|
	{   0xff80,     0x0000,     &adsp21062_device::sharcop_nop                                         },

	//  |0 0 0|0 0 0 0 0|1|
	{   0xff80,     0x0080,     &adsp21062_device::sharcop_idle                                        },
};
