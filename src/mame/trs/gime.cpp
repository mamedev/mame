// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    gime.cpp

    Implementation of CoCo GIME (Graphics Interrupt Memory Enhancement)
    video chip.

    Mid frame raster effects (source John Kowalski)

        Here are the things that get changed mid-frame:

            - Palette registers ($FFB0-$FFBF)
            - Horizontal resolution (switches between 256 and 320 pixels, $FF99)
            - Horizontal scroll position (bits 0-6 $FF9F)
            - Horizontal virtual screen (bit 7 $FF9F)
            - Pixel height (bits 0-2 $FF98)
            - Border color ($FF9A)

        On the positive side, you don't have to worry about registers
        $FF9D/$FF9E being changed mid-frame.  Even if they are changed
        mid-frame, they have no effect on the displayed image.  The video
        address only gets latched at the top of the frame.


    TIMING

    We divide a single frame into five regions

    Region              Scanlines (on 192)  Field Sync (before right border)
    ------              ------------------  --------------------------------
    Top Border          25                  1
    Body                192                 1
    Bottom Border       26                  1
    Vertical Retrace    6                   0
    Vertical Blanking   13 (or 13.5)        1

    Horizontal sync appears to be low only during retrace/blanking but
    field sync switches as soon as the last body pixel is displayed

    Note that the GIME trailing edges field sync after the bottom
    border; this is different than the MC6847

**********************************************************************

    SOFTWARE FOR TIMING TESTING

    Various CoCo 3 programs can "stress test" timing in various ways

    COLOR3: This program was in Rainbow January 1987.  It will synchronize
    with field sync by polling bit 7 of $FF03 (PIA0) (specifically, it polls
    until the rising edge of writes to PIA0 CB1).  This causes the loop to
    be broken out at the top of vblank.  COLOR3 also synchronizes on
    horizontal sync through PIA0; it counts 70 hsyncs to get to a point 32
    scanlines into the body.

    DEMO: This SockMaster demo has a main loop that adjusts GIME registers
    and shuffles some graphics bytes.  From a timing perspective, the
    critical piece is a SYNC with PIA field sync (specifically, it SYNCs
    with the trailing edge of writes to PIA0 CB1).  DEMO does not attempt
    to synchronize on horizontal sync; it relies on CPU timing.

    MOON: SockMaster demo.  Uses GIME interrupts; FIRQ gets fast interrupts
    and IRQ gets slow interrupts.  Since it does not use the PIA field
    sync, it can be used to demonstrate how the GIME's slow interrupt
    is distinct.

    BOINK: SockMaster demo.  Like DEMO, it SYNCs on the trailing edge
    of PIA field sync.  Also uses non standard $FF99 LPF mode.  If
    BOINK "wobbles vertically", it suggests that the main loop is not
    executing within a single field sync.  BOINK also makes extensive
    use of $FF9F scrolling with bit 7 cleared.

    CRYSTAL CITY: Jeremy Spiller game.  The intro uses an attribute-less
    GIME text mode

    ARKANOID: Abuses TIMER = 0 for ball ricochet sounds

    POP*STAR PILOT: Timer is synchronized with scanlines.

    Cloud Kingdoms: 512K bank switch on scanline.

**********************************************************************/


#include "emu.h"
#include "gime.h"
#include "bus/coco/cococart.h"
#include "machine/ram.h"




//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define USE_HORIZONTAL_CLIP     false
#define GIME_TYPE_1987          0
#define NO_ATTRIBUTE            0x80

#define LOG_INT_MASKING (1U <<  1)
#define LOG_GIME        (1U <<  2)
#define LOG_TIMER       (1U <<  3)
#define LOG_PALETTE     (1U <<  4)
#define LOG_MMU         (1U <<  5)
#define LOG_FBITS       (1U <<  6)
#define LOG_VBITS       (1U <<  7)
#define LOG_PBITS       (1U <<  8)
#define LOG_TBITS       (1U <<  9)
#define LOG_MBITS       (1U << 10)
#define LOG_RBITS       (1U << 11)

#define VERBOSE (0)
//#define VERBOSE (LOG_GIME|LOG_PBITS|LOG_PBITS|LOG_MMU)

#include "logmacro.h"

#define LOGINTMASKING(...) LOGMASKED(LOG_INT_MASKING, __VA_ARGS__)
#define LOGGIME(...) LOGMASKED(LOG_GIME, __VA_ARGS__)
#define LOGTIMER(...) LOGMASKED(LOG_TIMER, __VA_ARGS__)
#define LOGPALETTE(...) LOGMASKED(LOG_PALETTE, __VA_ARGS__)
#define LOGMMU(...) LOGMASKED(LOG_MMU, __VA_ARGS__)
#define LOGFBITS(...) LOGMASKED(LOG_FBITS, __VA_ARGS__)
#define LOGVBITS(...) LOGMASKED(LOG_VBITS, __VA_ARGS__)
#define LOGPBITS(...) LOGMASKED(LOG_PBITS, __VA_ARGS__)
#define LOGTBITS(...) LOGMASKED(LOG_TBITS, __VA_ARGS__)
#define LOGMBITS(...) LOGMASKED(LOG_MBITS, __VA_ARGS__)
#define LOGRBITS(...) LOGMASKED(LOG_RBITS, __VA_ARGS__)




//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

gime_device::gime_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint8_t *fontdata, bool pal)
	: mc6847_friend_device(mconfig, type, tag, owner, clock, fontdata, true, 262, 25+192+26+1, 8, false, pal)
	, sam6883_friend_device_interface(mconfig, *this, 8)
	, m_write_irq(*this)
	, m_write_firq(*this)
	, m_read_floating_bus(*this, 0)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_cart_device(*this, finder_base::DUMMY_TAG)
	, m_rom(nullptr)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gime_device::device_start()
{
	if (!m_ram->started())
		throw device_missing_dependencies();

	if (!m_cart_device->started())
		throw device_missing_dependencies();

	if (!m_cpu->started())
		throw device_missing_dependencies();

	// inherited device_start - need to do this after checking dependencies
	super::device_start();

	// initialize variables
	memset(m_scanlines, 0, sizeof(m_scanlines));
	m_interrupt_value = 0x00;
	m_irq = 0x00;
	m_firq = 0x00;

	// allocate timer
	m_gime_clock_timer = timer_alloc(FUNC(gime_device::timer_elapsed), this);

	// setup banks
	assert(std::size(m_read_banks) == std::size(m_write_banks));
	for (int i = 0; i < std::size(m_read_banks); i++)
	{
		char buffer[8];
		snprintf(buffer, std::size(buffer), "rbank%d", i);
		m_read_banks[i] = machine().root_device().membank(buffer);
		snprintf(buffer, std::size(buffer), "wbank%d", i);
		m_write_banks[i] = machine().root_device().membank(buffer);
	}

	// set up ROM/RAM pointers
	m_rom = m_rom_region->base();
	m_cart_rom = m_cart_device->get_cart_base();
	m_cart_size = m_cart_device->get_cart_size();

	// populate palettes
	m_composite_phase_invert = false;
	update_rgb_palette();
	update_composite_palette();

	// set up save states
	save_item(NAME(m_gime_registers));
	save_item(NAME(m_mmu));
	save_item(NAME(m_sam_state));
	save_item(NAME(m_ff22_value));
	save_item(NAME(m_ff23_value));
	save_item(NAME(m_interrupt_value));
	save_item(NAME(m_irq));
	save_item(NAME(m_firq));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_is_blinking));
	save_pointer(NAME(m_palette_rotated[0]), 16);
}



//-------------------------------------------------
//  update_rgb_palette
//-------------------------------------------------

void gime_device::update_rgb_palette()
{
	for (int color = 0; color < 64; color++)
	{
		m_rgb_palette[color] = get_rgb_color(color);
	}
}



//-------------------------------------------------
//  update_composite_palette
//-------------------------------------------------

void gime_device::update_composite_palette()
{
	for (int color = 0; color < 64; color++)
	{
		m_composite_palette[color] = get_composite_color(color);
		m_composite_bw_palette[color] = black_and_white(m_composite_palette[color]);
	}
}



//-------------------------------------------------
//  get_composite_color
//-------------------------------------------------

