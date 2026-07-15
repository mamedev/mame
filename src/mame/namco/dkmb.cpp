// license:BSD-3-Clause
// copyright-holders:R. Belmont, David Haywood

/*
Donkey Kong / Donkey Kong Jr / Mario Bros
Namco / Nintendo / Cosmodog 2003

PCB LUNA REV. 1.0 (C) 2003 COSMODOG, LTD.

Maincpu: Motorola MPC603RRX266LC
XTAL: SG-8002CA @ 75 MHz (not clearly readable)
Video: Xilinx Spartan XC2S200 FPGA
Memory: 2 x 48LC2M32B2 SDR SDRAM
PIC12C508 Microcontroller

Some interesting notes from the dumper:
* Donkey Kong and Donkey Kong Jr are vertical games, while Mario Bros is originally Horizontal.
  This is achieved on this board by cropping Mario Bros slightly on the sides, while the life bar BG area is compressed slightly.
* It appears that the program ROM is responsible for programming the FPGA bitstream.
  Sound effects are supported as samples and triggers are caught by the emulator.
* The continue features are not implemented through modifications of the game, but rather through hacks applied with the emulator.
  I've had it glitch out and give me a 'Continue?' screen while I still had remaining lives in Donkey Kong.
*/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/powerpc/ppc.h"
#include "machine/intelfsh.h"
#include "sound/dmadac.h"

#include "emupal.h"
#include "endianness.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE (0)

#include "logmacro.h"

namespace {

static constexpr int FB_WIDTH = 512, FB_HEIGHT = 1024;

static constexpr int BUS_CLOCK = 75'000'000;

class dkmb_state : public driver_device
{
public:
	dkmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_flash(*this, "flash"),
		m_dmadac(*this, "dac"),
		m_inputs(*this, "IN%u", 0U),
		m_vid_status(0),
		m_sound_irq(false),
		m_sound_timer(nullptr),
		m_vid_timer(nullptr),
		m_scanbase(0),
		m_snd_dc(0)
	{ }

	void dkmb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// video registers, they're 32 bit quantities aligned on 64-bit boundaries
	enum
	{
		VID_PIXSCALE = 0x00 / 4,    // debug TTY names this "pixscale", it's always just set to 0x5656
		VID_HACTIVE  = 0x08 / 4,    // horizontal active pixels - 1 (255 = 256 across)
		VID_HFP      = 0x10 / 4,    // horizontal front porch
		VID_HSYNC    = 0x18 / 4,    // horizontal sync start
		VID_HBP      = 0x20 / 4,    // horizontal back porch
		VID_VACTIVE  = 0x28 / 4,    // vertical active lines - 1 (239 = 240 down)
		VID_VFP      = 0x30 / 4,    // vertical front porch
		VID_VSYNC    = 0x38 / 4,    // vertical sync start
		VID_VBP      = 0x40 / 4,    // vertical back porch
		VID_SCANBASE = 0x48 / 4,    // framebuffer scanout base
		VID_MODE     = 0x50 / 4,    // seems to always be 0xc0a1
		VID_STATUS   = 0x58 / 4,    // interrupt status & acknowledge
		VID_DSTX1    = 0x90 / 4,    // destination rectangle; writing X1 executes the blit
		VID_DSTY1    = 0x98 / 4,
		VID_SRCSIZEX = 0xc0 / 4,    // source size X - 1
		VID_SRCSIZEY = 0xc8 / 4,    // source size Y - 1
		VID_DSTX2    = 0xd0 / 4,
		VID_DSTY2    = 0xd8 / 4,
		VID_SRCSTEPX = 0xe0 / 4,    // source texel step per destination X and per Y, <<9
		VID_SRCSTEPY = 0xe8 / 4,
		VID_SRCADDR  = 0xf0 / 4     // source texel address; byte address = (v >> 9) * 4
	};

	required_device<ppc603r_device> m_maincpu;
	required_shared_ptr<uint64_t> m_mainram;
	required_device<amd_29lv160dt_device> m_flash;
	required_device<dmadac_sound_device> m_dmadac;
	required_ioport_array<4> m_inputs;

	uint8_t m_vid_status;
	uint32_t m_vid_regs[0x40];
	bool m_sound_irq;
	emu_timer *m_sound_timer;
	emu_timer *m_vid_timer;
	std::string m_console_line;
	uint32_t m_scanbase;
	int32_t m_snd_dc;

