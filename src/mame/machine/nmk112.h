#ifndef NMK112_H
#define NMK112_H

void NMK112_set_paged_table( int chip, int value );

WRITE8_HANDLER( NMK112_okibank_w );
WRITE16_HANDLER( NMK112_okibank_lsb_w );

#endif
