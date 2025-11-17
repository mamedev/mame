// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/******************************************************************************

    Acorn Electron ULA

    TODO:
    - implement tape output.
    - interlaced video with 312.5 lines per field @ 50Hz.
    - improve interrupt timing.
    - generate on-screen 'snow' when contended RAM.

******************************************************************************/

/*
  From the ElectrEm site:

Timing is somewhat of a thorny issue on the Electron. It is almost certain the
Electron could have been a much faster machine if BBC Micro OS level
compatibility had not been not a design requirement.

When accessing the ROM regions, the CPU always runs at 2MHz. When accessing
the FC (1 MHz bus) or FD (JIM) pages, the CPU always runs at 1MHz.

The timing for RAM accesses varies depending on the graphics mode, and how
many bytes are required to be read by the video circuits per scanline. When
accessing RAM in modes 4-6, the CPU is simply moved to a 1MHz clock. This
occurs for any RAM access at any point during the frame.

In modes 0-3, if the CPU tries to access RAM at any time during which the
video circuits are fetching bytes, it is halted by means of receiving a
stopped clock until the video circuits next stop fetching bytes.

Each scanline is drawn in exactly 64us, and of that the video circuits fetch
bytes for 40us. In modes 0, 1 and 2, 256 scanlines have pixels on, whereas in
mode 3 only 250 scanlines are affected as mode 3 is a 'spaced' mode.

As opposed to one clock generator which changes pace, the 1MHz and 2MHz clocks
are always available, so the ULA acts to simply change which clock is piped to
the CPU. This means in half of all cases, a further 2MHz cycle is lost waiting
for the 2MHz and 1MHz clocks to synchronise during a 2MHz to 1MHz step.

The video circuits run from a constant 2MHz clock, and generate 312 scanlines
a frame, one scanline every 128 cycles. This actually gives means the Electron
is running at 50.08 frames a second.

Creating a scanline numbering scheme where the first scanline with pixels is
scanline 0, in all modes the end of display interrupt is generated at the end
of scanline 255, and the RTC interrupt is generated upon the end of scanline 99.

From investigating some code for vertical split modes printed in Electron User
volume 7, issue 7 it seems that the exact timing of the end of display interrupt
is somewhere between 24 and 40 cycles after the end of pixels. This may coincide
with HSYNC. I have no similarly accurate timing for the real time clock
interrupt at this time.

Mode changes are 'immediate', so any change in RAM access timing occurs exactly
after the write cycle of the changing instruction. Similarly palette changes
take effect immediately. VSYNC is not signalled in any way.

*/

#include "emu.h"
#include "electron_ula.h"

#include "screen.h"


#define LOG_TAPE  (1U << 1)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ELECTRON_ULA, electron_ula_device, "electron_ula", "Acorn Electron ULA")


electron_ula_device::electron_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ULA, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_rom(nullptr)
	, m_ram(nullptr)
	, m_sound_stream(nullptr)
	, m_sound_enable(false)
	, m_cas_in_cb(*this)
	, m_cas_out_cb(*this)
	, m_cas_mo_cb(*this)
	, m_irq_cb(*this)
	, m_caps_lock_cb(*this)
	, m_kbd_cb(*this, 0)
{
}


