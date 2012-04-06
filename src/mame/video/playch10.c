#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/playch10.h"



WRITE8_MEMBER(playch10_state::playch10_videoram_w)
{
	UINT8 *videoram = m_videoram;
	if (m_pc10_sdcs)
	{
		videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}
}

PALETTE_INIT( playch10 )
{
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */

		bit0 = ~(color_prom[0] >> 0) & 0x01;
		bit1 = ~(color_prom[0] >> 1) & 0x01;
		bit2 = ~(color_prom[0] >> 2) & 0x01;
		bit3 = ~(color_prom[0] >> 3) & 0x01;

		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = ~(color_prom[256] >> 0) & 0x01;
		bit1 = ~(color_prom[256] >> 1) & 0x01;
		bit2 = ~(color_prom[256] >> 2) & 0x01;
		bit3 = ~(color_prom[256] >> 3) & 0x01;

		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */

		bit0 = ~(color_prom[2*256] >> 0) & 0x01;
		bit1 = ~(color_prom[2*256] >> 1) & 0x01;
		bit2 = ~(color_prom[2*256] >> 2) & 0x01;
		bit3 = ~(color_prom[2*256] >> 3) & 0x01;

		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}

	ppu->init_palette_rgb(machine, 256);
}

static void ppu_irq( device_t *device, int *ppu_regs )
{
	playch10_state *state = device->machine().driver_data<playch10_state>();
	cputag_set_input_line(device->machine(), "cart", INPUT_LINE_NMI, PULSE_LINE );
	state->m_pc10_int_detect = 1;
}

/* our ppu interface                                           */
/* things like mirroring and whether to use vrom or vram       */
/* can be set by calling 'ppu2c0x_override_hardware_options'   */

const ppu2c0x_interface playch10_ppu_interface =
{
	"cart",
	"bottom",
	1,					/* gfxlayout num */
	256,				/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq				/* irq */
};

static TILE_GET_INFO( get_bg_tile_info )
{
	playch10_state *state = machine.driver_data<playch10_state>();
	UINT8 *videoram = state->m_videoram;
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x07) << 8);
	int color = (videoram[offs + 1] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( playch10 )
{
	playch10_state *state = machine.driver_data<playch10_state>();
	const UINT8 *bios = machine.region("maincpu")->base();
	state->m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

VIDEO_START( playch10_hboard )
{
	playch10_state *state = machine.driver_data<playch10_state>();
	const UINT8 *bios = machine.region("maincpu")->base();
	state->m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( playch10_single )
{
	playch10_state *state = screen.machine().driver_data<playch10_state>();
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu");

	rectangle top_monitor = screen.visible_area();

	top_monitor.max_y = ( top_monitor.max_y - top_monitor.min_y ) / 2;

	if(state->m_pc10_dispmask_old != state->m_pc10_dispmask)
	{
		state->m_pc10_dispmask_old = state->m_pc10_dispmask;

		if(state->m_pc10_dispmask)
			state->m_pc10_game_mode ^= 1;
	}

	if ( state->m_pc10_game_mode )
		/* render the ppu */
		ppu->render(bitmap, 0, 0, 0, 0 );
	else
	{
		/* When the bios is accessing vram, the video circuitry can't access it */
		if ( !state->m_pc10_sdcs )
			state->m_bg_tilemap->draw(bitmap, top_monitor, 0, 0);
	}
	return 0;
}

SCREEN_UPDATE_IND16( playch10_top )
{
	playch10_state *state = screen.machine().driver_data<playch10_state>();
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu");

	/* Single Monitor version */
	if (state->m_pc10_bios != 1)
		return SCREEN_UPDATE16_CALL(playch10_single);

	if (!state->m_pc10_dispmask)
		/* render the ppu */
		ppu->render(bitmap, 0, 0, 0, 0);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

SCREEN_UPDATE_IND16( playch10_bottom )
{
	playch10_state *state = screen.machine().driver_data<playch10_state>();

	/* Single Monitor version */
	if (state->m_pc10_bios != 1)
		return SCREEN_UPDATE16_CALL(playch10_single);

	/* When the bios is accessing vram, the video circuitry can't access it */

	if ( !state->m_pc10_sdcs )
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	return 0;
}
