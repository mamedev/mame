// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

    ISA cards

**********************************************************************/

#include "emu.h"
#include "isa_cards.h"

// video
#include "mda.h"
#include "cga.h"
#include "aga.h"
#include "ega.h"
#include "pgc.h"
#include "vga.h"
#include "vga_ati.h"
#include "svga_cirrus.h"
#include "svga_s3.h"
#include "svga_tseng.h"
#include "svga_trident.h"
#include "num9rev.h"

// storage
#include "fdc.h"
#include "mufdc.h"
#include "hdc.h"
#include "wdxt_gen.h"
#include "ide.h"
#include "xtide.h"
#include "side116.h"
#include "aha1542.h"
#include "wd1002a_wx1.h"
#include "wd1007a.h"
#include "mcd.h"
#include "lbaenhancer.h"
#include "cl_sh260.h"

// sound
#include "adlib.h"
#include "gblaster.h"
#include "gus.h"
#include "ibm_mfc.h"
#include "mpu401.h"
#include "sblaster.h"
#include "ssi2001.h"
#include "stereo_fx.h"
#include "dectalk.h"
#include "sb16.h"

// network
#include "3c503.h"
#include "ne1000.h"
#include "ne2000.h"
#include "3c505.h"
#include "eis_sad8852.h"
#include "np600.h"

// communication ports
#include "lpt.h"
#include "com.h"
#include "pds.h"

// other
#include "finalchs.h"


void pc_isa8_cards(device_slot_interface &device)
{
	device.option_add("mda", ISA8_MDA);
	device.option_add("cga", ISA8_CGA);
	device.option_add("cga_ec1841", ISA8_EC1841_0002);
	device.option_add("cga_poisk2", ISA8_CGA_POISK2);
	device.option_add("cga_mc1502", ISA8_CGA_MC1502);
	device.option_add("cga_m24", ISA8_CGA_M24);
	device.option_add("cga_cportiii", ISA8_CGA_CPORTIII);
	device.option_add("aga", ISA8_AGA);
	device.option_add("aga_pc200", ISA8_AGA_PC200);
	device.option_add("ega", ISA8_EGA);
	device.option_add("pgc", ISA8_PGC);
	device.option_add("vga", ISA8_VGA);
	device.option_add("svga_et4k", ISA8_SVGA_ET4K);
	device.option_add("num9rev",ISA8_NUM_9_REV);
	device.option_add("com", ISA8_COM);
	device.option_add("fdc", ISA8_FDC_SUPERIO);
	device.option_add("fdc_xt", ISA8_FDC_XT);
	device.option_add("fdc_at", ISA8_FDC_AT);
	device.option_add("fdc_smc", ISA8_FDC_SMC);
	device.option_add("fdc_ps2", ISA8_FDC_PS2);
	device.option_add("fdc344", ISA8_FDC344);
	device.option_add("fdcmag", ISA8_FDCMAG);
	device.option_add("wdxt_gen", ISA8_WDXT_GEN);
	device.option_add("finalchs", ISA8_FINALCHS);
	device.option_add("xtide", ISA8_XTIDE);
	device.option_add("side116", ISA8_SIDE116);
	device.option_add("hdc", ISA8_HDC);
	device.option_add("adlib", ISA8_ADLIB);
	device.option_add("hercules", ISA8_HERCULES);
	device.option_add("gblaster", ISA8_GAME_BLASTER);
	device.option_add("sblaster1_0", ISA8_SOUND_BLASTER_1_0);
	device.option_add("sblaster1_5", ISA8_SOUND_BLASTER_1_5);
	device.option_add("stereo_fx", ISA8_STEREO_FX);
	device.option_add("ssi2001", ISA8_SSI2001);
	device.option_add("mpu401", ISA8_MPU401);
	device.option_add("ne1000", NE1000);
	device.option_add("3c503", EL2_3C503);
	device.option_add("lpt", ISA8_LPT);
	device.option_add("ibm_mfc", ISA8_IBM_MFC);
	device.option_add("wd1002a_wx1", ISA8_WD1002A_WX1);
	device.option_add("dectalk", ISA8_DECTALK);
	device.option_add("pds", ISA8_PDS);
	device.option_add("lba_enhancer", ISA8_LBA_ENHANCER);
}

