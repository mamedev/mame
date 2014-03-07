/* 
   Dinamic / Inder arcade hardware

   Mega Phoenix
 
 also known to exist on this hardware:
   Hammer Boy
   Nonamed (ever finished? only code seen has 1991 date and is vastly incomplete) (versions exist for Amstrad CPC, MSX and Spectrum)



  trivia: Test mode graphics are the same as Little Robin(?!)

*/

/*

 need to work out what rand() bits are
  - game is very timing sensitive, to get all the gfx to copy i'm having to OC the 68k x 16 and it still glitches (phoenix ship is broken, round 4 gfx don't copy properly)
    probably due to above or irq handling
  - where should roms 6/7 map, they contain the 68k vectors, but the game expects RAM at 0, and it doesn't seem to read any of the other data from those roms.. they contain
    a cross hatch pattern amongst other things?



  
  - sound..

  /


  --


  Chips of note

  Main board:

  TS68000CP8
  TMS34010FNL-40
  TMP82C55AP-2
 
  Bt478KPJ35  Palette / RAMDAC

  Actel A1010A-PL68C  (custom blitter maybe?)

  2x 8 DSW, bottom corner, away from everything..

 Sub / Sound board:

  ST Z8430AB1

  custom INDER badged chip 40 pin?  (probably just a z80 - it's in the sound section)
	MODELO: MEGA PHOENIX
	KIT NO. 1.034
	FECHA FABRICACION 08.10.91
	LA MANIPULCION DE LA ETIQUETA O DE LA PLACA ANULA SU SARANTIA
	(this sticker is also present on the other PCB)


*/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "video/ramdac.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "cpu/z80/z80daisy.h"
#include "sound/dac.h"

class megaphx_state : public driver_device
{
public:
	megaphx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_ctc(*this, "ctc"),
		m_dac0(*this, "dac0" ),
		m_dac1(*this, "dac1" ),
		m_dac2(*this, "dac2" ),
		m_dac3(*this, "dac3" ),
		port_c_value(0),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_vram;
	required_device<z80ctc_device> m_ctc;

	required_device<dac_device> m_dac0;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<dac_device> m_dac3;

	DECLARE_DRIVER_INIT(megaphx);
	DECLARE_MACHINE_RESET(megaphx);


	DECLARE_CUSTOM_INPUT_MEMBER(megaphx_rand_r);

	DECLARE_READ16_MEMBER(tms_host_r);
	DECLARE_WRITE16_MEMBER(tms_host_w);

	DECLARE_READ16_MEMBER(megaphx_0x050002_r);
	DECLARE_WRITE16_MEMBER(megaphx_0x050000_w);
	DECLARE_READ8_MEMBER(megaphx_sound_sent_r);
	DECLARE_READ8_MEMBER(megaphx_sound_cmd_r);
	DECLARE_WRITE8_MEMBER(megaphx_sound_to_68k_w);


	DECLARE_WRITE8_MEMBER(dac0_value_write);
	DECLARE_WRITE8_MEMBER(dac0_gain_write);
	DECLARE_WRITE8_MEMBER(dac1_value_write);
	DECLARE_WRITE8_MEMBER(dac1_gain_write);
	DECLARE_WRITE8_MEMBER(dac2_value_write);
	DECLARE_WRITE8_MEMBER(dac2_gain_write);
	DECLARE_WRITE8_MEMBER(dac3_value_write);
	DECLARE_WRITE8_MEMBER(dac3_gain_write);

	DECLARE_WRITE8_MEMBER(dac0_rombank_write);
	DECLARE_WRITE8_MEMBER(dac1_rombank_write);
	DECLARE_WRITE8_MEMBER(dac2_rombank_write);
	DECLARE_WRITE8_MEMBER(dac3_rombank_write);

	UINT8 dac_gain[4];


	/*
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to0);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to1);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to2);
	*/

	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_c_w);

	int m_pic_is_reset;
	int m_pic_shift_pos;
	int m_pic_data;
	int m_pic_data_bit;
	int m_pic_clock;
	int m_pic_readbit;

	UINT16 m_pic_result;

	UINT8 port_c_value;
	required_device<palette_device> m_palette;
	int m_soundsent;
	UINT8 m_sounddata;
	UINT8 m_soundback;
};

