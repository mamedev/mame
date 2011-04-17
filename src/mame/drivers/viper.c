/*
    Konami Viper System

    Driver by Ville Linde



    Games on this hardware:

    Game ID       Year    Game
    ------------------------------------------------------------------------------------------------------
    GK922         2000    Code One Dispatch
    G????         2001    ParaParaParadise 2nd Mix
    GM941         2001    GTI Club 2
    G?A00         2001    Police 911 (USA) / Police 24/7 (World) / Keisatsukan Shinjuku 24ji (Japan)
    GKA13         2001    Silent Scope EX (USA/World) / Sogeki (Japan)
    G?A29         2001    Mocap Boxing
    G?A30         2001    Tsurugi
    GMA41         2001    Thrill Drive 2
    G?A45         2001    Boxing Mania
    G?B11         2001    Police 911 2 (USA) / Police 24/7 2 (World) / Keisatsukan Shinjuku 24ji 2 (Japan)
    G?B33         2001    Mocap Golf
    G?B41         2001    Jurassic Park 3
    G?B4x         2002    Xtrial Racing
    G?C00         2003    Pop'n Music 9
    G?C09         2002    Mahjong Fight Club
    G?C22         2002    World Combat (USA/Japan/Korea) / Warzaid (Europe)

DASM code snippets:

00FE0B8C: addi      r31,r3,0x0000
00FE0B90: lwz       r3,0x0040(r1)
00FE0B94: cmpi      r31,0x0000 ;offending check, understand where r3 comes from!
00FE0B98: lwz       r4,0x0044(r1)
00FE0B9C: addic     r5,r1,0x0058
00FE0BA0: bne       0x00FE0C00

*/

/*
    Software notes (as per Police 911)
    -- VL - 22.02.2010

    IRQs:

    IRQ0: ???               (Task 4)
    IRQ1: unused
    IRQ2: unused
    IRQ3: ???               (Task 5, sound?)
    IRQ4: Voodoo3           Currently only for User Interrupt Command, maybe a more extensive handler gets installed later?

    I2C:  ???               (no task switch) what drives this? network?
    DMA0: unused
    DMA1: unused
    IIVPR3: unused

    Memory:

    0x000001E0:             Current task
    0x000001E1:             Current FPU task
    0x000001E4:             Scheduled tasks bitvector (bit 31 = task0, etc.)
    0x00000A00...BFF:       Task structures
                            0x00-03:    unknown
                            0x04:       unknown
                            0x05:       if non-zero, this task uses FPU
                            0x06-07:    unknown
                            0x08:       unknown mem pointer, task stack pointer?
                            0x0c:       pointer to task PC (also top of stack?)


    0x00000310:             Global timer 0 IRQ handler
    0x00000320:             Global timer 1 IRQ handler
    0x00000330:             Global timer 2 IRQ handler
    0x00000340:             Global timer 3 IRQ handler
    0x00000350:             IRQ0 handler
    0x00000360:             IRQ1 handler
    0x00000370:             IRQ2 handler
    0x00000380:             IRQ3 handler
    0x00000390:             IRQ4 handler
    0x000003a0:             I2C IRQ handler
    0x000003b0:             DMA0 IRQ handler
    0x000003c0:             DMA1 IRQ handler
    0x000003d0:             Message Unit IRQ handler

    0x000004e4:             Global timer 0 IRQ handler function ptr
    0x000004e8:             Global timer 1 IRQ handler function ptr
    0x000004ec:             Global timer 2 IRQ handler function ptr
    0x000004f0:             Global timer 3 IRQ handler function ptr


    Functions of interest:

    0x0000f7b4:     SwitchTask()
    0x00009d00:     LoadProgram(): R3 = ptr to filename
*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/idectrl.h"
#include "machine/timekpr.h"
#include "video/voodoo.h"

#define VIPER_DEBUG_LOG

static SCREEN_UPDATE(viper)
{
	device_t *device = screen->machine().device("voodoo");
	return voodoo_update(device, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}

class viper_state : public driver_device
{
public:
	viper_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }


	UINT32 m_epic_iack;
	int m_cf_card_ide;
	int m_unk1_bit;
	UINT32 m_voodoo3_pci_reg[0x100];

};

UINT32 m_mpc8240_regs[256/4];

/*****************************************************************************/

static UINT32 mpc8240_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
// device is null?
//  viper_state *state = device->machine().driver_data<viper_state>();
	#ifdef VIPER_DEBUG_LOG
//  printf("MPC8240: PCI read %d, %02X, %08X\n", function, reg, mem_mask);
	#endif

	switch (reg)
	{
	}
	return m_mpc8240_regs[reg/4];
	//return state->m_mpc8240_regs[reg/4];
}

static void mpc8240_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
// device is null?
//  viper_state *state = device->machine().driver_data<viper_state>();
	#ifdef VIPER_DEBUG_LOG
//  printf("MPC8240: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	#endif
	COMBINE_DATA(m_mpc8240_regs + (reg/4));
	//COMBINE_DATA(state->m_mpc8240_regs + (reg/4));
}


static READ64_DEVICE_HANDLER( pci_config_addr_r )
{
	return pci_64be_r(device, 0, U64(0xffffffff00000000));
}

static WRITE64_DEVICE_HANDLER( pci_config_addr_w )
{
	pci_64be_w(device, 0, data, U64(0xffffffff00000000));
}

static READ64_DEVICE_HANDLER( pci_config_data_r )
{
	return pci_64be_r(device, 1, U64(0x00000000ffffffff)) << 32;
}

static WRITE64_DEVICE_HANDLER( pci_config_data_w )
{
	pci_64be_w(device, 1, data >> 32, U64(0x00000000ffffffff));
}



/*****************************************************************************/
// MPC8240 Embedded Programmable Interrupt Controller (EPIC)

