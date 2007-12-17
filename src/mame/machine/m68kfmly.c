/******************************************************************************

    TMP68301 support (dummy) driver

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/
/******************************************************************************
Memo:

******************************************************************************/

#include "driver.h"
#include "m68kfmly.h"
#include "cpu/m68000/m68000.h"


#define	TMP68301_DEBUG	0


//  TMP68301 System Memory Map
//
//  0xFFFC00 - 0xFFFC0F TMP68301 Address decoder
//  0xFFFC80 - 0xFFFC9F TMP68301 Interrupt controller
//  0xFFFD00 - 0xFFFD0F TMP68301 Parallel interface
//  0xFFFD80 - 0xFFFDAF TMP68301 Serial interface
//  0xFFFE00 - 0xFFFE4F TMP68301 Timer
//
//  Address decoder
//          15      7
//  0xFFFC00    AMAR0       AAMR0
//  0xFFFC02    -       AACR0
//  0xFFFC04    AMAR1       AAMR1
//  0xFFFC06    -       AACR1
//  0xFFFC08    -       AACR2
//  0xFFFC0A    -       ATOR
//  0xFFFC0C        ARELR
//  0xFFFC0E    -       -
//
//  Interrupt controller
//          15      7
//  0xFFFC80    -       ICR0
//  0xFFFC82    -       ICR1
//  0xFFFC84    -       ICR2
//  0xFFFC86    -       ICR3
//  0xFFFC88    -       ICR4
//  0xFFFC8A    -       ICR5
//  0xFFFC8C    -       ICR6
//  0xFFFC8E    -       ICR7
//  0xFFFC90    -       ICR8
//  0xFFFC92    -       ICR9
//  0xFFFC94        IMR
//  0xFFFC96        IPR
//  0xFFFC98        IISR
//  0xFFFC9A    -       IVNR
//  0xFFFC9C    -       IEIR
//  0xFFFC9E    -       -
//
//  Parallel interface
//          15      7
//  0xFFFD00        PDIR
//  0xFFFD02    -       PCR
//  0xFFFD04    -       PSR
//  0xFFFD06    -       PCMR
//  0xFFFD08    -       PMR
//  0xFFFD0A        PDR
//  0xFFFD0C    -       PPR1
//  0xFFFD0E    -       PPR2
//
//  Serial interface
//          15      7
//  0xFFFD80    -       SMR0
//  0xFFFD82    -       SCMR0
//  0xFFFD84    -       SBRR0
//  0xFFFD86    -       SSR0
//  0xFFFD88    -       SDR0
//  0xFFFD8A    -       -
//  0xFFFD8C    -       SPR
//  0xFFFD8E    -       SCR
//  0xFFFD90    -       SMR1
//  0xFFFD92    -       SCMR1
//  0xFFFD94    -       SBRR1
//  0xFFFD96    -       SSR1
//  0xFFFD98    -       SDR1
//  0xFFFD9A    -       -
//  0xFFFD9C    -       -
//  0xFFFD9E    -       -
//  0xFFFDA0    -       SMR2
//  0xFFFDA2    -       SCMR2
//  0xFFFDA4    -       SBRR2
//  0xFFFDA6    -       SSR2
//  0xFFFDA8    -       SDR2
//  0xFFFDAA    -       -
//  0xFFFDAC    -       -
//  0xFFFDAE    -       -
//
//  Timer
//          15      7
//  0xFFFE00        TCR0
//  0xFFFE02    -       -
//  0xFFFE04        TMCR0
//  0xFFFE06    -       -
//  0xFFFE08    -       -
//  0xFFFE0A    -       -
//  0xFFFE0C        TCTR0
//  0xFFFE0E    -       -
//  0xFFFE10    -       -
//  0xFFFE12    -       -
//  0xFFFE14    -       -
//  0xFFFE16    -       -
//  0xFFFE18    -       -
//  0xFFFE1A    -       -
//  0xFFFE1C    -       -
//  0xFFFE1E    -       -
//  0xFFFE20        TCR1
//  0xFFFE22    -       -
//  0xFFFE24        TMCR11
//  0xFFFE26    -       -
//  0xFFFE28        TMCR12
//  0xFFFE2A    -       -
//  0xFFFE2C        TCTR1
//  0xFFFE2E    -       -
//  0xFFFE30    -       -
//  0xFFFE32    -       -
//  0xFFFE34    -       -
//  0xFFFE36    -       -
//  0xFFFE38    -       -
//  0xFFFE3A    -       -
//  0xFFFE3C    -       -
//  0xFFFE3E    -       -
//  0xFFFE40        TCR2
//  0xFFFE42    -       -
//  0xFFFE44        TMCR21
//  0xFFFE46    -       -
//  0xFFFE48        TMCR22
//  0xFFFE4A    -       -
//  0xFFFE4C        TCTR2
//  0xFFFE4E    -       -


