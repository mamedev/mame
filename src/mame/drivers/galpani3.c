/*
    Gals Panic 3
    (c) Kaneko 1995

    Driver by David Haywood

    Original Skeleton driver by David Haywood
    Early Progress by Sebastien Volpe

Check done by main code, as part of EEPROM data:
'Gals Panic 3 v0.96 95/08/29(Tue)'

 Sprites are from Supernova
 Backgrounds are 3x bitmap layers + some kind of priority / mask layer
 The bitmaps have blitter devices to decompress RLE rom data into them

*/



/*

Gals Panic 3 (JPN Ver.)
(c)1995 Kaneko

CPU:    68000-16
Sound:  YMZ280B-F
OSC:    28.6363MHz
        33.3333MHz
EEPROM: 93C46
Chips.: GRAP2 x3                <- R/G/B Chips?
        APRIO-GL
        BABY004
        GCNT2
        TBSOP01                 <- ToyBox NEC uPD78324 series MCU with 32K internal rom
        CG24173 6186            <- Sprites, see suprnova.c
        CG24143 4181            <- ^


G3P0J1.71     prg.
G3P1J1.102

GP340000.123  chr.
GP340100.122
GP340200.121
GP340300.120
G3G0J0.101
G3G1J0.100

G3D0X0.134

GP320000.1    OBJ chr.

GP310000.41   sound data
GP310100.40


--- Team Japump!!! ---
Dumped by Uki
10/22/2000

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/ymz280b.h"
#include "includes/kaneko16.h"
#include "video/sknsspr.h"

class galpani3_state : public kaneko16_state
{
public:
	galpani3_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_sprite_bitmap_1(1024, 1024),
		m_maincpu(*this,"maincpu")
		{ }

	UINT16* m_priority_buffer;
	UINT16* m_framebuffer1;
	UINT16* m_framebuffer2;
	UINT16* m_framebuffer3;
	UINT16* m_framebuffer1_palette;
	UINT16* m_framebuffer2_palette;
	UINT16* m_framebuffer3_palette;
	UINT16 m_framebuffer3_scrolly;
	UINT16 m_framebuffer3_scrollx;
	UINT16 m_framebuffer2_scrolly;
	UINT16 m_framebuffer2_scrollx;
	UINT16 m_framebuffer1_scrolly;
	UINT16 m_framebuffer1_scrollx;
	UINT16 m_priority_buffer_scrollx;
	UINT16 m_priority_buffer_scrolly;
	UINT16 m_framebuffer1_enable;
	UINT16 m_framebuffer2_enable;
	UINT16 m_framebuffer3_enable;
	UINT16* m_framebuffer1_bgcol;
	UINT16* m_framebuffer2_bgcol;
	UINT16* m_framebuffer3_bgcol;
	UINT16* m_framebuffer3_bright1;
	UINT16* m_framebuffer3_bright2;
	UINT16* m_framebuffer2_bright1;
	UINT16* m_framebuffer2_bright2;
	UINT16* m_framebuffer1_bright1;
	UINT16* m_framebuffer1_bright2;
	UINT16 *m_sprregs;
	UINT32 m_spriteram32[0x4000/4];
	UINT32 m_spc_regs[0x40/4];
	bitmap_ind16 m_sprite_bitmap_1;
	UINT16 *m_mcu_ram;
	UINT16 m_mcu_com[4];
	int m_regs1_i;
	int m_regs2_i;
	int m_regs3_i;
	UINT16 m_regs1_address_regs[0x20];
	UINT16 m_regs2_address_regs[0x20];
	UINT16 m_regs3_address_regs[0x20];

	required_device<cpu_device> m_maincpu;
	sknsspr_device* m_spritegen;
	DECLARE_WRITE16_MEMBER(galpani3_suprnova_sprite32_w);
	DECLARE_WRITE16_MEMBER(galpani3_suprnova_sprite32regs_w);
	DECLARE_WRITE16_MEMBER(galpani3_mcu_com0_w);
	DECLARE_WRITE16_MEMBER(galpani3_mcu_com1_w);
	DECLARE_WRITE16_MEMBER(galpani3_mcu_com2_w);
	DECLARE_WRITE16_MEMBER(galpani3_mcu_com3_w);
	DECLARE_READ16_MEMBER(galpani3_mcu_status_r);
	DECLARE_READ16_MEMBER(galpani3_regs1_r);
	DECLARE_READ16_MEMBER(galpani3_regs2_r);
	DECLARE_READ16_MEMBER(galpani3_regs3_r);
	DECLARE_WRITE16_MEMBER(galpani3_regs1_address_w);
	DECLARE_WRITE16_MEMBER(galpani3_regs1_go_w);
	DECLARE_WRITE16_MEMBER(galpani3_regs2_address_w);
	DECLARE_WRITE16_MEMBER(galpani3_regs2_go_w);
	DECLARE_WRITE16_MEMBER(galpani3_regs3_address_w);
	DECLARE_WRITE16_MEMBER(galpani3_regs3_go_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer1_palette_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer2_palette_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer3_palette_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer3_scrolly_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer3_scrollx_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer2_scrolly_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer2_scrollx_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer1_scrolly_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer1_scrollx_w);
	DECLARE_WRITE16_MEMBER(galpani3_priority_buffer_scrollx_w);
	DECLARE_WRITE16_MEMBER(galpani3_priority_buffer_scrolly_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer1_enable_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer2_enable_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer3_enable_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer1_bgcol_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer2_bgcol_w);
	DECLARE_WRITE16_MEMBER(galpani3_framebuffer3_bgcol_w);
};


/***************************************************************************

 video

***************************************************************************/