#define MPC8240_IRQ0				0
#define MPC8240_IRQ1				1
#define MPC8240_IRQ2				2
#define MPC8240_IRQ3				3
#define MPC8240_IRQ4				4
#define MPC8240_IRQ5				5
#define MPC8240_IRQ6				6
#define MPC8240_IRQ7				7
#define MPC8240_IRQ8				8
#define MPC8240_IRQ9				9
#define MPC8240_IRQ10				10
#define MPC8240_IRQ11				11
#define MPC8240_IRQ12				12
#define MPC8240_IRQ13				13
#define MPC8240_IRQ14				14
#define MPC8240_IRQ15				15
#define MPC8240_I2C_IRQ				16
#define MPC8240_DMA0_IRQ			17
#define MPC8240_DMA1_IRQ			18
#define MPC8240_MSG_IRQ				19
#define MPC8240_GTIMER0_IRQ			20
#define MPC8240_GTIMER1_IRQ			21
#define MPC8240_GTIMER2_IRQ			22
#define MPC8240_GTIMER3_IRQ			23

#define MPC8240_NUM_INTERRUPTS		24


typedef struct
{
	UINT32 vector;
	int priority;
	int destination;
	int active;
	int pending;
	int mask;
} MPC8240_IRQ;



typedef struct
{
	UINT32 iack;
	UINT32 eicr;
	UINT32 svr;

	int active_irq;

	MPC8240_IRQ irq[MPC8240_NUM_INTERRUPTS];

} MPC8240_EPIC;


static MPC8240_EPIC epic;


