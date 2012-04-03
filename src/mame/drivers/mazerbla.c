/****************************************************************************

Mazer Blazer by Stern Electronics (c) 1983
Great Guns by Stern Electronics (c) 1983


Driver by Jarek Burczynski
2003.03.19


Issues:
======
Sprites leave trails in both ganes
Sprites should be transparent (color 0x0f)
Screen flickers heavily in Great Guns (double buffer issue?).


TO DO:
=====
- handle page flipping

- figure out the VCU modes used in "clr_r":
  0x13 -? sprites related
  0x03 -? could be collision detection (if there is such a thing)

- figure out how the palette is handled (partially done)

- find out if there are any CLUTs (might be the other unknown cases in mode 7)

- figure out what really should happen during VCU test in Great Guns (patched
  out at the moment) (btw. Mazer Blazer doesn't test VCU)

- add sound to Mazer Blazer - Speech processor is unknown chip


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"


#define MAZERBLA 0x01
#define GREATGUN 0x02

#define MASTER_CLOCK XTAL_4MHz
#define SOUND_CLOCK XTAL_14_31818MHz


class mazerbla_state : public driver_device
{
public:
	mazerbla_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *   m_cfb_ram;
	UINT8 *   m_videoram;
	size_t    m_videoram_size;

	/* video-related */
	bitmap_ind16 m_tmpbitmaps[4];

	UINT8 m_vcu_video_reg[4];
	UINT32 m_vcu_gfx_addr;
	UINT32 m_vcu_gfx_param_addr;

	UINT8 m_bknd_col;
	UINT8 m_port02_status;
	UINT8 m_vbank;		/* video page select signal, likely for double buffering ?*/
	UINT32 m_xpos;
	UINT32 m_ypos;
	UINT32 m_pix_xsize;
	UINT32 m_pix_ysize;
	UINT8 m_color1;
	UINT8 m_color2;
	UINT8 m_mode;
	UINT8 m_plane;
	UINT8 m_lookup_ram[0x100*4];
	UINT32 m_gfx_rom_bank;	/* graphics ROMs are banked */

	double m_weights_r[2];
	double m_weights_g[3];
	double m_weights_b[3];

	/* misc */
	UINT8 m_game_id; /* hacks per game */
	UINT8 m_ls670_0[4];
	UINT8 m_ls670_1[4];

	UINT8 m_zpu_int_vector;

	UINT8 m_bcd_7445;

	UINT8 m_vsb_ls273;
	UINT8 m_soundlatch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;

#if 0
	int m_dbg_info;
	int m_dbg_gfx_e;
	int m_dbg_clr_e;
	int m_dbg_vbank;
	int m_dbg_lookup;

	int m_planes_enabled[4];
#endif
	DECLARE_WRITE8_MEMBER(cfb_backgnd_color_w);
	DECLARE_WRITE8_MEMBER(cfb_vbank_w);
	DECLARE_WRITE8_MEMBER(cfb_rom_bank_sel_w);
	DECLARE_WRITE8_MEMBER(cfb_rom_bank_sel_w_gg);
	DECLARE_READ8_MEMBER(cfb_port_02_r);
	DECLARE_WRITE8_MEMBER(vcu_video_reg_w);
	DECLARE_READ8_MEMBER(vcu_set_cmd_param_r);
	DECLARE_READ8_MEMBER(vcu_set_gfx_addr_r);
	DECLARE_READ8_MEMBER(vcu_set_clr_addr_r);
	DECLARE_WRITE8_MEMBER(cfb_zpu_int_req_set_w);
	DECLARE_READ8_MEMBER(cfb_zpu_int_req_clr);
	DECLARE_READ8_MEMBER(ls670_0_r);
	DECLARE_WRITE8_MEMBER(ls670_0_w);
	DECLARE_READ8_MEMBER(ls670_1_r);
	DECLARE_WRITE8_MEMBER(ls670_1_w);
	DECLARE_WRITE8_MEMBER(zpu_bcd_decoder_w);
	DECLARE_READ8_MEMBER(zpu_inputs_r);
	DECLARE_WRITE8_MEMBER(zpu_led_w);
	DECLARE_WRITE8_MEMBER(zpu_lamps_w);
	DECLARE_WRITE8_MEMBER(zpu_coin_counter_w);
	DECLARE_WRITE8_MEMBER(cfb_led_w);
	DECLARE_WRITE8_MEMBER(vsb_ls273_audio_control_w);
	DECLARE_WRITE8_MEMBER(main_sound_w);
	DECLARE_WRITE8_MEMBER(sound_int_clear_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_clear_w);
};



/***************************************************************************

  Convert the color PROMs into a more useable format.


  bit 0 -- 10.0Kohm resistor--\
  bit 1 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- BLUE
  bit 2 -- 2.2 Kohm resistor --/

  bit 3 -- 10.0Kohm resistor--\
  bit 4 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- GREEN
  bit 5 -- 2.2 Kohm resistor --/

  bit 6 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- RED
  bit 7 -- 2.2 Kohm resistor --/

***************************************************************************/

static PALETTE_INIT( mazerbla )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	static const int resistances_r[2]  = { 4700, 2200 };
	static const int resistances_gb[3] = { 10000, 4700, 2200 };

	/* just to calculate coefficients for later use */
	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_gb,	state->m_weights_g,	3600,	0,
			3,	resistances_gb,	state->m_weights_b,	3600,	0,
			2,	resistances_r,	state->m_weights_r,	3600,	0);

}

static VIDEO_START( mazerbla )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();

#if 0
	state->m_planes_enabled[0] = state->m_planes_enabled[1] = state->m_planes_enabled[2] = state->m_planes_enabled[3] = 1;
	state->m_dbg_info = 1;
	state->m_dbg_gfx_e = 1;
	state->m_dbg_clr_e = 0;
	state->m_dbg_vbank = 1;
	state->m_dbg_lookup = 4;
#endif

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmaps[0]);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmaps[1]);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmaps[2]);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmaps[3]);

	state->save_item(NAME(state->m_tmpbitmaps[0]));
	state->save_item(NAME(state->m_tmpbitmaps[1]));
	state->save_item(NAME(state->m_tmpbitmaps[2]));
	state->save_item(NAME(state->m_tmpbitmaps[3]));
}

#ifdef UNUSED_DEFINITION