device_memory_interface::space_config_vector electron_ula_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_space_config) };
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_ula_device::device_start()
{
	m_cas_in_cb.resolve_safe(0);
	m_cas_out_cb.resolve_safe();

	m_int_status = INT_POWER_ON_RESET;
	m_int_control = 0x00;
	m_tape_timer = timer_alloc(FUNC(electron_ula_device::tape_timer_handler), this);

	m_scanline_timer = timer_alloc(FUNC(electron_ula_device::scanline_interrupt), this);
	m_scanline_timer->adjust(screen().time_until_pos(0), 0, screen().scan_period());

	// large stream buffer to favour emu/sound.cpp re-sample quality
	m_sound_stream = stream_alloc(0, 1, 48000 * 32);
	m_sound_enable = false;
	m_sound_freq = 300;
	m_sound_incr = 0;
	m_sound_signal = 1.0;

	// register save states
	save_item(NAME(m_int_status));
	save_item(NAME(m_int_control));
	save_item(NAME(m_rompage));
	save_item(NAME(m_screen_start));
	save_item(NAME(m_screen_addr));
	save_item(NAME(m_comms_mode));
	save_item(NAME(m_disp_mode));
	save_item(NAME(m_palette));
	save_item(NAME(m_cass_motor));

	save_item(NAME(m_sound_enable));
	save_item(NAME(m_sound_freq));
	save_item(NAME(m_sound_incr));
	save_item(NAME(m_sound_signal));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_ula_device::device_reset()
{
	// install RAM, ROM, and I/O
	space().install_ram(0x0000, 0x7fff, m_ram);
	space().install_read_handler(0x8000, 0xffff, emu::rw_delegate(*this, FUNC(electron_ula_device::rom_r)));
	space().install_readwrite_handler(0xfc00, 0xfeff, emu::rw_delegate(*this, FUNC(electron_ula_device::io_r)), emu::rw_delegate(*this, FUNC(electron_ula_device::io_w)));

	// install tap to adjust CPU clock
	space().install_readwrite_tap(0x0000, 0xffff, "cpu_clock_tap",
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); },
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); });

	m_comms_mode = 0x04;
	m_disp_mode = 0;
	m_cass_motor = 0;
	m_screen_start = 0x3000;
	m_screen_addr = 0x3000;
	m_tape_running = false;

	m_tape_timer->adjust(attotime::never);
}


