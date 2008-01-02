/*

    dc.c - Sega Dreamcast hardware

*/

#include "mamecore.h"
#include "driver.h"
#include "dc.h"

#define DEBUG_REGISTERS	(1)

#if DEBUG_REGISTERS

#define DEBUG_SYSCTRL	(1)
#define DEBUG_MAPLE	(0)

#if DEBUG_SYSCTRL
static const char *sysctrl_names[] =
{
	"CH2 DMA dest",
	"CH2 DMA length",
	"CH2 DMA start",
	"5f680c",
	"Sort DMA start link table addr",
	"Sort DMA link base addr",
	"Sort DMA link address bit width",
	"Sort DMA link address shift control",
	"Sort DMA start",
	"5f6824", "5f6828", "5f682c", "5f6830",
	"5f6834", "5f6838", "5f683c",
	"DBREQ # signal mask control",
	"BAVL # signal wait count",
	"DMA priority count",
	"CH2 DMA max burst length",
	"5f6850", "5f6854", "5f6858", "5f685c",
	"5f6860", "5f6864", "5f6868", "5f686c",
	"5f6870", "5f6874", "5f6878", "5f687c",
	"TA FIFO remaining",
	"TA texture memory bus select 0",
	"TA texture memory bus select 1",
	"FIFO status",
	"System reset", "5f6894", "5f6898",
	"System bus version",
	"SH4 root bus split enable",
	"5f68a4", "5f68a8", "5f68ac",
	"5f68b0", "5f68b4", "5f68b8", "5f68bc",
	"5f68c0", "5f68c4", "5f68c8", "5f68cc",
	"5f68d0", "5f68d4", "5f68d8", "5f68dc",
	"5f68e0", "5f68e4", "5f68e8", "5f68ec",
	"5f68f0", "5f68f4", "5f68f8", "5f68fc",
	"Normal IRQ status",
	"External IRQ status",
	"Error IRQ status", "5f690c",
	"Level 2 normal IRQ mask",
	"Level 2 external IRQ mask",
	"Level 2 error IRQ mask", "5f691c",
	"Level 4 normal IRQ mask",
	"Level 4 external IRQ mask",
	"Level 4 error IRQ mask", "5f692c",
	"Level 6 normal IRQ mask",
	"Level 6 external IRQ mask",
	"Level 6 error IRQ mask", "5f693c",
	"Normal IRQ PVR-DMA startup mask",
	"External IRQ PVR-DMA startup mask",
	"5f6948", "5f694c",
	"Normal IRQ G2-DMA startup mask",
	"External IRQ G2-DMA startup mask"
};
#endif

#if DEBUG_MAPLE
static const char *maple_names[] =
{
	"5f6c00",
	"DMA command table addr",
	"5f6c08", "5f6c0c",
	"DMA trigger select",
	"DMA enable",
	"DMA start", "5f6c1c",
	"5f6c20", "5f6c24", "5f6c28", "5f6c2c",
	"5f6c30", "5f6c34", "5f6c38", "5f6c3c",
	"5f6c40", "5f6c44", "5f6c48", "5f6c4c",
	"5f6c50", "5f6c54", "5f6c58", "5f6c5c",
	"5f6c60", "5f6c64", "5f6c68", "5f6c6c",
	"5f6c70", "5f6c74", "5f6c78", "5f6c7c",
	"System control",
	"Status",
	"DMA hard trigger clear",
	"DMA address range",
	"5f6c90", "5f6c94", "5f6c98", "5f6c9c",
	"5f6ca0", "5f6ca4", "5f6ca8", "5f6cac",
	"5f6cb0", "5f6cb4", "5f6cb8", "5f6cbc",
	"5f6cc0", "5f6cc4", "5f6cc8", "5f6ccc",
	"5f6cd0", "5f6cd4", "5f6cd8", "5f6cdc",
	"5f6ce0", "5f6ce4", 
	"MSB select", "5f6cec", "5f6cf0",
	"Txd address counter",
	"Rxd address counter",
	"Rxd base address"

};
#endif

#endif

// selected Maple registers
enum
{
	MAPLE_DMACMD = 1,
	MAPLE_DMATRIGGERSEL = 4,
	MAPLE_DMAENABLE = 5,
	MAPLE_DMASTART = 6,
	MAPLE_SYSCTRL = 0x20,
	MAPLE_STATUS = 0x21,
};

