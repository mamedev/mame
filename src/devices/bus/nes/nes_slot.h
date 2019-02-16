// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_NES_SLOT_H
#define MAME_BUS_NES_NES_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// uncomment this for *very* verbose logging of most cart accesses
//#define NES_PCB_DEBUG

/* Boards */
enum
{
	STD_NROM = 0,
	STD_AXROM, STD_AMROM, STD_BXROM,
	STD_CNROM, STD_CPROM,
	STD_EXROM, STD_FXROM, STD_GXROM,
	STD_HKROM, STD_PXROM,
	STD_SXROM, STD_TXROM, STD_TXSROM,
	STD_TKROM, STD_TQROM,
	STD_UXROM, STD_UN1ROM, UXROM_CC,
	HVC_FAMBASIC, NES_QJ, PAL_ZZ, STD_EVENT,
	STD_SXROM_A, STD_SOROM, STD_SOROM_A,
	STD_DISKSYS,
	STD_NROM368,//homebrew extension of NROM!
	/* Discrete components boards (by various manufacturer) */
	DIS_74X161X138, DIS_74X139X74,
	DIS_74X377, DIS_74X161X161X32,
	/* Active Enterprises */
	ACTENT_ACT52,
	/* AGCI */
	AGCI_50282,
	/* AVE */
	AVE_NINA01, AVE_NINA06, AVE_MAXI15,
	/* Bandai */
	BANDAI_FJUMP2, BANDAI_PT554,
	BANDAI_DATACH, BANDAI_KARAOKE, BANDAI_OEKAKIDS,
	BANDAI_FCG, BANDAI_LZ93, BANDAI_LZ93EX1, BANDAI_LZ93EX2,
	/* Caltron */
	CALTRON_6IN1,
	/* Camerica */
	CAMERICA_BF9093, CAMERICA_BF9096, CAMERICA_ALADDIN,
	CAMERICA_GOLDENFIVE, GG_NROM,
	/* Dreamtech */
	DREAMTECH_BOARD,
	/* Irem */
	IREM_G101, IREM_H3001, IREM_LROG017,
	IREM_TAM_S1, IREM_HOLYDIVR,
	/* Jaleco */
	JALECO_SS88006, JALECO_JF11, JALECO_JF13,
	JALECO_JF16, JALECO_JF17, JALECO_JF17_ADPCM,
	JALECO_JF19, JALECO_JF19_ADPCM, JALECO_JF23,
	JALECO_JF24, JALECO_JF29, JALECO_JF33,
	/* Konami */
	KONAMI_VRC1, KONAMI_VRC2, KONAMI_VRC3,
	KONAMI_VRC4, KONAMI_VRC6, KONAMI_VRC7,
	/* Namcot */
	NAMCOT_163, NAMCOT_175, NAMCOT_340,
	NAMCOT_3425, NAMCOT_34X3, NAMCOT_3446,
	/* NTDEC */
	NTDEC_ASDER, NTDEC_FIGHTINGHERO,
	/* Rex Soft */
	REXSOFT_SL1632, REXSOFT_DBZ5,
	/* Sachen */
	SACHEN_8259A, SACHEN_8259B, SACHEN_8259C, SACHEN_8259D,
	SACHEN_SA009, SACHEN_SA0036, SACHEN_SA0037,
	SACHEN_SA72007, SACHEN_SA72008, SACHEN_TCA01,
	SACHEN_TCU01, SACHEN_TCU02, SACHEN_SA9602B,
	SACHEN_74LS374, SACHEN_74LS374_ALT, SACHEN_SHERO,
	/* Sunsoft */
	SUNSOFT_1, SUNSOFT_2, SUNSOFT_3, SUNSOFT_4,
	SUNSOFT_DCS, SUNSOFT_5, SUNSOFT_FME7,
	/* Taito */
	TAITO_TC0190FMC, TAITO_TC0190FMCP,
	TAITO_X1_005, TAITO_X1_017,
	/* Tengen */
	TENGEN_800008, TENGEN_800032, TENGEN_800037,
	/* TXC */
	TXC_22211, TXC_DUMARACING, TXC_MJBLOCK,
	TXC_COMMANDOS, TXC_TW, TXC_STRIKEW,
	/* Multigame Carts */
	BMC_64IN1NR, BMC_190IN1, BMC_A65AS,
	BMC_HIK8IN1, BMC_NOVEL1, BMC_NOVEL2, BMC_S24IN1SC03, BMC_T262,
	BMC_WS, BMC_SUPERBIG_7IN1, BMC_SUPERHIK_4IN1, BMC_BALLGAMES_11IN1,
	BMC_MARIOPARTY_7IN1, BMC_GOLD_7IN1, BMC_SUPER_700IN1, BMC_FAMILY_4646,
	BMC_36IN1, BMC_21IN1, BMC_150IN1, BMC_35IN1, BMC_64IN1,
	BMC_15IN1, BMC_SUPERHIK_300IN1, BMC_SUPERGUN_20IN1,
	BMC_GOLDENCARD_6IN1, BMC_72IN1, BMC_SUPER_42IN1, BMC_76IN1,
	BMC_31IN1, BMC_22GAMES, BMC_20IN1, BMC_110IN1,
	BMC_70IN1, BMC_800IN1, BMC_1200IN1,
	BMC_GKA, BMC_GKB, BMC_VT5201, BMC_BENSHIENG, BMC_810544,
	BMC_NTD_03, BMC_G63IN1, BMC_FK23C, BMC_FK23CA, BMC_PJOY84,
	BMC_POWERFUL_255, BMC_11160, BMC_G146, BMC_8157, BMC_830118C,
	BMC_411120C, BMC_GOLD150, BMC_GOLD260, BMC_CH001, BMC_SUPER22,
	BMC_12IN1, BMC_4IN1RESET, BMC_42IN1RESET,
	/* Unlicensed */
	UNL_8237, UNL_CC21, UNL_AX5705, UNL_KOF97, UNL_KS7057,
	UNL_N625092, UNL_SC127, UNL_SMB2J, UNL_T230, UNL_MMALEE,
	UNL_UXROM, UNL_MK2, UNL_XIAOZY, UNL_KOF96,
	UNL_SF3, UNL_RACERMATE, UNL_EDU2K, UNL_LH53, UNL_LH32, UNL_LH10,
	UNL_STUDYNGAME, UNL_603_5052, UNL_H2288, UNL_2708,
	UNL_MALISB, UNL_BB, UNL_AC08, UNL_A9746, UNL_WORLDHERO,
	UNL_43272, UNL_TF1201, UNL_CITYFIGHT, UNL_RT01,
	/* Bootleg boards */
	BTL_SMB2JA, BTL_MARIOBABY, BTL_AISENSHINICOL, BTL_TOBIDASE,
	BTL_SMB2JB, BTL_09034A, BTL_SMB3, BTL_SBROS11, BTL_DRAGONNINJA,
	BTL_PIKACHUY2K, BTL_SHUIGUAN,
	/* Misc: these are needed to convert mappers to boards, I will sort them later */
	OPENCORP_DAOU306, HES_BOARD, SVISION16_BOARD, RUMBLESTATION_BOARD, JYCOMPANY_A, JYCOMPANY_B, JYCOMPANY_C,
	MAGICSERIES_MD, KASING_BOARD, FUTUREMEDIA_BOARD, FUKUTAKE_BOARD, SOMARI_SL12,
	HENGG_SRICH, HENGG_XHZS, HENGG_SHJY3, SUBOR_TYPE0, SUBOR_TYPE1, SUBOR_TYPE2,
	KAISER_KS7058, KAISER_KS7032, KAISER_KS7022, KAISER_KS7017,
	KAISER_KS7012, KAISER_KS7013B, KAISER_KS202, KAISER_KS7031,
	KAISER_KS7016, KAISER_KS7037,
	CNE_DECATHLON, CNE_FSB, CNE_SHLZ, CONY_BOARD, YOKO_BOARD,
	RCM_GS2015, RCM_GS2004, RCM_GS2013, RCM_TF9IN1, RCM_3DBLOCK,
	WAIXING_TYPE_A, WAIXING_TYPE_A1, WAIXING_TYPE_B, WAIXING_TYPE_C, WAIXING_TYPE_D,
	WAIXING_TYPE_E, WAIXING_TYPE_F, WAIXING_TYPE_G, WAIXING_TYPE_H, WAIXING_TYPE_H1,
	WAIXING_TYPE_I, WAIXING_TYPE_J, WAIXING_FS304,
	WAIXING_SGZLZ, WAIXING_SGZ, WAIXING_WXZS, WAIXING_SECURITY, WAIXING_SH2,
	WAIXING_DQ8, WAIXING_FFV, WAIXING_WXZS2, SUPERGAME_LIONKING, SUPERGAME_BOOGERMAN,
	KAY_BOARD, HOSENKAN_BOARD, NITRA_TDA, GOUDER_37017, NANJING_BOARD,
	WHIRLWIND_2706, ZEMINA_BOARD,
	NOCASH_NOCHR,   // homebrew PCB design which uses NTRAM for CHRRAM
	BTL_ACTION53,   // homebrew PCB for homebrew multicarts
	BTL_2A03_PURITANS,   // homebrew PCB
	/* FFE boards, for mappers 6, 8, 17 */
	FFE3_BOARD, FFE4_BOARD, FFE8_BOARD, TEST_BOARD,
	/* Unsupported (for place-holder boards, with no working emulation) & no-board (at init) */
	UNSUPPORTED_BOARD, UNKNOWN_BOARD, NO_BOARD
};