#include "sound/dac.h"



CUSTOM_INPUT_MEMBER(megaphx_state::megaphx_rand_r)
{
	return rand();
}





READ16_MEMBER(megaphx_state::tms_host_r)
{
	return tms34010_host_r(machine().device("tms"), offset);
}


WRITE16_MEMBER(megaphx_state::tms_host_w)
{
	tms34010_host_w(machine().device("tms"), offset, data);
}

READ16_MEMBER(megaphx_state::megaphx_0x050002_r)
{
//	int pc = machine().device("maincpu")->safe_pc();
	int ret = m_soundback;
	m_soundback = 0;
	//logerror("(%06x) megaphx_0x050002_r (from z80?) %04x\n", pc, mem_mask);
	return ret ^ (rand()&0x40);  // the 0x40 should be returned by the z80, so this still isn't working
}

WRITE16_MEMBER(megaphx_state::megaphx_0x050000_w)
{
//	int pc = machine().device("maincpu")->safe_pc();
	space.machine().scheduler().synchronize();

	//logerror("(%06x) megaphx_0x050000_w (to z80?) %04x %04x\n", pc, data, mem_mask);
	m_soundsent = 0xff;
	m_sounddata = data;

}


static ADDRESS_MAP_START( megaphx_68k_map, AS_PROGRAM, 16, megaphx_state )
	AM_RANGE(0x000000, 0x0013ff) AM_RAM AM_SHARE("mainram") // maps over part of the rom??

	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("roms67", 0x00000) // or the rom doesn't map here? it contains the service mode grid amongst other things..

	AM_RANGE(0x040000, 0x040007) AM_READWRITE(tms_host_r, tms_host_w)

	AM_RANGE(0x050000, 0x050001) AM_WRITE(megaphx_0x050000_w) // z80 comms?
	AM_RANGE(0x050002, 0x050003) AM_READ(megaphx_0x050002_r) // z80 comms?


	AM_RANGE(0x060004, 0x060005) AM_READ8( port_c_r, 0x00ff )
	
	AM_RANGE(0x060006, 0x060007) AM_WRITE8( port_c_w, 0x00ff )

	AM_RANGE(0x060000, 0x060003) AM_DEVREADWRITE8("ppi8255_0", i8255_device, read, write, 0x00ff)
	
	AM_RANGE(0x800000, 0x83ffff) AM_ROM  AM_REGION("roms01", 0x00000) // code + bg gfx are in here
	AM_RANGE(0x840000, 0x87ffff) AM_ROM  AM_REGION("roms23", 0x00000) // bg gfx are in here
	AM_RANGE(0x880000, 0x8bffff) AM_ROM  AM_REGION("roms45", 0x00000) // bg gfx + title screen in here

ADDRESS_MAP_END


static ADDRESS_MAP_START( megaphx_tms_map, AS_PROGRAM, 16, megaphx_state )

	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("vram") // vram?
//	AM_RANGE(0x00100000, 0x002fffff) AM_RAM  // vram?
//	AM_RANGE(0x00300000, 0x003fffff) AM_RAM
//	AM_RANGE(0x04000000, 0x040000ff) AM_WRITENOP

	AM_RANGE(0x04000000, 0x0400000f) AM_DEVWRITE8("ramdac",ramdac_device,index_w,0x00ff)
	AM_RANGE(0x04000010, 0x0400001f) AM_DEVREADWRITE8("ramdac",ramdac_device,pal_r,pal_w,0x00ff)
	AM_RANGE(0x04000030, 0x0400003f) AM_DEVWRITE8("ramdac",ramdac_device,index_r_w,0x00ff)
	AM_RANGE(0x04000090, 0x0400009f) AM_WRITENOP

	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, megaphx_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("snddata")
ADDRESS_MAP_END

READ8_MEMBER(megaphx_state::megaphx_sound_cmd_r)
{
	return m_sounddata;
}

READ8_MEMBER(megaphx_state::megaphx_sound_sent_r)
{
	int ret = m_soundsent;
	m_soundsent = 0;
	return ret;
}

