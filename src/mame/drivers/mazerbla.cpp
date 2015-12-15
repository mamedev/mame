// license:???
// copyright-holders:Jarek Burczynski, Angelo Salese
/****************************************************************************

Mazer Blazer by Stern Electronics (c) 1983
Great Guns by Stern Electronics (c) 1983


Driver by Jarek Burczynski
Added notes by Lord Nightmare
2003.03.19

Notes:
======
Mazer blazer consists of four boards in a cage:
ZPU-2000 - main cpu board (Zentral (sic) Processor Unit)
 - this board has the main cpu on it and four roms (seven sockets for roms, three empty)
   - roms in sockets "ROM0"@H1 "ROM1"@H2 "ROM2"@H3 "ROM3"@H4
   - "ROM4"@H5 "ROM5"@H6 "ROM6"@H7 are empty
   - roms are marked "MAZER BLAZER // ZPU <rom#> RA<revision#> // 1983 STERN"
 - 32 dipswitches in 4 banks of 8
 - four 'test button' style switches
 - one 4Mhz xtal @A1
 - this same board is shared with cliff hanger (cliffhgr.c)

CFB-1000 - video/subcpu board (Color Frame Board)
 - this board has a sub-cpu on it and four roms (six sockets for roms, two empty)
   - the roms go in sockets "ROM0"@G8, "ROM2"@K8, "ROM3"@K10, "ROM4"@K11
   - "ROM1"@G6 and "ROM5"@K12 are empty
   - roms are marked "MAZER BLAZER // CFB <rom#> RA<revision#> // 1983 STERN"
 - "shared ram" ?6116? SRAM @G3
 - DIP64 custom framebuffer controller "Video Controller"@E11
 - "Parameter ram" ?6116? SRAM @K13
 - "Frame buffer" 16x ?4116? DRAM @ right edge of pcb
 - "Erase PROM" @A16 (UNDUMPED)
 - 22.1164Mhz xtal @K14
 - 8 dipswitches in 2 banks of 4, @B5 and @B7
 - LED @B7

VSB-2000 - sound/speech/subcpu board (Voice and Sound Board)
 - this board has a sub-cpu on it, a digitalker speech chip, 4 roms (2 dumped, 2 undumped) and 4 PROMs (all dumped)
 - Z80 CPU @E6
   - Roms at "ROM0"@D2, "ROM1"@D4
 - MM54104 Digitalker "SPU" @A6
   - Roms at "ROM2"@A2, "ROM3"@A4 (UNDUMPED)
 - roms are marked "MAZER BLAZER // VSB <rom#> RA<revision#> // 1983 STERN"
 - Sound section
   - PROMS: 82s123: @B8 @B9; 82s129: @G8 @G9 (all dumped)

CRF-1001 - RF Filter board for video/audio output
 - this same board is shared with cliff hanger (cliffhgr.c)

Versions:
======
Mazer blazer's zpu-2000 roms are known to exist in at least the following versions:
RA3
and probably exist in versions RA1 and RA2 as well.
It is currently unknown what versions the two sets in MAME correspond to.
The other roms are likely always version RA1, as the RA3-zpu-2000 board has RA1
roms for all roms except the zpu-2000 board.

Issues:
======
Sprites leave trails in both games
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

====

Mazer Blazer DASM notes:
master z80
[0x0000]: clear 0x4c-0x4f i/o ports (ls670)
[0x0792]: z80 regs check
[0x0924]: z80 SP reg check
(following two happens quite often, basically after every POST test)
    [0x08e1]: writes 0xaa to led i/o port
    [0x08d2]: writes 0x02 to A, then 0 to led i/o port
[0x07d7]: checks ROM 0x0000-0x1fff
[0x07ee]: checks ROM 0x2000-0x3fff
[0x0805]: checks ROM 0x4000-0x5fff
[0x081c]: checks ROM 0x6000-0x7fff
[0x0833]: checks RAM 0xe800-0xefff, with a bit-wise pattern (1-2-4-8-0x10-0x20-0x40-0x80)
[0x0844]: transfers RAM 0xe000-0xe7ff to RAM 0xe800-0xefff
[0x0850]: checks RAM 0xe000-0xe7ff, with a bit-wise pattern (1-2-4-8-0x10-0x20-0x40-0x80)
[0x085a]: transfers RAM 0xe800-0xefff to RAM 0xe000-0xe7ff
[0x086b]: Puts some values to ls670 ports, reads back afterwards, does this twice (with different values)
[0x088b]: shared RAM check, values at 0xc000-0xc7ff must be 0x55 (otherwise wait until they are)
[0x089c]: writes 0xaa to shared RAM 0xc000-0xc7ff
[0x08ab]: shared RAM check, values at 0xc000-0xc7ff must be 0x00 (otherwise wait until they are)
[0x08c2]: writes 0 to shared RAM
[0x000d]: clears RAM 0xe0fb, 0xe0fd, 0xe0fe (word)
    [0x2138]: puts a 1 to 0xe0fb
[0x0021]: clears i/o at 0x6a (lamps), clears 0xe563, 0xe004, 0xe000, 0xe001, 0xe007, 0xe002, 0xe003, 0xe005,
          0xc04f, 0xe572, enables IM 2, clears 0xe581 / 0xe583 (word), puts default initials (0x2274->0xe058)

video z80
[0x535]:
[0x03c]: start of proper code

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "video/mb_vcu.h"


#define MAZERBLA 0x01
#define GREATGUN 0x02

#define MASTER_CLOCK XTAL_4MHz
#define SOUND_CLOCK XTAL_14_31818MHz


class mazerbla_state : public driver_device
{
public:
	mazerbla_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_vcu(*this,"vcu"),
		m_screen(*this, "screen")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<mb_vcu_device> m_vcu;
	required_device<screen_device> m_screen;

	/* video-related */
	bitmap_ind16 m_tmpbitmaps[4];

	UINT8 m_vcu_video_reg[4];
	UINT32 m_vcu_gfx_addr;
	UINT32 m_vcu_gfx_param_addr;

	UINT8 m_bknd_col;
	UINT8 m_port02_status;
	UINT8 m_vbank;      /* video page select signal, likely for double buffering ?*/
	UINT32 m_xpos;
	UINT32 m_ypos;
	UINT32 m_pix_xsize;
	UINT32 m_pix_ysize;
	UINT8 m_color1;
	UINT8 m_color2;
	UINT8 m_mode;
	UINT8 m_plane;
	UINT8 m_lookup_ram[0x100*4];
	UINT32 m_gfx_rom_bank;  /* graphics ROMs are banked */

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
	DECLARE_WRITE8_MEMBER(gg_led_ctrl_w);
	DECLARE_READ8_MEMBER(soundcommand_r);
	DECLARE_DRIVER_INIT(mazerbla);
	DECLARE_DRIVER_INIT(greatgun);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mazerbla);
	UINT32 screen_update_mazerbla(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(sound_interrupt);
	TIMER_CALLBACK_MEMBER(deferred_ls670_0_w);
	TIMER_CALLBACK_MEMBER(deferred_ls670_1_w);
	TIMER_CALLBACK_MEMBER(delayed_sound_w);
	IRQ_CALLBACK_MEMBER(irq_callback);
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

PALETTE_INIT_MEMBER(mazerbla_state, mazerbla)
{
	static const int resistances_r[2]  = { 4700, 2200 };
	static const int resistances_gb[3] = { 10000, 4700, 2200 };

	/* just to calculate coefficients for later use */
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_gb, m_weights_g,    3600,   0,
			3,  resistances_gb, m_weights_b,    3600,   0,
			2,  resistances_r,  m_weights_r,    3600,   0);
}

