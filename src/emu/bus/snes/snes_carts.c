// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNES carts

**********************************************************************/

#include "snes_carts.h"

SLOT_INTERFACE_START(snes_cart)
	SLOT_INTERFACE_INTERNAL("lorom",         SNS_LOROM)
	SLOT_INTERFACE_INTERNAL("lorom_bsx",     SNS_LOROM_BSX) // LoROM + BS-X slot - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_cx4",     SNS_LOROM) // Cart + CX4 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_dsp",     SNS_LOROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("lorom_dsp4",    SNS_LOROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("lorom_obc1",    SNS_LOROM_OBC1)
	SLOT_INTERFACE_INTERNAL("lorom_sa1",     SNS_LOROM_SA1) // Cart + SA1 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_sdd1",    SNS_LOROM_SDD1)
	SLOT_INTERFACE_INTERNAL("lorom_sfx",     SNS_LOROM_SUPERFX)
	SLOT_INTERFACE_INTERNAL("lorom_sgb",     SNS_LOROM_SUPERGB) // SuperGB base cart - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_st010",   SNS_LOROM_SETA10)
	SLOT_INTERFACE_INTERNAL("lorom_st011",   SNS_LOROM_SETA11)
	SLOT_INTERFACE_INTERNAL("lorom_st018",   SNS_LOROM) // Cart + ST018 - unsupported
	SLOT_INTERFACE_INTERNAL("lorom_sufami",  SNS_LOROM_SUFAMI)  // Sufami Turbo base cart
	SLOT_INTERFACE_INTERNAL("hirom",         SNS_HIROM)
	SLOT_INTERFACE_INTERNAL("hirom_bsx",     SNS_HIROM_BSX) // HiROM + BS-X slot - unsupported
	SLOT_INTERFACE_INTERNAL("hirom_dsp",     SNS_HIROM_NECDSP)
	SLOT_INTERFACE_INTERNAL("hirom_spc7110", SNS_HIROM_SPC7110)
	SLOT_INTERFACE_INTERNAL("hirom_spcrtc",  SNS_HIROM_SPC7110_RTC)
	SLOT_INTERFACE_INTERNAL("hirom_srtc",    SNS_HIROM_SRTC)
	SLOT_INTERFACE_INTERNAL("bsxrom",        SNS_ROM_BSX)   // BS-X base cart - partial support only
	SLOT_INTERFACE_INTERNAL("pfest94",       SNS_PFEST94)
	// pirate carts
	SLOT_INTERFACE_INTERNAL("lorom_poke",    SNS_LOROM_POKEMON)
	SLOT_INTERFACE_INTERNAL("lorom_tekken2", SNS_LOROM_TEKKEN2)
	SLOT_INTERFACE_INTERNAL("lorom_sbld",    SNS_LOROM_SOULBLAD)
	SLOT_INTERFACE_INTERNAL("lorom_mcpir1",  SNS_LOROM_MCPIR1)
	SLOT_INTERFACE_INTERNAL("lorom_mcpir2",  SNS_LOROM_MCPIR2)
	SLOT_INTERFACE_INTERNAL("lorom_20col",   SNS_LOROM_20COL)
	SLOT_INTERFACE_INTERNAL("lorom_pija",    SNS_LOROM_BANANA)  // not working yet
	SLOT_INTERFACE_INTERNAL("lorom_bugs",    SNS_LOROM_BUGSLIFE)    // not working yet
	// legacy slots to support DSPx games from fullpath
	SLOT_INTERFACE_INTERNAL("lorom_dsp1leg", SNS_LOROM_NECDSP1_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_dsp1bleg",SNS_LOROM_NECDSP1B_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_dsp2leg", SNS_LOROM_NECDSP2_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_dsp3leg", SNS_LOROM_NECDSP3_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_dsp4leg", SNS_LOROM_NECDSP4_LEG)
	SLOT_INTERFACE_INTERNAL("hirom_dsp1leg", SNS_HIROM_NECDSP1_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_st10leg", SNS_LOROM_SETA10_LEG)
	SLOT_INTERFACE_INTERNAL("lorom_st11leg", SNS_LOROM_SETA11_LEG)
SLOT_INTERFACE_END
