// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_NES_SLOT_H
#define MAME_BUS_NES_NES_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// uncomment this for *very* verbose logging of most cart accesses
//#define NES_PCB_DEBUG

// Boards
enum
{
	STD_NROM = 0,
	STD_AXROM, STD_AMROM, STD_BXROM,
	STD_CNROM, STD_CPROM,
	STD_EXROM, STD_FXROM, STD_GXROM,
	STD_HKROM, STD_PXROM,
	STD_SXROM, STD_SOROM, STD_SZROM,
	STD_TXROM, STD_TXSROM, STD_TKROM, STD_TQROM,
	STD_UXROM, STD_UN1ROM, UXROM_CC,
	HVC_FAMBASIC, NES_QJ, PAL_ZZ, STD_EVENT, STD_EVENT2,
	STD_DISKSYS,
	STD_NROM368,//homebrew extension of NROM!
	// Discrete components boards (by various manufacturer)
	DIS_74X161X138, DIS_74X139X74,
	DIS_74X377, DIS_74X161X161X32,
	// Active Enterprises
	ACTENT_ACT52,
	// AGCI
	AGCI_50282,
	// AVE
	AVE_NINA01, AVE_NINA06, AVE_MAXI15,
	// Bandai
	BANDAI_FJUMP2, BANDAI_PT554,
	BANDAI_DATACH, BANDAI_KARAOKE, BANDAI_OEKAKIDS,
	BANDAI_FCG, BANDAI_LZ93, BANDAI_LZ93EX1, BANDAI_LZ93EX2,
	// Caltron
	CALTRON_6IN1, CALTRON_9IN1,
	// Camerica
	CAMERICA_BF9093, CAMERICA_BF9096, CAMERICA_ALADDIN,
	CAMERICA_GOLDENFIVE, GG_NROM,
	// Dreamtech
	DREAMTECH_BOARD,
	// Irem
	IREM_G101, IREM_H3001, IREM_LROG017,
	IREM_TAM_S1, IREM_HOLYDIVR,
	// Jaleco
	JALECO_SS88006, JALECO_JF11, JALECO_JF13,
	JALECO_JF16, JALECO_JF17, JALECO_JF17_ADPCM,
	JALECO_JF19, JALECO_JF19_ADPCM, JALECO_JF23,
	JALECO_JF24, JALECO_JF29, JALECO_JF33,
	// Konami
	KONAMI_VRC1, KONAMI_VRC2, KONAMI_VRC3,
	KONAMI_VRC4, KONAMI_VRC6, KONAMI_VRC7,
	// Namcot
	NAMCOT_163, NAMCOT_175, NAMCOT_340,
	NAMCOT_3425, NAMCOT_34X3, NAMCOT_3446,
	// NTDEC
	NTDEC_ASDER, NTDEC_FIGHTINGHERO, NTDEC_N715021,
	// Rex Soft
	REXSOFT_SL1632, REXSOFT_DBZ5,
	// Sachen
	SACHEN_8259A, SACHEN_8259B, SACHEN_8259C, SACHEN_8259D,
	SACHEN_SA009, SACHEN_SA0036, SACHEN_SA0037,
	SACHEN_SA72007, SACHEN_SA72008, SACHEN_SA9602B,
	SACHEN_TCA01, SACHEN_TCU01, SACHEN_TCU02, SACHEN_3013, SACHEN_3014,
	SACHEN_74LS374, SACHEN_74LS374_ALT, SACHEN_SHERO,
	// Sunsoft
	SUNSOFT_1, SUNSOFT_2, SUNSOFT_3, SUNSOFT_4,
	SUNSOFT_DCS, SUNSOFT_5, SUNSOFT_FME7,
	// Taito
	TAITO_TC0190FMC, TAITO_TC0190FMCP,
	TAITO_X1_005, TAITO_X1_017,
	// Tengen
	TENGEN_800008, TENGEN_800032, TENGEN_800037,
	// TXC
	TXC_22110, TXC_22211, TXC_COMMANDOS, TXC_DUMARACING,
	TXC_MJBLOCK, TXC_STRIKEW, TXC_TW,
	// Multigame Carts
	BMC_64IN1NR, BMC_190IN1, BMC_A65AS, BMC_A88S1, BMC_F15, BMC_F600, BMC_L6IN1,
	BMC_GN45, BMC_HIK8IN1, BMC_SFC12, BMC_JY208, BMC_JY302, BMC_KC885,
	BMC_S24IN1SC03, BMC_T262, BMC_TELETUBBIES,
	BMC_WS, BMC_SUPERBIG_7IN1, BMC_SUPERHIK_4IN1, BMC_BALLGAMES_11IN1,
	BMC_MARIOPARTY_7IN1, BMC_GOLD_7IN1, BMC_SUPER_700IN1, BMC_FAMILY_4646,
	BMC_36IN1, BMC_21IN1, BMC_150IN1, BMC_35IN1, BMC_64IN1,
	BMC_8IN1, BMC_15IN1, BMC_SUPERHIK_300IN1, BMC_SUPERGUN_20IN1,
	BMC_72IN1, BMC_SUPER_42IN1, BMC_76IN1,
	BMC_31IN1, BMC_22GAMES, BMC_20IN1, BMC_5IN1_1993,
	BMC_70IN1, BMC_500IN1, BMC_800IN1, BMC_1200IN1,
	BMC_GKA, BMC_GKB, BMC_GKCXIN1, BMC_GN91B,
	BMC_HP898F, BMC_VT5201, BMC_BENSHIENG,
	BMC_CTC09, BMC_CTC_12IN1, BMC_60311C, BMC_80013B, BMC_810544C, BMC_82AB,
	BMC_830425C, BMC_830506C, BMC_830928C, BMC_850437C, BMC_891227, BMC_970630C,
	BMC_N32_4IN1, BMC_NC20MB, BMC_NT639, BMC_NTD_03, BMC_SRPG_5IN1,
	BMC_EL860947C, BMC_EL861121C, BMC_FAM250, BMC_FK23C, BMC_FK23CA,
	BMC_JY012005, BMC_JY820845C, BMC_PJOY84, BMC_TH22913, BMC_11160, BMC_G146,
	BMC_2751, BMC_8157, BMC_00202650,
	BMC_411120C, BMC_810305C, BMC_820720C, BMC_830118C,
	BMC_830832C, BMC_YY841101C, BMC_YY841155C,
	BMC_GOLD150, BMC_GOLD260,
	BMC_12IN1, BMC_4IN1RESET, BMC_42IN1RESET, BMC_LITTLECOM160,
	BMC_K1029, BMC_K3006, BMC_K3033, BMC_K3036, BMC_K3046, BMC_SA005A,
	BMC_TF2740, BMC_TJ03, BMC_RESETSXROM, BMC_RESETTXROM, BMC_TECHLINE9IN1,
	// Unlicensed
	UNL_8237, UNL_8237A, UNL_CC21, UNL_AX40G, UNL_AX5705, UNL_KN42,
	UNL_KOF97, UNL_N625092, UNL_SC127, UNL_SMB2J, UNL_T230, UNL_MMALEE,
	UNL_JY830623C, UNL_XIAOZY, UNL_KOF96, UNL_FS6,
	UNL_SF3, UNL_RACERMATE, UNL_EDU2K,
	UNL_STUDYNGAME, UNL_603_5052, UNL_H2288, UNL_158B, UNL_2708,
	UNL_MALISB, UNL_AC08, UNL_A9746, UNL_43272, UNL_TF1201, UNL_TH21311,
	UNL_BMW8544, UNL_CITYFIGHT, UNL_NINJARYU, UNL_EH8813A, UNL_RT01,
	// Bootleg boards
	BTL_0353, BTL_09034A, BTL_2YUDB, BTL_900218, BTL_AISENSHINICOL,
	BTL_BATMANFS, BTL_CONTRAJ, BTL_DRAGONNINJA, BTL_L001, BTL_MARIOBABY,
	BTL_PALTHENA, BTL_PIKACHUY2K, BTL_SBROS11, BTL_SHUIGUAN,
	BTL_SMB2JA, BTL_SMB2JB, BTL_SMB3, BTL_TOBIDASE, BTL_YUNG08,
	// Shenzhen Jncota
	JNCOTA_KT1001,
	// Kaiser
	KAISER_KS106C,  KAISER_KS202,  KAISER_KS7010,  KAISER_KS7012,
	KAISER_KS7013B, KAISER_KS7016, KAISER_KS7016B, KAISER_KS7017,
	KAISER_KS7021A, KAISER_KS7022, KAISER_KS7030,  KAISER_KS7031,
	KAISER_KS7032,  KAISER_KS7037, KAISER_KS7057,  KAISER_KS7058,
	// Whirlwind Manu
	UNL_DH08, UNL_LE05, UNL_LG25, UNL_LH10, UNL_LH28_LH54,
	UNL_LH31, UNL_LH32, UNL_LH42, UNL_LH51, UNL_LH53,
	// Misc: these are needed to convert mappers to boards, I will sort them later
	OPENCORP_DAOU306, HES_BOARD, SVISION16_BOARD, RUMBLESTATION_BOARD, JYCOMPANY_A, JYCOMPANY_B, JYCOMPANY_C,
	MAGICSERIES_MD, KASING_BOARD, FUTUREMEDIA_BOARD, FUKUTAKE_BOARD, SOMARI_SL12, SOMARI_HUANG2,
	HENGG_SRICH, HENGG_XHZS, HENGG_SHJY3, SUBOR_TYPE0, SUBOR_TYPE1, SUBOR_TYPE2,
	CNE_DECATHLON, CNE_FSB, CNE_SHLZ, COCOMA_BOARD, CONY_BOARD, CONY1K_BOARD, SMD133_BOARD, YOKO_BOARD,
	RCM_GS2015, RCM_GS2004, RCM_GS2013, RCM_TF9IN1, RCM_3DBLOCK,
	WAIXING_TYPE_A, WAIXING_TYPE_A1, WAIXING_TYPE_B, WAIXING_TYPE_C, WAIXING_TYPE_D,
	WAIXING_TYPE_E, WAIXING_TYPE_F, WAIXING_TYPE_G, WAIXING_TYPE_H, WAIXING_TYPE_H1,
	WAIXING_TYPE_I, WAIXING_TYPE_J, WAIXING_FS304,
	WAIXING_SGZLZ, WAIXING_SGZ, WAIXING_WXZS, WAIXING_SECURITY, WAIXING_SH2,
	WAIXING_DQ8, WAIXING_FFV, WAIXING_WXZS2, SUPERGAME_LIONKING, SUPERGAME_BOOGERMAN,
	KAY_BOARD, NITRA_TDA, GOUDER_37017, NANJING_BOARD, ZEMINA_BOARD,
	// homebrew PCBs
	NOCASH_NOCHR,   // homebrew PCB design which uses NTRAM for CHRRAM
	UNL_ACTION53,   // homebrew PCB for homebrew multicarts
	UNL_2A03PURITANS, FARID_SLROM8IN1, FARID_UNROM8IN1,
	// Batlab Electronics
	BATMAP_000, BATMAP_SRRX,
	// Sealie
	SEALIE_8BITXMAS, SEALIE_CUFROM, SEALIE_DPCMCART, SEALIE_UNROM512,
	// FFE boards, for mappers 6, 8, 17
	FFE3_BOARD, FFE4_BOARD, FFE8_BOARD, TEST_BOARD,
	// Unsupported (for place-holder boards, with no working emulation) & no-board (at init)
	UNSUPPORTED_BOARD, UNKNOWN_BOARD, NO_BOARD
};


