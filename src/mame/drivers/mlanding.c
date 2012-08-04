/***********************************************************************************************************

Midnight Landing (c) 1987 Taito Corporation

driver by Tomasz Slanina, Phil Bennett & Angelo Salese, based on early work by David Haywood

Dual 68k + 2xZ80 + tms DSP
no other hardware info..but it doesn't seem related to taitoair.c at all

TODO:
- Sound is nowhere near to be perfect, mode 5 causes "sound cpu error" for whatever reason;
- Palette banking;
- Comms between the five CPUs;
- Gameplay looks stiff;
- Needs a custom artwork for the cloche status;
- Current dump is weird, a mix between German, English and Japanese?
- clean-ups!

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32025/tms32025.h"
#include "cpu/z80/z80.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"


class mlanding_state : public driver_device
{
public:
	mlanding_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_g_ram(*this, "g_ram"),
		m_ml_tileram(*this, "ml_tileram"),
		m_dma_ram(*this, "dma_ram"),
		m_ml_dotram(*this, "ml_dotram"),
		m_mecha_ram(*this, "mecha_ram"){ }

	required_shared_ptr<UINT16> m_g_ram;
	required_shared_ptr<UINT16> m_ml_tileram;
	required_shared_ptr<UINT16> m_dma_ram;
	required_shared_ptr<UINT16> m_ml_dotram;
	required_shared_ptr<UINT8> m_mecha_ram;
	UINT32 m_adpcm_pos;
	UINT32 m_adpcm_end;
	int m_adpcm_data;
	UINT8 m_adpcm_idle;
	UINT8 m_pal_fg_bank;
	int m_dma_active;
	UINT16 m_dsp_HOLD_signal;
	UINT8 m_trigger;
	DECLARE_WRITE16_MEMBER(ml_tileram_w);
	DECLARE_READ16_MEMBER(ml_tileram_r);
	DECLARE_READ16_MEMBER(io1_r);
	DECLARE_WRITE16_MEMBER(ml_output_w);
	DECLARE_WRITE16_MEMBER(ml_sub_reset_w);
	DECLARE_WRITE16_MEMBER(ml_to_sound_w);
	DECLARE_WRITE8_MEMBER(ml_sound_to_main_w);
	DECLARE_READ16_MEMBER(ml_analog1_lsb_r);
	DECLARE_READ16_MEMBER(ml_analog2_lsb_r);
	DECLARE_READ16_MEMBER(ml_analog3_lsb_r);
	DECLARE_READ16_MEMBER(ml_analog1_msb_r);
	DECLARE_READ16_MEMBER(ml_analog2_msb_r);
	DECLARE_READ16_MEMBER(ml_analog3_msb_r);
	DECLARE_WRITE16_MEMBER(ml_nmi_to_sound_w);
	DECLARE_READ16_MEMBER(ml_mecha_ram_r);
	DECLARE_WRITE16_MEMBER(ml_mecha_ram_w);
	DECLARE_WRITE8_MEMBER(ml_msm_start_msb_w);
	DECLARE_READ16_MEMBER(ml_dotram_r);
	DECLARE_WRITE16_MEMBER(ml_dotram_w);
	DECLARE_READ16_MEMBER(dsp_HOLD_signal_r);
	DECLARE_READ8_MEMBER(test_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(ml_msm_start_lsb_w);
};



static VIDEO_START(mlanding)
{
}

// 000: ???????
// 256: Cockpit
// 512: control centre screen
// 768: plane landing sequence
static SCREEN_UPDATE_IND16(mlanding)
{
	mlanding_state *state = screen.machine().driver_data<mlanding_state>();
	int x, y;

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT16 *src = &state->m_g_ram[y * 512/2 + cliprect.min_x];
		UINT16 *dst = &bitmap.pix16(y, cliprect.min_x);

		for (x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			UINT16 srcpix = *src++;

			*dst++ = screen.machine().pens[256+(srcpix & 0xff) + (state->m_pal_fg_bank & 1 ? 0x100 : 0x000)];
			*dst++ = screen.machine().pens[256+(srcpix >> 8) + (state->m_pal_fg_bank & 1 ? 0x100 : 0x000)];
		}
	}

	return 0;
}

/* Return the number of pixels processed for timing purposes? */
static int start_dma(running_machine &machine)
{
	mlanding_state *state = machine.driver_data<mlanding_state>();
	/* Traverse the DMA RAM list */
	int offs;

	for (offs = 0; offs < 0x2000; offs +=4)
	{
		UINT16 code;
		UINT16 x;
		UINT16 y;
		UINT16 colour;
		UINT16 dx;
		UINT16 dy;

		int j, k;

		UINT16 attr = state->m_dma_ram[offs];

		if (attr == 0)
			continue;

		x = state->m_dma_ram[offs + 1];
		y = state->m_dma_ram[offs + 2];
		colour = state->m_dma_ram[offs + 3];

		dx = x >> 11;
		dy = y >> 11;
		dx &= 0x1f;
		dy &= 0x1f;
		dx++;
		dy++;

		x &= 0x1ff;
		y &= 0x1ff;

#if 0
		printf("OFFS: %.8x\n", offs*2);
		printf("CODE: %.4x %s\n", attr, ~attr & 0x8000 ? "TRANS" : "    ");
		printf("X   : %.3d\n", x);
		printf("Y   : %.3d\n", y);
		printf("DX  : %.3d (%d)\n", dx, dx*8);
		printf("DY  : %.3d (%d)\n", dy, dx*8);
		printf("COL : %.4x\n", colour);
#endif

		code = attr & 0x1fff;

		if (code)
		{
			for(j = 0; j < dx; j++)
			{
				for(k = 0; k < dy; k++)
				{
					int x1, y1;

					// Draw the 8x8 chunk
					for (y1 = 0; y1 < 8; ++y1)
					{
						UINT16 *src = &state->m_ml_tileram[(code * 2 * 8) + y1*2];
						UINT16 *dst = &state->m_g_ram[(y + k*8+y1)*512/2 + (j*8+x)/2];

						UINT8 p2 = *src & 0xff;
						UINT8 p1 = *src++ >> 8;
						UINT8 p4 = *src;
						UINT8 p3 = *src++ >> 8;

						// DRAW 8 pixels
						for (x1 = 0; x1 < 8; x1++)
						{
							UINT16 pix1, pix2;

							pix1 = (BIT(p4, x1) << 3) | (BIT(p3, x1) << 2) | (BIT(p2, x1) << 1) | BIT(p1, x1);
							x1++;
							pix2 = (BIT(p4, x1) << 3) | (BIT(p3, x1) << 2) | (BIT(p2, x1) << 1) | BIT(p1, x1);

							if (~attr & 0x8000)
							{
								if (pix1)
								{
									*dst = (*dst & 0xff00) | ((colour << 4) | pix1);
								}
								if (pix2)
								{
									*dst = (*dst & 0x00ff) | (((colour << 4) | pix2) << 8);
								}
							}
							else
							{
								*dst = (((colour << 4) | pix2) << 8) | ((colour << 4) | pix1);
							}

							dst++;
						}
					}
					code++;
				}
			}
		}
		else
		{
			int y1;
			UINT16 clear_colour = (colour << 12) | (colour << 4);

			for(y1 = 0; y1 < dy*8; y1++)
			{
				int x1;
				UINT16 *dst = &state->m_g_ram[((y + y1) * 512/2) + x/2];

				for(x1 = 0; x1 < dx*8; x1+=2)
				{
					*dst++ = clear_colour;
				}
			}
		}
	}
	return 1;
}

