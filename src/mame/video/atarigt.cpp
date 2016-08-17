// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari GT hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank

****************************************************************************/


#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarigt.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define RSHIFT      16
#define GSHIFT      8
#define BSHIFT      0



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarigt_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(atarigt_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}


TILEMAP_MAPPER_MEMBER(atarigt_state::atarigt_playfield_scan)
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(atarigt_state,atarigt)
{
	/* blend the playfields and free the temporary one */
	blend_gfx(0, 2, 0x0f, 0x30);

	/* allocate temp bitmaps */
	m_screen->register_screen_bitmap(m_pf_bitmap);
	m_screen->register_screen_bitmap(m_an_bitmap);

	/* reset statics */
	memset(m_colorram, 0, 0x80000);

	/* save states */
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_color_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
	save_item(NAME(m_tram_checksum));
	save_item(NAME(m_expanded_mram));
}



/*************************************
 *
 *  Color RAM access
 *
 *************************************/

void atarigt_state::atarigt_colorram_w(offs_t address, UINT16 data, UINT16 mem_mask)
{
	UINT16 olddata;

	/* update the raw data */
	address = (address & 0x7ffff) / 2;
	olddata = m_colorram[address];
	COMBINE_DATA(&m_colorram[address]);

	/* update the TRAM checksum */
	if (address >= 0x10000 && address < 0x14000)
		m_tram_checksum += m_colorram[address] - olddata;

	/* update expanded MRAM */
	else if (address >= 0x20000 && address < 0x28000)
	{
		m_expanded_mram[0 * MRAM_ENTRIES + (address & 0x7fff)] = (m_colorram[address] >> 8) << RSHIFT;
		m_expanded_mram[1 * MRAM_ENTRIES + (address & 0x7fff)] = (m_colorram[address] & 0xff) << GSHIFT;
	}
	else if (address >= 0x30000 && address < 0x38000)
		m_expanded_mram[2 * MRAM_ENTRIES + (address & 0x7fff)] = (m_colorram[address] & 0xff) << BSHIFT;
}