	u64 inputs_r(offs_t offset);
	u64 debug_status_r();
	u32 ram32_r(offs_t addr) const;
	void ram32_w(offs_t addr, u32 data);
	void vblank_w(int state);
	void update_irq();
	TIMER_CALLBACK_MEMBER(sound_tick);
	TIMER_CALLBACK_MEMBER(vid_tick);
	u64 irqcause_r();
	void console_tx_w(offs_t offset, u64 data, u64 mem_mask);
	void sound_fifo_w(offs_t offset, u64 data, u64 mem_mask);
	u32 vidreg_r(offs_t offset);
	void vidreg_w(offs_t offset, u32 data, u32 mem_mask);
	u64 flash_status_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


void dkmb_state::machine_start()
{
	// The sound mixer code on the 603 produces one sample per 423 (0x1a7) PowerPC time base
	// ticks (see the mftb-driven sample count at 0xc954).  The time base runs at bus/4 =
	// 18.75 MHz, so the sample rate is 18.75 MHz / 423 = 44326 Hz.
	static constexpr int TB_HZ = BUS_CLOCK / 4;
	static constexpr int TB_PER_SAMPLE = 423;

	m_sound_timer = timer_alloc(FUNC(dkmb_state::sound_tick), this);
	m_sound_timer->adjust(attotime::from_ticks(128 * TB_PER_SAMPLE, TB_HZ), 0, attotime::from_ticks(128 * TB_PER_SAMPLE, TB_HZ));

	m_dmadac->set_frequency(double(TB_HZ) / TB_PER_SAMPLE);
	m_dmadac->enable(1);

	// blitter completion timer
	m_vid_timer = timer_alloc(FUNC(dkmb_state::vid_tick), this);

	save_item(NAME(m_vid_status));
	save_item(NAME(m_vid_regs));
	save_item(NAME(m_sound_irq));
	save_item(NAME(m_scanbase));
	save_item(NAME(m_snd_dc));
}


void dkmb_state::machine_reset()
{
	// indicate the blitter is not busy; if it is the startup code will hang waiting for it to be not busy
	m_vid_status |= 0x20;
	update_irq();
}


void dkmb_state::update_irq()
{
	m_maincpu->set_input_line(PPC_IRQ, (m_vid_status || m_sound_irq) ? ASSERT_LINE : CLEAR_LINE);
}

// timer that fires when it's time to push 128 audio samples to the DMADAC
TIMER_CALLBACK_MEMBER(dkmb_state::sound_tick)
{
	m_sound_irq = true;
	update_irq();
}

// blitter completion timer
TIMER_CALLBACK_MEMBER(dkmb_state::vid_tick)
{
	m_vid_status |= 0x20;
	update_irq();
}

// helpers for accessing the main RAM share
u32 dkmb_state::ram32_r(offs_t addr) const
{
	return util::big_endian_cast<u32 const>(&m_mainram[0])[(addr & 0xfffffc) >> 2];
}

void dkmb_state::ram32_w(offs_t addr, u32 data)
{
	util::big_endian_cast<u32>(&m_mainram[0])[(addr & 0xfffffc) >> 2] = data;
}

void dkmb_state::sound_fifo_w(offs_t offset, u64 data, u64 mem_mask)
{
	if (offset == 0 && ACCESSING_BITS_48_63)
	{
		// Cosmodog's mixer firmware introduces a DC offset as part of the volume scaling, so to keep
		// things clean we do a very simple DC blocking filter here.
		s32 const in = s32(u16(data >> 48)) - 0x8000;
		m_snd_dc += ((in << 8) - m_snd_dc) >> 9;
		int16_t sample = int16_t(std::clamp(in - (m_snd_dc >> 8), -32768, 32767));
		m_dmadac->transfer(0, 1, 1, 1, &sample);
	}
	m_sound_irq = false;
	update_irq();
}

void dkmb_state::vblank_w(int state)
{
	if (state)
	{
		// latch where to scan out the just-completed frame
		m_scanbase = m_vid_regs[VID_SCANBASE] & 0xfffffc;
		// firing this IRQ will flip VID_SCANBASE and start emulating and rendering the next frame
		m_vid_status |= 0x80;
		update_irq();
	}
}

u64 dkmb_state::irqcause_r()
{
	u32 cause = 0;
	if (m_vid_status)
	{
		cause |= 0x00010000;
	}
	if (m_sound_irq)
	{
		cause |= 0x00000100;
	}
	return u64(cause) << 32;
}

u32 dkmb_state::vidreg_r(offs_t offset)
{
	if (offset == VID_STATUS)
	{
		return u32(m_vid_status) << 24;
	}
	return m_vid_regs[offset];
}

void dkmb_state::vidreg_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == VID_STATUS)
	{
		// writing a status bit back clears that interrupt
		if (ACCESSING_BITS_24_31)
		{
			m_vid_status &= ~u8(data >> 24);
			update_irq();
		}
		return;
	}
	COMBINE_DATA(&m_vid_regs[offset]);
	// A blit command executes when the destination X1 is written (the last coordinate).  It
	// copies an ARGB source image from main RAM to the destination rectangle, whose
	// coordinates are <<9.  Software is responsible for clearing the framebuffer, of course.
	if (offset == VID_DSTX1)
	{
		const u32 base = (m_vid_regs[VID_SRCADDR] >> 9) * 4;
		const int64_t lx = std::min(int32_t(m_vid_regs[VID_DSTX1]), int32_t(m_vid_regs[VID_DSTX2]));
		const int64_t ty = std::min(int32_t(m_vid_regs[VID_DSTY1]), int32_t(m_vid_regs[VID_DSTY2]));
		const int x0 = int(lx + 511) >> 9, x1 = std::max(int32_t(m_vid_regs[VID_DSTX1]), int32_t(m_vid_regs[VID_DSTX2])) >> 9;
		const int y0 = int(ty + 511) >> 9, y1 = std::max(int32_t(m_vid_regs[VID_DSTY1]), int32_t(m_vid_regs[VID_DSTY2])) >> 9;
		const int64_t xstep = int32_t(m_vid_regs[VID_SRCSTEPX]), ystep = int32_t(m_vid_regs[VID_SRCSTEPY]);

		for (int y = std::max(y0, 0); y <= std::min(y1, FB_HEIGHT - 1); y++)
		{
			const int64_t yoff = (((int64_t(y) << 9) - ty) * ystep) >> 18;

			for (int x = std::max(x0, 0); x <= std::min(x1, FB_WIDTH - 1); x++)
			{
				const int64_t xoff = (((int64_t(x) << 9) - lx) * xstep) >> 18;
				const u32 texel = ram32_r(base + u32(xoff + yoff) * 4);

				// bit 31 set means opaque
				if (BIT(texel, 31))
				{
					ram32_w((x << 12) | (y << 2), texel);
				}
			}
		}

		// Schedule the blit completion IRQ, assuming a 150 MPixel/second rate
		// (75 MHz bus clock, 64 bit wide data bus, 32 bits per ARGB pixel)
		const u32 npix = u32(std::max(x1 - x0 + 1, 1)) * u32(std::max(y1 - y0 + 1, 1));
		m_vid_timer->adjust(attotime::from_ticks(std::max(npix, 64U), BUS_CLOCK * 2));
	}
}

