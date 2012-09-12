/***************************************************************************

    Midway MCR system

***************************************************************************/

#include "emu.h"
#include "audio/midway.h"
#include "includes/mcr.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT8 mcr_cocktail_flip;

UINT32 mcr_cpu_board;
UINT32 mcr_sprite_board;



/*************************************
 *
 *  Statics
 *
 *************************************/

static emu_timer *ipu_watchdog_timer;


/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static TIMER_CALLBACK( ipu_watchdog_reset );
static WRITE8_DEVICE_HANDLER( ipu_break_changed );



/*************************************
 *
 *  Graphics declarations
 *
 *************************************/

const gfx_layout mcr_bg_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ STEP2(RGN_FRAC(1,2),1), STEP2(RGN_FRAC(0,2),1) },
	{ STEP8(0,2) },
	{ STEP8(0,16) },
	16*8
};


const gfx_layout mcr_sprite_layout =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ STEP4(0,1) },
	{ STEP2(RGN_FRAC(0,4)+0,4), STEP2(RGN_FRAC(1,4)+0,4), STEP2(RGN_FRAC(2,4)+0,4), STEP2(RGN_FRAC(3,4)+0,4),
	  STEP2(RGN_FRAC(0,4)+8,4), STEP2(RGN_FRAC(1,4)+8,4), STEP2(RGN_FRAC(2,4)+8,4), STEP2(RGN_FRAC(3,4)+8,4),
	  STEP2(RGN_FRAC(0,4)+16,4), STEP2(RGN_FRAC(1,4)+16,4), STEP2(RGN_FRAC(2,4)+16,4), STEP2(RGN_FRAC(3,4)+16,4),
	  STEP2(RGN_FRAC(0,4)+24,4), STEP2(RGN_FRAC(1,4)+24,4), STEP2(RGN_FRAC(2,4)+24,4), STEP2(RGN_FRAC(3,4)+24,4) },
	{ STEP32(0,32) },
	32*32
};



/*************************************
 *
 *  Generic MCR CTC interface
 *
 *************************************/

const z80_daisy_config mcr_daisy_chain[] =
{
	{ "ctc" },
	{ NULL }
};


const z80_daisy_config mcr_ipu_daisy_chain[] =
{
	{ "ipu_ctc" },
	{ "ipu_pio1" },
	{ "ipu_sio" },
	{ "ipu_pio0" },
	{ NULL }
};


Z80CTC_INTERFACE( mcr_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* interrupt handler */
	DEVCB_DEVICE_LINE_MEMBER("ctc", z80ctc_device, trg1),	/* ZC/TO0 callback */
	DEVCB_NULL,             	/* ZC/TO1 callback */
	DEVCB_NULL              	/* ZC/TO2 callback */
};


Z80CTC_INTERFACE( nflfoot_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("ipu", INPUT_LINE_IRQ0),  /* interrupt handler */
	DEVCB_NULL,			/* ZC/TO0 callback */
	DEVCB_NULL,			/* ZC/TO1 callback */
	DEVCB_NULL      	/* ZC/TO2 callback */
};


Z80PIO_INTERFACE( nflfoot_pio_intf )
{
	DEVCB_CPU_INPUT_LINE("ipu", INPUT_LINE_IRQ0),  /* interrupt handler */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static WRITE_LINE_DEVICE_HANDLER( ipu_ctc_interrupt )
{
	device->machine().device("ipu")->execute().set_input_line(0, state);
}


const z80sio_interface nflfoot_sio_intf =
{
	ipu_ctc_interrupt,	/* interrupt handler */
	0,					/* DTR changed handler */
	0,					/* RTS changed handler */
	ipu_break_changed,	/* BREAK changed handler */
	mcr_ipu_sio_transmit/* transmit handler */
};



/*************************************
 *
 *  Generic MCR machine initialization
 *
 *************************************/

MACHINE_START( mcr )
{
	state_save_register_global(machine, mcr_cocktail_flip);
}


MACHINE_START( nflfoot )
{
	/* allocate a timer for the IPU watchdog */
	ipu_watchdog_timer = machine.scheduler().timer_alloc(FUNC(ipu_watchdog_reset));
}


MACHINE_RESET( mcr )
{
	/* reset cocktail flip */
	mcr_cocktail_flip = 0;
}



/*************************************
 *
 *  Generic MCR interrupt handler
 *
 *************************************/

TIMER_DEVICE_CALLBACK( mcr_interrupt )
{
	//mcr_state *state = timer.machine().driver_data<mcr_state>();
	z80ctc_device *ctc = timer.machine().device<z80ctc_device>("ctc");
	int scanline = param;

	/* CTC line 2 is connected to VBLANK, which is once every 1/2 frame */
	/* for the 30Hz interlaced display */
	if(scanline == 0 || scanline == 240)
	{
		ctc->trg2(1);
		ctc->trg2(0);
	}

	/* CTC line 3 is connected to 493, which is signalled once every */
	/* frame at 30Hz */
	if (scanline == 0)
	{
		ctc->trg3(1);
		ctc->trg3(0);
	}
}

TIMER_DEVICE_CALLBACK( mcr_ipu_interrupt )
{
	//mcr_state *state = timer.machine().driver_data<mcr_state>();
	z80ctc_device *ctc = timer.machine().device<z80ctc_device>("ctc");
	int scanline = param;

	/* CTC line 3 is connected to 493, which is signalled once every */
	/* frame at 30Hz */
	if (scanline == 0)
	{
		ctc->trg3(1);
		ctc->trg3(0);
	}
}


/*************************************
 *
 *  NFL Football IPU board
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( ipu_break_changed )
{
	/* channel B is connected to the CED player */
	if (offset == 1)
	{
		logerror("DTR changed -> %d\n", data);
		if (data == 1)
			downcast<z80sio_device *>(device)->receive_data(1, 0);
	}
}


WRITE8_MEMBER(mcr_state::mcr_ipu_laserdisk_w)
{
	/* bit 3 enables (1) LD video regardless of PIX SW */
	/* bit 2 enables (1) LD right channel audio */
	/* bit 1 enables (1) LD left channel audio */
	/* bit 0 enables (1) LD video if PIX SW == 1 */
	if (data != 0)
		logerror("%04X:mcr_ipu_laserdisk_w(%d) = %02X\n", space.device().safe_pc(), offset, data);
}


static TIMER_CALLBACK( ipu_watchdog_reset )
{
	logerror("ipu_watchdog_reset\n");
	machine.device("ipu")->execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	machine.device("ipu_ctc")->reset();
	machine.device("ipu_pio0")->reset();
	machine.device("ipu_pio1")->reset();
	machine.device("ipu_sio")->reset();
}


READ8_MEMBER(mcr_state::mcr_ipu_watchdog_r)
{
	/* watchdog counter is clocked by 7.3728MHz crystal / 16 */
	/* watchdog is tripped when 14-bit counter overflows => / 32768 = 14.0625Hz*/
	ipu_watchdog_timer->adjust(attotime::from_hz(7372800 / 16 / 32768));
	return 0xff;
}


WRITE8_MEMBER(mcr_state::mcr_ipu_watchdog_w)
{
	mcr_ipu_watchdog_r(space,0);
}
