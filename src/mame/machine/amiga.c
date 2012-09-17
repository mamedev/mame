/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Aaron Giles, Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"
#include "machine/6526cia.h"
#include "machine/amigafdc.h"

/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_CUSTOM	0
#define LOG_CIA		0
#define LOG_BLITS	0



/*************************************
 *
 *  Constants
 *
 *************************************/

/* How many CPU cycles we delay until we fire a pending interrupt */
#define AMIGA_IRQ_DELAY_CYCLES		24

/* How many CPU cycles we wait until we process a blit when the blitter-nasty bit is set */
#define BLITTER_NASTY_DELAY			16



/*************************************
 *
 *  Globals
 *
 *************************************/

const char *const amiga_custom_names[0x100] =
{
	/* 0x000 */
	"BLTDDAT",		"DMACONR",		"VPOSR",		"VHPOSR",
	"DSKDATR",		"JOY0DAT",		"JOY1DAT",		"CLXDAT",
	"ADKCONR",		"POT0DAT",		"POT1DAT",		"POTGOR",
	"SERDATR",		"DSKBYTR",		"INTENAR",		"INTREQR",
	/* 0x020 */
	"DSKPTH",		"DSKPTL",		"DSKLEN",		"DSKDAT",
	"REFPTR",		"VPOSW",		"VHPOSW",		"COPCON",
	"SERDAT",		"SERPER",		"POTGO",		"JOYTEST",
	"STREQU",		"STRVBL",		"STRHOR",		"STRLONG",
	/* 0x040 */
	"BLTCON0",		"BLTCON1",		"BLTAFWM",		"BLTALWM",
	"BLTCPTH",		"BLTCPTL",		"BLTBPTH",		"BLTBPTL",
	"BLTAPTH",		"BLTAPTL",		"BLTDPTH",		"BLTDPTL",
	"BLTSIZE",		"BLTCON0L",		"BLTSIZV",		"BLTSIZH",
	/* 0x060 */
	"BLTCMOD",		"BLTBMOD",		"BLTAMOD",		"BLTDMOD",
	"UNK068",		"UNK06A",		"UNK06C",		"UNK06E",
	"BLTCDAT",		"BLTBDAT",		"BLTADAT",		"UNK076",
	"SPRHDAT",		"BPLHDAT",		"LISAID",		"DSRSYNC",
	/* 0x080 */
	"COP1LCH",		"COP1LCL",		"COP2LCH",		"COP2LCL",
	"COPJMP1",		"COPJMP2",		"COPINS",		"DIWSTRT",
	"DIWSTOP",		"DDFSTRT",		"DDFSTOP",		"DMACON",
	"CLXCON",		"INTENA",		"INTREQ",		"ADKCON",
	/* 0x0A0 */
	"AUD0LCH",		"AUD0LCL",		"AUD0LEN",		"AUD0PER",
	"AUD0VOL",		"AUD0DAT",		"UNK0AC",		"UNK0AE",
	"AUD1LCH",		"AUD1LCL",		"AUD1LEN",		"AUD1PER",
	"AUD1VOL",		"AUD1DAT",		"UNK0BC",		"UNK0BE",
	/* 0x0C0 */
	"AUD2LCH",		"AUD2LCL",		"AUD2LEN",		"AUD2PER",
	"AUD2VOL",		"AUD2DAT",		"UNK0CC",		"UNK0CE",
	"AUD3LCH",		"AUD3LCL",		"AUD3LEN",		"AUD3PER",
	"AUD3VOL",		"AUD3DAT",		"UNK0DC",		"UNK0DE",
	/* 0x0E0 */
	"BPL1PTH",		"BPL1PTL",		"BPL2PTH",		"BPL2PTL",
	"BPL3PTH",		"BPL3PTL",		"BPL4PTH",		"BPL4PTL",
	"BPL5PTH",		"BPL5PTL",		"BPL6PTH",		"BPL6PTL",
	"BPL7PTH",		"BPL7PTL",		"BPL8PTH",		"BPL8PTL",
	/* 0x100 */
	"BPLCON0",		"BPLCON1",		"BPLCON2",		"BPLCON3",
	"BPL1MOD",		"BPL2MOD",		"BPLCON4",		"CLXCON2",
	"BPL1DAT",		"BPL2DAT",		"BPL3DAT",		"BPL4DAT",
	"BPL5DAT",		"BPL6DAT",		"BPL7DAT",		"BPL8DAT",
	/* 0x120 */
	"SPR0PTH",		"SPR0PTL",		"SPR1PTH",		"SPR1PTL",
	"SPR2PTH",		"SPR2PTL",		"SPR3PTH",		"SPR3PTL",
	"SPR4PTH",		"SPR4PTL",		"SPR5PTH",		"SPR5PTL",
	"SPR6PTH",		"SPR6PTL",		"SPR7PTH",		"SPR7PTL",
	/* 0x140 */
	"SPR0POS",		"SPR0CTL",		"SPR0DATA", 	"SPR0DATB",
	"SPR1POS",		"SPR1CTL",		"SPR1DATA", 	"SPR1DATB",
	"SPR2POS",		"SPR2CTL",		"SPR2DATA", 	"SPR2DATB",
	"SPR3POS",		"SPR3CTL",		"SPR3DATA", 	"SPR3DATB",
	/* 0x160 */
	"SPR4POS",		"SPR4CTL",		"SPR4DATA", 	"SPR4DATB",
	"SPR5POS",		"SPR5CTL",		"SPR5DATA", 	"SPR5DATB",
	"SPR6POS",		"SPR6CTL",		"SPR6DATA", 	"SPR6DATB",
	"SPR7POS",		"SPR7CTL",		"SPR7DATA", 	"SPR7DATB",
	/* 0x180 */
	"COLOR00",		"COLOR01",		"COLOR02",		"COLOR03",
	"COLOR04",		"COLOR05",		"COLOR06",		"COLOR07",
	"COLOR08",		"COLOR09",		"COLOR10",		"COLOR11",
	"COLOR12",		"COLOR13",		"COLOR14",		"COLOR15",
	/* 0x1A0 */
	"COLOR16",		"COLOR17",		"COLOR18",		"COLOR19",
	"COLOR20",		"COLOR21",		"COLOR22",		"COLOR23",
	"COLOR24",		"COLOR25",		"COLOR26",		"COLOR27",
	"COLOR28",		"COLOR29",		"COLOR30",		"COLOR31",
	/* 0x1C0 */
	"HTOTAL",		"HSSTOP",		"HBSTRT",		"HBSTOP",
	"VTOTAL",		"VSSTOP",		"VBSTRT",		"VBSTOP",
	"SPRHSTRT",		"SPRHSTOP",		"BPLHSTRT",		"BPLHSTOP",
	"HHPOSW",		"HHPOSR",		"BEAMCON0",		"HSSTRT",
	/* 0x1E0 */
	"VSSTRT",		"HCENTER",		"DIWHIGH",		"BPLHMOD",
	"SPRHPTH",		"SPRHPTL",		"BPLHPTH",		"BPLHPTL",
	"UNK1F0",		"UNK1F2",		"UNK1F4",		"UNK1F6",
	"UNK1F8",		"UNK1FA",		"FMODE",		"UNK1FE"
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void custom_reset(running_machine &machine);
static void autoconfig_reset(running_machine &machine);
static TIMER_CALLBACK( amiga_irq_proc );
static TIMER_CALLBACK( amiga_blitter_proc );
static TIMER_CALLBACK( scanline_callback );



/*************************************
 *
 *  Chipmem 16/32 bit access
 *
 *************************************/

static UINT16 amiga_chip_ram16_r(amiga_state *state, offs_t offset)
{
	offset &= state->m_intf->chip_ram_mask;
	return (offset < state->m_chip_ram.bytes()) ? state->m_chip_ram[offset/2] : 0xffff;
}

static UINT16 amiga_chip_ram32_r(amiga_state *state, offs_t offset)
{
	offset &= state->m_intf->chip_ram_mask;

	if (offset < state->m_chip_ram.bytes())
	{
		UINT32 *amiga_chip_ram32 = reinterpret_cast<UINT32 *>(state->m_chip_ram.target());
		UINT32	dat = amiga_chip_ram32[offset / 4];

		if ( offset & 2 )
			return (dat & 0xffff);

		return (dat >> 16);
	}

	return 0xffff;
}

static void amiga_chip_ram16_w(amiga_state *state, offs_t offset, UINT16 data)
{
	offset &= state->m_intf->chip_ram_mask;

	if (offset < state->m_chip_ram.bytes())
		state->m_chip_ram[offset/2] = data;
}

static void amiga_chip_ram32_w(amiga_state *state, offs_t offset, UINT16 data)
{
	offset &= state->m_intf->chip_ram_mask;

	if (offset < state->m_chip_ram.bytes())
	{
		UINT32 *amiga_chip_ram32 = reinterpret_cast<UINT32 *>(state->m_chip_ram.target());
		UINT32	dat = amiga_chip_ram32[offset / 4];

		if ( offset & 2 )
		{
			dat &= 0xffff0000;
			dat |= data;
		}
		else
		{
			dat &= 0x0000ffff;
			dat |= ((UINT32)data) << 16;
		}

		amiga_chip_ram32[offset / 4] = dat;
	}
}


void amiga_chip_ram_w8(amiga_state *state, offs_t offset, UINT8 data)
{
	UINT16 dat;

	dat = (*state->m_chip_ram_r)(state, offset);
	if (offset & 0x01)
	{
		dat &= 0xff00;
		dat |= data;
	}
	else
	{
		dat &= 0x00ff;
		dat |= ((UINT16)data) << 8;
	}
	(*state->m_chip_ram_w)(state, offset, dat);
}

/*************************************
 *
 *  Machine config/reset
 *
 *************************************/

void amiga_machine_config(running_machine &machine, const amiga_machine_interface *intf)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	state->m_intf = intf;

	/* setup chipmem handlers */
	if ( IS_AGA(intf) )
	{
		state->m_chip_ram_r = amiga_chip_ram32_r;
		state->m_chip_ram_w = amiga_chip_ram32_w;
	}
	else
	{
		state->m_chip_ram_r = amiga_chip_ram16_r;
		state->m_chip_ram_w = amiga_chip_ram16_w;
	}

	/* setup the timers */
	state->m_irq_timer = machine.scheduler().timer_alloc(FUNC(amiga_irq_proc));
	state->m_blitter_timer = machine.scheduler().timer_alloc(FUNC(amiga_blitter_proc));

	state->m_sound_device = machine.device("amiga");
}


