#ifndef __MMC_H
#define __MMC_H

/* Boards */
enum
{
	STD_NROM = 0,
	STD_AXROM, STD_BXROM, STD_CNROM, STD_CPROM,
	STD_DXROM, STD_EXROM, STD_FXROM, STD_GXROM,
	STD_HKROM, STD_JXROM, STD_MXROM, STD_NXROM,
	STD_PXROM, STD_SXROM, STD_TXROM, STD_TXSROM,
	STD_TKROM, STD_TQROM, STD_TVROM,
	STD_UN1ROM, STD_UXROM,
	HVC_FAMBASIC, NES_QJ, PAL_ZZ, UXROM_CC,
	STD_DRROM, STD_SXROM_A, STD_SOROM, STD_SOROM_A,
	/* Discrete components boards (by various manufacturer) */
	DIS_74X161X138, DIS_74X139X74,
	DIS_74X377, DIS_74X161X161X32,
	/* Active Enterprises */
	ACTENT_ACT52,
	/* AGCI */
	AGCI_50282,
	/* AVE */
	AVE_NINA01, AVE_NINA06,
	/* Bandai */
	BANDAI_JUMP2, BANDAI_PT554,
	BANDAI_DATACH, BANDAI_KARAOKE, BANDAI_OEKAKIDS,
	BANDAI_FCG, BANDAI_LZ93, BANDAI_LZ93EX,
	/* Caltron */
	CALTRON_6IN1,
	/* Camerica */
	CAMERICA_BF9093, CAMERICA_BF9097, CAMERICA_BF9096,
	CAMERICA_GOLDENFIVE, GG_NROM,
	/* Dreamtech */
	DREAMTECH_BOARD,
	/* Irem */
	IREM_G101, IREM_H3001, IREM_LROG017,
	IREM_TAM_S1, IREM_HOLYDIV,
	/* Jaleco */
	JALECO_SS88006, JALECO_JF11, JALECO_JF13,
	JALECO_JF16, JALECO_JF17, JALECO_JF19,
	/* Konami */
	KONAMI_VRC1, KONAMI_VRC2, KONAMI_VRC3,
	KONAMI_VRC4, KONAMI_VRC6, KONAMI_VRC7,
	/* Namcot */
	NAMCOT_163, NAMCOT_3453,
	NAMCOT_3425, NAMCOT_34X3, NAMCOT_3446,
	/* NTDEC */
	NTDEC_ASDER, NTDEC_FIGHTINGHERO,
	/* Rex Soft */
	REXSOFT_SL1632, REXSOFT_DBZ5,
	/* Sachen */
	SACHEN_8259A, SACHEN_8259B, SACHEN_8259C, SACHEN_8259D,
	SACHEN_SA009, SACHEN_SA0036, SACHEN_SA0037,
	SACHEN_SA72007, SACHEN_SA72008, SACHEN_TCA01,
	SACHEN_TCU01, SACHEN_TCU02,
	SACHEN_74LS374, SACHEN_74LS374_A,
	/* Sunsoft */
	SUNSOFT_1, SUNSOFT_2, SUNSOFT_3, SUNSOFT_4,
	SUNSOFT_DCS, SUNSOFT_5B, SUNSOFT_FME7,
	/* Taito */
	TAITO_TC0190FMC, TAITO_TC0190FMCP,
	TAITO_X1_005, TAITO_X1_005_A, TAITO_X1_017,
	/* Tengen */
	TENGEN_800008, TENGEN_800032, TENGEN_800037,
	/* TXC */
	TXC_22211A, TXC_22211B, TXC_22211C,
	TXC_MXMDHTWO, TXC_TW, TXC_STRIKEWOLF,
	/* Multigame Carts */
	BMC_64IN1NR, BMC_190IN1, BMC_A65AS, BMC_GS2004, BMC_GS2013,
	BMC_HIK8IN1, BMC_NOVELDIAMOND, BMC_S24IN1SC03, BMC_T262,
	BMC_WS, BMC_SUPERBIG_7IN1, BMC_SUPERHIK_4IN1, BMC_BALLGAMES_11IN1,
	BMC_MARIOPARTY_7IN1, BMC_GOLD_7IN1, BMC_SUPER_700IN1, BMC_FAMILY_4646B,
	BMC_36IN1, BMC_21IN1, BMC_150IN1, BMC_35IN1, BMC_64IN1,
	BMC_15IN1, BMC_SUPERHIK_300IN1, BMC_9999999IN1, BMC_SUPERGUN_20IN1,
	BMC_GOLDENCARD_6IN1, BMC_72IN1, BMC_SUPER_42IN1, BMC_76IN1,
	BMC_1200IN1, BMC_31IN1, BMC_22GAMES, BMC_20IN1, BMC_110IN1,
	BMC_GKA, BMC_GKB, BMC_VT5201, BMC_BENSHENG_BS5, BMC_810544,
	BMC_NTD_03, BMC_G63IN1, BMC_FK23C, BMC_FK23CA, BMC_PJOY84,
	BMC_POWERFUL_255,
	/* Unlicensed */
	UNL_8237, UNL_CC21, UNL_AX5705, UNL_KOF97, UNL_KS7057,
	UNL_N625092, UNL_SC127, UNL_SMB2J, UNL_T230, UNL_FS304,
	UNL_UXROM, UNL_MK2, UNL_XZY, UNL_KOF96,
	UNL_SUPERFIGHTER3, UNL_RACERMATE, UNL_EDU2K,
	UNL_SHJY3, UNL_STUDYNGAME, UNL_603_5052, UNL_H2288,
	/* Bootleg boards */
	BTL_SMB2A, BTL_MARIOBABY, BTL_AISENSHINICOL, BTL_TOBIDASE,
	BTL_SMB2B, BTL_SMB3, BTL_SUPERBROS11, BTL_DRAGONNINJA,
	BTL_PIKACHUY2K,
	/* Misc: these are needed to convert mappers to boards, I will sort them later */
	OPENCORP_DAOU306, HES_BOARD, HES6IN1_BOARD, RUMBLESTATION_BOARD,
	MAGICSERIES_MD, KASING_BOARD, FUTUREMEDIA_BOARD, FUKUTAKE_BOARD, SOMERI_SL12,
	HENGEDIANZI_BOARD, HENGEDIANZI_XJZB, SUBOR_TYPE0, SUBOR_TYPE1,
	KAISER_KS7058, KAISER_KS7032, KAISER_KS7022, KAISER_KS7017, KAISER_KS7012, KAISER_KS202,
	CNE_DECATHLON, CNE_FSB, CNE_SHLZ, CONY_BOARD, YOKO_BOARD, RCM_GS2015, RCM_TETRISFAMILY,
	WAIXING_TYPE_A, WAIXING_TYPE_A_1, WAIXING_TYPE_B, WAIXING_TYPE_C, WAIXING_TYPE_D,
	WAIXING_TYPE_E, WAIXING_TYPE_F, WAIXING_TYPE_G, WAIXING_TYPE_H,
	WAIXING_TYPE_I, WAIXING_TYPE_J,
	WAIXING_SGZLZ, WAIXING_SGZ, WAIXING_ZS, WAIXING_SECURITY, WAIXING_SH2,
	WAIXING_DQ8, WAIXING_FFV, WAIXING_PS2, SUPERGAME_LIONKING, SUPERGAME_BOOGERMAN,
	KAY_PANDAPRINCE, HOSENKAN_BOARD, NITRA_TDA, GOUDER_37017, NANJING_BOARD,
	WHIRLWIND_2706,
	/* FFE boards, for mappers 6, 8, 17 */
	FFE_MAPPER6, FFE_MAPPER8, FFE_MAPPER17,
	/* Unsupported (for place-holder boards, with no working emulation) & no-board (at init) */
	UNSUPPORTED_BOARD, UNKNOWN_BOARD, NO_BOARD
};

