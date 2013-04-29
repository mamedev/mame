/* Modern device for the MCF5206e Peripherals
 this can be hooked properly to the CPU once the CPU is a modern device too
*/

#include "emu.h"
#include "mcf5206e.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type MCF5206E_PERIPHERAL = &device_creator<mcf5206e_peripheral_device>;

//-------------------------------------------------
//  mcf5206e_peripheral_device - constructor
//-------------------------------------------------

mcf5206e_peripheral_device::mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MCF5206E_PERIPHERAL, "MCF5206E Peripheral", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mcf5206e_peripheral_device::device_config_complete()
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mcf5206e_peripheral_device::device_start()
{

}



READ32_MEMBER(mcf5206e_peripheral_device::dev_r)
{
	return 0;
}

WRITE32_MEMBER(mcf5206e_peripheral_device::dev_w)
{

}


// ColdFire peripherals

enum {
	CF_PPDAT    =   0x1c8/4,
	CF_MBSR     =   0x1ec/4
};

WRITE32_MEMBER(mcf5206e_peripheral_device::seta2_coldfire_regs_w)
{
	COMBINE_DATA( &m_coldfire_regs[offset] );
}

READ32_MEMBER(mcf5206e_peripheral_device::seta2_coldfire_regs_r)
{
	switch( offset )
	{
		case CF_MBSR:
			return machine().rand();

		case CF_PPDAT:
			return ioport(":BATTERY")->read() << 16;
	}

	return m_coldfire_regs[offset];
}

