// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

    ISA cards

**********************************************************************/

#include "emu.h"
#include "isa_cards.h"

// video
#include "aga.h"
#include "cga.h"
#include "ega.h"
#include "eis_hgb107x.h"
#include "ex1280.h"
#include "mda.h"
#include "num9rev.h"
#include "pgc.h"
#include "svga_cirrus.h"
#include "svga_paradise.h"
#include "svga_s3.h"
#include "svga_trident.h"
#include "svga_tseng.h"
#include "vga.h"
#include "vga_ati.h"

// storage (floppy only)
#include "fdc.h"
#include "mufdc.h"

// storage (MFM/RLL/ESDI)
#include "acb2072.h"
#include "cl_sh260.h"
#include "hdc.h"
#include "lrk330.h"
#include "omti8621.h"
#include "ultra12f.h"
#include "wd1002a_wx1.h"
#include "wd1007a.h"
#include "wdxt_gen.h"

// storage (SCSI)
#include "aha1542b.h"
#include "aha1542c.h"
#include "aha174x.h"
#include "asc88.h"
#include "bt54x.h"
#include "dcb.h"
#include "tekram_dc820.h"
#include "ultra14f.h"
#include "ultra24f.h"

// storage (IDE/XT-IDE)
#include "ide.h"
#include "side116.h"
#include "xtide.h"

// storage (miscellaneous)
#include "lbaenhancer.h"
#include "mcd.h"

// sound
#include "adlib.h"
#include "dectalk.h"
#include "gblaster.h"
#include "gus.h"
#include "ibm_mfc.h"
#include "ibm_speech.h"
#include "mpu401.h"
#include "pcmidi.h"
#include "prose4k1.h"
#include "sb16.h"
#include "sblaster.h"
#include "ssi2001.h"
#include "stereo_fx.h"

// network
#include "3c503.h"
#include "3c505.h"
#include "3xtwin.h"
#include "eis_sad8852.h"
#include "eis_twib.h"
#include "ne1000.h"
#include "ne2000.h"
#include "np600.h"

// communication ports
#include "com.h"
#include "lpt.h"
#include "pds.h"

// other
#include "bblue2.h"
#include "chessmdr.h"
#include "chessmsr.h"
#include "finalchs.h"
#include "hpblp.h"
#include "opus100pm.h"


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
	device.option_add("wd90c90_jk", ISA8_WD90C90_JK);
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
	device.option_add("pcmidi", ISA8_PCMIDI);
	device.option_add("ne1000", NE1000);
	device.option_add("3c503", EL2_3C503);
	device.option_add("lpt", ISA8_LPT);
	device.option_add("ibm_mfc", ISA8_IBM_MFC);
	device.option_add("wd1002a_wx1", ISA8_WD1002A_WX1);
	device.option_add("dectalk", ISA8_DECTALK);
	device.option_add("prose4k1", ISA8_PROSE4001);
	device.option_add("pds", ISA8_PDS);
	device.option_add("lba_enhancer", ISA8_LBA_ENHANCER);
	device.option_add("asc88", ASC88);
	device.option_add("chessmdr", ISA8_CHESSMDR);
	device.option_add("chessmsr", ISA8_CHESSMSR);
	device.option_add("finalchs", ISA8_FINALCHS);
	device.option_add("epc_mda", ISA8_EPC_MDA);
	device.option_add("epc_twib", ISA8_EIS_TWIB);
	device.option_add("babyblue2", ISA8_BABYBLUE2);
	device.option_add("acb2072", ACB2072);
	device.option_add("3xtwin", ISA8_3XTWIN);
	device.option_add("opus108pm", ISA8_OPUS108PM);
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
	device.option_add("wd90c90_jk", ISA8_WD90C90_JK);
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
	device.option_add("pcmidi", ISA8_PCMIDI);
	device.option_add("lpt", ISA8_LPT);
	device.option_add("ibm_mfc", ISA8_IBM_MFC);
	device.option_add("fdcsmc", ISA8_FDC_SMC);
	device.option_add("dectalk", ISA8_DECTALK);
	device.option_add("prose4k1", ISA8_PROSE4001);
	device.option_add("pds", ISA8_PDS);
	device.option_add("lba_enhancer", ISA8_LBA_ENHANCER);
	device.option_add("chessmdr", ISA8_CHESSMDR);
	device.option_add("chessmsr", ISA8_CHESSMSR);
	device.option_add("finalchs", ISA8_FINALCHS);
	device.option_add("epc_mda", ISA8_EPC_MDA);
	device.option_add("epc_twib", ISA8_EIS_TWIB);
	device.option_add("babyblue2", ISA8_BABYBLUE2);
	device.option_add("acb2072", ACB2072);
	device.option_add("3xtwin", ISA8_3XTWIN);
	device.option_add("opus108pm", ISA8_OPUS108PM);
	device.option_add("ibm_speech", ISA8_IBM_SPEECH);
	// 16-bit
	device.option_add("ide", ISA16_IDE);
	device.option_add("ne2000", NE2000);
	device.option_add("aha1542a", AHA1542A);
	device.option_add("aha1542b", AHA1542B);
	device.option_add("aha1542c", AHA1542C);
	device.option_add("aha1542cf", AHA1542CF);
	device.option_add("aha1542cp", AHA1542CP);
	device.option_add("aha1740", AHA1740); // actually an EISA card
	device.option_add("aha1742a", AHA1742A); // actually an EISA card
	device.option_add("gus",ISA16_GUS);
	device.option_add("sblaster_16", ISA16_SOUND_BLASTER_16);
	device.option_add("svga_s3", ISA16_SVGA_S3);
	device.option_add("svga_dm",ISA16_SVGA_CIRRUS);
	device.option_add("clgd542x",ISA16_SVGA_CIRRUS_GD542X);
	device.option_add("gfxultra", ISA16_VGA_GFXULTRA);
	device.option_add("gfxultrap", ISA16_SVGA_GFXULTRAPRO);
	device.option_add("tvga9000", ISA16_SVGA_TVGA9000);
