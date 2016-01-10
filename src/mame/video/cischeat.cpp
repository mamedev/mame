// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Driving Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z or X with:

        Q,W,E       shows scroll 0,1,2
        R,T         shows road 0,1
        A,S,D,F     shows sprites with priority 0,1,2,3
        X           shows some info on each sprite
        M           disables sprites zooming
        U           toggles the display of some hardware registers'
                    values ($80000/2/4/6)

        Keys can be used together!
        Additionally, layers can be disabled, writing to $82400
        (fake video register):

            ---- ---- --54 ----     Enable Road 1,0
            ---- ---- ---- 3---     Enable Sprites
            ---- ---- ---- -210     Enable Scroll 2,1,0

        0 is the same as 0x3f

[ 3 Scrolling Layers ]

    see Megasys1.c

        Tile Format:            Cisco Heat              F1 GP Star

                Colour      fedc b--- ---- ----     fedc ---- ---- ----
                Code        ---- -a98 7654 3210     ---- ba98 7654 3210

        Layer Size:         May be different from Megasys1?

[ 2 Road Layers ]

    Each of the 256 (not all visible) lines of the screen
    can display any of the lines of gfx in ROM, which are
    larger than the sceen and can therefore be scrolled

                                    Cisco Heat              F1 GP Star
                Line Width          1024                    1024
                Zoom                No                      Yes

[ 256 Sprites ]

    Sprites are made of several 16x16 tiles (up to 256x256 pixels)
    and can be zoomed in and out. See below for sprite RAM format.

***************************************************************************/

#include "emu.h"
#include "includes/cischeat.h"

#ifdef MAME_DEBUG
#define SHOW_READ_ERROR(_format_,_offset_)\
{\
	popmessage(_format_,_offset_);\
	logerror("CPU #0 PC %06X : Warning, ",space.device().safe_pc()); \
	logerror(_format_ "\n",_offset_);\
}
#define SHOW_WRITE_ERROR(_format_,_offset_,_data_)\
{\
	popmessage(_format_,_offset_,_data_);\
	logerror("CPU #0 PC %06X : Warning, ",space.device().safe_pc()); \
	logerror(_format_ "\n",_offset_,_data_); \
}

#else

#define SHOW_READ_ERROR(_format_,_offset_)\
{\
	logerror("CPU #0 PC %06X : Warning, ",space.device().safe_pc()); \
	logerror(_format_ "\n",_offset_);\
}
#define SHOW_WRITE_ERROR(_format_,_offset_,_data_)\
{\
	logerror("CPU #0 PC %06X : Warning, ",space.device().safe_pc()); \
	logerror(_format_ "\n",_offset_,_data_); \
}

#endif

#define CISCHEAT_VREG_SCROLL(_n_, _dir_)    m_scroll##_dir_[_n_] = new_data


#define cischeat_tmap_SET_SCROLL(_n_) \
	if (m_tmap[_n_]) \
	{ \
		m_tmap[_n_]->set_scrollx(0, m_scrollx[_n_]); \
		m_tmap[_n_]->set_scrolly(0, m_scrolly[_n_]); \
	}

#define cischeat_tmap_DRAW(_n_) \
	if ( (m_tmap[_n_]) && (m_active_layers & (1 << _n_) ) ) \
	{ \
		m_tmap[_n_]->draw(screen, bitmap, cliprect, flag, 0 ); \
		flag = 0; \
	}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void cischeat_state::prepare_shadows()
{
	int i;
	for (i = 0;i < 16;i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;

	m_drawmode_table[ 0] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;
}

/**************************************************************************
                                Cisco Heat
**************************************************************************/

#define TILES_PER_PAGE_X (0x20)
#define TILES_PER_PAGE_Y (0x20)
#define TILES_PER_PAGE (TILES_PER_PAGE_X * TILES_PER_PAGE_Y)

inline void cischeat_state::scrollram_w(address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask, int which)
{
	COMBINE_DATA(&m_scrollram[which][offset]);
	if (offset < 0x40000/2 && m_tmap[which])
	{
		if (m_scroll_flag[which] & 0x10) /* tiles are 8x8 */
		{
			m_tmap[which]->mark_tile_dirty(offset);
		}
		else
		{
			m_tmap[which]->mark_tile_dirty(offset*4 + 0);
			m_tmap[which]->mark_tile_dirty(offset*4 + 1);
			m_tmap[which]->mark_tile_dirty(offset*4 + 2);
			m_tmap[which]->mark_tile_dirty(offset*4 + 3);
		}
	}
}

WRITE16_MEMBER(cischeat_state::cischeat_scrollram_0_w){ scrollram_w(space, offset, data, mem_mask, 0); }
WRITE16_MEMBER(cischeat_state::cischeat_scrollram_1_w){ scrollram_w(space, offset, data, mem_mask, 1); }
WRITE16_MEMBER(cischeat_state::cischeat_scrollram_2_w){ scrollram_w(space, offset, data, mem_mask, 2); }

TILEMAP_MAPPER_MEMBER(cischeat_state::cischeat_scan_8x8)
{
	return (col * TILES_PER_PAGE_Y) +
			(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE * (num_cols / TILES_PER_PAGE_X) +
			(row % TILES_PER_PAGE_Y);
}

TILEMAP_MAPPER_MEMBER(cischeat_state::cischeat_scan_16x16)
{
	return ( ((col / 2) * (TILES_PER_PAGE_Y / 2)) +
				((row / 2) / (TILES_PER_PAGE_Y / 2)) * (TILES_PER_PAGE / 4) * (num_cols / TILES_PER_PAGE_X) +
				((row / 2) % (TILES_PER_PAGE_Y / 2)) )*4 + (row&1) + (col&1)*2;
}

TILE_GET_INFO_MEMBER(cischeat_state::cischeat_get_scroll_tile_info_8x8)
{
	int tmap = (FPTR)tilemap.user_data();
	UINT16 code = m_scrollram[tmap][tile_index];
	SET_TILE_INFO_MEMBER(tmap, (code & 0xfff), code >> (16 - m_bits_per_color_code), 0);
}

TILE_GET_INFO_MEMBER(cischeat_state::cischeat_get_scroll_tile_info_16x16)
{
	int tmap = (FPTR)tilemap.user_data();
	UINT16 code = m_scrollram[tmap][tile_index/4];
	SET_TILE_INFO_MEMBER(tmap, (code & 0xfff) * 4 + (tile_index & 3), code >> (16 - m_bits_per_color_code), 0);
}

void cischeat_state::create_tilemaps()
{
	int layer, i;

	for (layer = 0; layer < 3; layer++)
	{
		/* 16x16 tilemaps */
		m_tilemap[layer][0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_16x16),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_16x16),this),
									8,8, TILES_PER_PAGE_X * 16, TILES_PER_PAGE_Y * 2);
		m_tilemap[layer][0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_16x16),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_16x16),this),
									8,8, TILES_PER_PAGE_X * 8, TILES_PER_PAGE_Y * 4);
		m_tilemap[layer][0][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_16x16),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_16x16),this),
									8,8, TILES_PER_PAGE_X * 4, TILES_PER_PAGE_Y * 8);
		m_tilemap[layer][0][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_16x16),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_16x16),this),
									8,8, TILES_PER_PAGE_X * 2, TILES_PER_PAGE_Y * 16);

		/* 8x8 tilemaps */
		m_tilemap[layer][1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_8x8),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_8x8),this),
									8,8, TILES_PER_PAGE_X * 8, TILES_PER_PAGE_Y * 1);
		m_tilemap[layer][1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_8x8),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_8x8),this),
									8,8, TILES_PER_PAGE_X * 4, TILES_PER_PAGE_Y * 2);
		m_tilemap[layer][1][2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_8x8),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_8x8),this),
									8,8, TILES_PER_PAGE_X * 4, TILES_PER_PAGE_Y * 2);
		m_tilemap[layer][1][3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cischeat_state::cischeat_get_scroll_tile_info_8x8),this), tilemap_mapper_delegate(FUNC(cischeat_state::cischeat_scan_8x8),this),
									8,8, TILES_PER_PAGE_X * 2, TILES_PER_PAGE_Y * 4);

		/* set user data and transparency */
		for (i = 0; i < 8; i++)
		{
			m_tilemap[layer][i/4][i%4]->set_user_data((void *)(FPTR)layer);
			m_tilemap[layer][i/4][i%4]->set_transparent_pen(15);
		}
	}
}

