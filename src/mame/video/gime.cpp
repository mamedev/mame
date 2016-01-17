// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    gime.c

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

    MOON: SockMaster demo.  Uses GIME interrupts; FIRQ gets TMR interrupts
    and IRQ gets VBORD interrupts.  Since it does not use the PIA field
    sync, it can be used to demonstrate how the GIME's VBORD interrupt
    is distinct.

    BOINK: SockMaster demo.  Like DEMO, it SYNCs on the trailing edge
    of PIA field sync.  Also uses non standard $FF99 LPF mode.  If
    BOINK "wobbles vertically", it suggests that the main loop is not
    executing within a single field sync.  BOINK also makes extensive
    use of $FF9F scrolling with bit 7 cleared.

    CRYSTAL CITY: Jeremy Spiller game.  The intro uses an attribute-less
    GIME text mode

**********************************************************************/


#include "emu.h"
#include "video/gime.h"
#include "machine/6883sam.h"
#include "bus/coco/cococart.h"
#include "machine/ram.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define USE_HORIZONTAL_CLIP     false
#define GIME_TYPE_1987          0
#define NO_ATTRIBUTE            0x80

#define LOG_INT_MASKING         0
#define LOG_GIME                0
#define LOG_TIMER               0



//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

gime_base_device::gime_base_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, const UINT8 *fontdata, std::string shortname, std::string source)
	:   mc6847_friend_device(mconfig, type, name, tag, owner, clock, fontdata, true, 263, 25+192+26+3, false, shortname, source),
		m_write_irq(*this),
		m_write_firq(*this),
		m_read_floating_bus(*this)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gime_base_device::device_start(void)
{
	// find the RAM device - make sure that it is started
	m_ram = machine().device<ram_device>(m_ram_tag);
	if (!m_ram->started())
		throw device_missing_dependencies();

	// find the CART device - make sure that it is started
	m_cart_device = machine().device<cococart_slot_device>(m_ext_tag);
	if (!m_cart_device->started())
		throw device_missing_dependencies();

	// find the CPU device - make sure that it is started
	m_cpu = machine().device<cpu_device>(m_maincpu_tag);
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
	m_gime_clock_timer = timer_alloc(TIMER_GIME_CLOCK);

	// setup banks
	assert(ARRAY_LENGTH(m_read_banks) == ARRAY_LENGTH(m_write_banks));
	for (int i = 0; i < ARRAY_LENGTH(m_read_banks); i++)
	{
		char buffer[8];
		snprintf(buffer, ARRAY_LENGTH(buffer), "rbank%d", i);
		m_read_banks[i] = machine().root_device().membank(buffer);
		snprintf(buffer, ARRAY_LENGTH(buffer), "wbank%d", i);
		m_write_banks[i] = machine().root_device().membank(buffer);
	}

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_firq.resolve_safe();
	m_read_floating_bus.resolve_safe(0);

	// set up ROM/RAM pointers
	m_rom = machine().root_device().memregion(m_maincpu_tag)->base();
	m_cart_rom = m_cart_device->get_cart_base();

	// populate palettes
	for (int color = 0; color < 64; color++)
	{
		m_composite_palette[color] = get_composite_color(color);
		m_composite_bw_palette[color] = black_and_white(m_composite_palette[color]);
		m_rgb_palette[color] = get_rgb_color(color);
	}

	// set up save states
	save_pointer(NAME(m_gime_registers), ARRAY_LENGTH(m_gime_registers));
	save_pointer(NAME(m_mmu), ARRAY_LENGTH(m_mmu));
	save_item(NAME(m_sam_state));
	save_item(NAME(m_ff22_value));
	save_item(NAME(m_interrupt_value));
	save_item(NAME(m_irq));
	save_item(NAME(m_firq));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_is_blinking));
	save_pointer(NAME(m_palette_rotated[0]), 16);
}



//-------------------------------------------------
//  get_composite_color
//-------------------------------------------------

