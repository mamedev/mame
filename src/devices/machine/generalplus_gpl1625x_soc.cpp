// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl1625x_soc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPAC800A register list
//
// 7000 - Tx3_X_Position
// 7001 - Tx3_Y_Position
// 7002 - Tx3_X_Offset
// 7003 - Tx3_Y_Offset
// 7004 - Tx3_Attribute
// 7005 - Tx3_Control
// 7006 - Tx3_N_PTR
// 7007 - Tx3_A_PTR
// 7008 - Tx4_X_Position
// 7009 - Tx4_Y_Position
// 700a - Tx4_X_Offset
// 700b - Tx4_Y_Offset
// 700c - Tx4_Attribute
// 700d - Tx4_Control
// 700e - Tx4_N_PTR
// 700f - Tx4_A_PTR
// 7010 - Tx1_X_Position
// 7011 - Tx1_Y_Position
// 7012 - Tx1_Attribute
// 7013 - Tx1_Control
// 7014 - Tx1_N_PTR
// 7015 - Tx1_A_PTR
// 7016 - Tx2_X_Position
// 7017 - Tx2_Y_Position
// 7018 - Tx2_Attribute
// 7019 - Tx2_Control
// 701a - Tx2_N_PTR
// 701b - Tx2_A_PTR
// 701c - VComValue
// 701d - VComOffset
// 701e - VComStep
// 701f
// 7020 - Segment_Tx1
// 7021 - Segment_Tx2 
// 7022 - Segment_sp
// 7023 - Segment_Tx3
// 7024 - Segment_Tx4
//
// 7028 - Tx4_Cosine
// 7029 - Tx4_Sine
// 702a - Blending
// 702b - Segment_Tx1H
// 702c - Segment_Tx2H
// 702d - Segment_spH
// 702e - Segment_Tx3H
// 702f - Segment_Tx4H
// 7030 - Fade_Control
//
// 7036 - IRQTMV
// 7037 - IRQTMH
// 7038 - Line_Counter
// 7039 - LightPen_Control
// 703a - Palette_Control
//
// 703e - LPHPosition
// 703f - LPVPosition
//
// 7042 - SControl
// 
// 7062 - PPU_IRQ_EN
// 7063 - PPU_IRQ_Status
//
// 7070 - SPDMA_Source
// 7071 - SPDMA_Target
// 7072 - SPDMA_Number
//
// 7078 - FBI_AddrL
// 7079 - FBI_AddrH
// 707a - FBO_AddrL
// 707b - FBO_AddrH
// 707c - FB_PPU_GO
// 707d - BLD_Color
// 707e - PPU_RAM_Bank
// 707f - PPU_Enable
//
// 7090 - TG_CTRL1
// 7091 - TG_CTRL2
// 7092 - TG_HLSTART
// 7093 - TG_HEND
// 7094 - TG_VL0START
// 7095 - MD_FBADDRL
// 7096 - TG_VEND
// 7097 - TG_HSTART
// 7098 - MD_RGBL
// 7099 - SEN_CTRL
// 709a - TG_BSUPPER
// 709b - TG_BSLOWER
// 709c - MD_RGBH
// 709d - MD_CR
// 709e - TG_FBSADDRL
// 709f - TG_FBSADDRH
// 70a0 - TG_VL1START
// 70a1 - TG_H_WIDTH
// 70a2 - TG_V_WIDTH
// 70a3 - TG_CUTSTART
// 70a4 - TG_CUTWIDTH
// 70a5 - TG_VSTART
// 70a6 - MD_FBADDRH
// 70a7 - TG_H_RATIO
// 70a8 - TG_V_RATIO
//
// 7100 to 71ff - Tx_Hvoffset (when PPU_RAM_BANK is 0)
// 7200 to 72ff - HCMValue (when PPU_RAM_BANK is 0)
// 7100 to 72ff - Tx3_Cos / Tx3_Sin (when PPU_RAM_BANK is 1)
//
// 7300 to 73ff - Palette0 / Palette1 / Palette2 / Palette3 (3 banks using PAL_RAM_SEL)
//
// 7400 to 77ff - Standard sprite list (when PPU_RAM_BANK is 0)
// 7400 to 74ff - Sprite0_Attribute list (when PPU_RAM_BANK is 1 and PPU_Enable bit 9 '3D' mode is disabled)
// 7400 to 77ff - 3D sprite attribute list (when PPU_RAM_BANK is 1 and PPU_Enable bit 9 '3D' mode is enabled)
//
// 7800 - P_BodyID
// 7803 - P_SYS_CTRL
// 7804 - P_CLK_Ctrl0
// 7805 - P_CLK_Ctrl1
// 7806 - P_Reset_Flag
// 7807 - P_Clock_Ctrl
// 7808 - P_LVR_Ctrl
// 780a - P_Watchdog_Ctrl
// 780b - P_Watchdog_Clear
// 780c - P_WAIT
// 780d - P_HALT
// 780e - P_SLEEP
// 780f - P_Power_State