void mazerbla_state::video_start()
{
#if 0
	m_planes_enabled[0] = m_planes_enabled[1] = m_planes_enabled[2] = m_planes_enabled[3] = 1;
	m_dbg_info = 1;
	m_dbg_gfx_e = 1;
	m_dbg_clr_e = 0;
	m_dbg_vbank = 1;
	m_dbg_lookup = 4;
#endif

	m_screen->register_screen_bitmap(m_tmpbitmaps[0]);
	m_screen->register_screen_bitmap(m_tmpbitmaps[1]);
	m_screen->register_screen_bitmap(m_tmpbitmaps[2]);
	m_screen->register_screen_bitmap(m_tmpbitmaps[3]);

	save_item(NAME(m_tmpbitmaps[0]));
	save_item(NAME(m_tmpbitmaps[1]));
	save_item(NAME(m_tmpbitmaps[2]));
	save_item(NAME(m_tmpbitmaps[3]));
}


UINT32 mazerbla_state::screen_update_mazerbla(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vcu->screen_update(screen,bitmap,cliprect);
	return 0;
}

void mazerbla_state::screen_eof(screen_device &screen, bool state)
{
	if (state)
	{
		m_vcu->screen_eof();
	}
}


/* reference */
#if 0
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

		m_palette->set_pen_color(255, rgb_t(r, g, b));
		//logerror("background color (port 01) write=%02x\n",data);
	}
}
#endif

