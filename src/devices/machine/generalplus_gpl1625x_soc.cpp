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
// 703c - TV_Control
//
// 703e - LPHPosition
// 703f - LPVPosition
//
// 7042 - SControl
//
// 7050 - TFT_Ctrl
// 7051 - TFT_V_Width      or STN_COM_Clip
// 7052 - TFT_VSync_Setup
// 7053 - TFT_V_Start
// 7054 - TFT_V_End
// 7055 - TFT_H_Width
// 7056 - TFT_HSync_Setup
// 7057 - TFT_H_ Start
// 7058 - TFT_H_ End
// 7059 - TFT_RGB_Ctrl
// 705a - TFT_Status
// 705b - TFT_MemMode_WriteCMD
// 705c - TFT_MemMode_ReadCMD
//
// 705f - STN_Ctrl1
//
// 7062 - PPU_IRQ_EN      or TFT_INT_EN
// 7063 - PPU_IRQ_Status  or TFT_INT_CLR
//
// 706c - TFT_V_Show_Start
// 706d - TFT_V_Show_End
// 706e - TFT_H_Show_Start
// 706f - TFT_H_Show_End
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
// 7080 - TV_Saturation   or STN_SEG
// 7081 - TV_Hue          or STN_COM
// 7082 - TV_Brightness   or STN_PIC_COM
// 7083 - TV_Sharpness    or STN_CPWAIT
// 7084 - TV_Y_Gain       or STN_Ctrl2
// 7085 - TV_Y_Delay      or STN_GTG_SEG
// 7086 - TV_V_Position   or STN_GTG_COM
// 7087 - TV_H_Position   or STN_SEG_Clip
// 7088 - TV_VedioDAC
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
// 70a9 - MD_HPOS
// 70aa - MD_VPOS
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
// 7800 - BodyID
// 7803 - SYS_CTRL
// 7804 - CLK_Ctrl0
// 7805 - CLK_Ctrl1
// 7806 - Reset_Flag
// 7807 - Clock_Ctrl
// 7808 - LVR_Ctrl
// 780a - Watchdog_Ctrl
// 780b - Watchdog_Clear
// 780c - WAIT
// 780d - HALT
// 780e - SLEEP
// 780f - Power_State

// 7810 - BankSwitch_Ctrl

// 7816 - MAPSEL
// 7817 - PLLN
// 7818 - PLLWiatCLK
// 7819 - Cache_Ctrl
// 781a - Cache_HitRate
//
// 781f - IO_SR_SMT

// 7820 - MCS0_Ctrl
// 7821 - MCS1_Ctrl
// 7822 - MCS2_Ctrl
// 7823 - MCS3_Ctrl
// 7824 - MCS4_Ctrl
// 7825 - PSRAM_Ctrl
// 7826 - MCS_BYTE_SEL
// 7827 - MCS3_WETimingCtrl
// 7828 - MCS4_WETimingCtrl
// 7829 - MCS3_RDTimingCtrl
// 782a - MCS4_RDTimingCtrl
// 782b - MCS3_TimingCtrl
// 782c - MCS4_TimingCtrl
// 782d - RAW_WAR
// 782e - NOR_WHold
// 782f - SDRAM_EN

// 7830 - CHECKSUM0_LB
// 7831 - CHECKSUM1_LB

// 7835 - MCS0_Page
// 7836 - MCS1_Page
// 7837 - MCS2_Page
// 7838 - MCS3_Page
// 7839 - MCS4_Page
// 783a - SDRAM_Ctrl0
// 783b - SDRAM_Ctrl1
// 783c - SDRAM_Timing
// 783d - SDRAM_CBRCYC
// 783e - SDRAM_MISC
// 783f - SDR_STATUS
//
// 7840 - Mem_Ctrl
// 7841 - Addr_Ctrl

// 784e - ECC_ERR0_HB (?)
// 784f - ECC_ERR1_HB (?)

// 7850 - NF_Ctrl
// 7851 - NF_CMD
// 7852 - NF_AddrL
// 7853 - NF_AddrH
// 7854 - NF_Data
// 7855 - NF_INT_Ctrl