// these are used to setup the proper PCB ID, for each supported type of files
int nes_get_pcb_id(running_machine &machine, const char *feature);	// for softlist
void unif_mapr_setup(running_machine &machine, const char *board);	// for UNIF files
int nes_get_mmc_id(running_machine &machine, int mapper);	// for iNES files

// these are used to setup handlers and callbacks necessary to the emulation (resp. at start and reset)
void pcb_handlers_setup(running_machine &machine);
int nes_pcb_reset(running_machine &machine);

//TEMPORARY PPU STUFF

/* mirroring types */
#define PPU_MIRROR_NONE		0
#define PPU_MIRROR_VERT		1
#define PPU_MIRROR_HORZ		2
#define PPU_MIRROR_HIGH		3
#define PPU_MIRROR_LOW		4
#define PPU_MIRROR_4SCREEN	5	// Same effect as NONE, but signals that we should never mirror

void set_nt_mirroring(running_machine &machine, int mirroring);

struct chr_bank
{
	int source;	//defines source of base pointer
	int origin; //defines offset of 0x400 byte segment at base pointer
	UINT8* access;	//source translated + origin -> valid pointer!
};


struct name_table
{
	int source;		/* defines source of base pointer */
	int origin;		/* defines offset of 0x400 byte segment at base pointer */
	int writable;	/* ExRAM, at least, can be write-protected AND used as nametable */
	UINT8* access;	/* direct access when possible */
};

typedef void (*nes_prg_callback)(running_machine &machine, int start, int bank);
typedef void (*nes_chr_callback)(running_machine &machine, int start, int bank, int source);

