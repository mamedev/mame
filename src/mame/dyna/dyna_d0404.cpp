// license:BSD-3-Clause
// copyright-holders:

/*
Dyna D0404 PCB

This PCB is newer hardware than the one in misc/cb2001.cpp, possibly the next one.
It seems Dyna got rid of some legacy features like physical DIPs, color PROMs and AY sound.
While the CPU is sanded off, ROM seems to contain valid ARM code.

The main components are:
208-pin custom (sanded off). MCU/CPU?
176-pin custom (sanded off). GFX?
4x AMIC LP61 1024S-12 SRAM or equivalent (for CPU?)
2x IDT71024 SRAM or equivalent (for GFX?)
2x A625308AM-70S SRAM or equivalent (for GFX?)
24.000 MHz XTAL
2x LH28F800SGE-L70 or equivalent flash ROMs
2x GAL16V8D
Oki M62X42B RTC

The program flash also holds data written by the BIOS: a 64KB block with the key checked
by the BIOS (see below) at +0x00/+0x04 and the operator settings from +0x20. It's at
0x0b0000 for cm2005 1.10U, 1.02U and 0.14H and at 0x0f0000 for cm2005 1.00H and supereld.
The 0.14H boards also have a per-board key at 0x0f0000, left over from an older BIOS.
Dumps of the same version that only differ in these blocks come from differently set up
boards, so the differences there don't determine a different set.
The BIOS only writes to the flash on the boards with Sharp flash ROMs: the cm2005 1.10U and
supereld BIOSes (AMIC A29800 flash ROMs) never write to it and only keep the settings in the
backup RAM.

Notes (all deduced from the code, no documentation available):
- the 208-pin chip is an ARM7 SoC with on-chip peripherals at 0xffff0000-0xffffffff
  (interrupt controller at 0xffffa800, 8254-like PIT at 0xffff1800, ports at 0xffff1c00,
  two UART-like devices at 0xffff0000 and 0xffff0400) and 2KB of internal RAM at 0x60000000.
- the 176-pin chip is probably an FPGA: at boot the BIOS bit-bangs a 0x577c bytes
  Xilinx XC4000 / Spartan style bitstream (8 dummy bits, '0010' preamble, 24-bit length
  count = 179153) through the port at 0xffff1c0c, unless 0xffff1c08 bits 7-6 already
  read 0xc0.
- the video chip is a blitter drawing RLE-compressed 4bpp images from the GFX ROM into
  two 512x256 framebuffers. The first 0x1000 bytes of the GFX ROM are 2048 colors in
  GGGGGRRRRRBBBBBx format (red and green are swapped compared to the usual layout: with
  RRRRRGGGGGBBBBBx cherries and 7s come out green, and the Triple 7 Bar "ALL RED" bonus
  on the D0703, which uses the same format, shows green symbols).
- RLE stream (16-bit words): 0nnnnnnnnnnn = n+1 literal words follow (4 pixels each,
  leftmost pixel in the high nibble); 1ccccnnnnnnnnnnn = n+1 words filled with pen ~cccc.
- sound is software mixed 8-bit PCM (samples in the program ROM), output at every PIT
  channel 2 IRQ (~11 kHz if the PIT is clocked at 24 MHz) to a 12-bit DAC through the ports
  at 0xffff1c00/0xffff1c04.
- the BIOS refuses to start the game (silent loop at 0x2320) unless a key stored in the
  battery backed RAM at 0x240010/0x240014 (kept in triplicate, like the rest of the backup
  data) matches a value derived from the program header at 0xc0034/0xc0038. If the backup
  RAM copy is 0 the key is read from the flash configuration block (see above). The two
  words are voted and reloaded from flash separately. Only cm2005 1.00H has a valid key in flash, so its
  default NVRAM only contains the backup data signature. For the other sets the default
  NVRAMs contain a key calculated to pass the check (the high word is arbitrary, 0 was
  chosen). All the other data is initialized by the game itself. All cm2005 versions
  share the same header values.

TODO:
- identify SoC
- blitter word 3 low 16 bits (always 0xffff in the static tables)
- display control register bits other than bit 1 (page select)
- outputs / hopper
- verify sound sample rate (depends on the PIT channel 2 input clock)
- UARTs, PIT input clocks, PIT channels 0 and 1, IRQs 7 and 9
- correct video timings
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
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

class dyna_d0404_state : public driver_device
{
public:
	dyna_d0404_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "flash"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_soc_regs(*this, "soc_regs"),
		m_gfxrom(*this, "gfx")
	{ }

	void dyna_d0404(machine_config &config) ATTR_COLD;
	void dyna_d0404_a29800(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<arm7_cpu_device> m_maincpu;
	required_device<intelfsh16_device> m_flash;
	required_device<palette_device> m_palette;
	required_device<dac_12bit_r2r_device> m_dac;

	required_shared_ptr<uint32_t> m_soc_regs;
	required_region_ptr<uint16_t> m_gfxrom;

	static constexpr uint32_t FB_WIDTH = 512;
	static constexpr uint32_t FB_HEIGHT = 256;

	std::unique_ptr<uint16_t[]> m_fb[2];
	uint32_t m_blit_regs[4] = { };
	uint32_t m_disp_ctrl = 0;
	uint32_t m_irq_pending = 0;
	uint32_t m_irq_mask = 0;
	uint16_t m_dac_data = 0;

	void palette_init(palette_device &palette) ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void set_irq(uint8_t bit, int state);
	void update_irq();
	uint32_t intc_r(offs_t offset, uint32_t mem_mask);
	void intc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	offs_t store_address(offs_t aligned);
	void flash_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t soc_r(offs_t offset, uint32_t mem_mask);
	void soc_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t blitter_status_r();
	void blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void do_blit();

	void program_map(address_map &map) ATTR_COLD;
};


void dyna_d0404_state::palette_init(palette_device &palette)
{
	// the palette is stored at the beginning of the GFX ROM
	palette.basemem().set(m_gfxrom.target(), 0x800 * 2, 16, ENDIANNESS_LITTLE, 2);

	for (int i = 0; i < 0x800; i++)
		palette.write16(i, m_gfxrom[i]);
}

void dyna_d0404_state::video_start()
{
	for (auto &fb : m_fb)
		fb = make_unique_clear<uint16_t[]>(FB_WIDTH * FB_HEIGHT);

	save_pointer(NAME(m_fb[0]), FB_WIDTH * FB_HEIGHT);
	save_pointer(NAME(m_fb[1]), FB_WIDTH * FB_HEIGHT);
}

uint32_t dyna_d0404_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const fb = m_fb[BIT(m_disp_ctrl, 1)].get();
	pen_t const *const pens = m_palette->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const src = &fb[(y & (FB_HEIGHT - 1)) * FB_WIDTH];
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			dst[x] = pens[src[x & (FB_WIDTH - 1)] & 0x7ff];
	}

	return 0;
}

void dyna_d0404_state::vblank_w(int state)
{
	if (state)
		set_irq(1, 1);
}


/*
 Blitter registers (written in order, the write to word 3 starts the operation):
 word 0: --x- ---- ---- ---- ---- ---- ---- ----  fill with the color in word 3 instead of drawing an image (always opaque)
         ---x ---- ---- ---- ---- ---- ---- ----  opaque (pen 0 is drawn too)
         ---- ---- ---- xxxx xxxx xxxx xxxx xxxx  source address in the GFX ROM (in 16-bit words)
 word 1: xxxx xxxx xxxx xxxx ---- ---- ---- ----  width - 1
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  height - 1
 word 2: xxxx xxxx xxxx xxxx ---- ---- ---- ----  destination x
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  destination y
 word 3: xxxx xxxx xxxx xxxx ---- ---- ---- ----  color base (added to the 4bpp pen, fill color when filling)
         ---- ---- ---- ---- xxxx xxxx xxxx xxxx  unknown (always 0xffff in the static tables)
*/
void dyna_d0404_state::do_blit()
{
	bool const fill = BIT(m_blit_regs[0], 29);
	bool const opaque = BIT(m_blit_regs[0], 28);
	uint32_t const width = (m_blit_regs[1] >> 16) + 1;
	uint32_t const height = (m_blit_regs[1] & 0xffff) + 1;
	int const dst_x = int16_t(m_blit_regs[2] >> 16);
	int const dst_y = int16_t(m_blit_regs[2] & 0xffff);
	uint16_t const color = m_blit_regs[3] >> 16;
	uint16_t *const fb = m_fb[BIT(m_disp_ctrl, 1) ^ 1].get(); // draws to the page not being displayed

	LOGBLIT("%s: blit %08x %08x %08x %08x\n", machine().describe_context(), m_blit_regs[0], m_blit_regs[1], m_blit_regs[2], m_blit_regs[3]);

	if ((m_blit_regs[0] & 0xcff00000) || ((m_blit_regs[3] & 0xffff) != 0xffff))
		logerror("%s: blit with unknown bits %08x %08x %08x %08x\n", machine().describe_context(), m_blit_regs[0], m_blit_regs[1], m_blit_regs[2], m_blit_regs[3]);

	// rows past the bottom of the framebuffer aren't decoded, nothing is done if the rectangle is completely outside it
	bool const visible = (dst_x < int(FB_WIDTH)) && ((dst_x + int(width)) > 0) && (dst_y < int(FB_HEIGHT)) && ((dst_y + int(height)) > 0);
	uint32_t const rows = visible ? std::min<uint32_t>(height, FB_HEIGHT - dst_y) : 0;
	uint32_t const total = width * rows / 4; // TODO: what happens if width isn't a multiple of 4?
	uint32_t addr = m_blit_regs[0] & 0x0fffff;
	uint32_t run_length = 0;
	uint16_t run_data = 0;
	bool literal = false;

	for (uint32_t i = 0; i < total; i++)
	{
		uint16_t data = 0;
		if (!fill)
		{
			if (!run_length)
			{
				uint16_t const cmd = m_gfxrom[addr++ & 0x7ffff];
				run_length = (cmd & 0x7ff) + 1;
				literal = !BIT(cmd, 15);
				run_data = ((~cmd >> 11) & 0x0f) * 0x1111;
			}

			data = literal ? m_gfxrom[addr++ & 0x7ffff] : run_data;
			run_length--;
		}

		int const y = dst_y + (i * 4 / width);
		int x = dst_x + (i * 4 % width);

		if (y < 0 || y >= FB_HEIGHT)
			continue;

		for (int p = 12; p >= 0; p -= 4, x++)
		{
			uint8_t const pen = (data >> p) & 0x0f;

			if ((pen || opaque || fill) && x >= 0 && x < FB_WIDTH)
				fb[y * FB_WIDTH + x] = (color + pen) & 0x7ff;
		}
	}

	// TODO: timing
	set_irq(0, 1);
}