inline gime_device::pixel_t gime_device::get_composite_color(int color)
{
	static pixel_t composite_palette[64] = {
		0x000000, 0x004c00, 0x004300, 0x0a3100, 0x2f1b00, 0x550100, 0x6c0000, 0x770006,
		0x71004b, 0x5c008b, 0x3b00b8, 0x1100ca, 0x001499, 0x002c62, 0x004011, 0x004b00,
		0x2d2d2d, 0x069800, 0x288f00, 0x537d00, 0x786700, 0xa04c00, 0xb63402, 0xc3224c,
		0xbd1693, 0xa814d5, 0x881cfe, 0x5e2cff, 0x105ee9, 0x0076b2, 0x008b60, 0x009618,
		0x747474, 0x41d714, 0x62cf00, 0x8ebd00, 0xb4a700, 0xdd8c01, 0xf5733a, 0xfe6085,
		0xfd53ce, 0xe950ff, 0xc958ff, 0x9e67ff, 0x4e9aff, 0x36b3f7, 0x26c9a3, 0x2bd558,
		0xfdfdfe, 0x88e85a, 0xa1e03f, 0xbed238, 0xd8c342, 0xf1b161, 0xfea08d, 0xfe95bf,
		0xfd8ef1, 0xef8eff, 0xd895ff, 0xb9a1ff, 0x86c4ff, 0x78d4f2, 0x71e2b6, 0xffffff,
	};

	// composite output with phase inverted
	static pixel_t composite_palette_180[64] = {
		0x000000, 0x5a0e5a, 0x4f0c4f, 0x360f40, 0x0d213c, 0x003334, 0x004141, 0x004943,
		0x005409, 0x005600, 0x114c00, 0x263700, 0x392500, 0x491d00, 0x4f0f3e, 0x590e59,
		0x2d2d2d, 0xb11fb7, 0x9932c1, 0x7248c5, 0x4a5bc2, 0x1a6eba, 0x0077a9, 0x008c62,
		0x009619, 0x039700, 0x238f00, 0x467800, 0x9c4e00, 0xb23c00, 0xb92e59, 0xb6209e,
		0x747474, 0xe852ff, 0xcd60ff, 0xa677ff, 0x7d8aff, 0x4d9eff, 0x32b4ed, 0x29c7a2,
		0x2ad459, 0x39d223, 0x50c11a, 0x72a911, 0xcf831e, 0xf47733, 0xff5f85, 0xfe54d1,
		0xfdfdfc, 0xef8fff, 0xd697ff, 0xb8a4ff, 0x9eb3ff, 0x86c6ff, 0x76d4e7, 0x74ddb3,
		0x77e683, 0x80e170, 0x92d56b, 0xacc466, 0xeaac71, 0xffa385, 0xff95c1, 0xffffff
	};

	return (m_composite_phase_invert) ? composite_palette_180[color] : composite_palette[color];
}



//-------------------------------------------------
//  get_rgb_color
//-------------------------------------------------

inline gime_device::pixel_t gime_device::get_rgb_color(int color)
{
	return  (((color >> 4) & 2) | ((color >> 2) & 1)) * 0x550000
		|   (((color >> 3) & 2) | ((color >> 1) & 1)) * 0x005500
		|   (((color >> 2) & 2) | ((color >> 0) & 1)) * 0x000055;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gime_device::device_reset()
{
	/* Tepolt verifies that the GIME registers are all cleared on initialization */
	memset(m_gime_registers, 0, sizeof(m_gime_registers));

	/* initialize MMU */
	for (int i = 0; i < 8; i++)
		m_mmu[i] = m_mmu[i + 8] = 56 + i;

	/* initialize palette */
	memset(m_palette_rotated, 0, sizeof(m_palette_rotated));
	m_palette_rotated_position = 0;
	m_palette_rotated_position_used = false;

	/* clear SAM state */
	m_sam_state = 0x0000;

	/* clear interrupts */
	m_interrupt_value = 0x00;
	m_irq = 0x00;
	m_firq = 0x00;
	m_is_blinking = false;

	m_legacy_video = false;

	m_displayed_rgb = false;

	m_ff22_value = 0;
	m_ff23_value = 0;

	update_memory();
	reset_timer();
}



//-------------------------------------------------
//  device_pre_save - device-specific pre save
//-------------------------------------------------

void gime_device::device_pre_save()
{
	super::device_pre_save();

	// copy to palette rotation position zero
	for (offs_t i = 0; i < 16; i++)
		m_palette_rotated[0][i] = m_palette_rotated[m_palette_rotated_position][i];
}


//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void gime_device::device_post_load()
{
	super::device_post_load();
	update_cart_rom();
	update_memory();
	update_cpu_clock();

	// we update to position zero
	m_palette_rotated_position = 0;
	m_palette_rotated_position_used = false;
}



//-------------------------------------------------
//  device_input_ports
//-------------------------------------------------

ioport_constructor gime_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mc6847_artifacting);
}



//**************************************************************************
//  TIMER
//
//  The CoCo 3 had a timer that was always running, it
//  would decrement over and over again until zero was reached, and at that
//  point, would flag an interrupt.  At this point, the timer starts back up
//  again.
//
//  I am deducing that the timer interrupt line was asserted if the timer was
//  zero and unasserted if the timer was non-zero.
//
//  Most CoCo 3 docs, including the specs that Tandy released, say that the
//  high speed timer is 70ns (half of the speed of the main clock crystal).
//  However, it seems that this is in error, and the GIME timer is really a
//  279ns timer (one eighth the speed of the main clock crystal.  Gault's
//  FAQ agrees with this
//
//**************************************************************************

//-------------------------------------------------
//  timer_type
//-------------------------------------------------

gime_device::timer_type_t gime_device::timer_type()
{
	// wraps the GIME register access and returns an enumeration
	return (m_gime_registers[0x01] & 0x20) ? GIME_TIMER_279NSEC : GIME_TIMER_63USEC;
}



//-------------------------------------------------
//  timer_type_string
//-------------------------------------------------

const char *gime_device::timer_type_string()
{
	const char *result;
	switch(timer_type())
	{
		case GIME_TIMER_63USEC:
			result = "63USEC";
			break;
		case GIME_TIMER_279NSEC:
			result = "279NSEC";
			break;
		default:
			fatalerror("Should not get here\n");
	}
	return result;
}



//-------------------------------------------------
//  timer_elapsed
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(gime_device::timer_elapsed)
{
	/* reset the timer; give her another run! */
	reset_timer();

	/* change the blinking value - note that we don't have to use update_value() */
	m_is_blinking = !m_is_blinking;

	/* twiddle the timer interrupt */
	set_interrupt_value(INTERRUPT_TMR, true);
	set_interrupt_value(INTERRUPT_TMR, false);
}



//-------------------------------------------------
//  reset_timer
//-------------------------------------------------

void gime_device::reset_timer()
{
	/* value is from 0-4095 */
	m_timer_value = ((m_gime_registers[0x04] & 0x0F) << 8) | m_gime_registers[0x05];

	/* depending on the GIME type, canonicalize the value */
	if (m_timer_value > 0)
	{
		if (GIME_TYPE_1987)
			m_timer_value += 1; /* the 1987 GIME reset to the value plus one */
		else
			m_timer_value += 2; /* the 1986 GIME reset to the value plus two */

		if (timer_type() == GIME_TIMER_63USEC)
		{
			// master clock divided by 8, divided by 228, divided by 2
			m_gime_clock_timer->adjust(attotime::from_hz(clock()) * 3648 * m_timer_value / 2);
		}
		else
		{
			// master clock divided by 8, divided by 2
			m_gime_clock_timer->adjust(attotime::from_hz(clock()) * 16 * m_timer_value / 2);
		}
	}
	else
	{
		m_gime_clock_timer->adjust(attotime::never);
	}

	LOGTIMER("%s: reset_timer(): timer_type=%s value=%d\n", describe_context(), timer_type_string(), m_timer_value);
}



//**************************************************************************
//  MEMORY AND REGISTERS
//**************************************************************************

//-------------------------------------------------
//  update_memory
//-------------------------------------------------