void cischeat_state::cischeat_set_vreg_flag(int which, int data)
{
	if (m_scroll_flag[which] != data)
	{
		m_scroll_flag[which] = data;
		m_tmap[which] = m_tilemap[which][(data >> 4) & 1][data & 3];
		m_tmap[which]->mark_all_dirty();
	}
}

/* 32 colour codes for the tiles */
VIDEO_START_MEMBER(cischeat_state,cischeat)
{
	int i;

	m_spriteram = &m_ram[0x8000/2];

	create_tilemaps();
	m_tmap[0] = m_tilemap[0][0][0];
	m_tmap[1] = m_tilemap[1][0][0];
	m_tmap[2] = m_tilemap[2][0][0];

	m_active_layers = 0;

	for (i = 0; i < 3; i ++)
	{
		m_scroll_flag[i] = m_scrollx[i] = m_scrolly[i] = 0;
	}

	m_bits_per_color_code = 5;

	prepare_shadows();
}

/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

/* 16 colour codes for the tiles */
VIDEO_START_MEMBER(cischeat_state,f1gpstar)
{
	VIDEO_START_CALL_MEMBER(cischeat);

	m_bits_per_color_code = 4;
}

VIDEO_START_MEMBER(cischeat_state,bigrun)
{
	VIDEO_START_CALL_MEMBER(f1gpstar);
}



/***************************************************************************


                        Hardware registers access


***************************************************************************/

/*
    F1 GP Star has a real pedal, while Cisco Heat's is connected to
    a switch. The Former game stores, during boot, the value that
    corresponds to the pedal not pressed, and compares against it:

    The value returned must decrease when the pedal is pressed.
*/

/**************************************************************************
                                Big Run
**************************************************************************/

READ16_MEMBER(cischeat_state::bigrun_vregs_r)
{
	switch (offset)
	{
		case 0x0000/2 : return ioport("IN1")->read();   // Coins
		case 0x0002/2 : return ioport("IN2")->read();   // Buttons
		case 0x0004/2 : return ioport("IN3")->read();   // Motor Limit Switches
		case 0x0006/2 : return ioport("IN4")->read();   // DSW 1 & 2

		case 0x0008/2 : return soundlatch2_word_r(space,0,0xffff);  // From sound cpu

		case 0x0010/2 :
			switch (m_ip_select & 0x3)
			{
				case 0 : return ioport("IN6")->read();      // Driving Wheel
				case 1 : return 0xffff;                 // Cockpit: Up / Down Position
				case 2 : return 0xffff;                 // Cockpit: Left / Right Position?
				case 3 : return ioport("PEDAL")->read();    // Accelerator (Pedal)
				default: return 0xffff;
			}


		case 0x2200/2 : return ioport("IN5")->read();   // DSW 3 (4 bits)

		default:    SHOW_READ_ERROR("vreg %04X read!",offset*2);
					return m_vregs[offset];
	}
}