void electron_ula_device::set_cpu_clock(offs_t offset)
{
	switch (offset & 0xc000)
	{
	case 0x0000: case 0x4000:
		// CPU slows to 1MHz when addressing RAM
		m_maincpu->set_clock_scale(0.5);

		// CPU stopped during video refresh
		waitforramsync();
		break;

	case 0x8000:
		switch (m_rompage & 0x0e)
		{
		case 8:
			// CPU slows to 1MHz when addressing keyboard
			m_maincpu->set_clock_scale(0.5);
			break;
		default:
			// CPU runs at 2MHz when addressing ROM
			m_maincpu->set_clock_scale(1.0);
			break;
		}
		break;

	case 0xc000:
		switch (offset & 0xff00)
		{
		case 0xfc00: case 0xfd00: case 0xfe00:
			// CPU slows to 1MHz when addressing IO
			//m_maincpu->set_clock_scale(0.5); // FIXME: this breaks tape loading
			break;
		default:
			// CPU runs at 2MHz when addressing ROM
			m_maincpu->set_clock_scale(1.0);
			break;
		}
		break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void electron_ula_device::sound_stream_update(sound_stream &stream)
{
	// if we're not enabled, just leave the default 0-fill
	if (!m_sound_enable || m_sound_freq == 0)
		return;

	// fill in the sample
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		m_sound_incr -= m_sound_freq;
		while (m_sound_incr < 0)
		{
			m_sound_incr += stream.sample_rate() / 2;
			m_sound_signal = -m_sound_signal;
		}

		stream.put(0, sampindex, m_sound_signal);
	}
}

//-------------------------------------------------
//  changing state to on from off will restart tone
//-------------------------------------------------

void electron_ula_device::set_sound_state(int state)
{
	// only update if new state is not the same as old state
	if (m_sound_enable == bool(state))
		return;

	m_sound_stream->update();
	m_sound_enable = bool(state);

	// restart wave from beginning
	m_sound_incr = 0;
	m_sound_signal = 1.0;
}

//-------------------------------------------------
//  setting new frequency starts from beginning
//-------------------------------------------------

void electron_ula_device::set_sound_frequency(uint32_t frequency)
{
	if (m_sound_freq == frequency)
		return;

	m_sound_stream->update();
	m_sound_freq = frequency;

	// restart wave from beginning
	m_sound_incr = 0;
	m_sound_signal = 1.0;
}


void electron_ula_device::tape_start()
{
	if (m_tape_running)
		return;

	m_tape_steps = 0;
	m_tape_value = 0x80808080;
	m_high_tone_set = 0;
	m_bit_count = 0;

	m_tape_running = true;
	m_tape_timer->adjust(attotime::zero, 0, attotime::from_hz(4800));
}

void electron_ula_device::tape_stop()
{
	m_tape_running = false;
	m_tape_timer->reset();
}


TIMER_CALLBACK_MEMBER(electron_ula_device::tape_timer_handler)
{
	if (m_cass_motor)
	{
		double tap_val = m_cas_in_cb();

		if (tap_val < -0.5)
		{
			m_tape_value = (m_tape_value << 8) | 0x00; // TAPE_LOW
			m_tape_steps++;
		}
		else if (tap_val > 0.5)
		{
			m_tape_value = (m_tape_value << 8) | 0xff; // TAPE_HIGH
			m_tape_steps++;
		}
		else
		{
			m_tape_steps = 0;
			m_bit_count = 0;
			m_high_tone_set = 0;
			m_tape_value = 0x80808080;
		}
		if (m_tape_steps > 2 && (m_tape_value == 0x0000ffff || m_tape_value == 0x00ff00ff))
		{
			m_tape_steps = 0;
			switch (m_bit_count)
			{
			case 0: // start bit
				m_start_bit = ((m_tape_value == 0x0000ffff) ? 0 : 1);
				LOGMASKED(LOG_TAPE, "++ Read start bit: %d\n", m_start_bit);
				if (m_start_bit)
				{
					if (m_high_tone_set)
					{
						m_bit_count--;
					}
				}
				else
				{
					m_high_tone_set = 0;
				}
				break;

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				LOGMASKED(LOG_TAPE, "++ Read regular bit: %d\n", m_tape_value == 0x0000ffff ? 0 : 1);
				m_tape_byte = (m_tape_byte >> 1) | (m_tape_value == 0x0000ffff ? 0 : 0x80);
				break;

			case 9: // stop bit
				m_stop_bit = ((m_tape_value == 0x0000ffff) ? 0 : 1);
				LOGMASKED(LOG_TAPE, "++ Read stop bit: %d\n", m_stop_bit);
				if (m_start_bit && m_stop_bit && m_tape_byte == 0xff && !m_high_tone_set)
				{
					m_int_status |= INT_HIGH_TONE;
					update_interrupts();
					m_high_tone_set = 1;
				}
				else if (!m_start_bit && m_stop_bit)
				{
					LOGMASKED(LOG_TAPE, "-- Byte read from tape: %02x\n", m_tape_byte);
					m_int_status |= INT_RECEIVE_FULL;
					update_interrupts();
				}
				else
				{
					LOGMASKED(LOG_TAPE, "Invalid start/stop bit combination detected: %d,%d\n", m_start_bit, m_stop_bit);
				}
				break;
			}
			m_bit_count = (m_bit_count + 1) % 10;
		}
	}
}


uint8_t electron_ula_device::rom_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0x4000)
	{
	case 0x0000:
		switch (m_rompage)
		{
		case 8: case 9:
			// Keyboard
			data = m_kbd_cb(offset) & 0x0f;
			break;
		case 10: case 11:
			// BASIC
			data = m_rom ? m_rom[offset] : 0xff;
			break;
		}
		break;

	case 0x4000:
		// MOS
		data = m_rom ? m_rom[offset] : 0xff;
		break;
	}

	return data;
}