// 7817 - P_PLLN
// 7818 - P_PLLWiatCLK
// 7819 - P_Cache_Ctrl
// 781a - P_Cache_HitRate
// 781f - P_IO_SR_SMT

DEFINE_DEVICE_TYPE(GPAC800,   generalplus_gpac800_device,  "gpac800",    "GeneralPlus GPL1625x System-on-a-Chip (with NAND handling)")
DEFINE_DEVICE_TYPE(GP_SPISPI, generalplus_gpspispi_device, "gpac800spi", "GeneralPlus GPL1625x System-on-a-Chip (with SPI handling)")

generalplus_gpac800_device::generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GPAC800, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpac800_device::gpac800_internal_map), this))
{
}

generalplus_gpspispi_device::generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GP_SPISPI, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpspispi_device::gpspispi_internal_map), this))
{
}


// GPR27P512A   = C2 76
// HY27UF081G2A = AD F1 80 1D
// H27U518S2C   = AD 76

uint16_t generalplus_gpac800_device::nand_7854_r()
{
	// TODO: use actual NAND / Smart Media devices once this is better understood.
	// The games have extensive checks on startup to determine the flash types, but then it appears that
	// certain games (eg jak_tsm) will only function correctly with specific ones, even if the code
	// continues regardless.  Others will bail early if they don't get what they want.

	// I think some unSP core maths bugs are causing severe issues after the initial load for jak_tsm
	// at the moment, possibly the same ones that are causing rendering issues in the jak_gtg bitmap
	// test and seemingly incorrect road data for jak_car2, either that or the hookup here is very
	// non-standard outside of the ident codes

	// real TSM code starts at 4c000


	//logerror("%s:sunplus_gcm394_base_device::nand_7854_r\n", machine().describe_context());

	if (m_nandcommand == 0x90) // read ident
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ IDENT byte %d\n", machine().describe_context(), m_curblockaddr);

		uint8_t data = 0x00;

		if (m_romtype == 0)
		{
			if (m_curblockaddr == 0)
				data = 0xc2;
			else
				data = 0x76;
		}
		else if (m_romtype == 1)
		{
			if (m_curblockaddr == 0)
				data = 0xad;
			else if (m_curblockaddr == 1)
				data = 0x76;
		}
		else
		{
			if (m_curblockaddr == 0)
				data = 0xad;
			else if (m_curblockaddr == 1)
				data = 0xf1;
			else if (m_curblockaddr == 2)
				data = 0x80;
			else if (m_curblockaddr == 3)
				data = 0x1d;
		}

		m_curblockaddr++;

		return data;
	}
	else if (m_nandcommand == 0x00 || m_nandcommand == 0x01 || m_nandcommand  == 0x50)
	{
		//logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ DATA byte %d\n", machine().describe_context(), m_curblockaddr);

		uint8_t data = m_nand_read_cb(m_effectiveaddress + m_curblockaddr);

		m_curblockaddr++;

		return data;
	}
	else if (m_nandcommand == 0x70) // read status
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ STATUS byte %d\n", machine().describe_context(), m_curblockaddr);

		return 0xffff;
	}
	else
	{
		logerror("%s:sunplus_gcm394_base_device::nand_7854_r   READ UNKNOWN byte %d\n", machine().describe_context(), m_curblockaddr);
		return 0xffff;
	}

	return 0x0000;
}