uint32_t dyna_d0404_state::blitter_status_r()
{
	// bit 15: idle
	return 0x00008000;
}

void dyna_d0404_state::blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_blit_regs[offset]);

	if (offset == 3)
		do_blit();
}


void dyna_d0404_state::machine_start()
{
	save_item(NAME(m_blit_regs));
	save_item(NAME(m_disp_ctrl));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_dac_data));
}

void dyna_d0404_state::machine_reset()
{
	m_irq_pending = 0;
	m_irq_mask = 0;
	m_disp_ctrl = 0;
	update_irq();
}


// interrupt controller
void dyna_d0404_state::update_irq()
{
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, (m_irq_pending & m_irq_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void dyna_d0404_state::set_irq(uint8_t bit, int state)
{
	// only rising edges are latched (TODO: verify)
	if (state)
	{
		m_irq_pending |= 1U << bit;
		update_irq();
	}
}

uint32_t dyna_d0404_state::intc_r(offs_t offset, uint32_t mem_mask)
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

void dyna_d0404_state::intc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x08 / 4: m_irq_pending &= ~(data & mem_mask); update_irq(); break;
		case 0x0c / 4: COMBINE_DATA(&m_irq_mask); update_irq(); break;
		default: LOGSOC("%s: intc_w %02x = %08x & %08x\n", machine().describe_context(), offset * 4, data, mem_mask); break;
	}
}