#define CHRROM 0
#define CHRRAM 1


#define CIRAM 0
#define VROM 1
#define EXRAM 2
#define MMC5FILL 3
#define CART_NTRAM 4


#define PPU_MIRROR_VERT     1
#define PPU_MIRROR_HORZ     2
#define PPU_MIRROR_HIGH     3
#define PPU_MIRROR_LOW      4
#define PPU_MIRROR_4SCREEN  5


// ======================> device_nes_cart_interface

class device_nes_cart_interface : public device_interface
{
public:
	enum class mmc1_type : u8 { MMC1, MMC1A, MMC1B, MMC1C };

	// construction/destruction
	virtual ~device_nes_cart_interface();

	// reading and writing
	virtual uint8_t read_l(offs_t offset);
	virtual uint8_t read_m(offs_t offset);
	virtual uint8_t read_h(offs_t offset) { return 0xff; }
	virtual uint8_t read_ex(offs_t offset) { return 0xff; }
	virtual void write_l(offs_t offset, uint8_t data);
	virtual void write_m(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data);
	virtual void write_ex(offs_t offset, uint8_t data) { }

	virtual uint8_t chr_r(offs_t offset);
	virtual void chr_w(offs_t offset, uint8_t data);
	virtual uint8_t nt_r(offs_t offset);
	virtual void nt_w(offs_t offset, uint8_t data);

