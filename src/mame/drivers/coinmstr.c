/*
x
  Coinmaster trivia games

  preliminary driver by Pierpaolo Prazzoli

  TODO:
  - is there extra colour info in attr3 for suprnudge 2 and pokeroul?
  - is there colour intensity control?
  - finish inputs
  - finish question roms reading
  - hook up all the PIAs

  Notes:
  - Some trivia seems to accept 2 type of eproms for question roms:
    0x4000 or 0x8000 bytes long. This check is done with the 1st read
    from the rom (I think from offset 0) and if it's 0x10, it means a
    0x4000 bytes eprom or if it's 0x20, it means a 0x8000 one.
    Also supnudg2 only tests 0x20 as 1st byte, so accepting only
    the 2nd type of eproms.

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"


class coinmstr_state : public driver_device
{
public:
	coinmstr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_attr_ram1;
	UINT8 *m_attr_ram2;
	UINT8 *m_attr_ram3;
	tilemap_t *m_bg_tilemap;
	UINT8 m_question_adr[4];
};


static WRITE8_HANDLER( quizmstr_bg_w )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	UINT8 *videoram = state->m_videoram;
	videoram[offset] = data;

	if(offset >= 0x0240)
		state->m_bg_tilemap->mark_tile_dirty(offset - 0x0240);
}


static void coinmstr_set_pal(running_machine &machine, UINT32 paldat, int col)
{
	col = col *4;

	{
		int r0, r1, r2, r3;
		int g0, g1, g2, g3;
		int b0, b1, b2, b3;

		r0 = (paldat & 0x1000) >> 12 ;
		g0 = (paldat & 0x0800) >> 11 ;
		b0 = (paldat & 0x0400) >> 10 ;
		r1 = (paldat & 0x0200) >> 9 ;
		g1 = (paldat & 0x0100) >> 8 ;
		b1 = (paldat & 0x0080) >> 7 ;

		r2 = (paldat & 0x0020) >> 5 ;
		g2 = (paldat & 0x0010) >> 4 ;
		b2 = (paldat & 0x0008) >> 3 ;
		r3 = (paldat & 0x0004) >> 2 ;
		g3 = (paldat & 0x0002) >> 1 ;
		b3 = (paldat & 0x0001) >> 0 ;


		palette_set_color_rgb(machine, col+0, (b0 * 255) << 5, (g0 * 255) << 5, (r0 * 255) << 5);
		palette_set_color_rgb(machine, col+2, (b1 * 255) << 5, (g1 * 255) << 5, (r1 * 255) << 5);
		palette_set_color_rgb(machine, col+1, (b2 * 255) << 5, (g2 * 255) << 5, (r2 * 255) << 5);
		palette_set_color_rgb(machine, col+3, (b3 * 255) << 5, (g3 * 255) << 5, (r3 * 255) << 5);

	}
}


static WRITE8_HANDLER( quizmstr_attr1_w )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	state->m_attr_ram1[offset] = data;

	if(offset >= 0x0240)
	{
		// the later games also use attr3 for something..
		UINT32	paldata = (state->m_attr_ram1[offset] & 0x7f) | ((state->m_attr_ram2[offset] & 0x7f) << 7);
		state->m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

		coinmstr_set_pal(space->machine(), paldata, offset - 0x240);

	}
}

static WRITE8_HANDLER( quizmstr_attr2_w )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	state->m_attr_ram2[offset] = data;

	if(offset >= 0x0240)
	{
		// the later games also use attr3 for something..
		UINT32	paldata = (state->m_attr_ram1[offset] & 0x7f) | ((state->m_attr_ram2[offset] & 0x7f) << 7);
		state->m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

		coinmstr_set_pal(space->machine(), paldata, offset - 0x240);

	}
}

static WRITE8_HANDLER( quizmstr_attr3_w )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	state->m_attr_ram3[offset] = data;

	if(offset >= 0x0240)
		state->m_bg_tilemap->mark_tile_dirty(offset - 0x0240);

}


static READ8_HANDLER( question_r )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	int address;
	UINT8 *questions = space->machine().region("user1")->base();

	switch(state->m_question_adr[2])
	{
		case 0x38: address = 0x00000; break; // state->m_question_adr[3] == 7
		case 0x39: address = 0x08000; break; // state->m_question_adr[3] == 7
		case 0x3a: address = 0x10000; break; // state->m_question_adr[3] == 7
		case 0x3b: address = 0x18000; break; // state->m_question_adr[3] == 7
		case 0x3c: address = 0x20000; break; // state->m_question_adr[3] == 7
		case 0x3d: address = 0x28000; break; // state->m_question_adr[3] == 7
		case 0x3e: address = 0x30000; break; // state->m_question_adr[3] == 7
		case 0x07: address = 0x38000; break; // state->m_question_adr[3] == 7
		case 0x0f: address = 0x40000; break; // state->m_question_adr[3] == 7
		case 0x17: address = 0x48000; break; // state->m_question_adr[3] == 7
		case 0x1f: address = 0x50000; break; // state->m_question_adr[3] == 7
		case 0x27: address = 0x58000; break; // state->m_question_adr[3] == 7
		case 0x2f: address = 0x60000; break; // state->m_question_adr[3] == 7
		case 0x37: address = 0x68000; break; // state->m_question_adr[3] == 7
		case 0x3f: address = 0x70000 + state->m_question_adr[3] * 0x8000; break;

		default:
			address = 0;
			logerror("unknown question rom # = %02X\n",state->m_question_adr[2]);
	}

	if(state->m_question_adr[3] == 6 || state->m_question_adr[3] > 7)
		logerror("question_adr[3] = %02X\n",state->m_question_adr[3]);

/*
    in these offsets they set 0x80... why?

    if( (state->m_question_adr[0] & 0x5f) == 0x00 ||
        (state->m_question_adr[0] & 0x5f) == 0x01 ||
        (state->m_question_adr[0] & 0x5f) == 0x0f ||
        (state->m_question_adr[0] & 0x5f) == 0x56 )
*/