// unknown SoC peripherals, just latch what's written
uint32_t dyna_d0404_state::soc_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGSOC("%s: soc_r %08x & %08x\n", machine().describe_context(), 0xffff0000 + offset * 4, mem_mask);

	return m_soc_regs[offset];
}

void dyna_d0404_state::soc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGSOC("%s: soc_w %08x = %08x & %08x\n", machine().describe_context(), 0xffff0000 + offset * 4, data, mem_mask);

	COMBINE_DATA(&m_soc_regs[offset]);
}


/*
 The BIOS programs the 16-bit flash with STR instructions to halfword aligned addresses and the
 SoC apparently turns them into a single halfword write of the low 16 bits to the actual address
 (the BIOS sets bit 8 of the configuration of the chip select covering the settings block while
 doing so).
 MAME's ARM7 core always word aligns the address of STR and nothing else in MAME passes A1 on, so
 the halfword can't be determined from the access alone. As a workaround (hack) the actual address
 is recalculated from the STR instruction being executed, which is still at R15 while the store is
 performed.
*/
offs_t dyna_d0404_state::store_address(offs_t aligned)
{
	// only the STR forms used by the BIOS are handled: [Rn, #+imm] and [Rn], #imm, with Rn in R0-R7
	// (R15 would need the pipeline offset, and state_int() returns the user bank for R8-R14)
	uint32_t const insn = m_maincpu->space(AS_PROGRAM).read_dword(m_maincpu->state_int(arm7_cpu_device::ARM7_R15));
	auto const reg = [this] (uint8_t r) { return uint32_t(m_maincpu->state_int(arm7_cpu_device::ARM7_R0 + r)); };
	uint8_t const rn = (insn >> 16) & 0x0f;

	if (rn < 8)
	{
		if ((insn & 0x0ff00000) == 0x05800000)
			return reg(rn) + (insn & 0xfff);
		else if ((insn & 0x0ff00000) == 0x04800000)
			return reg(rn); // post-indexed, the base register hasn't been updated yet
	}

	logerror("%s: unhandled store to flash, insn %08x\n", machine().describe_context(), insn);
	return aligned;
}

