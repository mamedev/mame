// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Aaron Giles, Mariusz Wojcieszek
/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Aaron Giles, Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_CUSTOM  0
#define LOG_CIA     0
#define LOG_BLITS   0
#define LOG_SERIAL  1



/*************************************
 *
 *  Constants
 *
 *************************************/

/* How many CPU cycles we delay until we fire a pending interrupt */
#define AMIGA_IRQ_DELAY_CYCLES      24

/* How many CPU cycles we wait until we process a blit when the blitter-nasty bit is set */
#define BLITTER_NASTY_DELAY         16



/*************************************
 *
 *  Globals
 *
 *************************************/

const char *const amiga_custom_names[0x100] =
{
	/* 0x000 */
	"BLTDDAT",      "DMACONR",      "VPOSR",        "VHPOSR",
	"DSKDATR",      "JOY0DAT",      "JOY1DAT",      "CLXDAT",
	"ADKCONR",      "POT0DAT",      "POT1DAT",      "POTGOR",
	"SERDATR",      "DSKBYTR",      "INTENAR",      "INTREQR",
	/* 0x020 */
	"DSKPTH",       "DSKPTL",       "DSKLEN",       "DSKDAT",
	"REFPTR",       "VPOSW",        "VHPOSW",       "COPCON",
	"SERDAT",       "SERPER",       "POTGO",        "JOYTEST",
	"STREQU",       "STRVBL",       "STRHOR",       "STRLONG",
	/* 0x040 */
	"BLTCON0",      "BLTCON1",      "BLTAFWM",      "BLTALWM",
	"BLTCPTH",      "BLTCPTL",      "BLTBPTH",      "BLTBPTL",
	"BLTAPTH",      "BLTAPTL",      "BLTDPTH",      "BLTDPTL",
	"BLTSIZE",      "BLTCON0L",     "BLTSIZV",      "BLTSIZH",
	/* 0x060 */
	"BLTCMOD",      "BLTBMOD",      "BLTAMOD",      "BLTDMOD",
	"UNK068",       "UNK06A",       "UNK06C",       "UNK06E",
	"BLTCDAT",      "BLTBDAT",      "BLTADAT",      "UNK076",
	"SPRHDAT",      "BPLHDAT",      "LISAID",       "DSRSYNC",
	/* 0x080 */
	"COP1LCH",      "COP1LCL",      "COP2LCH",      "COP2LCL",
	"COPJMP1",      "COPJMP2",      "COPINS",       "DIWSTRT",
	"DIWSTOP",      "DDFSTRT",      "DDFSTOP",      "DMACON",
	"CLXCON",       "INTENA",       "INTREQ",       "ADKCON",
	/* 0x0A0 */
	"AUD0LCH",      "AUD0LCL",      "AUD0LEN",      "AUD0PER",
	"AUD0VOL",      "AUD0DAT",      "UNK0AC",       "UNK0AE",
	"AUD1LCH",      "AUD1LCL",      "AUD1LEN",      "AUD1PER",
	"AUD1VOL",      "AUD1DAT",      "UNK0BC",       "UNK0BE",
	/* 0x0C0 */
	"AUD2LCH",      "AUD2LCL",      "AUD2LEN",      "AUD2PER",
	"AUD2VOL",      "AUD2DAT",      "UNK0CC",       "UNK0CE",
	"AUD3LCH",      "AUD3LCL",      "AUD3LEN",      "AUD3PER",
	"AUD3VOL",      "AUD3DAT",      "UNK0DC",       "UNK0DE",
	/* 0x0E0 */
	"BPL1PTH",      "BPL1PTL",      "BPL2PTH",      "BPL2PTL",
	"BPL3PTH",      "BPL3PTL",      "BPL4PTH",      "BPL4PTL",
	"BPL5PTH",      "BPL5PTL",      "BPL6PTH",      "BPL6PTL",
	"BPL7PTH",      "BPL7PTL",      "BPL8PTH",      "BPL8PTL",
	/* 0x100 */
	"BPLCON0",      "BPLCON1",      "BPLCON2",      "BPLCON3",
	"BPL1MOD",      "BPL2MOD",      "BPLCON4",      "CLXCON2",
	"BPL1DAT",      "BPL2DAT",      "BPL3DAT",      "BPL4DAT",
	"BPL5DAT",      "BPL6DAT",      "BPL7DAT",      "BPL8DAT",
	/* 0x120 */
	"SPR0PTH",      "SPR0PTL",      "SPR1PTH",      "SPR1PTL",
	"SPR2PTH",      "SPR2PTL",      "SPR3PTH",      "SPR3PTL",
	"SPR4PTH",      "SPR4PTL",      "SPR5PTH",      "SPR5PTL",
	"SPR6PTH",      "SPR6PTL",      "SPR7PTH",      "SPR7PTL",
	/* 0x140 */
	"SPR0POS",      "SPR0CTL",      "SPR0DATA",     "SPR0DATB",
	"SPR1POS",      "SPR1CTL",      "SPR1DATA",     "SPR1DATB",
	"SPR2POS",      "SPR2CTL",      "SPR2DATA",     "SPR2DATB",
	"SPR3POS",      "SPR3CTL",      "SPR3DATA",     "SPR3DATB",
	/* 0x160 */
	"SPR4POS",      "SPR4CTL",      "SPR4DATA",     "SPR4DATB",
	"SPR5POS",      "SPR5CTL",      "SPR5DATA",     "SPR5DATB",
	"SPR6POS",      "SPR6CTL",      "SPR6DATA",     "SPR6DATB",
	"SPR7POS",      "SPR7CTL",      "SPR7DATA",     "SPR7DATB",
	/* 0x180 */
	"COLOR00",      "COLOR01",      "COLOR02",      "COLOR03",
	"COLOR04",      "COLOR05",      "COLOR06",      "COLOR07",
	"COLOR08",      "COLOR09",      "COLOR10",      "COLOR11",
	"COLOR12",      "COLOR13",      "COLOR14",      "COLOR15",
	/* 0x1A0 */
	"COLOR16",      "COLOR17",      "COLOR18",      "COLOR19",
	"COLOR20",      "COLOR21",      "COLOR22",      "COLOR23",
	"COLOR24",      "COLOR25",      "COLOR26",      "COLOR27",
	"COLOR28",      "COLOR29",      "COLOR30",      "COLOR31",
	/* 0x1C0 */
	"HTOTAL",       "HSSTOP",       "HBSTRT",       "HBSTOP",
	"VTOTAL",       "VSSTOP",       "VBSTRT",       "VBSTOP",
	"SPRHSTRT",     "SPRHSTOP",     "BPLHSTRT",     "BPLHSTOP",
	"HHPOSW",       "HHPOSR",       "BEAMCON0",     "HSSTRT",
	/* 0x1E0 */
	"VSSTRT",       "HCENTER",      "DIWHIGH",      "BPLHMOD",
	"SPRHPTH",      "SPRHPTL",      "BPLHPTH",      "BPLHPTL",
	"UNK1F0",       "UNK1F2",       "UNK1F4",       "UNK1F6",
	"UNK1F8",       "UNK1FA",       "FMODE",        "UNK1FE"
};



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void amiga_state::machine_start()
{
	// add callback for RESET instruction
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(amiga_state::m68k_reset), this));

	// set up chip RAM access
	memory_share *share = memshare("chip_ram");
	if (share == NULL)
		fatalerror("Unable to find Amiga chip RAM\n");
	m_chip_ram.set(*share, 2);
	m_chip_ram_mask = (m_chip_ram.bytes() - 1) & ~1;

	// set up the timers
	m_irq_timer = timer_alloc(TIMER_AMIGA_IRQ);
	m_blitter_timer = timer_alloc(TIMER_AMIGA_BLITTER);
	m_serial_timer = timer_alloc(TIMER_SERIAL);

	// start the scanline timer
	timer_set(m_screen->time_until_pos(0), TIMER_SCANLINE);
}

