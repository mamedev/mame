#ifndef NMK112_H
#define NMK112_H

void NMK112_init(UINT8 disable_page_mask);

WRITE8_HANDLER( NMK112_okibank_w );
WRITE16_HANDLER( NMK112_okibank_lsb_w );

#endif