SCREEN_UPDATE_IND16( test_vcu )
{
	mazerbla_state *state = screen.machine().driver_data<mazerbla_state>();
	int *planes_enabled = state->m_planes_enabled;
	char buf[128];

	UINT32 color_base = 0;

	if (state->m_game_id == MAZERBLA)
		color_base = 0x80;	/* 0x80 constant: matches Mazer Blazer movie */

	if (state->m_game_id == GREATGUN)
		color_base = 0x00;

	bitmap.fill(0);
//  logerror("-->frame\n");

	if (planes_enabled[3])
		copybitmap(bitmap, state->m_tmpbitmaps[3], 0, 0, 0, 0, cliprect);

	if (planes_enabled[2])
		copybitmap_trans(bitmap, state->m_tmpbitmaps[2], 0, 0, 0, 0,cliprect, color_base);

	state->m_tmpbitmaps[2].fill(color_base);

	if (planes_enabled[1])
		copybitmap_trans(bitmap, state->m_tmpbitmaps[1], 0, 0, 0, 0,cliprect, color_base);

	state->m_tmpbitmaps[1].fill(color_base);

	if (planes_enabled[0])
		copybitmap_trans(bitmap, state->m_tmpbitmaps[0], 0, 0, 0, 0,cliprect, color_base);

	state->m_tmpbitmaps[0].fill(color_base);

	if (screen.machine().input().code_pressed_once(KEYCODE_1))	/* plane 1 */
		planes_enabled[0] ^= 1;

	if (screen.machine().input().code_pressed_once(KEYCODE_2))	/* plane 2 */
		planes_enabled[1] ^= 1;

	if (screen.machine().input().code_pressed_once(KEYCODE_3))	/* plane 3 */
		planes_enabled[2] ^= 1;

	if (screen.machine().input().code_pressed_once(KEYCODE_4))	/* plane 4 */
		planes_enabled[3] ^= 1;

	if (screen.machine().input().code_pressed_once(KEYCODE_I))	/* show/hide debug info */
		state->m_dbg_info = !state->m_dbg_info;

	if (screen.machine().input().code_pressed_once(KEYCODE_G))	/* enable gfx area handling */
		state->m_dbg_gfx_e = !state->m_dbg_gfx_e;

	if (screen.machine().input().code_pressed_once(KEYCODE_C))	/* enable color area handling */
		state->m_dbg_clr_e = !state->m_dbg_clr_e;

	if (screen.machine().input().code_pressed_once(KEYCODE_V))	/* draw only when vbank==dbg_vbank */
		state->m_dbg_vbank ^= 1;

	if (screen.machine().input().code_pressed_once(KEYCODE_L))	/* showlookup ram */
		state->m_dbg_lookup = (state->m_dbg_lookup + 1) % 5;	//0,1,2,3, 4-off


	if (state->m_dbg_info)
	{
		sprintf(buf,"I-info, G-gfx, C-color, V-vbank, 1-4 enable planes");
		ui_draw_text(buf, 10, 0 * ui_get_line_height(screen.machine()));

		sprintf(buf,"g:%1i c:%1i v:%1i vbk=%1i  planes=%1i%1i%1i%1i  ", state->m_dbg_gfx_e&1, state->m_dbg_clr_e&1, state->m_dbg_vbank, vbank&1,
			planes_enabled[0],
			planes_enabled[1],
			planes_enabled[2],
			planes_enabled[3] );

		ui_draw_text(buf, 10, 1 * ui_get_line_height(screen.machine()));

		if (state->m_dbg_lookup!=4)
		{
			int lookup_offs = (state->m_dbg_lookup)*256; //=0,1,2,3*256
			int y, x;

			for (y = 0; y < 16; y++)
			{
				memset(buf, 0, 128);
				sprintf(buf + strlen(buf), "%04x ", lookup_offs + y * 16);
				for (x = 0; x < 16; x++)
				{
					sprintf(buf + strlen(buf), "%02x ", lookup_ram[lookup_offs + x + y * 16]);
				}
				ui_draw_text(buf, 0, (2 + y) * ui_get_line_height(screen.machine()));
			}
		}
	}

	return 0;
}
#endif


static SCREEN_UPDATE_IND16( mazerbla )
{
	mazerbla_state *state = screen.machine().driver_data<mazerbla_state>();
//  UINT32 color_base = 0;

	if (state->m_game_id == MAZERBLA)
//      color_base = 0x80;  /* 0x80 constant: matches Mazer Blazer movie */

//  if (state->m_game_id == GREATGUN)
//      color_base = 0x00;

	//  bitmap.fill(0);

	copybitmap(bitmap, state->m_tmpbitmaps[3], 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, state->m_tmpbitmaps[2], 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, state->m_tmpbitmaps[1], 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, state->m_tmpbitmaps[0], 0, 0, 0, 0, cliprect, 0);
	return 0;
}


WRITE8_MEMBER(mazerbla_state::cfb_backgnd_color_w)
{

	if (m_bknd_col != data)
	{
		int r, g, b, bit0, bit1, bit2;

		m_bknd_col = data;

		/* red component */
		bit1 = BIT(data, 7);
		bit0 = BIT(data, 6);
		r = combine_2_weights(m_weights_r, bit0, bit1);

		/* green component */
		bit2 = BIT(data, 5);
		bit1 = BIT(data, 4);
		bit0 = BIT(data, 3);
		g = combine_3_weights(m_weights_g, bit0, bit1, bit2);

		/* blue component */
		bit2 = BIT(data, 2);
		bit1 = BIT(data, 1);
		bit0 = BIT(data, 0);
		b = combine_3_weights(m_weights_b, bit0, bit1, bit2);

		palette_set_color(machine(), 255, MAKE_RGB(r, g, b));
		//logerror("background color (port 01) write=%02x\n",data);
	}
}


WRITE8_MEMBER(mazerbla_state::cfb_vbank_w)
{

	/* only bit 6 connected */
	m_vbank = BIT(data, 6);
}