#if 0
WRITE8_MEMBER(mazerbla_state::cfb_vbank_w)
{
	/* only bit 6 connected */
	m_vbank = BIT(data, 6);
}
#endif


WRITE8_MEMBER(mazerbla_state::cfb_rom_bank_sel_w)
{
	m_gfx_rom_bank = data;

	membank("bank1")->set_entry(m_gfx_rom_bank);
}

#if 0
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

	return machine().rand();
}


READ8_MEMBER(mazerbla_state::vcu_set_gfx_addr_r)
{
	UINT8 * rom = memregion("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000;
	int offs;
	int x, y;
	int bits = 0;
	UINT8 color_base = 0;

	color_base = m_vcu_video_reg[1] << 4;

//    if ((mode <= 0x07) || (mode >= 0x10))
/*
    {
        printf("paradr=");
        printf("%3x ", m_vcu_gfx_param_addr );

        printf("%02x ", m_cfb_ram[m_vcu_gfx_param_addr + 0] );
        printf("x=%04x ", m_xpos );                 //1,2
        printf("y=%04x ", m_ypos );                 //3,4
        printf("color1=%02x ", m_color1);             //5
        printf("color2=%02x ", m_color2);           //6
        printf("mode=%02x ", m_mode );              //7
        printf("xpix=%02x ", m_pix_xsize );         //8
        printf("ypix=%02x ", m_pix_ysize );         //9

        printf("addr=%4i bank=%1i\n", offset, m_gfx_rom_bank);
    }
*/

	m_vcu_gfx_addr = offset;

	/* draw */
	offs = m_vcu_gfx_addr;

	if(0)
	{
		for (y = 0; y <= m_pix_ysize; y++)
		{
			for (x = 0; x <= m_pix_xsize; x++)
			{
				if(machine().input().code_pressed(KEYCODE_Z))
				{
					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256) )
						m_tmpbitmaps[0].pix16(m_ypos + y, m_xpos + x) = m_plane | 0x10;
				}
				else
				{
					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256) )
						m_tmpbitmaps[m_plane].pix16(m_ypos + y, m_xpos + x) = m_mode | 0x10;
				}
			}
		}

		return 0;
	}

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
							col = color_base | ((m_color1 & 0x0f));     //background PEN
							break;
						case 1:
							col = color_base | ((m_color1 & 0xf0) >> 4);    //foreground PEN
							break;
						case 2:
							col = color_base | ((m_color2 & 0x0f)); //background PEN2
							break;
						case 3:
							col = color_base | ((m_color2 & 0xf0) >> 4);    //foreground PEN2
							break;
					}

					if (((m_xpos + x) < 256) && ((m_ypos + y) < 256) )
						m_tmpbitmaps[0].pix16(m_ypos + y, m_xpos + x) = col;

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
						m_tmpbitmaps[0].pix16(m_ypos + y, m_xpos + x) = data ? color_base | ((m_color1 & 0xf0) >> 4): color_base | ((m_color1 & 0x0f));

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
						m_tmpbitmaps[0].pix16(m_ypos + y, m_xpos + x) = col;

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

	return machine().rand();
}