WRITE16_MEMBER(cischeat_state::bigrun_vregs_w)
{
	UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);

	switch (offset)
	{
		case 0x0000/2   :   // leds
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0,new_data & 0x01);
				machine().bookkeeping().coin_counter_w(1,new_data & 0x02);
				output().set_led_value(0,new_data & 0x10);   // start button
				output().set_led_value(1,new_data & 0x20);   // ?
			}
			break;

		case 0x0002/2   :   // ?? 91/1/91/1 ...
			break;

		case 0x0004/2   :   // motor (seat?)
			if (ACCESSING_BITS_0_7)
				output().set_led_value(2, (new_data != old_data) ? 1 : 0);
			break;

		case 0x0006/2   :   // motor (wheel?)
			break;

		case 0x000a/2   :   // to sound cpu
			soundlatch_word_w(space,0,new_data,0xffff);
			break;

		case 0x000c/2   :   break;  // ??

		case 0x0010/2   : m_ip_select = new_data;   break;
		case 0x0012/2   : m_ip_select = new_data+1; break; // value above + 1

		case 0x2000/2+0 : CISCHEAT_VREG_SCROLL(0,x);        break;
		case 0x2000/2+1 : CISCHEAT_VREG_SCROLL(0,y);        break;
		case 0x2000/2+2 : cischeat_set_vreg_flag(0,new_data);break;

		case 0x2008/2+0 : CISCHEAT_VREG_SCROLL(1,x);        break;
		case 0x2008/2+1 : CISCHEAT_VREG_SCROLL(1,y);        break;
		case 0x2008/2+2 : cischeat_set_vreg_flag(1,new_data);break;

		case 0x2100/2+0 : CISCHEAT_VREG_SCROLL(2,x);        break;
		case 0x2100/2+1 : CISCHEAT_VREG_SCROLL(2,y);        break;
		case 0x2100/2+2 : cischeat_set_vreg_flag(2,new_data);break;

		case 0x2108/2   : break;    // ? written with 0 only
		case 0x2208/2   : break;    // watchdog reset

		/* Not sure about this one.. */
		case 0x2308/2   :   m_cpu2->set_input_line(INPUT_LINE_RESET, (new_data & 2) ? ASSERT_LINE : CLEAR_LINE );
							m_cpu3->set_input_line(INPUT_LINE_RESET, (new_data & 2) ? ASSERT_LINE : CLEAR_LINE );
							m_soundcpu->set_input_line(INPUT_LINE_RESET, (new_data & 1) ? ASSERT_LINE : CLEAR_LINE );
							break;

		default: SHOW_WRITE_ERROR("vreg %04X <- %04X",offset*2,data);
	}
}


/**************************************************************************
                                Cisco Heat
**************************************************************************/

READ16_MEMBER(cischeat_state::cischeat_vregs_r)
{
	switch (offset)
	{
		case 0x0000/2 : return ioport("IN1")->read();   // Coins
		case 0x0002/2 : return ioport("IN2")->read();   // Buttons
		case 0x0004/2 : return ioport("IN3")->read();   // Motor Limit Switches
		case 0x0006/2 : return ioport("IN4")->read();   // DSW 1 & 2

		case 0x0010/2 :
			switch (m_ip_select & 0x3)
			{
				case 0 : return ioport("IN6")->read();  // Driving Wheel
				case 1 : return ~0;                 // Cockpit: Up / Down Position?
				case 2 : return ~0;                 // Cockpit: Left / Right Position?
				default: return ~0;
			}

		case 0x2200/2 : return ioport("IN5")->read();   // DSW 3 (4 bits)
		case 0x2300/2 : return soundlatch2_byte_r(space,0); // From sound cpu

		default:    SHOW_READ_ERROR("vreg %04X read!",offset*2);
					return m_vregs[offset];
	}
}

WRITE16_MEMBER(cischeat_state::cischeat_vregs_w)
{
	UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);

	switch (offset)
	{
		case 0x0000/2   :   // leds
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0,new_data & 0x01);
				machine().bookkeeping().coin_counter_w(1,new_data & 0x02);
				output().set_led_value(0,new_data & 0x10);   // start button
				output().set_led_value(1,new_data & 0x20);   // ?
			}
			break;

		case 0x0002/2   :   // ?? 91/1/91/1 ...
			break;

		case 0x0004/2   :   // motor (seat?)
			if (ACCESSING_BITS_0_7)
				output().set_led_value(2, (new_data != old_data) ? 1 : 0);
			break;

		case 0x0006/2   :   // motor (wheel?)
			break;

		case 0x0010/2   : m_ip_select = new_data;   break;
		case 0x0012/2   : break; // value above + 1

		case 0x2000/2+0 : CISCHEAT_VREG_SCROLL(0,x);        break;
		case 0x2000/2+1 : CISCHEAT_VREG_SCROLL(0,y);        break;
		case 0x2000/2+2 : cischeat_set_vreg_flag(0,new_data);break;

		case 0x2008/2+0 : CISCHEAT_VREG_SCROLL(1,x);        break;
		case 0x2008/2+1 : CISCHEAT_VREG_SCROLL(1,y);        break;
		case 0x2008/2+2 : cischeat_set_vreg_flag(1,new_data);break;

		case 0x2100/2+0 : CISCHEAT_VREG_SCROLL(2,x);        break;
		case 0x2100/2+1 : CISCHEAT_VREG_SCROLL(2,y);        break;
		case 0x2100/2+2 : cischeat_set_vreg_flag(2,new_data);break;

		case 0x2108/2   : break;    // ? written with 0 only
		case 0x2208/2   : break;    // watchdog reset

		case 0x2300/2   :   /* Sound CPU: reads latch during int 4, and stores command */
							soundlatch_word_w(space, 0, new_data, 0xffff);
							m_soundcpu->set_input_line(4, HOLD_LINE);
							break;

		/* Not sure about this one.. */
		case 0x2308/2   :   m_cpu2->set_input_line(INPUT_LINE_RESET, (new_data & 2) ? ASSERT_LINE : CLEAR_LINE );
							m_cpu3->set_input_line(INPUT_LINE_RESET, (new_data & 2) ? ASSERT_LINE : CLEAR_LINE );
							m_soundcpu->set_input_line(INPUT_LINE_RESET, (new_data & 1) ? ASSERT_LINE : CLEAR_LINE );
							break;

		default: SHOW_WRITE_ERROR("vreg %04X <- %04X",offset*2,data);
	}
}