uint8_t electron_ula_device::io_r(offs_t offset)
{
	// The Issue 4 ULA returns data from OS ROM, whereas Issue 6 ULA will return 0xfe
	uint8_t data = 0xff;

	switch (offset & 0x300)
	{
	case 0x000: // FRED
		break;
	case 0x100: // JIM
		break;
	case 0x200: // SHEILA
		switch (offset & 0x0f)
		{
		case 0x00:  // Interrupt status
			data = 0x80 | m_int_status;
			if (!machine().side_effects_disabled())
				m_int_status &= ~INT_POWER_ON_RESET;
			break;

		case 0x04:  // Cassette data shift register
			if (!machine().side_effects_disabled())
			{
				m_int_status &= ~INT_RECEIVE_FULL;
				update_interrupts();
			}
			data = m_tape_byte;
			break;
		}
		break;
	}

	return data;
}

void electron_ula_device::io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x300)
	{
	case 0x000: // FRED
		break;
	case 0x100: // JIM
		break;
	case 0x200: // SHEILA
		switch (offset & 0x0f)
		{
		case 0x00: // Interrupt control
			m_int_control = data;
			break;

		case 0x02: case 0x03: // Screen start address
			if (offset & 1)
				m_screen_start = (m_screen_start & 0x1ff) | ((data & 0x3f) << 9);
			else
				m_screen_start = (m_screen_start & 0x7e00) | ((data & 0xe0) << 1);
			break;

		case 0x04: // Cassette data shift register
			m_int_status &= ~INT_TRANSMIT_EMPTY;
			update_interrupts();
			m_tape_byte = data;
			break;

		case 0x05: // Interrupt clear and ROM paging
			if ((m_rompage & 0x0c) == 0x08)
			{
				// pages 8-11 currently selected, only allow selecting 8-15.
				if (data & 0x08)
					m_rompage = data & 0x0f;
			}
			else
			{
				// pages 0-7 or 12-15 currently selected, allow all.
				m_rompage = data & 0x0f;
			}

			if (data & 0xf0)
			{
				if (BIT(data, 4)) m_int_status &= ~INT_DISPLAY_END;
				if (BIT(data, 5)) m_int_status &= ~INT_RTC;
				if (BIT(data, 6)) m_int_status &= ~INT_HIGH_TONE;
				update_interrupts();
			}
			break;

		case 0x06: // Counter divider
			set_sound_frequency((clock() / 16) / (32 * (data + 1)));
			break;

		case 0x07: // Miscellaneous control
			m_comms_mode = (data >> 1) & 0x03;
			switch (m_comms_mode)
			{
			case 0x00: // Cassette Input
				set_sound_state(0);
				tape_start();
				break;
			case 0x01: // Sound Generation
				set_sound_state(1);
				tape_stop();
				break;
			case 0x02: // Cassette Output
				set_sound_state(0);
				tape_stop();
				// RxFull interrupt fires when tape output mode is entered. Games like Southern Belle rely on this behaviour.
				m_int_status |= INT_RECEIVE_FULL;
				update_interrupts();
				break;
			case 0x03: // not used
				set_sound_state(0);
				tape_stop();
				break;
			}
			m_disp_mode = (data >> 3) & 0x07;
			m_cass_motor = BIT(data, 6);
			m_cas_mo_cb(BIT(data, 6));
			m_caps_lock_cb(BIT(data, 7));
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Colour palette
			m_palette[offset & 0x07] = data ^ 0xff;
			update_palette(offset & 0x06);
			break;
		}
		break;
	}
}

void electron_ula_device::update_palette(offs_t offset)
{
	static const int pen_base[4] = { 0, 4, 5, 1 };
	set_pen_color(pen_base[offset >> 1] + 0,  pal1bit(BIT(m_palette[offset + 1], 0)), pal1bit(BIT(m_palette[offset + 1], 4)), pal1bit(BIT(m_palette[offset], 4)));
	set_pen_color(pen_base[offset >> 1] + 2,  pal1bit(BIT(m_palette[offset + 1], 1)), pal1bit(BIT(m_palette[offset + 1], 5)), pal1bit(BIT(m_palette[offset], 5)));
	set_pen_color(pen_base[offset >> 1] + 8,  pal1bit(BIT(m_palette[offset + 1], 2)), pal1bit(BIT(m_palette[offset], 2)),     pal1bit(BIT(m_palette[offset], 6)));
	set_pen_color(pen_base[offset >> 1] + 10, pal1bit(BIT(m_palette[offset + 1], 3)), pal1bit(BIT(m_palette[offset], 3)),     pal1bit(BIT(m_palette[offset], 7)));
}

