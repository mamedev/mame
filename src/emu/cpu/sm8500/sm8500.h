#ifndef sm8500_H
#define sm8500_H
#include "cpuintrf.h"

typedef struct {
	void (*handle_dma)(int cycles);
	void (*handle_timers)(int cycles);
} SM8500_CONFIG;

/* interrupts */
#define ILL_INT         0
#define DMA_INT         1
#define TIM0_INT        2
#define EXT_INT         3
#define UART_INT        4
#define LCDC_INT        5
#define TIM1_INT        6
#define CK_INT          7
#define PIO_INT         8
#define WDT_INT         9
#define NMI_INT         10

extern int sm8500_icount;

enum {
	/* "main" 16 bit register */
        SM8500_PC=1, SM8500_SP, SM8500_PS, SM8500_SYS16, SM8500_RR0, SM8500_RR2, SM8500_RR4, SM8500_RR6, SM8500_RR8, SM8500_RR10,
	SM8500_RR12, SM8500_RR14,
	/* additional internal 8 bit registers */
	SM8500_IE0, SM8500_IE1, SM8500_IR0, SM8500_IR1, SM8500_P0, SM8500_P1, SM8500_P2, SM8500_P3, SM8500_SYS, SM8500_CKC,
	SM8500_SPH, SM8500_SPL, SM8500_PS0, SM8500_PS1, SM8500_P0C, SM8500_P1C, SM8500_P2C, SM8500_P3C,
};

extern UINT8* sm8500_internal_ram( void ) ;
extern unsigned sm8500_get_reg( int regnum );
extern void sm8500_get_info(UINT32 state, cpuinfo *info);

UINT8 sm85cpu_mem_readbyte( UINT32 offset );
void sm85cpu_mem_writebyte( UINT32 offset, UINT8 data );

INLINE UINT16 sm85cpu_mem_readword( UINT32 address )
{
	UINT16 value = (UINT16) sm85cpu_mem_readbyte( address ) << 8;
	value |= sm85cpu_mem_readbyte( ( address + 1 ) & 0xffff );
	return value;
}

INLINE void sm85cpu_mem_writeword( UINT32 address, UINT16 value )
{
	sm85cpu_mem_writebyte( address, value >> 8 );
	sm85cpu_mem_writebyte( ( address + 1 ) & 0xffff, value & 0xff );
}

#ifdef MAME_DEBUG
extern unsigned sm8500_dasm( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram );
#endif /* MAME_DEBUG */

#endif

