// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl951xx_soc.h"


void generalplus_gpl951xx_device::ramwrite_w(offs_t offset, uint16_t data)
{
	m_mainram[offset] = data;
}

uint16_t generalplus_gpl951xx_device::ramread_r(offs_t offset)
{
	return m_mainram[offset];
}

uint16_t generalplus_gpl951xx_device::spi_direct_7b40_r()
{
	return 0xffff; // doesn't care for now
}

uint16_t generalplus_gpl951xx_device::spi_direct_79f5_r()
{
	return 0xffff; // hangs if returning 0
}

uint16_t generalplus_gpl951xx_device::spi_direct_7b46_r()
{
	int i = machine().rand();

	if (i & 1) return 0x01;
	else return 0x02;
}

uint16_t generalplus_gpl951xx_device::spi_direct_79f4_r()
{
	// status bits?
	return machine().rand();
}


uint16_t generalplus_gpl951xx_device::spi_direct_7af0_r()
{
	return m_7af0;
}

void generalplus_gpl951xx_device::spi_direct_7af0_w(uint16_t data)
{
	// words read from ROM are written here during the checksum routine in RAM, and must
	// be shifted for the checksum to pass.
	m_7af0 = data >> 8;
}


uint16_t generalplus_gpl951xx_device::spi_direct_78e8_r()
{
	return machine().rand();
}

void generalplus_gpl951xx_device::device_start()
{
	sunplus_gcm394_base_device::device_start();
	save_item(NAME(m_7af0));
}

void generalplus_gpl951xx_device::device_reset()
{
	sunplus_gcm394_base_device::device_reset();
	m_7af0 = 0;
}

void generalplus_gpl951xx_device::spi_direct_78e8_w(uint16_t data)
{
	logerror("%s: spi_direct_78e8_w %04x\n", machine().describe_context(), data);
}