void dyna_d0404_state::flash_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0xffffffff)
	{
		offs_t const address = store_address(offset << 2);
		m_flash->write((address & 0xfffff) >> 1, data & 0xffff);
	}
	else
	{
		if (ACCESSING_BITS_0_15)
			m_flash->write(offset << 1, data & 0xffff);
		if (ACCESSING_BITS_16_31)
			m_flash->write((offset << 1) | 1, data >> 16);
	}
}


void dyna_d0404_state::program_map(address_map &map)
{
	map(0x00000000, 0x000fffff).r(m_flash, FUNC(intelfsh16_device::read)).w(FUNC(dyna_d0404_state::flash_w));
	map(0x00200000, 0x0023ffff).ram();
	map(0x00240000, 0x0024ffff).ram().share("nvram"); // backup data is kept in triplicate at 0x240000, 0x240400 and 0x240800

	map(0x00250000, 0x0025000f).w(FUNC(dyna_d0404_state::blitter_w));
	map(0x00250000, 0x00250003).r(FUNC(dyna_d0404_state::blitter_status_r));
	map(0x00250010, 0x00250013).lrw32(
			NAME([this] () -> uint32_t { return m_disp_ctrl; }),
			NAME([this] (offs_t offset, uint32_t data, uint32_t mem_mask)
			{
				COMBINE_DATA(&m_disp_ctrl);
				LOGVIDEO("%s: disp_ctrl = %08x\n", machine().describe_context(), m_disp_ctrl);
			}));
	map(0x00250020, 0x00250023).portr("IN1").lw32(NAME([this] (uint32_t data) { LOG("%s: outputs 0x20 = %08x\n", machine().describe_context(), data); })); // TODO: lamps, counters?
	map(0x00250030, 0x00250033).portr("IN0").lw32(NAME([this] (uint32_t data) { LOG("%s: outputs 0x30 = %08x\n", machine().describe_context(), data); }));
	map(0x00250080, 0x002500bf).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask32(0x000000ff);

	map(0x60000000, 0x600007ff).ram(); // internal RAM

	map(0xffff0000, 0xffffffff).rw(FUNC(dyna_d0404_state::soc_r), FUNC(dyna_d0404_state::soc_w)).share(m_soc_regs);
	map(0xffff1800, 0xffff180f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0x000000ff);
	map(0xffff1c00, 0xffff1c00).lw8(NAME([this] (uint8_t data) { m_dac_data = (m_dac_data & 0xf00) | data; }));
	map(0xffff1c04, 0xffff1c04).lw8(NAME([this] (uint8_t data) { m_dac_data = (m_dac_data & 0x0ff) | ((data & 0x0f) << 8); m_dac->write(m_dac_data); }));
	map(0xffff1c08, 0xffff1c08).lr8(NAME([] () -> uint8_t { return 0xef; })); // bits 7-6 FPGA configuration status (0xc0 = already configured), bit 4 checked by IRQs 7 and 9
	map(0xffffa800, 0xffffa83f).rw(FUNC(dyna_d0404_state::intc_r), FUNC(dyna_d0404_state::intc_w));
}


