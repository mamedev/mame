// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles
/***************************************************************************

    Amiga AGA hardware

    Driver by: Ernesto Corvi, Mariusz Wojcieszek, Aaron Giles

Done:
    - palette
    - bitplane data fetching
    - support for up to 8 standard bitplanes
    - HAM8 mode
    - preliminary sprites

To do:
    - incorrect hstart/hstop values in CD32 logo, lsrquiz & lsrquiz2
    - sprite collisions

***************************************************************************/

/*************************************
 *
 *  Statics
 *
 *************************************/

/*************************************
 *
 *  Palette
 *
 *************************************/

void amiga_aga_palette_write(running_machine &machine, int color_reg, UINT16 data)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	rgb_t *aga_palette = state->m_aga_palette;
	int r,g,b;
	int cr,cg,cb;
	int color;

	color = ((CUSTOM_REG(REG_BPLCON3) >> 13) & 0x07)*32 + color_reg;
	r = (data & 0xf00) >> 8;
	g = (data & 0x0f0) >> 4;
	b = (data & 0x00f) >> 0;
	cr = aga_palette[color].r();
	cg = aga_palette[color].g();
	cb = aga_palette[color].b();
	if (BIT(CUSTOM_REG(REG_BPLCON3),9))
	{
		// load low nibbles
		cr = (cr & 0xf0) | r;
		cg = (cg & 0xf0) | g;
		cb = (cb & 0xf0) | b;
	}
	else
	{
		cr = (r << 4) | r;
		cg = (g << 4) | g;
		cb = (b << 4) | b;
	}
	aga_palette[color] = rgb_t(cr,cg,cb);
}

/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(amiga_state,amiga_aga)
{
	VIDEO_START_CALL_MEMBER( amiga );

	m_aga_diwhigh_written = 0;
	m_screen->register_screen_bitmap(m_flickerfixer32);
}


/*************************************
 *
 *  Single scanline rasterizer
 *
 *************************************/