static TIMER_DEVICE_CALLBACK( galpani3_vblank ) // 2, 3, 5 ?
{
	galpani3_state *state = timer.machine().driver_data<galpani3_state>();
	int scanline = param;

	if(scanline == 240)
		device_set_input_line(state->m_maincpu, 2, HOLD_LINE);

	if(scanline == 0)
		device_set_input_line(state->m_maincpu, 3, HOLD_LINE);

	if(scanline == 128)
		device_set_input_line(state->m_maincpu, 5, HOLD_LINE); // timer, related to sound chip?
}


static VIDEO_START(galpani3)
{
	galpani3_state *state = machine.driver_data<galpani3_state>();
	/* so we can use suprnova.c */

	state->m_spritegen = machine.device<sknsspr_device>("spritegen");
	state->m_spritegen->skns_sprite_kludge(0,0);
}


static int gp3_is_alpha_pen(running_machine &machine, int pen)
{
	galpani3_state *state = machine.driver_data<galpani3_state>();
	UINT16 dat = 0;

	if (pen<0x4000)
	{
		dat = state->m_generic_paletteram_16[pen];
	}
	else if (pen<0x4100)
	{
		dat = state->m_framebuffer1_palette[pen&0xff];
	}
	else if (pen<0x4200)
	{
		dat = state->m_framebuffer2_palette[pen&0xff];
	}
	else if (pen<0x4300)
	{
		dat = state->m_framebuffer3_palette[pen&0xff];
	}
	else if (pen<0x4301)
	{
		dat = state->m_framebuffer1_bgcol[0];
	}
	else if (pen<0x4302)
	{
		dat = state->m_framebuffer2_bgcol[0];
	}
	else if (pen<0x4303)
	{
		dat = state->m_framebuffer3_bgcol[0];
	}

	if (dat&0x8000) return 1;
	else return 0;

}

