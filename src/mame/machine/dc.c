/*

    dc.c - Sega Dreamcast hardware

*/

#include "driver.h"
#include "dc.h"

READ64_HANDLER( dc_sysctrl_r )
{
	return 0;
}

WRITE64_HANDLER( dc_sysctrl_w )
{
	mame_printf_verbose("SYSCTRL: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

READ64_HANDLER( dc_maple_r )
{
	return 0;
}

WRITE64_HANDLER( dc_maple_w )
{
	mame_printf_verbose("MAPLE: write %llx to %x, mask %llx\n", data, offset, mem_mask);
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

MACHINE_RESET( dc )
{
	/* halt the ARM7 */
	cpunum_set_input_line(1, INPUT_LINE_RESET, ASSERT_LINE);
}

READ64_HANDLER( dc_aica_reg_r )
{
	return 0;
}

WRITE64_HANDLER( dc_aica_reg_w )
{
	mame_printf_verbose("AICA REG: write %llx to %x, mask %llx\n", data, offset, mem_mask);
}

