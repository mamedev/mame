// license:BSD-3-Clause
// copyright-holders:

/*
Dyna D0703 PCB

This PCB is a slight evolution of the one in dyna/dyna_d0404.cpp.
It's called DYNA System2007(D0703) V1.05.01 in ROM.

The main components are:
208-pin custom (sanded off). MCU/CPU?
2x Lattice (unreadable model)
1x AMIC A67L93361E-7.5F SRAM
2x IDT71024 SRAM
2x A625308AM-70S SRAM
25.2000 MHz XTAL
Epson RTC62421

Notes (all deduced from the code, no documentation available):
- the 208-pin chip is the same ARM7 SoC as on the D0404 PCB (interrupt controller at
  0xffffa800, chip select setup at 0xffffa000, two UART-like devices at 0xffff0000 and
  0xffff0400, ports at 0xffff1c00, 2KB of internal RAM at 0x60000000). The BIOS checks that
  0xffffa40c reads 0x2100 (chip ID?).
- the program ROM is mapped at 0x000000 (BIOS) and mirrored at 0x080000 (game, header at
  0x090000). The h11 flash is mapped at 0x200000 and contains PCM samples, the key checked
  by the BIOS at 0x204004 and settings.
- the custom video / I/O chip (Lattice) at 0x150000 returns the ID string
  "DYNA System2007    D0703SC V1.11" at 0x150080-0x15009f, which the BIOS checks.
- the video chip is a blitter drawing RLE-compressed images from the GFX ROM into two
  framebuffers. Compared to the D0404 the RLE format is different and 8bpp images are
  supported. Counts are in groups of 4 pixels:
  0nnnnnnnnnnnnnnn: n+1 groups of literal pixels (4bpp: 1 word each, 8bpp: 2 words each)
  101111bbbbnnnnnn: n+1 words of 4bpp literal pixels, ~b is the high nibble of the pixels
  11ccccccccnnnnnn: n+1 groups filled with color ~c
- the palette is in RAM at 0x150400, written by the BIOS during vblank. The format is
  GGGGGRRRRRBBBBBx, as on the D0404 (the Triple 7 Bar "ALL RED" bonus is green otherwise).
  8bpp images use colors 0x000-0x0ff, 4bpp images 16 color banks from 0x100 (the BIOS
  screens only set up colors 0x100-0x19f).
- 0x150040 / 0x150044 are written with two words kept in the backup RAM, the results read
  back from 0x150048 / 0x15004c are stored back and the low 16 bits are returned to the
  game by SWI 0x21, which uses them as random numbers. The actual function is unknown.
- sound is software mixed 8-bit PCM (samples in the h11 flash), the BIOS writes 11 16-bit
  samples to 0x15003c at every IRQ 2, so it's probably a FIFO.

TODO:
- identify SoC
- blitter: what the low 4 bits of the word 3 color do (set to 1, 4 or 7 for some 8bpp images,
  e.g. the menu items, headers and paylines, but adding them to the pixels gives wrong colors)
- palette size, what's at 0x150038, 0x150030, 0x150034
- the random number generator at 0x150040-0x15004c
- sound FIFO size, sample rate
- IRQs 9, 10 and 11
- outputs, what DSW1 does (bit 0 is read by the BIOS and goes to the display control)
- correct video timings
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define LOG_BLIT    (1U << 1)
#define LOG_SOC     (1U << 2)
#define LOG_VIDEO   (1U << 3)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGBLIT(...)    LOGMASKED(LOG_BLIT,  __VA_ARGS__)
#define LOGSOC(...)     LOGMASKED(LOG_SOC,   __VA_ARGS__)
#define LOGVIDEO(...)   LOGMASKED(LOG_VIDEO, __VA_ARGS__)


namespace {

class dyna_d0703_state : public driver_device
{
public:
	dyna_d0703_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "flash"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_soc_regs(*this, "soc_regs"),
		m_gfxrom(*this, "gfx")
	{ }

	void dyna_d0703(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr uint32_t FB_WIDTH = 1024;
	static constexpr uint32_t FB_HEIGHT = 512;
	static constexpr uint8_t FIFO_SIZE = 64;

	required_device<arm7_cpu_device> m_maincpu;
	required_device<intelfsh16_device> m_flash;
	required_device<palette_device> m_palette;
	required_device<dac_16bit_r2r_twos_complement_device> m_dac;

	required_shared_ptr<uint32_t> m_soc_regs;
	required_region_ptr<uint16_t> m_gfxrom;

	std::unique_ptr<uint16_t[]> m_fb[2];
	uint32_t m_blit_regs[4] = { };
	uint32_t m_disp_ctrl = 0;
	uint32_t m_irq_pending = 0;
	uint32_t m_irq_mask = 0;
	uint32_t m_rng[2] = { };
	uint16_t m_sound_fifo[FIFO_SIZE] = { };
	uint8_t m_sound_fifo_rd = 0;
	uint8_t m_sound_fifo_count = 0;
	emu_timer *m_sound_timer = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void set_irq(uint8_t bit, int state);
	void update_irq();
	uint32_t intc_r(offs_t offset, uint32_t mem_mask);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	offs_t access_address(offs_t aligned);
	uint32_t flash_r(offs_t offset, uint32_t mem_mask);
	void flash_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t soc_r(offs_t offset, uint32_t mem_mask);
	void soc_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void do_blit();
	uint8_t id_r(offs_t offset);
	uint32_t rng_r(offs_t offset);
	void sound_fifo_w(uint16_t data);
	TIMER_CALLBACK_MEMBER(sound_tick);

	void program_map(address_map &map) ATTR_COLD;
};


void dyna_d0703_state::video_start()
{
	for (auto &fb : m_fb)
		fb = make_unique_clear<uint16_t[]>(FB_WIDTH * FB_HEIGHT);

	save_pointer(NAME(m_fb[0]), FB_WIDTH * FB_HEIGHT);
	save_pointer(NAME(m_fb[1]), FB_WIDTH * FB_HEIGHT);
}

uint32_t dyna_d0703_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const fb = m_fb[BIT(m_disp_ctrl, 1)].get();
	pen_t const *const pens = m_palette->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const src = &fb[(y & (FB_HEIGHT - 1)) * FB_WIDTH];
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			dst[x] = pens[src[x & (FB_WIDTH - 1)] & 0x1ff];
	}

	return 0;
}

void dyna_d0703_state::vblank_w(int state)
{
	if (state)
		set_irq(1, 1);
}


/*
 Blitter registers (written in order, the write to word 3 starts the operation):
 word 0: x--- ---- ---- ---- ---- ---- ---- ----  end of list marker (only used by the BIOS display list)
         --x- ---- ---- ---- ---- ---- ---- ----  fill with the color in word 3 instead of drawing an image (always opaque)
         ---x ---- ---- ---- ---- ---- ---- ----  opaque (pen 0 is drawn too)
         ---- -x-- ---- ---- ---- ---- ---- ----  8bpp image
         ---- ---- xxxx xxxx xxxx xxxx xxxx xxxx  source address in the GFX ROM (in 16-bit words)
 word 1: xxxx xxxx xxxx xxxx ---- ---- ---- ----  width - 1
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  height - 1
 word 2: xxxx xxxx xxxx xxxx ---- ---- ---- ----  destination x
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  destination y
 word 3: xxxx xxxx ---- ---- ---- ---- ---- ----  unknown (always 0 so far, ignored when drawing images)
         ---- ---- xxxx ---- ---- ---- ---- ----  palette bank for 4bpp images (16 colors each, from 0x100)
         ---- ---- ---- xxxx ---- ---- ---- ----  unknown (0x1, 0x4 or 0x7 for 8bpp images, 0xf for the BIOS text)
         xxxx xxxx xxxx xxxx ---- ---- ---- ----  fill color when filling (only the low 9 bits are used)
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  unknown (always 0xffff in the static tables)
*/
void dyna_d0703_state::do_blit()
{
	uint32_t addr = m_blit_regs[0] & 0x00ffffff;
	bool const fill = BIT(m_blit_regs[0], 29);
	bool const opaque = BIT(m_blit_regs[0], 28);
	bool const bpp8 = BIT(m_blit_regs[0], 26);
	uint32_t const width = (m_blit_regs[1] >> 16) + 1;
	uint32_t const height = (m_blit_regs[1] & 0xffff) + 1;
	int const dst_x = int16_t(m_blit_regs[2] >> 16);
	int const dst_y = int16_t(m_blit_regs[2] & 0xffff);
	uint16_t const color = m_blit_regs[3] >> 16;
	uint16_t *const fb = m_fb[BIT(m_disp_ctrl, 1) ^ 1].get(); // draws to the page not being displayed
	// rows past the bottom of the framebuffer aren't decoded, nothing is done if the rectangle is completely outside it
	bool const visible = (dst_x < int(FB_WIDTH)) && ((dst_x + int(width)) > 0) && (dst_y < int(FB_HEIGHT)) && ((dst_y + int(height)) > 0);
	uint32_t const rows = visible ? std::min<uint32_t>(height, FB_HEIGHT - dst_y) : 0;
	uint32_t const total = width * rows;

	LOGBLIT("%s: blit %08x %08x %08x %08x\n", machine().describe_context(), m_blit_regs[0], m_blit_regs[1], m_blit_regs[2], m_blit_regs[3]);

	if ((m_blit_regs[0] & 0x4b000000) || ((m_blit_regs[3] & 0xffff) != 0xffff))
		logerror("%s: blit with unknown bits %08x %08x %08x %08x\n", machine().describe_context(), m_blit_regs[0], m_blit_regs[1], m_blit_regs[2], m_blit_regs[3]);

	uint32_t count = 0;
	auto const plot =
			[&count, width, dst_x, dst_y, opaque, fill, bpp8, color, fb] (uint8_t pix)
			{
				int const x = dst_x + (count % width);
				int const y = dst_y + (count / width);
				count++;
				if ((pix || opaque || fill) && x >= 0 && x < FB_WIDTH && y >= 0 && y < FB_HEIGHT)
					fb[y * FB_WIDTH + x] = fill ? (color & 0x1ff) : bpp8 ? pix : ((0x100 + (color & 0xf0) + pix) & 0x1ff);
			};

	if (fill)
	{
		while (count < total)
			plot(0);
	}
	else
	{
		while (count < total)
		{
			uint16_t const cmd = m_gfxrom[addr++ & 0x7fffff];
			uint8_t const len = (cmd & 0x3f) + 1;

			if (!BIT(cmd, 15))
			{
				// literal groups
				uint32_t const groups = (cmd & 0x7fff) + 1;
				for (uint32_t i = 0; (i < groups) && (count < total); i++)
				{
					for (uint8_t w = 0; w < (bpp8 ? 2 : 1); w++)
					{
						uint16_t const data = m_gfxrom[addr++ & 0x7fffff];
						if (bpp8)
						{
							plot(data >> 8);
							plot(data & 0xff);
						}
						else
						{
							for (int p = 12; p >= 0; p -= 4)
								plot((data >> p) & 0x0f);
						}
					}
				}
			}
			else if (!BIT(cmd, 14))
			{
				// literal 4bpp words, the high nibble of the pixel comes from the command
				uint8_t const base = ((~cmd >> 6) & 0x0f) << 4;
				for (uint32_t i = 0; (i < len) && (count < total); i++)
				{
					uint16_t const data = m_gfxrom[addr++ & 0x7fffff];
					for (int p = 12; p >= 0; p -= 4)
						plot(base | ((data >> p) & 0x0f));
				}
			}
			else
			{
				// run
				uint8_t const pen = (~cmd >> 6) & (bpp8 ? 0xff : 0x0f);
				for (uint32_t i = 0; (i < len * 4) && (count < total); i++)
					plot(pen);
			}
		}
	}

	// TODO: timing
	set_irq(0, 1);
}

