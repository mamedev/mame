/*
    dc.c - Dreamcast video emulation

*/

#include "driver.h"
#include "dc.h"

#define DEBUG_PVRCTRL (1)
#define DEBUG_PVRTA   (1)

static UINT32 pvrctrl_regs[0x100/4];
static UINT32 pvrta_regs[0x2000/4];

VIDEO_START(dc)
{
	memset(pvrctrl_regs, 0, sizeof(pvrctrl_regs));
	memset(pvrta_regs, 0, sizeof(pvrta_regs));

	pvrta_regs[0] = 0x17fd11db;	// vendor and device ID of HOLLY chip
	pvrta_regs[1] = 1;		// chip revision
}

VIDEO_UPDATE(dc)
{
	dc_vblank();

	return 0;
}

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

READ64_HANDLER( pvr_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: read %x @ %x (reg %x), mask %llx (PC=%x)\n", pvrctrl_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
	#endif

	return (UINT64)pvrctrl_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ctrl_w )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: write %llx to %x (reg %x), mask %llx\n", data>>shift, offset, reg, mem_mask);
	#endif

	pvrctrl_regs[reg] |= data >> shift;
}

READ64_HANDLER( pvr_ta_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRTA
	mame_printf_verbose("PVRTA: read %x @ %x (reg %x), mask %llx (PC=%x)\n", pvrta_regs[reg], offset, reg, mem_mask, activecpu_get_pc());
	#endif

	return (UINT64)pvrta_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ta_w )
{
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRTA
	mame_printf_verbose("PVRTA: write %llx to %x (reg %x %x), mask %llx\n", data>>shift, offset, reg, (reg*4)+0x8000, mem_mask);
	#endif

	pvrta_regs[reg] |= data >> shift;
}


WRITE64_HANDLER( ta_fifo_poly_w )
{
	mame_printf_verbose("Poly FIFO: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

WRITE64_HANDLER( ta_fifo_yuv_w )
{
	mame_printf_verbose("YUV FIFO: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}