inline gime_base_device::pixel_t gime_base_device::get_composite_color(int color)
{
	/* CMP colors
	 *
	 * These colors are of the format IICCCC, where II is the intensity and
	 * CCCC is the base color.  There is some weirdness because intensity
	 * is often different for each base color.
	 *
	 * The code below is based on an algorithm specified in the following
	 * CoCo BASIC program was used to approximate composite colors.
	 * (Program by SockMaster):
	 *
	 * 10 POKE65497,0:DIMR(63),G(63),B(63):WIDTH80:PALETTE0,0:PALETTE8,54:CLS1
	 * 20 SAT=92:CON=70:BRI=-50:L(0)=0:L(1)=47:L(2)=120:L(3)=255
	 * 30 W=.4195456981879*1.01:A=W*9.2:S=A+W*5:D=S+W*5:P=0:FORH=0TO3:P=P+1
	 * 40 BRI=BRI+CON:FORG=1TO15:R(P)=COS(A)*SAT+BRI
	 * 50 G(P)=(COS(S)*SAT)*1+BRI:B(P)=(COS(D)*SAT)*1+BRI:P=P+1
	 * 55 A=A+W:S=S+W:D=D+W:NEXT:R(P-16)=L(H):G(P-16)=L(H):B(P-16)=L(H)
	 * 60 NEXT:R(63)=R(48):G(63)=G(48):B(63)=B(48)
	 * 70 FORH=0TO63STEP1:R=INT(R(H)):G=INT(G(H)):B=INT(B(H)):IFR<0THENR=0
	 * 80 IFG<0THENG=0
	 * 90 IFB<0THENB=0
	 * 91 IFR>255THENR=255
	 * 92 IFG>255THENG=255
	 * 93 IFB>255THENB=255
	 * 100 PRINTRIGHT$(STR$(H),2);" $";:R=R+256:G=G+256:B=B+256
	 * 110 PRINTRIGHT$(HEX$(R),2);",$";RIGHT$(HEX$(G),2);",$";RIGHT$(HEX$(B),2)
	 * 115 IF(H AND15)=15 THENIFINKEY$=""THEN115ELSEPRINT
	 * 120 NEXT
	 *
	 *  At one point, we used a different SockMaster program, but the colors
	 *  produced were too dark for people's taste
	 *
	 *  10 POKE65497,0:DIMR(63),G(63),B(63):WIDTH80:PALETTE0,0:PALETTE8,54:CLS1
	 *  20 SAT=92:CON=53:BRI=-16:L(0)=0:L(1)=47:L(2)=120:L(3)=255
	 *  30 W=.4195456981879*1.01:A=W*9.2:S=A+W*5:D=S+W*5:P=0:FORH=0TO3:P=P+1
	 *  40 BRI=BRI+CON:FORG=1TO15:R(P)=COS(A)*SAT+BRI
	 *  50 G(P)=(COS(S)*SAT)*.50+BRI:B(P)=(COS(D)*SAT)*1.9+BRI:P=P+1
	 *  55 A=A+W:S=S+W:D=D+W:NEXT:R(P-16)=L(H):G(P-16)=L(H):B(P-16)=L(H)
	 *  60 NEXT:R(63)=R(48):G(63)=G(48):B(63)=B(48)
	 *  70 FORH=0TO63STEP1:R=INT(R(H)):G=INT(G(H)):B=INT(B(H)):IFR<0THENR=0
	 *  80 IFG<0THENG=0
	 *  90 IFB<0THENB=0
	 *  91 IFR>255THENR=255
	 *  92 IFG>255THENG=255
	 *  93 IFB>255THENB=255
	 *  100 PRINTRIGHT$(STR$(H),2);" $";:R=R+256:G=G+256:B=B+256
	 *  110 PRINTRIGHT$(HEX$(R),2);",$";RIGHT$(HEX$(G),2);",$";RIGHT$(HEX$(B),2)
	 *  115 IF(H AND15)=15 THENIFINKEY$=""THEN115ELSEPRINT
	 *  120 NEXT
	 */

	double saturation, brightness, contrast;
	int offset;
	double w;
	int r, g, b;

	switch(color)
	{
		case 0:
			r = g = b = 0;
			break;

		case 16:
			r = g = b = 47;
			break;

		case 32:
			r = g = b = 120;
			break;

		case 48:
		case 63:
			r = g = b = 255;
			break;

		default:
			w = .4195456981879*1.01;
			contrast = 70;
			saturation = 92;
			brightness = -50;
			brightness += ((color / 16) + 1) * contrast;
			offset = (color % 16) - 1 + (color / 16)*15;
			r = cos(w*(offset +  9.2)) * saturation + brightness;
			g = cos(w*(offset + 14.2)) * saturation + brightness;
			b = cos(w*(offset + 19.2)) * saturation + brightness;

			if (r < 0)
				r = 0;
			else if (r > 255)
				r = 255;

			if (g < 0)
				g = 0;
			else if (g > 255)
				g = 255;

			if (b < 0)
				b = 0;
			else if (b > 255)
				b = 255;
			break;
	}
	return (pixel_t) ((r << 16) | (g << 8) | (b << 0));
}



//-------------------------------------------------
//  get_rgb_color
//-------------------------------------------------

inline gime_base_device::pixel_t gime_base_device::get_rgb_color(int color)
{
	return  (((color >> 4) & 2) | ((color >> 2) & 1)) * 0x550000
		|   (((color >> 3) & 2) | ((color >> 1) & 1)) * 0x005500
		|   (((color >> 2) & 2) | ((color >> 0) & 1)) * 0x000055;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gime_base_device::device_reset(void)
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

	update_memory();
	reset_timer();
}