/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

READ16_MEMBER(cischeat_state::f1gpstar_vregs_r)
{
	switch (offset)
	{
		case 0x0000/2 : return ioport("IN1")->read();   // DSW 1 & 2

//      case 0x0002/2 : return 0xFFFF;

		case 0x0004/2 : return ioport("IN2")->read();   // Buttons

		case 0x0006/2 : return ioport("IN3")->read();   // ? Read at boot only

		case 0x0008/2 : return soundlatch2_byte_r(space,0);     // From sound cpu

		case 0x000c/2 : return ioport("IN4")->read();   // DSW 3

		case 0x0010/2 : // Accel + Driving Wheel
			return (ioport("PEDAL")->read() & 0xff) + ((ioport("IN5")->read() & 0xff)<<8);

		default:        SHOW_READ_ERROR("vreg %04X read!",offset*2);
						return m_vregs[offset];
	}
}

READ16_MEMBER(cischeat_state::f1gpstr2_vregs_r)
{
	if ((offset >= 0x1000/2) && (offset < 0x2000/2))
		return m_vregs[offset];

	switch (offset)
	{
		case 0x0018/2 :
			return (m_f1gpstr2_ioready[0]&1) ? 0xff : 0xf0;

		default:
			return f1gpstar_vregs_r(space,offset,mem_mask);
	}
}

/**************************************************************************
                            Wild Pilot
**************************************************************************/

READ16_MEMBER(cischeat_state::wildplt_vregs_r)
{
	if ((offset >= 0x1000/2) && (offset < 0x2000/2))
		return m_vregs[offset];

	switch (offset)
	{
		case 0x0000/2 : return ioport("IN0")->read(); // DSW 1 & 2

		case 0x0004/2 : return ioport("IN1")->read(); // Buttons

		case 0x0008/2 : return soundlatch2_byte_r(space,0); // From sound cpu

		case 0x0010/2 : // X, Y
			return ioport("IN2")->read() | (ioport("IN3")->read()<<8);

		case 0x0018/2 :
			return (m_f1gpstr2_ioready[0]&1) ? 0xff : 0xf0;

		default: SHOW_READ_ERROR("vreg %04X read!",offset*2);
			return m_vregs[offset];
	}
}


WRITE16_MEMBER(cischeat_state::f1gpstar_vregs_w)
{
//  UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);

	switch (offset)
	{
/*
CPU #0 PC 00234A : Warning, vreg 0000 <- 0000
CPU #0 PC 002350 : Warning, vreg 0002 <- 0000
CPU #0 PC 00235C : Warning, vreg 0006 <- 0000
*/
		// "shudder" motors, leds
		case 0x0004/2   :
			if (ACCESSING_BITS_0_7)
			{
				machine().bookkeeping().coin_counter_w(0,new_data & 0x01);
				machine().bookkeeping().coin_counter_w(1,new_data & 0x02);
				output().set_led_value(0,new_data & 0x04);   // start button
				output().set_led_value(1,new_data & 0x20);   // ?
				// wheel | seat motor
				output().set_led_value(2, ((new_data >> 3) | (new_data >> 4)) & 1 );
			}
			break;
		case 0x0014/2   :   break;

		/* Usually written in sequence, but not always */
		case 0x0008/2   :   soundlatch_word_w(space, 0, new_data, 0xffff);  break;
		case 0x0018/2   :   m_soundcpu->set_input_line(4, HOLD_LINE);   break;

		case 0x0010/2   :   break;

		case 0x2000/2+0 : CISCHEAT_VREG_SCROLL(0,x);        break;
		case 0x2000/2+1 : CISCHEAT_VREG_SCROLL(0,y);        break;
		case 0x2000/2+2 : cischeat_set_vreg_flag(0,new_data);break;

		case 0x2008/2+0 : CISCHEAT_VREG_SCROLL(1,x);        break;
		case 0x2008/2+1 : CISCHEAT_VREG_SCROLL(1,y);        break;
		case 0x2008/2+2 : cischeat_set_vreg_flag(1,new_data);break;

		case 0x2100/2+0 : CISCHEAT_VREG_SCROLL(2,x);        break;
		case 0x2100/2+1 : CISCHEAT_VREG_SCROLL(2,y);        break;
		case 0x2100/2+2 : cischeat_set_vreg_flag(2,new_data);break;

		case 0x2108/2   : break;    // ? written with 0 only
		case 0x2208/2   : break;    // watchdog reset

		/* Not sure about this one. Values: $10 then 0, $7 then 0 */
		case 0x2308/2   :   m_cpu2->set_input_line(INPUT_LINE_RESET, (new_data & 1) ? ASSERT_LINE : CLEAR_LINE );
							m_cpu3->set_input_line(INPUT_LINE_RESET, (new_data & 2) ? ASSERT_LINE : CLEAR_LINE );
							m_soundcpu->set_input_line(INPUT_LINE_RESET, (new_data & 4) ? ASSERT_LINE : CLEAR_LINE );
							break;

		default:        SHOW_WRITE_ERROR("vreg %04X <- %04X",offset*2,data);
	}
}

