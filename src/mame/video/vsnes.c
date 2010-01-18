#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"


PALETTE_INIT( vsnes )
{
	ppu2c0x_init_palette(machine, 0 );
}

PALETTE_INIT( vsdual )
{
	ppu2c0x_init_palette(machine, 0 );
	ppu2c0x_init_palette(machine, 8*4*16 );
}

static void ppu_irq_1( running_device *device, int *ppu_regs )
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE );
}

static void ppu_irq_2( running_device *device, int *ppu_regs )
{
	cputag_set_input_line(device->machine, "sub", INPUT_LINE_NMI, PULSE_LINE );
}

/* our ppu interface                                            */
const ppu2c0x_interface vsnes_ppu_interface_1 =
{
	0,					/* gfxlayout num */
	0,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq_1			/* irq */
};

/* our ppu interface for dual games                             */
const ppu2c0x_interface vsnes_ppu_interface_2 =
{
	1,					/* gfxlayout num */
	64,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq_2			/* irq */
};

VIDEO_START( vsnes )
{
}

VIDEO_START( vsdual )
{
}

/***************************************************************************

  Display refresh

***************************************************************************/
VIDEO_UPDATE( vsnes )
{
	/* render the ppu */
	ppu2c0x_render( devtag_get_device(screen->machine, "ppu1"), bitmap, 0, 0, 0, 0 );
	return 0;
}


VIDEO_UPDATE( vsdual )
{
	running_device *top_screen = devtag_get_device(screen->machine, "top");
	running_device *bottom_screen = devtag_get_device(screen->machine, "bottom");

	/* render the ppu's */
	if (screen == top_screen)
		ppu2c0x_render(devtag_get_device(screen->machine, "ppu1"), bitmap, 0, 0, 0, 0);
	else if (screen == bottom_screen)
		ppu2c0x_render(devtag_get_device(screen->machine, "ppu2"), bitmap, 0, 0, 0, 0);

	return 0;
}