WRITE8_MEMBER(megaphx_state::megaphx_sound_to_68k_w)
{
//	int pc = machine().device("audiocpu")->safe_pc();

	//logerror("(%04x) megaphx_sound_to_68k_w (to 68k?) %02x\n", pc, data);

	m_soundback = data;
}

WRITE8_MEMBER(megaphx_state::dac0_value_write)
{
//	printf("dac0_data_write %02x\n", data);
	m_dac0->write_unsigned8(data);
}

WRITE8_MEMBER(megaphx_state::dac0_gain_write)
{
//	printf("dac0_gain_write %02x\n", data);
	dac_gain[0] = data;
}

WRITE8_MEMBER(megaphx_state::dac1_value_write)
{
//	printf("dac1_data_write %02x\n", data);
	m_dac1->write_unsigned8(data);
}

WRITE8_MEMBER(megaphx_state::dac1_gain_write)
{
//	printf("dac1_gain_write %02x\n", data);
	dac_gain[1] = data;
}

WRITE8_MEMBER(megaphx_state::dac2_value_write)
{
//	printf("dac2_data_write %02x\n", data);
	m_dac2->write_unsigned8(data);
}

WRITE8_MEMBER(megaphx_state::dac2_gain_write)
{
//	printf("dac2_gain_write %02x\n", data);
	dac_gain[2] = data;
}

WRITE8_MEMBER(megaphx_state::dac3_value_write)
{
//	printf("dac3_data_write %02x\n", data);
	m_dac3->write_unsigned8(data);
}

WRITE8_MEMBER(megaphx_state::dac3_gain_write)
{
//	printf("dac3_gain_write %02x\n", data);
	dac_gain[3] = data;
}

WRITE8_MEMBER(megaphx_state::dac0_rombank_write)
{

}

WRITE8_MEMBER(megaphx_state::dac1_rombank_write)
{

}

WRITE8_MEMBER(megaphx_state::dac2_rombank_write)
{

}

WRITE8_MEMBER(megaphx_state::dac3_rombank_write)
{

}


static ADDRESS_MAP_START( sound_io, AS_IO, 8, megaphx_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(dac0_value_write)
	AM_RANGE(0x01, 0x01) AM_WRITE(dac0_gain_write)
	AM_RANGE(0x02, 0x02) AM_WRITE(dac1_value_write)
	AM_RANGE(0x03, 0x03) AM_WRITE(dac1_gain_write)
	AM_RANGE(0x04, 0x04) AM_WRITE(dac2_value_write)
	AM_RANGE(0x05, 0x05) AM_WRITE(dac2_gain_write)
	AM_RANGE(0x06, 0x06) AM_WRITE(dac3_value_write)
	AM_RANGE(0x07, 0x07) AM_WRITE(dac3_gain_write)

	// not 100% sure how rom banking works.. but each channel can specify a different bank for the 0x8000 range.  Maybe the bank happens when the interrupt triggers so each channel reads the correct data? (so we'd need to put the actual functions in the CTC callbacks)
	AM_RANGE(0x10, 0x10) AM_WRITE(dac0_rombank_write)
	AM_RANGE(0x11, 0x11) AM_WRITE(dac1_rombank_write)
	AM_RANGE(0x12, 0x12) AM_WRITE(dac2_rombank_write)
	AM_RANGE(0x13, 0x13) AM_WRITE(dac3_rombank_write)


	

	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	
	AM_RANGE(0x30, 0x30) AM_READWRITE(megaphx_sound_cmd_r, megaphx_sound_to_68k_w)
	AM_RANGE(0x31, 0x31) AM_READ(megaphx_sound_sent_r)
ADDRESS_MAP_END


static void megaphx_scanline(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	megaphx_state *state = screen.machine().driver_data<megaphx_state>();

	UINT16 *vram = &state->m_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);

	const pen_t *paldata = state->m_palette->pens();

	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = paldata[pixels & 0xff];
		dest[x + 1] = paldata[pixels >> 8];
	}

}

static void megaphx_to_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	megaphx_state *state = space.machine().driver_data<megaphx_state>();
	memcpy(shiftreg, &state->m_vram[TOWORD(address)/* & ~TOWORD(0x1fff)*/], TOBYTE(0x2000));
}