void electron_ula_device::update_interrupts()
{
	if (m_int_status & m_int_control & ~0x83)
		m_int_status |= 0x01;
	else
		m_int_status &= ~0x01;

	m_irq_cb(BIT(m_int_status, 0) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  Video Controller
//-------------------------------------------------

void electron_ula_device::waitforramsync()
{
	int const vpos = screen().vpos();
	int cycles = 0;

	if (!(m_disp_mode & 4) && (vpos > screen().visible_area().top()) && (vpos < screen().visible_area().bottom()) && !screen().hblank())
	{
		// TODO: adjust this calculation to exclude blank lines in MODE3
		cycles += (screen().visible_area().right() - screen().hpos()) / 16;
	}
	if (cycles & 1) cycles++;

	m_maincpu->adjust_icount(-cycles);
}

inline uint8_t electron_ula_device::read_vram(uint16_t addr)
{
	if (addr & 0x8000) addr -= mode_size[m_disp_mode];
	return m_ram[addr];
}

uint32_t electron_ula_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		if ((y >= mode_dispend[m_disp_mode]) || (m_screen_ra >= 8))
		{
			// blank line
			rectangle r(cliprect.min_x, cliprect.max_x, y, y);
			bitmap.fill(rgb_t::black(), r);
		}
		else
		{
			// draw line
			int x = screen.visible_area().left();

			int const htot = (m_disp_mode & 4) ? 40 : 80;
			for (int hpos = 0; hpos < htot; hpos++)
			{
				uint8_t data = read_vram(m_screen_addr + hpos * 8);
				for (int ppb = 0; ppb < 8 / mode_bpp[m_disp_mode]; ppb++)
				{
					int const px_width = mode_bpp[m_disp_mode] * (80 / htot);
					for (int i = 0; i < px_width; i++)
					{
						switch (mode_bpp[m_disp_mode])
						{
						case 1: bitmap.pix(y, x++) = pen(bitswap<4>(data, 7, 5, 3, 1) & 0x8); break; // 2 colours
						case 2: bitmap.pix(y, x++) = pen(bitswap<4>(data, 7, 5, 3, 1) & 0xa); break; // 4 colours
						case 4: bitmap.pix(y, x++) = pen(bitswap<4>(data, 7, 5, 3, 1) & 0xf); break; // 16 colours
						}
					}
					data <<= 1;
				}
			}
		}

		// update raster/address counters
		if (m_screen_ra < 8)
			m_screen_addr++;

		m_screen_ra++;

		if (m_screen_ra == mode_max_ra[m_disp_mode])
		{
			m_screen_ra = 0;
			if (m_disp_mode & 4)
				m_screen_addr += 0x138;
			else
				m_screen_addr += 0x278;
		}

		if (m_screen_addr & 0x8000)
			m_screen_addr -= mode_size[m_disp_mode];
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(electron_ula_device::scanline_interrupt)
{
	int const vpos = screen().vpos();
	switch (vpos)
	{
	case 100:
		m_int_status |= INT_RTC;
		update_interrupts();
		break;
	case 250:
	case 256:
		if (vpos == mode_dispend[m_disp_mode])
		{
			m_int_status |= INT_DISPLAY_END;
			update_interrupts();
		}
		break;
	case 311:
		m_screen_addr = m_screen_start;
		m_screen_ra = 0;
		break;
	}
}