uint32_t dkmb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int r = cliprect.top(); r <= cliprect.bottom(); r++)
	{
		for (int c = cliprect.left(); c <= cliprect.right(); c++)
		{
			bitmap.pix(r, c) = ram32_r(m_scanbase + (r << 12) + (c << 2)) & 0xffffff;
		}
	}
	return 0;
}

u64 dkmb_state::inputs_r(offs_t offset)
{
	return u64(m_inputs[offset & 3]->read()) << 56;
}

u64 dkmb_state::flash_status_r()
{
	return u64(m_flash->is_ready() ? 0x80 : 0x00) << 56;
}

u64 dkmb_state::debug_status_r()
{
	// Debug UART status: bit 2 (0x04) = transmitter ready.  Suspiciously i8251-flavored...
	return u64(0x04) << 56;
}

void dkmb_state::console_tx_w(offs_t offset, u64 data, u64 mem_mask)
{
	if (!ACCESSING_BITS_56_63)
	{
		return;
	}

	s8 const c = s8((data >> 56) & 0xff);
	if (c == '\r')
	{
		return;
	}

	if (c == '\n')
	{
		LOG("CONSOLE: %s\n", m_console_line);
		m_console_line.clear();
	}
	else if (c)
	{
		m_console_line += c;
	}
}