WRITE_LINE_MEMBER( amiga_state::m68k_reset )
{
	logerror("%s: Executed RESET\n", space().machine().describe_context());
	machine_reset();
}

void amiga_state::machine_reset()
{
	// reset cia chips
	m_cia_0->reset();
	m_cia_1->reset();

	// reset custom chip registers
	custom_chip_reset();

	// map kickstart rom to location 0
	// this either done by reseting the cia chips
	// or directly by gayle where available
	m_gayle_reset = true;
	m_overlay->set_bank(1);
}

WRITE_LINE_MEMBER( amiga_state::kbreset_w )
{
	// this is connected to the gary chip, gary then resets the 68k, agnus, paula and the cias
	if (!state)
	{
		m_sound->reset();
		machine_reset();
		m_maincpu->reset();
	}
}

// simple mirror of region 0xf80000 to 0xfbffff
READ16_MEMBER( amiga_state::rom_mirror_r )
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset + 0xf80000, mem_mask);
}

READ32_MEMBER( amiga_state::rom_mirror32_r )
{
	return m_maincpu->space(AS_PROGRAM).read_dword(offset + 0xf80000, mem_mask);
}

void amiga_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SCANLINE:
		scanline_callback(ptr, param);
		break;
	case TIMER_AMIGA_IRQ:
		amiga_irq_proc(ptr, param);
		break;
	case TIMER_AMIGA_BLITTER:
		amiga_blitter_proc(ptr, param);
		break;
	case TIMER_SERIAL:
		serial_shift();
		break;
	default:
		fatalerror("Invalid timer: %d\n", id);
	}
}


/*************************************
 *
 *  Per scanline callback
 *
 *************************************/

void amiga_state::vblank()
{
}