	// hack until disk system is made modern!
	virtual void disk_flip_side() { }

	void prg_alloc(size_t size, const char *tag);
	void vrom_alloc(size_t size, const char *tag);
	void misc_rom_alloc(size_t size, const char *tag);
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
	void set_mmc1_type(mmc1_type val) {  m_mmc1_type = val; }
	void set_vrc_lines(int PRG_A, int PRG_B, int CHR) { m_vrc_ls_prg_a = PRG_A; m_vrc_ls_prg_b = PRG_B; m_vrc_ls_chr = CHR; }
	void set_n163_vol(int vol) { m_n163_vol = vol; }
	void set_outer_prg_size(int val) { m_outer_prg_size = val; }
	void set_outer_chr_size(int val) { m_outer_chr_size = val; }
	void set_smd133_addr(int val) {  m_smd133_addr = val; }
	void set_x1_005_alt(bool val) { m_x1_005_alt_mirroring = val; }
	void set_bus_conflict(bool val) { m_bus_conflict = val; }
	uint8_t get_open_bus() { return m_open_bus; }
	void set_open_bus(uint8_t val) { m_open_bus = val; }

	uint8_t *get_prg_base() { return m_prg; }
	uint8_t *get_prgram_base() { return &m_prgram[0]; }
	uint8_t *get_vrom_base() { return m_vrom; }
	uint8_t *get_vram_base() { return &m_vram[0]; }
	uint8_t *get_ciram_base() { return m_ciram; }
	uint8_t *get_battery_base() { return &m_battery[0]; }
	uint8_t *get_mapper_sram_base() { return m_mapper_sram; }
	uint8_t *get_misc_rom_base() { return m_misc_rom; }