#define CHRROM 0
#define CHRRAM 1


#define CIRAM 0
#define VROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4


#define PPU_MIRROR_NONE     0
#define PPU_MIRROR_VERT     1
#define PPU_MIRROR_HORZ     2
#define PPU_MIRROR_HIGH     3
#define PPU_MIRROR_LOW      4
#define PPU_MIRROR_4SCREEN  5


// ======================> device_nes_cart_interface

class device_nes_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_nes_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_READ8_MEMBER(read_h) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_ex) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_WRITE8_MEMBER(write_ex) { }

	virtual DECLARE_READ8_MEMBER(chr_r);
	virtual DECLARE_WRITE8_MEMBER(chr_w);
	virtual DECLARE_READ8_MEMBER(nt_r);
	virtual DECLARE_WRITE8_MEMBER(nt_w);

	// hack until disk system is made modern!
	virtual void disk_flip_side() { }

	void prg_alloc(size_t size, const char *tag);
	void vrom_alloc(size_t size, const char *tag);
	void prgram_alloc(size_t size);
	void vram_alloc(size_t size);
	void battery_alloc(size_t size);

	int get_mirroring() { return m_mirroring; }
	void set_mirroring(int val) { m_mirroring = val; }
	bool get_pcb_ctrl_mirror() { return m_pcb_ctrl_mirror; }
	void set_pcb_ctrl_mirror(bool val) { m_pcb_ctrl_mirror = val; }
	bool get_four_screen_vram() { return m_four_screen_vram; }
	void set_four_screen_vram(bool val) { m_four_screen_vram = val; }
	bool get_trainer() { return m_has_trainer; }
	void set_trainer(bool val) { m_has_trainer = val; }

	void set_ce(int mask, int state) {  m_ce_mask = mask; m_ce_state = state; }
	void set_vrc_lines(int PRG_A, int PRG_B, int CHR) { m_vrc_ls_prg_a = PRG_A; m_vrc_ls_prg_b = PRG_B; m_vrc_ls_chr = CHR; }
	void set_x1_005_alt(bool val) { m_x1_005_alt_mirroring = val; }
	void set_bus_conflict(bool val) { m_bus_conflict = val; }
	void set_open_bus(uint8_t val) { m_open_bus = val; }

	uint8_t *get_prg_base() { return m_prg; }
	uint8_t *get_prgram_base() { return &m_prgram[0]; }
	uint8_t *get_vrom_base() { return m_vrom; }
	uint8_t *get_vram_base() { return &m_vram[0]; }
	uint8_t *get_ciram_base() { return m_ciram; }
	uint8_t *get_battery_base() { return &m_battery[0]; }
	uint8_t *get_mapper_sram_base() { return m_mapper_sram; }

	uint32_t get_prg_size() const { return m_prg_size; }
	uint32_t get_prgram_size() const { return m_prgram.size(); }
	uint32_t get_vrom_size() const { return m_vrom_size; }
	uint32_t get_vram_size() const { return m_vram.size(); }
	uint32_t get_battery_size() const { return m_battery.size(); }
	uint32_t get_mapper_sram_size() const { return m_mapper_sram_size; }

	virtual void ppu_latch(offs_t offset) {}
	virtual void hblank_irq(int scanline, int vblank, int blanked) {}
	virtual void scanline_irq(int scanline, int vblank, int blanked) {}

	virtual void pcb_reset() {} // many pcb expect specific PRG/CHR banking at start
	virtual void pcb_start(running_machine &machine, uint8_t *ciram_ptr, bool cart_mounted);
	void pcb_reg_postload(running_machine &machine);
	void nes_banks_restore();

	uint8_t hi_access_rom(uint32_t offset);             // helper ROM access for a bunch of PCB reading 0x8000-0xffff for protection too
	uint8_t account_bus_conflict(uint32_t offset, uint8_t data);