static INPUT_PORTS_START( cm2005 ) // names from the switch test
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // BIOS checks the pulse is longer than 1 frame and not longer than 16
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BILL1 ) // BIOS counts the presses
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 ) // BIOS checks the pulse is longer than 1 frame and not longer than 16
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // Credit Down in test mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // Player Output in test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ticket Output")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // the BIOS asks to reset to factory defaults
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW ) // "Setup" in the switch test, checked when the BIOS starts the program
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // checked when the BIOS starts the program
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void dyna_d0404_state::dyna_d0404(machine_config &config)
{
	ARM7(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dyna_d0404_state::program_map);

	SHARP_LH28F800SG(config, m_flash);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	// inside the SoC, the input clocks are unknown: channel 2 is programmed with 2177 for the PCM IRQ, so
	// 24 MHz gives ~11025 Hz, but the clock could as well be divided (12 MHz would give ~5512 Hz).
	// Channel 0 is never programmed, channel 1 is only set up once at boot (mode 4) and otherwise used as a
	// timeout by the serial / printer IRQ handlers, and the counters are never read back, so they are left
	// unclocked: clocking channel 1 at 24 MHz made emulation about four times slower
	pit8254_device &pit(PIT8254(config, "pit"));
	pit.set_clk<0>(0); // unused
	pit.set_clk<1>(0); // only used by the serial / printer code, output not hooked up
	pit.set_clk<2>(24_MHz_XTAL); // TODO: verify clock
	pit.out_handler<2>().set([this] (int state) { set_irq(8, state); });

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(FB_WIDTH, FB_HEIGHT);
	screen.set_visarea(0, 512-1, 0, 240-1);
	screen.set_screen_update(FUNC(dyna_d0404_state::screen_update));
	screen.screen_vblank().set(FUNC(dyna_d0404_state::vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::GGGGGRRRRRBBBBBx, 0x800).set_init(FUNC(dyna_d0404_state::palette_init));

	SPEAKER(config, "mono").front_center();
	DAC_12BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.5); // TODO: actual DAC type unknown
}

void dyna_d0404_state::dyna_d0404_a29800(machine_config &config)
{
	dyna_d0404(config);

	// AMIC A29800 (AMD command set), the BIOS of these sets never writes to the flash
	AMD_29F800B_16BIT(config.replace(), m_flash);
}


ROM_START( cm2005 ) // DYNA CM2005 VER. 1.10U at 0xc0000 in ROM
	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "a29800uv.11b", 0x000000, 0x100000, CRC(b86c6953) SHA1(73c78a2529abad6aa61fda8285c172bdb6c380cd) )

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "a29800uv.12b", 0x000000, 0x100000, CRC(c1ae3e9a) SHA1(6cf883b586e074f7517aef4db1c7b006d0ed8df6) )

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )

	ROM_REGION( 0x10000, "nvram", 0 ) // handcrafted: just the key and the backup data signature, see notes at the top of the driver
	ROM_LOAD( "cm2005_nvram.bin", 0x00000, 0x10000, CRC(0618b7c8) SHA1(2fe54708ecb10b272489702fd6fcaab138e10f4f) )