inline void gime_device::update_memory()
{
	for (int bank = 0; bank <= 8; bank++)
	{
		update_memory(bank);
	}
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void gime_device::update_memory(int bank)
{
	// choose bank
	assert((bank >= 0) && (bank < std::size(m_read_banks)) && (bank < std::size(m_write_banks)));
	memory_bank *read_bank = m_read_banks[bank];
	memory_bank *write_bank = m_write_banks[bank];

	// bank 8 is really $FE00-$FEFF; it is weird so adjust for it
	offs_t offset;
	bool force_ram;
	bool enable_mmu = (m_gime_registers[0] & 0x40) ? true : false;
	if (bank == 8)
	{
		bank = 7;
		offset = 0x1E00;
		force_ram = (m_gime_registers[0] & 0x08);
		enable_mmu = enable_mmu && !(m_gime_registers[0] & 0x08);
	}
	else
	{
		offset = 0x0000;
		force_ram = false;
	}

	// is the MMU enabled at $FF90?
	int block;
	if (enable_mmu)
	{
		// check TR register at $FF91
		bank += (m_gime_registers[1] & 0x01) ? 8 : 0;

		// perform the MMU lookup
		block = m_mmu[bank];

		// also check $FF9B - relevant for the 2-8 MB upgrade
		block |= ((uint32_t) ((m_gime_registers[11] >> 4) & 0x03)) << 8;
	}
	else
	{
		// the MMU is not enabled
		block = bank + 56;
	}

	// are we actually in ROM?
	uint8_t *memory;
	bool is_read_only;
	if (((block & 0x3F) >= 0x3C) && !(m_sam_state & SAM_STATE_TY) && !force_ram)
	{
		// we're in ROM
		const uint8_t rom_mode = m_gime_registers[0] & 3;

		// are we in onboard ROM or cart ROM?
		if (rom_mode == 3 || (rom_mode < 2 && (block & 0x3F) >= 0x3E))
		{
			if (m_cart_rom)
			{
				// perform the look up (ROM page is pinned to MMU slot)
				memory = &m_cart_rom[(((bank & 3) ^ 2) * 0x2000) % m_cart_size];
			}
			else
			{
				memory = 0;
			}
		}
		else
		{
			memory = &m_rom[(bank & 3) * 0x2000];
		}
		is_read_only = true;
	}
	else
	{
		// we're in RAM
		memory = memory_pointer(block * 0x2000);
		is_read_only = false;
	}

	// set the banks
	if (memory)
	{
		read_bank->set_base(memory + offset);
		write_bank->set_base(is_read_only ? m_dummy_bank : memory + offset);
	}
	else
	{
		read_bank->set_base(m_dummy_bank);
		write_bank->set_base(m_dummy_bank);
	}
}



//-------------------------------------------------
//  memory_pointer
//-------------------------------------------------

uint8_t *gime_device::memory_pointer(uint32_t address)
{
	return &m_ram->pointer()[address % m_ram->size()];
}



//-------------------------------------------------
//  pia_write - observe writes to pia 1
//-------------------------------------------------

void gime_device::pia_write(offs_t offset, uint8_t data)
{
	if (offset == 0x03)
		m_ff23_value = data;

	/* only cache writes to $FF22 if the data register is addressed */
	if (offset == 0x02 && ((m_ff23_value & 0x04) == 0x04))
		m_ff22_value = data;
}



//-------------------------------------------------
//  update_cart_rom
//-------------------------------------------------

void gime_device::update_cart_rom()
{
	m_cart_rom = m_cart_device->get_cart_base();
	m_cart_size = m_cart_device->get_cart_size();
	update_memory();
}



//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t gime_device::read(offs_t offset)
{
	uint8_t data = 0x00;

	switch(offset & 0xF0)
	{
		case 0x00:
			data = read_gime_register(offset);          // $FF90 - $FF9F
			break;

		case 0x10:
			data = read_mmu_register(offset);           // $FFA0 - $FFAF
			break;

		case 0x20:
			data = read_palette_register(offset);       // $FFB0 - $FFBF
			break;

		default:
			data = read_floating_bus();
			break;
	}

	return data;
}



//-------------------------------------------------
//  read_gime_register
//-------------------------------------------------

inline uint8_t gime_device::read_gime_register(offs_t offset)
{
	offset &= 0x0F;

	uint8_t result;
	switch(offset)
	{
		case 2: // $FF92 - read pending IRQs
			result = m_irq;
			change_gime_irq(0x00);
			break;

		case 3: // $FF93 - read pending FIRQs
			result = m_firq;
			change_gime_firq(0x00);
			break;

#ifdef NOPE_NOT_READABLE
		case 14:
		case 15:
			// these (I guess) are readable (Mametesters bug #05135)
			result = m_gime_registers[offset];
			break;
#endif

		default:
			// the others are not readable; read floating bus (Mametesters bug #05135)
			result = read_floating_bus();
			break;
	}
	return result;
}



//-------------------------------------------------
//  read_mmu_register
//-------------------------------------------------

inline uint8_t gime_device::read_mmu_register(offs_t offset)
{
	return (m_mmu[offset & 0x0F] & 0x3F) | (read_floating_bus() & 0xC0);
}



//-------------------------------------------------
//  read_palette_register
//-------------------------------------------------

inline uint8_t gime_device::read_palette_register(offs_t offset)
{
	// Bits 7/6 are floating, and behave oddly.  On a real CoCo 3
	//
	//  POKE&HFFB1,255:PRINTPEEK(&HFFB1)    returns 127.
	//  POKE&HFFB1,0:PRINTPEEK(&HFFB1)      returns 64
	//
	// This is because of the floating bus
	return m_palette_rotated[m_palette_rotated_position][offset & 0x0F]
		| (read_floating_bus() & 0xC0);
}



//-------------------------------------------------
//  read_floating_bus
//-------------------------------------------------

inline uint8_t gime_device::read_floating_bus()
{
	return m_read_floating_bus(0);
}



//-------------------------------------------------
//  write
//-------------------------------------------------

void gime_device::write(offs_t offset, uint8_t data)
{
	switch(offset & 0xF0)
	{
		case 0x00:
			write_gime_register(offset & 0x0F, data);               // $FF90 - $FF9F
			break;

		case 0x10:
			write_mmu_register(offset & 0x0F, data);                // $FFA0 - $FFAF
			break;

		case 0x20:
			write_palette_register(offset & 0x0F, data & 0x3F);     // $FFB0 - $FFBF
			break;

		case 0x30:
		case 0x40:
			write_sam_register(offset - 0x30);                      // $FFC0 - $FFDF
			break;
	}
}



//-------------------------------------------------
//  write_gime_register
//-------------------------------------------------

inline void gime_device::write_gime_register(offs_t offset, uint8_t data)
{
	// sanity check input
	offset &= 0x0F;

	LOGGIME("%s: CoCo3 GIME: $%04x <== $%02x\n", describe_context(), offset + 0xff90, data);

	// make the change, and track the difference
	uint8_t xorval = m_gime_registers[offset] ^ data;
	m_gime_registers[offset] = data;

	switch(offset)
	{
		case 0x00:
			// $FF90 Initialization register 0
			//        Bit 7 COCO 1=CoCo compatible mode
			//        Bit 6 MMUEN 1=MMU enabled
			//        Bit 5 IEN 1 = GIME chip IRQ enabled
			//        Bit 4 FEN 1 = GIME chip FIRQ enabled
			//        Bit 3 MC3 1 = RAM at FEXX is constant
			//        Bit 2 MC2 1 = standard SCS (Spare Chip Select)
			//        Bit 1 MC1 ROM map control
			//        Bit 0 MC0 ROM map control
			if (xorval & 0x4B)
				update_memory();

			// IRQ master switch changed?
			if (xorval & 0x20)
			{
				if (data & 0x20)
					m_write_irq(irq_r());
				else
					m_write_irq(false);
			}

			// FIRQ master switch changed?
			if (xorval & 0x10)
			{
				if (data & 0x10)
					m_write_firq(firq_r());
				else
					m_write_firq(false);
			}
			break;

		case 0x01:
			//  $FF91 Initialization register 1
			//        Bit 7 Unused
			//        Bit 6 Unused
			//        Bit 5 TINS Timer input select; 1 = 280 nsec, 0 = 63.5 usec
			//        Bit 4 Unused
			//        Bit 3 Unused
			//        Bit 2 Unused
			//        Bit 1 Unused
			//        Bit 0 TR Task register select
			if (xorval & 0x01)
				update_memory();

			// Reset the timer with the _original_ value.  (This is correct!)
			if (xorval & 0x20)
				reset_timer();
			break;

		case 0x02:
			//  $FF92 Interrupt request enable register
			//        Bit 7 Unused
			//        Bit 6 Unused
			//        Bit 5 TMR Timer interrupt
			//        Bit 4 HBORD Horizontal border interrupt
			//        Bit 3 VBORD Vertical border interrupt
			//      ! Bit 2 EI2 Serial data interrupt
			//        Bit 1 EI1 Keyboard interrupt
			//        Bit 0 EI0 Cartridge interrupt
			LOGINTMASKING("%s: GIME IRQ: Interrupts { %s%s%s%s%s%s} enabled\n",
				describe_context(),
				(data & 0x20) ? "TMR " : "",
				(data & 0x10) ? "HBORD " : "",
				(data & 0x08) ? "VBORD " : "",
				(data & 0x04) ? "EI2 " : "",
				(data & 0x02) ? "EI1 " : "",
				(data & 0x01) ? "EI0 " : "");

			// While normally interrupts are acknowledged by reading from this
			// register and not writing to it, the act of disabling these interrupts
			// has the exact same effect
			//
			// Kudos to Glen Hewlett for identifying this problem
			change_gime_irq(m_irq & data);

			// special case timer reset value of zero
			if (m_timer_value == 0 && (xorval & INTERRUPT_TMR ))
			{
				set_interrupt_value(INTERRUPT_TMR, true);
				set_interrupt_value(INTERRUPT_TMR, false);
			}
			break;

		case 0x03:
			//  $FF93 Fast interrupt request enable register
			//        Bit 7 Unused
			//        Bit 6 Unused
			//        Bit 5 TMR Timer interrupt
			//        Bit 4 HBORD Horizontal border interrupt
			//        Bit 3 VBORD Vertical border interrupt
			//      ! Bit 2 EI2 Serial data interrupt
			//        Bit 1 EI1 Keyboard interrupt
			//        Bit 0 EI0 Cartridge interrupt
			LOGINTMASKING("%s: GIME FIRQ: Interrupts { %s%s%s%s%s%s} enabled\n",
				describe_context(),
				(data & 0x20) ? "TMR " : "",
				(data & 0x10) ? "HBORD " : "",
				(data & 0x08) ? "VBORD " : "",
				(data & 0x04) ? "EI2 " : "",
				(data & 0x02) ? "EI1 " : "",
				(data & 0x01) ? "EI0 " : "");

			// While normally interrupts are acknowledged by reading from this
			// register and not writing to it, the act of disabling these interrupts
			// has the exact same effect
			//
			// Kudos to Glen Hewlett for identifying this problem
			change_gime_firq(m_firq & data);

			// Special case timer reset value of zero
			if (m_timer_value == 0 && (xorval & INTERRUPT_TMR))
			{
				set_interrupt_value(INTERRUPT_TMR, true);
				set_interrupt_value(INTERRUPT_TMR, false);
			}

			break;

		case 0x04:
			//  $FF94 Timer register MSB
			//        Bits 4-7 Unused
			//        Bits 0-3 High order four bits of the timer
			reset_timer();
			break;

		case 0x05:
			//  $FF95 Timer register LSB
			//        Bits 0-7 Low order eight bits of the timer
			break;

		case 0x08:
			//  $FF98 Video Mode Register
			//        Bit 7 BP 0 = Text modes, 1 = Graphics modes
			//        Bit 6 Unused
			//      ! Bit 5 BPI Burst Phase Invert (Color Set)
			//        Bit 4 MOCH 1 = Monochrome on Composite
			//      ! Bit 3 H50 1 = 50 Hz power, 0 = 60 Hz power
			//        Bits 0-2 LPR Lines per row
			if (xorval & 0x20)
			{
				// on phase invert re-load the alternate composite palette
				m_composite_phase_invert = (data & 0x20);
				update_composite_palette();
			}

			break;

		case 0x09:
			//  $FF99 Video Resolution Register
			//        Bit 7 Undefined
			//        Bits 5-6 LPF Lines per Field (Number of Rows)
			//        Bits 2-4 HRES Horizontal Resolution
			//                     000=16 bytes per row
			//                     001=20 bytes per row
			//                     010=32 bytes per row
			//                     011=40 bytes per row
			//                     100=64 bytes per row
			//                     101=80 bytes per row
			//                     110=128 bytes per row
			//                     111=160 bytes per row
			//        Bits 0-1 CRES Color Resolution
			if (xorval & 0x60)
				update_geometry();
			break;

		case 0x0A:
			//  $FF9A Border Register
			//        Bits 6,7 Unused
			//        Bits 0-5 BRDR Border color
			break;

		case 0x0C:
			//  $FF9C Vertical Scroll Register
			//        Bits 4-7 Reserved
			//        Bits 0-3 VSC Vertical Scroll bits
			break;

		case 0x0B:
			//  $FF9B Two/Eight Megabyte Upgrade register
			//
			// This variable is weird; it affects both the video position, but
			// it also affects normal memory mapping.  However, changing $FF9B
			// alone won't affect the MMU; writes to $FFAx are required to "latch"
			// in the $FF9B value.
			//
			// The reason that $FF9B is not mentioned in official documentation
			// is because it is only meaningful in CoCo 3's with the 2MB upgrade
			break;

		case 0x0D:
		case 0x0E:
			//  $FF9B,$FF9D,$FF9E Vertical Offset Registers
			//
			//  According to JK, if an odd value is placed in $FF9E on the 1986
			//  GIME, the GIME crashes
			break;

		case 0x0F:
			//  $FF9F Horizontal Offset Register
			//       Bit 7 HVEN Horizontal Virtual Enable
			//       Bits 0-6 X0-X6 Horizontal Offset Address
			//
			//  Unline $FF9D-E, this value can be modified mid frame
			//
			//  Note that the FF9F offset is shifted by one bit (e.g. - $FF9F=$03 will
			//  be a six byte offset).  Also note that scanlines wrap at 256 byte boundaries
			//  without regard to bit 7.
			break;
	}
}



//-------------------------------------------------
//  write_mmu_register
//-------------------------------------------------

inline void gime_device::write_mmu_register(offs_t offset, uint8_t data)
{
	offset &= 0x0F;

	// Check to see if the MMU register has changed.  If we have more than 512k of RAM
	// then we have to always update because this is the point at which the MMU makes
	// decisions based off of $FF9B
	if ((m_mmu[offset] != data) || (m_ram->size() > 512*1024))
	{
		m_mmu[offset] = data;
		update_memory(offset & 0x07);
	}

	LOGMMU("%s: MMU Write: $%04x <== $%02X\n",describe_context(), 0xffa0+offset, data );
}



//-------------------------------------------------
//  write_palette_register
//-------------------------------------------------

inline void gime_device::write_palette_register(offs_t offset, uint8_t data)
{
	offset &= 0x0F;

	LOGPALETTE("%s: CoCo3 Palette: $%04x <== $%02x\n", describe_context(), offset + 0xffB0, data);

	/* has this entry changed? */
	if (m_palette_rotated[m_palette_rotated_position][offset] != data)
	{
		/* do we need to rotate the palette? */
		if (m_palette_rotated_position_used)
		{
			/* identify the new position */
			uint16_t new_palette_rotated_position = (m_palette_rotated_position + 1) % std::size(m_palette_rotated);

			/* copy the palette */
			for (int i = 0; i < std::size(m_palette_rotated[0]); i++)
				m_palette_rotated[new_palette_rotated_position][i] = m_palette_rotated[m_palette_rotated_position][i];

			/* and advance */
			m_palette_rotated_position = new_palette_rotated_position;
			m_palette_rotated_position_used = false;
		}

		/* record the change */;
		m_palette_rotated[m_palette_rotated_position][offset] = data;
	}
}



//-------------------------------------------------
//  write_sam_register
//-------------------------------------------------

inline void gime_device::write_sam_register(offs_t offset)
{
	/* change the SAM state */
	uint16_t xorval = alter_sam_state(offset);

	/* $FFDE-F can trigger a memory update */
	if (xorval & SAM_STATE_TY)
		update_memory();

	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
		update_cpu_clock();

	if (xorval & (SAM_STATE_F6|SAM_STATE_F5|SAM_STATE_F4|SAM_STATE_F3|SAM_STATE_F2|SAM_STATE_F1|SAM_STATE_F0))
	{
		LOGFBITS("%s: SAM F Address: $%04x\n",
			describe_context(),
			display_offset());
	}

	if (xorval & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
	{
		LOGVBITS("%s: SAM V Bits: $%02x\n",
			describe_context(),
			(m_sam_state & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2)));
	}

	if (xorval & (SAM_STATE_P1))
	{
		LOGPBITS("%s: SAM P1 Bit: $%02x\n",
			describe_context(),
			(m_sam_state & (SAM_STATE_P1)) >> 10);
	}

	if (xorval & (SAM_STATE_TY))
	{
		LOGTBITS("%s: SAM TY Bits: $%02x\n",
			describe_context(),
			(m_sam_state & (SAM_STATE_TY)) >> 15);
	}

	if (xorval & (SAM_STATE_M0|SAM_STATE_M1))
	{
		LOGMBITS("%s: SAM M Bits: $%02x\n",
			describe_context(),
			(m_sam_state & (SAM_STATE_M0|SAM_STATE_M1)) >> 9);
	}

	if (xorval & (SAM_STATE_R0|SAM_STATE_R1))
	{
		LOGRBITS("%s: SAM R Bits: $%02x\n",
			describe_context(),
			(m_sam_state & (SAM_STATE_R0|SAM_STATE_R1)) >> 11);
	}
}


//-------------------------------------------------
//  interrupt_rising_edge
//-------------------------------------------------

void gime_device::interrupt_rising_edge(uint8_t interrupt)
{
	// evaluate IRQ
	if (m_gime_registers[0x02] & interrupt)
		change_gime_irq(m_irq | interrupt);

	// evaluate FIRQ
	if (m_gime_registers[0x03] & interrupt)
		change_gime_firq(m_firq | interrupt);
}


//-------------------------------------------------
//  change_gime_irq
//-------------------------------------------------

void gime_device::change_gime_irq(uint8_t data)
{
	// did the value actually change?
	if (m_irq != data)
	{
		m_irq = data;
		if (m_gime_registers[0x00] & 0x20)
			m_write_irq(irq_r());
		else
			m_write_irq(false);
	}
}


//-------------------------------------------------
//  change_gime_firq
//-------------------------------------------------

void gime_device::change_gime_firq(uint8_t data)
{
	// did the value actually change?
	if (m_firq != data)
	{
		m_firq = data;
		if (m_gime_registers[0x00] & 0x10)
			m_write_firq(firq_r());
		else
			m_write_firq(false);
	}
}


//**************************************************************************
//  VIDEO STATE
//**************************************************************************

//-------------------------------------------------
//  get_video_base
//
//  The purpose of the ff9d_mask and ff9e_mask is to mask out bits that are
//  ignored in lo-res mode.  Specifically, $FF9D is masked with $E0, and
//  $FF9E is masked with $3F
//
//  John Kowalski confirms this behaviour
//-------------------------------------------------

inline offs_t gime_device::get_video_base()
{
	offs_t result;
	uint8_t ff9d_mask, ff9e_mask;

	if (m_legacy_video)
	{
		/* legacy video gets the base address from the SAM addresses, and masks out GIME variables */
		result = display_offset();
		ff9d_mask = 0xE0;
		ff9e_mask = 0x3F;
	}
	else
	{
		/* GIME video ignores the SAM addresses, relying on the GIME variables exclusively */
		result = 0;
		ff9d_mask = 0xFF;
		ff9e_mask = 0xFF;
	}

	result += ((offs_t) (m_gime_registers[0x0E] & ff9e_mask)    * 0x00008)
			| ((offs_t) (m_gime_registers[0x0D] & ff9d_mask)    * 0x00800);
	return result;
}



//-------------------------------------------------
//  new_frame
//-------------------------------------------------

void gime_device::new_frame()
{
	/* call inherited function */
	super::new_frame();

	/* latch in legacy video value */
	bool legacy_video_changed = update_value(&m_legacy_video, m_gime_registers[0] & 0x80 ? true : false);

	/* latch in the video position */
	m_video_position = get_video_base();
	m_line_in_row = m_gime_registers[0x0C] & 0x0F;

	/* update the geometry, as appropriate */
	if (legacy_video_changed)
		update_geometry();
}



//-------------------------------------------------
//  horizontal_sync_changed
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(gime_device::horizontal_sync_changed)
{
	set_interrupt_value(INTERRUPT_HBORD, (bool)param);
}



//-------------------------------------------------
//  enter_bottom_border
//-------------------------------------------------

void gime_device::enter_bottom_border()
{
	set_interrupt_value(INTERRUPT_VBORD, true);
	set_interrupt_value(INTERRUPT_VBORD, false);
}



//-------------------------------------------------
//  update_border
//-------------------------------------------------

void gime_device::update_border(uint16_t physical_scanline)
{
	uint8_t border;
	if (m_legacy_video)
	{
		/* legacy video */
		if (m_ff22_value & MODE_AG)
		{
			// graphics, green or white
			border = (~m_ff22_value & MODE_CSS) ? 0x12 : 0x3F;
		}
		else if (m_ff22_value & MODE_GM2)
		{
			// text, green or orange
			border = (~m_ff22_value & MODE_CSS) ? 0x12 : 0x26;
		}
		else
		{
			// text, black
			border = 0x00;
		}
	}
	else
	{
		/* get the border value from $FF9A */
		border = m_gime_registers[0x0A] & 0x3F;
	}
	update_value(&m_scanlines[physical_scanline].m_border, border);
}



//-------------------------------------------------
//  record_border_scanline
//-------------------------------------------------

void gime_device::record_border_scanline(uint16_t physical_scanline)
{
	m_line_in_row = 0;
	update_border(physical_scanline);
	update_value(&m_scanlines[physical_scanline].m_line_in_row, (uint8_t) ~0);
}



//-------------------------------------------------
//  get_lines_per_row
//-------------------------------------------------

inline uint16_t gime_device::get_lines_per_row()
{
	uint16_t lines_per_row;
	if (m_legacy_video)
	{
		switch(m_ff22_value & MODE_AG)
		{
			case 0:
			{
				// http://cocogamedev.mxf.yuku.com/topic/4299238#.VyC6ozArI-U
				static int ff9c_lines_per_row[16] =
				{
					11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 4, 3, 2, 1, 12
				};

				int i = m_gime_registers[0x0C] & 0x0F;
				//lines_per_row = 12;
				lines_per_row = ff9c_lines_per_row[i];
				break;
			}

			case MODE_AG:
				switch (m_sam_state & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
				{
				case 0:
					lines_per_row = 12;
					break;
				case SAM_STATE_V1:
					lines_per_row = 3;
					break;
				case SAM_STATE_V2:
				case SAM_STATE_V1|SAM_STATE_V0:
					lines_per_row = 2;
					break;
				default:
					lines_per_row = 1;
				}
				break;

			default:
				fatalerror("Should not get here\n");
		}
	}
	else
	{
		switch(m_gime_registers[0x08] & 0x07)
		{
			case 0x00:
			case 0x01:
				lines_per_row = 1;
				break;
			case 0x02:
				lines_per_row = 2;
				break;
			case 0x03:
				lines_per_row = 8;
				break;
			case 0x04:
				lines_per_row = 9;
				break;
			case 0x05:
				lines_per_row = 10;
				break;
			case 0x06:
				lines_per_row = 11;
				break;
			case 0x07:
				lines_per_row = 0xFFFF;
				break;
			default:
				fatalerror("Should not get here\n");
		}
	}
	return lines_per_row;
}



//-------------------------------------------------
//  record_scanline_res
//-------------------------------------------------

template<uint8_t xres, gime_device::get_data_func get_data, bool record_mode>
inline uint32_t gime_device::record_scanline_res(int scanline)
{
	int column;
	/* capture 512K memory bank per scan line */
	uint32_t bank_512k = (m_gime_registers[0x0B] & 0x0F) * 0x80000;
	uint32_t base_offset = m_legacy_video ? 0 : (m_gime_registers[0x0F] & 0x7F) * 2;
	uint32_t offset = 0;

	/* main loop */
	for (column = 0; column < xres; column++)
	{
		/* input data */
		uint8_t data, mode;
		offset += ((*this).*(get_data))((m_video_position + ((base_offset + offset) & 0xFF)) | bank_512k, &data, &mode);

		/* and record the pertinent values */
		if (record_mode)
			update_value(&m_scanlines[scanline].m_mode[column], mode);
		update_value(&m_scanlines[scanline].m_data[column], data);
		update_value(&m_scanlines[scanline].m_palette[column], m_palette_rotated_position);
	}

	return offset;
}



//-------------------------------------------------
//  get_data_mc6847 - used for retrieving data/mode
//  in legacy video modes
//-------------------------------------------------

uint32_t gime_device::get_data_mc6847(uint32_t video_position, uint8_t *data, uint8_t *mode)
{
	*data = *memory_pointer(video_position);
	*mode = (m_ff22_value & (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0|MODE_CSS))
		| ((*data & 0x80) == 0x80 ? MODE_AS : 0)
		| ((*data & 0xC0) == 0x40 ? MODE_INV : 0);

	// postprocess the mode to remove unnecessary flags
	*mode = mc6847_friend_device::simplify_mode(*data, *mode);
	return 1;
}



//-------------------------------------------------
//  get_data_without_attributes - used for
//  retrieving data/mode in GIME graphics or GIME
//  text modes without attributes
//-------------------------------------------------

uint32_t gime_device::get_data_without_attributes(uint32_t video_position, uint8_t *data, uint8_t *mode)
{
	*data = *memory_pointer(video_position);
	*mode = NO_ATTRIBUTE;
	return 1;
}



//-------------------------------------------------
//  get_data_with_attributes - used for retrieving
//  data/mode in GIME text modes with attributes
//-------------------------------------------------

uint32_t gime_device::get_data_with_attributes(uint32_t video_position, uint8_t *data, uint8_t *mode)
{
	*data = *memory_pointer(video_position + 0);
	*mode = *memory_pointer(video_position + 1);

	/* is the blink attribute specified? */
	if (UNEXPECTED(*mode & 0x80))
	{
		/* if so - and we're blinking - then clear the character */
		if (m_is_blinking)
		{
			*data = 0x20;
			*mode &= ~0x40;
		}

		/* clear the blink bit */
		*mode &= ~0x80;
	}
	return 2;
}



//-------------------------------------------------
//  record_body_scanline
//-------------------------------------------------

void gime_device::record_full_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline)
{
	/* update the border first */
	update_border(physical_scanline);

	/* set the line in row */
	update_value(&m_scanlines[physical_scanline].m_line_in_row, m_line_in_row);

	/* we're using this palette rotation */
	m_palette_rotated_position_used = true;

	uint32_t pitch = 0;
	if (m_legacy_video)
	{
		/* legacy video */
		update_value(&m_scanlines[physical_scanline].m_ff22_value, m_ff22_value);

		switch(m_ff22_value & (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0))
		{
			case MODE_AG:
			case MODE_AG|MODE_GM0:
			case MODE_AG|MODE_GM1|MODE_GM0:
			case MODE_AG|MODE_GM2|MODE_GM0:
				pitch = record_scanline_res<16, &gime_device::get_data_mc6847, true>(physical_scanline);
				break;

			case 0:
			case MODE_GM0:
			case MODE_GM1:
			case MODE_GM1|MODE_GM0:
			case MODE_GM2:
			case MODE_GM2|MODE_GM0:
			case MODE_GM2|MODE_GM1:
			case MODE_GM2|MODE_GM1|MODE_GM0:
			case MODE_AG|MODE_GM1:
			case MODE_AG|MODE_GM2:
			case MODE_AG|MODE_GM2|MODE_GM1:
			case MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0:
				pitch = record_scanline_res<32, &gime_device::get_data_mc6847, true>(physical_scanline);
				break;

			default:
				/* should not get here */
				fatalerror("Should not get here\n");
		}
	}
	else
	{
		/* CoCo 3 video */
		update_value(&m_scanlines[physical_scanline].m_ff98_value, m_gime_registers[0x08]);
		update_value(&m_scanlines[physical_scanline].m_ff99_value, m_gime_registers[0x09]);

		/* is this graphics or text? */
		if (m_gime_registers[0x08] & 0x80)
		{
			/* graphics */
			switch(m_gime_registers[0x09] & 0x1C)
			{
				case 0x00:  pitch = record_scanline_res< 16, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x04:  pitch = record_scanline_res< 20, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x08:  pitch = record_scanline_res< 32, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x0C:  pitch = record_scanline_res< 40, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x10:  pitch = record_scanline_res< 64, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x14:  pitch = record_scanline_res< 80, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x18:  pitch = record_scanline_res<128, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x1C:  pitch = record_scanline_res<160, &gime_device::get_data_without_attributes, false>(physical_scanline); break;
				default:
					fatalerror("Should not get here\n");
			}
		}
		else
		{
			/* text */
			switch(m_gime_registers[0x09] & 0x15)
			{
				case 0x00:  pitch = record_scanline_res< 32, &gime_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x01:  pitch = record_scanline_res< 32, &gime_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x04:  pitch = record_scanline_res< 40, &gime_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x05:  pitch = record_scanline_res< 40, &gime_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x10:  pitch = record_scanline_res< 64, &gime_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x11:  pitch = record_scanline_res< 64, &gime_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x14:  pitch = record_scanline_res< 80, &gime_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x15:  pitch = record_scanline_res< 80, &gime_device::get_data_with_attributes,    true>(physical_scanline);  break;
				default:
					fatalerror("Should not get here\n");
			}
		}

		/* is the GIME horizontal virtual screen enabled? */
		if (m_gime_registers[0x0F] & 0x80)
		{
			pitch = 256;
		}
	}

	/* sanity checks */
	assert(pitch > 0);

	/* are we moving on to the next line? */
	if (++m_line_in_row >= get_lines_per_row())
	{
		/* next row */
		m_line_in_row = 0;
		m_video_position += pitch;
	}
}



//-------------------------------------------------
//  record_partial_body_scanline
//-------------------------------------------------

void gime_device::record_partial_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline, int32_t start_clock, int32_t end_clock)
{
	fatalerror("NYI");
}