WRITE8_MEMBER(mazerbla_state::cfb_rom_bank_sel_w)/* mazer blazer */
{
	m_gfx_rom_bank = data;

	memory_set_bankptr(machine(),  "bank1", machine().region("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000);
}

WRITE8_MEMBER(mazerbla_state::cfb_rom_bank_sel_w_gg)/* great guns */
{
	m_gfx_rom_bank = data >> 1;

	memory_set_bankptr(machine(),  "bank1", machine().region("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000);
}


/* VCU status? */
READ8_MEMBER(mazerbla_state::cfb_port_02_r)
{
	m_port02_status ^= 0xff;
	return m_port02_status;
}


WRITE8_MEMBER(mazerbla_state::vcu_video_reg_w)
{
	if (m_vcu_video_reg[offset] != data)
	{
		m_vcu_video_reg[offset] = data;
		//popmessage("video_reg= %02x %02x %02x %02x", m_vcu_video_reg[0], m_vcu_video_reg[1], m_vcu_video_reg[2], m_vcu_video_reg[3]);
		//logerror("video_reg= %02x %02x %02x %02x\n", m_vcu_video_reg[0], m_vcu_video_reg[1], m_vcu_video_reg[2], m_vcu_video_reg[3]);
	}
}

READ8_MEMBER(mazerbla_state::vcu_set_cmd_param_r)
{
	m_vcu_gfx_param_addr = offset;

	/* offset  = 0 is not known */
	m_xpos      = m_cfb_ram[m_vcu_gfx_param_addr + 1] | (m_cfb_ram[m_vcu_gfx_param_addr + 2]<<8);
	m_ypos      = m_cfb_ram[m_vcu_gfx_param_addr + 3] | (m_cfb_ram[m_vcu_gfx_param_addr + 4]<<8);
	m_color1    = m_cfb_ram[m_vcu_gfx_param_addr + 5];
	m_color2    = m_cfb_ram[m_vcu_gfx_param_addr + 6];
	m_mode      = m_cfb_ram[m_vcu_gfx_param_addr + 7];
	m_pix_xsize = m_cfb_ram[m_vcu_gfx_param_addr + 8];
	m_pix_ysize = m_cfb_ram[m_vcu_gfx_param_addr + 9];

	m_plane = m_mode & 3;

	return 0;
}


READ8_MEMBER(mazerbla_state::vcu_set_gfx_addr_r)
{
	UINT8 * rom = machine().region("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000;
	int offs;
	int x, y;
	int bits = 0;
	UINT8 color_base = 0;

	if (m_game_id == MAZERBLA)
		color_base = 0x80;	/* 0x80 - good for Mazer Blazer: (only in game, CRT test mode is bad) */

	if (m_game_id == GREATGUN)
		color_base = 0x00;	/* 0x00 - good for Great Guns: (both in game and CRT test mode) */
/*
    if ((mode <= 0x07) || (mode >= 0x10))
    {
        logerror("paradr=");
        logerror("%3x ", m_vcu_gfx_param_addr );

        logerror("%02x ", m_cfb_ram[vcu_gfx_param_addr + 0] );
        logerror("x=%04x ", m_xpos );                 //1,2
        logerror("y=%04x ", m_ypos );                 //3,4
        logerror("color1=%02x ", m_color1);             //5
        logerror("color2=%02x ", m_color2);           //6
        logerror("mode=%02x ", m_mode );              //7
        logerror("xpix=%02x ", m_pix_xsize );         //8
        logerror("ypix=%02x ", m_pix_ysize );         //9

        logerror("addr=%4i bank=%1i\n", offset, m_gfx_rom_bank);
    }
*/

	m_vcu_gfx_addr = offset;

	/* draw */
	offs = m_vcu_gfx_addr;

	switch(m_mode)
	{
		/* 2 bits per pixel */
		case 0x0f:
		case 0x0e:
		case 0x0d:
		case 0x0c:
//      if (m_dbg_gfx_e)
//      {
//          if (m_vbank == m_dbg_vbank)
		{
			for (y = 0; y <= m_pix_ysize; y++)
			{
				for (x = 0; x <= m_pix_xsize; x++)
				{
					UINT8 pixeldata = rom[(offs + (bits >> 3)) % 0x2000];
					UINT8 data = (pixeldata >> (6 - (bits & 7))) & 3;
					UINT8 col = 0;

					switch(data)
					{
						case 0:
							col = color_base | ((m_color1 & 0x0f));		//background PEN
							break;
						case 1:
							col = color_base | ((m_color1 & 0xf0) >> 4);	//foreground PEN
							break;
						case 2:
							col = color_base | ((m_color2 & 0x0f));	//background PEN2
							break;
						case 3:
							col = color_base | ((m_color2 & 0xf0) >> 4);	//foreground PEN2
							break;
					}

					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256) )
						m_tmpbitmaps[m_plane].pix16(m_ypos + y, m_xpos + x) = col;

					bits += 2;
				}
			}
		}
//      }
		break;

		/* 1 bit per pixel */
		case 0x0b:/* verified - 1bpp ; used for 'cleaning' using color 0xff */
		case 0x0a:/* verified - 1bpp */
		case 0x09:/* verified - 1bpp: gun crosshair */
		case 0x08:/* */
//      if (m_dbg_gfx_e)
//      {
//          if (m_vbank == m_dbg_vbank)
		{

			for (y = 0; y <= m_pix_ysize; y++)
			{
				for (x = 0; x <= m_pix_xsize; x++)
				{
					UINT8 pixeldata = rom[(offs + (bits >> 3)) % 0x2000];
					UINT8 data = (pixeldata >> (7 - (bits & 7))) & 1;

					/* color = 4 MSB = front PEN, 4 LSB = background PEN */

					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256))
						m_tmpbitmaps[m_plane].pix16(m_ypos + y, m_xpos + x) = data ? color_base | ((m_color1 & 0xf0) >> 4): color_base | ((m_color1 & 0x0f));

					bits += 1;
				}
			}
		}
//      }
		break;

		/* 4 bits per pixel */
		case 0x03:
		case 0x01:
		case 0x00:
//      if (m_dbg_gfx_e)
//      {
//          if (m_vbank == m_dbg_vbank)
		{
			for (y = 0; y <= m_pix_ysize; y++)
			{
				for (x = 0; x <= m_pix_xsize; x++)
				{
					UINT8 pixeldata = rom[(offs + (bits >> 3)) % 0x2000];
					UINT8 data = (pixeldata >> (4 - (bits & 7))) & 15;
					UINT8 col = 0;

					col = color_base | data;

					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256))
						m_tmpbitmaps[m_plane].pix16(m_ypos + y, m_xpos + x) = col;

					bits += 4;
				}
			}
		}
//      }
		break;
	default:
		popmessage("not supported VCU drawing mode=%2x", m_mode);
		break;
	}

	return 0;
}