void dyna_d0703_state::blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_blit_regs[offset]);

	if (offset == 3)
		do_blit();
}

uint8_t dyna_d0703_state::id_r(offs_t offset)
{
	static constexpr char ID[] = "DYNA System2007    D0703SC V1.11";
	return ID[offset & 0x1f];
}

uint32_t dyna_d0703_state::rng_r(offs_t offset)
{
	// TODO: the actual function is unknown, the game only uses the results as random numbers
	// here it's a xorshift64 on the two words, stepped when the first one is read
	if (!offset && !machine().side_effects_disabled())
	{
		uint64_t state = (uint64_t(m_rng[0]) << 32) | m_rng[1];
		if (!state)
			state = 0x9e37'79b9'7f4a'7c15ULL;
		state ^= state << 13;
		state ^= state >> 7;
		state ^= state << 17;
		m_rng[0] = state >> 32;
		m_rng[1] = uint32_t(state);
	}

	return m_rng[offset];
}

void dyna_d0703_state::sound_fifo_w(uint16_t data)
{
	if (m_sound_fifo_count < FIFO_SIZE)
	{
		m_sound_fifo[(m_sound_fifo_rd + m_sound_fifo_count) % FIFO_SIZE] = data;
		m_sound_fifo_count++;
	}
	else
		logerror("%s: sound FIFO overflow\n", machine().describe_context());
}