class nes_carts_state : public driver_device
{
public:
	nes_carts_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag) { }

	int           m_prg_bank[5];
	chr_bank      m_chr_map[8];  //quick banking structure, because some of this changes multiple times per scanline!
	name_table    m_nt_page[4];  //quick banking structure for a maximum of 4K of RAM/ROM/ExRAM

	nes_prg_callback    m_mmc3_prg_cb;	// these are used to simplify a lot emulation of some MMC3 pirate clones
	nes_chr_callback    m_mmc3_chr_cb;

	// mapper variables (to be sorted out at some point, to split more complex mappers as separate devices)
	int m_chr_open_bus;
	int m_prgram_bank5_start, m_battery_bank5_start, m_empty_bank5_start;

	UINT8 m_ce_mask, m_ce_state;
	UINT8 m_vrc_ls_prg_a, m_vrc_ls_prg_b, m_vrc_ls_chr;

	int m_MMC5_floodtile;
	int m_MMC5_floodattr;
	int m_mmc5_vram_control;
	UINT8 m_mmc5_high_chr;
	UINT8 m_mmc5_split_scr;
	UINT8 *m_extended_ntram;

	UINT8 m_mmc5_last_chr_a;
	UINT16 m_mmc5_vrom_regA[8];
	UINT16 m_mmc5_vrom_regB[4];
	UINT8 m_mmc5_prg_regs[4];
	UINT8 m_mmc5_bank_security;
	UINT8 m_mmc5_prg_mode;
	UINT8 m_mmc5_chr_mode;
	UINT8 m_mmc5_chr_high;
	UINT8 m_mmc5_split_ctrl;
	UINT8 m_mmc5_split_yst;
	UINT8 m_mmc5_split_bank;

	/***** Mapper-related variables *****/

	// common ones
	int        m_IRQ_enable, m_IRQ_enable_latch;
	UINT16     m_IRQ_count, m_IRQ_count_latch;
	UINT8      m_IRQ_toggle;
	UINT8      m_IRQ_reset;
	UINT8      m_IRQ_status;
	UINT8      m_IRQ_mode;
	UINT8      m_IRQ_clear;
	int        m_mult1, m_mult2;

	UINT8 m_mmc_chr_source;			// This is set at init to CHRROM or CHRRAM. a few mappers can swap between
	// the two (this is done in the specific handlers).

	UINT8 m_mmc_cmd1, m_mmc_cmd2;		// These represent registers where the mapper writes important values
	UINT8 m_mmc_count;				// This is used as counter in mappers like 1 and 45

	int m_mmc_prg_base, m_mmc_prg_mask;	// MMC3 based multigame carts select a block of banks by using these (and then act like normal MMC3),
	int m_mmc_chr_base, m_mmc_chr_mask;	// while MMC3 and clones (mapper 118 & 119) simply set them as 0 and 0xff resp.

	UINT8 m_mmc_prg_bank[6];				// Many mappers writes only some bits of the selected bank (for both PRG and CHR),
	UINT8 m_mmc_vrom_bank[16];			// hence these are handy to latch bank values.

	UINT16 m_MMC5_vrom_bank[12];			// MMC5 has 10bit wide VROM regs!
	UINT8 m_mmc_extra_bank[16];			// some MMC3 clone have 2 series of PRG/CHR banks...
	// we collect them all here: first 4 elements PRG banks, then 6/8 CHR banks

	UINT8 m_mmc_latch1, m_mmc_latch2;
	UINT8 m_mmc_reg[16];

	UINT8 m_mmc_dipsetting;

	// misc mapper related variables which should be merged with the above one, where possible
	int m_mmc1_reg_write_enable;
	int m_mmc1_latch;
	int m_mmc1_count;

	int m_mmc3_latch;
	int m_mmc3_wram_protect;
	int m_mmc3_alt_irq;

	int m_MMC5_rom_bank_mode;
	int m_MMC5_vrom_bank_mode;
	int m_MMC5_vram_protect;
	int m_MMC5_scanline;
	int m_vrom_page_a;
	int m_vrom_page_b;
	// int vrom_next[4];

	UINT8 m_mmc6_reg;

	// these might be unified in single mmc_reg[] array, together with state->m_mmc_cmd1 & state->m_mmc_cmd2
	// but be careful that MMC3 clones often use state->m_mmc_cmd1/state->m_mmc_cmd2 (from base MMC3) AND additional regs below!
	UINT8 m_mapper83_reg[10];
	UINT8 m_mapper83_low_reg[4];
	UINT8 m_txc_reg[4];	// used by mappers 132, 172 & 173
	UINT8 m_subor_reg[4];	// used by mappers 166 & 167
	UINT8 m_sachen_reg[8];	// used by mappers 137, 138, 139 & 141
	UINT8 m_map52_reg_written;
	UINT8 m_map114_reg, m_map114_reg_enabled;

	// i/o handlers
	DECLARE_WRITE8_MEMBER(mapper6_l_w);
	DECLARE_WRITE8_MEMBER(mapper6_w);
	DECLARE_WRITE8_MEMBER(mapper8_w);
	DECLARE_WRITE8_MEMBER(mapper17_l_w);
	DECLARE_WRITE8_MEMBER(uxrom_w);
	DECLARE_WRITE8_MEMBER(uxrom_cc_w);
	DECLARE_WRITE8_MEMBER(un1rom_w);
	DECLARE_WRITE8_MEMBER(cnrom_w);
	DECLARE_WRITE8_MEMBER(bandai_pt554_m_w);
	DECLARE_WRITE8_MEMBER(cprom_w);
	DECLARE_WRITE8_MEMBER(axrom_w);
	DECLARE_WRITE8_MEMBER(bxrom_w);
	DECLARE_WRITE8_MEMBER(gxrom_w);
	DECLARE_WRITE8_MEMBER(sxrom_w);
	DECLARE_WRITE8_MEMBER(pxrom_w);
	DECLARE_WRITE8_MEMBER(fxrom_w);
	DECLARE_WRITE8_MEMBER(txrom_w);
	DECLARE_WRITE8_MEMBER(hkrom_m_w);
	DECLARE_READ8_MEMBER(hkrom_m_r);
	DECLARE_WRITE8_MEMBER(hkrom_w);
	DECLARE_WRITE8_MEMBER(txsrom_w);
	DECLARE_WRITE8_MEMBER(tqrom_w);
	DECLARE_WRITE8_MEMBER(zz_m_w);
	DECLARE_WRITE8_MEMBER(qj_m_w);
	DECLARE_READ8_MEMBER(exrom_l_r);
	DECLARE_WRITE8_MEMBER(exrom_l_w);
	DECLARE_WRITE8_MEMBER(ntbrom_w);
	DECLARE_WRITE8_MEMBER(jxrom_w);
	DECLARE_WRITE8_MEMBER(dxrom_w);
	DECLARE_WRITE8_MEMBER(namcot3453_w);
	DECLARE_WRITE8_MEMBER(namcot3446_w);
	DECLARE_WRITE8_MEMBER(namcot3425_w);
	DECLARE_WRITE8_MEMBER(dis_74x377_w);
	DECLARE_WRITE8_MEMBER(dis_74x139x74_m_w);
	DECLARE_WRITE8_MEMBER(dis_74x161x138_m_w);
	DECLARE_WRITE8_MEMBER(dis_74x161x161x32_w);
	DECLARE_WRITE8_MEMBER(lz93d50_w);
	DECLARE_WRITE8_MEMBER(lz93d50_m_w);
	DECLARE_WRITE8_MEMBER(fjump2_w);
	DECLARE_WRITE8_MEMBER(bandai_ks_w);
	DECLARE_WRITE8_MEMBER(bandai_ok_w);
	DECLARE_WRITE8_MEMBER(lrog017_w);
	DECLARE_WRITE8_MEMBER(irem_hd_w);
	DECLARE_WRITE8_MEMBER(tam_s1_w);
	DECLARE_WRITE8_MEMBER(g101_w);
	DECLARE_WRITE8_MEMBER(h3001_w);
	DECLARE_WRITE8_MEMBER(ss88006_w);
	DECLARE_WRITE8_MEMBER(jf11_m_w);
	DECLARE_WRITE8_MEMBER(jf13_m_w);
	DECLARE_WRITE8_MEMBER(jf16_w);
	DECLARE_WRITE8_MEMBER(jf17_w);
	DECLARE_WRITE8_MEMBER(jf19_w);
	DECLARE_WRITE8_MEMBER(konami_vrc1_w);
	DECLARE_WRITE8_MEMBER(konami_vrc2_w);
	DECLARE_WRITE8_MEMBER(konami_vrc3_w);
	DECLARE_WRITE8_MEMBER(konami_vrc4_w);
	DECLARE_WRITE8_MEMBER(konami_vrc6_w);
	DECLARE_WRITE8_MEMBER(konami_vrc7_w);
	DECLARE_WRITE8_MEMBER(namcot163_l_w);
	DECLARE_READ8_MEMBER(namcot163_l_r);
	DECLARE_WRITE8_MEMBER(namcot163_w);
	DECLARE_WRITE8_MEMBER(sunsoft1_m_w);
	DECLARE_WRITE8_MEMBER(sunsoft2_w);
	DECLARE_WRITE8_MEMBER(sunsoft3_w);
	DECLARE_WRITE8_MEMBER(tc0190fmc_w);
	DECLARE_WRITE8_MEMBER(tc0190fmc_p16_w);
	DECLARE_WRITE8_MEMBER(x1005_m_w);
	DECLARE_READ8_MEMBER(x1005_m_r);
	DECLARE_WRITE8_MEMBER(x1005a_m_w);
	DECLARE_WRITE8_MEMBER(x1017_m_w);
	DECLARE_READ8_MEMBER(x1017_m_r);
	DECLARE_WRITE8_MEMBER(agci_50282_w);
	DECLARE_WRITE8_MEMBER(nina01_m_w);
	DECLARE_WRITE8_MEMBER(nina06_l_w);
	DECLARE_WRITE8_MEMBER(ae_act52_w);
	DECLARE_WRITE8_MEMBER(cne_decathl_w);
	DECLARE_WRITE8_MEMBER(cne_fsb_m_w);
	DECLARE_WRITE8_MEMBER(cne_shlz_l_w);
	DECLARE_WRITE8_MEMBER(caltron6in1_m_w);
	DECLARE_WRITE8_MEMBER(caltron6in1_w);
	DECLARE_WRITE8_MEMBER(bf9093_w);
	DECLARE_WRITE8_MEMBER(bf9096_w);
	DECLARE_WRITE8_MEMBER(golden5_w);
	DECLARE_WRITE8_MEMBER(cony_l_w);
	DECLARE_READ8_MEMBER(cony_l_r);
	DECLARE_WRITE8_MEMBER(cony_w);
	DECLARE_WRITE8_MEMBER(yoko_l_w);
	DECLARE_READ8_MEMBER(yoko_l_r);
	DECLARE_WRITE8_MEMBER(yoko_w);
	DECLARE_WRITE8_MEMBER(dreamtech_l_w);
	DECLARE_WRITE8_MEMBER(fukutake_l_w);
	DECLARE_READ8_MEMBER(fukutake_l_r);
	DECLARE_WRITE8_MEMBER(futuremedia_w);
	DECLARE_WRITE8_MEMBER(gouder_sf4_l_w);
	DECLARE_READ8_MEMBER(gouder_sf4_l_r);
	DECLARE_WRITE8_MEMBER(henggedianzi_w);
	DECLARE_WRITE8_MEMBER(heng_xjzb_l_w);
	DECLARE_WRITE8_MEMBER(heng_xjzb_w);
	DECLARE_WRITE8_MEMBER(hes6in1_l_w);
	DECLARE_WRITE8_MEMBER(hes_l_w);
	DECLARE_WRITE8_MEMBER(hosenkan_w);
	DECLARE_WRITE8_MEMBER(ks7058_w);
	DECLARE_WRITE8_MEMBER(ks7022_w);
	DECLARE_READ8_MEMBER(ks7022_r);
	DECLARE_WRITE8_MEMBER(ks7032_w);
	DECLARE_WRITE8_MEMBER(ks202_w);
	DECLARE_WRITE8_MEMBER(ks7017_l_w);
	DECLARE_WRITE8_MEMBER(ks7017_extra_w);
	DECLARE_READ8_MEMBER(ks7017_extra_r);
	DECLARE_WRITE8_MEMBER(kay_pp_l_w);
	DECLARE_READ8_MEMBER(kay_pp_l_r);
	DECLARE_WRITE8_MEMBER(kay_pp_w);
	DECLARE_WRITE8_MEMBER(kasing_m_w);
	DECLARE_WRITE8_MEMBER(magics_md_w);
	DECLARE_WRITE8_MEMBER(nanjing_l_w);
	DECLARE_READ8_MEMBER(nanjing_l_r);
	DECLARE_WRITE8_MEMBER(nitra_w);
	DECLARE_WRITE8_MEMBER(ntdec_asder_w);
	DECLARE_WRITE8_MEMBER(ntdec_fh_m_w);
	DECLARE_WRITE8_MEMBER(daou306_w);
	DECLARE_WRITE8_MEMBER(gs2015_w);
	DECLARE_WRITE8_MEMBER(rcm_tf_w);
	DECLARE_WRITE8_MEMBER(rex_dbz_l_w);
	DECLARE_READ8_MEMBER(rex_dbz_l_r);
	DECLARE_WRITE8_MEMBER(rex_sl1632_w);
	DECLARE_WRITE8_MEMBER(rumblestation_m_w);
	DECLARE_WRITE8_MEMBER(rumblestation_w);
	DECLARE_WRITE8_MEMBER(sachen_74x374_l_w);
	DECLARE_READ8_MEMBER(sachen_74x374_l_r);
	DECLARE_WRITE8_MEMBER(sachen_74x374a_l_w);
	DECLARE_WRITE8_MEMBER(s8259_l_w);
	DECLARE_WRITE8_MEMBER(s8259_m_w);
	DECLARE_WRITE8_MEMBER(sa009_l_w);
	DECLARE_WRITE8_MEMBER(sa0036_w);
	DECLARE_WRITE8_MEMBER(sa0037_w);
	DECLARE_WRITE8_MEMBER(sa72007_l_w);
	DECLARE_WRITE8_MEMBER(sa72008_l_w);
	DECLARE_READ8_MEMBER(tca01_l_r);
	DECLARE_WRITE8_MEMBER(tcu01_l_w);
	DECLARE_WRITE8_MEMBER(tcu01_m_w);
	DECLARE_WRITE8_MEMBER(tcu01_w);
	DECLARE_WRITE8_MEMBER(tcu02_l_w);
	DECLARE_READ8_MEMBER(tcu02_l_r);
	DECLARE_WRITE8_MEMBER(subor0_w);
	DECLARE_WRITE8_MEMBER(subor1_w);
	DECLARE_WRITE8_MEMBER(sgame_boog_l_w);
	DECLARE_WRITE8_MEMBER(sgame_boog_m_w);
	DECLARE_WRITE8_MEMBER(sgame_boog_w);
	DECLARE_WRITE8_MEMBER(sgame_lion_m_w);
	DECLARE_WRITE8_MEMBER(sgame_lion_w);
	DECLARE_WRITE8_MEMBER(tengen_800008_w);
	DECLARE_WRITE8_MEMBER(tengen_800032_w);
	DECLARE_WRITE8_MEMBER(tengen_800037_w);
	DECLARE_WRITE8_MEMBER(txc_22211_l_w);
	DECLARE_READ8_MEMBER(txc_22211_l_r);
	DECLARE_WRITE8_MEMBER(txc_22211_w);
	DECLARE_WRITE8_MEMBER(txc_22211b_w);
	DECLARE_READ8_MEMBER(txc_22211c_l_r);
	DECLARE_WRITE8_MEMBER(txc_tw_l_w);
	DECLARE_WRITE8_MEMBER(txc_tw_m_w);
	DECLARE_WRITE8_MEMBER(txc_strikewolf_w);
	DECLARE_READ8_MEMBER(txc_mxmdhtwo_l_r);
	DECLARE_WRITE8_MEMBER(txc_mxmdhtwo_w);
	DECLARE_WRITE8_MEMBER(waixing_a_w);
	DECLARE_WRITE8_MEMBER(waixing_f_w);
	DECLARE_WRITE8_MEMBER(waixing_g_w);
	DECLARE_WRITE8_MEMBER(waixing_h_w);
	DECLARE_WRITE8_MEMBER(waixing_sgz_w);
	DECLARE_WRITE8_MEMBER(waixing_sgzlz_l_w);
	DECLARE_WRITE8_MEMBER(waixing_ffv_l_w);
	DECLARE_WRITE8_MEMBER(waixing_zs_w);
	DECLARE_WRITE8_MEMBER(waixing_dq8_w);
	DECLARE_WRITE8_MEMBER(waixing_ps2_w);
	DECLARE_WRITE8_MEMBER(waixing_sec_l_w);
	DECLARE_READ8_MEMBER(waixing_sh2_chr_r);
	DECLARE_WRITE8_MEMBER(unl_8237_l_w);
	DECLARE_WRITE8_MEMBER(unl_8237_w);
	DECLARE_WRITE8_MEMBER(unl_ax5705_w);
	DECLARE_WRITE8_MEMBER(unl_cc21_w);
	DECLARE_WRITE8_MEMBER(unl_kof97_w);
	DECLARE_WRITE8_MEMBER(ks7057_w);
	DECLARE_WRITE8_MEMBER(unl_t230_w);
	DECLARE_WRITE8_MEMBER(kof96_l_w);
	DECLARE_READ8_MEMBER(kof96_l_r);
	DECLARE_WRITE8_MEMBER(kof96_w);
	DECLARE_WRITE8_MEMBER(mk2_m_w);
	DECLARE_WRITE8_MEMBER(n625092_w);
	DECLARE_WRITE8_MEMBER(sc127_w);
	DECLARE_WRITE8_MEMBER(smb2j_w);
	DECLARE_WRITE8_MEMBER(smb2jb_l_w);
	DECLARE_WRITE8_MEMBER(smb2jb_extra_w);
	DECLARE_WRITE8_MEMBER(unl_sf3_w);
	DECLARE_WRITE8_MEMBER(unl_xzy_l_w);
	DECLARE_WRITE8_MEMBER(unl_racmate_w);
	DECLARE_WRITE8_MEMBER(unl_fs304_l_w);
	DECLARE_WRITE8_MEMBER(btl_smb11_w);
	DECLARE_WRITE8_MEMBER(btl_mariobaby_w);
	DECLARE_WRITE8_MEMBER(btl_smb2a_w);
	DECLARE_WRITE8_MEMBER(whirl2706_w);
	DECLARE_WRITE8_MEMBER(btl_tobi_l_w);
	DECLARE_WRITE8_MEMBER(btl_smb3_w);
	DECLARE_WRITE8_MEMBER(btl_dn_w);
	DECLARE_WRITE8_MEMBER(btl_pika_y2k_w);
	DECLARE_WRITE8_MEMBER(btl_pika_y2k_m_w);
	DECLARE_READ8_MEMBER(btl_pika_y2k_m_r);
	DECLARE_WRITE8_MEMBER(fk23c_l_w);
	DECLARE_WRITE8_MEMBER(fk23c_w);
	DECLARE_WRITE8_MEMBER(bmc_64in1nr_l_w);
	DECLARE_WRITE8_MEMBER(bmc_64in1nr_w);
	DECLARE_WRITE8_MEMBER(bmc_190in1_w);
	DECLARE_WRITE8_MEMBER(bmc_a65as_w);
	DECLARE_WRITE8_MEMBER(bmc_gs2004_w);
	DECLARE_WRITE8_MEMBER(bmc_gs2013_w);
	DECLARE_WRITE8_MEMBER(bmc_s24in1sc03_l_w);
	DECLARE_WRITE8_MEMBER(bmc_t262_w);
	DECLARE_WRITE8_MEMBER(bmc_ws_m_w);
	DECLARE_WRITE8_MEMBER(novel1_w);
	DECLARE_WRITE8_MEMBER(novel2_w);
	DECLARE_WRITE8_MEMBER(bmc_gka_w);
	DECLARE_WRITE8_MEMBER(sng32_w);
	DECLARE_WRITE8_MEMBER(bmc_gkb_w);
	DECLARE_WRITE8_MEMBER(bmc_super700in1_w);
	DECLARE_WRITE8_MEMBER(bmc_36in1_w);
	DECLARE_WRITE8_MEMBER(bmc_21in1_w);
	DECLARE_WRITE8_MEMBER(bmc_150in1_w);
	DECLARE_WRITE8_MEMBER(bmc_35in1_w);
	DECLARE_WRITE8_MEMBER(bmc_64in1_w);
	DECLARE_WRITE8_MEMBER(bmc_15in1_m_w);
	DECLARE_WRITE8_MEMBER(bmc_hik300_w);
	DECLARE_WRITE8_MEMBER(supergun20in1_w);
	DECLARE_WRITE8_MEMBER(bmc_72in1_w);
	DECLARE_WRITE8_MEMBER(bmc_76in1_w);
	DECLARE_WRITE8_MEMBER(bmc_1200in1_w);
	DECLARE_WRITE8_MEMBER(bmc_31in1_w);
	DECLARE_WRITE8_MEMBER(bmc_22g_w);
	DECLARE_WRITE8_MEMBER(bmc_20in1_w);
	DECLARE_WRITE8_MEMBER(bmc_110in1_w);
	DECLARE_WRITE8_MEMBER(bmc_sbig7_w);
	DECLARE_WRITE8_MEMBER(bmc_hik8_m_w);
	DECLARE_WRITE8_MEMBER(bmc_hik4in1_m_w);
	DECLARE_WRITE8_MEMBER(bmc_ball11_m_w);
	DECLARE_WRITE8_MEMBER(bmc_ball11_w);
	DECLARE_WRITE8_MEMBER(bmc_mario7in1_m_w);
	DECLARE_WRITE8_MEMBER(bmc_gold7in1_m_w);
	DECLARE_WRITE8_MEMBER(bmc_gc6in1_l_w);
	DECLARE_WRITE8_MEMBER(bmc_gc6in1_w);
	DECLARE_WRITE8_MEMBER(bmc_family4646_m_w);
	DECLARE_WRITE8_MEMBER(bmc_vt5201_w);
	DECLARE_READ8_MEMBER(bmc_vt5201_r);
	DECLARE_WRITE8_MEMBER(bmc_bs5_w);
	DECLARE_WRITE8_MEMBER(bmc_810544_w);
	DECLARE_WRITE8_MEMBER(bmc_ntd03_w);
	DECLARE_WRITE8_MEMBER(bmc_gb63_w);
	DECLARE_READ8_MEMBER(bmc_gb63_r);
	DECLARE_WRITE8_MEMBER(edu2k_w);
	DECLARE_WRITE8_MEMBER(h2288_l_w);
	DECLARE_READ8_MEMBER(h2288_l_r);
	DECLARE_WRITE8_MEMBER(h2288_w);
	DECLARE_WRITE8_MEMBER(shjy3_w);
	DECLARE_WRITE8_MEMBER(unl_6035052_extra_w);
	DECLARE_READ8_MEMBER(unl_6035052_extra_r);
	DECLARE_WRITE8_MEMBER(pjoy84_m_w);
	DECLARE_WRITE8_MEMBER(someri_mmc1_w);
	DECLARE_WRITE8_MEMBER(someri_mmc3_w);
	DECLARE_WRITE8_MEMBER(someri_vrc2_w);
	DECLARE_WRITE8_MEMBER(someri_w);
	DECLARE_WRITE8_MEMBER(someri_l_w);
	DECLARE_WRITE8_MEMBER(fujiya_m_w);
	DECLARE_READ8_MEMBER(fujiya_m_r);
	DECLARE_WRITE8_MEMBER(dummy_l_w);
	DECLARE_WRITE8_MEMBER(dummy_m_w);
	DECLARE_WRITE8_MEMBER(dummy_w);
	DECLARE_READ8_MEMBER(dummy_l_r);
	DECLARE_READ8_MEMBER(dummy_m_r);
	DECLARE_READ8_MEMBER(dummy_r);

	/* misc region to be allocated at init */
	// variables which don't change at run-time
	UINT8      *m_rom;
	UINT8      *m_prg;
	UINT8      *m_vrom;
	UINT8      *m_vram;
	UINT8      *m_ciram; //PPU nametable RAM - external to PPU!

	UINT8      *m_wram;
	UINT8      *m_battery_ram;
	UINT8      *m_mapper_ram;
	UINT8      *m_mapper_bram;
	UINT32      m_mapper_ram_size;
	UINT32      m_mapper_bram_size;

	/* load-time cart variables which remain constant */
	UINT16 m_prg_chunks;		// iNES 2.0 allows for more chunks (a recently dumped multigame cart has 256 chunks of both PRG & CHR!)
	UINT16 m_chr_chunks;
	UINT16 m_vram_chunks;
	UINT8 m_trainer;
	UINT8 m_battery;			// if there is PRG RAM with battery backup
	UINT32 m_battery_size;
	UINT8 m_prg_ram;			// if there is PRG RAM with no backup
	UINT32 m_wram_size;

	/* system variables which don't change at run-time */
	UINT16 m_mapper;		// for iNES
	UINT16 m_pcb_id;		// for UNIF & xml
	UINT8 m_four_screen_vram;
	UINT8 m_hard_mirroring;
	UINT8 m_crc_hack;	// this is needed to detect different boards sharing the same Mappers (shame on .nes format)
	UINT8 m_ines20;

	// things below are included here for the moment, even if probably would better fit nes_state
	// to allow compilation while progressing with the work

	/* devices */
	cpu_device        *m_maincpu;
	ppu2c0x_device    *m_ppu;
	device_t          *m_sound;
	emu_timer	      *m_irq_timer;

