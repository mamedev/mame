// license:BSD-3-Clause
// copyright-holders:A. Lenard
/**********************************************************************

    ZBI expansion cards

**********************************************************************/

#include "emu.h"
#include "s8k_cpu.h"
#include "s8k_ram.h"
#include "s8k_smdc.h"

void zbi_s8k_cpu_cards(device_slot_interface &device)
{
	device.option_add("cpu_a", ZBI_S8K_CPU);
	device.option_add("cpu_a10", ZBI_S8K_CPU10);
	device.option_add("hpcpu", ZBI_S8K_HPCPU);
}

void zbi_s8k_ram_cards(device_slot_interface &device)
{
	device.option_add("parity", ZBI_S8K_PARITY_RAM);
	device.option_add("ecc", ZBI_S8K_ECC_RAM);
}

void zbi_s8k_disk_cards(device_slot_interface &device)
{
	//TODO: WDC
	//TODO: MWDC
	device.option_add("smdc", ZBI_S8K_SMDC);
}

void zbi_s8k_tape_cards(device_slot_interface &device)
{
	//TODO: TCC
	//TODO: MTC (9-Track Tape Controller (2x drives))
}

void zbi_s8k_option1_cards(device_slot_interface &device)
{
	//TODO: SSB(1)
	//TODO: FPP
}

void zbi_s8k_option2_cards(device_slot_interface &device)
{
	//TODO: SSB(2)
	//TODO: ICP
}
