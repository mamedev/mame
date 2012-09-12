#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"


PALETTE_INIT( vsnes )
{
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu1");
	ppu->init_palette_rgb(machine, 0 );
}

PALETTE_INIT( vsdual )
{
	ppu2c0x_device *ppu1 = machine.device<ppu2c0x_device>("ppu1");
	ppu2c0x_device *ppu2 = machine.device<ppu2c0x_device>("ppu2");
	ppu1->init_palette_rgb(machine, 0 );
	ppu2->init_palette_rgb(machine, 8*4*16 );
}

static void ppu_irq_1( device_t *device, int *ppu_regs )
{
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE );
}

static void ppu_irq_2( device_t *device, int *ppu_regs )
{
	device->machine().device("sub")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE );
}

/* our ppu interface                                            */
const ppu2c0x_interface vsnes_ppu_interface_1 =
{
	"maincpu",
	"screen1",
	0,					/* gfxlayout num */
	0,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq_1			/* irq */
};

/* our ppu interface for dual games                             */
const ppu2c0x_interface vsnes_ppu_interface_2 =
{
	"sub",
	"screen2",
	1,					/* gfxlayout num */
	512,				/* color base */
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
SCREEN_UPDATE_IND16( vsnes )
{
	/* render the ppu */
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu1");
	ppu->render(bitmap, 0, 0, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( vsnes_bottom )
{
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu2");
	ppu->render(bitmap, 0, 0, 0, 0);
	return 0;
}