static void epic_update_interrupts(running_machine &machine)
{
	int i;

	int irq = -1;
	int priority = -1;

	// find the highest priority pending interrupt
	for (i=MPC8240_NUM_INTERRUPTS-1; i >= 0; i--)
	{
		if (epic.irq[i].pending)
		{
			if (epic.irq[i].priority > priority)
			{
				irq = i;
				priority = epic.irq[i].priority;
			}
		}
	}

	if (irq >= 0 && epic.active_irq == -1)
	{
		printf("EPIC IRQ%d taken\n", irq);

		epic.active_irq = irq;
		epic.irq[epic.active_irq].pending = 0;
		epic.irq[epic.active_irq].active = 1;

		epic.iack = epic.irq[epic.active_irq].vector;

		printf("vector = %02X\n", epic.iack);

		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, ASSERT_LINE);
	}
	else
	{
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

static READ32_HANDLER( epic_r )
{
	int reg;
	reg = offset * 4;

	//printf("EPIC: read %08X, %08X at %08X\n", reg, mem_mask, activecpu_get_pc());

	switch (reg >> 16)
	{
		// 0x50000 - 0x5FFFF
		case 0x5:
		{
			switch (reg & 0xffff)
			{
				case 0x0200:			// Offset 0x50200 - IRQ0 vector/priority register
				case 0x0220:			// Offset 0x50220 - IRQ1 vector/priority register
				case 0x0240:			// Offset 0x50240 - IRQ2 vector/priority register
				case 0x0260:			// Offset 0x50260 - IRQ3 vector/priority register
				case 0x0280:			// Offset 0x50280 - IRQ4 vector/priority register
				case 0x02a0:			// Offset 0x502a0 - IRQ5 vector/priority register
				case 0x02c0:			// Offset 0x502c0 - IRQ6 vector/priority register
				case 0x02e0:			// Offset 0x502e0 - IRQ7 vector/priority register
				case 0x0300:			// Offset 0x50300 - IRQ8 vector/priority register
				case 0x0320:			// Offset 0x50320 - IRQ9 vector/priority register
				case 0x0340:			// Offset 0x50340 - IRQ10 vector/priority register
				case 0x0360:			// Offset 0x50360 - IRQ11 vector/priority register
				case 0x0380:			// Offset 0x50380 - IRQ12 vector/priority register
				case 0x03a0:			// Offset 0x503a0 - IRQ13 vector/priority register
				case 0x03c0:			// Offset 0x503c0 - IRQ14 vector/priority register
				case 0x03e0:			// Offset 0x503e0 - IRQ15 vector/priority register
				{
					int irq = ((reg & 0xffff) - 0x200) >> 5;
					int value = 0;

					value |= epic.irq[MPC8240_IRQ0 + irq].mask ? 0x80000000 : 0;
					value |= epic.irq[MPC8240_IRQ0 + irq].priority << 16;
					value |= epic.irq[MPC8240_IRQ0 + irq].vector;
					value |= epic.irq[MPC8240_IRQ0 + irq].active ? 0x40000000 : 0;

					return value;
				}
			}
			break;
		}

		// 0x60000 - 0x6FFFF
		case 0x6:
		{
			switch (reg & 0xffff)
			{
				case 0x00a0:			// Offset 0x600A0 - IACK
				{
					epic_update_interrupts(space->machine());

					if (epic.active_irq >= 0)
					{
						return epic.iack;
					}
					else
					{
						// spurious vector register is returned if no pending interrupts
						return epic.svr;
					}
				}

			}
			break;
		}
	}

	return 0;
}

static WRITE32_HANDLER( epic_w )
{
	int reg;
	reg = offset * 4;

	printf("EPIC: write %08X, %08X, %08X at %08X\n", data, reg, mem_mask, cpu_get_pc(&space->device()));

	switch (reg >> 16)
	{
		// 0x40000 - 0x4FFFF
		case 4:
		{
			switch (reg & 0xffff)
			{
				case 0x1030:			// Offset 0x41030 - EICR
				{
					epic.eicr = data;
					if (data & 0x08000000)
						fatalerror("EPIC: serial interrupts mode not implemented");
					break;
				}
				case 0x10e0:			// Offset 0x410E0 - Spurious Vector Register
				{
					epic.svr = data;
					break;
				}
				case 0x1120:			// Offset 0x41120 - Global timer 0 vector/priority register
				case 0x1160:			// Offset 0x41160 - Global timer 1 vector/priority register
				case 0x11a0:			// Offset 0x411A0 - Global timer 2 vector/priority register
				case 0x11e0:			// Offset 0x411E0 - Global timer 3 vector/priority register
				{
					int timer = ((reg & 0xffff) - 0x1120) >> 6;

					epic.irq[MPC8240_GTIMER0_IRQ + timer].mask = (data & 0x80000000) ? 1 : 0;
					epic.irq[MPC8240_GTIMER0_IRQ + timer].priority = (data >> 16) & 0xf;
					epic.irq[MPC8240_GTIMER0_IRQ + timer].vector = data & 0xff;
					break;
				}
				case 0x1130:			// Offset 0x41130 - Global timer 0 destination register
				case 0x1170:			// Offset 0x41170 - Global timer 1 destination register
				case 0x11b0:			// Offset 0x411B0 - Global timer 2 destination register
				case 0x11f0:			// Offset 0x411F0 - Global timer 3 destination register
				{
					int timer = ((reg & 0xffff) - 0x1130) >> 6;

					epic.irq[MPC8240_GTIMER0_IRQ + timer].destination = data & 0x1;
					break;
				}
			}
			break;
		}

		// 0x50000 - 0x5FFFF
		case 0x5:
		{
			switch (reg & 0xffff)
			{
				case 0x0200:			// Offset 0x50200 - IRQ0 vector/priority register
				case 0x0220:			// Offset 0x50220 - IRQ1 vector/priority register
				case 0x0240:			// Offset 0x50240 - IRQ2 vector/priority register
				case 0x0260:			// Offset 0x50260 - IRQ3 vector/priority register
				case 0x0280:			// Offset 0x50280 - IRQ4 vector/priority register
				case 0x02a0:			// Offset 0x502a0 - IRQ5 vector/priority register
				case 0x02c0:			// Offset 0x502c0 - IRQ6 vector/priority register
				case 0x02e0:			// Offset 0x502e0 - IRQ7 vector/priority register
				case 0x0300:			// Offset 0x50300 - IRQ8 vector/priority register
				case 0x0320:			// Offset 0x50320 - IRQ9 vector/priority register
				case 0x0340:			// Offset 0x50340 - IRQ10 vector/priority register
				case 0x0360:			// Offset 0x50360 - IRQ11 vector/priority register
				case 0x0380:			// Offset 0x50380 - IRQ12 vector/priority register
				case 0x03a0:			// Offset 0x503a0 - IRQ13 vector/priority register
				case 0x03c0:			// Offset 0x503c0 - IRQ14 vector/priority register
				case 0x03e0:			// Offset 0x503e0 - IRQ15 vector/priority register
				{
					int irq = ((reg & 0xffff) - 0x200) >> 5;

					epic.irq[MPC8240_IRQ0 + irq].mask = (data & 0x80000000) ? 1 : 0;
					epic.irq[MPC8240_IRQ0 + irq].priority = (data >> 16) & 0xf;
					epic.irq[MPC8240_IRQ0 + irq].vector = data & 0xff;
					break;
				}
				case 0x0210:			// Offset 0x50210 - IRQ0 destination register
				case 0x0230:			// Offset 0x50230 - IRQ1 destination register
				case 0x0250:			// Offset 0x50250 - IRQ2 destination register
				case 0x0270:			// Offset 0x50270 - IRQ3 destination register
				case 0x0290:			// Offset 0x50290 - IRQ4 destination register
				case 0x02b0:			// Offset 0x502b0 - IRQ5 destination register
				case 0x02d0:			// Offset 0x502d0 - IRQ6 destination register
				case 0x02f0:			// Offset 0x502f0 - IRQ7 destination register
				case 0x0310:			// Offset 0x50310 - IRQ8 destination register
				case 0x0330:			// Offset 0x50330 - IRQ9 destination register
				case 0x0350:			// Offset 0x50350 - IRQ10 destination register
				case 0x0370:			// Offset 0x50370 - IRQ11 destination register
				case 0x0390:			// Offset 0x50390 - IRQ12 destination register
				case 0x03b0:			// Offset 0x503b0 - IRQ13 destination register
				case 0x03d0:			// Offset 0x503d0 - IRQ14 destination register
				case 0x03f0:			// Offset 0x503f0 - IRQ15 destination register
				{
					int irq = ((reg & 0xffff) - 0x210) >> 5;

					epic.irq[MPC8240_IRQ0 + irq].destination = data & 0x1;
					break;
				}
			}
			break;
		}

		// 0x60000 - 0x6FFFF
		case 0x6:
		{
			switch (reg & 0xffff)
			{
				case 0x00b0:			// Offset 0x600B0 - EOI
					printf("EPIC IRQ%d cleared.\n", epic.active_irq);
					epic.irq[epic.active_irq].active = 0;
					epic.active_irq = -1;

					epic_update_interrupts(space->machine());
					break;
			}
			break;
		}
	}
}

static READ64_HANDLER(epic_64be_r)
{
	return read64be_with_32le_handler(epic_r, space, offset, mem_mask);
}
static WRITE64_HANDLER(epic_64be_w)
{
	write64be_with_32le_handler(epic_w, space, offset, data, mem_mask);
}


static void mpc8240_interrupt(running_machine &machine, int irq)
{
	if (epic.irq[irq].mask == 0 && epic.irq[irq].priority > 0)
	{
		epic.irq[irq].pending = 1;

		epic_update_interrupts(machine);
	}
}

static void mpc8240_epic_reset(void)
{
	int i;

	for (i=0; i < MPC8240_NUM_INTERRUPTS; i++)
	{
		epic.irq[i].mask = 1;
	}

	epic.active_irq = -1;
}

/*****************************************************************************/


static const UINT8 cf_card_tuples[] =
{
	0x01,		// Device Tuple
	0x01,		// Tuple size
	0xd0,		// Device Type Func Spec

	0x1a,		// Config Tuple
	0xff,		// Tuple size (last?)
	0x03,		// CCR base size
	0x00,		// last config index?
	0x00, 0x01, 0x00, 0x00,		// CCR base (0x00000100)
};

static READ64_DEVICE_HANDLER(cf_card_data_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:	// Duplicate Even RD Data
			{
				r |= ide_bus_r(device, 0, 0) << 16;
				break;
			}

			default:
			{
				fatalerror("%s:cf_card_data_r: IDE reg %02X\n", device->machine().describe_context(), offset & 0xf);
			}
		}
	}
	return r;
}