static void amiga_m68k_reset(device_t *device)
{
	amiga_state *state = device->machine().driver_data<amiga_state>();
	address_space &space = *device->memory().space(AS_PROGRAM);

	logerror("Executed RESET at PC=%06x\n", space.device().safe_pc());

	/* Initialize the various chips */
	device->machine().device("cia_0")->reset();
	device->machine().device("cia_1")->reset();
	custom_reset(device->machine());
	autoconfig_reset(device->machine());

	/* set the overlay bit */
	if ( IS_AGA(state->m_intf) )
	{
		space.write_byte( 0xbfa001, 1 );
	}
	else
	{
		amiga_cia_w(space, 0x1001/2, 1, 0xffff);
	}
}


MACHINE_RESET_MEMBER(amiga_state,amiga)
{

	/* set m68k reset  function */
	m68k_set_reset_callback(machine().device("maincpu"), amiga_m68k_reset);

	amiga_m68k_reset(machine().device("maincpu"));

	/* call the system-specific callback */
	if (m_intf->reset_callback)
		(*m_intf->reset_callback)(machine());

	/* start the scanline timer */
	machine().scheduler().timer_set(machine().primary_screen->time_until_pos(0), FUNC(scanline_callback));
}



/*************************************
 *
 *  Per scanline callback
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int scanline = param;
	device_t *cia_0 = machine.device("cia_0");
	device_t *cia_1 = machine.device("cia_1");

	/* on the first scanline, we do some extra bookkeeping */
	if (scanline == 0)
	{
		/* signal VBLANK IRQ */
		amiga_custom_w(*machine.device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, 0x8000 | INTENA_VERTB, 0xffff);

		/* clock the first CIA TOD */
		mos6526_tod_w(cia_0, 1);

		/* call the system-specific callback */
		if (state->m_intf->scanline0_callback != NULL)
			(*state->m_intf->scanline0_callback)(machine);
	}

	/* on every scanline, clock the second CIA TOD */
	mos6526_tod_w(cia_1, 1);

	/* render up to this scanline */
	if (!machine.primary_screen->update_partial(scanline))
	{
		if (IS_AGA(state->m_intf))
		{
			bitmap_rgb32 dummy_bitmap;
			amiga_aga_render_scanline(machine, dummy_bitmap, scanline);
		}
		else
		{
			bitmap_ind16 dummy_bitmap;
			amiga_render_scanline(machine, dummy_bitmap, scanline);
		}
	}

	/* force a sound update */
	amiga_audio_update(state->m_sound_device);

	/* set timer for next line */
	scanline = (scanline + 1) % machine.primary_screen->height();
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(scanline), FUNC(scanline_callback), scanline);
}



/*************************************
 *
 *  Interrupt management
 *
 *************************************/

