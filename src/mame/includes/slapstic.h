/*************************************************************************

    Atari Slapstic decoding helper

**************************************************************************

    For more information on the slapstic, see slapstic.html, or go to
    http://www.aarongiles.com/slapstic.html

*************************************************************************/

/*----------- defined in machine/slapstic.c -----------*/

void slapstic_init(running_machine &machine, int chip);
void slapstic_reset(void);

int slapstic_bank(void);
int slapstic_tweak(address_space &space, offs_t offset);