static WRITE64_DEVICE_HANDLER(cf_card_data_w)
{
	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:	// Duplicate Even RD Data
			{
				ide_bus_w(device, 0, 0, (data >> 16) & 0xffff);
				break;
			}

			default:
			{
				fatalerror("%s:cf_card_data_w: IDE reg %02X, %04X\n", device->machine().describe_context(), offset & 0xf, (UINT16)(data >> 16));
			}
		}
	}
}

static READ64_DEVICE_HANDLER(cf_card_r)
{
	viper_state *state = device->machine().driver_data<viper_state>();

	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		if (state->m_cf_card_ide)
		{
			switch (offset & 0xf)
			{
				case 0x0:	// Even RD Data
				case 0x1:	// Error
				case 0x2:	// Sector Count
				case 0x3:	// Sector No.
				case 0x4:	// Cylinder Low
				case 0x5:	// Cylinder High
				case 0x6:	// Select Card/Head
				case 0x7:	// Status
				{
					r |= ide_bus_r(device, 0, offset & 7) << 16;
					break;
				}

				//case 0x8: // Duplicate Even RD Data
				//case 0x9: // Duplicate Odd RD Data

				case 0xd:	// Duplicate Error
				{
					r |= ide_bus_r(device, 0, 1) << 16;
					break;
				}
				case 0xe:	// Alt Status
				case 0xf:	// Drive Address
				{
					r |= ide_bus_r(device, 1, offset & 7) << 16;
					break;
				}

				default:
				{
					printf("%s:compact_flash_r: IDE reg %02X\n", device->machine().describe_context(), offset & 0xf);
				}
			}
		}
		else
		{
			int reg = offset;

			printf("cf_r: %04X\n", reg);

			if ((reg >> 1) < sizeof(cf_card_tuples))
			{
				r |= cf_card_tuples[reg >> 1] << 16;
			}
			else
			{
				fatalerror("%s:compact_flash_r: reg %02X\n", device->machine().describe_context(), reg);
			}
		}
	}
	return r;
}

static WRITE64_DEVICE_HANDLER(cf_card_w)
{
	viper_state *state = device->machine().driver_data<viper_state>();

	#ifdef VIPER_DEBUG_LOG
	//printf("%s:compact_flash_w: %08X%08X, %08X, %08X%08X\n", device->machine().describe_context(), (UINT32)(data>>32), (UINT32)(data), offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));
	#endif

	if (ACCESSING_BITS_16_31)
	{
		if (offset < 0x10)
		{
			switch (offset & 0xf)
			{
				case 0x0:	// Even WR Data
				case 0x1:	// Features
				case 0x2:	// Sector Count
				case 0x3:	// Sector No.
				case 0x4:	// Cylinder Low
				case 0x5:	// Cylinder High
				case 0x6:	// Select Card/Head
				case 0x7:	// Command
				{
					ide_bus_w(device, 0, offset & 7, (data >> 16) & 0xffff);
					break;
				}

				//case 0x8: // Duplicate Even WR Data
				//case 0x9: // Duplicate Odd WR Data

				case 0xd:	// Duplicate Features
				{
					ide_bus_w(device, 0, 1, (data >> 16) & 0xffff);
					break;
				}
				case 0xe:	// Device Ctl
				case 0xf:	// Reserved
				{
					ide_bus_w(device, 1, offset & 7, (data >> 16) & 0xffff);
					break;
				}

				default:
				{
					fatalerror("%s:compact_flash_w: IDE reg %02X, data %04X\n", device->machine().describe_context(), offset & 0xf, (UINT16)((data >> 16) & 0xffff));
				}
			}
		}
		else if (offset >= 0x100)
		{
			switch (offset)
			{
				case 0x100:
				{
					if ((data >> 16) & 0x80)
					{
						state->m_cf_card_ide = 1;

						// soft reset
						// sector count register is set to 0x01
						// sector number register is set to 0x01
						// cylinder low register is set to 0x00
						// cylinder high register is set to 0x00

						ide_bus_w(device, 1, 6, 0x04);

						ide_bus_w(device, 0, 2, 0x01);
						ide_bus_w(device, 0, 3, 0x01);
						ide_bus_w(device, 0, 4, 0x00);
						ide_bus_w(device, 0, 5, 0x00);
					}
					break;
				}
				default:
				{
					fatalerror("%s:compact_flash_w: reg %02X, data %04X\n", device->machine().describe_context(), offset, (UINT16)((data >> 16) & 0xffff));
				}
			}
		}
	}
}

static WRITE64_HANDLER(unk2_w)
{
	viper_state *state = space->machine().driver_data<viper_state>();

	if (ACCESSING_BITS_56_63)
	{
		state->m_cf_card_ide = 0;
	}
}




static READ64_DEVICE_HANDLER(ata_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		r |= ide_bus_r(device, (offset & 0x80) ? 1 : 0, reg) << 16;
	}

	return r;
}

static WRITE64_DEVICE_HANDLER(ata_w)
{
	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		ide_bus_w(device, (offset & 0x80) ? 1 : 0, reg, (UINT16)(data >> 16));
	}
}