static void megaphx_from_shiftreg(address_space &space, UINT32 address, UINT16 *shiftreg)
{
	megaphx_state *state = space.machine().driver_data<megaphx_state>();
	memcpy(&state->m_vram[TOWORD(address)/* & ~TOWORD(0x1fff)*/], shiftreg, TOBYTE(0x2000));
}

MACHINE_RESET_MEMBER(megaphx_state,megaphx)
{
}

static void m68k_gen_int(device_t *device, int state)
{
	megaphx_state *drvstate = device->machine().driver_data<megaphx_state>();
	if (state) drvstate->m_maincpu->set_input_line(4, ASSERT_LINE);
	else drvstate->m_maincpu->set_input_line(4, CLEAR_LINE);
//	printf("interrupt %d\n", state);
}


static const tms34010_config tms_config_megaphx =
{
	TRUE,                          /* halt on reset */
	"screen",                       /* the screen operated on */
	XTAL_40MHz/12,                   /* pixel clock */
	2,                              /* pixels per clock */
	NULL,                           /* scanline callback (indexed16) */
	megaphx_scanline,              /* scanline callback (rgb32) */
	m68k_gen_int,                   /* generate interrupt */
	megaphx_to_shiftreg,           /* write to shiftreg function */
	megaphx_from_shiftreg          /* read from shiftreg function */
};



static INPUT_PORTS_START( megaphx )
	PORT_START("P0") // verified in test mode
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // shield
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // unused ? (in test mode)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) // high score entry
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) // high score entry
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P1") // verified in test mode
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // shield
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // unused ? (in test mode)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) // high score entry
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) //high score entry
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)


	PORT_START("PIC1") // via PIC
	PORT_DIPNAME( 0x0001, 0x0001, "XX" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1") // via PIC
	PORT_DIPNAME( 0x0007, 0x0003, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0018, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00c0, 0x0080, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x00c0, "4" )


	PORT_START("DSW2") // via PIC  // some of these are difficulty
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0002, IP_ACTIVE_HIGH ) 
	PORT_DIPNAME( 0x0004, 0x0004, "DSW2-04"  )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DSW2-08"  )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DSW2-10"  )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DSW2-20"  )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DSW2-40"  )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DSW2-80" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, megaphx_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb888_w)
ADDRESS_MAP_END

static RAMDAC_INTERFACE( ramdac_intf )
{
	1
};

/* why don't the port_c read/writes work properly when hooked through the 8255? */

// the PIC is accessed serially through clock / data lines, each time 16-bits are accessed..
// not 100% sure if the command takes effect after all 16-bits are written, or after 8..

READ8_MEMBER(megaphx_state::port_c_r)
{
	
	//printf("read port c - write value was %02x\n", port_c_value);

//	int pc = machine().device("maincpu")->safe_pc();
	UINT8 ret = 0;

//	printf("(%06x) port_c_r (thru 8255)\n", pc);
	
	if (m_pic_clock == 1) ret |= 0x08;
	if (m_pic_readbit == 1) ret |= 0x02;
//	return ioport("SYS")->read();
	return ret;
}