//-------------------------------------------------
//  update_geometry
//-------------------------------------------------

void gime_device::update_geometry()
{
	uint16_t top_border_scanlines, body_scanlines;

	switch(m_gime_registers[9] & 0x60) // GIME affects scanlines change even in legacy modes
	{
		case 0x00:
			// 192 lines (and legacy video)
			top_border_scanlines = 25;
			body_scanlines = 192;
			break;

		case 0x20:
			// 200 lines
			top_border_scanlines = 23;
			body_scanlines = 200;
			break;

		case 0x40:
			// zero/infinite lines
			//
			// If specified within the border, the border will go forever.  If specified within the body, the body will go forever.  This
			// suggests that there is a counter that counts scanlines, and we switch border <=> body when we hit some maximum count
			top_border_scanlines = ~0;
			body_scanlines = ~0;
			break;

		case 0x60:
			// 225 lines
			top_border_scanlines = 8;
			body_scanlines = 225;
			break;

		default:
			fatalerror("Should not get here\n");
	}

	// bit 3 of $FF99 controls "wideness"
	bool wide = !m_legacy_video && (m_gime_registers[9] & 0x08);

	// set the geometry
	set_geometry(top_border_scanlines, body_scanlines, wide);
}



//**************************************************************************
//  VIDEO BLITTING
//**************************************************************************