TIMER_CALLBACK_MEMBER(dyna_d0703_state::sound_tick)
{
	if (m_sound_fifo_count)
	{
		m_dac->write(m_sound_fifo[m_sound_fifo_rd]);
		m_sound_fifo_rd = (m_sound_fifo_rd + 1) % FIFO_SIZE;
		m_sound_fifo_count--;
	}

	if (m_sound_fifo_count <= FIFO_SIZE / 4) // TODO: actual threshold
		set_irq(2, 1);
}


// interrupt controller
void dyna_d0703_state::update_irq()
{
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, (m_irq_pending & m_irq_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void dyna_d0703_state::set_irq(uint8_t bit, int state)
{
	// only rising edges are latched (TODO: verify)
	if (state)
	{
		m_irq_pending |= 1U << bit;
		update_irq();
	}
}

uint32_t dyna_d0703_state::intc_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x0c / 4: return m_irq_mask;
		case 0x14 / 4: return m_irq_pending & m_irq_mask;
		case 0x1c / 4: return m_irq_pending;
		default:
			if (!machine().side_effects_disabled())
				LOGSOC("%s: intc_r %02x & %08x\n", machine().describe_context(), offset * 4, mem_mask);
			return 0;
	}
}

void dyna_d0703_state::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x08 / 4: m_irq_pending &= ~(data & mem_mask); update_irq(); break;
		case 0x0c / 4: COMBINE_DATA(&m_irq_mask); update_irq(); break;
		default: LOGSOC("%s: intc_w %02x = %08x & %08x\n", machine().describe_context(), offset * 4, data, mem_mask); break;
	}
}

