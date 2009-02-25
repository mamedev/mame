/******************************************************************************

Sega System 1 / System 2

driver by Jarek Parchanski, Nicola Salmoria, Mirko Buffoni


Up'n Down, Mister Viking, Flicky, SWAT, Water Match and Bull Fight are known
to run on IDENTICAL hardware (they were sold by Bally-Midway as ROM swaps).

DIP locations verified from manual for:
      - wboy
      - chplft

TODO: - background is misplaced in wbmlju
      - sprites stick in Pitfall II
      - sprite priorities are probably wrong
      - remove patch in nobb if possible and fully understand the
        ports involved in the protection

******************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/system1.h"
#include "cpu/z80/z80.h"
#include "machine/segacrpt.h"
#include "machine/mc8123.h"
#include "sound/sn76496.h"

static UINT8 *system1_ram;
static UINT8 dakkochn_mux_data;


static MACHINE_RESET( system1 )
{
	system1_define_background_memory(system1_BACKGROUND_MEMORY_SINGLE);
}

static MACHINE_RESET( system1_banked )
{
	MACHINE_RESET_CALL(system1);
	memory_configure_bank(machine, 1, 0, 4, memory_region(machine, "maincpu") + 0x10000, 0x4000);
}

static MACHINE_RESET( dakkochn )
{
	MACHINE_RESET_CALL(system1);
	memory_configure_bank(machine, 1, 0, 4, memory_region(machine, "maincpu") + 0x10000, 0x4000);
	dakkochn_mux_data = 0;
}

static MACHINE_RESET( wbml )
{
	system1_define_background_memory(system1_BACKGROUND_MEMORY_BANKED);
	memory_configure_bank(machine, 1, 0, 4, memory_region(machine, "maincpu") + 0x10000, 0x4000);
}

// nobb - these ports are used for some kind of replacement protection system used by the bootleg
static int nobb_inport16_step,nobb_inport17_step,nobb_inport23_step;

static READ8_HANDLER( nobb_inport16_r )
{
//  logerror("IN  $16 : pc = %04x - data = %02x\n",cpu_get_pc(space->cpu),nobb_inport16_step);
	return(nobb_inport16_step);
}

static READ8_HANDLER( nobb_inport1c_r )
{
//  logerror("IN  $1c : pc = %04x - data = 0x80\n",cpu_get_pc(space->cpu));
	return(0x80);	// infinite loop (at 0x0fb3) until bit 7 is set
}

static READ8_HANDLER( nobb_inport22_r )
{
//  logerror("IN  $22 : pc = %04x - data = %02x\n",cpu_get_pc(space->cpu),nobb_inport17_step);
	return(nobb_inport17_step);
}

static READ8_HANDLER( nobb_inport23_r )
{
//  logerror("IN  $23 : pc = %04x - step = %02x\n",cpu_get_pc(space->cpu),nobb_inport23_step);
	return(nobb_inport23_step);
}

static WRITE8_HANDLER( nobb_outport16_w )
{
//  logerror("OUT $16 : pc = %04x - data = %02x\n",cpu_get_pc(space->cpu),data);
	nobb_inport16_step = data;
}

static WRITE8_HANDLER( nobb_outport17_w )
{
//  logerror("OUT $17 : pc = %04x - data = %02x\n",cpu_get_pc(space->cpu),data);
	nobb_inport17_step = data;
}

static WRITE8_HANDLER( nobb_outport24_w )
{
//  logerror("OUT $24 : pc = %04x - data = %02x\n",cpu_get_pc(space->cpu),data);
	nobb_inport23_step = data;
}

static WRITE8_HANDLER( hvymetal_videomode_w )
{
	memory_set_bank(space->machine, 1, ((data & 0x04)>>2) + ((data & 0x40)>>5));
	system1_videomode_w(space, 0, data);
}

static WRITE8_HANDLER( brain_videomode_w )
{
	memory_set_bank(space->machine, 1, ((data & 0x04)>>2) + ((data & 0x40)>>5));
	system1_videomode_w(space, 0, data);
}

static WRITE8_HANDLER( chplft_videomode_w )
{
	memory_set_bank(space->machine, 1, (data & 0x0c)>>2);
	system1_videomode_w(space, 0, data);
}


static WRITE8_HANDLER( system1_soundport_w )
{
	soundlatch_w(space,0,data);
	cpu_set_input_line(space->machine->cpu[1],INPUT_LINE_NMI,PULSE_LINE);
	/* spin for a while to let the Z80 read the command (fixes hanging sound in Regulus) */
	cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(50));
}

/* protection values from real hardware, these were verified to be the same on the title
   screen and in the first level... they're all jumps that the MCU appears to put in RAM
   at some point */
static const int shtngtab[]=
{
	0xC3,0xC1,0x39,
	0xC3,0x6F,0x0A,
	0xC3,0x56,0x39,
	0xC3,0x57,0x0C,
	0xC3,0xE2,0x0B,
	0xC3,0x68,0x03,
	0xC3,0xF1,0x06,
	0xC3,0xCA,0x06,
	0xC3,0xC4,0x06,
	0xC3,0xD6,0x07,
	0xC3,0x89,0x13,
	0xC3,0x75,0x13,
	0xC3,0x9F,0x13,
	0xC3,0xFF,0x38,
	0xC3,0x60,0x13,
	0xC3,0x62,0x00,
	0xC3,0x39,0x04,
	-1
};

static WRITE8_HANDLER(mcuenable_hack_w)
{
	//in fact it's gun feedback write, not mcu related
	int i=0;
	while(shtngtab[i]>=0)
	{
		system1_ram[i+0x40]=shtngtab[i];
		i++;
	}

	system1_ram[0x2ff]=0x49; // I ?
	system1_ram[0x3ff]=0x54; // T ?
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM)	// ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xefbd, 0xefbd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_y)
	AM_RANGE(0xeffc, 0xeffd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_x)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)

	/* these are required to make various games work (teddybb, imsorry, raflesia) */
	/* these may actually be backed by RAM, or may be mirrors of other RAM areas */
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xef00, 0xefbc) AM_WRITE(SMH_RAM)
	AM_RANGE(0xefbe, 0xeffb) AM_WRITE(SMH_RAM)
	AM_RANGE(0xeffe, 0xefff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xf400, 0xf7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xfc00, 0xffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blckgalb_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM) // ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xe000, 0xe6ff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xe7bd, 0xe7bd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_y)	// ???
	AM_RANGE(0xe7c0, 0xe7c1) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_x)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( brain_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK1)
	AM_RANGE(0xc000, 0xffff) AM_READ(SMH_RAM) AM_BASE(&system1_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wbml_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK1)
	AM_RANGE(0xc000, 0xdfff) AM_READ(SMH_RAM) AM_BASE(&system1_ram)
	AM_RANGE(0xe000, 0xefff) AM_READ(wbml_paged_videoram_r)
	AM_RANGE(0xf000, 0xf3ff) AM_READ(SMH_RAM)
	AM_RANGE(0xf800, 0xfbff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wbml_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM) // ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM) // mirror?
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(wbml_paged_videoram_w)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( chplft_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM) // ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe7c0, 0xe7ff) AM_WRITE(choplifter_scroll_x_w) AM_BASE(&system1_scrollx_ram)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)

	 /* needed for P.O.S.T. */
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xef00, 0xefff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gardiab_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM) // ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe7bd, 0xe7bd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_y)	// ???
	AM_RANGE(0xe7c0, 0xe7c1) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_x)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)

	 /* needed for P.O.S.T. */
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xef00, 0xefff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dakkochn_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(SMH_RAM) // ROM
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe7bb, 0xe7bb) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_y) //TODO: hook it up
	AM_RANGE(0xe7c0, 0xe7ff) AM_WRITE(choplifter_scroll_x_w) AM_BASE(&system1_scrollx_ram)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xf800, 0xfbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)

	 /* needed for P.O.S.T. */
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xef00, 0xefff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nobo_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xc3ff) AM_WRITE(system1_background_collisionram_w) AM_BASE(&system1_background_collisionram)
	AM_RANGE(0xc800, 0xcbff) AM_WRITE(system1_sprites_collisionram_w) AM_BASE(&system1_sprites_collisionram)
	AM_RANGE(0xd000, 0xd1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xd800, 0xddff) AM_WRITE(system1_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(system1_backgroundram_w) AM_BASE(&system1_backgroundram) AM_SIZE(&system1_backgroundram_size)
	AM_RANGE(0xe800, 0xeeff) AM_WRITE(SMH_RAM) AM_BASE(&system1_videoram) AM_SIZE(&system1_videoram_size)
	AM_RANGE(0xefbd, 0xefbd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_y)
	AM_RANGE(0xeffc, 0xeffd) AM_WRITE(SMH_RAM) AM_BASE(&system1_scroll_x)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(SMH_RAM)

	/* These addresses are written during P.O.S.T. but don't seem to be used after */
	AM_RANGE(0xc400, 0xc7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xcc00, 0xcfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xd200, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xde00, 0xdfff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xef00, 0xefbc) AM_WRITE(SMH_RAM)
	AM_RANGE(0xefbe, 0xeffb) AM_WRITE(SMH_RAM)
	AM_RANGE(0xeffe, 0xefff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0e, 0x0e) AM_READ_PORT("DSW2")	/* DIP2 blckgalb reads it from here */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")	/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x11, 0x11) AM_READ_PORT("DSW2")	/* DIP2 ... blockgal */
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)	/* sound commands */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, system1_videomode_w)	/* video control and (in some games) bank switching */
	AM_RANGE(0x18, 0x18) AM_WRITE(system1_soundport_w)	/* mirror address */
	AM_RANGE(0x19, 0x19) AM_READWRITE(system1_videomode_r, system1_videomode_w)    /* mirror address */
ADDRESS_MAP_END

static ADDRESS_MAP_START( wbml_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")	/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)    /* sound commands */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, chplft_videomode_w)
	AM_RANGE(0x16, 0x16) AM_READWRITE(wbml_videoram_bank_latch_r, wbml_videoram_bank_latch_w)
	AM_RANGE(0x19, 0x19) AM_READ(system1_videomode_r)  /* mirror address */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sht_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
/*  AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") */
/*  AM_RANGE(0x04, 0x04) AM_READ_PORT("P2") */
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1") AM_WRITE(mcuenable_hack_w)
												/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x12, 0x12) AM_READ_PORT("TRIGGER")
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)	/* sound commands */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, chplft_videomode_w)
	AM_RANGE(0x16, 0x16) AM_READWRITE(wbml_videoram_bank_latch_r, wbml_videoram_bank_latch_w)
	AM_RANGE(0x18, 0x18) AM_READ_PORT("18")				/* ?? */
	AM_RANGE(0x19, 0x19) AM_READ(system1_videomode_r)	/* mirror address */
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("GUNX")
	AM_RANGE(0x1d, 0x1d) AM_READ_PORT("GUNY")
ADDRESS_MAP_END

static ADDRESS_MAP_START( nob_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")		/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")		/* DIP1 some games read it from here... */
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)	/* sound commands ? */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, brain_videomode_w)	/* video control + bank switching */
ADDRESS_MAP_END

static ADDRESS_MAP_START( nobb_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")		/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")		/* DIP1 some games read it from here... */
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)	/* sound commands ? */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, brain_videomode_w)	/* video control + bank switching */

	/* protection on the bootleg */
	AM_RANGE(0x16, 0x16) AM_READWRITE(nobb_inport16_r, nobb_outport16_w)	/* Used - check code at 0x05cb */
	AM_RANGE(0x17, 0x17) AM_WRITE(nobb_outport17_w)		/* Not handled in emul. of other SS1/2 games */
	AM_RANGE(0x1c, 0x1c) AM_READ(nobb_inport1c_r)		/* Shouldn't be called ! */
	AM_RANGE(0x22, 0x22) AM_READ(nobb_inport22_r)		/* Used - check code at 0xb253 */
	AM_RANGE(0x23, 0x23) AM_READ(nobb_inport23_r)		/* Used - check code at 0xb275 and 0xb283 */
	AM_RANGE(0x24, 0x24) AM_WRITE(nobb_outport24_w)			/* Used - check code at 0xb24e and 0xb307 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( hvymetal_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")	/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x15, 0x15) AM_READ(system1_videomode_r)
	AM_RANGE(0x16, 0x16) AM_READ(wbml_videoram_bank_latch_r)
	AM_RANGE(0x18, 0x18) AM_WRITE(system1_soundport_w)    /* sound commands */
	AM_RANGE(0x19, 0x19) AM_READWRITE(system1_videomode_r, hvymetal_videomode_w)  /* mirror address */
ADDRESS_MAP_END

static ADDRESS_MAP_START( brain_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0e, 0x0e) AM_READ_PORT("DSW2")	/* DIP2 blckgalb reads it from here */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")	/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x11, 0x11) AM_READ_PORT("DSW2")	/* DIP2 ... blockgal */
	AM_RANGE(0x15, 0x15) AM_READ(system1_videomode_r)
	AM_RANGE(0x18, 0x18) AM_WRITE(system1_soundport_w)	/* sound commands */
	AM_RANGE(0x19, 0x19) AM_READWRITE(system1_videomode_r, brain_videomode_w)	/* mirror address */
ADDRESS_MAP_END

static ADDRESS_MAP_START( chplft_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("P2")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSW2")	/* DIP2 */
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("DSW1")	/* DIP1 some games read it from here... */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")	/* DIP1 ... and some others from here but there are games which check BOTH! */
	AM_RANGE(0x14, 0x14) AM_WRITE(system1_soundport_w)    /* sound commands */
	AM_RANGE(0x15, 0x15) AM_READWRITE(system1_videomode_r, chplft_videomode_w)
	AM_RANGE(0x16, 0x16) AM_READ(wbml_videoram_bank_latch_r)
	AM_RANGE(0x19, 0x19) AM_READ(system1_videomode_r)  /* mirror address */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x1ffc) AM_DEVWRITE(SOUND, "sn1", sn76496_w)    /* Choplifter writes to the four addresses */
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x1ffc) AM_DEVWRITE(SOUND, "sn2", sn76496_w)    /* in sequence */
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1fff) AM_READ(soundlatch_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system1_generic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
/* If you don't like the description, feel free to change it */
	PORT_DIPNAME( 0x80, 0x80, "SW 0 Read From" )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, "Port $0D" )
	PORT_DIPSETTING(	0x00, "Port $10" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
/*  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) Not allowed by mame coinage sorting, but valid */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
/*  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) Not allowed by mame coinage sorting, but valid */
INPUT_PORTS_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( starjack )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR (Bonus_Life ) )	PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(	0x38, "Every 20k" )
	PORT_DIPSETTING(	0x28, "Every 30k" )
	PORT_DIPSETTING(	0x18, "Every 40k" )
	PORT_DIPSETTING(	0x08, "Every 50k" )
	PORT_DIPSETTING(	0x30, "20k, then every 30k" )
	PORT_DIPSETTING(	0x20, "30k, then every 40k" )
	PORT_DIPSETTING(	0x10, "40k, then every 50k" )
	PORT_DIPSETTING(	0x00, "50k, then every 60k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(	0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( starjacs )
	PORT_INCLUDE( starjack )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x08, 0x08, "Ship" )			PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Single ) )
	PORT_DIPSETTING(	0x00, "Multi" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "30k, then every 40k" )
	PORT_DIPSETTING(	0x20, "40k, then every 50k" )
	PORT_DIPSETTING(	0x10, "50k, then every 60k" )
	PORT_DIPSETTING(	0x00, "60k, then every 70k" )
INPUT_PORTS_END

static INPUT_PORTS_START( regulus )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

/* Same as 'regulus', but no DEF_STR( Allow_Continue ) Dip Switch */
static INPUT_PORTS_START( reguluso )
	PORT_INCLUDE( regulus )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( upndown )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 2 */

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 2 */

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(	0x38, "10000" )
	PORT_DIPSETTING(	0x30, "20000" )
	PORT_DIPSETTING(	0x28, "30000" )
	PORT_DIPSETTING(	0x20, "40000" )
	PORT_DIPSETTING(	0x18, "50000" )
	PORT_DIPSETTING(	0x10, "60000" )
	PORT_DIPSETTING(	0x08, "70000" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(	0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mrviking )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, "Maximum Credits" )		PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, "9" )
	PORT_DIPSETTING(	0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "10k, 30k then every 30k" )
	PORT_DIPSETTING(	0x20, "20k, 40k then every 30k" )
	PORT_DIPSETTING(	0x10, "30k, then every 30k" )
	PORT_DIPSETTING(	0x00, "40k, then every 30k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

/* Same as 'mrviking', but no "Maximum Credits" Dip Switch and "Difficulty" Dip Switch is
   handled by bit 7 instead of bit 6 (so bit 6 is unused) */
static INPUT_PORTS_START( mrvikngj )
	PORT_INCLUDE( mrviking )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( swat )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(	0x38, "30000" )
	PORT_DIPSETTING(	0x30, "40000" )
	PORT_DIPSETTING(	0x28, "50000" )
	PORT_DIPSETTING(	0x20, "60000" )
	PORT_DIPSETTING(	0x18, "70000" )
	PORT_DIPSETTING(	0x10, "80000" )
	PORT_DIPSETTING(	0x08, "90000" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( flicky )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 2 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* only 2way inputs */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* only 2way inputs */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 2 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* only 2way inputs */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* only 2way inputs */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "30000 80000 160000" )
	PORT_DIPSETTING(	0x20, "30000 100000 200000" )
	PORT_DIPSETTING(	0x10, "40000 120000 240000" )
	PORT_DIPSETTING(	0x00, "40000 140000 280000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wmatch )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )			/* TURN P1 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL	/* TURN P2 */

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )			PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x08, "Fast" )
	PORT_DIPSETTING(	0x04, "Faster" )
	PORT_DIPSETTING(	0x00, "Fastest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bullfgt )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "30000" )
	PORT_DIPSETTING(	0x20, "50000" )
	PORT_DIPSETTING(	0x10, "70000" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spatter )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "40k, 120k and 480k" )
	PORT_DIPSETTING(	0x20, "50k and 200k" )
	PORT_DIPSETTING(	0x10, "100k only" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Reset Timer/Objects On Life Loss" )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pitfall2 )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "20000 50000" )
	PORT_DIPSETTING(	0x00, "30000 70000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Time" )			PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x00, "2 Minutes" )
	PORT_DIPSETTING(	0x40, "3 Minutes" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitfallu )
	PORT_INCLUDE( pitfall2 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x18, 0x18, "Starting Stage" )		PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(	0x18, "1" )
	PORT_DIPSETTING(	0x10, "2" )
	PORT_DIPSETTING(	0x08, "3" )
	PORT_DIPSETTING(	0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( seganinj )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x00, "240" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "20k 70k 120k 170k" )
	PORT_DIPSETTING(	0x00, "50k 100k 150k 200k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( imsorry )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0C, 0x0C, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0C, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "30000" )
	PORT_DIPSETTING(	0x20, "40000" )
	PORT_DIPSETTING(	0x10, "50000" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( teddybb )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x00, "252" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "100k 400k" )
	PORT_DIPSETTING(	0x20, "200k 600k" )
	PORT_DIPSETTING(	0x10, "400k 800k" )
	PORT_DIPSETTING(	0x00, "600k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hvymetal )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "50000 100000" )
	PORT_DIPSETTING(	0x20, "60000 120000" )
	PORT_DIPSETTING(	0x10, "70000 150000" )
	PORT_DIPSETTING(	0x00, "100000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( myhero )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "30000" )
	PORT_DIPSETTING(	0x20, "50000" )
	PORT_DIPSETTING(	0x10, "70000" )
	PORT_DIPSETTING(	0x00, "90000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 4dwarrio )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(	0x38, "30000" )
	PORT_DIPSETTING(	0x30, "40000" )
	PORT_DIPSETTING(	0x28, "50000" )
	PORT_DIPSETTING(	0x20, "60000" )
	PORT_DIPSETTING(	0x18, "70000" )
	PORT_DIPSETTING(	0x10, "80000" )
	PORT_DIPSETTING(	0x08, "90000" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( brain )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gardia )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "5k, 20k and 30k" )
	PORT_DIPSETTING(	0x20, "10k, 25k and 50k" )
	PORT_DIPSETTING(	0x10, "15k, 30k and 60k" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	 /* Manual states "Always On" */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( raflesia )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(	0x30, "20k, 70k and 120k" )
	PORT_DIPSETTING(	0x20, "30k, 80k and 150k" )
	PORT_DIPSETTING(	0x10, "50k, 100k and 200k" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wboy )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(	0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( wboy3 )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("DSW1")  /* DSW0 */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "1" )
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/* same as wboy, additional Energy Consumption switch */
static INPUT_PORTS_START( wbdeluxe )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Has to be 0 otherwise the game resets */
							/* if you die after level 1. */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Energy Consumption" )	PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x00, "Slow" )
	PORT_DIPSETTING(	0x80, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( wboyu )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(	0x00, "2" )
	PORT_DIPSETTING(	0x06, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x02, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(	0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Mode" )			PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(	0xc0, "Normal Game" )
	PORT_DIPSETTING(	0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(	0x40, "Test Mode" )
	PORT_DIPSETTING(	0x00, "Endless Game" )
INPUT_PORTS_END

static INPUT_PORTS_START( nob )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )				// shot
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )				// fly

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL		// shot
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL		// fly

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(	0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(	0x02, "2" )
	PORT_DIPSETTING(	0x03, "3" )
	PORT_DIPSETTING(	0x01, "5" )
	PORT_DIPSETTING(	0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x08, "40k, 80k, 120k and 160k" )
	PORT_DIPSETTING(	0x0c, "50k, 100k and 150k" )
	PORT_DIPSETTING(	0x04, "60k, 120k and 180k" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( chplft )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "20k 70k 120k 170k" )
	PORT_DIPSETTING(	0x00, "50k 100k 150k 200k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Easy ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( shtngmst )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )			PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TRIGGER")  /* trigger is in here */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("GUNX") /* 1c */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY") /* 1d */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("18") /* 18 */
	/* what is this? check the game code... */
	PORT_DIPNAME( 0x01, 0x01, "port 18" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( wboysys2 )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* down - unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* up - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(	0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

/* Notes about the bootleg version (as this is the only "working" one) :

Coinage is almost the same as the other Sega games.

However, when you set DSW1 to 00, you enter a pseudo "free play" mode :

  - When you insert a coin, or press the "Service" button, you are given 2 credits
    and this number is NEVER incremented nor decremented
  - You are given 3 lives at start and this number is NEVER decremented
    (it can however be incremented depending on the "Bonus Life" Dip Switch)

If only one nibble is set to 0, it will give a standard 1C_1C.


There is an ingame bug with the "Bonus Life" Dip Switch, but I don't know if it's only
a "feature" of the bootleg :

  - Check routine at 0x2366, and you'll notice that 0xc02d (player 1) and 0xc02e (player 2)
    act like a "pointer" to the bonus life table (0x5ab6 or 0x5abb)
  - Once you get enough points, 1 life is added, and the pointer is incremented
  - There is however NO test to the limit of this pointer ! So, once you've got your 5th
    extra life at 150k, the pointed value will be 3 (= extra life at 30k), and as your
    score is over this value, you'll be given another extra life ... and so on ...


Bits 2 and 6 of DSW0 aren't tested in the game (I can't tell about the "test mode")


Useful addresses:

  - 0xc040 : credits (0x00-0x09)
  - 0xc019 : player 1 lives
  - 0xc021 : player 2 lives
  - 0xc018 : player 1 level (0x01-0x14)
  - 0xc020 : player 2 level (0x01-0x14)
  - 0xc02d : player 1 bonus life "pointer"
  - 0xc02e : player 1 bonus life "pointer"
  - 0xc01a - 0xc01c : player 1 score (BCD coded - LSB first)
  - 0xc022 - 0xc024 : player 1 score (BCD coded - LSB first)

  - 0xc050 : when == 01, "free play" mode
  - 0xc00c : when == 01, you end the level

*/

static INPUT_PORTS_START( blockgal )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "10k 30k 60k 100k 150k" )
	PORT_DIPSETTING(	0x00, "30k 50k 100k 200k 300k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( tokisens )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x00, "1" )
	PORT_DIPSETTING(	0x08, "2" )
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wbml )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x0c, "4" )
	PORT_DIPSETTING(	0x08, "5" )
/* 0x00 gives 4 lives */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "30000 100000 200000" )
	PORT_DIPSETTING(	0x00, "50000 150000 250000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Test Mode" )			PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dakkochn )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) //start 1 & 2 not connected.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	/*TODO: Dip-Switches */
	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x0c, "4" )
	PORT_DIPSETTING(	0x08, "5" )
/* 0x00 gives 4 lives */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x10, "30000 100000 200000" )
	PORT_DIPSETTING(	0x00, "50000 150000 250000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Test Mode" )			PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*

 To enter Test Mode all DIP Switches in DSW1 must be ON (example Difficulty = Easy)
 To get Infinite Lives all DIP Switches in DSW0 must be ON

*/
static INPUT_PORTS_START( ufosensi )
	PORT_INCLUDE( chplft )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(	0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(	0x0c, "3" )
	PORT_DIPSETTING(	0x04, "4" )
//  PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(	0x20, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability" )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( system1 )
	/* sprites use colors 0-511, but are not defined here */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 512, 128 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( system1 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_4MHz)	/* My Hero has 2 OSCs 8 & 20 MHz (Cabbe Info) */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(io_map,0)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, XTAL_4MHz)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,4)		 /* NMIs are caused by the main CPU */

	MDRV_MACHINE_RESET(system1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)// hw collisions

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(system1)
	MDRV_PALETTE_LENGTH(1536)

	MDRV_PALETTE_INIT(system1)
	MDRV_VIDEO_START(system1)
	MDRV_VIDEO_UPDATE(system1)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

   MDRV_SOUND_ADD("sn1", SN76489A, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

   MDRV_SOUND_ADD("sn2", SN76489A, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/* driver with reduced visible area for scrolling games */
static MACHINE_DRIVER_START( small )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0*8+8, 32*8-1-8, 0*8, 28*8-1)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pitfall2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_REPLACE( "maincpu", Z80, 3600000 )/* should be 4 MHz but that value makes the title screen disappear */

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( hvymetal )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,writemem)
	MDRV_CPU_IO_MAP(hvymetal_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( chplft )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,chplft_writemem)
	MDRV_CPU_IO_MAP(chplft_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

	/* video hardware */
	MDRV_VIDEO_UPDATE(choplifter)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gardiab )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( chplft )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,gardiab_writemem)
	MDRV_CPU_IO_MAP(chplft_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

	/* video hardware */
	MDRV_VIDEO_UPDATE(system1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dakkochn )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( chplft )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,dakkochn_writemem)
	MDRV_CPU_IO_MAP(wbml_io_map,0)

	/* video hardware */
	MDRV_VIDEO_START(wbml)
	MDRV_MACHINE_RESET(dakkochn)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( brain )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,writemem)
	MDRV_CPU_IO_MAP(brain_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wbml )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(wbml_readmem,wbml_writemem)
	MDRV_CPU_IO_MAP(wbml_io_map,0)

	MDRV_MACHINE_RESET(wbml)

	/* video hardware */
	MDRV_VIDEO_START(wbml)
	MDRV_VIDEO_UPDATE(wbml)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( shtngmst )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(brain_readmem,chplft_writemem)
	MDRV_CPU_IO_MAP(sht_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

	/* video hardware - same as small - left / right 8 pixels clipped */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0*8+8, 32*8-1-8, 0*8, 28*8-1)

	MDRV_VIDEO_UPDATE(choplifter)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( nob )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_REPLACE( "maincpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(brain_readmem,nobo_writemem)
	MDRV_CPU_IO_MAP(nob_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 0*8, 28*8-1)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( nobb )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_REPLACE( "maincpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(brain_readmem,nobo_writemem)
	MDRV_CPU_IO_MAP(nobb_io_map,0)

	MDRV_MACHINE_RESET(system1_banked)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 0*8, 28*8-1)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( blckgalb )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( system1 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(readmem,blckgalb_writemem)

	/* video hardware */
	MDRV_VIDEO_UPDATE(blockgal)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ufosensi )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( wbml )

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 27*8-1)

	/* video hardware */
	MDRV_VIDEO_UPDATE(ufosensi)

MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/* Since the standard System 1 PROM has part # 5317, Star Jacker, whose first */
/* ROM is #5318, is probably the first or second System 1 game produced */
ROM_START( starjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5320b.129",	0x0000, 0x2000, CRC(7ab72ecd) SHA1(28d3f87851cccc94a86eb0217893de0baf8e62fd) )
	ROM_LOAD( "epr5321a.130",	0x2000, 0x2000, CRC(38b99050) SHA1(79fd23bb7db577d1dbf1b50503085eccdd17b98c) )
	ROM_LOAD( "epr5322a.131",	0x4000, 0x2000, CRC(103a595b) SHA1(6bb8a063279c93341ff472351b79c92795845f74) )
	ROM_LOAD( "epr-5323.132",	0x6000, 0x2000, CRC(46af0d58) SHA1(6cfa288e28e3b415db5ef3cef8e8849259234af9) )
	ROM_LOAD( "epr-5324.133",	0x8000, 0x2000, CRC(1e89efe2) SHA1(d36ef8f176d5e44884d3d0b9af81be6f7fbd0cde) )
	ROM_LOAD( "epr-5325.134",	0xa000, 0x2000, CRC(d6e379a1) SHA1(27362b3e10d9d4235647eadb9cd1404700a8fb49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5332.3",		0x0000, 0x2000, CRC(7a72ab3d) SHA1(4a6ad09949a438562d7043532d628cefdbb5a2fe) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5331.82",	0x0000, 0x2000, CRC(251d898f) SHA1(353067a75d583d5f53ce2f473b52a35dd912639f) )
	ROM_LOAD( "epr-5330.65",	0x2000, 0x2000, CRC(eb048745) SHA1(a2e90d20a07608f43273e84d1eb22f195b19626c) )
	ROM_LOAD( "epr-5329.81",	0x4000, 0x2000, CRC(3e8bcaed) SHA1(6d19543427b9c4d8d8f5ea0872cdf8cc4fe5018c) )
	ROM_LOAD( "epr-5328.64",	0x6000, 0x2000, CRC(9ed7849f) SHA1(cc30d144ff70539bbc82c848c154e156a33b638c) )
	ROM_LOAD( "epr-5327.80",	0x8000, 0x2000, CRC(79e92cb1) SHA1(03124ce123684b8469cf42be6ed5f0fffa64c480) )
	ROM_LOAD( "epr-5326.63",	0xa000, 0x2000, CRC(ba7e2b47) SHA1(bc7528456fe8dee9aa21212aa996fc347c5d55b4) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5318.86",	0x0000, 0x4000, CRC(6f2e1fd3) SHA1(326d538551245fce67d0fdba85975e27093b7a93) )
	ROM_LOAD( "epr-5319.93",	0x4000, 0x4000, CRC(ebee4999) SHA1(bb331be270dc1da63699533d9f02d73ce28f1bc5) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( starjacs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1_ic29.129",	0x0000, 0x2000, CRC(59a22a1f) SHA1(4827b537f8df04429ed53c2119c67a32efcf04a2) )
	ROM_LOAD( "a1_ic30.130",	0x2000, 0x2000, CRC(7f4597dc) SHA1(7574853fc2e38880f8493cf628100a890f7aa7dc) )
	ROM_LOAD( "a1_ic31.131",	0x4000, 0x2000, CRC(6074c046) SHA1(5d2bd679d6a13a6c3b203662ce5496b801383db9) )
	ROM_LOAD( "a1_ic32.132",	0x6000, 0x2000, CRC(1c48a3fa) SHA1(4a2e7798600bc4a5fd68533083547212d148d347) )
	ROM_LOAD( "a1_ic33.133",	0x8000, 0x2000, CRC(7598bd51) SHA1(0c18b83dc7874aefcd94593ffbe2b50cc0329367) )
	ROM_LOAD( "a1_ic34.134",	0xa000, 0x2000, CRC(f66fa604) SHA1(d7a81920217fcf7a687ba5e2d10abad5c78085d2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5332.3",		0x0000, 0x2000, CRC(7a72ab3d) SHA1(4a6ad09949a438562d7043532d628cefdbb5a2fe) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5331.82",	0x0000, 0x2000, CRC(251d898f) SHA1(353067a75d583d5f53ce2f473b52a35dd912639f) )
	ROM_LOAD( "a1_ic65.65",		0x2000, 0x2000, CRC(0ab1893c) SHA1(97877f5d8be7a7b80bbf9fe8dae2acd47c411d64) )
	ROM_LOAD( "epr-5329.81",	0x4000, 0x2000, CRC(3e8bcaed) SHA1(6d19543427b9c4d8d8f5ea0872cdf8cc4fe5018c) )
	ROM_LOAD( "a1_ic64.64",		0x6000, 0x2000, CRC(7f628ae6) SHA1(f859a505b543382b42a478c8ae5cd90f3ea2bc2c) )
	ROM_LOAD( "epr-5327.80",	0x8000, 0x2000, CRC(79e92cb1) SHA1(03124ce123684b8469cf42be6ed5f0fffa64c480) )
	ROM_LOAD( "a1_ic63.63",		0xa000, 0x2000, CRC(5bcb253e) SHA1(8c34a8377344940bcfb2495bfda3ffc6794f261b) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "a1_ic86.86",   0x0000, 0x4000, CRC(6f2e1fd3) SHA1(326d538551245fce67d0fdba85975e27093b7a93) )
	ROM_LOAD( "a1_ic93.93",   0x4000, 0x4000, CRC(07987244) SHA1(8468b95684b1f62c6de6af1b1d405bfb333e4e26) )

	ROM_REGION( 0x0100, "user1", 0 )	/* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( regulus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5640a.129",	0x0000, 0x2000, CRC(dafb1528) SHA1(9140c5507bd931df3f8ef8d2910bc74f737b1a5a) ) /* encrypted */
	ROM_LOAD( "epr-5641a.130",	0x2000, 0x2000, CRC(0fcc850e) SHA1(d2d6b06bf1e2dc404aa5451cc9f1b919fb5be0f5) ) /* encrypted */
	ROM_LOAD( "epr-5642a.131",	0x4000, 0x2000, CRC(4feffa17) SHA1(9d9f4227c4e60a5cc53c369e7c9ce59ea8df3553) ) /* encrypted */
	ROM_LOAD( "epr-5643a.132",	0x6000, 0x2000, CRC(b8ac7eb4) SHA1(f96bcde021060a8c1c5270b73487e24d1893e8e5) ) /* encrypted */
	ROM_LOAD( "epr-5644.133",	0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5645a.134",	0xa000, 0x2000, CRC(6b4bf77c) SHA1(0200efb58b85a6873db44ffa70c3c14dbca958a6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",		0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5651.82",	0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",	0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",	0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",	0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",	0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) )
	ROM_LOAD( "epr-5646.63",	0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5638.86",	0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",	0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( reguluso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5640.129",	0x0000, 0x2000, CRC(8324d0d4) SHA1(204713938bc85e8b62c161d8ae00d087ecc9089c) ) /* encrypted */
	ROM_LOAD( "epr-5641.130",	0x2000, 0x2000, CRC(0a09f5c7) SHA1(0d45bff29442908b9f4111c89baea0326f0a9ec9) ) /* encrypted */
	ROM_LOAD( "epr-5642.131",	0x4000, 0x2000, CRC(ff27b2f6) SHA1(fe294a53deffe2d46afa444fdae213e9d8763316) ) /* encrypted */
	ROM_LOAD( "epr-5643.132",	0x6000, 0x2000, CRC(0d867df0) SHA1(adccc78072c0772ec20c0178a0be3426759900bf) ) /* encrypted */
	ROM_LOAD( "epr-5644.133",	0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5645.134",	0xa000, 0x2000, CRC(57a2b4b4) SHA1(9de8f5948c7993f1b6d8bf7032f7fc3d9dff5c77) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",		0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5651.82",	0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",	0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",	0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",	0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",	0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) )
	ROM_LOAD( "epr-5646.63",	0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5638.86",	0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",	0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( regulusu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5950.129",	0x0000, 0x2000, CRC(3b047b67) SHA1(0164cb919a50013f23568f59caff19ff2d0bf11f) )
	ROM_LOAD( "epr-5951.130",	0x2000, 0x2000, CRC(d66453ab) SHA1(9e339c716c646bd02bedbe27096b75f633554e7c) )
	ROM_LOAD( "epr-5952.131",	0x4000, 0x2000, CRC(f3d0158a) SHA1(9b6d8b2e0a0bec45bfbb9f8ccc728e18e909685f) )
	ROM_LOAD( "epr-5953.132",	0x6000, 0x2000, CRC(a9ad4f44) SHA1(1e051595aff34db06186542bcfc3849bc88eb5d4) )
	ROM_LOAD( "epr-5644.133",	0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5955.134",	0xa000, 0x2000, CRC(65ddb2a3) SHA1(4f94eaac900da5ca512289e2339776b1139e03e1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",		0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5651.82",	0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",	0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",	0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",	0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",	0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) )
	ROM_LOAD( "epr-5646.63",	0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5638.86",	0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",	0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( upndown )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5516a.129",	0x0000, 0x2000, CRC(038c82da) SHA1(b7f403068ed9f97a4b960fb8863615892bb770ed) ) /* encrypted */
	ROM_LOAD( "epr5517a.130",	0x2000, 0x2000, CRC(6930e1de) SHA1(8a5564c76e1fd20c8e5d95e5f538980e13c41744) ) /* encrypted */
	ROM_LOAD( "epr-5518.131",	0x4000, 0x2000, CRC(2a370c99) SHA1(3d1b2f1cf0d5d2d6369a33e5b3b460a3113d6a3e) ) /* encrypted */
	ROM_LOAD( "epr-5519.132",	0x6000, 0x2000, CRC(9d664a58) SHA1(84f2d012dac63e8d0de3935a76f5202539423a74) ) /* encrypted */
	ROM_LOAD( "epr-5520.133",	0x8000, 0x2000, CRC(208dfbdf) SHA1(eff0c91ce6c2c1f6e191bcbf9ae83dd377cbb408) )
	ROM_LOAD( "epr-5521.134",	0xa000, 0x2000, CRC(e7b8d87a) SHA1(3419318bf6d87b902433bfe3b92baf5e5bad7df3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5535.3",		0x0000, 0x2000, CRC(cf4e4c45) SHA1(d14a204a9966d37f4b9f3ea4c1d371c9d04e750a) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5527.82",	0x0000, 0x2000, CRC(b2d616f1) SHA1(c079136a5d73e1d55ddbad6efb5e7067d0ff412b) )
	ROM_LOAD( "epr-5526.65",	0x2000, 0x2000, CRC(8a8b33c2) SHA1(db796d5c4ab3f749287133eaf05818f89dc2afb7) )
	ROM_LOAD( "epr-5525.81",	0x4000, 0x2000, CRC(e749c5ef) SHA1(2022cbd42ff0177cdd661bb00b1004459b6af83a) )
	ROM_LOAD( "epr-5524.64",	0x6000, 0x2000, CRC(8b886952) SHA1(6a9c909d10ccb03a8af6fa9d8067946d60b91592) )
	ROM_LOAD( "epr-5523.80",	0x8000, 0x2000, CRC(dede35d9) SHA1(6c47fa433e16ccc3fff9347a4fe8f0165d20a3d2) )
	ROM_LOAD( "epr-5522.63",	0xa000, 0x2000, CRC(5e6d9dff) SHA1(4f18274f5dc349b99b3daec517ccf5ccbb932d1c) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5514.86",	0x0000, 0x4000, CRC(fcc0a88b) SHA1(ca7db3df10deb6720096e6c50eddd9b74c47f0a0) )
	ROM_LOAD( "epr-5515.93",	0x4000, 0x4000, CRC(60908838) SHA1(aedff8ce07ab16942037e5aff212652e51c19e71) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( upndownu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5679.129",	0x0000, 0x2000, CRC(c4f2f9c2) SHA1(7904ffb46a2c3ef69b9784f343ff37d81bbee11d) )
	ROM_LOAD( "epr-5680.130",	0x2000, 0x2000, CRC(837f021c) SHA1(14cc846f03b71e0922689388a6757955cfd88bd8) )
	ROM_LOAD( "epr-5681.131",	0x4000, 0x2000, CRC(e1c7ff7e) SHA1(440dc8c18183612c32486c617f5d7f38fd804f0e) )
	ROM_LOAD( "epr-5682.132",	0x6000, 0x2000, CRC(4a5edc1e) SHA1(71f06d1c4a580fed07ad32c6d1f2d37d47ed95b1) )
	ROM_LOAD( "epr-5520.133",	0x8000, 0x2000, CRC(208dfbdf) SHA1(eff0c91ce6c2c1f6e191bcbf9ae83dd377cbb408) ) /* epr-5683.133 */
	ROM_LOAD( "epr-5684.133",	0xa000, 0x2000, CRC(32fa95da) SHA1(ebe87d28dde6b8356d40572e9f2cd35ec240075f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5528.3",		0x0000, 0x2000, CRC(00cd44ab) SHA1(7f5385aa0773681329a4759b0fa6f975e3de6755) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5527.82",	0x0000, 0x2000, CRC(b2d616f1) SHA1(c079136a5d73e1d55ddbad6efb5e7067d0ff412b) )
	ROM_LOAD( "epr-5526.65",	0x2000, 0x2000, CRC(8a8b33c2) SHA1(db796d5c4ab3f749287133eaf05818f89dc2afb7) )
	ROM_LOAD( "epr-5525.81",	0x4000, 0x2000, CRC(e749c5ef) SHA1(2022cbd42ff0177cdd661bb00b1004459b6af83a) )
	ROM_LOAD( "epr-5524.64",	0x6000, 0x2000, CRC(8b886952) SHA1(6a9c909d10ccb03a8af6fa9d8067946d60b91592) )
	ROM_LOAD( "epr-5523.80",	0x8000, 0x2000, CRC(dede35d9) SHA1(6c47fa433e16ccc3fff9347a4fe8f0165d20a3d2) )
	ROM_LOAD( "epr-5522.63",	0xa000, 0x2000, CRC(5e6d9dff) SHA1(4f18274f5dc349b99b3daec517ccf5ccbb932d1c) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5514.86",	0x0000, 0x4000, CRC(fcc0a88b) SHA1(ca7db3df10deb6720096e6c50eddd9b74c47f0a0) )
	ROM_LOAD( "epr-5515.93",	0x4000, 0x4000, CRC(60908838) SHA1(aedff8ce07ab16942037e5aff212652e51c19e71) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( mrviking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5873.129",	0x0000, 0x2000, CRC(14d21624) SHA1(70e185d03e782be908e6b5c6342cf6a7ebae618c) ) /* encrypted */
	ROM_LOAD( "epr-5874.130",	0x2000, 0x2000, CRC(6df7de87) SHA1(c2200e0c2f322a08af10e9c2e9191d1c595801a4) ) /* encrypted */
	ROM_LOAD( "epr-5875.131",	0x4000, 0x2000, CRC(ac226100) SHA1(11568db9fbca44013eeb0035c0a0a67d6dd18d00) ) /* encrypted */
	ROM_LOAD( "epr-5876.132",	0x6000, 0x2000, CRC(e77db1dc) SHA1(7b1aa19a16fb44f6c69cf053e2e10e5179416796) ) /* encrypted */
	ROM_LOAD( "epr-5755.133",	0x8000, 0x2000, CRC(edd62ae1) SHA1(9648f1ae3033c30ed8ab8d9c87b111756dab7b5e) )
	ROM_LOAD( "epr-5756.134",	0xa000, 0x2000, CRC(11974040) SHA1(a0904d19d06fb5ef5eb6da0dc4efe556bc29b33e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5763.3",		0x0000, 0x2000, CRC(d712280d) SHA1(8393dfb57d9af22b3280ecaef736b6f9d856dbee) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5762.82",	0x0000, 0x2000, CRC(4a91d08a) SHA1(4687ecc4061719fca5f85b2b290ebb7ced15ee5b) )
	ROM_LOAD( "epr-5761.65",	0x2000, 0x2000, CRC(f7d61b65) SHA1(a7a992f52406413e931945be60b35175f8aea6c2) )
	ROM_LOAD( "epr-5760.81",	0x4000, 0x2000, CRC(95045820) SHA1(d1848fc4f3d66603d0e8217373a37148aa2eeef5) )
	ROM_LOAD( "epr-5759.64",	0x6000, 0x2000, CRC(5f9bae4e) SHA1(6fff6086a96be6aa28bec05d1c94c257bb29ef1e) )
	ROM_LOAD( "epr-5758.80",	0x8000, 0x2000, CRC(808ee706) SHA1(d38ca7c6f36db6e35a3ce87bacdd70f293f23104) )
	ROM_LOAD( "epr-5757.63",	0xa000, 0x2000, CRC(480f7074) SHA1(c54a1fa02e312676658d7c5392a5a841bdb15d44) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5749.86",	0x0000, 0x4000, CRC(e24682cd) SHA1(3f626f3e5e2db486ccf727e9869ab488643b4a8c) )
	ROM_LOAD( "epr-5750.93",	0x4000, 0x4000, CRC(6564d1ad) SHA1(f246afee7e73bc30054b0e5dcb83fa0edd2d2164) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( mrvikngj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5751.129",	0x0000, 0x2000, CRC(ae97a4c5) SHA1(12edd757bd5b00d42ada1e10c43817f71cfe77dc) ) /* encrypted */
	ROM_LOAD( "epr-5752.130",	0x2000, 0x2000, CRC(d48e6726) SHA1(934b5e7568c85005c5ec40d75e49727a18562d50) ) /* encrypted */
	ROM_LOAD( "epr-5753.131",	0x4000, 0x2000, CRC(28c60887) SHA1(9673335586221336c3373f5d7c8ae4fc11cc4b7f) ) /* encrypted */
	ROM_LOAD( "epr-5754.132",	0x6000, 0x2000, CRC(1f47ed02) SHA1(d1147cd29fb342111f4f20a1d1d03263dce478f3) ) /* encrypted */
	ROM_LOAD( "epr-5755.133",	0x8000, 0x2000, CRC(edd62ae1) SHA1(9648f1ae3033c30ed8ab8d9c87b111756dab7b5e) )
	ROM_LOAD( "epr-5756.134",	0xa000, 0x2000, CRC(11974040) SHA1(a0904d19d06fb5ef5eb6da0dc4efe556bc29b33e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5763.3",		0x0000, 0x2000, CRC(d712280d) SHA1(8393dfb57d9af22b3280ecaef736b6f9d856dbee) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5762.82",	0x0000, 0x2000, CRC(4a91d08a) SHA1(4687ecc4061719fca5f85b2b290ebb7ced15ee5b) )
	ROM_LOAD( "epr-5761.65",	0x2000, 0x2000, CRC(f7d61b65) SHA1(a7a992f52406413e931945be60b35175f8aea6c2) )
	ROM_LOAD( "epr-5760.81",	0x4000, 0x2000, CRC(95045820) SHA1(d1848fc4f3d66603d0e8217373a37148aa2eeef5) )
	ROM_LOAD( "epr-5759.64",	0x6000, 0x2000, CRC(5f9bae4e) SHA1(6fff6086a96be6aa28bec05d1c94c257bb29ef1e) )
	ROM_LOAD( "epr-5758.80",	0x8000, 0x2000, CRC(808ee706) SHA1(d38ca7c6f36db6e35a3ce87bacdd70f293f23104) )
	ROM_LOAD( "epr-5757.63",	0xa000, 0x2000, CRC(480f7074) SHA1(c54a1fa02e312676658d7c5392a5a841bdb15d44) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5749.86",	0x0000, 0x4000, CRC(e24682cd) SHA1(3f626f3e5e2db486ccf727e9869ab488643b4a8c) )
	ROM_LOAD( "epr-5750.93",	0x4000, 0x4000, CRC(6564d1ad) SHA1(f246afee7e73bc30054b0e5dcb83fa0edd2d2164) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( swat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5807b.129",	0x0000, 0x2000, CRC(93db9c9f) SHA1(56e9d9a33f04b4d5971c0db24cc8719a52e64678) ) /* encrypted */
	ROM_LOAD( "epr-5808.130",	0x2000, 0x2000, CRC(67116665) SHA1(e8aa72f2835d38367be5e8a9313e51b64f452ee7) ) /* encrypted */
	ROM_LOAD( "epr-5809.131",	0x4000, 0x2000, CRC(fd792fc9) SHA1(a0b4f0c2e537bd16f7345590da00f2622947d7e4) ) /* encrypted */
	ROM_LOAD( "epr-5810.132",	0x6000, 0x2000, CRC(dc2b279d) SHA1(e740cbe239d379705fdffb3e500d6f5a2fece2e2) ) /* encrypted */
	ROM_LOAD( "epr-5811.133",	0x8000, 0x2000, CRC(093e3ab1) SHA1(abf1f23dc26a7518357d0c1749e869b539c3bbed) )
	ROM_LOAD( "epr-5812.134",	0xa000, 0x2000, CRC(5bfd692f) SHA1(adc8dcf643d8d0b0a1d0dda0494567263ea11a00) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5819.3",		0x0000, 0x2000, CRC(f6afd0fd) SHA1(06062648b9ebc70b4b5c30b043f537adc0052047) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5818.82",	0x0000, 0x2000, CRC(b22033d9) SHA1(ad217cd8dad178f3f2f1fd44a58adcc4887fb6b7) )
	ROM_LOAD( "epr-5817.65",	0x2000, 0x2000, CRC(fd942797) SHA1(da7378e8d12cc2970df2efa075c944c79b3b74d2) )
	ROM_LOAD( "epr-5816.81",	0x4000, 0x2000, CRC(4384376d) SHA1(78ae13a38d6368e44ba95642cce7f5515a5b6022) )
	ROM_LOAD( "epr-5815.64",	0x6000, 0x2000, CRC(16ad046c) SHA1(a0b97e016e5cf43f223ecb6c5fe7dec7c8e9c098) )
	ROM_LOAD( "epr-5814.80",	0x8000, 0x2000, CRC(be721c99) SHA1(bbb0afe2b195d60418014c36acf3de95adfd90d8) )
	ROM_LOAD( "epr-5813.63",	0xa000, 0x2000, CRC(0d42c27e) SHA1(06b1d23cacfef3017e5951dc10e8471e9b3103d5) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5805.86",	0x0000, 0x4000, CRC(5a732865) SHA1(55c54e54f052187ddd957131e56400c9c432a6b2) )
	ROM_LOAD( "epr-5806.93",	0x4000, 0x4000, CRC(26ac258c) SHA1(e4e9f929ab8ae7da74f885481cf94335d7553a1c) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( flicky )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5978a.116",	0x0000, 0x4000, CRC(296f1492) SHA1(52e2c63ce376ab8124b2c68bdfa432b6621cfa78) ) /* encrypted */
	ROM_LOAD( "epr5979a.109",	0x4000, 0x4000, CRC(64b03ef9) SHA1(7519aa7f036bce6d52a5d4be2418139559f9a8a5) ) /* encrypted */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",	0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5868.62",	0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",	0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",	0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",	0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",	0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",	0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5855.117",	0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",	0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( flicks2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6621.bin",	0x0000, 0x4000, CRC(b21ff546) SHA1(e1d5438eaf0efeaeb4687dcfc12bf325e804182f) )
	ROM_LOAD( "epr-6622.bin",	0x4000, 0x4000, CRC(133a8bf1) SHA1(e5e620797daace0843a680cb4572586b5e639ca0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",	0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5868.62",	0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",	0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",	0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",	0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",	0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",	0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5855.117",	0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",	0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END



ROM_START( flicks1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic129",	0x0000, 0x2000, CRC(7011275c) SHA1(69d9d1a66734bf859dbd0200b5a772110bd522c1) ) /* encrypted */
	ROM_LOAD( "ic130",	0x2000, 0x2000, CRC(e7ed012d) SHA1(7f378ad3e0b6721d7108b4ee10333422df92c039) ) /* encrypted */
	ROM_LOAD( "ic131",	0x4000, 0x2000, CRC(c5e98cd1) SHA1(ea8d97bebfce4e41242169d34bccbf430b094fd7) ) /* encrypted */
	ROM_LOAD( "ic132",	0x6000, 0x2000, CRC(0e5122c2) SHA1(cec34001d4eb8a983b3299462ec513049a3dab46) ) /* encrypted */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",	0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5868.62",	0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",	0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",	0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",	0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",	0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",	0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5855.117",	0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",	0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( flickyo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5857.bin",	0x0000, 0x2000, CRC(a65ac88e) SHA1(1d1c276f7ffb33bc9f216b6b69517f1783d435a4) ) /* encrypted */
	ROM_LOAD( "epr5858a.bin",	0x2000, 0x2000, CRC(18b412f4) SHA1(6205dc2a6c1092f9bc7752672b7c06d5faf2f65e) ) /* encrypted */
	ROM_LOAD( "epr-5859.bin",	0x4000, 0x2000, CRC(a5558d7e) SHA1(ca59c7e57ae45f960f769db9a04ffa5c870005dd) ) /* encrypted */
	ROM_LOAD( "epr-5860.bin",	0x6000, 0x2000, CRC(1b35fef1) SHA1(53ca5361309c59a2b3490ea0037c6e58f07837d9) ) /* encrypted */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",	0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-5868.62",	0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",	0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",	0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",	0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",	0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",	0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-5855.117",	0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",	0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wmatch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wm.129",			0x0000, 0x2000, CRC(b6db4442) SHA1(9f31b3b2d4b4a430f9de84141ebd66bdba063387) ) /* encrypted */
	ROM_LOAD( "wm.130",			0x2000, 0x2000, CRC(59a0a7a0) SHA1(a1707d08ba968d1ad01f3249c046a62dde8e2730) ) /* encrypted */
	ROM_LOAD( "wm.131",			0x4000, 0x2000, CRC(4cb3856a) SHA1(983f52bfb2f8e3871518137f424786a9a8e5c53d) ) /* encrypted */
	ROM_LOAD( "wm.132",			0x6000, 0x2000, CRC(e2e44b29) SHA1(53208666c1368887ab347ea1f261e692cc041d40) ) /* encrypted */
	ROM_LOAD( "wm.133",			0x8000, 0x2000, CRC(43a36445) SHA1(6cc5a6fa8319d4e2b454b326d8a908ff764fa65f) )
	ROM_LOAD( "wm.134",			0xa000, 0x2000, CRC(5624794c) SHA1(7cfb0a35b7fb8394e0e7efa6b63ba83bd5c9b8e7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "wm.3",			0x0000, 0x2000, CRC(50d2afb7) SHA1(21b109d389d0b52d89cf635467c3213f6b24d7df) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "wm.82",			0x0000, 0x2000, CRC(540f0bf3) SHA1(3898dee3ed9e7382a9dfc3ee2af177c5b832ea84) )
	ROM_LOAD( "wm.65",			0x2000, 0x2000, CRC(92c1e39e) SHA1(a701a66ed75fbc0be4819751dabb86e51a1dbbc4) )
	ROM_LOAD( "wm.81",			0x4000, 0x2000, CRC(6a01ff2a) SHA1(f609fe9ec648dd428a6e2fc544585935d7adc562) )
	ROM_LOAD( "wm.64",			0x6000, 0x2000, CRC(aae6449b) SHA1(852d6c01420ea55e4215ec99adbb6896fa16a02d) )
	ROM_LOAD( "wm.80",			0x8000, 0x2000, CRC(fc3f0bd4) SHA1(887ff0d6c5fff0d1e631518fc89901d43a0d7088) )
	ROM_LOAD( "wm.63",			0xa000, 0x2000, CRC(c2ce9b93) SHA1(934f4dddf2f42a23f91385dd62fc166b117063b8) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "wm.86",			0x0000, 0x4000, CRC(238ae0e5) SHA1(af18cfe7f8103358a0ce2aef9bbd949fdc0bfbfc) )
	ROM_LOAD( "wm.93",			0x4000, 0x4000, CRC(a2f19170) SHA1(47dacc380b09c6365c737d320145cedad54ecedb) )

	ROM_REGION( 0x0100, "user1", 0 )	/* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.106",	0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( bullfgt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-.129",		0x0000, 0x2000, CRC(29f19156) SHA1(86cca9601f63b9b3d3aaaf21c3a3e456a50ca6b8) ) /* encrypted */
	ROM_LOAD( "epr-.130",		0x2000, 0x2000, CRC(e37d2b95) SHA1(9d2523190e49c9d45a5832da912cbc0cd23e2496) ) /* encrypted */
	ROM_LOAD( "epr-.131",		0x4000, 0x2000, CRC(eaf5773d) SHA1(7db6a7c1c4d9e5f5b4de97b41ab5dd591e2e1548) ) /* encrypted */
	ROM_LOAD( "epr-.132",		0x6000, 0x2000, CRC(72c3c712) SHA1(1c1ac6d7248382228b99d2652f53fbe15246f253) ) /* encrypted */
	ROM_LOAD( "epr-.133",		0x8000, 0x2000, CRC(7d9fa4cd) SHA1(b6f0d86281c7e8de7a23b0c55c1991350d5bc9b1) )
	ROM_LOAD( "epr-.134",		0xa000, 0x2000, CRC(061f2797) SHA1(f13acd4c5b33ed85229a3907744283646e020867) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6077.120",	0x0000, 0x2000, CRC(02a37602) SHA1(1b67b0d80a228f7faf054bfd79aff120d92c8166) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-.82",		0x0000, 0x2000, CRC(b71c349f) SHA1(5a0e9b90c71708dadab201da09c71449e05268e1) )
	ROM_LOAD( "epr-.65",		0x2000, 0x2000, CRC(86deafa8) SHA1(b4b9d38bd4a47ce2e75ec0ef3d7507aef8a16858) )
	ROM_LOAD( "epr-6087.81",	0x4000, 0x2000, CRC(2677742c) SHA1(6a6154f1c2cc53b9d224fc73bab47e6deb7c505f) ) /* epr-6087.81 */
	ROM_LOAD( "epr-.64",		0x6000, 0x2000, CRC(6f0a62be) SHA1(30c93c4d7f916f7b9a725f412a3a4a71f24c4f22) )
	ROM_LOAD( "epr-6085.80",	0x8000, 0x2000, CRC(9c3ddc62) SHA1(3332824de114836760a40133fb65d8f40474bc81) ) /* epr-6085.80 */
	ROM_LOAD( "epr-.63",		0xa000, 0x2000, CRC(c0fce57c) SHA1(74f2c987f77e73b7069014d3bd6809d8bb3596c7) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-6069.86",	0x0000, 0x4000, CRC(fe691e41) SHA1(90faf26685202e2a25bb3024750456014d0722b3) ) /* epr-6069.86 */
	ROM_LOAD( "epr-6070.93",	0x4000, 0x4000, CRC(34f080df) SHA1(0e7d28e3325c8c3f06438fde29ea0ffe57fc325f) ) /* epr-6070.93 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( thetogyu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6071.116",	0x0000, 0x4000, CRC(96b57df9) SHA1(bfce24bf570961d3cfb449078e23e546fad7229e) ) /* encrypted */
	ROM_LOAD( "epr-6072.109",	0x4000, 0x4000, CRC(f7baadd0) SHA1(45a05b72561d47e4ac5475509fe2b57d870c89cd) ) /* encrypted */
	ROM_LOAD( "epr-6073.96",	0x8000, 0x4000, CRC(721af166) SHA1(0b345715227e70fa6857f5967f0c7da9577f8887) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6077.120",	0x0000, 0x2000, CRC(02a37602) SHA1(1b67b0d80a228f7faf054bfd79aff120d92c8166) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6089.62",	0x0000, 0x2000, CRC(a183e5ff) SHA1(bb710377a8e88f530b669141ab46abd867c6cb83) )
	ROM_LOAD( "epr-6088.61",	0x2000, 0x2000, CRC(b919b4a6) SHA1(ca11a96bee2e2059552ac6cce6f8dead1965ef4b) )
	ROM_LOAD( "epr-6087.64",	0x4000, 0x2000, CRC(2677742c) SHA1(6a6154f1c2cc53b9d224fc73bab47e6deb7c505f) )
	ROM_LOAD( "epr-6086.63",	0x6000, 0x2000, CRC(76b5a084) SHA1(32fd23f0d6fc8f5c3b5aae9a20907191a6d70611) )
	ROM_LOAD( "epr-6085.66",	0x8000, 0x2000, CRC(9c3ddc62) SHA1(3332824de114836760a40133fb65d8f40474bc81) )
	ROM_LOAD( "epr-6084.65",	0xa000, 0x2000, CRC(90e1fa5f) SHA1(e37a7f872229a93a70e42615e6452aa608d53a93) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-6069.117",	0x0000, 0x4000, CRC(fe691e41) SHA1(90faf26685202e2a25bb3024750456014d0722b3) )
	ROM_LOAD( "epr-6070.110",	0x4000, 0x4000, CRC(34f080df) SHA1(0e7d28e3325c8c3f06438fde29ea0ffe57fc325f) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( pitfall2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr6456a.116",	0x0000, 0x4000, CRC(bcc8406b) SHA1(2e5c76886fce2c9863db7a914b85b088971aceef) ) /* encrypted */
	ROM_LOAD( "epr6457a.109",	0x4000, 0x4000, CRC(a016fd2a) SHA1(866f82066466bc5eaf6ab1b6f85a1c173692a1f7) ) /* encrypted */
	ROM_LOAD( "epr6458a.96",	0x8000, 0x4000, CRC(5c30b3e8) SHA1(9048091ebf054d0ba0c6a92520ddfac38a479034) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",	0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr6474a.62",	0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr6473a.61",	0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr6472a.64",	0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr6471a.63",	0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr6470a.66",	0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr6469a.65",	0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr6454a.117",	0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",	0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( pitfalla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6505",	0x0000, 0x4000, CRC(b6769739) SHA1(e1b8401c20f77f8ec799b19d7bc94ae4f9ed702f) ) /* encrypted */
	ROM_LOAD( "epr-6506",	0x4000, 0x4000, CRC(1ce6aec4) SHA1(69b54c4569ccfb1166a901e7044ae1026db01a82) ) /* encrypted */
	ROM_LOAD( "epr6458a.96",	0x8000, 0x4000, CRC(5c30b3e8) SHA1(9048091ebf054d0ba0c6a92520ddfac38a479034) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",	0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr6474a.62",	0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr6473a.61",	0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr6472a.64",	0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr6471a.63",	0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr6470a.66",	0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr6469a.65",	0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr6454a.117",	0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",	0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( pitfallu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6623.116",	0x0000, 0x4000, CRC(bcb47ed6) SHA1(d33421999f899c0a4dc0d4553614c1f5c7027257) )
	ROM_LOAD( "epr6624a.109",	0x4000, 0x4000, CRC(6e8b09c1) SHA1(4869ca4d3f0b08cd3df4c82be9cfc774ddeb3010) )
	ROM_LOAD( "epr-6625.96",	0x8000, 0x4000, CRC(dc5484ba) SHA1(62fffff7d935c104def5f09e9dc4a26fa4ce4f94) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",	0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr6474a.62",	0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr6473a.61",	0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr6472a.64",	0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr6471a.63",	0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr6470a.66",	0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr6469a.65",	0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr6454a.117",	0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",	0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( seganinj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-.116",		0x0000, 0x4000, CRC(a5d0c9d0) SHA1(b60caccab8269f40d4f6e7a50f3aa0d4901c1e57) ) /* encrypted */
	ROM_LOAD( "epr-.109",		0x4000, 0x4000, CRC(b9e6775c) SHA1(f39e815c3c034015125b96de34a2a225b81392b5) ) /* encrypted */
	ROM_LOAD( "epr-6552.96",	0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) /* epr-7151.96 */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",	0x2000, 0x2000, CRC(7804db86) SHA1(8229781b8296d3ffdfa2f0901e2eed297cc3e160) )
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",	0x6000, 0x2000, CRC(bf858cad) SHA1(1c18c4aa4b9a59f3c06aa459eab6bdd1b298d848) )
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",	0xa000, 0x2000, CRC(dc931dbb) SHA1(4729b27843f226ba5861c3106f8418db70e7c47d) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( seganinu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7149.116",	0x0000, 0x4000, CRC(cd9fade7) SHA1(958ef5c449df6ef5346b8634cb34a646950f706e) )
	ROM_LOAD( "epr-7150.109",	0x4000, 0x4000, CRC(c36351e2) SHA1(17734d3f410feb4cad617d1931b3356192b69ac0) )
	ROM_LOAD( "epr-6552.96",	0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) /* epr-7151.96 */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",	0x2000, 0x2000, CRC(7804db86) SHA1(8229781b8296d3ffdfa2f0901e2eed297cc3e160) )
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",	0x6000, 0x2000, CRC(bf858cad) SHA1(1c18c4aa4b9a59f3c06aa459eab6bdd1b298d848) )
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",	0xa000, 0x2000, CRC(dc931dbb) SHA1(4729b27843f226ba5861c3106f8418db70e7c47d) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( nprinces )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6612.129",	0x0000, 0x2000, CRC(1b30976f) SHA1(f76b7f3d88985a5c190e7880c27ab057f102db31) ) /* encrypted */
	ROM_LOAD( "epr-6613.130",	0x2000, 0x2000, CRC(18281f27) SHA1(3fcf2fbd1fc13eda678b77c58c53aa881882286c) ) /* encrypted */
	ROM_LOAD( "epr-6614.131",	0x4000, 0x2000, CRC(69fc3d73) SHA1(287e6b252ae3cd23812b56afe23d4f239f3a76d5) ) /* encrypted */
	ROM_LOAD( "epr-6615.132",	0x6000, 0x2000, CRC(1d0374c8) SHA1(6d818470e294c03b51ec6db8a285d7b71ab2b61f) ) /* encrypted */
	ROM_LOAD( "epr-6577.133",	0x8000, 0x2000, CRC(73616e03) SHA1(429615ee1e041d3e14fc557ec39c380fea07de71) ) /* epr-6616.133 */
	ROM_LOAD( "epr-6617.134",	0xa000, 0x2000, CRC(20b6f895) SHA1(9c9cb3b0c33c4da2850a5756b63c3886634ec544) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) ) /* epr-6558.82 */
	ROM_LOAD( "epr-6557.61",	0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) ) /* epr-6557.65 */
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) ) /* epr-6556.81 */
	ROM_LOAD( "epr-6555.63",	0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) ) /* epr-6555.64 */
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) ) /* epr-6554.80 */
	ROM_LOAD( "epr-6553.65",	0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) ) /* epr-6553.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) ) /* epr-6546.3 */
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) ) /* epr-6548.1 */
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) ) /* epr-6547.4 */
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) ) /* epr-6549.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( nprincso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6550.116",	0x0000, 0x4000, CRC(5f6d59f1) SHA1(e151bf22799c6507a167f83262e48fe2ba74dbd9) ) /* encrypted */
	ROM_LOAD( "epr-6551.109",	0x4000, 0x4000, CRC(1af133b2) SHA1(d3ff924782223ea0566d52ab8b45f17af433966e) ) /* encrypted */
	ROM_LOAD( "epr-6552.96",	0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6557.61",	0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) )
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6555.63",	0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) )
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6553.65",	0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( nprincsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6573.129",	0x0000, 0x2000, CRC(d2919c7d) SHA1(993fdde7dd8d4dbad42f8072829cfea794693a37) )
	ROM_LOAD( "epr-6574.130",	0x2000, 0x2000, CRC(5a132833) SHA1(c21cdca6062a6ea2ca306a8dd26b572b3be86321) )
	ROM_LOAD( "epr-6575.131",	0x4000, 0x2000, CRC(a94b0bd4) SHA1(068db579de3dbd545ae41f930a24f2997a2efedf) )
	ROM_LOAD( "epr-6576.132",	0x6000, 0x2000, CRC(27d3bbdb) SHA1(c7f729798c174de73b6582087f6fe2d4db848b6b) )
	ROM_LOAD( "epr-6577.133",	0x8000, 0x2000, CRC(73616e03) SHA1(429615ee1e041d3e14fc557ec39c380fea07de71) )
	ROM_LOAD( "epr-6578.134",	0xa000, 0x2000, CRC(ab68499f) SHA1(6c662a0ff827cc68bcdb26f6b9d48add4f8ef2e9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) ) /* epr-6558.82 */
	ROM_LOAD( "epr-6557.61",	0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) ) /* epr-6557.65 */
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) ) /* epr-6556.81 */
	ROM_LOAD( "epr-6555.63",	0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) ) /* epr-6555.64 */
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) ) /* epr-6554.80 */
	ROM_LOAD( "epr-6553.65",	0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) ) /* epr-6553.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) ) /* epr-6546.3 */
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) ) /* epr-6548.1 */
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) ) /* epr-6547.4 */
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) ) /* epr-6549.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( nprincsb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nprinces.001",	0x0000, 0x4000, CRC(e0de073c) SHA1(26aec99ddb080124225e0abf17aac4cc4aed1834) )  /* encrypted */
	ROM_LOAD( "nprinces.002",	0x4000, 0x4000, CRC(27219c7f) SHA1(3f4b0ea9b49907231d10a38d89e2f1803dc168c9) )  /* encrypted */
	ROM_LOAD( "epr-6552.96",	0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6557.61",	0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) )
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6555.63",	0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) )
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6553.65",	0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0220, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
	ROM_LOAD( "nprinces.129",	0x0100, 0x0100, CRC(ae765f62) SHA1(9434b5a23d118a9c62015b479719826b38269cd4) ) /* decryption table (not used) */
	ROM_LOAD( "nprinces.123",	0x0200, 0x0020, CRC(ed5146e9) SHA1(7044035c07636e4029f4b746c1a92e15173869e9) ) /* decryption table (not used) */
ROM_END

ROM_START( ninja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr6594.bin",    0x0000, 0x4000, CRC(3ef0e5fc) SHA1(ba2d832aa33759c21582e728ca7e4a0ca03cb937) )
	ROM_LOAD( "epr6595.bin",    0x4000, 0x4000, CRC(b16f13cd) SHA1(e4649ce76393fdf8d2a1f53f1c25ee27ed35db45) )
	ROM_LOAD( "epr-6552.96",	0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) /* epr-7151.96 */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",	0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6558.62",	0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr6592.bin",    0x2000, 0x2000, CRC(88d0c7a1) SHA1(a649a56484f3cf466dbd4bc468d21220e638c5fe) )
	ROM_LOAD( "epr-6556.64",	0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr6590.bin",    0x6000, 0x2000, CRC(956e3b61) SHA1(47e797bcc39f3ef917848b64a3666e08f9498cc0) )
	ROM_LOAD( "epr-6554.66",	0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr6588.bin",    0xa000, 0x2000, CRC(023a14a3) SHA1(199bdf597ace496992f323c0eaa1e779920fb976) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6546.117",	0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",	0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",	0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",	0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( imsorry )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6676.116",	0x0000, 0x4000, CRC(eb087d7f) SHA1(b9bcc76bbdfa597d252e7db60fa0f7529e884cce) ) /* encrypted */
	ROM_LOAD( "epr-6677.109",	0x4000, 0x4000, CRC(bd244bee) SHA1(ad9c722fde08f48d8bc835b244450b01a3d747c2) ) /* encrypted */
	ROM_LOAD( "epr-6678.96",	0x8000, 0x4000, CRC(2e16b9fd) SHA1(3395fb769c79f048d099e2898bb7a15611b006c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6656.120",	0x0000, 0x2000, CRC(25e3d685) SHA1(a0267d6533af6ff5bf76b9858f2913821a915baf) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6684.62",	0x0000, 0x2000, CRC(2c8df377) SHA1(abcabdecee0ce52000dab831ae1e50fe12c97066) )
	ROM_LOAD( "epr-6683.61",	0x2000, 0x2000, CRC(89431c48) SHA1(99c0d141eb5519c31b194693a1fe9be882cb03fd) )
	ROM_LOAD( "epr-6682.64",	0x4000, 0x2000, CRC(256a9246) SHA1(6aed392a5dd639c54bf54acd3651a77274c0a277) )
	ROM_LOAD( "epr-6681.63",	0x6000, 0x2000, CRC(6974d189) SHA1(57999a73511b2b3f52d7d6a32addc0641255d7b1) )
	ROM_LOAD( "epr-6680.66",	0x8000, 0x2000, CRC(10a629d6) SHA1(fa2c7df33c685e48020ccabcfba5830e7609e392) )
	ROM_LOAD( "epr-6674.65",	0xa000, 0x2000, CRC(143d883c) SHA1(e35f6fae7feb9a353321d8239ac8990bc773e60b) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-6645.117",	0x0000, 0x4000, CRC(1ba167ee) SHA1(5a105cc3112f2533e7c5982233405d365402fba2) )
	ROM_LOAD( "epr-6646.04",	0x4000, 0x4000, CRC(edda7ad6) SHA1(eef7dcde632787283c4cb522380b138060018204) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( imsorryj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6647.116",	0x0000, 0x4000, CRC(cc5d915d) SHA1(1e2def1f7a03db3504177127dc784fe6c99a7440) ) /* encrypted */
	ROM_LOAD( "epr-6648.109",	0x4000, 0x4000, CRC(37574d60) SHA1(c7c8507b608976973e766956bd28dfb17222de35) ) /* encrypted */
	ROM_LOAD( "epr-6649.96",	0x8000, 0x4000, CRC(5f59bdee) SHA1(289ba35a7869a5b833c8aa4819e76fadde2d1ace) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6656.120",	0x0000, 0x2000, CRC(25e3d685) SHA1(a0267d6533af6ff5bf76b9858f2913821a915baf) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6655.62",	0x0000, 0x2000, CRC(be1f762f) SHA1(abf7af29b1fe4003342fbb431541921433a1fc7c) )
	ROM_LOAD( "epr-6654.61",	0x2000, 0x2000, CRC(ed5f7fc8) SHA1(2e77e8292f644f5bbeebc807f193f20d4591f47a) )
	ROM_LOAD( "epr-6653.64",	0x4000, 0x2000, CRC(8b4845a7) SHA1(048efa9d8122d4a91f4d005d023261a5a5b8b046) )
	ROM_LOAD( "epr-6652.63",	0x6000, 0x2000, CRC(001d68cb) SHA1(c23b4bfbb09b7d3047e04b92d19b69d2ea550879) )
	ROM_LOAD( "epr-6651.66",	0x8000, 0x2000, CRC(4ee9b5e6) SHA1(821bdeefea03c5d3be6d83d0dd30841969d81bd4) )
	ROM_LOAD( "epr-6650.65",	0xa000, 0x2000, CRC(3fca4414) SHA1(d4c80e06bb7027dbc8aea42fb48c71d9fa08ca40) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data */
	ROM_LOAD( "epr-6645.117",	0x0000, 0x4000, CRC(1ba167ee) SHA1(5a105cc3112f2533e7c5982233405d365402fba2) )
	ROM_LOAD( "epr-6646.04",	0x4000, 0x4000, CRC(edda7ad6) SHA1(eef7dcde632787283c4cb522380b138060018204) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( teddybb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6768.116",	0x0000, 0x4000, CRC(5939817e) SHA1(84d78412d3e13da493d08a40deb2ff3fd51ff9f8) ) /* encrypted */
	ROM_LOAD( "epr-6769.109",	0x4000, 0x4000, CRC(14a98ddd) SHA1(197fa05fb476c02d64e9027cde5aaac26f59b5e8) ) /* encrypted */
	ROM_LOAD( "epr-6770.96",	0x8000, 0x4000, CRC(67b0c7c2) SHA1(b955719c954af5266e06ae7b04ff20f9dc414997) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr6748x.120",	0x0000, 0x2000, CRC(c2a1b89d) SHA1(55c5461640ccb26bed332c13adfbb99c27237bcb) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6747.62",	0x0000, 0x2000, CRC(a0e5aca7) SHA1(e7d35ed5e1606a1ea8b29eeca3ca807ed163573b) ) /* epr-6776.62 */
	ROM_LOAD( "epr-6746.61",	0x2000, 0x2000, CRC(cdb77e51) SHA1(590855f41b62fe9a84db51f90242697abb603c00) ) /* epr-6775.61 */
	ROM_LOAD( "epr-6745.64",	0x4000, 0x2000, CRC(0cab75c3) SHA1(ef9b74c62fbd81db8942f0b7aa2569a8f4843e9d) ) /* epr-6774.64 */
	ROM_LOAD( "epr-6744.63",	0x6000, 0x2000, CRC(0ef8d2cd) SHA1(cf9ebf8e3c1d0794b3d3377464f3908d4fcee6f7) ) /* epr-6773.63 */
	ROM_LOAD( "epr-6743.66",	0x8000, 0x2000, CRC(c33062b5) SHA1(5845da895059ff0271a6ed6fd0fa1392be1ac223) ) /* epr-6772.66 */
	ROM_LOAD( "epr-6742.65",	0xa000, 0x2000, CRC(c457e8c5) SHA1(3c1008ae8b054c198cfeb0a66534fb51beaee0f6) ) /* epr-6771.65 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6735.117",	0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "epr-6737.04",	0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "epr-6736.110",	0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "epr-6738.05",	0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( teddybbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6739.116",	0x0000, 0x4000, CRC(81a37e69) SHA1(ddd0fd7ba5b3646c43ae4261f1e3fedd4184d92c) ) /* encrypted */
	ROM_LOAD( "epr-6740.109",	0x4000, 0x4000, CRC(715388a9) SHA1(5affc4ecb1e0d58b69093aed732b1e292b8d3118) ) /* encrypted */
	ROM_LOAD( "epr-6741.96",	0x8000, 0x4000, CRC(e5a74f5f) SHA1(ccf18b424d4aaeec0bae1e6f096b4c176f6ab554) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6748.120",	0x0000, 0x2000, CRC(9325a1cf) SHA1(555d137b1c974b144ebe6593b4c32c97b3bb5de9) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6747.62",	0x0000, 0x2000, CRC(a0e5aca7) SHA1(e7d35ed5e1606a1ea8b29eeca3ca807ed163573b) )
	ROM_LOAD( "epr-6746.61",	0x2000, 0x2000, CRC(cdb77e51) SHA1(590855f41b62fe9a84db51f90242697abb603c00) )
	ROM_LOAD( "epr-6745.64",	0x4000, 0x2000, CRC(0cab75c3) SHA1(ef9b74c62fbd81db8942f0b7aa2569a8f4843e9d) )
	ROM_LOAD( "epr-6744.63",	0x6000, 0x2000, CRC(0ef8d2cd) SHA1(cf9ebf8e3c1d0794b3d3377464f3908d4fcee6f7) )
	ROM_LOAD( "epr-6743.66",	0x8000, 0x2000, CRC(c33062b5) SHA1(5845da895059ff0271a6ed6fd0fa1392be1ac223) )
	ROM_LOAD( "epr-6742.65",	0xa000, 0x2000, CRC(c457e8c5) SHA1(3c1008ae8b054c198cfeb0a66534fb51beaee0f6) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6735.117",	0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "epr-6737.04",	0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "epr-6736.110",	0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "epr-6738.05",	0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

/* This is the first System 1 game to have extended ROM space */
ROM_START( hvymetal )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epra6790.1",   0x00000, 0x8000, CRC(59195bb9) SHA1(63dde673bd875dd23d445b152decb1d70c3750a4) ) /* encrypted */
	ROM_LOAD( "epra6789.2",   0x10000, 0x8000, CRC(83e1d18a) SHA1(07ef58ee2a5212e1e2800efc2bd48d2b2a9ed10d) )
	ROM_LOAD( "epra6788.3",   0x18000, 0x8000, CRC(6ecefd57) SHA1(3236313d5d826873d58af5ad80652c8d0ae0cc31) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr6787.120",  0x0000, 0x8000, CRC(b64ac7f0) SHA1(2b16c2702d3230891b700714a66ece95f1a74b44) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr6795.62",   0x00000, 0x4000, CRC(58a3d038) SHA1(9aabfad143748e2ec1b41fde72a1d533bac3f9d8) )
	ROM_LOAD( "epr6796.61",   0x04000, 0x4000, CRC(d8b08a55) SHA1(cfa5370aa430947637bfe57a5a1f802f273b43f7) )
	ROM_LOAD( "epr6793.64",   0x08000, 0x4000, CRC(487407c2) SHA1(9bb9fff24fe057fa17057ba9263d412905a0c036) )
	ROM_LOAD( "epr6794.63",   0x0c000, 0x4000, CRC(89eb3793) SHA1(90a0cc81d917122c726238585eb802763d34884e) )
	ROM_LOAD( "epr6791.66",   0x10000, 0x4000, CRC(a7dcd042) SHA1(d9bac10aa7ac591a20bfed4e391ec1669eadc32d) )
	ROM_LOAD( "epr6792.65",   0x14000, 0x4000, CRC(d0be5e33) SHA1(1e61c6e14c3c736e74e6c2ff5cde71d1d20b99a4) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr6778.117",  0x00000, 0x8000, CRC(0af61aee) SHA1(90879d4d1bef38714a39ca71c101bd103d250284) )
	ROM_LOAD( "epr6777.110",  0x08000, 0x8000, CRC(91d7a197) SHA1(34c12b7de22169d369ff5b8a8d86da62404267f8) )
	ROM_LOAD( "epr6780.4",    0x10000, 0x8000, CRC(55b31df5) SHA1(aa1ce0b1666e17db196bd1e079691fbe433a9226) )
	ROM_LOAD( "epr6779.5",    0x18000, 0x8000, CRC(e03a2b28) SHA1(7e742c09e832d01f74fe4025d194cbc8d2f24b70) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr7036.3",     0x0000, 0x0100, CRC(146f16fb) SHA1(0a2ac871383b115c16491b9ba5973f0d363eac49) ) /* palette red component */
	ROM_LOAD( "pr7035.2",     0x0100, 0x0100, CRC(50b201ed) SHA1(14c3a585c083dc387532d64bfd63e34f5220e6de) ) /* palette green component */
	ROM_LOAD( "pr7034.1",     0x0200, 0x0100, CRC(dfb5f139) SHA1(56cba261819fd5f2beab56ffd80bb3fd328efe3e) ) /* palette blue component */
	ROM_LOAD( "pr5317p.4",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( myhero )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr6963b.116",	0x0000, 0x4000, CRC(4daf89d4) SHA1(6fd69964d4e0dcd5637920711361f1879fcf330e) )
	ROM_LOAD( "epr6964a.109",	0x4000, 0x4000, CRC(c26188e5) SHA1(48d7871a9c63de774c48f1bd9dcaf84b4188f84f) )
	ROM_LOAD( "epr-6927.96",	0x8000, 0x4000, CRC(3cbbaf64) SHA1(fdb5f2ca38010729afa4ed24c087119cf398f27d) ) /* epr-6965.96 */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-69xx.120",	0x0000, 0x2000, CRC(0039e1e9) SHA1(ead2e8a8a518da5ac6ccd5cd6db4cf167ea47c76) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6966.62",	0x0000, 0x2000, CRC(157f0401) SHA1(f07eb40de95054d6a2c2ebec0b251685e8931b37) )
	ROM_LOAD( "epr-6961.61",	0x2000, 0x2000, CRC(be53ce47) SHA1(de6073e7a00cba7e13aca0248c55126b16595d50) )
	ROM_LOAD( "epr-6960.64",	0x4000, 0x2000, CRC(bd381baa) SHA1(e160db821422232fb8f6b4f1c4ce0b61f7bed463) )
	ROM_LOAD( "epr-6959.63",	0x6000, 0x2000, CRC(bc04e79a) SHA1(df93f96aabde981fe9ecf32ef1f99dfebe968835) )
	ROM_LOAD( "epr-6958.66",	0x8000, 0x2000, CRC(714f2c26) SHA1(4696c9322d7b9b27f56309312fe498f14cb32827) )
	ROM_LOAD( "epr-6957.65",	0xa000, 0x2000, CRC(80920112) SHA1(745d029f99b6878efcca535885b9bf98bf8702f2) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6921.117",	0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",	0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",	0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",	0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( sscandal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr6925b.116",	0x0000, 0x4000, CRC(ff54dcec) SHA1(634ba5c79dc20dc6ab3efd9597b9fb1e4f86f58f) ) /* encrypted */
	ROM_LOAD( "epr6926a.109",	0x4000, 0x4000, CRC(5c41eea8) SHA1(6a060a9739ee85c5c3a3e205bfac46bff1ed0b91) ) /* encrypted */
	ROM_LOAD( "epr-6927.96",	0x8000, 0x4000, CRC(3cbbaf64) SHA1(fdb5f2ca38010729afa4ed24c087119cf398f27d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6934.120",	0x0000, 0x2000, CRC(af467223) SHA1(d79a67e761fe483407cad645dd3b93d86e8790e3) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6933.62",	0x0000, 0x2000, CRC(e7304036) SHA1(cff10b180832703ef472a6abd481f8433308d462) )
	ROM_LOAD( "epr-6932.61",	0x2000, 0x2000, CRC(f5cfbfda) SHA1(52044e3eb6f2e82c9490856410758c5223eb116b) )
	ROM_LOAD( "epr-6931.64",	0x4000, 0x2000, CRC(599d7f87) SHA1(c581001b45856447b2878dc5bdeb92bffb15086a) )
	ROM_LOAD( "epr-6930.63",	0x6000, 0x2000, CRC(cb6616c2) SHA1(84d4f65379cb9d5c9774d29bbad137529ab221a6) )
	ROM_LOAD( "epr-6929.66",	0x8000, 0x2000, CRC(27a16856) SHA1(1e386dfa5178a0902f5d5e64f4d0414593f2e801) )
	ROM_LOAD( "epr-6928.65",	0xa000, 0x2000, CRC(c0c9cfa4) SHA1(3a98f25beab2dcacf5ec4457501ecfde9bc6e8eb) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6921.117",	0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",	0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",	0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",	0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( myherok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* all the three program ROMs have bits 0-1 swapped */
	/* when decoded, they are identical to the Japanese version */
	ROM_LOAD( "ry-11.rom",		0x0000, 0x4000, CRC(6f4c8ee5) SHA1(bbbb87a66be383d9d44ae3bb7f4d1ff56933fd57) ) /* encrypted */
	ROM_LOAD( "ry-09.rom",		0x4000, 0x4000, CRC(369302a1) SHA1(670bf97e401c0a665330d2264c126c275f4c5f8d) ) /* encrypted */
	ROM_LOAD( "ry-07.rom",		0x8000, 0x4000, CRC(b8e9922e) SHA1(f563fd415d5218c2c3e0071776c91b6250cacea3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6934.120",	0x0000, 0x2000, CRC(af467223) SHA1(d79a67e761fe483407cad645dd3b93d86e8790e3) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	/* all three gfx ROMs have address lines A4 and A5 swapped, also #1 and #3 */
	/* have data lines D0 and D6 swapped, while #2 has data lines D1 and D5 swapped. */
	ROM_LOAD( "ry-04.rom",		0x0000, 0x4000, CRC(dfb75143) SHA1(b1943e0b8ca4439d5ef27abecd48e6fc806d3a0e) )
	ROM_LOAD( "ry-03.rom",		0x4000, 0x4000, CRC(cf68b4a2) SHA1(7f1607320943c452bcc30b4805e8e9c9d2a61955) )
	ROM_LOAD( "ry-02.rom",		0x8000, 0x4000, CRC(d100eaef) SHA1(d917a85c3560578cc7640bfcb4725b4217f0ed91) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6921.117",	0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",	0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",	0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",	0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

/*
    Shooting Master (SEGA) - Rev A, 8751 315-5159a
    Year: 1985
    System 2

    Main Board        834-5719
    Light Gun Board?  834-5720
*/

ROM_START( shtngmst )
	ROM_REGION( 0x20000, "maincpu", 0 )
    /* This rom is located on the daughter board .*/
	ROM_LOAD( "epr7100a.ic18",  0x00000, 0x8000, CRC(661f0b6e) SHA1(a1d064839c95d8a6f6d89b0894259f666d637ec4) )
    /* These roms are located on the main board. */
	ROM_LOAD( "epr7101.ic91",   0x10000, 0x8000, CRC(ebf5ff72) SHA1(13ae06e3a81cf00b80ec939d5baf30143d61d480) )
	ROM_LOAD( "epr7102.ic92",   0x18000, 0x8000, CRC(c890a4ad) SHA1(4b59d37902ace3a69b380ff40652ee37c85f0e9d) )

    /* This rom is located on the main board. */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7043.ic126",  0x0000, 0x8000, CRC(99a368ab) SHA1(a9451f39ee2613e5c3e2791d4d8d837b4a3ab666) )

    /* This mcu is located on the main board. */
	ROM_REGION( 0x10000, "cpu2", 0 )	/* Internal i8751 MCU code */
	ROM_LOAD( "315-5159a.ic74", 0x00000, 0x1000, NO_DUMP )

    /* These roms are located on the main board. */
	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr7040.ic4",    0x00000, 0x8000, CRC(f30769fa) SHA1(366c1fbe4e1c8943b209f6c831c9a6b7e4372105) )
	ROM_LOAD( "epr7041.ic5",    0x08000, 0x8000, CRC(f3e273f9) SHA1(b8715c528299dc1e4f0c19c50d91ca9861a423a1) )
	ROM_LOAD( "epr7042.ic6",    0x10000, 0x8000, CRC(6841c917) SHA1(6553843eea0131eb7b5a9aa29dddf641e41d8cc3) )

    /* These roms are located on the daughter board. */
	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "epr7110.ic26",   0x00000, 0x8000, CRC(5d1a5048) SHA1(d1626ab1981080451c912df7e4ad7f76c0cb3459) )
	ROM_LOAD( "epr7106.ic22",   0x08000, 0x8000, CRC(ae7ab7a2) SHA1(153691e468d29d21b95f1fbffb6896a3140d7e14) )
	ROM_LOAD( "epr7108.ic24",   0x10000, 0x8000, CRC(816180ac) SHA1(a59670ec77d4359041ebf12dae5b74add55d82ac) )
	ROM_LOAD( "epr7104.ic20",   0x18000, 0x8000, CRC(84a679c5) SHA1(19a21b1b33fc215f606093bfd61d597e4bd0b3d0) )
	ROM_LOAD( "epr7109.ic25",   0x20000, 0x8000, CRC(097f7481) SHA1(4d93ea01b811af1cd3e136116625e4b8e06358a2) )
	ROM_LOAD( "epr7105.ic21",   0x28000, 0x8000, CRC(13111729) SHA1(57ca2b945db36b056d0e40a39456fd8bf9d0a3ec) )
	ROM_LOAD( "epr7107.ic23",   0x30000, 0x8000, CRC(8f50ea24) SHA1(781687e202dedca7b72c9bd5b97d9d46fcfd601c) )

    /* These proms are located on the main board. */
	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "epr7113.ic20",   0x00000, 0x0100, CRC(5c0e1360) SHA1(2011b3eef2a58f9bd3f3b1bb9e6c201db85727c2) ) /* palette red component */
	ROM_LOAD( "epr7112.ic14",   0x00100, 0x0100, CRC(46fbd351) SHA1(1fca7fbc5d5f8e13e58bbac735511bd0af392446) ) /* palette green component */
	ROM_LOAD( "epr7111.ic8",    0x00200, 0x0100, CRC(8123b6b9) SHA1(fb2c5498f0603b5cd270402a738c891a85453666) ) /* palette blue component - N82S129AN */
	ROM_LOAD( "epr5317.ic37",   0x00300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) - N82S129AN */

    /* These pld's are located on the main board. */
	ROM_REGION( 0x0618, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "315-5137.bin",   0x00000, 0x0104, CRC(6ffd9e6f) SHA1(a60a3a2ec5bc256b18bfff0fec0172ee2e4fd955) ) /* TI PAL16R4A-2CN Located at IC10 */
	ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) /* TI PAL16R4ACN Located at IC11 */
    ROM_LOAD( "315-5139.bin",   0x00000, 0x0104, NO_DUMP ) /* CK2605 located at IC50 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC7 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC13 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC19 */
ROM_END

/*
    Shooting Master (SEGA) - Set 1, 8751 315-5159
    Year: 1985
    System 2

    Main Board        834-5719
    Light Gun Board?  834-5720
*/

ROM_START( shtngms1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
    /* This rom is located on the daughter board .*/
	ROM_LOAD( "epr7100.ic18",   0x00000, 0x8000, CRC(45e64431) SHA1(7edf818dc1f65365641e51abc197d13db7a8d4d9) )
    /* These roms are located on the main board. */
	ROM_LOAD( "epr7101.ic91",   0x10000, 0x8000, CRC(ebf5ff72) SHA1(13ae06e3a81cf00b80ec939d5baf30143d61d480) )
	ROM_LOAD( "epr7102.ic92",   0x18000, 0x8000, CRC(c890a4ad) SHA1(4b59d37902ace3a69b380ff40652ee37c85f0e9d) )

    /* This rom is located on the main board. */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7043.ic126",  0x0000, 0x8000, CRC(99a368ab) SHA1(a9451f39ee2613e5c3e2791d4d8d837b4a3ab666) )

    /* This mcu is located on the main board. */
	ROM_REGION( 0x10000, "cpu2", 0 )	/* Internal i8751 MCU code */
	ROM_LOAD( "315-5159.ic74", 0x00000, 0x1000, NO_DUMP )

    /* These roms are located on the main board. */
	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr7040.ic4",    0x00000, 0x8000, CRC(f30769fa) SHA1(366c1fbe4e1c8943b209f6c831c9a6b7e4372105) )
	ROM_LOAD( "epr7041.ic5",    0x08000, 0x8000, CRC(f3e273f9) SHA1(b8715c528299dc1e4f0c19c50d91ca9861a423a1) )
	ROM_LOAD( "epr7042.ic6",    0x10000, 0x8000, CRC(6841c917) SHA1(6553843eea0131eb7b5a9aa29dddf641e41d8cc3) )

    /* These roms are located on the daughter board. */
	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "epr7110.ic26",   0x00000, 0x8000, CRC(5d1a5048) SHA1(d1626ab1981080451c912df7e4ad7f76c0cb3459) )
	ROM_LOAD( "epr7106.ic22",   0x08000, 0x8000, CRC(ae7ab7a2) SHA1(153691e468d29d21b95f1fbffb6896a3140d7e14) )
	ROM_LOAD( "epr7108.ic24",   0x10000, 0x8000, CRC(816180ac) SHA1(a59670ec77d4359041ebf12dae5b74add55d82ac) )
	ROM_LOAD( "epr7104.ic20",   0x18000, 0x8000, CRC(84a679c5) SHA1(19a21b1b33fc215f606093bfd61d597e4bd0b3d0) )
	ROM_LOAD( "epr7109.ic25",   0x20000, 0x8000, CRC(097f7481) SHA1(4d93ea01b811af1cd3e136116625e4b8e06358a2) )
	ROM_LOAD( "epr7105.ic21",   0x28000, 0x8000, CRC(13111729) SHA1(57ca2b945db36b056d0e40a39456fd8bf9d0a3ec) )
	ROM_LOAD( "epr7107.ic23",   0x30000, 0x8000, CRC(8f50ea24) SHA1(781687e202dedca7b72c9bd5b97d9d46fcfd601c) )

    /* These proms are located on the main board. */
	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "epr7113.ic20",   0x00000, 0x0100, CRC(5c0e1360) SHA1(2011b3eef2a58f9bd3f3b1bb9e6c201db85727c2) ) /* palette red component */
	ROM_LOAD( "epr7112.ic14",   0x00100, 0x0100, CRC(46fbd351) SHA1(1fca7fbc5d5f8e13e58bbac735511bd0af392446) ) /* palette green component */
	ROM_LOAD( "epr7111.ic8",    0x00200, 0x0100, CRC(8123b6b9) SHA1(fb2c5498f0603b5cd270402a738c891a85453666) ) /* palette blue component - N82S129AN */
	ROM_LOAD( "epr5317.ic37",   0x00300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) - N82S129AN */

    /* These pld's are located on the main board. */
	ROM_REGION( 0x0618, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "315-5137.bin",   0x00000, 0x0104, CRC(6ffd9e6f) SHA1(a60a3a2ec5bc256b18bfff0fec0172ee2e4fd955) ) /* TI PAL16R4A-2CN Located at IC10 */
	ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) /* TI PAL16R4ACN Located at IC11 */
    ROM_LOAD( "315-5139.bin",   0x00000, 0x0104, NO_DUMP ) /* CK2605 located at IC50 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC7 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC13 */
    ROM_LOAD( "315-5155.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC19 */
ROM_END

/*

Shooting Master (EVG)
Year: 1985
Manufacturer: E.V.G. SRL Milano made in Italy (Sega license)

CPU
1x Z8400AB1-Z80ACPU-Y28548 (main board)
1x iC8751H-88-L5310039 (main board)
1x AMD P8255A-8526YP (main board)
1x SEGA 315-5012-8605P5 (main board)
1x SEGA 315-5011-8549X5 (main board)
1x SEGA 315-5049-8551PX (main board)
1x SEGA 315-5139-8537-CK2605-V-J (main board)
1x oscillator 20.000MHz (main board)
1x SYS Z8400AB1-Z80ACPU-Y28535 (upper board)
1x NEC D8255AC-2 (upper board)
1x oscillator 4.9152MHz (upper board)

ROMs
1x HN27256G-25 (7043)(main board close to Z80)
2x HN27256G-25 (7101-7102)(main board close to C8751)
3x HN27256G-25 (7040-7041-7042)(main board close to 315-5049)
2x PAL16R4A (315-5137 and 315-5138)
1x HN27256G-25 (7100)(upper board close to oscillator)
7x HN27256G-25 (7104 to 7110)(upper board close to Z80 and 8255)

7040.4 = epr7040       Shooting Master
7041.5 = epr7041       Shooting Master
7042.6 = epr7042       Shooting Master
7043.126 = epr7043       Shooting Master
7100.18 NO MATCH
7101.91 = epr7101       Shooting Master
7102.92 = epr7102       Shooting Master
7104.20 = epr7104       Shooting Master
7105.21 = epr7105       Shooting Master
7106.22 = epr7106       Shooting Master
7107.23 = epr7107       Shooting Master
7108.24 = epr7108       Shooting Master
7109.25 = epr7109       Shooting Master
7110.26 = epr7110       Shooting Master
8751.74.bad.dump NO MATCH
pal16r4a(315-5137).jed NOT A ROM  --> converted into bin format on 20060915
pal16r4a(315-5138).jed = pal16r4a.ic9  Choplifter
             = pal16r4a.ic9  Choplifter (bootleg)

this set seems to hang sometimes, is it bad, it could be hacked and failing checks hidden in the game.

*/

ROM_START( shtngmsa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "7100.18",      0x00000, 0x8000, CRC(268ecb1d) SHA1(a9274c9718f7244235cc6df76331d6a0b7e4e4c8) ) // different..
	ROM_LOAD( "epr7101.ic91",   0x10000, 0x8000, CRC(ebf5ff72) SHA1(13ae06e3a81cf00b80ec939d5baf30143d61d480) )
	ROM_LOAD( "epr7102.ic92",   0x18000, 0x8000, CRC(c890a4ad) SHA1(4b59d37902ace3a69b380ff40652ee37c85f0e9d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7043.ic126",  0x0000, 0x8000, CRC(99a368ab) SHA1(a9451f39ee2613e5c3e2791d4d8d837b4a3ab666) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr7040.ic4",    0x00000, 0x8000, CRC(f30769fa) SHA1(366c1fbe4e1c8943b209f6c831c9a6b7e4372105) )
	ROM_LOAD( "epr7041.ic5",    0x08000, 0x8000, CRC(f3e273f9) SHA1(b8715c528299dc1e4f0c19c50d91ca9861a423a1) )
	ROM_LOAD( "epr7042.ic6",    0x10000, 0x8000, CRC(6841c917) SHA1(6553843eea0131eb7b5a9aa29dddf641e41d8cc3) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "epr7110",      0x00000, 0x8000, CRC(5d1a5048) SHA1(d1626ab1981080451c912df7e4ad7f76c0cb3459) )
	ROM_LOAD( "epr7106",      0x08000, 0x8000, CRC(ae7ab7a2) SHA1(153691e468d29d21b95f1fbffb6896a3140d7e14) )
	ROM_LOAD( "epr7108",      0x10000, 0x8000, CRC(816180ac) SHA1(a59670ec77d4359041ebf12dae5b74add55d82ac) )
	ROM_LOAD( "epr7104",      0x18000, 0x8000, CRC(84a679c5) SHA1(19a21b1b33fc215f606093bfd61d597e4bd0b3d0) )
	ROM_LOAD( "epr7109",      0x20000, 0x8000, CRC(097f7481) SHA1(4d93ea01b811af1cd3e136116625e4b8e06358a2) )
	ROM_LOAD( "epr7105",      0x28000, 0x8000, CRC(13111729) SHA1(57ca2b945db36b056d0e40a39456fd8bf9d0a3ec) )
	ROM_LOAD( "epr7107",      0x30000, 0x8000, CRC(8f50ea24) SHA1(781687e202dedca7b72c9bd5b97d9d46fcfd601c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "epr7113.ic20",   0x00000, 0x0100, CRC(5c0e1360) SHA1(2011b3eef2a58f9bd3f3b1bb9e6c201db85727c2) ) /* palette red component */
	ROM_LOAD( "epr7112.ic14",   0x00100, 0x0100, CRC(46fbd351) SHA1(1fca7fbc5d5f8e13e58bbac735511bd0af392446) ) /* palette green component */
	ROM_LOAD( "epr7111.ic8",    0x00200, 0x0100, CRC(8123b6b9) SHA1(fb2c5498f0603b5cd270402a738c891a85453666) ) /* palette blue component - N82S129AN */
	ROM_LOAD( "epr5317.ic37",   0x00300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) - N82S129AN */

	ROM_REGION( 0x0400, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "315-5137.bin",   0x00000, 0x0104, CRC(6ffd9e6f) SHA1(a60a3a2ec5bc256b18bfff0fec0172ee2e4fd955) ) /* TI PAL16R4A-2CN Located at IC10 */
	ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) /* TI PAL16R4ACN Located at IC11 */
ROM_END

/*
    Choplifter (8751 315-5151)
    Year: 1985
    System 2

    Main Board 834-5795
*/

ROM_START( chplft )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7124.ic90",	0x00000, 0x8000, CRC(678d5c41) SHA1(7553979f78270c2ddc5b3f3ebf7817ead8e08de7) )
	ROM_LOAD( "epr-7125.ic91",	0x10000, 0x8000, CRC(f5283498) SHA1(1ad40f6d7b4cd18212ee56917240c0796f1a4ec2) )
	ROM_LOAD( "epr-7126.ic92",	0x18000, 0x8000, CRC(dbd192ab) SHA1(03d280c82599a14fc6a2065d57c6241cdc6f1143) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.ic126",	0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x10000, "cpu2", 0 )	/* Internal i8751 MCU code */
	ROM_LOAD( "315-5151.ic74",  0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7127.ic4",	0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.ic5",	0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.ic6",	0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr-7121.ic87",	0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.ic86",	0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.ic89",	0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.ic88",	0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr7119.ic20",	0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) /* palette red component */
	ROM_LOAD( "pr7118.ic14",	0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) /* palette green component */
	ROM_LOAD( "pr7117.ic8",		0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) /* palette blue component */
	ROM_LOAD( "pr5317.ic28",	0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */

	ROM_REGION( 0x0618, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "315-5152.bin",   0x00000, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) ) /* PAL16R4A located at IC10. */
    ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) /* TI PAL16R4NC located at IC11. */
    ROM_LOAD( "315-5139.bin",   0x00000, 0x0104, NO_DUMP ) /* CK2605 located at IC50. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC7. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC13. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC19. */
ROM_END

/*
    Choplifter (Unprotected)
    Year: 1985
    System 2

    Main Board 834-5795
*/

ROM_START( chplftb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7152.ic90",	0x00000, 0x8000, CRC(fe49d83e) SHA1(307be38dd73ed37b275c1b464d266a752cb06132) )
	ROM_LOAD( "epr-7153.ic91",	0x10000, 0x8000, CRC(48697666) SHA1(0f4c6db9558272f5ceb347e742b284474f18b707) )
	ROM_LOAD( "epr-7154.ic92",	0x18000, 0x8000, CRC(56d6222a) SHA1(ad8ccf15fe7f1d6716f78490892da0167d79f678) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.ic126",	0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7127.ic4",	0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.ic5",	0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.ic6",	0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr-7121.ic87",	0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.ic86",	0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.ic89",	0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.ic88",	0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr7119.ic20",	0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) /* palette red component */
	ROM_LOAD( "pr7118.ic14",	0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) /* palette green component */
	ROM_LOAD( "pr7117.ic8",		0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) /* palette blue component */
	ROM_LOAD( "pr5317.ic28",	0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */

	ROM_REGION( 0x0618, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "315-5152.bin",   0x00000, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) ) /* PAL16R4A located at IC10. */
    ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) /* TI PAL16R4NC located at IC11. */
    ROM_LOAD( "315-5139.bin",   0x00000, 0x0104, NO_DUMP ) /* CK2605 located at IC50. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC7. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC13. */
    ROM_LOAD( "315-5025.bin",   0x00000, 0x0104, NO_DUMP ) /* Located at IC19. */
ROM_END

/*
    Choplifter (Bootleg)
    Year: 1985
    System 2



    Small Daughterboard marked 600A

      |--------------------------------------------------------|
      |                                                        |
    A |  74ls244  74ls244  74ls669  74ls669  74ls669  74ls669  |
      |                                                        |
    B |  74ls240  74ls240  74ls283  74ls283  74ls283  74ls283  |
      |                                                        |
    C |  74ls10   74ls86   74ls157  74ls157  74ls157  74ls157  |
      |                                                        |
    D |  74ls157  74ls157  74ls157  74ls139  74ls74            |
      |                                                        |
    E |  pal16r4  pal16l8  74ls161  74ls161  74ls109           |
      |                                                        |
    F |  74ls27   74ls08   74ls04   74ls74   74ls00            |
      |                                                 600A   |
      |--------------------------------------------------------|
           1        2        3        4        5        6


    Small Daughterboard marked 600B

      |--------------------------------------|
      |                              600B    |
    A |  74ls74            74ls174  pal20r4  |
      |                                      |
    B |  pal16l8  pal16l8  74ls374  74ls374  |
      |                                      |
    C |  74ls283  pal16l8  pal16l8  74ls32   |
      |                                      |
    D |  74ls283  74ls283  74ls85   74ls283  |
      |                                      |
    E |  74ls04   74ls00   74ls00   74ls32   |
      |                                      |
      |--------------------------------------|
           1        2        3        4
*/

ROM_START( chplftbl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ep7124bl.90",	0x00000, 0x8000, CRC(71a37932) SHA1(72b6f8949d356b3adc5248fdaa13c2a1b9c0fa70) )
	ROM_LOAD( "epr-7125.91",	0x10000, 0x8000, CRC(f5283498) SHA1(1ad40f6d7b4cd18212ee56917240c0796f1a4ec2) )
	ROM_LOAD( "epr-7126.92",	0x18000, 0x8000, CRC(dbd192ab) SHA1(03d280c82599a14fc6a2065d57c6241cdc6f1143) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.126",	0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7127.4",		0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.5",		0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.6",		0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr-7121.87",	0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.86",	0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.89",	0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.88",	0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr7119.20",		0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) /* palette red component */
	ROM_LOAD( "pr7118.14",		0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) /* palette green component */
	ROM_LOAD( "pr7117.8",		0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) /* palette blue component */
	ROM_LOAD( "pr5317.28",		0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */

	ROM_REGION( 0x0003, "plds_main", ROMREGION_DISPOSE )
    ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16R4 located at IC13. */
    ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16R4 located at IC14. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at IC62. */

	ROM_REGION( 0x0002, "plds_600a", ROMREGION_DISPOSE )
    ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16R4 located at E1. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at E2. */

	ROM_REGION( 0x0005, "plds_600b", ROMREGION_DISPOSE )
    ROM_LOAD( "pal20r4.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL20R4 located at A4. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at B1. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at B2. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at C2. */
    ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) /* PAL16L8 located at C3. */

	ROM_REGION( 0x0410, "plds_unk", ROMREGION_DISPOSE )
    /* Do any of these dumps match what's on the physical boards? */
	ROM_LOAD( "pal16r4a.ic9",         0x0000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) )
	ROM_LOAD( "pal16r4a.ic10",        0x0104, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) )
	ROM_LOAD( "pal16r4a-chopbl1.bin", 0x0208, 0x0104, CRC(e1628a8e) SHA1(6b6df079cfadec71b38a53f107475f0dda428b00) )
	ROM_LOAD( "pal16l8a-chopbl2.bin", 0x030c, 0x0104, CRC(afa7425d) SHA1(09d8607b69ecfc0b12c8610751d489500b63c7d6) )
ROM_END

ROM_START( 4dwarrio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4d.116",       0x0000, 0x4000, CRC(546d1bc7) SHA1(724bb2f77a2b82fae85e535ae4a37820cfb323d0) ) /* encrypted */
	ROM_LOAD( "4d.109",       0x4000, 0x4000, CRC(f1074ec3) SHA1(bc368abeb6c0a7172e03bd7a1754cf4a6ecbb4f8) ) /* encrypted */
	ROM_LOAD( "4d.96",        0x8000, 0x4000, CRC(387c1e8f) SHA1(520ecbafd1c7271dad24410a68067dfd801fa6d6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "4d.120",       0x0000, 0x2000, CRC(5241c009) SHA1(b7a21f95b63234f2496d5ea6e7dc8050ca1b39fc) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "4d.62",        0x0000, 0x2000, CRC(f31b2e09) SHA1(fdc288769495f4b0ca8c7594c9ab7dc0f29e57a4) )
	ROM_LOAD( "4d.61",        0x2000, 0x2000, CRC(5430e925) SHA1(55f92309223c41871175b1f54418c8b08339deb0) )
	ROM_LOAD( "4d.64",        0x4000, 0x2000, CRC(9f442351) SHA1(07076ef66e29c730050e38aecabdfbfced9f9bc4) )
	ROM_LOAD( "4d.63",        0x6000, 0x2000, CRC(633232bd) SHA1(c09c1df4f04608381d665a83776005607ad97ad4) )
	ROM_LOAD( "4d.66",        0x8000, 0x2000, CRC(52bfa2ed) SHA1(ea1c18d07957301f2006350b02fe40d13dbe2aa5) )
	ROM_LOAD( "4d.65",        0xa000, 0x2000, CRC(e9ba4658) SHA1(ba2581a52eb54e2d9f1e1bf30050280df3f5df1b) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "4d.117",       0x0000, 0x4000, CRC(436e4141) SHA1(2574d5c3b01c89d8a041c82af976147d3b87b36b) )
	ROM_LOAD( "4d.04",        0x4000, 0x4000, CRC(8b7cecef) SHA1(4851754cb56784ac248f699f0781646455dd556b) )
	ROM_LOAD( "4d.110",       0x8000, 0x4000, CRC(6ec5990a) SHA1(a26dbd470744c38a26a016e5d4792ac2f2b9bc4b) )
	ROM_LOAD( "4d.05",        0xc000, 0x4000, CRC(f31a1e6a) SHA1(f49dbc4b381e7096d5ffe3c16660dd63121dabf7) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( brain )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "brain.1",      0x00000, 0x8000, CRC(2d2aec31) SHA1(02dfbb0e9ca01b864e3aa594cf38306fe82a4b5d) )
	ROM_LOAD( "brain.2",      0x10000, 0x8000, CRC(810a8ab5) SHA1(87cd39f5b1047f355e1d257c691ef11fc55824ca) )
	ROM_RELOAD(               0x08000, 0x8000 )	/* there's code falling through from 7fff */
												/* so I have to copy the ROM there */
	ROM_LOAD( "brain.3",      0x18000, 0x8000, CRC(9a225634) SHA1(9f137938592dd9c5ab2273864a11a682e0f7f783) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "brain.120",    0x0000, 0x8000, CRC(c7e50278) SHA1(9709a59004c6bc39173d0cb94f3602c358367976) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "brain.62",     0x0000, 0x4000, CRC(7dce2302) SHA1(ebf15da3aea36f6a831a5395b0e5fc253852a3ee) )
	ROM_LOAD( "brain.64",     0x4000, 0x4000, CRC(7ce03fd3) SHA1(11f037c75d606276cbf4ec76a2cfdde94a756493) )
	ROM_LOAD( "brain.66",     0x8000, 0x4000, CRC(ea54323f) SHA1(08a4d2543a75a1fbb6ef2c126e3aeb4945bf458f) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "brain.117",    0x00000, 0x8000, CRC(92ff71a4) SHA1(856646c595e0ef7bbcf18844ee34b04e05893ffa) )
	ROM_LOAD( "brain.110",    0x08000, 0x8000, CRC(a1b847ec) SHA1(d71664822b9b863bd2a37da71b4e0850893b9876) )
	ROM_LOAD( "brain.4",      0x10000, 0x8000, CRC(fd2ea53b) SHA1(c7f2d267f19d2c27a550120e003ebfcb10d8af89) )
	/* 18000-1ffff empty */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, BAD_DUMP CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed)  ) /* palette red component */
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, BAD_DUMP CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76)  ) /* palette green component */
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, BAD_DUMP CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d)  ) /* palette blue component */
	ROM_LOAD( "pr5317.76",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( raflesia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7411.116",	0x0000, 0x4000, CRC(88a0c6c6) SHA1(1deaa8d8d607100966696e5e9dd5f799ba693af0) ) /* encrypted */
	ROM_LOAD( "epr-7412.109",	0x4000, 0x4000, CRC(d3b8cddf) SHA1(368c74d8ae46442cacdb67813dc1c039245da266) ) /* encrypted */
	ROM_LOAD( "epr-7413.96",	0x8000, 0x4000, CRC(b7e688b3) SHA1(ba5c6d5d19e7d51e41949fd5fa576fdae38f9c9c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7420.120",	0x0000, 0x2000, CRC(14387666) SHA1(9cb18e3002c32f658e4725707069f9cd2f496507) ) /* epr-7420.3 */

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7419.62",	0x0000, 0x2000, CRC(bfd5f34c) SHA1(78c4d380d5558212e535c3262223137447d64818) ) /* epr-7419.82 */
	ROM_LOAD( "epr-7418.61",	0x2000, 0x2000, CRC(f8cbc9b6) SHA1(48be9337f704a11ac1fdeb64a3b3518c796bcdd0) ) /* epr-7418.65 */
	ROM_LOAD( "epr-7417.64",	0x4000, 0x2000, CRC(e63501bc) SHA1(5cfd19241c54782c262bbb23c6f682534e77feb7) ) /* epr-7417.81 */
	ROM_LOAD( "epr-7416.63",	0x6000, 0x2000, CRC(093e5693) SHA1(78bb1c4651bd63a9f776766d2eac4f1c09242ed5) ) /* epr-7416.64 */
	ROM_LOAD( "epr-7415.66",	0x8000, 0x2000, CRC(1a8d6bd6) SHA1(b04ee35f603c6c9923ba888914eb43a8b7753d92) ) /* epr-7415.80 */
	ROM_LOAD( "epr-7414.65",	0xa000, 0x2000, CRC(5d20f218) SHA1(bdc0185d133f7bbe287106882bacde846634ffa4) ) /* epr-7414.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7407.117",	0x0000, 0x4000, CRC(f09fc057) SHA1(c6f06144b708055b31fbcba9f38b63736db789d8) ) /* epr-7407.3 */
	ROM_LOAD( "epr-7409.04",	0x4000, 0x4000, CRC(819fedb8) SHA1(e63f0422814423be91d8e1937a13d19693a1a5fc) ) /* epr-7409.1 */
	ROM_LOAD( "epr-7408.110",	0x8000, 0x4000, CRC(3189f33c) SHA1(8476c2c01920f0492cf643929d4f023f3afe0164) ) /* epr-7408.4 */
	ROM_LOAD( "epr-7410.05",	0xc000, 0x4000, CRC(ced74789) SHA1(d0ad845bfe83412ac8d43125e1c50d0581a5b47e) ) /* epr-7410.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( spatter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6392.116",	0x0000, 0x4000, CRC(329b4506) SHA1(8f71ffc3015c4fcf84a895bf53760830602f1040) ) /* encrypted */
	ROM_LOAD( "epr-6393.109",	0x4000, 0x4000, CRC(3b56e25f) SHA1(23f26f8632c8a370b5b3b7a3ec58f359cdf04f73) ) /* encrypted */
	ROM_LOAD( "epr-6394.96",	0x8000, 0x4000, CRC(647c1301) SHA1(5142abfcc63772fd1b47eb584ccda0bc3830e337) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6316.120",	0x0000, 0x2000, CRC(1df95511) SHA1(5780631c8c5a2c3fcd4085f217affa660d72a4e9) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6328.62",	0x0000, 0x2000, CRC(a2bf2832) SHA1(5d7047a6a0c0588a4e98b6ce94d5fd0e6ab963f9) )
	ROM_LOAD( "epr-6397.61",	0x2000, 0x2000, CRC(c60d4471) SHA1(9e8130d575fa342485dfe093e086a4b48e51b904) )
	ROM_LOAD( "epr-6326.64",	0x4000, 0x2000, CRC(269fbb4c) SHA1(7b91f551360698195bf9ce8e32dd2e8fa17e9db8) )
	ROM_LOAD( "epr-6396.63",	0x6000, 0x2000, CRC(c15ccf3b) SHA1(14809ab81816eedb85cacda042e437d48cf9b31a) )
	ROM_LOAD( "epr-6324.66",	0x8000, 0x2000, CRC(8ab3b563) SHA1(6ede93b9f1593dbcbabd6c875bac8ec01a1b40a2) )
	ROM_LOAD( "epr-6395.65",	0xa000, 0x2000, CRC(3f083065) SHA1(cb17c8c2fe04baa58863c10cd8f359a58def3417) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6306.04",	0x0000, 0x4000, CRC(e871e132) SHA1(55f7ab1a8c9a118911c64930452ea05f6ee37fc4) )
	ROM_LOAD( "epr-6308.117",	0x4000, 0x4000, CRC(99c2d90e) SHA1(5be54d931622892b7acc320e714d5b1cdce02d19) )
	ROM_LOAD( "epr-6307.05",	0x8000, 0x4000, CRC(0a5ad543) SHA1(5acada30c1affc4ffbebc8365a9ba4465f213d47) )
	ROM_LOAD( "epr-6309.110",	0xc000, 0x4000, CRC(7423ad98) SHA1(e19b4c64795f30e1491520160d315e4148d58df2) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( ssanchan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6310.116",	0x0000, 0x4000, CRC(26b43701) SHA1(e041bde10da12a3f698da09220f0a7cc2ee99abe) ) /* encrypted */
	ROM_LOAD( "epr-6311.109",	0x4000, 0x4000, CRC(cb2bc620) SHA1(ecc69360ad9fcc825b35955fbc29da9ea28b8846) ) /* encrypted */
	ROM_LOAD( "epr-6312.96",	0x8000, 0x4000, CRC(71b15b47) SHA1(7c955be049f9a8d7ca18d877183b698dd5ffe4da) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6316.120",	0x0000, 0x2000, CRC(1df95511) SHA1(5780631c8c5a2c3fcd4085f217affa660d72a4e9) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-6328.62",	0x0000, 0x2000, CRC(a2bf2832) SHA1(5d7047a6a0c0588a4e98b6ce94d5fd0e6ab963f9) )
	ROM_LOAD( "epr-6327.61",	0x2000, 0x2000, CRC(53298109) SHA1(75fd37034aee78d63939d8b4f584c1dc1042264b) )
	ROM_LOAD( "epr-6326.64",	0x4000, 0x2000, CRC(269fbb4c) SHA1(7b91f551360698195bf9ce8e32dd2e8fa17e9db8) )
	ROM_LOAD( "epr-6325.63",	0x6000, 0x2000, CRC(bf038745) SHA1(2fda2412f76b8971ba543ec10da07d4b0d1f2006) )
	ROM_LOAD( "epr-6324.66",	0x8000, 0x2000, CRC(8ab3b563) SHA1(6ede93b9f1593dbcbabd6c875bac8ec01a1b40a2) )
	ROM_LOAD( "epr-6323.65",	0xa000, 0x2000, CRC(0394673c) SHA1(fbee6a5cb37d0394db95781b9f165d766546eb33) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-6306.04",	0x0000, 0x4000, CRC(e871e132) SHA1(55f7ab1a8c9a118911c64930452ea05f6ee37fc4) )
	ROM_LOAD( "epr-6308.117",	0x4000, 0x4000, CRC(99c2d90e) SHA1(5be54d931622892b7acc320e714d5b1cdce02d19) )
	ROM_LOAD( "epr-6307.05",	0x8000, 0x4000, CRC(0a5ad543) SHA1(5acada30c1affc4ffbebc8365a9ba4465f213d47) )
	ROM_LOAD( "epr-6309.110",	0xc000, 0x4000, CRC(7423ad98) SHA1(e19b4c64795f30e1491520160d315e4148d58df2) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( wboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7489.116",	0x0000, 0x4000, CRC(130f4b70) SHA1(4a2ea5bc06f3a240c68813be3a9f9bef2bcf4e9c) ) /* encrypted */
	ROM_LOAD( "epr-7490.109",	0x4000, 0x4000, CRC(9e656733) SHA1(2233beb874b7cb48899afe603fef567932951a88) ) /* encrypted */
	ROM_LOAD( "epr-7491.96",	0x8000, 0x4000, CRC(1f7d0efe) SHA1(a1b4f8faf1614f4808df1292209c340f1490adbd) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",	0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wboyo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-.116",		0x0000, 0x4000, CRC(51d27534) SHA1(1cbc7201aacde89857f83b2600f309b514c5e758) ) /* encrypted */
	ROM_LOAD( "epr-.109",		0x4000, 0x4000, CRC(e29d1cd1) SHA1(f6ff4a6fffea77cc5706549bb2d8bf9e96ed0be0) ) /* encrypted */
	ROM_LOAD( "epr-7491.96",	0x8000, 0x4000, CRC(1f7d0efe) SHA1(a1b4f8faf1614f4808df1292209c340f1490adbd) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",	0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wboy2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7587.129",	0x0000, 0x2000, CRC(1bbb7354) SHA1(e299979299c93981f5d28a1a614ad644506911dd) ) /* encrypted */
	ROM_LOAD( "epr-7588.130",	0x2000, 0x2000, CRC(21007413) SHA1(f45443a49e916465e5c8a8b348897ab426a897bd) ) /* encrypted */
	ROM_LOAD( "epr-7589.131",	0x4000, 0x2000, CRC(44b30433) SHA1(558d799c8f48f76c651f19e2a81160eb78ac6642) ) /* encrypted */
	ROM_LOAD( "epr-7590.132",	0x6000, 0x2000, CRC(bb525a0b) SHA1(5cd4731e0adfb5c660144eccda759e12a30ce78e) ) /* encrypted */
	ROM_LOAD( "epr-7591.133",	0x8000, 0x2000, CRC(8379aa23) SHA1(da47e0150b724a00878ef5f953fa6ac80bb27d8d) )
	ROM_LOAD( "epr-7592.134",	0xa000, 0x2000, CRC(c767a5d7) SHA1(a4e8d6a8278ac2227bde8c24d45aa7ab2a273579) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",	0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) ) /* epr-7498.3 */

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) /* epr-7497.82 */
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) /* epr-7496.65 */
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) /* epr-7495.81 */
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) /* epr-7494.64 */
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) /* epr-7493.80 */
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) /* epr-7492.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) /* epr-7485.3 */
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) /* epr-7487.1 */
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) /* epr-7486.4 */
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) /* epr-7488.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( wboy2u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic129_02.bin",	0x0000, 0x2000, CRC(32c4b709) SHA1(e57b7b6818f12fdd5f1600ed54c0b8a7f538aa71) )
	ROM_LOAD( "ic130_03.bin",	0x2000, 0x2000, CRC(56463ede) SHA1(c58c220aa0d0e194581646e6db2491075fdc37b9) )
	ROM_LOAD( "ic131_04.bin",	0x4000, 0x2000, CRC(775ed392) SHA1(073f8f70685913736eb04be8215a47b5253cb531) )
	ROM_LOAD( "ic132_05.bin",	0x6000, 0x2000, CRC(7b922708) SHA1(c2e1f67b756f558d6904fe82d6f5483cda5f9045) )
	ROM_LOAD( "epr-7591.133",	0x8000, 0x2000, CRC(8379aa23) SHA1(da47e0150b724a00878ef5f953fa6ac80bb27d8d) )
	ROM_LOAD( "epr-7592.134",	0xa000, 0x2000, CRC(c767a5d7) SHA1(a4e8d6a8278ac2227bde8c24d45aa7ab2a273579) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7498a.3",		0x0000, 0x2000, CRC(c198205c) SHA1(d2d5cd154ce6a5a3c6a099b4ab2ea7cc045ab0a1) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) /* epr-7497.82 */
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) /* epr-7496.65 */
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) /* epr-7495.81 */
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) /* epr-7494.64 */
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) /* epr-7493.80 */
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) /* epr-7492.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) /* epr-7485.3 */
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) /* epr-7487.1 */
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) /* epr-7486.4 */
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) /* epr-7488.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( wboy3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wb_1",			0x0000, 0x4000, CRC(bd6fef49) SHA1(6469a84cc1fd4ebf8c58b6efd3b255414bc86699) ) /* encrypted */
	ROM_LOAD( "wb_2",			0x4000, 0x4000, CRC(4081b624) SHA1(892fd347638ec900a7afc3d338b68e9d0a14f2b4) ) /* encrypted */
	ROM_LOAD( "wb_3",			0x8000, 0x4000, CRC(c48a0e36) SHA1(c9b9e51334e8b698be2195dda7701bb51760e502) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",	0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

/*
This wonderboy romset runs on a system1 1985 pcb with some flying wires.
Serial number of the pcb is 257

There are 2 piggyback boards:

The first is marked "SEGA 834-5764"  and it is placed on the socket of the sega sys1 protection chip and on a eprom socket.
there are IC2 and IC3 eproms (I can't read the codes because the stickers are damaged)
There is also a 40 pin socket in which they have put an unknown 42 NEC cpu (they have scratched the codes) with pin 21 and 22 cut!

The second piggyback is marked "SEGA 834-5755" and it contains proms and some logic.
*/

ROM_START( wboy4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic2.bin",	0x0000, 0x8000, CRC(48b2c006) SHA1(35492330dae71d410712380466b4c09b81df8559) ) /* encrypted */
	ROM_LOAD( "ic3.bin",	0x8000, 0x8000, CRC(466cae31) SHA1(e47e9084c83796a0a0dfeaa1f8f868cadd5f32c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "7583.126", 0x0000, 0x8000, CRC(99334b3c) SHA1(dfc09f63082b7666fa2152e22810c0455a7e5051) )	// epr7583.ic120

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr7610.ic62",	0x0000, 0x4000, CRC(1685d26a) SHA1(d30d08d61d789fd5a0eb7ef2998eb9728dabf4c9) )
	ROM_LOAD( "epr7609.ic64",	0x4000, 0x4000, CRC(87ecba53) SHA1(b904d5af25e0c1f8c8ca8dc11a3bed508c868f19) )
	ROM_LOAD( "epr7608.ic66",	0x8000, 0x4000, CRC(e812b3ec) SHA1(3eebeaf3480a0370aa5ee031c25768ada17ad8a2) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "7578.87",  0x00000, 0x8000, CRC(6ff1637f) SHA1(9a6ddbd7b8d53273b30c3529b028c1f28bf3c63b) )	// epr7577.ic110
	ROM_LOAD( "7577.86",  0x08000, 0x8000, CRC(58b3705e) SHA1(1a8ff3f1765a3b21145bd1a6c85441f806f7b17d) )	// epr7576.ic117

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wboyu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic116_89.bin",	0x0000, 0x4000, CRC(73d8cef0) SHA1(a6f1f8de44a88f995836ce03b5a073306c56aaeb) )
	ROM_LOAD( "ic109_90.bin",	0x4000, 0x4000, CRC(29546828) SHA1(905d76bc2b212a161ad2f2bef144261bb73c94cb) )
	ROM_LOAD( "ic096_91.bin",	0x8000, 0x4000, CRC(c7145c2a) SHA1(0b2fd6f519a4b87bc27db5d03c489c7ff75e942a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",	0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) /* epr-7497.82 */
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) /* epr-7496.65 */
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) /* epr-7495.81 */
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) /* epr-7494.64 */
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) /* epr-7493.80 */
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) /* epr-7492.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "ic117_85.bin",	0x0000, 0x4000, CRC(1ee96ae8) SHA1(4e69b87e919894b961477e6cc5272f448495d847) )
	ROM_LOAD( "ic004_87.bin",	0x4000, 0x4000, CRC(119735bb) SHA1(001efa55d7fbcd2fdb6da17b136f295e5ea4a4c2) )
	ROM_LOAD( "ic110_86.bin",	0x8000, 0x4000, CRC(26d0fac4) SHA1(2e6a06f6850b2d19e3dd7dcdc6b700d0eda878cb) )
	ROM_LOAD( "ic005_88.bin",	0xc000, 0x4000, CRC(2602e519) SHA1(00e94ec7ae37b5063137d4d49af7806fb0357c4b) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wboysys2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "7580.90",  0x0000, 0x8000, CRC(d69927a5) SHA1(b633177146a83953131d4e03fa987416f222199a) ) /* encrypted */
	ROM_LOAD( "7579.91",  0x10000, 0x8000, CRC(8a6f4b00) SHA1(2b1c26daa2e9c668292db73e28318257c62b175c) ) /* encrypted */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "7583.126", 0x0000, 0x8000, CRC(99334b3c) SHA1(dfc09f63082b7666fa2152e22810c0455a7e5051) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "7581.4",   0x00000, 0x8000, CRC(d95565fd) SHA1(25f1653cca1d6432171a7b391cbb76bc18ddfb06) )
	ROM_LOAD( "7582.5",   0x08000, 0x8000, CRC(560cbac0) SHA1(851283e6d63e33d250840501dd22750b19772fb0) )
	ROM_LOAD( "7607.6",   0x10000, 0x8000, CRC(bd36df03) SHA1(7f7efac2c71fae48dd1dcb4dcc849f07e8127f7d) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "7578.87",  0x00000, 0x8000, CRC(6ff1637f) SHA1(9a6ddbd7b8d53273b30c3529b028c1f28bf3c63b) )
	ROM_LOAD( "7577.86",  0x08000, 0x8000, CRC(58b3705e) SHA1(1a8ff3f1765a3b21145bd1a6c85441f806f7b17d) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr-7345.20",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) /* palette red component */
	ROM_LOAD( "pr-7344.14",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) /* palette green component */
	ROM_LOAD( "pr-7343.8",      0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) /* palette blue component */
	ROM_LOAD( "pr-5317.28",     0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbdeluxe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wbd1.bin",		0x0000, 0x2000, CRC(a1bedbd7) SHA1(32d171847ca02b01a7ac810cac3185c81c923285) )
	ROM_LOAD( "ic130_03.bin",	0x2000, 0x2000, CRC(56463ede) SHA1(c58c220aa0d0e194581646e6db2491075fdc37b9) )
	ROM_LOAD( "wbd3.bin",		0x4000, 0x2000, CRC(6fcdbd4c) SHA1(4fb9a916c99bf267c0035cb80b16400732991f83) )
	ROM_LOAD( "ic132_05.bin",	0x6000, 0x2000, CRC(7b922708) SHA1(c2e1f67b756f558d6904fe82d6f5483cda5f9045) )
	ROM_LOAD( "wbd5.bin",		0x8000, 0x2000, CRC(f6b02902) SHA1(9a43b84d9537d70e9c0d75010a824bcaec57a50c) )
	ROM_LOAD( "wbd6.bin",		0xa000, 0x2000, CRC(43df21fe) SHA1(c1b88505942f48b0df2362bbb618689febe00d1f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7498a.3",		0x0000, 0x2000, CRC(c198205c) SHA1(d2d5cd154ce6a5a3c6a099b4ab2ea7cc045ab0a1) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr-7497.62",	0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) /* epr-7497.82 */
	ROM_LOAD( "epr-7496.61",	0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) /* epr-7496.65 */
	ROM_LOAD( "epr-7495.64",	0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) /* epr-7495.81 */
	ROM_LOAD( "epr-7494.63",	0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) /* epr-7494.64 */
	ROM_LOAD( "epr-7493.66",	0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) /* epr-7493.80 */
	ROM_LOAD( "epr-7492.65",	0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) /* epr-7492.63 */

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "epr-7485.117",	0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) /* epr-7485.3 */
	ROM_LOAD( "epr-7487.04",	0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) /* epr-7487.1 */
	ROM_LOAD( "epr-7486.110",	0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) /* epr-7486.4 */
	ROM_LOAD( "epr-7488.05",	0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) /* epr-7488.2 */

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr-5317.76",		0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
															 /* pr-5317.106 */
ROM_END

ROM_START( gardia )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr10255.1",   0x00000, 0x8000, CRC(89282a6b) SHA1(f19e345e5fae6a518276cc1bd09d1e2083672b25) ) /* encrypted */
	ROM_LOAD( "epr10254.2",   0x10000, 0x8000, CRC(2826b6d8) SHA1(de1faf33cca031b72052bf5244fcb0bd79d85659) )
	ROM_LOAD( "epr10253.3",   0x18000, 0x8000, CRC(7911260f) SHA1(44196f0a6c4c2b22a68609ddfc75be6a7877a69a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr10243.120", 0x0000, 0x4000, CRC(87220660) SHA1(3f2bfc03e0f1053a4aa0ec5ebb0d573f2e20964c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr10249.61",  0x0000, 0x4000, CRC(4e0ad0f2) SHA1(b76c155b674f3ad8938278d5dbb0452351c716a5) )
	ROM_LOAD( "epr10248.64",  0x4000, 0x4000, CRC(3515d124) SHA1(39b28a103d8bfe702a376ebd880d6060e3d1ab30) )
	ROM_LOAD( "epr10247.66",  0x8000, 0x4000, CRC(541e1555) SHA1(6660204c74a9f7e63b3ba08d99fb854aa863710e) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr10234.117", 0x00000, 0x8000, CRC(8a6aed33) SHA1(044836885ace8294124b1be9b3a4828f772bb9ee) )
	ROM_LOAD( "epr10233.110", 0x08000, 0x8000, CRC(c52784d3) SHA1(b37d7f261be12616dbe11dfa375eaf6878e4a0f3) )
	ROM_LOAD( "epr10236.04",  0x10000, 0x8000, CRC(b35ab227) SHA1(616f6097afddffa9af89fe84d8b6df59c567c1e6) )
	ROM_LOAD( "epr10235.5",   0x18000, 0x8000, CRC(006a3151) SHA1(a575f9d5c026e6b18e990720ec7520b6b5ae94e3) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) /* palette red component */
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) /* palette green component */
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) /* palette blue component */
	ROM_LOAD( "pr5317.4",     0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( gardiab )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gardiabl.5",   0x00000, 0x8000, CRC(207f9cbb) SHA1(647de15ac69a904344f3c18c9da8cefd626387db) ) /* encrypted */
	ROM_LOAD( "gardiabl.6",   0x10000, 0x8000, CRC(b2ed05dc) SHA1(c520bf7024c85dc759c27eccb0a31998f4d72b5f) )
	ROM_LOAD( "gardiabl.7",   0x18000, 0x8000, CRC(0a490588) SHA1(18df754ebdf062096f2d631a722b168901610345) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr10243.120", 0x0000, 0x4000, CRC(87220660) SHA1(3f2bfc03e0f1053a4aa0ec5ebb0d573f2e20964c) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gardiabl.8",   0x0000, 0x4000, CRC(367c9a17) SHA1(bde7592ce94bbc6674c04b427c52e74207066f56) )
	ROM_LOAD( "gardiabl.9",   0x4000, 0x4000, CRC(1540fd30) SHA1(e2d134e0715231a428fd112be81493a0e2a2642f) )
	ROM_LOAD( "gardiabl.10",  0x8000, 0x4000, CRC(e5c9af10) SHA1(6bff5bbc0f339e84a8e31446dc9897c02600fbcf) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr10234.117", 0x00000, 0x8000, CRC(8a6aed33) SHA1(044836885ace8294124b1be9b3a4828f772bb9ee) )
	ROM_LOAD( "epr10233.110", 0x08000, 0x8000, CRC(c52784d3) SHA1(b37d7f261be12616dbe11dfa375eaf6878e4a0f3) )
	ROM_LOAD( "epr10236.04",  0x10000, 0x8000, CRC(b35ab227) SHA1(616f6097afddffa9af89fe84d8b6df59c567c1e6) )
	ROM_LOAD( "epr10235.5",   0x18000, 0x8000, CRC(006a3151) SHA1(a575f9d5c026e6b18e990720ec7520b6b5ae94e3) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) /* palette red component */
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) /* palette green component */
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) /* palette blue component */
	ROM_LOAD( "pr5317.4",     0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( blockgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bg.116",       0x0000, 0x4000, CRC(a99b231a) SHA1(42ba45a4fd315255e9500bc3a0e8fe653c4c5a9c) ) /* encrypted */
	ROM_LOAD( "bg.109",       0x4000, 0x4000, CRC(a6b573d5) SHA1(33547a3895bbe65d5a6c40453eeb93e1fedad6de) ) /* encrypted */
	/* 0x8000-0xbfff empty (was same as My Hero) */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0029.key",  0x0000, 0x2000, CRC(350d7f93) SHA1(7ef12d63b2c7150f8e74f65ec8340471d72b1c03) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bg.120",       0x0000, 0x2000, CRC(d848faff) SHA1(5974cc0c3090800ca79f580a620f5b6615f5d039) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "bg.62",        0x0000, 0x2000, CRC(7e3ea4eb) SHA1(8bf020b083e2da12fe95ddae9ac7a385490525bc) )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, CRC(4dd3d39d) SHA1(759fca021f8d59e861dc19543d5a184428a5e472) )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, CRC(17368663) SHA1(e8f2ac6de0fddf08aefae07e693cac100cfb0db4) )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, CRC(0c8bc404) SHA1(fc96fb682da3af6b7fc852cea6d8a957c4ce57e3) )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, CRC(2b7dc4fa) SHA1(79d3677b24682cee0c08088433646800703be531) )
	ROM_LOAD( "bg.65",        0xa000, 0x2000, CRC(ed121306) SHA1(89f812b3954922e22fcf8d9cc4ee5ba295279cb6) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "bg.117",       0x0000, 0x4000, CRC(e99cc920) SHA1(b2b9199a9296e0c34fcf4dd20ffd3e8de08f42da) )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, CRC(213057f8) SHA1(a872631aaa2b73e9198f2ad6cede2a889279e610) )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, CRC(064c812c) SHA1(673790dc5131fd280333386a0e9915fb94e9f3e1) )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, CRC(02e0b040) SHA1(fb626fc31dfe25bf9fac0c8d76d5041609b06e82) )

	ROM_REGION( 0x0100, "user2", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( blckgalb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic62",         0x10000, 0x8000, CRC(65c47676) SHA1(bc283761e6f9ebf65fb405b1c8922c3c98c8d00e) ) /* decrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )             /* decrypted data */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bg.120",       0x0000, 0x2000, CRC(d848faff) SHA1(5974cc0c3090800ca79f580a620f5b6615f5d039) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "bg.62",        0x0000, 0x2000, CRC(7e3ea4eb) SHA1(8bf020b083e2da12fe95ddae9ac7a385490525bc) )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, CRC(4dd3d39d) SHA1(759fca021f8d59e861dc19543d5a184428a5e472) )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, CRC(17368663) SHA1(e8f2ac6de0fddf08aefae07e693cac100cfb0db4) )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, CRC(0c8bc404) SHA1(fc96fb682da3af6b7fc852cea6d8a957c4ce57e3) )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, CRC(2b7dc4fa) SHA1(79d3677b24682cee0c08088433646800703be531) )
	ROM_LOAD( "bg.65",        0xa000, 0x2000, CRC(ed121306) SHA1(89f812b3954922e22fcf8d9cc4ee5ba295279cb6) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* 64k for sprites data */
	ROM_LOAD( "bg.117",       0x0000, 0x4000, CRC(e99cc920) SHA1(b2b9199a9296e0c34fcf4dd20ffd3e8de08f42da) )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, CRC(213057f8) SHA1(a872631aaa2b73e9198f2ad6cede2a889279e610) )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, CRC(064c812c) SHA1(673790dc5131fd280333386a0e9915fb94e9f3e1) )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, CRC(02e0b040) SHA1(fb626fc31dfe25bf9fac0c8d76d5041609b06e82) )

	ROM_REGION( 0x0100, "user1", 0 ) /* misc PROMs, but not color so don't use "proms"! */
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( tokisens )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr10961.90",  0x00000, 0x8000, CRC(1466b61d) SHA1(99f93813834d3a7c9f6228076d400f74d9b6dea9) )
	ROM_LOAD( "epr10962.91",  0x10000, 0x8000, CRC(a8479f91) SHA1(0700746fb481fd2bd22ae82c9881aa61222a6379) )
	ROM_LOAD( "epr10963.92",  0x18000, 0x8000, CRC(b7193b39) SHA1(d40fb8591b1ff83f3d56b955ac11a07496a0adbb) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr10967.126", 0x0000, 0x8000, CRC(97966bf2) SHA1(b5a3d36afbb3d6e2e2e2c121609a30dc080ccf13) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr10964.4",   0x00000, 0x8000, CRC(9013b85c) SHA1(c27322245052ffc9d840fe683ed35965c61bf9e8) )
	ROM_LOAD( "epr10965.5",   0x08000, 0x8000, CRC(e4755cc6) SHA1(33370d556a70e19edce5e0c7fa8b11453ccbe91b) )
	ROM_LOAD( "epr10966.6",   0x10000, 0x8000, CRC(5bbfbdcc) SHA1(e7e679da874a79dfdda0be58d1352c192635296d) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr10958.87",  0x00000, 0x8000, CRC(fc2bcbd7) SHA1(6b9007f2057e4c860ecae4ba5db4e02b8aaae8fd) )
	ROM_LOAD( "epr10957.86",  0x08000, 0x8000, CRC(4ec56860) SHA1(9fd6ba8a68b4cb98183e8ac8643656c251f1c72d) )
	ROM_LOAD( "epr10960.89",  0x10000, 0x8000, CRC(880e0d44) SHA1(2b2dc144807d1d048ffe81bfd33a77ccf618dd3e) )
	ROM_LOAD( "epr10959.88",  0x18000, 0x8000, CRC(4deda48f) SHA1(12db2a69286f22cd8243be6faa9a075fafec1dfd) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "bprom.20",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) /* palette red component */
	ROM_LOAD( "bprom.14",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) /* palette green component */
	ROM_LOAD( "bprom.8",       0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) /* palette blue component */
	ROM_LOAD( "bprom.28",      0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbml )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ep11031a.90",  0x00000, 0x8000, CRC(bd3349e5) SHA1(65cc16e5d3b08429388946df254b8122ad1da339) ) /* encrypted */
	ROM_LOAD( "epr11032.91",  0x10000, 0x8000, CRC(9d03bdb2) SHA1(7dbab23e7c7972d9b51a0d3d046374720b7d6af5) ) /* encrypted */
	ROM_LOAD( "epr11033.92",  0x18000, 0x8000, CRC(7076905c) SHA1(562fbd9bd60851f7e4e60b725193395b4f193479) ) /* encrypted */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0043.key",  0x0000, 0x2000, CRC(e354abfc) SHA1(07b0d3c51301ebb25909234b6220a3ed20dbcc7d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbmljo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr11031.90",  0x00000, 0x8000, CRC(497ebfb4) SHA1(d90872c7d5285c85b05879bc67638f640e0339d5) ) /* encrypted */
	ROM_LOAD( "epr11032.91",  0x10000, 0x8000, CRC(9d03bdb2) SHA1(7dbab23e7c7972d9b51a0d3d046374720b7d6af5) ) /* encrypted */
	ROM_LOAD( "epr11033.92",  0x18000, 0x8000, CRC(7076905c) SHA1(562fbd9bd60851f7e4e60b725193395b4f193479) ) /* encrypted */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0043.key",  0x0000, 0x2000, CRC(e354abfc) SHA1(07b0d3c51301ebb25909234b6220a3ed20dbcc7d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbmljb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )             /* Now load the operands in RAM */
	ROM_LOAD( "m-6.bin",      0x30000, 0x8000, CRC(8c08cd11) SHA1(5103f3c887c213b09aee858c4a883f2869b9ffb5) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "m-7.bin",      0x38000, 0x8000, CRC(11881703) SHA1(b5e4d477158e7653b0fef5a4806be7b4871e917d) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbmlb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )             /* Now load the operands in RAM */
	ROM_LOAD( "wbml.02",      0x30000, 0x8000, CRC(48746bb6) SHA1(a0049cba53e7548afa8d7b16a7e9494e628d2a0f) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "wbml.03",      0x38000, 0x8000, CRC(d57ba8aa) SHA1(16f095cb78e31af5ce76d36c20fe4c3e0d027aea) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "wbml.08",      0x00000, 0x8000, CRC(bbea6afe) SHA1(ba56c6789a35eb57cd226296ebf57e9aa19ba625) )
	ROM_LOAD( "wbml.09",      0x08000, 0x8000, CRC(77567d41) SHA1(2ac501661522615859f8a1718dbb8451272d6931) )
	ROM_LOAD( "wbml.10",      0x10000, 0x8000, CRC(a52ffbdd) SHA1(609375112268b770a798186697ecab5853f29f89) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( wbmlbg )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "galaxy.ic90",  0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x00000, 0x8000 )             /* Now load the operands in RAM */
	ROM_LOAD( "galaxy.ic91",  0x30000, 0x8000, CRC(89a8ab93) SHA1(11389604017e15aed9a8fcef60e42740acd79917) ) /* Unencrypted opcodes */
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "galaxy.ic92",  0x38000, 0x8000, CRC(39e07286) SHA1(70192f03e52dd34c9fe5698a5ec1c24d3c58543c) ) /* Unencrypted opcodes */
	ROM_RELOAD(               0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "galaxy.ic4", 0x00000, 0x8000, CRC(ab75d056) SHA1(d90d9c723536d0ec21900dc70b51715300b01fe7) )
	ROM_LOAD( "galaxy.ic6", 0x08000, 0x8000, CRC(6bb5e601) SHA1(465d67dcde4e775d1b93640ef1a300e958cbe707) )
	ROM_LOAD( "galaxy.ic5", 0x10000, 0x8000, CRC(3c11d151) SHA1(7b0c6792ae919ac309a709ca0c89006487e1d6e9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( dakkochn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr11224.90",  0x00000, 0x8000, CRC(9fb1972b) SHA1(1bb61c6ec2b5b8eb39f74f20d5bcd0f14501bd21) ) /* encrypted */
	ROM_LOAD( "epr11225.91",  0x10000, 0x8000, CRC(c540f9e2) SHA1(dbda9355e8b796bcfaee2789714d248c4d7ad58c) ) /* encrypted */
	/* 18000-1ffff empty */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0014.key",  0x0000, 0x2000, CRC(bb9df5ad) SHA1(7e7b7255149ae01d19883ecf4a88989f8a9bf4c6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11229.126", 0x0000, 0x8000, CRC(c11648d0) SHA1(c2df3d767d497c3365ae70748c4790f4ee394958) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11226.4",   0x00000, 0x8000, CRC(3dbc2f78) SHA1(f3f7ee2c0bedcc21c1c1f5394838af6d0a8833d8) )
	ROM_LOAD( "epr11227.5",   0x08000, 0x8000, CRC(34156e8d) SHA1(e23d8604a3d5db413cf150f9891fca2b1e0163fa) )
	ROM_LOAD( "epr11228.6",   0x10000, 0x8000, CRC(fdd5323f) SHA1(c47099c78207bb2258d34b98b48e3c04beb6407e) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11221.87",  0x00000, 0x8000, CRC(f9a44916) SHA1(9d9ba96146cff4c1ed18b7134ab19919e144d326) )
	ROM_LOAD( "epr11220.86",  0x08000, 0x8000, CRC(fdd25d8a) SHA1(53636e2a9102a2d86277cff812639d1d92587fc4) )
	ROM_LOAD( "epr11223.89",  0x10000, 0x8000, CRC(538adc55) SHA1(542af53a56f580e5ab455aa6bed955ee5fd4a252) )
	ROM_LOAD( "epr11222.88",  0x18000, 0x8000, CRC(33fab0b2) SHA1(eb3c08009315e46590c2c0df17fc3fa391034c66) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11219.20",   0x0000, 0x0100, CRC(45e252d9) SHA1(92d8f1d0f1a9e65234521ce02d512f08b5e06d78) ) /* palette red component */
	ROM_LOAD( "pr11218.14",   0x0100, 0x0100, CRC(3eda3a1b) SHA1(cc98c792521845259088eb163a150cd5bb603d5d) ) /* palette green component */
	ROM_LOAD( "pr11217.8",    0x0200, 0x0100, CRC(49dbde88) SHA1(7057da5617de7e4775adf092cce1709135066129) ) /* palette blue component */
	ROM_LOAD( "pr5317.37",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( ufosensi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr11661.90",  0x00000, 0x8000, CRC(f3e394e2) SHA1(a295a2aa80a164a548995822c46f32fd9fad7a0b) ) /* encrypted */
	ROM_LOAD( "epr11662.91",  0x10000, 0x8000, CRC(0c2e4120) SHA1(d81fbefa95868e3efd29ef3bacf108329781ca17) ) /* encrypted */
	ROM_LOAD( "epr11663.92",  0x18000, 0x8000, CRC(4515ebae) SHA1(9b823f10999746292762c2f0a1ca9039efa22506) ) /* encrypted */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0064.key",  0x0000, 0x2000, CRC(da326f36) SHA1(0871b351379a094ac578e0eca5cb17797f9085aa) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11667.126", 0x0000, 0x8000, CRC(110baba9) SHA1(e14cf5af11ac9691eca897bbae7c238665cd2a4d) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11664.4",   0x00000, 0x8000, CRC(1b1bc3d5) SHA1(2a09e0dbe2d467c151dce705f249367df849eaeb) )
	ROM_LOAD( "epr11665.5",   0x08000, 0x8000, CRC(3659174a) SHA1(176d2436abb45827a8d387241082854f55dc0314) )
	ROM_LOAD( "epr11666.6",   0x10000, 0x8000, CRC(99dcc793) SHA1(ad1d0acb60e7c1a7016955e142ebca1cf07b4908) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11658.87",  0x00000, 0x8000, CRC(3b5a20f7) SHA1(03e0934b0913c3a2cadf1d28b8a700d70b80fbac) )
	ROM_LOAD( "epr11657.86",  0x08000, 0x8000, CRC(010f81a9) SHA1(1b7ee05c80edfa403e32c216fa69387ca556895e) )
	ROM_LOAD( "epr11660.89",  0x10000, 0x8000, CRC(e1e2e7c5) SHA1(434039a70049a6e74e2a2f48b60345f720e6b1af) )
	ROM_LOAD( "epr11659.88",  0x18000, 0x8000, CRC(286c7286) SHA1(449a19ea9a9f9df47005e8dac1b8eacaebc515e7) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11656.20",   0x0000, 0x0100, CRC(640740eb) SHA1(9a601a3665f612d00c70019d33c7abd3cca9434b) ) /* palette red component */
	ROM_LOAD( "pr11655.14",   0x0100, 0x0100, CRC(a0c3fa77) SHA1(cdffa1de06d30ec421323145dfc3271803fc25d4) ) /* palette green component */
	ROM_LOAD( "pr11654.8",    0x0200, 0x0100, CRC(ba624305) SHA1(eb1d0dde60f81ff510ac8c1212e0ed5703febaf3) ) /* palette blue component */
	ROM_LOAD( "pr5317.28",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

ROM_START( ufosensb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "k108.ic18.3-4s", 0x20000, 0x8000, CRC(6b1d0955) SHA1(dbda145d40eaecd30c1d55a9675c58a2967c20c4) )
	ROM_CONTINUE(            	0x00000, 0x8000 )             /* Now load the operands in RAM */
	ROM_LOAD( "k109.ic19.4s",   0x30000, 0x8000, CRC(fc543b26) SHA1(b9e1d2ca6f9811bf341edf104fe209dbf56e4b2d) )
	ROM_CONTINUE(               0x10000, 0x8000 )
	ROM_LOAD( "k110.ic20.4-5s", 0x38000, 0x8000, CRC(6ba2dc77) SHA1(09a65f55988ae28e285d402af9a2a1f1dc05a82c) )
	ROM_CONTINUE(               0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr11667.126", 0x0000, 0x8000, CRC(110baba9) SHA1(e14cf5af11ac9691eca897bbae7c238665cd2a4d) ) /* label on chip is "k111.ic168.10v" */

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "epr11664.4",   0x00000, 0x8000, CRC(1b1bc3d5) SHA1(2a09e0dbe2d467c151dce705f249367df849eaeb) ) /* label on chip is "k101.ic72.6d" */
	ROM_LOAD( "epr11665.5",   0x08000, 0x8000, CRC(3659174a) SHA1(176d2436abb45827a8d387241082854f55dc0314) ) /* label on chip is "k102.ic73.6-7d" */
	ROM_LOAD( "epr11666.6",   0x10000, 0x8000, CRC(99dcc793) SHA1(ad1d0acb60e7c1a7016955e142ebca1cf07b4908) ) /* label on chip is "k103.ic74.7d" */

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "epr11658.87",  0x00000, 0x8000, CRC(3b5a20f7) SHA1(03e0934b0913c3a2cadf1d28b8a700d70b80fbac) ) /* label on chip is "k105.ic15.1-2s" */
	ROM_LOAD( "epr11657.86",  0x08000, 0x8000, CRC(010f81a9) SHA1(1b7ee05c80edfa403e32c216fa69387ca556895e) ) /* label on chip is "k104.ic14.1s" */
	ROM_LOAD( "epr11660.89",  0x10000, 0x8000, CRC(e1e2e7c5) SHA1(434039a70049a6e74e2a2f48b60345f720e6b1af) ) /* label on chip is "k107.ic17.2-3s" */
	ROM_LOAD( "epr11659.88",  0x18000, 0x8000, CRC(286c7286) SHA1(449a19ea9a9f9df47005e8dac1b8eacaebc515e7) ) /* label on chip is "k106.ic16.2s" */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "pr11656.20",   0x0000, 0x0100, CRC(640740eb) SHA1(9a601a3665f612d00c70019d33c7abd3cca9434b) ) /* palette red component - label on chip is "74s287.ic134.9f" */
	ROM_LOAD( "pr11655.14",   0x0100, 0x0100, CRC(a0c3fa77) SHA1(cdffa1de06d30ec421323145dfc3271803fc25d4) ) /* palette green component - label on chip is "74s287.ic133.9e" */
	ROM_LOAD( "pr11654.8",    0x0200, 0x0100, CRC(ba624305) SHA1(eb1d0dde60f81ff510ac8c1212e0ed5703febaf3) ) /* palette blue component - label on chip is "74s287.ic132.9d" */
	ROM_LOAD( "pr5317.28",    0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) - label on chip is "74s287.ic115.8j" */

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "pal6l8.ic3.1c",   0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6l8.ic32.2c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6l8.ic33.2d",  0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6l8.ic4.1d",   0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6r4.1",        0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6r4.2",        0x0A00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6r4.ic34.2f",  0x0C00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal6r4.ic5.1f",   0x0E00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal20r4.ic69.4c", 0x1000, 0x0144, NO_DUMP ) /* PAL is read protected */
ROM_END

/*

Noboranka
Data East, 1986

PCB Layout
----------

Top

DE-0222-2                           /-Sub PCB on top
|-------------------------------|--/-----------|
| DSW2   PK-2                   |              |
| DSW1                          |              |
|                             6116             |
|                             6116             |
|        20MHz                  |              |
|J                            DM02             |
|A                            DM01-------------|
|M       8255                 DM00             |
|M  DM03 Z80A                                  |
|A  6116                                       |
|                                              |
|                                         DC-11|
|  76489                                       |
|  76489         *DM-12.IC3               PK-1 |
|MB3730 VOL 8MHz *DM-11       6116             |
|----------------------------------------------|
Notes:
      * - These parts below PCB on a small sub-board DE-0271-0
      PK1/PK2/DM-11 - PALs
      DC-11/DM-12 - 82S129 PROMs
      Z80A clock - 4.00MHz [20/5]
      76489 clock - 2.00MHz [8/4]
      VSync - 60.095Hz
      HSync - 15.444kHz


Sub PCB

DE-0272-0
|---------------|
|               |
|               |
| 8751H         |
|               |
|8MHz           |
|               |
|---------------|
8751 clock - 8.000MHz, labelled 'DM'


Bottom

DE-0223-2
|----------------------------------------------|
|          DECO_291-0  Z80B                    |
|                                              |
|    PK-3          TC15G008AP                  |
|                                       CXK5864|
|    PK-4      DM04        TMM2018             |
|                                         DM08 |
|              DM05        TMM2018             |
|                                         DM09 |
|              DM06                            |
|                                         DM10 |
|              DM07                            |
|                            8147       8147   |
|                                              |
|                   CXK5814  2148              |
|                   CXK5814  2148              |
|----------------------------------------------|
Notes:
      PK3/PK4 - PALS
      DECO_291-0 - Custom DIP28
      TC15G008AP - Gate Array DIP48
      Z80B clock - 4.00MHz [20/5]

*/

ROM_START( nob )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dm08.1f", 0x00000, 0x8000, CRC(98d602d6) SHA1(a0f1e6d243f2e07703bb641434dce46d0ddc15ae) )
	ROM_LOAD( "dm10.1k", 0x10000, 0x8000, CRC(e7c06663) SHA1(8ae42b0875afe60ef672f2285aeb72da1c7e167b) )
	ROM_LOAD( "dm09.1h", 0x18000, 0x8000, CRC(dc4c872f) SHA1(aab85203cfd2463ffddfd48e87733fb8d6d8bcf6) )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "8751.mcu", 0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dm03.9h", 0x0000, 0x4000, CRC(415adf76) SHA1(fbd6f8921aa3246702983ba81fa9ae53fa10c19d) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "dm02.13b", 0x08000, 0x8000, CRC(f12df039) SHA1(159de205f77fd74da30717054e6ddda2c0bb63d0) )
	ROM_LOAD( "dm01.12b", 0x00000, 0x8000, CRC(446fbcdd) SHA1(e3c8364eccfa6c8af7a57b599238b0e4ebe8cc59) )
	ROM_LOAD( "dm00.10b", 0x10000, 0x8000, CRC(35f396df) SHA1(ebf0a252513ae2b31ef012ac71d64fb20b8725cc) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "dm04.5f", 0x00000, 0x8000, CRC(2442b86d) SHA1(2eed80e1ff9cd782990142d0d73ca4fa13db4731) )
	ROM_LOAD( "dm06.5k", 0x08000, 0x8000, CRC(e33743a6) SHA1(56dce565523f19e673c9272992030386ca648e41) )
	ROM_LOAD( "dm05.5h", 0x10000, 0x8000, CRC(7fbba01d) SHA1(ded22806ae0d6642b45cd33c0ceab67390a6e319) )
	ROM_LOAD( "dm07.5l", 0x18000, 0x8000, CRC(85e7a29f) SHA1(0ca77c66599650f157450d703682ec114f0453cf) )

	ROM_REGION( 0x0400, "proms", 0 )
	/* the first 2 proms were missing from the dump, but are clearly needed... */
	ROM_LOAD( "nobo_pr.16d", 0x0000, 0x0100, CRC(95010ac2) SHA1(deaf84b408cd1f3396eb851ef04cc1654d5e9a46) ) /* palette red component */
	ROM_LOAD( "nobo_pr.15d", 0x0100, 0x0100, CRC(c55aac0c) SHA1(0f7f2d383a90e9f7f319626b4d5565805f44a1f9) ) /* palette green component */
	ROM_LOAD( "dm-12.ic3", 0x0200, 0x0100, CRC(de394cee) SHA1(511c53f22459e5e238b48685f85b10f5e15f2ac1) ) /* palette blue component */
	ROM_LOAD( "dc-11.6a", 0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

/* the bootleg has different protection.. */
 ROM_START( nobb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "nobo-t.bin", 0x00000, 0x8000, CRC(176fd168) SHA1(f262521f07e5340f175019e2a06a54120a4aa3b7) )
	ROM_LOAD( "nobo-r.bin", 0x10000, 0x8000, CRC(d61cf3c9) SHA1(0f80011d713c51e67853810813ebba579ade0303) )
	ROM_LOAD( "nobo-s.bin", 0x18000, 0x8000, CRC(b0e7697f) SHA1(ad5394ca629152a8c73fb85d3fce8ea620ae6ff1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "nobo-m.bin", 0x0000, 0x4000, CRC(415adf76) SHA1(fbd6f8921aa3246702983ba81fa9ae53fa10c19d) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "nobo-j.bin", 0x08000, 0x8000, CRC(f12df039) SHA1(159de205f77fd74da30717054e6ddda2c0bb63d0) )
	ROM_LOAD( "nobo-k.bin", 0x00000, 0x8000, CRC(446fbcdd) SHA1(e3c8364eccfa6c8af7a57b599238b0e4ebe8cc59) )
	ROM_LOAD( "nobo-l.bin", 0x10000, 0x8000, CRC(35f396df) SHA1(ebf0a252513ae2b31ef012ac71d64fb20b8725cc) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* 128k for sprites data */
	ROM_LOAD( "nobo-q.bin", 0x00000, 0x8000, CRC(2442b86d) SHA1(2eed80e1ff9cd782990142d0d73ca4fa13db4731) )
	ROM_LOAD( "nobo-o.bin", 0x08000, 0x8000, CRC(e33743a6) SHA1(56dce565523f19e673c9272992030386ca648e41) )
	ROM_LOAD( "nobo-p.bin", 0x10000, 0x8000, CRC(7fbba01d) SHA1(ded22806ae0d6642b45cd33c0ceab67390a6e319) )
	ROM_LOAD( "nobo-n.bin", 0x18000, 0x8000, CRC(85e7a29f) SHA1(0ca77c66599650f157450d703682ec114f0453cf) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "nobo_pr.16d", 0x0000, 0x0100, CRC(95010ac2) SHA1(deaf84b408cd1f3396eb851ef04cc1654d5e9a46) ) /* palette red component */
	ROM_LOAD( "nobo_pr.15d", 0x0100, 0x0100, CRC(c55aac0c) SHA1(0f7f2d383a90e9f7f319626b4d5565805f44a1f9) ) /* palette green component */
	ROM_LOAD( "nobo_pr.14d", 0x0200, 0x0100, CRC(de394cee) SHA1(511c53f22459e5e238b48685f85b10f5e15f2ac1) ) /* palette blue component */
	ROM_LOAD( "nobo_pr.13a", 0x0300, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) /* timing? (not used) */
ROM_END

/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

static DRIVER_INIT( regulus )	{ regulus_decode(machine, "maincpu"); }
static DRIVER_INIT( mrviking )	{ mrviking_decode(machine, "maincpu"); }
static DRIVER_INIT( swat )	{ swat_decode(machine, "maincpu"); }
static DRIVER_INIT( flicky )	{ flicky_decode(machine, "maincpu"); }
static DRIVER_INIT( wmatch )	{ wmatch_decode(machine, "maincpu"); }
static DRIVER_INIT( bullfgtj )	{ bullfgtj_decode(machine, "maincpu"); }
static DRIVER_INIT( spatter )	{ spatter_decode(machine, "maincpu"); }
static DRIVER_INIT( pitfall2 )	{ pitfall2_decode(machine, "maincpu"); }
static DRIVER_INIT( nprinces )	{ nprinces_decode(machine, "maincpu"); }
static DRIVER_INIT( seganinj )	{ seganinj_decode(machine, "maincpu"); }
static DRIVER_INIT( imsorry )	{ imsorry_decode(machine, "maincpu"); }
static DRIVER_INIT( teddybb )	{ teddybb_decode(machine, "maincpu"); }
static DRIVER_INIT( hvymetal )	{ hvymetal_decode(machine, "maincpu"); }
static DRIVER_INIT( myheroj )	{ myheroj_decode(machine, "maincpu"); }
static DRIVER_INIT( 4dwarrio )	{ fdwarrio_decode(machine, "maincpu"); }
static DRIVER_INIT( wboy )	{ astrofl_decode(machine, "maincpu"); }
static DRIVER_INIT( wboy2 )	{ wboy2_decode(machine, "maincpu"); }
static DRIVER_INIT( gardia )	{ gardia_decode(machine, "maincpu"); }
static DRIVER_INIT( gardiab )	{ gardiab_decode(machine, "maincpu"); }



static DRIVER_INIT( blockgal )	{ mc8123_decrypt_rom(machine, "maincpu", "user1", 0, 0); }
static DRIVER_INIT( wbml )	{ mc8123_decrypt_rom(machine, "maincpu", "user1", 1, 4); }
static DRIVER_INIT( ufosensi )  { mc8123_decrypt_rom(machine, "maincpu", "user1", 1, 4); }

static UINT8 dakkochn_control;

/*
This game doesn't seem to have any mux writes, so I'm assuming that the mux does HW auto-incrementing.
I need schematics to understand how it's properly mapped anyway, because it could be for example tied
with vblank/video bits.
The program flow is:
I/O $00 R
I/O $15 W 0xe
I/O $15 W 0xc
I/O $00 R
I/O $15 W 0xe
(and so on)

*/
static READ8_HANDLER( dakkochn_port_00_r )
{
	static UINT8 res;

	switch(dakkochn_mux_data)
	{
		case 0: res = input_port_read(space->machine, "KEY0"); break;
		case 1: res = input_port_read(space->machine, "KEY1"); break;
		case 2: res = input_port_read(space->machine, "KEY2"); break;
		case 3: res = input_port_read(space->machine, "KEY3"); break;
		case 4: res = input_port_read(space->machine, "KEY4"); break;
		case 5: res = input_port_read(space->machine, "KEY5"); break;
		case 6: res = input_port_read(space->machine, "KEY6"); break;
	}

	dakkochn_mux_data++;
	if(dakkochn_mux_data >= 7)
		dakkochn_mux_data = 0;

	return res;
}

static READ8_HANDLER( dakkochn_port_03_r )
{
	return 0;
}

static READ8_HANDLER( dakkochn_port_04_r )
{
	return 0;
}

static WRITE8_HANDLER( dakkochn_port_15_w )
{
	dakkochn_control = data; // check if any control multiplex bits are in here!
	chplft_videomode_w(space,offset,data);
}

static DRIVER_INIT( dakkochn )
{
	mc8123_decrypt_rom(machine, "maincpu", "user1", 1, 4);

	memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_IO), 0x00, 0x00, 0, 0, dakkochn_port_00_r);
	memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_IO), 0x03, 0x03, 0, 0, dakkochn_port_03_r);
	memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_IO), 0x04, 0x04, 0, 0, dakkochn_port_04_r);

	memory_install_write8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_IO), 0x15, 0x15, 0, 0, dakkochn_port_15_w);

}


static DRIVER_INIT( myherok )
{
	int A;
	UINT8 *rom;

	/* additionally to the usual protection, all the program ROMs have data lines */
	/* D0 and D1 swapped. */
	rom = memory_region(machine, "maincpu");
	for (A = 0;A < 0xc000;A++)
		rom[A] = (rom[A] & 0xfc) | ((rom[A] & 1) << 1) | ((rom[A] & 2) >> 1);

	/* the tile gfx ROMs are mangled as well: */
	rom = memory_region(machine, "gfx1");

	/* the first ROM has data lines D0 and D6 swapped. */
	for (A = 0x0000;A < 0x4000;A++)
		rom[A] = (rom[A] & 0xbe) | ((rom[A] & 0x01) << 6) | ((rom[A] & 0x40) >> 6);

	/* the second ROM has data lines D1 and D5 swapped. */
	for (A = 0x4000;A < 0x8000;A++)
		rom[A] = (rom[A] & 0xdd) | ((rom[A] & 0x02) << 4) | ((rom[A] & 0x20) >> 4);

	/* the third ROM has data lines D0 and D6 swapped. */
	for (A = 0x8000;A < 0xc000;A++)
		rom[A] = (rom[A] & 0xbe) | ((rom[A] & 0x01) << 6) | ((rom[A] & 0x40) >> 6);

	/* also, all three ROMs have address lines A4 and A5 swapped. */
	for (A = 0;A < 0xc000;A++)
	{
		int A1;
		UINT8 temp;

		A1 = (A & 0xffcf) | ((A & 0x0010) << 1) | ((A & 0x0020) >> 1);
		if (A < A1)
		{
			temp = rom[A];
			rom[A] = rom[A1];
			rom[A1] = temp;
		}
	}

	myheroj_decode(machine, "maincpu");
}

static DRIVER_INIT( nobb )
{
	/* Patch to get PRG ROMS ('T', 'R' and 'S) status as "GOOD" in the "test mode" */
	/* not really needed */

//  UINT8 *ROM = memory_region(machine, "maincpu");

//  ROM[0x3296] = 0x18;     // 'jr' instead of 'jr z' - 'T' (PRG Main ROM)
//  ROM[0x32be] = 0x18;     // 'jr' instead of 'jr z' - 'R' (Banked ROM 1)
//  ROM[0x32ea] = 0x18;     // 'jr' instead of 'jr z' - 'S' (Banked ROM 2)

	/* Patch to avoid the internal checksum that will hang the game after an amount of time
       (check code at 0x3313 in 'R' (banked ROM 1)) */

//  ROM[0x10000 + 0 * 0x8000 + 0x3347] = 0x18;  // 'jr' instead of 'jr z'

	/* Patch to get sound in later levels(the program enters into a tight loop)*/
	UINT8 *ROM2 = memory_region(machine, "soundcpu");

	ROM2[0x02f9] = 0x28;//'jr z' instead of 'jr'
}


static DRIVER_INIT( bootleg )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	memory_set_decrypted_region(space, 0x0000, 0x7fff, memory_region(machine, "maincpu") + 0x10000);
}


static DRIVER_INIT( bootlegb )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	memory_set_decrypted_region(space, 0x0000, 0x7fff, memory_region(machine, "maincpu") + 0x20000);
	memory_configure_bank_decrypted(machine, 1, 0, 4, memory_region(machine, "maincpu") + 0x30000, 0x4000);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, starjack, 0,        small,    starjack, 0,        ROT270, "Sega",            "Star Jacker (Sega)", GAME_SUPPORTS_SAVE )
GAME( 1983, starjacs, starjack, small,    starjacs, 0,        ROT270, "Stern",           "Star Jacker (Stern)", GAME_SUPPORTS_SAVE )
GAME( 1983, regulus,  0,        small,	  regulus,  regulus,  ROT270, "Sega",            "Regulus (315-5033, Rev A.)", GAME_SUPPORTS_SAVE )
GAME( 1983, reguluso, regulus,  small,	  reguluso, regulus,  ROT270, "Sega",            "Regulus (315-5033)", GAME_SUPPORTS_SAVE )
GAME( 1983, regulusu, regulus,  small,	  regulus,  0,        ROT270, "Sega",            "Regulus (not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1983, upndown,  0,        system1,  upndown,  nprinces, ROT270, "Sega",            "Up'n Down (315-5030)", GAME_SUPPORTS_SAVE )
GAME( 1983, upndownu, upndown,  system1,  upndown,  0,        ROT270, "Sega",            "Up'n Down (not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1984, mrviking, 0,        small,    mrviking, mrviking, ROT270, "Sega",            "Mister Viking (315-5041)", GAME_SUPPORTS_SAVE )
GAME( 1984, mrvikngj, mrviking, small,    mrvikngj, mrviking, ROT270, "Sega",            "Mister Viking (315-5041, Japan)", GAME_SUPPORTS_SAVE )
GAME( 1984, swat,     0,        system1,  swat,     swat,     ROT270, "Coreland / Sega", "SWAT (315-5048)", GAME_SUPPORTS_SAVE )
GAME( 1984, flicky,   0,        system1,  flicky,   flicky,   ROT0,   "Sega",            "Flicky (128k Version, System 2, 315-5051)", GAME_SUPPORTS_SAVE )
GAME( 1984, flicks2,  flicky,   system1,  flicky,   0,        ROT0,   "Sega",            "Flicky (128k Version, System 2, not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1984, flickyo,  flicky,   system1,  flicky,   flicky,   ROT0,   "Sega",            "Flicky (64k Version, System 1, 315-5051, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1984, flicks1,  flicky,   system1,  flicky,   flicky,   ROT0,   "Sega",            "Flicky (64k Version, System 1, 315-5051, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1984, wmatch,   0,        small, 	  wmatch,   wmatch,   ROT270, "Sega",            "Water Match (315-5064)", GAME_SUPPORTS_SAVE )
GAME( 1984, bullfgt,  0,        system1,  bullfgt,  bullfgtj, ROT0,   "Coreland / Sega", "Bullfight (315-5065)", GAME_SUPPORTS_SAVE )
GAME( 1984, thetogyu, bullfgt,  system1,  bullfgt,  bullfgtj, ROT0,   "Coreland / Sega", "The Togyu (315-5065, Japan)", GAME_SUPPORTS_SAVE )
GAME( 1984, spatter,  0,        small,    spatter,  spatter,  ROT0,   "Sega",            "Spatter", GAME_SUPPORTS_SAVE )
GAME( 1984, ssanchan, spatter,  small,    spatter,  spatter,  ROT0,   "Sega",            "Sanrin San Chan (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1985, pitfall2, 0,        pitfall2, pitfall2, pitfall2, ROT0,   "Sega",            "Pitfall II (315-5093)", GAME_SUPPORTS_SAVE )
GAME( 1985, pitfalla, pitfall2, pitfall2, pitfall2, pitfall2, ROT0,   "Sega",            "Pitfall II (315-5093, Flicky Conversion)", GAME_SUPPORTS_SAVE )
GAME( 1985, pitfallu, pitfall2, pitfall2, pitfallu, 0,        ROT0,   "Sega",            "Pitfall II (not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1985, seganinj, 0,        system1,  seganinj, seganinj, ROT0,   "Sega",            "Sega Ninja (315-5102)", GAME_SUPPORTS_SAVE )
GAME( 1985, seganinu, seganinj, system1,  seganinj, 0,        ROT0,   "Sega",            "Sega Ninja (not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1985, ninja,    seganinj, system1,  seganinj, seganinj, ROT0,   "Sega",            "Ninja (315-5102)", GAME_SUPPORTS_SAVE )
GAME( 1985, nprinces, seganinj, system1,  seganinj, flicky,   ROT0,   "bootleg?",        "Ninja Princess (315-5051, 64k Ver. bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1985, nprincso, seganinj, system1,  seganinj, nprinces, ROT0,   "Sega",            "Ninja Princess (315-5098, 128k Ver.)", GAME_SUPPORTS_SAVE )
GAME( 1985, nprincsu, seganinj, system1,  seganinj, 0,        ROT0,   "Sega",            "Ninja Princess (64k Ver. not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1985, nprincsb, seganinj, system1,  seganinj, flicky,   ROT0,   "bootleg?",        "Ninja Princess (315-5051?, 128k Ver. bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1985, imsorry,  0,        system1,  imsorry,  imsorry,  ROT0,   "Coreland / Sega", "I'm Sorry (315-5110, US)", GAME_SUPPORTS_SAVE )
GAME( 1985, imsorryj, imsorry,  system1,  imsorry,  imsorry,  ROT0,   "Coreland / Sega", "Gonbee no I'm Sorry (315-5110, Japan)", GAME_SUPPORTS_SAVE )
GAME( 1985, teddybb,  0,        system1,  teddybb,  teddybb,  ROT0,   "Sega",            "TeddyBoy Blues (315-5115, New Ver.)", GAME_SUPPORTS_SAVE )
GAME( 1985, teddybbo, teddybb,  system1,  teddybb,  teddybb,  ROT0,   "Sega",            "TeddyBoy Blues (315-5115, Old Ver.)", GAME_SUPPORTS_SAVE )
GAME( 1985, myhero,   0,        system1,  myhero,   0,        ROT0,   "Sega",            "My Hero (US, not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1985, sscandal, myhero,   system1,  myhero,   myheroj,  ROT0,   "Coreland / Sega", "Seishun Scandal (315-5132, Japan)", GAME_SUPPORTS_SAVE )
GAME( 1985, myherok,  myhero,   system1,  myhero,   myherok,  ROT0,   "Coreland / Sega", "My Hero (Korea)", GAME_SUPPORTS_SAVE )
GAME( 1985, hvymetal, 0,        hvymetal, hvymetal, hvymetal, ROT0,   "Sega",            "Heavy Metal (315-5135)", GAME_SUPPORTS_SAVE )
GAME( 1985, shtngmst, 0,        shtngmst, shtngmst, 0,        ROT0,   "Sega",            "Shooting Master (Rev A, 8751 315-5159a)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
GAME( 1985, shtngms1, shtngmst, shtngmst, shtngmst, 0,        ROT0,   "Sega",            "Shooting Master (8751 315-5159)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
GAME( 1985, shtngmsa, shtngmst, shtngmst, shtngmst, 0,        ROT0,   "Sega [EVG]",      "Shooting Master (EVG, 8751 315-5159)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
GAME( 1985, chplft,   0,        chplft,   chplft,   0,        ROT0,   "Sega",            "Choplifter (8751 315-5151)", GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
GAME( 1985, chplftb,  chplft,   chplft,   chplft,   0,        ROT0,   "Sega",            "Choplifter (unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1985, chplftbl, chplft,   chplft,   chplft,   0,        ROT0,   "bootleg",         "Choplifter (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1985, 4dwarrio, 0,        system1,  4dwarrio, 4dwarrio, ROT0,   "Coreland / Sega", "4-D Warriors (315-5162)", GAME_SUPPORTS_SAVE )
GAME( 1986, brain,    0,        brain,    brain,    0,        ROT0,   "Coreland / Sega", "Brain", GAME_SUPPORTS_SAVE )
GAME( 1986, raflesia, 0,        system1,  raflesia, 4dwarrio, ROT270, "Coreland / Sega", "Rafflesia (315-5162)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboy,     0,        system1,  wboy,     wboy,     ROT0,   "Sega (Escape license)", "Wonder Boy (set 1, 315-5177)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboyo,    wboy,     system1,  wboy,     hvymetal, ROT0,   "Sega (Escape license)", "Wonder Boy (set 1, 315-5135)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboy2,    wboy,     system1,  wboy,     wboy2,    ROT0,   "Sega (Escape license)", "Wonder Boy (set 2, 315-5178)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboy2u,   wboy,     system1,  wboy,     0,        ROT0,   "Sega (Escape license)", "Wonder Boy (set 2, not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboy3,    wboy,     system1,  wboy3,    hvymetal, ROT0,   "Sega (Escape license)", "Wonder Boy (set 3, 315-5135)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboy4,    wboy,     system1,  wboy,     4dwarrio, ROT0,   "Sega (Escape license)", "Wonder Boy (315-5162, 4-D Warriors Conversion)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboyu,    wboy,     system1,  wboyu,    0,        ROT0,   "Sega (Escape license)", "Wonder Boy (not encrypted)", GAME_SUPPORTS_SAVE )
GAME( 1986, wboysys2, wboy,     wbml,     wboysys2, wboy,     ROT0,   "Sega (Escape license)", "Wonder Boy (system 2)", GAME_SUPPORTS_SAVE )
GAME( 1986, wbdeluxe, wboy,     system1,  wbdeluxe, 0,        ROT0,   "Sega (Escape license)", "Wonder Boy Deluxe", GAME_SUPPORTS_SAVE )
GAME( 1986, gardia,   0,        brain,    gardia,   gardia,   ROT270, "Sega / Coreland", "Gardia (317-0006)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE)
GAME( 1986, gardiab,  gardia,   gardiab,  gardia,   gardiab,  ROT270, "bootleg",         "Gardia (317-0007?, bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1986, nob,      nobb,     nob,      nob,      0,        ROT270, "Data East Corporation", "Noboranka (Japan)", GAME_NOT_WORKING )
GAME( 1986, nobb,     0,        nobb,     nob,      nobb,     ROT270, "bootleg",               "Noboranka (Japan, bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1987, blockgal, 0,        system1,  blockgal, blockgal, ROT90,  "Sega / Vic Tokai", "Block Gal (MC-8123B, 317-0029)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE)
GAME( 1987, blckgalb, blockgal, blckgalb, blockgal, bootleg,  ROT90,  "bootleg",         "Block Gal (bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, tokisens, 0,        wbml,     tokisens, 0,        ROT90,  "Sega",            "Toki no Senshi - Chrono Soldier", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, wbml,     0,        wbml,     wbml,     wbml,     ROT0,   "Sega / Westone",  "Wonder Boy in Monster Land (Japan New Ver., MC-8123, 317-0043)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, wbmljo,   wbml,     wbml,     wbml,     wbml,     ROT0,   "Sega / Westone",  "Wonder Boy in Monster Land (Japan Old Ver., MC-8123, 317-0043)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, wbmljb,   wbml,     wbml,     wbml,     bootlegb, ROT0,   "bootleg",         "Wonder Boy in Monster Land (Japan not encrypted)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, wbmlb,    wbml,     wbml,     wbml,     bootlegb, ROT0,   "bootleg",         "Wonder Boy in Monster Land (English bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE)
GAME( 1987, wbmlbg,   wbml,     wbml,     wbml,     bootlegb, ROT0,   "bootleg",         "Wonder Boy in Monster Land (Galaxy Electronics English bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE)
GAME( 1987, dakkochn, 0,        dakkochn, dakkochn, dakkochn, ROT0,   "Whiteboard",      "DakkoChan House (MC-8123, 317-0014)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1988, ufosensi, 0,        ufosensi, ufosensi, ufosensi, ROT0,   "Sega",            "Ufo Senshi Yohko Chan (MC-8123, 317-0064)", GAME_SUPPORTS_SAVE )
GAME( 1988, ufosensb, ufosensi, ufosensi, ufosensi, bootlegb, ROT0,   "bootleg",         "Ufo Senshi Yohko Chan (not encrypted)", GAME_SUPPORTS_SAVE )