ROM_END

ROM_START( cm2005_102u ) // DYNA CM2005 VER. 1.02U at 0xc0000 in ROM
	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "lh28f800.11b", 0x000000, 0x100000, CRC(f567782c) SHA1(6138d3170dc4505e1ab2444f26e76e56d5393444) )

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "lh28f800.12b", 0x000000, 0x100000, CRC(c1ae3e9a) SHA1(6cf883b586e074f7517aef4db1c7b006d0ed8df6) )

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )

	ROM_REGION( 0x10000, "nvram", 0 ) // handcrafted: just the key and the backup data signature, see notes at the top of the driver
	ROM_LOAD( "cm2005_nvram.bin", 0x00000, 0x10000, CRC(0618b7c8) SHA1(2fe54708ecb10b272489702fd6fcaab138e10f4f) )
ROM_END

ROM_START( cm2005_100h ) // DYNA CM2005 VER. 1.00H at 0xc0000 in ROM
	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "lh28f800.11b", 0x000000, 0x100000, CRC(f1c68268) SHA1(e23d3fbb43bdcc226209aec1b65215cc572ef575) ) // SLDH

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "lh28f800.12b", 0x000000, 0x100000, CRC(d6ecf6a4) SHA1(1db677e4537c9011a49a4c831d729d856d90846a) ) // SLDH

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )

	ROM_REGION( 0x10000, "nvram", 0 ) // handcrafted: just the backup data signature, the key is in the flash configuration block
	ROM_LOAD( "cm2005_100h_nvram.bin", 0x00000, 0x10000, CRC(ce081b50) SHA1(bebe25a70cd9124e7fd91227049c7f614de97de7) )
ROM_END

ROM_START( cm2005_014h ) // DYNA CM2005 VER. 0.14H at 0xc0000 in ROM
	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "lh28f800.11b", 0x000000, 0x100000, CRC(1af9c956) SHA1(77daffbb8279098218e8401260363af9de4b1d7f) ) // SLDH

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "lh28f800.12b", 0x000000, 0x100000, CRC(74163225) SHA1(14853e347e4e439166e218cd2713e372c871a984) ) // SLDH

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )

	ROM_REGION( 0x10000, "nvram", 0 ) // handcrafted: just the key and the backup data signature, see notes at the top of the driver
	ROM_LOAD( "cm2005_nvram.bin", 0x00000, 0x10000, CRC(0618b7c8) SHA1(2fe54708ecb10b272489702fd6fcaab138e10f4f) )
ROM_END

ROM_START( supereld ) // DYNA SUPER EL DORADO Ver.1.04H at 0xc0000 in ROM
	ROM_REGION16_LE( 0x100000, "flash", 0 )
	ROM_LOAD( "a29800uv-70f.11b", 0x000000, 0x100000, CRC(d142d7b2) SHA1(33b02a9e9ebc21bda0f262d921388c9f5edf256e) )

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "a29800uv-70f.12b", 0x000000, 0x100000, CRC(080f5212) SHA1(b76041ec24cc4e3d4694b94977ead282feaaa772) )

	ROM_REGION( 0x400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8d.10a", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.10b", 0x200, 0x117, NO_DUMP )

	ROM_REGION( 0x10000, "nvram", 0 ) // handcrafted: just the key and the backup data signature, see notes at the top of the driver
	ROM_LOAD( "supereld_nvram.bin", 0x00000, 0x10000, CRC(31c66304) SHA1(f0ef835179bf28d2634dae3df9d00d5e7420a56d) )
ROM_END

} // anonymous namespace


GAME( 2005, cm2005,       0,      dyna_d0404_a29800, cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 1.10U)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, cm2005_102u,  cm2005, dyna_d0404,        cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 1.02U)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, cm2005_100h,  cm2005, dyna_d0404,        cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 1.00H)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, cm2005_014h,  cm2005, dyna_d0404,        cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 0.14H)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2008, supereld,     0,      dyna_d0404_a29800, cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Super El Dorado (Ver. 1.04H)",    MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
