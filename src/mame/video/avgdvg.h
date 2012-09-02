#ifndef __AVGDVG__
#define __AVGDVG__

CUSTOM_INPUT( avgdvg_done_r );
WRITE8_HANDLER( avgdvg_go_w );
WRITE8_HANDLER( avgdvg_reset_w );
WRITE16_HANDLER( avgdvg_go_word_w );
WRITE16_HANDLER( avgdvg_reset_word_w );

/* Tempest and Quantum use this capability */
void avg_set_flip_x(int flip);
void avg_set_flip_y(int flip);

VIDEO_START( dvg );
VIDEO_START( avg );
VIDEO_START( avg_tempest );
VIDEO_START( avg_mhavoc );
VIDEO_START( avg_starwars );
VIDEO_START( avg_quantum );
VIDEO_START( avg_bzone );
VIDEO_START( avg_tomcat );

MACHINE_RESET( avgdvg );

#endif
