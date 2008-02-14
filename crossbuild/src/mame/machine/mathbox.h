/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */

WRITE8_HANDLER( mb_go_w );
READ8_HANDLER( mb_status_r );
READ8_HANDLER( mb_lo_r );
READ8_HANDLER( mb_hi_r );
void mb_register_states(void);
