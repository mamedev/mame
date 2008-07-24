/***************************************************************************

    Atari Audio Board II Interface

****************************************************************************/


void atarijsa_init(running_machine *machine, const char *testport, int testmask);
void atarijsa3_init_adpcm(running_machine *machine, int region);
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
INPUT_PORTS_EXTERN( atarijsa_i );
INPUT_PORTS_EXTERN( atarijsa_ii );
INPUT_PORTS_EXTERN( atarijsa_iii );