static UINT32 sysctrl_regs[0x200/4];
static UINT32 maple_regs[0x100/4];

// register decode helper

INLINE int decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	if (mem_mask == U64(0x00000000ffffffff))
	{
		reg++;
		*shift = 32;
 	}

	return reg;
}

// I/O functions

READ64_HANDLER( dc_sysctrl_r )
{
	int reg;
	UINT64 shift;
	
	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	mame_printf_verbose("SYSCTRL: read %x @ %x (reg %x: %s), mask %llx (PC=%x)\n", sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, activecpu_get_pc());
	#endif

	return (UINT64)sysctrl_regs[reg] << shift;
}

WRITE64_HANDLER( dc_sysctrl_w )
{
	int reg;
	UINT64 shift;
	
	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	mame_printf_verbose("SYSCTRL: write %llx to %x (reg %x: %s), mask %llx\n", data>>shift, offset, reg, sysctrl_names[reg], mem_mask);
	#endif

	sysctrl_regs[reg] |= data >> shift;
}

READ64_HANDLER( dc_maple_r )
{
	int reg;
	UINT64 shift;
	
	reg = decode_reg_64(offset, mem_mask, &shift);

	return (UINT64)maple_regs[reg] << shift;
}

WRITE64_HANDLER( dc_maple_w )
{
	int reg;
	UINT64 shift;
	
	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_MAPLE
	mame_printf_verbose("MAPLE: write %llx to %x (reg %x: %s), mask %llx\n", data >> shift, offset, reg, maple_names[reg], mem_mask);
	#endif

	maple_regs[reg] |= data >> shift;
}

READ64_HANDLER( dc_gdrom_r )
{
	return 0;
}

WRITE64_HANDLER( dc_gdrom_w )
{
	mame_printf_verbose("GDROM: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_g1_ctrl_r )
{
	return 0;
}

WRITE64_HANDLER( dc_g1_ctrl_w )
{
	mame_printf_verbose("G1CTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_g2_ctrl_r )
{
	return 0;
}

WRITE64_HANDLER( dc_g2_ctrl_w )
{
	mame_printf_verbose("G2CTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_modem_r )
{
	return 0;
}

WRITE64_HANDLER( dc_modem_w )
{
	mame_printf_verbose("MODEM: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_rtc_r )
{
	return 0;
}

WRITE64_HANDLER( dc_rtc_w )
{
	mame_printf_verbose("RTC: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_aica_reg_r )
{
	return 0;
}

WRITE64_HANDLER( dc_aica_reg_w )
{
	mame_printf_verbose("AICA REG: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

MACHINE_RESET( dc )
{
	/* halt the ARM7 */
	cpunum_set_input_line(1, INPUT_LINE_RESET, ASSERT_LINE);

	memset(sysctrl_regs, 0, sizeof(sysctrl_regs));
	memset(maple_regs, 0, sizeof(maple_regs));

	sysctrl_regs[0x27] = 0x00000008;	// Naomi BIOS requires at least this version
}

// called at vblank
void dc_vblank( void )
{
	// is system control set for automatic polling on VBL?
	if ((maple_regs[MAPLE_SYSCTRL] & 0xffff0000) == 0x3a980000)
	{
		// is enabled?
		if (maple_regs[MAPLE_DMAENABLE] == 1)
		{
			// is started?
			if (maple_regs[MAPLE_DMASTART] == 1)
			{
				UINT32 cmd, dest, addr = maple_regs[MAPLE_DMACMD];

				// process the command list
				// first word: bit 31 set = last command in list, 16-17 = port, 8-10 = pattern, 0-7 = xfer length
				// second word: destination address
				// third word: what to send to port A
				cmd = 0;
				while (!(cmd & 0x80000000))
				{
					// read command word
					cmd = program_read_dword(addr);

					dest = program_read_dword(addr+4);

					// just indicate "no connection" for now
					program_write_dword(dest, 0);
					program_write_dword(dest+4, 0xffffffff);

					// skip fixed packet header
					addr += 8;
					// skip transfer data
					addr += ((cmd & 0xff) + 1) * 4;
				}


				#if DEBUG_MAPLE
				mame_printf_verbose("MAPLE: automatic read, table @ %x\n", addr);
				#endif

				// indicate transfer completed
				maple_regs[MAPLE_DMASTART] = 0;
			}
		}
	}
}