static void update_irqs(running_machine &machine)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int ints = CUSTOM_REG(REG_INTENA) & CUSTOM_REG(REG_INTREQ);

	/* Master interrupt switch */
	if (CUSTOM_REG(REG_INTENA) & 0x4000)
	{
		/* Serial transmit buffer empty, disk block finished, software interrupts */
		machine.device("maincpu")->execute().set_input_line(1, ints & 0x0007 ? ASSERT_LINE : CLEAR_LINE);

		/* I/O ports and timer interrupts */
		machine.device("maincpu")->execute().set_input_line(2, ints & 0x0008 ? ASSERT_LINE : CLEAR_LINE);

		/* Copper, VBLANK, blitter interrupts */
		machine.device("maincpu")->execute().set_input_line(3, ints & 0x0070 ? ASSERT_LINE : CLEAR_LINE);

		/* Audio interrupts */
		machine.device("maincpu")->execute().set_input_line(4, ints & 0x0780 ? ASSERT_LINE : CLEAR_LINE);

		/* Serial receive buffer full, disk sync match */
		machine.device("maincpu")->execute().set_input_line(5, ints & 0x1800 ? ASSERT_LINE : CLEAR_LINE);

		/* External interrupts */
		machine.device("maincpu")->execute().set_input_line(6, ints & 0x2000 ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		machine.device("maincpu")->execute().set_input_line(1, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(2, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(3, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(4, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(5, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(6, CLEAR_LINE);
	}
}


static TIMER_CALLBACK( amiga_irq_proc )
{
	amiga_state *state = machine.driver_data<amiga_state>();

	update_irqs(machine);
	state->m_irq_timer->reset( );
}



/*************************************
 *
 *  Standard joystick conversion
 *
 *************************************/

CUSTOM_INPUT_MEMBER( amiga_state::amiga_joystick_convert )
{
	UINT8 bits = ioport((const char *)param)->read();
	int up = (bits >> 0) & 1;
	int down = (bits >> 1) & 1;
	int left = (bits >> 2) & 1;
	int right = (bits >> 3) & 1;

	if (left) up ^= 1;
	if (right) down ^= 1;

	return down | (right << 1) | (up << 8) | (left << 9);
}



/*************************************
 *
 *  Ascending blitter variant
 *
 *************************************/

static UINT32 blit_ascending(amiga_state *state)
{
	UINT32 shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	UINT32 shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	UINT32 height = CUSTOM_REG(REG_BLTSIZV);
	UINT32 width = CUSTOM_REG(REG_BLTSIZH);
	UINT32 acca = 0, accb = 0;
	UINT32 blitsum = 0;
	UINT32 x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			UINT16 abc0, abc1, abc2, abc3;
			UINT32 tempa, tempd = 0;
			UINT32 b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) += 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) += 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTCPTH));
				CUSTOM_REG_LONG(REG_BLTCPTH) += 2;
			}

			/* apply start/end masks to the A data */
			tempa = CUSTOM_REG(REG_BLTADAT);
		    if (x == 0)
		    	tempa &= CUSTOM_REG(REG_BLTAFWM);
		    if (x == width - 1)
		    	tempa &= CUSTOM_REG(REG_BLTALWM);

			/* update the B accumulator applying shifts */
			acca = (acca << 16) | (tempa << (16 - shifta));
			accb = (accb << 16) | (CUSTOM_REG(REG_BLTBDAT) << (16 - shiftb));

			/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
			abc0 = ((acca >> 17) & 0x4444) | ((accb >> 18) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
			abc1 = ((acca >> 16) & 0x4444) | ((accb >> 17) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
			abc2 = ((acca >> 15) & 0x4444) | ((accb >> 16) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
			abc3 = ((acca >> 14) & 0x4444) | ((accb >> 15) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

			/* now loop over bits and compute the destination value */
			for (b = 0; b < 4; b++)
			{
				UINT32 bit;

				/* shift previous data up 4 bits */
				tempd <<= 4;

				/* lookup first bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 >> 12)) & 1;
				abc0 <<= 4;
				tempd |= bit << 3;

				/* lookup second bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 >> 12)) & 1;
				abc1 <<= 4;
				tempd |= bit << 2;

				/* lookup third bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 >> 12)) & 1;
				abc2 <<= 4;
				tempd |= bit << 1;

				/* lookup fourth bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 >> 12)) & 1;
				abc3 <<= 4;
				tempd |= bit << 0;
			}

			/* accumulate the sum */
			blitsum |= tempd;

			/* write to the destination */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			{
				(*state->m_chip_ram_w)(state, CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
				CUSTOM_REG_LONG(REG_BLTDPTH) += 2;
			}
		}

		/* apply end of line modulos */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			CUSTOM_REG_LONG(REG_BLTBPTH) += CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG_LONG(REG_BLTCPTH) += CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			CUSTOM_REG_LONG(REG_BLTDPTH) += CUSTOM_REG_SIGNED(REG_BLTDMOD) & ~1;
	}

	/* return the blit sum */
	return blitsum;
}



/*************************************
 *
 *  Descending blitter variant
 *
 *************************************/

static UINT32 blit_descending(amiga_state *state)
{
	UINT32 fill_exclusive = (CUSTOM_REG(REG_BLTCON1) >> 4);
	UINT32 fill_inclusive = (CUSTOM_REG(REG_BLTCON1) >> 3);
	UINT32 shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	UINT32 shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	UINT32 height = CUSTOM_REG(REG_BLTSIZV);
	UINT32 width = CUSTOM_REG(REG_BLTSIZH);
	UINT32 acca = 0, accb = 0;
	UINT32 blitsum = 0;
	UINT32 x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		UINT32 fill_state = (CUSTOM_REG(REG_BLTCON1) >> 2) & 1;

		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			UINT16 abc0, abc1, abc2, abc3;
			UINT32 tempa, tempd = 0;
			UINT32 b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) -= 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) -= 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTCPTH));
				CUSTOM_REG_LONG(REG_BLTCPTH) -= 2;
			}

			/* apply start/end masks to the A data */
			tempa = CUSTOM_REG(REG_BLTADAT);
		    if (x == 0)
		    	tempa &= CUSTOM_REG(REG_BLTAFWM);
		    if (x == width - 1)
		    	tempa &= CUSTOM_REG(REG_BLTALWM);

			/* update the B accumulator applying shifts */
			acca = (acca >> 16) | (tempa << shifta);
			accb = (accb >> 16) | (CUSTOM_REG(REG_BLTBDAT) << shiftb);

			/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
			abc0 = ((acca >> 1) & 0x4444) | ((accb >> 2) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
			abc1 = ((acca >> 0) & 0x4444) | ((accb >> 1) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
			abc2 = ((acca << 1) & 0x4444) | ((accb >> 0) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
			abc3 = ((acca << 2) & 0x4444) | ((accb << 1) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

			/* now loop over bits and compute the destination value */
			for (b = 0; b < 4; b++)
			{
				UINT32 prev_fill_state;
				UINT32 bit;

				/* shift previous data up 4 bits */
				tempd >>= 4;

				/* lookup fourth bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 & 0xf)) & 1;
				abc3 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 12;

				/* lookup third bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 & 0xf)) & 1;
				abc2 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 13;

				/* lookup second bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 & 0xf)) & 1;
				abc1 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 14;

				/* lookup first bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 & 0xf)) & 1;
				abc0 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 15;
			}

			/* accumulate the sum */
			blitsum |= tempd;

			/* write to the destination */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			{
				(*state->m_chip_ram_w)(state, CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
				CUSTOM_REG_LONG(REG_BLTDPTH) -= 2;
			}
		}

		/* apply end of line modulos */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			CUSTOM_REG_LONG(REG_BLTAPTH) -= CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			CUSTOM_REG_LONG(REG_BLTBPTH) -= CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG_LONG(REG_BLTCPTH) -= CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			CUSTOM_REG_LONG(REG_BLTDPTH) -= CUSTOM_REG_SIGNED(REG_BLTDMOD) & ~1;
	}

	/* return the blit sum */
	return blitsum;
}



/*************************************
 *
 *  Line drawing blitter variant
 *
 *************************************/

/*
    The exact line drawing algorithm is not known, but based on the cryptic
    setup instructions, it is clear that it is a basic Bresenham line
    algorithm. A standard Bresenham algorithm looks like this:

    epsilon = 0;
    while (length--)
    {
        plot(x, y);
        x++;
        epsilon += dy;
        if ((2 * epsilon) >= dx)
        {
            y++;
            epsilon -= dx;
        }
    }

    If you multiply the epsilon term by 4 and shuffle the logic a bit, the
    equivalent logic is:

    epsilon = 4 * dy - 2 * dx;
    while (length--)
    {
        plot(x, y);
        x++;
        if (epsilon >= 0)
        {
            y++;
            epsilon += 4 * (dy - dx);
        }
        else
            epsilon += 4 * dy;
    }

    With this refactoring, you can see that BLTAPT = epsilon,
    BLTAMOD = 4 * (dy - dx) and BLTBMOD = 4 * dy.
*/

static UINT32 blit_line(amiga_state *state)
{
	UINT32 singlemode = (CUSTOM_REG(REG_BLTCON1) & 0x0002) ? 0x0000 : 0xffff;
	UINT32 singlemask = 0xffff;
	UINT32 blitsum = 0;
	UINT32 height;

	/* see if folks are breaking the rules */
	if (CUSTOM_REG(REG_BLTSIZH) != 0x0002)
		logerror("Blitter: Blit width != 2 in line mode!\n");
	if ((CUSTOM_REG(REG_BLTCON0) & 0x0a00) != 0x0a00)
		logerror("Blitter: Channel selection incorrect in line mode!\n" );

	/* extract the length of the line */
	height = CUSTOM_REG(REG_BLTSIZV);

	/* iterate over the line height */
	while (height--)
	{
		UINT16 abc0, abc1, abc2, abc3;
		UINT32 tempa, tempb, tempd = 0;
		int b, dx, dy;

		/* fetch data for C */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG(REG_BLTCDAT) = (*state->m_chip_ram_r)(state, CUSTOM_REG_LONG(REG_BLTCPTH));

		/* rotate the A data according to the shift */
		tempa = CUSTOM_REG(REG_BLTADAT) >> (CUSTOM_REG(REG_BLTCON0) >> 12);

		/* apply single bit mask */
		tempa &= singlemask;
		singlemask &= singlemode;

		/* rotate the B data according to the shift and expand to 16 bits */
		tempb = -((CUSTOM_REG(REG_BLTBDAT) >> (CUSTOM_REG(REG_BLTCON1) >> 12)) & 1);

		/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
		abc0 = ((tempa >> 1) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
		abc1 = ((tempa >> 0) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
		abc2 = ((tempa << 1) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
		abc3 = ((tempa << 2) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

		/* now loop over bits and compute the destination value */
		for (b = 0; b < 4; b++)
		{
			UINT32 bit;

			/* shift previous data up 4 bits */
			tempd <<= 4;

			/* lookup first bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 >> 12)) & 1;
			abc0 <<= 4;
			tempd |= bit << 3;

			/* lookup second bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 >> 12)) & 1;
			abc1 <<= 4;
			tempd |= bit << 2;

			/* lookup third bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 >> 12)) & 1;
			abc2 <<= 4;
			tempd |= bit << 1;

			/* lookup fourth bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 >> 12)) & 1;
			abc3 <<= 4;
			tempd |= bit << 0;
		}

		/* accumulate the sum */
		blitsum |= tempd;

		/* write to the destination */
		(*state->m_chip_ram_w)(state, CUSTOM_REG_LONG(REG_BLTDPTH), tempd);

		/* always increment along the major axis */
		if (CUSTOM_REG(REG_BLTCON1) & 0x0010)
		{
			dx = (CUSTOM_REG(REG_BLTCON1) & 0x0004) ? -1 : 1;
			dy = 0;
		}
		else
		{
			dx = 0;
			dy = (CUSTOM_REG(REG_BLTCON1) & 0x0004) ? -1 : 1;
		}

		/* is the sign bit clear? */
		if (!(CUSTOM_REG(REG_BLTCON1) & 0x0040))
		{
			/* add 4 * (dy-dx) */
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;

			/* increment along the minor axis */
			if (CUSTOM_REG(REG_BLTCON1) & 0x0010)
				dy = (CUSTOM_REG(REG_BLTCON1) & 0x0008) ? -1 : 1;
			else
				dx = (CUSTOM_REG(REG_BLTCON1) & 0x0008) ? -1 : 1;
		}

		/* else add 4 * dy and don't increment along the minor axis */
		else
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;

		/* adjust X if necessary */
		if (dx)
		{
			/* adjust the A shift value */
			UINT32 temp = CUSTOM_REG(REG_BLTCON0) + (INT32)(dx << 12);
			CUSTOM_REG(REG_BLTCON0) = temp;

			/* if we went from 0xf to 0x0 or vice-versa, adjust the actual pointers */
			if (temp & 0x10000)
			{
				CUSTOM_REG_LONG(REG_BLTCPTH) += 2 * dx;
				CUSTOM_REG_LONG(REG_BLTDPTH) += 2 * dx;
			}
		}

		/* adjust Y if necessary */
		if (dy)
		{
			/* BLTCMOD seems to be used for both C and D pointers */
			CUSTOM_REG_LONG(REG_BLTCPTH) += dy * (INT16)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);
			CUSTOM_REG_LONG(REG_BLTDPTH) += dy * (INT16)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);

			/* reset the single mask since we're on a new line */
			singlemask = 0xffff;
		}

		/* set the new sign bit value */
		CUSTOM_REG(REG_BLTCON1) = (CUSTOM_REG(REG_BLTCON1) & ~0x0040) | ((CUSTOM_REG(REG_BLTAPTL) >> 9) & 0x0040);

		/* increment texture shift on every pixel */
		CUSTOM_REG(REG_BLTCON1) += 0x1000;
	}

	return blitsum;
}



/*************************************
 *
 *  Blitter deferred callback
 *
 *************************************/

static TIMER_CALLBACK( amiga_blitter_proc )
{
	amiga_state *state = machine.driver_data<amiga_state>();
	UINT32 blitsum = 0;

	/* logging */
	if (LOG_BLITS)
	{
		static const char *const type[] = { "ASCENDING", "LINE", "DESCENDING", "LINE" };
		logerror("BLIT %s: %dx%d  %04x %04x\n", type[CUSTOM_REG(REG_BLTCON1) & 0x0003], CUSTOM_REG(REG_BLTSIZH), CUSTOM_REG(REG_BLTSIZV), CUSTOM_REG(REG_BLTCON0), CUSTOM_REG(REG_BLTCON1));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			logerror("  A: addr=%06X mod=%3d shift=%2d maskl=%04x maskr=%04x\n", CUSTOM_REG_LONG(REG_BLTAPTH), CUSTOM_REG_SIGNED(REG_BLTAMOD), CUSTOM_REG(REG_BLTCON0) >> 12, CUSTOM_REG(REG_BLTAFWM), CUSTOM_REG(REG_BLTALWM));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			logerror("  B: addr=%06X mod=%3d shift=%2d\n", CUSTOM_REG_LONG(REG_BLTBPTH), CUSTOM_REG_SIGNED(REG_BLTBMOD), CUSTOM_REG(REG_BLTCON1) >> 12);
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			logerror("  C: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTCPTH), CUSTOM_REG_SIGNED(REG_BLTCMOD));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			logerror("  D: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTDPTH), CUSTOM_REG_SIGNED(REG_BLTDMOD));
	}

	/* set the zero flag */
	CUSTOM_REG(REG_DMACON) |= 0x2000;

	/* switch off the type of blit */
	switch (CUSTOM_REG(REG_BLTCON1) & 0x0003)
	{
		case 0:	/* ascending */
			blitsum = blit_ascending(state);
			break;

		case 2:	/* descending */
			blitsum = blit_descending(state);
			break;

		case 1:	/* line */
		case 3:
			blitsum = blit_line(state);
			break;
	}

	/* clear the zero flag if we actually wrote data */
	if (blitsum)
		CUSTOM_REG(REG_DMACON) &= ~0x2000;

	/* no longer busy */
	CUSTOM_REG(REG_DMACON) &= ~0x4000;

	/* signal an interrupt */
	amiga_custom_w(*machine.device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, 0x8000 | INTENA_BLIT, 0xffff);

	/* reset the blitter timer */
	state->m_blitter_timer->reset( );
}



/*************************************
 *
 *  Blitter setup
 *
 *************************************/

static void blitter_setup(address_space &space)
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	int ticks, width, height, blittime;

	/* is there another blitting in progress? */
	if (CUSTOM_REG(REG_DMACON) & 0x4000)
	{
		logerror("%s - This program is playing tricks with the blitter\n", space.machine().describe_context() );
		return;
	}

	/* line mode is 8 ticks/pixel */
	if (CUSTOM_REG(REG_BLTCON1) & 1)
		ticks = 8;

	/* standard mode is 4 ticks base */
	else
	{
		ticks = 4;

		/* plus 2 ticks if channel B is involved */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			ticks += 2;

		/* plus 2 ticks if both channel C and D are involved */
		if ((CUSTOM_REG(REG_BLTCON0) & 0x0300) == 0x0300)
			ticks += 2;
	}

	/* extract height/width */
	width = CUSTOM_REG(REG_BLTSIZH);
	height = CUSTOM_REG(REG_BLTSIZV);

	/* compute the blit time */
	blittime = ticks * height * width;

	/* if 'blitter-nasty' is set, then the blitter takes over the bus. Make the blit semi-immediate */
	if ( CUSTOM_REG(REG_DMACON) & 0x0400 )
	{
		/* simulate the 68k not running while the blit is going */
		space.device().execute().adjust_icount(-(blittime/2) );

		blittime = BLITTER_NASTY_DELAY;
	}

	/* AGA has twice the bus bandwidth, so blits take half the time */
	if ( IS_AGA(state->m_intf) )
		blittime /= 2;

	/* signal blitter busy */
	CUSTOM_REG(REG_DMACON) |= 0x4000;

	/* set a timer */
	state->m_blitter_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( blittime ));
}



/*************************************
 *
 *  8520 CIA read handler
 *
 *************************************/

READ16_HANDLER( amiga_cia_r )
{
	UINT8 data;
	int shift;
	device_t *cia;

	/* offsets 0000-07ff reference CIA B, and are accessed via the MSB */
	if ((offset & 0x0800) == 0)
	{
		cia = space.machine().device("cia_1");
		shift = 8;
	}

	/* offsets 0800-0fff reference CIA A, and are accessed via the LSB */
	else
	{
		cia = space.machine().device("cia_0");
		shift = 0;
	}

	/* handle the reads */
	data = mos6526_r(cia, space, offset >> 7);

	if (LOG_CIA)
		logerror("%06x:cia_%c_read(%03x) = %04x & %04x\n", space.device().safe_pc(), 'A' + ((~offset & 0x0800) >> 11), offset * 2, data << shift, mem_mask);

	return data << shift;
}



/*************************************
 *
 *  8520 CIA write handler
 *
 *************************************/

WRITE16_HANDLER( amiga_cia_w )
{
	device_t *cia;

	if (LOG_CIA)
		logerror("%06x:cia_%c_write(%03x) = %04x & %04x\n", space.device().safe_pc(), 'A' + ((~offset & 0x0800) >> 11), offset * 2, data, mem_mask);

	/* offsets 0000-07ff reference CIA B, and are accessed via the MSB */
	if ((offset & 0x0800) == 0)
	{
		if (!ACCESSING_BITS_8_15)
			return;
		cia = space.machine().device("cia_1");
		data >>= 8;
	}

	/* offsets 0800-0fff reference CIA A, and are accessed via the LSB */
	else
	{
		if (!ACCESSING_BITS_0_7)
			return;
		cia = space.machine().device("cia_0");
		data &= 0xff;
	}

	/* handle the writes */
	mos6526_w(cia, space, offset >> 7, (UINT8) data);
}



/*************************************
 *
 *  CIA interrupt callbacks
 *
 *************************************/

void amiga_cia_0_irq(device_t *device, int state)
{
	amiga_custom_w(*device->machine().device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, (state ? 0x8000 : 0x0000) | INTENA_PORTS, 0xffff);
}


void amiga_cia_1_irq(device_t *device, int state)
{
	amiga_custom_w(*device->machine().device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, (state ? 0x8000 : 0x0000) | INTENA_EXTER, 0xffff);
}



/*************************************
 *
 *  Custom chip reset
 *
 *************************************/

static void custom_reset(running_machine &machine)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	int clock = machine.device("maincpu")->unscaled_clock();
	UINT16	vidmode = (clock == AMIGA_68000_NTSC_CLOCK || clock == AMIGA_68EC020_NTSC_CLOCK ) ? 0x1000 : 0x0000; /* NTSC or PAL? */

	CUSTOM_REG(REG_DDFSTRT) = 0x18;
	CUSTOM_REG(REG_DDFSTOP) = 0xd8;
	CUSTOM_REG(REG_INTENA) = 0x0000;
	CUSTOM_REG(REG_VPOSR) = vidmode;
	CUSTOM_REG(REG_SERDATR) = 0x3000;

	switch (state->m_intf->chip_ram_mask)
	{
		case ANGUS_CHIP_RAM_MASK:
		case FAT_ANGUS_CHIP_RAM_MASK:
			CUSTOM_REG(REG_DENISEID) = 0x00FF;
			break;

		case ECS_CHIP_RAM_MASK:
			CUSTOM_REG(REG_VPOSR) |= 0x2000;
			CUSTOM_REG(REG_DENISEID) = 0x00FC;
			if (IS_AGA(state->m_intf))
			{
				CUSTOM_REG(REG_VPOSR) |= 0x0300;
				CUSTOM_REG(REG_DENISEID) = 0x00F8;
			}
			break;
	}
}



/*************************************
 *
 *  Custom chip register read
 *
 *************************************/

READ16_HANDLER( amiga_custom_r )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	UINT16 temp;

	switch (offset & 0xff)
	{
		case REG_BLTDDAT:
			return CUSTOM_REG(REG_BLTDDAT);

		case REG_DMACONR:
			return CUSTOM_REG(REG_DMACON);

		case REG_VPOSR:
			CUSTOM_REG(REG_VPOSR) &= 0xff00;
			CUSTOM_REG(REG_VPOSR) |= amiga_gethvpos(*space.machine().primary_screen) >> 16;
			return CUSTOM_REG(REG_VPOSR);

		case REG_VHPOSR:
			return amiga_gethvpos(*space.machine().primary_screen) & 0xffff;

		case REG_SERDATR:
			CUSTOM_REG(REG_SERDATR) &= ~0x4000;
			CUSTOM_REG(REG_SERDATR) |= (CUSTOM_REG(REG_INTREQ) & INTENA_RBF) ? 0x4000 : 0x0000;
			return CUSTOM_REG(REG_SERDATR);

		case REG_JOY0DAT:
			if (state->m_intf->joy0dat_r != NULL)
				return (*state->m_intf->joy0dat_r)(space.machine());
			return state->ioport("JOY0DAT")->read_safe(0xffff);

		case REG_JOY1DAT:
			if (state->m_intf->joy1dat_r != NULL)
				return (*state->m_intf->joy1dat_r)(space.machine());
			return state->ioport("JOY1DAT")->read_safe(0xffff);

		case REG_POTGOR:
			return state->ioport("POTGO")->read_safe(0x5500);

		case REG_POT0DAT:
			return state->ioport("POT0DAT")->read_safe(0x0000);

		case REG_POT1DAT:
			return state->ioport("POT1DAT")->read_safe(0x0000);

		case REG_DSKBYTR:
			return space.machine().device<amiga_fdc>("fdc")->dskbytr_r();

		case REG_INTENAR:
			return CUSTOM_REG(REG_INTENA);

		case REG_INTREQR:
			return CUSTOM_REG(REG_INTREQ);

		case REG_COPJMP1:
			amiga_copper_setpc(space.machine(), CUSTOM_REG_LONG(REG_COP1LCH));
			break;

		case REG_COPJMP2:
			amiga_copper_setpc(space.machine(), CUSTOM_REG_LONG(REG_COP2LCH));
			break;

		case REG_CLXDAT:
			temp = CUSTOM_REG(REG_CLXDAT);
			CUSTOM_REG(REG_CLXDAT) = 0;
			return temp;

		case REG_DENISEID:
			return CUSTOM_REG(REG_DENISEID);

		case REG_DSKPTH:
			return space.machine().device<amiga_fdc>("fdc")->dskpth_r();

		case REG_DSKPTL:
			return space.machine().device<amiga_fdc>("fdc")->dskptl_r();
	}

	if (LOG_CUSTOM)
		logerror("%06X:read from custom %s\n", space.device().safe_pc(), amiga_custom_names[offset & 0xff]);

	return 0xffff;
}



/*************************************
 *
 *  Custom chip register write
 *
 *************************************/

static TIMER_CALLBACK( finish_serial_write )
{
	amiga_state *state = machine.driver_data<amiga_state>();

	/* mark the transfer buffer empty */
	CUSTOM_REG(REG_SERDATR) |= 0x3000;

	/* signal an interrupt */
	amiga_custom_w(*machine.device("maincpu")->memory().space(AS_PROGRAM), REG_INTREQ, 0x8000 | INTENA_TBE, 0xffff);
}


WRITE16_HANDLER( amiga_custom_w )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	device_t *cia_0;
	device_t *cia_1;
	UINT16 temp;
	offset &= 0xff;

	if (LOG_CUSTOM)
		logerror("%06X:write to custom %s = %04X\n", space.device().safe_pc(), amiga_custom_names[offset & 0xff], data);

	switch (offset)
	{
		case REG_BLTDDAT:	case REG_DMACONR:	case REG_VPOSR:		case REG_VHPOSR:
		case REG_DSKDATR:	case REG_JOY0DAT:	case REG_JOY1DAT:	case REG_CLXDAT:
		case REG_ADKCONR:	case REG_POT0DAT:	case REG_POT1DAT:	case REG_POTGOR:
		case REG_SERDATR:	case REG_DSKBYTR:	case REG_INTENAR:	case REG_INTREQR:
			/* read-only registers */
			break;

		case REG_DSKSYNC:
			space.machine().device<amiga_fdc>("fdc")->dsksync_w(data);
			break;

		case REG_DSKPTH:
			space.machine().device<amiga_fdc>("fdc")->dskpth_w(data);
			break;

		case REG_DSKPTL:
			space.machine().device<amiga_fdc>("fdc")->dskptl_w(data);
			break;

		case REG_DSKLEN:
			space.machine().device<amiga_fdc>("fdc")->dsklen_w(data);
			break;

		case REG_POTGO:
			if (state->m_intf->potgo_w != NULL)
				(*state->m_intf->potgo_w)(space.machine(), data);
			break;

		case REG_SERDAT:
			if (state->m_intf->serdat_w != NULL)
				(*state->m_intf->serdat_w)(space.machine(), data);
			CUSTOM_REG(REG_SERDATR) &= ~0x3000;
			space.machine().scheduler().timer_set(amiga_get_serial_char_period(space.machine()), FUNC(finish_serial_write));
			break;

		case REG_BLTSIZE:
			CUSTOM_REG(REG_BLTSIZE) = data;
			CUSTOM_REG(REG_BLTSIZV) = (data >> 6) & 0x3ff;
			CUSTOM_REG(REG_BLTSIZH) = data & 0x3f;
			if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x400;
			if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x40;
			blitter_setup(space);
			break;

		case REG_BLTSIZV:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(state->m_intf) )
			{
				CUSTOM_REG(REG_BLTSIZV) = data & 0x7fff;
				if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x8000;
			}
			break;

		case REG_BLTSIZH:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(state->m_intf) )
			{
				CUSTOM_REG(REG_BLTSIZH) = data & 0x7ff;
				if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x800;
				blitter_setup(space);
			}
			break;

		case REG_BLTCON0L:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(state->m_intf) )
			{
				CUSTOM_REG(REG_BLTCON0) &= 0xff00;
				CUSTOM_REG(REG_BLTCON0) |= data & 0xff;
			}
			break;

		case REG_SPR0PTH:	case REG_SPR1PTH:	case REG_SPR2PTH:	case REG_SPR3PTH:
		case REG_SPR4PTH:	case REG_SPR5PTH:	case REG_SPR6PTH:	case REG_SPR7PTH:
			data &= ( state->m_intf->chip_ram_mask >> 16 );
			break;

		case REG_SPR0PTL:	case REG_SPR1PTL:	case REG_SPR2PTL:	case REG_SPR3PTL:
		case REG_SPR4PTL:	case REG_SPR5PTL:	case REG_SPR6PTL:	case REG_SPR7PTL:
			amiga_sprite_dma_reset(space.machine(), (offset - REG_SPR0PTL) / 2);
			break;

		case REG_SPR0CTL:	case REG_SPR1CTL:	case REG_SPR2CTL:	case REG_SPR3CTL:
		case REG_SPR4CTL:	case REG_SPR5CTL:	case REG_SPR6CTL:	case REG_SPR7CTL:
			/* disable comparitor on writes here */
			amiga_sprite_enable_comparitor(space.machine(), (offset - REG_SPR0CTL) / 4, FALSE);
			break;

		case REG_SPR0DATA:	case REG_SPR1DATA:	case REG_SPR2DATA:	case REG_SPR3DATA:
		case REG_SPR4DATA:	case REG_SPR5DATA:	case REG_SPR6DATA:	case REG_SPR7DATA:
			/* enable comparitor on writes here */
			amiga_sprite_enable_comparitor(space.machine(), (offset - REG_SPR0DATA) / 4, TRUE);
			break;

		case REG_COP1LCH:	case REG_COP2LCH:
			data &= ( state->m_intf->chip_ram_mask >> 16 );
			break;

		case REG_COPJMP1:
			amiga_copper_setpc(space.machine(), CUSTOM_REG_LONG(REG_COP1LCH));
			break;

		case REG_COPJMP2:
			amiga_copper_setpc(space.machine(), CUSTOM_REG_LONG(REG_COP2LCH));
			break;

		case REG_DDFSTRT:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= 0xfe;
			if (data < 0x18)
				data = 0x18;
			break;

		case REG_DDFSTOP:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= 0xfe;
			if (data > 0xd8)
				data = 0xd8;
			break;

		case REG_DMACON:
			amiga_audio_update(state->m_sound_device);

			/* bits BBUSY (14) and BZERO (13) are read-only */
			data &= 0x9fff;
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			space.machine().device<amiga_fdc>("fdc")->dmacon_set(data);

			/* if 'blitter-nasty' has been turned on and we have a blit pending, reschedule it */
			if ( ( data & 0x400 ) && ( CUSTOM_REG(REG_DMACON) & 0x4000 ) )
				state->m_blitter_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( BLITTER_NASTY_DELAY ));

			break;

		case REG_INTENA:
			temp = data;

			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			CUSTOM_REG(offset) = data;

			if ( temp & 0x8000  ) /* if we're enabling irq's, delay a bit */
				state->m_irq_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( AMIGA_IRQ_DELAY_CYCLES ));
			else /* if we're disabling irq's, process right away */
				update_irqs(space.machine());
			break;

		case REG_INTREQ:
			temp = data;
			/* Update serial data line status if appropiate */
			if (!(data & 0x8000) && (data & INTENA_RBF))
				CUSTOM_REG(REG_SERDATR) &= ~0x8000;

			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			cia_0 = space.machine().device("cia_0");
			cia_1 = space.machine().device("cia_1");
			if ( mos6526_irq_r( cia_0 ) ) data |= INTENA_PORTS;
			if ( mos6526_irq_r( cia_1 ) ) data |= INTENA_EXTER;
			CUSTOM_REG(offset) = data;

			if ( temp & 0x8000  ) /* if we're generating irq's, delay a bit */
				state->m_irq_timer->adjust( space.machine().device<cpu_device>("maincpu")->cycles_to_attotime( AMIGA_IRQ_DELAY_CYCLES ));
			else /* if we're clearing irq's, process right away */
				update_irqs(space.machine());
			break;

		case REG_ADKCON:
			amiga_audio_update(state->m_sound_device);
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			space.machine().device<amiga_fdc>("fdc")->adkcon_set(data);
			break;

		case REG_AUD0LCL:	case REG_AUD0LCH:	case REG_AUD0LEN:	case REG_AUD0PER:	case REG_AUD0VOL:
		case REG_AUD1LCL:	case REG_AUD1LCH:	case REG_AUD1LEN:	case REG_AUD1PER:	case REG_AUD1VOL:
		case REG_AUD2LCL:	case REG_AUD2LCH:	case REG_AUD2LEN:	case REG_AUD2PER:	case REG_AUD2VOL:
		case REG_AUD3LCL:	case REG_AUD3LCH:	case REG_AUD3LEN:	case REG_AUD3PER:	case REG_AUD3VOL:
			amiga_audio_update(state->m_sound_device);
			break;

		case REG_AUD0DAT:	case REG_AUD1DAT:	case REG_AUD2DAT:	case REG_AUD3DAT:
			amiga_audio_data_w(state->m_sound_device, (offset - REG_AUD0DAT) / 8, data);
			break;

		case REG_BPL1PTH:	case REG_BPL2PTH:	case REG_BPL3PTH:	case REG_BPL4PTH:
		case REG_BPL5PTH:	case REG_BPL6PTH:
			data &= ( state->m_intf->chip_ram_mask >> 16 );
			break;

		case REG_BPLCON0:
			if ((data & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) == (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2))
			{
				/* planes go from 0 to 6, inclusive */
				logerror( "This game is doing funky planes stuff. (planes > 6)\n" );
				data &= ~BPLCON0_BPU0;
			}
			break;

		case REG_COLOR00:	case REG_COLOR01:	case REG_COLOR02:	case REG_COLOR03:
		case REG_COLOR04:	case REG_COLOR05:	case REG_COLOR06:	case REG_COLOR07:
		case REG_COLOR08:	case REG_COLOR09:	case REG_COLOR10:	case REG_COLOR11:
		case REG_COLOR12:	case REG_COLOR13:	case REG_COLOR14:	case REG_COLOR15:
		case REG_COLOR16:	case REG_COLOR17:	case REG_COLOR18:	case REG_COLOR19:
		case REG_COLOR20:	case REG_COLOR21:	case REG_COLOR22:	case REG_COLOR23:
		case REG_COLOR24:	case REG_COLOR25:	case REG_COLOR26:	case REG_COLOR27:
		case REG_COLOR28:	case REG_COLOR29:	case REG_COLOR30:	case REG_COLOR31:
			if (IS_AGA(state->m_intf))
			{
				amiga_aga_palette_write(space.machine(), offset - REG_COLOR00, data);
			}
			else
			{
				data &= 0xfff;
				CUSTOM_REG(offset + 32) = (data >> 1) & 0x777;
			}
			break;
		case REG_DIWSTRT:
		case REG_DIWSTOP:
			if (IS_AGA(state->m_intf))
				amiga_aga_diwhigh_written(space.machine(), 0);
			break;
		case REG_DIWHIGH:
			if (IS_AGA(state->m_intf))
				amiga_aga_diwhigh_written(space.machine(), 1);
			break;

		default:
			break;
	}

	if (IS_AGA(state->m_intf))
		CUSTOM_REG(offset) = data;
	else
		if (offset <= REG_COLOR31)
			CUSTOM_REG(offset) = data;
}