//  don't know...
//  address |= ((state->m_question_adr[0] & 0x7f) << 8) | state->m_question_adr[1];
	address |= (state->m_question_adr[1] << 7) | (state->m_question_adr[0] & 0x7f);

	return questions[address];
}

static WRITE8_HANDLER( question_w )
{
	coinmstr_state *state = space->machine().driver_data<coinmstr_state>();
	if(data != state->m_question_adr[offset])
	{
		logerror("offset = %d data = %02X\n",offset,data);
	}

	state->m_question_adr[offset] = data;
}

static READ8_HANDLER( ff_r )
{
	return 0xff;
}

// Common memory map

static ADDRESS_MAP_START( coinmstr_map, AS_PROGRAM, 8, coinmstr_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(quizmstr_bg_w) AM_BASE_MEMBER(coinmstr_state, m_videoram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(quizmstr_attr1_w) AM_BASE_MEMBER(coinmstr_state, m_attr_ram1)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(quizmstr_attr2_w) AM_BASE_MEMBER(coinmstr_state, m_attr_ram2)
	AM_RANGE(0xf800, 0xffff) AM_RAM_WRITE(quizmstr_attr3_w) AM_BASE_MEMBER(coinmstr_state, m_attr_ram3)
ADDRESS_MAP_END

// Different I/O mappping for every game

static ADDRESS_MAP_START( quizmstr_io_map, AS_IO, 8, coinmstr_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(question_r)
	AM_RANGE(0x00, 0x03) AM_WRITE(question_w)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x41, 0x41) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x48, 0x4b) AM_DEVREADWRITE_MODERN("pia0", pia6821_device, read, write)
	AM_RANGE(0x50, 0x53) AM_READNOP
	AM_RANGE(0x50, 0x53) AM_WRITENOP
	AM_RANGE(0x58, 0x5b) AM_DEVREADWRITE_MODERN("pia2", pia6821_device, read, write)
	AM_RANGE(0x70, 0x70) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x71, 0x71) AM_DEVWRITE_MODERN("crtc", mc6845_device, register_w)
	AM_RANGE(0xc0, 0xc3) AM_READNOP
	AM_RANGE(0xc0, 0xc3) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( trailblz_io_map, AS_IO, 8, coinmstr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(question_r)
	AM_RANGE(0x00, 0x03) AM_WRITE(question_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x41, 0x41) AM_DEVWRITE_MODERN("crtc", mc6845_device, register_w)
	AM_RANGE(0x48, 0x49) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x49, 0x49) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE_MODERN("pia0", pia6821_device, read, write) //?
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE_MODERN("pia1", pia6821_device, read, write)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE_MODERN("pia2", pia6821_device, read, write)
	AM_RANGE(0xc1, 0xc3) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( supnudg2_io_map, AS_IO, 8, coinmstr_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(question_r)
	AM_RANGE(0x00, 0x03) AM_WRITE(question_w)
	AM_RANGE(0x40, 0x41) AM_READNOP
	AM_RANGE(0x40, 0x43) AM_WRITENOP
	AM_RANGE(0x43, 0x43) AM_READNOP
	AM_RANGE(0x48, 0x48) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x49, 0x49) AM_DEVWRITE_MODERN("crtc", mc6845_device, register_w)
	AM_RANGE(0x50, 0x51) AM_READNOP
	AM_RANGE(0x50, 0x53) AM_WRITENOP
	AM_RANGE(0x53, 0x53) AM_READNOP
	AM_RANGE(0x68, 0x69) AM_READNOP
	AM_RANGE(0x68, 0x6b) AM_WRITENOP
	AM_RANGE(0x6b, 0x6b) AM_READNOP
	AM_RANGE(0x78, 0x79) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x79, 0x79) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xc0, 0xc1) AM_READNOP
	AM_RANGE(0xc0, 0xc3) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( pokeroul_io_map, AS_IO, 8, coinmstr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x41, 0x41) AM_DEVWRITE_MODERN("crtc", mc6845_device, register_w)
	AM_RANGE(0x48, 0x49) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x49, 0x49) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x58, 0x5b) AM_DEVREADWRITE_MODERN("pia0", pia6821_device, read, write) /* confirmed */
	AM_RANGE(0x68, 0x6b) AM_DEVREADWRITE_MODERN("pia1", pia6821_device, read, write) /* confirmed */
	AM_RANGE(0x78, 0x7b) AM_DEVREADWRITE_MODERN("pia2", pia6821_device, read, write) /* confirmed */
	AM_RANGE(0xc0, 0xc1) AM_READ(ff_r)	/* needed to boot */