WRITE16_MEMBER(cischeat_state::f1gpstr2_vregs_w)
{
	UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);

	if ((offset >= 0x1000/2) && (offset < 0x2000/2))
		return;

	switch (offset)
	{
		case 0x0000/2   :
			if (ACCESSING_BITS_0_7)
			{
				if((old_data & 4) && ((new_data & 4) == 0))
					m_cpu5->set_input_line(4, HOLD_LINE);
				if((old_data & 2) && ((new_data & 2) == 0))
					m_cpu5->set_input_line(2, HOLD_LINE);
			}
			break;

		default:
			f1gpstar_vregs_w(space, offset, data, mem_mask);
			break;
	}
}

/**************************************************************************
                                Scud Hammer
**************************************************************************/

WRITE16_MEMBER(cischeat_state::scudhamm_vregs_w)
{
//  int old_data = m_vregs[offset];
	int new_data = COMBINE_DATA(&m_vregs[offset]);

	switch (offset)
	{
		case 0x000/2+0 : CISCHEAT_VREG_SCROLL(0,x);     break;
		case 0x000/2+1 : CISCHEAT_VREG_SCROLL(0,y);     break;
		case 0x000/2+2 : cischeat_set_vreg_flag(0,new_data);break;

//      UNUSED LAYER
		case 0x008/2+0 :
		case 0x008/2+1 :
		case 0x008/2+2 :
			break;

		case 0x100/2+0 : CISCHEAT_VREG_SCROLL(2,x);     break;
		case 0x100/2+1 : CISCHEAT_VREG_SCROLL(2,y);     break;
		case 0x100/2+2 : cischeat_set_vreg_flag(2,new_data);break;

		case 0x208/2   : watchdog_reset_w(space,0,0);   break;

		default: SHOW_WRITE_ERROR("vreg %04X <- %04X",offset*2,data);
	}
}




/***************************************************************************


                                Road Drawing


***************************************************************************/

/* horizontal size of 1 line and 1 tile of the road */
#define X_SIZE (1024)
#define TILE_SIZE (64)


/**************************************************************************

                        Cisco Heat road format


    Offset:     Bits:                   Value:

    00.w                                Code

    02.w                                X Scroll

    04.w        fedc ---- ---- ----     unused?
                ---- ba98 ---- ----     Priority
                ---- ---- 76-- ----     unused?
                ---- ---- --54 3210     Color

    06.w                                Unused

**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void cischeat_state::cischeat_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency)
{
	int curr_code,sx,sy;
	int min_priority, max_priority;

	rectangle rect      =   cliprect;
	gfx_element *gfx        =   m_gfxdecode->gfx((road_num & 1)?5:4);

	UINT16 *roadram         =   m_roadram[road_num & 1];

	int min_y = rect.min_y;
	int max_y = rect.max_y;

	int max_x = rect.max_x;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	/* Move the priority values in place */
	min_priority = (min_priority & 7) * 0x100;
	max_priority = (max_priority & 7) * 0x100;

	/* Let's draw from the top to the bottom of the visible screen */
	for (sy = min_y ; sy <= max_y ; sy ++)
	{
		int code    = roadram[ sy * 4 + 0 ];
		int xscroll = roadram[ sy * 4 + 1 ];
		int attr    = roadram[ sy * 4 + 2 ];

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code = code * (X_SIZE/TILE_SIZE);

		xscroll %= X_SIZE;
		curr_code = code + xscroll/TILE_SIZE;

		for (sx = -(xscroll%TILE_SIZE) ; sx <= max_x ; sx +=TILE_SIZE)
		{
			gfx->transpen(bitmap,rect,
					curr_code++,
					attr,
					0,0,
					sx,sy,
					transparency ? 15 : -1);

			/* wrap around */
			if (curr_code%(X_SIZE/TILE_SIZE)==0)    curr_code = code;
		}
	}

}



/**************************************************************************

                        F1 GrandPrix Star road format

    Offset:     Bits:                   Value:

    00.w        fedc ---- ---- ----     Priority
                ---- ba98 7654 3210     X Scroll (After Zoom)

    02.w        fedc ba-- ---- ----     unused?
                ---- --98 7654 3210     X Zoom

    04.w        fe-- ---- ---- ----     unused?
                --dc ba98 ---- ----     Color
                ---- ---- 7654 3210     ?

    06.w                                Code


    Imagine an "empty" line, 2 * X_SIZE wide, with the gfx from
    the ROM - whose original size is X_SIZE - in the middle.

    Zooming acts on this latter and can shrink it to 1 pixel or
    widen it to 2 * X_SIZE, while *keeping it centered* in the
    empty line. Scrolling acts on the resulting line.


**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void cischeat_state::f1gpstar_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency)
{
	int sx,sy;
	int xstart;
	int min_priority, max_priority;

	rectangle rect      =   cliprect;
	gfx_element *gfx        =   m_gfxdecode->gfx((road_num & 1)?5:4);

	UINT16 *roadram         =   m_roadram[road_num & 1];

	int min_y = rect.min_y;
	int max_y = rect.max_y;

	int max_x = rect.max_x << 16;   // use fixed point values (16.16), for accuracy

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	/* Move the priority values in place */
	min_priority = (min_priority & 7) * 0x1000;
	max_priority = (max_priority & 7) * 0x1000;

	/* Let's draw from the top to the bottom of the visible screen */
	for (sy = min_y ; sy <= max_y ; sy ++)
	{
		int xscale, xdim;

		int xscroll = roadram[ sy * 4 + 0 ];
		int xzoom   = roadram[ sy * 4 + 1 ];
		int attr    = roadram[ sy * 4 + 2 ];
		int code    = roadram[ sy * 4 + 3 ];

		/* highest nibble is a priority information */
		if ( ((xscroll & 0x7000) < min_priority) || ((xscroll & 0x7000) > max_priority) )
			continue;

		/* zoom code range: 000-3ff     scale range: 0.0-2.0 */
		xscale = ( ((xzoom & 0x3ff)+1) << (16+1) ) / 0x400;

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code    = code * (X_SIZE/TILE_SIZE);

		/* dimension of a tile after zoom */
		xdim = TILE_SIZE * xscale;

		xscroll %= 2 * X_SIZE;

		xstart  = (X_SIZE - xscroll) * 0x10000;
		xstart -= (X_SIZE * xscale) / 2;

		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		xscale += (1<<16)/TILE_SIZE;

		/* Draw the line */
		for (sx = xstart ; sx <= max_x ; sx += xdim)
		{
			gfx->zoom_transpen(bitmap,rect,
						code++,
						attr >> 8,
						0,0,
						sx / 0x10000, sy,
						xscale, 1 << 16,
						transparency ? 15 : -1);

			/* stop when the end of the line of gfx is reached */
			if ((code % (X_SIZE/TILE_SIZE)) == 0)   break;
		}

	}
}


