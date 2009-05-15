/***************************************************************************

    tlc34076.h

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

void tlc34076_reset(int dacwidth);
void tlc34076_state_save(running_machine *machine);

const pen_t *tlc34076_get_pens(void);

READ8_HANDLER( tlc34076_r );
WRITE8_HANDLER( tlc34076_w );

READ16_HANDLER( tlc34076_lsb_r );
WRITE16_HANDLER( tlc34076_lsb_w );
READ16_HANDLER( tlc34076_msb_r );
WRITE16_HANDLER( tlc34076_msb_w );