//  device.option_add("tgui9680",ISA16_SVGA_TGUI9680);
	device.option_add("pvga1a", ISA16_PVGA1A);
	device.option_add("pvga1a_jk", ISA16_PVGA1A_JK);
	device.option_add("svga_et4k", ISA16_SVGA_ET4K);
	device.option_add("svga_et4k_kasan16", ISA16_SVGA_ET4K_KASAN16);
	device.option_add("svga_et4kw32i", ISA16_SVGA_ET4K_W32I);
	device.option_add("wd90c00_jk", ISA16_WD90C00_JK);
	device.option_add("wd90c11_lr", ISA16_WD90C11_LR);
	device.option_add("wd90c30_lr", ISA16_WD90C30_LR);
	device.option_add("wd90c31_lr", ISA16_WD90C31_LR);
	device.option_add("wd90c31a_lr", ISA16_WD90C31A_LR);
	device.option_add("wd90c31a_zs", ISA16_WD90C31A_ZS);
	device.option_add("wd90c33_zz", ISA16_WD90C33_ZZ);
	device.option_add("3c505", ISA16_3C505);
	device.option_add("mach64", ISA16_SVGA_MACH64);
	device.option_add("sb16_lle", ISA16_SB16);
	device.option_add("mcd", ISA16_MCD);
	device.option_add("sad8852", ISA16_SAD8852);
	device.option_add("np600a3", NP600A3);
	device.option_add("wd1007a", WD1007A);
	device.option_add("ev346", EV346);
	device.option_add("jc1310", JC1310);
	device.option_add("bt542b", BT542B);
	device.option_add("bt542bh", BT542BH);
	device.option_add("bt545s", BT545S);
	device.option_add("dcb", NOVELL_DCB);
	device.option_add("ex1280", ISA16_EX1280);
	device.option_add("ultra12f", ULTRA12F);
	device.option_add("ultra12f32", ULTRA12F32);
	device.option_add("ultra14f", ULTRA14F);
	device.option_add("ultra24f", ULTRA24F); // actually an EISA card
	device.option_add("dc320b", TEKRAM_DC320B); // actually an EISA card
	device.option_add("dc320e", TEKRAM_DC320E); // actually an EISA card
	device.option_add("dc820", TEKRAM_DC820); // actually an EISA card
	device.option_add("dc820b", TEKRAM_DC820B); // actually an EISA card
	device.option_add("omti8621", ISA16_OMTI8621);
	device.option_add("lrk331", LRK331);
	device.option_add("hpblp", HPBLP);
}