// unknown SoC peripherals, just latch what's written
uint32_t dyna_d0703_state::soc_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGSOC("%s: soc_r %08x & %08x\n", machine().describe_context(), 0xffff0000 + offset * 4, mem_mask);

	if (offset == 0xa40c / 4)
		return 0x2100; // checked by the BIOS, chip ID?

	return m_soc_regs[offset];
}

void dyna_d0703_state::soc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGSOC("%s: soc_w %08x = %08x & %08x\n", machine().describe_context(), 0xffff0000 + offset * 4, data, mem_mask);

	COMBINE_DATA(&m_soc_regs[offset]);
}


/*
 The BIOS accesses the 16-bit flash with LDR / STR instructions to halfword aligned addresses
 (including the 0x555 / 0x2aa command addresses) and the SoC apparently turns them into a single
 halfword access to the actual address (the low 16 bits are written, the halfword read is in the
 low 16 bits after the LDR rotation). While doing so it sets bit 8 of the chip select 2
 configuration (0xffffa048), which is clear otherwise, so word reads are only handled this way
 while it's set: the samples are read normally, and the erase / program status polling needs
 single reads, otherwise the toggle bit is read twice and never seen toggling.
 MAME's ARM7 core always word aligns the address of LDR / STR and nothing else in MAME passes A1
 on, so the halfword can't be determined from the access alone. As a workaround (hack) the actual
 address is recalculated from the instruction being executed, which is still at R15 while the
 access is performed.
*/
offs_t dyna_d0703_state::access_address(offs_t aligned)
{
	// only the LDR / STR forms used by the BIOS are handled: [Rn, #+imm], [Rn, +Rm] and [Rn], #imm, with Rn and Rm in R0-R7
	// (R15 would need the pipeline offset, and state_int() returns the user bank for R8-R14)
	uint32_t const insn = m_maincpu->space(AS_PROGRAM).read_dword(m_maincpu->state_int(arm7_cpu_device::ARM7_R15));
	auto const reg = [this] (uint8_t r) { return uint32_t(m_maincpu->state_int(arm7_cpu_device::ARM7_R0 + r)); };
	uint8_t const rn = (insn >> 16) & 0x0f;

	uint8_t const rm = insn & 0x0f;

	if (rn < 8)
	{
		if ((insn & 0x0fe00000) == 0x05800000)
			return reg(rn) + (insn & 0xfff);
		else if (((insn & 0x0fe00ff0) == 0x07800000) && (rm < 8))
			return reg(rn) + reg(rm);
		else if ((insn & 0x0fe00000) == 0x04800000)
			return reg(rn); // post-indexed, the base register hasn't been updated yet
	}

	logerror("%s: unhandled flash access, insn %08x\n", machine().describe_context(), insn);
	return aligned;
}