//-------------------------------------------------
//  emit_dummy_samples
//-------------------------------------------------

uint32_t gime_device::emit_dummy_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	fatalerror("Should not get here\n");
}

//-------------------------------------------------
//  emit_mc6847_samples
//-------------------------------------------------

inline uint32_t gime_device::emit_mc6847_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	return super::emit_mc6847_samples<2>(
		scanline->m_mode[sample_start],
		&scanline->m_data[sample_start],
		sample_count,
		pixels,
		palette,
		super::m_charrom_cb,
		sample_start,
		scanline->m_line_in_row);
}



//-------------------------------------------------
//  emit_gime_graphics_samples
//-------------------------------------------------

template<int xscale, int bits_per_pixel>
inline uint32_t gime_device::emit_gime_graphics_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	const uint8_t *data = &scanline->m_data[sample_start];
	mc6847_friend_device::emit_graphics<bits_per_pixel, xscale>(data, sample_count, pixels, 0, palette);
	return sample_count * (8 / bits_per_pixel) * xscale;
}



//-------------------------------------------------
//  emit_gime_text_samples
//-------------------------------------------------

template<int xscale>
inline uint32_t gime_device::emit_gime_text_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	uint8_t attribute = scanline->m_mode[sample_start];
	const uint8_t *data = &scanline->m_data[sample_start];

	/* determine the background/foreground colors */
	uint8_t bg = (attribute == NO_ATTRIBUTE) ? 0 : ((attribute >> 0) & 0x07) + 0x00;
	uint8_t fg = (attribute == NO_ATTRIBUTE) ? 1 : ((attribute >> 3) & 0x07) + 0x08;

	/* underline attribute */
	int underline_line = -1;
	if ((attribute != NO_ATTRIBUTE) && (attribute & 0x40))
	{
		/* to quote SockMaster:
		*
		* The underline attribute will light up the bottom scan line of the character
		* if the lines are set to 8 or 9.  Not appear at all when less, or appear on
		* the 2nd to bottom scan line if set higher than 9.  Further exception being
		* the $x7 setting where the whole screen is filled with only one line of data
		* - but it's glitched - the line repeats over and over again every 16 scan
		* lines..  Nobody will use this mode, but that's what happens if you want to
		* make things really authentic :)
		*/
		switch(scanline->m_ff98_value & 0x07)
		{
			case 0x03:
				underline_line = 7;
				break;
			case 0x04:
			case 0x05:
				underline_line = 8;
				break;
			case 0x06:
				underline_line = 9;
				break;
		}
	}

	for (int i = 0; i < sample_count; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < xscale; k++)
			{
				uint8_t font_byte = hires_font[data[i] & 0x7F][scanline->m_line_in_row];
				bool bit = (scanline->m_line_in_row == underline_line) || ((font_byte >> (7 - j)) & 0x01);
				pixels[(i * 8 + j) * xscale + k] = bit ? palette[fg] : palette[bg];
			}
		}
	}
	return sample_count * 8 * xscale;
}



