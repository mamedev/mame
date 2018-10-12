// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNES carts

**********************************************************************/

#include "emu.h"
#include "snes_carts.h"

#include "rom.h"
#include "rom21.h"
#include "bsx.h"
#include "sa1.h"
#include "sdd1.h"
#include "sfx.h"
#include "sgb.h"
#include "spc7110.h"
#include "sufami.h"
#include "upd.h"
#include "event.h"


void snes_cart(device_slot_interface &device)
{
	device.option_add_internal("lorom",         SNS_LOROM);
	device.option_add_internal("lorom_bsx",     SNS_LOROM_BSX);         // LoROM + BS-X slot - unsupported
	device.option_add_internal("lorom_cx4",     SNS_LOROM);             // Cart + CX4 - unsupported
	device.option_add_internal("lorom_dsp",     SNS_LOROM_NECDSP);
	device.option_add_internal("lorom_dsp4",    SNS_LOROM_NECDSP);
	device.option_add_internal("lorom_obc1",    SNS_LOROM_OBC1);
	device.option_add_internal("lorom_sa1",     SNS_LOROM_SA1);         // Cart + SA1 - unsupported
	device.option_add_internal("lorom_sdd1",    SNS_LOROM_SDD1);
	device.option_add_internal("lorom_sfx",     SNS_LOROM_SUPERFX);
	device.option_add_internal("lorom_sgb",     SNS_LOROM_SUPERGB);     // SuperGB base cart - unsupported
	device.option_add_internal("lorom_sgb2",    SNS_LOROM_SUPERGB2);    // SuperGB2 base cart - unsupported
	device.option_add_internal("lorom_st010",   SNS_LOROM_SETA10);
	device.option_add_internal("lorom_st011",   SNS_LOROM_SETA11);
	device.option_add_internal("lorom_st018",   SNS_LOROM);             // Cart + ST018 - unsupported
	device.option_add_internal("lorom_sufami",  SNS_LOROM_SUFAMI);      // Sufami Turbo base cart
	device.option_add_internal("hirom",         SNS_HIROM);
	device.option_add_internal("hirom_bsx",     SNS_HIROM_BSX);         // HiROM + BS-X slot - unsupported
	device.option_add_internal("hirom_dsp",     SNS_HIROM_NECDSP);
	device.option_add_internal("hirom_spc7110", SNS_HIROM_SPC7110);
	device.option_add_internal("hirom_spcrtc",  SNS_HIROM_SPC7110_RTC);
	device.option_add_internal("hirom_srtc",    SNS_HIROM_SRTC);
	device.option_add_internal("bsxrom",        SNS_ROM_BSX);           // BS-X base cart - partial support only
	device.option_add_internal("pfest94",       SNS_PFEST94);
	// pirate carts
	device.option_add_internal("lorom_poke",    SNS_LOROM_POKEMON);
	device.option_add_internal("lorom_tekken2", SNS_LOROM_TEKKEN2);
	device.option_add_internal("lorom_sbld",    SNS_LOROM_SOULBLAD);
	device.option_add_internal("lorom_mcpir1",  SNS_LOROM_MCPIR1);
	device.option_add_internal("lorom_mcpir2",  SNS_LOROM_MCPIR2);
	device.option_add_internal("lorom_20col",   SNS_LOROM_20COL);
	device.option_add_internal("lorom_pija",    SNS_LOROM_BANANA);      // not working yet
	device.option_add_internal("lorom_bugs",    SNS_LOROM_BUGSLIFE);    // not working yet
	// legacy slots to support DSPx games from fullpath
	device.option_add_internal("lorom_dsp1leg", SNS_LOROM_NECDSP1_LEG);
	device.option_add_internal("lorom_dsp1bleg",SNS_LOROM_NECDSP1B_LEG);
	device.option_add_internal("lorom_dsp2leg", SNS_LOROM_NECDSP2_LEG);
	device.option_add_internal("lorom_dsp3leg", SNS_LOROM_NECDSP3_LEG);
	device.option_add_internal("lorom_dsp4leg", SNS_LOROM_NECDSP4_LEG);
	device.option_add_internal("hirom_dsp1leg", SNS_HIROM_NECDSP1_LEG);
	device.option_add_internal("lorom_st10leg", SNS_LOROM_SETA10_LEG);
	device.option_add_internal("lorom_st11leg", SNS_LOROM_SETA11_LEG);
}
