// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#include "emu.h"
#include "i8x9xd.h"

i8x9x_disassembler::i8x9x_disassembler() : mcs96_disassembler(disasm_entries)
{
}

std::string i8x9x_disassembler::regname8(uint8_t reg, bool is_dest) const
{
	switch(reg) {
	case 0x02:
		return is_dest ? "ad_command" : "ad_result_lo";

	case 0x03:
		return is_dest ? "hsi_mode" : "ad_result_hi";

	case 0x06:
		return is_dest ? "hso_command" : "hsi_status";

	case 0x07:
		return "sbuf";

	case 0x0a:
		if (is_dest)
			return "watchdog";
		break;

	case 0x0e:
		return is_dest ? "baud_rate" : "port0";

	case 0x0f:
		return "port1";

	case 0x10:
		return "port2";

	case 0x11:
		return is_dest ? "sp_con" : "sp_stat";

	case 0x15:
		return is_dest ? "ioc0" : "ios0";

	case 0x16:
		return is_dest ? "ioc1" : "ios1";

	case 0x17:
		if (is_dest)
			return "pwm_control";
		break;
	}

	return mcs96_disassembler::regname8(reg, is_dest);
}

std::string i8x9x_disassembler::regname16(uint8_t reg, bool is_dest) const
{
	switch(reg) {
	case 0x04:
		return is_dest ? "hso_time" : "hsi_time";

	case 0x0a:
		if (!is_dest)
			return "timer1";
		break;

	case 0x0c:
		if (!is_dest)
			return "timer2";
		break;
	}

	return mcs96_disassembler::regname16(reg, is_dest);
}

#include "cpu/mcs96/i8x9xd.hxx"

