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

	map(0x007af0, 0x007af0).rw(FUNC(generalplus_gpl951xx_device::spi_direct_7af0_r), FUNC(generalplus_gpl951xx_device::spi_direct_7af0_w));

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

	map(0x009000, 0x3fffff).rom().region("spidirect", 0);
}


DEFINE_DEVICE_TYPE(GPL951XX, generalplus_gpl951xx_device, "gpl951xx", "GeneralPlus GPL951xx")

generalplus_gpl951xx_device::generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sunplus_gcm394_base_device(mconfig, GPL951XX, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl951xx_device::gpspi_direct_internal_map), this))
{
}