	uint32_t get_prg_size() const { return m_prg_size; }
	uint32_t get_prgram_size() const { return m_prgram.size(); }
	uint32_t get_vrom_size() const { return m_vrom_size; }
	uint32_t get_vram_size() const { return m_vram.size(); }
	uint32_t get_battery_size() const { return m_battery.size(); }
	uint32_t get_mapper_sram_size() const { return m_mapper_sram_size; }
	uint32_t get_misc_rom_size() const { return m_misc_rom_size; }

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
	[[deprecated("IRQs should be cleared explicitly")]] void hold_irq_line();
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
	uint8_t *m_misc_rom;
	std::vector<uint8_t> m_ext_ntram;
	uint32_t m_mapper_sram_size;
	uint32_t m_misc_rom_size;

	int m_ce_mask;
	int m_ce_state;
	mmc1_type m_mmc1_type;
	int m_vrc_ls_prg_a;
	int m_vrc_ls_prg_b;
	int m_vrc_ls_chr;
	int m_n163_vol;
	int m_outer_prg_size;
	int m_outer_chr_size;
	int m_smd133_addr;

	int m_mirroring;
	bool m_pcb_ctrl_mirror, m_four_screen_vram, m_has_trainer;
	bool m_x1_005_alt_mirroring;    // temp hack for two kind of mirroring in Taito X1-005 boards (to be replaced with pin checking)
	bool m_bus_conflict;
private:
	uint8_t m_open_bus;

public:
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
	void prg8_x(int start, int bank);
	void prg8_89(int bank) { prg8_x(0, bank); }
	void prg8_ab(int bank) { prg8_x(1, bank); }
	void prg8_cd(int bank) { prg8_x(2, bank); }
	void prg8_ef(int bank) { prg8_x(3, bank); }