static SCREEN_UPDATE_RGB32(galpani3)
{
	galpani3_state *state = screen.machine().driver_data<galpani3_state>();
	int x,y;
	UINT16* src1;
	UINT32* dst;
	UINT16 pixdata1;
	const pen_t *paldata = screen.machine().pens;

	bitmap.fill(0x0000, cliprect);

	{
		int drawy, drawx;
		for (drawy=0;drawy<512;drawy++)
		{
			int srcline1 = (drawy+state->m_framebuffer1_scrolly+11)&0x1ff;
			int srcline2 = (drawy+state->m_framebuffer2_scrolly+11)&0x1ff;
			int srcline3 = (drawy+state->m_framebuffer3_scrolly+11)&0x1ff;

			int priline  = (drawy+state->m_priority_buffer_scrolly+11)&0x1ff;

			for (drawx=0;drawx<512;drawx++)
			{
				int srcoffs1 = (drawx+state->m_framebuffer1_scrollx+67)&0x1ff;
				int srcoffs2 = (drawx+state->m_framebuffer2_scrollx+67)&0x1ff;
				int srcoffs3 = (drawx+state->m_framebuffer3_scrollx+67)&0x1ff;

				int prioffs  = (drawx+state->m_priority_buffer_scrollx+66)&0x1ff;

				UINT8 dat1 = state->m_framebuffer1[(srcline1*0x200)+srcoffs1];
				UINT8 dat2 = state->m_framebuffer2[(srcline2*0x200)+srcoffs2];
				UINT8 dat3 = state->m_framebuffer3[(srcline3*0x200)+srcoffs3];

				UINT8 pridat = state->m_priority_buffer[(priline*0x200)+prioffs];

				UINT32* dst = &bitmap.pix32(drawy, drawx);



				// this is all wrong
				if (pridat==0x0f) // relates to the area you've drawn over
				{
					if (dat1 && state->m_framebuffer1_enable)
					{
						dst[0] = paldata[dat1+0x4000];
					}

					if (dat2 && state->m_framebuffer2_enable)
					{
						dst[0] = paldata[dat2+0x4100];
					}

				}
				else if (pridat==0xcf) // the girl
				{
					dst[0] = paldata[0x4300];
				}
				else
				{
					/* this isn't right, but the registers have something to do with
                       alpha / mixing, and bit 0x8000 of the palette is DEFINITELY alpha
                       enable -- see fading in intro */
					if (dat1 && state->m_framebuffer1_enable)
					{
						UINT16 pen = dat1+0x4000;
						UINT32 pal = paldata[pen];

						if (gp3_is_alpha_pen(screen.machine(), pen))
						{
							int r,g,b;
							r = (pal & 0x00ff0000)>>16;
							g = (pal & 0x0000ff00)>>8;
							b = (pal & 0x000000ff)>>0;

							r = (r * state->m_framebuffer1_bright2[0]) / 0xff;
							g = (g * state->m_framebuffer1_bright2[0]) / 0xff;
							b = (b * state->m_framebuffer1_bright2[0]) / 0xff;

							pal = (r & 0x000000ff)<<16;
							pal |=(g & 0x000000ff)<<8;
							pal |=(b & 0x000000ff)<<0;

							dst[0] = pal;
						}
						else
						{
							dst[0] = pal;
						}
					}

					if (dat2 && state->m_framebuffer2_enable)
					{
						UINT16 pen = dat2+0x4100;
						UINT32 pal = paldata[pen];

						if (gp3_is_alpha_pen(screen.machine(), pen))
						{
							int r,g,b;
							r = (pal & 0x00ff0000)>>16;
							g = (pal & 0x0000ff00)>>8;
							b = (pal & 0x000000ff)>>0;

							r = (r * state->m_framebuffer2_bright2[0]) / 0xff;
							g = (g * state->m_framebuffer2_bright2[0]) / 0xff;
							b = (b * state->m_framebuffer2_bright2[0]) / 0xff;

							pal = (r & 0x000000ff)<<16;
							pal |=(g & 0x000000ff)<<8;
							pal |=(b & 0x000000ff)<<0;

							dst[0] |= pal;
						}
						else
						{
							dst[0] = pal;
						}
					}

					if (dat3 && state->m_framebuffer3_enable)
					{
						dst[0] = paldata[dat3+0x4200];
					}
				}

				/*
                else if (pridat==0x2f) // area outside of the girl
                {
                    //dst[0] = screen.machine().rand()&0x3fff;
                }

                else if (pridat==0x00) // the initial line / box that gets drawn
                {
                    //dst[0] = screen.machine().rand()&0x3fff;
                }
                else if (pridat==0x30) // during the 'gals boxes' on the intro
                {
                    //dst[0] = screen.machine().rand()&0x3fff;
                }
                else if (pridat==0x0c) // 'nice' at end of level
                {
                    //dst[0] = screen.machine().rand()&0x3fff;
                }
                else
                {
                    //printf("%02x, ",pridat);
                }
                */
			}
		}
	}

	state->m_sprite_bitmap_1.fill(0x0000, cliprect);

	state->m_spritegen->skns_draw_sprites(screen.machine(), state->m_sprite_bitmap_1, cliprect, &state->m_spriteram32[0], 0x4000, screen.machine().region("gfx1")->base(), screen.machine().region ("gfx1")->bytes(), state->m_spc_regs );

	// ignoring priority bits for now..
	for (y=0;y<240;y++)
	{
		src1 = &state->m_sprite_bitmap_1.pix16(y);
		dst =  &bitmap.pix32(y);

		for (x=0;x<320;x++)
		{
			pixdata1 = src1[x];

			if (pixdata1 & 0x3fff)
			{
				dst[x] = paldata[(pixdata1 & 0x3fff)];
			}
		}
	}




	return 0;
}


static INPUT_PORTS_START( galpani3 )
	PORT_START("P1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1  ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2  ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED	)

	PORT_START("DSW")	/* provided by the MCU - $200386.b <- $400200 */
	PORT_DIPNAME( 0x0100, 0x0100, "Test Mode" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) // ?
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )	// unused
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )	// unused ?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


WRITE16_MEMBER(galpani3_state::galpani3_suprnova_sprite32_w)
{

	COMBINE_DATA(&m_spriteram[offset]);
	offset>>=1;
	m_spriteram32[offset]=(m_spriteram[offset*2+1]<<16) | (m_spriteram[offset*2]);
}

WRITE16_MEMBER(galpani3_state::galpani3_suprnova_sprite32regs_w)
{

	COMBINE_DATA(&m_sprregs[offset]);
	offset>>=1;
	m_spc_regs[offset]=(m_sprregs[offset*2+1]<<16) | (m_sprregs[offset*2]);
}



/***************************************************************************

                            MCU Code Simulation
                (follows the implementation of kaneko16.c)

***************************************************************************/