WRITE16_MEMBER(mlanding_state::ml_tileram_w)
{
	COMBINE_DATA(&m_ml_tileram[offset]);
}

READ16_MEMBER(mlanding_state::ml_tileram_r)
{
	return m_ml_tileram[offset];
}


READ16_MEMBER(mlanding_state::io1_r)//240006
{
	/*
    fedcba9876543210
                   x  - mecha driver status
                  x   - ???
                 x    - test 2
                x     - ???
    x                 - video status
        other bits = language(german, japan, english), video test
    */
// multiplexed? or just overriden?

	int retval = (m_dma_active << 15) | (ioport("DSW")->read() & 0x7fff);
	return retval;
}

/* output */
WRITE16_MEMBER(mlanding_state::ml_output_w)
{
	/*
    x--- ---- palette fg bankswitch
    ---x ---- coin lockout?
    ---- x--- coin counter B
    ---- -x-- coin counter A
    */
//  popmessage("%04x",data);

	m_pal_fg_bank = (data & 0x80)>>7;
}

WRITE8_MEMBER(mlanding_state::sound_bankswitch_w)
{
	data=0;
	machine().root_device().membank("bank1")->set_base(machine().root_device().memregion("audiocpu")->base() + ((data) & 0x03) * 0x4000 + 0x10000 );
}