// 7857 - ECC_Ctrl
// 7858 - ECC_LPRL_LB
// 7859 - ECC_LPRH_LB
// 785a - ECC_CPR_LB
// 785b - ECC_LPR_CKL_LB
// 785c - ECC_LPR_CKH_LB
// 785d - ECC_CPCKR_LB
// 785e - ECC_ERR0_LB
// 785f - ECC_ERR1_LB

// 7860 - IOA_Data
// 7861 - IOA_Buffer
// 7862 - IOA_Dir
// 7863 - IOA_Attrib
// 7854 - IOA_Drv
//
// 7868 - IOB_Data
// 7869 - IOB_Buffer
// 786a - IOB_Dir
// 786b - IOB_Attrib
// 786c - IOB_Latch
// 786d - IOB_Drv
//
// 7870 - IOC_Data
// 7871 - IOC_Buffer
// 7872 - IOC_Dir
// 7873 - IOC_Attrib
// 7874 - SDRAM_Drv
// 7875 - IOC_Drv
// 7876 - SDRAM_Dly
// 7877 - IOC_Latch
// 7878 - IOD_Data
// 7879 - IOD_Buffer
// 787a - IOD_Dir
// 787b - IOD_Attrib
// 787c - IOD_Drv
// 787d - IOD_Dly
// 787e - CS_Drv / MCS_Drv
// 787f - CS_Dly / MCS_Dly
// 7880 - IOE_Data
// 7881 - IOE_Buffer
// 7882 - IOE_Dir
// 7883 - IOE_Attrib
// 7884 - IOE_Drv

// 7888 - MEM_Drv
// 7889 - MEM_Dly0
// 788a - MEM_Dly1
// 788b - MEM_Dly2
// 788c - MEM_Dly3
// 788d - MEM_Dly4
// 788e - MEM_Dly5
// 788f - MEM_Dly6

// 78a0 - INT_Status1
// 78a1 - INT_Status2
// 78a3 - INT_Status3

// 78a4 - INT_Priority1
// 78a5 - INT_Priority2
// 78a6 - INT_Priority2

// 78a8 - MINT_Ctrl

// 78b0 - TimeBaseA_Ctrl
// 78b1 - TimeBaseB_Ctr
// 78b2 - TimeBaseC_Ctrl

// 78b8 - TimeBase_Reset

// 78c0 - TimerA_Ctrl
// 78c1 - TimerA_CCCtrl
// 78c2 - TimerA_Preload
// 78c3 - TimerA_CCReg
// 78c4 - TimerA_UpCount

// 78c8 - TimerB_Ctrl
// 78c9 - TimerB_CCCtrl
// 78ca - TimerB_Preload
// 78cb - TimerB_CCReg
// 78cc - TimerB_UpCount

// 78d0 - TimerC_Ctrl
// 78d1 - TimerC_CCCtrl
// 78d2 - TimerC_Preload
// 78d3 - TimerC_CCReg
// 78d4 - TimerC_UpCount

// 78d8 - TimerD_Ctrl
// 78da - TimerD_Preload
// 78dc - TimerD_UpCount

// 78e0 - TimerE_Ctrl
// 78e2 - TimerE_Preload
// 78e4 - TimerE_UpCount
// 78e8 - TimerF_Ctrl
// 78ea - TimerF_Preload
// 78ec - TimerF_UpCount

// 78f0 - CHA_Ctrl
// 78f1 - CHA_Data
// 78f2 - CHA_FIFO
//
// 78f8 - CHB_Ctrl
// 78f9 - CHB_Data
// 78fa - CHB_FIFO
// 78fb - DAC_PGA

// 78ff - IISEN

// 7900 - UART_Data
// 7901 - UART_RXStatus
// 7902 - UART_Ctrl
// 7903 - UART_BaudRate
// 7904 - UART_Status
// 7905 - UART_FIFO
// 7906 - UART_TXDLY
// 7907 - IrDA_BaudRate
// 7908 - IrDA_Ctrl
// 7909 - IrDA_LowPower

// 7920 - Second
// 7921 - Minute
// 7922 - Hour

// 7924 - Alarm_Second
// 7925 - Alarm_Minute
// 7926 - Alarm_Hour

