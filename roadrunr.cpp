// license:BSD-3-Clause
// copyright-holders:Ken Van Mersbergen (Dutchman2000)
/***************************************************************************

    Atari "Road Runner" (unreleased laserdisc game, 1984-1985)

    Programmer: Ed Logg.  Hardware: Atari "standard video disk hardware".

    The game was never released.  Everything in this driver was worked out
    from the original VAX development files ([LOGG.RUNNER]): the program
    sources (RRDEF.MAC holds the hardware equates), the hardware memo
    SPEC.DOC, the priority PROM equations PRIOR.ROM, and the linked load
    (.LDA) files.  No PCB is known to exist, so ROM chip placement and some
    video timing details are educated guesses.  The program/sound ROM images
    are the archived LDA files cut into 27128-sized pieces.

    Two hardware revisions appear in the files:

    1984 board (sources in [LOGG.RUNNER.OLD], LDA files of Oct/Nov 1984)
      main:  0000-07FF RAM, 0800-0FFF sound comm RAM, 1A00-1AFF I/O,
             1C00-1FFF colour RAM (2 halves, COLSEL), 2000-27FF alphanumerics,
             2800-2FFF motion objects (2 banks of 1K, byte arrays),
             3000-3FFF scrolling playfield, 5000-FFFF ROM.
      priority: 82S195 PROM (PRIOR.ROM equations)

    1985 board (sources in [LOGG.RUNNER], LDA files of Feb/Mar 1985)
      main:  0000-07FF RAM, 0800-0FFF sound comm RAM, 1000-17FF alphanumerics,
             1F00-1FFF I/O, 2000-27FF colour RAM (1024 x 16, MMRRRRRGGGGGBBBB),
             2800-2FFF priority RAM (odd bytes, 4 bits), 3000-3FFF motion
             objects (2 banks of 2K, 16-bit words), 4000-4FFF scrolling
             playfield, 5000-FFFF ROM.

    Common:
      - 6502 main CPU and 6502 sound CPU, both ~1.79 MHz
      - 4 IRQs per frame, laserdisc data ready on NMI
      - 56 motion objects, 8 pixels wide, up to 8 x 16-line pictures tall,
        4bpp, 4 palettes, interlaced; 1024 pictures in 4 x 27128
      - 8x8 2bpp alphanumerics (2 banks of 256 in a 2764), 64x32 RAM, 46x30 shown
      - 8x8 scrolling playfield, 64x32 words, only one 27128 stuffed (2bpp)
      - colour RAM entries carry two "mix" bits: 0 = graphics, 3 = disc video
      - Philips 22VP931 laserdisc (same interface as Fire Fox)
      - sound board: Quad POKEY, YM2151, TMS5220, 2 x X2212 NOVRAM,
        2K RAM shared with the main CPU (main 0800 = sound 3000)

    The archived LDA files are development builds.  On the development system
    the IRQ came in through the "UINT" vector at FFF8 (the FFFE vector was
    used by the debugger), so those sets have the IRQ vector fetch redirected
    to FFF8.  The "roadrunrnd" set is a rebuild of the March 1985 source with
    the program's own NODISK switch on and a real IRQ vector added.

    TODO:
    - everything that needs a real board to confirm: exact pixel offsets,
      sound IRQ source/rate, sound volume registers, MO V/H reflect details
    - laserdisc: no disc has ever been found (NO_DUMP); without a disc the
      player is reported as not operational so the game runs on graphics only
    - joystick calibration lives in NOVRAM; go through self test once

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/ldvp931.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/ymopm.h"

#include "romload.h"
#include "screen.h"
#include "speaker.h"


namespace {

static constexpr XTAL MASTER_CLOCK = XTAL(14'318'181);

class roadrunr_state : public driver_device
{
public:
	roadrunr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_laserdisc(*this, "laserdisc"),
		m_screen(*this, "screen"),
		m_pokey(*this, "pokey%u", 1U),
		m_ym(*this, "ym"),
		m_tms(*this, "tms"),
		m_alpha_ram(*this, "alpharam"),
		m_pf_ram(*this, "pfram"),
		m_prio_ram(*this, "prioram"),
		m_maincpu_rom(*this, "maincpu"),
		m_audiocpu_rom(*this, "audiocpu"),
		m_mo_rom(*this, "mo"),
		m_an_rom(*this, "alpha"),
		m_pf_rom(*this, "pf"),
		m_prom(*this, "prom"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_stick(*this, "STICK%c", 'X'),
		m_leds(*this, "led%u", 0U)
	{ }

	void roadrunr(machine_config &config) ATTR_COLD;
	void roadrunr_nodisc(machine_config &config) ATTR_COLD;
	void roadrunr84(machine_config &config) ATTR_COLD;

	void init_dev() ATTR_COLD;
	void init_nodisk() ATTR_COLD;   // also used by the rebuilt final set
	void init_dev84() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr int VIS_W = 368;   // 46 stamps
	static constexpr int VIS_H = 480;   // 30 stamps, line doubled
	static constexpr int VTOP = 44;     // first visible line of the 525 line raster

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<philips_22vp931_device> m_laserdisc;
	required_device<screen_device> m_screen;
	required_device_array<pokey_device, 4> m_pokey;
	required_device<ym2151_device> m_ym;
	required_device<tms5220_device> m_tms;

	required_shared_ptr<uint8_t> m_alpha_ram;
	required_shared_ptr<uint8_t> m_pf_ram;
	optional_shared_ptr<uint8_t> m_prio_ram;
	required_region_ptr<uint8_t> m_maincpu_rom;
	required_region_ptr<uint8_t> m_audiocpu_rom;
	required_region_ptr<uint8_t> m_mo_rom;
	required_region_ptr<uint8_t> m_an_rom;
	required_region_ptr<uint8_t> m_pf_rom;
	optional_region_ptr<uint8_t> m_prom;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport_array<2> m_stick;
	output_finder<4> m_leds;

	// configuration
	bool m_hw84 = false;
	bool m_main_uint_vector = false;   // IRQ vector fetched from FFF8 (development builds)
	bool m_disc_present = false;

	// video
	std::unique_ptr<uint8_t[]> m_mo_ram;      // 2 banks
	std::unique_ptr<uint8_t[]> m_color_ram;   // 2K (1024 words, big endian)
	uint8_t m_mo_bank_sel = 0;
	uint8_t m_colsel = 0;
	uint8_t m_alpha_bank = 0;
	bool m_mo_disable = false;
	bool m_graphics_off = false;
	bool m_video_blank = false;
	uint16_t m_hscroll = 0;
	uint8_t m_vscroll = 0;
	uint8_t m_mo_line_pix[VIS_W];
	uint8_t m_mo_line_pal[VIS_W];

	// misc
	uint8_t m_pot_select = 0;
	bool m_nmi_enable = false;
	bool m_sndxio = true;               // D7 of SNDXIO; 0 = NMI to sound CPU
	uint8_t m_disc_wrdata = 0;
	uint8_t m_disc_rddata = 0;
	uint8_t m_snd_latch[8]{};

	uint8_t main_vector_r(offs_t offset);
	uint8_t audio_vector_r(offs_t offset);
	uint8_t mo_ram_r(offs_t offset);
	void mo_ram_w(offs_t offset, uint8_t data);
	uint8_t color_ram_r(offs_t offset);
	void color_ram_w(offs_t offset, uint8_t data);

	uint8_t status85_r();
	void latch85a_w(offs_t offset, uint8_t data);
	void latch85b_w(offs_t offset, uint8_t data);
	void latch85c_w(offs_t offset, uint8_t data);
	void scroll85_w(offs_t offset, uint8_t data);

	uint8_t cheat84_r();
	uint8_t switches84_r();
	uint8_t status84_r();
	void latch84a_w(offs_t offset, uint8_t data);
	void latch84b_w(offs_t offset, uint8_t data);
	void latch84c_w(offs_t offset, uint8_t data);
	void scroll84_w(offs_t offset, uint8_t data);

	uint8_t vcount_r();
	uint8_t pot_r();
	void pot_select_w(offs_t offset, uint8_t data);
	void irq_ack_w(uint8_t data);
	void nmi_ack_w(uint8_t data);
	uint8_t disc_data_r();
	void disc_read_enable_w(uint8_t data);
	void disc_data_w(uint8_t data);
	void disc_write_strobe(int state);
	void disc_reset(int state);
	void disc_data_ready(philips_22vp931_device &device, int state);
	void sound_cpu_reset(int state);
	void set_sndxio(int state);
	void aux_shared_w(offs_t offset, uint8_t data);

	uint8_t snd_status_r();
	void snd_latch_w(offs_t offset, uint8_t data);
	uint8_t snd_latch_r(offs_t offset);
	void snd_irq_ack_w(uint8_t data);
	void snd_tms_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);

	void draw_frame(bitmap_rgb32 &bitmap, const rectangle &cliprect, int ybase, bool premultiply);
	uint32_t screen_update_disc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nodisc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main85_map(address_map &map) ATTR_COLD;
	void main84_map(address_map &map) ATTR_COLD;
	void audio85_map(address_map &map) ATTR_COLD;
	void audio84_map(address_map &map) ATTR_COLD;

	void common(machine_config &config) ATTR_COLD;
};


/*************************************
 *
 *  Video
 *
 *************************************/