// 7998

void generalplus_gpac800_device::nand_command_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_command_w %04x\n", machine().describe_context(), data);
	m_nandcommand = data;
}

void generalplus_gpac800_device::nand_addr_low_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_addr_low_w %04x\n", machine().describe_context(), data);
	m_nand_addr_low = data;
	m_curblockaddr = 0;
}

void generalplus_gpac800_device::recalculate_calculate_effective_nand_address()
{
	uint8_t type = m_nand_7856 & 0xf;
	uint8_t shift = 0;
	uint32_t page_offset = 0;

	if (type == 7)
		shift = 4;
	else if (type == 11)
		shift = 5;

	if (m_nandcommand == 0x01)
		page_offset = 256;
	else if (m_nandcommand == 0x50)
		page_offset = 512;

	uint32_t nandaddress = (m_nand_addr_high << 16) | m_nand_addr_low;

	if (m_nand_7850 & 0x4000)
		nandaddress *= 2;

	uint32_t page = type ? nandaddress : /*(m_nand_7850 & 0x4000) ?*/ nandaddress >> 8 /*: nandaddress >> 9*/;
	m_effectiveaddress = (page * 528 + page_offset) << shift;

	logerror("%s: Requested address is %08x, translating to %08x\n", machine().describe_context(), nandaddress, m_effectiveaddress);
}

void generalplus_gpac800_device::nand_addr_high_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_addr_high_w %04x\n", machine().describe_context(), data);
	m_nand_addr_high = data;

	recalculate_calculate_effective_nand_address();

	m_curblockaddr = 0;
}

void generalplus_gpac800_device::nand_dma_ctrl_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_dma_ctrl_w(?) %04x\n", machine().describe_context(), data);
	m_nand_dma_ctrl = data;
}

uint16_t generalplus_gpac800_device::nand_7850_status_r()
{
	// 0x8000 = ready
	return m_nand_7850 | 0x8000;
}

void generalplus_gpac800_device::nand_7850_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7850_w %04x\n", machine().describe_context(), data);
	m_nand_7850 = data;
}

void generalplus_gpac800_device::nand_7856_type_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7856_type_w %04x\n", machine().describe_context(), data);
	m_nand_7856 = data;

	recalculate_calculate_effective_nand_address();

	m_curblockaddr = 0;
}

void generalplus_gpac800_device::nand_7857_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_7857_w %04x\n", machine().describe_context(), data);
	m_nand_7857 = data;
}

void generalplus_gpac800_device::nand_785b_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785b_w %04x\n", machine().describe_context(), data);
	m_nand_785b = data;
}

void generalplus_gpac800_device::nand_785c_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785c_w %04x\n", machine().describe_context(), data);
	m_nand_785c = data;
}

void generalplus_gpac800_device::nand_785d_w(uint16_t data)
{
	logerror("%s:sunplus_gcm394_base_device::nand_785d_w %04x\n", machine().describe_context(), data);
	m_nand_785d = data;
}

// [:maincpu] ':maincpu' (00146D)  jak_tsm
uint16_t generalplus_gpac800_device::nand_785e_r()
{
	return 0x0000;
}

//[:maincpu] ':maincpu' (001490)  jak_tsm
uint16_t generalplus_gpac800_device::nand_ecc_low_byte_error_flag_1_r()
{
	return 0x0000;
}