	// CHR
	int m_chr_source;   // global source for the 8 VROM banks

	//these were previously called chr_map. they are a quick banking structure,
	//because some of these change multiple times per scanline!
	int m_chr_src[8]; //defines source of base pointer
	int m_chr_orig[8]; //defines offset of 0x400 byte segment at base pointer
	uint8_t *m_chr_access[8];  //source translated + origin -> valid pointer!

	uint32_t m_vrom_chunks;
	uint32_t m_vram_chunks;

	// CHR helpers
private:
	void bank_chr(int shift, int start, int bank, int source);
public:
	void chr8(int bank, int source) { bank_chr(3, 0, bank, source); }
	void chr4_x(int start, int bank, int source) { bank_chr(2, start, bank, source); }
	void chr4_0(int bank, int source) { bank_chr(2, 0, bank, source); }
	void chr4_4(int bank, int source) { bank_chr(2, 4, bank, source); }
	void chr2_x(int start, int bank, int source) { bank_chr(1, start, bank, source); }
	void chr2_0(int bank, int source) { bank_chr(1, 0, bank, source); }
	void chr2_2(int bank, int source) { bank_chr(1, 2, bank, source); }
	void chr2_4(int bank, int source) { bank_chr(1, 4, bank, source); }
	void chr2_6(int bank, int source) { bank_chr(1, 6, bank, source); }
	void chr1_x(int start, int bank, int source) { bank_chr(0, start, bank, source); }
	void chr1_0(int bank, int source) { bank_chr(0, 0, bank, source); }
	void chr1_1(int bank, int source) { bank_chr(0, 1, bank, source); }
	void chr1_2(int bank, int source) { bank_chr(0, 2, bank, source); }
	void chr1_3(int bank, int source) { bank_chr(0, 3, bank, source); }
	void chr1_4(int bank, int source) { bank_chr(0, 4, bank, source); }
	void chr1_5(int bank, int source) { bank_chr(0, 5, bank, source); }
	void chr1_6(int bank, int source) { bank_chr(0, 6, bank, source); }
	void chr1_7(int bank, int source) { bank_chr(0, 7, bank, source); }


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
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_nes_cart_interface>
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

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "nes_cart"; }
	virtual const char *file_extensions() const noexcept override { return "nes,unf,unif"; }
	virtual u32 unhashed_header_length() const noexcept override { return 16; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual uint8_t read_l(offs_t offset);
	virtual uint8_t read_m(offs_t offset);
	virtual uint8_t read_h(offs_t offset);
	virtual uint8_t read_ex(offs_t offset);
	virtual void write_l(offs_t offset, uint8_t data);
	virtual void write_m(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data);
	virtual void write_ex(offs_t offset, uint8_t data);

	// hack until disk system is made modern!
	virtual void disk_flip_side() { if (m_cart) m_cart->disk_flip_side(); }

	int get_pcb_id() { return m_pcb_id; }

	void pcb_start(uint8_t *ciram_ptr);
	void pcb_reset();

	// temporarily here
	int m_crc_hack;

	int get_crc_hack() { return m_crc_hack; }

//private:
	device_nes_cart_interface*      m_cart;
	int m_pcb_id;

protected:
	// device-level overrides
	virtual void device_start() override;

	const char * get_default_card_ines(get_default_card_software_hook &hook, const uint8_t *ROM, uint32_t len) const;
	static const char * get_default_card_unif(const uint8_t *ROM, uint32_t len);
	static const char * nes_get_slot(int pcb_id);
	int nes_get_pcb_id(const char *slot);

	void call_load_ines();
	void call_load_unif();
	void call_load_pcb();
};

// device type definition
DECLARE_DEVICE_TYPE(NES_CART_SLOT, nes_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define NESSLOT_PRGROM_REGION_TAG ":cart:prg_rom"
#define NESSLOT_CHRROM_REGION_TAG ":cart:chr_rom"
#define NESSLOT_MISC_ROM_REGION_TAG ":cart:misc_rom"

#endif // MAME_BUS_NES_NES_SLOT_H
