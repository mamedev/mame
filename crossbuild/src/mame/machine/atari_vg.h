/***************************************************************************

    Atari vector hardware

***************************************************************************/

READ8_HANDLER( atari_vg_earom_r );
WRITE8_HANDLER( atari_vg_earom_w );
WRITE8_HANDLER( atari_vg_earom_ctrl_w );

NVRAM_HANDLER( atari_vg );

void atari_vg_register_states(void);