//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void gime_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_GIME_CLOCK:
			timer_elapsed();
			break;

		default:
			super::device_timer(timer, id, param, ptr);
			break;
	}
}



//-------------------------------------------------
//  device_pre_save - device-specific pre save
//-------------------------------------------------

void gime_base_device::device_pre_save()
{
	super::device_pre_save();

	// copy to palette rotation position zero
	for (offs_t i = 0; i < 16; i++)
		m_palette_rotated[0][i] = m_palette_rotated[m_palette_rotated_position][i];
}


//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void gime_base_device::device_post_load()
{
	super::device_post_load();
	update_memory();
	update_cpu_clock();

	// we update to position zero
	m_palette_rotated_position = 0;
	m_palette_rotated_position_used = false;
}



//-------------------------------------------------
//  device_input_ports
//-------------------------------------------------

ioport_constructor gime_base_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mc6847_artifacting);
}



//**************************************************************************
//  TIMER
//
//  The CoCo 3 had a timer that had would activate when first written to, and
//  would decrement over and over again until zero was reached, and at that
//  point, would flag an interrupt.  At this point, the timer starts back up
//  again.
//
//  I am deducing that the timer interrupt line was asserted if the timer was
//  zero and unasserted if the timer was non-zero.  Since we never truly track
//  the timer, we just use timer callback (coco3_timer_callback() asserts the
//  line)
//
//  Most CoCo 3 docs, including the specs that Tandy released, say that the
//  high speed timer is 70ns (half of the speed of the main clock crystal).
//  However, it seems that this is in error, and the GIME timer is really a
//  280ns timer (one eighth the speed of the main clock crystal.  Gault's
//  FAQ agrees with this
//
//**************************************************************************

//-------------------------------------------------
//  timer_type
//-------------------------------------------------

gime_base_device::timer_type_t gime_base_device::timer_type(void)
{
	// wraps the GIME register access and returns an enumeration
	return (m_gime_registers[0x01] & 0x20) ? GIME_TIMER_CLOCK : GIME_TIMER_HBORD;
}



//-------------------------------------------------
//  timer_type_string
//-------------------------------------------------

const char *gime_base_device::timer_type_string(void)
{
	const char *result;
	switch(timer_type())
	{
		case GIME_TIMER_CLOCK:
			result = "CLOCK";
			break;
		case GIME_TIMER_HBORD:
			result = "HBORD";
			break;
		default:
			fatalerror("Should not get here\n");
	}
	return result;
}



//-------------------------------------------------
//  timer_elapsed
//-------------------------------------------------

void gime_base_device::timer_elapsed(void)
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

void gime_base_device::reset_timer(void)
{
	/* value is from 0-4095 */
	m_timer_value = ((m_gime_registers[0x04] & 0x0F) * 0x100) | m_gime_registers[0x05];

	/* depending on the GIME type, cannonicalize the value */
	if (m_timer_value > 0)
	{
		if (GIME_TYPE_1987)
			m_timer_value += 1; /* the 1987 GIME reset to the value plus one */
		else
			m_timer_value += 2; /* the 1986 GIME reset to the value plus two */
	}

	attotime duration;
	if ((timer_type() == GIME_TIMER_CLOCK) && (m_timer_value > 0))
	{
		/* we're starting a countdown on the GIME clock timer */
		attotime current_time = machine().time();
		UINT64 current_tick = current_time.as_ticks(m_clock);
		duration = attotime::from_ticks(current_tick + m_timer_value, m_clock) - current_time;
	}
	else
	{
		/* either the timer is off, or were not using the GIME clock timer */
		duration = attotime::never;
	}
	m_gime_clock_timer->adjust(duration);

	if (LOG_TIMER)
		logerror("%s: reset_timer(): timer_type=%s value=%d\n", describe_context(), timer_type_string(), m_timer_value);
}



//**************************************************************************
//  MEMORY AND REGISTERS
//**************************************************************************

//-------------------------------------------------
//  update_memory
//-------------------------------------------------

