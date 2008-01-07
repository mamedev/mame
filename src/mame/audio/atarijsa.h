/***************************************************************************

    Atari Audio Board II Interface

****************************************************************************/


void atarijsa_init(running_machine *machine, int testport, int testmask);
void atarijsa3_init_adpcm(int region);
void atarijsa_reset(void);


MACHINE_DRIVER_EXTERN( jsa_i_stereo );
MACHINE_DRIVER_EXTERN( jsa_i_stereo_swapped );
MACHINE_DRIVER_EXTERN( jsa_i_stereo_pokey );
MACHINE_DRIVER_EXTERN( jsa_i_mono_speech );
MACHINE_DRIVER_EXTERN( jsa_ii_mono );
MACHINE_DRIVER_EXTERN( jsa_iii_mono );
MACHINE_DRIVER_EXTERN( jsa_iii_mono_noadpcm );
MACHINE_DRIVER_EXTERN( jsa_iiis_stereo );


/* Board-specific port definitions */
#define JSA_I_PORT											\
	PORT_START_TAG("JSAI")									\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* speech chip ready */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

/* used by Xybots */
#define JSA_I_PORT_SWAPPED									\
	PORT_START_TAG("JSAI")									\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* speech chip ready */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

#define JSA_II_PORT											\
	PORT_START_TAG("JSAII")									\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )			\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )		/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

#define JSA_III_PORT										\
	PORT_START_TAG("JSAIII")								\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )				\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )				\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )				\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )			\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* output buffer full */\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )	/* input buffer full */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )	/* self test */

