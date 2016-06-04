// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/**********************************************************************

**********************************************************************/

#include "bus/neogeo/carts.h"
#include "bus/neogeo/rom.h"
#include "bus/neogeo/fatfury2.h"
#include "bus/neogeo/kof98.h"
#include "bus/neogeo/mslugx.h"
#include "bus/neogeo/cmc.h"
#include "bus/neogeo/sma.h"
#include "bus/neogeo/pcm2.h"
#include "bus/neogeo/kof2k2.h"
#include "bus/neogeo/pvc.h"
#include "bus/neogeo/boot_cthd.h"
#include "bus/neogeo/boot_misc.h"
#include "bus/neogeo/boot_svc.h"
#include "bus/neogeo/boot_kof2k2.h"
#include "bus/neogeo/boot_kof2k3.h"
#include "bus/neogeo/boot_kof10th.h"
#include "bus/neogeo/sbp.h"


SLOT_INTERFACE_START(neogeo_cart)
	SLOT_INTERFACE_INTERNAL("rom",  NEOGEO_ROM)	// Standard cart with banking

	SLOT_INTERFACE_INTERNAL("rom_vliner",  NEOGEO_VLINER_CART)	// Standard cart + RAM
	SLOT_INTERFACE_INTERNAL("rom_fatfur2", NEOGEO_FATFURY2_CART)	// Custom Fatal Fury 2 protection
	SLOT_INTERFACE_INTERNAL("rom_kof98",   NEOGEO_KOF98_CART)	// Custom King of Fighters 98 protection
	SLOT_INTERFACE_INTERNAL("rom_mslugx",  NEOGEO_MSLUGX_CART)	// Custom Metal Slug X protection

	// only CMC42 for gfx
	SLOT_INTERFACE_INTERNAL("cmc42_zupapa",   NEOGEO_CMC_ZUPAPA_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_mslug3h",  NEOGEO_CMC_MSLUG3H_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_ganryu",   NEOGEO_CMC_GANRYU_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_s1945p",   NEOGEO_CMC_S1945P_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_preisle2", NEOGEO_CMC_PREISLE2_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_bangbead", NEOGEO_CMC_BANGBEAD_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_nitd",     NEOGEO_CMC_NITD_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_sengoku3", NEOGEO_CMC_SENGOKU3_CART)
	SLOT_INTERFACE_INTERNAL("cmc42_kof99k",   NEOGEO_CMC_KOF99K_CART)

	// only CMC50 for gfx + audiocpu
	SLOT_INTERFACE_INTERNAL("cmc50_kof2001",  NEOGEO_CMC_KOF2001_CART)
	SLOT_INTERFACE_INTERNAL("cmc50_kof2000n", NEOGEO_CMC_KOF2000N_CART)
	SLOT_INTERFACE_INTERNAL("cmc50_jockeygp", NEOGEO_CMC_JOCKEYGP_CART)	// CMC50 + RAM

	// These use SMA for prg & CMC42 for gfx
	SLOT_INTERFACE_INTERNAL("sma_kof99",  NEOGEO_SMA_KOF99_CART)
	SLOT_INTERFACE_INTERNAL("sma_garou",  NEOGEO_SMA_GAROU_CART)
	SLOT_INTERFACE_INTERNAL("sma_garouh", NEOGEO_SMA_GAROUH_CART)
	SLOT_INTERFACE_INTERNAL("sma_mslug3", NEOGEO_SMA_MSLUG3_CART)
	// These use SMA for prg & CMC50 for gfx + audiocpu
	SLOT_INTERFACE_INTERNAL("sma_kof2k",  NEOGEO_SMA_KOF2000_CART)

	// CMC50 for gfx + audiocpu & NEOPCM2 for YM scramble
	SLOT_INTERFACE_INTERNAL("pcm2_mslug4", NEOGEO_PCM2_MSLUG4_CART)
	SLOT_INTERFACE_INTERNAL("pcm2_rotd",   NEOGEO_PCM2_ROTD_CART)
	SLOT_INTERFACE_INTERNAL("pcm2_pnyaa",  NEOGEO_PCM2_PNYAA_CART)
	SLOT_INTERFACE_INTERNAL("pcm2_ms4p",   NEOGEO_PCM2_MS4PLUS_CART) // regular encryption but external S1 rom = no audiocpu encryption from CMC

	// CMC50 for gfx + audiocpu & NEOPCM2 for YM scramble & additonal prg scramble
	SLOT_INTERFACE_INTERNAL("k2k2_kof2k2", NEOGEO_K2K2_KOF2002_CART)
	SLOT_INTERFACE_INTERNAL("k2k2_matrim", NEOGEO_K2K2_MATRIM_CART)
	SLOT_INTERFACE_INTERNAL("k2k2_samsh5", NEOGEO_K2K2_SAMSHO5_CART)
	SLOT_INTERFACE_INTERNAL("k2k2_sams5s", NEOGEO_K2K2_SAMSHO5SP_CART)
	SLOT_INTERFACE_INTERNAL("k2k2_kf2k2p", NEOGEO_K2K2_KF2K2PLS_CART) // regular encryption but external S1 rom = no audiocpu encryption from CMC

	// CMC50 for gfx + audiocpu & NEOPCM2 for YM scramble & PVC protection/encryption
	SLOT_INTERFACE_INTERNAL("pvc_mslug5", NEOGEO_PVC_MSLUG5_CART)
	SLOT_INTERFACE_INTERNAL("pvc_svc",    NEOGEO_PVC_SVC_CART)
	SLOT_INTERFACE_INTERNAL("pvc_kf2k3",  NEOGEO_PVC_KOF2003_CART)
	SLOT_INTERFACE_INTERNAL("pvc_kf2k3h", NEOGEO_PVC_KOF2003H_CART)

	// Bootleg logic for CTHD2K3 and clones
	SLOT_INTERFACE_INTERNAL("boot_cthd2k3",  NEOGEO_CTHD2K3_CART)
	SLOT_INTERFACE_INTERNAL("boot_ct2k3sp",  NEOGEO_CT2K3SP_CART)
	SLOT_INTERFACE_INTERNAL("boot_ct2k3sa",  NEOGEO_CT2K3SA_CART)
	SLOT_INTERFACE_INTERNAL("boot_matrimbl", NEOGEO_MATRIMBL_CART)	// this also uses a CMC for SFIX & addditional prg scramble from kof2002

	// Bootleg logic for SVC clones
	SLOT_INTERFACE_INTERNAL("boot_svcboot",  NEOGEO_SVCBOOT_CART)	// this also uses a PVC protection/encryption
	SLOT_INTERFACE_INTERNAL("boot_svcplus",  NEOGEO_SVCPLUS_CART)
	SLOT_INTERFACE_INTERNAL("boot_svcplusa", NEOGEO_SVCPLUSA_CART)
	SLOT_INTERFACE_INTERNAL("boot_svcsplus", NEOGEO_SVCSPLUS_CART)	// this also uses a PVC protection/encryption

	// Bootleg logic for KOF2002 clones
	SLOT_INTERFACE_INTERNAL("boot_kf2k2b",   NEOGEO_KOF2002B_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k2mp",  NEOGEO_KF2K2MP_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k2mp2", NEOGEO_KF2K2MP2_CART)

	// Bootleg logic for KOF2003 clones
	SLOT_INTERFACE_INTERNAL("boot_kf2k3bl",  NEOGEO_KF2K3BL_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k3pl",  NEOGEO_KF2K3PL_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k3upl", NEOGEO_KF2K3UPL_CART)

	// Misc carts with bootleg logic
	SLOT_INTERFACE_INTERNAL("boot_garoubl",  NEOGEO_GAROUBL_CART)
	SLOT_INTERFACE_INTERNAL("boot_kof97oro", NEOGEO_KOF97ORO_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf10thep", NEOGEO_KF10THEP_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k5uni", NEOGEO_KF2K5UNI_CART)
	SLOT_INTERFACE_INTERNAL("boot_kf2k4se",  NEOGEO_KF2K4SE_CART)
	SLOT_INTERFACE_INTERNAL("boot_lans2004", NEOGEO_LANS2004_CART)
	SLOT_INTERFACE_INTERNAL("boot_samsho5b", NEOGEO_SAMSHO5B_CART)
	SLOT_INTERFACE_INTERNAL("boot_mslug3b6", NEOGEO_MSLUG3B6_CART)	// this also uses a CMC42 for gfx
	SLOT_INTERFACE_INTERNAL("boot_ms5plus",  NEOGEO_MS5PLUS_CART)	// this also uses a CMC50 for gfx + audiocpu & NEOPCM2 for YM scramble
	SLOT_INTERFACE_INTERNAL("boot_kog",      NEOGEO_KOG_CART)

	SLOT_INTERFACE_INTERNAL("boot_kf10th",   NEOGEO_KOF10TH_CART)
	SLOT_INTERFACE_INTERNAL("boot_sbp",      NEOGEO_SBP_CART)
SLOT_INTERFACE_END
