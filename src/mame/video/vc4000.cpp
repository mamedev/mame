// license:GPL-2.0+
// copyright-holders:Peter Trauner, Manfred Schneider, Robbbert
// thanks-to:Manfred Schneider
#include "includes/vc4000.h"


/*
  emulation of signetics 2636 video/audio device

  seams to me like a special microcontroller
  mask programmed

  note about mame s2636 emulation:
  less encapsuled
  missing grid, sound generation, retriggered sprites, paddle reading, score display
  not "rasterline" based
 */

#define STICKCENTRE (105)
#define STICKLOW    (20)
#define STICKHIGH   (225)

#define VC4000_END_LINE (269)



void vc4000_state::video_start()
{
	int i;

	for (i=0;i<0x20; i++)
	{
		m_sprite_collision[i]=0;
		if ((i&3)==3) m_sprite_collision[i]|=0x20;
		if ((i&5)==5) m_sprite_collision[i]|=0x10;
		if ((i&9)==9) m_sprite_collision[i]|=8;
		if ((i&6)==6) m_sprite_collision[i]|=4;
		if ((i&0xa)==0xa) m_sprite_collision[i]|=2;
		if ((i&0xc)==0xc) m_sprite_collision[i]|=1;
		m_background_collision[i]=0;
		if ((i&0x11)==0x11) m_background_collision[i]|=0x80;
		if ((i&0x12)==0x12) m_background_collision[i]|=0x40;
		if ((i&0x14)==0x14) m_background_collision[i]|=0x20;
		if ((i&0x18)==0x18) m_background_collision[i]|=0x10;
	}

	m_joy1_x = STICKCENTRE;
	m_joy1_y = STICKCENTRE;
	m_joy2_x = STICKCENTRE;
	m_joy2_y = STICKCENTRE;

	memset(&m_video, 0, sizeof(m_video));
	for (i=0; i<3; i++)
	{
		m_video.sprites[i].data = &m_video.reg.d.sprites[i];
		m_video.sprites[i].mask = 1 << i;
	}
	m_video.sprites[3].data = &m_video.reg.d.sprite4;
	m_video.sprites[3].mask = 1 << 3;

	m_bitmap = std::make_unique<bitmap_ind16>(m_screen->width(), m_screen->height());
}

inline UINT8 vc4000_state::vc4000_joystick_return_to_centre(UINT8 joy)
{
	UINT8 data;

	if (joy > (STICKCENTRE+5))
		data=joy-5;
	else
	if (joy < (STICKCENTRE-5))
		data=joy+5;
	else
		data=105;

	return data;
}