protected:
	device_nes_cart_interface(const machine_config &mconfig, device_t &device);

	DECLARE_WRITE_LINE_MEMBER(set_irq_line);
	void hold_irq_line();
	void reset_cpu();

	// internal state
	uint8_t *m_prg;
	uint8_t *m_vrom;
	uint8_t *m_ciram;
	std::vector<uint8_t> m_prgram;
	std::vector<uint8_t> m_vram;
	std::vector<uint8_t> m_battery;
	uint32_t m_prg_size;
	uint32_t m_vrom_size;

private:
	// HACK: to reduce tagmap lookups for PPU-related IRQs, we add a hook to the
	// main NES CPU here, even if it does not belong to this device.
	required_device<cpu_device> m_maincpu;

protected:
	// these are specific of some boards but must be accessible from the driver
	// E.g. additional save ram for HKROM, X1-005 & X1-017 boards, or ExRAM for MMC5
	uint8_t *m_mapper_sram;
	std::vector<uint8_t> m_ext_ntram;
	uint32_t m_mapper_sram_size;

	int m_ce_mask;
	int m_ce_state;
	int m_vrc_ls_prg_a;
	int m_vrc_ls_prg_b;
	int m_vrc_ls_chr;

	int m_mirroring;
	bool m_pcb_ctrl_mirror, m_four_screen_vram, m_has_trainer;
	bool m_x1_005_alt_mirroring;    // temp hack for two kind of mirroring in Taito X1-005 boards (to be replaced with pin checking)
	bool m_bus_conflict;
	uint8_t m_open_bus;

	// PRG
	inline int prg_8k_bank_num(int bank);
	inline void update_prg_banks(int prg_bank_start, int prg_bank_end);
	memory_bank *m_prg_bank_mem[4];
	int m_prg_bank[4];
	uint32_t m_prg_chunks;
	uint32_t m_prg_mask;

	// PRG helpers
	void prg32(int bank);
	void prg16_89ab(int bank);
	void prg16_cdef(int bank);
	void prg8_89(int bank);
	void prg8_ab(int bank);
	void prg8_cd(int bank);
	void prg8_ef(int bank);
	void prg8_x(int start, int bank);


	// CHR
	int m_chr_source;   // global source for the 8 VROM banks
	inline void chr_sanity_check(int source);

	//these were previously called chr_map. they are a quick banking structure,
	//because some of these change multiple times per scanline!
	int m_chr_src[8]; //defines source of base pointer
	int m_chr_orig[8]; //defines offset of 0x400 byte segment at base pointer
	uint8_t *m_chr_access[8];  //source translated + origin -> valid pointer!

	uint32_t m_vrom_chunks;
	uint32_t m_vram_chunks;

	// CHR helpers
	void chr8(int bank, int source);
	void chr4_x(int start, int bank, int source);
	void chr4_0(int bank, int source){ chr4_x(0, bank, source); };
	void chr4_4(int bank, int source){ chr4_x(4, bank, source); };
	void chr2_x(int start, int bank, int source);
	void chr2_0(int bank, int source) { chr2_x(0, bank, source); };
	void chr2_2(int bank, int source) { chr2_x(2, bank, source); };
	void chr2_4(int bank, int source) { chr2_x(4, bank, source); };
	void chr2_6(int bank, int source) { chr2_x(6, bank, source); };
	void chr1_x(int start, int bank, int source);
	void chr1_0(int bank, int source) { chr1_x(0, bank, source); };
	void chr1_1(int bank, int source) { chr1_x(1, bank, source); };
	void chr1_2(int bank, int source) { chr1_x(2, bank, source); };
	void chr1_3(int bank, int source) { chr1_x(3, bank, source); };
	void chr1_4(int bank, int source) { chr1_x(4, bank, source); };
	void chr1_5(int bank, int source) { chr1_x(5, bank, source); };
	void chr1_6(int bank, int source) { chr1_x(6, bank, source); };
	void chr1_7(int bank, int source) { chr1_x(7, bank, source); };


	// NameTable & Mirroring
	//these were previously called nt_page. they are a quick banking structure for a maximum of 4K of RAM/ROM/ExRAM
	int m_nt_src[4];
	int m_nt_orig[4];
	int m_nt_writable[4];
	uint8_t *m_nt_access[4];  //quick banking structure for a maximum of 4K of RAM/ROM/ExRAM

	void set_nt_page(int page, int source, int bank, int writable);
	void set_nt_mirroring(int mirroring);

	std::vector<uint16_t> m_prg_bank_map;
};