// 7934 - RTC_Ctrl
// 7935 - RTC_INT_Status
// 7936 - RTC_INT_Ctrl
// 7937 - RTC_Busy

// 7940 - SPI_Ctrl
// 7941 - SPI_TXStatus
// 7942 - SPI_TXData
// 7943 - SPI_RXStatus
// 7944 - SPI_RXData
// 7945 - SPI_Misc

// 7960 - ADC_Setup
// 7961 - MADC_Ctrl
// 7962 - MADC_Data
// 7963 - ASADC_Ctrl
// 7964 - ASDAC_Data

// 7966 - USELINEIN
// 7967 - SH_WAIT

// 79d0 - SD1_DataTX
// 79d1 - SD1_DataRX
// 79d2 - SD1_CMD
// 79d3 - SD1_ArgL
// 79d4 - SD1_ArgH
// 79d5 - SD1_RespL
// 79d6 - SD1_RespH
// 79d7 - SD1_Status
// 79d8 - SD1_Ctrl
// 79d9 - SD1_BLKLEN
// 79da - SD1_INT

// 79e0 - SD2_DataTX
// 79e1 - SD2_DataRX
// 79e2 - SD2_CMD
// 79e3 - SD2_ArgL
// 79e4 - SD2_ArgH
// 79e5 - SD2_RespL
// 79e6 - SD2_RespH
// 79e7 - SD2_Status
// 79e8 - SD2_Ctrl
// 79e9 - SD2_BLKLEN
// 79ea - SD2_INT

// 79f0 - CMA_R_Y_In
// 79f1 - CMA_G_U_In
// 79f2 - CMA_B_V_In
// 79f3 - CMA_Ctrl
// 79f4 - CMA_R_Y_Out
// 79f5 - CMA_G_U_Out
// 79f6 - CMA_B_V_Out

// 7a00 - USBH_Config
// 7a01 - USBH_TimeConfig
// 7a02 - USBH_Data
// 7a03 - USBH_Transfer
// 7a04 - USBH_DveAddr
// 7a05 - USBH_DveEP
// 7a06 - USBH_TXCount
// 7a07 - USBH_RXCount
// 7a08 - USBH_FIFOInPointer
// 7a09 - USBH_FIFOOutPointer
// 7a0a - USBH_AutoInByteCount
// 7a0b - USBH_AutoOutByteCount
// 7a0c - USBH_AutoTrans
// 7a0d - USBH_Status
// 7a0e - USBH_INT
// 7a0f - USBH_INTEN

// 7a11 - USBH_SoftRST
// 7a12 - USBH_SOFTimer
// 7a13 - USBH_FrameNum
//
// 7a17 - USBH_INAckCount
// 7a18 - USBH_OutAckCount
// 7a19 - USBH_RSTAckCount

// 7a1b - USBH_DReadback

// 7a20 - USBH_SOF_BOND
// 7a21 - USBH_ISOConfig

// 7a30 - USBD_Config
// 7a31 - USBD_Function
// 7a32 - USBD_PMR
// 7a33 - USBD_EP0Data
// 7a34 - USBD_BIData
// 7a35 - USBD_BOData
// 7a36 - USBD_INTINData
// 7a37 - USBD_EPEvent
// 7a38 - USBD_GLOINT
// 7a39 - USBD_INTEN
// 7a3a - USBD_INT
// 7a3b - USBD_SCI NTEN
// 7a3c - USBD_SCINT
// 7a3d - USBD_EPAutoSet
// 7a3e - USBD_EPSetStall
// 7a3f - USBD_EPBufClear
// 7a40 - USBD_EPEvntClear
// 7a41 - USBD_EP0WrtCount
// 7a42 - USBD_BOWrtCount
// 7a43 - USBD_EP0BufPointer
// 7a44 - USBD_BIBufPointer
// 7a45 - USBD_BOBufPointer
// 7a46 - USBD_EP0RTR
// 7a47 - USBD_EP0RR
// 7a48 - USBD_ EP0VR
// 7a49 - USBD_ EP0IR
// 7a4a - USBD_ EP0LR
// 7a4b - USBD_INTBufPointer
// 7a4c - USBD_INTF
// 7a4d - USBD_ALT
// 7a4e - USBD_ISOOData
// 7a4f - USBD_ISOIData
// 7a50 - USBD_DMAWrtCountL
// 7a51 - USBD_DMAWrtCountH
// 7a52 - USBD_DMAAck
// 7a53 - USBD_DMAAckH
// 7a54 - USBD_EPStall
// 7a55 - USBD_CALT
// 7a56 - USBD_MAXALT
// 7a57 - USBD_Device
// 7a58 - USBD_NullPkt
// 7a59 - USBD_DMAINT