uint8_t roadrunr_state::mo_ram_r(offs_t offset)
{
	// the lower window (3000/2800) holds the bank being displayed,
	// the upper window (3800/2C00) the bank the program is building
	const offs_t half = m_hw84 ? 0x400 : 0x800;
	const int bank = (offset >= half) ? m_mo_bank_sel : (m_mo_bank_sel ^ 1);
	return m_mo_ram[bank * half + (offset & (half - 1))];
}

void roadrunr_state::mo_ram_w(offs_t offset, uint8_t data)
{
	const offs_t half = m_hw84 ? 0x400 : 0x800;
	const int bank = (offset >= half) ? m_mo_bank_sel : (m_mo_bank_sel ^ 1);
	m_mo_ram[bank * half + (offset & (half - 1))] = data;
}

uint8_t roadrunr_state::color_ram_r(offs_t offset)
{
	if (m_hw84)
		offset = (m_colsel << 10) | (offset & 0x3ff);
	return m_color_ram[offset];
}

void roadrunr_state::color_ram_w(offs_t offset, uint8_t data)
{
	if (m_hw84)
		offset = (m_colsel << 10) | (offset & 0x3ff);
	m_color_ram[offset] = data;
}


void roadrunr_state::draw_frame(bitmap_rgb32 &bitmap, const rectangle &cliprect, int ybase, bool premultiply)
{
	static const uint8_t mix_alpha[4] = { 0xff, 0xaa, 0x80, 0x00 };

	const offs_t mo_half = m_hw84 ? 0x400 : 0x800;
	const uint8_t *moram = &m_mo_ram[(m_mo_bank_sel ^ 1) * mo_half];
	const uint8_t *colram = m_hw84 ? &m_color_ram[m_colsel << 10] : &m_color_ram[0];

	for (int y = std::max(cliprect.min_y, ybase); y <= std::min(cliprect.max_y, ybase + VIS_H - 1); y++)
	{
		const int fy = y - ybase;      // frame line 0-479
		const int line = fy >> 1;      // field line 0-239
		uint32_t *dest = &bitmap.pix(y);

		if (m_video_blank)
		{
			for (int x = 0; x < VIS_W; x++)
				dest[x] = 0xff000000;
			continue;
		}

		// ---- motion objects for this line ----
		std::fill(std::begin(m_mo_line_pix), std::end(m_mo_line_pix), 0);
		if (!m_mo_disable && !m_graphics_off)
		{
			for (int n = 0; n < 56; n++)   // higher numbers have priority
			{
				int hpos, top, pal, bank, hflip, vflip;
				uint8_t pics[8];
				if (!m_hw84)
				{
					// 16-bit words, high byte at the even address
					const uint8_t vhi = moram[0x700 + n * 2], vlo = moram[0x701 + n * 2];
					const uint8_t hhi = moram[0x780 + n * 2], hlo = moram[0x781 + n * 2];
					const int v9 = (vhi << 1) | (vlo >> 7);
					hpos = (hhi << 1) | (hlo >> 7);
					bank = hlo & 3;
					pal = (vlo >> 2) & 3;
					hflip = BIT(vlo, 0);
					vflip = BIT(vlo, 1);
					top = (256 - v9) & 0x1ff;           // field line of picture slot 0 (top)
					for (int s = 0; s < 8; s++)
						pics[s] = moram[s * 0x80 + n * 2 + 1];
				}
				else
				{
					// byte arrays of 64 entries
					const uint8_t aux = moram[0x000 + n];
					const uint8_t v = moram[0x040 + n];
					hpos = moram[0x0c0 + n] | (BIT(aux, 0) << 8);
					bank = (aux >> 2) & 3;
					pal = (aux >> 4) & 3;
					hflip = BIT(aux, 6);
					vflip = BIT(aux, 7);
					top = (190 - v) & 0x1ff;            // MOP7 is the top picture
					for (int s = 0; s < 8; s++)
						pics[s] = moram[0x200 + s * 0x40 + n];
				}
				if (top >= 0x180)
					top -= 0x200;

				int row = fy - top * 2;                 // 16 interlaced lines per picture
				if (row < 0 || row >= 8 * 16)
					continue;
				if (vflip)
					row = 8 * 16 - 1 - row;
				// one 27128 per bit plane; inside each, 4 banks of 256 pictures x 16 lines
				const uint8_t *src = &m_mo_rom[(bank << 12) | (pics[row >> 4] << 4) | (row & 15)];
				const uint8_t p0 = src[0x0000], p1 = src[0x4000], p2 = src[0x8000], p3 = src[0xc000];
				for (int px = 0; px < 8; px++)
				{
					const int bit = 7 - px;
					const uint8_t c = BIT(p0, bit) | (BIT(p1, bit) << 1) | (BIT(p2, bit) << 2) | (BIT(p3, bit) << 3);
					if (!c)
						continue;
					const int x = (hpos + (hflip ? 7 - px : px)) & 0x1ff;
					if (x < VIS_W)
					{
						m_mo_line_pix[x] = c;
						m_mo_line_pal[x] = pal;
					}
				}
			}
		}

		// ---- playfield / alphanumerics / priority ----
		const int pfy = (line + m_vscroll) & 0xff;
		const int arow = line >> 3;
		for (int x = 0; x < VIS_W; x++)
		{
			int alpha = 0, pf = 0;
			if (!m_graphics_off)
			{
				const uint8_t code = m_alpha_ram[arow * 64 + (x >> 3)];
				const uint8_t *a = &m_an_rom[(m_alpha_bank << 12) | (code << 4) | ((line & 7) << 1)];
				const uint16_t bits = (a[0] << 8) | a[1];
				alpha = (bits >> (14 - 2 * (x & 7))) & 3;

				const int pfx = (x + m_hscroll) & 0x1ff;
				const offs_t offs = ((pfy >> 3) * 64 + (pfx >> 3)) * 2;
				const uint8_t hi = m_pf_ram[offs], lo = m_pf_ram[offs + 1];
				const int tx = BIT(hi, 6) ? 7 - (pfx & 7) : (pfx & 7);
				const int ty = BIT(hi, 7) ? 7 - (pfy & 7) : (pfy & 7);
				const offs_t tile = (((hi & 7) << 8) | lo) & 0x3ff;
				const uint8_t *p = &m_pf_rom[(tile << 3) | ty];
				pf = BIT(p[0], 7 - tx) | (BIT(p[0x2000], 7 - tx) << 1);
			}

			const int mopix = m_mo_line_pix[x];
			const int mopal = mopix ? m_mo_line_pal[x] : 0;
			offs_t word;
			if (!m_hw84)
			{
				// priority RAM: A1-A4 MO pixel, A5-A6 MO palette, A7-A8 playfield,
				// A9 rest of playfield, A10 alphanumerics
				const offs_t pidx = (mopix << 1) | (mopal << 5) | (pf << 7) | ((alpha ? 1 : 0) << 10) | 1;
				const uint8_t pr = m_prio_ram[pidx];
				const int bnk = (pr >> 2) & 3;
				if ((pr & 3) == 0)
					word = (bnk << 8) | (mopal << 6) | mopix;
				else
					word = (bnk << 8) | (alpha << 6) | pf;
			}
			else
			{
				// 82S195: A0-A3 playfield, A4-A7 MO pixel, A9-A10 MO palette, A11 alpha
				const offs_t pidx = pf | (mopix << 4) | (mopal << 9) | ((alpha ? 1 : 0) << 11);
				const uint8_t pr = m_prom[pidx];
				switch (pr & 3)
				{
					case 3:  word = (((pr >> 2) & 3) << 6) | mopix; break;  // motion object
					case 1:  word = 0x140 | alpha; break;                    // alphanumerics
					default: word = 0x100 | pf; break;                       // playfield
				}
			}
			word &= 0x3ff;
			const uint16_t c = (colram[word * 2] << 8) | colram[word * 2 + 1];
			const uint8_t r = pal5bit((c >> 9) & 0x1f);
			const uint8_t g = pal5bit((c >> 4) & 0x1f);
			const uint8_t b = pal4bit(c & 0x0f);
			const uint8_t a = mix_alpha[c >> 14];
			if (premultiply)
				dest[x] = rgb_t(0xff, r * a / 255, g * a / 255, b * a / 255);
			else
				dest[x] = rgb_t(a, r, g, b);
		}
	}
}