//-------------------------------------------------
//  render_scanline
//-------------------------------------------------

template<int sample_count, gime_device::emit_samples_proc emit_samples>
inline void gime_device::render_scanline(const scanline_record *scanline, pixel_t *pixels, int min_x, int max_x, palette_resolver *resolver)
{
	int left_border, right_border;
	int x, x2, pixel_position;
	pixel_t border_color = resolver->lookup(scanline->m_border);
	const pixel_t *resolved_palette = nullptr;

	/* is this a wide video mode? */
	bool wide = !m_legacy_video && (scanline->m_ff99_value & 0x04);

	/* size up the borders */
	if (sample_count > 0)
	{
		left_border = wide ? 0 : 64;
		right_border = wide ? 640 : 512;
	}
	else
	{
		left_border = 640;
		right_border = 640;
	}

	/* left border */
	for (x = min_x; x < left_border; x++)
	{
		pixels[x] = border_color;
	}

	/* right border */
	for (x = right_border; x <= max_x; x++)
	{
		pixels[x] = border_color;
	}

	/* offset the pixel counts depending on wide */
	pixels += wide ? 0 : 64;

	/* body */
	x = 0;
	pixel_position = 0;
	while(x < sample_count)
	{
		/* determine how many bytes exist for which the mode is identical */
		for (x2 = x + 1; (x2 < sample_count) && (scanline->m_mode[x] == scanline->m_mode[x2]) && (scanline->m_palette[x] == scanline->m_palette[x2]); x2++)
			;

		/* resolve the palette */
		resolved_palette = resolver->get_palette(scanline->m_palette[x]);

		/* emit the samples, with a (hopefully) aggressively inlined function */
		pixel_position += ((*this).*(emit_samples))(scanline, x, x2 - x, &pixels[pixel_position], resolved_palette);

		/* update x */
		x = x2;
	}

	/* artifacting */
	if (m_legacy_video && (sample_count > 0))
	{
		m_artifacter.process_artifacts<2>(pixels, scanline->m_mode[0], resolved_palette);
	}
}



//-------------------------------------------------
//  update_screen
//-------------------------------------------------

