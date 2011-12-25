/******************************************************************************************

PC-XT (c) 1987 IBM

(Actually Arcade games running on more or less modified PC-XT HW)

driver by Angelo Salese & Chris Hardy
original tetriunk.c by David Haywood & Tomasz Slanina

Notes:
- The Korean Tetris is a blantant rip-off of the Mirrorsoft/Andromeda Software Tetris PC
  version;

TODO:
- 02851: tetriskr: Corrupt game graphics after some time of gameplay, caused by a wrong
  reading of the i/o $3c8 bit 1.
- Filetto: Add a proper FDC device.
- Filetto: Add sound,"buzzer" PC sound plus the UM5100 sound chip ,might be connected to the
  prototyping card;
- Korean Tetris: Add the aforementioned "buzzer" plus identify if there's any kind of sound
  chip on it;

********************************************************************************************
Filetto HW notes:
The PCB is a un-modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.Additionally there's also a UM5100 sound
chip for the sound.
PCB Part Number: S/N 90289764 NOVARXT
PCB Contents:
1x UMC 8923S-UM5100 voice processor (upper board)
1x MMI PAL16L8ACN-940CRK9 (upper board)
1x AMD AMPAL16R8APC-8804DM (upper board)
1x AMD P8088-1 main processor 8.000MHz (lower board)
1x Proton PT8010AF PLCC 28.636MHz (lower board)
1x UMC 8928LP-UM8272A floppy disk controller (lower board)
1x UMC 8935CS-UM82C11 Printer Adapter Interface (lower board)
1x UMC 8936CS-UM8250B Programmable asynchronous communications element (lower board)
There isn't any keyboard found connected to the pcb.
********************************************************************************************
Filetto SW notes:
The software of this game can be extracted with a normal Windows program extractor.
The files names are:
-command.com  (1)
-ibmbio.com   (1)
-ibmdos.com   (1)
-ansi.sys     (1)
-config.sys   (2)
-autoexec.bat (3)
-x.exe        (4)
(1)This is an old Italian version of MS-DOS (v3.30 18th March 1987).
(2)Contains "device=ansi.sys",it's an hook-up for the graphics used by the BIOS.
(3)It has an Echo off (as you can notice from the game itself) and then the loading of the
main program (x.exe).
(4)The main program,done in plain Basic with several Italian comments in it.The date of
the main program is 9th October 1990.

******************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pit8253.h"
#include "machine/8255ppi.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "sound/hc55516.h"
#include "sound/speaker.h"
#include "video/pc_cga.h"


class pcxt_state : public driver_device
{
public:
	pcxt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_bg_bank;
	int m_bank;
	int m_lastvalue;
	UINT8 m_disk_data[2];
	UINT8 m_port_b_data;
	UINT8 m_wss1_data;
	UINT8 m_wss2_data;
	UINT8 m_status;
	UINT8 m_clr_status;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_pc_spkrdata, m_pc_input;

	device_t	*m_pit8253;
	device_t	*m_pic8259_1;
	device_t	*m_pic8259_2;
	device_t	*m_dma8237_1;
	device_t	*m_dma8237_2;
};

static SCREEN_UPDATE( tetriskr )
{
	pcxt_state *state = screen->machine().driver_data<pcxt_state>();
	int x,y;
	int yi;
	const UINT8 *bg_rom = screen->machine().region("gfx2")->base();

	//popmessage("%04x",m_start_offs);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine()));

	for(y=0;y<200/8;y++)
	{
		for(yi=0;yi<8;yi++)
		{
			for(x=0;x<320/8;x++)
			{
				UINT8 color;
				int xi,pen_i;

				for(xi=0;xi<8;xi++)
				{
					color = 0;
					/* TODO: first byte seems bogus? */
					for(pen_i = 0;pen_i<4;pen_i++)
						color |= ((bg_rom[y*320/8+x+(pen_i*0x20000)+yi*0x400+state->m_bg_bank*0x2000+1] >> (7-xi)) & 1) << pen_i;

					if((x+xi)<screen->visible_area().max_x && ((y)+yi)<screen->visible_area().max_y)
						*BITMAP_ADDR16(bitmap, y*8+yi, x*8+xi) = screen->machine().pens[color];
				}
			}
		}
	}

	SCREEN_UPDATE_CALL(mc6845_cga);
	return 0;
}