void generalplus_gpl951xx_device::gpspi_direct_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::base_internal_map(map);

	map(0x000000, 0x0027ff).rw(FUNC(generalplus_gpl951xx_device::ramread_r), FUNC(generalplus_gpl951xx_device::ramwrite_w));
	// TODO: RAM is only 0x2800 on this, like earlier SPG2xx models? unmap the extra from the base_internal_map?

	// 7000 - Tx3_X_Position
	// 7001 - Tx3_Y_Position
	// 7002 - Tx3_X_Offset
	// 7003 - Tx3_Y_Offset
	// 7004 - Tx3_Attribute
	// 7005 - Tx3_Control
	// 7006 - Tx3_N_PTR
	// 7007 - Tx3_A_PTR

	// 7010 - Tx1_X_Position
	// 7011 - Tx1_Y_Position
	// 7012 - Tx1_Attribute
	// 7013 - Tx1_Control
	// 7014 - Tx1_N_PTR
	// 7015 - Tx1_A_PTR
	// 7016 - Tx2_X_Position
	// 7017 - Tx2_Y_Position 
	// 7018 - Tx2_Attribute
	// 7019 - Tx2_Contro
	// 701a - Tx2_N_PTR
	// 701b - Tx2_A_PTR
	// 701c - VComValue
	// 701d - VComOffset
	// 701e - VComStep

	// 7020 - Segment_Tx1
	// 7021 - Segment_Tx2
	// 7022 - Segment_sp
	// 7023 - Segment_Tx3
	//
	// 702a - Blending
	// 702b - Segment_Tx1H
	// 702c - Segment_Tx2H
	// 702d - Segment_spH
	// 702e - Segment_Tx3H
	//
	// 7030 - Fade_Control
	// 
	// 703a - Palette_Control
	// 
	// 7042 - SControl
	//
	// 7050 - TFT_Ctrl
	// 7051 - TFT_V_Width
	// 7052 - TFT_VSync_Setup
	// 7053 - TFT_V_Start
	// 7054 - TFT_V_End
	// 7055 - TFT_H_Width
	// 7056 - TFT_HSync_Setup
	// 7057 - TFT_H_Start
	// 7058 - TFT_H_End
	// 7059 - TFT_RGB_Ctrl
	// 705a - TFT_Status
	// 705b - TFT_MemMode_WCmd
	// 705c - TFT_MemMode_RCmd
	//
	// 705e - STN_PIC_SEG
	// 705f - STN_Ctrl1
	// 
	// 7062 - TFT_INT_EN
	// 7063 - TFT_INT_CLR
	// 7064 - US_Ctrl
	// 7065 - US_Hscaling
	// 7066 - US_Vscaling
	// 7067 - US_Width
	// 7068 - US_Height
	// 7069 - US_Hoffset 
	// 706a - US_Voffset
	//
	// 706c - TFT_V_Show_Start
	// 706d - TFT_V_Show_End
	// 706e - TFT_H_Show_Start
	// 706f - TFT_H_Show_End 
	// 
	// 7070 - SPDMA_Source
	// 7071 - SPDMA_Target
	// 7072 - SPDMA_Number
	// 7073 - HB_Ctrl
	// 7074 - HB_GO
	// 
	// 707d - BLD_Color 
	//
	// 707e - PPU_RAM_BANK
	// 707f - PPU_Enable 
	//
	// 7080 - STN_SEG
	// 7081 - STN_COM
	// 7082 - STN_PIC_COM
	// 7083 - STN_CPWait
	// 7084 - STN_Ctrl2
	// 7085 - STN_GTG_SEG
	// 7086 - STN_GTG_COM 
	// 
	// 70b4 - Tx1_N_PTRH
	// 70b5 - Tx1_A_PTRH
	// 70b6 - Tx2_N_PTRH
	// 70b7 - Tx2_A_PTRH
	// 70b8 - Tx3_N_PTRH
	// 70b9 - Tx3_A_PTRH
	//
	// 70db - Free_Height
	// 70dc - Free_Width
	//
	// 70e0 - Random0 (15-bit)
	// 70e1 - Random1 (15-bit)
	// 
	// 7100 to 71ff - Tx_Hvoffset
	// 7200 to 72ff - HCMValue
	// 7300 to 73ff - Palette (banked)
	// 7400 to 74ff - Sprite Ram (banked)
	//
	// 7800 - BodyID
	// 7801 - unused
	// 7802 - PwrKey_State
	// 7803 - SYS_CTRL
	// 7804 - CLK_Ctrl0
	// 7805 - CLK_Ctrl1
	// 7806 - Reset_Flag
	// 7807 - Clock_Ctrl
	// 7808 - LVR_Ctrl
	// 7809 - PM_Ctrl
	// 780a - Watchdog_Ctrl 
	map(0x00780b, 0x00780b).nopw(); // Watchdog_Clear
	// 780c - WAIT
	// 780d - HALT
	// 780e - unused
	// 780f - Power_State
	// 7810 - BankSwitch
	// 7811 - unused
	// 7812 - unused
	// 7813 - unused
	// 7814 - unused
	// 7815 - unused
	// 7816 - unused
	// 7817 - PLL_Sel
	// 7818 - PLLWaitCLK
	// 7819 - Cache_Ctrl
	// 781a - Cache_HitRate
	// 781b - unused
	// 781c - unused
	// 781d - unused
	// 781e - unused
	// 781f - SYS_Misc

	// 7825 - Unexpect_Flag

	// 7830 - CHECKSUM0_LB
	// 7831 - CHECKSUM1_LB
	// 7832 - CHECKSUM0_HB
	// 7833 - CHECKSUM1_HB
	// 
	// 7848 - ECC_LPRL_HB
	// 7849 - ECC_LPRH_HB
	// 784a - ECC_CPR_HB
	// 784b - ECC_LPR_CKL_HB
	// 784c - ECC_LPR_CKH_HB
	// 784d - ECC_CPCKR_HB
	// 784e - ECC_ERR0_HB
	// 784f - ECC_ERR1_HB

	// 7850 - NF_Ctrl
	// 7851 - NF_CMD
	// 7852 - NF_AddrL
	// 7853 - NF_AddrH
	// 7854 - NF_Data
	// 7855 - NF_INT_Ctrl
	// 7856 - unused            or BCH_Control 
	// 7857 - ECC_Ctrl
	// 7858 - ECC_LPRL_LB     or BCH_Error
	// 7859 - ECC_LPRH_LB     or BCH_Parity0 
	// 785a - ECC_CPR_LB      or BCH_Parity1
	// 785b - ECC_LPR_CKL_LB  or BCH_Parity2
	// 785c - ECC_LPR_CKH_LB  or BCH_Parity3
	// 785d - ECC_CPCKR_LB    or BCH_Parity4
	// 785e - ECC_ERR0_LB     or BCH_Parity5
	// 785f - ECC_ERR1_LB     or BCH_Parity6

	// 7860 - IOA_Data
	// 7861 - IOA_Buffer
	// 7862 - IOA_Dir
	// 7863 - IOA_Attrib
	// 7864 - IOA_Drv
	// 7865 - IOA_Mux
	// 7866 - IOA_Latch
	// 7867 - IOA_KeyEN

	// 7868 - IOB_Data
	// 7869 - IOB_Buffer
	// 786a - IOB_Dir
	// 786b - IOB_Attrib
	// 786c - IOB_Drv
	// 786d - IOB_Mux
	// 786e - IOB_Latch
	// 786f - IOB_KeyEN

	// 7870 - IOC_Data
	// 7871 - IOC_Buffer
	// 7872 - IOC_Dir
	// 7873 - IOC_Attrib
	// 7874 - IOC_Drv
	// 7875 - IOC_Mux
	// 7876 - IOC_Latch
	// 7877 - IOC_KeyEN

	// 7878 - IOD_Data
	// 7879 - IOD_Buffer
	// 787a - IOD_Dir
	// 787b - IOD_Attrib
	// 787c - IOD_Drv
	// 787d - IOD_Mux

	// 7880 - IOE_Data
	// 7881 - IOE_Buffer
	// 7882 - IOE_Dir
	// 7883 - IOE_Attrib
	// 7884 - IOE_Drv
	// 7885 - IOE_Mux
	// 7886 - IOE_Latch
	// 7877 - IOE_KeyEN

	// 7888 - IOF_Data
	// 7889 - IOF_Buffer
	// 788a - IOF_Dir
	// 788b - IOF_Attrib
	// 788c - IOF_Drv
	// 788d - IOF_Mux
	// 788e - IOF_Latch
	// 788f - IOF_KeyEN

	// 78a0 - INT_Status1
	// 78a1 - INT_Status2
	// 78a2 - INT_Status3
	// 78a3 - INT_Priority1
	// 78a4 - INT_Priority2
	// 78a5 - INT_Priority3
	// 78a6 - MINT_Ctrl
	// 78a7 - IOAB_KCIEN
	// 78a8 - IOC_KCIEN
	// 78a9 - IOE_KCIEN
	// 78aa - IOF_KCIEN
	// 78ab - IOAB_KCIFC
	// 78ac - IOC_ KCIFC
	// 78ad - IOE_ KCIFC
	// 78ae - IOF_ KCIFC

	// 78b0 - TimeBaseA_Ctrl
	// 78b1 - TimeBaseB_Ctrl
	// 78b2 - TimeBaseC_Ctrl

	// 78b8 - TimeBase_Reset 

	// 78c0 - I2C_Ctrl
	// 78c1 - I2C_Status
	// 78c2 - I2C_Address
	// 78c3 - I2C_Data
	// 78c4 - I2C_Debounce
	// 78c5 - I2C_Clk
	// 78c6 - I2C_MISC

	// 78e0 - TimerG_Ctrl
	// 78e1
	// 78e2 - TimerG_Preload
	// 78e3
	// 78e4 - TimerG_UpCount
	// 78e5
	// 78e6
	// 78e7
	map(0x0078e8, 0x0078e8).rw(FUNC(generalplus_gpl951xx_device::spi_direct_78e8_r), FUNC(generalplus_gpl951xx_device::spi_direct_78e8_w)); // TimerH_Ctrl
	// 78e9
	// 78ea - TimerH_Preload
	// 78eb
	// 78ec - TimerH_UpCount
	// 78ed
	// 78ee
	// 78ef

	// 78f0 - CHA_Ctrl 
	// 78f1 - CHA_Data
	// 78f2 - CHA_FIFO
	// 78f3
	// 78f4
	// 78f5
	// 78f6
	// 78f7
	// 78f8 - CHB_Ctrl
	// 78f9 - CHB_Data
	// 78fa - CHB_FIFO
	// 78fb
	// 78fc
	// 78fd
	// 78fe
	// 78ff

	// 7900 - UART_Data
	// 7901 - UART_RXStatus
	// 7902 - UAR_Ctrl
	// 7903 - UART_BaudRate
	// 7904 - UART_Status
	// 7905 - UART_FIFO
	// 7906 - UART_TXDelay

	// 7920 - SPI1_Ctrl
	// 7921 - SPI1_TXStatus
	// 7922 - SPI1_TXData
	// 7923 - SPI1_RXStatus
	// 7924 - SPI1_RXData
	// 7925 - SPI1_Misc

	// 7940 - SPI0_Ctrl
	// 7941 - SPI0_TXStatus
	// 7942 - SPI0_TXData
	// 7943 - SPI0_RXStatus
	// 7944 - SPI0_RXData
	// 7945 - SPI0_Misc

	// 79a0 - ADC_Setup
	// 79a1 - MADC_Ctrl
	// 79a2 - MADC_Data
	// 79a3 - ASADC_Ctrl
	// 79a4 - ASDAC_Data
	// 79a5
	// 79a6 - ADC_LineCH_En
	// 79a7 - ADC_SH_Wait

	// 79b0 - MICADC_Setup
	// 79b1 - MICGAIN_Ctrl
	// 79b2
	// 79b3 - ASMICADC_Ctrl
	// 79b4 - ASMICDAC_Data
	// 79b5 - MICAGC_UpThres
	// 79b6
	// 79b7 - MICADC_SH_WAIT
	// 79b8 - MICADC_DataMAX
	// 79b9 - MICADC_DataMIN
	// 79ba - MICADC_FLAG
	// 79bb - MICADC_GAIN
	// 79bc - MICAGC_Ctrl
	// 79bd - MICAGC_Time
	// 79be - MICAGC_Enable
	// 79bf - MICAGC_Status

	// 79f0 - RTC_Ctrl
	// 79f1 - RTC_Addr
	// 79f2 - RTC_WriteData
	// 79f3 - RTC_Request
	map(0x0079f4, 0x0079f4).r(FUNC(generalplus_gpl951xx_device::spi_direct_79f4_r)); // RTC_Ready
	map(0x0079f5, 0x0079f5).r(FUNC(generalplus_gpl951xx_device::spi_direct_79f5_r)); // RTC_ReadData
	// 79f6
	// 79f7
	// 79f8
	// 79fa
	// 79fb - RTC_ClkDiv

	// 7a00 - TimerA_Ctr
	// 7a01 - TimerA_CCPB_Ctrl
	// 7a02 - TimerA_Preload
	// 7a03 - TimerA_CCPB_Reg
	// 7a04 - TimerA_UpCount

	// 7a08 - TimerB_Ctrl
	// 7a09 - TimerB_CCPB_Ctrl
	// 7a0a - TimerB_Preload
	// 7a0b - TimerB_CCPB_Reg
	// 7a0c - TimerB_UpCount

	// 7a10 - TimerC_Ctrl
	// 7a11 - TimerC_CCPB_Ctrl
	// 7a12 - TimerC_Preload
	// 7a13 - TimerC_CCPB_Reg
	// 7a14 - TimerC_UpCount

	// 7a18 - TimerD_Ctrl
	// 7a19 - TimerD_CCPB_Ctrl
	// 7a1a - TimerD_Preload
	// 7a1b - TimerD_CCPB_Reg
	// 7a1c - TimerD_UpCount

	// 7a20 - TimerE_Ctrl
	// 7a21 - TimerF_Ctrl
	// 7a22 - TimerE_CCPB_Ctrl
	// 7a23 - TimerF_CCPB_Ctrl
	// 7a24 - TimerE_Preload
	// 7a25 - TimerF_Preload
	// 7a26 - TimerEF_CCPB4_Reg
	// 7a27 - TimerEF_CCPB5_Reg
	// 7a28 - TimerEF_CCPB6_Reg
	// 7a29 - TimerEF_CCPB7_Reg
	// 7a2a - TimerE_UpCount
	// 7a2b - TimerF_UpCount
	// 7a2c - TimerEF_CCPB_Se

	// 7a40 - USBD_Config
	// 7a41 - USBD_Function
	// 7a42 - USBD_PMR
	// 7a43 - USBD_EP0Data
	// 7a44 - USBD_BIData
	// 7a45 - USBD_BOData
	// 7a46 - USBD_INTINData
	// 7a47 - USBD_EPEvent
	// 7a48 - USBD_GLOINT
	// 7a49 - USBD_INTEN
	// 7a4a - USBD_INT
	// 7a4b - USBD_SCI NTEN
	// 7a4c - USBD_SCINT
	// 7a4d - USBD_EPAutoSet
	// 7a4e - USBD_EPSetStall
	// 7a4f - USBD_EPBufClear
	// 7a50 - USBD_EPEvntClear
	// 7a51 - USBD_EP0WrtCount
	// 7a52 - USBD_BOWrtCount
	// 7a53 - USBD_EP0BufPointer
	// 7a54 - USBD_BIBufPointer
	// 7a55 - USBD_BOBufPointer
	// 7a56 - USBD_EP0RTR
	// 7a57 - USBD_EP0RR
	// 7a58 - USBD_ EP0VR
	// 7a59 - USBD_ EP0IR
	// 7a5a - USBD_ EP0LR
	// 
	// 7a60 - USBD_DMAWrtCountL
	// 7a61 - USBD_DMAWrtCountH
	// 7a62 - USBD_DMAAckL
	// 7a63 - USBD_DMAAckH
	// 7a64 - USBD_EPStall
	//
	// 7a67 - USBD_Device
	// 7a68 - USBD_NullPkt
	// 7a69 - USBD_DMAINT
	//
	// 7a6c - USBD_INTF

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
	//
	// 7ab0 - DMA_SPRISize0
	// 7ab1 - DMA_SPRISize1
	//
	// 7abd - DMA_LineLength
	// 7abe - DMA_SS
	// 7abf - DMA_INT
	//
	// 7ac0 - CTS_Ctrl1
	// 7ac1 - CTS_CH
	// 7ac2 - CTS_DIV
	// 7ac3 - CTS_CYCLE
	// 7ac4 - CTS_Ctrl2
	// 7ac5 - CTS_Status
	// 7ac6 - CTS_Ctrl3
	// 
	// 7ac8 - CTS_FIFOLevel
	// 7ac9 - CTS_CNT

	map(0x007af0, 0x007af0).rw(FUNC(generalplus_gpl951xx_device::spi_direct_7af0_r), FUNC(generalplus_gpl951xx_device::spi_direct_7af0_w)); // Byte_Swap
	// 7af1 - Nibble_Swap
	// 7af2 - TwoBit_Swap
	// 7af3 - Bit_Reverse

	// 7b20 - KS_Ctrl1
	// 7b21 - KS_Ctrl2
	//
	// 7b28 - KS_Data0
	// 7b29 - KS_Data1
	// 7b2a - KS_Data2
	// 7b2b - KS_Data3
	// 7b2c - KS_Data4
	// 7b2d - KS_Data5
	// 7b2e - KS_Data6
	// 7b2f - KS_Data7
	// 7b30 - KS_Data8
	// 7b31 - KS_Data9
	// 7b32 - KS_Data10

	map(0x007b40, 0x007b40).r(FUNC(generalplus_gpl951xx_device::spi_direct_7b40_r)).nopw();; // SPIFC_Ctrl1
	map(0x007b41, 0x007b41).nopw(); // SPIFC_CMD
	map(0x007b42, 0x007b42).nopw(); // SPIFC_PARA
	// 7b43 - SPIFC_ADDRL
	// 7b44 - SPIFC_ADDRH
	// 7b45 - SPIFC_TX_Dat
	map(0x007b46, 0x007b46).ram(); // SPIFC_RX_Data - values must be written and read from here, but is there any transformation?
	map(0x007b47, 0x007b47).nopw(); // SPIFC_TX_BC
	map(0x007b48, 0x007b48).nopw(); // SPIFC_RX_BC
	map(0x007b49, 0x007b49).ram(); // SPIFC_TIMING

	// 7b4b - SPIFC_Ctrl2

	// 7b80 to 7b9f - Audio
	// 7c00 to 7cff - Audio
	// 7e00 to 7eff - Audio

	// 8000 - 8fff internal boot ROM (same on all devices of the same type, not OTP)

	map(0x009000, 0x3fffff).rom().region("spidirect", 0);
}


DEFINE_DEVICE_TYPE(GPL951XX, generalplus_gpl951xx_device, "gpl951xx", "GeneralPlus GPL951xx")

generalplus_gpl951xx_device::generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GPL951XX, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl951xx_device::gpspi_direct_internal_map), this))
{
}

