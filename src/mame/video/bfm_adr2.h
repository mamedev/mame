#ifndef INC_BFMADDER2
#define INC_BFMADDER2

extern UINT8 adder2_data_from_sc2;	// data available for adder from sc2
extern UINT8 adder2_sc2data;			// data
extern UINT8 adder2_data_to_sc2;	// data available for sc2 from adder
extern UINT8 adder2_data;			// data

extern int adder2_acia_triggered;	// flag <>0, ACIA receive IRQ

GFXDECODE_EXTERN( adder2 );
extern void adder2_decode_char_roms(void);

MACHINE_RESET( adder2 );
INTERRUPT_GEN( adder2_vbl );

ADDRESS_MAP_EXTERN( adder2_memmap, 8 );

VIDEO_START(  adder2 );
VIDEO_RESET(  adder2 );
VIDEO_UPDATE( adder2 );
PALETTE_INIT( adder2 );

#endif