READ8_MEMBER( vc4000_state::vc4000_video_r )
{
	UINT8 data=0;
	if (offset > 0xcf) offset &= 0xcf;  // c0-cf is mirrored at d0-df, e0-ef, f0-ff
	switch (offset)
	{
	case 0xca:      // Background-sprite collision (bit 7-4) and sprite finished (bit 3-0)
		data |= m_video.background_collision;
		m_video.background_collision=0;
		if (m_video.sprites[3].finished)
		{
			data |= 1;
			m_video.sprites[3].finished = FALSE;
		}

		if (m_video.sprites[2].finished)
		{
			data |= 2;
			m_video.sprites[2].finished = FALSE;
		}
		if (m_video.sprites[1].finished)
		{
			data |= 4;
			m_video.sprites[1].finished = FALSE;
		}
		if (m_video.sprites[0].finished)
		{
			data |= 8;
			m_video.sprites[0].finished = FALSE;
		}
		break;

	case 0xcb:      //    VRST-Flag (bit 6) and intersprite collision (bit5-0)
		data = m_video.sprite_collision | (m_video.reg.d.sprite_collision & 0xc0);
		m_video.sprite_collision = 0;
		m_video.reg.d.sprite_collision &= 0xbf;
		break;

#ifndef ANALOG_HACK
	case 0xcc:
		if (!activeS2650_FO)) data = m_io_joy1_x->read();
		else data = m_io_joy1_y->read();
		break;
	case 0xcd:
		if (!activecpu_get_reg(S2650_FO)) data = m_io_joy2_x->read();
		else data = m_io_joy2_y->read();
		break;
#else

	case 0xcc:      /* left joystick */
		if (m_config->read()&1)
		{       /* paddle */
			if (!m_maincpu->state_int(S2650_FO))
			{
				data = m_joys->read() & 0x03;
				switch (data)
				{
				case 0x01:
					m_joy1_x-=5;
					if (m_joy1_x < STICKLOW) m_joy1_x=STICKLOW;
					break;
				case 0x02:
					m_joy1_x+=5;
					if (m_joy1_x > STICKHIGH) m_joy1_x=STICKHIGH;
					break;
				case 0x00:
					m_joy1_x = vc4000_joystick_return_to_centre(m_joy1_x);
				}
				data = m_joy1_x;
			}
			else
			{
				data = m_joys->read() & 0x0c;
				switch (data)
				{
				case 0x08:
					m_joy1_y-=5;
					if (m_joy1_y < STICKLOW) m_joy1_y=STICKLOW;
					break;
				case 0x04:
					m_joy1_y+=5;
					if (m_joy1_y > STICKHIGH) m_joy1_y=STICKHIGH;
					break;
		//      case 0x00:
		//          m_joy1_y = vc4000_joystick_return_to_centre(m_joy1_y);
				}
				data = m_joy1_y;
			}
		}
		else
		{       /* buttons */
			if (!m_maincpu->state_int(S2650_FO))
			{
				data = m_joys->read() & 0x03;
				switch (data)
				{
				case 0x01:
					m_joy1_x=STICKLOW;
					break;
				case 0x02:
					m_joy1_x=STICKHIGH;
					break;
				default:            /* autocentre */
					m_joy1_x=STICKCENTRE;
				}
				data = m_joy1_x;
			}
			else
			{
				data = m_joys->read() & 0x0c;
				switch (data)
				{
				case 0x08:
					m_joy1_y=STICKLOW;
					break;
				case 0x04:
					m_joy1_y=STICKHIGH;
					break;
				default:
					m_joy1_y=STICKCENTRE;
				}
				data = m_joy1_y;
			}
		}
		break;

	case 0xcd:      /* right joystick */
		if (m_config->read()&1)
		{
			if (!m_maincpu->state_int(S2650_FO))
			{
				data = m_joys->read() & 0x30;
				switch (data)
				{
				case 0x10:
					m_joy2_x-=5;
					if (m_joy2_x < STICKLOW) m_joy2_x=STICKLOW;
					break;
				case 0x20:
					m_joy2_x+=5;
					if (m_joy2_x > STICKHIGH) m_joy2_x=STICKHIGH;
				case 0x00:
					m_joy2_x = vc4000_joystick_return_to_centre(m_joy2_x);
					break;
				}
				data = m_joy2_x;
			}
			else
			{
				data = m_joys->read() & 0xc0;
				switch (data)
				{
				case 0x80:
					m_joy2_y-=5;
					if (m_joy2_y < STICKLOW) m_joy2_y=STICKLOW;
					break;
				case 0x40:
					m_joy2_y+=5;
					if (m_joy2_y > STICKHIGH) m_joy2_y=STICKHIGH;
					break;
			//  case 0x00:
			//      m_joy2_y = vc4000_joystick_return_to_centre(m_joy2_y);
				}
				data = m_joy2_y;
			}
		}
		else
		{
			if (!m_maincpu->state_int(S2650_FO))
			{
				data = m_joys->read() & 0x30;
				switch (data)
				{
				case 0x10:
					m_joy2_x=STICKLOW;
					break;
				case 0x20:
					m_joy2_x=STICKHIGH;
					break;
				default:            /* autocentre */
					m_joy2_x=STICKCENTRE;
				}
				data = m_joy2_x;
			}
			else
			{
				data = m_joys->read() & 0xc0;
				switch (data)
				{
				case 0x80:
					m_joy2_y=STICKLOW;
					break;
				case 0x40:
					m_joy2_y=STICKHIGH;
					break;
				default:
					m_joy2_y=STICKCENTRE;
				}
				data = m_joy2_y;
			}
		}
		break;
#endif

	default:
		data = m_video.reg.data[offset];
		break;
	}
	return data;
}

WRITE8_MEMBER( vc4000_state::vc4000_video_w )
{
//  m_video.reg.data[offset]=data;
	if (offset > 0xcf) offset &= 0xcf;  // c0-cf is mirrored at d0-df, e0-ef, f0-ff

	switch (offset)
	{
	case 0xc0:                      // Sprite size
		m_video.sprites[0].size=1<<(data&3);
		m_video.sprites[1].size=1<<((data>>2)&3);
		m_video.sprites[2].size=1<<((data>>4)&3);
		m_video.sprites[3].size=1<<((data>>6)&3);
		break;

	case 0xc1:                      // Sprite 1+2 color
		m_video.sprites[0].scolor=((~data>>3)&7);
		m_video.sprites[1].scolor=(~data&7);
		break;

	case 0xc2:                      // Sprite 2+3 color
		m_video.sprites[2].scolor=((~data>>3)&7);
		m_video.sprites[3].scolor=(~data&7);
		break;

	case 0xc3:                      // Score control
		m_video.reg.d.score_control = data;
		break;

	case 0xc6:                      // Background color
		m_video.reg.d.background = data;
		break;

	case 0xc7:                      // Soundregister
		m_video.reg.data[offset] = data;
		m_custom->soundport_w(0, data);
		break;

	case 0xc8:                      // Digits 1 and 2
		m_video.reg.d.bcd[0] = data;
		break;

	case 0xc9:                      // Digits 3 and 4
		m_video.reg.d.bcd[1] = data;
		break;

	case 0xca:          // Background-sprite collision (bit 7-4) and sprite finished (bit 3-0)
		m_video.reg.data[offset]=data;
		m_video.background_collision=data;
		break;

	case 0xcb:                  // VRST-Flag (bit 6) and intersprite collision (bit5-0)
		m_video.reg.data[offset]=data;
		m_video.sprite_collision=data;
		break;

	default:
		m_video.reg.data[offset]=data;
	}
}