// todo: cia a clock can be connected to either a fixed 50/60hz signal from the power supply, or the vblank
TIMER_CALLBACK_MEMBER( amiga_state::scanline_callback )
{
	amiga_state *state = this;
	int scanline = param;

	// vblank start
	if (scanline == 0)
	{
		// signal vblank irq
		set_interrupt(INTENA_SETCLR | INTENA_VERTB);

		// clock tod
		m_cia_0->tod_w(1);

		// additional bookkeeping by drivers
		vblank();
	}

	// vblank end
	if (scanline == m_screen->visible_area().min_y)
	{
		m_cia_0->tod_w(0);
	}

	if (m_potgo_port)
	{
		// pot counters (start counting at 7 (ntsc) or 8 (pal))
		if (BIT(CUSTOM_REG(REG_POTGO), 0) && (scanline /2 ) > 7)
		{
			m_pot0x += !(m_potgo_port->read() & 0x0100);
			m_pot0y += !(m_potgo_port->read() & 0x0400);
			m_pot1x += !(m_potgo_port->read() & 0x1000);
			m_pot1y += !(m_potgo_port->read() & 0x4000);
		}
	}

	// render up to this scanline
	if (!m_screen->update_partial(scanline))
	{
		if (IS_AGA(this))
		{
			bitmap_rgb32 dummy_bitmap;
			aga_render_scanline(dummy_bitmap, scanline);
		}
		else
		{
			bitmap_ind16 dummy_bitmap;
			render_scanline(dummy_bitmap, scanline);
		}
	}

	// clock tod (if we actually render this scanline)
	m_cia_1->tod_w((scanline & 1) ^ BIT(CUSTOM_REG(REG_VPOSR), 15));

	// force a sound update
	m_sound->update();

	// set timer for next line
	scanline = (scanline + 1) % m_screen->height();
	timer_set(m_screen->time_until_pos(scanline), TIMER_SCANLINE, scanline);
}



/*************************************
 *
 *  Interrupt management
 *
 *************************************/

void amiga_state::set_interrupt(int interrupt)
{
	custom_chip_w(m_maincpu->space(AS_PROGRAM), REG_INTREQ, interrupt, 0xffff);
}

bool amiga_state::int2_pending()
{
	return m_cia_0_irq;
}

bool amiga_state::int6_pending()
{
	return m_cia_1_irq;
}

void amiga_state::update_int2()
{
	set_interrupt((int2_pending() ? INTENA_SETCLR : 0x0000) | INTENA_PORTS);
}

void amiga_state::update_int6()
{
	set_interrupt((int6_pending() ? INTENA_SETCLR : 0x0000) | INTENA_EXTER);
}