/*
UNMAPPED reads  writes

jak_tsm uses these (all iniitalized near start)
unclear if these are specific to the GPAC800 type, or present in the older types

[:maincpu] ':maincpu' (00043F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (000442):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0000)
[:maincpu] ':maincpu' (000445):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (000449):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00044D):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0a57)
[:maincpu] ':maincpu' (000451):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (000454):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (000458):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x3011)
[:maincpu] ':maincpu' (00045B):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x0000)
[:maincpu] ':maincpu' (00045D):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x0000)
[:maincpu] ':maincpu' (00045F):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x0000)
[:maincpu] ':maincpu' (000461):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x0000)

jak_car2 uses these

[:maincpu] ':maincpu' (004056):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (004059):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00405C):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0a57)
[:maincpu] ':maincpu' (00405F):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (004062):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (004065):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x3011)
[:maincpu] ':maincpu' (004069):sunplus_gcm394_base_device::unk_r @ 0x7880
[:maincpu] ':maincpu' (00406F):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x1249)
[:maincpu] ':maincpu' (004071):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x1249)
[:maincpu] ':maincpu' (004073):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x1249)
[:maincpu] ':maincpu' (004075):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x1249)
[:maincpu] ':maincpu' (004088):sunplus_gcm394_base_device::unk_w @ 0x7841 (data 0x000f)
[:maincpu] ':maincpu' (00408F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (004092):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0002)

[:maincpu] ':maincpu' (03000A):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x36db)
[:maincpu] ':maincpu' (03000C):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x36db)
[:maincpu] ':maincpu' (03000E):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x36db)
[:maincpu] ':maincpu' (030010):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x36db)
[:maincpu] ':maincpu' (030013):sunplus_gcm394_base_device::unk_w @ 0x787f (data 0x0010)
[:maincpu] ':maincpu' (03001D):sunplus_gcm394_base_device::unk_w @ 0x7804 (data 0x1c7f)
[:maincpu] ':maincpu' (030023):sunplus_gcm394_base_device::unk_w @ 0x7805 (data 0xcdf0)
[:maincpu] ':maincpu' (03E645):sunplus_gcm394_base_device::unk_w @ 0x7861 (data 0x1f66)
[:maincpu] ':maincpu' (03E64C):sunplus_gcm394_base_device::unk_w @ 0x786b (data 0x0000)
[:maincpu] ':maincpu' (03E64F):sunplus_gcm394_base_device::unk_w @ 0x7869 (data 0x0000)
[:maincpu] ':maincpu' (03E652):sunplus_gcm394_base_device::unk_w @ 0x786a (data 0x0000)
[:maincpu] ':maincpu' (03E65B):sunplus_gcm394_base_device::unk_w @ 0x7966 (data 0x0001)
[:maincpu] ':maincpu' (03CBD0):sunplus_gcm394_base_device::unk_w @ 0x7871 (data 0x0000)

-- this one seems like a common alt type of DMA, used in both hw types as it polls 707c status before doing it
[:maincpu] ':maincpu' (03B4C7):sunplus_gcm394_base_device::unk_w @ 0x707c (data 0x0001)
-- also video / alt dma?
[:maincpu] ':maincpu' (068C15):sunplus_gcm394_base_device::unk_r @ 0x707e

beambox sets things up with different values (ultimately stalls on some check, maybe seeprom?)

[:maincpu] ':maincpu' (00043F):sunplus_gcm394_base_device::unk_w @ 0x780a (data 0x0000)
[:maincpu] ':maincpu' (000442):sunplus_gcm394_base_device::unk_w @ 0x7808 (data 0x0000)
[:maincpu] ':maincpu' (000445):sunplus_gcm394_base_device::unk_w @ 0x782f (data 0x0002)
[:maincpu] ':maincpu' (000449):sunplus_gcm394_base_device::unk_w @ 0x783d (data 0x05d9)
[:maincpu] ':maincpu' (00044D):sunplus_gcm394_base_device::unk_w @ 0x783c (data 0x0f58)
[:maincpu] ':maincpu' (000451):sunplus_gcm394_base_device::unk_w @ 0x783b (data 0x2400)
[:maincpu] ':maincpu' (000454):sunplus_gcm394_base_device::unk_w @ 0x783e (data 0x0002)
[:maincpu] ':maincpu' (000458):sunplus_gcm394_base_device::unk_w @ 0x783a (data 0x4011)
[:maincpu] ':maincpu' (00045C):sunplus_gcm394_base_device::unk_w @ 0x7874 (data 0x2492)   -- note pair of 4, but different values to above games
[:maincpu] ':maincpu' (00045E):sunplus_gcm394_base_device::unk_w @ 0x787c (data 0x2492)
[:maincpu] ':maincpu' (000460):sunplus_gcm394_base_device::unk_w @ 0x7888 (data 0x2492)
[:maincpu] ':maincpu' (000462):sunplus_gcm394_base_device::unk_w @ 0x787e (data 0x2492)

vbaby code is very differet, attempts to load NAND block manually, not with DMA

*/


