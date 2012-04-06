/***************************************************************************

  Nintendo 8080 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/n8080.h"


WRITE8_MEMBER(n8080_state::n8080_video_control_w)
{

	m_sheriff_color_mode = (data >> 3) & 3;
	m_sheriff_color_data = (data >> 0) & 7;
	flip_screen_set_no_update(machine(), data & 0x20);
}


PALETTE_INIT( n8080 )
{
	int i;

	for (i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


PALETTE_INIT( helifire )
{
	int i;

	PALETTE_INIT_CALL(n8080);

	for (i = 0; i < 0x100; i++)
	{
		int level = 0xff * exp(-3 * i / 255.); /* capacitor discharge */

		palette_set_color(machine, 0x000 + 8 + i, MAKE_RGB(0x00, 0x00, level));   /* shades of blue */
		palette_set_color(machine, 0x100 + 8 + i, MAKE_RGB(0x00, 0xC0, level));   /* shades of blue w/ green star */

		palette_set_color(machine, 0x200 + 8 + i, MAKE_RGB(level, 0x00, 0x00));   /* shades of red */
		palette_set_color(machine, 0x300 + 8 + i, MAKE_RGB(level, 0xC0, 0x00));   /* shades of red w/ green star */
	}
}


void spacefev_start_red_cannon( running_machine &machine )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_spacefev_red_cannon = 1;
	state->m_cannon_timer->adjust(attotime::from_usec(550 * 68 * 10));
}


static TIMER_CALLBACK( spacefev_stop_red_cannon )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_spacefev_red_cannon = 0;
	state->m_cannon_timer->adjust(attotime::never);
}


static void helifire_next_line( running_machine &machine )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_helifire_mv++;

	if (state->m_helifire_sc % 4 == 2)
	{
		state->m_helifire_mv %= 256;
	}
	else
	{
		if (flip_screen_get(machine))
			state->m_helifire_mv %= 255;
		else
			state->m_helifire_mv %= 257;
	}

	if (state->m_helifire_mv == 128)
	{
		state->m_helifire_sc++;
	}
}


VIDEO_START( spacefev )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_cannon_timer = machine.scheduler().timer_alloc(FUNC(spacefev_stop_red_cannon));

	flip_screen_set_no_update(machine, 0);

	state->save_item(NAME(state->m_spacefev_red_screen));
	state->save_item(NAME(state->m_spacefev_red_cannon));
}


VIDEO_START( sheriff )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	flip_screen_set_no_update(machine, 0);

	state->save_item(NAME(state->m_sheriff_color_mode));
	state->save_item(NAME(state->m_sheriff_color_data));
}


VIDEO_START( helifire )
{
	n8080_state *state = machine.driver_data<n8080_state>();
	UINT8 data = 0;
	int i;

	state->save_item(NAME(state->m_helifire_mv));
	state->save_item(NAME(state->m_helifire_sc));
	state->save_item(NAME(state->m_helifire_flash));
	state->save_item(NAME(state->m_helifire_LSFR));

	for (i = 0; i < 63; i++)
	{
		int bit =
			(data >> 6) ^
			(data >> 7) ^ 1;

		data = (data << 1) | (bit & 1);

		state->m_helifire_LSFR[i] = data;
	}

	flip_screen_set_no_update(machine, 0);
}