// 7a5b - USBD_MAXINT
// 7a5c - USBD_ISOEvent
// 7a5d - USBD_ISOINTE
// 7a5e - USBD_ISOINT
// 7a5f - USBD_ISOOWrtCount
// 7a60 - USBD_IOUTData
// 7a61 - USBD_IOUTEvent
// 7a62 - USBD_IOUTINTEN
// 7a63 - USBD_IOUTINT
// 7a64 - USBD_IOUTWrtCount
// 7a65 - USBD_ISOOBufPointer
// 7a66 - USBD_ISOIBufPointer

// 7a80 - DMA_Ctrl0
// 7a81 - DMA_SRC_AddrL0
// 7a82 - DMA_TAR_AddrL0
// 7a83 - DMA_TCountL0
// 7a84 - DMA_SRC_AddrH0
// 7a85 - DMA_TAR_AddrH0
// 7a86 - DMA_TCountH0
// 7a87 - DMA_MISC0
// 7a88 - DMA_Ctrl1
// 7a89 - DMA_SRC_AddrL1
// 7a8a - DMA_TAR_AddrL1
// 7a8b - DMA_TCountL1
// 7a8c - DMA_SRC_AddrH1
// 7a8d - DMA_TAR_AddrH1
// 7a8e - DMA_TCountH1
// 7a8f - DMA_MISC1
// 7a90 - DMA_Ctrl2
// 7a91 - DMA_SRC_AddrL2
// 7a92 - DMA_TAR_AddrL2
// 7a93 - DMA_TCountL2
// 7a94 - DMA_SRC_AddrH2
// 7a95 - DMA_TAR_AddrH2
// 7a96 - DMA_TCountH2
// 7a97 - DMA_MISC2
// 7a98 - DMA_Ctrl3
// 7a99 - DMA_SRC_AddrL3
// 7a9a - DMA_TAR_AddrL3
// 7a9b - DMA_TCountL3
// 7a9c - DMA_SRC_AddrH3
// 7a9d - DMA_TAR_AddrH3
// 7a9e - DMA_TCountH3
// 7a9f - DMA_MISC3
//
// 7ab0 - DMA_SPRISIZE0
// 7ab1 - DMA_SPRISIZE1
// 7ab2 - DMA_SPRISIZE2
// 7ab3 - DMA_SPRISIZE3
//
// 7abd - DMA_LineLength
// 7abe - DMA_SS
// 7abf - DMA_INT
//
// 7ac0 - KS_Ctrl1
// 7ac1 - KS_Ctrl2
// 7ac2 - KS_Addr
// 7ac3 - KS_Velocity
//
// 7ac8 - KS_Data0
// 7ac9 - KS_Data1
// 7aca - KS_Data2
// 7acb - KS_Data3
// 7acc - KS_Data4
// 7acd - KS_Data5
// 7ace - KS_Data6
// 7acf - KS_Data7
// 7ad0 - KS_Data8
// 7ad1 - KS_Data9
// 7ad2 - KS_Data10
//
// 7ae0 - E-Fuse0
// 7ae1 - E-Fuse1
// 7ae2 - E-Fuse2
// 7ae3 - E-Fuse3
//
// 7af0 - Byte_Swap
// 7af1 - Nibble_Swap
// 7af2 - TwoBit_Swap
// 7af3 - Bit_Reverse
//
// 7b80 to 7b9f Sound Channel 0-15 regs
// 7ba0 to 7bbf Sound Channel 16-31 regs
//
// 7c00 - 7dff Sound Attribute
// 7e00 - 7fff Sound Phase

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