bool gime_device::update_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pixel_t *RESTRICT palette)
{
	int base_x = 64;
	int min_x = USE_HORIZONTAL_CLIP ? cliprect.min_x : 0;
	int max_x = USE_HORIZONTAL_CLIP ? cliprect.max_x : (base_x * 2 + 512 - 1);
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	palette_resolver resolver(*this, palette);

	/* if the video didn't change, indicate as much */
	if (!has_video_changed())
		return UPDATE_HAS_NOT_CHANGED;

	for (int y = min_y; y <= max_y; y++)
	{
		const scanline_record *scanline = &m_scanlines[y];
		pixel_t *RESTRICT pixels = bitmap_addr(bitmap, y, 0);

		/* render the scanline */
		if (m_scanlines[y].m_line_in_row == (uint8_t) ~0)
		{
			/* this is a border scanline */
			render_scanline<0, &gime_device::emit_dummy_samples>(scanline, pixels, min_x, max_x, &resolver);
		}
		else if (m_legacy_video)
		{
			/* this is a legacy body scanline */
			switch(scanline->m_ff22_value & (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0))
			{
				case MODE_AG:
				case MODE_AG|MODE_GM0:
				case MODE_AG|MODE_GM1|MODE_GM0:
				case MODE_AG|MODE_GM2|MODE_GM0:
					render_scanline<16, &gime_device::emit_mc6847_samples>(scanline, pixels, min_x, max_x, &resolver);
					break;

				default:
					render_scanline<32, &gime_device::emit_mc6847_samples>(scanline, pixels, min_x, max_x, &resolver);
					break;
			}
		}
		else if (scanline->m_ff98_value & 0x80)
		{
			/* GIME graphics */
			switch(scanline->m_ff99_value & 0x1F)
			{
				case 0x00:  render_scanline< 16, &gime_device::emit_gime_graphics_samples< 4, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x01:  render_scanline< 16, &gime_device::emit_gime_graphics_samples< 8, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x02:
				case 0x03:  render_scanline< 16, &gime_device::emit_gime_graphics_samples<16, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x04:  render_scanline< 20, &gime_device::emit_gime_graphics_samples< 4, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x05:  render_scanline< 20, &gime_device::emit_gime_graphics_samples< 8, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x06:
				case 0x07:  render_scanline< 20, &gime_device::emit_gime_graphics_samples<16, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x08:  render_scanline< 32, &gime_device::emit_gime_graphics_samples< 2, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x09:  render_scanline< 32, &gime_device::emit_gime_graphics_samples< 4, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0A:
				case 0x0B:  render_scanline< 32, &gime_device::emit_gime_graphics_samples< 8, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0C:  render_scanline< 40, &gime_device::emit_gime_graphics_samples< 2, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0D:  render_scanline< 40, &gime_device::emit_gime_graphics_samples< 4, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0E:
				case 0x0F:  render_scanline< 40, &gime_device::emit_gime_graphics_samples< 8, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x10:  render_scanline< 64, &gime_device::emit_gime_graphics_samples< 1, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x11:  render_scanline< 64, &gime_device::emit_gime_graphics_samples< 2, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x12:
				case 0x13:  render_scanline< 64, &gime_device::emit_gime_graphics_samples< 4, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x14:  render_scanline< 80, &gime_device::emit_gime_graphics_samples< 1, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x15:  render_scanline< 80, &gime_device::emit_gime_graphics_samples< 2, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x16:
				case 0x17:  render_scanline< 80, &gime_device::emit_gime_graphics_samples< 4, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x18:
				case 0x19:  render_scanline<128, &gime_device::emit_gime_graphics_samples< 1, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1A:
				case 0x1B:  render_scanline<128, &gime_device::emit_gime_graphics_samples< 2, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1C:
				case 0x1D:  render_scanline<160, &gime_device::emit_gime_graphics_samples< 1, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1E:
				case 0x1F:  render_scanline<160, &gime_device::emit_gime_graphics_samples< 2, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
			}
		}
		else
		{
			/* GIME text */
			switch(scanline->m_ff99_value & 0x14)
			{
				case 0x00:  render_scanline<32, &gime_device::emit_gime_text_samples<2> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x04:  render_scanline<40, &gime_device::emit_gime_text_samples<2> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x10:  render_scanline<64, &gime_device::emit_gime_text_samples<1> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x14:  render_scanline<80, &gime_device::emit_gime_text_samples<1> >(scanline, pixels, min_x, max_x, &resolver);  break;
			}
		}
	}
	return 0;
}



//-------------------------------------------------
//  update_composite
//-------------------------------------------------

bool gime_device::update_composite(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_value(&m_displayed_rgb, false);
	const pixel_t *palette = (m_gime_registers[0x08] & 0x10)
		? m_composite_bw_palette
		: m_composite_palette;
	return update_screen(bitmap, cliprect, palette);
}



//-------------------------------------------------
//  update_rgb
//-------------------------------------------------

bool gime_device::update_rgb(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_value(&m_displayed_rgb, true);
	return update_screen(bitmap, cliprect, m_rgb_palette);
}



//**************************************************************************
//  PALETTE RESOLVER MEMBER CLASS
//**************************************************************************

//-------------------------------------------------
//  palette_resolver::palette_resolver
//-------------------------------------------------

inline gime_device::palette_resolver::palette_resolver(gime_device &gime, const pixel_t *palette)
	: m_gime(gime)
	, m_palette(palette)
{
	memset(m_resolved_palette, 0, sizeof(m_resolved_palette));
	m_current_resolved_palette = -1;
}



//-------------------------------------------------
//  palette_resolver::get_palette
//-------------------------------------------------

inline const gime_device::pixel_t *gime_device::palette_resolver::get_palette(uint16_t palette_rotation)
{
	if (UNEXPECTED(m_current_resolved_palette != palette_rotation))
	{
		for (int i = 0; i < 16; i++)
			m_resolved_palette[i] = lookup(m_gime.m_palette_rotated[palette_rotation][i]);
		m_current_resolved_palette = palette_rotation;
	}
	return m_resolved_palette;
}



//-------------------------------------------------
//  palette_resolver::lookup
//-------------------------------------------------

inline gime_device::pixel_t gime_device::palette_resolver::lookup(uint8_t color)
{
	assert(color <= 63);
	return m_palette[color];
}



//-------------------------------------------------
//  hires_font
//-------------------------------------------------

const uint8_t gime_device::hires_font[128][12] =
{
	{ 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x10}, { 0x44, 0x00, 0x44, 0x44, 0x44, 0x4C, 0x34, 0x00},
	{ 0x08, 0x10, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00}, { 0x10, 0x28, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00},
	{ 0x28, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00}, { 0x20, 0x10, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00},
	{ 0x10, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00}, { 0x00, 0x00, 0x38, 0x44, 0x40, 0x44, 0x38, 0x10},
	{ 0x10, 0x28, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00}, { 0x28, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00},
	{ 0x20, 0x10, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00}, { 0x28, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00},
	{ 0x10, 0x28, 0x00, 0x30, 0x10, 0x10, 0x38, 0x00}, { 0x00, 0x18, 0x24, 0x38, 0x24, 0x24, 0x38, 0x40},
	{ 0x44, 0x10, 0x28, 0x44, 0x7C, 0x44, 0x44, 0x00}, { 0x10, 0x10, 0x28, 0x44, 0x7C, 0x44, 0x44, 0x00},
	{ 0x08, 0x10, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, { 0x00, 0x00, 0x68, 0x14, 0x3C, 0x50, 0x3C, 0x00},
	{ 0x3C, 0x50, 0x50, 0x78, 0x50, 0x50, 0x5C, 0x00}, { 0x10, 0x28, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00},
	{ 0x28, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, { 0x00, 0x00, 0x38, 0x4C, 0x54, 0x64, 0x38, 0x00},
	{ 0x10, 0x28, 0x00, 0x44, 0x44, 0x4C, 0x34, 0x00}, { 0x20, 0x10, 0x44, 0x44, 0x44, 0x4C, 0x34, 0x00},
	{ 0x38, 0x4C, 0x54, 0x54, 0x54, 0x64, 0x38, 0x00}, { 0x44, 0x38, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00},
	{ 0x28, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00}, { 0x38, 0x40, 0x38, 0x44, 0x38, 0x04, 0x38, 0x00},
	{ 0x08, 0x14, 0x10, 0x38, 0x10, 0x14, 0x78, 0x00}, { 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x7C, 0x00},
	{ 0x10, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x08, 0x14, 0x10, 0x38, 0x10, 0x10, 0x50, 0x20},
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00},
	{ 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x28, 0x28, 0x7C, 0x28, 0x7C, 0x28, 0x28, 0x00},
	{ 0x10, 0x3C, 0x50, 0x38, 0x14, 0x78, 0x10, 0x00}, { 0x60, 0x64, 0x08, 0x10, 0x20, 0x4C, 0x0C, 0x00},
	{ 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00}, { 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00},
	{ 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00}, { 0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00},
	{ 0x00, 0x10, 0x54, 0x38, 0x38, 0x54, 0x10, 0x00}, { 0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00},
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20}, { 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00},
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00}, { 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00},
	{ 0x38, 0x44, 0x4C, 0x54, 0x64, 0x44, 0x38, 0x00}, { 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00},
	{ 0x38, 0x44, 0x04, 0x38, 0x40, 0x40, 0x7C, 0x00}, { 0x38, 0x44, 0x04, 0x08, 0x04, 0x44, 0x38, 0x00},
	{ 0x08, 0x18, 0x28, 0x48, 0x7C, 0x08, 0x08, 0x00}, { 0x7C, 0x40, 0x78, 0x04, 0x04, 0x44, 0x38, 0x00},
	{ 0x38, 0x40, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00}, { 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00},
	{ 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00}, { 0x38, 0x44, 0x44, 0x38, 0x04, 0x04, 0x38, 0x00},
	{ 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00}, { 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x20},
	{ 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00}, { 0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00},
	{ 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00}, { 0x38, 0x44, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00},
	{ 0x38, 0x44, 0x04, 0x34, 0x4C, 0x4C, 0x38, 0x00}, { 0x10, 0x28, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x00},
	{ 0x78, 0x24, 0x24, 0x38, 0x24, 0x24, 0x78, 0x00}, { 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00},
	{ 0x78, 0x24, 0x24, 0x24, 0x24, 0x24, 0x78, 0x00}, { 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x7C, 0x00},
	{ 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x40, 0x00}, { 0x38, 0x44, 0x40, 0x40, 0x4C, 0x44, 0x38, 0x00},
	{ 0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00}, { 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00},
	{ 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00}, { 0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00},
	{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C, 0x00}, { 0x44, 0x6C, 0x54, 0x54, 0x44, 0x44, 0x44, 0x00},
	{ 0x44, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x44, 0x00}, { 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00},
	{ 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00}, { 0x38, 0x44, 0x44, 0x44, 0x54, 0x48, 0x34, 0x00},
	{ 0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44, 0x00}, { 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00},
	{ 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00}, { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00},
	{ 0x44, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00}, { 0x44, 0x44, 0x44, 0x44, 0x54, 0x6C, 0x44, 0x00},
	{ 0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44, 0x00}, { 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00},
	{ 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00}, { 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00},
	{ 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00}, { 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00},
	{ 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x10, 0x00}, { 0x00, 0x10, 0x20, 0x7C, 0x20, 0x10, 0x00, 0x00},
	{ 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x00, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00},
	{ 0x40, 0x40, 0x58, 0x64, 0x44, 0x64, 0x58, 0x00}, { 0x00, 0x00, 0x38, 0x44, 0x40, 0x44, 0x38, 0x00},
	{ 0x04, 0x04, 0x34, 0x4C, 0x44, 0x4C, 0x34, 0x00}, { 0x00, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00},
	{ 0x08, 0x14, 0x10, 0x38, 0x10, 0x10, 0x10, 0x00}, { 0x00, 0x00, 0x34, 0x4C, 0x4C, 0x34, 0x04, 0x38},
	{ 0x40, 0x40, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00}, { 0x00, 0x10, 0x00, 0x30, 0x10, 0x10, 0x38, 0x00},
	{ 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x44, 0x38}, { 0x40, 0x40, 0x48, 0x50, 0x60, 0x50, 0x48, 0x00},
	{ 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00}, { 0x00, 0x00, 0x68, 0x54, 0x54, 0x54, 0x54, 0x00},
	{ 0x00, 0x00, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00}, { 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00},
	{ 0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40}, { 0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x04},
	{ 0x00, 0x00, 0x58, 0x64, 0x40, 0x40, 0x40, 0x00}, { 0x00, 0x00, 0x3C, 0x40, 0x38, 0x04, 0x78, 0x00},
	{ 0x20, 0x20, 0x70, 0x20, 0x20, 0x24, 0x18, 0x00}, { 0x00, 0x00, 0x44, 0x44, 0x44, 0x4C, 0x34, 0x00},
	{ 0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00}, { 0x00, 0x00, 0x44, 0x54, 0x54, 0x28, 0x28, 0x00},
	{ 0x00, 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, { 0x00, 0x00, 0x44, 0x44, 0x44, 0x3C, 0x04, 0x38},
	{ 0x00, 0x00, 0x7C, 0x08, 0x10, 0x20, 0x7C, 0x00}, { 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x00},
	{ 0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00}, { 0x20, 0x10, 0x10, 0x08, 0x10, 0x10, 0x20, 0x00},
	{ 0x20, 0x54, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00}
};