static void ml_msm5205_vck(device_t *device)
{
	mlanding_state *state = device->machine().driver_data<mlanding_state>();

//  popmessage("%08x",state->m_adpcm_pos);

	if (state->m_adpcm_pos >= 0x50000  || state->m_adpcm_idle)
	{
		//state->m_adpcm_idle = 1;
		msm5205_reset_w(device,1);
		state->m_trigger = 0;
	}
	else
	{
		UINT8 *ROM = device->machine().root_device().memregion("adpcm")->base();

		state->m_adpcm_data = ((state->m_trigger ? (ROM[state->m_adpcm_pos] & 0x0f) : (ROM[state->m_adpcm_pos] & 0xf0)>>4) );
		msm5205_data_w(device,state->m_adpcm_data & 0xf);
		state->m_trigger^=1;
		if(state->m_trigger == 0)
		{
			state->m_adpcm_pos++;
			//cputag_set_input_line(device->machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
			/*TODO: simplify this */
			if(ROM[state->m_adpcm_pos] == 0x00 && ROM[state->m_adpcm_pos+1] == 0x00 && ROM[state->m_adpcm_pos+2] == 0x00 && ROM[state->m_adpcm_pos+3] == 0x00
		       && ROM[state->m_adpcm_pos+4] == 0x00 && ROM[state->m_adpcm_pos+5] == 0x00 && ROM[state->m_adpcm_pos+6] == 0x00 && ROM[state->m_adpcm_pos+7] == 0x00
		       && ROM[state->m_adpcm_pos+8] == 0x00 && ROM[state->m_adpcm_pos+9] == 0x00 && ROM[state->m_adpcm_pos+10] == 0x00 && ROM[state->m_adpcm_pos+11] == 0x00
		       && ROM[state->m_adpcm_pos+12] == 0x00 && ROM[state->m_adpcm_pos+13] == 0x00 && ROM[state->m_adpcm_pos+14] == 0x00 && ROM[state->m_adpcm_pos+15] == 0x00)
				state->m_adpcm_idle = 1;
		}
	}
}

static TIMER_CALLBACK( dma_complete )
{
	mlanding_state *state = machine.driver_data<mlanding_state>();
	state->m_dma_active = 0;
}

/* TODO: this uses many bits */
WRITE16_MEMBER(mlanding_state::ml_sub_reset_w)
{
	int pixels;

	// Return the number of pixels drawn?
	pixels = start_dma(machine());

	if (pixels)
	{
		m_dma_active = 1;
		machine().scheduler().timer_set(attotime::from_msec(20), FUNC(dma_complete));
	}

	if(!(data & 0x40)) // unknown line used
		cputag_set_input_line(machine(), "sub", INPUT_LINE_RESET, CLEAR_LINE);

	//data & 0x20 sound cpu?

	if(!(data & 0x80)) // unknown line used
	{
		cputag_set_input_line(machine(), "dsp", INPUT_LINE_RESET, CLEAR_LINE);
		m_dsp_HOLD_signal = data & 0x80;
	}
}

WRITE16_MEMBER(mlanding_state::ml_to_sound_w)
{
	device_t *tc0140syt = machine().device("tc0140syt");
	if (offset == 0)
		tc0140syt_port_w(tc0140syt, 0, data & 0xff);
	else if (offset == 1)
	{
		//cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
		tc0140syt_comm_w(tc0140syt, 0, data & 0xff);
	}
}

WRITE8_MEMBER(mlanding_state::ml_sound_to_main_w)
{
	device_t *tc0140syt = machine().device("tc0140syt");
	if (offset == 0)
		tc0140syt_slave_port_w(tc0140syt, 0, data & 0xff);
	else if (offset == 1)
	{
		//cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
		tc0140syt_slave_comm_w(tc0140syt, 0, data & 0xff);
	}
}

READ16_MEMBER(mlanding_state::ml_analog1_lsb_r)
{
	return ioport("STICKX")->read() & 0xff;
}