//private:

	// banking helpers
	// WRAM bankswitch
	void wram_bank(int bank, int source);
	// PRG bankswitch
	void prg32(int bank);
	void prg16_89ab(int bank);
	void prg16_cdef(int bank);
	void prg8_89(int bank);
	void prg8_ab(int bank);
	void prg8_cd(int bank);
	void prg8_ef(int bank);
	void prg8_67(int bank);	// a bunch of pcbs can bank ROM in WRAM area!
	void prg8_x(int start, int bank);
	// CHR 8k bankswitch
	void chr8(int bank, int source);
	// CHR 4k bankswitch
	void chr4_x(int start, int bank, int source);
	void chr4_0(int bank, int source){ chr4_x(0, bank, source); };
	void chr4_4(int bank, int source){ chr4_x(4, bank, source); };
	// CHR 2k bankswitch
	void chr2_x(int start, int bank, int source);
	void chr2_0(int bank, int source) { chr2_x(0, bank, source); };
	void chr2_2(int bank, int source) { chr2_x(2, bank, source); };
	void chr2_4(int bank, int source) { chr2_x(4, bank, source); };
	void chr2_6(int bank, int source) { chr2_x(6, bank, source); };
	// CHR 1k bankswitch
	void chr1_x(int start, int bank, int source);
	void chr1_0(int bank, int source) { chr1_x(0, bank, source); };
	void chr1_1(int bank, int source) { chr1_x(1, bank, source); };
	void chr1_2(int bank, int source) { chr1_x(2, bank, source); };
	void chr1_3(int bank, int source) { chr1_x(3, bank, source); };
	void chr1_4(int bank, int source) { chr1_x(4, bank, source); };
	void chr1_5(int bank, int source) { chr1_x(5, bank, source); };
	void chr1_6(int bank, int source) { chr1_x(6, bank, source); };
	void chr1_7(int bank, int source) { chr1_x(7, bank, source); };
	// NT bankswitch
	void set_nt_page(int page, int source, int bank, int writable);
	void set_nt_mirroring(int mirroring);

};



#endif