UINT16 atarigt_state::atarigt_colorram_r(offs_t address)
{
	address &= 0x7ffff;
	return m_colorram[address / 2];
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void atarigt_state::scanline_update(screen_device &screen, int scanline)
{
	int i;

	/* keep in range */
	int offset = (scanline / 8) * 64 + 48;
	if (offset >= 0x800)
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word = m_alpha_tilemap->basemem_read(offset++);

		if (word & 0x8000)
		{
			int newscroll = (word >> 5) & 0x3ff;
			int newbank = (word >> 0) & 0x1f;
			if (newscroll != m_playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrollx(0, newscroll);
				m_playfield_xscroll = newscroll;
			}
			if (newbank != m_playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->set_palette_offset((newbank & 0x1f) << 8);
				m_playfield_color_bank = newbank;
			}
		}

		word = m_alpha_tilemap->basemem_read(offset++);
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 15;
			if (newscroll != m_playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrolly(0, newscroll);
				m_playfield_yscroll = newscroll;
			}
			if (newbank != m_playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->mark_all_dirty();
				m_playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

/*

    How it works:

        Incoming data from AN = AN.VID0-7
        Incoming data from PF = PF.VID0-12
        Incoming data from MO = MVID0-11
        Incoming data from TMO = TVID0-11

    !MGEP = 1 if:
        MVID9-11 < PF.VID10-12
        or (PF.VID12 and R188 is installed [yes on T-Mek & Rage])
    !MGEP = 0 if:
        R177 is installed [no on T-Mek & Rage]

    First, the CRAM:

        GAL @ 13M, takes as input:
            ANZ (AN.VID0-3 == 0)
            MOZ (MVID0-5 == 0)
            PFZ (PF.VID0-5 == 0)
            MVID10
            MVID11
            PF.VID10
            PF.VID11
            AN.VID7
            !MGEP
        And outputs:
            CRA10-12
            CRMUXA-B

        Index to the CRAM:
            CRA13  = LATCH & 0x08
            CRA12 \
            CRA11  = output from GAL
            CRA10 /
            CRA9 \
            CRA8  |
            CRA7  |
            CRA6  |
            CRA5   = output from MUX, as selected by GAL, either PF.VID0-9, MVID0-9, or AN.VID0-7
            CRA4  |
            CRA3  |
            CRA2  |
            CRA1  |
            CRA0 /
        Output from CRAM (16 bits):
            /TLEN
            CRR0-4
            CRG0-4
            CRB0-4

    Next, the TRAM:

        GAL @ 14K, takes as input:
            ANZ (AN.VID0-3 == 0)
            MOZ (MVID0-5 == 0)
            PFZ (PF.VID0-5 == 0)
            TVID8
            TVID9
            TVID10
            PF.VID10
            PF.VID11
            AN.VID7
            !MGEP
            !68.TRAN
        And outputs:
            TRA8-11
            TRMUXA-B

        Index to the TRAM:
            TRA13  = LATCH & 0x20
            TRA12  = LATCH & 0x10
            TRA11 \
            TRA10  = output from GAL
            TRA9   |
            TRA8  /
            TRA7  \
            TRA6  |
            TRA5  |
            TRA4   = output from MUX, as selected by GAL, either PF.VID0-7, MVID0-7, or AN.VID0-7
            TRA3  |
            TRA2  |
            TRA1  |
            TRA0 /
        Output from TRAM (16 bits):
            TLPRI
            TRR0-4
            TRG0-4
            TRB0-4

    Finally, the MRAM:

        GAL @ 5F, takes as input:
            TLPRI
            !TLEN
            PFZ
            PF.VID12
            TVID9-11
        And outputs:
            TLDIS
            (PFPRI)
            (TLPRID)
            enable for CRAM latches
            MRA10-12

        The CRAM outputs are gated by the enable signal from 5F; it outputs
            BMRA0-4
            GMRA0-4
            RMRA0-4

        The TRAM outputs are gated by the TLDIS signal from 5F
            BMRA5-9
            GMRA5-9
            RMRA5-9

        Index to MRAM:
            MRA14  = LATCH & 0x80
            MRA13  = LATCH & 0x40
            MRA12 \
            MRA11  = output from GAL
            MRA10 /
            MRA9 \
            MRA8  |
            MRA7   = TRAM output
            MRA6  |
            MRA5 /
            MRA4 \
            MRA3  |
            MRA2   = CRAM output
            MRA1  |
            MRA0 /

    And even beyond that:

        LATCH & 0x04 -> LCRD2
        LATCH & 0x02 -> LCRD1
        LATCH & 0x01 -> LCRD0

        GAL @ 22E, takes as input:
            LCRD0-2
            PF.VID12
            PFZ
            ANZ
            MVZ

        And outputs SUMRED, SUMGRN, SUMBLU,
        which bypass everything!

-------------------------------------------------

TMEK GALs:

    13M:
        CRA12=68.A13*!7M3                           -- when writing with 68020's A13 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*7M3              -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0
           +PFZ*!AN.VID7*ANZ*!MVZ*7M3               -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri

        CRA11=68.A12*!7M3                           -- when writing with 68020's A12 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID11*7M3       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix11!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID11*7M3        -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix11!=0
           +!AN.VID7*ANZ*MVZ*PF.VID11*7M3           -- !opaque_alpha && apix==0 && mopix==0 && pfpix11!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID11*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix11!=0

        CRA10=68.A11*!7M3                           -- when writing with 68020's A11 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID10*7M3       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix10!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID10*7M3        -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix10!=0
           +!AN.VID7*ANZ*MVZ*PF.VID10*7M3           -- !opaque_alpha && apix==0 && mopix==0 && pfpix10!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID10*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix10!=0

        CRMUXB=!AN.VID7*ANZ*7M3                     -- !opaque_alpha && apix==0

        !CRMUXA=!7M3
            +MGEP*!AN.VID7*ANZ*!MVZ
            +PFZ*!AN.VID7*ANZ*!MVZ


    14K:
        TRA11=68.A12*!7M1                   -- when writing with 68020's A12 == 1

        TRA10=68.A11*!7M1                   -- when writing with 68020's A11 == 1
           +!AN.VID7*ANZ*PFZ*7M1*!MVZ       -- !opaque_alpha && apix==0 && pfpix==0 && mopix!=0
           +MGEP*!AN.VID7*ANZ*7M1*!MVZ      -- !opaque_alpha && apix==0 && mopri>=pfpri && mopix!=0

        TRA9=68.A10*!7M1                    -- when writing with 68020's A10 == 1
           +!AN.VID7*ANZ*TVID9*7M1          -- !opaque_alpha && apix==0 && tvid9==1

        TRA8=68.A9*!7M1                     -- when writing with 68020's A9 == 1
           +!AN.VID7*ANZ*TVID8*7M1          -- !opaque_alpha && apix==0 && tvid8==1

        TRMUXB=7M1                          -- 1
        TRMUXA=GND                          -- 0

    5F:
        MRA12:=TVID11
        MRA11:=TVID10
        MRA10:=TVID9
        (PFPRI):=PF.VID12

        !TLDIS=PFZ*TLEN                     -- enabled if pfpix==0 && tlen
            +!(PFPRI)*TLEN                  -- or if pf.vid12==0 && tlen

        !(CRADIS)=!(PFPRI)*TLPRI            -- enabled if pfvid.12==0 && tlpri
        (TLPRID):=TLPRI

    22E:
        LCRD0 LCRD1 LCRD2 MVZ ANZ PFZ PF.VID12 LLD3 LLA3 GND J SUMBLU1K SUMGRN1K SUMRED1K (OE2) (OE1) SUMBLU SUMGRN SUMRED VCC
        SUMRED.OE=(OE1)
        !SUMRED=!SUMGRN
        SUMGRN.OE=(OE1)
        SUMGRN=!LCRD1*LCRD2*LLA3
           +LCRD1*LCRD0*LCRD2*J
           +LCRD1*LCRD0*!LCRD2*!LLD3
           +!LCRD1*LCRD0*!LCRD2*LLD3
           +!LCRD0*LCRD2*LLA3
           +LCRD1*!LCRD0*!LCRD2*LLD3
        SUMBLU.OE=(OE1)
        !SUMBLU=!SUMGRN

        (OE1)=!LCRD1*LCRD2*!PF.VID12*LLD3*J             -- (LCR & 6)==4 && LLD3 && !PF.VID12    [LCR==4 || LCR==5]
           +!LCRD1*LCRD2*PFZ*LLD3*J                     -- (LCR & 6)==4 && LLD3 && PFZ          [LCR==4 || LCR==5]
           +!LCRD0*LCRD2*!PF.VID12*LLD3                 -- (LCR & 5)==4 && LLD3 && !PF.VID12    [LCR==4 || LCR==6]
           +!LCRD0*LCRD2*PFZ*LLD3                       -- (LCR & 5)==4 && LLD3 && PFZ          [LCR==4 || LCR==6]
           +!LCRD1*!LCRD0*LCRD2*!PF.VID12*J             -- LCR==4 && !PF.VID12                  [LCR==4]
           +!LCRD1*!LCRD0*LCRD2*PFZ*J                   -- LCR==4 && PFZ                        [LCR==4]

        (OE2)=LCRD1*LCRD0*!LCRD2*!PF.VID12*LLA3*J       -- LCR==3 && LLA3 && !PF.VID12          [LCR==3]
           +LCRD1*LCRD0*!LCRD2*PFZ*LLA3*J               -- LCR==3 && LLA3 && PFZ                [LCR==3]
           +!LCRD1*LCRD0*!LCRD2*!MVZ*!PF.VID12*J        -- LCR==1 && !MVZ && !PF.VID12          [LCR==1]
           +!LCRD1*LCRD0*!LCRD2*!MVZ*PFZ*J              -- LCR==1 && !MVZ && PFZ                [LCR==1]
           +LCRD1*!LCRD0*LCRD2*!PF.VID12*LLD3           -- LCR==6 && LLD3 && !PF.VID12          [LCR==6]
           +LCRD1*!LCRD0*LCRD2*PFZ*LLD3                 -- LCR==6 && LLD3 && PFZ                [LCR==6]

        SUMRED1K.OE=(OE2)
        !SUMRED1K=!LCRD1*!LCRD2*!LLD3                   -- (LCR & 6)==0 && !LLD3
            +LCRD0*LCRD2*!LLA3                          -- (LCR & 5)==5 && !LLA3
            +LCRD1*LCRD0*!LCRD2*LLD3                    -- LCR==3 && LLD3
            +!LCRD0*!LCRD2*!LLD3                        -- (LCR & 5)==0 && !LLD3
            +LCRD1*!LCRD0*LCRD2*!J                      -- LCR==6 && !J
            +!LCRD1*!LCRD0*LLD3                         -- (LCR & 3)==0 && LLD3
        SUMGRN1K.OE=(OE2)
        !SUMGRN1K=!SUMRED1K
        SUMBLU1K.OE=(OE2)
        !SUMBLU1K=!SUMRED1K

-------------------------------------------------

PrimRage GALs:

    13M:
        CRA12=68.A13*!7M3                                   -- when writing with 68020's A13 == 1
           +!AN.VID7*ANZ*!MVZ*MVID11*7M3                    -- !opaque_alpha && apix==0 && mopix!=0 && mvid11!=0
           +MGEP*!AN.VID7*ANZ*!MVZ*7M3                      -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0
           +PFZ*!AN.VID7*ANZ*!MVZ*7M3                       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri

        CRA11=68.A12*!7M3                                   -- when writing with 68020's A12 == 1
           +!AN.VID7*ANZ*MVZ*PF.VID11*7M3                   -- !opaque_alpha && apix==0 && mopix==0 && pfpix11!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID11*!MVID11*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix11!=0 && mvid11==0

        CRA10=68.A11*!7M3                                   -- when writing with 68020's A11 == 1
           +!AN.VID7*ANZ*MVZ*PF.VID10*7M3                   -- !opaque_alpha && apix==0 && mopix==0 && pfpix10!=0
           +!AN.VID7*ANZ*!MVZ*MVID11*MVID10*7M3             *- !opaque_alpha && apix==0 && mopix!=0 && mvid11 && mvid10
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID10*7M3               -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix10!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID10*7M3                -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix10!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID10*!MVID11*7M3    *- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && mopix11==0

        CRMUXB=!AN.VID7*ANZ*7M3

        !CRMUXA=!7M3
            +!AN.VID7*ANZ*!MVZ*MVID11
            +MGEP*!AN.VID7*ANZ*!MVZ
            +PFZ*!AN.VID7*ANZ*!MVZ

*/


UINT32 atarigt_state::screen_update_atarigt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 &mo_bitmap = m_rle->vram(0);
	bitmap_ind16 &tm_bitmap = m_rle->vram(1);
	UINT16 *cram, *tram;
	int color_latch;
	UINT32 *mram;
	int x, y;

	/* draw the playfield */
	m_playfield_tilemap->draw(screen, m_pf_bitmap, cliprect, 0, 0);

	/* draw the alpha layer */
	m_alpha_tilemap->draw(screen, m_an_bitmap, cliprect, 0, 0);

	/* cache pointers */
	color_latch = m_colorram[0x30000/2];
	cram = (UINT16 *)&m_colorram[0x00000/2] + 0x2000 * ((color_latch >> 3) & 1);
	tram = (UINT16 *)&m_colorram[0x20000/2] + 0x1000 * ((color_latch >> 4) & 3);
	mram = m_expanded_mram + 0x2000 * ((color_latch >> 6) & 3);

	/* now do the nasty blend */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *an = &m_an_bitmap.pix16(y);
		UINT16 *pf = &m_pf_bitmap.pix16(y);
		UINT16 *mo = &mo_bitmap.pix16(y);
		UINT16 *tm = &tm_bitmap.pix16(y);
		UINT32 *dst = &bitmap.pix32(y);

		/* Primal Rage: no TRAM, slightly different priorities */
		if (m_is_primrage)
		{
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT8 pfpri = (pf[x] >> 10) & 7;
				UINT8 mopri = mo[x] >> ATARIRLE_PRIORITY_SHIFT;
				UINT8 mgep = (mopri >= pfpri) && !(pfpri & 4);
				UINT16 cra;
				UINT32 rgb;

				/* compute CRA -- unlike T-Mek, MVID11 enforces MO priority and is ignored */
				if (an[x] & 0x8f)
					cra = an[x] & 0xff;
				else if ((mo[x] & 0x3f) && ((mo[x] & 0x800) || mgep || !(pf[x] & 0x3f)))
					cra = 0x1000 | (mo[x] & 0x7ff);
				else
					cra = pf[x] & 0xfff;
				cra = cram[cra];

				/* compute the result */
				rgb  = mram[0 * MRAM_ENTRIES + ((cra >> 10) & 0x01f)];
				rgb |= mram[1 * MRAM_ENTRIES + ((cra >>  5) & 0x01f)];
				rgb |= mram[2 * MRAM_ENTRIES + ((cra >>  0) & 0x01f)];

				/* final override */
				if (color_latch & 7)
					if (!(pf[x] & 0x3f) || !(pf[x] & 0x2000))
						rgb = (0xff << RSHIFT) | (0xff << GSHIFT) | (0xff << BSHIFT);

				dst[x] = rgb;
			}
		}

		/* T-Mek: full TRAM and all effects */
		else
		{
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT8 pfpri = (pf[x] >> 10) & 7;
				UINT8 mopri = mo[x] >> ATARIRLE_PRIORITY_SHIFT;
				UINT8 mgep = (mopri >= pfpri) && !(pfpri & 4);
				int no_tra = 0, no_cra = 0;
				UINT16 cra, tra, mra;
				UINT32 rgb;

				/* compute CRA/TRA */
				if (an[x] & 0x8f)
				{
					cra = an[x] & 0xff;
					tra = tm[x] & 0xff;
				}
				else if ((mo[x] & 0x3f) && (mgep || !(pf[x] & 0x3f)))
				{
					cra = 0x1000 | (mo[x] & 0xfff);
					tra = 0x400 | (tm[x] & 0x3ff);
				}
				else
				{
					cra = pf[x] & 0xfff;
					tra = tm[x] & 0x3ff;
				}
				cra = cram[cra];
				tra = tram[tra];

				/* compute MRA */
				mra = (tm[x] & 0xe00) << 1;

				/* turn off CRA/TRA as appropriate */
				if (!(pf[x] & 0x1000) && (tra & 0x8000))
					no_cra = 1;
				if (!(!(cra & 0x8000) && (!(pf[x] & 0x1000) || !(pf[x] & 0x3f))))
					no_tra = 1;
				if (no_cra)
					cra = 0;
				if (no_tra)
					tra = 0;

				/* compute the result */
				rgb  = mram[0 * MRAM_ENTRIES + mra + ((cra >> 10) & 0x01f) + ((tra >> 5) & 0x3e0)];
				rgb |= mram[1 * MRAM_ENTRIES + mra + ((cra >>  5) & 0x01f) + ((tra >> 0) & 0x3e0)];
				rgb |= mram[2 * MRAM_ENTRIES + mra + ((cra >>  0) & 0x01f) + ((tra << 5) & 0x3e0)];

				/* final override */
				if (color_latch & 7)
					if (!(pf[x] & 0x3f) || !(pf[x] & 0x2000))
						rgb = (0xff << RSHIFT) | (0xff << GSHIFT) | (0xff << BSHIFT);

				dst[x] = rgb;
			}
		}
	}
	return 0;
}