READ16_MEMBER(mlanding_state::ml_analog2_lsb_r)
{
	return ioport("STICKY")->read() & 0xff;
}

READ16_MEMBER(mlanding_state::ml_analog3_lsb_r)
{
	return (ioport("STICKZ")->read() & 0xff);
}

/*
    PORT_START("IN3")
    PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) //high bits of counter 3
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_TOGGLE
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slot Down") PORT_TOGGLE
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slot Up") PORT_TOGGLE
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START("IN4")
    PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) //high bits of counter 2
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_TOGGLE
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_TOGGLE
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_TOGGLE
    PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
*/

/* high bits of analog inputs + "limiters"/ADC converters. */
READ16_MEMBER(mlanding_state::ml_analog1_msb_r)
{
	return ((ioport("STICKY")->read() & 0xf00)>>8) | (ioport("IN2")->read() & 0xf0);
}

READ16_MEMBER(mlanding_state::ml_analog2_msb_r)
{
	UINT8 res;
	UINT16 y_adc,x_adc;

	y_adc = ioport("STICKY")->read();
	x_adc = ioport("STICKZ")->read();

	res = 0;

	if(x_adc == 0 || (!(x_adc & 0x800)))
		res = 0x20;

	if(y_adc == 0)
		res|= 0x50;
	else if(y_adc & 0x800)
		res|= 0x10;
	else
		res|= 0x40;

//  popmessage("%04x %04x",x_adc,y_adc);

	return ((ioport("STICKZ")->read() & 0xf00)>>8) | res;
}

READ16_MEMBER(mlanding_state::ml_analog3_msb_r)
{
	UINT8 z_adc,res;
	UINT16 x_adc;

	z_adc = ioport("STICKX")->read();
	x_adc = ioport("STICKZ")->read();

	res = 0;

	if(z_adc == 0)
		res = 0x60;
	else if(z_adc & 0x80)
		res = 0x20;
	else
		res = 0x40;

	if(x_adc & 0x800 || x_adc == 0)
		res|= 0x10;

	return ((ioport("STICKX")->read() & 0xf00)>>8) | res;
}

WRITE16_MEMBER(mlanding_state::ml_nmi_to_sound_w)
{
//  cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
}

READ16_MEMBER(mlanding_state::ml_mecha_ram_r)
{
	return (m_mecha_ram[offset*2]<<8)|m_mecha_ram[offset*2+1];
}

WRITE16_MEMBER(mlanding_state::ml_mecha_ram_w)
{
	COMBINE_DATA(m_mecha_ram+offset*2+1);
	data >>= 8;
	mem_mask >>= 8;
	COMBINE_DATA(m_mecha_ram+offset*2);
}