READ8_MEMBER(mazerbla_state::vcu_set_clr_addr_r)
{
	UINT8 * rom = machine().region("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000;
	int offs;
	int x, y;
	int bits = 0;

	UINT8 color_base = 0;


/*
    //if (0) //(mode != 0x07)
    {
        logerror("paladr=");
        logerror("%3x ", m_vcu_gfx_param_addr );

        logerror("%02x ", m_cfb_ram[m_vcu_gfx_param_addr + 0] );
        logerror("x=%04x ", m_xpos );                 //1,2
        logerror("y=%04x ", m_ypos );                 //3,4
        logerror("color1=%02x ", m_color1);             //5
        logerror("color2=%02x ", m_color2 );          //6
        logerror("mode=%02x ", m_mode );              //7
        logerror("xpix=%02x ", m_pix_xsize );         //8
        logerror("ypix=%02x ", m_pix_ysize );         //9

        logerror("addr=%4i bank=%1i\n", offset, m_gfx_rom_bank);

        for (y = 0; y < 16; y++)
        {
            logerror("%04x: ", offset + y * 16);
            for (x = 0; x < 16; x++)
            {
                logerror("%02x ", m_cfb_ram[offset + x + y * 16]);
            }
            logerror("\n");
        }
    }

*/

/* copy palette / CLUT(???) */


	switch (m_mode)
	{
		case 0x13: /* draws sprites?? in mazer blazer and ... wrong sprite in place of targeting-cross and UFO laser */
		case 0x03:
		/* ... this may proove that there is really only one area and that
           the draw command/palette selector is done via the 'mode' only ... */
		//if (m_dbg_clr_e)
		{
			offs = m_vcu_gfx_addr;

			if (m_game_id == MAZERBLA)
				color_base = 0x80;	/* 0x80 constant: matches Mazer Blazer movie */

			if (m_game_id == GREATGUN)
				color_base = 0x00;

			for (y = 0; y <= m_pix_ysize; y++)
			{
				for (x = 0; x <= m_pix_xsize; x++)
				{
					UINT8 pixeldata = rom[(offs + (bits >> 3)) % 0x2000];
					UINT8 data = (pixeldata >> (6 - (bits & 7))) & 3;
					UINT8 col = 0;

					switch(data)
					{
						case 0:
							col = color_base | ((m_color1 & 0x0f));		//background PEN
							break;
						case 1:
							col = color_base | ((m_color1 & 0xf0) >> 4);	//foreground PEN
							break;
						case 2:
							col = color_base | ((m_color2 & 0x0f));	//background PEN2
							break;
						case 3:
							col = color_base | ((m_color2 & 0xf0) >> 4);	//foreground PEN2
							break;
					}

					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256))
						m_tmpbitmaps[m_plane].pix16(m_ypos + y, m_xpos + x) = col;

						bits += 2;
				}
			}
		}
		break;

		/* Palette / "something else" write mode */
		case 0x07:
		offs = offset;

		switch(m_ypos)
		{
			case 6: //seems to encode palette write
			{
				int r, g, b, bit0, bit1, bit2;

				//pix_xsize and pix_ysize seem to be related to palette length ? (divide by 2)
				int lookup_offs = (m_ypos >> 1) * 256; //=3*256

				for (y = 0; y < 16; y++)
				{
					for (x = 0; x < 16; x++)
					{
						UINT8 colour = m_cfb_ram[offs + x + y * 16];

						/* red component */
						bit1 = (colour >> 7) & 0x01;
						bit0 = (colour >> 6) & 0x01;
						r = combine_2_weights(m_weights_r, bit0, bit1);

						/* green component */
						bit2 = (colour >> 5) & 0x01;
						bit1 = (colour >> 4) & 0x01;
						bit0 = (colour >> 3) & 0x01;
						g = combine_3_weights(m_weights_g, bit0, bit1, bit2);

						/* blue component */
						bit2 = (colour >> 2) & 0x01;
						bit1 = (colour >> 1) & 0x01;
						bit0 = (colour >> 0) & 0x01;
						b = combine_3_weights(m_weights_b, bit0, bit1, bit2);

						if ((x + y * 16) < 255)//keep color 255 free for use as background color
							palette_set_color(machine(), x + y * 16, MAKE_RGB(r, g, b));

						m_lookup_ram[lookup_offs + x + y * 16] = colour;
					}
				}
			}
			break;
			case 4: //seems to encode lookup???? table write
			{
				int lookup_offs = (m_ypos >> 1) * 256; //=2*256

				for (y = 0; y < 16; y++)
				{
					for (x = 0; x < 16; x++)
					{
						UINT8 dat = m_cfb_ram[offs + x + y * 16];
						m_lookup_ram[lookup_offs + x + y * 16] = dat;
					}
				}
			}
			break;
			case 2: //seems to encode lookup???? table write
			{
				int lookup_offs = (m_ypos >> 1) * 256; //=1*256

				for (y = 0; y < 16; y++)
				{
					for (x = 0; x < 16; x++)
					{
						UINT8 dat = m_cfb_ram[offs + x + y * 16];
						m_lookup_ram[lookup_offs + x + y * 16] = dat;
					}
				}
			}
			break;
			case 0: //seems to encode lookup???? table write
			{
				int lookup_offs = (m_ypos >> 1) * 256; //=0*256

				for (y = 0; y < 16; y++)
				{
					for (x = 0; x < 16; x++)
					{
						UINT8 dat = m_cfb_ram[offs + x + y * 16];
						m_lookup_ram[lookup_offs + x + y * 16] = dat;
					}
				}
			}
			break;

			default:
			popmessage("not supported lookup/color write mode=%2x", m_ypos);
			break;
		}
		break;

	default:
		popmessage("not supported VCU color mode=%2x", m_mode);
		break;
	}

	return 0;
}

/*************************************
 *
 *  IRQ & Timer handlers
 *
 *************************************/

WRITE8_MEMBER(mazerbla_state::cfb_zpu_int_req_set_w)
{

	m_zpu_int_vector &= ~2;	/* clear D1 on INTA (interrupt acknowledge) */

	device_set_input_line(m_maincpu, 0, ASSERT_LINE);	/* main cpu interrupt (comes from CFB (generated at the start of INT routine on CFB) - vblank?) */
}

READ8_MEMBER(mazerbla_state::cfb_zpu_int_req_clr)
{

	m_zpu_int_vector |= 2;

	/* clear the INT line when there are no more interrupt requests */
	if (m_zpu_int_vector == 0xff)
		device_set_input_line(m_maincpu, 0, CLEAR_LINE);

	return 0;
}

READ8_MEMBER(mazerbla_state::ls670_0_r)
{

	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_0[offset];
}

static TIMER_CALLBACK( deferred_ls670_0_w )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	int offset = (param >> 8) & 255;
	int data = param & 255;

	state->m_ls670_0[offset] = data;
}

WRITE8_MEMBER(mazerbla_state::ls670_0_w)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(FUNC(deferred_ls670_0_w), (offset << 8) | data);
}

READ8_MEMBER(mazerbla_state::ls670_1_r)
{

	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_1[offset];
}

static TIMER_CALLBACK( deferred_ls670_1_w )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	int offset = (param >> 8) & 255;
	int data = param & 255;

	state->m_ls670_1[offset] = data;
}

WRITE8_MEMBER(mazerbla_state::ls670_1_w)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(FUNC(deferred_ls670_1_w), (offset << 8) | data);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

/*
name:           Strobe(bcd_value)   BIT
---------------------------------------
ZPU switch 1    0                   6
ZPU switch 2    0                   7

dipsw 35        1                   7
dipsw 34        1                   6
dipsw 33        1                   5
dipsw 32        1                   4
dipsw 31        1                   3
dipsw 30        1                   2
dipsw 29        1                   1
dipsw 28        1                   0
dipsw 27        2                   7
dipsw 26        2                   6
...
dipsw 8         4                   4
dipsw 7         4                   3
dipsw 6         4                   2
dipsw 5         4                   1
dipsw 4         4                   0

Right Coin Sw.  5                   0
Left Coin Sw.   5                   1
Player One      5                   2
Player Two      5                   3
Fire Button     5                   4

Horizontal movement of gun is Strobe 6, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

Vertical movement of gun is Strobe 7, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111


Great Guns has two guns and here is necessary support for second gun:

Horizontal movement of gun is Strobe 8, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

Vertical movement of gun is Strobe 9, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

*/

WRITE8_MEMBER(mazerbla_state::zpu_bcd_decoder_w)
{

	/* bcd decoder used a input select (a mux) for reads from port 0x62 */
	m_bcd_7445 = data & 0xf;
}

READ8_MEMBER(mazerbla_state::zpu_inputs_r)
{
	static const char *const strobenames[] = { "ZPU", "DSW0", "DSW1", "DSW2", "DSW3", "BUTTONS", "STICK0_X", "STICK0_Y",
	                                           "STICK1_X", "STICK1_Y", "UNUSED", "UNUSED", "UNUSED", "UNUSED", "UNUSED", "UNUSED" };

	UINT8 ret = 0;

	ret = input_port_read(machine(), strobenames[m_bcd_7445]);

	return ret;
}

WRITE8_MEMBER(mazerbla_state::zpu_led_w)
{
	/* 0x6e - reset (offset = 0)*/
	/* 0x6f - set */
	set_led_status(machine(), 0, offset & 1);
}

WRITE8_MEMBER(mazerbla_state::zpu_lamps_w)
{
	/* bit 4 = /LAMP0 */
	/* bit 5 = /LAMP1 */

	/*set_led_status(machine(), 0, (data & 0x10) >> 4);*/
	/*set_led_status(machine(), 1, (data & 0x20) >> 4);*/
}