static void galpani3_mcu_run(running_machine &machine)
{
	galpani3_state *state = machine.driver_data<galpani3_state>();
	UINT16 mcu_command = state->m_mcu_ram[0x0010/2];		/* command nb */
	UINT16 mcu_offset  = state->m_mcu_ram[0x0012/2] / 2;	/* offset in shared RAM where MCU will write */
	UINT16 mcu_subcmd  = state->m_mcu_ram[0x0014/2];		/* sub-command parameter, happens only for command #4 */

	logerror("%s: MCU executed command : %04X %04X\n",machine.describe_context(),mcu_command,mcu_offset*2);

	/* the only MCU commands found in program code are:
         0x04: protection: provide code/data,
         0x03: read DSW
         0x02: load NVRAM settings \ ATMEL AT93C46 chip,
         0x42: save NVRAM settings / 128 bytes serial EEPROM
    */
	switch (mcu_command >> 8)
	{
		case 0x03:	// DSW
		{
			state->m_mcu_ram[mcu_offset] = input_port_read(machine, "DSW");
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", machine.describe_context(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x02: // $38950 - load NVRAM settings
		{
			/* NOTE: code @ $38B46 & $38ab8 does exactly what is checked after MCU command
                     so that's what we'll mimic here... probably the initial NVRAM settings */
			int i;

			/* MCU writes 128 bytes to shared ram: last byte is the byte-sum */
			/* first 32 bytes (header): 0x8BE08E71.L, then the string "95/06/30 Gals Panic3Ver 0.95"; */
			state->m_mcu_ram[mcu_offset +  0] = 0x8BE0; state->m_mcu_ram[mcu_offset +  1] = 0x8E71;
			state->m_mcu_ram[mcu_offset +  2] = 0x3935; state->m_mcu_ram[mcu_offset +  3] = 0x2F30;
			state->m_mcu_ram[mcu_offset +  4] = 0x362F; state->m_mcu_ram[mcu_offset +  5] = 0x3330;
			state->m_mcu_ram[mcu_offset +  6] = 0x2047; state->m_mcu_ram[mcu_offset +  7] = 0x616C;
			state->m_mcu_ram[mcu_offset +  8] = 0x7320; state->m_mcu_ram[mcu_offset +  9] = 0x5061;
			state->m_mcu_ram[mcu_offset + 10] = 0x6E69; state->m_mcu_ram[mcu_offset + 11] = 0x6333;
			state->m_mcu_ram[mcu_offset + 12] = 0x5665; state->m_mcu_ram[mcu_offset + 13] = 0x7220;
			state->m_mcu_ram[mcu_offset + 14] = 0x302E; state->m_mcu_ram[mcu_offset + 15] = 0x3935;
			/* next 11 bytes - initial NVRAM settings */
			state->m_mcu_ram[mcu_offset + 16] = 0x0001; state->m_mcu_ram[mcu_offset + 17] = 0x0101;
			state->m_mcu_ram[mcu_offset + 18] = 0x0100; state->m_mcu_ram[mcu_offset + 19] = 0x0208;
			state->m_mcu_ram[mcu_offset + 20] = 0x02FF; state->m_mcu_ram[mcu_offset + 21] = 0x0000;
			/* rest is zeroes */
			for (i=22;i<63;i++)
				state->m_mcu_ram[mcu_offset + i] = 0;
			/* and sum is $0c.b */
			state->m_mcu_ram[mcu_offset + 63] = 0x000c;
		}
		break;

		case 0x04: // $38842 - provides code/data
		{
			toxboy_handle_04_subcommand(machine, mcu_subcmd, state->m_mcu_ram);
		}
		break;

		case 0x42: // $389ee - save NVRAM settings
		{
			// found, TODO: trace call in code !!!
		}
		break;

		default:
			logerror("UNKNOWN COMMAND\n");
	}
}

/*
  MCU doesn't execute exactly as it is coded right know (ala jchan):
   * com0=com1=0xFFFF -> command to execute
   * com2=com3=0xFFFF -> status reading only
*/

INLINE void galpani3_mcu_com_w(address_space *space, offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	galpani3_state *state = space->machine().driver_data<galpani3_state>();
	COMBINE_DATA(&state->m_mcu_com[_n_]);
	if (state->m_mcu_com[0] != 0xFFFF)	return;
	if (state->m_mcu_com[1] != 0xFFFF)	return;
	if (state->m_mcu_com[2] != 0xFFFF)	return;
	if (state->m_mcu_com[3] != 0xFFFF)	return;

	memset(state->m_mcu_com, 0, 4 * sizeof( UINT16 ) );
	galpani3_mcu_run(space->machine());
}

WRITE16_MEMBER(galpani3_state::galpani3_mcu_com0_w){ galpani3_mcu_com_w(&space, offset, data, mem_mask, 0); }
WRITE16_MEMBER(galpani3_state::galpani3_mcu_com1_w){ galpani3_mcu_com_w(&space, offset, data, mem_mask, 1); }
WRITE16_MEMBER(galpani3_state::galpani3_mcu_com2_w){ galpani3_mcu_com_w(&space, offset, data, mem_mask, 2); }
WRITE16_MEMBER(galpani3_state::galpani3_mcu_com3_w){ galpani3_mcu_com_w(&space, offset, data, mem_mask, 3); }

READ16_MEMBER(galpani3_state::galpani3_mcu_status_r)
{
	logerror("cpu '%s' (PC=%06X): read mcu status\n", space.device().tag(), cpu_get_previouspc(&space.device()));
	return 0;
}

// might be blitter regs? - there are 3, probably GRAP2 chips

READ16_MEMBER(galpani3_state::galpani3_regs1_r)
{

	switch (offset)
	{
		case 0x2:
			return m_framebuffer1_enable;

		case 0xb:
		{
			m_regs1_i^=1;
			if (m_regs1_i) return 0xfffe;
			else return 0xffff;
		}

		default:
			logerror("cpu '%s' (PC=%06X): galpani3_regs1_r %02x %04x\n", space.device().tag(), cpu_get_previouspc(&space.device()), offset, mem_mask);
			break;

	}

	return 0x0000;
}


READ16_MEMBER(galpani3_state::galpani3_regs2_r)
{

	switch (offset)
	{
		case 0x2:
			return m_framebuffer2_enable;

		case 0xb:
		{
			m_regs2_i^=1;
			if (m_regs2_i) return 0xfffe;
			else return 0xffff;
		}

		default:
			logerror("cpu '%s' (PC=%06X): galpani3_regs2_r %02x %04x\n", space.device().tag(), cpu_get_previouspc(&space.device()), offset, mem_mask);
			break;

	}

	return 0x0000;
}


READ16_MEMBER(galpani3_state::galpani3_regs3_r)
{

	switch (offset)
	{
		case 0x2:
			return m_framebuffer3_enable;

		case 0xb:
		{
			m_regs3_i^=1;
			if (m_regs3_i) return 0xfffe;
			else return 0xffff;
		}

		default:
			logerror("cpu '%s' (PC=%06X): galpani3_regs3_r %02x %04x\n", space.device().tag(), cpu_get_previouspc(&space.device()), offset, mem_mask);
			break;

	}

	return 0x0000;
}



static void gp3_do_rle(UINT32 address, UINT16*framebuffer, UINT8* rledata)
{
	int rle_count = 0;
	int normal_count = 0;
	UINT32 dstaddress = 0;

	UINT8 thebyte;

	while (dstaddress<0x40000)
	{
		if (rle_count==0 && normal_count==0) // we need a new code byte
		{
			thebyte = rledata[address];

			if ((thebyte & 0x80)) // stream of normal bytes follows
			{
				normal_count = (thebyte & 0x7f)+1;
				address++;
			}
			else // rle block
			{
				rle_count = (thebyte & 0x7f)+1;
				address++;
			}
		}
		else if (rle_count)
		{
			thebyte = rledata[address];
			framebuffer[dstaddress] = thebyte;
			dstaddress++;
			rle_count--;

			if (rle_count==0)
			{
				address++;
			}
		}
		else if (normal_count)
		{
			thebyte = rledata[address];
			framebuffer[dstaddress] = thebyte;
			dstaddress++;
			normal_count--;
			address++;

		}
	}

}

WRITE16_MEMBER(galpani3_state::galpani3_regs1_address_w)
{

	logerror("galpani3_regs1_address_w %04x\n",data);
	COMBINE_DATA(&m_regs1_address_regs[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_regs1_go_w)
{

	UINT32 address = m_regs1_address_regs[1]| (m_regs1_address_regs[0]<<16);
	UINT8* rledata = machine().region("gfx2")->base();

	printf("galpani3_regs1_go_w? %08x\n",address );
	if ((data==0x2000) || (data==0x3000)) gp3_do_rle(address, m_framebuffer1, rledata);
}


WRITE16_MEMBER(galpani3_state::galpani3_regs2_address_w)
{

	logerror("galpani3_regs2_address_w %04x\n",data);
	COMBINE_DATA(&m_regs2_address_regs[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_regs2_go_w)
{

	UINT32 address = m_regs2_address_regs[1]| (m_regs2_address_regs[0]<<16);
	UINT8* rledata = machine().region("gfx2")->base();

	printf("galpani3_regs2_go_w? %08x\n", address );

	// hack to prevent title screen being corrupt - these might actually be size registers
	// for the RLE request
	if ((data==0x2000) || (data==0x3000)) gp3_do_rle(address, m_framebuffer2, rledata);
}



WRITE16_MEMBER(galpani3_state::galpani3_regs3_address_w)
{

	logerror("galpani3_regs3_address_w %04x\n",data);
	COMBINE_DATA(&m_regs3_address_regs[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_regs3_go_w)
{

	UINT32 address =  m_regs3_address_regs[1]| (m_regs3_address_regs[0]<<16);
	UINT8* rledata = machine().region("gfx2")->base();

	printf("galpani3_regs3_go_w? %08x\n",address );

	if ((data==0x2000) || (data==0x3000)) gp3_do_rle(address, m_framebuffer3, rledata);
}

static void set_color_555_gp3(running_machine &machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer1_palette_w)
{

	COMBINE_DATA(&m_framebuffer1_palette[offset]);
	set_color_555_gp3(machine(), offset+0x4000, 5, 10, 0, m_framebuffer1_palette[offset]);
}


WRITE16_MEMBER(galpani3_state::galpani3_framebuffer2_palette_w)
{

	COMBINE_DATA(&m_framebuffer2_palette[offset]);
	set_color_555_gp3(machine(), offset+0x4100, 5, 10, 0, m_framebuffer2_palette[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer3_palette_w)
{

	COMBINE_DATA(&m_framebuffer3_palette[offset]);
	set_color_555_gp3(machine(), offset+0x4200, 5, 10, 0, m_framebuffer3_palette[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer3_scrolly_w)
{

	m_framebuffer3_scrolly = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer3_scrollx_w)
{

	m_framebuffer3_scrollx = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer2_scrolly_w)
{

	m_framebuffer2_scrolly = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer2_scrollx_w)
{

	m_framebuffer2_scrollx = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer1_scrolly_w)
{

	m_framebuffer1_scrolly = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer1_scrollx_w)
{

	m_framebuffer1_scrollx = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_priority_buffer_scrollx_w)
{

	m_priority_buffer_scrollx = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_priority_buffer_scrolly_w)
{

	m_priority_buffer_scrolly = data;
}

/* I'm not convinced these are enables */
WRITE16_MEMBER(galpani3_state::galpani3_framebuffer1_enable_w)
{

	m_framebuffer1_enable = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer2_enable_w)
{

	m_framebuffer2_enable = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer3_enable_w)
{

	m_framebuffer3_enable = data;
}

/* definitely looks like a cycling bg colour used for the girls */
WRITE16_MEMBER(galpani3_state::galpani3_framebuffer1_bgcol_w)
{

	COMBINE_DATA(&m_framebuffer1_bgcol[offset]);
	set_color_555_gp3(machine(), offset+0x4300, 5, 10, 0, m_framebuffer1_bgcol[offset]);
}

WRITE16_MEMBER(galpani3_state::galpani3_framebuffer2_bgcol_w)
{

	COMBINE_DATA(&m_framebuffer2_bgcol[offset]);
	set_color_555_gp3(machine(), offset+0x4301, 5, 10, 0, m_framebuffer2_bgcol[offset]);
}


WRITE16_MEMBER(galpani3_state::galpani3_framebuffer3_bgcol_w)
{

	COMBINE_DATA(&m_framebuffer3_bgcol[offset]);
	set_color_555_gp3(machine(), offset+0x4302, 5, 10, 0, m_framebuffer3_bgcol[offset]);
}



static ADDRESS_MAP_START( galpani3_map, AS_PROGRAM, 16, galpani3_state )
	AM_RANGE(0x000000, 0x17ffff) AM_ROM

	AM_RANGE(0x200000, 0x20ffff) AM_RAM // area [B] - Work RAM
	AM_RANGE(0x280000, 0x287fff) AM_RAM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w)   AM_SHARE("paletteram") // area [A] - palette for sprites

	AM_RANGE(0x300000, 0x303fff) AM_RAM_WRITE(galpani3_suprnova_sprite32_w) AM_SHARE("spriteram")
	AM_RANGE(0x380000, 0x38003f) AM_RAM_WRITE(galpani3_suprnova_sprite32regs_w) AM_BASE(m_sprregs)

	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_BASE(m_mcu_ram) // area [C]

	AM_RANGE(0x580000, 0x580001) AM_WRITE(galpani3_mcu_com0_w)	// ] see $387e8: these 2 locations are written (w.#$ffff)
	AM_RANGE(0x600000, 0x600001) AM_WRITE(galpani3_mcu_com1_w)	// ] then bit #0 of $780000.l is tested: 0 = OK!
	AM_RANGE(0x680000, 0x680001) AM_WRITE(galpani3_mcu_com2_w)	// ] see $387e8: these 2 locations are written (w.#$ffff)
	AM_RANGE(0x700000, 0x700001) AM_WRITE(galpani3_mcu_com3_w)	// ] then bit #0 of $780000.l is tested: 0 = OK!
	AM_RANGE(0x780000, 0x780001) AM_READ(galpani3_mcu_status_r)

	// GRAP2 1?
	AM_RANGE(0x800000, 0x8003ff) AM_RAM // ??? see subroutine $39f42 (R?)
	AM_RANGE(0x800400, 0x800401) AM_WRITE(galpani3_framebuffer1_scrollx_w) // scroll?
	AM_RANGE(0x800800, 0x800bff) AM_RAM // ??? see subroutine $39f42 (R?)
	AM_RANGE(0x800c00, 0x800c01) AM_WRITE(galpani3_framebuffer1_scrolly_w) // scroll?
	AM_RANGE(0x800c02, 0x800c03) AM_WRITE(galpani3_framebuffer1_enable_w) // enable?
	AM_RANGE(0x800c06, 0x800c07) AM_WRITE(galpani3_framebuffer1_bgcol_w) AM_BASE(m_framebuffer1_bgcol) // bg colour? cycles ingame, for girls?
	AM_RANGE(0x800c10, 0x800c11) AM_RAM AM_BASE(m_framebuffer1_bright1) // brightness / blend amount?
	AM_RANGE(0x800c12, 0x800c13) AM_RAM AM_BASE(m_framebuffer1_bright2) // similar.
	AM_RANGE(0x800c18, 0x800c1b) AM_WRITE(galpani3_regs1_address_w) // ROM address of RLE data, in bytes
	AM_RANGE(0x800c1e, 0x800c1f) AM_WRITE(galpani3_regs1_go_w) // ?
	AM_RANGE(0x800c00, 0x800c1f) AM_READ(galpani3_regs1_r)// ? R layer regs ? see subroutine $3a03e
	AM_RANGE(0x880000, 0x8801ff) AM_RAM_WRITE(galpani3_framebuffer1_palette_w) AM_BASE(m_framebuffer1_palette) // palette
	AM_RANGE(0x900000, 0x97ffff) AM_RAM AM_BASE(m_framebuffer1)// area [D] - R area ? odd bytes only, initialized 00..ff,00..ff,...

	// GRAP2 2?
	AM_RANGE(0xa00000, 0xa003ff) AM_RAM // ??? see subroutine $39f42 (G?)
	AM_RANGE(0xa00400, 0xa00401) AM_WRITE(galpani3_framebuffer2_scrollx_w)
	AM_RANGE(0xa00800, 0xa00bff) AM_RAM // ??? see subroutine $39f42 (G?)
	AM_RANGE(0xa00c00, 0xa00c01) AM_WRITE(galpani3_framebuffer2_scrolly_w)
	AM_RANGE(0xa00c02, 0xa00c03) AM_WRITE(galpani3_framebuffer2_enable_w) // enable?
	AM_RANGE(0xa00c06, 0xa00c07) AM_WRITE(galpani3_framebuffer2_bgcol_w) AM_BASE(m_framebuffer2_bgcol) // bg colour? same values as previous layer
	AM_RANGE(0xa00c10, 0xa00c11) AM_RAM AM_BASE(m_framebuffer2_bright1) // similar..
	AM_RANGE(0xa00c12, 0xa00c13) AM_RAM AM_BASE(m_framebuffer2_bright2) // brightness / blend amount?
	AM_RANGE(0xa00c00, 0xa00c1f) AM_READ(galpani3_regs2_r) // ? G layer regs ? see subroutine $3a03e
	AM_RANGE(0xa00c18, 0xa00c1b) AM_WRITE(galpani3_regs2_address_w) // ROM address of RLE data, in bytes
	AM_RANGE(0xa00c1e, 0xa00c1f) AM_WRITE(galpani3_regs2_go_w) // ?
	AM_RANGE(0xa80000, 0xa801ff) AM_RAM_WRITE(galpani3_framebuffer2_palette_w) AM_BASE(m_framebuffer2_palette) // palette
	AM_RANGE(0xb00000, 0xb7ffff) AM_RAM AM_BASE(m_framebuffer2) // area [E] - G area ? odd bytes only, initialized 00..ff,00..ff,...

	// GRAP2 3?
	AM_RANGE(0xc00000, 0xc003ff) AM_RAM // row scroll??
	AM_RANGE(0xc00400, 0xc00401) AM_WRITE(galpani3_framebuffer3_scrollx_w) // scroll?
	AM_RANGE(0xc00800, 0xc00bff) AM_RAM // column scroll??
	AM_RANGE(0xc00c00, 0xc00c01) AM_WRITE(galpani3_framebuffer3_scrolly_w) // scroll?
	AM_RANGE(0xc00c02, 0xc00c03) AM_WRITE(galpani3_framebuffer3_enable_w) // enable?
	AM_RANGE(0xc00c06, 0xc00c07) AM_WRITE(galpani3_framebuffer3_bgcol_w) AM_BASE(m_framebuffer3_bgcol) // bg colour? not used?
	AM_RANGE(0xc00c10, 0xc00c11) AM_RAM AM_BASE(m_framebuffer3_bright1) // brightness / blend amount?
	AM_RANGE(0xc00c12, 0xc00c13) AM_RAM AM_BASE(m_framebuffer3_bright2) // similar..
	AM_RANGE(0xc00c18, 0xc00c1b) AM_WRITE(galpani3_regs3_address_w) // ROM address of RLE data, in bytes
	AM_RANGE(0xc00c1e, 0xc00c1f) AM_WRITE(galpani3_regs3_go_w) // ?
	AM_RANGE(0xc00c00, 0xc00c1f) AM_READ(galpani3_regs3_r) // ? B layer regs ? see subroutine $3a03e
	AM_RANGE(0xc80000, 0xc801ff) AM_RAM_WRITE(galpani3_framebuffer3_palette_w) AM_BASE(m_framebuffer3_palette) // palette
	AM_RANGE(0xd00000, 0xd7ffff) AM_RAM AM_BASE(m_framebuffer3) // area [F] - B area ? odd bytes only, initialized 00..ff,00..ff,...

	// ?? priority / alpha buffer?
	AM_RANGE(0xe00000, 0xe7ffff) AM_RAM AM_BASE(m_priority_buffer) // area [J] - A area ? odd bytes only, initialized 00..ff,00..ff,..., then cleared
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(galpani3_priority_buffer_scrollx_w) // scroll?
	AM_RANGE(0xe80002, 0xe80003) AM_WRITE(galpani3_priority_buffer_scrolly_w) // scroll?


	AM_RANGE(0xf00000, 0xf00001) AM_NOP // ? written once (2nd opcode, $1.b)
	AM_RANGE(0xf00010, 0xf00011) AM_READ_PORT("P1")
	AM_RANGE(0xf00012, 0xf00013) AM_READ_PORT("P2")
	AM_RANGE(0xf00014, 0xf00015) AM_READ_PORT("COIN")
	AM_RANGE(0xf00016, 0xf00017) AM_NOP // ? read, but overwritten
	AM_RANGE(0xf00020, 0xf00023) AM_DEVWRITE8_LEGACY("ymz", ymz280b_w, 0x00ff)	// sound
	AM_RANGE(0xf00040, 0xf00041) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)	// watchdog
	AM_RANGE(0xf00050, 0xf00051) AM_NOP // ? written once (3rd opcode, $30.b)
ADDRESS_MAP_END


static const ymz280b_interface ymz280b_intf =
{
	0	// irq ?
};

static MACHINE_CONFIG_START( galpani3, galpani3_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_28_63636MHz/2)	// Confirmed from PCB
	MCFG_CPU_PROGRAM_MAP(galpani3_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", galpani3_vblank, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	//MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 64*8-1)
	MCFG_SCREEN_UPDATE_STATIC(galpani3)

	MCFG_PALETTE_LENGTH(0x4303)

	MCFG_VIDEO_START(galpani3)

	MCFG_DEVICE_ADD("spritegen", SKNS_SPRITE, 0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_33_333MHz / 2)	// Confirmed from PCB
	MCFG_SOUND_CONFIG(ymz280b_intf)
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( galpani3 ) /* All game text in English */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0e0.u71",  0x000000, 0x080000, CRC(fa681118) SHA1(982b568a77ed620ba5708fec4c186d329d48cb48) )
	ROM_LOAD16_BYTE( "g3p1j1.u102", 0x000001, 0x080000, CRC(f1150f1b) SHA1(a6fb719937927a9a39c7a4888017c63c47c2dd6c) ) /* Is it really G3P1J1 like below or G3P1J0?? */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )		// 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )		// 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )		// 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )		// 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0j0.101", 0xe00000, 0x040000, CRC(fbb1e0dc) SHA1(14f6377afd93054aa5dc38af235ae12b932e847f) )	// 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1j0.100", 0xe00001, 0x040000, CRC(18edb5f0) SHA1(5e2ed0105b3e6037f6116494d3b186a368824171) )	//

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END

ROM_START( galpani3j ) /* Some game text in Japanese, but no "For use in Japan" type region notice */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0j1.71",  0x000000, 0x080000, CRC(52893326) SHA1(78fdbf3436a4ba754d7608fedbbede5c719a4505) )
	ROM_LOAD16_BYTE( "g3p1j1.102", 0x000001, 0x080000, CRC(05f935b4) SHA1(81e78875585bcdadad1c302614b2708e60563662) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )		// 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )		// 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )		// 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )		// 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0j0.101", 0xe00000, 0x040000, CRC(fbb1e0dc) SHA1(14f6377afd93054aa5dc38af235ae12b932e847f) )	// 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1j0.100", 0xe00001, 0x040000, CRC(18edb5f0) SHA1(5e2ed0105b3e6037f6116494d3b186a368824171) )	//

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END


static DRIVER_INIT( galpani3 )
{
	galpani3_state *state = machine.driver_data<galpani3_state>();
	DRIVER_INIT_CALL( decrypt_toybox_rom );

	memset(state->m_mcu_com, 0, 4 * sizeof( UINT16) );
}

GAME( 1995, galpani3,  0,        galpani3, galpani3, galpani3, ROT90, "Kaneko", "Gals Panic 3 (Euro)", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, galpani3j, galpani3, galpani3, galpani3, galpani3, ROT90, "Kaneko", "Gals Panic 3 (Japan)", GAME_IMPERFECT_GRAPHICS )