uint32_t dyna_d0703_state::flash_r(offs_t offset, uint32_t mem_mask)
{
	if ((mem_mask == 0xffffffff) && BIT(m_soc_regs[0xa048 / 4], 8) && !machine().side_effects_disabled())
	{
		uint16_t const data = m_flash->read((access_address(0x200000 | (offset << 2)) & 0xfffff) >> 1);
		return (uint32_t(data) << 16) | data;
	}

	uint32_t data = 0;
	if (ACCESSING_BITS_0_15)
		data |= m_flash->read(offset << 1);
	if (ACCESSING_BITS_16_31)
		data |= uint32_t(m_flash->read((offset << 1) | 1)) << 16;
	return data;
}

void dyna_d0703_state::flash_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0xffffffff)
	{
		m_flash->write((access_address(0x200000 | (offset << 2)) & 0xfffff) >> 1, data & 0xffff);
	}
	else
	{
		if (ACCESSING_BITS_0_15)
			m_flash->write(offset << 1, data & 0xffff);
		if (ACCESSING_BITS_16_31)
			m_flash->write((offset << 1) | 1, data >> 16);
	}
}


void dyna_d0703_state::program_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).mirror(0x00080000).rom().region("maincpu", 0);
	map(0x00100000, 0x0013ffff).ram();
	map(0x00140000, 0x0014ffff).ram().share("nvram"); // backup data is kept in triplicate

	map(0x00150000, 0x00150003).lrw32(
			NAME([this] () -> uint32_t { return m_disp_ctrl; }),
			NAME([this] (offs_t offset, uint32_t data, uint32_t mem_mask)
			{
				COMBINE_DATA(&m_disp_ctrl);
				LOGVIDEO("%s: disp_ctrl = %08x\n", machine().describe_context(), m_disp_ctrl);
			}));
	map(0x00150004, 0x00150007).lr32(NAME([] () -> uint32_t { return 0x80000000; })); // bit 31: blitter idle, bit 16 checked when waiting for vblank
	map(0x00150010, 0x0015001f).w(FUNC(dyna_d0703_state::blitter_w));
	map(0x00150020, 0x00150023).portr("IN0");
	map(0x00150024, 0x00150027).portr("IN1");
	map(0x00150030, 0x0015003b).lw32(NAME([this] (offs_t offset, uint32_t data) { LOG("%s: outputs %02x = %08x\n", machine().describe_context(), 0x30 + offset * 4, data); })); // TODO: lamps, counters?
	map(0x0015003c, 0x0015003f).w(FUNC(dyna_d0703_state::sound_fifo_w)).umask32(0x0000ffff); // always written with 32-bit stores
	map(0x00150040, 0x00150047).lw32(NAME([this] (offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_rng[offset]); }));
	map(0x00150048, 0x0015004f).r(FUNC(dyna_d0703_state::rng_r));
	map(0x00150080, 0x0015009f).r(FUNC(dyna_d0703_state::id_r));
	map(0x001500c0, 0x001500ff).rw("rtc", FUNC(rtc62421_device::read), FUNC(rtc62421_device::write)).umask32(0x000000ff);
	map(0x00150400, 0x001507ff).w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x00151958, 0x0015195b).lw32(NAME([this] (uint32_t data) { LOG("%s: 0x151958 = %08x\n", machine().describe_context(), data); })); // written with 0x728, 0x531, 0xe6a7, 0x1958 at boot, unlock sequence?

	map(0x00200000, 0x002fffff).rw(FUNC(dyna_d0703_state::flash_r), FUNC(dyna_d0703_state::flash_w));

	map(0x60000000, 0x600007ff).mirror(0x0000f800).ram(); // internal RAM, the BIOS triplicate write helper is also used on it, so it's probably mirrored

	map(0xffff0000, 0xffffffff).rw(FUNC(dyna_d0703_state::soc_r), FUNC(dyna_d0703_state::soc_w)).share(m_soc_regs);
	map(0xffff1c00, 0xffff1c03).portr("DSW1").umask32(0x000000ff); // bit 0 goes to display control bit 2
	map(0xffff1c04, 0xffff1c04).lr8(NAME([] () -> uint8_t { return 0x01; })); // bit 0: battery OK
	map(0xffffa800, 0xffffa83f).rw(FUNC(dyna_d0703_state::intc_r), FUNC(dyna_d0703_state::intc_w));
}