WRITE8_MEMBER(megaphx_state::port_c_w)
{
	


//	int pc = machine().device("maincpu")->safe_pc();
	port_c_value = (data & 0x0f);

	if (port_c_value == 0x9)
	{
	//	printf("Assert PIC reset line\n");
		m_pic_is_reset = 1;
	}
	else if (port_c_value == 0x8)
	{
	//	printf("Clear PIC reset line\n");
		m_pic_is_reset = 0;
	
		m_pic_shift_pos = 0;
		m_pic_data = 0;
		m_pic_data_bit = 0;
		m_pic_readbit = 0;
		m_pic_clock = 1;

	}
	else if (port_c_value == 0xd)
	{
	//	printf("Set PIC data line\n");
		m_pic_data_bit = 1;
	}
	else if (port_c_value == 0xc)
	{
	//	printf("Clear PIC data line\n");
		m_pic_data_bit = 0;
	}
	else if (port_c_value == 0xf)
	{
		if (m_pic_clock == 0)
		{
		//	printf("Set PIC clock line | pos %d | bit %d\n", m_pic_shift_pos, m_pic_data_bit);
			




			m_pic_clock = 1;
		
		}
	}
	else if (port_c_value == 0xe)
	{

		if (m_pic_clock == 1)
		{
			m_pic_data |= m_pic_data_bit << m_pic_shift_pos;

			if (m_pic_shift_pos == 8)
			{
				//printf("------------------ sending command %02x\n", m_pic_data);

				if (m_pic_data == 0xfe) // get software version??
				{
					m_pic_result = (ioport("PIC1")->read()) | (0XFF << 8);
				}
				else if (m_pic_data == 0x82) // dsw1
				{
					m_pic_result = (ioport("PIC1")->read()) | ((ioport("DSW1")->read()) << 8);
				}
				else if (m_pic_data == 0x86) // dsw2
				{
					m_pic_result = (ioport("PIC1")->read()) | ((ioport("DSW2")->read()) << 8);
				}
				else
				{
					printf("unknown PIC command %02x\n", m_pic_data);
				}
			}

			m_pic_readbit = (m_pic_result >> (m_pic_shift_pos)) & 1;


			m_pic_shift_pos++;


			//	printf("Clear PIC clock line\n");
			m_pic_clock = 0;
		}
	}
	else
	{
	//	printf("Unknown write to PIC %02x (PC %06x)\n", port_c_value, pc);
	}



}


static I8255A_INTERFACE( ppi8255_intf_0 )
{
	DEVCB_INPUT_PORT("P0"),        /* Port A read */
	DEVCB_NULL,                     /* Port A write */
	DEVCB_INPUT_PORT("P1"),        /* Port B read */
	DEVCB_NULL,                     /* Port B write */
	DEVCB_NULL,        /* Port C read */ // should be connected to above functions but values are incorrect
	DEVCB_NULL,        /* Port C write */  // should be connected to above functions but values are incorrect
};


/*
WRITE_LINE_MEMBER(megaphx_state::z80ctc_to0)
{
	logerror("z80ctc_to0 %d\n", state);
}

WRITE_LINE_MEMBER(megaphx_state::z80ctc_to1)
{
	logerror("z80ctc_to1 %d\n", state);
}

WRITE_LINE_MEMBER(megaphx_state::z80ctc_to2)
{
	logerror("z80ctc_to2 %d\n", state);
}
*/
	

static Z80CTC_INTERFACE( z80ctc_intf ) // runs in IM2 , vector set to 0x20 , values there are 0xCC, 0x02, 0xE6, 0x02, 0x09, 0x03, 0x23, 0x03  (so 02cc, 02e6, 0309, 0323, all of which are valid irq handlers)
{
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),    // for channel 0
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),    // for channel 1
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),    // for channel 2
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0)     // for channel 3
};

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ NULL }
};


// just for debug.. so we can see what is in each of the roms
static GFXLAYOUT_RAW( megaphxlay, 336, 1, 336*8, 336*8 )

static GFXDECODE_START( megaphx )
	GFXDECODE_ENTRY( "roms01", 0, megaphxlay,     0x0000, 1 )
	GFXDECODE_ENTRY( "roms23", 0, megaphxlay,     0x0000, 1 )
	GFXDECODE_ENTRY( "roms45", 0, megaphxlay,     0x0000, 1 )
	GFXDECODE_ENTRY( "roms67", 0, megaphxlay,     0x0000, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( megaphx, megaphx_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000*16) // ??  can't read xtal due to reflections, CPU is an 8Mhz part  // CLEARLY the 'rand' flags have more meaning (but don't seem to be vblank) I shouldn't have to do a *16 on the 68k clock just to get all the gfx!
	MCFG_CPU_PROGRAM_MAP(megaphx_68k_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", megaphx_state,  irq6_line_hold)


	MCFG_CPU_ADD("tms", TMS34010, XTAL_40MHz)
	MCFG_CPU_CONFIG(tms_config_megaphx)
	MCFG_CPU_PROGRAM_MAP(megaphx_tms_map)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) // unk freq
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io)

	MCFG_I8255A_ADD( "ppi8255_0", ppi8255_intf_0 )
	MCFG_Z80CTC_ADD( "ctc", 4000000, z80ctc_intf ) // unk freq

	MCFG_MACHINE_RESET_OVERRIDE(megaphx_state,megaphx)