uint32_t roadrunr_state::screen_update_disc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_frame(bitmap, cliprect, 0, false);
	return 0;
}

uint32_t roadrunr_state::screen_update_nodisc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	draw_frame(bitmap, cliprect, VTOP, true);
	return 0;
}


/*************************************
 *
 *  Interrupts / misc
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(roadrunr_state::irq_timer)
{
	m_screen->update_partial(m_screen->vpos());
	m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
}

void roadrunr_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

void roadrunr_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint8_t roadrunr_state::main_vector_r(offs_t offset)
{
	if (m_main_uint_vector)
		return m_maincpu_rom[0xfff8 + offset];
	return m_maincpu_rom[0xfffe + offset];
}

uint8_t roadrunr_state::audio_vector_r(offs_t offset)
{
	// every archived sound program points FFFE at the development system
	// monitor (F9B8); the real handler is behind the "UINT" vector at FFF8
	return m_audiocpu_rom[0xfff8 + offset];
}

uint8_t roadrunr_state::vcount_r()
{
	return std::max(m_screen->vpos() - VTOP, 0) >> 1;
}

uint8_t roadrunr_state::pot_r()
{
	return m_stick[m_pot_select]->read();
}

void roadrunr_state::pot_select_w(offs_t offset, uint8_t data)
{
	if (m_hw84)
		m_pot_select = BIT(offset, 0) ? 0 : 1;   // A0 = 1 selects H
	else
		m_pot_select = BIT(offset, 1);           // A1 = 0 selects H
}


/*************************************
 *
 *  Laserdisc
 *
 *************************************/

uint8_t roadrunr_state::disc_data_r()
{
	return m_disc_rddata;
}

void roadrunr_state::disc_read_enable_w(uint8_t data)
{
	m_disc_rddata = m_disc_present ? m_laserdisc->data_r() : 0xff;
}

void roadrunr_state::disc_data_w(uint8_t data)
{
	m_disc_wrdata = data;
}

void roadrunr_state::disc_write_strobe(int state)
{
	if (!state && m_disc_present)
		m_laserdisc->data_w(m_disc_wrdata);
}

void roadrunr_state::disc_reset(int state)
{
	if (m_disc_present)
		m_laserdisc->reset_w(state ? CLEAR_LINE : ASSERT_LINE);
}

void roadrunr_state::disc_data_ready(philips_22vp931_device &device, int state)
{
	// without a disc the player is reported as not fitted, so keep its
	// firmware's status traffic away from the main CPU
	if (state && m_nmi_enable && m_disc_present)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


/*************************************
 *
 *  Sound board interface
 *
 *************************************/

void roadrunr_state::sound_cpu_reset(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}

void roadrunr_state::set_sndxio(int state)
{
	if (state != m_sndxio)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
	m_sndxio = state;
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? CLEAR_LINE : ASSERT_LINE);
}

