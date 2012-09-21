/*****************************************************************************
 *
 * includes/nes.h
 *
 * Nintendo Entertainment System (Famicom)
 *
 ****************************************************************************/

#ifndef NES_H_
#define NES_H_


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NTSC_CLOCK           N2A03_DEFAULTCLOCK     /* 1.789772 MHz */
#define PAL_CLOCK	           (26601712.0/16)        /* 1.662607 MHz */

#define NES_BATTERY_SIZE 0x2000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct nes_input
{
	UINT32 shift;
	UINT32 i0, i1, i2;
};

/*PPU fast banking constants and structures */

#define CHRROM 0
#define CHRRAM 1

struct chr_bank
{
	int source;	//defines source of base pointer
	int origin; //defines offset of 0x400 byte segment at base pointer
	UINT8* access;	//source translated + origin -> valid pointer!
};

/*PPU nametable fast banking constants and structures */

#define CIRAM 0
#define ROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4

#define NES_BATTERY 0
#define NES_WRAM 1

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

	UINT8      *m_wram;
	UINT8      *m_battery_ram;
	UINT8      *m_mapper_ram;
	UINT8      *m_mapper_bram;
	UINT32      m_mapper_ram_size;
	UINT32      m_mapper_bram_size;
	
	// things below are included here for the moment, even if probably would better fit nes_state
	// to allow compilation while progressing with the work

	/* devices */
	cpu_device        *m_maincpu;
	ppu2c0x_device    *m_ppu;
	device_t          *m_sound;
	emu_timer	      *m_irq_timer;

	/* load-time cart variables which remain constant */
	UINT16 m_prg_chunks;		// iNES 2.0 allows for more chunks (a recently dumped multigame cart has 256 chunks of both PRG & CHR!)
	UINT16 m_chr_chunks;
	UINT8 m_battery;			// if there is PRG RAM with battery backup
	
	/* system variables which don't change at run-time */
	UINT16 m_pcb_id;		// for UNIF & xml
	UINT8 m_hard_mirroring;
};

class nes_state : public nes_carts_state
{
public:
	nes_state(const machine_config &mconfig, device_type type, const char *tag)
	: nes_carts_state(mconfig, type, tag) { }

	/* input_related - this part has to be cleaned up (e.g. in_2 and in_3 are not really necessary here...) */
	nes_input m_in_0, m_in_1, m_in_2, m_in_3;
	UINT8 m_fck_scan, m_fck_mode;

	/* video-related */
	int m_nes_vram_sprite[8]; /* Used only by mmc5 for now */
	int m_last_frame_flip;

	void init_nes_core();
	void pcb_handlers_setup();
	int pcb_initialize(int idx);
	
	DECLARE_WRITE8_MEMBER(nes_chr_w);
	DECLARE_READ8_MEMBER(nes_chr_r);
	DECLARE_WRITE8_MEMBER(nes_nt_w);
	DECLARE_READ8_MEMBER(nes_nt_r);
	DECLARE_WRITE8_MEMBER(nes_low_mapper_w);
	DECLARE_READ8_MEMBER(nes_low_mapper_r);

	/* misc */
	write8_delegate   m_mmc_write_low;
	write8_delegate   m_mmc_write_mid;
	write8_delegate   m_mmc_write;
	read8_delegate    m_mmc_read_low;
	read8_delegate    m_mmc_read_mid;
	read8_delegate    m_mmc_read;
	
	/* devices */
//	cpu_device        *m_maincpu;
//	ppu2c0x_device    *m_ppu;
//	device_t          *m_sound;
	device_t          *m_cart;
//	emu_timer	      *m_irq_timer;
	
	/* misc region to be allocated at init */
	// variables which don't change at run-time
	UINT8      *m_rom;
	UINT8      *m_prg;
	UINT8      *m_vrom;
	UINT8      *m_vram;
	UINT8      *m_ciram; //PPU nametable RAM - external to PPU!
	
	
	/* load-time cart variables which remain constant */
//	UINT16 m_prg_chunks;		// iNES 2.0 allows for more chunks (a recently dumped multigame cart has 256 chunks of both PRG & CHR!)
//	UINT16 m_chr_chunks;
	UINT16 m_vram_chunks;
	UINT8 m_trainer;
//	UINT8 m_battery;			// if there is PRG RAM with battery backup
	UINT32 m_battery_size;
	UINT8 m_prg_ram;			// if there is PRG RAM with no backup
	UINT32 m_wram_size;
	
	/* system variables which don't change at run-time */
	UINT16 m_mapper;		// for iNES
//	UINT16 m_pcb_id;		// for UNIF & xml
	UINT8 m_four_screen_vram;
//	UINT8 m_hard_mirroring;
	UINT8 m_crc_hack;	// this is needed to detect different boards sharing the same Mappers (shame on .nes format)
	UINT8 m_ines20;
	
	
	/***** FDS-floppy related *****/

	int     m_disk_expansion;

	UINT8   m_fds_sides;
	UINT8   *m_fds_data;	// here, we store a copy of the disk
	UINT8   *m_fds_ram;	// here, we emulate the RAM adapter

	/* Variables which can change */
	UINT8   m_fds_motor_on;
	UINT8   m_fds_door_closed;
	UINT8   m_fds_current_side;
	UINT32  m_fds_head_position;
	UINT8   m_fds_status0;
	UINT8   m_fds_read_mode;
	UINT8   m_fds_write_reg;

	/* these are used in the mapper 20 handlers */
	int     m_fds_last_side;
	int     m_fds_count;
	DECLARE_READ8_MEMBER(nes_IN0_r);
	DECLARE_READ8_MEMBER(nes_IN1_r);
	DECLARE_WRITE8_MEMBER(nes_IN0_w);
	DECLARE_WRITE8_MEMBER(nes_IN1_w);
	DECLARE_READ8_MEMBER(nes_fds_r);
	DECLARE_WRITE8_MEMBER(nes_fds_w);
	DECLARE_WRITE8_MEMBER(nes_vh_sprite_dma_w);
	DECLARE_DRIVER_INIT(famicom);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_nes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in machine/nes.c -----------*/


/* protos */

DEVICE_IMAGE_LOAD(nes_cart);
DEVICE_START(nes_disk);
DEVICE_IMAGE_LOAD(nes_disk);
DEVICE_IMAGE_UNLOAD(nes_disk);

int nes_ppu_vidaccess( device_t *device, int address, int data );

void nes_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);


#endif /* NES_H_ */