//	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_40MHz/12, 424, 0, 338-1, 262, 0, 246-1)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)

	MCFG_PALETTE_ADD("palette", 256)
	
	MCFG_GFXDECODE_ADD("gfxdecode", megaphx)

	MCFG_RAMDAC_ADD("ramdac", ramdac_intf, ramdac_map, "palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")	
	MCFG_DAC_ADD("dac0")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)


MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(megaphx_state,megaphx)
{
	UINT16 *src = (UINT16*)memregion( "roms67" )->base();
	// copy vector table? - it must be writable because the game write the irq vector..
	memcpy(m_mainram, src, 0x80);

	membank("snddata")->configure_entries(0, 8, memregion("user2")->base(), 0x8000);
	membank("snddata")->set_entry(0);
}


ROM_START( megaphx )
	ROM_REGION16_BE( 0x40000, "roms67", 0 )
	ROM_LOAD16_BYTE( "mph6.u32", 0x000001, 0x20000, CRC(b99703d4) SHA1(393b6869e71d4c61060e66e0e9e36a1e6ca345d1) )
	ROM_LOAD16_BYTE( "mph7.u21", 0x000000, 0x20000, CRC(f11e7449) SHA1(1017142d10011d68e49d3ccdb1ac4e815c03b17a) )

	ROM_REGION16_BE( 0x40000, "roms01", 0 )
	ROM_LOAD16_BYTE( "mph0.u38", 0x000001, 0x20000, CRC(b63dd20f) SHA1(c8ce5985a6ba49428d66a49d9d623ccdfce422c2) )
	ROM_LOAD16_BYTE( "mph1.u27", 0x000000, 0x20000, CRC(4dcbf44b) SHA1(a8fa49ecd033f1aeb323e0032ddcf5f8f9463ac0) )

	ROM_REGION16_BE( 0x40000, "roms23", 0 )
	ROM_LOAD16_BYTE( "mph2.u37", 0x000001, 0x20000, CRC(a0f69c27) SHA1(d0c5c241d94a1f03f51e7e517e2f9dec6abcf75a) )
	ROM_LOAD16_BYTE( "mph3.u26", 0x000000, 0x20000, CRC(4db84cc5) SHA1(dd74acd4b32c7e7553554ac0f9ba13503358e869) )

	ROM_REGION16_BE( 0x40000, "roms45", 0 )
	ROM_LOAD16_BYTE( "mph4.u36", 0x000001, 0x20000, CRC(c8e0725e) SHA1(b3af315b9a94a692e81e0dbfd4035036c2af4f50) )
	ROM_LOAD16_BYTE( "mph5.u25", 0x000000, 0x20000, CRC(c95ccb69) SHA1(9d14cbfafd943f6ff461a7f373170a35e36eb695) )

	ROM_REGION( 0x200000, "user2", 0 )
	ROM_LOAD( "sonido_mph1.u39", 0x00000, 0x20000, CRC(f5e65557) SHA1(5ae759c2bcef96fbda42f088c02b6dec208030f3) )
	ROM_LOAD( "sonido_mph2.u38", 0x20000, 0x20000, CRC(7444d0f9) SHA1(9739b48993bccea5530533b67808d13d6155ffe3) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD( "sonido_mph0.u35", 0x000000, 0x2000,  CRC(abc1b140) SHA1(8384a162d85cf9ea870d22f44b1ca64001c6a083) )

	ROM_REGION( 0x100000, "pals", 0 ) // jedutil won't convert these? are they bad?
	ROM_LOAD( "p31_u31_palce16v8h-25.jed", 0x000, 0xbd4, CRC(05ef04b7) SHA1(330dd81a832b6675fb0473868c26fe9bec2da854) )
	ROM_LOAD( "p40_u29_palce16v8h-25.jed", 0x000, 0xbd4, CRC(44b7e51c) SHA1(b8b34f3b319d664ec3ad72ed87d9f65701f183a5) )

	// there is a PIC responsible for some I/O tasks (what type? what internal rom size?)
ROM_END

GAME( 1991, megaphx,  0,        megaphx, megaphx, megaphx_state, megaphx, ROT0, "Dinamic / Inder", "Mega Phoenix", GAME_NO_SOUND | GAME_NOT_WORKING )
