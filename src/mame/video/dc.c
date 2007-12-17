/*
    dc.c - Dreamcast video emulation

*/

#include "driver.h"
#include "dc.h"

VIDEO_START(dc)
{
}

VIDEO_UPDATE(dc)
{
	return 0;
}

READ64_HANDLER( pvr_ctrl_r )
{
	return 0;
}

WRITE64_HANDLER( pvr_ctrl_w )
{
	mame_printf_verbose("PVRCTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( pvr_ta_r )
{
	return 0;
}

WRITE64_HANDLER( pvr_ta_w )
{
	mame_printf_verbose("PVR TA: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}


WRITE64_HANDLER( ta_fifo_poly_w )
{
	mame_printf_verbose("Poly FIFO: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

WRITE64_HANDLER( ta_fifo_yuv_w )
{
	mame_printf_verbose("YUV FIFO: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