void dkmb_state::main_map(address_map &map)
{
	// 16MB SDRAM (2x 48LC2M32B2), one linear block: code is copied to and run from the
	// bottom, 0x100000-0x1fffff holds the FPGA framebuffer/compose region, heap/data above.
	// One share so the blitter and scan-out can access it directly (the FPGA works on
	// physical RAM, not through the 603's MMU).
	map(0x00000000, 0x00ffffff).ram().share(m_mainram);

	map(0x02080000, 0x02080007).ram(); // write
	map(0x02060000, 0x0206001f).r(FUNC(dkmb_state::inputs_r)); // input ports (bytes at 0x00/08/10/18, active low)
	map(0x02060020, 0x0206003f).ram(); // control/output writes (coin counters, watchdog, etc.)
	map(0x020c0000, 0x020c0007).w(FUNC(dkmb_state::console_tx_w)); // debug UART transmit data
	map(0x020c0010, 0x020c0017).r(FUNC(dkmb_state::debug_status_r)); // debug UART status (bit 2 = TX ready)

	map(0x020b0000, 0x020bffff).w(FUNC(dkmb_state::sound_fifo_w)); // sound sample FIFO (write acks the sound interrupt)

	map(0x020d0000, 0x020d0007).r(FUNC(dkmb_state::irqcause_r)); // top-level interrupt cause

	// FPGA video controller / blit-engine registers; 0x58 is the frame interrupt status/ack (handled inside vidreg)
	map(0x020f0000, 0x020f00ff).rw(FUNC(dkmb_state::vidreg_r), FUNC(dkmb_state::vidreg_w));

	map(0x04090000, 0x0409ffff).ram(); // size uncertain

	// flash RY/BY# (ready/busy) status, bit 7 = ready: the firmware's program/erase
	// routines poll this after issuing a command (blinking the service LED on port
	// 0x02060028 and patting the watchdog at 0x02080000 while they wait)
	map(0x02090000, 0x02090007).r(FUNC(dkmb_state::flash_status_r));

	// 0xff000000-0xff7fffff is the development "promulator" (ROM emulator) slot: leave it
	// unmapped so the firmware's probe there fails and it takes the production
	// "booting from FLASH..." path.  The Am29LV160 flash (2MB, byte mode) sits at
	// 0xff800000 and mirrors up through the PowerPC reset vector at 0xfff00100.
	map(0xff800000, 0xff9fffff).mirror(0x00600000).rw(m_flash, FUNC(amd_29lv160dt_device::read), FUNC(amd_29lv160dt_device::write));
}

static INPUT_PORTS_START( dkmb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_NAME("P1 Jump")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_NAME("P2 Jump") PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void dkmb_state::dkmb(machine_config &config)
{
	// The part is rated for 266 MHz but could be strapped for any 0.5 step multiplier up to 3.5
	// (1, 1.5, 2, 2.5, 3, 3.5).
	PPC603R(config, m_maincpu, BUS_CLOCK * 7 / 2);
	m_maincpu->set_bus_frequency(BUS_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dkmb_state::main_map);

	PIC16C56(config, "pic", 4'000'000);  // Actually PIC12C508, clock not verified

	AMD_29LV160DT(config, m_flash);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(dkmb_state::screen_update));
	screen.set_size(256, 240);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(dkmb_state::vblank_w));

	PALETTE(config, "palette").set_entries(65536);

	SPEAKER(config, "speaker", 2).front();
	DMADAC(config, m_dmadac).add_route(ALL_OUTPUTS, "speaker", 1.0, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}


ROM_START( dkmb )
	ROM_REGION(0x200000, "flash", 0) // AMD Am29LV160 (or compatible) flash, byte mode; also holds the operator settings/audits
	ROM_LOAD( "donkeykong.u14", 0x000000, 0x200000, CRC(6cb9adc9) SHA1(4f59ac0d6448380f1a18ffacb83f68583a5c0840) )

	ROM_REGION( 0x1000, "pic", 0 )
	/*
	    Intel HEX format dump.  When converted to binary using
	    "srec_cat.exe 12c508.u12 -intel -o 12c508.bin -binary"
	    this contains only the string
	    "Copyright 2001 Core Technologies why are you looking in here?"
	    in the first 0x80 bytes, which is the unprotected area of the PIC.
	    the rest is blank, therefore protected and a bad dump
	*/
	ROM_LOAD("12c508.u12", 0x000, 0x09db, BAD_DUMP CRC(3adb3e33) SHA1(36a96886d83b64633eea83e57bdfa8a20c6d4f6a) )
ROM_END

} // anonymous namespace


GAME( 2003, dkmb, 0, dkmb, dkmb, dkmb_state, empty_init, ROT270, "Namco / Nintendo / Cosmodog", "Donkey Kong / Donkey Kong Jr / Mario Bros", MACHINE_SUPPORTS_SAVE )