// ======================> nes_cart_slot_device

class nes_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: nes_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_cart_slot_device();

	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	void call_load_ines();
	void call_load_unif();
	void call_load_pcb();

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "nes_cart"; }
	virtual const char *file_extensions() const override { return "nes,unf,unif"; }
	virtual u32 unhashed_header_length() const override { return 16; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;
	const char * get_default_card_ines(get_default_card_software_hook &hook, const uint8_t *ROM, uint32_t len) const;
	static const char * get_default_card_unif(const uint8_t *ROM, uint32_t len);
	static const char * nes_get_slot(int pcb_id);
	int nes_get_pcb_id(const char *slot);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_READ8_MEMBER(read_ex);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_WRITE8_MEMBER(write_ex);

	// hack until disk system is made modern!
	virtual void disk_flip_side() { if (m_cart) m_cart->disk_flip_side(); }

	int get_pcb_id() { return m_pcb_id; };

	void pcb_start(uint8_t *ciram_ptr);
	void pcb_reset();

	// temporarily here
	int m_crc_hack;

	int get_crc_hack() { return m_crc_hack; };

	void set_must_be_loaded(bool _must_be_loaded) { m_must_be_loaded = _must_be_loaded; }

//private:
	device_nes_cart_interface*      m_cart;
	int m_pcb_id;
	bool                            m_must_be_loaded;
};

// device type definition
DECLARE_DEVICE_TYPE(NES_CART_SLOT, nes_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define NESSLOT_PRGROM_REGION_TAG ":cart:prg_rom"
#define NESSLOT_CHRROM_REGION_TAG ":cart:chr_rom"

#endif // MAME_BUS_NES_NES_SLOT_H