void amiga_state::aga_render_scanline(bitmap_rgb32 &bitmap, int scanline)
{
	amiga_state *state = this;
	UINT16 save_color0 = CUSTOM_REG(REG_COLOR00);
	int ddf_start_pixel = 0, ddf_stop_pixel = 0;
	int hires = 0, dualpf = 0, ham = 0;
	int pf1pri = 0, pf2pri = 0;
	int planes = 0;

	UINT32 *dst = NULL;
	int ebitoffs = 0, obitoffs = 0;
	int ecolmask = 0, ocolmask = 0;
	int edelay = 0, odelay = 0;
	int next_copper_x;
	int pl;
	int defbitoffs = 0;
	rgb_t *aga_palette = m_aga_palette;

	int save_scanline = scanline;

	/* on the first scanline, reset the COPPER and HAM color */
	if (scanline == 0)
	{
		m_previous_lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		// toggle lof if enabled
		if (CUSTOM_REG(REG_BPLCON0) & BPLCON0_LACE)
			CUSTOM_REG(REG_VPOSR) ^= VPOSR_LOF;

		amiga_copper_setpc(machine(), CUSTOM_REG_LONG(REG_COP1LCH));
		m_ham_color = CUSTOM_REG(REG_COLOR00);
	}

	// in visible area?
	if (bitmap.valid())
	{
		bool lof = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;

		if ((scanline & 1) ^ lof)
		{
			// lof matches? then render this scanline
			dst = &bitmap.pix32(scanline);
		}
		else
		{
			// lof doesn't match, we don't render this scanline
			// if we didn't switch lof we have a full non-interlace screen,
			// so we fill the black gaps with the contents of the previous scanline
			// otherwise just render the contents of the previous frame's scanline
			int shift = (m_previous_lof == lof) ? 1 : 0;

			memcpy(&bitmap.pix32(scanline), &m_flickerfixer32.pix32(scanline - shift), amiga_state::SCREEN_WIDTH * 4);
			return;
		}
	}

	scanline /= 2;

	m_last_scanline = scanline;

	/* update sprite data fetching */
	update_sprite_dma(state, scanline);

	/* all sprites off at the start of the line */
	memset(m_sprite_remain, 0, sizeof(m_sprite_remain));

	/* temporary set color 0 to the genlock color */
	if (m_genlock_color != 0xffff)
		CUSTOM_REG(REG_COLOR00) = m_genlock_color;

	/* loop over the line */
	next_copper_x = 2;  /* copper runs on odd timeslots */
	for (int x = 0; x < amiga_state::SCREEN_WIDTH / 2; x++)
	{
		int sprpix;

		/* time to execute the copper? */
		if (x == next_copper_x)
		{
			/* execute the next batch, restoring and re-saving color 0 around it */
			CUSTOM_REG(REG_COLOR00) = save_color0;
			next_copper_x = amiga_copper_execute_next(machine(), x);
			save_color0 = CUSTOM_REG(REG_COLOR00);
			if (m_genlock_color != 0xffff)
				CUSTOM_REG(REG_COLOR00) = m_genlock_color;

			/* compute update-related register values */
			planes = (CUSTOM_REG(REG_BPLCON0) & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) >> 12;
			if ( CUSTOM_REG(REG_BPLCON0) & BPLCON0_BPU3 )
				planes |= 8;

			hires = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HIRES;
			ham = CUSTOM_REG(REG_BPLCON0) & BPLCON0_HOMOD;
			dualpf = CUSTOM_REG(REG_BPLCON0) & BPLCON0_DBLPF;

			/* get default bitoffset */
			switch(CUSTOM_REG(REG_FMODE) & 0x3)
			{
				case 0: defbitoffs = 15; break;
				case 1:
				case 2: defbitoffs = 31; break;
				case 3: defbitoffs = 63; break;
			}

			/* compute the pixel fetch parameters */
			ddf_start_pixel = ( CUSTOM_REG(REG_DDFSTRT) & 0xfc ) * 2 + (hires ? 9 : 17);
			ddf_stop_pixel = ( CUSTOM_REG(REG_DDFSTOP) & 0xfc ) * 2 + (hires ? (9 + defbitoffs - ((defbitoffs >= 31) ? 16 : 0)) : (17 + defbitoffs));

			if ( ( CUSTOM_REG(REG_DDFSTRT) ^ CUSTOM_REG(REG_DDFSTOP) ) & 0x04 )
				ddf_stop_pixel += 8;

			// display window
			update_display_window();

			/* extract playfield priorities */
			pf1pri = CUSTOM_REG(REG_BPLCON2) & 7;
			pf2pri = (CUSTOM_REG(REG_BPLCON2) >> 3) & 7;

			/* extract collision masks */
			ocolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x15;
			ecolmask = (CUSTOM_REG(REG_CLXCON) >> 6) & 0x2a;
		}

		/* clear the target pixels to the background color as a starting point */
		if (dst != NULL)
			dst[x*2+0] =
			dst[x*2+1] = aga_palette[0];

		/* if we hit the first fetch pixel, reset the counters and latch the delays */
		if (x == ddf_start_pixel)
		{
			odelay = CUSTOM_REG(REG_BPLCON1) & 0xf;
			edelay = ( CUSTOM_REG(REG_BPLCON1) >> 4 ) & 0x0f;

			if ( hires )
			{
				obitoffs = defbitoffs + ( odelay << 1 );
				ebitoffs = defbitoffs + ( edelay << 1 );
			}
			else
			{
				if ( CUSTOM_REG(REG_DDFSTRT) & 0x04 )
				{
					odelay = ( odelay + 8 ) & 0x0f;
					edelay = ( edelay + 8 ) & 0x0f;
				}

				obitoffs = defbitoffs + odelay;
				ebitoffs = defbitoffs + edelay;
			}

			for (pl = 0; pl < 8; pl++)
				m_aga_bpldat[pl] = 0;
		}

		/* need to run the sprite engine every pixel to ensure display */
		sprpix = get_sprite_pixel(state, x);

		/* to render, we must have bitplane DMA enabled, at least 1 plane, and be within the */
		/* vertical display window */
		if ((CUSTOM_REG(REG_DMACON) & (DMACON_BPLEN | DMACON_DMAEN)) == (DMACON_BPLEN | DMACON_DMAEN) &&
			planes > 0 && scanline >= m_diw.min_y && scanline < m_diw.max_y)
		{
			int pfpix0 = 0, pfpix1 = 0, collide;

			/* fetch the odd bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + odelay)
			{
				/* if we need to fetch more data, do it now */
				if (obitoffs == defbitoffs)
				{
					for (pl = 0; pl < planes; pl += 2)
					{
						fetch_bitplane_data(state, pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_odd_bitplanes(state, planes, obitoffs);
				obitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (obitoffs < 0)
					{
						obitoffs = defbitoffs;

						for (pl = 0; pl < planes; pl += 2)
						{
							fetch_bitplane_data(state, pl);
						}
					}

					pfpix1 |= assemble_odd_bitplanes(state, planes, obitoffs);
					obitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0x55; // 0x15

				/* reset bit offsets if needed */
				if (obitoffs < 0)
					obitoffs = defbitoffs;
			}

			/* fetch the even bits if we are within the fetching region */
			if (x >= ddf_start_pixel && x <= ddf_stop_pixel + edelay)
			{
				/* if we need to fetch more data, do it now */
				if (ebitoffs == defbitoffs)
				{
					for (pl = 1; pl < planes; pl += 2)
					{
						fetch_bitplane_data(state, pl);
					}
				}

				/* now assemble the bits */
				pfpix0 |= assemble_even_bitplanes(state, planes, ebitoffs);
				ebitoffs--;

				/* for high res, assemble a second set of bits */
				if (hires)
				{
					/* reset bit offsets and fetch more data if needed */
					if (ebitoffs < 0)
					{
						ebitoffs = defbitoffs;

						for (pl = 1; pl < planes; pl += 2)
						{
							fetch_bitplane_data(state, pl);
						}
					}

					pfpix1 |= assemble_even_bitplanes(state, planes, ebitoffs);
					ebitoffs--;
				}
				else
					pfpix1 |= pfpix0 & 0xaa;

				/* reset bit offsets if needed */
				if (ebitoffs < 0)
					ebitoffs = defbitoffs;
			}

			/* compute playfield/sprite collisions for first pixel */
			collide = pfpix0 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* compute playfield/sprite collisions for second pixel */
			collide = pfpix1 ^ CUSTOM_REG(REG_CLXCON);
			if ((collide & ocolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 5) & 0x01e;
			if ((collide & ecolmask) == 0)
				CUSTOM_REG(REG_CLXDAT) |= (sprpix >> 1) & 0x1e0;
			if ((collide & (ecolmask | ocolmask)) == 0)
				CUSTOM_REG(REG_CLXDAT) |= 0x001;

			/* if we are within the display region, render */
			if (dst != NULL && x >= m_diw.min_x && x < m_diw.max_x)
			{
				int pix, pri;

				/* hold-and-modify mode -- assume low-res (hi-res not supported by the hardware) */
				if (ham)
				{
					/* update the HAM color */
					pfpix0 = update_ham(state, pfpix0);

					pix = sprpix & 0xff;
					pri = (sprpix >> 10);

					/* sprite has priority */
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = aga_palette[pix];
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] =
						dst[x*2+1] = pfpix0;
					}
				}

				/* dual playfield mode */
				else if (dualpf)
				{
					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0x1f;
					pri = (sprpix >> 12);
					if (pix)
					{
						if ((pfpix0 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix0 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the left pixel */
					if (pix)
						dst[x*2+0] = aga_palette[pix];
					else
						dst[x*2+0] = aga_palette[m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix0]];

					/* mask out the sprite if it doesn't have priority */
					pix = sprpix & 0xff;
					if (pix)
					{
						if ((pfpix1 & 0x15) && pf1pri <= pri)
							pix = 0;
						if ((pfpix1 & 0x2a) && pf2pri <= pri)
							pix = 0;
					}

					/* write out the right pixel */
					if (pix)
						dst[x*2+1] = aga_palette[pix];
					else
						dst[x*2+1] = aga_palette[m_separate_bitplanes[(CUSTOM_REG(REG_BPLCON2) >> 6) & 1][pfpix1]];
				}

				/* single playfield mode */
				else
				{
					pix = sprpix & 0xff;
					pri = (sprpix >> 12);

					/* sprite has priority */
					if (sprpix && pf1pri > pri)
					{
						dst[x*2+0] =
						dst[x*2+1] = aga_palette[pix];
					}

					/* playfield has priority */
					else
					{
						dst[x*2+0] = aga_palette[pfpix0];
						dst[x*2+1] = aga_palette[pfpix1];
					}
				}
			}
		}
	}

#if 0
	if ( m_screen->frame_number() % 16 == 0 && scanline == 250 )
	{
		const char *m_lores = "LORES";
		const char *m_hires = "HIRES";
		const char *m_ham = "HAM";
		const char *m_dualpf = "DUALPF";
		const char *m_lace = "LACE";
		const char *m_hilace = "HI-LACE";
		const char *p = m_lores;

		if ( hires ) p = m_hires;
		if ( ham ) p = m_ham;
		if ( dualpf ) p = m_dualpf;
		if ( lace ) p = m_lace;

		if ( hires && lace ) p = m_hilace;

		popmessage("%s(%d pl od=%02x ed=%02x start=%d stop=%d hstart=%04x hstop=%04x diwhigh=%04x fetchbits=%d )", p, planes, odelay, edelay, ddf_start_pixel, ddf_stop_pixel, CUSTOM_REG(REG_DIWSTRT), CUSTOM_REG(REG_DIWSTOP), CUSTOM_REG(REG_DIWHIGH), defbitoffs );
		//popmessage("%s(%d pl bplpt1=%06X, bpl1mod=%04x, offset=%x)", p, planes, CUSTOM_REG_LONG(REG_BPL1PTH), CUSTOM_REG(REG_BPL1MOD), hires_modulo_offset );
	}
#endif

	/* end of the line: time to add the modulos */
	if (scanline >= m_diw.min_y && scanline < m_diw.max_y)
	{
		/* update odd planes */
		for (pl = 0; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL1MOD);

		/* update even planes */
		for (pl = 1; pl < planes; pl += 2)
			CUSTOM_REG_LONG(REG_BPL1PTH + pl * 2) += CUSTOM_REG_SIGNED(REG_BPL2MOD);
	}

	/* restore color00 */
	CUSTOM_REG(REG_COLOR00) = save_color0;

	// save
	if (dst != NULL)
		memcpy(&m_flickerfixer32.pix32(save_scanline), dst, amiga_state::SCREEN_WIDTH * 4);

#if GUESS_COPPER_OFFSET
	if (m_screen->frame_number() % 64 == 0 && scanline == 0)
	{
		if (machine.input().code_pressed(KEYCODE_Q))
			popmessage("%d", wait_offset -= 1);
		if (machine.input().code_pressed(KEYCODE_W))
			popmessage("%d", wait_offset += 1);
	}
#endif
}



/*************************************
 *
 *  Update
 *
 *************************************/

UINT32 amiga_state::screen_update_amiga_aga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (cliprect.min_y != cliprect.max_y)
		return 0;

	// render each scanline in the visible region
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		aga_render_scanline(bitmap, y);

	return 0;
}
