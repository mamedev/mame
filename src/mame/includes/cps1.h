#ifndef _CPS1_H_
#define _CPS1_H_

/*----------- defined in drivers/cps1.c -----------*/

ADDRESS_MAP_EXTERN(qsound_map, 8);

READ16_HANDLER( qsound_sharedram1_r );
WRITE16_HANDLER( qsound_sharedram1_w );

READ16_HANDLER( cps1_eeprom_port_r );
WRITE16_HANDLER( cps1_eeprom_port_w );

INTERRUPT_GEN( cps1_qsound_interrupt );

extern const struct QSound_interface qsound_interface;

READ16_HANDLER( cps1_dsw_r );
READ16_HANDLER( cps1_in0_r );
READ16_HANDLER( cps1_in1_r );
READ16_HANDLER( cps1_in2_r );
READ16_HANDLER( cps1_in3_r );
WRITE16_HANDLER( cps1_coinctrl_w );
INTERRUPT_GEN( cps1_interrupt );

GFXDECODE_EXTERN( cps1 );


/*----------- defined in machine/cps2crpt.c -----------*/

DRIVER_INIT( cps2crpt );


/*----------- defined in machine/kabuki.c -----------*/

void wof_decode(void);
void dino_decode(void);
void punisher_decode(void);
void slammast_decode(void);


/*----------- defined in video/cps1.c -----------*/

extern int cps1_scanline1;
extern int cps1_scanline2;
extern int cps1_scancalls;

extern UINT16 *cps1_gfxram;     /* Video RAM */
extern UINT16 *cps1_output;     /* Output ports */
extern size_t cps1_gfxram_size;
extern size_t cps1_output_size;

extern UINT16 *cps1_other;
extern tilemap *cps1_bg_tilemap[3];

extern int cps1_scroll1x, cps1_scroll1y;
extern int cps1_scroll2x, cps1_scroll2y;
extern int cps1_scroll3x, cps1_scroll3y;

extern UINT16 *cps2_objram1,*cps2_objram2;
extern UINT16 *cps2_output;
extern size_t cps2_output_size;

int cps1_port(int offset);
READ16_HANDLER( cps1_output_r );
WRITE16_HANDLER( cps1_output_w );

DRIVER_INIT( cps1 );
DRIVER_INIT( cps2_video );

void cps1_get_video_base(void);

WRITE16_HANDLER( cps1_gfxram_w );

VIDEO_START( cps1 );
VIDEO_START( cps2 );

WRITE16_HANDLER( cps2_objram_bank_w );
READ16_HANDLER( cps2_objram1_r );
READ16_HANDLER( cps2_objram2_r );
WRITE16_HANDLER( cps2_objram1_w );
WRITE16_HANDLER( cps2_objram2_w );

VIDEO_UPDATE( cps1 );
VIDEO_EOF( cps1 );

void cps2_set_sprite_priorities(void);
void cps2_objram_latch(void);

#endif
