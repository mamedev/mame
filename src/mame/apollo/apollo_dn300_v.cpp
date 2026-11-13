// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont, Chris Toshok, Vladimir Vukicevic
/*
 * apollo_dn300_v.cpp - the Apollo DN300/DN320 monochrome display controller
 *
 * A 1024x1024 bit image memory, 1024x800 of it shown, and a BLT engine
 * that copies or fills rectangles of it.
 *
 *  Contributors:
 *    Hans Ostermeyer
 *    Chris Toshok
 *    Vladimir Vukicevic
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf (page 12-16 ...)
 */

#include "emu.h"
#include "apollo_dn300_v.h"

#include "video.h"

#include "apollo_dn300.lh"

#define LOG_BLT      (1U << 1) // BLT operations
#define LOG_BLT_CHAR (1U << 2) // BLT operations that draw a character

#define VERBOSE (0)
#include "logmacro.h"

#define VIDEO_SCREEN_TAG "screen"

// status register
#define SR_BLT_IN_PROGRESS 0x8000
#define SR_END_OF_FRAME_IRQ 0x0080

// control register
#define CR_GO 0x8000 // start BLT operation
#define CR_IRQ_AFTER_FRAME 0x0020
#define CR_IRQ_AFTER_BLT 0x0010
#define CR_INC_Y 0x0008
#define CR_INC_X 0x0004
#define CR_FILL_MODE 0x0002
#define CR_ENABLE_DISPLAY 0x0001

void apollo_dn300_graphics::reg_w(offs_t offset, uint16_t data)
{
	int reg = offset;

	switch (reg)
	{
	case 0: // DISPLAY CONTROL
		m_cr = data;
		// CR write acknowledges BLT-done
		m_blt_irq = false;
		update_irq();
		if ((m_cr & CR_ENABLE_DISPLAY) != 0)
		{
			// update screen
			m_update_flag = 1;
		}
		else
		{
			m_update_flag = 0;
		}
		if ((m_cr & CR_GO) != 0)
		{
			blt();
		}
		break;
	case 1: // DEB
		m_deb = data;
		break;
	case 2: // WSSY
		m_wssy = data;
		break;
	case 3: // WSSX
		m_wssx = data;
		break;
	case 4: // DCY
		m_dcy = data;
		break;
	case 5: // DCX
		m_dcx = data;
		break;
	case 6: // WSDY
		m_wsdy = data;
		break;
	case 7: // WSDX
		m_wsdx = data;
		break;
	}
}

uint16_t apollo_dn300_graphics::reg_r(offs_t offset)
{
	int reg = offset;
	uint16_t value = 0;

	switch (reg)
	{
	case 0: // DISPLAY STATUS
		value = m_sr;
		break;
	case 1: // DEB
		value = m_deb;
		if (!machine().side_effects_disabled()) {
			// DISP1_INT reads +2 to ack end-of-frame
			m_eof_irq = false;
			m_sr &= ~SR_END_OF_FRAME_IRQ;
			update_irq();
		}
		break;
	case 2: // WSSY
		value = m_wssy;
		break;
	case 3: // WSSX
		value = m_wssx;
		break;
	case 4: // DCY
		value = m_dcy;
		break;
	case 5: // DCX
		value = m_dcx;
		break;
	case 6: // WSDY
		value = m_wsdy;
		break;
	case 7: // WSDX
		value = m_wsdx;
		break;
	}

	return value;
}

uint16_t apollo_dn300_graphics::mem_r(offs_t offset, uint16_t mem_mask)
{
	return m_image_memory[offset];
}

void apollo_dn300_graphics::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_image_memory[offset] &= ~mem_mask;
	m_image_memory[offset] |= data & mem_mask;
	// a CPU write to the image (the kernel's cursor blink XORs the cursor
	// in this way) is shown at the next frame, as a BLT's is
	if ((m_cr & CR_ENABLE_DISPLAY) != 0)
		m_update_flag = 1;
}

// 64 words per scan line
#define WORD_INDEX_XY(x, y) ((y)*64 + (x)/16)

namespace {

uint16_t low_bits_mask(uint32_t num_bits)
{
	return (1 << num_bits) - 1;
}

uint16_t high_bits_mask(uint32_t num_bits)
{
	return ~low_bits_mask(16 - num_bits);
}

} // anonymous namespace