/***************************************************************************

                Cisco Heat & F1 GP Star Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02/04   fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Flip X/Y
            ---- ba9- ---- ----     ? X/Y zoom ?
            ---- ---8 7654 3210     X/Y zoom

    06/08   fedc ba-- ---- ----     ? X/Y position ?
            ---- --98 7654 3210     X/Y position

    0A                              0 ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 7--- ----     unused?
            ---- ---- -654 3210     Color

***************************************************************************/

#define SHRINK(_org_,_fact_) ( ( ( (_org_) << 16 ) * (_fact_ & 0x01ff) ) / 0x80 )

/*  Draw sprites, in the given priority range, to a bitmap.

    Priorities between 0 and 15 cover sprites whose priority nibble
    is between 0 and 15. Priorities between 0+16 and 15+16 cover
    sprites whose priority nibble is between 0 and 15 and whose
    colour code's high bit is set.  */

void cischeat_state::cischeat_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2)
{
	int x, sx, flipx, xzoom, xscale, xdim, xnum, xstart, xend, xinc;
	int y, sy, flipy, yzoom, yscale, ydim, ynum, ystart, yend, yinc;
	int code, attr, color, size, shadow;

	int min_priority, max_priority, high_sprites;

	UINT16      *source =   m_spriteram;
	const UINT16    *finish =   source + 0x1000/2;


	/* Move the priority values in place */
	high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 = (priority1 & 0x0f) * 0x100;
	priority2 = (priority2 & 0x0f) * 0x100;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	for (; source < finish; source += 0x10/2 )
	{
		size    =   source[ 0 ];
		if (size & 0x1000)  continue;

		/* number of tiles */
		xnum    =   ( (size & 0x0f) >> 0 ) + 1;
		ynum    =   ( (size & 0xf0) >> 4 ) + 1;

		xzoom   =   source[ 1 ];
		yzoom   =   source[ 2 ];
		flipx   =   xzoom & 0x1000;
		flipy   =   yzoom & 0x1000;

		sx      =   source[ 3 ];
		sy      =   source[ 4 ];
		sx      =   (sx & 0x1ff) - (sx & 0x200);
		sy      =   (sy & 0x1ff) - (sy & 0x200);

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* dimension of a tile after zoom */
#ifdef MAME_DEBUG
		if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed(KEYCODE_M) )
		{
			xdim    =   16 << 16;
			ydim    =   16 << 16;
		}
		else
#endif
		{
			xdim    =   SHRINK(16,xzoom);
			ydim    =   SHRINK(16,yzoom);
		}

		if ( ( (xdim / 0x10000) == 0 ) || ( (ydim / 0x10000) == 0) )    continue;

		/* the y pos passed to the hardware is the that of the last line,
		   we need the y pos of the first line  */
		sy -= (ydim * ynum);

		code    =   source[ 6 ];
		attr    =   source[ 7 ];
		color   =   attr & 0x007f;
		shadow  =   attr & 0x1000;

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		if ( high_sprites && !(color & 0x80) )
			continue;

#ifdef MAME_DEBUG
if ( (m_debugsprites) && ( ((attr & 0x0300)>>8) != (m_debugsprites-1) ) ) { continue; };
#endif

		xscale = xdim / 16;
		yscale = ydim / 16;


		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1<<16)/16;
		if (yscale & 0xffff)    yscale += (1<<16)/16;


		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		m_drawmode_table[ 0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				m_gfxdecode->gfx(3)->zoom_transtable(bitmap,cliprect,code++,color,flipx,flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, m_drawmode_table);
			}
		}
#ifdef MAME_DEBUG
#if 0
if (machine().input().code_pressed(KEYCODE_X))
{   /* Display some info on each sprite */
	sprintf(buf, "%04x",attr);
	ui_draw_text(buf, sx>>16, sy>>16);
}
#endif
#endif
	}   /* end sprite loop */
}


/***************************************************************************

                            Big Run Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02      fedc ba98 ---- ----     Y zoom
            ---- ---- 7654 3210     X zoom

    04/06   fed- ---- ---- ----
            ---c ---- ---- ----     X/Y flip
            ---- ba9- ---- ----
            ---- ---8 7654 3210     X/Y position (signed)

    08                              ?
    0A                              ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 76-- ----     unused?
            ---- ---- --54 3210     Color

***************************************************************************/

