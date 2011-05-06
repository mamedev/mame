/*****************************************************************************************

    NES MMC Emulation

    Support for iNES Mappers

****************************************************************************************/

typedef struct _nes_mmc  nes_mmc;
struct _nes_mmc
{
	int    iNesMapper; /* iNES Mapper # */
	int    pcb_id;
};

/*************************************************************

    Mapper 6

    Known Boards: FFE4 Copier Board
    Games: Hacked versions of games

    In MESS: Supported? Not sure if we could also have ExRAM or not...
       However, priority is pretty low for this mapper.

*************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void ffe_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	/* 114 is the number of cycles per scanline */
	/* TODO: change to reflect the actual number of cycles spent */
	if (state->m_IRQ_enable)
	{
		if ((0xffff - state->m_IRQ_count) < 114)
		{
			device_set_input_line(state->m_maincpu, M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_count = 0xffff;
			state->m_IRQ_enable = 0;
		}
		state->m_IRQ_count -= 114;
	}
}

static WRITE8_HANDLER( mapper6_l_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	LOG_MMC(("mapper6_l_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1fe:
			state->m_mmc_latch1 = data & 0x80;
			set_nt_mirroring(space->machine(), BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x1ff:
			set_nt_mirroring(space->machine(), BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x401:
			state->m_IRQ_enable = data & 0x01;
			break;
		case 0x402:
			state->m_IRQ_count = (state->m_IRQ_count & 0xff00) | data;
			break;
		case 0x403:
			state->m_IRQ_enable = 1;
			state->m_IRQ_count = (state->m_IRQ_count & 0x00ff) | (data << 8);
			break;
	}
}

static WRITE8_HANDLER( mapper6_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	LOG_MMC(("mapper6_w, offset: %04x, data: %02x\n", offset, data));

	if (!state->m_mmc_latch1)	// when in "FFE mode" we are forced to use CHRRAM/EXRAM bank?
	{
		prg16_89ab(space->machine(), data >> 2);
		// chr8(space->machine(), data & 0x03, ???);
		// due to lack of info on the exact behavior, we simply act as if mmc_latch1=1
		if (state->m_mmc_chr_source == CHRROM)
			chr8(space->machine(), data & 0x03, CHRROM);
	}
	else if (state->m_mmc_chr_source == CHRROM)			// otherwise, we can use CHRROM (when present)
		chr8(space->machine(), data, CHRROM);
}

/*************************************************************

    Mapper 8

    Known Boards: FFE3 Copier Board
    Games: Hacked versions of games

    In MESS: Supported? (I have no games to test this)

*************************************************************/

static WRITE8_HANDLER( mapper8_w )
{
	LOG_MMC(("mapper8_w, offset: %04x, data: %02x\n", offset, data));

	chr8(space->machine(), data & 0x07, CHRROM);
	prg16_89ab(space->machine(), data >> 3);
}

/*************************************************************

    Mapper 17

    Known Boards: FFE8 Copier Board
    Games: Hacked versions of games

    In MESS: Supported?. IRQ support is just a guess (used to
      use MMC3 IRQ but it was wrong and it never enabled it)

*************************************************************/

static WRITE8_HANDLER( mapper17_l_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	LOG_MMC(("mapper17_l_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1fe:
			set_nt_mirroring(space->machine(), BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x1ff:
			set_nt_mirroring(space->machine(), BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x401:
			state->m_IRQ_enable = data & 0x01;
			break;
		case 0x402:
			state->m_IRQ_count = (state->m_IRQ_count & 0xff00) | data;
			break;
		case 0x403:
			state->m_IRQ_enable = 1;
			state->m_IRQ_count = (state->m_IRQ_count & 0x00ff) | (data << 8);
			break;

		case 0x404:
			prg8_89(space->machine(), data);
			break;
		case 0x405:
			prg8_ab(space->machine(), data);
			break;
		case 0x406:
			prg8_cd(space->machine(), data);
			break;
		case 0x407:
			prg8_ef(space->machine(), data);
			break;

		case 0x410:
		case 0x411:
		case 0x412:
		case 0x413:
		case 0x414:
		case 0x415:
		case 0x416:
		case 0x417:
			chr1_x(space->machine(), offset & 7, data, CHRROM);
			break;
	}
}

/*************************************************************

    mmc_list

    Supported mappers and corresponding pcb id

*************************************************************/

static const nes_mmc mmc_list[] =
{
/*  INES   DESC                          LOW_W, LOW_R, MED_W, HIGH_W, PPU_latch, scanline CB, hblank CB */
	{  0, STD_NROM },
	{  1, STD_SXROM },
	{  2, STD_UXROM },
	{  3, STD_CNROM },
	{  4, STD_TXROM },
	{  5, STD_EXROM },
	{  6, FFE_MAPPER6 },
	{  7, STD_AXROM },
	{  8, FFE_MAPPER8 },
	{  9, STD_PXROM },
	{ 10, STD_FXROM },
	{ 11, DIS_74X377 },
	{ 12, REXSOFT_DBZ5 },
	{ 13, STD_CPROM },
	{ 14, REXSOFT_SL1632 },
	{ 15, WAIXING_PS2 },
	{ 16, BANDAI_LZ93EX },	// with 24c02
	{ 17, FFE_MAPPER17 },
	{ 18, JALECO_SS88006 },
	{ 19, NAMCOT_163 },
	{ 21, KONAMI_VRC4 },
	{ 22, KONAMI_VRC2 },
	{ 23, KONAMI_VRC2 },
	{ 24, KONAMI_VRC6 },
	{ 25, KONAMI_VRC4 },
	{ 26, KONAMI_VRC6 },
	// 27 World Hero board - Unsupported
	// 28 Unused
	// 29 Unused
	// 30 Unused
	// 31 Unused
	{ 32, IREM_G101 },
	{ 33, TAITO_TC0190FMC },
	{ 34, STD_BXROM },
	{ 35, UNL_SC127 },
	{ 36, TXC_STRIKEWOLF },
	{ 37, PAL_ZZ },
	{ 38, DIS_74X161X138 },
	{ 39, UNL_STUDYNGAME },
	{ 40, BTL_SMB2A },
	{ 41, CALTRON_6IN1 },
	{ 42, BTL_MARIOBABY },	// ai senshi nicol?
	{ 43, UNL_SMB2J },
	{ 44, BMC_SUPERBIG_7IN1 },
	{ 45, BMC_HIK8IN1 },
	{ 46, RUMBLESTATION_BOARD },
	{ 47, NES_QJ },
	{ 48, TAITO_TC0190FMCP },
	{ 49, BMC_SUPERHIK_4IN1 },
	{ 50, BTL_SMB2B },
	{ 51, BMC_BALLGAMES_11IN1 },
	{ 52, BMC_MARIOPARTY_7IN1 },
	// 53 Supervision 16-in-1 - Unsupported
	{ 54, BMC_NOVELDIAMOND },
	// 55 Genius SMB - No info (nor images) available
	{ 56, KAISER_KS202 },
	{ 57, BMC_GKA },
	{ 58, BMC_GKB },
	// 59 Unused
	// 60 4-in-1, 35-in-1 Reset based
	{ 61, RCM_TETRISFAMILY },
	{ 62, BMC_SUPER_700IN1 },
	// 63 CH001 X-in-1 - No info (nor images) available
	{ 64, TENGEN_800032 },
	{ 65, IREM_H3001 },
	{ 66, STD_GXROM },
	{ 67, SUNSOFT_3 },
	{ 68, STD_NXROM },
	{ 69, STD_JXROM },
	{ 70, DIS_74X161X161X32 },
	{ 71, CAMERICA_BF9093 },
	{ 72, JALECO_JF17 },
	{ 73, KONAMI_VRC3 },
	{ 74, WAIXING_TYPE_A },
	{ 75, KONAMI_VRC1 },
	{ 76, NAMCOT_3446 },
	{ 77, IREM_LROG017 },
	{ 78, IREM_HOLYDIV },
	{ 79, AVE_NINA06 },
	{ 80, TAITO_X1_005 },
	// 81 Unused
	{ 82, TAITO_X1_017 },
	{ 83, CONY_BOARD },
	// 84 Pasofami hacked images?
	{ 85, KONAMI_VRC7 },
	{ 86, JALECO_JF13 },
	{ 87, DIS_74X139X74 },
	{ 88, NAMCOT_34X3 },
	{ 89, SUNSOFT_2 },
	// 90 JY Company Type A (Aladdin, Final Fight 3, Super Mario World, Tekken 2) - Unsupported
	{ 91, UNL_MK2 },
	{ 92, JALECO_JF19 },
	{ 93, SUNSOFT_2 },
	{ 94, STD_UN1ROM },
	{ 95, NAMCOT_3425 },
	{ 96, BANDAI_OEKAKIDS },
	{ 97, IREM_TAM_S1 },
	// 98 Unused
	// 99 VS. system - Not going to be implemented (use MAME instead)
	// 100 images hacked to work with nesticle?
	// 101 Unused (Urusei Yatsura had been assigned to this mapper, but it's Mapper 87)
	// 102 Unused
	// 103 Bootleg cart 2708 (Doki Doki Panic - FDS Conversion) - Unsupported
	{ 104, CAMERICA_GOLDENFIVE },
	// 105 Nintendo World Championship - Unsupported
	{ 106, BTL_SMB3 },
	{ 107, MAGICSERIES_MD },
	{ 108, WHIRLWIND_2706 },
	// 109 Unused
	// 110 Unused
	// 111 Ninja Ryuukenden Chinese? - Unsupported
	{ 112, NTDEC_ASDER },
	{ 113, HES_BOARD },
	{ 114, SUPERGAME_LIONKING },
	{ 115, KASING_BOARD },
	{ 116, SOMERI_SL12 },
	{ 117, FUTUREMEDIA_BOARD },
	{ 118, STD_TXSROM },
	{ 119, STD_TQROM },
	{ 120, BTL_TOBIDASE },
	{ 121, KAY_PANDAPRINCE },
	// 122 Unused
	{ 123, UNL_H2288 },
	// 124 Unused
	// 125 Unused
	{ 126, BMC_PJOY84 },
	// 127 Unused
	// 128 Unused
	// 129 Unused
	// 130 Unused
	// 131 Unused
	{ 132, TXC_22211B },
	{ 133, SACHEN_SA72008 },
	{ 134, BMC_FAMILY_4646B },
	// 135 Unused
	{ 136, SACHEN_TCU02 },
	{ 137, SACHEN_8259D },
	{ 138, SACHEN_8259B },
	{ 139, SACHEN_8259C },
	{ 140, JALECO_JF11 },
	{ 141, SACHEN_8259A },
	{ 142, KAISER_KS7032},
	{ 143, SACHEN_TCA01 },
	{ 144, AGCI_50282 },
	{ 145, SACHEN_SA72007 },
	{ 146, AVE_NINA06 }, // basically same as Mapper 79 (Nina006)
	{ 147, SACHEN_TCU01 },
	{ 148, SACHEN_SA0037 },
	{ 149, SACHEN_SA0036 },
	{ 150, SACHEN_74LS374 },
	// 151 VS. system by Konami - Not going to be implemented (use MAME instead)
	{ 152, DIS_74X161X161X32 },
	{ 153, BANDAI_LZ93 },
	{ 154, NAMCOT_3453 },
	{ 155, STD_SXROM_A }, // diff compared to MMC1 concern WRAM
	{ 156, OPENCORP_DAOU306 },
	{ 157, BANDAI_DATACH },	// no Datach Reader -> we fall back to mapper 16
	{ 158, TENGEN_800037 },
	{ 159, BANDAI_LZ93EX },	// with 24c01
	{ 160, SACHEN_SA009 },
	// 161 Unused
	// 162 Unused
	{ 163, NANJING_BOARD},
	{ 164, WAIXING_FFV },
	{ 165, WAIXING_SH2 },
	{ 166, SUBOR_TYPE1 },
	{ 167, SUBOR_TYPE0 },
	{ 168, UNL_RACERMATE },
	// 169 Unused
	// 170 Fujiya
	{ 171, KAISER_KS7058 },
	{ 172, TXC_22211B },
	{ 173, TXC_22211C },
	// 174 Unused
	{ 175, KAISER_KS7022},
	{ 176, UNL_XZY },
	{ 177, HENGEDIANZI_BOARD },
	{ 178, WAIXING_SGZLZ },
	{ 179, HENGEDIANZI_XJZB },
	{ 180, UXROM_CC },
	// 181 Unused
	{ 182, HOSENKAN_BOARD },
	// 183 FDS bootleg (Shui Guan Pipe) - Unsupported
	{ 184, SUNSOFT_1 },
	{ 185, STD_CNROM },
	{ 186, FUKUTAKE_BOARD },
	{ 187, UNL_KOF96 },
	{ 188, BANDAI_KARAOKE },
	{ 189, TXC_TW },
	// 190 Unused
	{ 191, WAIXING_TYPE_B },
	{ 192, WAIXING_TYPE_C },
	{ 193, NTDEC_FIGHTINGHERO },
	{ 194, WAIXING_TYPE_D },
	{ 195, WAIXING_TYPE_E },
	{ 196, BTL_SUPERBROS11 },
	{ 197, UNL_SUPERFIGHTER3 },
	{ 198, WAIXING_TYPE_F },
	{ 199, WAIXING_TYPE_G },
	{ 200, BMC_36IN1 },
	{ 201, BMC_21IN1 },
	{ 202, BMC_150IN1 },
	{ 203, BMC_35IN1 },
	{ 204, BMC_64IN1 },
	{ 205, BMC_15IN1 },
	{ 206, STD_DXROM },
	{ 207, TAITO_X1_005_A },
	{ 208, GOUDER_37017 },
	// 209 JY Company Type B (Power Rangers 3 & 4, Shin Samurai Spirits 2) - Unsupported
	// 210 Some emu uses this as Mapper 19 without some features
	// 211 JY Company Type C (Tiny Toon Adventures 6, Donkey Kong Country 4) - Unsupported
	{ 212, BMC_SUPERHIK_300IN1 },
	{ 213, BMC_9999999IN1 },
	{ 214, BMC_SUPERGUN_20IN1 },
	{ 215, SUPERGAME_BOOGERMAN },
	{ 216, RCM_GS2015 },
	{ 217, BMC_GOLDENCARD_6IN1 },
	// 218 Unused
	// 219 Bootleg A9746 - Unsupported
	// 220 Unused
	{ 221, UNL_N625092 },
	{ 222, BTL_DRAGONNINJA },
	{ 223, WAIXING_TYPE_I },	// (according to NEStopia source, it's MMC3 with more WRAM)
	{ 224, WAIXING_TYPE_J },	// (according to NEStopia source, it's MMC3 with more WRAM)
	{ 225, BMC_72IN1 },
	{ 226, BMC_76IN1 },
	{ 227, BMC_1200IN1 },
	{ 228, ACTENT_ACT52 },
	{ 229, BMC_31IN1 },
	{ 230, BMC_22GAMES },
	{ 231, BMC_20IN1 },
	{ 232, CAMERICA_BF9096 },
	// 233 Super 22 Games - Unsupported
	// 234 AVE Maxi 15 - Unsupported
	// 235 Golden Game x-in-1 - Unsupported
	// 236 Game 800-in-1 - Unsupported
	// 237 Unused
	{ 238, UNL_603_5052 },
	// 239 Unused
	{ 240, CNE_SHLZ },
	{ 241, TXC_MXMDHTWO },
	{ 242, WAIXING_ZS },
	{ 243, SACHEN_74LS374_A },
	{ 244, CNE_DECATHLON },
	{ 245, WAIXING_TYPE_H },
	{ 246, CNE_FSB },
	// 247 Unused
	// 248 Unused
	{ 249, WAIXING_SECURITY },
	{ 250, NITRA_TDA },
	// 251 Shen Hua Jian Yun III?? - Unsupported
	{ 252, WAIXING_SGZ },
	// 253 Super 8-in-1 99 King Fighter?? - Unsupported
	{ 254, BTL_PIKACHUY2K },
	{ 255, BMC_110IN1 },
};

const nes_mmc *nes_mapper_lookup( int mapper )
{
	int i;

	for (i = 0; i < ARRAY_LENGTH(mmc_list); i++)
	{
		if (mmc_list[i].iNesMapper == mapper)
			return &mmc_list[i];
	}

	return NULL;
}

int nes_get_mmc_id( running_machine &machine, int mapper )
{
	const nes_mmc *mmc = nes_mapper_lookup(mapper);

	if (mmc == NULL)
		fatalerror("Unimplemented Mapper %d", mapper);

	return mmc->pcb_id;
}