static INPUT_PORTS_START( dyna_d0703 ) // names from the switch test
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ticket Output")
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // removes 100 credits in dynasty (BOX OUT in bookkeeping), not shown in the switch test
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_COIN1 ) // BIOS checks the pulse is longer than 1 frame and not longer than 16, same counter as bit 9
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BILL1 ) // BIOS counts the presses
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_COIN2 ) // BIOS checks the pulse is longer than 1 frame and not longer than 16
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_COIN3 ) // BIOS checks the pulse is longer than 1 frame and not longer than 16, same counter as bit 6
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) // also checked when the BIOS starts the program
	PORT_SERVICE_NO_TOGGLE( 0x00000800, IP_ACTIVE_HIGH ) // "Confirm" in the switch test, checked when the BIOS starts the program, enters the setup menu
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // Player Output in test mode
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT ) // Credit Down in test mode
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

static INPUT_PORTS_START( dynasty )
	PORT_INCLUDE( dyna_d0703 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )
INPUT_PORTS_END

static INPUT_PORTS_START( trip7ba )
	PORT_INCLUDE( dyna_d0703 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
INPUT_PORTS_END


void dyna_d0703_state::machine_start()
{
	m_sound_timer = timer_alloc(FUNC(dyna_d0703_state::sound_tick), this);

	save_item(NAME(m_blit_regs));
	save_item(NAME(m_disp_ctrl));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_rng));
	save_item(NAME(m_sound_fifo));
	save_item(NAME(m_sound_fifo_rd));
	save_item(NAME(m_sound_fifo_count));
}

void dyna_d0703_state::machine_reset()
{
	m_irq_pending = 0;
	m_irq_mask = 0;
	m_disp_ctrl = 0;
	m_sound_fifo_rd = 0;
	m_sound_fifo_count = 0;
	update_irq();

	attotime const period = attotime::from_hz(25.2_MHz_XTAL / 2286); // TODO: actual sample rate, ~11025 Hz is a guess
	m_sound_timer->adjust(period, 0, period);
}


void dyna_d0703_state::dyna_d0703(machine_config &config)
{
	ARM7(config, m_maincpu, 25.2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dyna_d0703_state::program_map);

	AMD_29F800B_16BIT(config, m_flash); // AMIC A29800UV, AMD command set

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	RTC62421(config, "rtc", 32.768_kHz_XTAL);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(FB_WIDTH, FB_HEIGHT);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(dyna_d0703_state::screen_update));
	screen.screen_vblank().set(FUNC(dyna_d0703_state::vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::GGGGGRRRRRBBBBBx, 0x200); // TODO: size

	SPEAKER(config, "mono").front_center();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.5); // TODO: actual DAC type unknown
}


ROM_START( dynasty ) // DYNASTY   Ver.1.02 at 0x10000 in ROM
	ROM_REGION32_LE( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dynasty_ver_102.j11", 0x00000, 0x80000, CRC(c52b5482) SHA1(7c63f1817f30d40362af51f6ccb549e42dcd6fe2) )

	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "a29800uv.h11",  0x000000, 0x100000, CRC(592fe6e6) SHA1(9775182eb9038034e3667dcfeeb914a8978f42a8) )

	ROM_REGION16_LE( 0x1000100, "gfx", 0 )
	ROM_LOAD( "m29w128gh.f11", 0x0000000, 0x1000100, CRC(5153985e) SHA1(3cf995080e1e716567c300f2004b1310097ab843) )
ROM_END

ROM_START( trip7ba ) // T7BA VER. 1.01J at 0x10000 in ROM, BIOS is DYNA System2007(D0703) V1.06.02
	ROM_REGION32_LE( 0x80000, "maincpu", 0 )
	ROM_LOAD( "triple_7_bar_v_1_01.j11", 0x00000, 0x80000, CRC(4ee816d9) SHA1(20c43d102f198dd5e0973552ae8281cad9bb6aaa) )

	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "a29800uv.h11",  0x000000, 0x100000, CRC(83dd3fe9) SHA1(aeb4babbcd68a753b0ee32e72cf67bc39901de80) )

	ROM_REGION16_LE( 0x1000100, "gfx", 0 )
	ROM_LOAD( "m29w128gh.f11", 0x0000000, 0x1000100, CRC(9383b272) SHA1(eb6da3dfd43a8b24e967db41b4eb0c30b0a2afc6) )
ROM_END

} // anonymous namespace


GAME( 2009, dynasty, 0, dyna_d0703, dynasty, dyna_d0703_state, empty_init, ROT0, "Dyna", "Dynasty (2009, Ver. 1.02)",    MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2010, trip7ba, 0, dyna_d0703, trip7ba, dyna_d0703_state, empty_init, ROT0, "Dyna", "Triple 7 Bar A (Ver. 1.01J)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