/*************************************
 *
 *  Serial writes
 *
 *************************************/

void amiga_serial_in_w(running_machine &machine, UINT16 data)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	address_space &space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	int mask = (CUSTOM_REG(REG_SERPER) & 0x8000) ? 0x1ff : 0xff;

	/* copy the data to the low 8 bits of SERDATR and set RBF */
	CUSTOM_REG(REG_SERDATR) &= ~0x3ff;
	CUSTOM_REG(REG_SERDATR) |= (data & mask) | (mask + 1) | 0x4000;

	/* set overrun if we weren't cleared */
	if (CUSTOM_REG(REG_INTREQ) & INTENA_RBF)
	{
		mame_printf_debug("Serial data overflow\n");
		CUSTOM_REG(REG_SERDATR) |= 0x8000;
	}

	/* signal an interrupt */
	amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_RBF, 0xffff);
}


attotime amiga_get_serial_char_period(running_machine &machine)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	UINT32 divisor = (CUSTOM_REG(REG_SERPER) & 0x7fff) + 1;
	UINT32 baud = machine.device("maincpu")->unscaled_clock() / 2 / divisor;
	UINT32 numbits = 2 + ((CUSTOM_REG(REG_SERPER) & 0x8000) ? 9 : 8);
	return attotime::from_hz(baud) * numbits;
}



