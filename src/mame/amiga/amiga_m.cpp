// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Aaron Giles, Mariusz Wojcieszek
/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Aaron Giles, Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
#include "amiga.h"
#include "cpu/m68000/m68000.h"

/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_CUSTOM  (1U << 1)
#define LOG_CIA     (1U << 2)
#define LOG_BLITS   (1U << 3)
#define LOG_SERIAL  (1U << 4)

#define VERBOSE (LOG_SERIAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"



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

const char *const amiga_state::s_custom_reg_names[0x100] =
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

constexpr XTAL amiga_state::CLK_28M_PAL;
constexpr XTAL amiga_state::CLK_7M_PAL;
constexpr XTAL amiga_state::CLK_C1_PAL;
constexpr XTAL amiga_state::CLK_E_PAL;

constexpr XTAL amiga_state::CLK_28M_NTSC;
constexpr XTAL amiga_state::CLK_7M_NTSC;
constexpr XTAL amiga_state::CLK_C1_NTSC;
constexpr XTAL amiga_state::CLK_E_NTSC;

/*************************************
 *
 *  Machine reset
 *
 *************************************/

void amiga_state::machine_start()
{
	m_power_led.resolve();

	// set up chip RAM access
	memory_share *share = memshare("chip_ram");
	if (share == nullptr)
		fatalerror("Unable to find Amiga chip RAM\n");
	m_chip_ram.set(*share, 2);
	m_chip_ram_mask = (m_chip_ram.bytes() - 1) & ~1;

	// set up the timers
	m_irq_timer = timer_alloc(FUNC(amiga_state::irq_process_callback), this);
	m_blitter_timer = timer_alloc(FUNC(amiga_state::blitter_process_callback), this);
	m_serial_timer = timer_alloc(FUNC(amiga_state::serial_shift), this);
	m_scanline_timer = timer_alloc(FUNC(amiga_state::scanline_callback), this);

	// start the scanline timer
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}

void amiga_state::m68k_reset(int state)
{
	logerror("%s: Executed RESET\n", machine().describe_context());
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

void amiga_state::fdc_dskblk_w(int state)
{
	set_interrupt(INTENA_SETCLR | INTENA_DSKBLK);
}

void amiga_state::fdc_dsksyn_w(int state)
{
	set_interrupt((state ? INTENA_SETCLR : 0) | INTENA_DSKSYN);
}

void amiga_state::kbreset_w(int state)
{
	// this is connected to the gary chip, gary then resets the 68k, agnus, paula and the cias
	if (!state)
	{
		m_paula->reset();
		machine_reset();
		m_maincpu->reset();
	}
}

// simple mirror of region 0xf80000 to 0xfbffff
uint16_t amiga_state::rom_mirror_r(offs_t offset, uint16_t mem_mask)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset + 0xf80000, mem_mask);
}

uint32_t amiga_state::rom_mirror32_r(offs_t offset, uint32_t mem_mask)
{
	return m_maincpu->space(AS_PROGRAM).read_dword(offset + 0xf80000, mem_mask);
}


/*************************************
 *
 *  Per scanline callback
 *
 *************************************/

void amiga_state::vblank()
{
}

// TODO: CIA A clock can be connected to either a fixed 50/60hz signal from the power supply, or the vblank
// TODO: move almost everything to a DMA scheduler, cfr. HRM diagram
TIMER_CALLBACK_MEMBER( amiga_state::scanline_callback )
{
	int scanline = param;

	// vblank start
	if (scanline == 0)
	{
		// signal vblank irq
		set_interrupt(INTENA_SETCLR | INTENA_VERTB);

		// additional bookkeeping by drivers
		vblank();
	}

	// vblank end
	if (scanline == m_screen->visible_area().top())
	{
		// clock tod
		m_cia_0->tod_w(1);
		m_cia_0->tod_w(0);
	}

	if (m_potgo_port.found())
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
	//if (!m_screen->update_partial(scanline))
	{
		//bitmap_rgb32 dummy_bitmap;
		if (IS_AGA())
			aga_render_scanline(m_scanline_bitmap, scanline);
		else
			render_scanline(m_scanline_bitmap, scanline);
	}

	// clock tod (if we actually render this scanline)
	m_cia_1->tod_w((scanline & 1) ^ BIT(CUSTOM_REG(REG_VPOSR), 15));

	// force a sound update
	m_paula->update();

	// set timer for next line
	scanline = (scanline + 1) % m_screen->height();
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}



/*************************************
 *
 *  Interrupt management
 *
 *************************************/

