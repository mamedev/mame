#include "sound/262intf.h"
#include "sound/2151intf.h"


#define CPUTAG_T5182 "t5182"
#define T5182COINPORT "T5182_COIN"

void t5182_init(running_machine &machine);

ADDRESS_MAP_EXTERN( t5182_map, 8 );
ADDRESS_MAP_EXTERN( t5182_io, 8 );

WRITE8_HANDLER( t5182_sound_irq_w );
READ8_HANDLER(t5182_sharedram_semaphore_snd_r);
WRITE8_HANDLER(t5182_sharedram_semaphore_main_acquire_w);
WRITE8_HANDLER(t5182_sharedram_semaphore_main_release_w);

READ8_HANDLER( t5182_sharedram_r );
WRITE8_HANDLER( t5182_sharedram_w );

extern const ym2151_interface t5182_ym2151_interface;