// all tilemap registers etc. appear to be in the same place as the above system, including the 'extra' ones not on the earlier models
// so it's likely this is built on top of that just with NAND support
void generalplus_gpac800_device::gpac800_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	// 785x = NAND device
	map(0x007850, 0x007850).rw(FUNC(generalplus_gpac800_device::nand_7850_status_r), FUNC(generalplus_gpac800_device::nand_7850_w)); // NAND Control Reg
	map(0x007851, 0x007851).w(FUNC(generalplus_gpac800_device::nand_command_w)); // NAND Command Reg
	map(0x007852, 0x007852).w(FUNC(generalplus_gpac800_device::nand_addr_low_w)); // NAND Low Address Reg
	map(0x007853, 0x007853).w(FUNC(generalplus_gpac800_device::nand_addr_high_w)); // NAND High Address Reg
	map(0x007854, 0x007854).r(FUNC(generalplus_gpac800_device::nand_7854_r)); // NAND Data Reg
	map(0x007855, 0x007855).w(FUNC(generalplus_gpac800_device::nand_dma_ctrl_w)); // NAND DMA / INT Control
	map(0x007856, 0x007856).w(FUNC(generalplus_gpac800_device::nand_7856_type_w)); // usually 0x0021?
	map(0x007857, 0x007857).w(FUNC(generalplus_gpac800_device::nand_7857_w));

	// most of these are likely ECC stuff for testing the ROM?
	map(0x00785b, 0x00785b).w(FUNC(generalplus_gpac800_device::nand_785b_w));
	map(0x00785c, 0x00785c).w(FUNC(generalplus_gpac800_device::nand_785c_w));
	map(0x00785d, 0x00785d).w(FUNC(generalplus_gpac800_device::nand_785d_w));
	map(0x00785e, 0x00785e).r(FUNC(generalplus_gpac800_device::nand_785e_r)); // also ECC status related?
	map(0x00785f, 0x00785f).r(FUNC(generalplus_gpac800_device::nand_ecc_low_byte_error_flag_1_r)); // ECC Low Byte Error Flag 1 (maybe)

	// 128kwords internal ROM
	//map(0x08000, 0x0ffff).rom().region("internal", 0); // lower 32kwords of internal ROM is visible / shadowed depending on boot pins and register
	map(0x08000, 0x0ffff).r(FUNC(generalplus_gpac800_device::internalrom_lower32_r)).nopw();
	map(0x10000, 0x27fff).rom().region("internal", 0x10000); // upper 96kwords of internal ROM is always visible
	map(0x28000, 0x2ffff).noprw(); // reserved
	// 0x30000+ is CS access

	map(0x030000, 0x1fffff).rw(FUNC(generalplus_gpac800_device::cs_space_r), FUNC(generalplus_gpac800_device::cs_space_w));
	map(0x200000, 0x3fffff).rw(FUNC(generalplus_gpac800_device::cs_bank_space_r), FUNC(generalplus_gpac800_device::cs_bank_space_w));
}

void generalplus_gpac800_device::device_reset()
{
	sunplus_gcm394_base_device::device_reset();

	m_nand_addr_low = 0x0000;
	m_nand_addr_high = 0x0000;
	m_nand_dma_ctrl = 0x0000;
	m_nand_7850 = 0x0000;
	m_nand_785d = 0x0000;
	m_nand_785c = 0x0000;
	m_nand_785b = 0x0000;
	m_nand_7856 = 0x0000;
	m_nand_7857 = 0x0000;
}


uint16_t generalplus_gpspispi_device::spi_unk_7943_r()
{
	return 0x0007;
}

void generalplus_gpspispi_device::gpspispi_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	map(0x007943, 0x007943).r(FUNC(generalplus_gpspispi_device::spi_unk_7943_r));

	map(0x008000, 0x00ffff).rom().region("internal", 0);
}