void roadrunr_state::aux_shared_w(offs_t offset, uint8_t data)
{
	// the sound CPU sees the main CPU's I/O latch page at 3800-3FFF;
	// it writes the SNDXIO location to release its own NMI
	const offs_t sndxio = m_hw84 ? 0x244 : 0x702;
	if (offset == sndxio)
		set_sndxio(BIT(data, 7));
}

uint8_t roadrunr_state::snd_status_r()
{
	// TMS5220 READY (active low): D3 on the 1985 board (status at 1200),
	// D6 on the 1984 board (status at 1000) - the 1984 RRRPM does
	// LDA 1000 / AND #40 before each speech byte, the 1985 one LDA 1200 / AND #08
	if (m_hw84)
		return 0xbf | (m_tms->readyq_r() ? 0x40 : 0x00);
	return 0xf7 | (m_tms->readyq_r() ? 0x08 : 0x00);
}

void roadrunr_state::snd_latch_w(offs_t offset, uint8_t data)
{
	m_snd_latch[offset & 7] = data;
}

uint8_t roadrunr_state::snd_latch_r(offs_t offset)
{
	return m_snd_latch[offset & 7];
}

void roadrunr_state::snd_irq_ack_w(uint8_t data)
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

void roadrunr_state::snd_tms_w(uint8_t data)
{
	m_tms->data_w(data);
}


/*************************************
 *
 *  1985 board I/O
 *
 *************************************/

uint8_t roadrunr_state::status85_r()
{
	// D0 VBLANK (1=on), D1 field, D3 sound CPU NMI (0=pending),
	// D4 slam / cheat (0=on), D5 disc operational (0=yes), D6 disc ready for data, D7 disc data available
	uint8_t data = (m_in0->read() & 0x10) | 0x04;
	data |= m_screen->vblank() ? 0x01 : 0x00;
	data |= (m_screen->frame_number() & 1) ? 0x02 : 0x00;
	data |= m_sndxio ? 0x08 : 0x00;
	if (m_disc_present)
	{
		data |= m_laserdisc->ready_r() ? 0x40 : 0x00;
		data |= m_laserdisc->data_available_r() ? 0x80 : 0x00;
	}
	else
		data |= 0x20;
	return data;
}

// 1F00-1F0E: SNDRES, SNDXIO, BLANKV, VDLOCK, NMIEN, VDRES, VDWREN
void roadrunr_state::latch85a_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	switch ((offset >> 1) & 7)
	{
		case 0: sound_cpu_reset(state); break;
		case 1: set_sndxio(state); break;
		case 2: m_video_blank = state; break;
		case 3: break; // lock to disc sync
		case 4:
			m_nmi_enable = state;
			if (!state)
				m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;
		case 5: disc_reset(state); break;
		case 6: disc_write_strobe(state); break;
		case 7: break;
	}
}

// 1F20-1F2E: coin counters, LEDs
void roadrunr_state::latch85b_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	const int bit = (offset >> 1) & 7;
	switch (bit)
	{
		case 0: machine().bookkeeping().coin_counter_w(0, state); break;
		case 1: machine().bookkeeping().coin_counter_w(1, state); break;
		case 4: case 5: case 6: case 7: m_leds[bit - 4] = state ? 0 : 1; break;
	}
}

// 1F40-1F4E: MODIS, MOSEL, ANSEL, MOCONF, GROFF
void roadrunr_state::latch85c_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	switch ((offset >> 1) & 7)
	{
		case 3: m_mo_disable = !state; break;
		case 4: m_mo_bank_sel = state; break;
		case 5: m_alpha_bank = state; break;   // 0 = first set (RRDEF comment says the opposite)
		case 6: break; // MO RAM byte order for 6502 vs 6809
		case 7: m_graphics_off = state; break;
	}
}

// 1F60/1F62 HSCRL (A1 = bit 8), 1F64/1F66 VSCRL
void roadrunr_state::scroll85_w(offs_t offset, uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	if (offset < 4)
		m_hscroll = data | (BIT(offset, 1) << 8);
	else
		m_vscroll = data;
}


void roadrunr_state::main85_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).ram().share("commram");
	map(0x1000, 0x17ff).ram().share("alpharam");
	map(0x1f00, 0x1f00).r(FUNC(roadrunr_state::status85_r));
	map(0x1f00, 0x1f0f).w(FUNC(roadrunr_state::latch85a_w));
	map(0x1f20, 0x1f20).portr("IN1");
	map(0x1f20, 0x1f2f).w(FUNC(roadrunr_state::latch85b_w));
	map(0x1f40, 0x1f4f).w(FUNC(roadrunr_state::latch85c_w));
	map(0x1f60, 0x1f60).r(FUNC(roadrunr_state::vcount_r));
	map(0x1f60, 0x1f67).w(FUNC(roadrunr_state::scroll85_w));
	map(0x1f68, 0x1f6f).nopw(); // disc picture H/V shift
	map(0x1f70, 0x1f70).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1f74, 0x1f74).w(FUNC(roadrunr_state::irq_ack_w));
	map(0x1f80, 0x1f80).r(FUNC(roadrunr_state::pot_r));
	map(0x1f80, 0x1f83).w(FUNC(roadrunr_state::pot_select_w));
	map(0x1fa0, 0x1fa0).portr("DSW").w(FUNC(roadrunr_state::nmi_ack_w));
	map(0x1fc0, 0x1fc0).w(FUNC(roadrunr_state::disc_read_enable_w));
	map(0x1fe0, 0x1fe0).rw(FUNC(roadrunr_state::disc_data_r), FUNC(roadrunr_state::disc_data_w));
	map(0x2000, 0x27ff).rw(FUNC(roadrunr_state::color_ram_r), FUNC(roadrunr_state::color_ram_w));
	map(0x2800, 0x2fff).ram().share("prioram");
	map(0x3000, 0x3fff).rw(FUNC(roadrunr_state::mo_ram_r), FUNC(roadrunr_state::mo_ram_w));
	map(0x4000, 0x4fff).ram().share("pfram");
	map(0x5000, 0xffff).rom();
	map(0xfffe, 0xffff).r(FUNC(roadrunr_state::main_vector_r));
}

void roadrunr_state::audio85_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x10ff).ram().share("nvram");      // 2 x X2212
	map(0x1200, 0x1200).r(FUNC(roadrunr_state::snd_status_r));
	map(0x1230, 0x1231).rw(m_ym, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1240, 0x124f).rw(m_pokey[0], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1250, 0x125f).rw(m_pokey[1], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1260, 0x126f).rw(m_pokey[2], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1270, 0x127f).rw(m_pokey[3], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1280, 0x1287).rw(FUNC(roadrunr_state::snd_latch_r), FUNC(roadrunr_state::snd_latch_w));
	map(0x1290, 0x12a0).nopw();                       // NOVRAM recall / store
	map(0x12b0, 0x12b0).w(FUNC(roadrunr_state::snd_irq_ack_w));
	map(0x12c0, 0x12c0).w(FUNC(roadrunr_state::snd_tms_w));
	map(0x12d0, 0x12f0).nopw();                       // volume controls
	map(0x3000, 0x37ff).ram().share("commram");
	map(0x3800, 0x3fff).w(FUNC(roadrunr_state::aux_shared_w));
	map(0x4000, 0xffff).rom();
	map(0xfffe, 0xffff).r(FUNC(roadrunr_state::audio_vector_r));
}


