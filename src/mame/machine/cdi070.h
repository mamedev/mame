/******************************************************************************


    CD-i-specific SCC68070 SoC peripheral emulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Skeleton.  Just enough for the CD-i to run.

TODO:

- Proper handling of the 68070's internal devices (UART, DMA, Timers, etc.)

*******************************************************************************/

#ifndef _MACHINE_CDI070_H_
#define _MACHINE_CDI070_H_

#include "emu.h"

struct scc68070_i2c_regs_t
{
	UINT8 reserved0;
	UINT8 data_register;
	UINT8 reserved1;
	UINT8 address_register;
	UINT8 reserved2;
	UINT8 status_register;
	UINT8 reserved3;
	UINT8 control_register;
	UINT8 reserved;
	UINT8 clock_control_register;
};

#define ISR_MST		0x80	// Master
#define ISR_TRX		0x40	// Transmitter
#define ISR_BB		0x20	// Busy
#define ISR_PIN		0x10	// No Pending Interrupt
#define ISR_AL		0x08	// Arbitration Lost
#define ISR_AAS		0x04	// Addressed As Slave
#define ISR_AD0		0x02	// Address Zero
#define ISR_LRB		0x01	// Last Received Bit

struct scc68070_uart_regs_t
{
	UINT8 reserved0;
	UINT8 mode_register;
	UINT8 reserved1;
	UINT8 status_register;
	UINT8 reserved2;
	UINT8 clock_select;
	UINT8 reserved3;
	UINT8 command_register;
	UINT8 reserved4;
	UINT8 transmit_holding_register;
	UINT8 reserved5;
	UINT8 receive_holding_register;

	INT16 receive_pointer;
	UINT8 receive_buffer[32768];
	emu_timer* rx_timer;

	INT16 transmit_pointer;
	UINT8 transmit_buffer[32768];
	emu_timer* tx_timer;
};

#define UMR_OM			0xc0
#define UMR_OM_NORMAL	0x00
#define UMR_OM_ECHO		0x40
#define UMR_OM_LOOPBACK	0x80
#define UMR_OM_RLOOP	0xc0
#define UMR_TXC			0x10
#define UMR_PC			0x08
#define UMR_P			0x04
#define UMR_SB			0x02
#define UMR_CL			0x01

#define USR_RB			0x80
#define USR_FE			0x40
#define USR_PE			0x20
#define USR_OE			0x10
#define USR_TXEMT		0x08
#define USR_TXRDY		0x04
#define USR_RXRDY		0x01

struct scc68070_timer_regs_t
{
	UINT8 timer_status_register;
	UINT8 timer_control_register;
	UINT16 reload_register;
	UINT16 timer0;
	UINT16 timer1;
	UINT16 timer2;
	emu_timer* timer0_timer;
};

#define TSR_OV0			0x80
#define TSR_MA1			0x40
#define TSR_CAP1		0x20
#define TSR_OV1			0x10
#define TSR_MA2			0x08
#define TSR_CAP2		0x04
#define TSR_OV2			0x02

#define TCR_E1			0xc0
#define TCR_E1_NONE		0x00
#define TCR_E1_RISING	0x40
#define TCR_E1_FALLING	0x80
#define TCR_E1_BOTH		0xc0
#define TCR_M1			0x30
#define TCR_M1_NONE		0x00
#define TCR_M1_MATCH	0x10
#define TCR_M1_CAPTURE	0x20
#define TCR_M1_COUNT	0x30
#define TCR_E2			0x0c
#define TCR_E2_NONE		0x00
#define TCR_E2_RISING	0x04
#define TCR_E2_FALLING	0x08
#define TCR_E2_BOTH		0x0c
#define TCR_M2			0x03
#define TCR_M2_NONE		0x00
#define TCR_M2_MATCH	0x01
#define TCR_M2_CAPTURE	0x02
#define TCR_M2_COUNT	0x03

struct scc68070_dma_channel_t
{
	UINT8 channel_status;
	UINT8 channel_error;

	UINT8 reserved0[2];

	UINT8 device_control;
	UINT8 operation_control;
	UINT8 sequence_control;
	UINT8 channel_control;

	UINT8 reserved1[3];

	UINT16 transfer_counter;

	UINT32 memory_address_counter;

	UINT8 reserved2[4];

	UINT32 device_address_counter;

	UINT8 reserved3[40];
};

#define CSR_COC			0x80
#define CSR_NDT			0x20
#define CSR_ERR			0x10
#define CSR_CA			0x08

#define CER_EC			0x1f
#define CER_NONE		0x00
#define CER_TIMING		0x02
#define CER_BUSERR_MEM	0x09
#define CER_BUSERR_DEV	0x0a
#define CER_SOFT_ABORT	0x11

#define DCR1_ERM		0x80
#define DCR1_DT			0x30

#define DCR2_ERM		0x80
#define DCR2_DT			0x30
#define DCR2_DS			0x08

#define OCR_D			0x80
#define OCR_D_M2D		0x00
#define OCR_D_D2M		0x80
#define OCR_OS			0x30
#define OCR_OS_BYTE		0x00
#define OCR_OS_WORD		0x10

#define SCR2_MAC		0x0c
#define SCR2_MAC_NONE	0x00
#define SCR2_MAC_INC	0x04
#define SCR2_DAC		0x03
#define SCR2_DAC_NONE	0x00
#define SCR2_DAC_INC	0x01

#define CCR_SO			0x80
#define CCR_SA			0x10
#define CCR_INE			0x08
#define CCR_IPL			0x07

struct scc68070_dma_regs_t
{
	scc68070_dma_channel_t channel[2];
};

struct scc68070_mmu_desc_t
{
	UINT16 attr;
	UINT16 length;
	UINT8  undefined;
	UINT8  segment;
	UINT16 base;
};

struct scc68070_mmu_regs_t
{
	UINT8 status;
	UINT8 control;

	UINT8 reserved[0x3e];

	scc68070_mmu_desc_t desc[8];
};

struct scc68070_regs_t
{
	UINT16 lir;
	UINT8 picr1;
	UINT8 picr2;

	scc68070_i2c_regs_t i2c;
	scc68070_uart_regs_t uart;
	scc68070_timer_regs_t timers;
	scc68070_dma_regs_t dma;
	scc68070_mmu_regs_t mmu;
};

// Member functions
TIMER_CALLBACK( scc68070_timer0_callback );
TIMER_CALLBACK( scc68070_rx_callback );
TIMER_CALLBACK( scc68070_tx_callback );
READ16_HANDLER( scc68070_periphs_r );
WRITE16_HANDLER( scc68070_periphs_w );
//READ16_HANDLER( uart_loopback_enable );

void scc68070_init(running_machine &machine, scc68070_regs_t *scc68070);
void scc68070_uart_rx(running_machine &machine, scc68070_regs_t *scc68070, UINT8 data);
void scc68070_uart_tx(running_machine &machine, scc68070_regs_t *scc68070, UINT8 data);
void scc68070_register_globals(running_machine &machine, scc68070_regs_t *scc68070);

// UART Access for Quizard
void scc68070_set_quizard_mcu_value(running_machine &machine, UINT16 value);
void scc68070_set_quizard_mcu_ack(running_machine &machine, UINT8 ack);
void scc68070_quizard_rx(running_machine &machine, scc68070_regs_t *scc68070, UINT8 data);

INTERRUPT_GEN( scc68070_mcu_frame );

#endif // _MACHINE_CDI070_H_