void amiga_state::set_interrupt(int interrupt)
{
	custom_chip_w(REG_INTREQ, interrupt);
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

	// int 2 and 6 are level triggered
	if (int2_pending())
		CUSTOM_REG(REG_INTREQ) |= INTENA_PORTS;

	if (int6_pending())
		CUSTOM_REG(REG_INTREQ) |= INTENA_EXTER;
}

TIMER_CALLBACK_MEMBER( amiga_state::irq_process_callback )
{
	update_irqs();
	m_irq_timer->reset();
}

void amiga_state::paula_int_w (offs_t channel, u8 state)
{
	set_interrupt(INTENA_SETCLR | (0x80 << channel));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

uint16_t amiga_state::joy0dat_r()
{
	if (!m_input_device.found() || (m_input_device->read() & 0x10))
		return m_joy0dat_port.read_safe(0xffff);
	else
		return (m_p1_mouse_y.read_safe(0xff) << 8) | m_p1_mouse_x.read_safe(0xff);
}

uint16_t amiga_state::joy1dat_r()
{
	if (!m_input_device.found() || m_input_device->read() & 0x20)
		return m_joy1dat_port.read_safe(0xffff);
	else
		return (m_p2_mouse_y.read_safe(0xff) << 8) | m_p2_mouse_x.read_safe(0xff);
}



/*************************************
 *
 *  Ascending blitter variant
 *
 *************************************/

uint32_t amiga_state::blit_ascending()
{
	uint32_t shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	uint32_t shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	uint32_t height = CUSTOM_REG(REG_BLTSIZV);
	uint32_t width = CUSTOM_REG(REG_BLTSIZH);
	uint32_t acca = 0, accb = 0;
	uint32_t blitsum = 0;
	uint32_t x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			uint16_t abc0, abc1, abc2, abc3;
			uint32_t tempa, tempd = 0;
			uint32_t b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) += 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) += 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTCPTH));
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
				uint32_t bit;

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
				write_chip_ram(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
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

uint32_t amiga_state::blit_descending()
{
	uint32_t fill_exclusive = (CUSTOM_REG(REG_BLTCON1) >> 4);
	uint32_t fill_inclusive = (CUSTOM_REG(REG_BLTCON1) >> 3);
	uint32_t shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	uint32_t shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	uint32_t height = CUSTOM_REG(REG_BLTSIZV);
	uint32_t width = CUSTOM_REG(REG_BLTSIZH);
	uint32_t acca = 0, accb = 0;
	uint32_t blitsum = 0;
	uint32_t x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		uint32_t fill_state = (CUSTOM_REG(REG_BLTCON1) >> 2) & 1;

		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			uint16_t abc0, abc1, abc2, abc3;
			uint32_t tempa, tempd = 0;
			uint32_t b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) -= 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) -= 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTCPTH));
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
				uint32_t prev_fill_state;
				uint32_t bit;

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
				write_chip_ram(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
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

uint32_t amiga_state::blit_line()
{
	uint32_t singlemode = (CUSTOM_REG(REG_BLTCON1) & 0x0002) ? 0x0000 : 0xffff;
	uint32_t singlemask = 0xffff;
	uint32_t blitsum = 0;
	uint32_t height;

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
		uint16_t abc0, abc1, abc2, abc3;
		uint32_t tempa, tempb, tempd = 0;
		int b, dx, dy;

		/* fetch data for C */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG(REG_BLTCDAT) = read_chip_ram(CUSTOM_REG_LONG(REG_BLTCPTH));

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
			uint32_t bit;

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
		write_chip_ram(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);

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
			uint32_t temp = CUSTOM_REG(REG_BLTCON0) + (int32_t)(dx << 12);
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
			CUSTOM_REG_LONG(REG_BLTCPTH) += dy * (int16_t)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);
			CUSTOM_REG_LONG(REG_BLTDPTH) += dy * (int16_t)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);

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