/*************************************
 *
 *  1984 board I/O
 *
 *************************************/

uint8_t roadrunr_state::cheat84_r()
{
	// D7 cheat, D3 slam
	return 0x77 | (BIT(m_in0->read(), 4) ? 0x88 : 0x00);
}

uint8_t roadrunr_state::switches84_r()
{
	// the 1984 board has the two hop buttons the other way round
	const uint8_t in = m_in1->read();
	return (in & 0x3f) | (BIT(in, 7) << 6) | (BIT(in, 6) << 7);
}

uint8_t roadrunr_state::status84_r()
{
	// D2 VBLANK (0=on), D4 sound CPU NMI (0=pending),
	// D5 disc operational (0=yes), D6 disc ready for data, D7 disc data available (0=yes)
	uint8_t data = 0x0b;
	data |= m_screen->vblank() ? 0x00 : 0x04;
	data |= m_sndxio ? 0x10 : 0x00;
	if (m_disc_present)
	{
		data |= m_laserdisc->ready_r() ? 0x40 : 0x00;
		data |= m_laserdisc->data_available_r() ? 0x00 : 0x80;
	}
	else
		data |= 0xa0;
	return data;
}

// 1A40-1A47: MOSCRL, VDLOCK, SNDRES, NMIEN, SNDXIO, BLANKV, VDRES, VDWREN
void roadrunr_state::latch84a_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	switch (offset & 7)
	{
		case 0: break; // one pixel playfield/MO shift
		case 1: break; // lock to disc sync
		case 2: sound_cpu_reset(state); break;
		case 3:
			// RRDEF.MAC says D7=0 enables, but the code writes 00 to "stop NMI's"
			// and FF to "allow NMI's again", same as the 1985 board
			m_nmi_enable = state;
			if (!state)
				m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;
		case 4: set_sndxio(state); break;
		case 5: m_video_blank = !state; break;
		case 6: disc_reset(state); break;
		case 7: disc_write_strobe(state); break;
	}
}

// 1A48-1A4F: MOSEL, MODIS, MOCONF, -, -, MOWDTH, ANSEL, COLSEL
void roadrunr_state::latch84b_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	switch (offset & 7)
	{
		case 0: m_mo_bank_sel = state; break;
		case 1: m_mo_disable = state; break;
		case 2: break; // MO RAM configuration
		case 5: break; // 16 pixel wide objects (never used)
		case 6: m_alpha_bank = state; break;
		case 7: m_colsel = state; break;
	}
}

// 1A50-1A57: coin counters, LEDs
void roadrunr_state::latch84c_w(offs_t offset, uint8_t data)
{
	const int state = BIT(data, 7);
	switch (offset & 7)
	{
		case 0: machine().bookkeeping().coin_counter_w(1, state); break;
		case 1: machine().bookkeeping().coin_counter_w(0, state); break;
		case 4: case 5: case 6: case 7: m_leds[7 - (offset & 7)] = state ? 0 : 1; break;
	}
}

// 1AA8/1AA9 HSCRL (A0 = bit 8, program subtracts 8), 1AB0 VSCRL
void roadrunr_state::scroll84_w(offs_t offset, uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	if (offset < 2)
		m_hscroll = ((data | (BIT(offset, 0) << 8)) + 8) & 0x1ff;
	else
		m_vscroll = data;
}


void roadrunr_state::main84_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).ram().share("commram");
	map(0x1a00, 0x1a00).r(FUNC(roadrunr_state::cheat84_r));
	map(0x1a08, 0x1a08).r(FUNC(roadrunr_state::switches84_r));
	map(0x1a10, 0x1a10).r(FUNC(roadrunr_state::status84_r));
	map(0x1a18, 0x1a18).r(FUNC(roadrunr_state::vcount_r));
	map(0x1a20, 0x1a20).portr("DSW");
	map(0x1a28, 0x1a28).r(FUNC(roadrunr_state::disc_data_r));
	map(0x1a38, 0x1a38).r(FUNC(roadrunr_state::pot_r));
	map(0x1a40, 0x1a47).w(FUNC(roadrunr_state::latch84a_w));
	map(0x1a48, 0x1a4f).w(FUNC(roadrunr_state::latch84b_w));
	map(0x1a50, 0x1a57).w(FUNC(roadrunr_state::latch84c_w));
	map(0x1a68, 0x1a68).w(FUNC(roadrunr_state::disc_read_enable_w));
	map(0x1a70, 0x1a70).w(FUNC(roadrunr_state::disc_data_w));
	map(0x1a78, 0x1a7b).w(FUNC(roadrunr_state::pot_select_w));
	map(0x1a80, 0x1a80).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1a88, 0x1a88).w(FUNC(roadrunr_state::irq_ack_w));
	map(0x1a90, 0x1a90).w(FUNC(roadrunr_state::nmi_ack_w));
	map(0x1aa8, 0x1aa9).w(FUNC(roadrunr_state::scroll84_w));
	map(0x1ab0, 0x1ab0).lw8(NAME([this] (uint8_t data) { scroll84_w(2, data); }));
	map(0x1c00, 0x1fff).rw(FUNC(roadrunr_state::color_ram_r), FUNC(roadrunr_state::color_ram_w));
	map(0x2000, 0x27ff).ram().share("alpharam");
	map(0x2800, 0x2fff).rw(FUNC(roadrunr_state::mo_ram_r), FUNC(roadrunr_state::mo_ram_w));
	map(0x3000, 0x3fff).ram().share("pfram");
	map(0x5000, 0xffff).rom();
	map(0xfffe, 0xffff).r(FUNC(roadrunr_state::main_vector_r));
}

void roadrunr_state::audio84_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x08ff).ram().share("nvram");      // 2 x X2212
	map(0x1000, 0x1000).r(FUNC(roadrunr_state::snd_status_r));
	map(0x1030, 0x1031).rw(m_ym, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1040, 0x104f).rw(m_pokey[0], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1050, 0x105f).rw(m_pokey[1], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1060, 0x106f).rw(m_pokey[2], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1070, 0x107f).rw(m_pokey[3], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1800, 0x1807).rw(FUNC(roadrunr_state::snd_latch_r), FUNC(roadrunr_state::snd_latch_w));
	map(0x1810, 0x1837).nopw();                       // NOVRAM recall / store
	map(0x1840, 0x1840).w(FUNC(roadrunr_state::snd_irq_ack_w));
	map(0x1850, 0x1850).w(FUNC(roadrunr_state::snd_tms_w));
	map(0x1860, 0x1870).nopw();                       // volume controls
	map(0x3000, 0x37ff).ram().share("commram");
	map(0x3800, 0x3fff).w(FUNC(roadrunr_state::aux_shared_w));
	map(0x8000, 0xffff).rom();
	map(0xfffe, 0xffff).r(FUNC(roadrunr_state::audio_vector_r));
}