static READ64_HANDLER(unk1_r)
{
	viper_state *state = space->machine().driver_data<viper_state>();

	UINT64 r = 0;
	//return 0;//U64(0x0000400000000000);

	if (ACCESSING_BITS_40_47)
	{
		r |= (UINT64)(state->m_unk1_bit << 5) << 40;
		r |= U64(0x0000400000000000);
	}

	return r;
}

static WRITE64_HANDLER(unk1a_w)
{
	viper_state *state = space->machine().driver_data<viper_state>();

	if (ACCESSING_BITS_56_63)
	{
		state->m_unk1_bit = 1;
	}
}

static WRITE64_HANDLER(unk1b_w)
{
	viper_state *state = space->machine().driver_data<viper_state>();

	if (ACCESSING_BITS_56_63)
	{
		state->m_unk1_bit = 0;
	}
}


static UINT32 voodoo3_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	viper_state *state = device->machine().driver_data<viper_state>();

	switch (reg)
	{
		case 0x00:		// PCI Vendor ID (0x121a = 3dfx), Device ID (0x0005 = Voodoo 3)
		{
			return 0x0005121a;
		}
		case 0x08:		// Device class code
		{
			return 0x03000000;
		}
		case 0x10:		// memBaseAddr0
		{
			return state->m_voodoo3_pci_reg[0x10/4];
		}
		case 0x14:		// memBaseAddr1
		{
			return state->m_voodoo3_pci_reg[0x14/4];
		}
		case 0x18:		// memBaseAddr1
		{
			return state->m_voodoo3_pci_reg[0x18/4];
		}
		case 0x40:		// fabId
		{
			return state->m_voodoo3_pci_reg[0x40/4];
		}
		case 0x50:		// cfgScratch
		{
			return state->m_voodoo3_pci_reg[0x50/4];
		}

		default:
			fatalerror("voodoo3_pci_r: %08X at %08X", reg, cpu_get_pc(device->machine().device("maincpu")));
	}
	return 0;
}

static void voodoo3_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	viper_state *state = device->machine().driver_data<viper_state>();

//  printf("voodoo3_pci_w: %08X, %08X\n", reg, data);

	switch (reg)
	{
		case 0x04:		// Command register
		{
			state->m_voodoo3_pci_reg[0x04/4] = data;
			break;
		}
		case 0x10:		// memBaseAddr0
		{
			if (data == 0xffffffff)
			{
				state->m_voodoo3_pci_reg[0x10/4] = 0xfe000000;
			}
			else
			{
				state->m_voodoo3_pci_reg[0x10/4] = data;
			}
			break;
		}
		case 0x14:		// memBaseAddr1
		{
			if (data == 0xffffffff)
			{
				state->m_voodoo3_pci_reg[0x14/4] = 0xfe000008;
			}
			else
			{
				state->m_voodoo3_pci_reg[0x14/4] = data;
			}
			break;
		}
		case 0x18:		// ioBaseAddr
		{
			if (data == 0xffffffff)
			{
				state->m_voodoo3_pci_reg[0x18/4] = 0xffffff01;
			}
			else
			{
				state->m_voodoo3_pci_reg[0x18/4] = data;
			}
			break;
		}
		case 0x3c:		// InterruptLine
		{
			break;
		}
		case 0x40:		// fabId
		{
			state->m_voodoo3_pci_reg[0x40/4] = data;
			break;
		}
		case 0x50:		// cfgScratch
		{
			state->m_voodoo3_pci_reg[0x50/4] = data;
			break;
		}

		default:
			fatalerror("voodoo3_pci_w: %08X, %08X at %08X", data, reg, cpu_get_pc(device->machine().device("maincpu")));
	}
}

