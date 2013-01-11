/*****************************************************************************************

    NES Cart PCBs Emulation

****************************************************************************************/

struct nes_pcb
{
	const char              *pcb_name;
	int                     pcb_id;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const nes_pcb pcb_list[] =
{
	/* Nintendo HROM, NROM, RROM, SROM & STROM */
	{ "HVC-HROM",         STD_NROM },
	{ "HVC-NROM",         STD_NROM },
	{ "HVC-NROM-128",     STD_NROM },
	{ "HVC-NROM-256",     STD_NROM },
	{ "HVC-RROM",         STD_NROM },
	{ "HVC-RROM-128",     STD_NROM },
	{ "HVC-RTROM",        STD_NROM },
	{ "HVC-SROM",         STD_NROM },
	{ "HVC-STROM",        STD_NROM },
	{ "NES-HROM",         STD_NROM },
	{ "NES-NROM",         STD_NROM },
	{ "NES-NROM-128",     STD_NROM },
	{ "NES-NROM-256",     STD_NROM },
	{ "NES-RROM",         STD_NROM },
	{ "NES-RROM-128",     STD_NROM },
	{ "NES-SROM",         STD_NROM },
	{ "NES-STROM",        STD_NROM },
	/* "No mapper" boards by other manufacturer */
	{ "IREM-NROM-128",    STD_NROM },
	{ "IREM-NROM-256",    STD_NROM },
	{ "BANDAI-NROM-128",  STD_NROM },
	{ "BANDAI-NROM-256",  STD_NROM },
	{ "KONAMI-NROM-128",  STD_NROM },
	{ "SETA-NROM-128",    STD_NROM },
	{ "SUNSOFT-NROM-128", STD_NROM },
	{ "SUNSOFT-NROM-256", STD_NROM },
	{ "TAITO-NROM-128",   STD_NROM },
	{ "TAITO-NROM-256",   STD_NROM },
	{ "NAMCOT-3301",      STD_NROM },
	{ "NAMCOT-3302",      STD_NROM },
	{ "NAMCOT-3303",      STD_NROM },
	{ "NAMCOT-3304",      STD_NROM },
	{ "NAMCOT-3305",      STD_NROM },
	{ "NAMCOT-3311",      STD_NROM },
	{ "NAMCOT-3312",      STD_NROM },
	{ "NAMCOT-3411",      STD_NROM },
	{ "JALECO-JF-01",     STD_NROM },
	{ "JALECO-JF-02",     STD_NROM },
	{ "JALECO-JF-03",     STD_NROM },
	{ "JALECO-JF-04",     STD_NROM },
	{ "TENGEN-800003",    STD_NROM },
	{ "SACHEN-NROM",      STD_NROM },
	/* Nintendo Family BASIC pcb (NROM + 2K or 4K WRAM) */
	{ "HVC-FAMILYBASIC",  HVC_FAMBASIC },
	/* Game Genie */
	{ "CAMERICA-GAMEGENIE", GG_NROM },
//
	/* Nintendo UNROM/UOROM */
	{ "HVC-UNROM",        STD_UXROM },
	{ "HVC-UNEPROM",      STD_UXROM },
	{ "HVC-UOROM",        STD_UXROM },
	{ "NES-UNROM",        STD_UXROM },
	{ "NES-UNEPROM",      STD_UXROM },
	{ "NES-UOROM",        STD_UXROM },
	{ "HVC-UNROM+74HC08", UXROM_CC },
	/* UxROM boards by other manufacturer */
	{ "IREM-UNROM",       STD_UXROM },
	{ "KONAMI-UNROM",     STD_UXROM },
	{ "TAITO-UNROM",      STD_UXROM },
	{ "JALECO-JF-15",     STD_UXROM },
	{ "JALECO-JF-18",     STD_UXROM },
	{ "JALECO-JF-39",     STD_UXROM },
//
	/* Nintendo CNROM */
	{ "HVC-CNROM",        STD_CNROM },
	{ "NES-CNROM",        STD_CNROM },
	/* CxROM boards by other manufacturer */
	{ "BANDAI-CNROM",     STD_CNROM },
	{ "BANDAI-74*161/32", STD_CNROM },
	{ "KONAMI-CNROM",     STD_CNROM },
	{ "NAMCOT-CNROM+WRAM", STD_CNROM },
	{ "TAITO-CNROM",      STD_CNROM },
	{ "AVE-74*161",       STD_CNROM },
	{ "NTDEC-N715061",    STD_CNROM },
	{ "NTDEC-N715062",    STD_CNROM },
	{ "SACHEN-CNROM",     STD_CNROM },
	/* Bandai Aerobics Studio (CNROM boards + special audio chip) */
	{ "BANDAI-PT-554",    BANDAI_PT554 },
	/* FIXME: Is this the same as mapper 3? */
	{ "TENGEN-800008",    TENGEN_800008 },
//
	/* Nintendo AxROM */
	{ "HVC-AMROM",        STD_AXROM },
	{ "HVC-AN1ROM",       STD_AXROM },
	{ "HVC-ANROM",        STD_AXROM },
	{ "HVC-AOROM",        STD_AXROM },
	{ "NES-AMROM",        STD_AXROM },
	{ "NES-AN1ROM",       STD_AXROM },
	{ "NES-ANROM",        STD_AXROM },
	{ "NES-AOROM",        STD_AXROM },
	/* AxROM boards by other manufacturer */
	{ "ACCLAIM-AOROM",    STD_AXROM },
//
	/* Nintendo PxROM */
	{ "HVC-PEEOROM",      STD_PXROM },
	{ "HVC-PNROM",        STD_PXROM },
	{ "NES-PEEOROM",      STD_PXROM },
	{ "NES-PNROM",        STD_PXROM },
//
	/* Nintendo FxROM */
	{ "HVC-FJROM",        STD_FXROM },
	{ "HVC-FKROM",        STD_FXROM },
	{ "NES-FJROM",        STD_FXROM },
	{ "NES-FKROM",        STD_FXROM },
//
	/* Nintendo BXROM */
	{ "HVC-BNROM",        STD_BXROM },
	{ "NES-BNROM",        STD_BXROM },
	/* BxROM boards by other manufacturer */
	{ "IREM-BNROM",       STD_BXROM },
//
	/* Nintendo CPROM */
	{ "HVC-CPROM",        STD_CPROM },
	{ "NES-CPROM",        STD_CPROM },
//
	/* Nintendo GNROM & MHROM */
	{ "HVC-GNROM",        STD_GXROM },
	{ "NES-GNROM",        STD_GXROM },
	{ "HVC-MHROM",        STD_MXROM },
	{ "NES-MHROM",        STD_MXROM },
	{ "PAL-MH",           STD_MXROM },
	/* GxROM boards by other manufacturer */
	{ "BANDAI-GNROM",     STD_GXROM },
//
	/* Nintendo NxROM */
	{ "HVC-NTBROM",       STD_NXROM },
	{ "NES-NTBROM",       STD_NXROM },
	/* NxROM boards by other manufacturer (this board was mainly used by Sunsoft?) */
	{ "SUNSOFT-4",        STD_NXROM },
	{ "SUNSOFT-DCS",      SUNSOFT_DCS },
	{ "TENGEN-800042",    STD_NXROM },
//
	/* Nintendo JxROM */
	{ "HVC-JLROM",        STD_JXROM },
	{ "HVC-JSROM",        STD_JXROM },
	{ "NES-JLROM",        STD_JXROM },
	{ "NES-JSROM",        STD_JXROM },
	{ "NES-BTR",          STD_JXROM },
	/* JxROM boards by other manufacturer (this board was mainly used by Sunsoft?) */
	{ "SUNSOFT-5A",       STD_JXROM },
	{ "SUNSOFT-5B",       STD_JXROM },
	{ "SUNSOFT-FME-7",    STD_JXROM },
//
	/* Nintendo UN1ROM */
	{ "HVC-UN1ROM",       STD_UN1ROM },
	{ "NES-UN1ROM",       STD_UN1ROM },
//
	/* Nintendo SxROM */
	{ "HVC-SAROM",        STD_SXROM },
	{ "HVC-SBROM",        STD_SXROM },
	{ "HVC-SC1ROM",       STD_SXROM },
	{ "HVC-SCROM",        STD_SXROM },
	{ "HVC-SEROM",        STD_SXROM },
	{ "HVC-SF1ROM",       STD_SXROM },
	{ "HVC-SFROM",        STD_SXROM },
	{ "HVC-SGROM",        STD_SXROM },
	{ "HVC-SH1ROM",       STD_SXROM },
	{ "HVC-SHROM",        STD_SXROM },
	{ "HVC-SIROM",        STD_SXROM },
	{ "HVC-SJROM",        STD_SXROM },
	{ "HVC-SKROM",        STD_SXROM },
	{ "HVC-SKEPROM",      STD_SXROM },
	{ "HVC-SL1ROM",       STD_SXROM },
	{ "HVC-SL2ROM",       STD_SXROM },
	{ "HVC-SL3ROM",       STD_SXROM },
	{ "HVC-SLROM",        STD_SXROM },
	{ "HVC-SLRROM",       STD_SXROM },
	{ "HVC-SMROM",        STD_SXROM },
	{ "HVC-SNROM",        STD_SXROM },
	{ "HVC-SUROM",        STD_SXROM },
	{ "HVC-SXROM",        STD_SXROM },
	{ "NES-SAROM",        STD_SXROM },
	{ "NES-SBROM",        STD_SXROM },
	{ "NES-SC1ROM",       STD_SXROM },
	{ "NES-SCROM",        STD_SXROM },
	{ "NES-SEROM",        STD_SXROM },
	{ "NES-SF1ROM",       STD_SXROM },
	{ "NES-SFROM",        STD_SXROM },
	{ "NES-SGROM",        STD_SXROM },
	{ "NES-SH1ROM",       STD_SXROM },
	{ "NES-SHROM",        STD_SXROM },
	{ "NES-SIROM",        STD_SXROM },
	{ "NES-SJROM",        STD_SXROM },
	{ "NES-SKROM",        STD_SXROM },
	{ "NES-SKEPROM",      STD_SXROM },
	{ "NES-SL1ROM",       STD_SXROM },
	{ "NES-SL2ROM",       STD_SXROM },
	{ "NES-SL3ROM",       STD_SXROM },
	{ "NES-SLROM",        STD_SXROM },
	{ "NES-SLRROM",       STD_SXROM },
	{ "NES-SMROM",        STD_SXROM },
	{ "NES-SNROM",        STD_SXROM },
	{ "NES-SUROM",        STD_SXROM },
	{ "NES-SXROM",        STD_SXROM },
	{ "NES-WH",           STD_SXROM },
	{ "NES-SOROM",        STD_SOROM },
	{ "HVC-SOROM",        STD_SOROM },
	/* SxROM boards by other manufacturer */
	{ "KONAMI-SLROM",     STD_SXROM },
	{ "VIRGIN-SNROM",     STD_SXROM },
//
	/* Nintendo TxROM */
	{ "HVC-TBROM",        STD_TXROM },
	{ "HVC-TEROM",        STD_TXROM },
	{ "HVC-TFROM",        STD_TXROM },
	{ "HVC-TGROM",        STD_TXROM },
	{ "HVC-TKROM",        STD_TXROM },
	{ "HVC-TKEPROM",      STD_TXROM },
	{ "HVC-TL1ROM",       STD_TXROM },
	{ "HVC-TL2ROM",       STD_TXROM },
	{ "HVC-TLROM",        STD_TXROM },
	{ "HVC-TNROM",        STD_TXROM },
	{ "HVC-TR1ROM",       STD_TVROM },
	{ "HVC-TSROM",        STD_TXROM },
	{ "HVC-TVROM",        STD_TVROM },
	{ "NES-B4",           STD_TXROM },
	{ "NES-TBROM",        STD_TXROM },
	{ "NES-TEROM",        STD_TXROM },
	{ "NES-TFROM",        STD_TXROM },
	{ "NES-TGROM",        STD_TXROM },
	{ "NES-TKROM",        STD_TXROM },
	{ "NES-TKEPROM",      STD_TXROM },
	{ "NES-TL1ROM",       STD_TXROM },
	{ "NES-TL2ROM",       STD_TXROM },
	{ "NES-TLROM",        STD_TXROM },
	{ "NES-TNROM",        STD_TXROM },
	{ "NES-TR1ROM",       STD_TVROM },
	{ "NES-TSROM",        STD_TXROM },
	{ "NES-TVROM",        STD_TVROM },
	/* TxROM boards by other manufacturer */
	{ "ACCLAIM-MC-ACC",   STD_TXROM },
	{ "ACCLAIM-TLROM",    STD_TXROM },
	{ "KONAMI-TLROM",     STD_TXROM },
//
	/* Nintendo DxROM */
	{ "HVC-DE1ROM",       STD_DXROM },
	{ "HVC-DEROM",        STD_DXROM },
	{ "HVC-DRROM",        STD_DRROM },
	{ "NES-DE1ROM",       STD_DXROM },
	{ "NES-DEROM",        STD_DXROM },
	{ "NES-DRROM",        STD_DRROM },
	/* DxROM boards by other manufacturer */
	{ "NAMCOT-3401",      STD_DXROM },
	{ "NAMCOT-3405",      STD_DXROM },
	{ "NAMCOT-3406",      STD_DXROM },
	{ "NAMCOT-3407",      STD_DXROM },
	{ "NAMCOT-3413",      STD_DXROM },
	{ "NAMCOT-3414",      STD_DXROM },
	{ "NAMCOT-3415",      STD_DXROM },
	{ "NAMCOT-3416",      STD_DXROM },
	{ "NAMCOT-3417",      STD_DXROM },
	{ "NAMCOT-3451",      STD_DXROM },
	{ "TENGEN-800002",    STD_DXROM },
	{ "TENGEN-800004",    STD_DRROM },
	{ "TENGEN-800030",    STD_DXROM },
//
	/* Nintendo HKROM */
	{ "HVC-HKROM",        STD_HKROM },
	{ "NES-HKROM",        STD_HKROM },
//
	/* Nintendo BNROM */
	{ "HVC-TQROM",        STD_TQROM },
	{ "NES-TQROM",        STD_TQROM },
//
	/* Nintendo TxSROM */
	{ "HVC-TKSROM",       STD_TXSROM },
	{ "HVC-TLSROM",       STD_TXSROM },
	{ "NES-TKSROM",       STD_TXSROM },
	{ "NES-TLSROM",       STD_TXSROM },
//
	/* Nintendo ExROM */
	{ "HVC-EKROM",        STD_EXROM },
	{ "HVC-ELROM",        STD_EXROM },
	{ "HVC-ETROM",        STD_EXROM },
	{ "HVC-EWROM",        STD_EXROM },
	{ "NES-EKROM",        STD_EXROM },
	{ "NES-ELROM",        STD_EXROM },
	{ "NES-ETROM",        STD_EXROM },
	{ "NES-EWROM",        STD_EXROM },
//
	/* Nintendo Custom boards */
	{ "PAL-ZZ",           PAL_ZZ },
	{ "NES-QJ",           NES_QJ },
	{ "NES-EVENT",        UNSUPPORTED_BOARD },
//
	/* Discrete board IC_74x139x74 */
	{ "JALECO-JF-05",     DIS_74X139X74 },
	{ "JALECO-JF-06",     DIS_74X139X74 },
	{ "JALECO-JF-07",     DIS_74X139X74 },
	{ "JALECO-JF-08",     DIS_74X139X74 },
	{ "JALECO-JF-09",     DIS_74X139X74 },
	{ "JALECO-JF-10",     DIS_74X139X74 },
	{ "KONAMI-74*139/74", DIS_74X139X74 },
	{ "TAITO-74*139/74",  DIS_74X139X74 },
	/* Discrete board IC_74x377 */
	{ "AGCI-47516",       DIS_74X377 },
	{ "AVE-NINA-07",      DIS_74X377 },
	{ "COLORDREAMS-74*377", DIS_74X377 },
	/* Discrete board IC_74x161x161x32 */
	{ "BANDAI-74*161/161/32", DIS_74X161X161X32 },
	{ "TAITO-74*161/161/32", DIS_74X161X161X32 },
	/* Discrete board IC_74x161x138 */
	{ "BIT-CORP-74*161/138", DIS_74X161X138 },
//
	{ "BANDAI-LZ93D50",   BANDAI_LZ93 },
	{ "BANDAI-LZ93D50+24C01", BANDAI_LZ93EX },
	{ "BANDAI-LZ93D50+24C02", BANDAI_LZ93EX },
	{ "BANDAI-FCG-1",     BANDAI_FCG },
	{ "BANDAI-FCG-2",     BANDAI_FCG },
	{ "BANDAI-JUMP2",     BANDAI_JUMP2 },
	{ "BANDAI-DATACH",    BANDAI_DATACH },
	{ "BANDAI-KARAOKE",   BANDAI_KARAOKE },
	{ "BANDAI-OEKAKIDS",  BANDAI_OEKAKIDS },
	{ "IREM-FCG-1",       BANDAI_FCG },
//
	{ "IREM-G101",        IREM_G101 },
	{ "IREM-74*161/161/21/138", IREM_LROG017 },
	{ "IREM-H-3001",      IREM_H3001 },
	{ "IREM-H3001",       IREM_H3001 },
	{ "IREM-HOLYDIVER",   IREM_HOLYDIV },
	{ "IREM-TAM-S1",      IREM_TAM_S1 },
//
	{ "JALECO-JF-23",     JALECO_SS88006 },
	{ "JALECO-JF-24",     JALECO_SS88006 },
	{ "JALECO-JF-25",     JALECO_SS88006 },
	{ "JALECO-JF-27",     JALECO_SS88006 },
	{ "JALECO-JF-29",     JALECO_SS88006 },
	{ "JALECO-JF-30",     JALECO_SS88006 },
	{ "JALECO-JF-31",     JALECO_SS88006 },
	{ "JALECO-JF-32",     JALECO_SS88006 },
	{ "JALECO-JF-33",     JALECO_SS88006 },
	{ "JALECO-JF-34",     JALECO_SS88006 },
	{ "JALECO-JF-35",     JALECO_SS88006 },
	{ "JALECO-JF-36",     JALECO_SS88006 },
	{ "JALECO-JF-37",     JALECO_SS88006 },
	{ "JALECO-JF-38",     JALECO_SS88006 },
	{ "JALECO-JF-40",     JALECO_SS88006 },
	{ "JALECO-JF-41",     JALECO_SS88006 },
	{ "JALECO-JF-11",     JALECO_JF11 },
	{ "JALECO-JF-12",     JALECO_JF11 },
	{ "JALECO-JF-14",     JALECO_JF11 },
	{ "JALECO-JF-13",     JALECO_JF13 },
	{ "JALECO-JF-16",     JALECO_JF16 },
	{ "JALECO-JF-17",     JALECO_JF17 },
	{ "JALECO-JF-26",     JALECO_JF17 },
	{ "JALECO-JF-28",     JALECO_JF17 },
	{ "JALECO-JF-19",     JALECO_JF19 },
	{ "JALECO-JF-21",     JALECO_JF19 },
//
	{ "KONAMI-VRC-1",     KONAMI_VRC1 },
	{ "JALECO-JF-20",     KONAMI_VRC1 },
	{ "JALECO-JF-22",     KONAMI_VRC1 },
	{ "KONAMI-VRC-2",     KONAMI_VRC2 },
	{ "KONAMI-VRC-3",     KONAMI_VRC3 },
	{ "KONAMI-VRC-4",     KONAMI_VRC4 },
	{ "KONAMI-VRC-6",     KONAMI_VRC6 },
	{ "KONAMI-VRC-7",     KONAMI_VRC7 },
	{ "UNL-VRC7",         KONAMI_VRC7 },
//
	{ "NAMCOT-163",       NAMCOT_163 },
	{ "NAMCOT-175",       NAMCOT_163 },
	{ "NAMCOT-340",       NAMCOT_163 },
	{ "NAMCOT-3425",      NAMCOT_3425 },
	{ "NAMCOT-3433",      NAMCOT_34X3 },
	{ "NAMCOT-3443",      NAMCOT_34X3 },
	{ "NAMCOT-3446",      NAMCOT_3446 },
	{ "NAMCOT-3453",      NAMCOT_3453 },
//
	{ "SUNSOFT-1",        SUNSOFT_1 },
	{ "SUNSOFT-2",        SUNSOFT_2 },
	{ "SUNSOFT-3",        SUNSOFT_3 },
	//
	{ "TAITO-TC0190FMC",  TAITO_TC0190FMC },
	{ "TAITO-TC0190FMC+PAL16R4", TAITO_TC0190FMCP },
	{ "TAITO-TC0350FMR",  TAITO_TC0190FMC },
	{ "TAITO-X1-005",     TAITO_X1_005 },   // two variants exist, depending on pin17 & pin31 connections
	{ "TAITO-X1-017",     TAITO_X1_017 },
//
	{ "AGCI-50282",       AGCI_50282 },
	{ "AVE-NINA-01",      AVE_NINA01 },
	{ "AVE-NINA-02",      AVE_NINA01 },
	{ "AVE-NINA-03",      AVE_NINA06 },
	{ "AVE-NINA-06",      AVE_NINA06 },
	{ "AVE-MB-91",        AVE_NINA06 },
	{ "TXC-74*138/175",   AVE_NINA06 },
	{ "UNL-SA-016-1M",    AVE_NINA06 },
	{ "CAMERICA-ALGN",    CAMERICA_BF9093 },
	{ "CAMERICA-BF9093",  CAMERICA_BF9093 },
	{ "CODEMASTERS-NR8N", CAMERICA_BF9093 },
	{ "NR8NV1-1",         CAMERICA_BF9093 },
	{ "CAMERICA-BF9097",  CAMERICA_BF9097 },
	{ "CAMERICA-ALGQ",    CAMERICA_BF9096 },
	{ "CAMERICA-BF9096",  CAMERICA_BF9096 },
	{ "CAMERICA-GOLDENFIVE", CAMERICA_GOLDENFIVE },
	{ "CNE-DECATHLON",    CNE_DECATHLON },
	{ "CNE-PSB",          CNE_FSB },
	{ "CNE-SHLZ",         CNE_SHLZ },
	{ "JYCOMPANY-A",      UNSUPPORTED_BOARD },  // mapper 90
	{ "JYCOMPANY-B",      UNSUPPORTED_BOARD },  // mapper 209
	{ "JYCOMPANY-C",      UNSUPPORTED_BOARD },  // mapper 211
	{ "NTDEC-112",        NTDEC_ASDER },    // mapper 112 (better rename this board tag, to avoid confusion with TC-112!!
	{ "NTDEC-TC-112",     NTDEC_FIGHTINGHERO },
	{ "NTDEC-193",        NTDEC_FIGHTINGHERO }, // mapper 193
	{ "UNL-TEK90",        UNSUPPORTED_BOARD },  // related to JY Company? (i.e. mappers 90, 209, 211?)
	{ "UNL-SA-002",       SACHEN_TCU02 },
	{ "UNL-SA-009",       SACHEN_SA009 },
	{ "UNL-SA-0036",      SACHEN_SA0036 },
	{ "UNL-SA-0037",      SACHEN_SA0037 },
	{ "UNL-SA-72007",     SACHEN_SA72007 },
	{ "UNL-SA-72008",     SACHEN_SA72008 },
	{ "UNL-SA-NROM",      SACHEN_TCA01 },
	{ "UNL-SACHEN-TCA01", SACHEN_TCA01 },
	{ "SACHEN-8259A",     SACHEN_8259A },
	{ "UNL-SACHEN-8259A", SACHEN_8259A },
	{ "SACHEN-8259B",     SACHEN_8259B },
	{ "UNL-SACHEN-8259B", SACHEN_8259B },
	{ "SACHEN-8259C",     SACHEN_8259C },
	{ "UNL-SACHEN-8259C", SACHEN_8259C },
	{ "SACHEN-8259D",     SACHEN_8259D },
	{ "UNL-SACHEN-8259D", SACHEN_8259D },
	{ "UNL-SACHEN-74LS374N", SACHEN_74LS374 },
	{ "UNL-SACHEN-74LS374N-A", SACHEN_74LS374_A },  /* FIXME: Made up boards the different mirroring handling */
	{ "UNL-TC-U01-1.5M",  SACHEN_TCU01 },
	{ "UNL-SA-9602B",     UNSUPPORTED_BOARD },
	{ "TENGEN-800032",    TENGEN_800032 },
	{ "TENGEN-800037",    TENGEN_800037 },
	{ "WAIXING-A",        WAIXING_TYPE_A },
	{ "WAIXING-A-1",      WAIXING_TYPE_A_1 },   /* FIXME: Made up boards the different CHRRAM banks (see Ji Jia Zhan Shi) */
	{ "WAIXING-B",        WAIXING_TYPE_B },
	{ "WAIXING-C",        WAIXING_TYPE_C },
	{ "WAIXING-D",        WAIXING_TYPE_D },
	{ "WAIXING-E",        WAIXING_TYPE_E },
	{ "WAIXING-F",        WAIXING_TYPE_F },
	{ "WAIXING-G",        WAIXING_TYPE_G },
	{ "WAIXING-H",        WAIXING_TYPE_H },
	{ "WAIXING-I",        WAIXING_TYPE_I },
	{ "WAIXING-J",        WAIXING_TYPE_J },
	{ "WAIXING-SGZLZ",    WAIXING_SGZLZ },
	{ "WAIXING-SEC",      WAIXING_SECURITY },
	{ "WAIXING-SGZ",      WAIXING_SGZ },
	{ "WAIXING-PS2",      WAIXING_PS2 },
	{ "WAIXING-FFV",      WAIXING_FFV },
	{ "WAIXING-ZS",       WAIXING_ZS },
	{ "WAIXING-DQ8",      WAIXING_DQ8 },
	{ "WAIXING-SH2",      WAIXING_SH2 },
//
	{ "TXC-TW",           TXC_TW },
	{ "TXC-STRIKEWOLF",   TXC_STRIKEWOLF },
	{ "TXC-MXMDHTWO",     TXC_MXMDHTWO },
	{ "UNL-22211",        TXC_22211A },
	/* FIXME: Made up boards the different mirroring handling */
	{ "UNL-22211-A",      TXC_22211A },
	{ "UNL-22211-B",      TXC_22211B },
	{ "UNL-22211-C",      TXC_22211C },
	{ "UNL-REXSOFT-DBZ5", REXSOFT_DBZ5 },
	{ "UNL-SL1632",       REXSOFT_SL1632 },
	{ "SUBOR-BOARD-0",    SUBOR_TYPE0 },
	{ "SUBOR-BOARD-1",    SUBOR_TYPE1 },
	{ "SOMERITEAM-SL-12", SOMERI_SL12 }, // mapper 116
	{ "UNL-CONY",         CONY_BOARD },
	{ "UNL-YOKO",         YOKO_BOARD },
	{ "UNL-GOUDER",       GOUDER_37017 },
	{ "UNL-NITRA",        NITRA_TDA },
	{ "UNL-HOSENKAN",     HOSENKAN_BOARD },
	{ "UNL-SUPERGAME",    SUPERGAME_LIONKING },
	{ "UNL-PANDAPRINCE",  KAY_PANDAPRINCE },
	{ "DREAMTECH01",      DREAMTECH_BOARD },
	{ "DAOU-306",         OPENCORP_DAOU306 },
	{ "HES",              HES_BOARD },
	/* FIXME: Made up boards the different mirroring handling? */
	{ "HES-6IN1",         HES6IN1_BOARD },
	{ "SUPERGAME-BOOGERMAN", SUPERGAME_BOOGERMAN },
	{ "FUTUREMEDIA",      FUTUREMEDIA_BOARD },
	{ "FUKUTAKE",         FUKUTAKE_BOARD },
	{ "MAGICSERIES",      MAGICSERIES_MD },
	{ "KASING",           KASING_BOARD },
	{ "HENGGEDIANZI",     HENGEDIANZI_BOARD },
	{ "HENGGEDIANZI-XJZB", HENGEDIANZI_XJZB },
	{ "KAISER-KS7058",    KAISER_KS7058 },
	{ "KAISER-KS202",     KAISER_KS202 },// mapper 56
	{ "KAISER-KS7022",    KAISER_KS7022 },// mapper 175
	{ "UNL-KS7012",       UNSUPPORTED_BOARD /*KAISER_KS7012*/ },    // used in Zanac (FDS Conversion) (support missing atm)
	{ "UNL-KS7013B",      UNSUPPORTED_BOARD },  // used in Highway Star (FDS Conversion) (support missing atm)
	{ "UNL-KS7017",       KAISER_KS7017 },
	{ "UNL-KS7032",       KAISER_KS7032 }, //  mapper 142
	{ "RCM-GS2015",       RCM_GS2015 },
	{ "RCM-TETRISFAMILY", RCM_TETRISFAMILY },
	{ "UNL-NINJARYU",     UNSUPPORTED_BOARD },// mapper 111
	{ "UNL-NANJING",      NANJING_BOARD },// mapper 163
	{ "WHIRLWIND-2706",   WHIRLWIND_2706 },
	{ "UNL-H2288",        UNL_H2288 },
	{ "UNL-DANCE",        UNSUPPORTED_BOARD },
	{ "UNL-EDU2000",      UNL_EDU2K },
	{ "UNL-SHERO",        UNSUPPORTED_BOARD /*SACHEN_SHERO*/ },
	{ "UNL-TF1201",       UNSUPPORTED_BOARD /*UNL_TF1201*/ },
	{ "RUMBLESTATION",    RUMBLESTATION_BOARD },    // mapper 46
	{ "UNL-WORLDHERO",    UNSUPPORTED_BOARD },// mapper 27
	{ "UNL-A9746",        UNSUPPORTED_BOARD },// mapper 219
	{ "UNL-603-5052",     UNL_603_5052 },// mapper 238?
	{ "UNL-SHJY3",        UNL_SHJY3 },// mapper 253
	{ "UNL-RACERMATE",    UNL_RACERMATE },// mapper 168
	{ "UNL-N625092",      UNL_N625092 },
	{ "BMC-N625092",      UNL_N625092 },
	{ "UNL-SC-127",       UNL_SC127 },
	{ "UNL-SMB2J",        UNL_SMB2J },
	{ "BTL-SMB2C",        UNL_SMB2J },
	{ "UNL-MK2",          UNL_MK2 },
	{ "UNL-XZY",          UNL_XZY },
	{ "UNL-KOF96",        UNL_KOF96 },
	{ "UNL-SUPERFIGHTER3", UNL_SUPERFIGHTER3 },
	{ "UNL-8237",         UNL_8237 },
	{ "UNL-8237A",        UNSUPPORTED_BOARD },
	{ "UNL-AX5705",       UNL_AX5705 },
	{ "UNL-CC-21",        UNL_CC21 },
	{ "UNL-KOF97",        UNL_KOF97 },
	{ "UNL-KS7057",       UNL_KS7057 }, // mapper 196 alt (for Street Fighter VI / Fight Street VI)
	{ "UNL-T-230",        UNL_T230 },
	{ "UNL-STUDYNGAME",   UNL_STUDYNGAME }, // mapper 39
	{ "UNL-OneBus",       UNSUPPORTED_BOARD },
	{ "UNL-FS304",        UNL_FS304 },  // used in Zelda 3 by Waixing
	{ "UNL-43272",        UNSUPPORTED_BOARD },  // used in Gaau Hok Gwong Cheung (support missing atm)
	{ "UNL-LH10",         UNSUPPORTED_BOARD },  // used in Fuuun Shaolin Kyo (FDS Conversion) (support missing atm)
//
	{ "BTL-SMB2A",         BTL_SMB2A },
	{ "BTL-MARIOBABY",     BTL_MARIOBABY },
	{ "BTL-AISENSHINICOL", BTL_AISENSHINICOL },
	{ "BTL-SMB2B",         BTL_SMB2B },
	{ "BTL-SMB3",          BTL_SMB3 },
	{ "BTL-SUPERBROS11",   BTL_SUPERBROS11 },
	{ "BTL-DRAGONNINJA",   BTL_DRAGONNINJA },
	{ "BTL-MARIO1-MALEE2", UNSUPPORTED_BOARD }, // mapper 55?
	{ "BTL-2708",          UNSUPPORTED_BOARD },// mapper 103
	{ "BTL-TOBIDASEDAISAKUSEN", BTL_TOBIDASE },// mapper 120
	{ "BTL-SHUIGUANPIPE",  UNSUPPORTED_BOARD },// mapper 183
	{ "BTL-PIKACHUY2K",    BTL_PIKACHUY2K },// mapper 254
//
	{ "BMC-190IN1",          BMC_190IN1 },
	{ "BMC-64IN1NOREPEAT",   BMC_64IN1NR },
	{ "BMC-A65AS",           BMC_A65AS },
	{ "BMC-GS-2004",         BMC_GS2004 },
	{ "BMC-GS-2013",         BMC_GS2013 },
	{ "BMC-NOVELDIAMOND9999999IN1", BMC_NOVELDIAMOND },
	{ "BMC-9999999IN1",      BMC_9999999IN1 }, // mapper 213... same as BMC-NOVELDIAMOND9999999IN1 ??
	{ "BMC-SUPER24IN1SC03",  BMC_S24IN1SC03 },
	{ "BMC-SUPERHIK8IN1",    BMC_HIK8IN1 },
	{ "BMC-T-262",           BMC_T262 },
	{ "BMC-WS",              BMC_WS },
	{ "MLT-ACTION52",        ACTENT_ACT52 },
	{ "MLT-CALTRON6IN1",     CALTRON_6IN1 },
	{ "MLT-MAXI15",          UNSUPPORTED_BOARD}, //  mapper 234
	{ "BMC-SUPERBIG-7IN1",   BMC_SUPERBIG_7IN1 },
	{ "BMC-SUPERHIK-4IN1",   BMC_SUPERHIK_4IN1 },
	{ "BMC-BALLGAMES-11IN1", BMC_BALLGAMES_11IN1 },
	{ "BMC-MARIOPARTY-7IN1", BMC_MARIOPARTY_7IN1 },
	{ "BMC-GOLD-7IN1",       BMC_GOLD_7IN1 },
	{ "BMC-GKA",             BMC_GKA },
	{ "BMC-GKB",             BMC_GKB },
	{ "BMC-SUPER700IN1",     BMC_SUPER_700IN1 },
	{ "BMC-FAMILY-4646B",    BMC_FAMILY_4646B },
	{ "BMC-36IN1",           BMC_36IN1 },
	{ "BMC-21IN1",           BMC_21IN1 },
	{ "BMC-150IN1",          BMC_150IN1 },
	{ "BMC-35IN1",           BMC_35IN1 },
	{ "BMC-64IN1",           BMC_64IN1 },
	{ "BMC-15IN1",           BMC_15IN1 },
	{ "BMC-SUPERHIK-300IN1", BMC_SUPERHIK_300IN1 },
	{ "BMC-SUPERGUN-20IN1",  BMC_SUPERGUN_20IN1 },
	{ "BMC-GOLDENCARD-6IN1", BMC_GOLDENCARD_6IN1 },
	{ "BMC-72IN1",           BMC_72IN1 },
	{ "BMC-76IN1",           BMC_76IN1 },
	{ "BMC-SUPER42IN1",      BMC_SUPER_42IN1 },
	{ "BMC-1200IN1",         BMC_1200IN1 },
	{ "BMC-31IN1",           BMC_31IN1 },
	{ "BMC-22GAMES",         BMC_22GAMES },
	{ "BMC-20IN1",           BMC_20IN1 },
	{ "BMC-110IN1",          BMC_110IN1 },
	{ "BMC-810544-C-A1",     BMC_810544 },
	{ "BMC-411120-C",        UNSUPPORTED_BOARD },
	{ "BMC-830118C",         UNSUPPORTED_BOARD },
	{ "BMC-12-IN-1",         UNSUPPORTED_BOARD },
	{ "BMC-NTD-03",          BMC_NTD_03 },
	{ "BMC-8157",            UNSUPPORTED_BOARD /*BMC_8157*/ },
	{ "BMC-BS-5",            BMC_BENSHENG_BS5 },
	{ "BMC-FK23C",           BMC_FK23C },
	{ "BMC-FK23CA",          BMC_FK23CA },
	{ "BMC-GHOSTBUSTERS63IN1", BMC_G63IN1 },
	{ "BMC-SUPERVISION16IN1", UNSUPPORTED_BOARD },  // mapper 53
	{ "BMC-RESETBASED-4IN1", UNSUPPORTED_BOARD },// mapper 60 with 64k prg and 32k chr
	{ "BMC-VT5201",          BMC_VT5201 },// mapper 60 otherwise
	{ "BMC-D1038",           BMC_VT5201 }, // mapper 60?
	{ "BMC-42IN1RESETSWITCH", UNSUPPORTED_BOARD },  // mapper 60?
	{ "BMC-SUPER22GAMES",    UNSUPPORTED_BOARD },// mapper 233
	{ "BMC-GOLDENGAME-150IN1", UNSUPPORTED_BOARD },// mapper 235 with 2M PRG
	{ "BMC-GOLDENGAME-260IN1", UNSUPPORTED_BOARD },// mapper 235 with 4M PRG
	{ "BMC-70IN1",           UNSUPPORTED_BOARD },   // mapper 236?
	{ "BMC-70IN1B",          UNSUPPORTED_BOARD },   // mapper 236?
	{ "BMC-SUPERHIK-KOF",    UNSUPPORTED_BOARD },// mapper 251
	{ "BMC-POWERJOY",        BMC_PJOY84 },
	{ "BMC-POWERFUL-255",    UNSUPPORTED_BOARD },   // mapper 63?
	{ "UNL-AC08",            UNSUPPORTED_BOARD }, //  used by Green Beret FDS conversions
	{ "UNL-BB",              UNSUPPORTED_BOARD }, //  used by a few FDS conversions
	{ "UNL-LH32",            UNSUPPORTED_BOARD }, //  used by Monty no Doki Doki Daidassou FDS conversion
	{ "UNL-CITYFIGHT",       UNSUPPORTED_BOARD }, //  used by City Fighter IV
	{ "BMC-G-146",           UNSUPPORTED_BOARD }, // multigame mapper
	{ "BMC-11160",           UNSUPPORTED_BOARD }, // multigame mapper
// are there dumps of games with these boards?
	{ "BMC-13IN1JY110",   UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
	{ "BMC-GK-192",       UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
	{ "KONAMI-QTAI",      UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
	{ "UNL-3D-BLOCK",     UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
	{ "UNL-C-N22M",       UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
	{ "UNL-PEC-586",      UNSUPPORTED_BOARD }, //  [mentioned in FCEUMM source - we need more info]
//
	{ "UNKNOWN",          UNKNOWN_BOARD }  //  a few pirate dumps uses the wrong mapper...
};

const nes_pcb *nes_pcb_lookup( const char *board )
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, board))
			return &pcb_list[i];
	}
	return NULL;
}

int nes_get_pcb_id( running_machine &machine, const char *feature )
{
	const nes_pcb *pcb = nes_pcb_lookup(feature);

	if (pcb == NULL)
		fatalerror("Unimplemented PCB type %s\n", feature);

	return pcb->pcb_id;
}

/************************************************

   PCB Emulation (to be moved to MAME later)

   In the end, iNES, UNIF and xml should be
   simply handled through a look-up table
   which associates the mapper/board/feature
   to the correct pcb_id and then the core
   function pcb_handlers_setup should be called
   to set-up the expected handlers and callbacks

   Similarly, PC-10, VSNES and NES-based
   multigame boards, should simply call
   pcb_handlers_setup with the proper pcb_id
   at the beginning, rather than use ad hoc
   implementations.

************************************************/

// helper function for the few mappers reading from 0x8000-0xffff for protection
INLINE UINT8 mmc_hi_access_rom( running_machine &machine, UINT32 offset )
{
	nes_state *state = machine.driver_data<nes_state>();

	// usual ROM access
	switch (offset & 0x6000)
	{
		case 0x0000:
			return state->m_prg[state->m_prg_bank[0] * 0x2000 + (offset & 0x1fff)];
		case 0x2000:
			return state->m_prg[state->m_prg_bank[1] * 0x2000 + (offset & 0x1fff)];
		case 0x4000:
			return state->m_prg[state->m_prg_bank[2] * 0x2000 + (offset & 0x1fff)];
		case 0x6000:
			return state->m_prg[state->m_prg_bank[3] * 0x2000 + (offset & 0x1fff)];
	}
	return 0;
}

/*************************************************************

 NROM board emulation

 Games: Mario Bros., Super Mario Bros., Tennis and most of
 the first generation games

 iNES: mapper 0

 In MESS: Supported, no need of specific handlers or IRQ

 *************************************************************/

/*************************************************************

 UxROM board emulation

 Games: Castlevania, Dragon Quest II, Duck Tales, MegaMan,
 Metal Gear

 writes to 0x8000-0xffff change PRG 16K lower banks

 missing BC?

 iNES: mapper 2

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::uxrom_w)
{
	LOG_MMC(("uxrom_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(data);
}

/*************************************************************

 Nihon Bussan UNROM M5

 Games: Crazy Climber Jpn

 Very simple mapper: prg16_89ab is always set to bank 0,
 while prg16_cdef is set by writes to 0x8000-0xffff. The game
 uses a custom controller.

 iNES: mapper 180

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::uxrom_cc_w)
{
	LOG_MMC(("uxrom_cc_w, offset: %04x, data: %02x\n", offset, data));

	prg16_cdef(data);
}

/*************************************************************

 UN1ROM board emulation

 Games: Senjou no Okami

 writes to 0x8000-0xffff change PRG 16K lower banks

 missing BC?

 iNES: mapper 94

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::un1rom_w)
{
	LOG_MMC(("un1rom_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(data >> 2);
}

/*************************************************************

 CNROM board emulation

 Games: B-Wings, Mighty Bomb Jack, Seicross, Spy vs. Spy,
 Adventure Island, Flipull, Friday 13th, GeGeGe no
 Kitarou, Ghostbusters, Gradius, Hokuto no Ken, Milon's
 Secret Castle

 writes to 0x8000-0xffff change CHR 8K banks

 missing BC?

 iNES: mappers 3 & 185 (the latter for games using Pins as
 protection)

 Notice that BANDAI_PT554 board (Aerobics Studio) uses very
 similar hardware but with an additional sound chip which
 gets writes to 0x6000 (currently unemulated in MESS)

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cnrom_w)
{
	LOG_MMC(("cnrom_w, offset: %04x, data: %02x\n", offset, data));

	if (m_ce_mask)
	{
		chr8(data & ~m_ce_mask, CHRROM);

		if ((data & m_ce_mask) == m_ce_state)
			m_chr_open_bus = 0;
		else
			m_chr_open_bus = 1;
	}
	else
		chr8(data, CHRROM);
}

/*************************************************************

 Bandai PT-554 board emulation

 This is used by Aerobics Studio. It is basically a CNROM board
 with an additional sound chip which is not currently emulated by
 MESS

 iNES: mapper 3?

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bandai_pt554_m_w)
{
	LOG_MMC(("Bandai PT-554 Sound write, data: %02x\n", data));

	// according to NEStopia, this is the effect
	if (!BIT(data, 6))
	{
		// send command (data & 0x07) to sound chip
	}
}

/*************************************************************

 CPROM board emulation

 Games: Videomation

 writes to 0x8000-0xffff change CHR 4K lower banks

 iNES: mapper 13

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cprom_w)
{
	LOG_MMC(("cprom_w, offset: %04x, data: %02x\n", offset, data));
	chr4_4(data, CHRRAM);
}

/*************************************************************

 AxROM board emulation

 Games: Arch Rivals, Battletoads, Cabal, Commando, Solstice

 writes to 0x8000-0xffff change PRG banks + sets mirroring

 missing BC for AMROM?

 iNES: mapper 7

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::axrom_w)
{
	LOG_MMC(("axrom_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	prg32(data);
}

/*************************************************************

 BxROM board emulation

 writes to 0x8000-0xffff change PRG banks

 missing BC?

 iNES: mapper 34

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bxrom_w)
{
	/* This portion of the mapper is nearly identical to Mapper 7, except no one-screen mirroring */
	/* Deadly Towers is really a BxROM game - the demo screens look wrong using mapper 7. */
	LOG_MMC(("bxrom_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
}

/*************************************************************

 GxROM/MxROM board emulation

 writes to 0x8000-0xffff change PRG and CHR banks

 missing BC?

 iNES: mapper 66

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::gxrom_w)
{
	LOG_MMC(("gxrom_w, offset %04x, data: %02x\n", offset, data));

	prg32((data & 0xf0) >> 4);
	chr8(data & 0x0f, CHRROM);
}

/*************************************************************

 SxROM (MMC1 based) board emulation

 iNES: mapper 1 (and 155 for the MMC1A variant which does not
 have WRAM disable bit)

 *************************************************************/

static TIMER_CALLBACK( mmc1_resync_callback )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->m_mmc1_reg_write_enable = 1;
}


static void mmc1_set_wram( address_space &space, int board )
{
	running_machine &machine = space.machine();
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 bank = BIT(state->m_mmc_reg[0], 4) ? BIT(state->m_mmc_reg[1], 4) : BIT(state->m_mmc_reg[1], 3);

	switch (board)
	{
		case STD_SXROM:     // here also reads are disabled!
			if (!BIT(state->m_mmc_reg[3], 4))
				space.install_readwrite_bank(0x6000, 0x7fff, "bank5");
			else
			{
				space.unmap_readwrite(0x6000, 0x7fff);
				break;
			}
		case STD_SXROM_A:   // ignore WRAM enable bit
			if (state->m_battery_size > 0x2000)
				state->wram_bank(((state->m_mmc_reg[1] & 3) >> 2), NES_BATTERY);
			else if (state->m_battery_size)
				state->wram_bank(0, NES_BATTERY);
			break;
		case STD_SOROM:     // there are 2 WRAM banks only and battery is bank 2 for the cart (hence, we invert bank, because we have battery first)
			if (!BIT(state->m_mmc_reg[3], 4))
				space.install_readwrite_bank(0x6000, 0x7fff, "bank5");
			else
			{
				space.unmap_readwrite(0x6000, 0x7fff);
				break;
			}
		case STD_SOROM_A:   // ignore WRAM enable bit
			state->wram_bank(0, bank ? NES_BATTERY : NES_WRAM);
			break;
	}
}

static void mmc1_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 prg_mode, prg_offset;

	prg_mode = state->m_mmc_reg[0] & 0x0c;
	/* prg_mode&0x8 determines bank size: 32k (if 0) or 16k (if 1)? when in 16k mode,
	 prg_mode&0x4 determines which half of the PRG space we can swap: if it is 4,
	 mmc_reg[3] sets banks at 0x8000; if it is 0, mmc_reg[3] sets banks at 0xc000. */

	prg_offset = state->m_mmc_reg[1] & 0x10;
	/* In principle, mmc_reg[2]&0x10 might affect "extended" banks as well, when chr_mode=1.
	 However, quoting Disch's docs: When in 4k CHR mode, 0x10 in both $A000 and $C000 *must* be
	 set to the same value, or else pages will constantly be swapped as graphics render!
	 Hence, we use only mmc_reg[1]&0x10 for prg_offset */

	switch (prg_mode)
	{
		case 0x00:
		case 0x04:
//          printf("PRG 32 bank %d \n", (prg_offset + state->m_mmc_reg[3]) >> 1);
			state->prg32((prg_offset + state->m_mmc_reg[3]) >> 1);
			break;
		case 0x08:
//          printf("PRG 16 bank %d (high) \n", prg_offset + state->m_mmc_reg[3]);
			state->prg16_89ab(prg_offset + 0);
			state->prg16_cdef(prg_offset + state->m_mmc_reg[3]);
			break;
		case 0x0c:
//          printf("PRG 16 bank %d (low) \n", prg_offset + state->m_mmc_reg[3]);
			state->prg16_89ab(prg_offset + state->m_mmc_reg[3]);
			state->prg16_cdef(prg_offset + 0x0f);
			break;
	}
}

static void mmc1_set_prg_wram( address_space &space, int board )
{
	mmc1_set_prg(space.machine());
	mmc1_set_wram(space, board);
}

static void mmc1_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_mode = BIT(state->m_mmc_reg[0], 4);

	if (chr_mode)
	{
		state->chr4_0(state->m_mmc_reg[1] & 0x1f, state->m_mmc_chr_source);
		state->chr4_4(state->m_mmc_reg[2] & 0x1f, state->m_mmc_chr_source);
	}
	else
		state->chr8((state->m_mmc_reg[1] & 0x1f) >> 1, state->m_mmc_chr_source);
}

static void common_sxrom_write_handler( address_space &space, offs_t offset, UINT8 data, int board )
{
	running_machine &machine = space.machine();
	nes_state *state = machine.driver_data<nes_state>();
	/* Note that there is only one latch and shift counter, shared amongst the 4 regs */
	/* Space Shuttle will not work if they have independent variables. */

	/* here we would need to add an if(cpu_cycles_passed>1) test, and
	 if requirement is not met simply return without writing anything.
	 Some games (AD&D Hillsfar, Bill & Ted Excellent Adventure, Cosmic
	 Wars, Rocket Ranger, Sesame Street 123 and Snow Brothers) rely on
	 this behavior!! */
	if (state->m_mmc1_reg_write_enable == 0)
	{
		return;
	}
	else
	{
		state->m_mmc1_reg_write_enable = 0;
		machine.scheduler().synchronize(FUNC(mmc1_resync_callback));
	}

	if (data & 0x80)
	{
		state->m_mmc1_count = 0;
		state->m_mmc1_latch = 0;

		/* Set reg at 0x8000 to size 16k and lower half swap - needed for Robocop 3, Dynowars */
		state->m_mmc_reg[0] |= 0x0c;
		mmc1_set_prg_wram(space, board);
		return;
	}

	if (state->m_mmc1_count < 5)
	{
		if (state->m_mmc1_count == 0) state->m_mmc1_latch = 0;
		state->m_mmc1_latch >>= 1;
		state->m_mmc1_latch |= (data & 0x01) ? 0x10 : 0x00;
		state->m_mmc1_count++;
	}

	if (state->m_mmc1_count == 5)
	{
		switch (offset & 0x6000)    /* Which reg shall we write to? */
		{
			case 0x0000:
				state->m_mmc_reg[0] = state->m_mmc1_latch;

				switch (state->m_mmc_reg[0] & 0x03)
				{
				case 0: state->set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 1: state->set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 2: state->set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 3: state->set_nt_mirroring(PPU_MIRROR_HORZ); break;
				}
				mmc1_set_chr(machine);
				mmc1_set_prg_wram(space, board);
				break;
			case 0x2000:
				state->m_mmc_reg[1] = state->m_mmc1_latch;
				mmc1_set_chr(machine);
				mmc1_set_prg_wram(space, board);
				break;
			case 0x4000:
				state->m_mmc_reg[2] = state->m_mmc1_latch;
				mmc1_set_chr(machine);
				break;
			case 0x6000:
				state->m_mmc_reg[3] = state->m_mmc1_latch;
				mmc1_set_prg_wram(space, board);
				break;
		}
		state->m_mmc1_count = 0;
	}
}

WRITE8_MEMBER(nes_carts_state::sxrom_w)
{
	LOG_MMC(("sxrom_w, offset: %04x, data: %02x\n", offset, data));
	common_sxrom_write_handler(space, offset, data, m_pcb_id);
}

/*************************************************************

 PxROM (MMC2 based) board emulation

 Games: Punch Out!!, Mike Tyson's Punch Out!!

 iNES: mapper 9

 In MESS: Supported

 *************************************************************/

static void mmc2_latch( device_t *device, offs_t offset )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if ((offset & 0x3ff0) == 0x0fd0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 low): %02x\n", state->m_mmc_reg[0]));
		state->m_mmc_latch1 = 0xfd;
		state->chr4_0(state->m_mmc_reg[0], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x0fe0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 high): %02x\n", state->m_mmc_reg[1]));
		state->m_mmc_latch1 = 0xfe;
		state->chr4_0(state->m_mmc_reg[1], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x1fd0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 1 low): %02x\n", state->m_mmc_reg[2]));
		state->m_mmc_latch2 = 0xfd;
		state->chr4_4(state->m_mmc_reg[2], CHRROM);
	}
	else if ((offset & 0x3ff0) == 0x1fe0)
	{
		LOG_MMC(("mmc2 vrom latch switch (bank 0 high): %02x\n", state->m_mmc_reg[3]));
		state->m_mmc_latch2 = 0xfe;
		state->chr4_4(state->m_mmc_reg[3], CHRROM);
	}
}

WRITE8_MEMBER(nes_carts_state::pxrom_w)
{
	LOG_MMC(("pxrom_w, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x7000)
	{
		case 0x2000:
			prg8_89(data);
			break;
		case 0x3000:
			m_mmc_reg[0] = data;
			if (m_mmc_latch1 == 0xfd)
				chr4_0(m_mmc_reg[0], CHRROM);
			break;
		case 0x4000:
			m_mmc_reg[1] = data;
			if (m_mmc_latch1 == 0xfe)
				chr4_0(m_mmc_reg[1], CHRROM);
			break;
		case 0x5000:
			m_mmc_reg[2] = data;
			if (m_mmc_latch2 == 0xfd)
				chr4_4(m_mmc_reg[2], CHRROM);
			break;
		case 0x6000:
			m_mmc_reg[3] = data;
			if (m_mmc_latch2 == 0xfe)
				chr4_4(m_mmc_reg[3], CHRROM);
			break;
		case 0x7000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		default:
			LOG_MMC(("MMC2 uncaught w: %04x:%02x\n", offset, data));
			break;
	}
}

/*************************************************************

 FxROM (MMC4 based) board emulation

 Games: Famicom Wars, Fire Emblem, Fire Emblem Gaiden

 iNES: mapper 10

 In MESS: Supported

*************************************************************/

WRITE8_MEMBER(nes_carts_state::fxrom_w)
{
	LOG_MMC(("fxrom_w, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x7000)
	{
		case 0x2000:
			prg16_89ab(data);
			break;
		default:
			pxrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 TxROM (MMC3 based) board emulation

 iNES: mapper 4

 *************************************************************/

static void mmc3_set_wram( address_space &space )
{
	running_machine &machine = space.machine();
	nes_state *state = machine.driver_data<nes_state>();

	// skip this function if we are emulating a MMC3 clone with mid writes
	if (!state->m_mmc_write_mid.isnull())
		return;

	if (BIT(state->m_mmc3_wram_protect, 7))
		space.install_readwrite_bank(0x6000, 0x7fff, "bank5");
	else
	{
		space.unmap_readwrite(0x6000, 0x7fff);
		return;
	}

	if (!BIT(state->m_mmc3_wram_protect, 6))
		space.install_write_bank(0x6000, 0x7fff, "bank5");
	else
	{
		space.unmap_write(0x6000, 0x7fff);
		return;
	}
}

// base MMC3 simply calls prg8_x
static void mmc3_base_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->prg8_x(start, bank);
}

// base MMC3 simply calls chr1_x
static void mmc3_base_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->chr1_x(start, bank, source);
}


static void mmc3_set_prg( running_machine &machine, int prg_base, int prg_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 prg_flip = (state->m_mmc3_latch & 0x40) ? 2 : 0;

	state->m_mmc3_prg_cb(machine, 0, prg_base | (state->m_mmc_prg_bank[0 ^ prg_flip] & prg_mask));
	state->m_mmc3_prg_cb(machine, 1, prg_base | (state->m_mmc_prg_bank[1] & prg_mask));
	state->m_mmc3_prg_cb(machine, 2, prg_base | (state->m_mmc_prg_bank[2 ^ prg_flip] & prg_mask));
	state->m_mmc3_prg_cb(machine, 3, prg_base | (state->m_mmc_prg_bank[3] & prg_mask));
}

static void mmc3_set_chr( running_machine &machine, UINT8 chr, int chr_base, int chr_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc3_latch & 0x80) >> 5;

	state->m_mmc3_chr_cb(machine, chr_page ^ 0, chr_base | ((state->m_mmc_vrom_bank[0] & ~0x01) & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 1, chr_base | ((state->m_mmc_vrom_bank[0] |  0x01) & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 2, chr_base | ((state->m_mmc_vrom_bank[1] & ~0x01) & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 3, chr_base | ((state->m_mmc_vrom_bank[1] |  0x01) & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 4, chr_base | (state->m_mmc_vrom_bank[2] & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 5, chr_base | (state->m_mmc_vrom_bank[3] & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 6, chr_base | (state->m_mmc_vrom_bank[4] & chr_mask), chr);
	state->m_mmc3_chr_cb(machine, chr_page ^ 7, chr_base | (state->m_mmc_vrom_bank[5] & chr_mask), chr);
}

/* Here, IRQ counter decrements every scanline. */
static void mmc3_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int priorCount = state->m_IRQ_count;
		if ((state->m_IRQ_count == 0) || state->m_IRQ_clear)
			state->m_IRQ_count = state->m_IRQ_count_latch;
		else
			state->m_IRQ_count--;

		if (state->m_IRQ_enable && !blanked && (state->m_IRQ_count == 0) && (priorCount || state->m_IRQ_clear /*|| !state->m_mmc3_alt_irq*/)) // according to blargg the latter should be present as well, but it breaks Rampart and Joe & Mac US: they probably use the alt irq!
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						device->machine().primary_screen->vpos(), device->machine().primary_screen->hpos()));
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
	state->m_IRQ_clear = 0;
}

WRITE8_MEMBER(nes_carts_state::txrom_w)
{
	UINT8 mmc_helper, cmd;

	LOG_MMC(("txrom_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_mmc3_latch ^ data;
			m_mmc3_latch = data;

			/* Has PRG Mode changed? */
			if (mmc_helper & 0x40)
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			/* Has CHR Mode changed? */
			if (mmc_helper & 0x80)
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

		case 0x0001:
			cmd = m_mmc3_latch & 0x07;
			switch (cmd)
			{
			case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
			case 2: case 3: case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
				break;
			case 6:
			case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;
			}
			break;

		case 0x2000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2001:
			m_mmc3_wram_protect = data;
			mmc3_set_wram(space);
			break;

		case 0x4000:
			m_IRQ_count_latch = data;
			break;

		case 0x4001:
			m_IRQ_count = 0;
			break;

		case 0x6000:
			m_IRQ_enable = 0;
			break;

		case 0x6001:
			m_IRQ_enable = 1;
			break;

		default:
			logerror("txrom_w uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*************************************************************

 HKROM (MMC6 based) board emulation

 iNES: mapper 4

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::hkrom_m_w)
{
	UINT8 write_hi, write_lo;
	LOG_MMC(("hkrom_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
		return;

	// banks can be written only if both read & write is enabled!
	write_hi = ((m_mmc6_reg & 0xc0) == 0xc0);
	write_lo = ((m_mmc6_reg & 0x30) == 0x30);

	if (BIT(offset, 9) && write_hi) // access to upper 1k
		m_mapper_bram[offset & (m_mapper_bram_size - 1)] = data;

	if (!BIT(offset, 9) && write_lo)    // access to lower 1k
		m_mapper_bram[offset & (m_mapper_bram_size - 1)] = data;
}

READ8_MEMBER(nes_carts_state::hkrom_m_r)
{
	LOG_MMC(("hkrom_m_r, offset: %04x\n", offset));

	if (offset < 0x1000)
		return 0xff;    // here it should be open bus

	if (!(m_mmc6_reg & 0xa0))
		return 0xff;    // here it should be open bus

	if (BIT(offset, 9) && BIT(m_mmc6_reg, 7))   // access to upper 1k when upper read is enabled
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];

	if (!BIT(offset, 9) && BIT(m_mmc6_reg, 5))  // access to lower 1k when lower read is enabled
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];

	// If only one bank is enabled for reading, the other reads back as zero
	return 0x00;
}

WRITE8_MEMBER(nes_carts_state::hkrom_w)
{
	UINT8 mmc6_helper;
	LOG_MMC(("hkrom_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc6_helper = m_mmc3_latch ^ data;
			m_mmc3_latch = data;

			if (!m_mmc_latch2 && BIT(data, 5))  // if WRAM is disabled and has to be enabled, write
				m_mmc_latch2 = BIT(data, 5);    // (once WRAM has been enabled, it cannot be disabled without resetting the game)

			/* Has PRG Mode changed? */
			if (BIT(mmc6_helper, 6))
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			/* Has CHR Mode changed? */
			if (BIT(mmc6_helper, 7))
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

		case 0x2001:
			if (m_mmc_latch2)
				m_mmc6_reg = data;
			break;

		case 0x4001:
			m_IRQ_count = 0;
			m_IRQ_clear = 1;
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 TxSROM (MMC3 based) board emulation

 Games: Armadillo, Play Action Football, Pro Hockey, RPG
 Jinsei Game, Y's 3

 iNES: mapper 118

 In MESS: Supported. It also uses mmc3_irq.

 *************************************************************/

static void txsrom_set_mirror( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (state->m_mmc3_latch & 0x80)
	{
		state->set_nt_page(0, CIRAM, (state->m_mmc_vrom_bank[2] & 0x80) >> 7, 1);
		state->set_nt_page(1, CIRAM, (state->m_mmc_vrom_bank[3] & 0x80) >> 7, 1);
		state->set_nt_page(2, CIRAM, (state->m_mmc_vrom_bank[4] & 0x80) >> 7, 1);
		state->set_nt_page(3, CIRAM, (state->m_mmc_vrom_bank[5] & 0x80) >> 7, 1);
	}
	else
	{
		state->set_nt_page(0, CIRAM, (state->m_mmc_vrom_bank[0] & 0x80) >> 7, 1);
		state->set_nt_page(1, CIRAM, (state->m_mmc_vrom_bank[0] & 0x80) >> 7, 1);
		state->set_nt_page(2, CIRAM, (state->m_mmc_vrom_bank[1] & 0x80) >> 7, 1);
		state->set_nt_page(3, CIRAM, (state->m_mmc_vrom_bank[1] & 0x80) >> 7, 1);
	}
}

static void txsrom_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	txsrom_set_mirror(machine); // we could probably update only for one (e.g. the first) call, to slightly optimize the code
	state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::txsrom_w)
{
	LOG_MMC(("txsrom_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 TQROM (MMC3 based) board emulation

 Games: Pin Bot, High Speed

 iNES: mapper 119

 In MESS: Supported. It also uses mmc3_irq.

 *************************************************************/

static void tqrom_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc3_latch & 0x80) >> 5;
	UINT8 chr_src[6], chr_mask[6];
	int i;

	for (i = 0; i < 6; i++)
	{
		chr_src[i] = (state->m_mmc_vrom_bank[i] & 0x40) ? CHRRAM : CHRROM;
		chr_mask[i] =  (state->m_mmc_vrom_bank[i] & 0x40) ? 0x07 : 0x3f;
	}

	state->chr1_x(chr_page ^ 0, ((state->m_mmc_vrom_bank[0] & ~0x01) & chr_mask[0]), chr_src[0]);
	state->chr1_x(chr_page ^ 1, ((state->m_mmc_vrom_bank[0] |  0x01) & chr_mask[0]), chr_src[0]);
	state->chr1_x(chr_page ^ 2, ((state->m_mmc_vrom_bank[1] & ~0x01) & chr_mask[1]), chr_src[1]);
	state->chr1_x(chr_page ^ 3, ((state->m_mmc_vrom_bank[1] |  0x01) & chr_mask[1]), chr_src[1]);
	state->chr1_x(chr_page ^ 4, (state->m_mmc_vrom_bank[2] & chr_mask[2]), chr_src[2]);
	state->chr1_x(chr_page ^ 5, (state->m_mmc_vrom_bank[3] & chr_mask[3]), chr_src[3]);
	state->chr1_x(chr_page ^ 6, (state->m_mmc_vrom_bank[4] & chr_mask[4]), chr_src[4]);
	state->chr1_x(chr_page ^ 7, (state->m_mmc_vrom_bank[5] & chr_mask[5]), chr_src[5]);
}

WRITE8_MEMBER(nes_carts_state::tqrom_w)
{
	UINT8 mmc_helper, cmd;
	LOG_MMC(("tqrom_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_mmc3_latch ^ data;
			m_mmc3_latch = data;

			/* Has PRG Mode changed? */
			if (mmc_helper & 0x40)
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			/* Has CHR Mode changed? */
			if (mmc_helper & 0x80)
				tqrom_set_chr(machine());
			break;
		case 0x0001: /* $8001 */
			cmd = m_mmc3_latch & 0x07;
			switch (cmd)
			{
			case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
			case 2: case 3: case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				tqrom_set_chr(machine());
				break;
			case 6:
			case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;
			}
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 PAL-ZZ board (MMC3 variant for European 3-in-1 Nintendo cart
 Super Mario Bros. + Tetris + Nintendo World Cup)

 iNES: mapper 37

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::zz_m_w)
{
	UINT8 mmc_helper = data & 0x07;
	LOG_MMC(("zz_m_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_prg_base = (BIT(mmc_helper, 2) << 4) | (((mmc_helper & 0x03) == 0x03) ? 0x08 : 0);
	m_mmc_prg_mask = (mmc_helper << 1) | 0x07;
	m_mmc_chr_base = BIT(mmc_helper, 2) << 7;
	m_mmc_chr_mask = 0x7f;
	mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
}

/*************************************************************

 NES-QJ board (MMC3 variant for US 2-in-1 Nintendo cart
 Super Spike V'Ball + Nintendo World Cup)

 iNES: mapper 47

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::qj_m_w)
{
	LOG_MMC(("qj_m_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_prg_base = BIT(data, 0) << 4;
	m_mmc_prg_mask = 0x0f;
	m_mmc_chr_base = BIT(data, 0) << 7;
	m_mmc_chr_mask = 0x7f;
	mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
}

/*************************************************************

 ExROM (MMC5 based) board emulation

 Games: Castlevania III, Just Breed, many Koei titles

 iNES: mapper 5

 MESS status: Mostly Unsupported

 *************************************************************/

#if 0
static void mmc5_update_chr_a( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (state->m_mmc5_chr_mode)
	{
		case 0: // 8k banks
			state->chr8(state->m_mmc5_vrom_regA[7] & 0xff, CHRROM);
			break;

		case 1: // 4k banks
			state->chr4_0(state->m_mmc5_vrom_regA[3] & 0xff, CHRROM);
			state->chr4_4(state->m_mmc5_vrom_regA[7] & 0xff, CHRROM);
			break;

		case 2: // 2k banks
			state->chr2_0(state->m_mmc5_vrom_regA[1], CHRROM);
			state->chr2_2(state->m_mmc5_vrom_regA[3], CHRROM);
			state->chr2_4(state->m_mmc5_vrom_regA[5], CHRROM);
			state->chr2_6(state->m_mmc5_vrom_regA[7], CHRROM);
			break;

		case 3: // 1k banks
			state->chr1_0(state->m_mmc5_vrom_regA[0], CHRROM);
			state->chr1_1(state->m_mmc5_vrom_regA[1], CHRROM);
			state->chr1_2(state->m_mmc5_vrom_regA[2], CHRROM);
			state->chr1_3(state->m_mmc5_vrom_regA[3], CHRROM);
			state->chr1_4(state->m_mmc5_vrom_regA[4], CHRROM);
			state->chr1_5(state->m_mmc5_vrom_regA[5], CHRROM);
			state->chr1_6(state->m_mmc5_vrom_regA[6], CHRROM);
			state->chr1_7(state->m_mmc5_vrom_regA[7], CHRROM);
			break;
	}
}

static void mmc5_update_chr_b( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (state->m_mmc5_chr_mode)
	{
		case 0: // 8k banks
			state->chr8(state->m_mmc5_vrom_regB[3] & 0xff, CHRROM);
			break;

		case 1: // 4k banks
			state->chr4_0(state->m_mmc5_vrom_regB[3] & 0xff, CHRROM);
			state->chr4_4(state->m_mmc5_vrom_regB[3] & 0xff, CHRROM);
			break;

		case 2: // 2k banks
			state->chr2_0(state->m_mmc5_vrom_regB[1], CHRROM);
			state->chr2_2(state->m_mmc5_vrom_regB[3], CHRROM);
			state->chr2_4(state->m_mmc5_vrom_regB[1], CHRROM);
			state->chr2_6(state->m_mmc5_vrom_regB[3], CHRROM);
			break;

		case 3: // 1k banks
			state->chr1_0(state->m_mmc5_vrom_regB[0], CHRROM);
			state->chr1_1(state->m_mmc5_vrom_regB[1], CHRROM);
			state->chr1_2(state->m_mmc5_vrom_regB[2], CHRROM);
			state->chr1_3(state->m_mmc5_vrom_regB[3], CHRROM);
			state->chr1_4(state->m_mmc5_vrom_regB[0], CHRROM);
			state->chr1_5(state->m_mmc5_vrom_regB[1], CHRROM);
			state->chr1_6(state->m_mmc5_vrom_regB[2], CHRROM);
			state->chr1_7(state->m_mmc5_vrom_regB[3], CHRROM);
			break;
	}
}
#endif

static void mmc5_update_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	int bank1, bank2, bank3;

	switch (state->m_mmc5_prg_mode)
	{
		case 0: // 32k banks
			state->prg32(state->m_mmc5_prg_regs[3] >> 2);
			break;

		case 1: // 16k banks
			//          printf("problema 1: %x\n", state->m_mmc5_prg_regs[1]);
			bank1 = state->m_mmc5_prg_regs[1];

			if (!BIT(bank1, 7))
			{
				state->m_prg_bank[0] = state->m_prg_chunks + (bank1 & 0x06);
				state->m_prg_bank[1] = state->m_prg_chunks + (bank1 & 0x06) + 1;
				state->membank("bank1")->set_entry(state->m_prg_bank[0]);
				state->membank("bank2")->set_entry(state->m_prg_bank[1]);
			}
			else
				state->prg16_89ab(bank1 >> 1);

			state->prg16_cdef(state->m_mmc5_prg_regs[3] >> 1);
			break;

		case 2: // 16k-8k banks
			//          printf("problema 2: %x %x\n", state->m_mmc5_prg_regs[1], state->m_mmc5_prg_regs[2]);
			bank1 = state->m_mmc5_prg_regs[1];
			bank3 = state->m_mmc5_prg_regs[2];

			if (!BIT(bank1, 7))
			{
				state->m_prg_bank[0] = state->m_prg_chunks + (bank1 & 0x06);
				state->m_prg_bank[1] = state->m_prg_chunks + (bank1 & 0x06) + 1;
				state->membank("bank1")->set_entry(state->m_prg_bank[0]);
				state->membank("bank2")->set_entry(state->m_prg_bank[1]);
			}
			else
				state->prg16_89ab((bank1 & 0x7f) >> 1);

			if (!BIT(bank3, 7))
			{
				state->m_prg_bank[2] = state->m_prg_chunks + (bank3 & 0x07);
				state->membank("bank3")->set_entry(state->m_prg_bank[2]);
			}
			else
				state->prg8_cd(bank3 & 0x7f);

			state->prg8_ef(state->m_mmc5_prg_regs[3]);
			break;

		case 3: // 8k banks
			//          printf("problema 3: %x %x %x\n", state->m_mmc5_prg_regs[0], state->m_mmc5_prg_regs[1], state->m_mmc5_prg_regs[2]);
			bank1 = state->m_mmc5_prg_regs[0];
			bank2 = state->m_mmc5_prg_regs[1];
			bank3 = state->m_mmc5_prg_regs[2];

			if (!BIT(bank1, 7))
			{
				state->m_prg_bank[0] = state->m_prg_chunks + (bank1 & 0x07);
				state->membank("bank1")->set_entry(state->m_prg_bank[0]);
			}
			else
				state->prg8_89(bank1 & 0x7f);

			if (!BIT(bank2, 7))
			{
				state->m_prg_bank[1] = state->m_prg_chunks + (bank2 & 0x07);
				state->membank("bank2")->set_entry(state->m_prg_bank[1]);
			}
			else
				state->prg8_ab(bank2 & 0x7f);

			if (!BIT(bank3, 7))
			{
				state->m_prg_bank[2] = state->m_prg_chunks + (bank3 & 0x07);
				state->membank("bank3")->set_entry(state->m_prg_bank[2]);
			}
			else
				state->prg8_cd(bank3 & 0x7f);

			state->prg8_ef(state->m_mmc5_prg_regs[3]);
			break;
	}
}

static void mmc5_update_render_mode( running_machine &machine )
{
}

static void mmc5_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

#if 1
	if (scanline == 0)
		state->m_IRQ_status |= 0x40;
	else if (scanline > PPU_BOTTOM_VISIBLE_SCANLINE)
		state->m_IRQ_status &= ~0x40;
#endif

	if (scanline == state->m_IRQ_count)
	{
		if (state->m_IRQ_enable)
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);

		state->m_IRQ_status = 0xff;
	}

	/* FIXME: this is ok, but then we would need to update them again when we have the BG Hblank
	 I leave it commented out until the PPU is updated for this */
	//  if (ppu2c0x_is_sprite_8x16(state->m_ppu) || state->m_mmc5_last_chr_a)
	//      mmc5_update_chr_a(device->machine());
	//  else
	//      mmc5_update_chr_b(device->machine());
}

static void mmc5_ppu_mirror( running_machine &machine, int page, int src )
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (src)
	{
		case 0: /* CIRAM0 */
			state->set_nt_page(page, CIRAM, 0, 1);
			break;
		case 1: /* CIRAM1 */
			state->set_nt_page(page, CIRAM, 1, 1);
			break;
		case 2: /* ExRAM */
			state->set_nt_page(page, EXRAM, 0, 1);  // actually only works during rendering.
			break;
		case 3: /* Fill Registers */
			state->set_nt_page(page, MMC5FILL, 0, 0);
			break;
		default:
			fatalerror("This should never happen\n");
			break;
	}
}

READ8_MEMBER(nes_carts_state::exrom_l_r)
{
	int retVal;

	/* $5c00 - $5fff: extended videoram attributes */
	if ((offset >= 0x1b00) && (offset <= 0x1eff))
	{
		return m_mapper_ram[offset - 0x1b00];
	}

	switch (offset)
	{
		case 0x1104: /* $5204 */
#if 0
			if (current_scanline == MMC5_scanline)
				return 0x80;
			else
				return 0x00;
#else
			retVal = m_IRQ_status;
			m_IRQ_status &= ~0x80;
			return retVal;
#endif

		case 0x1105: /* $5205 */
			return (m_mult1 * m_mult2) & 0xff;
		case 0x1106: /* $5206 */
			return ((m_mult1 * m_mult2) & 0xff00) >> 8;

		default:
			logerror("** MMC5 uncaught read, offset: %04x\n", offset + 0x4100);
			return 0x00;
	}
}


WRITE8_MEMBER(nes_carts_state::exrom_l_w)
{
	//  LOG_MMC(("Mapper 5 write, offset: %04x, data: %02x\n", offset + 0x4100, data));
	/* Send $5000-$5015 to the sound chip */
	if ((offset >= 0xf00) && (offset <= 0xf15))
	{
		nes_psg_w(m_sound, space, offset & 0x1f, data);
		return;
	}

	/* $5c00 - $5fff: extended videoram attributes */
	if ((offset >= 0x1b00) && (offset <= 0x1eff))
	{
		if (m_MMC5_vram_protect == 0x03)
			m_mapper_ram[offset - 0x1b00] = data;
		return;
	}

	switch (offset)
	{
		case 0x1000: /* $5100 */
			m_mmc5_prg_mode = data & 0x03;
			//          mmc5_update_prg(machine());
			LOG_MMC(("MMC5 rom bank mode: %02x\n", data));
			break;

		case 0x1001: /* $5101 */
			m_mmc5_chr_mode = data & 0x03;
			// update chr
			LOG_MMC(("MMC5 vrom bank mode: %02x\n", data));
			break;

		case 0x1002: /* $5102 */
			if (data == 0x02)
				m_MMC5_vram_protect |= 1;
			else
				m_MMC5_vram_protect = 0;
			LOG_MMC(("MMC5 vram protect 1: %02x\n", data));
			break;
		case 0x1003: /* 5103 */
			if (data == 0x01)
				m_MMC5_vram_protect |= 2;
			else
				m_MMC5_vram_protect = 0;
			LOG_MMC(("MMC5 vram protect 2: %02x\n", data));
			break;

		case 0x1004: /* $5104 - Extra VRAM (EXRAM) control */
			m_mmc5_vram_control = data & 0x03;
			// update render
			mmc5_update_render_mode(machine());
			LOG_MMC(("MMC5 exram control: %02x\n", data));
			break;

		case 0x1005: /* $5105 */
			mmc5_ppu_mirror(machine(), 0, data & 0x03);
			mmc5_ppu_mirror(machine(), 1, (data & 0x0c) >> 2);
			mmc5_ppu_mirror(machine(), 2, (data & 0x30) >> 4);
			mmc5_ppu_mirror(machine(), 3, (data & 0xc0) >> 6);
			// update render
			mmc5_update_render_mode(machine());
			break;

			/* tile data for MMC5 flood-fill NT mode */
		case 0x1006:
			m_MMC5_floodtile = data;
			break;

			/* attr data for MMC5 flood-fill NT mode */
		case 0x1007:
			switch (data & 3)
		{
			default:
			case 0: m_MMC5_floodattr = 0x00; break;
			case 1: m_MMC5_floodattr = 0x55; break;
			case 2: m_MMC5_floodattr = 0xaa; break;
			case 3: m_MMC5_floodattr = 0xff; break;
		}
			break;

		case 0x1013: /* $5113 */
			LOG_MMC(("MMC5 mid RAM bank select: %02x\n", data & 0x07));
			// FIXME: a few Koei games have both WRAM & BWRAM but here we don't support this (yet)
			if (m_battery)
				wram_bank(data, NES_BATTERY);
			else
				wram_bank(data, NES_WRAM);
			break;


		case 0x1014: /* $5114 */
		case 0x1015: /* $5115 */
		case 0x1016: /* $5116 */
		case 0x1017: /* $5117 */
			m_mmc5_prg_regs[offset & 3] = data;
			mmc5_update_prg(machine());
			break;

#if 0
		// these do not work as expected... I guess we definitely need the bank update at proper times (with BG vs. sprite timing)
		case 0x1020: /* $5120 */
		case 0x1021: /* $5121 */
		case 0x1022: /* $5122 */
		case 0x1023: /* $5123 */
		case 0x1024: /* $5124 */
		case 0x1025: /* $5125 */
		case 0x1026: /* $5126 */
		case 0x1027: /* $5127 */
			data |= (m_mmc5_chr_high << 8);
			if (!m_mmc5_last_chr_a)
			{
				m_mmc5_vrom_regA[offset & 0x07] = data;
				m_mmc5_last_chr_a = 1;
				if (m_ppu->get_current_scanline() == 240 || !m_ppu->is_sprite_8x16())
					mmc5_update_chr_a(machine());
			}
			break;


		case 0x1028: /* $5128 */
		case 0x1029: /* $5129 */
		case 0x102a: /* $512a */
		case 0x102b: /* $512b */
			data |= (m_mmc5_chr_high << 8);
			m_mmc5_vrom_regB[offset & 0x03] = data;
			m_mmc5_last_chr_a = 0;
			if (m_ppu->get_current_scanline() == 240 || !m_ppu->is_sprite_8x16())
				mmc5_update_chr_b(machine());
			break;

		case 0x1030: /* $5130 */
			m_mmc5_chr_high = data & 0x03;
			if (m_mmc5_vram_control == 1)
			{
				// in this case m_mmc5_chr_high selects which 256KB of CHR ROM
				// is to be used for all background tiles on the screen.
			}
			break;

#endif

		case 0x1020: /* $5120 */
			LOG_MMC(("MMC5 $5120 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[0] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_0(m_MMC5_vrom_bank[0], CHRROM);
				//                  m_nes_vram_sprite[0] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[0] = 4;
				//                  vrom_page_a = 1;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1021: /* $5121 */
			LOG_MMC(("MMC5 $5121 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x02:
				/* 2k switch */
				chr2_0(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[1] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_1(m_MMC5_vrom_bank[1], CHRROM);
				//                  m_nes_vram_sprite[1] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[1] = 5;
				//                  vrom_page_a = 1;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1022: /* $5122 */
			LOG_MMC(("MMC5 $5122 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[2] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_2(m_MMC5_vrom_bank[2], CHRROM);
				//                  m_nes_vram_sprite[2] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[2] = 6;
				//                  vrom_page_a = 1;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1023: /* $5123 */
			LOG_MMC(("MMC5 $5123 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x01:
				chr4_0(data, CHRROM);
				break;
			case 0x02:
				/* 2k switch */
				chr2_2(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[3] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_3(m_MMC5_vrom_bank[3], CHRROM);
				//                  m_nes_vram_sprite[3] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[3] = 7;
				//                  vrom_page_a = 1;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1024: /* $5124 */
			LOG_MMC(("MMC5 $5124 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[4] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_4(m_MMC5_vrom_bank[4], CHRROM);
				//                  m_nes_vram_sprite[4] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[0] = 0;
				//                  vrom_page_a = 0;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1025: /* $5125 */
			LOG_MMC(("MMC5 $5125 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x02:
				/* 2k switch */
				chr2_4(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[5] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_5(m_MMC5_vrom_bank[5], CHRROM);
				//                  m_nes_vram_sprite[5] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[1] = 1;
				//                  vrom_page_a = 0;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1026: /* $5126 */
			LOG_MMC(("MMC5 $5126 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[6] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_6(m_MMC5_vrom_bank[6], CHRROM);
				//                  m_nes_vram_sprite[6] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[2] = 2;
				//                  vrom_page_a = 0;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1027: /* $5127 */
			LOG_MMC(("MMC5 $5127 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x00:
				/* 8k switch */
				chr8(data, CHRROM);
				break;
			case 0x01:
				/* 4k switch */
				chr4_4(data, CHRROM);
				break;
			case 0x02:
				/* 2k switch */
				chr2_6(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[7] = data | (m_mmc5_high_chr << 8);
				//                  mapper5_sync_vrom(0);
				chr1_7(m_MMC5_vrom_bank[7], CHRROM);
				//                  m_nes_vram_sprite[7] = m_MMC5_vrom_bank[0] * 64;
				//                  vrom_next[3] = 3;
				//                  vrom_page_a = 0;
				//                  vrom_page_b = 0;
				break;
		}
			break;
		case 0x1028: /* $5128 */
			LOG_MMC(("MMC5 $5128 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[8] = data | (m_mmc5_high_chr << 8);
				//                  nes_vram[vrom_next[0]] = data * 64;
				//                  nes_vram[0 + (vrom_page_a*4)] = data * 64;
				//                  nes_vram[0] = data * 64;
				chr1_4(m_MMC5_vrom_bank[8], CHRROM);
				//                  mapper5_sync_vrom(1);
				if (!m_vrom_page_b)
				{
					m_vrom_page_a ^= 0x01;
					m_vrom_page_b = 1;
				}
				break;
		}
			break;
		case 0x1029: /* $5129 */
			LOG_MMC(("MMC5 $5129 vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x02:
				/* 2k switch */
				chr2_0(data | (m_mmc5_high_chr << 8), CHRROM);
				chr2_4(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[9] = data | (m_mmc5_high_chr << 8);
				//                  nes_vram[vrom_next[1]] = data * 64;
				//                  nes_vram[1 + (vrom_page_a*4)] = data * 64;
				//                  nes_vram[1] = data * 64;
				chr1_5(m_MMC5_vrom_bank[9], CHRROM);
				//                  mapper5_sync_vrom(1);
				if (!m_vrom_page_b)
				{
					m_vrom_page_a ^= 0x01;
					m_vrom_page_b = 1;
				}
				break;
		}
			break;
		case 0x102a: /* $512a */
			LOG_MMC(("MMC5 $512a vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[10] = data | (m_mmc5_high_chr << 8);
				//                  nes_vram[vrom_next[2]] = data * 64;
				//                  nes_vram[2 + (vrom_page_a*4)] = data * 64;
				//                  nes_vram[2] = data * 64;
				chr1_6(m_MMC5_vrom_bank[10], CHRROM);
				//                  mapper5_sync_vrom(1);
				if (!m_vrom_page_b)
				{
					m_vrom_page_a ^= 0x01;
					m_vrom_page_b = 1;
				}
				break;
		}
			break;
		case 0x102b: /* $512b */
			LOG_MMC(("MMC5 $512b vrom select: %02x (mode: %d)\n", data, m_mmc5_chr_mode));
			switch (m_mmc5_chr_mode)
		{
			case 0x00:
				/* 8k switch */
				/* switches in first half of an 8K bank!) */
				chr4_0(data << 1, CHRROM);
				chr4_4(data << 1, CHRROM);
				break;
			case 0x01:
				/* 4k switch */
				chr4_0(data, CHRROM);
				chr4_4(data, CHRROM);
				break;
			case 0x02:
				/* 2k switch */
				chr2_2(data | (m_mmc5_high_chr << 8), CHRROM);
				chr2_6(data | (m_mmc5_high_chr << 8), CHRROM);
				break;
			case 0x03:
				/* 1k switch */
				m_MMC5_vrom_bank[11] = data | (m_mmc5_high_chr << 8);
				//                  nes_vram[vrom_next[3]] = data * 64;
				//                  nes_vram[3 + (vrom_page_a*4)] = data * 64;
				//                  nes_vram[3] = data * 64;
				chr1_7(m_MMC5_vrom_bank[11], CHRROM);
				//                  mapper5_sync_vrom(1);
				if (!m_vrom_page_b)
				{
					m_vrom_page_a ^= 0x01;
					m_vrom_page_b = 1;
				}
				break;
		}
			break;

		case 0x1030: /* $5130 */
			m_mmc5_high_chr = data & 0x03;
			if (m_mmc5_vram_control == 1)
			{
				// in this case m_mmc5_high_chr selects which 256KB of CHR ROM
				// is to be used for all background tiles on the screen.
			}
			break;


		case 0x1100: /* $5200 */
			m_mmc5_split_scr = data;
			// in EX2 and EX3 modes, no split screen
			if (m_mmc5_vram_control & 0x02)
				m_mmc5_split_scr &= 0x7f;
			m_mmc5_split_ctrl = data;
			break;

		case 0x1101: /* $5201 */
			m_mmc5_split_yst = (data >= 240) ? data - 16 : data;
			break;

		case 0x1102: /* $5202 */
			m_mmc5_split_bank = data;
			break;

		case 0x1103: /* $5203 */
			m_IRQ_count = data;
			m_MMC5_scanline = data;
			LOG_MMC(("MMC5 irq scanline: %d\n", m_IRQ_count));
			break;
		case 0x1104: /* $5204 */
			m_IRQ_enable = data & 0x80;
			LOG_MMC(("MMC5 irq enable: %02x\n", data));
			break;
		case 0x1105: /* $5205 */
			m_mult1 = data;
			break;
		case 0x1106: /* $5206 */
			m_mult2 = data;
			break;

		default:
			logerror("** MMC5 uncaught write, offset: %04x, data: %02x\n", offset + 0x4100, data);
			break;
	}
}

/*************************************************************

 NTBROM & Sunsoft-4 board emulation

 Part of this can be used also for Sunsoft Double Cassette
 System... which games use it?

 iNES: mapper 68

 *************************************************************/

static void ntbrom_mirror( running_machine &machine, int mirror, int mirr0, int mirr1 )
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (mirror)
	{
		case 0x00:
			state->set_nt_mirroring(PPU_MIRROR_HORZ);
			break;
		case 0x01:
			state->set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case 0x02:
			state->set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case 0x03:
			state->set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
		case 0x10:
			state->set_nt_page(0, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(1, ROM, mirr1 | 0x80, 0);
			state->set_nt_page(2, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(3, ROM, mirr1 | 0x80, 0);
			break;
		case 0x11:
			state->set_nt_page(0, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(1, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(2, ROM, mirr1 | 0x80, 0);
			state->set_nt_page(3, ROM, mirr1 | 0x80, 0);
			break;
		case 0x12:
			state->set_nt_page(0, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(1, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(2, ROM, mirr0 | 0x80, 0);
			state->set_nt_page(3, ROM, mirr0 | 0x80, 0);
			break;
		case 0x13:
			state->set_nt_page(0, ROM, mirr1 | 0x80, 0);
			state->set_nt_page(1, ROM, mirr1 | 0x80, 0);
			state->set_nt_page(2, ROM, mirr1 | 0x80, 0);
			state->set_nt_page(3, ROM, mirr1 | 0x80, 0);
			break;
	}
}

WRITE8_MEMBER(nes_carts_state::ntbrom_w)
{
	LOG_MMC(("ntbrom_w, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			chr2_0(data, CHRROM);
			break;
		case 0x1000:
			chr2_2(data, CHRROM);
			break;
		case 0x2000:
			chr2_4(data, CHRROM);
			break;
		case 0x3000:
			chr2_6(data, CHRROM);
			break;
		case 0x4000:
			m_mmc_latch1 = data & 0x7f;
			ntbrom_mirror(machine(), m_mmc_reg[0], m_mmc_latch1, m_mmc_latch2);
			break;
		case 0x5000:
			m_mmc_latch2 = data & 0x7f;
			ntbrom_mirror(machine(), m_mmc_reg[0], m_mmc_latch1, m_mmc_latch2);
			break;
		case 0x6000:
			m_mmc_reg[0] = data & 0x13;
			ntbrom_mirror(machine(), m_mmc_reg[0], m_mmc_latch1, m_mmc_latch2);
			break;
		case 0x7000:
			prg16_89ab(data);
			break;
		default:
			LOG_MMC(("ntbrom_w uncaught write, offset: %04x, data: %02x\n", offset, data));
			break;
	}
}

/*************************************************************

 JxROM & Sunsoft 5A / 5B / FME7 board emulation

 Notice that Sunsoft-5B = FME7 + sound chip (the latter being
 currently unemulated in MESS)

 iNES: mapper 69

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void jxrom_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	/* TODO: change to reflect the actual number of cycles spent */
	if ((state->m_IRQ_enable & 0x80) && (state->m_IRQ_enable & 0x01))
	{
		if (state->m_IRQ_count <= 114)
		{
			state->m_IRQ_count = 0xffff;
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			state->m_IRQ_count -= 114;
	}
	else if (state->m_IRQ_enable & 0x01)    // if enable bit 7 is not set, only decrement the counter!
	{
		if (state->m_IRQ_count <= 114)
			state->m_IRQ_count = 0xffff;
		else
			state->m_IRQ_count -= 114;
	}
}

WRITE8_MEMBER(nes_carts_state::jxrom_w)
{
	LOG_MMC(("jxrom_w, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
			m_mmc_latch1 = data & 0x0f;
			break;

		case 0x2000:
			switch (m_mmc_latch1)
			{
			case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				chr1_x(m_mmc_latch1, data, CHRROM);
				break;

			case 8:
				if (!(data & 0x40))
				{
					// is PRG ROM
					space.unmap_write(0x6000, 0x7fff);
					prg8_67(data & 0x3f);
				}
				else if (data & 0x80)
				{
					// is PRG RAM
					space.install_write_bank(0x6000, 0x7fff, "bank5");
					m_prg_bank[4] = m_battery_bank5_start + (data & 0x3f);
					membank("bank5")->set_entry(m_prg_bank[4]);
				}
				break;

			case 9:
				prg8_89(data);
				break;
			case 0x0a:
				prg8_ab(data);
				break;
			case 0x0b:
				prg8_cd(data);
				break;
			case 0x0c:
				switch (data & 0x03)
				{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
				break;
			case 0x0d:
				m_IRQ_enable = data;
				break;
			case 0x0e:
				m_IRQ_count = (m_IRQ_count & 0xff00) | data;
				break;
			case 0x0f:
				m_IRQ_count = (m_IRQ_count & 0x00ff) | (data << 8);
				break;
			}
			break;

			/* Here we would have sound command for Sunsoft 5b variant */
			//      case 0x4000:
			//      case 0x6000:

		case 0x4000:
			LOG_MMC(("Sunsoft-5B Sound: Register select write, data: %02x\n", data));
			break;
		case 0x6000:
			LOG_MMC(("Sunsoft-5B Sound: Register write, data: %02x\n", data));
			break;
		default:
			logerror("jxrom_w uncaught %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*************************************************************

 DxROM & Namcot 3433 - 3443 board emulation

 Games: Dragon Spirit - Aratanaru Densetsu, Namcot Mahjong, Quinty

 These are the same board, but DRROM (and Tengen 800004) have
 4-screen mirroring

 iNES: mappers 88, 206 (same as 88 but possibly 4-screen mirroring)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dxrom_w)
{
	LOG_MMC(("dxrom_w, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			switch (m_mmc_latch1 & 0x07)
			{
			case 0: chr2_0(data >> 1, CHRROM); break;
			case 1: chr2_2(data >> 1, CHRROM); break;
			case 2: chr1_4(data | 0x40, CHRROM); break;
			case 3: chr1_5(data | 0x40, CHRROM); break;
			case 4: chr1_6(data | 0x40, CHRROM); break;
			case 5: chr1_7(data | 0x40, CHRROM); break;
			case 6: prg8_89(data); break;
			case 7: prg8_ab(data); break;
			}
			break;
		case 0:
			m_mmc_latch1 = data;
			break;
	}
}


/*************************************************************

 Namcot 3453 board emulation

 Games: Devil Man

 These are the same as Namcot 34x3, but with additional mirroring
 control

 iNES: mapper 154

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::namcot3453_w)
{
	LOG_MMC(("namcot3453_w, offset: %04x, data: %02x\n", offset, data));

	// additional mirroring control when writing to even addresses
	if (!(offset & 1))
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);

	dxrom_w(space, offset, data, mem_mask);
}

/*************************************************************

 Namcot 3446 board emulation

 Games: Digital Devil Monogatari - Megami Tensei

 These are similar Namcot 34x3, but different bankswitch capabilities

 iNES: mapper 76

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::namcot3446_w)
{
	LOG_MMC(("namcot3446_w, offset: %04x, data: %02x\n", offset, data));

	// NEStopia does not have this!
	if (offset >= 0x2000)
	{
		if (!(offset & 1))
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		return;
	}

	switch (offset & 1)
	{
		case 1:
			switch (m_mmc_latch1 & 0x07)
			{
			case 2: chr2_0(data, CHRROM); break;
			case 3: chr2_2(data, CHRROM); break;
			case 4: chr2_4(data, CHRROM); break;
			case 5: chr2_6(data, CHRROM); break;
			case 6: BIT(m_mmc_latch1, 6) ? prg8_cd(data) : prg8_89(data); break;
			case 7: prg8_ab(data); break;
			}
			break;
		case 0:
			m_mmc_latch1 = data;
			break;
	}
}

/*************************************************************

 Namcot 3425 board emulation

 Games: Dragon Buster

 These are similar Namcot 34x3, but with NT mirroring (two
 different modes)

 iNES: mapper 95

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::namcot3425_w)
{
	UINT8 mode;
	LOG_MMC(("namcot3425_w, offset: %04x, data: %02x\n", offset, data));
	if (offset >= 0x2000)
		return;

	switch (offset & 1)
	{
		case 1:
			mode = m_mmc_latch1 & 0x07;
			switch (mode)
			{
			case 0: chr2_0(data >> 1, CHRROM); break;
			case 1: chr2_2(data >> 1, CHRROM); break;
			case 2:
			case 3:
			case 4:
			case 5:
				chr1_x(2 + mode, data, CHRROM);
				m_mmc_reg[mode - 2] = BIT(data, 5);
				if (!BIT(m_mmc_latch1, 7))
				{
						set_nt_page(0, CIRAM, m_mmc_reg[0], 1);
						set_nt_page(1, CIRAM, m_mmc_reg[1], 1);
						set_nt_page(2, CIRAM, m_mmc_reg[2], 1);
						set_nt_page(3, CIRAM, m_mmc_reg[3], 1);
				}
				else
					set_nt_mirroring(PPU_MIRROR_HORZ);
				break;
			case 6: prg8_89(data); break;
			case 7: prg8_ab(data); break;
			}
			break;
		case 0:
			m_mmc_latch1 = data;
			break;
	}
}

/*************************************************************

 Discrete Logic board IC 74x377 by Color Dreams / Nina-007 emulation

 Games: many Color Dreams and Wisdom Tree titles

 iNES: mapper 11

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dis_74x377_w)
{
	LOG_MMC(("dis_74x377_w, offset: %04x, data: %02x\n", offset, data));

	chr8(data >> 4, m_mmc_chr_source);
	prg32(data & 0x0f);
}

/*************************************************************

 Discrete Logic board IC 74x139x74 by Konami & Jaleco

 iNES: mapper 87

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dis_74x139x74_m_w)
{
	LOG_MMC(("dis_74x139x74_m_w, offset: %04x, data: %02x\n", offset, data));

	chr8(((data & 0x02) >> 1) | ((data & 0x01) << 1), CHRROM);
}

/*************************************************************

 Discrete Logic board IC 74x161x138

 Games: Crime Busters

 iNES: mapper 38

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dis_74x161x138_m_w)
{
	LOG_MMC(("dis_74x161x138_m_w, offset: %04x, data: %02x\n", offset, data));

	chr8(data >> 2, CHRROM);
	prg32(data);
}

/*************************************************************

 Discrete Logic board IC 74x161x161x32

 There are two variants (one with hardwired mirroring, the
 other with a mirroring control), making necessary two distinct
 mappers & pcb_id

 iNES: mappers 70 & 152

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dis_74x161x161x32_w)
{
	LOG_MMC(("dis_74x161x161x32_w, offset: %04x, data: %02x\n", offset, data));

	if (!m_hard_mirroring)  // there are two 'variants' depending on hardwired or mapper ctrl mirroring
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data, CHRROM);
	prg16_89ab(data >> 4);
}

/*************************************************************

 Bandai LZ93D50 boards emulation

 There are several variants: plain board with or without SRAM,
 board + 24C01 EEPROM, board + 24C02 EEPROM, board + Barcode
 Reader (DATACH).
 We currently only emulate the base hardware.

 Games: Crayon Shin-Chan - Ora to Poi Poi, Dragon Ball Z Gaiden,
 Dragon Ball Z II & III, Rokudenashi Blues, SD Gundam
 Gaiden - KGM2, Dragon Ball Z, Magical Taruruuto-kun, SD Gundam
 Gaiden [with EEPROM], Dragon Ball, Dragon Ball 3, Famicom Jump,
 Famicom Jump II [no EEPROM], Datach Games

 At the moment, we don't support EEPROM I/O

 iNES: mappers 16, 153, 157 & 159

 In MESS: Supported

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void bandai_lz_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	/* 114 is the number of cycles per scanline */
	/* TODO: change to reflect the actual number of cycles spent */
	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count <= 114)
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_count = (0xffff - 114 + state->m_IRQ_count);   // wrap around the 16 bits counter
		}
		state->m_IRQ_count -= 114;
	}
}

WRITE8_MEMBER(nes_carts_state::lz93d50_w)
{
	LOG_MMC(("lz93d50_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x000f)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			chr1_x(offset & 0x07, data, m_mmc_chr_source);
			break;
		case 8:
			prg16_89ab(data);
			break;
		case 9:
			switch (data & 0x03)
			{
			case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x0a:
			m_IRQ_enable = data & 0x01;
			break;
		case 0x0b:
			m_IRQ_count = (m_IRQ_count & 0xff00) | data;
			break;
		case 0x0c:
			m_IRQ_count = (m_IRQ_count & 0x00ff) | (data << 8);
			break;
		default:
			logerror("lz93d50_w uncaught write, offset: %04x, data: %02x\n", offset, data);
			break;
	}
}

WRITE8_MEMBER(nes_carts_state::lz93d50_m_w)
{
	LOG_MMC(("lz93d50_m_w, offset: %04x, data: %02x\n", offset, data));

	if (!m_battery && !m_wram)
		lz93d50_w(space, offset & 0x0f, data, mem_mask);
	else if (m_battery)
		m_battery_ram[offset] = data;
	else
		m_wram[offset] = data;
}

static void fjump2_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 mmc_helper = 0;
	int i;

	for (i = 0; i < 8; i++)
		mmc_helper |= ((state->m_mmc_reg[i] & 0x01) << 4);

	state->prg16_89ab(mmc_helper | state->m_mmc_latch1);
	state->prg16_cdef(mmc_helper | 0x0f);
}

WRITE8_MEMBER(nes_carts_state::fjump2_w)
{
	LOG_MMC(("fjump2_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x000f)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			m_mmc_reg[offset & 0x000f] = data;
			fjump2_set_prg(machine());
			break;
		case 8:
			m_mmc_latch1 = (data & 0x0f);
			fjump2_set_prg(machine());
			break;
		default:
			lz93d50_m_w(space, offset & 0x0f, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bandai Karaoke Studio board emulation

 Games: Karaoke Studio

 Note: we currently do not emulate the mic

 iNES: mapper 188

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bandai_ks_w)
{
	LOG_MMC(("bandai_ks_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(data ^ 0x08);
}

/*************************************************************

 Bandai Oeka Kids board emulation

 Games: Oeka Kids - Anpanman no Hiragana Daisuki, Oeka
 Kids - Anpanman to Oekaki Shiyou!!

 iNES: mapper 96

 In MESS: Preliminary Support.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bandai_ok_w)
{
	UINT8 mmc_helper;
	LOG_MMC(("mapper96_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data);

	m_mmc_latch1 = data;
	mmc_helper = (m_mmc_latch1 & 0x03) | (data & 0x04);
	chr4_0(mmc_helper, CHRRAM);
	chr4_4(0x03 | (data & 0x04), CHRRAM);
}

/*************************************************************

 Irem LROG017 - Discrete board emulation (74*161/161/21/138)

 Games: Napoleon Senki

 iNES: mapper 77

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::lrog017_w)
{
	LOG_MMC(("lrog017_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
	chr2_0((data >> 4), CHRROM);
}

/*************************************************************

 Irem Holy Diver board emulation

 iNES: mapper 78 (shared with JF-16)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::irem_hd_w)
{
	LOG_MMC(("irem_hd_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	chr8(data >> 4, CHRROM);
	prg16_89ab(data);
}

/*************************************************************

 Irem TAM-S1 board emulation

 Games: Kaiketsu Yanchamaru

 iNES: mapper 97

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tam_s1_w)
{
	LOG_MMC(("tam_s1_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
		prg16_cdef(data);
	}
}

/*************************************************************

 Irem G-101 board emulation

 Major League uses hardwired mirroring

 iNES: mapper 32

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::g101_w)
{
	LOG_MMC(("g101_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			// NEStopia here differs a little bit
			m_mmc_latch1 ? prg8_cd(data) : prg8_89(data);
			break;
		case 0x1000:
			m_mmc_latch1 = BIT(data, 1);
			if (!m_hard_mirroring)  // there are two 'variants' depending on hardwired or mapper ctrl mirroring
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000:
			chr1_x(offset & 0x07, data, CHRROM);
			break;
	}
}

/*************************************************************

 Irem H-3001 board emulation

 Games: Daiku no Gen San 2 - Akage no Dan no Gyakushuu,
 Kaiketsu Yanchamaru 3, Spartan X 2

 iNES: mapper 65

 In MESS: Supported.

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void h3001_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if (state->m_IRQ_enable)
	{
		state->m_IRQ_count -= 114;

		if (state->m_IRQ_count <= 114)
		{
			state->m_IRQ_enable = 0;
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
}

WRITE8_MEMBER(nes_carts_state::h3001_w)
{
	LOG_MMC(("h3001_w, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7fff)
	{
		case 0x0000:
			prg8_89(data);
			break;

		case 0x1001:
			set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x1003:
			m_IRQ_enable = data & 0x80;
			break;

		case 0x1004:
			m_IRQ_count = m_IRQ_count_latch;
			break;

		case 0x1005:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0x00ff) | (data << 8);
			break;

		case 0x1006:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0xff00) | data;
			break;

		case 0x2000:
			prg8_ab(data);
			break;

		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x3004: case 0x3005: case 0x3006: case 0x3007:
			chr1_x(offset & 0x07, data, CHRROM);
			break;

		case 0x4000:
			prg8_cd(data);
			break;

		default:
			break;
	}
}

/*************************************************************

 Jaleco SS88006 board emulation, aka JF-27, JF-29, JF-30, ...,
 JF-38, JF-40, JF-41

 Games: Lord of King, Magic John, Moe Pro '90, Ninja Jajamaru,
 Pizza Pop, Plasma Ball

 iNES: mapper 18

 In MESS: Supported

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void ss88006_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	/* Increment & check the IRQ scanline counter */
	if (state->m_IRQ_enable)
	{
		LOG_MMC(("scanline: %d, irq count: %04x\n", scanline, state->m_IRQ_count));
		if (state->m_IRQ_mode & 0x08)
		{
			if ((state->m_IRQ_count & 0x000f) < 114)    // always true, but we only update the IRQ once per scanlines so we cannot be more precise :(
			{
				state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				state->m_IRQ_count = (state->m_IRQ_count & ~0x000f) | (0x0f - (114 & 0x0f) + (state->m_IRQ_count & 0x000f)); // sort of wrap around the counter
			}
			// decrements should not affect upper bits, so we don't do anything here (114 > 0x0f)
		}
		else if (state->m_IRQ_mode & 0x04)
		{
			if ((state->m_IRQ_count & 0x00ff) < 114)
			{
				state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				state->m_IRQ_count = (state->m_IRQ_count & ~0x00ff) | (0xff - 114 + (state->m_IRQ_count & 0x00ff)); // wrap around the 8 bits counter
			}
			else
				state->m_IRQ_count -= 114;
		}
		else if (state->m_IRQ_mode & 0x02)
		{
			if ((state->m_IRQ_count & 0x0fff)  < 114)
			{
				state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				state->m_IRQ_count = (state->m_IRQ_count & ~0x0fff) | (0xfff - 114 + (state->m_IRQ_count & 0x0fff));    // wrap around the 12 bits counter
			}
			else
				state->m_IRQ_count -= 114;
		}
		else if (state->m_IRQ_count < 114)
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_count = (0xffff - 114 + state->m_IRQ_count);   // wrap around the 16 bits counter
		}
		else
			state->m_IRQ_count -= 114;
	}
}

WRITE8_MEMBER(nes_carts_state::ss88006_w)
{
	UINT8 bank;
	LOG_MMC(("mapper18_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0xf0) | (data & 0x0f);
			prg8_89(m_mmc_prg_bank[0]);
			break;
		case 0x0001:
			m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0x0f) | (data << 4);
			prg8_89(m_mmc_prg_bank[0]);
			break;
		case 0x0002:
			m_mmc_prg_bank[1] = (m_mmc_prg_bank[1] & 0xf0) | (data & 0x0f);
			prg8_ab(m_mmc_prg_bank[1]);
			break;
		case 0x0003:
			m_mmc_prg_bank[1] = (m_mmc_prg_bank[1] & 0x0f) | (data << 4);
			prg8_ab(m_mmc_prg_bank[1]);
			break;
		case 0x1000:
			m_mmc_prg_bank[2] = (m_mmc_prg_bank[2] & 0xf0) | (data & 0x0f);
			prg8_cd(m_mmc_prg_bank[2]);
			break;
		case 0x1001:
			m_mmc_prg_bank[2] = (m_mmc_prg_bank[2] & 0x0f) | (data << 4);
			prg8_cd(m_mmc_prg_bank[2]);
			break;

			/* $9002, 3 (1002, 3) uncaught = Jaleco Baseball writes 0 */
			/* believe it's related to battery-backed ram enable/disable */

		case 0x2000: case 0x2001: case 0x2002: case 0x2003:
		case 0x3000: case 0x3001: case 0x3002: case 0x3003:
		case 0x4000: case 0x4001: case 0x4002: case 0x4003:
		case 0x5000: case 0x5001: case 0x5002: case 0x5003:
			bank = ((offset & 0x7000) - 0x2000) / 0x0800 + ((offset & 0x0002) >> 1);
			if (offset & 0x0001)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f)<< 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);

			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;

		case 0x6000:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0xfff0) | (data & 0x0f);
			break;
		case 0x6001:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x6002:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x6003:
			m_IRQ_count_latch = (m_IRQ_count_latch & 0x0fff) | ((data & 0x0f) << 12);
			break;

		case 0x7000:
			m_IRQ_count = m_IRQ_count_latch;
			break;
		case 0x7001:
			m_IRQ_enable = data & 0x01;
			m_IRQ_mode = data & 0x0e;
			break;

		case 0x7002:
			switch (data & 0x03)
			{
			case 0: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 1: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;

		default:
			logerror("Jaleco SS88006 uncaught write, addr: %04x, value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*************************************************************

 Jaleco JF-11, JF-12 & JF-14 boards emulation

 Games: Bio Senshi Dan, Mississippi Satsujin Jiken

 iNES: mapper 140

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::jf11_m_w)
{
	LOG_MMC(("jf11_m_w, offset: %04x, data: %02x\n", offset, data));
	chr8(data, CHRROM);
	prg32(data >> 4);
}

/*************************************************************

 Jaleco JF-13 board emulation

 Games: Moero Pro Yakyuu

 Note: we don't emulate the additional sound hardware.

 iNES: mapper 86

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::jf13_m_w)
{
	LOG_MMC(("jf13_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)
	{
		prg32((data >> 4) & 0x03);
		chr8(((data >> 4) & 0x04) | (data & 0x03), CHRROM);
	}

	if (offset == 0x1000)
	{
		LOG_MMC(("Jaleco JF-13 sound write, data: %02x\n", data));
		// according to NEStopia, this is the effect
		if ((data & 0x30) == 0x20)
		{
			// send command (data & 0x1f) to sound chip
		}
	}
}

/*************************************************************

 Jaleco JF-16 board emulation

 iNES: mapper 78 (shared with a diff Irem board)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::jf16_w)
{
	LOG_MMC(("jf16_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data >> 4, CHRROM);
	prg16_89ab(data);
}

/*************************************************************

 Jaleco JF-17, JF-26 & JF-28 boards emulation

 Note: we don't emulate the additional sound hardware.

 Games: Moero!! Juudou Warriors, Moero!! Pro Tennis, Pinball
 Quest Jpn

 iNES: mapper 72

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::jf17_w)
{
	LOG_MMC(("jf17_w, offset: %04x, data: %02x\n", offset, data));

	if (BIT(data, 7))
		prg16_89ab(data & 0x0f);
	if (BIT(data, 6))
		chr8(data & 0x0f, CHRROM);
	if (BIT(data, 5) && !BIT(data,4))
		LOG_MMC(("Jaleco JF-17 sound write, data: %02x\n", data & 0x1f));
}

/*************************************************************

 Jaleco JF-19 & JF-21 boards emulation

 Note: we don't emulate the additional sound hardware.

 Games: Moero Pro Soccer, Moero Pro Yakyuu '88

 iNES: mapper 92

 In MESS: Supported (no samples).

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::jf19_w)
{
	LOG_MMC(("jf19_w, offset: %04x, data: %02x\n", offset, data));

	if (BIT(data, 7))
		prg16_cdef(data & 0x0f);
	if (BIT(data, 6))
		chr8(data & 0x0f, CHRROM);
	if (BIT(data, 5) && !BIT(data,4))
		LOG_MMC(("Jaleco JF-19 sound write, data: %02x\n", data & 0x1f));
}

/*************************************************************

 Konami VRC1 and Jaleco JF20, JF22

 Games: Exciting Boxing, Ganbare Goemon!, Tetsuwan Atom

 iNES: mapper 75

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::konami_vrc1_w)
{
	LOG_MMC(("konami_vrc1_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x1000:
			set_nt_mirroring((data & 0x01) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x0f) | ((data & 0x02) << 3);
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x0f) | ((data & 0x04) << 2);
			chr4_0(m_mmc_vrom_bank[0], CHRROM);
			chr4_4(m_mmc_vrom_bank[1], CHRROM);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x4000:
			prg8_cd(data);
			break;
		case 0x6000:
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x10) | (data & 0x0f);
			chr4_0(m_mmc_vrom_bank[0], CHRROM);
			break;
		case 0x7000:
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x10) | (data & 0x0f);
			chr4_4(m_mmc_vrom_bank[1], CHRROM);
			break;
	}
}

/*************************************************************

 Konami VRC-2

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::konami_vrc2_w)
{
	UINT8 bank, shift, mask;
	UINT32 shifted_offs = (offset & 0x7000)
						| ((offset << (9 - m_vrc_ls_prg_a)) & 0x200)
						| ((offset << (8 - m_vrc_ls_prg_b)) & 0x100);
	LOG_MMC(("konami_vrc2_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
		prg8_89(data);
	else if (offset < 0x2000)
	{
		switch (data & 0x03)
		{
			case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
	else if (offset < 0x3000)
		prg8_ab(data);
	else if (offset < 0x7000)
	{
		bank = ((shifted_offs & 0x7000) - 0x3000) / 0x0800 + BIT(shifted_offs, 9);
		shift = BIT(shifted_offs, 8) * 4;
		mask = (0xf0 >> shift);
		m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & mask)
									| (((data >> m_vrc_ls_chr) & 0x0f) << shift);
		chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
	}
	else
		logerror("konami_vrc2_w uncaught write, addr: %04x value: %02x\n", offset + 0x8000, data);
}

/*************************************************************

 Konami VRC3

 Games: Salamander

 iNES: mapper 73

 In MESS: Supported. It also uses konami_irq.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::konami_vrc3_w)
{
	LOG_MMC(("konami_vrc3_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
			/* dunno which address controls these */
			m_IRQ_count_latch = data;
			m_IRQ_enable_latch = data;
			break;
		case 0x2000:
			m_IRQ_enable = data;
			break;
		case 0x3000:
			m_IRQ_count &= ~0x0f;
			m_IRQ_count |= data & 0x0f;
			break;
		case 0x4000:
			m_IRQ_count &= ~0xf0;
			m_IRQ_count |= (data & 0x0f) << 4;
			break;
		case 0x7000:
			prg16_89ab(data);
			break;
		default:
			logerror("konami_vrc3_w uncaught write, offset %04x, data: %02x\n", offset, data);
			break;
	}
}

/*************************************************************

 Konami VRC-4

 In MESS: Supported

 *************************************************************/

static void vrc4_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (state->m_mmc_latch1 & 0x02)
	{
		state->prg8_89(0xfe);
		state->prg8_cd(state->m_mmc_prg_bank[0]);
	}
	else
	{
		state->prg8_89(state->m_mmc_prg_bank[0]);
		state->prg8_cd(0xfe);
	}
}

static void konami_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	/* Increment & check the IRQ scanline counter */
	if (state->m_IRQ_enable && (++state->m_IRQ_count == 0x100))
	{
		state->m_IRQ_count = state->m_IRQ_count_latch;
		state->m_IRQ_enable = state->m_IRQ_enable_latch;
		state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_carts_state::konami_vrc4_w)
{
	UINT8 bank, shift, mask;
	UINT32 shifted_offs = (offset & 0x7000)
						| ((offset << (9 - m_vrc_ls_prg_a)) & 0x200)
						| ((offset << (8 - m_vrc_ls_prg_b)) & 0x100);
	LOG_MMC(("konami_vrc4_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
	{
		m_mmc_prg_bank[0] = data;
		vrc4_set_prg(machine());
	}
	else if (offset >= 0x2000 && offset < 0x3000)
		prg8_ab(data);
	else
	{
		switch (shifted_offs & 0x7300)
		{
			case 0x1000:
			case 0x1100:
				switch (data & 0x03)
				{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
				break;
			case 0x1200:
			case 0x1300:
				m_mmc_latch1 = data & 0x02;
				vrc4_set_prg(machine());
				break;
			case 0x3000:
			case 0x3100:
			case 0x3200:
			case 0x3300:
			case 0x4000:
			case 0x4100:
			case 0x4200:
			case 0x4300:
			case 0x5000:
			case 0x5100:
			case 0x5200:
			case 0x5300:
			case 0x6000:
			case 0x6100:
			case 0x6200:
			case 0x6300:
				bank = ((shifted_offs & 0x7000) - 0x3000) / 0x0800 + BIT(shifted_offs, 9);
				shift = BIT(shifted_offs, 8) * 4;
				mask = (0xf0 >> shift);
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & mask) | ((data & 0x0f) << shift);
				chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
				break;
			case 0x7000:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0xf0) | (data & 0x0f);
				break;
			case 0x7100:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x7200:
				m_IRQ_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
				m_IRQ_enable = data & 0x02;
				m_IRQ_enable_latch = data & 0x01;
				if (data & 0x02)
					m_IRQ_count = m_IRQ_count_latch;
				break;
			case 0x7300:
				m_IRQ_enable = m_IRQ_enable_latch;
				break;
			default:
				logerror("konami_vrc4_w uncaught write, addr: %04x value: %02x\n", shifted_offs + 0x8000, data);
				break;
		}
	}
}

/*************************************************************

 Konami VRC-6

 In MESS: Supported. It also uses konami_irq (there are IRQ
 issues though: see Akumajou Densetsu intro).

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::konami_vrc6_w)
{
	UINT8 bank;
	UINT32 shifted_offs = (offset & 0x7000)
						| ((offset << (9 - m_vrc_ls_prg_a)) & 0x200)
						| ((offset << (8 - m_vrc_ls_prg_b)) & 0x100);
	LOG_MMC(("konami_vrc6_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
		prg16_89ab(data);
	else if (offset >= 0x4000 && offset < 0x5000)
		prg8_cd(data);
	else
	{
		switch (shifted_offs & 0x7300)
		{
			case 0x1000:
			case 0x1100:
			case 0x1200:
			case 0x2000:
			case 0x2100:
			case 0x2200:
			case 0x3000:
			case 0x3100:
			case 0x3200:
				LOG_MMC(("Konami VRC-6 Sound write, offset: %04x, data: %02x\n", shifted_offs & 0x7300, data));
				break;
			case 0x3300:
				switch (data & 0x0c)
				{
				case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 0x04: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				case 0x08: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 0x0c: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
				break;
			case 0x5000:
			case 0x5100:
			case 0x5200:
			case 0x5300:
			case 0x6000:
			case 0x6100:
			case 0x6200:
			case 0x6300:
				bank = ((shifted_offs & 0x7000) - 0x5000) / 0x0400 + ((shifted_offs & 0x0300) >> 8);
				chr1_x(bank, data, CHRROM);
				break;
			case 0x7000:
				m_IRQ_count_latch = data;
				break;
			case 0x7100:
				m_IRQ_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
				m_IRQ_enable = data & 0x02;
				m_IRQ_enable_latch = data & 0x01;
				if (data & 0x02)
					m_IRQ_count = m_IRQ_count_latch;
				break;
			case 0x7200:
				m_IRQ_enable = m_IRQ_enable_latch;
				break;
			default:
				logerror("konami_vrc6_w uncaught write, addr: %04x value: %02x\n", shifted_offs + 0x8000, data);
				break;
		}
	}
}

/*************************************************************

 Konami VRC7

 Games: Lagrange Point, Tiny Toon Adventures 2

 iNES: mapper 85

 In MESS: Supported. It also uses konami_irq.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::konami_vrc7_w)
{
	UINT8 bank;
	LOG_MMC(("konami_vrc7_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7018)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0008:
		case 0x0010:
		case 0x0018:
			prg8_ab(data);
			break;

		case 0x1000:
			prg8_cd(data);
			break;

			/* TODO: there are sound regs in here */

		case 0x2000:
		case 0x2008:
		case 0x2010:
		case 0x2018:
		case 0x3000:
		case 0x3008:
		case 0x3010:
		case 0x3018:
		case 0x4000:
		case 0x4008:
		case 0x4010:
		case 0x4018:
		case 0x5000:
		case 0x5008:
		case 0x5010:
		case 0x5018:
			bank = ((offset & 0x7000) - 0x2000) / 0x0800 + ((offset & 0x0018) ? 1 : 0);
			chr1_x(bank, data, m_mmc_chr_source);
			break;

		case 0x6000:
			switch (data & 0x03)
			{
			case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x6008: case 0x6010: case 0x6018:
			m_IRQ_count_latch = data;
			break;
		case 0x7000:
			m_IRQ_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
			m_IRQ_enable = data & 0x02;
			m_IRQ_enable_latch = data & 0x01;
			if (data & 0x02)
				m_IRQ_count = m_IRQ_count_latch;
			break;
		case 0x7008: case 0x7010: case 0x7018:
			m_IRQ_enable = m_IRQ_enable_latch;
			break;

		default:
			logerror("konami_vrc7_w uncaught write, addr: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*************************************************************

 Namcot-163 board emulation

 Games: Battle Fleet, Family Circuit '91, Famista '90, '91,
 '92 & '94, Megami Tensei II, Top Striker, Wagyan Land 2 & 3

 iNES: mapper 19

 In MESS: Supported

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void namcot_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count >= (0x7fff - 114))
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_count = 0;
		}
		else
			state->m_IRQ_count += 114;
	}
}

WRITE8_MEMBER(nes_carts_state::namcot163_l_w)
{
	LOG_MMC(("namcot163_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x0800:
			LOG_MMC(("Namcot-163 sound reg write, data: %02x\n", data));
			break;
		case 0x1000: /* low byte of IRQ */
			m_IRQ_count = (m_IRQ_count & 0x7f00) | data;
			break;
		case 0x1800: /* high byte of IRQ, IRQ enable in high bit */
			m_IRQ_count = (m_IRQ_count & 0xff) | ((data & 0x7f) << 8);
			m_IRQ_enable = data & 0x80;
			break;
	}
}

READ8_MEMBER(nes_carts_state::namcot163_l_r)
{
	LOG_MMC(("namcot163_l_r, offset: %04x\n", offset));
	offset += 0x100;

	switch (offset & 0x1800)
	{
		case 0x1000:
			return m_IRQ_count & 0xff;
		case 0x1800:
			return (m_IRQ_count >> 8) & 0xff;
		case 0x0800:
			LOG_MMC(("Namcot-163 sound reg read\n"));
		default:
			return 0x00;
	}
}

static void namcot163_set_mirror( running_machine &machine, UINT8 page, UINT8 data )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (!(data < 0xe0))
		state->set_nt_page(page, CIRAM, data & 0x01, 1);
	else
		state->set_nt_page(page, ROM, data, 0);
}

WRITE8_MEMBER(nes_carts_state::namcot163_w)
{
	LOG_MMC(("namcot163_w, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x7800)
	{
		case 0x0000: case 0x0800:
		case 0x1000: case 0x1800:
		case 0x2000: case 0x2800:
		case 0x3000: case 0x3800:
			chr1_x(offset / 0x800, data, CHRROM);
			break;
		case 0x4000:
			namcot163_set_mirror(machine(), 0, data);
			break;
		case 0x4800:
			namcot163_set_mirror(machine(), 1, data);
			break;
		case 0x5000:
			namcot163_set_mirror(machine(), 2, data);
			break;
		case 0x5800:
			namcot163_set_mirror(machine(), 3, data);
			break;
		case 0x6000:
			prg8_89(data & 0x3f);
			break;
		case 0x6800:
			m_mmc_latch1 = data & 0xc0;     // this should enable High CHRRAM, but we still have to properly implement it!
			prg8_ab(data & 0x3f);
			break;
		case 0x7000:
			prg8_cd(data & 0x3f);
			break;
		case 0x7800:
			LOG_MMC(("Namcot-163 sound address write, data: %02x\n", data));
			break;
	}
}

/*************************************************************

 Sunsoft-1 board emulation

 Games: Atlantis no Nazo, Kanshakudama Nage Kantarou no
 Toukaidou Gojuusan Tsugi, Wing of Madoola, Fantasy Zone

 iNES: mapper 184 (Fantasy Zone uses this board with no
 CHRROM, and the register switches PRG banks)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sunsoft1_m_w)
{
	LOG_MMC(("sunsoft1_m_w, offset: %04x, data: %02x\n", offset, data));

	if (m_chr_chunks)
	{
		chr4_0(data & 0x0f, CHRROM);
		chr4_4(data >> 4, CHRROM);
	}
	else
		prg16_89ab(data & 0x0f);
}

/*************************************************************

 Sunsoft-2 board emulation

 The two games using this board have incompatible mirroring
 wiring, making necessary two distinct mappers & pcb_id

 iNES: mapper 89 & 93

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sunsoft2_w)
{
	UINT8 sunsoft_helper = (data & 0x07) | ((data & 0x80) ? 0x08 : 0x00);
	LOG_MMC(("sunsoft2_w, offset: %04x, data: %02x\n", offset, data));

	if (!m_hard_mirroring)  // there are two 'variants' depending on hardwired or mapper ctrl mirroring
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	if (m_chr_chunks)
		chr8(sunsoft_helper, CHRROM);

	prg16_89ab(data >> 4);
}

/*************************************************************

 Sunsoft-3 board emulation

 The two games using this board have incompatible mirroring
 wiring, making necessary two distinct mappers & pcb_id

 iNES: mapper 67

 *************************************************************/

/* Here, IRQ counter decrements every CPU cycle. Since we update it every scanline,
 we need to decrement it by 114 (Each scanline consists of 341 dots and, on NTSC,
 there are 3 dots to every 1 CPU cycle, hence 114 is the number of cycles per scanline ) */
static void sunsoft3_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	/* TODO: change to reflect the actual number of cycles spent: both using 114 or cycling 114,114,113
	 produces a 1-line glitch in Fantasy Zone 2: it really requires the counter to be updated each CPU cycle! */
	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count <= 114)
		{
			state->m_IRQ_enable = 0;
			state->m_IRQ_count = 0xffff;
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			state->m_IRQ_count -= 114;
	}
}

WRITE8_MEMBER(nes_carts_state::sunsoft3_w)
{
	LOG_MMC(("sunsoft3_w, offset %04x, data: %02x\n", offset, data));

	switch (offset & 0x7800)
	{
		case 0x0800:
			chr2_0(data, CHRROM);
			break;
		case 0x1800:
			chr2_2(data, CHRROM);
			break;
		case 0x2800:
			chr2_4(data, CHRROM);
			break;
		case 0x3800:
			chr2_6(data, CHRROM);
			break;
		case 0x4000:
		case 0x4800:
			m_IRQ_toggle ^= 1;
			if (m_IRQ_toggle)
				m_IRQ_count = (m_IRQ_count & 0x00ff) | (data << 8);
			else
				m_IRQ_count = (m_IRQ_count & 0xff00) | data;
			break;
		case 0x5800:
			m_IRQ_enable = BIT(data, 4);
			m_IRQ_toggle = 0;
			break;
		case 0x6800:
			switch (data & 3)
			{
			case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;
		case 0x7800:
			prg16_89ab(data);
			break;
		default:
			LOG_MMC(("sunsoft3_w uncaught write, offset: %04x, data: %02x\n", offset, data));
			break;
	}
}

/*************************************************************

 Taito TC0190FMC + board emulation

 Games: Akira, Bakushou!! Jinsei Gekijou, Don Doko Don,
 Insector X, Operation Wolf, Power Blazer, Takeshi no
 Sengoku Fuuunji

 iNES: mapper 33

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tc0190fmc_w)
{
	LOG_MMC(("tc0190fmc_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
			chr2_0(data, CHRROM);
			break;
		case 0x0003:
			chr2_2(data, CHRROM);
			break;
		case 0x2000:
			chr1_4(data, CHRROM);
			break;
		case 0x2001:
			chr1_5(data, CHRROM);
			break;
		case 0x2002:
			chr1_6(data, CHRROM);
			break;
		case 0x2003:
			chr1_7(data, CHRROM);
			break;
	}
}

/*************************************************************

 Taito TC0190FMC + PAL16R4 board emulation

 Games: Bakushou!! Jinsei Gekijou 3, Bubble Bobble 2,
 Captain Saver, Don Doko Don 2, Flintstones, Jetsons

 This is basically Mapper 33 + IRQ. Notably, IRQ works the
 same as MMC3 irq, BUT latch values are "inverted" (XOR'ed
 with 0xff) and there is a little delay (not implemented yet)
 We simply use MMC3 IRQ and XOR the value written in the
 register 0xc000 below

 iNES: mapper 48

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tc0190fmc_p16_w)
{
	LOG_MMC(("tc0190fmc_p16_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
			tc0190fmc_w(space, offset, data, mem_mask);
			break;
		case 0x4000:
			m_IRQ_count_latch = (0x100 - data) & 0xff;
			break;
		case 0x4001:
			m_IRQ_count = m_IRQ_count_latch;
			break;
		case 0x4002:
			m_IRQ_enable = 1;
			break;
		case 0x4003:
			m_IRQ_enable = 0;
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*************************************************************

 Taito X1-005 board emulation

 Actually, Fudou Myouou Den uses a variant of the board with
 CIRAM, making necessary two distinct mappers & pcb_id.

 Also, we miss to emulate the security check at 0x7ef8 / 0x7ef9
 and the 0x80 ram!

 iNES: mappers 80 & 207

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::x1005_m_w)
{
	LOG_MMC(("x1005_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ef0:
			chr2_0((data & 0x7f) >> 1, CHRROM);
			break;
		case 0x1ef1:
			chr2_2((data & 0x7f) >> 1, CHRROM);
			break;
		case 0x1ef2:
			chr1_4(data, CHRROM);
			break;
		case 0x1ef3:
			chr1_5(data, CHRROM);
			break;
		case 0x1ef4:
			chr1_6(data, CHRROM);
			break;
		case 0x1ef5:
			chr1_7(data, CHRROM);
			break;
		case 0x1ef6:
		case 0x1ef7:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			break;
		case 0x1ef8:
		case 0x1ef9:
			m_mmc_latch1 = data;
			break;
		case 0x1efa:
		case 0x1efb:
			prg8_89(data);
			break;
		case 0x1efc:
		case 0x1efd:
			prg8_ab(data);
			break;
		case 0x1efe:
		case 0x1eff:
			prg8_cd(data);
			break;
		default:
			logerror("mapper80_m_w uncaught addr: %04x, value: %02x\n", offset + 0x6000, data);
			break;
	}

	if (offset >= 0x1f00 && m_mapper_ram != NULL && m_mmc_latch1 == 0xa3)
		m_mapper_ram[offset & (m_mapper_ram_size - 1)] = data;
	else if (offset >= 0x1f00 && m_mapper_bram != NULL && m_mmc_latch1 == 0xa3)
		m_mapper_bram[offset & (m_mapper_bram_size - 1)] = data;
}

READ8_MEMBER(nes_carts_state::x1005_m_r)
{
	LOG_MMC(("x1005a_m_r, offset: %04x\n", offset));

	if (offset >= 0x1f00 && m_mapper_ram != NULL && m_mmc_latch1 == 0xa3)
		return m_mapper_ram[offset & (m_mapper_ram_size - 1)];
	else if (offset >= 0x1f00 && m_mapper_bram != NULL && m_mmc_latch1 == 0xa3)
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];

	return 0xff;
}

WRITE8_MEMBER(nes_carts_state::x1005a_m_w)
{
	LOG_MMC(("x1005a_m_w, offset: %04x, data: %02x\n", offset, data));

	// similar to x1005_m_w but mirroring is handled differently
	if (offset == 0x1ef6 || offset == 0x1ef7)
		return;

	switch (offset)
	{
		case 0x1ef0:
			set_nt_page(0, CIRAM, (data & 0x80) ? 1 : 0, 1);
			set_nt_page(1, CIRAM, (data & 0x80) ? 1 : 0, 1);
			break;
		case 0x1ef1:
			set_nt_page(2, CIRAM, (data & 0x80) ? 1 : 0, 1);
			set_nt_page(3, CIRAM, (data & 0x80) ? 1 : 0, 1);
			break;
	}

	x1005_m_w(space, offset, data, mem_mask);
}

/*************************************************************

 Taito X1-017 board emulation

 We miss to emulate the security check at 0x6000-0x73ff
 and the ram!

 Games: Kyuukyoku Harikiri Koushien, Kyuukyoku Harikiri
 Stadium, SD Keiji - Blader

 iNES: mapper 82

 In MESS: Supported.

 *************************************************************/

static void x1017_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (state->m_mmc_latch1)
	{
		state->chr2_4(state->m_mmc_vrom_bank[0] >> 1, CHRROM);
		state->chr2_6(state->m_mmc_vrom_bank[1] >> 1, CHRROM);
	}
	else
	{
		state->chr2_0(state->m_mmc_vrom_bank[0] >> 1, CHRROM);
		state->chr2_2(state->m_mmc_vrom_bank[1] >> 1, CHRROM);
	}
	state->chr1_x(4 ^ state->m_mmc_latch1, state->m_mmc_vrom_bank[2], CHRROM);
	state->chr1_x(5 ^ state->m_mmc_latch1, state->m_mmc_vrom_bank[3], CHRROM);
	state->chr1_x(6 ^ state->m_mmc_latch1, state->m_mmc_vrom_bank[4], CHRROM);
	state->chr1_x(7 ^ state->m_mmc_latch1, state->m_mmc_vrom_bank[5], CHRROM);
}

WRITE8_MEMBER(nes_carts_state::x1017_m_w)
{
	UINT8 reg = offset & 0x07;
	LOG_MMC(("x1017_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ef0:
		case 0x1ef1:
			if (m_mmc_vrom_bank[reg] != data)
			{
				m_mmc_vrom_bank[reg] = data;
				x1017_set_chr(machine());
			}
			break;
		case 0x1ef2:
		case 0x1ef3:
		case 0x1ef4:
		case 0x1ef5:
			if (m_mmc_vrom_bank[reg] != data)
			{
				m_mmc_vrom_bank[reg] = data;
				x1017_set_chr(machine());
			}
			break;
		case 0x1ef6:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
			m_mmc_latch1 = ((data & 0x02) << 1);
			x1017_set_chr(machine());
			break;
		case 0x1ef7:
		case 0x1ef8:
		case 0x1ef9:
			m_mmc_reg[(offset & 0x0f) - 7] = data;
			break;
		case 0x1efa:
			prg8_89(data >> 2);
			break;
		case 0x1efb:
			prg8_ab(data >> 2);
			break;
		case 0x1efc:
			prg8_cd(data >> 2);
			break;
		default:
			logerror("x1017_m_w uncaught write, addr: %04x, value: %02x\n", offset + 0x6000, data);
			break;
	}
}

READ8_MEMBER(nes_carts_state::x1017_m_r)
{
	LOG_MMC(("x1017_m_r, offset: %04x\n", offset));

	// 2+2+1 KB of Internal RAM can be independently enabled/disabled!
	if (offset < 0x0800 && m_mapper_bram != NULL && m_mmc_reg[0] == 0xca)
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];
	if (offset < 0x1000 && m_mapper_bram != NULL && m_mmc_reg[1] == 0x69)
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];
	if (offset < 0x1800 && m_mapper_bram != NULL && m_mmc_reg[2] == 0x84)
		return m_mapper_bram[offset & (m_mapper_bram_size - 1)];

	return 0xff;
}

/*************************************************************

      MISC UNLICENSED BOARDS

 *************************************************************/

/*************************************************************

 AGCI 50282 bootleg board emulation

 Games: Death Race

 iNES: mapper 144

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::agci_50282_w)
{
	LOG_MMC(("agci_50282_w, offset: %04x, data: %02x\n", offset, data));

	offset += 0x8000;
	data |= (space.read_byte(offset) & 1);

	chr8(data >> 4, CHRROM);
	prg32(data);
}

/*************************************************************

 AVE NINA-001 board emulation

 iNES: mapper 34

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::nina01_m_w)
{
	LOG_MMC(("nina01_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ffd:
			prg32(data);
			break;
		case 0x1ffe:
			chr4_0(data, CHRROM);
			break;
		case 0x1fff:
			chr4_4(data, CHRROM);
			break;
	}
}

/*************************************************************

 AVE NINA-003, NINA-006 and MB-91 boards emulation

 Games: Krazy Kreatures, Poke Block, Puzzle, Pyramid,
 Solitaire, Ultimate League Soccer

 iNES: mapper 79

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::nina06_l_w)
{
	LOG_MMC(("nina06_l_w, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x0100))
	{
		prg32(data >> 3);
		chr8(data, CHRROM);
	}
}

/*************************************************************

 Active Entertainment Action 52 board emulation

 iNES: mapper 228

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ae_act52_w)
{
	int pbank, cbank;
	UINT8 pmode;
	LOG_MMC(("ae_act52_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	cbank = (data & 0x03) | ((offset & 0x0f) << 2);
	chr8(cbank, CHRROM);

	pmode = offset & 0x20;
	pbank = (offset & 0x1fc0) >> 6;
	if (pmode)
	{
		prg16_89ab(pbank);
		prg16_cdef(pbank);
	}
	else
		prg32(pbank >> 1);
}


/*************************************************************

 C & E Bootleg Board for Decathlon

 Games: Decathlon

 Pretty simple mapper: writes to 0x8065-0x80a4 set prg32 to
 data & 3; writes to 0x80a5-0x80e4 set chr8 to data & 7

 iNES: mapper 244

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cne_decathl_w)
{
	LOG_MMC(("cne_decathl_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x0065)
		return;
	if (offset < 0x00a5)
	{
		prg32((offset - 0x0065) & 0x03);
		return;
	}
	if (offset < 0x00e5)
	{
		chr8((offset - 0x00a5) & 0x07, CHRROM);
	}
}

/*************************************************************

 C & E Bootleg Board for Fong Shen Bang

 Games: Fong Shen Bang - Zhu Lu Zhi Zhan

 Simple mapper: writes to 0x6000-0x67ff set PRG and CHR banks.
 Namely, 0x6000->0x6003 select resp. prg8_89, prg8_ab, prg8_cd
 and prg8_ef. 0x6004->0x6007 select resp. crh2_0, chr2_2,
 chr2_4 and chr2_6. In 0x6800-0x7fff lies WRAM.

 iNES: mapper 246

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cne_fsb_m_w)
{
	LOG_MMC(("cne_fsb_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x0800)
	{
		switch (offset & 0x0007)
		{
			case 0x0000:
				prg8_89(data);
				break;
			case 0x0001:
				prg8_ab(data);
				break;
			case 0x0002:
				prg8_cd(data);
				break;
			case 0x0003:
				prg8_ef(data);
				break;
			case 0x0004:
				chr2_0(data, CHRROM);
				break;
			case 0x0005:
				chr2_2(data, CHRROM);
				break;
			case 0x0006:
				chr2_4(data, CHRROM);
				break;
			case 0x0007:
				chr2_6(data, CHRROM);
				break;
		}
	}
	else
		m_battery_ram[offset] = data;
}

/*************************************************************

 C & E Bootleg Board for Sheng Huo Lie Zhuan

 Games: Jing Ke Xin Zhuan, Sheng Huo Lie Zhuan

 Simple Mapper: writes to 0x4020-0x5fff sets prg32 to
 data>>4 and chr8 to data&f. We currently do not map
 writes to 0x4020-0x40ff (to do: verify if this produces
 issues)

 iNES: mapper 240

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cne_shlz_l_w)
{
	LOG_MMC(("cne_shlz_l_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 4);
	chr8(data & 0x0f, CHRROM);
}

/*************************************************************

 Caltron 6 in 1 Board

 Games: 6 in 1 by Caltron

 iNES: mapper 41

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::caltron6in1_m_w)
{
	LOG_MMC(("caltron6in1_m_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_latch1 = offset & 0xff;
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	prg32(offset & 0x07);
}

WRITE8_MEMBER(nes_carts_state::caltron6in1_w)
{
	LOG_MMC(("caltron6in1_w, offset: %04x, data: %02x\n", offset, data));

	if (m_mmc_latch1 & 0x04)
		chr8(((m_mmc_latch1 & 0x18) >> 1) | (data & 0x03), CHRROM);
}

/*************************************************************

 Camerica Boards (BF9093, BF9097, BF909X, ALGNV11)

 Games: Linus Spacehead's Cosmic Crusade, Micro Machines,
 Mig-29, Stunt Kids

 To emulate NT mirroring for BF9097 board (missing in BF9093)
 we use crc_hack, however Fire Hawk is broken (but without
 mirroring there would be no helicopter graphics).

 iNES: mapper 71

 In MESS: Partially Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bf9093_w)
{
	LOG_MMC(("bf9093_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
			if (!m_hard_mirroring)
				set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
			break;
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			prg16_89ab(data);
			break;
	}
}

/*************************************************************

 Camerica BF9096 & ALGQV11 Boards

 Games: Quattro Adventure, Quattro Arcade, Quattro Sports

 Writes to 0x8000-0x9fff set prg block to (data&0x18)>>1,
 writes to 0xa000-0xbfff set prg page to data&3. selected
 prg are: prg16_89ab = block|page, prg_cdef = 3|page.
 For more info on the hardware to bypass the NES lockout, see
 Kevtris' Camerica Mappers documentation.

 iNES: mapper 232

 In MESS: Supported.

 *************************************************************/

static void bf9096_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->prg16_89ab((state->m_mmc_latch2 & 0x03) | ((state->m_mmc_latch1 & 0x18) >> 1));
	state->prg16_cdef(0x03 | ((state->m_mmc_latch1 & 0x18) >> 1));
}

WRITE8_MEMBER(nes_carts_state::bf9096_w)
{
	LOG_MMC(("bf9096_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x2000)
		m_mmc_latch1 = data;
	else
		m_mmc_latch2 = data;

	bf9096_set_prg(machine());
}

/*************************************************************

 Camerica Golden Five board

 Games: Pegasus 5 in 1

 iNES: mapper 104

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::golden5_w)
{
	LOG_MMC(("golden5_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		if (data & 0x08)
		{
			m_mmc_prg_bank[0] = ((data & 0x07) << 4) | (m_mmc_prg_bank[0] & 0x0f);
			prg16_89ab(m_mmc_prg_bank[0]);
			prg16_cdef(((data & 0x07) << 4) | 0x0f);
		}

	}
	else
	{
		m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0x70) | (data & 0x0f);
		prg16_89ab(m_mmc_prg_bank[0]);
	}
}

/*************************************************************

 Cony Bootleg Board

 Games: Dragon Ball Party, Fatal Fury 2, Street Blaster II
 Pro, World Heroes 2

 iNES: mapper 83

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::cony_l_w)
{
	LOG_MMC(("cony_l_w, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x1000 && offset < 0x1103) // from 0x5100-0x51ff
		m_mapper83_low_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_carts_state::cony_l_r)
{
	LOG_MMC(("cony_l_r, offset: %04x\n", offset));

	if (offset == 0x0f00)   // 0x5000
	{
		// read dipswitch bit! - currently unimplemented
	}
	if (offset >= 0x1000 && offset < 0x1103) // from 0x5100-0x51ff
		return m_mapper83_low_reg[offset & 0x03];
	else
		return 0x00;
}

static void cony_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->prg16_89ab(state->m_mapper83_reg[8] & 0x3f);
	state->prg16_cdef((state->m_mapper83_reg[8] & 0x30) | 0x0f);
}

static void cony_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	// FIXME: here we emulate at least 3 different boards!!!
	// one board switches 1k VROM banks only
	// one writes to 0x8000 and then switches 2k VROM banks only
	// one writes to 0x831n (n=2,3,4,5) and then switches 2k VROM banks only
	// we should split them and possibly document the proper behavior of each variant
	if (state->m_mmc_latch1 && !state->m_mmc_latch2)
	{
		state->chr2_0(state->m_mapper83_reg[0], CHRROM);
		state->chr2_2(state->m_mapper83_reg[1], CHRROM);
		state->chr2_4(state->m_mapper83_reg[6], CHRROM);
		state->chr2_6(state->m_mapper83_reg[7], CHRROM);
	}
	else
	{
		state->chr1_0(state->m_mapper83_reg[0] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_1(state->m_mapper83_reg[1] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_2(state->m_mapper83_reg[2] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_3(state->m_mapper83_reg[3] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_4(state->m_mapper83_reg[4] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_5(state->m_mapper83_reg[5] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_6(state->m_mapper83_reg[6] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
		state->chr1_7(state->m_mapper83_reg[7] | ((state->m_mapper83_reg[8] & 0x30) << 4), CHRROM);
	}
}

WRITE8_MEMBER(nes_carts_state::cony_w)
{
	LOG_MMC(("cony_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			m_mmc_latch1 = 1;
		case 0x3000:
		case 0x30ff:
		case 0x31ff:
			m_mapper83_reg[8] = data;
			cony_set_prg(machine());
			cony_set_chr(machine());
			break;
		case 0x0100:
			m_mmc_reg[0] = data & 0x80;
			switch (data & 0x03)
			{
			case 0:
				set_nt_mirroring(PPU_MIRROR_VERT);
				break;
			case 1:
				set_nt_mirroring(PPU_MIRROR_HORZ);
				break;
			case 2:
				set_nt_mirroring(PPU_MIRROR_LOW);
				break;
			case 3:
				set_nt_mirroring(PPU_MIRROR_HIGH);
				break;
			}
			break;
		case 0x0200:
			m_IRQ_count = (m_IRQ_count & 0xff00) | data;
			break;
		case 0x0201:
			m_IRQ_enable = m_mmc_reg[0];
			m_IRQ_count = (data << 8) | (m_IRQ_count & 0xff);
			break;
		case 0x0300:
			prg8_89(data);
			break;
		case 0x0301:
			prg8_ab(data);
			break;
		case 0x0302:
			prg8_cd(data);
			break;
		case 0x0312:
		case 0x0313:
		case 0x0314:
		case 0x0315:
			m_mmc_latch2 = 1;
		case 0x0310:
		case 0x0311:
		case 0x0316:
		case 0x0317:
			m_mapper83_reg[offset - 0x0310] = data;
			cony_set_chr(machine());
			break;
		case 0x0318:
			m_mapper83_reg[9] = data;
			cony_set_prg(machine());
			break;
	}
}

/*************************************************************

 Yoko Bootleg Board

 Games: Mortal Kombat II, Master Figther VI'


 Very similar to Cony board

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::yoko_l_w)
{
	LOG_MMC(("cony_l_w, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x1300) // from 0x5400
		m_mapper83_low_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_carts_state::yoko_l_r)
{
	LOG_MMC(("cony_l_r, offset: %04x\n", offset));

	if (offset >= 0x0f00 && offset < 0x1300)    // 0x5000
	{
		// read dipswitch bit! - currently unimplemented
	}
	if (offset >= 0x1300) // from 0x5400
		return m_mapper83_low_reg[offset & 0x03];
	else
		return 0x00;
}

static void yoko_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (state->m_mmc_reg[0] & 0x10)
	{
		int base = (state->m_mmc_reg[1] & 0x08) << 1;
		state->prg8_89(base | (state->m_mapper83_reg[0] & 0x0f));
		state->prg8_ab(base | (state->m_mapper83_reg[1] & 0x0f));
		state->prg8_cd(base | (state->m_mapper83_reg[2] & 0x0f));
		state->prg8_ef(base | 0x0f);
	}
	else if (state->m_mmc_reg[0] & 0x08)
		state->prg32(state->m_mmc_reg[1] >> 1);
	else
	{
		state->prg16_89ab(state->m_mmc_reg[1]);
		state->prg16_cdef(0xff);
	}
}

static void yoko_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->chr2_0(state->m_mapper83_reg[4], CHRROM);
	state->chr2_2(state->m_mapper83_reg[5], CHRROM);
	state->chr2_4(state->m_mapper83_reg[6], CHRROM);
	state->chr2_6(state->m_mapper83_reg[7], CHRROM);
}

WRITE8_MEMBER(nes_carts_state::yoko_w)
{
	LOG_MMC(("yoko_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0c17)
	{
		case 0x0000:
			m_mmc_reg[1] = data;
			yoko_set_prg(machine());
			break;
		case 0x400:
			m_mmc_reg[0] = data;
			if (data & 1)
				set_nt_mirroring(PPU_MIRROR_HORZ);
			else
				set_nt_mirroring(PPU_MIRROR_VERT);
			yoko_set_prg(machine());
			break;
		case 0x0800:
			m_IRQ_count = (m_IRQ_count & 0xff00) | data;
			break;
		case 0x0801:
			m_IRQ_enable = m_mmc_reg[0] & 0x80;
			m_IRQ_count = (data << 8) | (m_IRQ_count & 0xff);
			break;
		case 0x0c00:
		case 0x0c01:
		case 0x0c02:
			m_mapper83_reg[offset & 3] = data;
			yoko_set_prg(machine());
			break;
		case 0x0c10:
		case 0x0c11:
		case 0x0c16:
		case 0x0c17:
			m_mapper83_reg[4 + (offset & 3)] = data;
			yoko_set_chr(machine());
			break;
	}
}

/*************************************************************

 Board DREAMTECH01

 Games: Korean Igo

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::dreamtech_l_w)
{
	LOG_MMC(("dreamtech_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1020)   /* 0x5020 */
		prg16_89ab(data);
}

/*************************************************************

 Bootleg Board by Fukutake

 Games: Study Box

 iNES: mapper 186

 In MESS: Unsupported.


 *************************************************************/

WRITE8_MEMBER(nes_carts_state::fukutake_l_w)
{
	LOG_MMC(("fukutake_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x200 && offset < 0x400)
	{
		if (offset & 1)
			prg16_89ab(data);
		else
			wram_bank(data >> 6, NES_WRAM);
	}
	else if (offset >= 0x400 && offset < 0xf00)
		m_mapper_ram[offset - 0x400] = data;
}

READ8_MEMBER(nes_carts_state::fukutake_l_r)
{
	LOG_MMC(("fukutake_l_r offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x200 && offset < 0x400)
	{
		if (offset == 0x200 || offset == 0x201 || offset == 0x203)
			return 0x00;
		else if (offset == 0x202)
			return 0x40;
		else
			return 0xff;
	}
	else if (offset >= 0x400 && offset < 0xf00)
		return m_mapper_ram[offset - 0x400];

	return 0;
}

/*************************************************************

 Bootleg Board by Future Media

 Games: Crayon Shin-chan (C), San Guo Zhi 4 - Chi Bi Feng Yun

 iNES: mapper 117

 In MESS: Unsupported.

 *************************************************************/

static void futuremedia_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	//  if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		if (state->m_IRQ_enable && state->m_IRQ_count)
		{
			state->m_IRQ_count--;
			if (!state->m_IRQ_count)
				state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
	}
}

WRITE8_MEMBER(nes_carts_state::futuremedia_w)
{
	LOG_MMC(("futuremedia_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
			prg8_cd(data);
			break;
		case 0x0003:
			prg8_ef(data);
			break;
		case 0x2000:
		case 0x2001:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
			chr1_x(offset & 0x07, data, CHRROM);
			break;

		case 0x5000:
			set_nt_mirroring(BIT(data, 0) ?  PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x4001:
			m_IRQ_count_latch = data;
			break;
		case 0x4002:
			// IRQ cleared
			break;
		case 0x4003:
			m_IRQ_count = m_IRQ_count_latch;
			break;
		case 0x6000:
			m_IRQ_enable = data & 0x01;
			break;
	}
}

/*************************************************************

 Bootleg Board 37017 (?) by Gouder

 Games: Street Fighter IV

 MMC3 clone. It also requires reads from 0x5000-0x7fff.

 iNES: mapper 208

 In MESS: Preliminary Support.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::gouder_sf4_l_w)
{
	static const UINT8 conv_table[256] =
	{
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	LOG_MMC(("gouder_sf4_l_w, offset: %04x, data: %02x\n", offset, data));

	if (!(offset < 0x1700))
		m_mmc_reg[offset & 0x03] = data ^ conv_table[m_mmc_reg[4]];
	else if (!(offset < 0xf00))
		m_mmc_reg[4] = data;
	else if (!(offset < 0x700))
		prg32(((data >> 3) & 0x02) | (data & 0x01));
}

READ8_MEMBER(nes_carts_state::gouder_sf4_l_r)
{
	LOG_MMC(("gouder_sf4_l_r, offset: %04x\n", offset));

	if (!(offset < 0x1700))
		return m_mmc_reg[offset & 0x03];

	return 0x00;
}

/* writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (beacuse it is handled by low writes) */
static void gouder_sf4_prg_cb( running_machine &machine, int start, int bank )
{
	return;
}


/*************************************************************

 Bootleg Board by Henggedianzi

 Games: Mei Guo Fu Hao, Shang Gu Shen Jian , Wang Zi Fu
 Chou Ji

 Writes to 0x8000-0xffff set prg32. Moreover, data&0x20 sets
 NT mirroring.

 iNES: mapper 177

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::henggedianzi_w)
{
	LOG_MMC(("henggedianzi_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 Bootleg Board by Henggedianzi

 Games: Xing He Zhan Shi

 Writes to 0x5000-0x5fff set prg32 banks, writes to 0x8000-
 0xffff set NT mirroring

 iNES: mapper 179

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::heng_xjzb_l_w)
{
	LOG_MMC(("heng_xjzb_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x4100;

	if (offset & 0x5000)
		prg32(data >> 1);
}

WRITE8_MEMBER(nes_carts_state::heng_xjzb_w)
{
	LOG_MMC(("heng_xjzb_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 Bootleg Board by HES (also used by others)

 Games: AV Hanafuda Club, AV Soccer, Papillon, Sidewinder,
 Total Funpack

 Actually, two variant: one for HES 6-in-1 with mirroring control
 and one for AV Soccer and others with hardwired mirroring

 iNES: mapper 113

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::hes6in1_l_w)
{
	LOG_MMC(("hes6in1_l_w, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x100))
	{
		prg32((data & 0x38) >> 3);
		chr8((data & 0x07) | ((data & 0x40) >> 3), CHRROM);
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}

WRITE8_MEMBER(nes_carts_state::hes_l_w)
{
	LOG_MMC(("hes_l_w, offset: %04x, data: %02x\n", offset, data));

	if (!(offset & 0x100))
	{
		prg32((data & 0x38) >> 3);
		chr8((data & 0x07) | ((data & 0x40) >> 3), CHRROM);
	}
}

/*************************************************************

 Bootleg Board by Hosenkan

 Games: Pocahontas, Super Donkey Kong

 iNES: mapper 182

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::hosenkan_w)
{
	LOG_MMC(("hosenkan_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0001:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			m_mmc_latch1 = data;
			break;
		case 0x4000:
			switch (m_mmc_latch1)
		{
			case 0:
				chr2_0(data >> 1, CHRROM);
				break;
			case 1:
				chr1_5(data, CHRROM);
				break;
			case 2:
				chr2_2(data >> 1, CHRROM);
				break;
			case 3:
				chr1_7(data, CHRROM);
				break;
			case 4:
				prg8_89(data);
				break;
			case 5:
				prg8_ab(data);
				break;
			case 6:
				chr1_4(data, CHRROM);
				break;
			case 7:
				chr1_6(data, CHRROM);
				break;
		}
			break;
		case 0x6003:
			if (data)
			{
				m_IRQ_count = data;
				m_IRQ_enable = 1;
			}
			else
				m_IRQ_enable = 0;
			break;
	}
}

/*************************************************************

 Kaiser Board KS7058

 Games: Tui Do Woo Ma Jeung

 Writes to 0xf000-0xffff set 4k chr banks. Namely, if
 offset&0x80 is 0 the lower 4k are set, if it is 1 the
 upper 4k are set.

 iNES: mapper 171

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ks7058_w)
{
	LOG_MMC(("ks7058_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7080)
	{
		case 0x7000:
			chr4_0(data, CHRROM);
			break;
		case 0x7080:
			chr4_4(data, CHRROM);
			break;
	}
}

/*************************************************************

 Kaiser Board KS7022

 Games: 15 in 1

 iNES: mapper 175

 In MESS: Supported?

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ks7022_w)
{
	LOG_MMC(("ks7022_w, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0)
		set_nt_mirroring(BIT(data, 2) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset == 0x2000)
		m_mmc_latch1 = data & 0x0f;
}

READ8_MEMBER(nes_carts_state::ks7022_r)
{
	LOG_MMC(("ks7022_r, offset: %04x\n", offset));

	if (offset == 0x7ffc)
	{
		chr8(m_mmc_latch1, CHRROM);
		prg16_89ab(m_mmc_latch1);
		prg16_cdef(m_mmc_latch1);
	}

	return mmc_hi_access_rom(machine(), offset);
}

/*************************************************************

 Kaiser Board KS7032

 Games:

 iNES:

 In MESS:

 *************************************************************/

static void ks7032_prg_update( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->prg8_67(state->m_mmc_reg[4]);
	state->prg8_89(state->m_mmc_reg[1]);
	state->prg8_ab(state->m_mmc_reg[2]);
	state->prg8_cd(state->m_mmc_reg[3]);
}

static void ks7032_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count >= (0xffff - 114))
		{
			state->m_IRQ_enable = 0;
			state->m_IRQ_count = state->m_IRQ_count_latch;
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			state->m_IRQ_count += 114;
	}
}

WRITE8_MEMBER(nes_carts_state::ks7032_w)
{
	LOG_MMC(("ks7032_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_IRQ_count = (m_IRQ_count & 0xfff0) | (data & 0x0f);
			break;
		case 0x1000:
			m_IRQ_count = (m_IRQ_count & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x2000:
			m_IRQ_count = (m_IRQ_count & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x3000:
			m_IRQ_count = (m_IRQ_count & 0x0fff) | ((data & 0x0f) << 12);
			break;
		case 0x4000:
			m_IRQ_enable = 1;
			break;
		case 0x6000:
			m_mmc_latch1 = data & 0x07;
			break;
		case 0x7000:
			m_mmc_reg[m_mmc_latch1] = data;
			ks7032_prg_update(machine());
			break;
	}
}

/*************************************************************

 Kaiser Board KS202

 Games:

 iNES:

 In MESS: Supported?

 *************************************************************/


WRITE8_MEMBER(nes_carts_state::ks202_w)
{
	LOG_MMC(("ks202_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_IRQ_count = (m_IRQ_count & 0xfff0) | (data & 0x0f);
			break;
		case 0x1000:
			m_IRQ_count = (m_IRQ_count & 0xff0f) | ((data & 0x0f) << 4);
			break;
		case 0x2000:
			m_IRQ_count = (m_IRQ_count & 0xf0ff) | ((data & 0x0f) << 8);
			break;
		case 0x3000:
			m_IRQ_count = (m_IRQ_count & 0x0fff) | ((data & 0x0f) << 12);
			break;
		case 0x4000:
			m_IRQ_enable = 1;
			break;
		case 0x6000:
			m_mmc_latch1 = data & 0x07;
			break;
		case 0x7000:
			m_mmc_reg[m_mmc_latch1] = data;
			ks7032_prg_update(machine());
			switch (offset & 0xc00)
			{
			case 0x800:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;
			case 0xc00:
				chr1_x(offset & 0x07, data, CHRROM);
				break;
			}
			break;
	}
}

/*************************************************************

 Kaiser Board KS7017

 Games:

 iNES:

 In MESS: Not working

 *************************************************************/

static void mmc_fds_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count <= 114)
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_enable = 0;
			state->m_IRQ_status |= 0x01;
		}
		else
			state->m_IRQ_count -= 114;
	}
}

WRITE8_MEMBER(nes_carts_state::ks7017_l_w)
{
	LOG_MMC(("ks7022_w, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset >= 0xa00 && offset < 0xb00)
		m_mmc_latch1 = ((offset >> 2) & 0x03) | ((offset >> 4) & 0x04);

	if (offset >= 0x1000 && offset < 0x1100)
		prg16_89ab(m_mmc_latch1);
}

WRITE8_MEMBER(nes_carts_state::ks7017_extra_w)
{
	LOG_MMC(("ks7017_extra_w, offset: %04x, data: %02x\n", offset, data));

	offset += 0x20;

	if (offset == 0x0020) /* 0x4020 */
		m_IRQ_count = (m_IRQ_count & 0xff00) | data;

	if (offset == 0x0021) /* 0x4021 */
		m_IRQ_count = (m_IRQ_count & 0x00ff) | (data << 8);

	if (offset == 0x0025) /* 0x4025 */
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

READ8_MEMBER(nes_carts_state::ks7017_extra_r)
{
	LOG_MMC(("ks7017_extra_r, offset: %04x\n", offset));

	m_IRQ_status &= ~0x01;
	return m_IRQ_status;
}

/*************************************************************

 Bootleg Board by Kay (for Panda Prince)

 Games: The Panda Prince, Sonic 3d Blast 6, SFZ2 '97, YuYu '97
 (and its title hack MK6), UMK3, Super Lion King 2

 MMC3 clone. This is basically KOF96 board + protection

 iNES: mapper 121

 In MESS: Most game works, with some graphical issues.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::kay_pp_l_w)
{
	LOG_MMC(("kay_pp_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		switch (data & 0x03)
		{
			case 0x00:
			case 0x01:
				m_mmc_reg[0] = 0x83;
				break;
			case 0x02:
				m_mmc_reg[0] = 0x42;
				break;
			case 0x03:
				m_mmc_reg[0] = 0x00;
				break;
		}
	}
}

READ8_MEMBER(nes_carts_state::kay_pp_l_r)
{
	LOG_MMC(("kay_pp_l_r, offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
		return m_mmc_reg[0];
	else
		return 0xff;
}

static void kay_pp_update_regs( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	switch (state->m_mmc_reg[5] & 0x3f)
	{
		case 0x20:
		case 0x29:
		case 0x2b:
		case 0x3f:
			state->m_mmc_reg[7] = 1;
			state->m_mmc_reg[1] = state->m_mmc_reg[6];
			break;
		case 0x26:
			state->m_mmc_reg[7] = 0;
			state->m_mmc_reg[1] = state->m_mmc_reg[6];
			break;
		case 0x2c:
			state->m_mmc_reg[7] = 1;
			if (state->m_mmc_reg[6])
				state->m_mmc_reg[1] = state->m_mmc_reg[6];
			break;

		case 0x28:
			state->m_mmc_reg[7] = 0;
			state->m_mmc_reg[2] = state->m_mmc_reg[6];
			break;

		case 0x2a:
			state->m_mmc_reg[7] = 0;
			state->m_mmc_reg[3] = state->m_mmc_reg[6];
			break;

		case 0x2f:
			break;

		default:
			state->m_mmc_reg[5] = 0;
			break;
	}
}

static void kay_pp_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (state->m_mmc_reg[5] & 0x3f)
	{
		state->prg8_x(start, bank & 0x3f);
		state->prg8_ef(state->m_mmc_reg[1]);
		state->prg8_cd(state->m_mmc_reg[2]);
		state->prg8_ab(state->m_mmc_reg[3]);
	}
	else
		state->prg8_x(start, bank & 0x3f);
}

static void kay_pp_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc3_latch & 0x80) >> 5;

	if ((start & 0x04) == chr_page)
		bank |= 0x100;

	state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::kay_pp_w)
{
	LOG_MMC(("kay_pp_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6003)
	{
		case 0x0000:
			txrom_w(space, offset, data, mem_mask);
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

		case 0x0001:
			m_mmc_reg[6] = (BIT(data, 0) << 5) | (BIT(data, 1) << 4) | (BIT(data, 2) << 3)
								| (BIT(data, 3) << 2) | (BIT(data, 4) << 1) | BIT(data, 5);
			if (!m_mmc_reg[7])
				kay_pp_update_regs(machine());
			txrom_w(space, offset, data, mem_mask);
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

		case 0x0003:
			m_mmc_reg[5] = data;
			kay_pp_update_regs(machine());
			txrom_w(space, 0x0000, data, mem_mask);
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board by Kasing

 Games: AV Jiu Ji Mahjong, Bao Qing Tian, Thunderbolt 2,
 Shisen Mahjong 2

 MMC3 clone

 iNES: mapper 115

 In MESS: Supported

 *************************************************************/

static void kasing_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (BIT(state->m_mmc_reg[0], 7))
		state->prg32(state->m_mmc_reg[0] >> 1);
	else
		state->prg8_x(start, bank);
}

WRITE8_MEMBER(nes_carts_state::kasing_m_w)
{
	LOG_MMC(("kasing_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x01)
	{
		case 0x00:
			m_mmc_reg[0] = data;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;
		case 0x01:
			m_mmc_chr_base = (data & 0x01) ? 0x100 : 0x000;
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board by Magic Series

 Games: Magic Dragon

 Very simple mapper: writes to 0x8000-0xffff set prg32 and chr8
 banks

 iNES: mapper 107

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::magics_md_w)
{
	LOG_MMC(("magics_md_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 1);
	chr8(data, CHRROM);
}

/*************************************************************

 Bootleg Board by Nanjing

 Games: A lot of pirate originals

 iNES: mapper 163

 In MESS: Unsupported.

 *************************************************************/

static void nanjing_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (BIT(state->m_mmc_reg[0], 7))
	{
		if (scanline == 127)
		{
			state->chr4_0(1, CHRRAM);
			state->chr4_4(1, CHRRAM);
		}

		if (scanline == 239)
		{
			state->chr4_0(0, CHRRAM);
			state->chr4_4(0, CHRRAM);
		}
	}

}

WRITE8_MEMBER(nes_carts_state::nanjing_l_w)
{
	LOG_MMC(("nanjing_l_w, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset < 0x1000)
		return;

	if (offset == 0x1100)   // 0x5100
	{
		if (data == 6)
			prg32(3);
		return;
	}

	if (offset == 0x1101)   // 0x5101
	{
		UINT8 temp = m_mmc_count;
		m_mmc_count = data;

		if (temp & !data)
			m_mmc_latch2 ^= 0xff;
	}

	switch (offset & 0x300)
	{
		case 0x000:
		case 0x200:
			m_mmc_reg[BIT(offset, 9)] = data;
			if (!BIT(m_mmc_reg[0], 7) && m_ppu->get_current_scanline() <= 127)
				chr8(0, CHRRAM);
			break;
		case 0x300:
			m_mmc_latch1 = data;
			break;
	}

	prg32((m_mmc_reg[0] & 0x0f) | ((m_mmc_reg[1] & 0x0f) << 4));
}

READ8_MEMBER(nes_carts_state::nanjing_l_r)
{
	UINT8 value = 0;
	LOG_MMC(("nanjing_l_r, offset: %04x\n", offset));

	offset += 0x100;

	if (offset < 0x1000)
		return 0;

	switch (offset & 0x700)
	{
		case 0x100:
			value = m_mmc_latch1;
			break;
		case 0x500:
			value = m_mmc_latch2 & m_mmc_latch1;
			break;
		case 0x000:
		case 0x200:
		case 0x300:
		case 0x400:
		case 0x600:
		case 0x700:
			value = 4;
			break;
	}
	return value;
}

/*************************************************************

 Bootleg Board by Nitra

 Games: Time Diver Avenger

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 250

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::nitra_w)
{
	LOG_MMC(("nitra_w, offset: %04x, data: %02x\n", offset, data));

	txrom_w(space, (offset & 0x6000) | ((offset & 0x400) >> 10), offset & 0xff, mem_mask);
}

/*************************************************************

 NTDEC ASDER Bootleg Board

 Games: Cobra Mission, Fighting Hero III, Huang Di, Master
 Shooter

 iNES: mapper 112

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ntdec_asder_w)
{
	LOG_MMC(("ntdec_asder_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			m_mmc_latch1 = data & 0x07;
			break;
		case 0x2000:
			switch (m_mmc_latch1)
		{
			case 0:
				prg8_89(data);
				break;
			case 1:
				prg8_ab(data);
				break;
			case 2:
				data &= 0xfe;
				chr1_0(data, CHRROM);
				chr1_1(data + 1, CHRROM);
				break;
			case 3:
				data &= 0xfe;
				chr1_2(data, CHRROM);
				chr1_3(data + 1, CHRROM);
				break;
			case 4:
				chr1_4(data, CHRROM);
				break;
			case 5:
				chr1_5(data, CHRROM);
				break;
			case 6:
				chr1_6(data, CHRROM);
				break;
			case 7:
				chr1_7(data, CHRROM);
				break;
		}
			break;
		case 0x6000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*************************************************************

 Bootleg Board by NTDEC for Fighting Hero

 Games: Fighting Hero

 Very simple mapper: writes to 0x6000-0x7fff swap PRG and
 CHR banks.

 iNES: mapper 193

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ntdec_fh_m_w)
{
	LOG_MMC(("ntdec_fh_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x03)
	{
		case 0:
			chr4_0(data >> 2, CHRROM);
			break;
		case 1:
			chr2_4(data >> 1, CHRROM);
			break;
		case 2:
			chr2_6(data >> 1 , CHRROM);
			break;
		case 3:
			prg8_89(data);
			break;
	}
}

/*************************************************************

 Open Corp DAOU306 board

 Games: Metal Force (K)

 iNES: mapper 156

 In MESS: Supported.

 Notes: Metal Force and Buzz & Waldog only use the the first
 4 regs and no mirroring. Janggun ui Adeul uses all features

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::daou306_w)
{
	LOG_MMC(("daou306_w, offset: %04x, data: %02x\n", offset, data));
	int reg = BIT(offset, 2) ? 8 : 0;

	switch (offset)
	{
		case 0x4000:
		case 0x4004:
			m_mmc_reg[reg + 0] = data;
			chr1_0(m_mmc_reg[0] | (m_mmc_reg[8] << 8), CHRROM);
			break;
		case 0x4001:
		case 0x4005:
			m_mmc_reg[reg + 1] = data;
			chr1_1(m_mmc_reg[1] | (m_mmc_reg[9] << 8), CHRROM);
			break;
		case 0x4002:
		case 0x4006:
			m_mmc_reg[reg + 2] = data;
			chr1_2(m_mmc_reg[2] | (m_mmc_reg[10] << 8), CHRROM);
			break;
		case 0x4003:
		case 0x4007:
			m_mmc_reg[reg + 3] = data;
			chr1_3(m_mmc_reg[3] | (m_mmc_reg[11] << 8), CHRROM);
			break;
		case 0x4008:
		case 0x400c:
			m_mmc_reg[reg + 4] = data;
			chr1_4(m_mmc_reg[4] | (m_mmc_reg[12] << 8), CHRROM);
			break;
		case 0x4009:
		case 0x400d:
			m_mmc_reg[reg + 5] = data;
			chr1_5(m_mmc_reg[5] | (m_mmc_reg[13] << 8), CHRROM);
			break;
		case 0x400a:
		case 0x400e:
			m_mmc_reg[reg + 6] = data;
			chr1_6(m_mmc_reg[6] | (m_mmc_reg[14] << 8), CHRROM);
			break;
		case 0x400b:
		case 0x400f:
			m_mmc_reg[reg + 7] = data;
			chr1_7(m_mmc_reg[7] | (m_mmc_reg[15] << 8), CHRROM);
			break;
		case 0x4010:
			prg16_89ab(data);
			break;
		case 0x4014:
			if (data & 1)
				set_nt_mirroring(PPU_MIRROR_HORZ);
			else
				set_nt_mirroring(PPU_MIRROR_VERT);
			break;
	}
}

/*************************************************************

 RCM GS2015 Board

 Games: Bonza, Magic Jewelry 2

 Very simple mapper: writes to 0x8000-0xffff sets prg32
 to offset and chr8 to offset>>1 (when chrrom is present)

 iNES: mapper 216

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::gs2015_w)
{
	LOG_MMC(("gs2015_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset);
	chr8(offset >> 1, m_mmc_chr_source);
}

/*************************************************************

 Bootleg Board by RCM for Tetris Family

 Games: Tetris Family 9 in 1, 20 in 1

 Simple Mapper: prg/chr/nt are swapped depending on the offset
 of writes in 0x8000-0xffff. offset&0x80 set NT mirroring,
 when (offset&0x30) is 0,3 prg32 is set; when it is 1,2
 two 16k prg banks are set. See below for the values used in
 these banks.

 iNES: mapper 61

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::rcm_tf_w)
{
	LOG_MMC(("rcm_tf_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x30)
	{
		case 0x00:
		case 0x30:
			prg32(offset & 0x0f);
			break;
		case 0x10:
		case 0x20:
			prg16_89ab(((offset & 0x0f) << 1) | ((offset & 0x20) >> 4));
			prg16_cdef(((offset & 0x0f) << 1) | ((offset & 0x20) >> 4));
			break;
	}
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 Bootleg Board by Rex Soft

 Games: Dragon Ball Z 5, Dragon Ball Z Super

 MMC3 clone

 iNES: mapper 12

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::rex_dbz_l_w)
{
	LOG_MMC(("rex_dbz_l_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_reg[0] = data;
	mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
}

/* we would need to use this read handler in 0x6000-0x7fff as well */
READ8_MEMBER(nes_carts_state::rex_dbz_l_r)
{
	LOG_MMC(("rex_dbz_l_r, offset: %04x\n", offset));
	return 0x01;
}

static void rex_dbz_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int shift = (start < 4) ? 8 : 4;

	bank |= ((state->m_mmc_reg[0] << shift) & 0x100);
	state->chr1_x(start, bank, source);
}

/*************************************************************

 Rex Soft SL1632 Board

 Games: Samurai Spirits

 MMC3 clone

 iNES: mapper 14

 In MESS: Supported

 *************************************************************/

static void rex_sl1632_set_prg( running_machine &machine, int prg_base, int prg_mask )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (state->m_mmc_reg[0] & 0x02)
	{
		mmc3_set_prg(machine, prg_base, prg_mask);
	}
	else
	{
		state->prg8_89(state->m_mmc_extra_bank[0]);
		state->prg8_ab(state->m_mmc_extra_bank[1]);
		state->prg8_cd(state->m_mmc_extra_bank[2]);
		state->prg8_ef(state->m_mmc_extra_bank[3]);
	}
}

static void rex_sl1632_set_chr( running_machine &machine, UINT8 chr, int chr_base, int chr_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	static const UINT8 conv_table[8] = {5, 5, 5, 5, 3, 3, 1, 1};
	UINT8 chr_page = (state->m_mmc3_latch & 0x80) >> 5;
	UINT8 bank[8];
	UINT8 chr_base2[8];
	int i;

	if (state->m_mmc_reg[0] & 0x02)
	{
		for(i = 0; i < 8; i++)
		{
			bank[i] = state->m_mmc_vrom_bank[i];
			chr_base2[i] = chr_base | ((state->m_mmc_reg[0] << conv_table[i]) & 0x100);
		}
	}
	else
	{
		for(i = 0; i < 8; i++)
		{
			bank[i] = state->m_mmc_extra_bank[i + 4];   // first 4 state->m_mmc_extra_banks are PRG
			chr_base2[i] = chr_base;
		}
	}

	state->chr1_x(chr_page ^ 0, chr_base2[0] | (bank[0] & chr_mask), chr);
	state->chr1_x(chr_page ^ 1, chr_base2[1] | (bank[1] & chr_mask), chr);
	state->chr1_x(chr_page ^ 2, chr_base2[2] | (bank[2] & chr_mask), chr);
	state->chr1_x(chr_page ^ 3, chr_base2[3] | (bank[3] & chr_mask), chr);
	state->chr1_x(chr_page ^ 4, chr_base2[4] | (bank[4] & chr_mask), chr);
	state->chr1_x(chr_page ^ 5, chr_base2[5] | (bank[5] & chr_mask), chr);
	state->chr1_x(chr_page ^ 6, chr_base2[6] | (bank[6] & chr_mask), chr);
	state->chr1_x(chr_page ^ 7, chr_base2[7] | (bank[7] & chr_mask), chr);
}

WRITE8_MEMBER(nes_carts_state::rex_sl1632_w)
{
	UINT8 map14_helper1, map14_helper2, mmc_helper, cmd;
	LOG_MMC(("rex_sl1632_w, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x2131)
	{
		m_mmc_reg[0] = data;
		rex_sl1632_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		rex_sl1632_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);

		if (!(m_mmc_reg[0] & 0x02))
			set_nt_mirroring(BIT(m_mmc_reg[1], 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	}

	if (m_mmc_reg[0] & 0x02)
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				mmc_helper = m_mmc3_latch ^ data;
				m_mmc3_latch = data;

				/* Has PRG Mode changed? */
				if (mmc_helper & 0x40)
					rex_sl1632_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

				/* Has CHR Mode changed? */
				if (mmc_helper & 0x80)
					rex_sl1632_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
				break;

			case 0x0001:
				cmd = m_mmc3_latch & 0x07;
				switch (cmd)
				{
				case 0: case 1: // these have to be changed due to the different way rex_sl1632_set_chr works (it handles 1k banks)!
					m_mmc_vrom_bank[2 * cmd] = data;
					m_mmc_vrom_bank[2 * cmd + 1] = data;
					rex_sl1632_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
					break;
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd + 2] = data;
					rex_sl1632_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					rex_sl1632_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
					break;
				}
				break;

			case 0x2000:
				set_nt_mirroring(BIT(m_mmc_reg[1], 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
				break;

			default:
				txrom_w(space, offset, data, mem_mask);
				break;
		}
	}
	else if (offset >= 0x3000 && offset <= 0x6003 )
	{
		map14_helper1 = (offset & 0x01) << 2;
		offset = ((offset & 0x02) | (offset >> 10)) >> 1;
		map14_helper2 = ((offset + 2) & 0x07) + 4; // '+4' because first 4 m_mmc_extra_banks are for PRG!
		m_mmc_extra_bank[map14_helper2] = (m_mmc_extra_bank[map14_helper2] & (0xf0 >> map14_helper1)) | ((data & 0x0f) << map14_helper1);
		rex_sl1632_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
	else
	{
		switch (offset & 0x7003)
		{
			case 0x0000:
			case 0x2000:
				m_mmc_extra_bank[offset >> 13] = data;
				rex_sl1632_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;

			case 0x1000:
				m_mmc_reg[1] = data;
				set_nt_mirroring(BIT(m_mmc_reg[1], 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
		}
	}
}

/*************************************************************

 Rumblestation Board

 Games: Rumblestation 15 in 1

 iNES: mapper 46

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::rumblestation_m_w)
{
	LOG_MMC(("rumblestation_m_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & 0x01) | ((data & 0x0f) << 1);
	m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x07) | ((data & 0xf0) >> 1);
	prg32(m_mmc_prg_bank[0]);
	chr8(m_mmc_vrom_bank[0], CHRROM);
}

WRITE8_MEMBER(nes_carts_state::rumblestation_w)
{
	LOG_MMC(("rumblestation_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_prg_bank[0] = (m_mmc_prg_bank[0] & ~0x01) | (data & 0x01);
	m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x07) | ((data & 0x70) >> 4);
	prg32(m_mmc_prg_bank[0]);
	chr8(m_mmc_vrom_bank[0], CHRROM);
}

/*************************************************************

 Sachen 74x374 bootleg boards

 Games: Chess Academy, Chinese Checkers Jpn, Mahjong Academy,
 Olympic IQ, Poker II, Tasac [150], Poker III [243]

 iNES: mappers 150 & 243

 *************************************************************/

static void sachen_set_mirror( running_machine &machine, UINT8 nt ) // used by mappers 137, 138, 139, 141
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (nt)
	{
		case 0:
		case 1:
			state->set_nt_mirroring(nt ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 2:
			state->set_nt_page(0, CIRAM, 0, 1);
			state->set_nt_page(1, CIRAM, 1, 1);
			state->set_nt_page(2, CIRAM, 1, 1);
			state->set_nt_page(3, CIRAM, 1, 1);
			break;
		case 3:
			state->set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		default:
			LOG_MMC(("Mapper set NT to invalid value %02x", nt));
			break;
	}
}

WRITE8_MEMBER(nes_carts_state::sachen_74x374_l_w)
{
	LOG_MMC(("sachen_74x374_l_w, offset: %04x, data: %02x\n", offset, data));

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_mmc_latch1 = data & 0x07;
		else
		{
			switch (m_mmc_latch1)
			{
				case 0x02:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x08) | ((data << 3) & 0x08);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					prg32(data & 0x01);
					break;
				case 0x04:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x04) | ((data << 2) & 0x04);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					break;
				case 0x05:
					prg32(data & 0x07);
					break;
				case 0x06:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x03) | ((data << 0) & 0x03);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					break;
				case 0x07:
					sachen_set_mirror(machine(), (data >> 1) & 0x03);
					break;
				default:
					break;
			}
		}
	}
}

READ8_MEMBER(nes_carts_state::sachen_74x374_l_r)
{
	LOG_MMC(("sachen_74x374_l_r, offset: %04x", offset));

	/* read  happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		return (~m_mmc_latch1 & 0x3f) /* ^ dips*/;  // we would need to check the Dips here
	else
		return 0;
}

WRITE8_MEMBER(nes_carts_state::sachen_74x374a_l_w)
{
	LOG_MMC(("sachen_74x374a_l_w, offset: %04x, data: %02x\n", offset, data));

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			m_mmc_latch1 = data;
		else
		{
			switch (m_mmc_latch1 & 0x07)
			{
				case 0x00:
					prg32(0);
					chr8(3, CHRROM);
					break;
				case 0x02:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x08) | ((data << 3) & 0x08);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					break;
				case 0x04:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x01) | ((data << 0) & 0x01);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					break;
				case 0x05:
					prg32(data & 0x01);
					break;
				case 0x06:
					m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & ~0x06) | ((data << 1) & 0x06);
					chr8(m_mmc_vrom_bank[0], CHRROM);
					break;
				case 0x07:
					sachen_set_mirror(machine(), BIT(data, 0));
					break;
				default:
					break;
			}
		}
	}
}

/*************************************************************

 Sachen S8259 bootleg boards

 iNES: mapper 141 (A), 138 (B), 139 (C), 137 (D)

 *************************************************************/

static void common_s8259_write_handler( address_space &space, offs_t offset, UINT8 data, int board )
{
	running_machine &machine = space.machine();
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 bank_helper1, bank_helper2, shift, add1, add2, add3;

	/* write happens only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
	{
		if (!(offset & 0x01))
			state->m_mmc_latch1 = data & 0x07;
		else
		{
			state->m_sachen_reg[state->m_mmc_latch1] = data;

			switch (state->m_mmc_latch1)
			{
				case 0x05:
					state->prg32(data);
					break;
				case 0x07:
					sachen_set_mirror(machine, BIT(data, 0) ? 0 : (data >> 1) & 0x03);
					break;
				default:
					if (board == SACHEN_8259D)
					{
						if (state->m_mmc_chr_source == CHRROM)
						{
							state->chr1_0((state->m_sachen_reg[0] & 0x07), CHRROM);
							state->chr1_1((state->m_sachen_reg[1] & 0x07) | (state->m_sachen_reg[4] << 4 & 0x10), CHRROM);
							state->chr1_2((state->m_sachen_reg[2] & 0x07) | (state->m_sachen_reg[4] << 3 & 0x10), CHRROM);
							state->chr1_3((state->m_sachen_reg[3] & 0x07) | (state->m_sachen_reg[4] << 2 & 0x10) | (state->m_sachen_reg[6] << 3 & 0x08), CHRROM);
						}
					}
					else
					{
						bank_helper1 = state->m_sachen_reg[7] & 0x01;
						bank_helper2 = (state->m_sachen_reg[4] & 0x07) << 3;
						shift = (board == SACHEN_8259A) ? 1 : (board == SACHEN_8259C) ? 2 : 0;
						add1 = (board == SACHEN_8259B) ? 0 : 1;
						add2 = (board == SACHEN_8259C) ? 2 : 0;
						add3 = (board == SACHEN_8259A) ? 1 : (board == SACHEN_8259C) ? 3 : 0;

						if (state->m_mmc_chr_source == CHRROM)
						{
							state->chr2_0(((state->m_sachen_reg[bank_helper1 ? 0 : 0] & 0x07) | bank_helper2) << shift, CHRROM);
							state->chr2_2(((state->m_sachen_reg[bank_helper1 ? 0 : 1] & 0x07) | bank_helper2) << shift | add1, CHRROM);
							state->chr2_4(((state->m_sachen_reg[bank_helper1 ? 0 : 2] & 0x07) | bank_helper2) << shift | add2, CHRROM);
							state->chr2_6(((state->m_sachen_reg[bank_helper1 ? 0 : 3] & 0x07) | bank_helper2) << shift | add3, CHRROM);
						}
					}
					break;
			}
		}
	}
}

WRITE8_MEMBER(nes_carts_state::s8259_l_w)
{
	LOG_MMC(("s8259_w, type: %d, offset: %04x, data: %02x\n", m_pcb_id, offset, data));

	common_s8259_write_handler(space, offset, data, m_pcb_id);
}

WRITE8_MEMBER(nes_carts_state::s8259_m_w)
{
	LOG_MMC(("s8259_w, type: %d, offset: %04x, data: %02x\n", m_pcb_id, offset, data));

	common_s8259_write_handler(space, (offset + 0x100) & 0xfff, data, m_pcb_id);
}


/*************************************************************

 Sachen SA009 bootleg boards

 Games: Pipe 5

 iNES: mapper 160

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sa009_l_w)
{
	LOG_MMC(("sa009_l_w, offset: %04x, data: %02x\n", offset, data));

	chr8(data, m_mmc_chr_source);
}

/*************************************************************

 Sachen SA0036 bootleg boards

 Games: Taiwan Mahjong 16

 iNES: mapper 149

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sa0036_w)
{
	LOG_MMC(("sa0036_w, offset: %04x, data: %02x\n", offset, data));

	chr8(data >> 7, CHRROM);
}

/*************************************************************

 Sachen SA0037 bootleg boards

 Games: Mahjong World, Shisen Mahjong

 iNES: mapper 148

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sa0037_w)
{
	LOG_MMC(("sa0037_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 3);
	chr8(data, CHRROM);
}

/*************************************************************

 Sachen SA72007 bootleg boards

 Games: Sidewinder

 iNES: mapper 145

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sa72007_l_w)
{
	LOG_MMC(("sa72007_l_w, offset: %04x, data: %02x\n", offset, data));

	/* only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		chr8(data >> 7, CHRROM);
}

/*************************************************************

 Sachen SA72008 bootleg boards

 Games: Jovial Race, Qi Wang

 iNES: mapper 133

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sa72008_l_w)
{
	LOG_MMC(("sa72008_l_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 2);
	chr8(data, CHRROM);
}

/*************************************************************

 Sachen TCA-01 bootleg boards

 iNES: mapper 143

 Games: Dancing Blocks, Magic Mathematic

 In MESS: Supported.

 *************************************************************/

READ8_MEMBER(nes_carts_state::tca01_l_r)
{
	LOG_MMC(("tca01_l_r, offset: %04x\n", offset));

	/* the address is read only if we are at 0x4100 + k * 0x200, but 0x4100 is offset = 0 */
	if (!(offset & 0x100))
		return (~offset & 0x3f) | 0x40;
	else
		return 0x00;
}

/*************************************************************

 Sachen TCU-01 bootleg boards

 Games: Challenge of the Dragon, Chinese Kungfu

 iNES: mapper 147

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tcu01_l_w)
{
	LOG_MMC(("tcu01_l_w, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x103) == 0x002)
	{
		prg32(((data >> 6) & 0x02) | ((data >> 2) & 0x01));
		chr8(data >> 3, CHRROM);
	}
}

WRITE8_MEMBER(nes_carts_state::tcu01_m_w)
{
	LOG_MMC(("tcu01_m_w, offset: %04x, data: %02x\n", offset, data));

	tcu01_l_w(space, (offset + 0x100) & 0xfff, data, mem_mask);
}

WRITE8_MEMBER(nes_carts_state::tcu01_w)
{
	LOG_MMC(("tcu01_w, offset: %04x, data: %02x\n", offset, data));

	tcu01_l_w(space, (offset + 0x100) & 0xfff, data, mem_mask);
}

/*************************************************************

 Sachen TCU-02 bootleg boards

 Games: Mei Loi Siu Ji

 iNES: mapper 136

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tcu02_l_w)
{
	LOG_MMC(("tcu02_l_w, offset: %04x, data: %02x\n", offset, data));

	if ((offset & 0x103) == 0x002)
	{
		m_mmc_latch1 = (data & 0x30) | ((data + 3) & 0x0f);
		chr8(m_mmc_latch1, CHRROM);
	}
}

READ8_MEMBER(nes_carts_state::tcu02_l_r)
{
	LOG_MMC(("tcu02_l_r, offset: %04x\n", offset));

	if ((offset & 0x103) == 0x000)
		return m_mmc_latch1 | 0x40;
	else
		return 0x00;
}


/*************************************************************

 Subor bootleg board Type 0

 iNES: mapper 167

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::subor0_w)
{
	UINT8 subor_helper1, subor_helper2;
	LOG_MMC(("subor0_w, offset: %04x, data: %02x\n", offset, data));

	m_subor_reg[(offset >> 13) & 0x03] = data;
	subor_helper1 = ((m_subor_reg[0] ^ m_subor_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_subor_reg[2] ^ m_subor_reg[3]) << 0) & 0x1f;

	if (m_subor_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper1 += 1;
	}
	else if (m_subor_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x20;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

/*************************************************************

 Subor bootleg board Type 1

 iNES: mapper 166

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::subor1_w)
{
	UINT8 subor_helper1, subor_helper2;
	LOG_MMC(("subor1_w, offset: %04x, data: %02x\n", offset, data));

	m_subor_reg[(offset >> 13) & 0x03] = data;
	subor_helper1 = ((m_subor_reg[0] ^ m_subor_reg[1]) << 1) & 0x20;
	subor_helper2 = ((m_subor_reg[2] ^ m_subor_reg[3]) << 0) & 0x1f;

	if (m_subor_reg[1] & 0x08)
	{
		subor_helper1 += subor_helper2 & 0xfe;
		subor_helper2 = subor_helper1;
		subor_helper2 += 1;
	}
	else if (m_subor_reg[1] & 0x04)
	{
		subor_helper2 += subor_helper1;
		subor_helper1 = 0x1f;
	}
	else
	{
		subor_helper1 += subor_helper2;
		subor_helper2 = 0x07;
	}

	prg16_89ab(subor_helper1);
	prg16_cdef(subor_helper2);
}

/*************************************************************

 Bootleg Board by Super Game

 Games: Boogerman, Mortal Kombat III

 MMC3 clone. Also, it probably needs a hack to support both
 variants (Boogerman & MK3).

 iNES: mapper 215

 In MESS: Preliminary support.

 *************************************************************/

static void sgame_boog_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (!(state->m_mmc_reg[0] & 0x80))  // if this is != 0 we should never even arrive here
	{
		if (state->m_mmc_reg[1] & 0x08)
			bank = (bank & 0x1f) | 0x20;
		else
			bank = (bank & 0x0f) | (state->m_mmc_reg[1] & 0x10);

		state->prg8_x(start, bank);
	}
}

static void sgame_boog_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if ((state->m_mmc_reg[1] & 0x04))
		bank |= 0x100;
	else
		bank = (bank & 0x7f) | ((state->m_mmc_reg[1] & 0x10) << 3);

	state->chr1_x(start, bank, source);
}

static void sgame_boog_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (state->m_mmc_reg[0] & 0x80)
	{
		state->prg16_89ab((state->m_mmc_reg[0] & 0xf0) | (state->m_mmc_reg[1] & 0x10));
		state->prg16_cdef((state->m_mmc_reg[0] & 0xf0) | (state->m_mmc_reg[1] & 0x10));
	}
	else
		mmc3_set_prg(machine, state->m_mmc_prg_base, state->m_mmc_prg_mask);
}

WRITE8_MEMBER(nes_carts_state::sgame_boog_l_w)
{
	LOG_MMC(("sgame_boog_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_mmc_reg[0] = data;
		sgame_boog_set_prg(machine());
	}
	else if (offset == 0x1001)
	{
		m_mmc_reg[1] = data;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
	else if (offset == 0x1007)
	{
		m_mmc3_latch = 0;
		m_mmc_reg[2] = data;
		sgame_boog_set_prg(machine());
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

WRITE8_MEMBER(nes_carts_state::sgame_boog_m_w)
{
	LOG_MMC(("sgame_boog_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x0000)
	{
		m_mmc_reg[0] = data;
		sgame_boog_set_prg(machine());
	}
	else if (offset == 0x0001)
	{
		m_mmc_reg[1] = data;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
	else if (offset == 0x0007)
	{
		m_mmc3_latch = 0;
		m_mmc_reg[2] = data;
		sgame_boog_set_prg(machine());
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

WRITE8_MEMBER(nes_carts_state::sgame_boog_w)
{
	static const UINT8 conv_table[8] = {0,2,5,3,6,1,7,4};
	LOG_MMC(("sgame_boog_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x0000, data, mem_mask);
			break;

		case 0x0001:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x0001, data, mem_mask);
			else if (m_mmc_reg[3] && ((m_mmc_reg[0] & 0x80) == 0 || (m_mmc_latch1 & 0x07) < 6)) // if we use the prg16 banks and cmd=6,7 DON'T enter!
			{
				m_mmc_reg[3] = 0;
				txrom_w(space, 0x0001, data, mem_mask);
			}
			break;

		case 0x2000:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x2000, data, mem_mask);
			else
			{
				data = (data & 0xc0) | conv_table[data & 0x07];
				m_mmc_reg[3] = 1;
				txrom_w(space, 0x0000, data, mem_mask);
				break;
			}
			break;

		case 0x4000:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x4000, data, mem_mask);
			else
				set_nt_mirroring(((data >> 7) | data) & 0x01 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x4001:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x4001, data, mem_mask);
			else
				txrom_w(space, 0x6001, data, mem_mask);
			break;

		case 0x6001:
			if (!m_mmc_reg[2])
				txrom_w(space, 0x6001, data, mem_mask);
			else
			{
				txrom_w(space, 0x4000, data, mem_mask);
				txrom_w(space, 0x4001, data, mem_mask);
			}
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board by Super Game

 Games: The Lion King

 MMC3 clone.

 iNES: mapper 114

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sgame_lion_m_w)
{
	LOG_MMC(("sgame_lion_m_w, offset: %04x, data: %02x\n", offset, data));

	m_map114_reg = data;

	if (m_map114_reg & 0x80)
	{
		prg16_89ab(data & 0x1f);
		prg16_cdef(data & 0x1f);
	}
	else
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

}

WRITE8_MEMBER(nes_carts_state::sgame_lion_w)
{
	static const UINT8 conv_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};
	LOG_MMC(("sgame_lion_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x6000)
	{
		switch (offset & 0x6000)
		{
			case 0x0000:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x2000:
				m_map114_reg_enabled = 1;
				data = (data & 0xc0) | conv_table[data & 0x07];
				txrom_w(space, 0x0000, data, mem_mask);
				break;
			case 0x4000:
				if (m_map114_reg_enabled && (m_map114_reg & 0x80) == 0)
				{
					m_map114_reg_enabled = 0;
					txrom_w(space, 0x0001, data, mem_mask);
				}
				break;
		}
	}
	else
	{
		switch (offset & 0x03)
		{
			case 0x02:
				txrom_w(space, 0x6000, data, mem_mask);
				break;
			case 0x03:
				txrom_w(space, 0x6001, data, mem_mask);
				txrom_w(space, 0x4000, data, mem_mask);
				txrom_w(space, 0x4001, data, mem_mask);
				break;
		}
	}
}

/*************************************************************

 Tengen 800008 Board

 iNES: mapper 3?

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::tengen_800008_w)
{
	LOG_MMC(("tengen_800008_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data >> 3);
	chr8(data, CHRROM);
}

/*************************************************************

 Tengen 800032 Board

 Games: Klax, Road Runner, Rolling Thunder, Shinobi, Skulls
 & Croosbones, Xybots

 iNES: mapper 64

 In MESS: Partially Supported.

 *************************************************************/

static void tengen_800032_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if (!state->m_IRQ_mode) // we are in scanline mode!
	{
		if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
		{
			if (!state->m_IRQ_reset)
			{
				if (!state->m_IRQ_count)
					state->m_IRQ_count = state->m_IRQ_count_latch;
				else
				{
					state->m_IRQ_count--;
					if (state->m_IRQ_enable && !blanked && !state->m_IRQ_count)
					{
						LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
									device->machine().primary_screen->vpos(), device->machine().primary_screen->hpos()));
						state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
					}
				}
			}
			else
			{
				state->m_IRQ_reset = 0;
				state->m_IRQ_count = state->m_IRQ_count_latch + 1;
			}
		}
	}
	/* otherwise, we are in CPU cycles mode --> decrement count of 114 every scanline
	 --> in the meanwhile anything can have happened to IRQ_reset and we would not know
	 --> Skulls and Crossbones does not show anything!! */
	else
	{
		//      if (!state->m_IRQ_reset)
		{
			if (state->m_IRQ_count <= 114)
				state->m_IRQ_count = state->m_IRQ_count_latch;
			else
			{
				state->m_IRQ_count -= 114;
				if (state->m_IRQ_enable && !blanked && (state->m_IRQ_count <= 114))
				{
					LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
								device->machine().primary_screen->vpos(), device->machine().primary_screen->hpos()));
					state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				}
			}
		}
		//      else
		//      {
		//          state->m_IRQ_reset = 0;
		//          state->m_IRQ_count = state->m_IRQ_count_latch + 1;
		//      }
	}
}

static void tengen_800032_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 prg_mode = state->m_mmc_latch1 & 0x40;

	state->prg8_89(state->m_mmc_prg_bank[prg_mode ? 2: 0]);
	state->prg8_ab(state->m_mmc_prg_bank[prg_mode ? 0: 1]);
	state->prg8_cd(state->m_mmc_prg_bank[prg_mode ? 1: 2]);
}

static void tengen_800032_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc_latch1 & 0x80) >> 5;

	if (state->m_mmc_latch1 & 0x20)
	{
		state->chr1_x(0 ^ chr_page, state->m_mmc_vrom_bank[0], CHRROM);
		state->chr1_x(1 ^ chr_page, state->m_mmc_vrom_bank[8], CHRROM);
		state->chr1_x(2 ^ chr_page, state->m_mmc_vrom_bank[1], CHRROM);
		state->chr1_x(3 ^ chr_page, state->m_mmc_vrom_bank[9], CHRROM);
	}
	else
	{
		state->chr1_x(0 ^ chr_page, state->m_mmc_vrom_bank[0] & ~0x01, CHRROM);
		state->chr1_x(1 ^ chr_page, state->m_mmc_vrom_bank[0] |  0x01, CHRROM);
		state->chr1_x(2 ^ chr_page, state->m_mmc_vrom_bank[1] & ~0x01, CHRROM);
		state->chr1_x(3 ^ chr_page, state->m_mmc_vrom_bank[1] |  0x01, CHRROM);
	}

	state->chr1_x(4 ^ chr_page, state->m_mmc_vrom_bank[2], CHRROM);
	state->chr1_x(5 ^ chr_page, state->m_mmc_vrom_bank[3], CHRROM);
	state->chr1_x(6 ^ chr_page, state->m_mmc_vrom_bank[4], CHRROM);
	state->chr1_x(7 ^ chr_page, state->m_mmc_vrom_bank[5], CHRROM);
}

WRITE8_MEMBER(nes_carts_state::tengen_800032_w)
{
	UINT8 map64_helper, cmd;
	LOG_MMC(("tengen_800032_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			map64_helper = m_mmc_latch1 ^ data;
			m_mmc_latch1 = data;

			/* Has PRG Mode changed? */
			if (map64_helper & 0x40)
				tengen_800032_set_prg(machine());

			/* Has CHR Mode changed? */
			if (map64_helper & 0xa0)
				tengen_800032_set_chr(machine());
			break;

		case 0x0001:
			cmd = m_mmc_latch1 & 0x0f;
			switch (cmd)
		{
			case 0: case 1:
			case 2: case 3:
			case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				tengen_800032_set_chr(machine());
				break;
			case 6: case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				tengen_800032_set_prg(machine());
				break;
			case 8: case 9:
				m_mmc_vrom_bank[cmd - 2] = data;
				tengen_800032_set_chr(machine());
				break;
			case 0x0f:
				m_mmc_prg_bank[2] = data;
				tengen_800032_set_prg(machine());
				break;
		}
			break;

		case 0x2000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x4000:
			m_IRQ_count_latch = data;
			break;

		case 0x4001: /* $c001 - IRQ scanline latch */
			m_IRQ_mode = data & 0x01;
			m_IRQ_reset = 1;
			break;

		case 0x6000:
			m_IRQ_enable = 0;
			break;

		case 0x6001:
			m_IRQ_enable = 1;
			break;

		default:
			LOG_MMC(("Mapper 64 write. addr: %04x value: %02x\n", offset + 0x8000, data));
			break;
	}
}

/*************************************************************

 Tengen 800037 Board

 Games: Alien Syndrome

 iNES: mapper 158

 In MESS: Very preliminary support.

 *************************************************************/

// probably wrong...
static void tengen_800037_set_mirror( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 nt_mode = state->m_mmc_latch1 & 0x80;

	state->set_nt_page(0, ROM, state->m_mmc_vrom_bank[nt_mode ? 2 : 0], 0);
	state->set_nt_page(1, ROM, state->m_mmc_vrom_bank[nt_mode ? 3 : 0], 0);
	state->set_nt_page(2, ROM, state->m_mmc_vrom_bank[nt_mode ? 4 : 1], 0);
	state->set_nt_page(3, ROM, state->m_mmc_vrom_bank[nt_mode ? 5 : 1], 0);
}

WRITE8_MEMBER(nes_carts_state::tengen_800037_w)
{
	UINT8 map158_helper, cmd;
	LOG_MMC(("tengen_800037_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			map158_helper = m_mmc_latch1 ^ data;
			m_mmc_latch1 = data;

			/* Has PRG Mode changed? */
			if (map158_helper & 0x40)
				tengen_800032_set_prg(machine());

			/* Has CHR Mode changed? */
			if (map158_helper & 0xa0)
			{
				tengen_800032_set_chr(machine());
				tengen_800037_set_mirror(machine());
			}
			break;

		case 0x0001:
			cmd = m_mmc_latch1 & 0x0f;
			switch (cmd)
		{
			case 0: case 1:
			case 2: case 3:
			case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				tengen_800032_set_chr(machine());
				tengen_800037_set_mirror(machine());
				break;
			case 6: case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				tengen_800032_set_prg(machine());
				break;
			case 8: case 9:
				m_mmc_vrom_bank[cmd - 2] = data;
				tengen_800032_set_chr(machine());
				tengen_800037_set_mirror(machine());
				break;
			case 0x0f:
				m_mmc_prg_bank[2] = data;
				tengen_800032_set_prg(machine());
				break;
		}
			break;

		case 0x2000:
			break;

		default:
			tengen_800032_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board 22211 by TXC (Type A)

 Games: Creatom

 Info from NEStopia: this mapper features write to four
 registers (0x4100-0x4103). The third one is used to select
 PRG and CHR banks.

 iNES: mapper 132

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::txc_22211_l_w)
{
	LOG_MMC(("txc_22211_l_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 4)
		m_txc_reg[offset & 0x03] = data;
}

READ8_MEMBER(nes_carts_state::txc_22211_l_r)
{
	LOG_MMC(("txc_22211_l_r, offset: %04x\n", offset));

	if (offset == 0x0000)
		return (m_txc_reg[1] ^ m_txc_reg[2]) | 0x40;
	else
		return 0x00;
}

WRITE8_MEMBER(nes_carts_state::txc_22211_w)
{
	LOG_MMC(("txc_22211_w, offset: %04x, data: %02x\n", offset, data));

	prg32(m_txc_reg[2] >> 2);
	chr8(m_txc_reg[2], CHRROM);
}

/*************************************************************

 Bootleg Board 22211 by TXC (Type B)

 Games: 1991 Du Ma Racing

 This mapper is basically the same as Type A. Only difference is
 in the way CHR banks are selected (see below)

 iNES: mapper 172

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::txc_22211b_w)
{
	LOG_MMC(("txc_22211b_w, offset: %04x, data: %02x\n", offset, data));

	prg32(m_txc_reg[2] >> 2);
	chr8((((data ^ m_txc_reg[2]) >> 3) & 0x02) | (((data ^ m_txc_reg[2]) >> 5) & 0x01), CHRROM);
}

/*************************************************************

 Bootleg Board 22211 by TXC (Type C)

 Games: Mahjong Block, Xiao Ma Li

 This mapper is basically the same as 132 too. Only difference is
 in 0x4100 reads which expect also bit 0 to be set

 iNES: mapper 172

 In MESS: Supported.

 *************************************************************/

READ8_MEMBER(nes_carts_state::txc_22211c_l_r)
{
	LOG_MMC(("txc_22211c_l_r, offset: %04x\n", offset));

	if (offset == 0x0000)
		return (m_txc_reg[1] ^ m_txc_reg[2]) | 0x41;
	else
		return 0x00;
}

/*************************************************************

 Bootleg Board 'Thunder Warrior' by TXC

 Games: Master Fighter II, Master Fighter 3, Thunder Warrior

 MMC3 clone

 iNES: mapper 189

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::txc_tw_l_w)
{
	LOG_MMC(("txctw_l_w, offset: %04x, data: %02x\n", offset, data));

	prg32((data >> 4) | data);
}

WRITE8_MEMBER(nes_carts_state::txc_tw_m_w)
{
	LOG_MMC(("txctw_m_w, offset: %04x, data: %04x\n", offset, data));

	txc_tw_l_w(space, offset & 0xff, data, mem_mask);   // offset does not really count for this mapper
}

/* writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (beacuse it is handled by low writes) */
static void txc_tw_prg_cb( running_machine &machine, int start, int bank )
{
	return;
}

/*************************************************************

 Bootleg Board 'Strike Wolf' by TXC

 Games: Strike Wolf (also Policeman?? according to Nestopia)

 iNES: mapper 36

 Known Boards: Bootleg Board by TXC

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::txc_strikewolf_w)
{
	LOG_MMC(("txc_strikewolf_w, offset: %04x, data: %02x\n", offset, data));

	if ((offset >= 0x400) && (offset < 0x7fff))
	{
		prg32(data >> 4);
		chr8(data & 0x0f, CHRROM);
	}
}

/*************************************************************

 Bootleg Board MXMDHTWO by TXC

 Games: Commandos, Journey to the West, Ma Bu Mi Zhen &
 Qu Wei Cheng Yu Wu, Si Lu Chuan Qi

 Simple Mapper: writes to 0x8000-0xffff sets the prg32 bank.
 Not sure if returning 0x50 for reads in 0x4100-0x5000 is correct.

 iNES: mapper 241

 In MESS: Supported.

 *************************************************************/

READ8_MEMBER(nes_carts_state::txc_mxmdhtwo_l_r)
{
	return 0x50;
}

WRITE8_MEMBER(nes_carts_state::txc_mxmdhtwo_w)
{
	LOG_MMC(("txc_mxmdhtwo_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data);
}

/*************************************************************

 Waixing Board Type A

 Games: Columbus - Ougon no Yoake (C), Ji Jia Zhan Shi,
 Jia A Fung Yun, Wei Luo Chuan Qi

 This mapper is quite similar to MMC3, but with two differences:
 mirroring is not the same, and when VROM banks 8,9 are accessed
 they point to CHRRAM and not CHRROM.

 iNES: mapper 74

 In MESS: Supported

 *************************************************************/

/* MIRROR_LOW and MIRROR_HIGH are swapped! */
static void waixing_set_mirror( running_machine &machine, UINT8 nt )
{
	nes_state *state = machine.driver_data<nes_state>();
	switch (nt)
	{
		case 0:
		case 1:
			state->set_nt_mirroring(nt ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 2:
			state->set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case 3:
			state->set_nt_mirroring(PPU_MIRROR_HIGH);
			break;
		default:
			LOG_MMC(("Mapper set NT to invalid value %02x", nt));
			break;
	}
}

/* Luo Ke Ren X only works with this */
static void waixing_a_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = (bank <= 9) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

/* Ji Jia Zhan Shi only works with this */
static void waixing_a1_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = ((bank == 8) || (bank == 9)) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

WRITE8_MEMBER(nes_carts_state::waixing_a_w)
{
	LOG_MMC(("waixing_a_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2000:
			waixing_set_mirror(machine(), data);    //maybe data & 0x03?
			break;

		case 0x2001:
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Waixing Board Type B

 Games: Sugoro Quest (C)

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 191

 In MESS: Supported.

 *************************************************************/

static void waixing_b_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = BIT(bank, 7) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

/*************************************************************

 Waixing Board Type C

 Games: Ying Lie Qun Xia Zhuan, Young Chivalry

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 192

 In MESS: Supported.

 *************************************************************/

static void waixing_c_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = ((bank == 0x08) || (bank == 0x09) || (bank == 0x0a) || (bank == 0x0b)) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

/*************************************************************

 Waixing Board Type D

 Games: Super Robot Taisen (C)

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 194

 In MESS: Supported.

 *************************************************************/

static void waixing_d_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = (bank < 0x02) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

/*************************************************************

 Waixing Board Type E

 Games: Captain Tsubasa Vol. II (C), Chaos World, God
 Slayer (C), Zu Qiu Xiao Jiang

 MMC3 clone. This is a minor modification of Mapper 74,
 in the sense that it is the same board except for the
 CHRRAM pages.

 iNES: mapper 195

 In MESS: Supported.

 *************************************************************/

static void waixing_e_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = (bank < 0x04) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

/*************************************************************

 Waixing Board Type F

 Games: Tenchi wo Kurau II (C)

 MMC3 clone.

 iNES: mapper 198

 In MESS: Preliminary support.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_f_w)
{
	UINT8 cmd;
	LOG_MMC(("waixing_f_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_mmc_latch1 & 0x07;
			if (cmd >= 6)
			{
				m_mmc_prg_bank[cmd - 6] = data & ((data > 0x3f) ? 0x4f : 0x3f);
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			}
			else
				waixing_a_w(space, offset, data, mem_mask);
			break;

		default:
			waixing_a_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Waixing Board Type G

 Games: San Guo Zhi 2, Dragon Ball Z Gaiden (C), Dragon
 Ball Z II (C)

 MMC3 clone

 iNES: mapper 199

 In MESS: Supported.

 *************************************************************/

static void waixing_g_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int chr_src = (bank < 0x08) ? CHRRAM : CHRROM;
	state->chr1_x(start, bank, chr_src);
}

static void waixing_g_set_chr( running_machine &machine, int chr_base, int chr_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc_latch1 & 0x80) >> 5;

	state->m_mmc3_chr_cb(machine, chr_page ^ 0, chr_base | (state->m_mmc_vrom_bank[0] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 1, chr_base | (state->m_mmc_vrom_bank[6] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 2, chr_base | (state->m_mmc_vrom_bank[1] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 3, chr_base | (state->m_mmc_vrom_bank[7] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 4, chr_base | (state->m_mmc_vrom_bank[2] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 5, chr_base | (state->m_mmc_vrom_bank[3] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 6, chr_base | (state->m_mmc_vrom_bank[4] & chr_mask), state->m_mmc_chr_source);
	state->m_mmc3_chr_cb(machine, chr_page ^ 7, chr_base | (state->m_mmc_vrom_bank[5] & chr_mask), state->m_mmc_chr_source);
}

WRITE8_MEMBER(nes_carts_state::waixing_g_w)
{
	UINT8 MMC3_helper, cmd;
	LOG_MMC(("waixing_g_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			MMC3_helper = m_mmc_latch1 ^ data;
			m_mmc_latch1 = data;

			/* Has PRG Mode changed? */
			if (MMC3_helper & 0x40)
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			/* Has CHR Mode changed? */
			if (MMC3_helper & 0x80)
				waixing_g_set_chr(machine(), m_mmc_chr_base, m_mmc_chr_mask);
			break;

		case 0x0001:
			cmd = m_mmc_latch1 & 0x0f;
			switch (cmd)
			{
			case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
			case 2: case 3: case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				waixing_g_set_chr(machine(), m_mmc_chr_base, m_mmc_chr_mask);
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				m_mmc_prg_bank[cmd - 6] = data;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;
			case 0x0a: case 0x0b:
				m_mmc_vrom_bank[cmd - 4] = data;
				waixing_g_set_chr(machine(), m_mmc_chr_base, m_mmc_chr_mask);
				break;
			}
			break;

		default:
			waixing_a_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Waixing Board Type H

 Games: Ying Xiong Yuan Yi Jing Chuan Qi, Yong Zhe Dou E
 Long - Dragon Quest VII

 MMC3 clone. More info to come.

 iNES: mapper 245

 In MESS: Supported.

 *************************************************************/

static void waixing_h_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	if (source == CHRROM)
		state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::waixing_h_w)
{
	UINT8 cmd;
	LOG_MMC(("waixing_h_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_mmc3_latch & 0x07;
			switch (cmd)
			{
			case 0:     // in this case we set prg_base in addition to m_mmc_vrom_bank!
				m_mmc_prg_base = (data << 5) & 0x40;
				m_mmc_prg_mask = 0x3f;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				txrom_w(space, offset, data, mem_mask);
			default:
				txrom_w(space, offset, data, mem_mask);
				break;
			}
			break;

		case 0x2001:
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Waixing San Guo Zhi Board

 Games: San Guo Zhi

 This board uses Konami IRQ

 iNES: mapper 252

 In MESS: Unsupported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_sgz_w)
{
	UINT8 mmc_helper, bank;
	LOG_MMC(("waixing_sgz_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + ((offset & 0x0008) >> 3);
			mmc_helper = offset & 0x04;
			if (mmc_helper)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x0f) << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
		case 0x7000:
			switch (offset & 0x0c)
			{
			case 0x00:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0xf0) | (data & 0x0f);
				break;
			case 0x04:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x08:
				m_IRQ_enable = data & 0x02;
				m_IRQ_enable_latch = data & 0x01;
				if (data & 0x02)
					m_IRQ_count = m_IRQ_count_latch;
				break;
			case 0x0c:
				m_IRQ_enable = m_IRQ_enable_latch;
				break;
			}
			break;
	}
}


/*************************************************************

 Waixing San Guo Zhong Lie Zhuan Board

 Games: Fan Kong Jing Ying, San Guo Zhong Lie Zhuan, Xing
 Ji Zheng Ba

 iNES: mapper 178

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_sgzlz_l_w)
{
	LOG_MMC(("waixing_sgzlz_l_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x700:
			set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x701:
			m_mmc_latch1 = (m_mmc_latch1 & 0x0c) | ((data >> 1) & 0x03);
			prg32(m_mmc_latch1);
			break;
		case 0x702:
			m_mmc_latch1 = (m_mmc_latch1 & 0x03) | ((data << 2) & 0x0c);
			break;
	}
}

/*************************************************************

 Waixing Final Fantasy V Board

 Games: Darkseed, Digital Dragon, Final Fantasy V, Pocket
 Monster Red

 iNES: mapper 164

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_ffv_l_w)
{
	UINT8 mmc_helper;
	LOG_MMC(("waixing_ffv_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100; /* the checks work better on addresses */

	if (0x1000 == (offset & 0x1200))
	{
		m_mmc_reg[BIT(offset, 8)] = data;
		mmc_helper = BIT(m_mmc_reg[1], 0) << 5;
		switch (m_mmc_reg[0] & 0x70)
		{
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				prg16_89ab(mmc_helper | ((m_mmc_reg[0] >> 1) & 0x10) | (m_mmc_reg[0] & 0x0f));
				prg16_cdef(mmc_helper & 0x1f);
				break;
			case 0x50:
				prg32((mmc_helper >> 1) | (m_mmc_reg[0] & 0x0f));
				break;
			case 0x70:
				prg16_89ab(mmc_helper | ((m_mmc_reg[0] << 1) & 0x10) | (m_mmc_reg[0] & 0x0f));
				prg16_cdef(mmc_helper & 0x1f);
				break;
		}
	}
}

/*************************************************************

 Waixing Zhan Shi Board

 Games: Wai Xing Zhan Shi

 Simple mapper: writes to 0x8000-0xffff sets prg32 banks to
 (offset>>3)&f. written data&3 sets the mirroring (with
 switched high/low compared to the standard one).

 A crc check is required to support Dragon Quest VIII (which
 uses a slightly different board)

 iNES: mapper 242

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_zs_w)
{
	LOG_MMC(("waixing_zs_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 3);

	switch (data & 0x03)
	{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

/*************************************************************

 Waixing Dragon Quest VIII Board

 Games: Dragon Quest VIII

 Simple mapper: writes to 0x8000-0xffff sets prg32 banks to
 (offset>>3)&f.

 iNES: mapper 242

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_dq8_w)
{
	LOG_MMC(("waixing_dq8_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 3);
}


/*************************************************************

 Waixing PS2 board

 Games: Bao Xiao Tien Guo, Bio Hazard, Pokemon Gold, Subor (R)

 iNES: mapper 15

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::waixing_ps2_w)
{
	UINT8 map15_flip = (data & 0x80) >> 7;
	UINT8 map15_helper = (data & 0x7f) << 1;

	LOG_MMC(("waixing_ps2_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	switch (offset & 0x0fff)
	{
		case 0x000:
			prg8_89((map15_helper + 0) ^ map15_flip);
			prg8_ab((map15_helper + 1) ^ map15_flip);
			prg8_cd((map15_helper + 2) ^ map15_flip);
			prg8_ef((map15_helper + 3) ^ map15_flip);
			break;
		case 0x001:
			map15_helper |= map15_flip;
			prg8_89(map15_helper);
			prg8_ab(map15_helper + 1);
			prg8_cd(map15_helper + 1);
			prg8_ef(map15_helper + 1);
			break;
		case 0x002:
			map15_helper |= map15_flip;
			prg8_89(map15_helper);
			prg8_ab(map15_helper);
			prg8_cd(map15_helper);
			prg8_ef(map15_helper);
			break;
		case 0x003:
			map15_helper |= map15_flip;
			prg8_89(map15_helper);
			prg8_ab(map15_helper + 1);
			prg8_cd(map15_helper);
			prg8_ef(map15_helper + 1);
			break;
	}
}

/*************************************************************

 Waixing Board with Security Chip

 Games: Duo Bao Xiao Ying Hao - Guang Ming yu An Hei Chuan Shuo,
 Myth Struggle, San Shi Liu Ji, Shui Hu Zhuan

 MMC3 clone

 iNES: mapper 249

 In MESS: Partially Supported.

 *************************************************************/

static void waixing_sec_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (state->m_mmc_reg[0])
		bank = ((bank & 0x01)) | ((bank >> 3) & 0x02) | ((bank >> 1) & 0x04) | ((bank << 2) & 0x18);

	state->prg8_x(start, bank);
}

static void waixing_sec_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (state->m_mmc_reg[0])
		bank = ((bank & 0x03)) | ((bank >> 1) & 0x04) | ((bank >> 4) & 0x08) |
				((bank >> 2) & 0x10) | ((bank << 3) & 0x20) | ((bank << 2) & 0xc0);

	state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::waixing_sec_l_w)
{
	LOG_MMC(("waixing_sec_l_w, offset: %04x, data: %02x\n", offset, data));

	offset += 0x100;

	if (offset == 0x1000)
	{
		m_mmc_reg[0] = data & 0x02;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

/*************************************************************

 Waixing SH2 Board

 Games: Fire Emblem (C) and Fire Emblem Gaiden (C)

 MMC3 clone with different access to CHR

 iNES: mapper 165

 In MESS: Partially Supported.

 *************************************************************/

static void waixing_sh2_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->chr4_0(state->m_mmc_reg[0], state->m_mmc_reg[0] ? CHRRAM : CHRROM);
	state->chr4_4(state->m_mmc_reg[1], state->m_mmc_reg[1] ? CHRRAM : CHRROM);
}

READ8_MEMBER(nes_carts_state::waixing_sh2_chr_r)
{
	int bank = offset >> 10;
	UINT8 val = m_chr_map[bank].access[offset & 0x3ff]; // this would be usual return value
	int chr_helper;

	switch (offset & 0xff8)
	{
		case 0xfd0: chr_helper = (bank & 0x4) | 0x0; break;
		case 0xfe8: chr_helper = (bank & 0x4) | 0x2; break;
		default: return val;
	}

	m_mmc_reg[offset >> 12] = chr_helper;
	if (offset & 0x1000)
		chr4_4(m_mmc_reg[1], m_mmc_reg[1] ? CHRRAM : CHRROM);
	else
		chr4_0(m_mmc_reg[0], m_mmc_reg[0] ? CHRRAM : CHRROM);

	return val;
}

/*************************************************************

 Board UNL-8237

 Games: Pocahontas 2

 MMC3 clone

 In MESS: Supported

 *************************************************************/

static void unl_8237_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[0] & 0x80))
		state->prg8_x(start, bank);
}

static void unl_8237_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	bank |= ((state->m_mmc_reg[1] << 6) & 0x100);

	state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::unl_8237_l_w)
{
	LOG_MMC(("unl_8237_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_mmc_reg[0] = data;
		if (m_mmc_reg[0] & 0x80)
		{
			if (m_mmc_reg[0] & 0x20)
				prg32((m_mmc_reg[0] & 0x0f) >> 1);
			else
			{
				prg16_89ab(m_mmc_reg[0] & 0x1f);
				prg16_cdef(m_mmc_reg[0] & 0x1f);
			}
		}
		else
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}

	if (offset == 0x1001)
	{
		m_mmc_reg[1] = data;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

WRITE8_MEMBER(nes_carts_state::unl_8237_w)
{
	static const UINT8 conv_table[8] = {0, 2, 6, 1, 7, 3, 4, 5};
	LOG_MMC(("unl_8237_w offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
			set_nt_mirroring((data | (data >> 7)) & 0x01 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2000:
		case 0x3000:
			m_mmc_reg[2] = 1;
			data = (data & 0xc0) | conv_table[data & 0x07];
			txrom_w(space, 0x0000, data, mem_mask);
			break;

		case 0x4000:
		case 0x5000:
			if (m_mmc_reg[2])
			{
				m_mmc_reg[2] = 0;
				txrom_w(space, 0x0001, data, mem_mask);
			}
			break;

		case 0x6000:
			break;

		case 0x7000:
			txrom_w(space, 0x6001, data, mem_mask);
			txrom_w(space, 0x4000, data, mem_mask);
			txrom_w(space, 0x4001, data, mem_mask);
			break;
	}
}

/*************************************************************

 Board UNL-AX5705

 Games: Super Mario Bros. Pocker Mali (Crayon Shin-chan pirate hack)

 In MESS: Supported

 *************************************************************/

static void unl_ax5705_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->prg8_89(state->m_mmc_prg_bank[0]);
	state->prg8_ab(state->m_mmc_prg_bank[1]);
}

WRITE8_MEMBER(nes_carts_state::unl_ax5705_w)
{
	UINT8 bank;
	LOG_MMC(("unl_ax5705_w offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x700f)
	{
		case 0x0000:
			m_mmc_prg_bank[0] = (data & 0x05) | ((data & 0x08) >> 2) | ((data & 0x02) << 2);
			unl_ax5705_set_prg(machine());
			break;
		case 0x0008:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			m_mmc_prg_bank[1] = (data & 0x05) | ((data & 0x08) >> 2) | ((data & 0x02) << 2);
			unl_ax5705_set_prg(machine());
			break;
			/* CHR banks 0, 1, 4, 5 */
		case 0x2008:
		case 0x200a:
		case 0x4008:
		case 0x400a:
			bank = ((offset & 0x4000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
		case 0x2009:
		case 0x200b:
		case 0x4009:
		case 0x400b:
			bank = ((offset & 0x4000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x09) << 4);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
			/* CHR banks 2, 3, 6, 7 */
		case 0x4000:
		case 0x4002:
		case 0x6000:
		case 0x6002:
			bank = 2 + ((offset & 0x2000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
		case 0x4001:
		case 0x4003:
		case 0x6001:
		case 0x6003:
			bank = 2 + ((offset & 0x2000) ? 4 : 0) + ((offset & 0x0002) ? 1 : 0);
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | ((data & 0x04) << 3) | ((data & 0x02) << 5) | ((data & 0x09) << 4);
			chr1_x(bank, m_mmc_vrom_bank[bank], CHRROM);
			break;
	}
}

/*************************************************************

 Board UNL-CC-21

 Games: Mi Hun Che

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::unl_cc21_w)
{
	LOG_MMC(("unl_cc21_w offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 1) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8((offset & 0x01), CHRROM);
}

/*************************************************************

 Board UNL-KOF97

 Games: King of Fighters 97 (Rex Soft)

 MMC3 clone

 In MESS: Not working

 *************************************************************/

static UINT8 unl_kof97_unscramble( UINT8 data )
{
	return ((data >> 1) & 0x01) | ((data >> 4) & 0x02) | ((data << 2) & 0x04) | ((data >> 0) & 0xd8) | ((data << 3) & 0x20);
}

WRITE8_MEMBER(nes_carts_state::unl_kof97_w)
{
	LOG_MMC(("unl_kof97_w offset: %04x, data: %02x\n", offset, data));

	/* Addresses 0x9000, 0xa000, 0xd000 & 0xf000 behaves differently than MMC3 */
	if (offset == 0x1000)
	{
		data = unl_kof97_unscramble(data);
		txrom_w(space, 0x0001, data, mem_mask);
	}
	else if (offset == 0x2000)
	{
		data = unl_kof97_unscramble(data);
		txrom_w(space, 0x0000, data, mem_mask);
	}
	else if (offset == 0x5000)
	{
		data = unl_kof97_unscramble(data);
		txrom_w(space, 0x4001, data, mem_mask);
	}
	else if (offset == 0x7000)
	{
		data = unl_kof97_unscramble(data);
		txrom_w(space, 0x6001, data, mem_mask);
	}
	else        /* Other addresses behaves like MMC3, up to unscrambling data */
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
			case 0x0001:
			case 0x4000:
			case 0x4001:
			case 0x6000:
			case 0x6001:
			case 0x2000:    /* are these ever called?!? */
			case 0x2001:
				data = unl_kof97_unscramble(data);
				txrom_w(space, offset, data, mem_mask);
				break;
		}
	}
}

/*************************************************************

 Board UNL-KS7057

 Games: Street Fighter VI / Fight Street VI

 MMC3 clone (identical, but for switched address lines)

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::ks7057_w)
{
	LOG_MMC(("ks7057_w, offset: %04x, data: %02x\n", offset, data));
	offset = (BIT(offset, 0) << 1) | BIT(offset, 1) | (offset & ~0x03);
	txrom_w(space, offset, data, mem_mask);
}

/*************************************************************

 Board UNL-T-230

 Games: Dragon Ball Z IV (Unl)

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::unl_t230_w)
{
	UINT8 bank;
	LOG_MMC(("unl_t230_w offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x700c)
	{
		case 0x0000:
			break;
		case 0x2000:
			prg16_89ab(data);
			break;

		// the part below works like VRC-2. how was the original board wired up?
		// was there a VRC2? if so, we can use VRC-2 and add proper pin settings to xml!
		case 0x1000:
		case 0x1004:
		case 0x1008:
		case 0x100c:
			switch (data & 0x03)
			{
			case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
			}
			break;

		case 0x3000:
		case 0x3004:
		case 0x3008:
		case 0x300c:
		case 0x4000:
		case 0x4004:
		case 0x4008:
		case 0x400c:
		case 0x5000:
		case 0x5004:
		case 0x5008:
		case 0x500c:
		case 0x6000:
		case 0x6004:
		case 0x6008:
		case 0x600c:
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + ((offset & 0x0008) >> 2);
			if (offset & 0x0004)
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0x0f) | (data << 4);
			else
				m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & 0xf0) | (data & 0x0f);

			chr1_x(bank, m_mmc_vrom_bank[bank], m_mmc_chr_source);
			break;
		case 0x7000:
			m_IRQ_count_latch &= ~0x0f;
			m_IRQ_count_latch |= data & 0x0f;
			break;
		case 0x7004:
			m_IRQ_count_latch &= ~0xf0;
			m_IRQ_count_latch |= (data << 4) & 0xf0;
			break;
		case 0x7008:
			m_IRQ_mode = data & 0x04;   // currently not implemented: 0 = prescaler mode / 1 = CPU mode
			m_IRQ_enable = data & 0x02;
			m_IRQ_enable_latch = data & 0x01;
			if (data & 0x02)
				m_IRQ_count = m_IRQ_count_latch;
			break;

		default:
			logerror("unl_t230_w uncaught offset: %04x value: %02x\n", offset, data);
			break;
	}
}

/*************************************************************

 Bootleg Board for KOF96

 Games: The King of Fighters 96, Sonic 3D Blast 6, Street
 Fighter Zero 2

 MMC3 clone

 iNES: mapper 187

 In MESS: Preliminary Support.

 *************************************************************/

static void kof96_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[0] & 0x80))
		state->prg8_x(start, bank);
}

static void kof96_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc_latch1 & 0x80) >> 5;

	if ((start & 0x04) == chr_page)
		bank |= 0x100;

	state->chr1_x(start, bank, source);
}

WRITE8_MEMBER(nes_carts_state::kof96_l_w)
{
	UINT8 new_bank;
	LOG_MMC(("kof96_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_mmc_reg[0] = data;

		if (m_mmc_reg[0] & 0x80)
		{
			new_bank = (m_mmc_reg[0] & 0x1f);

			if (m_mmc_reg[0] & 0x20)
				prg32(new_bank >> 2);
			else
			{
				prg16_89ab(new_bank);
				prg16_cdef(new_bank);
			}
		}
		else
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}

	if (offset >= 0x1000)
	{
		switch (data & 0x03)
		{
			case 0x00:
			case 0x01:
				m_mmc_reg[1] = 0x83;
				break;
			case 0x02:
				m_mmc_reg[1] = 0x42;
				break;
			case 0x03:
				m_mmc_reg[1] = 0x00;
				break;
		}

	}

	if (!m_mmc_reg[3] && offset > 0x1000)
	{
		m_mmc_reg[3] = 1;
		space.write_byte(0x4017, 0x40);
	}
}

READ8_MEMBER(nes_carts_state::kof96_l_r)
{
	LOG_MMC(("kof96_l_r, offset: %04x\n", offset));
	offset += 0x100;

	if (!(offset < 0x1000))
		return m_mmc_reg[1];
	else
		return 0;
}

WRITE8_MEMBER(nes_carts_state::kof96_w)
{
	LOG_MMC(("kof96_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6003)
	{
		case 0x0000:
			m_mmc_reg[2] = 1;
			txrom_w(space, 0x0000, data, mem_mask);
			break;

		case 0x0001:
			if (m_mmc_reg[2])
				txrom_w(space, 0x0001, data, mem_mask);
			break;

		case 0x0002:
			break;

		case 0x0003:
			m_mmc_reg[2] = 0;

			if (data == 0x28)
				prg8_cd(0x17);
			else if (data == 0x2a)
				prg8_ab(0x0f);
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board for MK2

 Games: Mortal Kombat II, Street Fighter III, Super Mario
 Kart Rider

 This board uses an IRQ system very similar to MMC3. We indeed
 use mapper4_irq, but there is some small glitch!

 iNES: mapper 91

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::mk2_m_w)
{
	LOG_MMC(("mk2_m_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x1000)
	{
		case 0x0000:
			switch (offset & 0x03)
			{
			case 0x00: chr2_0(data, CHRROM); break;
			case 0x01: chr2_2(data, CHRROM); break;
			case 0x02: chr2_4(data, CHRROM); break;
			case 0x03: chr2_6(data, CHRROM); break;
			}
			break;
		case 0x1000:
			switch (offset & 0x03)
			{
			case 0x00: prg8_89(data); break;
			case 0x01: prg8_ab(data); break;
			case 0x02: m_IRQ_enable = 0; m_IRQ_count = 0; break;
			case 0x03: m_IRQ_enable = 1; m_IRQ_count = 7; break;
			}
			break;
		default:
			logerror("mk2_m_w uncaught addr: %04x value: %02x\n", offset + 0x6000, data);
			break;
	}
}

/*************************************************************

 Bootleg Board N625092

 Games: 400 in 1, 700 in 1, 1000 in 1

 iNES: mapper 221

 In MESS: Supported.

 *************************************************************/

static void n625092_set_prg( running_machine &machine, UINT8 reg1, UINT8 reg2 )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 map221_helper1, map221_helper2;

	map221_helper1 = !(reg1 & 0x01) ? reg2 :
	(reg1 & 0x80) ? reg2 : (reg2 & 0x06) | 0x00;
	map221_helper2 = !(reg1 & 0x01) ? reg2 :
	(reg1 & 0x80) ? 0x07 : (reg2 & 0x06) | 0x01;

	state->prg16_89ab(map221_helper1 | ((reg1 & 0x70) >> 1));
	state->prg16_cdef(map221_helper2 | ((reg1 & 0x70) >> 1));
}

WRITE8_MEMBER(nes_carts_state::n625092_w)
{
	LOG_MMC(("n625092_w, offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x4000)
	{
		set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		offset = (offset >> 1) & 0xff;

		if (m_mmc_latch1 != offset)
		{
			m_mmc_latch1 = offset;
			n625092_set_prg(machine(), m_mmc_latch1, m_mmc_latch2);
		}
	}
	else
	{
		offset &= 0x07;

		if (m_mmc_latch2 != offset)
		{
			m_mmc_latch2 = offset;
			n625092_set_prg(machine(), m_mmc_latch1, m_mmc_latch2);
		}
	}
}

/*************************************************************

 SC-127 Board

 Games: Wario World II (Kirby Hack)

 iNES: mapper 35

 In MESS: Supported

 *************************************************************/

static void sc127_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE && state->m_IRQ_enable)
	{
		state->m_IRQ_count--;

		if (!blanked && (state->m_IRQ_count == 0))
		{
			LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
						device->machine().primary_screen->vpos(), device->machine().primary_screen->hpos()));
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_enable = 0;
		}
	}
}

WRITE8_MEMBER(nes_carts_state::sc127_w)
{
	LOG_MMC(("sc127_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x0001:
			prg8_ab(data);
			break;
		case 0x0002:
			//      m_mmc_prg_bank[offset & 0x02] = data;
			prg8_cd(data);
			break;
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
		case 0x1004:
		case 0x1005:
		case 0x1006:
		case 0x1007:
			//      m_mmc_vrom_bank[offset & 0x07] = data;
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x4002:
			m_IRQ_enable = 0;
			break;
		case 0x4003:
			m_IRQ_enable = 1;
			break;
		case 0x4005:
			m_IRQ_count = data;
			break;
		case 0x5001:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

/*************************************************************

 Bootleg Board SMB2J

 Games: Super Mario Bros. 2 Pirate (LF36)

 iNES: mapper 43

 In MESS: Supported? The only image I found is not working
 (not even in NEStopia).

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::smb2j_w)
{
	int bank = (((offset >> 8) & 0x03) * 0x20) + (offset & 0x1f);

	LOG_MMC(("smb2j_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring((offset & 0x2000) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (offset & 0x0800)
	{
		if (offset & 0x1000)
		{
			if (bank * 2 >= m_prg_chunks)
			{
				membank("bank3")->set_base(m_wram);
				membank("bank4")->set_base(m_wram);
			}
			else
			{
				LOG_MMC(("smb2j_w, selecting upper 16KB bank of #%02x\n", bank));
				prg16_cdef(2 * bank + 1);
			}
		}
		else
		{
			if (bank * 2 >= m_prg_chunks)
			{
				membank("bank1")->set_base(m_wram);
				membank("bank2")->set_base(m_wram);
			}
			else
			{
				LOG_MMC(("smb2j_w, selecting lower 16KB bank of #%02x\n", bank));
				prg16_89ab(2 * bank);
			}
		}
	}
	else
	{
		if (bank * 2 >= m_prg_chunks)
		{
			membank("bank1")->set_base(m_wram);
			membank("bank2")->set_base(m_wram);
			membank("bank3")->set_base(m_wram);
			membank("bank4")->set_base(m_wram);
		}
		else
		{
			LOG_MMC(("smb2j_w, selecting 32KB bank #%02x\n", bank));
			prg32(bank);
		}
	}
}

/*************************************************************

 BTL-SMB2B

 Games: Super Mario Bros. 2 Pirate (Jpn version of SMB2)

 This was marked as Alt. Levels. is it true?

 iNES: mapper 50

 In MESS: Supported.

 *************************************************************/

static void smb2jb_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if (state->m_IRQ_enable)
	{
		if (state->m_IRQ_count < 0x1000)
		{
			if ((0x1000 - state->m_IRQ_count) <= 114)
				state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			else
				state->m_IRQ_count += 114;
		}
		else
			state->m_IRQ_count += 114;

		state->m_IRQ_count &= 0xffff;   // according to docs is 16bit counter -> it wraps only after 0xffff
	}
}

WRITE8_MEMBER(nes_carts_state::smb2jb_l_w)
{
	UINT8 prg;
	LOG_MMC(("smb2jb_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset & 0x160)
	{
		case 0x020:
			prg = (data & 0x08) | ((data & 0x06) >> 1) | ((data & 0x01) << 2);
			prg8_cd(prg);
			break;
		case 0x120:
			m_IRQ_enable = data & 0x01;
			break;
	}
}

/* This goes to 0x4020-0x403f */
WRITE8_MEMBER(nes_carts_state::smb2jb_extra_w)
{
	UINT8 prg;
	LOG_MMC(("smb2jb_extra_w, offset: %04x, data: %02x\n", offset, data));

	prg = (data & 0x08) | ((data & 0x06) >> 1) | ((data & 0x01) << 2);
	prg8_cd(prg);
}

/*************************************************************

 Bootleg Board for Super Fighter III

 MMC3 clone

 iNES: mapper 197

 In MESS: Supported.

 *************************************************************/

static void unl_sf3_set_chr( running_machine &machine, UINT8 chr_source, int chr_base, int chr_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->chr4_0(chr_base | ((state->m_mmc_vrom_bank[0] >> 1) & chr_mask), chr_source);
	state->chr2_4(chr_base | (state->m_mmc_vrom_bank[1] & chr_mask), chr_source);
	state->chr2_6(chr_base | (state->m_mmc_vrom_bank[2] & chr_mask), chr_source);
}

WRITE8_MEMBER(nes_carts_state::unl_sf3_w)
{
	UINT8 mmc_helper, cmd;
	LOG_MMC(("unl_sf3_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_mmc3_latch ^ data;
			m_mmc3_latch = data;

			/* Has PRG Mode changed? */
			if (mmc_helper & 0x40)
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			/* Has CHR Mode changed? */
			if (mmc_helper & 0x80)
				unl_sf3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

		case 0x0001:
			cmd = m_mmc3_latch & 0x0f;
			switch (cmd)
			{
			case 0: case 2: case 4:
				m_mmc_vrom_bank[cmd >> 1] = data;
				unl_sf3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
				break;
			case 6:
			case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;
			}
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 Bootleg Board for Xiao Zhuan Yuan

 Games: Shu Qi Yu - Zhi Li Xiao Zhuan Yuan

 Very simple mapper: writes to 0x5ff1 set prg32 (to data>>1),
 while writes to 0x5ff2 set chr8

 iNES: mapper 176

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::unl_xzy_l_w)
{
	LOG_MMC(("unl_xzy_l_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset)
	{
		case 0x1ef1:    /* 0x5ff1 */
			prg32(data >> 1);
			break;
		case 0x1ef2:    /* 0x5ff2 */
			chr8(data, CHRROM);
			break;
	}
}

/*************************************************************

 Board UNL-RACERMATE

 In MESS: *VERY* preliminary support. Also, it seems that this
 board saves to battery the CHRRAM!!!

 *************************************************************/

static void racmate_update_banks( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->chr4_4(state->m_mmc_latch1 & 0x0f, state->m_mmc_chr_source);
	state->prg16_89ab(state->m_mmc_latch1 >> 1);
}

WRITE8_MEMBER(nes_carts_state::unl_racmate_w)
{
	LOG_MMC(("unl_racmate_w offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x3000)
	{
		m_mmc_latch1 = data;
		racmate_update_banks(machine());
	}
}


/*************************************************************

 Board UNL-FS304

 Games: A Link to the Past by Waixing

 iNES: mapper 162? (only found in UNIF format)

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::unl_fs304_l_w)
{
	LOG_MMC(("unl_fs304_l_w, offset: %04x, data: %02x\n", offset, data));
	int bank;
	offset += 0x100;

	if (offset >= 0x1000)
	{
		m_mmc_reg[(offset >> 8) & 3] = data;
		bank = ((m_mmc_reg[2] & 0x0f) << 4) | BIT(m_mmc_reg[1], 1) | (m_mmc_reg[0] & 0x0e);
		prg32(bank);
		chr8(0, CHRRAM);
	}
}


/*************************************************************

        BOOTLEG CART VERSIONS OF FDS GAMES

 *************************************************************/

/*************************************************************

 BTL-SUPERBROS11

 Games: Super Mario Bros. 11, Super Mario Bros. 17

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 196

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::btl_smb11_w)
{
	LOG_MMC(("btl_smb11_w, offset: %04x, data: %02x\n", offset, data));

	txrom_w(space, (offset & 0x6000) | ((offset & 0x04) >> 2), data, mem_mask);
}

/*************************************************************

 BTL-MARIOBABY

 Games: Mario Baby, Ai Senshi Nicol

 iNES: mapper 42

 In MESS: Supported.

 *************************************************************/

// is the code fine for ai senshi nicol?!?
WRITE8_MEMBER(nes_carts_state::btl_mariobaby_w)
{
	LOG_MMC(("btl_mariobaby_w, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x7000)
	{
		switch (offset & 0x03)
		{
			case 0x00:
				prg8_67(data);
				break;
			case 0x01:
				set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;
			case 0x02:
				/* Check if IRQ is being enabled */
				if (!m_IRQ_enable && (data & 0x02))
				{
					m_IRQ_enable = 1;
					m_irq_timer->adjust(downcast<cpu_device *>(m_maincpu)->cycles_to_attotime(24576));
				}
				if (!(data & 0x02))
				{
					m_IRQ_enable = 0;
					m_irq_timer->adjust(attotime::never);
				}
				break;
		}
	}
}

/*************************************************************

 BTL-SMB2A

 Games: Super Mario Bros. 2 Pirate (Jpn version of SMB2)

 iNES: mapper 40

 In MESS: Supported.

 *************************************************************/

static void btl_smb2a_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable)
	{
		if ((0xfff - state->m_IRQ_count) <= 114)
		{
			state->m_IRQ_count = (state->m_IRQ_count + 1) & 0xfff;
			state->m_IRQ_enable = 0;
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			state->m_IRQ_count += 114;
	}
}

WRITE8_MEMBER(nes_carts_state::btl_smb2a_w)
{
	LOG_MMC(("btl_smb2a_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x0000:
			m_IRQ_enable = 0;
			m_IRQ_count = 0;
			break;
		case 0x2000:
			m_IRQ_enable = 1;
			break;
		case 0x6000:
			prg8_cd(data);
			break;
	}
}

/*************************************************************

 WHIRLWIND-2706

 Games: Meikyuu Jiin Dababa (FDS conversion)

 iNES: mapper 108

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::whirl2706_w)
{
	LOG_MMC(("whirl2706_w, offset: %04x, data: %02x\n", offset, data));
	prg8_67(data);
}

/*************************************************************

 Bootleg Board used for FDS conversion

 Games: Tobidase Daisakusen (FDS conversion)

 iNES: mapper 120

 In MESS: Partially Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::btl_tobi_l_w)
{
	LOG_MMC(("btl_tobi_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if ((offset & 0x43c0) == 0x41c0)
		prg8_67(data & 0x07);
}

/*************************************************************

 BTL-SMB3

 Games: Super Mario Bros. 3 Pirate

 iNES: mapper 106

 In MESS: Supported.

 *************************************************************/

static void btl_smb3_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();

	if (state->m_IRQ_enable)
	{
		if ((0xffff - state->m_IRQ_count) < 114)
		{
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			state->m_IRQ_enable = 0;
		}

		state->m_IRQ_count = (state->m_IRQ_count + 114) & 0xffff;
	}
}

WRITE8_MEMBER(nes_carts_state::btl_smb3_w)
{
	LOG_MMC(("btl_smb3_w, offset: %04x, data: %02x\n", offset, data));
	switch (offset & 0x0f)
	{
		case 0x00:
		case 0x02:
			chr1_x(offset & 0x07, data & 0xfe, CHRROM);
			break;
		case 0x01:
		case 0x03:
			chr1_x(offset & 0x07, data | 0x01, CHRROM);
			break;
		case 0x04: case 0x05:
		case 0x06: case 0x07:
			chr1_x(offset & 0x07, data, CHRROM);
			break;
		case 0x08:
			prg8_89(data | 0x10);
			break;
		case 0x09:
			prg8_ab(data);
			break;
		case 0x0a:
			prg8_cd(data);
			break;
		case 0x0b:
			prg8_ef(data | 0x10);
			break;
		case 0x0c:
			set_nt_mirroring(BIT(data, 0) ?  PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x0d:
			m_IRQ_count = 0;
			m_IRQ_enable = 0;
			break;
		case 0x0e:
			m_IRQ_count = (m_IRQ_count & 0xff00) | data;
			break;
		case 0x0f:
			m_IRQ_count = (m_IRQ_count & 0x00ff) | (data << 8);
			m_IRQ_enable = 1;
			break;
	}
}

/*************************************************************

 BTL-DRAGONNINJA

 Games: Dragon Ninja (Bootleg), Super Mario Bros. 8

 iNES: mapper 222

 In MESS: Unsupported.

 *************************************************************/

/* Scanline based IRQ ? */
static void btl_dn_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		if (!state->m_IRQ_count || ++state->m_IRQ_count < 240)
			return;

		state->m_IRQ_count = 0;
		LOG_MMC(("irq fired, scanline: %d (MAME %d, beam pos: %d)\n", scanline,
					device->machine().primary_screen->vpos(), device->machine().primary_screen->hpos()));
		state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
	}
}

WRITE8_MEMBER(nes_carts_state::btl_dn_w)
{
	UINT8 bank;
	LOG_MMC(("btl_dn_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x7003)
	{
		case 0x0000:
			prg8_89(data);
			break;
		case 0x1000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2000:
			prg8_ab(data);
			break;
		case 0x3000:
		case 0x3002:
		case 0x4000:
		case 0x4002:
		case 0x5000:
		case 0x5002:
		case 0x6000:
		case 0x6002:
			bank = ((offset & 0x7000) - 0x3000) / 0x0800 + ((offset & 0x0002) >> 3);
			chr1_x(bank, data, CHRROM);
			break;
		case 0x7000:
			m_IRQ_count = data;
			break;
	}
}

/*************************************************************

 BTL-PIKACHUY2K

 Games: Pikachu Y2k

 iNES: mapper 254

 In MESS:

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::btl_pika_y2k_w)
{
	LOG_MMC(("btl_pika_y2k_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2001:
			m_mmc_latch2 = data;
			break;

		case 0x2000:
			m_mmc_reg[0] = 0;
		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

// strange WRAM usage: it is protected at start, and gets unprotected after the first write to 0xa000
WRITE8_MEMBER(nes_carts_state::btl_pika_y2k_m_w)
{
	LOG_MMC(("btl_pika_y2k_m_w, offset: %04x, data: %02x\n", offset, data));

	m_wram[offset] = data;
}

READ8_MEMBER(nes_carts_state::btl_pika_y2k_m_r)
{
	LOG_MMC(("btl_pika_y2k_m_r, offset: %04x\n", offset));

	return  m_wram[offset] ^ (m_mmc_latch2 & m_mmc_reg[0]);
}

/*************************************************************

          MULTICART

 *************************************************************/

/*************************************************************

 Board BMC-FK23C

 In MESS: partially supported (still to sort initial banking
 for many games)

 *************************************************************/

#if 0
static void fk23c_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (((state->m_mmc_reg[0] & 0x07) - 3) > 1 && (!(state->m_mmc_reg[3] & 0x02) || start < 2))
	{
		if (state->m_mmc_reg[0] & 0x03)
			bank = (bank & (0x3f >> (state->m_mmc_reg[0] & 0x03))) | (state->m_mmc_reg[1] << 1);

		state->prg8_x(start, bank);
	}
}

static void fk23c_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[0] & 0x40) && (!(state->m_mmc_reg[3] & 0x02) || (start != 1 && start != 3)))
		state->chr1_x(start, ((state->m_mmc_reg[2] & 0x7f) << 3) | bank, source);
}

#endif

static void fk23c_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 mask = (0x3f >> (state->m_mmc_reg[0] & 0x03));

	if ((state->m_mmc_reg[0] & 0x07) < 3)
	{
		if (!(state->m_mmc_reg[0] & 0x03))
			bank = (bank & mask) | ((state->m_mmc_reg[1]  & (0x7f ^ mask)) << 1);

		state->prg8_x(start, bank);
	}
}

static void fk23c_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[0] & 0x40) && (!(state->m_mmc_reg[3] & 0x02) || (start != 1 && start != 3)))
		state->chr1_x(start, ((state->m_mmc_reg[2] & 0x7f) << 3) | bank, source);
}

static void fk23c_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	if ((state->m_mmc_reg[0] & 0x07) == 4)
		state->prg32((state->m_mmc_reg[1] & 0x7f) >> 1);
	else if ((state->m_mmc_reg[0] & 0x07) == 3)
	{
		state->prg16_89ab(state->m_mmc_reg[1] & 0x7f);
		state->prg16_cdef(state->m_mmc_reg[1] & 0x7f);
	}
	else
	{
		if (state->m_mmc_reg[3] & 0x02)
		{
			state->prg8_cd(state->m_mmc_reg[4]);
			state->prg8_ef(state->m_mmc_reg[5]);
		}
		else
			mmc3_set_prg(machine, state->m_mmc_prg_base, state->m_mmc_prg_mask);
	}
}

static void fk23c_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (state->m_mmc_reg[0] & 0x40)
		state->chr8(state->m_mmc_reg[2] | state->m_mmc_cmd1, state->m_mmc_chr_source);
	else
	{
		if (state->m_mmc_reg[3] & 0x02)
		{
			int base = (state->m_mmc_reg[2] & 0x7f) << 3;
			state->chr1_x(1, base | state->m_mmc_reg[6], state->m_mmc_chr_source);
			state->chr1_x(3, base | state->m_mmc_reg[7], state->m_mmc_chr_source);
		}
		else
			mmc3_set_chr(machine, state->m_mmc_chr_source, state->m_mmc_chr_base, state->m_mmc_chr_mask);
	}
}

WRITE8_MEMBER(nes_carts_state::fk23c_l_w)
{
	LOG_MMC(("fk23c_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		if (offset & (1 << 4))  // here it should be (4 + m_mmc_dipsetting)
		{
			m_mmc_reg[offset & 0x03] = data;

			fk23c_set_prg(machine());
			fk23c_set_chr(machine());
		}
	}
}

WRITE8_MEMBER(nes_carts_state::fk23c_w)
{
	LOG_MMC(("fk23c_w, offset: %04x, data: %02x\n", offset, data));

	if (m_mmc_reg[0] & 0x40)
	{
		if (m_mmc_reg[0] & 0x30)
			m_mmc_cmd1 = 0;
		else
		{
			m_mmc_cmd1 = data & 0x03;
			fk23c_set_chr(machine());
		}
	}
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0001:
				if ((m_mmc_reg[3] & 0x02) && (m_mmc3_latch & 0x08))
				{
					m_mmc_reg[4 | (m_mmc3_latch & 0x03)] = data;
					fk23c_set_prg(machine());
					fk23c_set_chr(machine());
				}
				else
					txrom_w(space, offset, data, mem_mask);
				break;

			case 0x2000:
				set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_w(space, offset, data, mem_mask);
				break;
		}
	}
}


/*************************************************************

 Board BMC-64IN1NOREPEAT

 Games: 64-in-1 Y2K

 In MESS: Supported

 *************************************************************/

static void bmc_64in1nr_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 helper1 = (state->m_mmc_reg[1] & 0x1f);
	UINT8 helper2 = (helper1 << 1) | ((state->m_mmc_reg[1] & 0x40) >> 6);

	if (state->m_mmc_reg[0] & 0x80)
	{
		if (state->m_mmc_reg[1] & 0x80)
			state->prg32(helper1);
		else
		{
			state->prg16_89ab(helper2);
			state->prg16_cdef(helper2);
		}
	}
	else
		state->prg16_cdef(helper2);
}

WRITE8_MEMBER(nes_carts_state::bmc_64in1nr_l_w)
{
	LOG_MMC(("bmc_64in1nr_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	switch (offset)
	{
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
			m_mmc_reg[offset & 0x03] = data;
			bmc_64in1nr_set_prg(machine());
			chr8(((m_mmc_reg[0] >> 1) & 0x03) | (m_mmc_reg[2] << 2), CHRROM);
			break;
	}
	if (offset == 0x1000)   /* reg[0] also sets mirroring */
		set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

WRITE8_MEMBER(nes_carts_state::bmc_64in1nr_w)
{
	LOG_MMC(("bmc_64in1nr_w offset: %04x, data: %02x\n", offset, data));

	m_mmc_reg[3] = data;    // reg[3] is currently unused?!?
}

/*************************************************************

 Board BMC-190IN1

 Games: 190-in-1

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_190in1_w)
{
	LOG_MMC(("bmc_190in1_w offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	offset >>= 2;
	prg16_89ab(offset);
	prg16_cdef(offset);
	chr8(offset, CHRROM);
}

/*************************************************************

 Board BMC-A65AS

 Games: 3-in-1 (N068)

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_a65as_w)
{
	UINT8 helper = (data & 0x30) >> 1;
	LOG_MMC(("bmc_a65as_w offset: %04x, data: %02x\n", offset, data));

	if (data & 0x80)
		set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (data & 0x40)
		prg32(data >> 1);
	else
	{
		prg16_89ab(helper | (data & 0x07));
		prg16_cdef(helper | 0x07);
	}
}

/*************************************************************

 Board BMC-GS2004

 Games: Tetris Family 6-in-1

 In MESS: Preliminary Support. It also misses WRAM handling
 (we need reads from 0x6000-0x7fff)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_gs2004_w)
{
	LOG_MMC(("bmc_gs2004_w offset: %04x, data: %02x\n", offset, data));

	prg32(data);
}

/*************************************************************

 Board BMC-GS2013

 Games: Tetris Family 12-in-1

 In MESS: Preliminary Support. It also misses WRAM handling
 (we need reads from 0x6000-0x7fff)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_gs2013_w)
{
	LOG_MMC(("bmc_gs2013_w offset: %04x, data: %02x\n", offset, data));

	if (data & 0x08)
		prg32(data & 0x09);
	else
		prg32(data & 0x07);
}

/*************************************************************

 Board BMC-SUPER24IN1SC03

 Games: Super 24-in-1

 In MESS: Partially Supported

 *************************************************************/

static void bmc_s24in1sc03_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	static const UINT8 masks[8] = {0x3f, 0x1f, 0x0f, 0x01, 0x03, 0x00, 0x00, 0x00};
	int prg_base = state->m_mmc_reg[1] << 1;
	int prg_mask = masks[state->m_mmc_reg[0] & 0x07];

	bank = prg_base | (bank & prg_mask);
	state->prg8_x(start, bank);
}

static void bmc_s24in1sc03_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr = BIT(state->m_mmc_reg[0], 5) ? CHRRAM : CHRROM;
	int chr_base = (state->m_mmc_reg[2] << 3) & 0xf00;

	state->chr1_x(start, chr_base | bank, chr);
}

WRITE8_MEMBER(nes_carts_state::bmc_s24in1sc03_l_w)
{
	LOG_MMC(("bmc_s24in1sc03_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1ff0)
	{
		m_mmc_reg[0] = data;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}

	if (offset == 0x1ff1)
	{
		m_mmc_reg[1] = data;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}

	if (offset == 0x1ff2)
	{
		m_mmc_reg[2] = data;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

/*************************************************************

 Board BMC-T-262

 Games: 4-in-1 (D-010), 8-in-1 (A-020)

 In MESS: Supported

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_t262_w)
{
	UINT8 mmc_helper;
	LOG_MMC(("bmc_t262_w offset: %04x, data: %02x\n", offset, data));

	if (m_mmc_latch2 || offset == 0)
	{
		m_mmc_latch1 = (m_mmc_latch1 & 0x38) | (data & 0x07);
		prg16_89ab(m_mmc_latch1);
	}
	else
	{
		m_mmc_latch2 = 1;
		set_nt_mirroring(BIT(data, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
		mmc_helper = ((offset >> 3) & 0x20) | ((offset >> 2) & 0x18);
		m_mmc_latch1 = mmc_helper | (m_mmc_latch1 & 0x07);
		prg16_89ab(m_mmc_latch1);
		prg16_cdef(mmc_helper | 0x07);
	}
}

/*************************************************************

 Board BMC-WS

 Games: Super 40-in-1

 In MESS: Partially Supported (some games, like Galaxian, have
 issues)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_ws_m_w)
{
	UINT8 mmc_helper;
	LOG_MMC(("bmc_ws_m_w offset: %04x, data: %02x\n", offset, data));

	if (offset < 0x1000)
	{
		switch (offset & 0x01)
		{
			case 0:
				if (!m_mmc_latch1)
				{
					m_mmc_latch1 = data & 0x20;
					set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
					mmc_helper = (~data & 0x08) >> 3;
					prg16_89ab(data & ~mmc_helper);
					prg16_cdef(data |  mmc_helper);
				}
				break;
			case 1:
				if (!m_mmc_latch1)
				{
					chr8(data, CHRROM);
				}
				break;
		}
	}
}

/*************************************************************

 BMC-NOVELDIAMOND and BMC-999999in1

 Unknown Bootleg Multigame Board
 Games: I only found 'Novel Diamond 999999-in-1.unf' using
 this mapper (hence the code is used for BMC_NOVELDIAMOND
 board). The code is included here in case a mapper 54
 dump arises.

 iNES: mappers 54 and 213

 In MESS: Partial Support.

 *************************************************************/

// Are this correct or should they work the same?
WRITE8_MEMBER(nes_carts_state::novel1_w)
{
	LOG_MMC(("novel1_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset & 0x03);
	chr8(offset & 0x07, CHRROM);
}

WRITE8_MEMBER(nes_carts_state::novel2_w)
{
	LOG_MMC(("novel2_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset >> 1);
	chr8(offset >> 3, CHRROM);
}

/*************************************************************

 Board BMC-GKA

 Unknown Bootleg Multigame Board
 Games: 6 in 1, 54 in 1, 106 in 1

 iNES: mapper 57

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_gka_w)
{
	LOG_MMC(("bmc_gka_w, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x0800)
		m_mmc_latch2 = data;
	else
		m_mmc_latch1 = data;

	if (m_mmc_latch2 & 0x80)
		prg32(2 | (m_mmc_latch2 >> 6));
	else
	{
		prg16_89ab((m_mmc_latch2 >> 5) & 0x03);
		prg16_cdef((m_mmc_latch2 >> 5) & 0x03);
	}

	set_nt_mirroring((m_mmc_latch2 & 0x08) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8((m_mmc_latch1 & 0x03) | (m_mmc_latch2 & 0x07) | ((m_mmc_latch2 & 0x10) >> 1), CHRROM);
}


/*************************************************************

 Board UNL-STUDYNGAME

 Games: Study n Game 32 in 1

 iNES: mapper 39

 In MESS: Partially Supported (problems with PRG bankswitch,
 only keyboard exercise work).

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::sng32_w)
{
	LOG_MMC(("sng32_w, offset: %04x, data: %02x\n", offset, data));
	prg32(data);
}

/*************************************************************

 Board BMC-GKB

 Unknown Bootleg Multigame Board
 Games: 68 in 1, 73 in 1, 98 in 1

 iNES: mapper 58

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_gkb_w)
{
	UINT8 bank = (offset & 0x40) ? 0 : 1;
	LOG_MMC(("bmc_gkb_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & ~bank);
	prg16_cdef(offset | bank);
	chr8(offset >> 3, m_mmc_chr_source);
	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-SUPER-700IN1

 Unknown Bootleg Multigame Board
 Games: Super 700 in 1

 iNES: mapper 62

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_super700in1_w)
{
	LOG_MMC(("bmc_super700in1_w, offset :%04x, data: %02x\n", offset, data));

	chr8(((offset & 0x1f) << 2) | (data & 0x03), CHRROM);

	if (offset & 0x20)
	{
		prg16_89ab((offset & 0x40) | ((offset >> 8) & 0x3f));
		prg16_cdef((offset & 0x40) | ((offset >> 8) & 0x3f));
	}
	else
	{
		prg32(((offset & 0x40) | ((offset >> 8) & 0x3f)) >> 1);
	}

	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-36IN1

 Unknown Bootleg Multigame Board
 Games: 36 in 1, 1200 in 1

 iNES: mapper 200

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_36in1_w)
{
	LOG_MMC(("bmc_36in1_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & 0x07);
	prg16_cdef(offset & 0x07);
	chr8(offset & 0x07, CHRROM);

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-21IN1

 Unknown Bootleg Multigame Board
 Games: 8 in 1, 21 in 1

 iNES: mapper 201

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_21in1_w)
{
	LOG_MMC(("bmc_21in1_w, offset: %04x, data: %02x\n", offset, data));

	prg32(offset & 0x03);
	chr8(offset & 0x03, CHRROM);
}

/*************************************************************

 BMC-150IN1

 Unknown Bootleg Multigame Board
 Games: 150 in 1

 iNES: mapper 202

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_150in1_w)
{
	int bank = (offset >> 1) & 0x07;

	LOG_MMC(("bmc_150in1_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(bank);
	prg16_cdef(bank + (((bank & 0x06) == 0x06) ? 1 : 0));
	chr8(bank, CHRROM);

	set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-35IN1

 Unknown Bootleg Multigame Board
 Games: 35 in 1

 iNES: mapper 203

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_35in1_w)
{
	LOG_MMC(("bmc_35in1_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab((data >> 2) & 0x03);
	prg16_cdef((data >> 2) & 0x03);
	chr8(data & 0x03, CHRROM);
}

/*************************************************************

 BMC-64IN1

 Unknown Bootleg Multigame Board
 Games: 64 in 1

 iNES: mapper 204

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_64in1_w)
{
	int bank = (offset >> 1) & (offset >> 2) & 0x01;

	LOG_MMC(("bmc_64in1_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset & ~bank);
	prg16_cdef(offset | bank);
	chr8(offset & ~bank, CHRROM);

	set_nt_mirroring(BIT(data, 4) ? PPU_MIRROR_HORZ: PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-15IN1

 Unknown Bootleg Multigame Board
 Games: 3 in 1, 15 in 1

 iNES: mapper 205, MMC3 clone

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_15in1_m_w)
{
	LOG_MMC(("bmc_15in1_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x0800)
	{
		m_mmc_prg_base = (data & 0x03) << 4;
		m_mmc_prg_mask = (data & 0x02) ? 0x0f : 0x1f;
		m_mmc_chr_base = (data & 0x03) << 7;
		m_mmc_chr_mask = (data & 0x02) ? 0x7f : 0xff;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

/*************************************************************

 BMC-SUPERHIK_300IN1

 Unknown Bootleg Multigame Board
 Games: 100000 in 1, Super HIK 300 in 1, 1997 in 1

 iNES: mapper 212

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_hik300_w)
{
	LOG_MMC(("bmc_hik300_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	chr8(offset, CHRROM);

	if (offset < 0x4000)
	{
		prg16_89ab(offset);
		prg16_cdef(offset);
	}
	else
		prg32(offset >> 1);
}

/*************************************************************

 BMC-SUPERGUN-20IN1

 Unknown Bootleg Multigame Board
 Games: Super Gun 20 in 1

 iNES: mapper 214

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::supergun20in1_w)
{
	LOG_MMC(("supergun20in1_w, offset: %04x, data: %02x\n", offset, data));

	prg16_89ab(offset >> 2);
	prg16_cdef(offset >> 2);
	chr8(offset, CHRROM);
}

/*************************************************************

 BMC-72IN1

 Unknown Bootleg Multigame Board
 Games: 72 in 1, 115 in 1 and other multigame carts

 iNES: mapper 225

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_72in1_w)
{
	int hi_bank;
	int size_16;
	int bank;

	LOG_MMC(("bmc_72in1_w, offset: %04x, data: %02x\n", offset, data));

	chr8(offset, CHRROM);
	set_nt_mirroring((offset & 0x2000) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	hi_bank = offset & 0x40;
	size_16 = offset & 0x1000;
	bank = (offset & 0xf80) >> 7;
	if (size_16)
	{
		bank <<= 1;
		if (hi_bank)
			bank ++;

		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}

/*************************************************************

 BMC-76IN1

 Unknown Bootleg Multigame Board
 Games: 76 in 1, Super 42 in 1

 iNES: mapper 226

 In MESS: Supported.

 *************************************************************/

// does this work for super42in1 as well?!?
WRITE8_MEMBER(nes_carts_state::bmc_76in1_w)
{
	int hi_bank;
	int size_16;
	int bank;

	LOG_MMC(("bmc_76in1_w, offset: %04x, data: %02x\n", offset, data));

	if (offset & 0x01)
		m_mmc_latch2 = data;
	else
		m_mmc_latch1 = data;

	set_nt_mirroring(BIT(m_mmc_latch1, 6) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	hi_bank = m_mmc_latch1 & 0x01;
	size_16 = m_mmc_latch1 & 0x20;
	bank = ((m_mmc_latch1 & 0x1e) >> 1) | ((m_mmc_latch1 & 0x80) >> 3) | ((m_mmc_latch2 & 0x01) << 5);

	if (size_16)
	{
		bank <<= 1;
		if (hi_bank)
			bank ++;

		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);
}

/*************************************************************

 BMC-1200IN1

 Unknown Bootleg Multigame Board
 Games: 1200 in 1, 295 in 1, 76 in 1

 iNES: mapper 227

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_1200in1_w)
{
	int hi_bank;
	int size_32;
	int bank;

	LOG_MMC(("bmc_1200in1_w, offset: %04x, data: %02x\n", offset, data));

	hi_bank = offset & 0x04;
	size_32 = offset & 0x01;
	bank = ((offset & 0x78) >> 3) | ((offset & 0x0100) >> 4);
	if (!size_32)
	{
		bank <<= 1;
		if (hi_bank)
			bank ++;

		prg16_89ab(bank);
		prg16_cdef(bank);
	}
	else
		prg32(bank);

	if (!(offset & 0x80))
	{
		if (offset & 0x200)
			prg16_cdef(((bank << 1) & 0x38) + 7);
		else
			prg16_cdef(((bank << 1) & 0x38));
	}

	set_nt_mirroring(BIT(data, 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

/*************************************************************

 BMC-31IN1

 Unknown Bootleg Multigame Board
 Games: 31 in 1

 iNES: mapper 229

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_31in1_w)
{
	LOG_MMC(("bmc_31in1_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	chr8(offset, CHRROM);

	if ((offset & 0x1e) == 0)
	{
		prg16_89ab(0);
		prg16_89ab(1);
	}
	else
	{
		prg16_89ab(offset & 0x1f);
		prg16_89ab(offset & 0x1f);
	}
}

/*************************************************************

 BMC-22GAMES

 Unknown Bootleg Multigame Board
 Games: 22 in 1

 iNES: mapper 230

 In MESS: Partially Supported. It would need a reset
 to work (not possible yet)

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_22g_w)
{
	LOG_MMC(("bmc_22g_w, offset: %04x, data: %02x\n", offset, data));

	if (1)  // this should flip at reset
	{
		prg16_89ab(data & 0x07);
	}
	else
	{
		if (data & 0x20)
		{
			prg16_89ab((data & 0x1f) + 8);
			prg16_cdef((data & 0x1f) + 8);
		}
		else
		{
			prg16_89ab((data & 0x1f) + 8);
			prg16_cdef((data & 0x1f) + 9);
		}
		set_nt_mirroring(BIT(data, 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	}
}

/*************************************************************

 BMC-20IN1

 Unknown Bootleg Multigame Board
 Games: 20 in 1

 iNES: mapper 231

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_20in1_w)
{
	LOG_MMC(("bmc_20in1_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	prg16_89ab((offset & 0x1e));
	prg16_cdef((offset & 0x1e) | ((offset & 0x20) ? 1 : 0));
}

/*************************************************************

 BMC-110IN1

 Known Boards: Unknown Bootleg Board
 Games: 110 in 1

 iNES: mapper 255

 In MESS: Preliminary support.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_110in1_w)
{
	UINT8 map255_helper1 = (offset >> 12) ? 0 : 1;
	UINT8 map255_helper2 = ((offset >> 8) & 0x40) | ((offset >> 6) & 0x3f);

	LOG_MMC(("bmc_110in1_w, offset: %04x, data: %02x\n", offset, data));

	set_nt_mirroring((offset & 0x2000) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	prg16_89ab(map255_helper1 & ~map255_helper2);
	prg16_cdef(map255_helper1 | map255_helper2);
	chr8(((offset >> 8) & 0x40) | (offset & 0x3f), CHRROM);
}

/*************************************************************

 BMC-SUPERBIG-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Kunio 8 in 1, Super Big 7 in 1

 iNES: mapper 44

 In MESS: Supported. It also uses mmc3_irq.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_sbig7_w)
{
	UINT8 page;
	LOG_MMC(("bmc_sbig7_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x2001: /* $a001 - Select 128K ROM/VROM base (0..5) or last 256K ROM/VRAM base (6) */
			page = (data & 0x07);
			if (page > 6)
				page = 6;

			m_mmc_prg_base = page << 4;
			m_mmc_prg_mask = (page > 5) ? 0x1f : 0x0f;
			m_mmc_chr_base = page << 7;
			m_mmc_chr_mask = (page > 5) ? 0xff : 0x7f;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 BMC-HIK8IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Street Fighter V, various multigame carts

 iNES: mapper 45

 In MESS: Supported. It also uses mmc3_irq.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_hik8_m_w)
{
	LOG_MMC(("bmc_hik8_m_w, offset: %04x, data: %02x\n", offset, data));

	/* This bit is the "register lock". Once register are locked, writes go to WRAM
	 and there is no way to unlock them (except by resetting the machine) */
	if ((m_mmc_reg[3] & 0x40) && m_wram != NULL)
		m_wram[offset] = data;
	else
	{
		m_mmc_reg[m_mmc_count] = data;
		m_mmc_count = (m_mmc_count + 1) & 0x03;

		if (!m_mmc_count)
		{
			LOG_MMC(("bmc_hik8_m_w, command completed %02x %02x %02x %02x\n", m_mmc_reg[3],
						m_mmc_reg[2], m_mmc_reg[1], m_mmc_reg[0]));

			m_mmc_prg_base = m_mmc_reg[1];
			m_mmc_prg_mask = 0x3f ^ (m_mmc_reg[3] & 0x3f);
			m_mmc_chr_base = ((m_mmc_reg[2] & 0xf0) << 4) | m_mmc_reg[0];
			if (BIT(m_mmc_reg[2], 3))
				m_mmc_chr_mask = (1 << ((m_mmc_reg[2] & 7) + 1)) - 1;
			else if (m_mmc_reg[2])
				m_mmc_chr_mask = 0;
			else
				m_mmc_chr_mask = 0xff;  // i.e. we use the vrom_bank with no masking

			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
		}
	}
}

/*************************************************************

 BMC-SUPERHIK-4IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK 4 in 1

 iNES: mapper 49

 In MESS: Supported. It also uses mmc3_irq.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_hik4in1_m_w)
{
	LOG_MMC(("bmc_hik4in1_m_w, offset: %04x, data: %02x\n", offset, data));

	/* mid writes only work when WRAM is enabled. not sure if I should
	 change the condition to m_mmc_latch2==0x80 (i.e. what is the effect of
	 the read-only bit?) */
	if (m_mmc3_wram_protect & 0x80)
	{
		if (data & 0x01)    /* if this is 0, then we have 32k PRG blocks */
		{
			m_mmc_prg_base = (data & 0xc0) >> 2;
			m_mmc_prg_mask = 0x0f;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		}
		else
			prg32((data & 0x30) >> 4);

		m_mmc_chr_base = (data & 0xc0) << 1;
		m_mmc_chr_mask = 0x7f;
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

/*************************************************************

 BMC-BALLGAMES-11IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: 11 in 1 Ball Games

 iNES: mapper 51

 In MESS: Partially Supported.

 *************************************************************/

static void bmc_ball11_set_banks( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->set_nt_mirroring((state->m_mmc_reg[0] == 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	if (state->m_mmc_reg[0] & 0x01)
	{
		state->prg32(state->m_mmc_reg[1]);
	}
	else
	{
		state->prg16_89ab((state->m_mmc_reg[1] << 1) | (state->m_mmc_reg[0] >> 1));
		state->prg16_cdef((state->m_mmc_reg[1] << 1) | 0x07);
	}
}

WRITE8_MEMBER(nes_carts_state::bmc_ball11_m_w)
{
	LOG_MMC(("bmc_ball11_m_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_reg[0] = ((data >> 1) & 0x01) | ((data >> 3) & 0x02);
	bmc_ball11_set_banks(machine());
}

WRITE8_MEMBER(nes_carts_state::bmc_ball11_w)
{
	LOG_MMC(("bmc_ball11_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6000)
	{
		case 0x4000:    // here we also update reg[0] upper bit
			m_mmc_reg[0] = (m_mmc_reg[0] & 0x01) | ((data >> 3) & 0x02);
		case 0x0000:
		case 0x2000:
		case 0x6000:
			m_mmc_reg[1] = data & 0x0f;
			bmc_ball11_set_banks(machine());
			break;
	}
}

/*************************************************************

 BMC-MARIOPARTY-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Mario 7 in 1

 MMC3 clone

 iNES: mapper 52

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_mario7in1_m_w)
{
	UINT8 map52_helper1, map52_helper2;
	LOG_MMC(("bmc_mario7in1_m_w, offset: %04x, data: %02x\n", offset, data));

	/* mid writes only work when WRAM is enabled. not sure if I should
	 change the condition to m_map52_reg_written == 0x80 (i.e. what is the effect of
	 the read-only bit?) and it only can happen once! */
	if ((m_mmc3_wram_protect & 0x80) && !m_map52_reg_written)
	{
		map52_helper1 = (data & 0x08);
		map52_helper2 = (data & 0x40);

		m_mmc_prg_base = map52_helper1 ? ((data & 0x07) << 4) : ((data & 0x06) << 4);
		m_mmc_prg_mask = map52_helper1 ? 0x0f : 0x1f;
		m_mmc_chr_base = ((data & 0x20) << 4) | ((data & 0x04) << 6) | (map52_helper2 ? ((data & 0x10) << 3) : 0);
		m_mmc_chr_mask = map52_helper2 ? 0x7f : 0xff;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);

		m_map52_reg_written = 1;
	}
	else
		m_wram[offset] = data;
}

/*************************************************************

 BMC-GOLD-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK Gold 7 in 1, Golden 7 in 1 and many more

 MMC3 clone, same as BMC-MARIOPARTY-7IN1 but with switched CHR
 bank lines

 iNES: mapper 52

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_gold7in1_m_w)
{
	UINT8 map52_helper1, map52_helper2;
	LOG_MMC(("bmc_gold7in1_m_w, offset: %04x, data: %02x\n", offset, data));

	if ((m_mmc3_wram_protect & 0x80) && !m_map52_reg_written)
	{
		map52_helper1 = (data & 0x08);
		map52_helper2 = (data & 0x40);

		m_mmc_prg_base = map52_helper1 ? ((data & 0x07) << 4) : ((data & 0x06) << 4);
		m_mmc_prg_mask = map52_helper1 ? 0x0f : 0x1f;
		m_mmc_chr_base = ((data & 0x20) << 3) | ((data & 0x04) << 7) | (map52_helper2 ? ((data & 0x10) << 3) : 0);
		m_mmc_chr_mask = map52_helper2 ? 0x7f : 0xff;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);

		m_map52_reg_written = BIT(data, 7); // mc_2hikg & mc_s3nt3 write here multiple time
	}
	else
		m_wram[offset] = data;
}

/*************************************************************

 BMC-GOLDENCARD-6IN1

 Known Boards: Unknown Bootleg Multigame Board
 Games: Golden Card 6 in 1

 MMC3 clone

 iNES: mapper 217

 In MESS: Supported.

 *************************************************************/

// remove mask & base parameters!
static void bmc_gc6in1_set_prg( running_machine &machine, int prg_base, int prg_mask )
{
	nes_state *state = machine.driver_data<nes_state>();
	state->m_mmc_prg_base = (state->m_mmc_reg[1] & 0x08) ? 0 : (state->m_mmc_reg[1] & 0x10);
	state->m_mmc_prg_mask = (state->m_mmc_reg[1] & 0x08) ? 0x1f : 0x0f;

	state->m_mmc_prg_base |= ((state->m_mmc_reg[1] & 0x03) << 5);

	mmc3_set_prg(machine, state->m_mmc_prg_base, state->m_mmc_prg_mask);
}

static void bmc_gc6in1_set_chr( running_machine &machine, UINT8 chr )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_page = (state->m_mmc_latch1 & 0x80) >> 5;
	int chr_base = (state->m_mmc_reg[1] & 0x08) ? 0x00 : ((state->m_mmc_reg[1] & 0x10) << 3);
	int chr_mask = (state->m_mmc_reg[1] & 0x08) ? 0xff : 0x7f;

	chr_base |= ((state->m_mmc_reg[1] & 0x03) << 8);

	state->chr1_x(chr_page ^ 0, chr_base | ((state->m_mmc_vrom_bank[0] & ~0x01) & chr_mask), chr);
	state->chr1_x(chr_page ^ 1, chr_base | ((state->m_mmc_vrom_bank[0] |  0x01) & chr_mask), chr);
	state->chr1_x(chr_page ^ 2, chr_base | ((state->m_mmc_vrom_bank[1] & ~0x01) & chr_mask), chr);
	state->chr1_x(chr_page ^ 3, chr_base | ((state->m_mmc_vrom_bank[1] |  0x01) & chr_mask), chr);
	state->chr1_x(chr_page ^ 4, chr_base | (state->m_mmc_vrom_bank[2] & chr_mask), chr);
	state->chr1_x(chr_page ^ 5, chr_base | (state->m_mmc_vrom_bank[3] & chr_mask), chr);
	state->chr1_x(chr_page ^ 6, chr_base | (state->m_mmc_vrom_bank[4] & chr_mask), chr);
	state->chr1_x(chr_page ^ 7, chr_base | (state->m_mmc_vrom_bank[5] & chr_mask), chr);
}

WRITE8_MEMBER(nes_carts_state::bmc_gc6in1_l_w)
{
	UINT8 bank;
	LOG_MMC(("bmc_gc6in1_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset == 0x1000)
	{
		m_mmc_reg[0] = data;
		if (data & 0x80)
		{
			bank = (data & 0x0f) | ((m_mmc_reg[1] & 0x03) << 4);
			prg16_89ab(bank);
			prg16_cdef(bank);
		}
		else
			bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}
	else if (offset == 0x1001)
	{
		m_mmc_reg[1] = data;
		bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}
	else if (offset == 0x1007)
	{
		m_mmc_reg[2] = data;
	}
}

WRITE8_MEMBER(nes_carts_state::bmc_gc6in1_w)
{
	UINT8 mmc_helper, cmd;
	static const UINT8 conv_table[8] = {0, 6, 3, 7, 5, 2, 4, 1};
	LOG_MMC(("bmc_gc6in1_w, offset: %04x, data: %02x\n", offset, data));

	if (!m_mmc_reg[2]) // in this case we act like MMC3, only with alt prg/chr handlers
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				mmc_helper = m_mmc3_latch ^ data;
				m_mmc3_latch = data;

				/* Has PRG Mode changed? */
				if (mmc_helper & 0x40)
					bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

				/* Has CHR Mode changed? */
				if (mmc_helper & 0x80)
					bmc_gc6in1_set_chr(machine(), m_mmc_chr_source);
				break;

			case 0x0001:
				cmd = m_mmc3_latch & 0x07;
				switch (cmd)
			{
				case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					m_mmc_vrom_bank[cmd] = data;
					bmc_gc6in1_set_chr(machine(), m_mmc_chr_source);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
					break;
			}
				break;

			default:
				txrom_w(space, offset, data, mem_mask);
				break;
		}
	}
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0000:
				txrom_w(space, 0x4000, data, mem_mask);
				break;

			case 0x0001:
				data = (data & 0xc0) | conv_table[data & 0x07];
				mmc_helper = m_mmc3_latch ^ data;
				m_mmc3_latch = data;

				/* Has PRG Mode changed? */
				if (mmc_helper & 0x40)
					bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

				/* Has CHR Mode changed? */
				if (mmc_helper & 0x80)
					bmc_gc6in1_set_chr(machine(), m_mmc_chr_source);

				m_mmc_reg[3] = 1;
				break;

			case 0x2000:
				cmd = m_mmc3_latch & 0x07;
				if (m_mmc_reg[3])
				{
					m_mmc_reg[3] = 0;
					switch (cmd)
					{
						case 0: case 1: // these do not need to be separated: we take care of them in set_chr!
						case 2: case 3: case 4: case 5:
							m_mmc_vrom_bank[cmd] = data;
							bmc_gc6in1_set_chr(machine(), m_mmc_chr_source);
							break;
						case 6:
						case 7:
							m_mmc_prg_bank[cmd - 6] = data;
							bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
							break;
					}
				}
				break;


			case 0x2001:
				set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_w(space, offset, data, mem_mask);
				break;
		}
	}
}

/*************************************************************

 BMC-FAMILY-4646B

 Known Boards: Unknown Multigame Bootleg Board (4646B)
 Games: 2 in 1 - Family Kid & Aladdin 4

 MMC3 clone

 iNES: mapper 134

 In MESS: Supported.

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_family4646_m_w)
{
	LOG_MMC(("bmc_family4646_m_w, offset: %04x, data: %02x\n", offset, data));

	if (offset == 0x01)
	{
		m_mmc_prg_base = (data & 0x02) << 4;
		m_mmc_prg_mask = 0x1f;
		m_mmc_chr_base = (data & 0x20) << 3;
		m_mmc_chr_mask = 0xff;
		mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
		mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
	}
}

/*************************************************************

 BMC-VT5201

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_vt5201_w)
{
	LOG_MMC(("bmc_vt5201_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_latch1 = BIT(offset, 8);

	// not sure about this mirroring bit!!
	// without it TN 95 in 1 has glitches in Lunar Ball; with it TN 95 in 1 has glitches in Galaxian!
	set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	if (BIT(offset, 7))
	{
		prg16_89ab((offset >> 4) & 0x07);
		prg16_cdef((offset >> 4) & 0x07);
	}
	else
		prg32((offset >> 5) & 0x03);
	chr8(offset, CHRROM);
}

READ8_MEMBER(nes_carts_state::bmc_vt5201_r)
{
	LOG_MMC(("bmc_vt5201_r, offset: %04x\n", offset));
	//  m_mmc_dipsetting = ioport("CARTDIPS")->read();

	if (m_mmc_latch1)
		return m_mmc_dipsetting; // cart mode, depending on the Dip Switches (always zero atm, given we have no way to add cart-based DIPs)
	else
		return mmc_hi_access_rom(machine(), offset);
}

/*************************************************************

 BMC-BS-5

 Games: a few 4 in 1 multicarts

 *************************************************************/

static void bmc_bs5_update_banks( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->prg8_89(state->m_mmc_prg_bank[0]);
	state->prg8_ab(state->m_mmc_prg_bank[1]);
	state->prg8_cd(state->m_mmc_prg_bank[2]);
	state->prg8_ef(state->m_mmc_prg_bank[3]);
	state->chr2_0(state->m_mmc_vrom_bank[0], CHRROM);
	state->chr2_2(state->m_mmc_vrom_bank[1], CHRROM);
	state->chr2_4(state->m_mmc_vrom_bank[2], CHRROM);
	state->chr2_6(state->m_mmc_vrom_bank[3], CHRROM);
}

WRITE8_MEMBER(nes_carts_state::bmc_bs5_w)
{
	UINT8 bs5_helper = (offset & 0xc00) >> 10;
	LOG_MMC(("bmc_bs5_w, offset: %04x, data: %02x\n", offset, data));
//  m_mmc_dipsetting = ioport("CARTDIPS")->read();

	switch (offset & 0x7000)
	{
		case 0x0000:
			m_mmc_vrom_bank[bs5_helper] = offset & 0x1f;
			break;
		case 0x2000:
			if (BIT(offset, m_mmc_dipsetting + 4))  // mmc_dipsetting is always zero atm, given we have no way to add cart-based DIPs
				m_mmc_prg_bank[bs5_helper] = offset & 0x0f;
			break;
	}
	bmc_bs5_update_banks(machine());
}

/*************************************************************

 BMC-810544-C-A1

 Games: 200-in-1 Elfland

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_810544_w)
{
	UINT8 bank = (offset >> 7);
	LOG_MMC(("bmc_810544_w, offset: %04x, data: %02x\n", offset, data));

	if (!BIT(offset, 6))
	{
		prg16_89ab((bank << 1) | BIT(offset, 5));
		prg16_cdef((bank << 1) | BIT(offset, 5));
	}
	else
		prg32(bank);

	set_nt_mirroring(BIT(offset, 4) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8(offset & 0x0f, CHRROM);
}

/*************************************************************

 BMC-NTD-03

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::bmc_ntd03_w)
{
	UINT8 pbank, cbank;
	LOG_MMC(("bmc_ntd03_w, offset: %04x, data: %02x\n", offset, data));

	pbank = (offset >> 10) & 0x1e;
	cbank = ((offset & 0x300) >> 5) | (offset & 0x07);

	if (BIT(offset, 7))
	{
		prg16_89ab(pbank | BIT(offset, 6));
		prg16_cdef(pbank | BIT(offset, 6));
	}
	else
		prg32(pbank >> 1);

	set_nt_mirroring(BIT(offset, 10) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	chr8(cbank, CHRROM);
}

/*************************************************************

 BMC-GHOSTBUSTERS63IN1

 in MESS: only preliminar support

 *************************************************************/

static void bmc_gb63_update( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->set_nt_mirroring(BIT(state->m_mmc_reg[0], 6) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);

	if (BIT(state->m_mmc_reg[0], 5))
	{
		state->prg16_89ab(state->m_mmc_reg[0] & 0x1f);  // FIXME: here probably we could also have PRGRAM, depending on mmc_latch1!
		state->prg16_cdef(state->m_mmc_reg[0] & 0x1f);  // FIXME: here probably we could also have PRGRAM, depending on mmc_latch1!
	}
	else
		state->prg32((state->m_mmc_reg[0] & 0x1f) >> 1);    // FIXME: here probably we could also have PRGRAM, depending on mmc_latch1!

	if (BIT(state->m_mmc_reg[1], 2))
		state->chr8(0, CHRRAM);
//  else
//      state->chr8(0, CHRROM);
}

WRITE8_MEMBER(nes_carts_state::bmc_gb63_w)
{
	LOG_MMC(("bmc_gb63_w, offset: %04x, data: %02x\n", offset, data));

	m_mmc_reg[offset & 1] = data;
	m_mmc_latch1 = BIT(m_mmc_reg[0], 7) | (BIT(m_mmc_reg[1], 0) << 1);

}

READ8_MEMBER(nes_carts_state::bmc_gb63_r)
{
	LOG_MMC(("bmc_gb63_r, offset: %04x\n", offset));
	//  m_mmc_dipsetting = ioport("CARTDIPS")->read();

	if (m_mmc_latch1 == 1)
		return 0xff;    // open bus
	else
		return mmc_hi_access_rom(machine(), offset);
}

/*************************************************************

 UNL-EDU2000

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::edu2k_w)
{
	LOG_MMC(("edu2k_w, offset: %04x, data: %02x\n", offset, data));

	prg32(data & 0x1f);
	wram_bank((data & 0xc0) >> 6, NES_WRAM);
}

/*************************************************************

 UNL-H2288

 *************************************************************/

static void h2288_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[0] & 0x40))
		state->prg8_x(start, bank);
}

WRITE8_MEMBER(nes_carts_state::h2288_l_w)
{
	LOG_MMC(("h2288_l_w offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset >= 0x1800)
	{
		m_mmc_reg[offset & 1] = data;
		if (m_mmc_reg[0] & 0x40)
		{
			UINT8 helper1 = (m_mmc_reg[0] & 0x05) | ((m_mmc_reg[0] >> 2) & 0x0a);
			UINT8 helper2 = BIT(m_mmc_reg[0], 1);
			prg16_89ab(helper1 & ~helper2);
			prg16_cdef(helper1 |  helper2);
		}
		else
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
	}
}

READ8_MEMBER(nes_carts_state::h2288_l_r)
{
	LOG_MMC(("h2288_l_r offset: %04x\n", offset));
	offset += 0x100;

	if (offset >= 0x1000)
	{
		int helper = offset >> 8;
		if (offset & 1)
			return helper | 0x01;
		else
			return helper ^ 0x01;
	}

	return 0;
}

WRITE8_MEMBER(nes_carts_state::h2288_w)
{
	static const UINT8 conv_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};
	LOG_MMC(("h2288_w, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x6001)
	{
		case 0x0000:
			txrom_w(space, 0x0000, (data & 0xc0) | conv_table[data & 0x07], mem_mask);
			break;

		default:
			txrom_w(space, offset, data, mem_mask);
			break;
	}
}

/*************************************************************

 UNL-SHJY3

 *************************************************************/

/* I think the IRQ should only get fired if enough CPU cycles have passed, but we don't implement (yet) this part */
static void shjy3_irq( device_t *device, int scanline, int vblank, int blanked )
{
	nes_state *state = device->machine().driver_data<nes_state>();
	if (state->m_IRQ_enable & 0x02)
	{
		if (state->m_IRQ_count == 0xff)
		{
			state->m_IRQ_count = state->m_IRQ_count_latch;
			state->m_IRQ_enable = state->m_IRQ_enable | ((state->m_IRQ_enable & 0x01) << 1);
			state->m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
		}
		else
			state->m_IRQ_count++;
	}
}

static void shjy3_update( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	int i;

	state->prg8_89(state->m_mmc_prg_bank[0]);
	state->prg8_ab(state->m_mmc_prg_bank[1]);

	for (i = 0; i < 8; i++)
	{
		UINT8 chr_bank = state->m_mmc_vrom_bank[i] | (state->m_mmc_extra_bank[i] << 4);
		if (state->m_mmc_vrom_bank[i] == 0xc8)
		{
			state->m_mmc_latch1 = 0;
			continue;
		}
		else if (state->m_mmc_vrom_bank[i] == 0x88)
		{
			state->m_mmc_latch1 = 1;
			continue;
		}
		if ((state->m_mmc_vrom_bank[i] == 4 || state->m_mmc_vrom_bank[i] == 5) && !state->m_mmc_latch1)
			state->chr1_x(i, chr_bank & 1, CHRRAM);
		else
			state->chr1_x(i, chr_bank, CHRROM);
	}
}

WRITE8_MEMBER(nes_carts_state::shjy3_w)
{
	UINT8 mmc_helper, shift;
	LOG_MMC(("shjy3_w, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x3000 && offset <= 0x600c)
	{
		mmc_helper = ((offset & 8) | (offset >> 8)) >> 3;
		mmc_helper += 2;
		mmc_helper &= 7;
		shift = offset & 4;

		m_mmc_vrom_bank[mmc_helper] = (m_mmc_vrom_bank[mmc_helper] & (0xf0 >> shift)) | ((data & 0x0f) << shift);
		if (shift)
			m_mmc_extra_bank[mmc_helper] = data >> 4;
	}
	else
	{
		switch (offset)
		{
			case 0x0010:
				m_mmc_prg_bank[0] = data;
				break;
			case 0x2010:
				m_mmc_prg_bank[1] = data;
				break;
			case 0x1400:
				switch (data & 0x03)
				{
					case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
					case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
					case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
					case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				}
				break;
			case 0x7000:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0xf0) | (data & 0x0f);
				break;
			case 0x7004:
				m_IRQ_count_latch = (m_IRQ_count_latch & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x7008:
				m_IRQ_enable = data & 0x03;
				if (m_IRQ_enable & 0x02)
					m_IRQ_count = m_IRQ_count_latch;
				break;
		}
	}
	shjy3_update(machine());
}

/*************************************************************

 UNL-603-5052

 MMC3 + protection access in 0x4020 - 0x7fff

 in MESS: Partial support

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::unl_6035052_extra_w)
{
	LOG_MMC(("unl_6035052_extra_w, offset: %04x, data: %02x\n", offset, data));
	m_mmc_latch1 = data & 0x03;
	if (m_mmc_latch1 == 1)
		m_mmc_latch1 = 2;
}

READ8_MEMBER(nes_carts_state::unl_6035052_extra_r)
{
	LOG_MMC(("unl_6035052_extra_r, offset: %04x\n", offset));
	return m_mmc_latch1;
}


/*************************************************************

 BMC-POWERJOY

 *************************************************************/

static void pjoy84_prg_cb( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 flip = (state->m_mmc3_latch & 0x40) ? 2 : 0;

	if (!(state->m_mmc_reg[3] & 0x03))
		state->prg8_x(start, bank);
	else if (start == flip)
	{
		if ((state->m_mmc_reg[3] & 0x03) == 0x03)
			state->prg32(bank >> 2);
		else
		{
			state->prg16_89ab(bank >> 1);
			state->prg16_cdef(bank >> 1);
		}
	}
}

static void pjoy84_chr_cb( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (!(state->m_mmc_reg[3] & 0x10))
		state->chr1_x(start, bank, source);
}

INLINE void pjoy84_set_base_mask( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->m_mmc_prg_base = ((state->m_mmc_reg[0] & (0x06 | BIT(state->m_mmc_reg[0], 6))) << 4) |
						(BIT(state->m_mmc_reg[0], 4) << 7);

	state->m_mmc_chr_base = ((~state->m_mmc_reg[0] << 0) & 0x080 & state->m_mmc_reg[2]) |
						((state->m_mmc_reg[0] << 4) & 0x080 & state->m_mmc_reg[0]) |
						((state->m_mmc_reg[0] << 3) & 0x100) |
						((state->m_mmc_reg[0] << 5) & 0x200);

	state->m_mmc_prg_mask = BIT(state->m_mmc_reg[0], 6) ? 0x0f : 0x1f;
	state->m_mmc_chr_mask = BIT(state->m_mmc_reg[0], 7) ? 0x7f : 0xff;
}

WRITE8_MEMBER(nes_carts_state::pjoy84_m_w)
{
	LOG_MMC(("pjoy84_m_w offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x03)
	{
		case 0x00:
		case 0x03:
			if (m_mmc_reg[3] & 0x80)
				return; // else we act as if offset & 3 = 1,2
		case 0x01:
		case 0x02:
			m_mmc_reg[offset & 0x03] = data;
			pjoy84_set_base_mask(machine());
			if (m_mmc_reg[3] & 0x10)
				chr8((m_mmc_chr_base >> 3) | (m_mmc_reg[2] & 0x0f), m_mmc_chr_source);
			else
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;
	}
}

/*************************************************************

 SOMERI TEAM

 iNES: mapper 116

 Emulation note about regs in MESS: currently,
 - state->m_mmc_cmd1 represent the board mode
 - state->m_mmc_reg[n] for n=0,1,2,3 represent the MMC1 regs
 - state->m_mmc_prg_bank[n] for n=0,...,3 represent the MMC3 PRG banks
 - state->m_mmc_vrom_bank[n] for n=0,...,5 represent the MMC3 CHR banks
 - state->m_mmc_prg_bank[n] for n=4,5 represent the VRC2 PRG banks
 - state->m_mmc_vrom_bank[n] for n=6,...,13 represent the VRC2 CHR banks

 In MESS: Preliminary support

 *************************************************************/

// MMC1 Mode emulation
static void someri_mmc1_set_prg( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 prg_mode = state->m_mmc_reg[0] & 0x0c;
	UINT8 prg_offset = state->m_mmc_reg[1] & 0x10;

	switch (prg_mode)
	{
		case 0x00:
		case 0x04:
			state->prg32((prg_offset + state->m_mmc_reg[3]) >> 1);
			break;
		case 0x08:
			state->prg16_89ab(prg_offset + 0);
			state->prg16_cdef(prg_offset + state->m_mmc_reg[3]);
			break;
		case 0x0c:
			state->prg16_89ab(prg_offset + state->m_mmc_reg[3]);
			state->prg16_cdef(prg_offset + 0x0f);
			break;
	}
}

static void someri_mmc1_set_chr( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8 chr_mode = BIT(state->m_mmc_reg[0], 4);

	if (chr_mode)
	{
		state->chr4_0(state->m_mmc_reg[1] & 0x1f, state->m_mmc_chr_source);
		state->chr4_4(state->m_mmc_reg[2] & 0x1f, state->m_mmc_chr_source);
	}
	else
		state->chr8((state->m_mmc_reg[1] & 0x1f) >> 1, state->m_mmc_chr_source);
}

WRITE8_MEMBER(nes_carts_state::someri_mmc1_w)
{
	assert(m_mmc_cmd1 == 2);

	if (data & 0x80)
	{
		m_mmc1_count = 0;
		m_mmc1_latch = 0;

		m_mmc_reg[0] |= 0x0c;
		someri_mmc1_set_prg(machine());
		return;
	}

	if (m_mmc1_count < 5)
	{
		if (m_mmc1_count == 0) m_mmc1_latch = 0;
		m_mmc1_latch >>= 1;
		m_mmc1_latch |= (data & 0x01) ? 0x10 : 0x00;
		m_mmc1_count++;
	}

	if (m_mmc1_count == 5)
	{
		switch (offset & 0x6000)
		{
			case 0x0000:
				m_mmc_reg[0] = m_mmc1_latch;
				switch (m_mmc_reg[0] & 0x03)
				{
				case 0: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 2: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HORZ); break;
				}
				someri_mmc1_set_chr(machine());
				someri_mmc1_set_prg(machine());
				break;
			case 0x2000:
				m_mmc_reg[1] = m_mmc1_latch;
				someri_mmc1_set_chr(machine());
				someri_mmc1_set_prg(machine());
				break;
			case 0x4000:
				m_mmc_reg[2] = m_mmc1_latch;
				someri_mmc1_set_chr(machine());
				break;
			case 0x6000:
				m_mmc_reg[3] = m_mmc1_latch;
				someri_mmc1_set_prg(machine());
				break;
		}

		m_mmc1_count = 0;
	}
}

// MMC3 Mode emulation
WRITE8_MEMBER(nes_carts_state::someri_mmc3_w)
{
	UINT8 mmc_helper, cmd;

	assert(m_mmc_cmd1 == 1);
	switch (offset & 0x6001)
	{
		case 0x0000:
			mmc_helper = m_mmc3_latch ^ data;
			m_mmc3_latch = data;

			if (mmc_helper & 0x40)
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);

			if (mmc_helper & 0x80)
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

		case 0x0001:
			cmd = m_mmc3_latch & 0x07;
			switch (cmd)
			{
			case 0: case 1:
			case 2: case 3: case 4: case 5:
				m_mmc_vrom_bank[cmd] = data;
				mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
				break;
			case 6:
			case 7:
				m_mmc_prg_bank[cmd - 6] = data;
				mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
				break;
			}
			break;

		case 0x2000:
			set_nt_mirroring(BIT(data, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x2001: break;
		case 0x4000: m_IRQ_count_latch = data; break;
		case 0x4001: m_IRQ_count = 0; break;
		case 0x6000: m_IRQ_enable = 0; break;
		case 0x6001: m_IRQ_enable = 1; break;
	}
}

// VRC2 Mode emulation
WRITE8_MEMBER(nes_carts_state::someri_vrc2_w)
{
	UINT8 bank, shift;

	assert(m_mmc_cmd1 == 0);

	if (offset < 0x1000)
	{
		m_mmc_prg_bank[4] = data;
		prg8_89(m_mmc_prg_bank[4]);
	}
	else if (offset < 0x2000)
	{
		switch (data & 0x03)
		{
			case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
	else if (offset < 0x3000)
	{
		m_mmc_prg_bank[5] = data;
		prg8_ab(m_mmc_prg_bank[5]);
	}
	else if (offset < 0x7000)
	{
		bank = ((offset & 0x7000) - 0x3000) / 0x0800 + BIT(offset, 1);
		shift = BIT(offset, 2) * 4;
		data = (data & 0x0f) << shift;
		m_mmc_vrom_bank[6 + bank] = data | m_mmc_chr_base;
		chr1_x(bank, m_mmc_vrom_bank[6 + bank], CHRROM);
	}
}

WRITE8_MEMBER(nes_carts_state::someri_w)
{
	LOG_MMC(("someri_w mode %d, offset: %04x, data: %02x\n", m_mmc_cmd1, offset, data));

	switch (m_mmc_cmd1)
	{
		case 0x00: someri_vrc2_w(space, offset, data, mem_mask); break;
		case 0x01: someri_mmc3_w(space, offset, data, mem_mask); break;
		case 0x02: someri_mmc1_w(space, offset, data, mem_mask); break;
	}
}

static void someri_mode_update( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	int i;

	switch (state->m_mmc_cmd1)
	{
		case 0x00:
			state->prg8_89(state->m_mmc_prg_bank[4]);
			state->prg8_ab(state->m_mmc_prg_bank[5]);
			for (i = 0; i < 8; i++)
				state->chr1_x(i, state->m_mmc_vrom_bank[6 + i], CHRROM);
			break;
		case 0x01:
			mmc3_set_prg(machine, state->m_mmc_prg_base, state->m_mmc_prg_mask);
			mmc3_set_chr(machine, state->m_mmc_chr_source, state->m_mmc_chr_base, state->m_mmc_chr_mask);
			break;
		case 0x02:
			someri_mmc1_set_prg(machine);
			someri_mmc1_set_chr(machine);
			break;
	}
}

WRITE8_MEMBER(nes_carts_state::someri_l_w)
{
	LOG_MMC(("someri_l_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x100;

	if (offset & 0x100)
	{
		m_mmc_cmd1 = data & 0x03;
		m_mmc_chr_base = ((m_mmc_cmd1 & 0x04) << 6);
		if (m_mmc_cmd1 != 1)
			m_IRQ_enable = 0;
		someri_mode_update(machine());
	}
}

#ifdef UNUSED_FUNCTION
/*************************************************************

 FUJIYA Board - mapper 170 according to NEStopia

 Which games are supposed to use this?

 *************************************************************/

WRITE8_MEMBER(nes_carts_state::fujiya_m_w)
{
	LOG_MMC(("fujiya_m_w, offset: %04x, data: %02x\n", offset, data));
	offset += 0x6000;

	if (offset == 0x6502 || offset == 0x7000)
		m_mmc_latch1 = (data & 0x40) << 1;
}

READ8_MEMBER(nes_carts_state::fujiya_m_r)
{
	LOG_MMC(("fujiya_m_r, offset: %04x\n", offset));
	offset += 0x6000;

	if (offset == 0x7001 || offset == 0x7777)
		return m_mmc_latch1 | ((offset >> 8) & 0x7f);

	return 0;
}
#endif




typedef void (*nes_ppu_latch)(device_t *device, offs_t offset);

struct nes_memory_accessor
{
	write8_delegate  write;
	read8_delegate  read;
};

struct nes_pcb_intf
{
	int                     mmc_pcb;
	nes_memory_accessor     mmc_l;  /* $4100-$5fff read/write routines */
	nes_memory_accessor     mmc_m;  /* $6000-$7fff read/write routines */
	nes_memory_accessor     mmc_h;  /* $8000-$ffff read/write routines */
	nes_ppu_latch           mmc_ppu_latch;
	ppu2c0x_scanline_cb     mmc_scanline;
	ppu2c0x_hblank_cb       mmc_hblank;
};


#define NES_NOACCESS \
{write8_delegate(), read8_delegate()}

#define NES_READONLY(a) \
{write8_delegate(), read8_delegate(FUNC(a),(nes_state *)0)}

#define NES_WRITEONLY(a) \
{write8_delegate(FUNC(a),(nes_state *)0), read8_delegate()}

#define NES_READWRITE(a, b) \
{write8_delegate(FUNC(a),(nes_state *)0), read8_delegate(FUNC(b),(nes_state *)0)}


WRITE8_MEMBER(nes_carts_state::dummy_l_w)
{
	logerror("write access, offset: %04x, data: %02x\n", offset + 0x4100, data);
}

WRITE8_MEMBER(nes_carts_state::dummy_m_w)
{
	logerror("write access, offset: %04x, data: %02x\n", offset + 0x6000, data);
}

WRITE8_MEMBER(nes_carts_state::dummy_w)
{
	logerror("write access, offset: %04x, data: %02x\n", offset + 0x8000, data);
}

READ8_MEMBER(nes_carts_state::dummy_l_r)
{
	logerror("read access, offset: %04x\n", offset + 0x4100);
	return 0x00;
}

READ8_MEMBER(nes_carts_state::dummy_m_r)
{
	logerror("read access, offset: %04x\n", offset + 0x6000);
	return 0x00;
}

READ8_MEMBER(nes_carts_state::dummy_r)
{
	logerror("read access, offset: %04x\n", offset + 0x8000);
	return 0x00;
}

static const nes_pcb_intf nes_intf_list[] =
{
	{ STD_NROM,             NES_NOACCESS, NES_NOACCESS, NES_NOACCESS,                         NULL, NULL, NULL },
	{ HVC_FAMBASIC,         NES_NOACCESS, NES_NOACCESS, NES_NOACCESS,                         NULL, NULL, NULL },
	{ GG_NROM,              NES_NOACCESS, NES_NOACCESS, NES_NOACCESS,                         NULL, NULL, NULL },
	{ STD_UXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::uxrom_w),               NULL, NULL, NULL },
	{ STD_UN1ROM,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::un1rom_w),              NULL, NULL, NULL },
	{ STD_CPROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::cprom_w),               NULL, NULL, NULL },
	{ STD_CNROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::cnrom_w),               NULL, NULL, NULL },
	{ BANDAI_PT554,         NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bandai_pt554_m_w), NES_WRITEONLY(nes_carts_state::cnrom_w), NULL, NULL, NULL },
	{ STD_AXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::axrom_w),               NULL, NULL, NULL },
	{ STD_PXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::pxrom_w),               mmc2_latch, NULL, NULL },
	{ STD_FXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::fxrom_w),               mmc2_latch, NULL, NULL },
	{ STD_BXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bxrom_w),               NULL, NULL, NULL },
	{ STD_GXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::gxrom_w),               NULL, NULL, NULL },
	{ STD_MXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::gxrom_w),               NULL, NULL, NULL },
	{ STD_NXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ntbrom_w),              NULL, NULL, NULL },
	{ SUNSOFT_DCS,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ntbrom_w),              NULL, NULL, NULL },
	{ STD_JXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jxrom_w),               NULL, NULL, jxrom_irq },
	{ STD_SXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sxrom_w),               NULL, NULL, NULL },
	{ STD_SOROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sxrom_w),               NULL, NULL, NULL },
	{ STD_SXROM_A,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sxrom_w),               NULL, NULL, NULL },
	{ STD_SOROM_A,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sxrom_w),               NULL, NULL, NULL },
	{ STD_TXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },
	{ STD_TVROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },
	{ STD_TKROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },
	{ STD_HKROM,            NES_NOACCESS, NES_READWRITE(nes_carts_state::hkrom_m_w, nes_carts_state::hkrom_m_r), NES_WRITEONLY(nes_carts_state::hkrom_w),     NULL, NULL, mmc3_irq },
	{ STD_TQROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tqrom_w),               NULL, NULL, mmc3_irq },
	{ STD_TXSROM,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txsrom_w),              NULL, NULL, mmc3_irq },
	{ STD_DXROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dxrom_w),               NULL, NULL, NULL },
	{ STD_DRROM,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dxrom_w),               NULL, NULL, NULL },
	{ NAMCOT_34X3,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dxrom_w),               NULL, NULL, NULL },
	{ NAMCOT_3425,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::namcot3425_w),          NULL, NULL, NULL },
	{ NAMCOT_3446,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::namcot3446_w),          NULL, NULL, NULL },
	{ NAMCOT_3453,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::namcot3453_w),          NULL, NULL, NULL },
	{ STD_EXROM,            NES_READWRITE(nes_carts_state::exrom_l_w, nes_carts_state::exrom_l_r), NES_NOACCESS, NES_NOACCESS,               NULL, NULL, mmc5_irq },
	{ NES_QJ,               NES_NOACCESS, NES_WRITEONLY(nes_carts_state::qj_m_w), NES_WRITEONLY(nes_carts_state::txrom_w),      NULL, NULL, mmc3_irq },
	{ PAL_ZZ,               NES_NOACCESS, NES_WRITEONLY(nes_carts_state::zz_m_w), NES_WRITEONLY(nes_carts_state::txrom_w),      NULL, NULL, mmc3_irq },
	{ UXROM_CC,             NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::uxrom_cc_w),            NULL, NULL, NULL },
	//
	{ DIS_74X139X74,        NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dis_74x139x74_m_w), NES_NOACCESS,     NULL, NULL, NULL },
	{ DIS_74X377,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dis_74x377_w),          NULL, NULL, NULL },
	{ DIS_74X161X161X32,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dis_74x161x161x32_w),   NULL, NULL, NULL },
	{ DIS_74X161X138,       NES_NOACCESS, NES_WRITEONLY(nes_carts_state::dis_74x161x138_m_w), NES_NOACCESS,    NULL, NULL, NULL },
	{ BANDAI_LZ93,          NES_NOACCESS, NES_WRITEONLY(nes_carts_state::lz93d50_m_w), NES_WRITEONLY(nes_carts_state::lz93d50_w), NULL, NULL, bandai_lz_irq },
	{ BANDAI_LZ93EX,        NES_NOACCESS, NES_WRITEONLY(nes_carts_state::lz93d50_m_w), NES_WRITEONLY(nes_carts_state::lz93d50_w), NULL, NULL, bandai_lz_irq },
	{ BANDAI_FCG,           NES_NOACCESS, NES_WRITEONLY(nes_carts_state::lz93d50_m_w), NES_WRITEONLY(nes_carts_state::lz93d50_w), NULL, NULL, bandai_lz_irq },
	{ BANDAI_DATACH,        NES_NOACCESS, NES_WRITEONLY(nes_carts_state::lz93d50_m_w), NES_WRITEONLY(nes_carts_state::lz93d50_w), NULL, NULL, bandai_lz_irq },
	{ BANDAI_JUMP2,         NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::fjump2_w),              NULL, NULL, bandai_lz_irq },
	{ BANDAI_KARAOKE,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bandai_ks_w),           NULL, NULL, NULL },
	{ BANDAI_OEKAKIDS,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bandai_ok_w),           NULL, NULL, NULL },
	{ IREM_G101,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::g101_w),                NULL, NULL, NULL },
	{ IREM_LROG017,         NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::lrog017_w),             NULL, NULL, NULL },
	{ IREM_H3001,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::h3001_w),               NULL, NULL, h3001_irq },
	{ IREM_TAM_S1,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tam_s1_w),              NULL, NULL, NULL },
	{ IREM_HOLYDIV,         NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::irem_hd_w),             NULL, NULL, NULL },
	{ JALECO_SS88006,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ss88006_w),             NULL, NULL, ss88006_irq },
	{ JALECO_JF11,          NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jf11_m_w), NES_NOACCESS,              NULL, NULL, NULL },
	{ JALECO_JF13,          NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jf13_m_w), NES_NOACCESS,              NULL, NULL, NULL },
	{ JALECO_JF16,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jf16_w),                NULL, NULL, NULL },
	{ JALECO_JF17,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jf17_w),                NULL, NULL, NULL },
	{ JALECO_JF19,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::jf19_w),                NULL, NULL, NULL },
	{ KONAMI_VRC1,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc1_w),         NULL, NULL, NULL },
	{ KONAMI_VRC2,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc2_w),         NULL, NULL, NULL },
	{ KONAMI_VRC3,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc3_w),         NULL, NULL, konami_irq },
	{ KONAMI_VRC4,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc4_w),         NULL, NULL, konami_irq },
	{ KONAMI_VRC6,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc6_w),         NULL, NULL, konami_irq },
	{ KONAMI_VRC7,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::konami_vrc7_w),         NULL, NULL, konami_irq },
	{ NAMCOT_163,           NES_READWRITE(nes_carts_state::namcot163_l_w, nes_carts_state::namcot163_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::namcot163_w), NULL, NULL, namcot_irq },
	{ SUNSOFT_1,            NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sunsoft1_m_w), NES_NOACCESS,          NULL, NULL, NULL },
	{ SUNSOFT_2,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sunsoft2_w),            NULL, NULL, NULL },
	{ SUNSOFT_3,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sunsoft3_w),            NULL, NULL, sunsoft3_irq },
	{ TAITO_TC0190FMC,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tc0190fmc_w),           NULL, NULL, NULL },
	{ TAITO_TC0190FMCP,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tc0190fmc_p16_w),       NULL, NULL, mmc3_irq },
	{ TAITO_X1_005,         NES_NOACCESS, NES_READWRITE(nes_carts_state::x1005_m_w, nes_carts_state::x1005_m_r), NES_NOACCESS,               NULL, NULL, NULL },
	{ TAITO_X1_005_A,       NES_NOACCESS, NES_READWRITE(nes_carts_state::x1005a_m_w, nes_carts_state::x1005_m_r), NES_NOACCESS,              NULL, NULL, NULL },
	{ TAITO_X1_017,         NES_NOACCESS, NES_READWRITE(nes_carts_state::x1017_m_w, nes_carts_state::x1017_m_r), NES_NOACCESS,               NULL, NULL, NULL },
	//
	{ AGCI_50282,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::agci_50282_w),          NULL, NULL, NULL },
	{ ACTENT_ACT52,         NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ae_act52_w),            NULL, NULL, NULL },
	{ AVE_NINA01,           NES_NOACCESS, NES_WRITEONLY(nes_carts_state::nina01_m_w), NES_NOACCESS,            NULL, NULL, NULL },
	{ AVE_NINA06,           NES_WRITEONLY(nes_carts_state::nina06_l_w), NES_NOACCESS, NES_NOACCESS,            NULL, NULL, NULL },
	{ CNE_DECATHLON,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::cne_decathl_w),         NULL, NULL, NULL },
	{ CNE_FSB,              NES_NOACCESS, NES_WRITEONLY(nes_carts_state::cne_fsb_m_w), NES_NOACCESS,           NULL, NULL, NULL },
	{ CNE_SHLZ,             NES_WRITEONLY(nes_carts_state::cne_shlz_l_w), NES_NOACCESS, NES_NOACCESS,          NULL, NULL, NULL },
	{ CALTRON_6IN1,         NES_NOACCESS, NES_WRITEONLY(nes_carts_state::caltron6in1_m_w), NES_WRITEONLY(nes_carts_state::caltron6in1_w),      NULL, NULL, NULL },
	{ CAMERICA_BF9093,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bf9093_w),              NULL, NULL, NULL },
	{ CAMERICA_BF9097,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bf9093_w),              NULL, NULL, NULL },
	{ CAMERICA_BF9096,      NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bf9096_w), NES_WRITEONLY(nes_carts_state::bf9096_w),   NULL, NULL, NULL },
	{ CAMERICA_GOLDENFIVE,  NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::golden5_w),             NULL, NULL, NULL },
	{ CONY_BOARD,           NES_READWRITE(nes_carts_state::cony_l_w, nes_carts_state::cony_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::cony_w),        NULL, NULL, sunsoft3_irq },
	{ YOKO_BOARD,           NES_READWRITE(nes_carts_state::yoko_l_w, nes_carts_state::yoko_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::yoko_w),        NULL, NULL, sunsoft3_irq },
	{ DREAMTECH_BOARD,      NES_WRITEONLY(nes_carts_state::dreamtech_l_w), NES_NOACCESS, NES_NOACCESS,         NULL, NULL, NULL },
	{ FUTUREMEDIA_BOARD,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::futuremedia_w),         NULL, NULL, futuremedia_irq },
	{ FUKUTAKE_BOARD,       NES_READWRITE(nes_carts_state::fukutake_l_w, nes_carts_state::fukutake_l_r), NES_NOACCESS, NES_NOACCESS,         NULL, NULL, NULL },
	{ GOUDER_37017,         NES_READWRITE(nes_carts_state::gouder_sf4_l_w, nes_carts_state::gouder_sf4_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ HENGEDIANZI_BOARD,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::henggedianzi_w),        NULL, NULL, NULL },
	{ HENGEDIANZI_XJZB,     NES_WRITEONLY(nes_carts_state::heng_xjzb_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::heng_xjzb_w), NULL, NULL, NULL },
	{ HES6IN1_BOARD,        NES_WRITEONLY(nes_carts_state::hes6in1_l_w), NES_NOACCESS, NES_NOACCESS,           NULL, NULL, NULL },
	{ HES_BOARD,            NES_WRITEONLY(nes_carts_state::hes_l_w), NES_NOACCESS, NES_NOACCESS,               NULL, NULL, NULL },
	{ HOSENKAN_BOARD,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::hosenkan_w),            NULL, NULL, mmc3_irq },
	{ KAISER_KS7058,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ks7058_w),              NULL, NULL, NULL },
	{ KAISER_KS7022,        NES_NOACCESS, NES_NOACCESS, NES_READWRITE(nes_carts_state::ks7022_w, nes_carts_state::ks7022_r),                 NULL, NULL, NULL },
	{ KAISER_KS7032,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ks7032_w),              NULL, NULL, ks7032_irq },
	{ KAISER_KS202,         NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ks202_w),               NULL, NULL, ks7032_irq },
	{ KAISER_KS7017,        NES_WRITEONLY(nes_carts_state::ks7017_l_w), NES_NOACCESS, NES_NOACCESS,            NULL, NULL, mmc_fds_irq },
	{ KAY_PANDAPRINCE,      NES_READWRITE(nes_carts_state::kay_pp_l_w, nes_carts_state::kay_pp_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::kay_pp_w),  NULL, NULL, mmc3_irq },
	{ KASING_BOARD,         NES_NOACCESS, NES_WRITEONLY(nes_carts_state::kasing_m_w), NES_WRITEONLY(nes_carts_state::txrom_w),  NULL, NULL, mmc3_irq },
	{ SACHEN_74LS374,       NES_READWRITE(nes_carts_state::sachen_74x374_l_w, nes_carts_state::sachen_74x374_l_r), NES_NOACCESS, NES_NOACCESS, NULL, NULL, NULL },
	{ SACHEN_74LS374_A,     NES_WRITEONLY(nes_carts_state::sachen_74x374a_l_w), NES_NOACCESS, NES_NOACCESS,    NULL, NULL, NULL },
	{ SACHEN_8259A,         NES_WRITEONLY(nes_carts_state::s8259_l_w), NES_WRITEONLY(nes_carts_state::s8259_m_w), NES_NOACCESS, NULL, NULL, NULL },
	{ SACHEN_8259B,         NES_WRITEONLY(nes_carts_state::s8259_l_w), NES_WRITEONLY(nes_carts_state::s8259_m_w), NES_NOACCESS, NULL, NULL, NULL },
	{ SACHEN_8259C,         NES_WRITEONLY(nes_carts_state::s8259_l_w), NES_WRITEONLY(nes_carts_state::s8259_m_w), NES_NOACCESS, NULL, NULL, NULL },
	{ SACHEN_8259D,         NES_WRITEONLY(nes_carts_state::s8259_l_w), NES_WRITEONLY(nes_carts_state::s8259_m_w), NES_NOACCESS, NULL, NULL, NULL },
	{ SACHEN_SA009,         NES_WRITEONLY(nes_carts_state::sa009_l_w), NES_NOACCESS, NES_NOACCESS,             NULL, NULL, NULL },
	{ SACHEN_SA0036,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sa0036_w),              NULL, NULL, NULL },
	{ SACHEN_SA0037,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sa0037_w),              NULL, NULL, NULL },
	{ SACHEN_SA72007,       NES_WRITEONLY(nes_carts_state::sa72007_l_w), NES_NOACCESS, NES_NOACCESS,           NULL, NULL, NULL },
	{ SACHEN_SA72008,       NES_WRITEONLY(nes_carts_state::sa72008_l_w), NES_NOACCESS, NES_NOACCESS,           NULL, NULL, NULL },
	{ SACHEN_TCA01,         NES_READONLY(nes_carts_state::tca01_l_r), NES_NOACCESS, NES_NOACCESS,              NULL, NULL, NULL },
	{ SACHEN_TCU01,         NES_WRITEONLY(nes_carts_state::tcu01_l_w), NES_WRITEONLY(nes_carts_state::tcu01_m_w), NES_WRITEONLY(nes_carts_state::tcu01_w), NULL, NULL, NULL },
	{ SACHEN_TCU02,         NES_READWRITE(nes_carts_state::tcu02_l_w, nes_carts_state::tcu02_l_r), NES_NOACCESS, NES_NOACCESS,               NULL, NULL, NULL },
	{ SUBOR_TYPE0,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::subor0_w),              NULL, NULL, NULL },
	{ SUBOR_TYPE1,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::subor1_w),              NULL, NULL, NULL },
	{ MAGICSERIES_MD,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::magics_md_w),           NULL, NULL, NULL },
	{ NANJING_BOARD,        NES_READWRITE(nes_carts_state::nanjing_l_w, nes_carts_state::nanjing_l_r), NES_NOACCESS, NES_NOACCESS,           NULL, NULL, nanjing_irq },
	{ NITRA_TDA,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::nitra_w),               NULL, NULL, mmc3_irq },
	{ NTDEC_ASDER,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ntdec_asder_w),         NULL, NULL, NULL },
	{ NTDEC_FIGHTINGHERO,   NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ntdec_fh_m_w), NES_NOACCESS,          NULL, NULL, NULL },
	{ OPENCORP_DAOU306,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::daou306_w),             NULL, NULL, NULL },
	{ RCM_GS2015,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::gs2015_w),              NULL, NULL, NULL },
	{ RCM_TETRISFAMILY,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::rcm_tf_w),              NULL, NULL, NULL },
	{ REXSOFT_DBZ5,         NES_READWRITE(nes_carts_state::rex_dbz_l_w, nes_carts_state::rex_dbz_l_r), NES_READONLY(nes_carts_state::rex_dbz_l_r), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ REXSOFT_SL1632,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::rex_sl1632_w),          NULL, NULL, mmc3_irq },
	{ RUMBLESTATION_BOARD,  NES_NOACCESS, NES_WRITEONLY(nes_carts_state::rumblestation_m_w), NES_WRITEONLY(nes_carts_state::rumblestation_w),      NULL, NULL, NULL },
	{ SOMERI_SL12,          NES_WRITEONLY(nes_carts_state::someri_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::someri_w), NULL, NULL, mmc3_irq },
	{ SUPERGAME_BOOGERMAN,  NES_WRITEONLY(nes_carts_state::sgame_boog_l_w), NES_WRITEONLY(nes_carts_state::sgame_boog_m_w), NES_WRITEONLY(nes_carts_state::sgame_boog_w), NULL, NULL, mmc3_irq },
	{ SUPERGAME_LIONKING,   NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sgame_lion_m_w), NES_WRITEONLY(nes_carts_state::sgame_lion_w), NULL, NULL, mmc3_irq },
	{ TENGEN_800008,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tengen_800008_w),       NULL, NULL, NULL },
	{ TENGEN_800032,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tengen_800032_w),       NULL, NULL, tengen_800032_irq },
	{ TENGEN_800037,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::tengen_800037_w),       NULL, NULL, tengen_800032_irq },
	{ TXC_22211A,           NES_READWRITE(nes_carts_state::txc_22211_l_w, nes_carts_state::txc_22211_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txc_22211_w), NULL, NULL, NULL },
	{ TXC_22211B,           NES_READWRITE(nes_carts_state::txc_22211_l_w, nes_carts_state::txc_22211_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txc_22211b_w), NULL, NULL, NULL },
	{ TXC_22211C,           NES_READWRITE(nes_carts_state::txc_22211_l_w, nes_carts_state::txc_22211c_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txc_22211_w), NULL, NULL, NULL },
	{ TXC_TW,               NES_WRITEONLY(nes_carts_state::txc_tw_l_w), NES_WRITEONLY(nes_carts_state::txc_tw_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ TXC_STRIKEWOLF,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txc_strikewolf_w),      NULL, NULL, NULL },
	{ TXC_MXMDHTWO,         NES_READONLY(nes_carts_state::txc_mxmdhtwo_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txc_mxmdhtwo_w), NULL, NULL, NULL },
	{ WAIXING_TYPE_A,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_A_1,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_B,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_C,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_D,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_E,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_a_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_F,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_f_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_G,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_g_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_H,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_h_w),           NULL, NULL, mmc3_irq },
	{ WAIXING_TYPE_I,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },  // this is MMC3 + possibly additional WRAM added in 0x5000-0x5fff
	{ WAIXING_TYPE_J,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },  // this is MMC3 + possibly additional WRAM added in 0x5000-0x5fff
	{ WAIXING_SGZ,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_sgz_w),         NULL, NULL, konami_irq },
	{ WAIXING_SGZLZ,        NES_WRITEONLY(nes_carts_state::waixing_sgzlz_l_w), NES_NOACCESS, NES_NOACCESS,     NULL, NULL, NULL },
	{ WAIXING_FFV,          NES_WRITEONLY(nes_carts_state::waixing_ffv_l_w), NES_NOACCESS, NES_NOACCESS,       NULL, NULL, NULL },
	{ WAIXING_ZS,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_zs_w),          NULL, NULL, NULL },
	{ WAIXING_DQ8,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_dq8_w),         NULL, NULL, NULL },
	{ WAIXING_SECURITY,     NES_WRITEONLY(nes_carts_state::waixing_sec_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ WAIXING_SH2,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w),               NULL, NULL, mmc3_irq },  // this is MMC3 + possibly additional WRAM added in 0x5000-0x5fff
	{ WAIXING_PS2,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::waixing_ps2_w),         NULL, NULL, NULL },
	{ UNL_8237,             NES_WRITEONLY(nes_carts_state::unl_8237_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_8237_w),      NULL, NULL, mmc3_irq },
	{ UNL_AX5705,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_ax5705_w),          NULL, NULL, NULL },
	{ UNL_CC21,             NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_cc21_w),            NULL, NULL, NULL },
	{ UNL_KOF97,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_kof97_w),           NULL, NULL, mmc3_irq },
	{ UNL_KS7057,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::ks7057_w),              NULL, NULL, mmc3_irq },
	{ UNL_T230,             NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_t230_w),            NULL, NULL, konami_irq },
	{ UNL_KOF96,            NES_READWRITE(nes_carts_state::kof96_l_w, nes_carts_state::kof96_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::kof96_w),     NULL, NULL, mmc3_irq },
	{ UNL_MK2,              NES_NOACCESS, NES_WRITEONLY(nes_carts_state::mk2_m_w), NES_NOACCESS,               NULL, NULL, mmc3_irq },
	{ UNL_N625092,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::n625092_w),             NULL, NULL, NULL },
	{ UNL_SC127,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sc127_w),               NULL, NULL, sc127_irq },
	{ UNL_SMB2J,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::smb2j_w),               NULL, NULL, NULL },
	{ UNL_SUPERFIGHTER3,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_sf3_w),             NULL, NULL, mmc3_irq },
	{ UNL_XZY,              NES_WRITEONLY(nes_carts_state::unl_xzy_l_w), NES_NOACCESS, NES_NOACCESS,           NULL, NULL, NULL },
	{ UNL_RACERMATE,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::unl_racmate_w),         NULL, NULL, NULL },
	{ UNL_STUDYNGAME,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::sng32_w),               NULL, NULL, NULL },
	{ UNL_603_5052,         NES_READWRITE(nes_carts_state::unl_6035052_extra_w, nes_carts_state::unl_6035052_extra_r), NES_READWRITE(nes_carts_state::unl_6035052_extra_w, nes_carts_state::unl_6035052_extra_r), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ UNL_EDU2K,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::edu2k_w),               NULL, NULL, NULL },
	{ UNL_SHJY3,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::shjy3_w),               NULL, NULL, shjy3_irq },
	{ UNL_H2288,            NES_READWRITE(nes_carts_state::h2288_l_w, nes_carts_state::h2288_l_r), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::h2288_w),     NULL, NULL, mmc3_irq },
	{ UNL_FS304,            NES_WRITEONLY(nes_carts_state::unl_fs304_l_w), NES_NOACCESS, NES_NOACCESS,         NULL, NULL, NULL },
	//
	{ BTL_AISENSHINICOL,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_mariobaby_w),       NULL, NULL, NULL },
	{ BTL_DRAGONNINJA,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_dn_w),              NULL, NULL, btl_dn_irq },
	{ BTL_MARIOBABY,        NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_mariobaby_w),       NULL, NULL, NULL },
	{ BTL_SMB2A,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_smb2a_w),           NULL, NULL, btl_smb2a_irq },
	{ BTL_SMB2B,            NES_WRITEONLY(nes_carts_state::smb2jb_l_w), NES_NOACCESS, NES_NOACCESS,            NULL, NULL, smb2jb_irq },
	{ BTL_SMB3,             NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_smb3_w),            NULL, NULL, btl_smb3_irq },
	{ BTL_SUPERBROS11,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::btl_smb11_w),           NULL, NULL, mmc3_irq },
	{ BTL_TOBIDASE,         NES_WRITEONLY(nes_carts_state::btl_tobi_l_w), NES_NOACCESS, NES_NOACCESS,          NULL, NULL, NULL },
	{ BTL_PIKACHUY2K,       NES_NOACCESS, NES_READWRITE(nes_carts_state::btl_pika_y2k_m_w, nes_carts_state::btl_pika_y2k_m_r), NES_WRITEONLY(nes_carts_state::btl_pika_y2k_w),  NULL, NULL, mmc3_irq },
	{ WHIRLWIND_2706,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::whirl2706_w),           NULL, NULL, NULL },
	//
	{ BMC_190IN1,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_190in1_w),          NULL, NULL, NULL },
	{ BMC_A65AS,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_a65as_w),           NULL, NULL, NULL },
	{ BMC_GS2004,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gs2004_w),          NULL, NULL, NULL },
	{ BMC_GS2013,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gs2013_w),          NULL, NULL, NULL },
	{ BMC_NOVELDIAMOND,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::novel1_w),              NULL, NULL, NULL },
	{ BMC_9999999IN1,       NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::novel2_w),              NULL, NULL, NULL },
	{ BMC_T262,             NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_t262_w),            NULL, NULL, NULL },
	{ BMC_WS,               NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_ws_m_w), NES_NOACCESS,            NULL, NULL, NULL },
	{ BMC_GKA,              NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gka_w),             NULL, NULL, NULL },
	{ BMC_GKB,              NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gkb_w),             NULL, NULL, NULL },
	{ BMC_SUPER_700IN1,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_super700in1_w),     NULL, NULL, NULL },
	{ BMC_36IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_36in1_w),           NULL, NULL, NULL },
	{ BMC_21IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_21in1_w),           NULL, NULL, NULL },
	{ BMC_150IN1,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_150in1_w),          NULL, NULL, NULL },
	{ BMC_35IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_35in1_w),           NULL, NULL, NULL },
	{ BMC_64IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_64in1_w),           NULL, NULL, NULL },
	{ BMC_SUPERHIK_300IN1,  NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_hik300_w),          NULL, NULL, NULL },
	{ BMC_SUPERGUN_20IN1,   NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::supergun20in1_w),       NULL, NULL, NULL },
	{ BMC_72IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_72in1_w),           NULL, NULL, NULL },
	{ BMC_76IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_76in1_w),           NULL, NULL, NULL },
	{ BMC_SUPER_42IN1,      NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_76in1_w),           NULL, NULL, NULL },
	{ BMC_1200IN1,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_1200in1_w),         NULL, NULL, NULL },
	{ BMC_31IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_31in1_w),           NULL, NULL, NULL },
	{ BMC_22GAMES,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_22g_w),             NULL, NULL, NULL },
	{ BMC_20IN1,            NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_20in1_w),           NULL, NULL, NULL },
	{ BMC_110IN1,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_110in1_w),          NULL, NULL, NULL },
	{ BMC_64IN1NR,          NES_WRITEONLY(nes_carts_state::bmc_64in1nr_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_64in1nr_w), NULL, NULL, NULL },
	{ BMC_S24IN1SC03,       NES_WRITEONLY(nes_carts_state::bmc_s24in1sc03_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_HIK8IN1,          NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_hik8_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_SUPERHIK_4IN1,    NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_hik4in1_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_SUPERBIG_7IN1,    NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_sbig7_w),           NULL, NULL, mmc3_irq },
	{ BMC_MARIOPARTY_7IN1,  NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_mario7in1_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_GOLD_7IN1,        NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gold7in1_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_FAMILY_4646B,     NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_family4646_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_15IN1,            NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_15in1_m_w), NES_WRITEONLY(nes_carts_state::txrom_w), NULL, NULL, mmc3_irq },
	{ BMC_BALLGAMES_11IN1,  NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_ball11_m_w), NES_WRITEONLY(nes_carts_state::bmc_ball11_w), NULL, NULL, NULL },
	{ BMC_GOLDENCARD_6IN1,  NES_WRITEONLY(nes_carts_state::bmc_gc6in1_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_gc6in1_w), NULL, NULL, mmc3_irq },
	{ BMC_VT5201,           NES_NOACCESS, NES_NOACCESS, NES_READWRITE(nes_carts_state::bmc_vt5201_w, nes_carts_state::bmc_vt5201_r),         NULL, NULL, NULL },
	{ BMC_BENSHENG_BS5,     NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_bs5_w),             NULL, NULL, NULL },
	{ BMC_810544,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_810544_w),          NULL, NULL, NULL },
	{ BMC_NTD_03,           NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::bmc_ntd03_w),           NULL, NULL, NULL },
	{ BMC_G63IN1,           NES_NOACCESS, NES_NOACCESS, NES_READWRITE(nes_carts_state::bmc_gb63_w, nes_carts_state::bmc_gb63_r),             NULL, NULL, NULL },
	{ BMC_FK23C,            NES_WRITEONLY(nes_carts_state::fk23c_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::fk23c_w),   NULL, NULL, mmc3_irq },
	{ BMC_FK23CA,           NES_WRITEONLY(nes_carts_state::fk23c_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::fk23c_w),   NULL, NULL, mmc3_irq },
	{ BMC_PJOY84,           NES_NOACCESS, NES_WRITEONLY(nes_carts_state::pjoy84_m_w), NES_WRITEONLY(nes_carts_state::txrom_w),  NULL, NULL, mmc3_irq },
	//
	{ FFE_MAPPER6,          NES_WRITEONLY(nes_carts_state::mapper6_l_w), NES_NOACCESS, NES_WRITEONLY(nes_carts_state::mapper6_w), NULL, NULL, ffe_irq },
	{ FFE_MAPPER8,          NES_NOACCESS, NES_NOACCESS, NES_WRITEONLY(nes_carts_state::mapper8_w),             NULL, NULL, NULL },
	{ FFE_MAPPER17,         NES_WRITEONLY(nes_carts_state::mapper17_l_w), NES_NOACCESS, NES_NOACCESS,          NULL, NULL, ffe_irq },
	// for debug and development
	{ UNKNOWN_BOARD,        NES_READWRITE(nes_carts_state::dummy_l_w, nes_carts_state::dummy_l_r), NES_READWRITE(nes_carts_state::dummy_m_w, nes_carts_state::dummy_m_r), NES_READWRITE(nes_carts_state::dummy_w, nes_carts_state::dummy_r), NULL, NULL, NULL },
	//
	{ UNSUPPORTED_BOARD,    NES_NOACCESS, NES_NOACCESS, NES_NOACCESS,                         NULL, NULL, NULL },
	//
};

const nes_pcb_intf *nes_pcb_intf_lookup( int pcb_id )
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(nes_intf_list); i++)
	{
		if (nes_intf_list[i].mmc_pcb == pcb_id)
			return &nes_intf_list[i];
	}
	return NULL;
}

void nes_state::pcb_handlers_setup()
{
	const nes_pcb_intf *intf = nes_pcb_intf_lookup(m_pcb_id);

	if (intf == NULL)
		fatalerror("Missing PCB interface\n");

	if (intf)
	{
		m_mmc_write_low = intf->mmc_l.write;
		if (!m_mmc_write_low.isnull()) m_mmc_write_low.late_bind(*this);
		m_mmc_write_mid = intf->mmc_m.write;
		if (!m_mmc_write_mid.isnull()) m_mmc_write_mid.late_bind(*this);
		m_mmc_write = intf->mmc_h.write;
		if (!m_mmc_write.isnull()) m_mmc_write.late_bind(*this);
		m_mmc_read_low = intf->mmc_l.read;
		if (!m_mmc_read_low.isnull()) m_mmc_read_low.late_bind(*this);
		m_mmc_read_mid = intf->mmc_m.read;  // in progress
		if (!m_mmc_read_mid.isnull()) m_mmc_read_mid.late_bind(*this);
		m_mmc_read = intf->mmc_h.read;  // in progress
		if (!m_mmc_read.isnull()) m_mmc_read.late_bind(*this);
		m_ppu->set_latch(intf->mmc_ppu_latch);
	}
	else
	{
		logerror("PCB %d is not yet supported, defaulting to no mapper.\n", m_pcb_id);
		m_mmc_write_low = write8_delegate();
		m_mmc_write_mid = write8_delegate();
		m_mmc_write = write8_delegate();
		m_mmc_read_low = read8_delegate();
		m_mmc_read_mid = read8_delegate();  // in progress
		m_mmc_read = read8_delegate();  // in progress
		m_ppu->set_latch(NULL);
	}

	m_mmc3_prg_cb = mmc3_base_prg_cb;
	m_mmc3_chr_cb = mmc3_base_chr_cb;

	switch (m_pcb_id)
	{
		case STD_TXSROM:
			m_mmc3_chr_cb = txsrom_chr_cb;
			break;
		case GOUDER_37017:
			m_mmc3_prg_cb = gouder_sf4_prg_cb;
			break;
		case KASING_BOARD:
			m_mmc3_prg_cb = kasing_prg_cb;
			break;
		case REXSOFT_DBZ5:
			m_mmc3_chr_cb = rex_dbz_chr_cb;
			break;
		case TXC_TW:
			m_mmc3_prg_cb = txc_tw_prg_cb;
			break;
		case WAIXING_TYPE_A:
			m_mmc3_chr_cb = waixing_a_chr_cb;
			break;
		case WAIXING_TYPE_A_1:
			m_mmc3_chr_cb = waixing_a1_chr_cb;
			break;
		case WAIXING_TYPE_B:
			m_mmc3_chr_cb = waixing_b_chr_cb;
			break;
		case WAIXING_TYPE_C:
			m_mmc3_chr_cb = waixing_c_chr_cb;
			break;
		case WAIXING_TYPE_D:
			m_mmc3_chr_cb = waixing_d_chr_cb;
			break;
		case WAIXING_TYPE_E:
			m_mmc3_chr_cb = waixing_e_chr_cb;
			break;
		case WAIXING_TYPE_G:
			m_mmc3_chr_cb = waixing_g_chr_cb;
			break;
		case WAIXING_TYPE_H:
			m_mmc3_chr_cb = waixing_h_chr_cb;
			break;
		case WAIXING_SECURITY:
			m_mmc3_prg_cb = waixing_sec_prg_cb;
			m_mmc3_chr_cb = waixing_sec_chr_cb;
			break;
		case WAIXING_SH2:
			m_mmc3_chr_cb = waixing_sh2_chr_cb;
			break;
		case UNL_8237:
			m_mmc3_prg_cb = unl_8237_prg_cb;
			m_mmc3_chr_cb = unl_8237_chr_cb;
			break;
		case UNL_H2288:
			m_mmc3_prg_cb = h2288_prg_cb;
			break;
		case SUPERGAME_BOOGERMAN:
			m_mmc3_prg_cb = sgame_boog_prg_cb;
			m_mmc3_chr_cb = sgame_boog_chr_cb;
			break;
		case UNL_KOF96:
			m_mmc3_prg_cb = kof96_prg_cb;
			m_mmc3_chr_cb = kof96_chr_cb;
			break;
		case KAY_PANDAPRINCE:
			m_mmc3_prg_cb = kay_pp_prg_cb;
			m_mmc3_chr_cb = kay_pp_chr_cb;
			break;
		case BMC_FK23C:
		case BMC_FK23CA:
			m_mmc3_prg_cb = fk23c_prg_cb;
			m_mmc3_chr_cb = fk23c_chr_cb;
			break;
		case BMC_S24IN1SC03:
			m_mmc3_prg_cb = bmc_s24in1sc03_prg_cb;
			m_mmc3_chr_cb = bmc_s24in1sc03_chr_cb;
			break;
		case BMC_PJOY84:
			m_mmc3_prg_cb = pjoy84_prg_cb;
			m_mmc3_chr_cb = pjoy84_chr_cb;
			break;
	}
}

/*************************************************************

 pcb_initialize

 Initialize all the necessary registers and quantities for
 emulation of each board

 *************************************************************/

/* this is used by many boards (MMC3, MMC6 + most MMC3 pirate clone boards) */
static void mmc3_common_initialize( running_machine &machine, int prg_mask, int chr_mask, int IRQ_type )
{
	nes_state *state = machine.driver_data<nes_state>();

	state->m_mmc3_alt_irq = IRQ_type;       // later MMC3 boards seem to use MMC6-type IRQ... more investigations are in progress at NESDev...
	state->m_mmc_prg_bank[0] = state->m_mmc_prg_bank[2] = 0xfe; // prg_bank[2] & prg_bank[3] remain always the same in most MMC3 variants
	state->m_mmc_prg_bank[1] = state->m_mmc_prg_bank[3] = 0xff; // but some pirate clone mappers change them after writing certain registers
	state->m_mmc3_latch = 0;
	state->m_mmc3_wram_protect = 0x80;
	state->m_mmc_prg_base = state->m_mmc_chr_base = 0;
	state->m_mmc_prg_mask = prg_mask;
	state->m_mmc_chr_mask = chr_mask;
	mmc3_set_prg(machine, state->m_mmc_prg_base, state->m_mmc_prg_mask);
	mmc3_set_chr(machine, state->m_mmc_chr_source, state->m_mmc_chr_base, state->m_mmc_chr_mask);
}

// WIP code
int nes_state::pcb_initialize( int idx )
{
	int err = 0, i;

	/* basic PRG config */
	prg32(0);

	/* some boards will not use this, but directly CHRROM (resp. CHRRAM) if the board only has VROM (resp. VRAM) */
	m_mmc_chr_source = m_chr_chunks ? CHRROM : CHRRAM;
	chr8(0, m_mmc_chr_source);

	/* Here, we init a few helpers: 4 prg banks and 16 chr banks - some mappers use them */
	for (i = 0; i < 4; i++)
		m_mmc_prg_bank[i] = 0;
	for (i = 0; i < 16; i++)
		m_mmc_vrom_bank[i] = 0;
	for (i = 0; i < 16; i++)
		m_mmc_extra_bank[i] = 0;

	m_mmc_latch1 = 0;
	m_mmc_latch2 = 0;

	/* Finally, we init IRQ-related quantities. */
	m_IRQ_enable = m_IRQ_enable_latch = 0;
	m_IRQ_count = m_IRQ_count_latch = 0;
	m_IRQ_toggle = 0;

	switch (idx)
	{
			/* many boards only needs PRG to point at first 32k and CHR at first 8k */
		case STD_NROM:  // mapper 0
		case HVC_FAMBASIC:
		case GG_NROM:
		case UXROM_CC:  // mapper 180
		case STD_CNROM: // mapper 3, 185
		case BANDAI_PT554:
		case TENGEN_800008:
		case STD_BXROM: // mapper 34
		case STD_GXROM: // mapper 66
		case STD_MXROM:
		case BANDAI_OEKAKIDS:   // mapper 96
		case JALECO_JF11:   // mapper 140
		case JALECO_JF13:   // mapper 86
		case JALECO_JF19:   // mapper 92
		case DIS_74X377:    // mapper 11
		case DIS_74X161X138:    // mapper 38
		case CALTRON_6IN1:  // mapper 41
		case TXC_STRIKEWOLF:    // mapper 36
		case UNL_STUDYNGAME:    // mapper 39
		case RUMBLESTATION_BOARD:   // mapper 46
		case BMC_GKB:   // mapper 58
		case RCM_TETRISFAMILY:  // mapper 61
		case BMC_SUPER_700IN1:  // mapper 62
		case MAGICSERIES_MD:    // mapper 107
		case HES_BOARD: // mapper 113
		case HES6IN1_BOARD: // mapper 113
		case SACHEN_SA72008:    // mapper 133
		case SACHEN_TCU02:  // mapper 136
		case SACHEN_SA72007:    // mapper 145
		case SACHEN_TCU01:  // mapper 147
		case SACHEN_SA009:  // mapper 160
		case SACHEN_SA0037: // mapper 148
		case SACHEN_SA0036: // mapper 149
		case AGCI_50282:    // mapper 144
		case KAISER_KS7058: // mapper 171
		case HENGEDIANZI_BOARD: // mapper 177
		case WAIXING_SGZLZ: // mapper 178
		case HENGEDIANZI_XJZB:  // mapper 179
		case BMC_21IN1: // mapper 201
		case BMC_9999999IN1:    // mapper 213
		case RCM_GS2015:    // mapper 216
		case BMC_72IN1: // mapper 225
		case BMC_76IN1: // mapper 226
		case BMC_SUPER_42IN1:   // mapper 226
		case ACTENT_ACT52:  // mapper 228
		case CNE_SHLZ:  // mapper 240
		case TXC_MXMDHTWO:  // mapper 241
		case WAIXING_ZS:    // mapper 242
		case WAIXING_DQ8:   // mapper 242
		case CNE_DECATHLON: // mapper 244
		case UNL_CC21:
		case BMC_VT5201:
		case BMC_WS:
		case UNL_EDU2K:
			break;

			/* other boards need additional initialization */
		case STD_UXROM: // mapper 2
		case STD_UN1ROM:    // mapper 94
		case STD_JXROM:     // mapper 69
		case SUNSOFT_5B:
		case SUNSOFT_FME7:
		case NAMCOT_3425:   // mapper 95
		case DIS_74X139X74: // mapper 87
		case DIS_74X161X161X32: // mapper 152 & 70
		case BANDAI_LZ93:   // mapper 16, 157
		case BANDAI_LZ93EX: // same + EEPROM
		case BANDAI_DATACH:
		case BANDAI_FCG:
		case IREM_HOLYDIV:  // mapper 78
		case IREM_G101: // mapper 32
		case IREM_H3001:    // mapper 65
		case JALECO_SS88006:    // mapper 18
		case JALECO_JF16:   // mapper 78
		case JALECO_JF17:   // mapper 72
		case KONAMI_VRC1:   // mapper 75
		case KONAMI_VRC2:
		case KONAMI_VRC3:   // mapper 73
		case KONAMI_VRC4:
		case KONAMI_VRC6:
		case SUNSOFT_3: // mapper 67
		case TAITO_TC0190FMC:   // mapper 33
		case TAITO_TC0190FMCP:  // mapper 48
		case TAITO_X1_005:  // mapper 80
		case TAITO_X1_005_A:    // mapper 207
		case TAITO_X1_017:  // mapper 82
		case NTDEC_ASDER:   // mapper 112
		case FUTUREMEDIA_BOARD: // mapper 117
		case BTL_DRAGONNINJA:   // mapper 222
		case WAIXING_SGZ:   // mapper 252
		case UNL_T230:
		case BMC_NTD_03:
		case KAISER_KS7022:
		case UNL_SHJY3:
		case KAISER_KS7017:
		case KAISER_KS7032:
		case KAISER_KS202:
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			break;

		case STD_CPROM: // mapper 13
			chr4_0(0, CHRRAM);
			chr4_4(0, CHRRAM);
			break;
		case STD_AXROM: // mapper 7
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
		case STD_SXROM: // mapper 1, 155
		case STD_SOROM:
		case STD_SXROM_A:
		case STD_SOROM_A:
			m_mmc1_latch = 0;
			m_mmc1_count = 0;
			m_mmc_reg[0] = 0x0f;
			m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = 0;
			m_mmc1_reg_write_enable = 1;
			set_nt_mirroring(PPU_MIRROR_HORZ);
			mmc1_set_chr(machine());
			mmc1_set_prg(machine());
			if (m_battery || m_wram)
				wram_bank(0, (idx == STD_SOROM) ? NES_WRAM : NES_BATTERY);
			break;
		case STD_PXROM: // mapper 9
			m_mmc_reg[0] = m_mmc_reg[2] = 0;
			m_mmc_reg[1] = m_mmc_reg[3] = 0;
			m_mmc_latch1 = m_mmc_latch2 = 0xfe;
			prg8_89(0);
			prg8_ab((m_prg_chunks << 1) - 3);
			prg8_cd((m_prg_chunks << 1) - 2);
			prg8_ef((m_prg_chunks << 1) - 1);
			break;
		case STD_FXROM: // mapper 10
			m_mmc_reg[0] = m_mmc_reg[2] = 0;
			m_mmc_reg[1] = m_mmc_reg[3] = 0;
			m_mmc_latch1 = m_mmc_latch2 = 0xfe;
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			break;
		case STD_TXROM: // mapper 4
		case STD_TVROM:
		case STD_TKROM:
		case REXSOFT_DBZ5:  // mapper 12
		case WAIXING_TYPE_A:    // mapper 74
		case WAIXING_TYPE_A_1:
		case STD_TXSROM:    // mapper 118
		case STD_TQROM: // mapper 119
		case WAIXING_SH2:   // mapper 165
		case WAIXING_TYPE_B:    // mapper 191
		case WAIXING_TYPE_C:    // mapper 192
		case WAIXING_TYPE_D:    // mapper 194
		case WAIXING_TYPE_E:    // mapper 195
		case WAIXING_TYPE_H:    // mapper 245
		case BTL_SUPERBROS11:   // mapper 196
		case UNL_KS7057:        // mapper 196 alt (for Street Fighter VI / Fight Street VI)
		case UNL_H2288:     // mapper 123
		case UNL_KOF97:
		case UNL_603_5052:
		case NITRA_TDA: // mapper 250
			if (m_four_screen_vram) // only TXROM and DXROM have 4-screen mirroring
			{
				set_nt_page(0, CART_NTRAM, 0, 1);
				set_nt_page(1, CART_NTRAM, 1, 1);
				set_nt_page(2, CART_NTRAM, 2, 1);
				set_nt_page(3, CART_NTRAM, 3, 1);
			}
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
		case STD_HKROM: // MMC6 (basically the same as TxROM, but alt IRQ behaviour)
			mmc3_common_initialize(machine(), 0xff, 0xff, 1);
			m_mmc6_reg = 0xf0;
			m_mmc_latch2 = 0;   // this is used differently here compared to MMC3
			break;
		case PAL_ZZ:    // mapper 37
			mmc3_common_initialize(machine(), 0x07, 0x7f, 0);
			break;
		case NES_QJ:    // mapper 47
			mmc3_common_initialize(machine(), 0x0f, 0x7f, 0);
			break;
		case STD_EXROM: // mapper 5
			m_MMC5_rom_bank_mode = 3;
			m_MMC5_vrom_bank_mode = 0;
			m_MMC5_vram_protect = 0;
			m_mmc5_high_chr = 0;
			m_mmc5_vram_control = 0;
			m_mmc5_split_scr = 0;
			memset(m_MMC5_vrom_bank, 0, ARRAY_LENGTH(m_MMC5_vrom_bank));
			m_mmc5_prg_mode = 3;
			m_mmc5_last_chr_a = 1;
			m_mmc5_prg_regs[0] = 0xfc;
			m_mmc5_prg_regs[1] = 0xfd;
			m_mmc5_prg_regs[2] = 0xfe;
			m_mmc5_prg_regs[3] = 0xff;
			memset(m_mmc5_vrom_regA, ~0, ARRAY_LENGTH(m_mmc5_vrom_regA));
			memset(m_mmc5_vrom_regB, ~0, ARRAY_LENGTH(m_mmc5_vrom_regB));
			prg16_89ab(m_prg_chunks - 2);
			prg16_cdef(m_prg_chunks - 1);
			break;
		case STD_NXROM:     // mapper 68
		case SUNSOFT_DCS:       // mapper 68
			m_mmc_reg[0] = 0;
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			break;
		case NAMCOT_34X3:   // mapper 88
		case STD_DXROM: // mapper 206
		case STD_DRROM:
			if (m_four_screen_vram) // only TXROM and DXROM have 4-screen mirroring
			{
				set_nt_page(0, CART_NTRAM, 0, 1);
				set_nt_page(1, CART_NTRAM, 1, 1);
				set_nt_page(2, CART_NTRAM, 2, 1);
				set_nt_page(3, CART_NTRAM, 3, 1);
			}
		case NAMCOT_3453:   // mapper 154
			prg16_89ab(m_prg_chunks - 2);
			prg16_cdef(m_prg_chunks - 1);
			break;
		case NAMCOT_3446:   // mapper 76
			prg8_89(0);
			prg8_ab(1);
			prg16_cdef(m_prg_chunks - 1);
			chr2_0(0, CHRROM);
			chr2_2(1, CHRROM);
			chr2_4(2, CHRROM);
			chr2_6(3, CHRROM);
			break;
		case BANDAI_JUMP2:  // mapper 153
			for (i = 0; i < 8; i++)
				m_mmc_reg[i] = 0;
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			fjump2_set_prg(machine());
			break;
		case BANDAI_KARAOKE:    // mapper 188
			prg16_89ab(0);
			prg16_cdef((m_prg_chunks - 1) ^ 0x08);
			break;
		case IREM_LROG017:  // mapper 77
			chr2_2(0, CHRROM);
			chr2_4(1, CHRROM);
			chr2_6(2, CHRROM);
			break;
		case IREM_TAM_S1:   // mapper 97
			prg16_89ab(m_prg_chunks - 1);
			prg16_cdef(0);
			break;
		case KONAMI_VRC7:   // mapper 85
			prg8_89(0);
			prg8_ab(0);
			prg8_cd(0);
			prg8_ef(0xff);
			break;
		case NAMCOT_163:    // mapper 19
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case SUNSOFT_1: // mapper 184
		case SUNSOFT_2: // mapper 89 & 93
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			if (!m_hard_mirroring)
				set_nt_mirroring(PPU_MIRROR_LOW);
			break;

			// mapper 14
		case REXSOFT_SL1632:
			m_mmc_extra_bank[2] = 0xfe;
			m_mmc_extra_bank[3] = 0xff;
			m_mmc_extra_bank[0] = m_mmc_extra_bank[1] = m_mmc_extra_bank[4] = m_mmc_extra_bank[5] = m_mmc_extra_bank[6] = 0;
			m_mmc_extra_bank[7] = m_mmc_extra_bank[8] = m_mmc_extra_bank[9] = m_mmc_extra_bank[0xa] = m_mmc_extra_bank[0xb] = 0;
			m_mmc_reg[0] = m_mmc_reg[1] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
			// mapper 15
		case WAIXING_PS2:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;

			// mapper 35
		case UNL_SC127:
			// mapper 42
		case BTL_MARIOBABY:
		case BTL_AISENSHINICOL:
			prg32(0xff);
			break;

			// mapper 40
		case BTL_SMB2A:
			prg8_67(0xfe);
			prg8_89(0xfc);
			prg8_ab(0xfd);
			prg8_cd(0xfe);
			prg8_ef(0xff);
			break;

			// mapper 43
		case UNL_SMB2J:
			if (m_battery)
				memset(m_battery_ram, 0x2000, 0xff);
			else if (m_prg_ram)
				memset(m_wram, 0x2000, 0xff);
			break;
			// mapper 44
		case BMC_SUPERBIG_7IN1:
			// mapper 49
		case BMC_SUPERHIK_4IN1:
			mmc3_common_initialize(machine(), 0x0f, 0x7f, 0);
			break;
			// mapper 45
		case BMC_HIK8IN1:
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = 0;
			mmc3_common_initialize(machine(), 0x3f, 0xff, 0);
			break;

			// mapper 50
		case BTL_SMB2B:
			prg8_67(0x0f);
			prg8_89(0x08);
			prg8_ab(0x09);
			prg8_cd(0);
			prg8_ef(0x0b);
			break;
			// mapper 51
		case BMC_BALLGAMES_11IN1:
			m_mmc_reg[0] = 0x01;
			m_mmc_reg[1] = 0x00;
			bmc_ball11_set_banks(machine());
			break;
			// mapper 52
		case BMC_MARIOPARTY_7IN1:
		case BMC_GOLD_7IN1:
			m_map52_reg_written = 0;
			mmc3_common_initialize(machine(), 0x1f, 0xff, 0);
			break;
			// mapper 54
		case BMC_NOVELDIAMOND:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 57
		case BMC_GKA:
			prg16_89ab(0);
			prg16_cdef(0);
			break;

			// mapper 64
		case TENGEN_800032:
			// mapper 158
		case TENGEN_800037:
			prg16_89ab(m_prg_chunks - 1);
			prg16_cdef(m_prg_chunks - 1);
			break;
			// mapper 71
		case CAMERICA_BF9097:
			set_nt_mirroring(PPU_MIRROR_HORZ);
		case CAMERICA_BF9093:
			prg32(0xff);
			break;

			// mapper 79 (& 146)
		case AVE_NINA06:
			set_nt_mirroring(PPU_MIRROR_HORZ);
			break;

			// mapper 83
		case CONY_BOARD:
		case YOKO_BOARD:
			m_mapper83_reg[9] = 0x0f;
			prg8_cd(0x1e);
			prg8_ef(0x1f);
			break;

			// mapper 91
		case UNL_MK2:
			set_nt_mirroring(PPU_MIRROR_VERT);
			prg16_89ab(m_prg_chunks - 1);
			prg16_cdef(m_prg_chunks - 1);
			break;

			// mapper 104
		case CAMERICA_GOLDENFIVE:
			prg16_89ab(0x00);
			prg16_cdef(0x0f);
			break;
			// mapper 106
		case BTL_SMB3:
			prg8_89((m_prg_chunks << 1) - 1);
			prg8_ab(0);
			prg8_cd(0);
			prg8_ef((m_prg_chunks << 1) - 1);
			break;

			// mapper 108
		case WHIRLWIND_2706:
			prg32(0xff);
			break;

			// mapper 114
		case SUPERGAME_LIONKING:
			m_map114_reg = m_map114_reg_enabled = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
			// mapper 115
		case KASING_BOARD:
			m_mmc_reg[0] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
			// mapper 116
		case SOMERI_SL12:
			m_mmc_prg_base = m_mmc_chr_base = 0;
			m_mmc_prg_mask = 0xff;
			m_mmc_chr_mask = 0xff;
			m_mmc_cmd1 = 2; // mode
			m_mmc3_latch = 0;
			m_mmc3_wram_protect = 0;
			// MMC1 regs
			m_mmc1_count = 0;
			m_mmc_reg[0] = 0x0c;
			m_mmc_reg[1] = 0x00;
			m_mmc_reg[2] = 0x00;
			m_mmc_reg[3] = 0x00;
			// MMC3 regs
			m_mmc_prg_bank[0] = 0x3c;
			m_mmc_prg_bank[1] = 0x3d;
			m_mmc_prg_bank[2] = 0xfe;
			m_mmc_prg_bank[3] = 0xff;
			m_mmc_vrom_bank[0] = 0x00;
			m_mmc_vrom_bank[1] = 0x01;
			m_mmc_vrom_bank[2] = 0x04;
			m_mmc_vrom_bank[3] = 0x05;
			m_mmc_vrom_bank[4] = 0x06;
			m_mmc_vrom_bank[5] = 0x07;
			// VRC2 regs
			m_mmc_prg_bank[4] = 0x00;
			m_mmc_prg_bank[5] = 0x01;
			for (i = 0; i < 8; ++i)
				m_mmc_vrom_bank[6 + i] = i;
			someri_mode_update(machine());
			break;

			// mapper 120
		case BTL_TOBIDASE:
			prg32(2);
			break;

			// mapper 121
		case KAY_PANDAPRINCE:
			m_mmc_reg[5] = m_mmc_reg[6] = m_mmc_reg[7] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;

			// mapper 126
		case BMC_PJOY84:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = 0;
			pjoy84_set_base_mask(machine());
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

			// mapper 132
		case TXC_22211A:
			// mapper 172
		case TXC_22211B:
			// mapper 173
		case TXC_22211C:
			m_txc_reg[0] = m_txc_reg[1] = m_txc_reg[2] = m_txc_reg[3] = 0;
			break;

			// mapper 134
		case BMC_FAMILY_4646B:
			mmc3_common_initialize(machine(), 0x1f, 0xff, 0);
			break;

			// mapper 137
		case SACHEN_8259D:
			chr8(m_chr_chunks - 1, CHRROM);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 138
		case SACHEN_8259B:
			// mapper 139
		case SACHEN_8259C:
			// mapper 141
		case SACHEN_8259A:
			// mapper 150
		case SACHEN_74LS374:
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 143
		case SACHEN_TCA01:
			prg16_89ab(0);
			prg16_cdef(1);
			break;

			// mapper 156
		case OPENCORP_DAOU306:
			prg16_89ab(m_prg_chunks - 2);
			prg16_cdef(m_prg_chunks - 1);
			set_nt_mirroring(PPU_MIRROR_LOW);
			break;
			// mapper 163
		case NANJING_BOARD:
			m_mmc_count = 0xff;
			m_mmc_reg[0] = 0xff;
			m_mmc_reg[1] = 0;
			prg16_89ab(m_prg_chunks - 2);
			prg16_cdef(m_prg_chunks - 1);
			break;
			// mapper 164
		case WAIXING_FFV:
			prg16_89ab(0);
			prg16_cdef(0x1f);
			break;
			// mapper 166
		case SUBOR_TYPE1:
			m_subor_reg[0] = m_subor_reg[1] = m_subor_reg[2] = m_subor_reg[3] = 0;
			prg16_89ab(0);
			prg16_cdef(0x07);
			break;
			// mapper 167
		case SUBOR_TYPE0:
			m_subor_reg[0] = m_subor_reg[1] = m_subor_reg[2] = m_subor_reg[3] = 0;
			prg16_89ab(0);
			prg16_cdef(0x20);
			break;

			// mapper 176
		case UNL_XZY:
			// mapper 182
		case HOSENKAN_BOARD:
			prg32((m_prg_chunks - 1) >> 1);
			break;

		case FUKUTAKE_BOARD:    // mapper 186
			prg16_89ab(0);
			prg16_cdef(0);
			break;

			// mapper 187
		case UNL_KOF96:
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
			// mapper 189
		case TXC_TW:
			m_mmc_latch1 = 0;
			m_mmc_latch2 = 0x80;
			m_mmc_chr_base = 0;
			m_mmc_chr_mask = 0xff;
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;
			// mapper 193
		case NTDEC_FIGHTINGHERO:
			prg32((m_prg_chunks - 1) >> 1);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 197
		case UNL_SUPERFIGHTER3:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			unl_sf3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;
			// mapper 198
		case WAIXING_TYPE_F:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc_prg_bank[0] = 0x00;
			m_mmc_prg_bank[1] = 0x01;
			m_mmc_prg_bank[2] = 0x4e;
			m_mmc_prg_bank[3] = 0x4f;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;
			// mapper 199
		case WAIXING_TYPE_G:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc_prg_bank[0] = 0x00;
			m_mmc_prg_bank[1] = 0x01;
			m_mmc_prg_bank[2] = 0x3e;
			m_mmc_prg_bank[3] = 0x3f;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			m_mmc_vrom_bank[0] = 0x00;
			m_mmc_vrom_bank[1] = 0x02;
			m_mmc_vrom_bank[2] = 0x04;
			m_mmc_vrom_bank[3] = 0x05;
			m_mmc_vrom_bank[4] = 0x06;
			m_mmc_vrom_bank[5] = 0x07;
			m_mmc_vrom_bank[6] = 0x01;
			m_mmc_vrom_bank[7] = 0x03;
			waixing_g_set_chr(machine(), m_mmc_chr_base, m_mmc_chr_mask);
			break;

			// mapper 200
		case BMC_36IN1:
			prg16_89ab(m_prg_chunks - 1);
			prg16_cdef(m_prg_chunks - 1);
			break;

			// mapper 202
		case BMC_150IN1:
			// mapper 203
		case BMC_35IN1:
			// mapper 204
		case BMC_64IN1:
			// mapper 214
		case BMC_SUPERGUN_20IN1:
			prg16_89ab(0);
			prg16_cdef(0);
			break;
			// mapper 205
		case BMC_15IN1:
			mmc3_common_initialize(machine(), 0x1f, 0xff, 0);
			m_mmc_prg_base = 0x10;  // this board has a diff prg_base
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

			// mapper 208
		case GOUDER_37017:
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = m_mmc_reg[4] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
			// mapper 212
		case BMC_SUPERHIK_300IN1:
			chr8(0xff, CHRROM);
			prg32(0xff);
			break;

			// mapper 215
		case SUPERGAME_BOOGERMAN:
			m_mmc_reg[0] = 0x00;
			m_mmc_reg[1] = 0xff;
			m_mmc_reg[2] = 0x04;
			m_mmc_reg[3] = 0;
			mmc3_common_initialize(machine(), 0x1f, 0xff, 0);
			sgame_boog_set_prg(machine());
			mmc3_set_chr(machine(), m_mmc_chr_source, m_mmc_chr_base, m_mmc_chr_mask);
			break;

			// mapper 217
		case BMC_GOLDENCARD_6IN1:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc_reg[0] = 0x00;
			m_mmc_reg[1] = 0xff;
			m_mmc_reg[2] = 0x03;
			m_mmc_reg[3] = 0;
			bmc_gc6in1_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			bmc_gc6in1_set_chr(machine(), m_mmc_chr_source);
			break;
			// mapper 221
		case UNL_N625092:
			prg16_89ab(0);
			prg16_cdef(0);
			break;

			// mapper 223?
		case WAIXING_TYPE_I:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc3_wram_protect = 0;
			break;

			// mapper 224?
		case WAIXING_TYPE_J:
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			m_mmc_prg_bank[0] = 0x01;
			m_mmc_prg_bank[1] = 0x02;
			m_mmc_prg_bank[2] = 0x7e;
			m_mmc_prg_bank[3] = 0x7f;
			mmc3_set_prg(machine(), m_mmc_prg_base, m_mmc_prg_mask);
			break;

			// mapper 227
		case BMC_1200IN1:
			prg16_89ab(0);
			prg16_cdef(0);
			break;

			// mapper 229
		case BMC_31IN1:
			prg16_89ab(0);
			prg16_cdef(1);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 230
		case BMC_22GAMES:
			prg16_89ab(0);
			prg16_cdef(7);
			break;
			// mapper 231
		case BMC_20IN1:
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
			// mapper 232
		case CAMERICA_BF9096:
			m_mmc_latch1 = 0x18;
			m_mmc_latch2 = 0x00;
			bf9096_set_prg(machine());
			break;

			// mapper 243
		case SACHEN_74LS374_A:
			m_mmc_vrom_bank[0] = 3;
			chr8(3, CHRROM);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;

			// mapper 246
		case CNE_FSB:
			prg32(0xff);
			break;
			// mapper 249
		case WAIXING_SECURITY:
			m_mmc_reg[0] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;

			// mapper 254
		case BTL_PIKACHUY2K:
			m_mmc_reg[0] = 0xff;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;

			// mapper 255
		case BMC_110IN1:
			prg16_89ab(0);
			prg16_cdef(1);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;

			// UNIF only
		case BMC_64IN1NR:
			m_mmc_reg[0] = 0x80;
			m_mmc_reg[1] = 0x43;
			m_mmc_reg[2] = m_mmc_reg[3] = 0;
			bmc_64in1nr_set_prg(machine());
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case BMC_190IN1:
			prg16_89ab(0);
			prg16_cdef(0);
			break;
		case BMC_A65AS:
			prg16_89ab(0);
			prg16_cdef(7);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;
		case BMC_GS2004:
		case BMC_GS2013:
			prg32(0xff);
			break;
		case BMC_S24IN1SC03:
			m_mmc_reg[0] = 0x24;
			m_mmc_reg[1] = 0x9f;
			m_mmc_reg[2] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
		case BMC_T262:
			m_mmc_latch1 = 0;
			m_mmc_latch2 = 0;
			prg16_89ab(0);
			prg16_cdef(7);
			break;
		case DREAMTECH_BOARD:
			prg16_89ab(0);
			prg16_cdef(8);
			break;
		case UNL_8237:
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = 0;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			break;
		case UNL_AX5705:
			m_mmc_prg_bank[0] = 0;
			m_mmc_prg_bank[1] = 1;
			prg8_89(m_mmc_prg_bank[0]);
			prg8_ab(m_mmc_prg_bank[1]);
			prg8_cd(0xfe);
			prg8_ef(0xff);
			break;
		case UNL_RACERMATE:
			chr4_0(0, m_mmc_chr_source);
			chr4_4(0, m_mmc_chr_source);
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			break;

		case BMC_BENSHENG_BS5:
			m_mmc_prg_bank[0] = 0xff;
			m_mmc_prg_bank[1] = 0xff;
			m_mmc_prg_bank[2] = 0xff;
			m_mmc_prg_bank[3] = 0xff;
			bmc_bs5_update_banks(machine());
			break;

		case BMC_810544:
			prg16_89ab(0);
			prg16_cdef(0);
			set_nt_mirroring(PPU_MIRROR_VERT);
			break;

		case BMC_G63IN1:
			bmc_gb63_update(machine());
			break;

		case BMC_FK23C:
			m_mmc_reg[0] = 4;
			m_mmc_reg[1] = 0xff;
			m_mmc_reg[2] = m_mmc_reg[3] = 0;
			m_mmc_reg[4] = m_mmc_reg[5] = m_mmc_reg[6] = m_mmc_reg[7] = 0xff;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			fk23c_set_prg(machine());
			fk23c_set_chr(machine());
			break;

		case BMC_FK23CA:
			m_mmc_reg[0] = m_mmc_reg[1] = m_mmc_reg[2] = m_mmc_reg[3] = 0;
			m_mmc_reg[4] = m_mmc_reg[5] = m_mmc_reg[6] = m_mmc_reg[7] = 0xff;
			mmc3_common_initialize(machine(), 0xff, 0xff, 0);
			fk23c_set_prg(machine());
			fk23c_set_chr(machine());
			break;


		case FFE_MAPPER6:
			prg16_89ab(0);
			prg16_cdef(7);
			break;
		case FFE_MAPPER8:
			prg32(0);
			break;
		case FFE_MAPPER17:
			prg16_89ab(0);
			prg16_cdef(m_prg_chunks - 1);
			break;

		case UNSUPPORTED_BOARD:
		default:
			/* Mapper not supported */
			err = 2;
			break;
	}

	return err;
}

/*************************************************************

 nes_pcb_reset

 Resets the mmc bankswitch areas to their defaults.
 It returns a value "err" that indicates if it was
 successful. Possible values for err are:

 0 = success
 1 = no pcb found
 2 = pcb not supported

 *************************************************************/

int nes_state::nes_pcb_reset()
{
	int err = 0;
	const nes_pcb_intf *intf = nes_pcb_intf_lookup(m_pcb_id);

	if (intf == NULL)
		fatalerror("Missing PCB interface\n");

	/* Set the mapper irq callback */
	m_ppu->set_scanline_callback(intf ? intf->mmc_scanline : NULL);
	m_ppu->set_hblank_callback(intf ? intf->mmc_hblank : NULL);

	err = pcb_initialize(m_pcb_id);

	return err;
}