/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( roadrunr )
	PORT_START("IN0")
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )   // slam (also the development "cheat" button)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Hop")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Hop")

	// The two option switch banks are only read when the game options in
	// NOVRAM are reset (self test "USE DEFAULT SETTINGS", or bad NOVRAM);
	// after that the settings live in NOVRAM and are changed in self test.
	// Bit layouts from RRDEF.MAC (EEOPS0/EEOPS1), names from RRMSG.MAC.
	PORT_START("DSW")   // option switch 0, main board -> coin options
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x02, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, "4 Coins/1 Credit" )
	PORT_DIPNAME( 0x04, 0x00, "Left Mech Value" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPNAME( 0x18, 0x00, "Right Mech Value" )           PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Adder" )                PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "2 Gives 1" )
	PORT_DIPSETTING(    0x40, "4 Gives 1" )
	PORT_DIPSETTING(    0x60, "4 Gives 2" )
	PORT_DIPSETTING(    0x80, "5 Gives 1" )
	PORT_DIPSETTING(    0xa0, "3 Gives 1" )
	PORT_DIPSETTING(    0xc0, "None (6)" )
	PORT_DIPSETTING(    0xe0, DEF_STR( Free_Play ) )

	PORT_START("DSW1")  // option switch 1, sound board (POKEY 1 pot inputs) -> game options
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )             PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x10, DEF_STR( Bonus_Life ) )        PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x04, "Every 35000" )
	PORT_DIPSETTING(    0x08, "Every 40000" )
	PORT_DIPSETTING(    0x0c, "Every 45000" )
	PORT_DIPSETTING(    0x10, "Every 50000" )
	PORT_DIPSETTING(    0x14, "Every 55000" )
	PORT_DIPSETTING(    0x18, "Every 60000" )
	PORT_DIPSETTING(    0x1c, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )        PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )

	// Hall-effect analog joystick (magnets + sensors, see JOYSTK.DOC): the
	// deflection picks one of 16 running speeds.  The program inverts the readings, learns the
	// low/centre/high values (self test: press both hops) and rejects a stick
	// that reads 00 or FF, so keep the range inside that (JOYSTK.DOC)
	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20, 0xe0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20, 0xe0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END


/*************************************
 *
 *  Machine
 *
 *************************************/

void roadrunr_state::machine_start()
{
	m_mo_ram = std::make_unique<uint8_t[]>(0x1000);
	m_color_ram = std::make_unique<uint8_t[]>(0x800);
	std::fill_n(m_mo_ram.get(), 0x1000, 0);
	std::fill_n(m_color_ram.get(), 0x800, 0);
	std::fill(std::begin(m_mo_line_pal), std::end(m_mo_line_pal), 0);

	m_disc_present = m_laserdisc.found() && machine().rom_load().get_disk_handle(":laserdisc") != nullptr;
	if (m_laserdisc.found())
	{
		m_laserdisc->set_data_ready_callback(philips_22vp931_device::data_ready_delegate(&roadrunr_state::disc_data_ready, this));
		// with no disc the player would show its blue "empty" frame where
		// the colour RAM selects disc video; show black instead
		if (!m_disc_present)
			m_laserdisc->video_enable(false);
	}

	save_pointer(NAME(m_mo_ram), 0x1000);
	save_pointer(NAME(m_color_ram), 0x800);
	save_item(NAME(m_mo_bank_sel));
	save_item(NAME(m_colsel));
	save_item(NAME(m_alpha_bank));
	save_item(NAME(m_mo_disable));
	save_item(NAME(m_graphics_off));
	save_item(NAME(m_video_blank));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_vscroll));
	save_item(NAME(m_pot_select));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_sndxio));
	save_item(NAME(m_disc_wrdata));
	save_item(NAME(m_disc_rddata));
	save_item(NAME(m_snd_latch));
}

void roadrunr_state::machine_reset()
{
	m_nmi_enable = false;
	m_video_blank = false;
	m_sndxio = true;
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);   // held until SNDRES
}


void roadrunr_state::common(machine_config &config)
{
	M6502(config, m_maincpu, MASTER_CLOCK / 8);
	M6502(config, m_audiocpu, MASTER_CLOCK / 8);

	config.set_maximum_quantum(attotime::from_hz(60000));

	TIMER(config, "irq").configure_scanline(FUNC(roadrunr_state::irq_timer), "screen", 10, 131);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "speaker", 2).front();

	for (int i = 0; i < 4; i++)
	{
		POKEY(config, m_pokey[i], MASTER_CLOCK / 8);
		if (i == 0)
			m_pokey[i]->allpot_r().set_ioport("DSW1");   // sound board option switches
		m_pokey[i]->add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
		m_pokey[i]->add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
	}

	YM2151(config, m_ym, MASTER_CLOCK / 4);
	m_ym->add_route(0, "speaker", 0.60, 0);
	m_ym->add_route(1, "speaker", 0.60, 1);

	TMS5220(config, m_tms, MASTER_CLOCK / 2 / 11);
	m_tms->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_tms->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}

void roadrunr_state::roadrunr(machine_config &config)
{
	common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &roadrunr_state::main85_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &roadrunr_state::audio85_map);

	PHILIPS_22VP931(config, m_laserdisc);
	m_laserdisc->set_overlay(VIS_W, VIS_H, FUNC(roadrunr_state::screen_update_disc));
	m_laserdisc->set_overlay_clip(0, VIS_W - 1, 0, VIS_H - 1);
	m_laserdisc->add_route(0, "speaker", 0.50, 0);
	m_laserdisc->add_route(1, "speaker", 0.50, 1);
	m_laserdisc->add_ntsc_screen(config, "screen");
}

void roadrunr_state::roadrunr_nodisc(machine_config &config)
{
	common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &roadrunr_state::main85_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &roadrunr_state::audio85_map);

	SCREEN(config, m_screen);
	m_screen->set_raw(MASTER_CLOCK, 455, 0, VIS_W, 525, VTOP, VTOP + VIS_H);
	m_screen->set_screen_update(FUNC(roadrunr_state::screen_update_nodisc));
}

void roadrunr_state::roadrunr84(machine_config &config)
{
	roadrunr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &roadrunr_state::main84_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &roadrunr_state::audio84_map);
}


/*************************************
 *
 *  ROMs
 *
 *  Program and sound images are the archived LDA files split into
 *  27128-sized pieces (4000-4FFF of the first main CPU chip is unused,
 *  the scrolling playfield RAM lives there).
 *
 *************************************/