READ8_MEMBER( vc4000_state::vc4000_vsync_r )
{
	return m_video.line >= VC4000_END_LINE ? 0x80 : 0;
}

static const char led[20][12+1] =
{
	"aaaabbbbcccc",
	"aaaabbbbcccc",
	"aaaabbbbcccc",
	"aaaabbbbcccc",
	"llll    dddd",
	"llll    dddd",
	"llll    dddd",
	"llll    dddd",
	"kkkkmmmmeeee",
	"kkkkmmmmeeee",
	"kkkkmmmmeeee",
	"kkkkmmmmeeee",
	"jjjj    ffff",
	"jjjj    ffff",
	"jjjj    ffff",
	"jjjj    ffff",
	"iiiihhhhgggg",
	"iiiihhhhgggg",
	"iiiihhhhgggg",
	"iiiihhhhgggg"
};


void vc4000_state::vc4000_draw_digit(bitmap_ind16 &bitmap, int x, int y, int d, int line)
{
	static const int digit_to_segment[0x10]={
	0x0fff, 0x007c, 0x17df, 0x15ff, 0x1c7d, 0x1df7, 0x1ff7, 0x007f, 0x1fff, 0x1dff
	};

	int i=line,j;

	for (j=0; j<sizeof(led[0])-1; j++)
	{
		if (led[i][j] != ' ' && digit_to_segment[d]&(1<<(led[i][j]-'a')) )
			bitmap.pix16(y+i, x+j) = ((m_video.reg.d.background>>4)&7)^7;
	}
}

inline void vc4000_state::vc4000_collision_plot(UINT8 *collision, UINT8 data, UINT8 color, int scale)
{
	int i,j,m;

	for (j=0,m=0x80; j<8; j++, m>>=1)
	{
		if (data&m)
			for (i=0; i<scale; i++, collision++) *collision|=color;
		else
			collision+=scale;
	}
}


void vc4000_state::vc4000_sprite_update(bitmap_ind16 &bitmap, UINT8 *collision, SPRITE *This)
{
	int i,j,m;

	if (m_video.line==0)
	{
		This->y=This->data->y1;
		This->state=0;
		This->delay=0;
		This->finished=FALSE;
	}

	This->finished_now=FALSE;

	if (m_video.line>VC4000_END_LINE) return;

	switch (This->state)
	{
	case 0:
		if (m_video.line != This->y + 2) break;
		This->state++;

	case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:case 9:case 10:

		vc4000_collision_plot(collision+This->data->x1, This->data->bitmap[This->state-1],This->mask,This->size);

		for (j=0,m=0x80; j<8; j++, m>>=1)
		{
			if (This->data->bitmap[This->state-1]&m)
			{
				for (i=0; i<This->size; i++)
				{
					m_objects[This->data->x1 + i + j*This->size] |= This->scolor;
					m_objects[This->data->x1 + i + j*This->size] &= 7;
				}
			}
		}

		This->delay++;

		if (This->delay>=This->size)
		{
			This->delay=0;
			This->state++;
		}

		if (This->state>10)
		{
			This->finished=TRUE;
			This->finished_now=TRUE;
		}

		break;

	case 11:
		This->y=This->data->y2;

		if (This->y==255)
			This->y=0;
		else
			This->y++;

		if (This->y>252) break;

		This->delay=0;
		This->state++;

	case 12:
		if (This->y!=0)
		{
			This->y--;
			break;
		}
		This->state++;

	case 13: case 14: case 15: case 16: case 17: case 18: case 19:case 20:case 21:case 22:

		vc4000_collision_plot(collision+This->data->x2,This->data->bitmap[This->state-13],This->mask,This->size);
		for (j=0,m=0x80; j<8; j++, m>>=1)
		{
			if (This->data->bitmap[This->state-13]&m)
			{
				for (i=0; i<This->size; i++)
				{
					int offset = This->data->x2 + i + j*This->size;

					// clip objects outside the object buffer.
					// someone who knows this hardware should look at
					// it properly.
					if (offset < 256)
					{
						m_objects[offset] |= This->scolor;
						m_objects[offset] &= 7;
					}
				}
			}
		}
		This->delay++;
		if (This->delay<This->size) break;
		This->delay=0;
		This->state++;
		if (This->state<23) break;
		This->finished=TRUE;
		This->finished_now=TRUE;
		This->state=11;
		break;
	}
}