static READ8_HANDLER( disk_iobank_r )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	//printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,cpu_get_pc(&space->device()));
	//if(offset == 0) return input_port_read(space->machine(), "DSW");
	if(offset == 1) return input_port_read(space->machine(), "IN1");

	return state->m_disk_data[offset];
}

static WRITE8_HANDLER( disk_iobank_w )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
/*
    BIOS does a single out $0310,$F0 on reset

    Then does 2 outs to set the bank..

        X1  X2

        $F0 $F2 = m0
        $F1 $F2 = m1
        $F0 $F3 = m2
        $F1 $F3 = m3

    The sequence of

    out $0310,X1
    out $0310,X2

    sets the selected rom that appears in $C0000-$CFFFF

*/
	int newbank = 0;

//  printf("bank %d set to %02X\n", offset,data);

	if (data == 0xF0)
	{
		newbank = 0;
	}
	else
	{
		if((state->m_lastvalue == 0xF0) && (data == 0xF2))
			newbank = 0;
		else if ((state->m_lastvalue == 0xF1) && (data == 0xF2))
			newbank = 1;
		else if ((state->m_lastvalue == 0xF0) && (data == 0xF3))
			newbank = 2;
		else if ((state->m_lastvalue == 0xF1) && (data == 0xF3))
			newbank = 3;
	}

//  printf("newbank = %d\n", newbank);

	if (newbank != state->m_bank)
	{
		state->m_bank = newbank;
		memory_set_bankptr(space->machine(),  "bank1",space->machine().region("user1")->base() + 0x10000 * state->m_bank );
	}

	state->m_lastvalue = data;

	state->m_disk_data[offset] = data;
}

/*********************************
Pit8253
*********************************/

UINT8 pc_speaker_get_spk(running_machine &machine)
{
	pcxt_state *state = machine.driver_data<pcxt_state>();
	return state->m_pc_spkrdata & state->m_pc_input;
}


void pc_speaker_set_spkrdata(running_machine &machine, UINT8 data)
{
	device_t *speaker = machine.device("speaker");
	pcxt_state *state = machine.driver_data<pcxt_state>();
	state->m_pc_spkrdata = data ? 1 : 0;
	speaker_level_w( speaker, pc_speaker_get_spk(machine) );
}


void pc_speaker_set_input(running_machine &machine, UINT8 data)
{
	device_t *speaker = machine.device("speaker");
	pcxt_state *state = machine.driver_data<pcxt_state>();
	state->m_pc_input = data ? 1 : 0;
	speaker_level_w( speaker, pc_speaker_get_spk(machine) );
}


static WRITE_LINE_DEVICE_HANDLER( ibm5150_pit8253_out2_changed )
{
	pc_speaker_set_input( device->machine(), state );
}


static const struct pit8253_config pc_pit8253_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_LINE(ibm5150_pit8253_out2_changed)
		}
	}
};


static READ8_DEVICE_HANDLER( port_a_r )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	if(!(state->m_port_b_data & 0x80))//???
	{
		/*
        x--- ---- Undefined (Always 0)
        -x-- ---- B: Floppy disk drive installed.
        --xx ---- Default Display Mode
        ---- xx-- Undefined (Always 1)
        ---- --x- 8087 NDP installed
        ---- ---x Undefined (Always 1)
        */
		return state->m_wss1_data;
	}
	else//keyboard emulation
	{
		//cputag_set_input_line(device->machine(), "maincpu", 1, PULSE_LINE);
		return 0x00;//Keyboard is disconnected
		//return 0xaa;//Keyboard code
	}
}