//-------------------------------------------------
//  lowres_font
//-------------------------------------------------

const uint8_t gime_device::lowres_font[] =
{
	0x00, 0x38, 0x44, 0x04, 0x34, 0x4C, 0x4C, 0x38, 0x00, 0x00, 0x00, 0x00, /* @ */
	0x00, 0x10, 0x28, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* A */
	0x00, 0x78, 0x24, 0x24, 0x38, 0x24, 0x24, 0x78, 0x00, 0x00, 0x00, 0x00, /* B */
	0x00, 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* C */
	0x00, 0x78, 0x24, 0x24, 0x24, 0x24, 0x24, 0x78, 0x00, 0x00, 0x00, 0x00, /* D */
	0x00, 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, 0x00, /* E */
	0x00, 0x7C, 0x40, 0x40, 0x70, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, /* F */
	0x00, 0x38, 0x44, 0x40, 0x40, 0x4C, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* G */
	0x00, 0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* H */
	0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, /* I */
	0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* J */
	0x00, 0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00, 0x00, /* K */
	0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, 0x00, /* L */
	0x00, 0x44, 0x6C, 0x54, 0x54, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* M */
	0x00, 0x44, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* N */
	0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* O */
	0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, /* P */
	0x00, 0x38, 0x44, 0x44, 0x44, 0x54, 0x48, 0x34, 0x00, 0x00, 0x00, 0x00, /* Q */
	0x00, 0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00, 0x00, /* R */
	0x00, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* S */
	0x00, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* T */
	0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* U */
	0x00, 0x44, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* V */
	0x00, 0x44, 0x44, 0x44, 0x44, 0x54, 0x6C, 0x44, 0x00, 0x00, 0x00, 0x00, /* W */
	0x00, 0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* X */
	0x00, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* Y */
	0x00, 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00, 0x00, 0x00, 0x00, /* Z */
	0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x00, 0x00, /* [ */
	0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, /* \ */
	0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x00, 0x00, /* ] */
	0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* up arrow */
	0x00, 0x00, 0x10, 0x20, 0x7C, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, /* left arrow */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* space */
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /* ! */
	0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* " */
	0x00, 0x28, 0x28, 0x7C, 0x28, 0x7C, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, /* # */
	0x00, 0x10, 0x3C, 0x50, 0x38, 0x14, 0x78, 0x10, 0x00, 0x00, 0x00, 0x00, /* $ */
	0x00, 0x60, 0x64, 0x08, 0x10, 0x20, 0x4C, 0x0C, 0x00, 0x00, 0x00, 0x00, /* % */
	0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x00, 0x00, 0x00, /* & */
	0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ' */
	0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, /* ( */
	0x00, 0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, /* ) */
	0x00, 0x00, 0x10, 0x54, 0x38, 0x38, 0x54, 0x10, 0x00, 0x00, 0x00, 0x00, /* * */
	0x00, 0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, /* + */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, /* , */
	0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* - */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /* . */
	0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, /* / */
	0x00, 0x38, 0x44, 0x4C, 0x54, 0x64, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* 0 */
	0x00, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, /* 1 */
	0x00, 0x38, 0x44, 0x04, 0x38, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00, 0x00, /* 2 */
	0x00, 0x38, 0x44, 0x04, 0x08, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* 3 */
	0x00, 0x08, 0x18, 0x28, 0x48, 0x7C, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, /* 4 */
	0x00, 0x7C, 0x40, 0x78, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* 5 */
	0x00, 0x38, 0x40, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* 6 */
	0x00, 0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, /* 7 */
	0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* 8 */
	0x00, 0x38, 0x44, 0x44, 0x38, 0x04, 0x04, 0x38, 0x00, 0x00, 0x00, 0x00, /* 9 */
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, /* : */
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, /* ; */
	0x00, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, /* < */
	0x00, 0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* = */
	0x00, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, /* > */
	0x00, 0x38, 0x44, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /* ? */

	/* Lower case */
	0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ^ */
	0x00, 0x00, 0x00, 0x38, 0x04, 0x3C, 0x44, 0x3C, 0x00, 0x00, 0x00, 0x00, /* a */
	0x00, 0x40, 0x40, 0x58, 0x64, 0x44, 0x64, 0x58, 0x00, 0x00, 0x00, 0x00, /* b */
	0x00, 0x00, 0x00, 0x38, 0x44, 0x40, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* c */
	0x00, 0x04, 0x04, 0x34, 0x4C, 0x44, 0x4C, 0x34, 0x00, 0x00, 0x00, 0x00, /* d */
	0x00, 0x00, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x38, 0x00, 0x00, 0x00, 0x00, /* e */
	0x00, 0x08, 0x14, 0x10, 0x38, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* f */
	0x00, 0x00, 0x00, 0x34, 0x4C, 0x4C, 0x34, 0x04, 0x38, 0x00, 0x00, 0x00, /* g */
	0x00, 0x40, 0x40, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* h */
	0x00, 0x00, 0x10, 0x00, 0x30, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, /* i */
	0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00, 0x00, /* j */
	0x00, 0x40, 0x40, 0x48, 0x50, 0x60, 0x50, 0x48, 0x00, 0x00, 0x00, 0x00, /* k */
	0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, /* l */
	0x00, 0x00, 0x00, 0x68, 0x54, 0x54, 0x54, 0x54, 0x00, 0x00, 0x00, 0x00, /* m */
	0x00, 0x00, 0x00, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, /* n */
	0x00, 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, /* o */
	0x00, 0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x00, 0x00, 0x00, /* p */
	0x00, 0x00, 0x00, 0x3C, 0x44, 0x44, 0x3C, 0x04, 0x04, 0x00, 0x00, 0x00, /* q */
	0x00, 0x00, 0x00, 0x58, 0x64, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, /* r */
	0x00, 0x00, 0x00, 0x3C, 0x40, 0x38, 0x04, 0x78, 0x00, 0x00, 0x00, 0x00, /* s */
	0x00, 0x20, 0x20, 0x70, 0x20, 0x20, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, /* t */
	0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x4C, 0x34, 0x00, 0x00, 0x00, 0x00, /* u */
	0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, /* v */
	0x00, 0x00, 0x00, 0x44, 0x54, 0x54, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, /* w */
	0x00, 0x00, 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, /* x */
	0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x3C, 0x04, 0x38, 0x00, 0x00, 0x00, /* y */
	0x00, 0x00, 0x00, 0x7C, 0x08, 0x10, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x00, /* z */
	0x00, 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, /* { */
	0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, /* | */
	0x00, 0x20, 0x10, 0x10, 0x08, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, /* } */
	0x00, 0x20, 0x54, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ~ */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00  /* _ */
};

//**************************************************************************
//  VARIATIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GIME_NTSC, gime_ntsc_device, "gime_ntsc", "TCC1014 (VC2645QC) GIME (NTSC)")
DEFINE_DEVICE_TYPE(GIME_PAL,  gime_pal_device,  "gime_pal",  "TCC1014 (VC2645QC) GIME (PAL)")

gime_ntsc_device::gime_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gime_device(mconfig, GIME_NTSC, tag, owner, clock, lowres_font, false) { }

gime_pal_device::gime_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gime_device(mconfig, GIME_PAL, tag, owner, clock, lowres_font, true) { }

template class device_finder<gime_device, false>;
template class device_finder<gime_device, true>;