void cischeat_state::bigrun_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2)
{
	int x, sx, flipx, xzoom, xscale, xdim, xnum, xstart, xend, xinc;
	int y, sy, flipy, yzoom, yscale, ydim, ynum, ystart, yend, yinc;
	int code, attr, color, size, shadow;

	int min_priority, max_priority, high_sprites;

	UINT16      *source =   m_spriteram;
	const UINT16    *finish =   source + 0x1000/2;

	/* Move the priority values in place */
	high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 = (priority1 & 0x0f) * 0x100;
	priority2 = (priority2 & 0x0f) * 0x100;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	for (; source < finish; source += 0x10/2 )
	{
		size    =   source[ 0 ];
		if (size & 0x1000)  continue;

		/* number of tiles */
		xnum    =   ( (size & 0x0f) >> 0 ) + 1;
		ynum    =   ( (size & 0xf0) >> 4 ) + 1;

		yzoom   =   (source[ 1 ] >> 8) & 0xff;
		xzoom   =   (source[ 1 ] >> 0) & 0xff;

		sx      =   source[ 2 ];
		sy      =   source[ 3 ];
		flipx   =   sx & 0x1000;
		flipy   =   sy & 0x1000;
//      sx      =   (sx & 0x1ff) - (sx & 0x200);
//      sy      =   (sy & 0x1ff) - (sy & 0x200);
		sx      =   (sx & 0x0ff) - (sx & 0x100);
		sy      =   (sy & 0x0ff) - (sy & 0x100);

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* dimension of a tile after zoom */
#ifdef MAME_DEBUG
		if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed(KEYCODE_M) )
		{
			xdim    =   16 << 16;
			ydim    =   16 << 16;
		}
		else
#endif
		{
			xdim    =   SHRINK(16,xzoom);
			ydim    =   SHRINK(16,yzoom);
		}

		if ( ( (xdim / 0x10000) == 0 ) || ( (ydim / 0x10000) == 0) )    continue;

//      sy -= (ydim * ynum);

		code    =   source[ 6 ];
		attr    =   source[ 7 ];
		color   =   attr & 0x007f;
		shadow  =   attr & 0x1000;

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		if ( high_sprites && !(color & 0x80) )
			continue;

#ifdef MAME_DEBUG
if ( (m_debugsprites) && ( ((attr & 0x0300)>>8) != (m_debugsprites-1) ) ) { continue; };
#endif

		xscale = xdim / 16;
		yscale = ydim / 16;


		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1<<16)/16;
		if (yscale & 0xffff)    yscale += (1<<16)/16;


		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		m_drawmode_table[ 0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				m_gfxdecode->gfx(3)->zoom_transtable(bitmap,cliprect,code++,color,flipx,flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, m_drawmode_table);
			}
		}
#ifdef MAME_DEBUG
#if 0
if (machine().input().code_pressed(KEYCODE_X))
{   /* Display some info on each sprite */
	sprintf(buf, "%04x",attr);
	ui_draw_text(buf, sx>>16, sy>>16);
}
#endif
#endif
	}   /* end sprite loop */
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

#ifdef MAME_DEBUG
#define CISCHEAT_LAYERSCTRL \
m_debugsprites = 0; \
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) ) \
{ \
	int msk = 0; \
	if (machine().input().code_pressed(KEYCODE_Q))  { msk |= 0x01;} \
	if (machine().input().code_pressed(KEYCODE_W))  { msk |= 0x02;} \
	if (machine().input().code_pressed(KEYCODE_E))  { msk |= 0x04;} \
	if (machine().input().code_pressed(KEYCODE_A))  { msk |= 0x08; m_debugsprites = 1;} \
	if (machine().input().code_pressed(KEYCODE_S))  { msk |= 0x08; m_debugsprites = 2;} \
	if (machine().input().code_pressed(KEYCODE_D))  { msk |= 0x08; m_debugsprites = 3;} \
	if (machine().input().code_pressed(KEYCODE_F))  { msk |= 0x08; m_debugsprites = 4;} \
	if (machine().input().code_pressed(KEYCODE_R))  { msk |= 0x10;} \
	if (machine().input().code_pressed(KEYCODE_T))  { msk |= 0x20;} \
	\
	if (msk != 0) m_active_layers &= msk; \
} \
\
{ \
	if ( machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed_once(KEYCODE_U) ) \
		m_show_unknown ^= 1; \
	if (m_show_unknown) \
		popmessage("0:%04X 2:%04X 4:%04X 6:%04X c:%04X", \
			m_vregs[0],m_vregs[1],m_vregs[2],m_vregs[3],m_vregs[0xc/2] ); \
}
#else
#define CISCHEAT_LAYERSCTL
#endif

/**************************************************************************
                                Big Run
**************************************************************************/

UINT32 cischeat_state::screen_update_bigrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int active_layers1, flag;

#ifdef MAME_DEBUG
	/* FAKE Videoreg */
	m_active_layers = m_vregs[0x2400/2];
	if (m_active_layers == 0)   m_active_layers = 0x3f;
#else
	m_active_layers = 0x3f;
#endif

	active_layers1 = m_active_layers;

#ifdef MAME_DEBUG
	CISCHEAT_LAYERSCTRL
#endif

	cischeat_tmap_SET_SCROLL(0)
	cischeat_tmap_SET_SCROLL(1)
	cischeat_tmap_SET_SCROLL(2)

	bitmap.fill(0, cliprect);

	for (i = 7; i >= 4; i--)
	{                                           /* bitmap, road, min_priority, max_priority, transparency */
		if (m_active_layers & 0x10) cischeat_draw_road(bitmap,cliprect,0,i,i,FALSE);
		if (m_active_layers & 0x20) cischeat_draw_road(bitmap,cliprect,1,i,i,TRUE);
	}

	flag = 0;
	cischeat_tmap_DRAW(0)
	cischeat_tmap_DRAW(1)

	for (i = 3; i >= 0; i--)
	{                                           /* bitmap, road, min_priority, max_priority, transparency */
		if (m_active_layers & 0x10) cischeat_draw_road(bitmap,cliprect,0,i,i,TRUE);
		if (m_active_layers & 0x20) cischeat_draw_road(bitmap,cliprect,1,i,i,TRUE);
	}

	if (m_active_layers & 0x08) bigrun_draw_sprites(bitmap,cliprect,15,0);

	cischeat_tmap_DRAW(2)

	m_active_layers = active_layers1;
	return 0;
}


