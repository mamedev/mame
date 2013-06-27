/***************************************************************************

    Atari Audio Board II Interface

****************************************************************************/


void atarijsa_init(running_machine &machine, const char *testport, int testmask);
void atarijsa_reset(running_machine &machine);


MACHINE_CONFIG_EXTERN( jsa_i_stereo );
MACHINE_CONFIG_EXTERN( jsa_i_stereo_swapped );
MACHINE_CONFIG_EXTERN( jsa_i_stereo_pokey );
MACHINE_CONFIG_EXTERN( jsa_i_mono_speech );
MACHINE_CONFIG_EXTERN( jsa_ii_mono );
MACHINE_CONFIG_EXTERN( jsa_iii_mono );
MACHINE_CONFIG_EXTERN( jsa_iii_mono_noadpcm );
MACHINE_CONFIG_EXTERN( jsa_iiis_stereo );


/* Board-specific port definitions */
INPUT_PORTS_EXTERN( atarijsa_i );
INPUT_PORTS_EXTERN( atarijsa_ii );
INPUT_PORTS_EXTERN( atarijsa_iii );
