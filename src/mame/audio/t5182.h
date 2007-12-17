#define CPUTAG_T5182 "T5182"
#define T5182COINPORT "T5182_COIN"

ADDRESS_MAP_EXTERN( t5182_map );
ADDRESS_MAP_EXTERN( t5182_io );

WRITE8_HANDLER( t5182_sound_irq_w );
READ8_HANDLER(t5182_sharedram_semaphore_snd_r);
WRITE8_HANDLER(t5182_sharedram_semaphore_main_acquire_w);
WRITE8_HANDLER(t5182_sharedram_semaphore_main_release_w);

extern UINT8 *t5182_sharedram;

extern struct YM2151interface t5182_ym2151_interface;