/**************************************************************************
                                Cisco Heat
**************************************************************************/

UINT32 cischeat_state::screen_update_cischeat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1, flag;

#ifdef MAME_DEBUG
	/* FAKE Videoreg */
	m_active_layers = m_vregs[0x2400/2];
	if (m_active_layers == 0)   m_active_layers = 0x3f;
#else
	m_active_layers = 0x3f;
#endif

	active_layers1 = m_active_layers;

#ifdef MAME_DEBUG
	CISCHEAT_LAYERSCTRL
#endif

	cischeat_tmap_SET_SCROLL(0)
	cischeat_tmap_SET_SCROLL(1)
	cischeat_tmap_SET_SCROLL(2)

	bitmap.fill(0, cliprect);

										/* bitmap, road, priority, transparency */
	if (m_active_layers & 0x10) cischeat_draw_road(bitmap,cliprect,0,7,5,FALSE);
	if (m_active_layers & 0x20) cischeat_draw_road(bitmap,cliprect,1,7,5,TRUE);

	flag = 0;
	cischeat_tmap_DRAW(0)
//  else bitmap.fill(0, cliprect);
	cischeat_tmap_DRAW(1)

	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,15,3);
	if (m_active_layers & 0x10) cischeat_draw_road(bitmap,cliprect,0,4,1,TRUE);
	if (m_active_layers & 0x20) cischeat_draw_road(bitmap,cliprect,1,4,1,TRUE);
	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,2,2);
	if (m_active_layers & 0x10) cischeat_draw_road(bitmap,cliprect,0,0,0,TRUE);
	if (m_active_layers & 0x20) cischeat_draw_road(bitmap,cliprect,1,0,0,TRUE);
	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,1,0);
	cischeat_tmap_DRAW(2)

	/* for the map screen */
	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,0+16,0+16);


	m_active_layers = active_layers1;
	return 0;
}



/**************************************************************************
                            F1 GrandPrix Star
**************************************************************************/

UINT32 cischeat_state::screen_update_f1gpstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1, flag;

#ifdef MAME_DEBUG
	/* FAKE Videoreg */
	m_active_layers = m_vregs[0x2400/2];
	if (m_active_layers == 0)   m_active_layers = 0x3f;
#else
	m_active_layers = 0x3f;
#endif

	active_layers1 = m_active_layers;

#ifdef MAME_DEBUG
	CISCHEAT_LAYERSCTRL
#endif

	cischeat_tmap_SET_SCROLL(0);
	cischeat_tmap_SET_SCROLL(1);
	cischeat_tmap_SET_SCROLL(2);

	bitmap.fill(0, cliprect);

/*  1: clouds 5, grad 7, road 0     2: clouds 5, grad 7, road 0, tunnel roof 0 */

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (m_active_layers & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,6,7,TRUE);
	if (m_active_layers & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,6,7,TRUE);

	flag = 0;
	cischeat_tmap_DRAW(0)
//  else bitmap.fill(0, cliprect);
	cischeat_tmap_DRAW(1)

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (m_active_layers & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,1,5,TRUE);
	if (m_active_layers & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,1,5,TRUE);

	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,15,2);

	/* road 1!! 0!! */                  /* bitmap, road, min_priority, max_priority, transparency */
	if (m_active_layers & 0x20) f1gpstar_draw_road(bitmap,cliprect,1,0,0,TRUE);
	if (m_active_layers & 0x10) f1gpstar_draw_road(bitmap,cliprect,0,0,0,TRUE);

	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,1,1);
	cischeat_tmap_DRAW(2)
	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,0,0);


	m_active_layers = active_layers1;
	return 0;
}



/**************************************************************************
                                Scud Hammer
**************************************************************************/

UINT32 cischeat_state::screen_update_scudhamm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int active_layers1, flag;
	active_layers1 = m_active_layers;
	m_active_layers = 0x0d;

#ifdef MAME_DEBUG
m_debugsprites = 0;
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  { msk |= 0x1;}
	if (machine().input().code_pressed(KEYCODE_W))  { msk |= 0x2;}
	if (machine().input().code_pressed(KEYCODE_E))  { msk |= 0x4;}
	if (machine().input().code_pressed(KEYCODE_A))  { msk |= 0x8; m_debugsprites = 1;}
	if (machine().input().code_pressed(KEYCODE_S))  { msk |= 0x8; m_debugsprites = 2;}
	if (machine().input().code_pressed(KEYCODE_D))  { msk |= 0x8; m_debugsprites = 3;}
	if (machine().input().code_pressed(KEYCODE_F))  { msk |= 0x8; m_debugsprites = 4;}

	if (msk != 0) m_active_layers &= msk;
#if 1
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		popmessage("Cmd: %04X Pos:%04X Lim:%04X Inp:%04X",
							m_scudhamm_motor_command,
							scudhamm_motor_pos_r(space,0,0xffff),
							scudhamm_motor_status_r(space,0,0xffff),
							scudhamm_analog_r(space,0,0xffff) );

#if 0
	// captflag
	{
		popmessage( "[%04x] [%04x]\nLEDS %04x", m_captflag_motor_command[LEFT], m_captflag_motor_command[RIGHT], m_captflag_leds );
		m_captflag_motor_command[LEFT] = m_captflag_motor_command[RIGHT] = 0;
	}
#endif

	}
#endif

}
#endif

	cischeat_tmap_SET_SCROLL(0)
	cischeat_tmap_SET_SCROLL(2)

	bitmap.fill(0, cliprect);

	flag = 0;
	cischeat_tmap_DRAW(0)
	// no layer 1
	if (m_active_layers & 0x08) cischeat_draw_sprites(bitmap,cliprect,0,15);
	cischeat_tmap_DRAW(2)

	m_active_layers = active_layers1;
	return 0;
}
