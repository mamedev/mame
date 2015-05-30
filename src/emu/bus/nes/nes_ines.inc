// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*****************************************************************************************

    NES MMC Emulation

    Support for iNES Mappers

****************************************************************************************/


/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0


/*************************************************************

 mmc_list

 Supported mappers and corresponding pcb id

 *************************************************************/

struct nes_mmc
{
	int    iNesMapper; /* iNES Mapper # */
	int    pcb_id;
};


static const nes_mmc mmc_list[] =
{
/*  INES   DESC                          LOW_W, LOW_R, MED_W, HIGH_W, PPU_latch, scanline CB, hblank CB */
	{  0, STD_NROM },
	{  1, STD_SXROM },
	{  2, STD_UXROM },
	{  3, STD_CNROM },
	{  4, STD_TXROM },
	{  5, STD_EXROM },
	{  6, FFE4_BOARD },
	{  7, STD_AXROM },
	{  8, FFE3_BOARD },
	{  9, STD_PXROM },
	{ 10, STD_FXROM },
	{ 11, DIS_74X377 },
	{ 12, REXSOFT_DBZ5 },
	{ 13, STD_CPROM },
	{ 14, REXSOFT_SL1632 },
	{ 15, WAIXING_WXZS2 },
	{ 16, BANDAI_LZ93EX2 },  // with 24c02
	{ 17, FFE8_BOARD },
	{ 18, JALECO_SS88006 },
	{ 19, NAMCOT_163 },
	{ 21, KONAMI_VRC4 },
	{ 22, KONAMI_VRC2 },
	{ 23, KONAMI_VRC2 },
	{ 24, KONAMI_VRC6 },
	{ 25, KONAMI_VRC4 },
	{ 26, KONAMI_VRC6 },
	{ 27, UNL_WORLDHERO },  // 27 World Hero board - Unsupported
	{ 28, BTL_ACTION53 },   // 28 - Multi-discrete PCB designed by Tepples for Action 53
	// 29 Unused
	// 30 UNROM 512 + Flash, currently unsupported
	{ 31, BTL_2A03_PURITANS },   // 31 - PCB designed by infinitelives & rainwarrior for 2A03 Puritans Album
	{ 32, IREM_G101 },
	{ 33, TAITO_TC0190FMC },
	{ 34, STD_BXROM },
	{ 35, UNL_SC127 },
	{ 36, TXC_STRIKEW },
	{ 37, PAL_ZZ },
	{ 38, DIS_74X161X138 },
	{ 39, UNL_STUDYNGAME },
	{ 40, BTL_SMB2JA },
	{ 41, CALTRON_6IN1 },
	{ 42, BTL_MARIOBABY },  // ai senshi nicole too, changed by crc_hack
	{ 43, UNL_SMB2J },
	{ 44, BMC_SUPERBIG_7IN1 },
	{ 45, BMC_HIK8IN1 },
	{ 46, RUMBLESTATION_BOARD },
	{ 47, NES_QJ },
	{ 48, TAITO_TC0190FMCP },
	{ 49, BMC_SUPERHIK_4IN1 },
	{ 50, BTL_SMB2JB },
	{ 51, BMC_BALLGAMES_11IN1 },
	{ 52, BMC_GOLD_7IN1 },
	{ 53, SVISION16_BOARD },
	{ 54, BMC_NOVEL1 },
	// 55 Genius SMB - No info (nor images) available
	{ 56, KAISER_KS202 },
	{ 57, BMC_GKA },
	{ 58, BMC_GKB },
	// 59 Unused
	// 60 4-in-1, 35-in-1 Reset based
	{ 61, RCM_TF9IN1 },
	{ 62, BMC_SUPER_700IN1 },
	{ 63, BMC_CH001 },  // Powerful 255
	{ 64, TENGEN_800032 },
	{ 65, IREM_H3001 },
	{ 66, STD_GXROM },
	{ 67, SUNSOFT_3 },
	{ 68, SUNSOFT_DCS },
	{ 69, SUNSOFT_FME7 },
	{ 70, DIS_74X161X161X32 },
	{ 71, CAMERICA_BF9093 },
	{ 72, JALECO_JF17 },
	{ 73, KONAMI_VRC3 },
	{ 74, WAIXING_TYPE_A },
	{ 75, KONAMI_VRC1 },
	{ 76, NAMCOT_3446 },
	{ 77, IREM_LROG017 },
	{ 78, IREM_HOLYDIVR },
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
	{ 90, JYCOMPANY_A },
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
	{ 103, UNL_2708 },  // 103 Bootleg cart 2708 (Doki Doki Panic - FDS Conversion) - Unsupported
	{ 104, CAMERICA_GOLDENFIVE },
	{ 105, STD_EVENT },
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
	{ 116, SOMARI_SL12 },
	{ 117, FUTUREMEDIA_BOARD },
	{ 118, STD_TXSROM },
	{ 119, STD_TQROM },
	{ 120, BTL_TOBIDASE },
	{ 121, KAY_BOARD },
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
	{ 132, TXC_22211 },
	{ 133, SACHEN_SA72008 },
	{ 134, BMC_FAMILY_4646 },
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
	{ 154, NAMCOT_34X3 },
	{ 155, STD_SXROM_A }, // diff compared to MMC1 concern WRAM
	{ 156, OPENCORP_DAOU306 },
	{ 157, BANDAI_DATACH }, // Datach Reader games -> must go in the Datach subslot
	{ 158, TENGEN_800037 },
	{ 159, BANDAI_LZ93EX1 }, // with 24c01
	{ 160, SACHEN_SA009 },
	// 161 Unused
	{ 162, WAIXING_FS304},  // not confirmed, but a lot of chinese releases use it like this...
	{ 163, NANJING_BOARD},
	{ 164, WAIXING_FFV },
	{ 165, WAIXING_SH2 },
	{ 166, SUBOR_TYPE1 },
	{ 167, SUBOR_TYPE0 },
	{ 168, UNL_RACERMATE },
	// 169 Unused
	// 170 Fujiya
	{ 171, KAISER_KS7058 },
	{ 172, TXC_DUMARACING },
	{ 173, TXC_MJBLOCK },
	// 174 Unused
	{ 175, KAISER_KS7022},
	{ 176, UNL_XIAOZY },
	{ 177, HENGG_SRICH },
	{ 178, WAIXING_SGZLZ },
	{ 179, HENGG_XHZS },
	{ 180, UXROM_CC },
	// 181 Unused
	{ 182, HOSENKAN_BOARD },
	{ 183, BTL_SHUIGUAN },
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
	{ 196, BTL_SBROS11 },
	{ 197, UNL_SF3 },
	{ 198, WAIXING_TYPE_F },
	{ 199, WAIXING_TYPE_G },
	{ 200, BMC_36IN1 },
	{ 201, BMC_21IN1 },
	{ 202, BMC_150IN1 },
	{ 203, BMC_35IN1 },
	{ 204, BMC_64IN1 },
	{ 205, BMC_15IN1 },
	{ 206, NAMCOT_34X3 },
	{ 207, TAITO_X1_005 },
	{ 208, GOUDER_37017 },
	{ 209, JYCOMPANY_C },
	{ 210, NAMCOT_175 },
	{ 211, JYCOMPANY_B },
	{ 212, BMC_SUPERHIK_300IN1 },
	{ 213, BMC_NOVEL2 },
	{ 214, BMC_SUPERGUN_20IN1 },
	{ 215, SUPERGAME_BOOGERMAN },
	{ 216, RCM_GS2015 },
	{ 217, BMC_GOLDENCARD_6IN1 },
	{ 218, NOCASH_NOCHR },
	// 219 UNL-A9746 (according to Cah4e3's code, no dump available (yet)
	// 220 Unused
	{ 221, UNL_N625092 },
	{ 222, BTL_DRAGONNINJA },
	{ 223, WAIXING_TYPE_I },    // (according to NEStopia source, it's MMC3 with more WRAM)
	{ 224, WAIXING_TYPE_J },    // (according to NEStopia source, it's MMC3 with more WRAM)
	{ 225, BMC_72IN1 },
	{ 226, BMC_76IN1 },
	{ 227, BMC_1200IN1 },
	{ 228, ACTENT_ACT52 },
	{ 229, BMC_31IN1 },
	{ 230, BMC_22GAMES },
	{ 231, BMC_20IN1 },
	{ 232, CAMERICA_BF9096 },
	{ 233, BMC_SUPER22 },
	{ 234, AVE_MAXI15 },
	{ 235, BMC_GOLD150 },   // 235 Golden Game x-in-1 - Unsupported
	// 236 Game 800-in-1 - Unsupported
	// 237 Unused
	{ 238, UNL_603_5052 },
	// 239 Unused
	{ 240, CNE_SHLZ },
	{ 241, TXC_COMMANDOS },
	{ 242, WAIXING_WXZS },
	{ 243, SACHEN_74LS374_ALT },
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

#if 0
int nes_get_mmc_id( running_machine &machine, int mapper )
{
	const nes_mmc *mmc = nes_mapper_lookup(mapper);

	if (mmc == NULL)
		fatalerror("Unimplemented Mapper %d\n", mapper);

	return mmc->pcb_id;
}
#endif

/*************************************************************

 ines_mapr_setup

 setup the board specific pcb_id for a given mapper

 *************************************************************/

void ines_mapr_setup( int mapper, int *pcb_id )
{
	const nes_mmc *mmc = nes_mapper_lookup(mapper);
	if (mmc == NULL)
		fatalerror("Unimplemented Mapper %d\n", mapper);

	*pcb_id = mmc->pcb_id;
}

/*************************************************************

 call_load_ines

 *************************************************************/

void nes_cart_slot_device::call_load_ines()
{
	UINT32 vram_size = 0, prgram_size = 0, battery_size = 0, mapper_sram_size = 0;
	UINT32 prg_size, vrom_size;
	UINT8 header[0x10];
	UINT8 mapper, submapper = 0, local_options;
	bool ines20 = FALSE, prg16k;
	std::string mapinfo;
	int pcb_id = 0, mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0;
	int crc_hack = 0;
	bool bus_conflict = FALSE;

	// read out the header
	fseek(0, SEEK_SET);
	fread(&header, 0x10);

	// SETUP step 1: getting PRG, VROM, VRAM sizes
	prg16k = (header[4] == 1);
	prg_size = prg16k ? 2 * 0x4000 : header[4] * 0x4000;
	vrom_size = header[5] * 0x2000;
	vram_size = 0x4000;

	// SETUP step 2: getting PCB and other settings
	mapper = (header[6] & 0xf0) >> 4;
	local_options = header[6] & 0x0f;

	switch (header[7] & 0xc)
	{
		case 0x4:
		case 0xc:
			// probably the header got corrupted: don't trust upper bits for mapper
			break;

		case 0x8:   // it's iNES 2.0 format
			ines20 = TRUE;
		case 0x0:
		default:
			mapper |= header[7] & 0xf0;
			break;
	}

	// use info from nes.hsi if available!
	if (hashfile_extrainfo(*this, mapinfo))
	{
		if (4 == sscanf(mapinfo.c_str(),"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
		{
			/* image is present in nes.hsi: overwrite the header settings with these */
			mapper = mapint1;
			local_options = mapint2 & 0x0f;
			crc_hack = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
			prg16k = (mapint3 == 1);
			prg_size = prg16k ? 2 * 0x4000 : mapint3 * 0x4000;
			vrom_size = mapint4 * 0x2000;
			logerror("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
		}
		else
		{
			logerror("NES: [%s], Invalid mapinfo found\n", mapinfo.c_str());
		}
	}
	else
	{
		logerror("NES: No extrainfo found\n");
	}

	// use extended iNES2.0 info if available!
	if (ines20)
	{
		mapper |= (header[8] & 0x0f) << 8;
		// read submappers (based on 20140116 specs)
		submapper = (header[8] & 0xf0 >> 8);
		prg_size += ((header[9] & 0x0f) << 8) * 0x4000;
		vrom_size += ((header[9] & 0xf0) << 4) * 0x2000;
	}
	ines_mapr_setup(mapper, &pcb_id);

	// handle submappers
	if (submapper)
	{
		// 001: MMC1
		if (mapper == 1 && submapper == 3)
			pcb_id = STD_SXROM_A;
		else if (mapper == 1 && submapper == 5)
			logerror("Unimplemented iNES2.0 submapper: SEROM/SHROM/SH1ROM.\n");
		// 002, 003, 007: UxROM, CNROM, AxROM
		else if (mapper == 2 && submapper == 2)
			bus_conflict = TRUE;
		else if (mapper == 3 && submapper == 2)
			bus_conflict = TRUE;
		else if (mapper == 7 && submapper == 2)
			bus_conflict = TRUE;
		// 021, 023, 025: VRC4 / VRC2
		else if (mapper == 21 || mapper == 23 || mapper == 25)
		{
			// 021, 023, 025: VRC4
			int line_1 = submapper & 0x07;
			int line_2 = (submapper & 0x08) ? line_1 + 1 : line_1 - 1;
			if (line_2 >= 0 && line_2 <= 7)
			{
				pcb_id = KONAMI_VRC4;
				m_cart->set_vrc_lines(line_1, line_2, 0);
			}
			else if (submapper == 15)
			{
				pcb_id = KONAMI_VRC2;
				m_cart->set_vrc_lines(1, 0, 0);
			}
		}
		// 032: Irem G101
		else if (mapper == 32 && submapper == 1)
		{
			m_cart->set_mirroring(PPU_MIRROR_HIGH); // Major League has hardwired mirroring
		}
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
		}
		// iNES Mapper 068 / Sunsoft 4
		else if (mapper == 68 && submapper == 1)
		{
			submapper = 0;
			logerror("Unimplemented iNES2.0 submapper: SUNSOFT-DCS.\n");
		}
		// iNES Mapper 071
		else if (mapper == 71 && submapper == 1)
		{
			m_cart->set_pcb_ctrl_mirror(TRUE);    // Mapper 71 is used for 2 diff boards
		}
		// iNES Mapper 078
		else if (mapper == 78)
		{
			if (submapper == 1)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			else if (submapper == 3)
				pcb_id = IREM_HOLYDIVR;
		}
		// iNES Mapper 185
		else if (mapper == 185)
		{
			int ce_state = (submapper & 0x0c) >> 2;
			m_cart->set_ce(0x03, ce_state);
		}
		// iNES Mapper 232
		else if (mapper == 210 && submapper == 1)
		{
			submapper = 0;
			logerror("Unimplemented iNES2.0 submapper: CAMERICA-BF9096.\n");
		}
		else if (submapper)
		{
			submapper = 0;
			logerror("Undocumented iNES2.0 submapper, please report it to the MESS boards!\n");
		}
	}

	// SETUP step 3: storing the info needed for emulation
	m_pcb_id = pcb_id;
	m_cart->set_mirroring(BIT(local_options, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	if (BIT(local_options, 1))
		battery_size = NES_BATTERY_SIZE; // with original iNES format we can only support 8K WRAM battery
	m_cart->set_trainer(BIT(local_options, 2) ? TRUE : FALSE);
	m_cart->set_four_screen_vram(BIT(local_options, 3) ? TRUE : FALSE);

	if (ines20)
	{
		// PRGRAM/BWRAM (not fully supported, also due to lack of 2.0 files)
		if ((header[10] & 0x0f) > 0)
			prgram_size = 0x80 << ((header[10] & 0x0f) - 1);
		if ((header[10] & 0xf0) > 0)
			battery_size = 0x80 << (((header[10] & 0xf0) >> 4) - 1);
		// VRAM
		vram_size = 0;
		if ((header[11] & 0x0f) > 0)
			vram_size = 0x80 << ((header[11] & 0x0f) - 1);
		if ((header[11] & 0xf0) > 0)
			vram_size |= 0x80 << (((header[11] & 0xf0) >> 4) - 1);
		// header[11] & 0xf0 is the size of battery backed VRAM, found so far in Racermate II only and not supported yet
	}
	else
	{
		// PRGRAM size is 8k for most games, but pirate carts often use different sizes,
		// so its size has been added recently to the iNES format spec, but almost no image uses it
		prgram_size = header[8] ? header[8] * 0x2000 : 0x2000;
	}

	// a few mappers correspond to multiple PCBs, so we need a few additional checks and tweaks
	switch (m_pcb_id)
	{
		case STD_NROM:
			if (prg_size == 3 * 0x4000) // NROM368 are padded with 2k empty data at start to accomplish with iNES standard
			{
				m_pcb_id = STD_NROM368;
				fseek(0x810, SEEK_SET);
				prg_size = 0xb800;
			}
			break;

		case NOCASH_NOCHR:
			// this mapper uses mirroring flags differently
			m_cart->set_four_screen_vram(FALSE);
			switch (local_options & 0x09)
			{
				case 0x00:
					m_cart->set_mirroring(PPU_MIRROR_HORZ);
					break;
				case 0x01:
					m_cart->set_mirroring(PPU_MIRROR_VERT);
					break;
				case 0x08:
					m_cart->set_mirroring(PPU_MIRROR_LOW);
					break;
				case 0x09:
					m_cart->set_mirroring(PPU_MIRROR_HIGH);
					break;
			}
			break;

		case STD_CNROM:
			if (mapper == 185 && !submapper)
			{
				switch (crc_hack)
				{
					case 0x0: // pin26: CE, pin27: CE (B-Wings, Bird Week)
						m_cart->set_ce(0x03, 0x03);
						break;
					case 0x4: // pin26: CE, pin27: /CE (Mighty Bomb Jack, Spy Vs. Spy)
						m_cart->set_ce(0x03, 0x01);
						break;
					case 0x8: // pin26: /CE, pin27: CE (Sansu 1, 2, 3 Nen, Othello)
						m_cart->set_ce(0x03, 0x02);
						break;
					case 0xc: // pin26: /CE, pin27: /CE (Seicross v2.0)
						m_cart->set_ce(0x03, 0x00);
						break;
				}
			}
			break;

		case KONAMI_VRC2:
			if (mapper == 22)
				m_cart->set_vrc_lines(0, 1, 1);
			if (mapper == 23 && !crc_hack && !submapper)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 23 && crc_hack && !submapper)
			{
				// here there are also Akumajou Special, Crisis Force, Parodius da!, Tiny Toons which are VRC-4
				m_cart->set_vrc_lines(3, 2, 0);
				m_pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			}
			break;

		case KONAMI_VRC4:
			if (mapper == 21 && !submapper)   // Wai Wai World 2 & Ganbare Goemon Gaiden 2 (the latter with crc_hack)
				m_cart->set_vrc_lines(crc_hack ? 7 : 2, crc_hack ? 6 : 1, 0);
			if (mapper == 25 && !submapper)   // here there is also Ganbare Goemon Gaiden which is VRC-2
				m_cart->set_vrc_lines(crc_hack ? 2 : 0, crc_hack ? 3 : 1, 0);
			break;

		case KONAMI_VRC6:
			if (mapper == 24)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 26)
				m_cart->set_vrc_lines(0, 1, 0);
			break;

		case IREM_G101:
			if (crc_hack && !submapper)
				m_cart->set_mirroring(PPU_MIRROR_HIGH); // Major League has hardwired mirroring
			else if (!submapper)
				m_cart->set_pcb_ctrl_mirror(TRUE);
			break;

		case DIS_74X161X161X32:
			if (mapper == 70)
				m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 70 & 152
			else
				m_cart->set_pcb_ctrl_mirror(TRUE);
			break;

		case SUNSOFT_2:
			if (mapper == 93)
				m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 89 & 93
			else
				m_cart->set_pcb_ctrl_mirror(TRUE);
			break;

		case HES_BOARD:
			if (crc_hack)
				m_cart->set_pcb_ctrl_mirror(TRUE);    // Mapper 113 is used for 2 diff boards
			break;

		case CAMERICA_BF9093:
			if (crc_hack && !submapper)
				m_cart->set_pcb_ctrl_mirror(TRUE);    // Mapper 71 is used for 2 diff boards
			break;

		case STD_BXROM:
			if (crc_hack && !submapper)
				m_pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			if (crc_hack)
				m_pcb_id = BANDAI_FJUMP2;   // Mapper 153 is used for 2 diff boards
			break;

		case IREM_HOLYDIVR:
			if (crc_hack && !submapper)
				m_pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			if (crc_hack)
				m_pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			break;

		case BMC_GOLD_7IN1:
			if (crc_hack)
				m_pcb_id = BMC_MARIOPARTY_7IN1;    // Mapper 52 is used for 2 diff boards
			break;

		case BTL_MARIOBABY:
			if (crc_hack)
				m_pcb_id = BTL_AISENSHINICOL;    // Mapper 42 is used for 2 diff boards
			break;

		case TAITO_X1_017:
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;

		case TAITO_X1_005:
			if (mapper == 207)
				m_cart->set_x1_005_alt(TRUE);
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;

		case NAMCOT_163:
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;
			//FIXME: we also have to fix Action 52 PRG loading somewhere...

		case BANDAI_DATACH:
			fatalerror("Bandai Datach games have to be mounted in the Datach subslot!\n");
			break;
	}

	// Finally turn off bus conflict emulation, because the pirate variants of the boards are bus conflict free and games would glitch
	m_cart->set_bus_conflict(bus_conflict);

	// SETUP step 4: logging what we have found
	if (!ines20)
	{
		logerror("Loaded game in iNES format:\n");
		logerror("-- Mapper %d\n", mapper);
		logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
		logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
		logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
		logerror("-- Mirroring %s\n", BIT(header[6], 0) ? "Vertical" : "Horizontal");
		if (battery_size)
			logerror("-- Battery found\n");
		if (m_cart->get_trainer())
			logerror("-- Trainer found\n");
		if (m_cart->get_four_screen_vram())
			logerror("-- 4-screen VRAM\n");
		logerror("-- TV System: %s\n", ((header[10] & 3) == 0) ? "NTSC" : (header[10] & 1) ? "Both NTSC and PAL" : "PAL");
	}
	else
	{
		logerror("Loaded game in Extended iNES format:\n");
		logerror("-- Mapper: %d\n", mapper);
		logerror("-- Submapper: %d\n", (header[8] & 0xf0) >> 4);
		logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
		logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
		logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
		logerror("-- PRG NVWRAM: %d\n", (header[10] & 0xf0) >> 4);
		logerror("-- PRG WRAM: %d\n", header[10] & 0x0f);
		logerror("-- CHR NVWRAM: %d\n", (header[11] & 0xf0) >> 4);
		logerror("-- CHR WRAM: %d\n", header[11] & 0x0f);
		logerror("-- TV System: %s\n", (header[12] & 2) ? "Both NTSC and PAL" : (header[12] & 1) ? "PAL" : "NTSC");
	}

	// SETUP step 5: allocate pointers for PRG/VROM
	if (prg_size)
		m_cart->prg_alloc(prg_size, tag());
	if (vrom_size)
		m_cart->vrom_alloc(vrom_size, tag());

	// if there is a trainer, skip it for the moment
	if (m_cart->get_trainer())
		fseek(0x210, SEEK_SET);

	// SETUP step 6: at last load the data!
	// Read in the program chunks
	if (prg16k)
	{
		fread(m_cart->get_prg_base(), 0x4000);
		memcpy(m_cart->get_prg_base() + 0x4000, m_cart->get_prg_base(), 0x4000);
	}
	else
		fread(m_cart->get_prg_base(), m_cart->get_prg_size());
#if SPLIT_PRG
	{
		FILE *prgout;
		char outname[255];

		sprintf(outname, "%s.prg", filename());
		prgout = fopen(outname, "wb");
		if (prgout)
		{
			fwrite(m_cart->get_prg_base(), 1, 0x4000 * m_cart->get_prg_size(), prgout);
			osd_printf_error("Created PRG chunk\n");
		}

		fclose(prgout);
	}
#endif

	// Read in any chr chunks
	if (m_cart->get_vrom_size())
		fread(m_cart->get_vrom_base(), m_cart->get_vrom_size());

#if SPLIT_CHR
	if (state->m_chr_chunks > 0)
	{
		FILE *chrout;
		char outname[255];

		sprintf(outname, "%s.chr", filename());
		chrout= fopen(outname, "wb");
		if (chrout)
		{
			fwrite(m_cart->get_vrom_base(), 1, m_cart->get_vrom_size(), chrout);
			osd_printf_error("Created CHR chunk\n");
		}
		fclose(chrout);
	}
#endif

	// SETUP steps 7: allocate the remaining pointer, when needed
	if (vram_size)
		m_cart->vram_alloc(vram_size);
	if (prgram_size || m_cart->get_trainer())
	{
		if (prgram_size)
			m_cart->prgram_alloc(prgram_size);
		else
			m_cart->prgram_alloc(0x2000);
		if (m_cart->get_trainer())
		{
			fseek(0x10, SEEK_SET);
			fread(m_cart->get_prgram_base() + 0x1000, 0x200);
		}
	}


	// Attempt to load a battery file for this ROM
	// A few boards have internal RAM with a battery (MMC6, Taito X1-005 & X1-017, etc.)
	if (battery_size || mapper_sram_size)
	{
		UINT32 tot_size = battery_size + mapper_sram_size;
		dynamic_buffer temp_nvram(tot_size);
		battery_load(&temp_nvram[0], tot_size, 0x00);
		if (battery_size)
		{
			//printf("here %d\n", battery_size);
			m_cart->battery_alloc(battery_size);
			memcpy(m_cart->get_battery_base(), &temp_nvram[0], battery_size);
		}
		if (mapper_sram_size)
			memcpy(m_cart->get_mapper_sram_base(), &temp_nvram[battery_size], m_cart->get_mapper_sram_size());
	}
}

const char * nes_cart_slot_device::get_default_card_ines(UINT8 *ROM, UINT32 len)
{
	UINT8 mapper, submapper = 0;
	bool ines20 = FALSE;
	std::string mapinfo;
	int pcb_id = 0, mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0;
	int crc_hack = 0;

	mapper = (ROM[6] & 0xf0) >> 4;

	switch (ROM[7] & 0xc)
	{
		case 0x4:
		case 0xc:
			// probably the header got corrupted: don't trust upper bits for mapper
			break;

		case 0x8:   // it's iNES 2.0 format
			ines20 = TRUE;
		case 0x0:
		default:
			mapper |= ROM[7] & 0xf0;
			break;
	}

	// use info from nes.hsi if available!
	if (hashfile_extrainfo(*this, mapinfo))
	{
		if (4 == sscanf(mapinfo.c_str(),"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
		{
			/* image is present in nes.hsi: overwrite the header settings with these */
			mapper = mapint1;
			crc_hack = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
		}
	}

	// use extended iNES2.0 info if available!
	if (ines20)
	{
		mapper |= (ROM[8] & 0x0f) << 8;
		// read submappers (based on 20140116 specs)
		submapper = (ROM[8] & 0xf0 >> 8);
	}

	ines_mapr_setup(mapper, &pcb_id);

	// handle submappers
	if (submapper)
	{
		// 001: MMC1
		if (mapper == 1 && submapper == 3)
			pcb_id = STD_SXROM_A;
		else if (mapper == 1 && submapper == 5)
			logerror("Unimplemented iNES2.0 submapper: SEROM/SHROM/SH1ROM.\n");
		// 021, 023, 025: VRC4 / VRC2
		else if (mapper == 21 || mapper == 23 || mapper == 25)
		{
			// 021, 023, 025: VRC4
			int line_1 = submapper & 0x07;
			int line_2 = (submapper & 0x08) ? line_1 + 1 : line_1 - 1;
			if (line_2 >= 0 && line_2 <= 7)
				pcb_id = KONAMI_VRC4;
			else if (submapper == 15)
				pcb_id = KONAMI_VRC2;
		}
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
		}
		// iNES Mapper 078
		else if (mapper == 78)
		{
			if (submapper == 1)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			else if (submapper == 3)
				pcb_id = IREM_HOLYDIVR;
		}
	}

	// solve mapper conflicts
	switch (pcb_id)
	{
		case STD_NROM:
			if (ROM[4] == 3)
				pcb_id = STD_NROM368;
			break;

		case KONAMI_VRC2:
			if (mapper == 23 && crc_hack && !submapper)
				pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			break;

		case STD_BXROM:
			if (crc_hack && !submapper)
				pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			if (crc_hack)
				pcb_id = BANDAI_FJUMP2;   // Mapper 153 is used for 2 diff boards
			break;

		case IREM_HOLYDIVR:
			if (crc_hack && !submapper)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			if (crc_hack)
				pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			break;

		case BMC_GOLD_7IN1:
			if (crc_hack)
				pcb_id = BMC_MARIOPARTY_7IN1;    // Mapper 52 is used for 2 diff boards
			break;

		case BTL_MARIOBABY:
			if (crc_hack)
				pcb_id = BTL_AISENSHINICOL;    // Mapper 42 is used for 2 diff boards
			break;
	}

	return nes_get_slot(pcb_id);
}