SCREEN_UPDATE_IND16( spacefev )
{
	n8080_state *state = screen.machine().driver_data<n8080_state>();
	UINT8 mask = flip_screen_get(screen.machine()) ? 0xff : 0x00;

	int x;
	int y;

	const UINT8* pRAM = state->m_videoram;
	const UINT8* pPROM = screen.machine().region("proms")->base();

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y ^ mask);

		for (x = 0; x < 256; x += 8)
		{
			int n;

			UINT8 color = 0;

			if (state->m_spacefev_red_screen)
				color = 1;
			else
			{
				UINT8 val = pPROM[x >> 3];

				if ((x >> 3) == 0x06)
				{
					color = state->m_spacefev_red_cannon ? 1 : 7;
				}

				if ((x >> 3) == 0x1b)
				{
					static const UINT8 ufo_color[] =
					{
						1, /* red     */
						2, /* green   */
						7, /* white   */
						3, /* yellow  */
						5, /* magenta */
						6, /* cyan    */
					};

					int cycle = screen.frame_number() / 32;

					color = ufo_color[cycle % 6];
				}

				for (n = color + 1; n < 8; n++)
				{
					if (~val & (1 << n))
					{
						color = n;
					}
				}
			}

			for (n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = (pRAM[x >> 3] & (1 << n)) ? color : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


SCREEN_UPDATE_IND16( sheriff )
{
	n8080_state *state = screen.machine().driver_data<n8080_state>();
	UINT8 mask = flip_screen_get(screen.machine()) ? 0xff : 0x00;

	const UINT8* pPROM = screen.machine().region("proms")->base();

	int x;
	int y;

	const UINT8* pRAM = state->m_videoram;

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y ^ mask);

		for (x = 0; x < 256; x += 8)
		{
			int n;

			UINT8 color = pPROM[32 * (y >> 3) + (x >> 3)];

			if (state->m_sheriff_color_mode == 1 && !(color & 8))
				color = state->m_sheriff_color_data ^ 7;

			if (state->m_sheriff_color_mode == 2)
				color = state->m_sheriff_color_data ^ 7;

			if (state->m_sheriff_color_mode == 3)
				color = 7;

			for (n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = ((pRAM[x >> 3] >> n) & 1) ? (color & 7) : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


SCREEN_UPDATE_IND16( helifire )
{
	n8080_state *state = screen.machine().driver_data<n8080_state>();
	int SUN_BRIGHTNESS = input_port_read(screen.machine(), "POT0");
	int SEA_BRIGHTNESS = input_port_read(screen.machine(), "POT1");

	static const int wave[8] = { 0, 1, 2, 2, 2, 1, 0, 0 };

	unsigned saved_mv = state->m_helifire_mv;
	unsigned saved_sc = state->m_helifire_sc;

	int x;
	int y;

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y);

		int level = 120 + wave[state->m_helifire_mv & 7];

		/* draw sky */

		for (x = level; x < 256; x++)
		{
			pLine[x] = 0x200 + 8 + SUN_BRIGHTNESS + x - level;
		}

		/* draw stars */

		if (state->m_helifire_mv % 8 == 4) /* upper half */
		{
			int step = (320 * (state->m_helifire_mv - 0)) % sizeof state->m_helifire_LSFR;

			int data =
				((state->m_helifire_LSFR[step] & 1) << 6) |
				((state->m_helifire_LSFR[step] & 2) << 4) |
				((state->m_helifire_LSFR[step] & 4) << 2) |
				((state->m_helifire_LSFR[step] & 8) << 0);

			pLine[0x80 + data] |= 0x100;
		}

		if (state->m_helifire_mv % 8 == 5) /* lower half */
		{
			int step = (320 * (state->m_helifire_mv - 1)) % sizeof state->m_helifire_LSFR;

			int data =
				((state->m_helifire_LSFR[step] & 1) << 6) |
				((state->m_helifire_LSFR[step] & 2) << 4) |
				((state->m_helifire_LSFR[step] & 4) << 2) |
				((state->m_helifire_LSFR[step] & 8) << 0);

			pLine[0x00 + data] |= 0x100;
		}

		/* draw sea */

		for (x = 0; x < level; x++)
		{
			pLine[x] = 8 + SEA_BRIGHTNESS + x;
		}

		/* draw foreground */

		for (x = 0; x < 256; x += 8)
		{
			int offset = 32 * y + (x >> 3);

			int n;

			for (n = 0; n < 8; n++)
			{
				if (flip_screen_get(screen.machine()))
				{
					if ((state->m_videoram[offset ^ 0x1fff] << n) & 0x80)
					{
						pLine[x + n] = state->m_colorram[offset ^ 0x1fff] & 7;
					}
				}
				else
				{
					if ((state->m_videoram[offset] >> n) & 1)
					{
						pLine[x + n] = state->m_colorram[offset] & 7;
					}
				}
			}
		}

		/* next line */

		helifire_next_line(screen.machine());
	}

	state->m_helifire_mv = saved_mv;
	state->m_helifire_sc = saved_sc;
	return 0;
}


SCREEN_VBLANK( helifire )
{
	// falling edge
	if (!vblank_on)
	{
		n8080_state *state = screen.machine().driver_data<n8080_state>();
		int n = (screen.machine().primary_screen->frame_number() >> 1) % sizeof state->m_helifire_LSFR;

		int i;

		for (i = 0; i < 8; i++)
		{
			int R = (i & 1);
			int G = (i & 2);
			int B = (i & 4);

			if (state->m_helifire_flash)
			{
				if (state->m_helifire_LSFR[n] & 0x20)
				{
					G |= B;
				}

				if (screen.machine().primary_screen->frame_number() & 0x04)
				{
					R |= G;
				}
			}

			palette_set_color_rgb(screen.machine(),i,
				R ? 255 : 0,
				G ? 255 : 0,
				B ? 255 : 0);
		}

		for (i = 0; i < 256; i++)
		{
			helifire_next_line(screen.machine());
		}
	}
}