inline void gime_base_device::update_memory(void)
{
	for (int bank = 0; bank <= 8; bank++)
	{
		update_memory(bank);
	}
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void gime_base_device::update_memory(int bank)
{
	// choose bank
	assert((bank >= 0) && (bank < ARRAY_LENGTH(m_read_banks)) && (bank < ARRAY_LENGTH(m_write_banks)));
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
		force_ram = true;
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
		block |= ((UINT32) ((m_gime_registers[11] >> 4) & 0x03)) << 8;
	}
	else
	{
		// the MMU is not enabled
		block = bank + 56;
	}

	// are we actually in ROM?
	UINT8 *memory;
	bool is_read_only;
	if (((block & 0x3F) >= 0x3C) && !(m_sam_state & SAM_STATE_TY) && !force_ram)
	{
		// we're in ROM
		static const UINT8 rom_map[4][4] =
		{
			{ 0, 1, 6, 7 },
			{ 0, 1, 6, 7 },
			{ 0, 1, 2, 3 },
			{ 4, 5, 6, 7 }
		};

		// look up the block in the ROM map
		block = rom_map[m_gime_registers[0] & 3][(block & 0x3F) - 0x3C];

		// are we in onboard ROM or cart ROM?
		UINT8 *rom_ptr = (block & 4) ? m_cart_rom : m_rom;
		// TODO: make this unmapped
		if (rom_ptr==nullptr) rom_ptr = m_rom;
		// perform the look up
		memory = &rom_ptr[(block & 3) * 0x2000];
		is_read_only = true;
	}
	else
	{
		// we're in RAM
		memory = memory_pointer(block * 0x2000);
		is_read_only = false;
	}

	// compensate for offset
	memory += offset;

	// set the banks
	read_bank->set_base(memory);
	write_bank->set_base(is_read_only ? m_dummy_bank : memory);
}



//-------------------------------------------------
//  memory_pointer
//-------------------------------------------------

UINT8 *gime_base_device::memory_pointer(UINT32 address)
{
	return &m_ram->pointer()[address % m_ram->size()];
}



//-------------------------------------------------
//  update_cart_rom
//-------------------------------------------------

void gime_base_device::update_cart_rom(void)
{
	m_cart_rom = m_cart_device->get_cart_base();
	update_memory();
}