static READ64_HANDLER(voodoo3_io_r)
{
	device_t *device = space->machine().device("voodoo");
	return read64be_with_32le_device_handler(banshee_io_r, device, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_io_w)
{
//  printf("voodoo3_io_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(&space->device()));

	device_t *device = space->machine().device("voodoo");
	write64be_with_32le_device_handler(banshee_io_w, device, offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_r)
{
	device_t *device = space->machine().device("voodoo");
	return read64be_with_32le_device_handler(banshee_r, device, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_w)
{
//  printf("voodoo3_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(&space->device()));

	device_t *device = space->machine().device("voodoo");
	write64be_with_32le_device_handler(banshee_w, device,  offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_lfb_r)
{
	device_t *device = space->machine().device("voodoo");
	return read64be_with_32le_device_handler(banshee_fb_r, device, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_lfb_w)
{
//  printf("voodoo3_lfb_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(&space->device()));

	device_t *device = space->machine().device("voodoo");
	write64be_with_32le_device_handler(banshee_fb_w, device, offset, data, mem_mask);
}


/*****************************************************************************/

static ADDRESS_MAP_START(viper_map, AS_PROGRAM, 64)
	AM_RANGE(0x00000000, 0x00ffffff) AM_MIRROR(0x1000000) AM_RAM
	AM_RANGE(0x80000000, 0x800fffff) AM_READWRITE(epic_64be_r, epic_64be_w)
//  AM_RANGE(0x82000000, 0x83ffffff) AM_DEVREADWRITE32("voodoo", banshee_r, banshee_w, U64(0xffffffffffffffff))
//  AM_RANGE(0x84000000, 0x85ffffff) AM_DEVREADWRITE32("voodoo", banshee_fb_r, banshee_fb_w, U64(0xffffffffffffffff))
//  AM_RANGE(0xfe800000, 0xfe8000ff) AM_DEVREADWRITE32("voodoo", banshee_io_r, banshee_io_w, U64(0xffffffffffffffff))
	AM_RANGE(0x82000000, 0x83ffffff) AM_READWRITE(voodoo3_r, voodoo3_w)
	AM_RANGE(0x84000000, 0x85ffffff) AM_READWRITE(voodoo3_lfb_r, voodoo3_lfb_w)
	AM_RANGE(0xfe800000, 0xfe8000ff) AM_READWRITE(voodoo3_io_r, voodoo3_io_w)
	AM_RANGE(0xfec00000, 0xfedfffff) AM_DEVREADWRITE("pcibus", pci_config_addr_r, pci_config_addr_w)
	AM_RANGE(0xfee00000, 0xfeefffff) AM_DEVREADWRITE("pcibus", pci_config_data_r, pci_config_data_w)
	// 0xff000000, 0xff000fff - cf_card_data_r/w (installed in DRIVER_INIT(vipercf))
	// 0xff200000, 0xff200fff - cf_card_r/w (installed in DRIVER_INIT(vipercf))
	AM_RANGE(0xff300000, 0xff300fff) AM_DEVREADWRITE("ide", ata_r, ata_w)
	AM_RANGE(0xffe10000, 0xffe10007) AM_READ(unk1_r)
	AM_RANGE(0xffe30000, 0xffe31fff) AM_DEVREADWRITE8("m48t58",timekeeper_r, timekeeper_w, U64(0xffffffffffffffff))
	AM_RANGE(0xffe40000, 0xffe4000f) AM_NOP
	AM_RANGE(0xffe50000, 0xffe50007) AM_WRITE(unk2_w)
	AM_RANGE(0xffe80000, 0xffe80007) AM_WRITE(unk1a_w)
	AM_RANGE(0xffe88000, 0xffe88007) AM_WRITE(unk1b_w)
	AM_RANGE(0xfff00000, 0xfff3ffff) AM_ROM AM_REGION("user1", 0)		// Boot ROM
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( viper )
INPUT_PORTS_END

/*****************************************************************************/


static const powerpc_config viper_ppc_cfg =
{
	66000000
};

static INTERRUPT_GEN(viper_vblank)
{
	mpc8240_interrupt(device->machine(), MPC8240_IRQ0);
	//mpc8240_interrupt(device->machine, MPC8240_IRQ3);
}

static void voodoo_vblank(const device_config *device, int param)
{
	//mpc8240_interrupt(device->machine, MPC8240_IRQ4);
}

static void ide_interrupt(device_t *device, int state)
{
}

static MACHINE_RESET(viper)
{
	devtag_reset(machine, "ide");
	mpc8240_epic_reset();
}




static MACHINE_CONFIG_START( viper, viper_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MPC8240, 200000000)
	MCFG_CPU_CONFIG(viper_ppc_cfg)
	MCFG_CPU_PROGRAM_MAP(viper_map)
	MCFG_CPU_VBLANK_INT("screen", viper_vblank)

	MCFG_MACHINE_RESET(viper)

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE(0, "mpc8240", mpc8240_pci_r, mpc8240_pci_w)
	MCFG_PCI_BUS_DEVICE(12, "voodoo", voodoo3_pci_r, voodoo3_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt)
	MCFG_3DFX_VOODOO_3_ADD("voodoo", STD_VOODOO_3_CLOCK, 16, "screen")
	MCFG_3DFX_VOODOO_CPU("maincpu")
	MCFG_3DFX_VOODOO_VBLANK(voodoo_vblank)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(800, 600)
	MCFG_SCREEN_VISIBLE_AREA(0, 799, 0, 599)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_SCREEN_UPDATE(viper)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_M48T58_ADD( "m48t58" )
MACHINE_CONFIG_END

/*****************************************************************************/

static DRIVER_INIT(viper)
{
//  machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler( *ide, 0xff200000, 0xff207fff, FUNC(hdd_r), FUNC(hdd_w) ); //TODO
}

static DRIVER_INIT(vipercf)
{
	device_t *ide = machine.device("ide");

	DRIVER_INIT_CALL(viper);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler( *ide, 0xff000000, 0xff000fff, FUNC(cf_card_data_r), FUNC(cf_card_data_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler( *ide, 0xff200000, 0xff200fff, FUNC(cf_card_r), FUNC(cf_card_w) );
}


/*****************************************************************************/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define VIPER_BIOS \
	ROM_REGION64_BE(0x40000, "user1", 0)	/* Boot ROM */ \
	ROM_SYSTEM_BIOS(0, "bios0", "GM941B01 (01/15/01)") \
		ROM_LOAD_BIOS(0, "941b01.u25", 0x00000, 0x40000, CRC(233e5159) SHA1(66ff268d5bf78fbfa48cdc3e1b08f8956cfd6cfb)) \
	ROM_SYSTEM_BIOS(1, "bios1", "GM941A01 (03/10/00)") \
		ROM_LOAD_BIOS(1, "941a01.u25", 0x00000, 0x40000, CRC(df6f88d6) SHA1(2bc10e4fbec36573aa8b6878492d37665f074d87)) \


ROM_START(kviper)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
ROM_END


/* Viper games with hard disk */
ROM_START(ppp2nd)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
	DISK_IMAGE( "ppp2nd", 0, SHA1(b8b90483d515c83eac05ffa617af19612ea990b0))
ROM_END

/* Viper games with Compact Flash card */
ROM_START(boxingm) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a45jaa_nvram.u39", 0x00000, 0x2000, CRC(c24e29fc) SHA1(efb6ecaf25cbdf9d8dfcafa85e38a195fa5ff6c4))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a45a02", 0, SHA1(9af2481f53de705ae48fad08d8dd26553667c2d0) )
ROM_END

ROM_START(code1d) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "922d02", 0, SHA1(01f35e324c9e8567da0f51b3e68fff1562c32116) )
ROM_END

ROM_START(code1db) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "922b02", 0, SHA1(4d288b5dcfab3678af662783e7083a358eee99ce) )
ROM_END

ROM_START(gticlub2) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(d0604e84) SHA1(18d1183f1331af3e655a56692eb7ab877b4bc239)) //old dump, probably has non-default settings.
	ROM_LOAD("941jab_nvram.u39", 0x00000, 0x2000, CRC(6c4a852f) SHA1(2753dda42cdd81af22dc6780678f1ddeb3c62013))

	DISK_REGION( "ide" )
	DISK_IMAGE( "941b02", 0,  SHA1(943bc9b1ea7273a8382b94c8a75010dfe296df14) )
ROM_END

ROM_START(gticlub2ea) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, CRC(5ee7004d) SHA1(92e0ce01049308f459985d466fbfcfac82f34a47))

	DISK_REGION( "ide" )
	DISK_IMAGE( "941a02", 0,  NO_DUMP )
ROM_END

ROM_START(jpark3) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b41ebc_nvram.u39", 0x00000, 0x2000, CRC(55d1681d) SHA1(26868cf0d14f23f06b81f2df0b4186924439bb43))

	DISK_REGION( "ide" )
	DISK_IMAGE( "b41c02", 0, SHA1(fb6b0b43a6f818041d644bcd711f6a727348d3aa) )
ROM_END

/* This CF card has sticker B33A02 */
ROM_START(mocapglf) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b33uaa_nvram.u39", 0x00000, 0x1ff8, BAD_DUMP CRC(0f0ba988) SHA1(5618c03b21fc2ba14b2e159cee3aab7f53c2c34d)) //data looks plain bad (compared to the other games)

	DISK_REGION( "ide" )
	DISK_IMAGE( "b33a02", 0, SHA1(819d8fac5d2411542c1b989105cffe38a5545fc2) )
ROM_END

ROM_START(mocapb) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29aaa_nvram.u39", 0x000000, 0x2000, CRC(14b9fe68) SHA1(3c59e6df1bb46bc1835c13fd182b1bb092c08759)) //supposed to be aab version?

	DISK_REGION( "ide" )
	DISK_IMAGE( "a29b02", 0, SHA1(f0c04310caf2cca804fde20805eb30a44c5a6796) ) //missing bootloader
ROM_END

ROM_START(mocapbj) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29jaa_nvram.u39", 0x000000, 0x2000, CRC(2f7cdf27) SHA1(0b69d8728be12909e235268268a312982f81d46a))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a29a02", 0, SHA1(00afad399737652b3e17257c70a19f62e37f3c97) )
ROM_END

ROM_START(p911) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00uad_nvram.u39", 0x000000, 0x2000, CRC(cca056ca) SHA1(de1a00d84c1311d48bbe6d24f5b36e22ecf5e85a))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00uad02", 0, SHA1(6acb8dc41920e7025b87034a3a62b185ef0109d9) )
ROM_END

