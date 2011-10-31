#include "emu.h"
#include "netdev_tap.h"

void sdlnetdev_init(running_machine &machine)
{
	init_tap();
}