//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 gime_base_device::read(offs_t offset)
{
	UINT8 data = 0x00;

	switch(offset & 0xF0)
	{
		case 0x00:
			data = read_gime_register(offset);
			break;

		case 0x10:
			data = read_mmu_register(offset);
			break;

		case 0x20:
			data = read_palette_register(offset);
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

inline UINT8 gime_base_device::read_gime_register(offs_t offset)
{
	offset &= 0x0F;

	UINT8 result;
	switch(offset)
	{
		case 2: // read pending IRQs
			result = m_irq;
			if (result != 0x00)
			{
				m_irq = 0x00;
				recalculate_irq();
			}
			break;

		case 3: // read pending FIRQs
			result = m_firq;
			if (result != 0x00)
			{
				m_firq = 0x00;
				recalculate_firq();
			}
			break;

		case 14:
		case 15:
			// these (I guess) are readable (Mametesters bug #05135)
			result = m_gime_registers[offset];
			break;

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

inline UINT8 gime_base_device::read_mmu_register(offs_t offset)
{
	return (m_mmu[offset & 0x0F] & 0x3F) | (read_floating_bus() & 0xC0);
}



//-------------------------------------------------
//  read_palette_register
//-------------------------------------------------

inline UINT8 gime_base_device::read_palette_register(offs_t offset)
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

inline UINT8 gime_base_device::read_floating_bus(void)
{
	return m_read_floating_bus(0);
}



//-------------------------------------------------
//  write
//-------------------------------------------------

void gime_base_device::write(offs_t offset, UINT8 data)
{
	switch(offset & 0xF0)
	{
		case 0x00:
			write_gime_register(offset & 0x0F, data);
			break;

		case 0x10:
			write_mmu_register(offset & 0x0F, data);
			break;

		case 0x20:
			write_palette_register(offset & 0x0F, data & 0x3F);
			break;

		case 0x30:
		case 0x40:
			write_sam_register(offset - 0x30);
			break;
	}
}



//-------------------------------------------------
//  write_gime_register
//-------------------------------------------------

inline void gime_base_device::write_gime_register(offs_t offset, UINT8 data)
{
	// this is needed for writes to FF95
	bool timer_was_off = (m_gime_registers[0x04] == 0x00) && (m_gime_registers[0x05] == 0x00);

	// sanity check input
	offset &= 0x0F;

	// perform logging
	if (LOG_GIME)
		logerror("%s: CoCo3 GIME: $%04x <== $%02x\n", describe_context(), offset + 0xff90, data);

	// make the change, and track the difference
	UINT8 xorval = m_gime_registers[offset] ^ data;
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

			// I'm not sure about this one; as written this code will reset the timer
			// with the _original_ value.  This is probably not correct.
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
			if (LOG_INT_MASKING)
			{
				logerror("%s: GIME IRQ: Interrupts { %s%s%s%s%s%s} enabled\n",
					describe_context(),
					(data & 0x20) ? "TMR " : "",
					(data & 0x10) ? "HBORD " : "",
					(data & 0x08) ? "VBORD " : "",
					(data & 0x04) ? "EI2 " : "",
					(data & 0x02) ? "EI1 " : "",
					(data & 0x01) ? "EI0 " : "");
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
			if (LOG_INT_MASKING)
			{
				logerror("%s: GIME FIRQ: Interrupts { %s%s%s%s%s%s} enabled\n",
					describe_context(),
					(data & 0x20) ? "TMR " : "",
					(data & 0x10) ? "HBORD " : "",
					(data & 0x08) ? "VBORD " : "",
					(data & 0x04) ? "EI2 " : "",
					(data & 0x02) ? "EI1 " : "",
					(data & 0x01) ? "EI0 " : "");
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
			if (timer_was_off && (m_gime_registers[0x05] != 0x00))
			{
				// Writes to $FF95 do not cause the timer to reset, but MESS
				// will invoke coco3_timer_reset() if $FF94/5 was previously
				// $0000.  The reason for this is because the timer is not
				// actually off when $FF94/5 are loaded with $0000; rather it
				// is continuously reloading the GIME's internal countdown
				// register, even if it isn't causing interrupts to be raised.
				//
				// Failure to do this was the cause of bug #1065.  Special
				// thanks to John Kowalski for pointing me in the right
				// direction
				reset_timer();
			}
			break;

		case 0x08:
			//  $FF98 Video Mode Register
			//        Bit 7 BP 0 = Text modes, 1 = Graphics modes
			//        Bit 6 Unused
			//      ! Bit 5 BPI Burst Phase Invert (Color Set)
			//        Bit 4 MOCH 1 = Monochrome on Composite
			//      ! Bit 3 H50 1 = 50 Hz power, 0 = 60 Hz power
			//        Bits 0-2 LPR Lines per row
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
			// The reason that $FF9B is not mentioned in offical documentation
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

inline void gime_base_device::write_mmu_register(offs_t offset, UINT8 data)
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
}



//-------------------------------------------------
//  write_palette_register
//-------------------------------------------------

inline void gime_base_device::write_palette_register(offs_t offset, UINT8 data)
{
	offset &= 0x0F;

	/* has this entry changed? */
	if (m_palette_rotated[m_palette_rotated_position][offset] != data)
	{
		/* do we need to rotate the palette? */
		if (m_palette_rotated_position_used)
		{
			/* identify the new position */
			UINT16 new_palette_rotated_position = (m_palette_rotated_position + 1) % ARRAY_LENGTH(m_palette_rotated);

			/* copy the palette */
			for (int i = 0; i < ARRAY_LENGTH(m_palette_rotated[0]); i++)
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

inline void gime_base_device::write_sam_register(offs_t offset)
{
	/* change the SAM state */
	UINT16 xorval = alter_sam_state(offset);

	/* $FFDE-F can trigger a memory update */
	if (xorval & SAM_STATE_TY)
		update_memory();

	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
		update_cpu_clock();
}



//-------------------------------------------------
//  interrupt_rising_edge
//-------------------------------------------------

void gime_base_device::interrupt_rising_edge(UINT8 interrupt)
{
	/* evaluate IRQ */
	if ((m_gime_registers[0x00] & 0x20) && (m_gime_registers[0x02] & interrupt))
	{
		m_irq |= interrupt;
		recalculate_irq();
	}

	/* evaluate FIRQ */
	if ((m_gime_registers[0x00] & 0x10) && (m_gime_registers[0x03] & interrupt))
	{
		m_firq |= interrupt;
		recalculate_firq();
	}
}



//-------------------------------------------------
//  recalculate_irq
//-------------------------------------------------

void gime_base_device::recalculate_irq(void)
{
	m_write_irq(irq_r());
}



//-------------------------------------------------
//  recalculate_firq
//-------------------------------------------------

void gime_base_device::recalculate_firq(void)
{
	m_write_firq(firq_r());
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
//  John Kowalski confirms this behavior
//-------------------------------------------------

inline offs_t gime_base_device::get_video_base(void)
{
	offs_t result;
	UINT8 ff9d_mask, ff9e_mask;

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
			| ((offs_t) (m_gime_registers[0x0D] & ff9d_mask)    * 0x00800)
			| ((offs_t) (m_gime_registers[0x0B] & 0x0F)         * 0x80000);
	return result;
}



//-------------------------------------------------
//  new_frame
//-------------------------------------------------

void gime_base_device::new_frame(void)
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

void gime_base_device::horizontal_sync_changed(bool line)
{
	set_interrupt_value(INTERRUPT_HBORD, line);

	/* decrement timer if appropriate */
	if ((timer_type() == GIME_TIMER_HBORD) && (m_timer_value > 0) && line)
	{
		if (--m_timer_value == 0)
			timer_elapsed();
	}

}



//-------------------------------------------------
//  enter_bottom_border
//-------------------------------------------------

void gime_base_device::enter_bottom_border(void)
{
	set_interrupt_value(INTERRUPT_VBORD, true);
	set_interrupt_value(INTERRUPT_VBORD, false);
}



//-------------------------------------------------
//  update_border
//-------------------------------------------------

void gime_base_device::update_border(UINT16 physical_scanline)
{
	UINT8 border;
	if (m_legacy_video)
	{
		/* legacy video */
		switch(border_value(m_ff22_value, true))
		{
			case BORDER_COLOR_GREEN:
				border = 0x12;      /* green */
				break;
			case BORDER_COLOR_WHITE:
				border = 0x3F;      /* white */
				break;
			case BORDER_COLOR_BLACK:
				border = 0x00;      /* black */
				break;
			case BORDER_COLOR_ORANGE:
				border = 0x26;      /* orange */
				break;
			default:
				fatalerror("Should not get here\n");
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

void gime_base_device::record_border_scanline(UINT16 physical_scanline)
{
	m_line_in_row = 0;
	update_border(physical_scanline);
	update_value(&m_scanlines[physical_scanline].m_line_in_row, (UINT8) ~0);
}



//-------------------------------------------------
//  get_lines_per_row
//-------------------------------------------------

inline UINT16 gime_base_device::get_lines_per_row(void)
{
	UINT16 lines_per_row;
	if (m_legacy_video)
	{
		switch(m_ff22_value & (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0))
		{
			case 0:
			case MODE_GM0:
			case MODE_GM1:
			case MODE_GM1|MODE_GM0:
			case MODE_GM2:
			case MODE_GM2|MODE_GM0:
			case MODE_GM2|MODE_GM1:
			case MODE_GM2|MODE_GM1|MODE_GM0:
				lines_per_row = 12;
				break;

			case MODE_AG:
			case MODE_AG|MODE_GM0:
			case MODE_AG|MODE_GM1:
				lines_per_row = 3;
				break;

			case MODE_AG|MODE_GM1|MODE_GM0:
			case MODE_AG|MODE_GM2:
				lines_per_row = 2;
				break;

			case MODE_AG|MODE_GM2|MODE_GM0:
			case MODE_AG|MODE_GM2|MODE_GM1:
			case MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0:
				lines_per_row = 1;
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

template<UINT8 xres, gime_base_device::get_data_func get_data, bool record_mode>
inline UINT32 gime_base_device::record_scanline_res(int scanline)
{
	int column;
	UINT32 base_offset = m_legacy_video ? 0 : (m_gime_registers[0x0F] & 0x7F) * 2;
	UINT32 offset = 0;

	/* main loop */
	for (column = 0; column < xres; column++)
	{
		/* input data */
		UINT8 data, mode;
		offset += ((*this).*(get_data))(m_video_position + ((base_offset + offset) & 0xFF), &data, &mode);

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

UINT32 gime_base_device::get_data_mc6847(UINT32 video_position, UINT8 *data, UINT8 *mode)
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

UINT32 gime_base_device::get_data_without_attributes(UINT32 video_position, UINT8 *data, UINT8 *mode)
{
	*data = *memory_pointer(video_position);
	*mode = NO_ATTRIBUTE;
	return 1;
}



//-------------------------------------------------
//  get_data_with_attributes - used for retrieving
//  data/mode in GIME text modes with attributes
//-------------------------------------------------

UINT32 gime_base_device::get_data_with_attributes(UINT32 video_position, UINT8 *data, UINT8 *mode)
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

void gime_base_device::record_body_scanline(UINT16 physical_scanline, UINT16 logical_scanline)
{
	/* update the border first */
	update_border(physical_scanline);

	/* set the line in row */
	update_value(&m_scanlines[physical_scanline].m_line_in_row, m_line_in_row);

	/* we're using this palette rotation */
	m_palette_rotated_position_used = true;

	UINT32 pitch = 0;
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
				pitch = record_scanline_res<16, &gime_base_device::get_data_mc6847, true>(physical_scanline);
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
				pitch = record_scanline_res<32, &gime_base_device::get_data_mc6847, true>(physical_scanline);
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
				case 0x00:  pitch = record_scanline_res< 16, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x04:  pitch = record_scanline_res< 20, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x08:  pitch = record_scanline_res< 32, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x0C:  pitch = record_scanline_res< 40, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x10:  pitch = record_scanline_res< 64, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x14:  pitch = record_scanline_res< 80, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x18:  pitch = record_scanline_res<128, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				case 0x1C:  pitch = record_scanline_res<160, &gime_base_device::get_data_without_attributes, false>(physical_scanline); break;
				default:
					fatalerror("Should not get here\n");
			}
		}
		else
		{
			/* text */
			switch(m_gime_registers[0x09] & 0x15)
			{
				case 0x00:  pitch = record_scanline_res< 32, &gime_base_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x01:  pitch = record_scanline_res< 32, &gime_base_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x04:  pitch = record_scanline_res< 40, &gime_base_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x05:  pitch = record_scanline_res< 40, &gime_base_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x10:  pitch = record_scanline_res< 64, &gime_base_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x11:  pitch = record_scanline_res< 64, &gime_base_device::get_data_with_attributes,    true>(physical_scanline);  break;
				case 0x14:  pitch = record_scanline_res< 80, &gime_base_device::get_data_without_attributes, true>(physical_scanline);  break;
				case 0x15:  pitch = record_scanline_res< 80, &gime_base_device::get_data_with_attributes,    true>(physical_scanline);  break;
				default:
					fatalerror("Should not get here\n");
			}
		}

		/* is the GIME hoizontal virtual screen enabled? */
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

void gime_base_device::record_partial_body_scanline(UINT16 physical_scanline, UINT16 logical_scanline, INT32 start_clock, INT32 end_clock)
{
	fatalerror("NYI");
}



//-------------------------------------------------
//  update_geometry
//-------------------------------------------------

void gime_base_device::update_geometry(void)
{
	UINT16 top_border_scanlines, body_scanlines;

	switch(m_legacy_video ? 0x00 : (m_gime_registers[9] & 0x60))
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

UINT32 gime_base_device::emit_dummy_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	fatalerror("Should not get here\n");
}

//-------------------------------------------------
//  emit_mc6847_samples
//-------------------------------------------------

inline UINT32 gime_base_device::emit_mc6847_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
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
inline UINT32 gime_base_device::emit_gime_graphics_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	const UINT8 *data = &scanline->m_data[sample_start];
	mc6847_friend_device::emit_graphics<bits_per_pixel, xscale>(data, sample_count, pixels, 0, palette);
	return sample_count * (8 / bits_per_pixel) * xscale;
}



//-------------------------------------------------
//  emit_gime_text_samples
//-------------------------------------------------

template<int xscale>
inline UINT32 gime_base_device::emit_gime_text_samples(const scanline_record *scanline, int sample_start, int sample_count, pixel_t *pixels, const pixel_t *palette)
{
	UINT8 attribute = scanline->m_mode[sample_start];
	const UINT8 *data = &scanline->m_data[sample_start];

	/* determine the background/foreground colors */
	UINT8 bg = (attribute == NO_ATTRIBUTE) ? 0 : ((attribute >> 0) & 0x07) + 0x00;
	UINT8 fg = (attribute == NO_ATTRIBUTE) ? 1 : ((attribute >> 3) & 0x07) + 0x08;

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
				UINT8 font_byte = hires_font[data[i] & 0x7F][scanline->m_line_in_row];
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

template<int sample_count, gime_base_device::emit_samples_proc emit_samples>
inline void gime_base_device::render_scanline(const scanline_record *scanline, pixel_t *pixels, int min_x, int max_x, palette_resolver *resolver)
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

bool gime_base_device::update_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pixel_t *RESTRICT palette)
{
	int base_x = 64;
	int min_x = USE_HORIZONTAL_CLIP ? cliprect.min_x : 0;
	int max_x = USE_HORIZONTAL_CLIP ? cliprect.max_x : (base_x * 2 + 512 - 1);
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	palette_resolver resolver(this, palette);

	/* if the video didn't change, indicate as much */
	if (!has_video_changed())
		return UPDATE_HAS_NOT_CHANGED;

	for (int y = min_y; y <= max_y; y++)
	{
		const scanline_record *scanline = &m_scanlines[y];
		pixel_t *RESTRICT pixels = bitmap_addr(bitmap, y, 0);

		/* render the scanline */
		if (m_scanlines[y].m_line_in_row == (UINT8) ~0)
		{
			/* this is a border scanline */
			render_scanline<0, &gime_base_device::emit_dummy_samples>(scanline, pixels, min_x, max_x, &resolver);
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
					render_scanline<16, &gime_base_device::emit_mc6847_samples>(scanline, pixels, min_x, max_x, &resolver);
					break;

				default:
					render_scanline<32, &gime_base_device::emit_mc6847_samples>(scanline, pixels, min_x, max_x, &resolver);
					break;
			}
		}
		else if (scanline->m_ff98_value & 0x80)
		{
			/* GIME graphics */
			switch(scanline->m_ff99_value & 0x1F)
			{
				case 0x00:  render_scanline< 16, &gime_base_device::emit_gime_graphics_samples< 4, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x01:  render_scanline< 16, &gime_base_device::emit_gime_graphics_samples< 8, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x02:
				case 0x03:  render_scanline< 16, &gime_base_device::emit_gime_graphics_samples<16, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x04:  render_scanline< 20, &gime_base_device::emit_gime_graphics_samples< 4, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x05:  render_scanline< 20, &gime_base_device::emit_gime_graphics_samples< 8, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x06:
				case 0x07:  render_scanline< 20, &gime_base_device::emit_gime_graphics_samples<16, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x08:  render_scanline< 32, &gime_base_device::emit_gime_graphics_samples< 2, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x09:  render_scanline< 32, &gime_base_device::emit_gime_graphics_samples< 4, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0A:
				case 0x0B:  render_scanline< 32, &gime_base_device::emit_gime_graphics_samples< 8, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0C:  render_scanline< 40, &gime_base_device::emit_gime_graphics_samples< 2, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0D:  render_scanline< 40, &gime_base_device::emit_gime_graphics_samples< 4, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x0E:
				case 0x0F:  render_scanline< 40, &gime_base_device::emit_gime_graphics_samples< 8, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x10:  render_scanline< 64, &gime_base_device::emit_gime_graphics_samples< 1, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x11:  render_scanline< 64, &gime_base_device::emit_gime_graphics_samples< 2, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x12:
				case 0x13:  render_scanline< 64, &gime_base_device::emit_gime_graphics_samples< 4, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x14:  render_scanline< 80, &gime_base_device::emit_gime_graphics_samples< 1, 1> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x15:  render_scanline< 80, &gime_base_device::emit_gime_graphics_samples< 2, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x16:
				case 0x17:  render_scanline< 80, &gime_base_device::emit_gime_graphics_samples< 4, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x18:
				case 0x19:  render_scanline<128, &gime_base_device::emit_gime_graphics_samples< 1, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1A:
				case 0x1B:  render_scanline<128, &gime_base_device::emit_gime_graphics_samples< 2, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1C:
				case 0x1D:  render_scanline<160, &gime_base_device::emit_gime_graphics_samples< 1, 2> >(scanline, pixels, min_x, max_x, &resolver); break;
				case 0x1E:
				case 0x1F:  render_scanline<160, &gime_base_device::emit_gime_graphics_samples< 2, 4> >(scanline, pixels, min_x, max_x, &resolver); break;
			}
		}
		else
		{
			/* GIME text */
			switch(scanline->m_ff99_value & 0x14)
			{
				case 0x00:  render_scanline<32, &gime_base_device::emit_gime_text_samples<2> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x04:  render_scanline<40, &gime_base_device::emit_gime_text_samples<2> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x10:  render_scanline<64, &gime_base_device::emit_gime_text_samples<1> >(scanline, pixels, min_x, max_x, &resolver);  break;
				case 0x14:  render_scanline<80, &gime_base_device::emit_gime_text_samples<1> >(scanline, pixels, min_x, max_x, &resolver);  break;
			}
		}
	}
	return 0;
}



//-------------------------------------------------
//  update_composite
//-------------------------------------------------

bool gime_base_device::update_composite(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

bool gime_base_device::update_rgb(bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

inline gime_base_device::palette_resolver::palette_resolver(gime_base_device *gime, const pixel_t *palette)
{
	m_gime = gime;
	m_palette = palette;
	memset(m_resolved_palette, 0, sizeof(m_resolved_palette));
	m_current_resolved_palette = -1;
}



//-------------------------------------------------
//  palette_resolver::get_palette
//-------------------------------------------------

inline const gime_base_device::pixel_t *gime_base_device::palette_resolver::get_palette(UINT16 palette_rotation)
{
	if (UNEXPECTED(m_current_resolved_palette != palette_rotation))
	{
		for (int i = 0; i < 16; i++)
			m_resolved_palette[i] = lookup(m_gime->m_palette_rotated[palette_rotation][i]);
		m_current_resolved_palette = palette_rotation;
	}
	return m_resolved_palette;
}



//-------------------------------------------------
//  palette_resolver::lookup
//-------------------------------------------------

inline gime_base_device::pixel_t gime_base_device::palette_resolver::lookup(UINT8 color)
{
	assert(color <= 63);
	return m_palette[color];
}



//-------------------------------------------------
//  hires_font
//-------------------------------------------------

const UINT8 gime_base_device::hires_font[128][12] =
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
	{ 0x08, 0x14, 0x10, 0x38, 0x10, 0x50, 0x3C, 0x00}, { 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x7C, 0x00},
	{ 0x10, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00}, { 0x08, 0x14, 0x10, 0x38, 0x10, 0x10, 0x20, 0x40},
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



//**************************************************************************
//  VARIATIONS
//**************************************************************************

const device_type GIME_NTSC = &device_creator<gime_ntsc_device>;
const device_type GIME_PAL = &device_creator<gime_pal_device>;



//-------------------------------------------------
//  gime_ntsc_device
//-------------------------------------------------

gime_ntsc_device::gime_ntsc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: gime_base_device(mconfig, GIME_NTSC, "GIME_NTSC", tag, owner, clock, ntsc_round_fontdata8x12, "gime_ntsc", __FILE__)
{
}



//-------------------------------------------------
//  gime_pal_device
//-------------------------------------------------

gime_pal_device::gime_pal_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: gime_base_device(mconfig, GIME_PAL, "GIME_PAL", tag, owner, clock, pal_round_fontdata8x12, "gime_pal", __FILE__)
{
}
