/***************************************************************************

    tlc34076.h

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

void tlc34076_reset(int dacwidth);

READ8_HANDLER( tlc34076_r );
WRITE8_HANDLER( tlc34076_w );

READ16_HANDLER( tlc34076_lsb_r );
WRITE16_HANDLER( tlc34076_lsb_w );
READ16_HANDLER( tlc34076_msb_r );
WRITE16_HANDLER( tlc34076_msb_w );

