#ifndef INC_BFMADDER2
#define INC_BFMADDER2

int adder2_receive(void);
void adder2_send(int data);
int adder2_status(void);

GFXDECODE_EXTERN( adder2 );
void adder2_decode_char_roms(running_machine &machine);

MACHINE_RESET( adder2 );
INTERRUPT_GEN( adder2_vbl );

ADDRESS_MAP_EXTERN( adder2_memmap, 8 );

VIDEO_START(  adder2 );
VIDEO_RESET(  adder2 );
SCREEN_UPDATE( adder2 );
PALETTE_INIT( adder2 );

#endif