static UINT16 tmp68301_address_decoder[0x10];
static UINT16 tmp68301_interrupt_controller[0x20];
static UINT16 tmp68301_parallel_interface[0x10];
static UINT16 tmp68301_serial_interface[0x30];
static UINT16 tmp68301_timer[0x50];


READ16_HANDLER( tmp68301_address_decoder_r )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_address_decoder_r (%08X)\n", activecpu_get_pc(), (0xfffc00 + (offset * 2)));
#endif

	return tmp68301_address_decoder[offset];
}

WRITE16_HANDLER( tmp68301_address_decoder_w )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_address_decoder_w (%08X = %04X)\n", activecpu_get_pc(), (0xfffc00 + (offset * 2)), data);
#endif

	tmp68301_address_decoder[offset] = data;
}

READ16_HANDLER( tmp68301_interrupt_controller_r )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_interrupt_controller_r (%08X)\n", activecpu_get_pc(), (0xfffc80 + (offset * 2)));
#endif

	return tmp68301_interrupt_controller[offset];
}

WRITE16_HANDLER( tmp68301_interrupt_controller_w )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_interrupt_controller_w (%08X = %04X)\n", activecpu_get_pc(), (0xfffc80 + (offset * 2)), data);
#endif

	tmp68301_interrupt_controller[offset] = data;
}

READ16_HANDLER( tmp68301_parallel_interface_r )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_parallel_interface_r (%08X)\n", activecpu_get_pc(), (0xfffd00 + (offset * 2)));
#endif

	return tmp68301_parallel_interface[offset];
}

WRITE16_HANDLER( tmp68301_parallel_interface_w )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_parallel_interface_w (%08X = %04X)\n", activecpu_get_pc(), (0xfffd00 + (offset * 2)), data);
#endif

	tmp68301_parallel_interface[offset] = data;
}

READ16_HANDLER( tmp68301_serial_interface_r )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_serial_interface_r (%08X)\n", activecpu_get_pc(), (0xfffd80 + (offset * 2)));
#endif

	return tmp68301_serial_interface[offset];
}

WRITE16_HANDLER( tmp68301_serial_interface_w )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_serial_interface_w (%08X = %04X)\n", activecpu_get_pc(), (0xfffd80 + (offset * 2)), data);
#endif

	tmp68301_serial_interface[offset] = data;
}

READ16_HANDLER( tmp68301_timer_r )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_timer_r (%08X)\n", activecpu_get_pc(), (0xfffe00 + (offset * 2)));
#endif

	return tmp68301_timer[offset];
}

WRITE16_HANDLER( tmp68301_timer_w )
{
#if TMP68301_DEBUG
	logerror("PC %08X: TMP68301_timer_w (%08X = %04X)\n", activecpu_get_pc(), (0xfffe00 + (offset * 2)), data);
#endif

	tmp68301_timer[offset] = data;
}