static ADDRESS_MAP_START( mlanding_mem, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM

	AM_RANGE(0x100000, 0x17ffff) AM_RAM AM_SHARE("g_ram")// 512kB G RAM - enough here for double buffered 512x400x8 frame
	AM_RANGE(0x180000, 0x1bffff) AM_READWRITE(ml_tileram_r, ml_tileram_w) AM_SHARE("ml_tileram")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM AM_SHARE("dma_ram")
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE("share1")

	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(ml_sub_reset_w)
	AM_RANGE(0x1d0002, 0x1d0003) AM_WRITE(ml_nmi_to_sound_w) //sound reset ??

	AM_RANGE(0x2d0000, 0x2d0003) AM_WRITE(ml_to_sound_w)
	AM_RANGE(0x2d0000, 0x2d0001) AM_READNOP
	AM_RANGE(0x2d0002, 0x2d0003) AM_DEVREAD8_LEGACY("tc0140syt", tc0140syt_comm_r, 0x00ff)

	AM_RANGE(0x200000, 0x20ffff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x280000, 0x2807ff) AM_READWRITE(ml_mecha_ram_r,ml_mecha_ram_w)

	AM_RANGE(0x290000, 0x290001) AM_READ_PORT("IN1")
	AM_RANGE(0x290002, 0x290003) AM_READ_PORT("IN0")

	AM_RANGE(0x240004, 0x240005) AM_NOP //watchdog ??
	AM_RANGE(0x240006, 0x240007) AM_READ(io1_r) // vblank ?
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(ml_output_w)

	/*  */
	AM_RANGE(0x2b0000, 0x2b0001) AM_READ(ml_analog1_lsb_r)		//-40 .. 40 analog controls ?
	AM_RANGE(0x2b0004, 0x2b0005) AM_READ(ml_analog2_lsb_r)		//-40 .. 40 analog controls ?
	AM_RANGE(0x2b0006, 0x2b0007) AM_READ(ml_analog1_msb_r) // tested in service mode, dips?
	AM_RANGE(0x2c0000, 0x2c0001) AM_READ(ml_analog3_lsb_r)		//-60 .. 60 analog controls ?
	AM_RANGE(0x2c0002, 0x2c0003) AM_READ(ml_analog2_msb_r)
	AM_RANGE(0x2b0002, 0x2b0003) AM_READ(ml_analog3_msb_r)		// IN2/IN3 could be switched
ADDRESS_MAP_END


/* Sub CPU Map */

static ADDRESS_MAP_START( mlanding_sub_mem, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x050000, 0x0503ff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM AM_SHARE("dma_ram")
	AM_RANGE(0x1c4000, 0x1cffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("ml_dotram")
ADDRESS_MAP_END

WRITE8_MEMBER(mlanding_state::ml_msm_start_lsb_w)
{
	device_t *device = machine().device("msm");
	m_adpcm_pos = (m_adpcm_pos & 0x0f0000) | ((data & 0xff)<<8) | 0x20;
	m_adpcm_idle = 0;
	msm5205_reset_w(device,0);
	m_adpcm_end = (m_adpcm_pos+0x800);
}

WRITE8_MEMBER(mlanding_state::ml_msm_start_msb_w)
{
	m_adpcm_pos = (m_adpcm_pos & 0x00ff00) | ((data & 0x0f)<<16) | 0x20;
}

static ADDRESS_MAP_START( mlanding_z80_mem, AS_PROGRAM, 8, mlanding_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_MIRROR(0x00fe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xa000, 0xa001) AM_WRITE(ml_sound_to_main_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREAD_LEGACY("tc0140syt", tc0140syt_slave_comm_r)

//  AM_RANGE(0xb000, 0xb000) AM_WRITE_LEGACY(ml_msm5205_address_w) //guess
//  AM_RANGE(0xc000, 0xc000) AM_DEVWRITE_LEGACY("msm", ml_msm5205_start_w)
//  AM_RANGE(0xd000, 0xd000) AM_DEVWRITE_LEGACY("msm", ml_msm5205_stop_w)

	AM_RANGE(0xf000, 0xf000) AM_WRITE(ml_msm_start_lsb_w)
	AM_RANGE(0xf200, 0xf200) AM_WRITE(ml_msm_start_msb_w)
ADDRESS_MAP_END

READ16_MEMBER(mlanding_state::ml_dotram_r)
{
	return m_ml_dotram[offset];
}

WRITE16_MEMBER(mlanding_state::ml_dotram_w)
{
	m_ml_dotram[offset] = data;
}

READ16_MEMBER(mlanding_state::dsp_HOLD_signal_r)
{
	return m_dsp_HOLD_signal;
}


READ8_MEMBER(mlanding_state::test_r)
{
	return machine().rand();
}

//mecha driver ?
static ADDRESS_MAP_START( mlanding_z80_sub_mem, AS_PROGRAM, 8, mlanding_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("mecha_ram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM

	AM_RANGE(0x9000, 0x9001) AM_READ(test_r)
	AM_RANGE(0x9800, 0x9803) AM_READ(test_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( DSP_map_program, AS_PROGRAM, 16, mlanding_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("share3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( DSP_map_data, AS_DATA, 16, mlanding_state )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(ml_dotram_r,ml_dotram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( DSP_map_io, AS_IO, 16, mlanding_state )
	AM_RANGE(TMS32025_HOLD, TMS32025_HOLD) AM_READ(dsp_HOLD_signal_r)
//  AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITE_LEGACY(dsp_HOLDA_signal_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( mlanding )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))
	PORT_DIPSETTING(	0x01, DEF_STR( Standard ))
	PORT_DIPSETTING(	0x00, "Deluxe" ) //with Mecha driver
	PORT_DIPNAME( 0x02, 0x02, "$2000-1")
	PORT_DIPSETTING(	0x02, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x04, 0x04, "Test Mode")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "$2000-3")
	PORT_DIPSETTING(	0x08, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x10, 0x10, "$2000-4")
	PORT_DIPSETTING(	0x10, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x20, 0x20, "$2000-5")
	PORT_DIPSETTING(	0x20, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x40, 0x40, "$2000-6")
	PORT_DIPSETTING(	0x40, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x80, 0x80, "$2000-7")
	PORT_DIPSETTING(	0x80, "H" )
	PORT_DIPSETTING(	0x00, "L" )
	PORT_DIPNAME( 0x0100, 0x0100, "$2001-0")
	PORT_DIPSETTING(	0x0100, "H" )
	PORT_DIPSETTING(	0x0000, "L" )
	PORT_DIPNAME( 0x0200, 0x0200, "$2001-1")
	PORT_DIPSETTING(	0x0200, "H" )
	PORT_DIPSETTING(	0x0000, "L" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(	0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "$2001-3")
	PORT_DIPSETTING(	0x0800, "H" )
	PORT_DIPSETTING(	0x0000, "L" )
	PORT_DIPNAME( 0x1000, 0x1000, "Test Mode 2")
	PORT_DIPSETTING(	0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "$2001-5")
	PORT_DIPSETTING(	0x2000, "H" )
	PORT_DIPSETTING(	0x0000, "L" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) )
	PORT_DIPSETTING(	0x4000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(	0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x8000, 0x8000, "$2001-7")
	PORT_DIPSETTING(	0x8000, "H" )
	PORT_DIPSETTING(	0x0000, "L" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Door") PORT_TOGGLE
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

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
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

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) //high bits of counter 1
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

	PORT_START("IN3")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) //high bits of counter 3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slot Down") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slot Up") PORT_TOGGLE
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) //high bits of counter 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_TOGGLE
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STICKX")	/* Stick 1 (3) */
	PORT_BIT( 0x00ff, 0x0000, IPT_AD_STICK_Z ) PORT_MINMAX(0x0080,0x007f) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("STICKY")	/* Stick 2 (4) */
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x0800,0x07ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("STICKZ")	/* Stick 3 (5) */
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0x0800,0x07ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)
INPUT_PORTS_END

static void irq_handler(device_t *device, int irq)
{
	cputag_set_input_line(device->machine(), "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const msm5205_interface msm5205_config =
{
	ml_msm5205_vck,	/* VCK function */
	MSM5205_S48_4B		/* 8 kHz */
};

static const ym2151_interface ym2151_config =
{
	DEVCB_LINE(irq_handler),
	DEVCB_DRIVER_MEMBER(mlanding_state,sound_bankswitch_w)
};

static const tc0140syt_interface mlanding_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};

static MACHINE_RESET( mlanding )
{
	mlanding_state *state = machine.driver_data<mlanding_state>();
	cputag_set_input_line(machine, "sub", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);
	state->m_adpcm_pos = 0;
	state->m_adpcm_data = -1;
	state->m_adpcm_idle = 1;
	state->m_dsp_HOLD_signal = 0;
}

static MACHINE_CONFIG_START( mlanding, mlanding_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000 )		/* 12 MHz ??? (guess) */
	MCFG_CPU_PROGRAM_MAP(mlanding_mem)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("sub", M68000, 12000000 )		/* 12 MHz ??? (guess) */
	MCFG_CPU_PROGRAM_MAP(mlanding_sub_mem)
	MCFG_CPU_PERIODIC_INT(irq6_line_hold,7*60) /* ??? */

	MCFG_CPU_ADD("audiocpu", Z80, 4000000 )		/* 4 MHz ??? (guess) */
	MCFG_CPU_PROGRAM_MAP(mlanding_z80_mem)

	MCFG_CPU_ADD("z80sub", Z80, 4000000 )		/* 4 MHz ??? (guess) */
	MCFG_CPU_PROGRAM_MAP(mlanding_z80_sub_mem)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("dsp", TMS32025,12000000)			/* 12 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(DSP_map_program)
	MCFG_CPU_DATA_MAP(DSP_map_data)
	MCFG_CPU_IO_MAP(DSP_map_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 14*8, 511)
	MCFG_SCREEN_UPDATE_STATIC(mlanding)

	MCFG_PALETTE_LENGTH(512*16)

	MCFG_VIDEO_START(mlanding)

	MCFG_MACHINE_RESET(mlanding)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, 4000000)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)

	MCFG_TC0140SYT_ADD("tc0140syt", mlanding_tc0140syt_intf)
MACHINE_CONFIG_END

ROM_START( mlanding )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 */
	ROM_LOAD16_BYTE( "ml_b0929.epr", 0x00000, 0x10000, CRC(ab3f38f3) SHA1(4357112ca11a8e7bfe08ba99ac3bddac046c230a))
	ROM_LOAD16_BYTE( "ml_b0928.epr", 0x00001, 0x10000, CRC(21e7a8f6) SHA1(860d3861d4375866cd27d426d546ddb2894a6629) )
	ROM_LOAD16_BYTE( "ml_b0927.epr", 0x20000, 0x10000, CRC(b02f1805) SHA1(b8050f955c7070dc9b962db329b5b0ee8b2acb70) )
	ROM_LOAD16_BYTE( "ml_b0926.epr", 0x20001, 0x10000, CRC(d57ff428) SHA1(8ff1ab666b06fb873f1ba9b25edf4cd49b9861a1) )
	ROM_LOAD16_BYTE( "ml_b0925.epr", 0x40000, 0x10000, CRC(ff59f049) SHA1(aba490a28aba03728415f34d321fd599c31a5fde) )
	ROM_LOAD16_BYTE( "ml_b0924.epr", 0x40001, 0x10000, CRC(9bc3e1b0) SHA1(6d86804327df11a513a0f06dceb57b83b34ac007) )

	ROM_REGION( 0x20000, "audiocpu", 0 )	/* z80 */
	ROM_LOAD( "ml_b0935.epr", 0x00000, 0x4000, CRC(b85915c5) SHA1(656e97035ae304f84e90758d0dd6f0616c40f1db) )
	ROM_CONTINUE(             0x10000, 0x04000 )	/* banked stuff */
	ROM_LOAD( "ml_b0936.epr", 0x14000, 0x02000, CRC(51fd3a77) SHA1(1fcbadf1877e25848a1d1017322751560a4823c0) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "sub", 0 )	/* 68000 */
	ROM_LOAD16_BYTE( "ml_b0923.epr", 0x00000, 0x10000, CRC(81b2c871) SHA1(a085bc528c63834079469db6ae263a5b9b984a7c) )
	ROM_LOAD16_BYTE( "ml_b0922.epr", 0x00001, 0x10000, CRC(36923b42) SHA1(c31d7c45a563cfc4533379f69f32889c79562534) )

	ROM_REGION( 0x10000, "z80sub", 0 )	/* z80 */
	ROM_LOAD( "ml_b0937.epr", 0x00000, 0x08000, CRC(4bdf15ed) SHA1(b960208e63cede116925e064279a6cf107aef81c) )

	ROM_REGION( 0x80000, "adpcm", ROMREGION_ERASEFF )
	ROM_LOAD( "ml_b0930.epr", 0x40000, 0x10000, CRC(214a30e2) SHA1(3dcc3a89ed52e4dbf232d2a92a3e64975b46c2dd) )
	ROM_LOAD( "ml_b0931.epr", 0x30000, 0x10000, CRC(9c4a82bf) SHA1(daeac620c636013a36595ce9f37e84e807f88977) )
	ROM_LOAD( "ml_b0932.epr", 0x20000, 0x10000, CRC(4721dc59) SHA1(faad75d577344e9ba495059040a2cf0647567426) )
	ROM_LOAD( "ml_b0933.epr", 0x10000, 0x10000, CRC(f5cac954) SHA1(71abdc545e0196ad4d357af22dd6312d10a1323f) )
	ROM_LOAD( "ml_b0934.epr", 0x00000, 0x10000, CRC(0899666f) SHA1(032e3ddd4caa48f82592570616e16c084de91f3e) )
ROM_END

static DRIVER_INIT(mlanding)
{
//  UINT8 *rom = machine.root_device().memregion("sub")->base();
//  rom[0x88b]=0x4e;
//  rom[0x88a]=0x71;
}

GAME( 1987, mlanding, 0,        mlanding,   mlanding, mlanding_state, mlanding,        ROT0,    "Taito America Corporation", "Midnight Landing (Germany)", GAME_NOT_WORKING|GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