/*

ADDRESS                 REG         WIDTH   NAME/DESCRIPTION                                    INIT VALUE (MR=Master Reset, NR=Normal Reset)       Read or Write access

op MOVEC with $C0F		MBAR		32		Module Base Address Register						uninit (except V=0)									W
$003					SIMR		8		SIM Configuration Register							C0													R/W
$014					ICR1		8		Interrupt Control Register 1 - External IRQ1/IPL1	04													R/W
$015					ICR2		8		Interrupt Control Register 2 - External IPL2		08													R/W
$016					ICR3		8		Interrupt Control Register 3 - External IPL3		0C													R/W
$017					ICR4		8		Interrupt Control Register 4 - External IRQ4/IPL4	10													R/W
$018					ICR5		8		Interrupt Control Register 5 - External IPL5		14													R/W
$019					ICR6		8		Interrupt Control Register 6 - External IPL6		18													R/W
$01A					ICR7		8		Interrupt Control Register 7 - External IRQ7/IPL7	1C													R/W
$01B					ICR8		8		Interrupt Control Register 8 - SWT					1C													R/W
$01C					ICR9		8		Interrupt Control Register 9 - Timer 1 Interrupt	80													R/W
$01D					ICR10		8		Interrupt Control Register 10 - Timer 2 Interrupt	80													R/W
$01E					ICR11		8		Interrupt Control Register 11 - MBUS Interrupt		80													R/W
$01F					ICR12		8		Interrupt Control Register 12 - UART 1 Interrupt	00													R/W
$020					ICR13		8		Interrupt Control Register 13 - UART 2 Interrupt	00													R/W
$036					IMR			16		Interrupt Mask Register								3FFE												R/W
$03A					IPR			16		Interrupt Pending Register							0000												R
$040					RSR			8		Reset Status Register								80 / 20												R/W
$041					SYPCR		8		System Protection Control Register					00													R/W
$042					SWIVR		8		Software Watchdog Interrupt Vector Register			0F													R/W
$043					SWSR		8		Software Watchdog Service Register					uninit												W
$046					DCRR		16		DRAM Controller Refresh								MR 0000   - NR uninit								R/W
$04A					DCTR		16		DRAM Controller Timing Register						MR 0000   - NR uninit								R/W
$04C					DCAR0		16		DRAM Controller 0 Address Register					MR uninit - NR uninit								R/W
$050					DCMR0		32		DRAM Controller 0 Mask Register						MR uninit - NR uninit								R/W
$057					DCCR0		8		DRAM Controller 0 Control Register					MR 00     - NR 00									R/W
$058					DCAR1		16		DRAM Controller 1 Address Register					MR uninit - NR uninit								R/W
$05C					DCMR1		32		DRAM Controller 1 Mask Register						MR uninit - NR uninit								R/W
$063					DCCR1		8		DRAM Controller 1 Control Register					MR 00     - NR 00									R/W
--------- CHIP SELECTS -----------
$064					CSAR0		16		Chip-Select 0 Address Register						0000												R/W
$068					CSMR0		32		Chip-Select 0 Mask Register							00000000											R/W
$06E					CSCR0		16		Chip-Select 0 Control Register						3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF		R/W
                                                                                                AA set by IRQ 7 at reset									
																								PS1 set by IRQ 4 at reset
																								PS0 set by IRQ 1 at reset		
$070					CSAR1		16		Chip-Select 1 Address Register						uninit												R/W
$074					CSMR1		32		Chip-Select 1 Mask Register							uninit												R/W
$07A					CSCR1		16		Chip-Select 1 Control Register						uninit *1											R/W
$07C					CSAR2		16		Chip-Select 2 Address Register						uninit												R/W
$080					CSMR2		32		Chip-Select 2 Mask Register							uninit												R/W
$086					CSCR2		16		Chip-Select 2 Control Register						uninit *1											R/W
$088					CSAR3		16		Chip-Select 3 Address Register						uninit												R/W
$08C					CSMR3		32		Chip-Select 3 Mask Register							uninit												R/W
$092					CSCR3		16		Chip-Select 3 Control Register						uninit *1											R/W
$094					CSAR4		16		Chip-Select 4 Address Register						uninit												R/W
$098					CSMR4		32		Chip-Select 4 Mask Register							uninit												R/W
$09E					CSCR4		16		Chip-Select 4 Control Register						uninit *1											R/W
$0A0					CSAR5		16		Chip-Select 5 Address Register						uninit												R/W
$0A4					CSMR5		32		Chip-Select 5 Mask Register							uninit												R/W
$0AA					CSCR5		16		Chip-Select 5 Control Register						uninit *1											R/W
$0AC					CSAR6		16		Chip-Select 6 Address Register						uninit												R/W
$0B0					CSMR6		32		Chip-Select 6 Mask Register							uninit												R/W
$0B6					CSCR6		16		Chip-Select 6 Control Register						uninit *1											R/W
$0B8					CSAR7		16		Chip-Select 7 Address Register						uninit												R/W
$0BC					CSMR7		32		Chip-Select 7 Mask Register							uninit												R/W
$0C2					CSCR7		16		Chip-Select 7 Control Register						uninit *1											R/W
$0C6					DMCR		16		Default Memory Control Register						0000												R/W
$0CA					PAR			16		Pin Assignment Register								00													R/W
--------- TIMER MODULE -----------
$100					TMR1		16		Timer 1 Mode Register								0000												R/W
$104					TRR1		16		Timer 1 Reference Register							FFFF												R/W
$108					TCR1		16		Timer 1 Capture Register							0000												R
$10C					TCN1		16		Timer 1 Counter										0000												R/W
$111					TER1		8		Timer 1 Event Register								00													R/W
$120					TMR2		16		Timer 2 Mode Register								0000												R/W
$124					TRR2		16		Timer 2 Reference Register							FFFF												R/W
$128					TCR2		16		Timer 2 Capture Register							0000												R
$12C					TCN2		16		Timer 2 Counter										0000												R/W
$131					TER2		8		Timer 2 Event Register								00													R/W
------------ UART SERIAL PORTS  -----------
$140					UMR1,2		8		UART 1 Mode Registers								00													R/W
$144					USR			8		UART 1 Status Register								00		 											R
						UCSR		8		UART 1 Clock-Select Register						DD													W
$148					UCR			8		UART 1 Command Register								00													W
$14C					URB			8		UART 1 Receive Buffer								FF													R
						UTB			8		UART 1 Transmit Buffer								00													W
$150					UIPCR		8		UART Input Port Change Register						0F													R
						UACR		8		UART 1 Auxilary Control Register					00													W
$154					UISR		8		UART 1 Interrupt Status Register					00													R
						UIMR		8		UART 1 Interrupt Mask Register						00													W
$158					UBG1		8		UART 1 Baud Rate Generator Prescale MSB				uninit												W
$15C					UBG2		8		UART 1 Baud Rate Generator Prescale LSB				uninit												W
$170					UIVR		8		UART 1 Interrupt Vector Register					0F													R/W
$174					UIP			8		UART 1 Input Port Register							FF													R
$178					UOP1		8		UART 1 Output Port Bit Set CMD						UOP1[7-1]=undef; UOP1=0								W
$17C					UOP0		8		UART 1 Output Port Bit Reset CMD					uninit												W

$180					UMR1,2		8		UART 2 Mode Registers								00													R/W
$184					USR			8		UART 2 Status Register								00													R
						UCSR		8		UART 2 Clock-Select Register						DD													W
$188					UCR			8		UART 2 Command Register								00													W
$18C					URB			8		UART 2 Receive Buffer								FF													R
						UTB			8		UART 2 Transmit Buffer								00													W
$190					UIPCR		8		UART 2 Input Port Change Register					0F													R
						UACR		8		UART 2 Auxilary Control Register					00													W
$194					UISR		8		UART 2 Interrupt Status Register					00													R
						UIMR		8		UART 2 Interrupt Mask Register						00													W
$198					UBG1		8		UART 2 Baud Rate Generator Prescale MSB				uninit												R/W
$19C					UBG2		8		UART 2 Barud Rate Generator Prescale LSB			uninit												R/W
$1B0					UIVR		8		UART 2 Interrupt Vector Register					0F													R/W
$1B4					UIP			8		UART 2 Input Port Register							FF													R
$1B8					UOP1		8		UART 2 Output Port Bit Set CMD						UOP1[7-1]=undef; UOP1=0								W
$1BC					UOP0		8		UART 2 Output Port Bit Reset CMD					uninit												W

$1C5					PPDDR		8		Port A Data Direction Register						00													R/W
$1C9					PPDAT		8		Port A Data Register								00													R/W
------------ MBUS  -----------
$1E0					MADR		8		M-Bus Address Register								00													R/W
$1E4					MFDR		8		M-Bus Frequency Divider Register					00													R/W
$1E8					MBCR		8		M-Bus Control Register								00													R/W
$1EC					MBSR		8		M-Bus Status Register								00													R/W
$1F0					MBDR		8		M-Bus Data I/O Register								00													R/W
------------ DMA Controller -----------
$200					DMASAR0		32		Source Address Register 0							00													R/W
$204					DMADAR0		32		Destination Address Register 0						00													R/W
$208					DCR0		16		DMA Control Register 0								00													R/W
$20C					BCR0		16		Byte Count Register 0								00													R/W
$210					DSR0		8		Status Register 0									00													R/W
$214					DIVR0		8		Interrupt Vector Register 0							0F													R/W
$240					DMASAR1		32		Source Address Register 1							00													R/W
$244					DMADAR1		32		Destination Address Register 1						00													R/W
$248					DCR1		16		DMA Control Register 1								00													R/W
$24C					BCR1		16		Byte Count Register 1								00													R/W
$250					DSR1		8		Status Register 1									00													R/W
$254					DIVR1		8		Interrupt Vector Register 1							0F													R/W

*1 - uninit except BRST=ASET=WRAH=RDAH=WR=RD=0

*/