TIMER_CALLBACK_MEMBER( amiga_state::blitter_process_callback )
{
	uint32_t blitsum = 0;

	/* logging */
	static const char *const type[] = { "ASCENDING", "LINE", "DESCENDING", "LINE" };
	LOGMASKED(LOG_BLITS, "BLIT %s: %dx%d  %04x %04x\n", type[CUSTOM_REG(REG_BLTCON1) & 0x0003], CUSTOM_REG(REG_BLTSIZH), CUSTOM_REG(REG_BLTSIZV), CUSTOM_REG(REG_BLTCON0), CUSTOM_REG(REG_BLTCON1));
	if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
		LOGMASKED(LOG_BLITS, "  A: addr=%06X mod=%3d shift=%2d maskl=%04x maskr=%04x\n", CUSTOM_REG_LONG(REG_BLTAPTH), CUSTOM_REG_SIGNED(REG_BLTAMOD), CUSTOM_REG(REG_BLTCON0) >> 12, CUSTOM_REG(REG_BLTAFWM), CUSTOM_REG(REG_BLTALWM));
	if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
		LOGMASKED(LOG_BLITS, "  B: addr=%06X mod=%3d shift=%2d\n", CUSTOM_REG_LONG(REG_BLTBPTH), CUSTOM_REG_SIGNED(REG_BLTBMOD), CUSTOM_REG(REG_BLTCON1) >> 12);
	if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
		LOGMASKED(LOG_BLITS, "  C: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTCPTH), CUSTOM_REG_SIGNED(REG_BLTCMOD));
	if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
		LOGMASKED(LOG_BLITS, "  D: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTDPTH), CUSTOM_REG_SIGNED(REG_BLTDMOD));

	/* set the zero flag */
	CUSTOM_REG(REG_DMACON) |= 0x2000;

	/* switch off the type of blit */
	switch (CUSTOM_REG(REG_BLTCON1) & 0x0003)
	{
		case 0: /* ascending */
			blitsum = blit_ascending();
			break;

		case 2: /* descending */
			blitsum = blit_descending();
			break;

		case 1: /* line */
		case 3:
			blitsum = blit_line();
			break;
	}

	/* clear the zero flag if we actually wrote data */
	if (blitsum)
		CUSTOM_REG(REG_DMACON) &= ~0x2000;
	LOGMASKED(LOG_BLITS, "%04x ZF=%d\n", blitsum, bool(BIT(CUSTOM_REG(REG_DMACON), 13)));

	/* no longer busy */
	CUSTOM_REG(REG_DMACON) &= ~0x4000;

	// signal an interrupt
	set_interrupt(INTENA_SETCLR | INTENA_BLIT);

	/* reset the blitter timer */
	m_blitter_timer->reset();
}



/*************************************
 *
 *  Blitter setup
 *
 *************************************/

void amiga_state::blitter_setup()
{
	int ticks, width, height, blittime;

	/* is there another blitting in progress? */
	if (CUSTOM_REG(REG_DMACON) & 0x4000)
	{
		logerror("%s - This program is playing tricks with the blitter\n", machine().describe_context() );
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
	// TODO: verify timings
	// - https://github.com/alpine9000/amiga_examples test 010,
	//   blitting is currently taking half the time than necessary.
	// - viz does heavy bbusy checks.
	blittime = ticks * height * width;

	/* if 'blitter-nasty' is set, then the blitter takes over the bus. Make the blit semi-immediate */
	// FIXME: emulate bus priority implications here
	// cfr. spinwrld no backgrounds
	if ( CUSTOM_REG(REG_DMACON) & 0x0400 )
	{
		/* simulate the 68k not running while the blit is going */
		m_maincpu->adjust_icount(-(blittime/2) );

		blittime = BLITTER_NASTY_DELAY;
	}

	/* AGA has twice the bus bandwidth, so blits take half the time */
	if (IS_AGA())
		blittime /= 2;

	/* signal blitter busy */
	CUSTOM_REG(REG_DMACON) |= 0x4000;

	/* set a timer */
	m_blitter_timer->adjust( m_maincpu->cycles_to_attotime( blittime ));
}


//**************************************************************************
//  CENTRONICS
//**************************************************************************

void amiga_state::centronics_ack_w(int state)
{
	m_cia_0->flag_w(state);
}

void amiga_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
	m_cia_1->sp_w(state);
}

void amiga_state::centronics_perror_w(int state)
{
	m_centronics_perror = state;
	m_cia_1->cnt_w(state);
}

void amiga_state::centronics_select_w(int state)
{
	m_centronics_select = state;
}


//**************************************************************************
//  8520 CIA
//**************************************************************************

// CIA-A access: 101x xxxx xxx0 oooo xxxx xxx1
// CIA-B access: 101x xxxx xx0x oooo xxxx xxx0

uint16_t amiga_state::cia_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if ((offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
		data |= m_cia_0->read(offset >> 7);

	if ((offset & 0x2000/2) == 0 && ACCESSING_BITS_8_15)
		data |= m_cia_1->read(offset >> 7) << 8;

	LOGMASKED(LOG_CIA, "%s: cia_r(%06x) = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);

	return data;
}

void amiga_state::cia_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_CIA, "%s: cia_w(%06x) = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);

	if ((offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
		m_cia_0->write(offset >> 7, data & 0xff);

	if ((offset & 0x2000/2) == 0 && ACCESSING_BITS_8_15)
		m_cia_1->write(offset >> 7, data >> 8);
}

void amiga_state::gayle_cia_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// the first write to cia 0 after a reset switches in chip ram
	if (m_gayle_reset && (offset & 0x1000/2) == 0 && ACCESSING_BITS_0_7)
	{
		m_gayle_reset = false;
		m_overlay->set_bank(0);
	}

	// hand down to the standard cia handler
	cia_w(offset, data, mem_mask);
}

ioport_value amiga_state::floppy_drive_status()
{
	return m_fdc->ciaapra_r();
}

void amiga_state::cia_0_port_a_write(uint8_t data)
{
	// bit 0, kickstart overlay
	m_overlay->set_bank(BIT(data, 0));

	// bit 1, power led
	m_power_led = BIT(~data, 1);
}

void amiga_state::cia_0_irq(int state)
{
	LOGMASKED(LOG_CIA, "%s: cia_0_irq: %d\n", machine().describe_context(), state);

	m_cia_0_irq = state;
	update_int2();
}

uint8_t amiga_state::cia_1_port_a_read()
{
	uint8_t data = 0;

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

void amiga_state::cia_1_port_a_write(uint8_t data)
{
	if (m_rs232)
	{
		m_rs232->write_rts(BIT(data, 6));
		m_rs232->write_dtr(BIT(data, 7));
	}
}

void amiga_state::cia_1_irq(int state)
{
	LOGMASKED(LOG_CIA, "%s: cia_1_irq: %d\n", machine().describe_context(), state);

	m_cia_1_irq = state;
	update_int6();
}


//**************************************************************************
//  CUSTOM CHIPS
//**************************************************************************

void amiga_state::ocs_map(address_map &map)
{
	// In progress: remove this catch-all trampoline, move almost everything into devices
	map(0x000, 0x1ff).rw(FUNC(amiga_state::custom_chip_r), FUNC(amiga_state::custom_chip_w));

	// Reading state section
//  map(0x000, 0x001).r(FUNC(amiga_state::bltddat_r));
//  map(0x002, 0x003).r(FUNC(amiga_state::dmaconr_r));
	map(0x004, 0x005).r(FUNC(amiga_state::vposr_r));
	map(0x006, 0x007).r(FUNC(amiga_state::vhposr_r));
//  map(0x008, 0x009).r(FUNC(amiga_state::dskdatr_r));
	// TODO: verify if JOYxDAT really belongs to Denise (???)
//  map(0x00a, 0x00b).r(FUNC(amiga_state::joydat_r<0>));
//  map(0x00c, 0x00d).r(FUNC(amiga_state::joydat_r<1>));
//  map(0x00e, 0x00f).r(FUNC(amiga_state::clxdat_r));
	map(0x010, 0x011).r(m_fdc, FUNC(amiga_fdc_device::adkcon_r));
//  map(0x012, 0x013).r(FUNC(amiga_state::potdat_r<0>)); // POT0DAT
//  map(0x014, 0x015).r(FUNC(amiga_state::potdat_r<1>));
//  map(0x016, 0x017).r(FUNC(amiga_state::potgor_r)); // a.k.a. POTINP
//  map(0x018, 0x019).r(FUNC(amiga_state::serdat_r));
	map(0x01a, 0x01b).r(m_fdc, FUNC(amiga_fdc_device::dskbytr_r));
//  map(0x01c, 0x01d).r(m_paula, FUNC(paula_8364_device::intenar_r));
//  map(0x01e, 0x01f).r(m_paula, FUNC(paula_8364_device::intreqr_r));

	// FDC writes
	// FIXME: these two belongs to Agnus, also shouldn't be readable
	map(0x020, 0x021).rw(m_fdc, FUNC(amiga_fdc_device::dskpth_r), FUNC(amiga_fdc_device::dskpth_w));
	map(0x022, 0x023).rw(m_fdc, FUNC(amiga_fdc_device::dskptl_r), FUNC(amiga_fdc_device::dskptl_w));
	map(0x024, 0x025).w(m_fdc, FUNC(amiga_fdc_device::dsklen_w));
//  map(0x026, 0x027).w(m_fdc, FUNC(amiga_fdc_device::dskdat_w));

//  map(0x028, 0x029).w(FUNC(amiga_state::refptr_w));
	map(0x02a, 0x02b).w(FUNC(amiga_state::vposw_w));
//  map(0x02c, 0x02d).w(FUNC(amiga_state::vhposw_w));
	map(0x02e, 0x02f).w(m_copper, FUNC(amiga_copper_device::copcon_w));

	// input strobes
//  map(0x030, 0x031).w(FUNC(amiga_state::serdat_w));
//  map(0x032, 0x033).w(FUNC(amiga_state::serper_w));
//  map(0x034, 0x035).w(FUNC(amiga_state::potgo_w));
//  map(0x036, 0x037).w(FUNC(amiga_state::joytest_w));

	// video beam strobes
//  map(0x038, 0x039).w(FUNC(amiga_state::strequ_w));
//  map(0x03a, 0x03b).w(FUNC(amiga_state::strvbl_w));
//  map(0x03c, 0x03d).w(FUNC(amiga_state::strhor_w));

//  map(0x040, 0x075).m(m_blitter, FUNC(amiga_blitter_device::regs_map));
//  map(0x07c, 0x07d).r <open bus for OCS>
	map(0x07e, 0x07f).w(m_fdc, FUNC(amiga_fdc_device::dsksync_w));

	// Copper
	map(0x080, 0x08b).m(m_copper, FUNC(amiga_copper_device::regs_map));
	map(0x08c, 0x08d).w(m_copper, FUNC(amiga_copper_device::copins_w));
	// Display window
//  map(0x08e, 0x08f).w(FUNC(amiga_state::diwstrt_w));
//  map(0x090, 0x091).w(FUNC(amiga_state::diwstop_w));
	// Display horizontal fetch
//  map(0x092, 0x093).w(FUNC(amiga_state::ddfstrt_w));
//  map(0x094, 0x095).w(FUNC(amiga_state::ddfstop_w));

//  map(0x096, 0x097).w(FUNC(amiga_state::dmacon_w));
//  map(0x098, 0x099).w(FUNC(amiga_state::clxcon_w));
//  map(0x09a, 0x09b).w(m_paula, FUNC(paula_8364_device::intena_w));
//  map(0x09c, 0x09d).w(m_paula, FUNC(paula_8364_device::intreq_w));
//  map(0x09e, 0x09f).w(m_paula, FUNC(paula_8364_device::adkcon_w));
	// Audio section
	map(0x0a0, 0x0ab).m(m_paula, FUNC(paula_8364_device::audio_channel_map<0>));
	map(0x0b0, 0x0bb).m(m_paula, FUNC(paula_8364_device::audio_channel_map<1>));
	map(0x0c0, 0x0cb).m(m_paula, FUNC(paula_8364_device::audio_channel_map<2>));
	map(0x0d0, 0x0db).m(m_paula, FUNC(paula_8364_device::audio_channel_map<3>));

	// Bitplane pointer section
//  map(0x0e0, 0x0ff).m(amiga_state::bplxptr_map));

	// Video bitplane registers
	map(0x100, 0x101).w(FUNC(amiga_state::bplcon0_w));
//  map(0x102, 0x103).w(FUNC(amiga_state::bplcon1_w));
//  map(0x104, 0x105).w(FUNC(amiga_state::bplcon2_w));

//  map(0x108, 0x109).w(FUNC(amiga_state::bpl1mod_w));
//  map(0x10a, 0x10b).w(FUNC(amiga_state::bpl2mod_w));

//  map(0x110, 0x11f).m(amiga_state::bplxdat_map));
	// Sprite section
//  map(0x120, 0x17f).m(amiga_state::sprxpt_map));
	// Color section
//  map(0x180, 0x1bf).m(amiga_state::colorxx_map));
}

void amiga_state::ecs_map(address_map &map)
{
	ocs_map(map);
//  map(0x03e, 0x03f).w(FUNC(amiga_state::strlong_w));
//  map(0x078, 0x079).w(FUNC(amiga_state::sprhdat_w));
//  map(0x07c, 0x07d).r(FUNC(amiga_state::deniseid_r));

//  map(0x106, 0x107).w(FUNC(amiga_state::bplcon3_w));

	// Extended ECS registers
	// video geometry regs
//  map(0x1c0, 0x1c7).m(FUNC(amiga_state::horz_screen_map));
//  map(0x1c8, 0x1cf).m(FUNC(amiga_state::vert_screen_map));
//  map(0x1dc, 0x1dd).w(FUNC(amiga_state::beamcon0_w));
	// Screen sync regs, VARHSY / VARVSY
//  map(0x1de, 0x1df).w(FUNC(amiga_state::hsstrt_w));
//  map(0x1e0, 0x1e1).w(FUNC(amiga_state::vsstrt_w));

//  map(0x1e2, 0x1e3).w(FUNC(amiga_state::hcenter_w));
//  map(0x1e4, 0x1e5).w(FUNC(amiga_state::diwhigh_w));

	// dummy, either related to copper lockup restart or "last N refresh cycles"
//  map(0x1fe, 0x1ff).?(FUNC(amiga_state::no_op_*));
}

void amiga_state::aga_map(address_map &map)
{
	ecs_map(map);

//  map(0x078, 0x079).w(FUNC(amiga_state::sprhdat_w));

	map(0x100, 0x101).w(FUNC(amiga_state::aga_bplcon0_w));

	// UHRES regs
	// TODO: may be shared with ECS?
//  map(0x1e6, 0x1e7).w(FUNC(amiga_state::bplhmod_w));
//  map(0x1e8, 0x1e9).w(FUNC(amiga_state::sprhpth_w));
//  map(0x1ea, 0x1eb).w(FUNC(amiga_state::sprhptl_w));
//  map(0x1ec, 0x1ed).w(FUNC(amiga_state::bplhpth_w));
//  map(0x1ed, 0x1ef).w(FUNC(amiga_state::bplhptl_w));

//  map(0x1fc, 0x1fd).w(FUNC(amiga_state::fmode_w));
}


void amiga_state::custom_chip_reset()
{
	CUSTOM_REG(REG_DENISEID) = m_denise_id;
	CUSTOM_REG(REG_VPOSR) = m_agnus_id << 8;
	CUSTOM_REG(REG_DDFSTRT) = 0x18;
	CUSTOM_REG(REG_DDFSTOP) = 0xd8;
	CUSTOM_REG(REG_INTENA) = 0x0000;
	CUSTOM_REG(REG_SERDATR) = SERDATR_RXD | SERDATR_TSRE | SERDATR_TBE;
	CUSTOM_REG(REG_BEAMCON0) = (m_agnus_id & 0x10) ? 0x0000 : 0x0020;
}

u16 amiga_state::vposr_r()
{
	u16 res = CUSTOM_REG(REG_VPOSR) & VPOSR_LOF;
	res |= m_agnus_id << 8;
	res |= ((amiga_gethvpos() >> 16) & 0xff);
	return res;
}

u16 amiga_state::vhposr_r()
{
	return amiga_gethvpos() & 0xffff;
}

void amiga_state::vposw_w(u16 data)
{
	// TODO: data actually resync the screen?
	// TODO: ECS always resets the LOF no matter the setting
	CUSTOM_REG(REG_VPOSR) = (data & VPOSR_LOF) | (data & 7);
	// TODO: high bits of screen height
//  if (data & 7)
//      popmessage("Upper VPOSW set %02x", data);
}

void amiga_state::bplcon0_w(u16 data)
{
	// FIXME: verify OCS / ECS games trying to set this up
	if ((data & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) == (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2))
	{
		/* planes go from 0 to 6, inclusive */
		popmessage( "bplcon0_w: setting up planes > 6, %04x", data );
		data &= ~BPLCON0_BPU0;
	}
	CUSTOM_REG(REG_BPLCON0) = data;
}

void amiga_state::aga_bplcon0_w(u16 data)
{
	// just allow all (AGA surfninj title screen relies on this)
	CUSTOM_REG(REG_BPLCON0) = data;
}

uint16_t amiga_state::custom_chip_r(offs_t offset)
{
	uint16_t temp;

	LOGMASKED(LOG_CUSTOM, "%06X:read from custom %s\n", m_maincpu->pc(), s_custom_reg_names[offset & 0xff]);

	switch (offset & 0xff)
	{
		case REG_BLTDDAT:
			return CUSTOM_REG(REG_BLTDDAT);

		case REG_DMACONR:
			return CUSTOM_REG(REG_DMACON);

		case REG_VHPOSR:
			return amiga_gethvpos() & 0xffff;

		case REG_SERDATR:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_SERIAL, "r SERDATR: %04x\n", CUSTOM_REG(REG_SERDATR));
			return CUSTOM_REG(REG_SERDATR);

		case REG_JOY0DAT:
			if (m_joy0dat_port.found())
				return joy0dat_r();
			[[fallthrough]]; // FIXME: Really?  Fall through to potentially reading the other joystick?
		case REG_JOY1DAT:
			if (m_joy1dat_port.found())
				return joy1dat_r();
			[[fallthrough]]; // TODO: check that this is correct
		case REG_POTGOR:
			return m_potgo_port.read_safe(0x5500);

		case REG_POT0DAT:
			if (m_pot0dat_port.found())
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
			if (m_pot1dat_port.found())
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
			popmessage("DSKDAT R");
			break;
	}

	return 0xffff;
}

void amiga_state::custom_chip_w(offs_t offset, uint16_t data)
{
	uint16_t temp;
	offset &= 0xff;

	LOGMASKED(LOG_CUSTOM, "%06X:write to custom %s = %04X\n", m_maincpu->pc(), s_custom_reg_names[offset & 0xff], data);

	switch (offset)
	{
		case REG_BLTDDAT:   case REG_DMACONR:   case REG_VPOSR:     case REG_VHPOSR:
		case REG_DSKDATR:   case REG_JOY0DAT:   case REG_JOY1DAT:   case REG_CLXDAT:
		case REG_ADKCONR:   case REG_POT0DAT:   case REG_POT1DAT:   case REG_POTGOR:
		case REG_SERDATR:   case REG_DSKBYTR:   case REG_INTENAR:   case REG_INTREQR:
			// read-only registers
			return;

		case REG_DSKDAT:
			popmessage("DSKDAT W %04x",data);
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
			LOGMASKED(LOG_SERIAL, "w SERDAT: %04x\n", data);
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
			LOGMASKED(LOG_SERIAL, "w SERPER: %04x\n", data);
			CUSTOM_REG(REG_SERPER) = data;
			serial_adjust();

			return;

		case REG_BLTSIZE:
			CUSTOM_REG(REG_BLTSIZE) = data;
			CUSTOM_REG(REG_BLTSIZV) = (data >> 6) & 0x3ff;
			CUSTOM_REG(REG_BLTSIZH) = data & 0x3f;
			if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x400;
			if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x40;
			blitter_setup();
			break;

		case REG_BLTSIZV:
			if (IS_ECS() || IS_AGA())
			{
				CUSTOM_REG(REG_BLTSIZV) = data & 0x7fff;
				if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x8000;
			}
			break;

		case REG_BLTSIZH:
			if (IS_ECS() || IS_AGA())
			{
				CUSTOM_REG(REG_BLTSIZH) = data & 0x7ff;
				if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x800;
				blitter_setup();
			}
			break;

		case REG_BLTCON0L:
			if (IS_ECS() || IS_AGA())
			{
				CUSTOM_REG(REG_BLTCON0) &= 0xff00;
				CUSTOM_REG(REG_BLTCON0) |= data & 0xff;
			}
			break;

		// soccerkd (OCS) and sockid_a (AGA) explicitly writes blitter addresses way beyond chip RAM
		// This is clearly deterministic: it draws an individual empty tile scattered across playfield
		// (which also collides on top of it)
		case REG_BLTAPTH:   case REG_BLTBPTH:   case REG_BLTCPTH:   case REG_BLTDPTH:
			data &= ( m_chip_ram_mask >> 16 );
			break;

		case REG_SPR0PTH:   case REG_SPR1PTH:   case REG_SPR2PTH:   case REG_SPR3PTH:
		case REG_SPR4PTH:   case REG_SPR5PTH:   case REG_SPR6PTH:   case REG_SPR7PTH:
			data &= ( m_chip_ram_mask >> 16 );
			break;

		case REG_SPR0PTL:   case REG_SPR1PTL:   case REG_SPR2PTL:   case REG_SPR3PTL:
		case REG_SPR4PTL:   case REG_SPR5PTL:   case REG_SPR6PTL:   case REG_SPR7PTL:
			sprite_dma_reset((offset - REG_SPR0PTL) / 2);
			break;

		case REG_SPR0CTL:   case REG_SPR1CTL:   case REG_SPR2CTL:   case REG_SPR3CTL:
		case REG_SPR4CTL:   case REG_SPR5CTL:   case REG_SPR6CTL:   case REG_SPR7CTL:
			/* disable comparitor on writes here */
			sprite_enable_comparitor((offset - REG_SPR0CTL) / 4, false);
			break;

		case REG_SPR0DATA:  case REG_SPR1DATA:  case REG_SPR2DATA:  case REG_SPR3DATA:
		case REG_SPR4DATA:  case REG_SPR5DATA:  case REG_SPR6DATA:  case REG_SPR7DATA:
			/* enable comparitor on writes here */
			sprite_enable_comparitor((offset - REG_SPR0DATA) / 4, true);
			break;

		case REG_DDFSTRT:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= (IS_AGA() || IS_ECS()) ? 0xfe : 0xfc;
			if (data < 0x18)
			{
				logerror("%s: Attempt to underrun DDFSTRT with %04x\n", machine().describe_context(), data);
				data = 0x18;
			}
			break;

		case REG_DDFSTOP:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= (IS_AGA() || IS_ECS()) ? 0xfe : 0xfc;
			if (data > 0xd8)
			{
				logerror("%s: Attempt to overrun DDFSTOP with %04x\n", machine().describe_context(), data);
				data = 0xd8;
			}
			break;

		case REG_DMACON:
			/* bits BBUSY (14) and BZERO (13) are read-only */
			data &= 0x9fff;
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			m_fdc->dmacon_set(data);
			m_paula->dmacon_set(data);
			m_copper->dmacon_set(data);

			/* if 'blitter-nasty' has been turned on and we have a blit pending, reschedule it */
			if ( ( data & 0x400 ) && ( CUSTOM_REG(REG_DMACON) & 0x4000 ) )
				m_blitter_timer->adjust(m_maincpu->cycles_to_attotime(BLITTER_NASTY_DELAY));

			break;

		case REG_INTENA:
			temp = data;

			data = (data & INTENA_SETCLR) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			CUSTOM_REG(offset) = data;
			if (temp & INTENA_SETCLR)
			{
				/* if we're enabling irq's, delay a bit */
				m_irq_timer->adjust(m_maincpu->cycles_to_attotime(AMIGA_IRQ_DELAY_CYCLES));
			}
			else
			{
				/* if we're disabling irq's, process right away */
				update_irqs();
			}
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
			{
				/* if we're generating irq's, delay a bit */
				m_irq_timer->adjust(m_maincpu->cycles_to_attotime(AMIGA_IRQ_DELAY_CYCLES));
			}
			else
			{
				/* if we're clearing irq's, process right away */
				update_irqs();
			}
			break;

		case REG_ADKCON:
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			m_fdc->adkcon_set(data);
			m_paula->adkcon_set(data);
			break;

		case REG_BPL1PTH:   case REG_BPL2PTH:   case REG_BPL3PTH:   case REG_BPL4PTH:
		case REG_BPL5PTH:   case REG_BPL6PTH:
			data &= ( m_chip_ram_mask >> 16 );
			break;

		case REG_BPL1MOD:   case REG_BPL2MOD:
			// bit 0 is implicitly ignored on writes,
			// and wouldn't otherwise make sense with 68k inability of word reading with odd addresses.
			// hpoker/hpokera would otherwise draw misaligned bottom GFX area without this (writes 0x27)
			data &= ~1;
			break;

		case REG_COLOR00:   case REG_COLOR01:   case REG_COLOR02:   case REG_COLOR03:
		case REG_COLOR04:   case REG_COLOR05:   case REG_COLOR06:   case REG_COLOR07:
		case REG_COLOR08:   case REG_COLOR09:   case REG_COLOR10:   case REG_COLOR11:
		case REG_COLOR12:   case REG_COLOR13:   case REG_COLOR14:   case REG_COLOR15:
		case REG_COLOR16:   case REG_COLOR17:   case REG_COLOR18:   case REG_COLOR19:
		case REG_COLOR20:   case REG_COLOR21:   case REG_COLOR22:   case REG_COLOR23:
		case REG_COLOR24:   case REG_COLOR25:   case REG_COLOR26:   case REG_COLOR27:
		case REG_COLOR28:   case REG_COLOR29:   case REG_COLOR30:   case REG_COLOR31:
			if (IS_AGA())
			{
				aga_palette_write(offset - REG_COLOR00, data);
			}
			else
			{
				data &= 0xfff;
				// Extra Half-Brite
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
			if (IS_ECS() || IS_AGA())
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

	// FIXME: no ECS?
	if (IS_AGA())
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
	uint32_t divisor = (CUSTOM_REG(REG_SERPER) & 0x7fff) + 1;
	uint32_t baud = m_paula->clock() / divisor;

	m_serial_timer->adjust(attotime::from_hz(baud) / 2, 0, attotime::from_hz(baud));
}

TIMER_CALLBACK_MEMBER(amiga_state::serial_shift)
{
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

void amiga_state::rx_write(int level)
{
	m_rx_previous = BIT(CUSTOM_REG(REG_SERDATR), 11);
	CUSTOM_REG(REG_SERDATR) &= ~SERDATR_RXD;
	CUSTOM_REG(REG_SERDATR) |= level << 11;
}

void amiga_state::rs232_rx_w(int state)
{
	rx_write(state);

	// start bit received?
	if (m_rx_state == 1)
		serial_adjust();
}

void amiga_state::rs232_dcd_w(int state)
{
	m_rs232_dcd = state;
}

void amiga_state::rs232_dsr_w(int state)
{
	m_rs232_dsr = state;
}

void amiga_state::rs232_ri_w(int state)
{
	m_rs232_ri = state;
}

void amiga_state::rs232_cts_w(int state)
{
	m_rs232_cts = state;
}