void amiga_state::update_irqs()
{
	amiga_state *state = this;

	// if the external interrupt line is still active, set the interrupt request bit
	if (int2_pending())
		CUSTOM_REG(REG_INTREQ) |= INTENA_PORTS;

	if (int6_pending())
		CUSTOM_REG(REG_INTREQ) |= INTENA_EXTER;

	int ints = CUSTOM_REG(REG_INTENA) & CUSTOM_REG(REG_INTREQ);

	// master interrupt switch
	if (CUSTOM_REG(REG_INTENA) & INTENA_INTEN)
	{
		m_maincpu->set_input_line(1, ints & (INTENA_TBE | INTENA_DSKBLK | INTENA_SOFT) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(2, ints & (INTENA_PORTS) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(3, ints & (INTENA_COPER | INTENA_VERTB | INTENA_BLIT) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(4, ints & (INTENA_AUD0 | INTENA_AUD1 | INTENA_AUD2 | INTENA_AUD3) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(5, ints & (INTENA_RBF | INTENA_DSKSYN) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(6, ints & (INTENA_EXTER) ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
		m_maincpu->set_input_line(2, CLEAR_LINE);
		m_maincpu->set_input_line(3, CLEAR_LINE);
		m_maincpu->set_input_line(4, CLEAR_LINE);
		m_maincpu->set_input_line(5, CLEAR_LINE);
		m_maincpu->set_input_line(6, CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER( amiga_state::amiga_irq_proc )
{
	update_irqs();
	m_irq_timer->reset();
}


//**************************************************************************
//  INPUTS
//**************************************************************************

UINT16 amiga_state::joy0dat_r()
{
	if (m_input_device == NULL)
		return m_joy0dat_port ? m_joy0dat_port->read() : 0xffff;

	if (m_input_device->read() & 0x10)
		return m_joy0dat_port ? m_joy0dat_port->read() : 0xffff;
	else
		return ((m_p1_mouse_y ? m_p1_mouse_y->read() : 0xff) << 8) | (m_p1_mouse_x? m_p1_mouse_x->read() : 0xff);
}

UINT16 amiga_state::joy1dat_r()
{
	if (m_input_device == NULL)
		return m_joy1dat_port ? m_joy1dat_port->read() : 0xffff;

	if (m_input_device->read() & 0x20)
		return m_joy1dat_port ? m_joy1dat_port->read() : 0xffff;
	else
		return ((m_p2_mouse_y ? m_p2_mouse_y->read() : 0xff) << 8) | (m_p2_mouse_x ? m_p2_mouse_x->read() : 0xff);
}

CUSTOM_INPUT_MEMBER( amiga_state::amiga_joystick_convert )
{
	ioport_port *ports[2] = { m_p1joy_port, m_p2joy_port };
	UINT8 bits = 0xff;

	if (ports[(int)(FPTR)param])
		bits = ports[(int)(FPTR)param]->read();

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
				//CUSTOM_REG(REG_BLTADAT) = state->m_maincpu->space(AS_PROGRAM).read_word(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG(REG_BLTADAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) += 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) += 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));
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
				state->chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
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
				CUSTOM_REG(REG_BLTADAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) -= 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) -= 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));
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
				state->chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
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
		state->logerror("Blitter: Blit width != 2 in line mode!\n");
	if ((CUSTOM_REG(REG_BLTCON0) & 0x0a00) != 0x0a00)
		state->logerror("Blitter: Channel selection incorrect in line mode!\n" );

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
			CUSTOM_REG(REG_BLTCDAT) = state->chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));

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
		state->chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);

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

TIMER_CALLBACK_MEMBER( amiga_state::amiga_blitter_proc )
{
	amiga_state *state = machine().driver_data<amiga_state>();
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
		case 0: /* ascending */
			blitsum = blit_ascending(this);
			break;

		case 2: /* descending */
			blitsum = blit_descending(this);
			break;

		case 1: /* line */
		case 3:
			blitsum = blit_line(this);
			break;
	}

	/* clear the zero flag if we actually wrote data */
	if (blitsum)
		CUSTOM_REG(REG_DMACON) &= ~0x2000;

	/* no longer busy */
	CUSTOM_REG(REG_DMACON) &= ~0x4000;

	// signal an interrupt
	set_interrupt(0x8000 | INTENA_BLIT);

	/* reset the blitter timer */
	m_blitter_timer->reset();
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
		state->logerror("%s - This program is playing tricks with the blitter\n", space.machine().describe_context() );
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
	if (IS_AGA(state))
		blittime /= 2;

	/* signal blitter busy */
	CUSTOM_REG(REG_DMACON) |= 0x4000;

	/* set a timer */
	state->m_blitter_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( blittime ));
}


//**************************************************************************
//  CENTRONICS
//**************************************************************************

WRITE_LINE_MEMBER( amiga_state::centronics_ack_w )
{
	m_cia_0->flag_w(state);
}

WRITE_LINE_MEMBER( amiga_state::centronics_busy_w )
{
	m_centronics_busy = state;
	m_cia_1->sp_w(state);
}

WRITE_LINE_MEMBER( amiga_state::centronics_perror_w )
{
	m_centronics_perror = state;
	m_cia_1->cnt_w(state);
}

WRITE_LINE_MEMBER( amiga_state::centronics_select_w )
{
	m_centronics_select = state;
}


//**************************************************************************
//  8520 CIA
//**************************************************************************

// CIA-A access: 101x xxxx xxx0 oooo xxxx xxx1
// CIA-B access: 101x xxxx xx0x oooo xxxx xxx0

READ16_MEMBER( amiga_state::cia_r )
{
	UINT16 data = 0;

	if ((offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
		data |= m_cia_0->read(space, offset >> 7);

	if ((offset & 0x2000/2) == 0 && ACCESSING_BITS_8_15)
		data |= m_cia_1->read(space, offset >> 7) << 8;

	if (LOG_CIA)
		logerror("%s: cia_r(%06x) = %04x & %04x\n", space.machine().describe_context(), offset, data, mem_mask);

	return data;
}

WRITE16_MEMBER( amiga_state::cia_w )
{
	if (LOG_CIA)
		logerror("%s: cia_w(%06x) = %04x & %04x\n", space.machine().describe_context(), offset, data, mem_mask);

	if ((offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
		m_cia_0->write(space, offset >> 7, data & 0xff);

	if ((offset & 0x2000/2) == 0 && ACCESSING_BITS_8_15)
		m_cia_1->write(space, offset >> 7, data >> 8);
}

WRITE16_MEMBER( amiga_state::gayle_cia_w )
{
	// the first write to cia 0 after a reset switches in chip ram
	if (m_gayle_reset && (offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
	{
		m_gayle_reset = false;
		m_overlay->set_bank(0);
	}

	// hand down to the standard cia handler
	cia_w(space, offset, data, mem_mask);
}

CUSTOM_INPUT_MEMBER( amiga_state::floppy_drive_status )
{
	return m_fdc->ciaapra_r();
}

WRITE8_MEMBER( amiga_state::cia_0_port_a_write )
{
	// bit 0, kickstart overlay
	m_overlay->set_bank(BIT(data, 0));

	// bit 1, power led
	set_led_status(space.machine(), 0, !BIT(data, 1));
	output_set_value("power_led", !BIT(data, 1));
}

WRITE_LINE_MEMBER( amiga_state::cia_0_irq )
{
	if (LOG_CIA)
		logerror("%s: cia_0_irq: %d\n", machine().describe_context(), state);

	m_cia_0_irq = state;
	update_int2();
}

READ8_MEMBER( amiga_state::cia_1_port_a_read )
{
	UINT8 data = 0;

	// bit 0 to 2, centronics
	data |= m_centronics_busy << 0;
	data |= m_centronics_perror << 1;
	data |= m_centronics_select << 2;

	// bit 2 to 7, serial line
	data |= m_rs232_ri << 2;
	data |= m_rs232_dsr << 3;
	data |= m_rs232_cts << 4;
	data |= m_rs232_dcd << 5;

	return data;
}

WRITE8_MEMBER( amiga_state::cia_1_port_a_write )
{
	if (m_rs232)
	{
		m_rs232->write_rts(BIT(data, 6));
		m_rs232->write_dtr(BIT(data, 7));
	}
}

WRITE_LINE_MEMBER( amiga_state::cia_1_irq )
{
	if (LOG_CIA)
		logerror("%s: cia_1_irq: %d\n", machine().describe_context(), state);

	m_cia_1_irq = state;
	update_int6();
}


//**************************************************************************
//  CUSTOM CHIPS
//**************************************************************************

void amiga_state::custom_chip_reset()
{
	amiga_state *state = this;

	CUSTOM_REG(REG_DENISEID) = m_denise_id;
	CUSTOM_REG(REG_VPOSR) = m_agnus_id << 8;
	CUSTOM_REG(REG_DDFSTRT) = 0x18;
	CUSTOM_REG(REG_DDFSTOP) = 0xd8;
	CUSTOM_REG(REG_INTENA) = 0x0000;
	CUSTOM_REG(REG_SERDATR) = SERDATR_RXD | SERDATR_TSRE | SERDATR_TBE;
	CUSTOM_REG(REG_BEAMCON0) = (m_agnus_id & 0x10) ? 0x0000 : 0x0020;
}

READ16_MEMBER( amiga_state::custom_chip_r )
{
	amiga_state *state = this;
	UINT16 temp;

	if (LOG_CUSTOM)
		logerror("%06X:read from custom %s\n", space.device().safe_pc(), amiga_custom_names[offset & 0xff]);

	switch (offset & 0xff)
	{
		case REG_BLTDDAT:
			return CUSTOM_REG(REG_BLTDDAT);

		case REG_DMACONR:
			return CUSTOM_REG(REG_DMACON);

		case REG_VPOSR:
			CUSTOM_REG(REG_VPOSR) &= 0xff00;
			CUSTOM_REG(REG_VPOSR) |= amiga_gethvpos() >> 16;

			return CUSTOM_REG(REG_VPOSR);

		case REG_VHPOSR:
			return amiga_gethvpos() & 0xffff;

		case REG_SERDATR:
			if (LOG_SERIAL)
				logerror("r SERDATR: %04x\n", CUSTOM_REG(REG_SERDATR));

			return CUSTOM_REG(REG_SERDATR);

		case REG_JOY0DAT:
			if (m_joy0dat_port)
				return joy0dat_r();

		case REG_JOY1DAT:
			if (m_joy1dat_port)
				return joy1dat_r();

		case REG_POTGOR:
			if (m_potgo_port)
				return m_potgo_port->read();
			else
				return 0x5500;

		case REG_POT0DAT:
			if (m_pot0dat_port)
			{
				return m_pot0dat_port->read();
			}
			else
			{
				int scale = m_agnus_id & 0x10 ? 525 : 625;

				m_pot0dat  = (int) ((double) m_pot0x / scale) * 0xff;
				m_pot0dat |= (int)(((double) m_pot0y / scale) * 0xff) << 8;

				return m_pot0dat;
			}

		case REG_POT1DAT:
			if (m_pot1dat_port)
			{
				return m_pot1dat_port->read();
			}
			else
			{
				int scale = m_agnus_id & 0x10 ? 525 : 625;

				m_pot1dat  = (int) ((double) m_pot1x / scale) * 0xff;
				m_pot1dat |= (int)(((double) m_pot1y / scale) * 0xff) << 8;

				return m_pot1dat;
			}

		case REG_DSKBYTR:
			return m_fdc->dskbytr_r();

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
			return m_fdc->dskpth_r();

		case REG_DSKPTL:
			return m_fdc->dskptl_r();

		case REG_ADKCONR:
			return m_fdc->adkcon_r();

		case REG_DSKDATR:
			popmessage("DSKDAT R, contact MESSdev");
			break;
	}

	return 0xffff;
}

WRITE16_MEMBER( amiga_state::custom_chip_w )
{
	amiga_state *state = space.machine().driver_data<amiga_state>();
	UINT16 temp;
	offset &= 0xff;

	if (LOG_CUSTOM)
		logerror("%06X:write to custom %s = %04X\n", space.device().safe_pc(), amiga_custom_names[offset & 0xff], data);

	switch (offset)
	{
		case REG_BLTDDAT:   case REG_DMACONR:   case REG_VPOSR:     case REG_VHPOSR:
		case REG_DSKDATR:   case REG_JOY0DAT:   case REG_JOY1DAT:   case REG_CLXDAT:
		case REG_ADKCONR:   case REG_POT0DAT:   case REG_POT1DAT:   case REG_POTGOR:
		case REG_SERDATR:   case REG_DSKBYTR:   case REG_INTENAR:   case REG_INTREQR:
			// read-only registers
			return;

		case REG_DSKDAT:
			popmessage("DSKDAT W %04x, contact MESSdev",data);
			break;

		case REG_DSKSYNC:
			m_fdc->dsksync_w(data);
			break;

		case REG_DSKPTH:
			m_fdc->dskpth_w(data);
			break;

		case REG_DSKPTL:
			m_fdc->dskptl_w(data);
			break;

		case REG_DSKLEN:
			m_fdc->dsklen_w(data);
			break;

		case REG_POTGO:
			if (BIT(data, 0))
			{
				// start counters
				m_pot0x = 0;
				m_pot0y = 0;
				m_pot1x = 0;
				m_pot1y = 0;
			}
			potgo_w(data);
			break;

		case REG_SERDAT:
			if (LOG_SERIAL)
				logerror("w SERDAT: %04x\n", data);

			CUSTOM_REG(REG_SERDAT) = data;

			// transmit shift register currently empty?
			if (CUSTOM_REG(REG_SERDATR) & SERDATR_TSRE)
			{
				// transfer new data to shift register
				m_tx_shift = CUSTOM_REG(REG_SERDAT);
				CUSTOM_REG(REG_SERDAT) = 0;

				// and signal transmit buffer empty
				CUSTOM_REG(REG_SERDATR) &= ~SERDATR_TSRE;
				CUSTOM_REG(REG_SERDATR) |= SERDATR_TBE;
				set_interrupt(INTENA_SETCLR | INTENA_TBE);
			}
			else
			{
				// transmit buffer now full
				CUSTOM_REG(REG_SERDATR) &= ~SERDATR_TBE;
			}

			return;

		case REG_SERPER:
			if (LOG_SERIAL)
				logerror("w SERPER: %04x\n", data);

			CUSTOM_REG(REG_SERPER) = data;
			serial_adjust();

			return;

		case REG_BLTSIZE:
			CUSTOM_REG(REG_BLTSIZE) = data;
			CUSTOM_REG(REG_BLTSIZV) = (data >> 6) & 0x3ff;
			CUSTOM_REG(REG_BLTSIZH) = data & 0x3f;
			if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x400;
			if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x40;
			blitter_setup(m_maincpu->space(AS_PROGRAM));
			break;

		case REG_BLTSIZV:
			if (IS_ECS(state) || IS_AGA(state))
			{
				CUSTOM_REG(REG_BLTSIZV) = data & 0x7fff;
				if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x8000;
			}
			break;

		case REG_BLTSIZH:
			if (IS_ECS(state) || IS_AGA(state))
			{
				CUSTOM_REG(REG_BLTSIZH) = data & 0x7ff;
				if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x800;
				blitter_setup(m_maincpu->space(AS_PROGRAM));
			}
			break;

		case REG_BLTCON0L:
			if (IS_ECS(state) || IS_AGA(state))
			{
				CUSTOM_REG(REG_BLTCON0) &= 0xff00;
				CUSTOM_REG(REG_BLTCON0) |= data & 0xff;
			}
			break;

		case REG_SPR0PTH:   case REG_SPR1PTH:   case REG_SPR2PTH:   case REG_SPR3PTH:
		case REG_SPR4PTH:   case REG_SPR5PTH:   case REG_SPR6PTH:   case REG_SPR7PTH:
			data &= ( m_chip_ram_mask >> 16 );
			break;

		case REG_SPR0PTL:   case REG_SPR1PTL:   case REG_SPR2PTL:   case REG_SPR3PTL:
		case REG_SPR4PTL:   case REG_SPR5PTL:   case REG_SPR6PTL:   case REG_SPR7PTL:
			amiga_sprite_dma_reset(space.machine(), (offset - REG_SPR0PTL) / 2);
			break;

		case REG_SPR0CTL:   case REG_SPR1CTL:   case REG_SPR2CTL:   case REG_SPR3CTL:
		case REG_SPR4CTL:   case REG_SPR5CTL:   case REG_SPR6CTL:   case REG_SPR7CTL:
			/* disable comparitor on writes here */
			amiga_sprite_enable_comparitor(space.machine(), (offset - REG_SPR0CTL) / 4, FALSE);
			break;

		case REG_SPR0DATA:  case REG_SPR1DATA:  case REG_SPR2DATA:  case REG_SPR3DATA:
		case REG_SPR4DATA:  case REG_SPR5DATA:  case REG_SPR6DATA:  case REG_SPR7DATA:
			/* enable comparitor on writes here */
			amiga_sprite_enable_comparitor(space.machine(), (offset - REG_SPR0DATA) / 4, TRUE);
			break;

		case REG_COP1LCH:
		case REG_COP2LCH:
			data &= ( m_chip_ram_mask >> 16 );
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
			m_sound->update();

			/* bits BBUSY (14) and BZERO (13) are read-only */
			data &= 0x9fff;
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			m_fdc->dmacon_set(data);

			/* if 'blitter-nasty' has been turned on and we have a blit pending, reschedule it */
			if ( ( data & 0x400 ) && ( CUSTOM_REG(REG_DMACON) & 0x4000 ) )
				m_blitter_timer->adjust(m_maincpu->cycles_to_attotime(BLITTER_NASTY_DELAY));

			break;

		case REG_INTENA:
			temp = data;

			data = (data & INTENA_SETCLR) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			CUSTOM_REG(offset) = data;

			if (temp & INTENA_SETCLR)
				// if we're enabling irq's, delay a bit
				m_irq_timer->adjust(m_maincpu->cycles_to_attotime(AMIGA_IRQ_DELAY_CYCLES));
			else
				// if we're disabling irq's, process right away
				update_irqs();
			break;

		case REG_INTREQ:
			temp = data;

			// clear receive buffer full?
			if (!(data & INTENA_SETCLR) && (data & INTENA_RBF))
			{
				CUSTOM_REG(REG_SERDATR) &= ~SERDATR_OVRUN;
				CUSTOM_REG(REG_SERDATR) &= ~SERDATR_RBF;
			}

			// clear transmit buffer empty?
			if (!(data & INTENA_SETCLR) && (data & INTENA_TBE))
				CUSTOM_REG(REG_SERDATR) |= SERDATR_TBE;

			data = (data & INTENA_SETCLR) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			CUSTOM_REG(offset) = data;

			if (temp & INTENA_SETCLR)
				// if we're generating irq's, delay a bit
				m_irq_timer->adjust(m_maincpu->cycles_to_attotime(AMIGA_IRQ_DELAY_CYCLES));
			else
				// if we're clearing irq's, process right away
				update_irqs();
			break;

		case REG_ADKCON:
			m_sound->update();
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			m_fdc->adkcon_set(data);
			break;

		case REG_AUD0LCL:   case REG_AUD0LCH:   case REG_AUD0LEN:   case REG_AUD0PER:   case REG_AUD0VOL:
		case REG_AUD1LCL:   case REG_AUD1LCH:   case REG_AUD1LEN:   case REG_AUD1PER:   case REG_AUD1VOL:
		case REG_AUD2LCL:   case REG_AUD2LCH:   case REG_AUD2LEN:   case REG_AUD2PER:   case REG_AUD2VOL:
		case REG_AUD3LCL:   case REG_AUD3LCH:   case REG_AUD3LEN:   case REG_AUD3PER:   case REG_AUD3VOL:
			m_sound->update();
			break;

		case REG_AUD0DAT:   case REG_AUD1DAT:   case REG_AUD2DAT:   case REG_AUD3DAT:
			m_sound->data_w((offset - REG_AUD0DAT) / 8, data);
			break;

		case REG_BPL1PTH:   case REG_BPL2PTH:   case REG_BPL3PTH:   case REG_BPL4PTH:
		case REG_BPL5PTH:   case REG_BPL6PTH:
			data &= ( m_chip_ram_mask >> 16 );
			break;

		case REG_BPLCON0:
			if ((data & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) == (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2))
			{
				/* planes go from 0 to 6, inclusive */
				logerror( "This game is doing funky planes stuff. (planes > 6)\n" );
				data &= ~BPLCON0_BPU0;
			}
			CUSTOM_REG(offset) = data;
			break;

		case REG_COLOR00:   case REG_COLOR01:   case REG_COLOR02:   case REG_COLOR03:
		case REG_COLOR04:   case REG_COLOR05:   case REG_COLOR06:   case REG_COLOR07:
		case REG_COLOR08:   case REG_COLOR09:   case REG_COLOR10:   case REG_COLOR11:
		case REG_COLOR12:   case REG_COLOR13:   case REG_COLOR14:   case REG_COLOR15:
		case REG_COLOR16:   case REG_COLOR17:   case REG_COLOR18:   case REG_COLOR19:
		case REG_COLOR20:   case REG_COLOR21:   case REG_COLOR22:   case REG_COLOR23:
		case REG_COLOR24:   case REG_COLOR25:   case REG_COLOR26:   case REG_COLOR27:
		case REG_COLOR28:   case REG_COLOR29:   case REG_COLOR30:   case REG_COLOR31:
			if (IS_AGA(state))
			{
				amiga_aga_palette_write(space.machine(), offset - REG_COLOR00, data);
			}
			else
			{
				data &= 0xfff;
				CUSTOM_REG(offset + 32) = (data >> 1) & 0x777;
			}
			break;

		// display window start/stop
		case REG_DIWSTRT:
		case REG_DIWSTOP:
			m_diwhigh_valid = false;
			break;

		// display window high
		case REG_DIWHIGH:
			if (IS_ECS(state) || IS_AGA(state))
			{
				m_diwhigh_valid = true;
				CUSTOM_REG(REG_DIWHIGH) = data;
			}
			break;

		case REG_BEAMCON0:
			// only available on ecs agnus
			if (m_agnus_id >= AGNUS_HR_PAL)
			{
				CUSTOM_REG(REG_BEAMCON0) = data;
				update_screenmode();
			}
			break;

		default:
			break;
	}

	if (IS_AGA(state))
		CUSTOM_REG(offset) = data;
	else
		if (offset <= REG_COLOR31)
			CUSTOM_REG(offset) = data;
}


//**************************************************************************
//  SERIAL
//**************************************************************************

void amiga_state::serial_adjust()
{
	amiga_state *state = this;

	UINT32 divisor = (CUSTOM_REG(REG_SERPER) & 0x7fff) + 1;
	UINT32 baud = m_sound->clock() / divisor;

	m_serial_timer->adjust(attotime::from_hz(baud) / 2, 0, attotime::from_hz(baud));
}

void amiga_state::serial_shift()
{
	amiga_state *state = this;

	if (CUSTOM_REG(REG_ADKCON) & ADKCON_UARTBRK)
	{
		// break active, force low
		rs232_tx(0);
	}
	else
	{
		// transmit shift register not empty?
		if ((CUSTOM_REG(REG_SERDATR) & SERDATR_TSRE) == 0)
		{
			if (m_tx_state == 0)
			{
				// transmit start bit
				rs232_tx(0);
				m_tx_state++;
			}
			else if (m_tx_state <= 8 + BIT(CUSTOM_REG(REG_SERPER), 15))
			{
				// send data bits
				rs232_tx(m_tx_shift & 1);
				m_tx_shift >>= 1;
				m_tx_state++;
			}
			else
			{
				// send stop bits until we run out
				if (m_tx_shift & 1)
				{
					rs232_tx(m_tx_shift & 1);
					m_tx_shift >>= 1;
				}
				else
				{
					// more data?
					if (CUSTOM_REG(REG_SERDAT))
					{
						// transfer to shift register
						m_tx_shift = CUSTOM_REG(REG_SERDAT);
						CUSTOM_REG(REG_SERDAT) = 0;

						// signal buffer empty
						CUSTOM_REG(REG_SERDATR) |= SERDATR_TBE;
						set_interrupt(INTENA_SETCLR | INTENA_TBE);
					}
					else
					{
						// we're done
						CUSTOM_REG(REG_SERDATR) |= SERDATR_TSRE;
					}

					m_tx_state = 0;
				}
			}
		}
		else
		{
			// transmit register empty
			rs232_tx(1);
		}
	}

	// waiting for start bit?
	if (m_rx_state == 0)
	{
		// start bit seen (high to low transition)
		if (m_rx_previous && (CUSTOM_REG(REG_SERDATR) & SERDATR_RXD) == 0)
		{
			m_rx_state++;
		}
	}
	else if (m_rx_state <= 8 + BIT(CUSTOM_REG(REG_SERPER), 15))
	{
		// receive data
		m_rx_shift >>= 1;
		m_rx_shift = (m_rx_shift & 0x7fff) | (BIT(CUSTOM_REG(REG_SERDATR), 11) << 15);
		m_rx_state++;
	}
	else
	{
		// stop bit
		m_rx_shift >>= 1;
		m_rx_shift = (m_rx_shift & 0x7fff) | (BIT(CUSTOM_REG(REG_SERDATR), 11) << 15);

		// shift to start
		m_rx_shift >>= (15 - (8 + BIT(CUSTOM_REG(REG_SERPER), 15)));

		// save data
		CUSTOM_REG(REG_SERDATR) &= ~0x3ff;
		CUSTOM_REG(REG_SERDATR) |= m_rx_shift & 0x3ff;

		// overrun?
		if (CUSTOM_REG(REG_SERDATR) & SERDATR_RBF)
			CUSTOM_REG(REG_SERDATR) |= SERDATR_OVRUN;

		// set ready and signal interrupt
		CUSTOM_REG(REG_SERDATR) |= SERDATR_RBF;
		set_interrupt(INTENA_SETCLR | INTENA_RBF);

		m_rx_shift = 0;
		m_rx_state = 0;
	}
}

void amiga_state::rs232_tx(int state)
{
	if (m_rs232)
		m_rs232->write_txd(state);
}

void amiga_state::rx_write(amiga_state *state, int level)
{
	m_rx_previous = BIT(CUSTOM_REG(REG_SERDATR), 11);
	CUSTOM_REG(REG_SERDATR) &= ~SERDATR_RXD;
	CUSTOM_REG(REG_SERDATR) |= level << 11;
}

WRITE_LINE_MEMBER( amiga_state::rs232_rx_w )
{
	rx_write(this, state);

	// start bit received?
	if (m_rx_state == 1)
		serial_adjust();
}

WRITE_LINE_MEMBER( amiga_state::rs232_dcd_w )
{
	m_rs232_dcd = state;
}

WRITE_LINE_MEMBER( amiga_state::rs232_dsr_w )
{
	m_rs232_dsr = state;
}

WRITE_LINE_MEMBER( amiga_state::rs232_ri_w )
{
	m_rs232_ri = state;
}

WRITE_LINE_MEMBER( amiga_state::rs232_cts_w )
{
	m_rs232_cts = state;
}