WRITE8_MEMBER(mazerbla_state::zpu_coin_counter_w)
{
	/* bit 6 = coin counter */
	coin_counter_w(machine(), offset, BIT(data, 6));
}

WRITE8_MEMBER(mazerbla_state::cfb_led_w)
{
	/* bit 7 - led on */
	set_led_status(machine(), 2, BIT(data, 7));
}

static WRITE8_DEVICE_HANDLER( gg_led_ctrl_w )
{
	/* bit 0, bit 1 - led on */
	set_led_status(device->machine(), 1, BIT(data, 0));
}


/*************************************
 *
 *  Sound comms
 *
 *************************************/

WRITE8_MEMBER(mazerbla_state::vsb_ls273_audio_control_w)
{
	m_vsb_ls273 = data;

	/* bit 5 - led on */
	set_led_status(machine(), 1, BIT(data, 5));
}

static READ8_DEVICE_HANDLER( soundcommand_r )
{
	mazerbla_state *state = device->machine().driver_data<mazerbla_state>();
	return state->m_soundlatch;
}

static TIMER_CALLBACK( delayed_sound_w )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	state->m_soundlatch = param;

	/* cause NMI on sound CPU */
	device_set_input_line(state->m_subcpu, INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(mazerbla_state::main_sound_w)
{
	machine().scheduler().synchronize(FUNC(delayed_sound_w), data & 0xff);
}

WRITE8_MEMBER(mazerbla_state::sound_int_clear_w)
{
	device_set_input_line(m_subcpu, 0, CLEAR_LINE);
}

WRITE8_MEMBER(mazerbla_state::sound_nmi_clear_w)
{
	device_set_input_line(m_subcpu, INPUT_LINE_NMI, CLEAR_LINE);
}


/*************************************
 *
 *  Memory Maps (Mazer Blazer)
 *
 *************************************/

static ADDRESS_MAP_START( mazerbla_map, AS_PROGRAM, 8, mazerbla_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xd800, 0xd800) AM_READ(cfb_zpu_int_req_clr)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_BASE_SIZE(m_videoram, m_videoram_size)
	AM_RANGE(0xe800, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x4c, 0x4f) AM_READWRITE(ls670_1_r, ls670_0_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(zpu_bcd_decoder_w)
	AM_RANGE(0x62, 0x62) AM_READ(zpu_inputs_r)
	AM_RANGE(0x68, 0x68) AM_WRITE(zpu_coin_counter_w)
	AM_RANGE(0x6a, 0x6a) AM_WRITE(zpu_lamps_w)
	AM_RANGE(0x6e, 0x6f) AM_WRITE(zpu_led_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_cpu2_map, AS_PROGRAM, 8, mazerbla_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM /* main RAM (stack) */
	AM_RANGE(0x8000, 0x83ff) AM_RAM /* waveform ???*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_cpu2_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(vsb_ls273_audio_control_w)
	AM_RANGE(0x80, 0x83) AM_READWRITE(ls670_0_r, ls670_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_cpu3_map, AS_PROGRAM, 8, mazerbla_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")					/* GFX roms */
	AM_RANGE(0x4000, 0x4003) AM_WRITE(vcu_video_reg_w)
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_BASE(m_cfb_ram)		/* Color Frame Buffer PCB, a.k.a. RAM for VCU commands and parameters */
	AM_RANGE(0xa000, 0xa7ff) AM_READ(vcu_set_cmd_param_r)	/* VCU command and parameters LOAD */
	AM_RANGE(0xc000, 0xdfff) AM_READ(vcu_set_gfx_addr_r)	/* gfx LOAD (blit) */
	AM_RANGE(0xe000, 0xffff) AM_READ(vcu_set_clr_addr_r)	/* palette? LOAD */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_cpu3_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_WRITE(cfb_backgnd_color_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(cfb_port_02_r, cfb_led_w)	/* Read = VCU status ? */
	AM_RANGE(0x03, 0x03) AM_WRITE(cfb_zpu_int_req_set_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(cfb_rom_bank_sel_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cfb_vbank_w)	//visible/writable videopage select?
ADDRESS_MAP_END


/*************************************
 *
 *  Memory Maps (Great Guns)
 *
 *************************************/

static ADDRESS_MAP_START( greatgun_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x4c, 0x4c) AM_WRITE(main_sound_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(zpu_bcd_decoder_w)
	AM_RANGE(0x62, 0x62) AM_READ(zpu_inputs_r)
	AM_RANGE(0x66, 0x66) AM_WRITENOP
	AM_RANGE(0x68, 0x68) AM_WRITENOP
	AM_RANGE(0x6e, 0x6f) AM_WRITE(zpu_led_w)
ADDRESS_MAP_END

/* Great Guns has a little different banking layout */
static ADDRESS_MAP_START( greatgun_cpu3_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP
	AM_RANGE(0x01, 0x01) AM_WRITE(cfb_backgnd_color_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(cfb_port_02_r, cfb_led_w)	/* Read = VCU status ? */
	AM_RANGE(0x03, 0x03) AM_WRITE(cfb_zpu_int_req_set_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(cfb_rom_bank_sel_w_gg)
	AM_RANGE(0x05, 0x05) AM_WRITE(cfb_vbank_w)	//visible/writable videopage select?
ADDRESS_MAP_END

static ADDRESS_MAP_START( greatgun_sound_map, AS_PROGRAM, 8, mazerbla_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_DEVREAD_LEGACY("ay1", ay8910_r)
	AM_RANGE(0x4000, 0x4001) AM_DEVWRITE_LEGACY("ay1", ay8910_address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE_LEGACY("ay2", ay8910_address_data_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(sound_int_clear_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(sound_nmi_clear_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( mazerbla )
	PORT_START("ZPU")	/* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")	/* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x03, "6" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Freeze Time" )
	PORT_DIPSETTING(	0x0c, "1.5 seconds" )
	PORT_DIPSETTING(	0x08, "2.0 seconds" )
	PORT_DIPSETTING(	0x04, "2.5 seconds" )
	PORT_DIPSETTING(	0x00, "3.0 seconds" )
	PORT_DIPNAME( 0x30, 0x00, "Number of points for extra frezze & first life" )
	PORT_DIPSETTING(	0x30, "20000" )
	PORT_DIPSETTING(	0x20, "25000" )
	PORT_DIPSETTING(	0x10, "30000" )
	PORT_DIPSETTING(	0x00, "35000" )
	PORT_DIPNAME( 0xc0, 0x00, "Number of points for extra life other than first" )
	PORT_DIPSETTING(	0xc0, "40000" )
	PORT_DIPSETTING(	0x80, "50000" )
	PORT_DIPSETTING(	0x40, "60000" )
	PORT_DIPSETTING(	0x00, "70000" )

	PORT_START("DSW1")	/* Strobe 2: Dip Switches 20-27*/
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/14 Credits" )

	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x70, "1 Coin/14 Credits" )

	PORT_START("DSW2")	/* Strobe 3: Dip Switches 12-19*/
	PORT_DIPNAME( 0x01, 0x01, "Service Index" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Switch Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Player Immortality" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Super Shot" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")	/* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x03, 0x02, "Number of Freezes" )
	PORT_DIPSETTING(	0x03, "4" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "2" )
	PORT_DIPSETTING(	0x00, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Gun Knocker" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//dips 7-11 - not listed in manual
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTONS")	/* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STICK0_X")	/* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(1)
	PORT_START("STICK0_Y")	/* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	/* Mazer Blazer cabinet has only one gun, really */
	PORT_START("STICK1_X")	/* Strobe 8: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(2)
	PORT_START("STICK1_Y")	/* Strobe 9: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( greatgun )
	PORT_START("ZPU")	/* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")	/* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x00, "Starting Number of Bullets/Credit" )
	PORT_DIPSETTING(	0x03, "60" )
	PORT_DIPSETTING(	0x02, "70" )
	PORT_DIPSETTING(	0x01, "80" )
	PORT_DIPSETTING(	0x00, "90" )
	PORT_DIPNAME( 0x0c, 0x00, "Target Size" )
	PORT_DIPSETTING(	0x0c, "7 x 7" )
	PORT_DIPSETTING(	0x08, "9 x 9" )
	PORT_DIPSETTING(	0x04, "11x11" )
	PORT_DIPSETTING(	0x00, "7 x 7" )
	PORT_DIPNAME( 0x70, 0x00, "Number of points for extra bullet" )
	PORT_DIPSETTING(	0x70, "1000" )
	PORT_DIPSETTING(	0x60, "2000" )
	PORT_DIPSETTING(	0x50, "3000" )
	PORT_DIPSETTING(	0x40, "4000" )
	PORT_DIPSETTING(	0x30, "5000" )
	PORT_DIPSETTING(	0x20, "6000" )
	PORT_DIPSETTING(	0x10, "7000" )
	PORT_DIPSETTING(	0x00, "8000" )
	/* from manual:
        "This switch is used when an optional coin return or ticket dispenser is used"
    */
	PORT_DIPNAME( 0x80, 0x00, "Number of coins or tickets returned" )
	PORT_DIPSETTING(	0x80, "1" )
	PORT_DIPSETTING(	0x00, "2" )

	PORT_START("DSW1")	/* Strobe 2: Dip Switches 20-27*/
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/14 Credits" )

	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x70, "1 Coin/14 Credits" )

	PORT_START("DSW2")	/* Strobe 3: Dip Switches 12-19*/
	PORT_DIPNAME( 0x01, 0x01, "Service Index" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Switch Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Player Immortality" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Rack Advance" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	//probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")	/* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x01, 0x01, "Free game/coin return" )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	//dips 5-11 - not listed in manual
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTONS")	/* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("STICK0_X")	/* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)
	PORT_START("STICK0_Y")	/* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	PORT_START("STICK1_X")	/* Strobe 8: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)
	PORT_START("STICK1_Y")	/* Strobe 9: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*************************************
 *
 *  Sound HW Configs
 *
 *************************************/

/* only Great Guns */
static const ay8910_interface ay8912_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_HANDLER(soundcommand_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8912_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(gg_led_ctrl_w)
};

static IRQ_CALLBACK(irq_callback)
{
	/* all data lines are tied to +5V via 10K resistors */
	/* D1 is set to GND when INT comes from CFB */
	/* D2 is set to GND when INT comes from ZPU board - from 6850 on schematics (RS232 controller) */

	/* resulting vectors:
    1111 11000 (0xf8)
    1111 11010 (0xfa)
    1111 11100 (0xfc)

    note:
    1111 11110 (0xfe) - cannot happen and is not handled by game */

	mazerbla_state *state = device->machine().driver_data<mazerbla_state>();
	return (state->m_zpu_int_vector & ~1);	/* D0->GND is performed on CFB board */
}

/* frequency is 14.318 MHz/16/16/16/16 */
static INTERRUPT_GEN( sound_interrupt )
{
	device_set_input_line(device, 0, ASSERT_LINE);
}


/*************************************
 *
 *  Machine driver definitions
 *
 *************************************/

static MACHINE_START( mazerbla )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_subcpu = machine.device("sub");

	state->save_item(NAME(state->m_vcu_video_reg));
	state->save_item(NAME(state->m_vcu_gfx_addr));
	state->save_item(NAME(state->m_vcu_gfx_param_addr));

	state->save_item(NAME(state->m_bknd_col));
	state->save_item(NAME(state->m_port02_status));
	state->save_item(NAME(state->m_vbank));
	state->save_item(NAME(state->m_xpos));
	state->save_item(NAME(state->m_ypos));
	state->save_item(NAME(state->m_pix_xsize));
	state->save_item(NAME(state->m_pix_ysize));
	state->save_item(NAME(state->m_color1));
	state->save_item(NAME(state->m_color2));
	state->save_item(NAME(state->m_mode));
	state->save_item(NAME(state->m_plane));
	state->save_item(NAME(state->m_lookup_ram));
	state->save_item(NAME(state->m_gfx_rom_bank));

	state->save_item(NAME(state->m_ls670_0));
	state->save_item(NAME(state->m_ls670_1));

	state->save_item(NAME(state->m_zpu_int_vector));

	state->save_item(NAME(state->m_bcd_7445));

	state->save_item(NAME(state->m_vsb_ls273));
	state->save_item(NAME(state->m_soundlatch));
}

static MACHINE_RESET( mazerbla )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	int i;

	state->m_zpu_int_vector = 0xff;

	state->m_bknd_col = 0xaa;
	state->m_gfx_rom_bank = 0xff;

	state->m_vcu_gfx_addr = 0;
	state->m_vcu_gfx_param_addr = 0;
	state->m_port02_status = 0;
	state->m_vbank = 0;
	state->m_xpos = 0;
	state->m_ypos = 0;
	state->m_pix_xsize = 0;
	state->m_pix_ysize = 0;
	state->m_color1 = 0;
	state->m_color2 = 0;
	state->m_mode = 0;
	state->m_plane = 0;
	state->m_bcd_7445 = 0;
	state->m_vsb_ls273 = 0;
	state->m_soundlatch = 0;

	for (i = 0; i < 4; i++)
	{
		state->m_vcu_video_reg[i] = 0;
		state->m_ls670_0[i] = 0;
		state->m_ls670_1[i] = 0;
	}

	memset(state->m_lookup_ram, 0, ARRAY_LENGTH(state->m_lookup_ram));

	device_set_irq_callback(machine.device("maincpu"), irq_callback);
}


static MACHINE_CONFIG_START( mazerbla, mazerbla_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)	/* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	MCFG_CPU_PROGRAM_MAP(mazerbla_map)
	MCFG_CPU_IO_MAP(mazerbla_io_map)

	MCFG_CPU_ADD("sub", Z80, MASTER_CLOCK)	/* 4 MHz, NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu2_map)
	MCFG_CPU_IO_MAP(mazerbla_cpu2_io_map)
//  MCFG_CPU_PERIODIC_INT(irq0_line_hold, 400 ) /* frequency in Hz */

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK)	/* 4 MHz, no  NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu3_map)
	MCFG_CPU_IO_MAP(mazerbla_cpu3_io_map)
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* synchronization forced on the fly */
	MCFG_MACHINE_START(mazerbla)
	MCFG_MACHINE_RESET(mazerbla)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(mazerbla)

	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(mazerbla)
	MCFG_VIDEO_START(mazerbla)

	/* sound hardware */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( greatgun, mazerbla_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)	/* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	MCFG_CPU_PROGRAM_MAP(mazerbla_map)
	MCFG_CPU_IO_MAP(greatgun_io_map)

	MCFG_CPU_ADD("sub", Z80, SOUND_CLOCK / 4)	/* 3.579500 MHz, NMI - caused by sound command write, periodic INT */
	MCFG_CPU_PROGRAM_MAP(greatgun_sound_map)
	MCFG_CPU_PERIODIC_INT(sound_interrupt, (double)14318180/16/16/16/16 )

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK)	/* 4 MHz, no  NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu3_map)
	MCFG_CPU_IO_MAP(greatgun_cpu3_io_map)
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(mazerbla)
	MCFG_MACHINE_RESET(mazerbla)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(mazerbla)

	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(mazerbla)
	MCFG_VIDEO_START(mazerbla)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, SOUND_CLOCK / 8)
	MCFG_SOUND_CONFIG(ay8912_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, SOUND_CLOCK / 8)
	MCFG_SOUND_CONFIG(ay8912_interface_2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

ROM_START( mazerbla )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "mblzpu0.1h",0x0000, 0x2000, CRC(82766187) SHA1(cfc425c87cccb84180f1091998eafeaede126d9d) )
	ROM_LOAD( "mblzpu1.2h",0x2000, 0x2000, CRC(8ba2b3f9) SHA1(1d203332e434d1d9821f98c6ac959ae65dcc51ef) )
	ROM_LOAD( "mblzpu2.3h",0x4000, 0x2000, CRC(48e5306c) SHA1(d27cc85d24c7b6c23c5c96be4dad5cae6e8069be) )
	ROM_LOAD( "mblzpu3.4h",0x6000, 0x2000, CRC(eba91546) SHA1(8c1da4e0d9b562dbbf7c7583dbf567c804eb670f) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (VSB board) */
	ROM_LOAD( "mblvsb0.2d",0x0000, 0x1000, CRC(0cf7a1c3) SHA1(af27e3a3b51d03d46c62c2797268744d0577d075) )
	ROM_LOAD( "mblvsb1.4d",0x1000, 0x1000, CRC(0b8d0e43) SHA1(b3ddb7561e715a58ca512fe76e53cda39402a8e4) )

	ROM_REGION( 0x10000, "digitalker", 0) /* 64k? for digitalker voice samples */
	ROM_LOAD( "mblvsb2.2a",0x0000, 0x1000, NO_DUMP ) /* size may be wrong */
	ROM_LOAD( "mblvsb3.4a",0x1000, 0x1000, NO_DUMP ) /* size may be wrong */

	ROM_REGION( 0x18000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "mblcfb0.8g",0x0000, 0x2000, CRC(948a2c5e) SHA1(d693f1b96caf31649f600c5038bb79b0d1d16133) )

	ROM_LOAD( "mblcfb2.8k",0x10000,0x2000, CRC(36237058) SHA1(9db8fced37a3d40c4ea5b87ea18ac8e75d71e586) )/*banked at 0x4000 (select=0)*/
	ROM_LOAD( "mblcfb3.10k",0x12000,0x2000, CRC(18d75d7f) SHA1(51c35ea4a2127439a1299863eb74e57be833e2e4) )/*banked at 0x4000 (select=1)*/
	/* empty socket??? (the *name* of next rom seems good ?) or wrong schematics ?*/
	ROM_LOAD( "mblcfb4.14k",0x16000,0x2000, CRC(1805acdc) SHA1(40b8e70e6ba69ac864af0b276e81218e63e48deb) )/*banked at 0x4000 (select=3) (assumed to be at 14k, may be at 12k)*/

	ROM_REGION( 0x00640, "proms", 0 )
	ROM_LOAD( "82s123.8b", 0x0000, 0x0020, CRC(d558af5a) SHA1(060556beeb1f6732c4520dcfb0086c428f7b9ce3) )
	ROM_LOAD( "82s123.9b", 0x0020, 0x0020, CRC(0390d748) SHA1(df0f750c1df45cc7bfb9dbabfa2b94563d19172a) )
	ROM_LOAD( "82s129.8g", 0x0040, 0x0100, CRC(19680615) SHA1(c309eb83e66b202bae9174dc2ffce231fca40644) )
	ROM_LOAD( "82s129.9g", 0x0140, 0x0100, CRC(f8c2c85b) SHA1(d9514af5682a2c5dec5366dcbdf5c7f6ef9f5380) )
	ROM_LOAD( "6353-1.16a", 0x240, 0x0400, NO_DUMP ) /* 82s137-equivalent video prom, next to VCU */

	ROM_REGION( 0x00240, "pals", 0 )
	ROM_LOAD( "pal16r8.7d", 0x0000, 0x098, NO_DUMP ) /* pal on zpu board, for ?protection? (similar to bagman?) */
ROM_END

ROM_START( mazerblaa )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "zpu0.1h",       0x0000, 0x2000, CRC(aa77705c) SHA1(ef93c3eaa66591bef495caa101ef2aff93f2de8c) )
	ROM_LOAD( "zpu1.2h",       0x2000, 0x2000, CRC(599e1b97) SHA1(ceeb3017d6130d4d54ff4436261f2d3f2a29f8ab) )
	ROM_LOAD( "zpu2.3h",       0x4000, 0x2000, CRC(e1504613) SHA1(815b56e067d60dda6c5ebed97ef8da3f6c2927ad) )
	ROM_LOAD( "zpu3.4h",       0x6000, 0x2000, CRC(fd27f409) SHA1(e3d49b931325c75cc0c1075944095bb48501501f) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (VSB board) */
	ROM_LOAD( "mblvsb0.2d",0x0000, 0x1000, CRC(0cf7a1c3) SHA1(af27e3a3b51d03d46c62c2797268744d0577d075) )
	ROM_LOAD( "mblvsb1.4d",0x1000, 0x1000, CRC(0b8d0e43) SHA1(b3ddb7561e715a58ca512fe76e53cda39402a8e4) )

	ROM_REGION( 0x10000, "digitalker", 0) /* 64k? for digitalker voice samples */
	ROM_LOAD( "mblvsb2.2a",0x0000, 0x1000, NO_DUMP ) /* size may be wrong */
	ROM_LOAD( "mblvsb3.4a",0x1000, 0x1000, NO_DUMP ) /* size may be wrong */

	ROM_REGION( 0x18000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "mblcfb0.8g",0x0000, 0x2000, CRC(948a2c5e) SHA1(d693f1b96caf31649f600c5038bb79b0d1d16133) )

	ROM_LOAD( "mblcfb2.8k",0x10000,0x2000, CRC(36237058) SHA1(9db8fced37a3d40c4ea5b87ea18ac8e75d71e586) )/*banked at 0x4000 (select=0)*/
	ROM_LOAD( "mblcfb3.10k",0x12000,0x2000, CRC(18d75d7f) SHA1(51c35ea4a2127439a1299863eb74e57be833e2e4) )/*banked at 0x4000 (select=1)*/
	/* empty socket??? (the *name* of next rom seems good ?) or wrong schematics ?*/
	ROM_LOAD( "mblcfb4.14k",0x16000,0x2000, CRC(1805acdc) SHA1(40b8e70e6ba69ac864af0b276e81218e63e48deb) )/*banked at 0x4000 (select=3) (assumed to be at 14k, may be at 12k)*/

	ROM_REGION( 0x00640, "proms", 0 )
	ROM_LOAD( "82s123.8b", 0x0000, 0x0020, CRC(d558af5a) SHA1(060556beeb1f6732c4520dcfb0086c428f7b9ce3) )
	ROM_LOAD( "82s123.9b", 0x0020, 0x0020, CRC(0390d748) SHA1(df0f750c1df45cc7bfb9dbabfa2b94563d19172a) )
	ROM_LOAD( "82s129.8g", 0x0040, 0x0100, CRC(19680615) SHA1(c309eb83e66b202bae9174dc2ffce231fca40644) )
	ROM_LOAD( "82s129.9g", 0x0140, 0x0100, CRC(f8c2c85b) SHA1(d9514af5682a2c5dec5366dcbdf5c7f6ef9f5380) )
	ROM_LOAD( "6353-1.16a", 0x240, 0x0400, NO_DUMP ) /* 82s137-equivalent video prom, next to VCU */

	ROM_REGION( 0x00240, "pals", 0 )
	ROM_LOAD( "pal16r8.7d", 0x0000, 0x098, NO_DUMP ) /* pal on zpu board, for ?protection? (similar to bagman?) */
ROM_END


ROM_START( greatgun )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "zpu0",0x0000, 0x2000, CRC(80cf2cbf) SHA1(ea24b844ea6d8fc54adb2e28be68e1f3e1184b8b) )
	ROM_LOAD( "zpu1",0x2000, 0x2000, CRC(fc12af94) SHA1(65f5bca2853271c232bd02dfc3467e6a4f7f0a6f) )
	ROM_LOAD( "zpu2",0x4000, 0x2000, CRC(b34cfa26) SHA1(903adc6de0d34e5bc8fb0f8d3e74ff53204d8c68) )
	ROM_LOAD( "zpu3",0x6000, 0x2000, CRC(c142ebdf) SHA1(0b87740d26b19a05f65b811225ee0053ddb27d22) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (PSB board) */
	ROM_LOAD( "psba4",0x0000, 0x2000, CRC(172a793e) SHA1(3618a778af1f4a6267bf7e0786529be731ac9b76) )

	ROM_REGION( 0x38000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "cfb0",0x0000, 0x2000, CRC(ee372b1f) SHA1(b630fd659d59eb8c2540f18d91ae0d72e859fc4f) )
	ROM_LOAD( "cfb1",0x2000, 0x2000, CRC(b76d9527) SHA1(8f16b850bd67d553aaaf7e176754e36aba581445) )

	ROM_LOAD( "psb00",0x10000,0x2000, CRC(b4956100) SHA1(98baf5c27c76dc5c4eafc44f42705239504637fe) )/*banked at 0x4000*/
	ROM_LOAD( "psb01",0x12000,0x2000, CRC(acdce2ee) SHA1(96b8961afbd0006b10cfdc825aefe27ec18121ff) )
	ROM_LOAD( "psb02",0x14000,0x2000, CRC(cb840fc6) SHA1(c30c72d355e1957f3715e9fab701f65b9d7d632a) )
	ROM_LOAD( "psb03",0x16000,0x2000, CRC(86ea6f99) SHA1(ce5d42557d0a62eebe3d0cee28587d60707573e4) )
	ROM_LOAD( "psb04",0x18000,0x2000, CRC(65379893) SHA1(84bb755e23d5ce13b1c82e59f24f3890c50697cc) )
	ROM_LOAD( "psb05",0x1a000,0x2000, CRC(f82245cb) SHA1(fa1cab94a03ce7b8e45ea6eec572b21f268f7547) )
	ROM_LOAD( "psb06",0x1c000,0x2000, CRC(6b86794f) SHA1(72cf67ecf5a9198ecb44dd846de968e6cdd6458d) )
	ROM_LOAD( "psb07",0x1e000,0x2000, CRC(60a7abf3) SHA1(44b932d8af29ec706c29d6b71a8bac6318d92315) )
	ROM_LOAD( "psb08",0x20000,0x2000, CRC(854be14e) SHA1(ae9b1fe2443c87bb4334bc776f7bc7e5fa874f38) )
	ROM_LOAD( "psb09",0x22000,0x2000, CRC(b2e8afa3) SHA1(30a3d83bf1ec7885549b47f9569e9ae0d05b948d) )
	ROM_LOAD( "psb10",0x24000,0x2000, CRC(fbfb0aab) SHA1(2eb666a5eff31019b4ffdfc82e242ff47cd59527) )
	ROM_LOAD( "psb11",0x26000,0x2000, CRC(ddcd3cec) SHA1(7d0c3b4160b11ebe9b097664190d8ae605413baa) )
	ROM_LOAD( "psb12",0x28000,0x2000, CRC(c6617377) SHA1(29a6fc52e06c41f06ee333aad707c3a1952dff4d) )
	ROM_LOAD( "psb13",0x2a000,0x2000, CRC(aeab8555) SHA1(c398cac5210022e3c9e25a9f2ef1017b27c21e62) )
	ROM_LOAD( "psb14",0x2c000,0x2000, CRC(ef35e314) SHA1(2e20517ff89b153fd888cf4eb0404a802e16b1b7) )
	ROM_LOAD( "psb15",0x2e000,0x2000, CRC(1fafe83d) SHA1(d1d406275f50d87547aabe1295795099f341433d) )
	ROM_LOAD( "psb16",0x30000,0x2000, CRC(ec49864f) SHA1(7a3b295972b52682406f75c4fe12c29632452491) )
	ROM_LOAD( "psb17",0x32000,0x2000, CRC(d9778e85) SHA1(2998f0a08cdba8a75e687a54cb9a03edeb4b22cd) )
	ROM_LOAD( "psb18",0x34000,0x2000, CRC(ef61b6c0) SHA1(7e8a82beefb9fd8e219fc4d7d25a3a43ab8aadf7) )
	ROM_LOAD( "psb19",0x36000,0x2000, CRC(68752e0d) SHA1(58a4921e4f774af5e1ef7af67f06e9b43643ffab) )
ROM_END

static DRIVER_INIT( mazerbla )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	state->m_game_id = MAZERBLA;
}

static DRIVER_INIT( greatgun )
{
	mazerbla_state *state = machine.driver_data<mazerbla_state>();
	UINT8 *rom = machine.region("sub2")->base();

	state->m_game_id = GREATGUN;

	//  patch VCU test
	//  VCU test starts at PC=0x56f
	rom[0x05b6] = 0;
	rom[0x05b7] = 0;
	//  so we also need to patch ROM checksum test
	rom[0x037f] = 0;
	rom[0x0380] = 0;
}

GAME( 1983, mazerbla,  0,        mazerbla,  mazerbla, mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 1)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1983, mazerblaa, mazerbla, mazerbla,  mazerbla, mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 2)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1983, greatgun,  0,        greatgun,  greatgun, greatgun, ROT0, "Stern Electronics", "Great Guns", GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