void apollo_dn300_graphics::blt()
{
	m_sr |= SR_BLT_IN_PROGRESS;

	int lines = 1 + (int16_t)(~(m_dcy | 0xe000)); // only low 13 bits are used
	int words = 1 + (int16_t)(~(m_dcx | 0xffc0)); // only low  6 bits are used

	// words is the number of words involved in the blt; it is _not_ the number of bits.

	// m_deb is the bit index from the left of the last bit; so
	// to get the number of bits we don't want, we subtract it from 16
	int deb_sub_bits = 16 - (m_deb & 0xf);
	int bits_per_line =
			(16 - (m_wsdx % 16)) // number of bits from the first word
		  + ((words - 1) * 16)   // number of bits from the remaining words
		  - deb_sub_bits         // minus end word bit index
		  + 1;                   // and 1 (the bit indexes are inclusive)

	// a character from the font (7x13, held right of x=500) has its own log channel
	bool const is_rom_char = bits_per_line == 7 && lines == 13 && m_wssx > 500;

	LOGMASKED(is_rom_char ? LOG_BLT_CHAR : LOG_BLT,
			"BLT s=% 4d, % 4d  d=% 4d, % 4d sz=% 4d, % 4d (w: %d) -- "
			"CR=%04x DCX=%04x DCY=%04x DEB=%04X (%s, %s, %s, %s)\n",
			m_wssx, m_wssy,
			m_wsdx, m_wsdy,
			bits_per_line, lines, words,
			m_cr, m_dcx, m_dcy, m_deb,
			(m_cr & CR_INC_Y) ? "+Y" : "-Y",
			(m_cr & CR_INC_X) ? "+X" : "-X",
			(m_cr & CR_FILL_MODE) ? "fill" : "copy",
			(m_cr & CR_ENABLE_DISPLAY) ? "en" : "");

	bool inc_x = (m_cr & CR_INC_X) != 0;
	bool inc_y = (m_cr & CR_INC_Y) != 0;

	// Fill mode replicates a 16-bit pattern: the source address does not
	// advance, so every destination word is fed from the word at (WSSX, WSSY)
	// through the same shifter as a copy.  SR10.2's SMD_$RESET_UNIT_DISPLAY
	// stores four words of 0000 at display memory +0x1FFF0
	// (x=896, y=1023, "clear") and four of FFFF at +0x1FFF8 (x=960, "draw");
	// SMD_$CLEAR_WINDOW/HORIZ_LINE/VERT_LINE fill from WSSY=0x3FF, WSSX=0x380
	// or 0x3C0, SMD_$INVERT_DISP swaps them, and the DM keeps a stipple
	// (2222) at x=832 that it fills from WSSX=0x340/0x342.
	uint16_t const fill_word = m_image_memory[WORD_INDEX_XY(m_wssx, m_wssy)];

	// rearrange so that we always go top to bottom, left to right
	// TODO -- we need to blit in the right direction to handle overlapping regions properly.
	// we can cheat and make a copy of the source buffer if that's the case.
	if (!inc_x) {
		m_wssx -= bits_per_line;
		m_wsdx -= bits_per_line;
	}

	if (!inc_y) {
		m_wssy -= lines;
		m_wsdy -= lines;
	}

	bool clear_mode = (m_cr & CR_FILL_MODE) != 0;

	// work top to bottom, left to right
	for (int line = 0; line < lines; line++)
	{
		int source_x = m_wssx;
		int source_y = m_wssy + line;

		int dst_x = m_wsdx;
		int dst_y = m_wsdy + line;

		int num_bits_left = bits_per_line;
		while (num_bits_left > 0) {
			// assume we need to read the sx, sy word no matter what
			uint16_t source_word = clear_mode ? fill_word : m_image_memory[WORD_INDEX_XY(source_x, source_y)];
			// the number of bits we're going to copy in this iteration; cap it to
			// bits we can read from a word from source.
			uint16_t num_bits = std::min(num_bits_left, 16 - (source_x % 16));

			// align the high bit of source_word with what we're actually writing,
			// and mask off just the relevant bits
			source_word <<= (source_x % 16);
			source_word &= high_bits_mask(num_bits);

			// number of high order bits we need to skip before we start writing
			uint16_t bit_skip = dst_x % 16;

			// If all the bits we need to write fit within a single destination word, we can take a simpler path.
			// Otherwise, split the write into two parts.

			if (num_bits <= 16 - bit_skip) {
				uint16_t value = m_image_memory[WORD_INDEX_XY(dst_x, dst_y)];

				// zero out the bits we're going to write
				value &= high_bits_mask(bit_skip) | low_bits_mask(16 - bit_skip - num_bits);

				// write our bits in the appropriate place
				value |= source_word >> bit_skip;

				m_image_memory[WORD_INDEX_XY(dst_x, dst_y)] = value;
			} else {
				// we need to split the write across two words.
				uint16_t first_word_bits  = 16 - bit_skip;
				uint16_t second_word_bits = num_bits - first_word_bits;

				int dst_word  = WORD_INDEX_XY(dst_x, dst_y);

				// pull out the two dest words
				uint16_t first_word  = m_image_memory[dst_word    ] & high_bits_mask(bit_skip);
				uint16_t second_word = m_image_memory[dst_word + 1] & low_bits_mask(16 - second_word_bits);

				// now shove our bits in there
				first_word  |= source_word >> bit_skip;
				second_word |= source_word << first_word_bits;

				// and write two values
				m_image_memory[dst_word    ] = first_word;
				m_image_memory[dst_word + 1] = second_word;
			}

			source_x += num_bits;
			dst_x += num_bits;
			num_bits_left -= num_bits;
		}
	}

	m_sr &= ~SR_BLT_IN_PROGRESS;

	if ((m_cr & CR_IRQ_AFTER_BLT) != 0) {
		m_blt_irq = true;
		m_irq_cb(ASSERT_LINE);
	}

	m_update_flag = 1;
	m_update_pending = 0;

	// with the debugger active, show each BLT at once
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		machine().video().frame_update(true);
	}
}