#define ROADRUNR_GFX \
	ROM_REGION( 0x10000, "mo", 0 ) /* RRMO.SB4: one bit plane per chip */ \
	ROM_LOAD( "rrmo0.bin",  0x0000, 0x4000, CRC(74a91443) SHA1(acbbc0bb08a025e4ad1e6dbfeab39f3bf68b52a0) ) \
	ROM_LOAD( "rrmo1.bin",  0x4000, 0x4000, CRC(ef6976cd) SHA1(e7c6e57967190c4d62e55071591922aaa452ae63) ) \
	ROM_LOAD( "rrmo2.bin",  0x8000, 0x4000, CRC(abb2136b) SHA1(c82c2fe015292168547bae1745d17d14688155ac) ) \
	ROM_LOAD( "rrmo3.bin",  0xc000, 0x4000, CRC(abd2f9b4) SHA1(9f78ab5baf318766f0ef28fcd5c57554ea64b355) ) \
	ROM_REGION( 0x2000, "alpha", 0 ) /* RRAN.SKH */ \
	ROM_LOAD( "rran.bin",   0x0000, 0x2000, CRC(d8a4a15e) SHA1(aa9cca810d931649c317f53b2e90d59512ac59c3) )

#define ROADRUNR_PF85 \
	ROM_REGION( 0x4000, "pf", 0 ) /* RRPL.SB4, 07-FEB-1985 */ \
	ROM_LOAD( "rrpl.bin",   0x0000, 0x4000, CRC(0714284f) SHA1(3c12474c2fe4980ac2814aa517109ce5eabb06aa) )

#define ROADRUNR_PF84 \
	ROM_REGION( 0x4000, "pf", 0 ) /* RRPL.RS4, 1984 */ \
	ROM_LOAD( "rrpl84.bin", 0x0000, 0x4000, CRC(4a9eb57a) SHA1(83cdc69c90a2788c7bb7f05af28f7260ad17301c) ) \
	ROM_REGION( 0x1000, "prom", 0 ) /* compiled from PRIOR.ROM */ \
	ROM_LOAD( "prior.82s195", 0x0000, 0x1000, CRC(515fe117) SHA1(045c5884449a5c0c4d590970d7566f34f169cfe9) )

#define ROADRUNR_DISC \
	DISK_REGION( "laserdisc" ) \
	DISK_IMAGE_READONLY( "roadrunr", 0, NO_DUMP )