/*************************************
 *
 *  Autoconfig registration
 *
 *************************************/

void amiga_add_autoconfig(running_machine &machine, const amiga_autoconfig_device *device)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	autoconfig_device *dev, **d;

	/* validate the data */
	assert_always(machine.phase() == MACHINE_PHASE_INIT, "Can only call amiga_add_autoconfig at init time!");
	assert_always((device->size & (device->size - 1)) == 0, "device->size must be power of 2!");

	/* allocate memory and link it in at the end of the list */
	dev = auto_alloc(machine, autoconfig_device);
	dev->next = NULL;
	for (d = &state->m_autoconfig_list; *d; d = &(*d)->next) ;
	*d = dev;

	/* fill in the data */
	dev->device = *device;
	dev->base = 0;
}



/*************************************
 *
 *  Autoconfig reset
 *
 *************************************/

static void autoconfig_reset(running_machine &machine)
{
	amiga_state *state = machine.driver_data<amiga_state>();
	autoconfig_device *dev;

	/* uninstall any installed devices */
	for (dev = state->m_autoconfig_list; dev; dev = dev->next)
		if (dev->base && dev->device.uninstall)
		{
			(*dev->device.uninstall)(machine, dev->base);
			dev->base = 0;
		}

	/* reset the current autoconfig */
	state->m_cur_autoconfig = state->m_autoconfig_list;
}