static READ8_DEVICE_HANDLER( port_b_r )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	return state->m_port_b_data;
}

static READ8_DEVICE_HANDLER( port_c_r )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	int timer2_output = pit8253_get_output( state->m_pit8253, 2 );
	if ( state->m_port_b_data & 0x01 )
	{
		state->m_wss2_data = ( state->m_wss2_data & ~0x10 ) | ( timer2_output ? 0x10 : 0x00 );
	}
	state->m_wss2_data = ( state->m_wss2_data & ~0x20 ) | ( timer2_output ? 0x20 : 0x00 );

	return state->m_wss2_data;//TODO
}

/*'buzzer' sound routes here*/
/* Filetto uses this for either beep and um5100 sound routing,probably there's a mux somewhere.*/
/* The Korean Tetris uses it as a regular buzzer,probably the sound is all in there...*/
static WRITE8_DEVICE_HANDLER( port_b_w )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();

	/* PPI controller port B*/
	pit8253_gate2_w(state->m_pit8253, BIT(data, 0));
	pc_speaker_set_spkrdata( device->machine(), data & 0x02 );
	state->m_port_b_data = data;
// device_t *beep = device->machine().device("beep");
// device_t *cvsd = device->machine().device("cvsd");
//  hc55516_digit_w(cvsd, data);
//  popmessage("%02x\n",data);
//  beep_set_state(beep, 0);
//  beep_set_state(beep, 1);
//  beep_set_frequency(beep, state->m_port_b_data);
}

static WRITE8_DEVICE_HANDLER( wss_1_w )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	state->m_wss1_data = data;
}

static WRITE8_DEVICE_HANDLER( wss_2_w )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	state->m_wss2_data = data;
}

static WRITE8_DEVICE_HANDLER( sys_reset_w )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_RESET, PULSE_LINE);
}


static const ppi8255_interface filetto_ppi8255_intf[2] =
{
	{
		DEVCB_HANDLER(port_a_r),		/* Port A read */
		DEVCB_HANDLER(port_b_r),		/* Port B read */
		DEVCB_HANDLER(port_c_r),		/* Port C read */
		DEVCB_NULL, 					/* Port A write */
		DEVCB_HANDLER(port_b_w),		/* Port B write */
		DEVCB_NULL						/* Port C write */
	},
	{
		DEVCB_NULL,						/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_HANDLER(wss_1_w),			/* Port A write */
		DEVCB_HANDLER(wss_2_w),			/* Port B write */
		DEVCB_HANDLER(sys_reset_w)		/* Port C write */
	}
};

/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

static READ8_HANDLER( fdc765_status_r )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	UINT8 tmp;
//  popmessage("Read FDC status @ PC=%05x",cpu_get_pc(&space->device()));
	tmp = state->m_status | 0x80;
	state->m_clr_status++;
	if(state->m_clr_status == 0x10)
	{
		state->m_status = 0;
		state->m_clr_status = 0;
	}
	return tmp;
}

static READ8_HANDLER( fdc765_data_r )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	state->m_status = (FDC_READ);
	return 0xc0;
}

static WRITE8_HANDLER( fdc765_data_w )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	state->m_status = (FDC_WRITE);
}


static WRITE8_HANDLER( drive_selection_w )
{
//	pcxt_state *state = space->machine().driver_data<pcxt_state>();

	/* TODO: properly hook-up upd765 FDC there */
	pic8259_ir6_w(space->machine().device("pic8259_1"), 1);
}

/******************
DMA8237 Controller
******************/


static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}