ROM_START(p911uc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00uac_nvram.u39", 0x000000, 0x2000,  NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00uac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) )
ROM_END

ROM_START(p911kc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00kac_nvram.u39", 0x000000, 0x2000,  CRC(8ddc921c) SHA1(901538da237679fc74966a301278b36d1335671f) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00kac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) )
ROM_END

ROM_START(p911e) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00eaa_nvram.u39", 0x000000, 0x2000,  CRC(4f3497b6) SHA1(3045c54f98dff92cdf3a1fc0cd4c76ba82d632d7) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00eaa02", 0, SHA1(81565a2dce2e2b0a7927078a784354948af1f87c) )
ROM_END

ROM_START(p911j) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00jaa_nvram.u39", 0x000000, 0x2000, CRC(9ecf70dc) SHA1(4769a99b0cc28563e219860b8d480f32d1e21f60))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00jac02", 0, SHA1(d962d3a8ea84c380767d0fe336296911c289c224) )
ROM_END

ROM_START(p9112) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "b11a02", 0, SHA1(57665664321b78c1913d01f0d2c0b8d3efd42e04) )
ROM_END

ROM_START(popn9) //Note: this is actually a Konami Pyson HW! (PlayStation 2-based) move out of here.
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "c00jab", 0, BAD_DUMP SHA1(3763aaded9b45388a664edd84a3f7f8ff4101be4) )
ROM_END

ROM_START(sscopex)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a13uaa_nvram.u39", 0x000000, 0x2000, CRC(7b0e1ac8) SHA1(1ea549964539e27f87370e9986bfa44eeed037cd))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a13c02", 0, SHA1(d740784fa51a3f43695ea95e23f92ef05f43284a) )
ROM_END

//TODO: sscopexb + many nvram clone versions.

ROM_START(sogeki) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2f325c55) SHA1(0bc44f40f981a815c8ce64eae95ae55db510c565))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a13b02", 0, SHA1(c25a61b76d365794c2da4a9e7de88a5519e944ec) )
ROM_END

ROM_START(thrild2) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41ebb_nvram.u39", 0x00000, 0x2000, CRC(22f59ac0) SHA1(e14ea2ba95b72edf0a3331ab82c192760bfdbce3))
//  a41eba_nvram == a41ebb_nvram

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41b02", 0, SHA1(0426f4bb9001cf457f44e2c22e3d7575b8049aa3) )
ROM_END

ROM_START(thrild2a) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41aaa_nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41a02", 0, SHA1(bbb71e23bddfa07dfa30b6565a35befd82b055b8) )
ROM_END

/* This CF card has sticker 941EAA02 */
ROM_START(thrild2c) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41c02", 0, SHA1(ab3020e8709768c0fd2467573e92b679a05944e5) )
ROM_END

ROM_START(tsurugi) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30eab_nvram.u39", 0x00000, 0x2000, CRC(c123342c) SHA1(55416767608fe0311a362854a16b214b04435a31))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a30b02", 0, SHA1(d2be83b7323c365ba445de7697c3fb8eb83d0212) )
ROM_END

ROM_START(tsurugij) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30jac_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a30c02", 0, SHA1(533b5669b00884a800df9ba29651777a76559862) )
ROM_END