READ8_MEMBER(mazerbla_state::vcu_set_clr_addr_r)
{
	UINT8 * rom = memregion("sub2")->base() + (m_gfx_rom_bank * 0x2000) + 0x10000;
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
				color_base = 0x80;  /* 0x80 constant: matches Mazer Blazer movie */

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
							col = color_base | ((m_color1 & 0x0f));     //background PEN
							break;
						case 1:
							col = color_base | ((m_color1 & 0xf0) >> 4);    //foreground PEN
							break;
						case 2:
							col = color_base | ((m_color2 & 0x0f)); //background PEN2
							break;
						case 3:
							col = color_base | ((m_color2 & 0xf0) >> 4);    //foreground PEN2
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
							m_palette->set_pen_color(x + y * 16, rgb_t(r, g, b));

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

	return machine().rand();
}
#endif

/*************************************
 *
 *  IRQ & Timer handlers
 *
 *************************************/

WRITE8_MEMBER(mazerbla_state::cfb_zpu_int_req_set_w)
{
	m_zpu_int_vector &= ~2; /* clear D1 on INTA (interrupt acknowledge) */

	m_maincpu->set_input_line(0, ASSERT_LINE);  /* main cpu interrupt (comes from CFB (generated at the start of INT routine on CFB) - vblank?) */
}

READ8_MEMBER(mazerbla_state::cfb_zpu_int_req_clr)
{
	// this clears all interrupts
	m_zpu_int_vector = 0xff;
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}

READ8_MEMBER(mazerbla_state::ls670_0_r)
{
	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_0[offset];
}

TIMER_CALLBACK_MEMBER(mazerbla_state::deferred_ls670_0_w)
{
	int offset = (param >> 8) & 255;
	int data = param & 255;

	m_ls670_0[offset] = data;
}

WRITE8_MEMBER(mazerbla_state::ls670_0_w)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(mazerbla_state::deferred_ls670_0_w),this), (offset << 8) | data);
}

READ8_MEMBER(mazerbla_state::ls670_1_r)
{
	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_1[offset];
}

TIMER_CALLBACK_MEMBER(mazerbla_state::deferred_ls670_1_w)
{
	int offset = (param >> 8) & 255;
	int data = param & 255;

	m_ls670_1[offset] = data;
}

WRITE8_MEMBER(mazerbla_state::ls670_1_w)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(mazerbla_state::deferred_ls670_1_w),this), (offset << 8) | data);
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

	ret = ioport(strobenames[m_bcd_7445])->read();

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

WRITE8_MEMBER(mazerbla_state::gg_led_ctrl_w)
{
	/* bit 0, bit 1 - led on */
	set_led_status(machine(), 1, BIT(data, 0));
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

READ8_MEMBER(mazerbla_state::soundcommand_r)
{
	return m_soundlatch;
}

TIMER_CALLBACK_MEMBER(mazerbla_state::delayed_sound_w)
{
	m_soundlatch = param;

	/* cause NMI on sound CPU */
	m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(mazerbla_state::main_sound_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(mazerbla_state::delayed_sound_w),this), data & 0xff);
}