uint32_t apollo_dn300_graphics::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int has_changed = 0;

	if (m_update_flag && !m_update_pending)
	{
		has_changed = 1;
		m_update_flag = 0;
		m_update_pending = 1;
		screen_update1(bitmap, cliprect);
		m_update_pending = 0;
	}
	return has_changed ? 0 : UPDATE_HAS_NOT_CHANGED;
}

void apollo_dn300_graphics::screen_update1(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *source_ptr = m_image_memory.get();

	for (int y = 0; y < HEIGHT; y++)
	{
		int dest = 0;
		for (int x = 0; x < WIDTH; x += 16)
		{
			uint16_t const data = *source_ptr++;
			for (uint16_t mask = 0x8000; mask; mask >>= 1)
			{
				bitmap.pix(y, dest++) = data & mask ? 0x00ffffff : 0;
			}
		}
		source_ptr += (BUFFER_WIDTH - WIDTH) / 16;
	}
}

/*-------------------------------------------------
 vblank_state_changed -
 called on each state change of the VBLANK signal
 -------------------------------------------------*/

void apollo_dn300_graphics::vblank_state_changed(screen_device &screen, bool vblank_state)
{
	// at the start of vertical blanking, the end-of-frame interrupt
	if (vblank_state) {
		// TODO: is the status bit set when the interrupt is not enabled?
		if ((m_cr & CR_IRQ_AFTER_FRAME) != 0)
		{
			m_sr |= SR_END_OF_FRAME_IRQ;
			m_eof_irq = true;
			update_irq();
		}
	}
	// SR bit 7 stays latched until the read of DEB (+2) acknowledges it:
	// the CPU may take longer than the blanking period to read it
}

/***************************************************************************
 MACHINE DRIVERS
 ***************************************************************************/

void apollo_dn300_graphics::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_apollo_dn300);
	SCREEN(config, m_screen);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK | VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(68000000, 1346, 0, 1024, 841, 0, 800);
	m_screen->set_screen_update(FUNC(apollo_dn300_graphics::screen_update));
}

apollo_dn300_graphics::apollo_dn300_graphics(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APOLLO_DN300_GRAPHICS, tag, owner, clock)
	, m_screen(*this, VIDEO_SCREEN_TAG)
	, m_irq_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_dn300_graphics::device_start()
{
	m_cr = 0;
	m_sr = 0;
	m_blt_irq = false;
	m_eof_irq = false;

	m_image_memory = std::make_unique<uint16_t[]>(IMAGE_MEMORY_SIZE);

	m_screen->register_vblank_callback(vblank_state_delegate(&apollo_dn300_graphics::vblank_state_changed, this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_dn300_graphics::device_reset()
{
	memset(m_image_memory.get(), 0, IMAGE_MEMORY_SIZE * 2);
}

DEFINE_DEVICE_TYPE(APOLLO_DN300_GRAPHICS, apollo_dn300_graphics, "apollo_dn300_graphics", "Apollo DN300 Screen")