/* This CF card has sticker C22D02 */
ROM_START(wcombat) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(4f8b5858) SHA1(68066241c6f9db7f45e55b3c5da101987f4ce53c))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22d02", 0, BAD_DUMP SHA1(85d2a8b5ec4cfd932190486cad991f0c180ca6b3) )
ROM_END

ROM_START(wcombatk) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(ebd4d645) SHA1(2fa7e2c6b113214f3eb1900c8ceef4d5fcf0bb76))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22c02", 0, BAD_DUMP SHA1(8bd1dfbf926ad5b28fa7dafd7e31c475325ec569) )
ROM_END

ROM_START(wcombatj) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(bd8a6640) SHA1(2d409197ef3fb07d984d27fa943f29c7a711d715))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22a02", 0, BAD_DUMP SHA1(b607fb2ddfd0bd552b7a736cea4ac1aa3ea021bd) )
ROM_END

ROM_START(xtrial) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b4xjab_nvram.u39", 0x00000, 0x2000, CRC(33708a93) SHA1(715968e3c9c15edf628fa6ac655dc0864e336c6c))

	DISK_REGION( "ide" )
	DISK_IMAGE( "b4xb02", 0, SHA1(d8d54f3f16b762bf0187fe29b2f8696015c0a940) )
ROM_END

/* Viper Satellite Terminal games */

/*
Mahjong Fight Club (Konami Viper h/w)
Konami, 2002

PCB number - GM941-PWB(A)C Copyright 1999 Konami Made In Japan

Mahjong Fight Club is a multi player Mahjong battle game for up to 8 players. A
single PCB will not boot unless all of the other units are connected and powered
on, although how exactly they're connected is unknown. There is probably a
master unit that talks to all of the 8 satellite units. At the moment I have
only 2 of the 8 satellite units so I can't confirm that.
However, I don't have access to the main unit anyway as it was not included in
the auction we won :(

The Viper hardware can accept additional PCBs inside the metal box depending on
the game. For Mahjong Fight Club, no additional PCBs are present or required.

The main CPU is a Motorola XPC8240LZU200E
The main graphics chip is heatsinked. It's a BGA chip, and might be something
like a Voodoo chip? Maybe :-)
There's 1 Konami chip stamped 056879
There's also a bunch of video RAMs and several PLCC FPGAs or CPLDs
There's also 1 PLCC44 chip stamped PC16552

Files
-----
c09jad04.bin is a 64M Compact Flash card. The image was simply copied from the
card as it is PC readable. The card contains only 1 file named c09jad04.bin

941b01.u25 is the BIOS, held in a 2MBit PLCC32 Fujitsu MBM29F002 EEPROM and
surface mounted at location U25. The BIOS is common to ALL Viper games.

nvram.u39 is a ST M48T58Y Timekeeper NVRAM soldered-in at location U39. The
codes at the start of the image (probably just the first 16 or 32 bytes) are
used as a simple (and very weak) protection check to stop game swaps. The
contents of the NVRAM is different for ALL games on this hardware.

Some games use a dongle and swapping games won't work unless the dongle is also provided.
The following games comes with a dongle....
Mahjong Fight Club

For non-dongled games, I have verified the following games will work when the
CF card and NVRAM are swapped....
*/

/* This CF card has sticker C09JAD04 */
ROM_START(mfightc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(9fb551a5) SHA1(a33d185e186d404c3bf62277d7e34e5ad0000b09)) //likely non-default settings
	ROM_LOAD("c09jad_nvram.u39", 0x00000, 0x2000, CRC(33e960b7) SHA1(a9a249e68c89b18d4685f1859fe35dc21df18e14))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c09d04", 0, SHA1(7395b7a33e953f65827aea44461e49f8388464fb) )
ROM_END

/* This CF card has sticker C09JAC04 */
ROM_START(mfightcc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("c09jac_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "c09c04", 0, SHA1(bf5f7447d74399d34edd4eb6dfcca7f6fc2154f2) )
ROM_END

/*****************************************************************************/

/* Viper BIOS */
GAME(1999, kviper,    0,         viper, viper, viper,    ROT0,  "Konami", "Konami Viper BIOS", GAME_IS_BIOS_ROOT)

GAME(2001, ppp2nd,    kviper,    viper, viper, viper,    ROT0,  "Konami", "ParaParaParadise 2nd Mix", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2001, boxingm,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Boxing Mania (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1d,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Code One Dispatch (ver D)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1db,   code1d,    viper, viper, vipercf,  ROT0,  "Konami", "Code One Dispatch (ver B)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, gticlub2,  kviper,    viper, viper, vipercf,  ROT0,  "Konami", "GTI Club 2 (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, gticlub2ea,gticlub2,  viper, viper, vipercf,  ROT0,  "Konami", "GTI Club 2 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, jpark3,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Jurassic Park 3 (ver EBC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapglf,  kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Golf (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapb,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapbj,   mocapb,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911,      kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver UAD)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911uc,    p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver UAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911kc,    p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver KAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911e,     p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 24/7 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911j,     p911,      viper, viper, vipercf,  ROT0,  "Konami", "Keisatsukan Shinjuku 24ji (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p9112,     kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Police 911 2 (ver A)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2003, popn9,     kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Pop'n Music 9 (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sscopex,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Silent Scope EX (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sogeki,    sscopex,   viper, viper, vipercf,  ROT0,  "Konami", "Sogeki (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EBB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2a,  thrild2,   viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2c,  thrild2,   viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugi,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Tsurugi (ver EAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugij,  tsurugi,   viper, viper, vipercf,  ROT0,  "Konami", "Tsurugi (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombat,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver UAA?)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombatk,  wcombat,   viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver KBC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombatj,  wcombat,   viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, xtrial,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Xtrial Racing (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2002, mfightc,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAD)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, mfightcc,  mfightc,   viper, viper, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