/*************************************
 *
 *  Autoconfig space read
 *
 *************************************/

READ16_HANDLER( amiga_autoconfig_r )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	autoconfig_device *cur_autoconfig = state->m_cur_autoconfig;
	UINT8 byte;
	int i;

	/* if nothing present, just return */
	if (!cur_autoconfig)
	{
		logerror("autoconfig_r(%02X) but no device selected\n", offset);
		return 0;
	}

	/* switch off of the base offset */
	switch (offset/2)
	{
		/*
           00/02        1  1  x  x     x  0  0  0 = 8 Megabytes
                              ^  ^     ^  0  0  1 = 64 Kbytes
                              |  |     |  0  1  0 = 128 Kbytes
                              |  |     |  0  1  1 = 256 Kbytes
                              |  |     |  1  0  0 = 1 Megabyte
                              |  |     |  1  1  0 = 2 Megabytes
                              |  |     |  1  1  1 = 4 Megabytes
                              |  |     |
                              |  |     `-- 1 = multiple devices on this card
                              |  `-------- 1 = ROM vector offset is valid
                              `----------- 1 = link into free memory list
        */
		case 0x00/4:
			byte = 0xc0;
			if (cur_autoconfig->device.link_memory)
				byte |= 0x20;
			if (cur_autoconfig->device.rom_vector_valid)
				byte |= 0x10;
			if (cur_autoconfig->device.multi_device)
				byte |= 0x08;
			for (i = 0; i < 8; i++)
				if (cur_autoconfig->device.size & (1 << i))
					break;
			byte |= (i + 1) & 7;
			break;

		/*
           04/06          product number (all bits inverted)
        */
		case 0x04/4:
			byte = ~cur_autoconfig->device.product_number;
			break;

		/*
           08/0a        x  x  1  1     1  1  1  1
                        ^  ^
                        |  |
                        |  `-- 1 = this board can be shut up
                        `----- 0 = prefer 8 Meg address space
        */
		case 0x08/4:
			byte = 0x3f;
			if (!cur_autoconfig->device.prefer_8meg)
				byte |= 0x80;
			if (cur_autoconfig->device.can_shutup)
				byte |= 0x40;
			break;

		/*
           10/12         manufacturers number (high byte, all inverted)
           14/16                  ''          (low byte, all inverted)
        */
		case 0x10/4:
			byte = ~cur_autoconfig->device.mfr_number >> 8;
			break;

		case 0x14/4:
			byte = ~cur_autoconfig->device.mfr_number >> 0;
			break;

		/*
           18/1a         optional serial number (all bits inverted) byte0
           1c/1e                              ''                    byte1
           20/22                              ''                    byte2
           24/26                              ''                    byte3
        */
		case 0x18/4:
			byte = ~cur_autoconfig->device.serial_number >> 24;
			break;

		case 0x1c/4:
			byte = ~cur_autoconfig->device.serial_number >> 16;
			break;

		case 0x20/4:
			byte = ~cur_autoconfig->device.serial_number >> 8;
			break;

		case 0x24/4:
			byte = ~cur_autoconfig->device.serial_number >> 0;
			break;

		/*
           28/2a         optional ROM vector offset (all bits inverted) high byte
           2c/2e                              ''                        low byte
        */
		case 0x28/4:
			byte = ~cur_autoconfig->device.rom_vector >> 8;
			break;

		case 0x2c/4:
			byte = ~cur_autoconfig->device.rom_vector >> 0;
			break;

		/*
           40/42   optional interrupt control and status register
        */
		case 0x40/4:
			byte = 0x00;
			if (cur_autoconfig->device.int_control_r)
				byte = (*cur_autoconfig->device.int_control_r)(space.machine());
			break;

		default:
			byte = 0xff;
			break;
	}

	/* return the appropriate nibble */
	logerror("autoconfig_r(%02X) = %04X\n", offset, (offset & 1) ? ((byte << 12) | 0xfff) : ((byte << 8) | 0xfff));
	return (offset & 1) ? ((byte << 12) | 0xfff) : ((byte << 8) | 0xfff);
}