WRITE8_MEMBER(mazerbla_state::sound_int_clear_w)
{
	m_subcpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(mazerbla_state::sound_nmi_clear_w)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
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
	AM_RANGE(0xe000, 0xefff) AM_RAM
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
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")                    /* GFX roms */
	AM_RANGE(0x4000, 0x4003) AM_DEVWRITE("vcu", mb_vcu_device, write_vregs)
	AM_RANGE(0x6000, 0x67ff) AM_DEVREADWRITE("vcu", mb_vcu_device, read_ram, write_ram)
	AM_RANGE(0xa000, 0xa7ff) AM_DEVREAD("vcu", mb_vcu_device, load_params)
	AM_RANGE(0xc000, 0xdfff) AM_DEVREAD("vcu", mb_vcu_device, load_gfx)
	AM_RANGE(0xe000, 0xffff) AM_DEVREAD("vcu", mb_vcu_device, load_set_clr)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazerbla_cpu3_io_map, AS_IO, 8, mazerbla_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("vcu", mb_vcu_device, background_color_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("vcu", mb_vcu_device, status_r) AM_WRITE(cfb_led_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(cfb_zpu_int_req_set_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(cfb_rom_bank_sel_w)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE("vcu", mb_vcu_device, vbank_w)
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

static ADDRESS_MAP_START( greatgun_sound_map, AS_PROGRAM, 8, mazerbla_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x4000, 0x4001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(sound_int_clear_w)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(sound_nmi_clear_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( mazerbla )
	PORT_START("ZPU")   /* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")  /* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Freeze Time" )
	PORT_DIPSETTING(    0x0c, "1.5 seconds" )
	PORT_DIPSETTING(    0x08, "2.0 seconds" )
	PORT_DIPSETTING(    0x04, "2.5 seconds" )
	PORT_DIPSETTING(    0x00, "3.0 seconds" )
	PORT_DIPNAME( 0x30, 0x00, "Number of points for extra frezze & first life" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x20, "25000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x00, "35000" )
	PORT_DIPNAME( 0xc0, 0x00, "Number of points for extra life other than first" )
	PORT_DIPSETTING(    0xc0, "40000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x40, "60000" )
	PORT_DIPSETTING(    0x00, "70000" )

	PORT_START("DSW1")  /* Strobe 2: Dip Switches 20-27*/
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

	PORT_START("DSW2")  /* Strobe 3: Dip Switches 12-19*/
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x03, 0x02, "Number of Freezes" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Gun Knocker" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//dips 7-11 - not listed in manual
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTONS")   /* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STICK0_X")  /* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(1)
	PORT_START("STICK0_Y")  /* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	/* Mazer Blazer cabinet has only one gun, really */
	PORT_START("STICK1_X")  /* Strobe 8: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(2)
	PORT_START("STICK1_Y")  /* Strobe 9: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( greatgun )
	PORT_START("ZPU")   /* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")  /* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x00, "Starting Number of Bullets/Credit" )
	PORT_DIPSETTING(    0x03, "60" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0x0c, 0x00, "Target Size" )
	PORT_DIPSETTING(    0x0c, "7 x 7" )
	PORT_DIPSETTING(    0x08, "9 x 9" )
	PORT_DIPSETTING(    0x04, "11x11" )
	PORT_DIPSETTING(    0x00, "7 x 7" )
	PORT_DIPNAME( 0x70, 0x00, "Number of points for extra bullet" )
	PORT_DIPSETTING(    0x70, "1000" )
	PORT_DIPSETTING(    0x60, "2000" )
	PORT_DIPSETTING(    0x50, "3000" )
	PORT_DIPSETTING(    0x40, "4000" )
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "6000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x00, "8000" )
	/* from manual:
	    "This switch is used when an optional coin return or ticket dispenser is used"
	*/
	PORT_DIPNAME( 0x80, 0x00, "Number of coins or tickets returned" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW1")  /* Strobe 2: Dip Switches 20-27*/
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

	PORT_START("DSW2")  /* Strobe 3: Dip Switches 12-19*/
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x01, 0x01, "Free game/coin return" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//dips 5-11 - not listed in manual
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("BUTTONS")   /* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("STICK0_X")  /* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)
	PORT_START("STICK0_Y")  /* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	PORT_START("STICK1_X")  /* Strobe 8: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)
	PORT_START("STICK1_Y")  /* Strobe 9: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*************************************
 *
 *  Sound HW Configs
 *
 *************************************/

IRQ_CALLBACK_MEMBER(mazerbla_state::irq_callback)
{
	/* all data lines are tied to +5V via 10K resistors */
	/* D1 is set to GND when INT comes from CFB */
	/* D2 is set to GND when INT comes from ZPU board - from 6850 on schematics (RS232 controller) */

	/* resulting vectors:    - effect according to disasm:
	--------------------------------------------------------
	1111 11000 (0xf8)        - results in same as 0xfc
	1111 11010 (0xfa)        - does nothing, assume it was used for debugging
	1111 11100 (0xfc)        - calls several routines

	note:
	1111 11110 (0xfe) - cannot happen and is not handled by game */

	return (m_zpu_int_vector & ~1);  /* D0->GND is performed on CFB board */
}

/* frequency is 14.318 MHz/16/16/16/16 */
INTERRUPT_GEN_MEMBER(mazerbla_state::sound_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Machine driver definitions
 *
 *************************************/

void mazerbla_state::machine_start()
{
	membank("bank1")->configure_entries(0, 256, memregion("sub2")->base() + 0x10000, 0x2000);

	save_item(NAME(m_vcu_video_reg));
	save_item(NAME(m_vcu_gfx_addr));
	save_item(NAME(m_vcu_gfx_param_addr));

	save_item(NAME(m_bknd_col));
	save_item(NAME(m_port02_status));
	save_item(NAME(m_vbank));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_pix_xsize));
	save_item(NAME(m_pix_ysize));
	save_item(NAME(m_color1));
	save_item(NAME(m_color2));
	save_item(NAME(m_mode));
	save_item(NAME(m_plane));
	save_item(NAME(m_lookup_ram));
	save_item(NAME(m_gfx_rom_bank));

	save_item(NAME(m_ls670_0));
	save_item(NAME(m_ls670_1));

	save_item(NAME(m_zpu_int_vector));

	save_item(NAME(m_bcd_7445));

	save_item(NAME(m_vsb_ls273));
	save_item(NAME(m_soundlatch));
}

void mazerbla_state::machine_reset()
{
	int i;

	m_zpu_int_vector = 0xff;

	m_bknd_col = 0xaa;
	m_gfx_rom_bank = 0xff;

	m_vcu_gfx_addr = 0;
	m_vcu_gfx_param_addr = 0;
	m_port02_status = 0;
	m_vbank = 0;
	m_xpos = 0;
	m_ypos = 0;
	m_pix_xsize = 0;
	m_pix_ysize = 0;
	m_color1 = 0;
	m_color2 = 0;
	m_mode = 0;
	m_plane = 0;
	m_bcd_7445 = 0;
	m_vsb_ls273 = 0;
	m_soundlatch = 0;

	for (i = 0; i < 4; i++)
	{
		m_vcu_video_reg[i] = 0;
		m_ls670_0[i] = 0;
		m_ls670_1[i] = 0;
	}

	memset(m_lookup_ram, 0, ARRAY_LENGTH(m_lookup_ram));
}

static MACHINE_CONFIG_START( mazerbla, mazerbla_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)  /* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	MCFG_CPU_PROGRAM_MAP(mazerbla_map)
	MCFG_CPU_IO_MAP(mazerbla_io_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mazerbla_state,irq_callback)

	MCFG_CPU_ADD("sub", Z80, MASTER_CLOCK)  /* 4 MHz, NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu2_map)
	MCFG_CPU_IO_MAP(mazerbla_cpu2_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mazerbla_state, irq0_line_hold,  400) /* frequency in Hz */

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK) /* 4 MHz, no  NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu3_map)
	MCFG_CPU_IO_MAP(mazerbla_cpu3_io_map)
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mazerbla_state,  irq0_line_hold)

	/* synchronization forced on the fly */
	MCFG_DEVICE_ADD("vcu", MB_VCU, SOUND_CLOCK/4)
	MCFG_MB_VCU_CPU("sub2")
	MCFG_MB_VCU_PALETTE("palette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mazerbla_state, screen_update_mazerbla)

	MCFG_PALETTE_ADD("palette", 256+1)
	MCFG_PALETTE_INIT_OWNER(mazerbla_state, mazerbla)

	/* sound hardware */
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( greatgun, mazerbla_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)  /* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	MCFG_CPU_PROGRAM_MAP(mazerbla_map)
	MCFG_CPU_IO_MAP(greatgun_io_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mazerbla_state,irq_callback)

	MCFG_CPU_ADD("sub", Z80, SOUND_CLOCK / 4)   /* 3.579500 MHz, NMI - caused by sound command write, periodic INT */
	MCFG_CPU_PROGRAM_MAP(greatgun_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mazerbla_state, sound_interrupt,  (double)14318180/16/16/16/16 )

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK) /* 4 MHz, no  NMI, IM1 INT */
	MCFG_CPU_PROGRAM_MAP(mazerbla_cpu3_map)
	MCFG_CPU_IO_MAP(mazerbla_cpu3_io_map)
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mazerbla_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("vcu", MB_VCU, SOUND_CLOCK/4)
	MCFG_MB_VCU_CPU("sub2")
	MCFG_MB_VCU_PALETTE("palette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mazerbla_state, screen_update_mazerbla)
	MCFG_SCREEN_VBLANK_DRIVER(mazerbla_state, screen_eof)

	MCFG_PALETTE_ADD("palette", 256+1)
	MCFG_PALETTE_INIT_OWNER(mazerbla_state, mazerbla)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, SOUND_CLOCK / 8)
	MCFG_AY8910_PORT_B_READ_CB(READ8(mazerbla_state, soundcommand_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, SOUND_CLOCK / 8)
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(mazerbla_state, gg_led_ctrl_w))
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
	ROM_LOAD( "zpu0.1h",0x0000, 0x2000, CRC(80cf2cbf) SHA1(ea24b844ea6d8fc54adb2e28be68e1f3e1184b8b) )
	ROM_LOAD( "zpu1.2h",0x2000, 0x2000, CRC(fc12af94) SHA1(65f5bca2853271c232bd02dfc3467e6a4f7f0a6f) )
	ROM_LOAD( "zpu2.3h",0x4000, 0x2000, CRC(b34cfa26) SHA1(903adc6de0d34e5bc8fb0f8d3e74ff53204d8c68) )
	ROM_LOAD( "zpu3.4h",0x6000, 0x2000, CRC(c142ebdf) SHA1(0b87740d26b19a05f65b811225ee0053ddb27d22) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (PSB board) */
	ROM_LOAD( "psb.4a",0x0000, 0x2000, CRC(172a793e) SHA1(3618a778af1f4a6267bf7e0786529be731ac9b76) )

	ROM_REGION( 0x60000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "cfb0.7f",0x00000, 0x2000, CRC(ee372b1f) SHA1(b630fd659d59eb8c2540f18d91ae0d72e859fc4f) )
	ROM_LOAD( "cfb1.8f",0x02000, 0x2000, CRC(b76d9527) SHA1(8f16b850bd67d553aaaf7e176754e36aba581445) )

	ROM_LOAD( "psb00.5g",0x10000,0x2000, CRC(b4956100) SHA1(98baf5c27c76dc5c4eafc44f42705239504637fe) )/*banked at 0x4000*/
	ROM_LOAD( "psb01.5f",0x14000,0x2000, CRC(acdce2ee) SHA1(96b8961afbd0006b10cfdc825aefe27ec18121ff) )
	ROM_LOAD( "psb02.5d",0x18000,0x2000, CRC(cb840fc6) SHA1(c30c72d355e1957f3715e9fab701f65b9d7d632a) )
	ROM_LOAD( "psb03.5c",0x1c000,0x2000, CRC(86ea6f99) SHA1(ce5d42557d0a62eebe3d0cee28587d60707573e4) )
	ROM_LOAD( "psb04.6g",0x20000,0x2000, CRC(65379893) SHA1(84bb755e23d5ce13b1c82e59f24f3890c50697cc) )
	ROM_LOAD( "psb05.6f",0x24000,0x2000, CRC(f82245cb) SHA1(fa1cab94a03ce7b8e45ea6eec572b21f268f7547) )
	ROM_LOAD( "psb06.6d",0x28000,0x2000, CRC(6b86794f) SHA1(72cf67ecf5a9198ecb44dd846de968e6cdd6458d) )
	ROM_LOAD( "psb07.6c",0x2c000,0x2000, CRC(60a7abf3) SHA1(44b932d8af29ec706c29d6b71a8bac6318d92315) )
	ROM_LOAD( "psb08.7g",0x30000,0x2000, CRC(854be14e) SHA1(ae9b1fe2443c87bb4334bc776f7bc7e5fa874f38) )
	ROM_LOAD( "psb09.7f",0x34000,0x2000, CRC(b2e8afa3) SHA1(30a3d83bf1ec7885549b47f9569e9ae0d05b948d) )
	ROM_LOAD( "psb10.7d",0x38000,0x2000, CRC(fbfb0aab) SHA1(2eb666a5eff31019b4ffdfc82e242ff47cd59527) )
	ROM_LOAD( "psb11.7c",0x3c000,0x2000, CRC(ddcd3cec) SHA1(7d0c3b4160b11ebe9b097664190d8ae605413baa) )
	ROM_LOAD( "psb12.8g",0x40000,0x2000, CRC(c6617377) SHA1(29a6fc52e06c41f06ee333aad707c3a1952dff4d) )
	ROM_LOAD( "psb13.8f",0x44000,0x2000, CRC(aeab8555) SHA1(c398cac5210022e3c9e25a9f2ef1017b27c21e62) )
	ROM_LOAD( "psb14.8d",0x48000,0x2000, CRC(ef35e314) SHA1(2e20517ff89b153fd888cf4eb0404a802e16b1b7) )
	ROM_LOAD( "psb15.8c",0x4c000,0x2000, CRC(1fafe83d) SHA1(d1d406275f50d87547aabe1295795099f341433d) )
	ROM_LOAD( "psb16.9g",0x50000,0x2000, CRC(ec49864f) SHA1(7a3b295972b52682406f75c4fe12c29632452491) )
	ROM_LOAD( "psb17.9f",0x54000,0x2000, CRC(d9778e85) SHA1(2998f0a08cdba8a75e687a54cb9a03edeb4b22cd) )
	ROM_LOAD( "psb18.9d",0x58000,0x2000, CRC(ef61b6c0) SHA1(7e8a82beefb9fd8e219fc4d7d25a3a43ab8aadf7) )
	ROM_LOAD( "psb19.9c",0x5c000,0x2000, CRC(68752e0d) SHA1(58a4921e4f774af5e1ef7af67f06e9b43643ffab) )
//  10g missing?
ROM_END

DRIVER_INIT_MEMBER(mazerbla_state,mazerbla)
{
	m_game_id = MAZERBLA;
}

DRIVER_INIT_MEMBER(mazerbla_state,greatgun)
{
	UINT8 *rom = memregion("sub2")->base();

	m_game_id = GREATGUN;

	//  patch VCU test
	//  VCU test starts at PC=0x56f
	rom[0x05b6] = 0;
	rom[0x05b7] = 0;
	//  so we also need to patch ROM checksum test
	rom[0x037f] = 0;
	rom[0x0380] = 0;
}

GAME( 1983, mazerbla,  0,        mazerbla,  mazerbla, mazerbla_state, mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1983, mazerblaa, mazerbla, mazerbla,  mazerbla, mazerbla_state, mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1983, greatgun,  0,        greatgun,  greatgun, mazerbla_state, greatgun, ROT0, "Stern Electronics", "Great Guns", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