// RR.LDA 05-MAR-1985 11:05, AUDIO.LDA 28-FEB-1985
ROM_START( roadrunr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunr_4000.bin", 0x4000, 0x4000, CRC(76a86cf4) SHA1(d0831392dce33a3afcb453be569e1b189e1699cb) )
	ROM_LOAD( "roadrunr_8000.bin", 0x8000, 0x4000, CRC(d3834423) SHA1(5296d472819960aa67d573ec6307cb78cb3057bd) )
	ROM_LOAD( "roadrunr_c000.bin", 0xc000, 0x4000, CRC(53951a7a) SHA1(d4200f78a22622260c68b322c2fb24cb16a5f408) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr_snd_4000.bin", 0x4000, 0x4000, CRC(d204604e) SHA1(2416441952ef59dbdaaffc0acc2472c631e3d352) )
	ROM_LOAD( "roadrunr_snd_8000.bin", 0x8000, 0x4000, CRC(225426ea) SHA1(f34cf1c410bb42efa08b40ed2b1a11656ce2bdd1) )
	ROM_LOAD( "roadrunr_snd_c000.bin", 0xc000, 0x4000, CRC(1027c068) SHA1(7fed567019df56c83fb3af95960a88f71a9e2bcd) )

	ROADRUNR_GFX
	ROADRUNR_PF85
	ROADRUNR_DISC
ROM_END

// RR.OLD 06-MAR-1985 (program linked at 6000), AUDIO.OLD 06-MAR-1985
ROM_START( roadrunra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunra_4000.bin", 0x4000, 0x4000, CRC(b22e8b7d) SHA1(11eef7db89c3eb92ac3c978ed2d98d41e2436be1) )
	ROM_LOAD( "roadrunra_8000.bin", 0x8000, 0x4000, CRC(44d48fbc) SHA1(26b8415a28174dc7b4c714b3bc0f44c14b6b9b42) )
	ROM_LOAD( "roadrunra_c000.bin", 0xc000, 0x4000, CRC(6ef8333e) SHA1(0ebf1daef91e0caaf6ac50dc57eaf174513200ac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunra_snd_4000.bin", 0x4000, 0x4000, CRC(f669aee6) SHA1(cd493cfd461b1c5e7695693fa81ae01dacde25ff) )
	ROM_LOAD( "roadrunr_snd_8000.bin",  0x8000, 0x4000, CRC(225426ea) SHA1(f34cf1c410bb42efa08b40ed2b1a11656ce2bdd1) )
	ROM_LOAD( "roadrunr_snd_c000.bin",  0xc000, 0x4000, CRC(1027c068) SHA1(7fed567019df56c83fb3af95960a88f71a9e2bcd) )

	ROADRUNR_GFX
	ROADRUNR_PF85
	ROADRUNR_DISC
ROM_END

// rebuilt from the March 1985 source as a "final program" (DEBUG=0): laserdisc
// support, real IRQ vector and the program ROM checksums filled in.  RRATR.MAC
// needed one CLC: with DEBUG=0 the secret-switch check is assembled out and
// a stale carry made the attract mode always show the accounting page.
// The sound ROMs are AUDIO.LDA with the sound CPU ROM test bytes filled in.
ROM_START( roadrunrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunrf_4000.bin", 0x4000, 0x4000, CRC(20cff858) SHA1(d86eb8f1bc09e5950b5e5c93382e49da4b38048f) )
	ROM_LOAD( "roadrunrf_8000.bin", 0x8000, 0x4000, CRC(7a82d6b4) SHA1(bed5be45f3127cf0d3dfeba168587033116f9c3c) )
	ROM_LOAD( "roadrunrf_c000.bin", 0xc000, 0x4000, CRC(d0a40741) SHA1(d7730802b3f917d9357aa444088f90bdbb7ced46) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr_snd_4000.bin", 0x4000, 0x4000, CRC(d204604e) SHA1(2416441952ef59dbdaaffc0acc2472c631e3d352) )
	ROM_LOAD( "roadrunrf_snd_8000.bin", 0x8000, 0x4000, CRC(4de4510b) SHA1(386d342e9c50704aaad280d36c329cbcde36efbc) ) // AUDIO.LDA, ROM test bytes filled in (BFFF)
	ROM_LOAD( "roadrunrf_snd_c000.bin", 0xc000, 0x4000, CRC(70ee65ea) SHA1(088e307d4372ef1fad460f8c7ccf768d1248d294) ) // (FFF7), real IRQ vector at FFFE

	ROADRUNR_GFX
	ROADRUNR_PF85
	ROADRUNR_DISC
ROM_END

// rebuilt from the March 1985 source with NODISK=1 (DEBUG=2), a real IRQ vector and
// the program ROM checksums filled in
ROM_START( roadrunrnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunrnd_4000.bin", 0x4000, 0x4000, CRC(69a76fc2) SHA1(b504068a93dc5f58277c3dad382677eec70dc08b) )
	ROM_LOAD( "roadrunrnd_8000.bin", 0x8000, 0x4000, CRC(98cbcd8e) SHA1(0100acc6e4b663e8337dc5071bffcf4b97ec1707) )
	ROM_LOAD( "roadrunrnd_c000.bin", 0xc000, 0x4000, CRC(518825b7) SHA1(230e8d0b2cbabebe309a773e488c8a69660dcd8d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr_snd_4000.bin", 0x4000, 0x4000, CRC(d204604e) SHA1(2416441952ef59dbdaaffc0acc2472c631e3d352) )
	ROM_LOAD( "roadrunr_snd_8000.bin", 0x8000, 0x4000, CRC(225426ea) SHA1(f34cf1c410bb42efa08b40ed2b1a11656ce2bdd1) )
	ROM_LOAD( "roadrunr_snd_c000.bin", 0xc000, 0x4000, CRC(1027c068) SHA1(7fed567019df56c83fb3af95960a88f71a9e2bcd) )

	ROADRUNR_GFX
	ROADRUNR_PF85
ROM_END

// [LOGG.RUNNER.OLD]RR.LDA 05-NOV-1984, AUDIO3.LDA 01-NOV-1984
ROM_START( roadrunr84 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunr84_4000.bin", 0x4000, 0x4000, CRC(4c36e11c) SHA1(4fb805d8388fdbf257409eb960f0a849e4849fae) )
	ROM_LOAD( "roadrunr84_8000.bin", 0x8000, 0x4000, CRC(2502ccb9) SHA1(92f91014cd5a8441e861a5033c05e5b5d8b68c91) )
	ROM_LOAD( "roadrunr84_c000.bin", 0xc000, 0x4000, CRC(114c4be0) SHA1(52ad83c5b87328646083e6f24f8e9e4c25255d02) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr84_snd_8000.bin", 0x8000, 0x4000, CRC(0f4fabba) SHA1(d3a72a8c34f8b8c2b21c542ad49eef0eeda2acae) )
	ROM_LOAD( "roadrunr84_snd_c000.bin", 0xc000, 0x4000, CRC(664e4d10) SHA1(f4980697baa8afd74ca0a28ce5d1beff4d7ec643) )

	ROADRUNR_GFX
	ROADRUNR_PF84
	ROADRUNR_DISC
ROM_END

// RR2.LDA 09-OCT-1984, AUDIO2.LDA 09-OCT-1984
ROM_START( roadrunr84b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunr84b_4000.bin", 0x4000, 0x4000, CRC(1cb0fbf9) SHA1(d60e1e290f304647acd760b60cd9bd0286f53d4c) )
	ROM_LOAD( "roadrunr84b_8000.bin", 0x8000, 0x4000, CRC(58d288e0) SHA1(86e885adbde4fcb9e1894ac40ca2d380cfd94b86) )
	ROM_LOAD( "roadrunr84b_c000.bin", 0xc000, 0x4000, CRC(6d724265) SHA1(e3b38d88e61db7ddf8ccc1a73d305ce4963760f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr84b_snd_8000.bin", 0x8000, 0x4000, CRC(9ac1687b) SHA1(c2be05a46b19bee39de1f045fcc6edd04ae33ca6) )
	ROM_LOAD( "roadrunr84b_snd_c000.bin", 0xc000, 0x4000, CRC(649a6f38) SHA1(c4dc7f55f0b5d396125c83ff8bb623015ab74231) )

	ROADRUNR_GFX
	ROADRUNR_PF84
	ROADRUNR_DISC
ROM_END

// RR1.LDA 03-OCT-1984, AUDIO1.LDA 03-OCT-1984
ROM_START( roadrunr84a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roadrunr84a_4000.bin", 0x4000, 0x4000, CRC(14c2aa53) SHA1(c092744a10076a78a3ea883f5f7c5abf48c5e77a) )
	ROM_LOAD( "roadrunr84a_8000.bin", 0x8000, 0x4000, CRC(92da05ef) SHA1(85f0b620d71aeed4188591323b9163a6ceeb5d95) )
	ROM_LOAD( "roadrunr84a_c000.bin", 0xc000, 0x4000, CRC(0258de06) SHA1(8d50e101b3625c4ddc722f5678efc33964a4c846) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "roadrunr84a_snd_8000.bin", 0x8000, 0x4000, CRC(4e96996f) SHA1(655f41c6dee9eafbaef0947cf9319bad46b3e566) )
	ROM_LOAD( "roadrunr84a_snd_c000.bin", 0xc000, 0x4000, CRC(2bd3eadc) SHA1(48dd4328aca94946c43080af15d611f600729f9d) )

	ROADRUNR_GFX
	ROADRUNR_PF84
	ROADRUNR_DISC
ROM_END


void roadrunr_state::init_dev()
{
	m_hw84 = false;
	m_main_uint_vector = true;
}

void roadrunr_state::init_nodisk()
{
	m_hw84 = false;
	m_main_uint_vector = false;
}

void roadrunr_state::init_dev84()
{
	m_hw84 = true;
	m_main_uint_vector = true;
}

} // anonymous namespace


GAME( 1985, roadrunr,    0,        roadrunr,        roadrunr, roadrunr_state, init_dev,    ROT0, "Atari Games", "Road Runner (prototype, 05-MAR-1985, laserdisc)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1985, roadrunra,   roadrunr, roadrunr,        roadrunr, roadrunr_state, init_dev,    ROT0, "Atari Games", "Road Runner (prototype, 06-MAR-1985, laserdisc)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1985, roadrunrf,   roadrunr, roadrunr,        roadrunr, roadrunr_state, init_nodisk, ROT0, "Atari Games", "Road Runner (prototype, 05-MAR-1985 source, final-build rebuild, laserdisc)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1985, roadrunrnd,  roadrunr, roadrunr_nodisc, roadrunr, roadrunr_state, init_nodisk, ROT0, "Atari Games", "Road Runner (prototype, 05-MAR-1985 source, no-laserdisc rebuild)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, roadrunr84,  roadrunr, roadrunr84,      roadrunr, roadrunr_state, init_dev84,  ROT0, "Atari, Inc.", "Road Runner (prototype, 05-NOV-1984, laserdisc)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, roadrunr84b, roadrunr, roadrunr84,      roadrunr, roadrunr_state, init_dev84,  ROT0, "Atari, Inc.", "Road Runner (prototype, 09-OCT-1984, laserdisc)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, roadrunr84a, roadrunr, roadrunr84,      roadrunr, roadrunr_state, init_dev84,  ROT0, "Atari, Inc.", "Road Runner (prototype, 03-OCT-1984, laserdisc)",                    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