ADDRESS_MAP_END


static INPUT_PORTS_START( quizmstr )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Bookkeeping") PORT_TOGGLE /* Button 2 for second page, Button 3 erases data */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	/* If 0x40 is HIGH the DIP Test Mode does work but bookkeeping shows always 0's */
	/* If 0x40 is LOW Bookkeeping does work, but the second page (selected categories) is missing */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, "4-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Test Mode" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Self Test" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))
INPUT_PORTS_END

static INPUT_PORTS_START( trailblz )
	PORT_START("PIA0.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, "1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Cont")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pass")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sel")
	PORT_DIPNAME( 0x40, 0x40, "Show Refill" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Stats" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tests" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( supnudg2 )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.A")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.A" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA1.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, "1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Cont")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pass")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sel")
	PORT_DIPNAME( 0x40, 0x40, "Show Refill?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Stats?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Tests?" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "NVRAM Reset?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "First Install (DIL 8)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pokeroul )
	PORT_START("PIA0.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Cancel / Collect")                           PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 1 & 5 (auto?)")                         PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 2 / Bet / Half Gamble / Previous Hand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Deal / Draw / Gamble")                       PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Auto Hold")                                  PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Short Term Meters")              PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill Mode")                    PORT_TOGGLE PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("0-8")                                        PORT_CODE(KEYCODE_S)

	PORT_START("PIA0.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA0.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("PIA1.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE (2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE (2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIA1.B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIA2.A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PIA2.B")
	PORT_DIPNAME( 0x01, 0x01, "PIA2.B" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x80, 0x80, "Factory Install Switch" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( coinmstr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END


static TILE_GET_INFO( get_bg_tile_info )
{
	coinmstr_state *state = machine.driver_data<coinmstr_state>();
	UINT8 *videoram = state->m_videoram;
	int tile = videoram[tile_index + 0x0240];
	int color = tile_index;

	tile |= (state->m_attr_ram1[tile_index + 0x0240] & 0x80) << 1;
	tile |= (state->m_attr_ram2[tile_index + 0x0240] & 0x80) << 2;

	tile |= (state->m_attr_ram3[tile_index + 0x0240] & 0x03) << (6+4);

	SET_TILE_INFO(0, tile, color, 0);
}

static VIDEO_START( coinmstr )
{
	coinmstr_state *state = machine.driver_data<coinmstr_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 46, 32);
}

static SCREEN_UPDATE_IND16( coinmstr )
{
	coinmstr_state *state = screen.machine().driver_data<coinmstr_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/* Declare PIA structure */

/* PIA 0 */
static const pia6821_interface pia_0_intf =
{
	DEVCB_INPUT_PORT("PIA0.A"),		/* port A in */
	DEVCB_INPUT_PORT("PIA0.B"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

/* PIA 1 */
static const pia6821_interface pia_1_intf =
{
	DEVCB_INPUT_PORT("PIA1.A"),		/* port A in */
	DEVCB_INPUT_PORT("PIA1.B"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

/* PIA 2 */
static const pia6821_interface pia_2_intf =
{
	DEVCB_INPUT_PORT("PIA2.A"),		/* port A in */
	DEVCB_INPUT_PORT("PIA2.B"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



static const ay8910_interface ay8912_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const mc6845_interface h46505_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


static MACHINE_CONFIG_START( coinmstr, coinmstr_state )
	MCFG_CPU_ADD("maincpu",Z80,8000000) // ?
	MCFG_CPU_PROGRAM_MAP(coinmstr_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_PIA6821_ADD("pia0", pia_0_intf)
	MCFG_PIA6821_ADD("pia1", pia_1_intf)
	MCFG_PIA6821_ADD("pia2", pia_2_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 46*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(coinmstr)

	MCFG_GFXDECODE(coinmstr)
	MCFG_PALETTE_LENGTH(46*32*4)

	MCFG_VIDEO_START(coinmstr)

	MCFG_MC6845_ADD("crtc", H46505, 14000000 / 16, h46505_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_SOUND_CONFIG(ay8912_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizmstr, coinmstr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(quizmstr_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( trailblz, coinmstr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(trailblz_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supnudg2, coinmstr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(supnudg2_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pokeroul, coinmstr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(pokeroul_io_map)
MACHINE_CONFIG_END

/*

Quizmaster
Coinmaster, 1985

PCB Layout
----------


PCB 001-POK                                            5MBYTE MEMORY EXPANSION
|--------------------------------------------------|   |------------------------------|
|   ULN2803      6821                              |   |                              |
|                                                  |   |                              |
|                                NPC_QM4_11.IC45   |   |GESCHICH2.IC19  GESCHICH1.IC20|
|   ULN2003      6821                              |   |                              |
|     ULN2003                    NPC_QM4_21.IC41   |   |      *         GESCHICH3.IC18|
|                                                  |   |                              |
|7               6821                              |   |POPMUSIK2.IC15  POPMUSIK1.IC16|
|2                               HD465055          |   |                              |
|W                                                 |   |      *               *       |
|A                                                 |   |                              |
|Y           AY3-8912                       6116   |   |SPORT2.IC11     SPORT1.IC12   |
|                                                  |   |                              |
|                                                  |   |SPORT4.IC9      SPORT3.IC10   |
|                BATTERY                    6116   |   |                              |
|     DSW1(8)              NM_QM4_11.IC9           |   |GEOGRAPH2.IC7   GEOGRAPH1.IC8 |
|                  NE555                           |   |                              |
|            VOL           NP_QM4_21.IC6    6116   |   |      *               *       |
| LM380                                  PAL       |   |                              |
|                                                  |   |ALLGEME2.IC3    ALLGEME1.IC4  |
|                                        PAL       |   |                              |
|                                                  |   |      *         ALLGEME3.IC2  |
|                                        PAL       |   |                              |
|                                            14MHz |   |                              |
|                              Z80       CN1       |   |                       CN2    |
|                                                  |   |     PAL                      |
|--------------------------------------------------|   |------------------------------|
CN1/2 is connector for top ROM board                    * - unpopulated socket


*/

ROM_START( quizmstr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "nm_qm4_11.ic9", 0x0000, 0x4000, CRC(3a233bf0) SHA1(7b91b6f19093e67dd5513a000138421d4ef6f0af) )
	ROM_LOAD( "np_qm4_21.ic6", 0x4000, 0x4000, CRC(a1cd39e4) SHA1(420b0726577471c762ae470bc2138c035f295ad9) )
	/* 0x8000 - 0xbfff empty */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "npc_qm4_11.ic45", 0x0000, 0x2000, CRC(ed48582a) SHA1(0aa2434a43af2990b8ad1cd3fc9f2e5e962f99c7) )
	ROM_LOAD( "npc_qm4_21.ic41", 0x2000, 0x2000, CRC(b67b0183) SHA1(018cabace593e795edfe914cdaedb9ebdf158567) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* Question roms */
	/* empty ic1 */
	ROM_LOAD( "allgeme3.ic2",   0x08000, 0x8000, CRC(e9ead7f0) SHA1(c0b8e4e7905f31b74c8d217f0afc91f73d52927b) )
	ROM_LOAD( "allgeme2.ic3",   0x10000, 0x8000, CRC(ac4d2ee8) SHA1(3a64fba8a24ae2e8bfd9d1c27804342e1779bcf6) )
	ROM_LOAD( "allgeme1.ic4",   0x18000, 0x8000, CRC(896e619b) SHA1(6f0faf0ae206f20387024a4a426b3e92429b3b1d) )
	/* empty ic5 */
	/* empty ic6 */
	ROM_LOAD( "geograph2.ic7",  0x30000, 0x8000, CRC(d809eeb6) SHA1(c557cecd3dd641a9c293f1865a423dafcd71af82) )
	ROM_LOAD( "geograph1.ic8",  0x38000, 0x8000, CRC(8984e83c) SHA1(d22c02e9297f804f8560e2e46793e4b6654d0785) )
	ROM_LOAD( "sport4.ic9",     0x40000, 0x8000, CRC(3c37de48) SHA1(bee26e9b15cec0b8e81af59810db17a8f2bdc299) )
	ROM_LOAD( "sport3.ic10",    0x48000, 0x8000, CRC(24abe1e7) SHA1(77373b1fafa4b117b3a1e4c6e8b530e0bb3b4f42) )
	ROM_LOAD( "sport2.ic11",    0x50000, 0x8000, CRC(26645e8e) SHA1(4922dcd417f7d098aaaa6a0320ed1d3e488d3e63) )
	ROM_LOAD( "sport1.ic12",    0x58000, 0x8000, CRC(7be41758) SHA1(8e6452fd902d25a73d3fa89bd7b4c5563669cc92) )
	/* empty ic13 */
	/* empty ic14 */
	ROM_LOAD( "popmusik2.ic15", 0x70000, 0x8000, CRC(d3b9ea70) SHA1(0a92ecdc4e2ddd3c0f40682a46a88bc617829481) )
	ROM_LOAD( "popmusik1.ic16", 0x78000, 0x8000, CRC(685f047e) SHA1(c0254130d57f60435a70effe6376e0cb3f50223f) )
	/* empty ic17 */
	ROM_LOAD( "geschich3.ic18", 0x88000, 0x8000, CRC(26c3ceec) SHA1(bf6fd24576c6159bf7730b04d2ac451bfcf3f757) )
	ROM_LOAD( "geschich2.ic19", 0x90000, 0x8000, CRC(387d166e) SHA1(14edac9ef550ce64fd81567520f3009612aa7221) )
	ROM_LOAD( "geschich1.ic20", 0x98000, 0x8000, CRC(bf4c097f) SHA1(eb14e7bad713d3b03fa3978a7f0087312517cf9e) )
ROM_END


ROM_START( trailblz )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1-4.09",       0x0000, 0x4000, CRC(7c34749c) SHA1(3847188a734b32979f376f51f74dff050b610dfb) )
	ROM_LOAD( "2-4.06",       0x4000, 0x4000, CRC(81a9809b) SHA1(4d2bfd5223713a9e2e15130a3176118d400ee63e) )
	/* 0x8000 - 0xbfff empty */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "1-2.45",       0x0000, 0x2000, CRC(b4a807b1) SHA1(f00a4790adb0c25917a0dc8c98c9b65526304fd3) )
	ROM_LOAD( "2-2.41",       0x2000, 0x2000, CRC(756dd230) SHA1(6d6f440bf1f48cc33d5e46cfc645809d5f8b1f3a) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* Question roms */
	ROM_LOAD( "questions.bin", 0x00000, 0xa0000, NO_DUMP )
ROM_END


ROM_START( supnudg2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u3.bin",       0x0000, 0x8000, CRC(ed04e2cc) SHA1(7d90a588cca2d113487710e897771f9d99e37e62) )
	ROM_LOAD( "u4.bin",       0x8000, 0x8000, CRC(0551e859) SHA1(b71640097cc75b78f3013f0e77de328bf1a205b1) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u25.bin",      0x0000, 0x8000, CRC(1f7cef5e) SHA1(3abc31d400a0f5dc29c70d8aac42fd6302290cc9) )
	ROM_LOAD( "u23.bin",      0x8000, 0x8000, CRC(726a48ac) SHA1(cd17840067294812edf5bfa88d71fc967388df8e) )

	ROM_REGION( 0xa0000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "q1.bin",       0x00000, 0x8000, CRC(245d679a) SHA1(2d3fbed8c1b3d0bffe7f3bd9088e0a5d207654c7) )
	ROM_LOAD( "q2.bin",       0x08000, 0x8000, CRC(e41ae8fb) SHA1(526c7b60e6ee4dfe05bbabf0e1e986e04ac2f544) )
	ROM_LOAD( "q3.bin",       0x10000, 0x8000, CRC(692218a2) SHA1(b9548dd835d9f3fb3e09bd018c7f9cbecafaee28) )
	ROM_LOAD( "q4.bin",       0x18000, 0x8000, CRC(ce4be482) SHA1(4fd8f24d22d3f1789fc728445cbc5339ed454bb4) )
	ROM_LOAD( "q5.bin",       0x20000, 0x8000, CRC(805672bf) SHA1(0fa68cad0d1c2b11a04a364b5ff64facfa573bbc) )
	ROM_LOAD( "q6.bin",       0x28000, 0x8000, CRC(b4405848) SHA1(5f8ca8b017966e6f358f603efde83f45897f3476) )
	ROM_LOAD( "q7.bin",       0x30000, 0x8000, CRC(32329b78) SHA1(114f097678be734355b8f36f6af7f1cb75ece191) )
	ROM_LOAD( "q8.bin",       0x38000, 0x8000, CRC(25c2aa26) SHA1(7f95553bf98381ced086b6606345bef62fe89a3a) )
	ROM_LOAD( "q9.bin",       0x40000, 0x8000, CRC(c98cb15a) SHA1(7d12064c2bcb34668299cadae3072c7f8434c405) )
	ROM_LOAD( "q10.bin",      0x48000, 0x8000, CRC(0c6c2df5) SHA1(49c92e498a0556032bb8ca56ff5afb9f69a80b3f) )
	ROM_LOAD( "q11.bin",      0x50000, 0x8000, CRC(1c53a264) SHA1(c10cc32b032bd4f890497bdc942e7e8c75ea1d6f) )
	ROM_LOAD( "q12.bin",      0x58000, 0x8000, CRC(c9535bff) SHA1(9c9873642c62971f805dc629f8d1006e35a675f9) )
	ROM_LOAD( "q13.bin",      0x60000, 0x8000, CRC(7a9b9f61) SHA1(7e39fef67fc3c29604ae68358e01330cf5130c06) )
	ROM_LOAD( "q14.bin",      0x68000, 0x8000, CRC(ec35e800) SHA1(0e0ca6fec760f31f464b282a1d7341cc4a29c064) )
	ROM_LOAD( "q15.bin",      0x70000, 0x8000, CRC(9f3738eb) SHA1(e841958f37167e7f9adcd3c965d31e2b7e02f52c) )
	ROM_LOAD( "q16.bin",      0x78000, 0x8000, CRC(af92277c) SHA1(093079fab28e3de443b640d2777cc2980b20af6c) )
	ROM_LOAD( "q17.bin",      0x80000, 0x8000, CRC(522fd485) SHA1(6c2a2626c00015962c460eac0dcb46ea263a4a23) )
	ROM_LOAD( "q18.bin",      0x88000, 0x8000, CRC(54d50510) SHA1(2a8ad2a2e1735f9c7d606b99b3653f823f09d1e8) )
	ROM_LOAD( "q19.bin",      0x90000, 0x8000, CRC(30aa2ff5) SHA1(4a2b4fc9c0c5cab3d374ee4738152209589e0807) )
	ROM_LOAD( "q20.bin",      0x98000, 0x8000, CRC(0845b450) SHA1(c373839ee1ad983e2df41cb22f625c14972372b0) )
ROM_END

/*

  Poker Roulette (c) 1990 Coinmaster


  Hardware notes:

  CPU:  1x z80
  RAM:  2x 6264
  I/O:  3x 6821 PIAs
  CRTC: 1x 6845
  SND:  1x ay-3-8912
  Xtal: 1x 14MHz.


  Dev notes:

  2x gfx banks, switched by bit4 of attr RAM.

*/

ROM_START( pokeroul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker1.ic3",   0x0000, 0x8000, CRC(bfe78d09) SHA1(7cc0f57714ff808a41ce20027a283e5dff60f752) )
	ROM_LOAD( "poker2.ic4",   0x8000, 0x8000, CRC(34c1b55c) SHA1(fa562d230a57dce3fff176c21c86b461a02749f6) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "027c1.00_e14.7.88.ic25",   0x0000, 0x8000, CRC(719f9879) SHA1(122e2689a21a22713f938e3bf6cfb72c60fb9a16) )
	ROM_LOAD( "027c1.01_e14.7.88.ic23",   0x8000, 0x8000, CRC(71e5a2fc) SHA1(c28efcea1cf6c9872e70ff191932e3cdb5618917) )
ROM_END


ROM_START( jpcoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x002000, CRC(67e1aa60) SHA1(32301f60a7325f23047d84bb1e9416ea05753493) )
	ROM_LOAD( "1.bin", 0x2000, 0x002000, CRC(6c79e430) SHA1(56e026329ea6aba122d1f66c375bf4c3cc829feb) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "3.bin", 0x0000, 0x004000, CRC(4984053e) SHA1(e0f7c56160f48f7b1c2c407f448c13a191770adc) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "4.bin", 0x4000, 0x004000, CRC(2bac1c0b) SHA1(3e45fc38ed6d332e1d49b2b66bf8001610f914c5) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END

static DRIVER_INIT( coinmstr )
{
	UINT8 *rom = machine.region("user1")->base();
	int length = machine.region("user1")->bytes();
	UINT8 *buf = auto_alloc_array(machine, UINT8, length);
	int i;

	memcpy(buf,rom,length);

	for(i = 0; i < length; i++)
	{
		int adr = BITSWAP24(i, 23,22,21,20,19,18,17,16,15, 14,8,7,2,5,12,10,9,11,13,3,6,0,1,4);
		rom[i] = BITSWAP8(buf[adr],3,2,4,1,5,0,6,7);
	}

	auto_free(machine, buf);
}




GAME( 1985, quizmstr, 0, quizmstr, quizmstr, coinmstr, ROT0, "Loewen Spielautomaten", "Quizmaster (German)",            GAME_UNEMULATED_PROTECTION )
GAME( 1987, trailblz, 0, trailblz, trailblz, coinmstr, ROT0, "Coinmaster",            "Trail Blazer",                   GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING ) // or Trail Blazer 2 ?
GAME( 1989, supnudg2, 0, supnudg2, supnudg2, coinmstr, ROT0, "Coinmaster",            "Super Nudger II (Version 5.21)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1990, pokeroul, 0, pokeroul, pokeroul, 0,        ROT0, "Coinmaster",            "Poker Roulette (Version 8.22)",  GAME_NOT_WORKING )
GAME( 1990, jpcoin,   0, pokeroul, pokeroul, 0,        ROT0, "Coinmaster",            "Joker Poker (Coinmaster)",  GAME_NOT_WORKING ) // io stuff is different at least