inline void vc4000_state::vc4000_draw_grid(UINT8 *collision)
{
	int width = m_screen->width();
	int height = m_screen->height();
	int i, j, m, x, line=m_video.line-20;
	int w, k;

	if (m_video.line>=height) return;

	m_bitmap->plot_box(0, m_video.line, width, 1, (m_video.reg.d.background)&7);

	if (line<0 || line>=200) return;
	if (~m_video.reg.d.background & 8) return;

	i=(line/20)*2;
	if (line%20>=2) i++;

	k=m_video.reg.d.grid_control[i>>2];
	switch (k>>6) {
		default:
		case 0:case 2: w=1;break;
		case 1: w=2;break;
		case 3: w=4;break;
	}
	switch (i&3) {
	case 0:
		if (k&1) w=8;break;
	case 1:
		if ((line%40)<=10) {
			if (k&2) w=8;
		} else {
			if (k&4) w=8;
		}
		break;
	case 2: if (k&8) w=8;break;
	case 3:
		if ((line%40)<=30) {
			if (k&0x10) w=8;
		} else {
			if (k&0x20) w=8;
		}
		break;
	}
	for (x=30, j=0, m=0x80; j<16; j++, x+=8, m>>=1)
	{
		if (m_video.reg.d.grid[i][j>>3]&m)
		{
			int l;
			for (l=0; l<w; l++) collision[x+l]|=0x10;
			m_bitmap->plot_box(x, m_video.line, w, 1, (m_video.reg.d.background>>4)&7);
		}
		if (j==7) m=0x100;
	}
}

INTERRUPT_GEN_MEMBER(vc4000_state::vc4000_video_line)
{
	int x,y,i;
	UINT8 collision[400]={0}; // better alloca or gcc feature of non constant long automatic arrays
	const rectangle &visarea = m_screen->visible_area();
	assert(ARRAY_LENGTH(collision) >= m_screen->width());

	m_video.line++;
	if (m_irq_pause) m_irq_pause++;
	if (m_video.line>311) m_video.line=0;

	if (m_video.line==0)
	{
		m_video.background_collision=0;
		m_video.sprite_collision=0;
		m_video.reg.d.sprite_collision=0;
//      logerror("begin of frame\n");
	}

	if (m_irq_pause>10)
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
		m_irq_pause = 0;
	}

	if (m_video.line <= VC4000_END_LINE)
	{
		vc4000_draw_grid(collision);

		/* init object colours */
		for (i=visarea.min_x; i<visarea.max_x; i++) m_objects[i]=8;

		/* calculate object colours and OR overlapping object colours */
		vc4000_sprite_update(*m_bitmap, collision, &m_video.sprites[0]);
		vc4000_sprite_update(*m_bitmap, collision, &m_video.sprites[1]);
		vc4000_sprite_update(*m_bitmap, collision, &m_video.sprites[2]);
		vc4000_sprite_update(*m_bitmap, collision, &m_video.sprites[3]);

		for (i=visarea.min_x; i<visarea.max_x; i++)
		{
			m_video.sprite_collision|=m_sprite_collision[collision[i]];
			m_video.background_collision|=m_background_collision[collision[i]];
			/* display final object colours */
			if (m_objects[i] < 8)
					m_bitmap->pix16(m_video.line, i) = m_objects[i];
		}

		y = m_video.reg.d.score_control&1?200:20;

		if ((m_video.line>=y)&&(m_video.line<y+20))
		{
			x = 60;
			vc4000_draw_digit(*m_bitmap, x, y, m_video.reg.d.bcd[0]>>4, m_video.line-y);
			vc4000_draw_digit(*m_bitmap, x+16, y, m_video.reg.d.bcd[0]&0xf, m_video.line-y);
			if (m_video.reg.d.score_control&2)  x -= 16;
			vc4000_draw_digit(*m_bitmap, x+48, y, m_video.reg.d.bcd[1]>>4, m_video.line-y);
			vc4000_draw_digit(*m_bitmap, x+64, y, m_video.reg.d.bcd[1]&0xf, m_video.line-y);
		}
	}
	if (m_video.line==VC4000_END_LINE) m_video.reg.d.sprite_collision |=0x40;

	if (((m_video.line == VC4000_END_LINE) |
		(m_video.sprites[3].finished_now) |
		(m_video.sprites[2].finished_now) |
		(m_video.sprites[1].finished_now) |
		(m_video.sprites[0].finished_now)) && (!m_irq_pause))
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 3);
			m_irq_pause=1;
		}
}

UINT32 vc4000_state::screen_update_vc4000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