void pc_isa16_cards(device_slot_interface &device)
{
	// 8-bit
	device.option_add("mda", ISA8_MDA);
	device.option_add("cga", ISA8_CGA);
	device.option_add("cga_cportiii", ISA8_CGA_CPORTIII);
	device.option_add("wyse700", ISA8_WYSE700);
	device.option_add("ega", ISA8_EGA);
	device.option_add("pgc", ISA8_PGC);
	device.option_add("vga", ISA8_VGA);
	device.option_add("svga_et4k", ISA8_SVGA_ET4K);
	device.option_add("num9rev",ISA8_NUM_9_REV);
	device.option_add("com", ISA8_COM);
	device.option_add("comat", ISA8_COM_AT);
	device.option_add("fdc", ISA8_FDC_AT);
	device.option_add("fdc344", ISA8_FDC344);
	device.option_add("fdcmag", ISA8_FDCMAG);
	device.option_add("hdc", ISA8_HDC);
	device.option_add("side116", ISA8_SIDE116);
	device.option_add("adlib", ISA8_ADLIB);
	device.option_add("hercules", ISA8_HERCULES);
	device.option_add("gblaster", ISA8_GAME_BLASTER);
	device.option_add("sblaster1_0", ISA8_SOUND_BLASTER_1_0);
	device.option_add("sblaster1_5", ISA8_SOUND_BLASTER_1_5);
	device.option_add("stereo_fx", ISA8_STEREO_FX);
	device.option_add("ssi2001", ISA8_SSI2001);
	device.option_add("ne1000", NE1000);
	device.option_add("3c503", EL2_3C503);
	device.option_add("mpu401", ISA8_MPU401);
	device.option_add("lpt", ISA8_LPT);
	device.option_add("ibm_mfc", ISA8_IBM_MFC);
	device.option_add("fdcsmc", ISA8_FDC_SMC);
	device.option_add("dectalk", ISA8_DECTALK);
	device.option_add("pds", ISA8_PDS);
	device.option_add("lba_enhancer", ISA8_LBA_ENHANCER);
	// 16-bit
	device.option_add("ide", ISA16_IDE);
	device.option_add("ne2000", NE2000);
	device.option_add("aha1542", AHA1542);
	device.option_add("gus",ISA16_GUS);
	device.option_add("sblaster_16", ISA16_SOUND_BLASTER_16);
	device.option_add("svga_s3", ISA16_SVGA_S3);
	device.option_add("s3virge", ISA16_S3VIRGE);
	device.option_add("s3virgedx", ISA16_S3VIRGEDX);
	device.option_add("dms3d2kp", ISA16_DMS3D2KPRO);
	device.option_add("svga_dm",ISA16_SVGA_CIRRUS);
	device.option_add("clgd542x",ISA16_SVGA_CIRRUS_GD542X);
	device.option_add("gfxultra", ISA16_VGA_GFXULTRA);
	device.option_add("gfxultrap", ISA16_SVGA_GFXULTRAPRO);
	device.option_add("tgui9680",ISA16_SVGA_TGUI9680);
	device.option_add("3c505", ISA16_3C505);
	device.option_add("mach64", ISA16_SVGA_MACH64);
	device.option_add("sb16_lle", ISA16_SB16);
	device.option_add("mcd", ISA16_MCD);
	device.option_add("sad8852", ISA16_SAD8852);
	device.option_add("np600a3", NP600A3);
	device.option_add("wd1007a", WD1007A);
	device.option_add("ev346", EV346);
	device.option_add("jc1310", JC1310);
}
