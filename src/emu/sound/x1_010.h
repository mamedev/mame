struct x1_010_interface
{
	int adr;	/* address */
};


READ8_HANDLER ( seta_sound_r );
WRITE8_HANDLER( seta_sound_w );

READ16_HANDLER ( seta_sound_word_r );
WRITE16_HANDLER( seta_sound_word_w );

void seta_sound_enable_w(int);