static READ8_HANDLER(dma_page_select_r)
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	UINT8 data = state->m_at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = state->m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = state->m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = state->m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = state->m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(dma_page_select_w)
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();
	state->m_at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		state->m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		state->m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		state->m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		state->m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	pcxt_state *drvstate = device->machine().driver_data<pcxt_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { set_dma_channel(device, 0, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { set_dma_channel(device, 1, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { set_dma_channel(device, 2, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { set_dma_channel(device, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_LINE(pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_LINE(pc_dack0_w), DEVCB_LINE(pc_dack1_w), DEVCB_LINE(pc_dack2_w), DEVCB_LINE(pc_dack3_w) }
};

/******************
8259 IRQ controller
******************/

static WRITE_LINE_DEVICE_HANDLER( pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(state->m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface pic8259_1_config =
{
	DEVCB_LINE(pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static IRQ_CALLBACK(irq_callback)
{
	pcxt_state *state = device->machine().driver_data<pcxt_state>();
	return pic8259_acknowledge(state->m_pic8259_1);
}

static ADDRESS_MAP_START( filetto_map, AS_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x9ffff) AM_RAM //work RAM 640KB
	AM_RANGE(0xa0000, 0xbffff) AM_RAM
	AM_RANGE(0xc0000, 0xcffff) AM_ROMBANK("bank1")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcxt_io_common, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE("dma8237_1", i8237_r, i8237_w ) //8237 DMA Controller
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE("pic8259_1", pic8259_r, pic8259_w ) //8259 Interrupt control
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_r, pit8253_w)    //8253 PIT
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0064, 0x0066) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)  //PPI 8255
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE_MODERN("rtc", mc146818_device, read, write)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE(dma_page_select_r,dma_page_select_w)
	AM_RANGE(0x00a0, 0x00af) AM_DEVREADWRITE("pic8259_2", pic8259_r, pic8259_w )
	AM_RANGE(0x0278, 0x027f) AM_RAM //printer (parallel) port latch
	AM_RANGE(0x02f8, 0x02ff) AM_RAM //Modem port
	AM_RANGE(0x0378, 0x037f) AM_RAM //printer (parallel) port
	AM_RANGE(0x03bc, 0x03bf) AM_RAM //printer port
	AM_RANGE(0x03f2, 0x03f2) AM_WRITE(drive_selection_w)
	AM_RANGE(0x03f4, 0x03f4) AM_READ(fdc765_status_r) //765 Floppy Disk Controller (FDC) Status
	AM_RANGE(0x03f5, 0x03f5) AM_READWRITE(fdc765_data_r,fdc765_data_w)//FDC Data
	AM_RANGE(0x03f8, 0x03ff) AM_RAM //rs232c (serial) port
ADDRESS_MAP_END

static ADDRESS_MAP_START( filetto_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_IMPORT_FROM( pcxt_io_common )
//  AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x0201, 0x0201) AM_READ_PORT("COIN") //game port
	AM_RANGE(0x0310, 0x0311) AM_READWRITE(disk_iobank_r,disk_iobank_w) //Prototyping card
	AM_RANGE(0x0312, 0x0312) AM_READ_PORT("IN0") //Prototyping card,read only
ADDRESS_MAP_END

static WRITE8_HANDLER( tetriskr_bg_bank_w )
{
	pcxt_state *state = space->machine().driver_data<pcxt_state>();

	state->m_bg_bank = (data & 0x0f) ^ 8;
}

static ADDRESS_MAP_START( tetriskr_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_IMPORT_FROM( pcxt_io_common )
	AM_RANGE(0x0200, 0x020f) AM_RAM //game port
	AM_RANGE(0x03c0, 0x03c0) AM_WRITE(tetriskr_bg_bank_w)
	AM_RANGE(0x03c8, 0x03c8) AM_READ_PORT("IN0")
	AM_RANGE(0x03c9, 0x03c9) AM_READ_PORT("IN1")
//  AM_RANGE(0x03ce, 0x03ce) AM_READ_PORT("IN1")
ADDRESS_MAP_END

static INPUT_PORTS_START( filetto )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Extra Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "Play at 6th match reached" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tetriskr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //probably unused
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("IN1") //dip-switches?
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/* F4 Character Displayer */
static const gfx_layout pc_16_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8					/* every char takes 2 x 8 bytes */
};

static const gfx_layout pc_8_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	512,					/* 512 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8					/* every char takes 8 bytes */
};

static GFXDECODE_START( pcxt )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout,    3, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pc_8_charlayout,   3, 1 )
GFXDECODE_END


static MACHINE_RESET( filetto )
{
	pcxt_state *state = machine.driver_data<pcxt_state>();
	device_t *speaker = machine.device("speaker");
	state->m_bank = -1;
	state->m_lastvalue = -1;
	device_set_irq_callback(machine.device("maincpu"), irq_callback);
	state->m_pit8253 = machine.device( "pit8253" );
	state->m_pic8259_1 = machine.device( "pic8259_1" );
	state->m_pic8259_2 = machine.device( "pic8259_2" );
	state->m_dma8237_1 = machine.device( "dma8237_1" );
	state->m_dma8237_2 = machine.device( "dma8237_2" );

	state->m_pc_spkrdata = 0;
	state->m_pc_input = 0;

	speaker_level_w( speaker, 0 );
}

static MACHINE_CONFIG_START( filetto, pcxt_state )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(filetto_map)
	MCFG_CPU_IO_MAP(filetto_io)

	MCFG_MACHINE_RESET( filetto )

	MCFG_PIT8253_ADD( "pit8253", pc_pit8253_config )

	MCFG_PPI8255_ADD( "ppi8255_0", filetto_ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", filetto_ppi8255_intf[1] )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )

	MCFG_PIC8259_ADD( "pic8259_1", pic8259_1_config )

	MCFG_PIC8259_ADD( "pic8259_2", pic8259_2_config )

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_FRAGMENT_ADD( pcvideo_cga_320x200 ) // TODO
	MCFG_GFXDECODE(pcxt)

	/*Sound Hardware*/
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("voice", HC55516, 8000000/4)//8923S-UM5100 is a HC55536 with ROM hook-up
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

//  PC "buzzer" sound
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tetriskr, filetto )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(tetriskr_io)

	MCFG_DEVICE_MODIFY("screen")
	MCFG_VIDEO_START(pc_cga_superimpose)
	MCFG_SCREEN_UPDATE(tetriskr)

	MCFG_DEVICE_REMOVE("voice")
MACHINE_CONFIG_END

ROM_START( filetto )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("u49.bin", 0xfc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0xf8000, 0x2000 )
	ROM_RELOAD(         0xf4000, 0x2000 )
	ROM_RELOAD(         0xf0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xfe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xfa000, 0x2000 )
	ROM_RELOAD(         0xf6000, 0x2000 )
	ROM_RELOAD(         0xf2000, 0x2000 )

	ROM_REGION( 0x40000, "user1", 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )

	ROM_REGION( 0x40000, "user2", 0 ) // UM5100 sample roms?
	ROM_LOAD16_BYTE("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD16_BYTE("v2.u14",  0x00001, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END

ROM_START( tetriskr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0xf0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )

	ROM_REGION( 0x2000, "gfx1",ROMREGION_ERASE00 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x1800, 0x0800, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )
	ROM_IGNORE( 0x1800 )

	ROM_REGION( 0x80000+1, "gfx2",ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))
ROM_END

static DRIVER_INIT( filetto )
{
	//...
}

static DRIVER_INIT( tetriskr )
{
	//...
}

GAME( 1990, filetto,  0, filetto,  filetto,  filetto,  ROT0,  "Novarmatic", "Filetto (v1.05 901009)",GAME_IMPERFECT_SOUND )
GAME( 1988?,tetriskr, 0, tetriskr, tetriskr, tetriskr, ROT0,  "bootleg",    "Tetris (bootleg of Mirrorsoft PC-XT Tetris version)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