/*************************************
 *
 *  Autoconfig space write
 *
 *************************************/

WRITE16_HANDLER( amiga_autoconfig_w )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	autoconfig_device *cur_autoconfig = state->m_cur_autoconfig;
	int move_to_next = FALSE;

	logerror("autoconfig_w(%02X) = %04X & %04X\n", offset, data, mem_mask);

	/* if no current device, bail */
	if (!cur_autoconfig || !ACCESSING_BITS_8_15)
		return;

	/* switch off of the base offset */
	switch (offset/2)
	{
		/*
           48/4a        write-only register for base address (A23-A16)
        */
		case 0x48/4:
			if ((offset & 1) == 0)
				cur_autoconfig->base = (cur_autoconfig->base & ~0xf00000) | ((data & 0xf000) << 8);
			else
				cur_autoconfig->base = (cur_autoconfig->base & ~0x0f0000) | ((data & 0xf000) << 4);
			move_to_next = TRUE;
			break;

		/*
           4c/4e        optional write-only 'shutup' trigger
        */
		case 0x4c/4:
			cur_autoconfig->base = 0;
			move_to_next = TRUE;
			break;
	}

	/* install and move to the next device if requested */
	if (move_to_next && (offset & 1) == 0)
	{
		logerror("Install to %06X\n", cur_autoconfig->base);
		if (cur_autoconfig->base && cur_autoconfig->device.install)
			(*cur_autoconfig->device.install)(space.machine(), cur_autoconfig->base);
		state->m_cur_autoconfig = cur_autoconfig->next;
	}
}



/*************************************
 *
 *  Get interface
 *
 *************************************/

const amiga_machine_interface *amiga_get_interface(running_machine &machine)
{
	return machine.driver_data<amiga_state>()->m_intf;
}
