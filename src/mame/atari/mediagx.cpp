// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Atari MediaGX

    Driver by Ville Linde

*/

/*
    Main Board number: P5GX-LG REV:1.1

    Components list on mainboard, clockwise from top left.
    ----------------------------
    SST29EE020 (2 Megabyte EEPROM, PLCC32, surface mounted, labelled 'PhoenixBIOS PENTIUM(tm) CPU (c)PHOENIX 1992-1993')
    Y1: XTAL KDS7M (To pin 122 of FDC37C932)
    SMC FDC37C932 (QFP160)
    Cyrix CX5510Rev2 (QFP208)
    40 PIN IDE CONNECTOR
    2.5 GIGABYTE IDE HARD DRIVE (Quantum EL2.5AT, CHS: 5300,15,63)
    74F245 (x3, SOIC20)
    78AT3TK HC14 (SOIC14)
    ICS9159M 9743-14 (SOIC28)
    Y2: XTAL 14.3N98F
    Y3: XTAL KDS7M
    MT4C1M16E5DJ-6 (x8, located on 2x 72 PIN SIMM Memory modules)
    CPU (?, located centrally on board, large QFP)
    ICS GENDAC ICS5342-3 (PLCC68, surface mounted)
    74HCT14 (SOIC14)
    ANALOG DEVICES AD1847JP 'SOUNDPORT' (PLCC44, surface mounted)
    AD 706 (SOIC8)
    15 PIN VGA CONNECTOR (DSUB15, plugged into custom Atari board)
    25 PIN PARALLEL CONNECTOR (DSUB25, plugged into custom Atari board)
    ICS9120 (SOIC8)
    AD706 (SOIC8)
    NE558 (SOIC16)
    74F245 (SOIC20)
    LGS GD75232 (x2, SOIC20)
    9 PIN SERIAL (x2, DSUB9, both not connected)
    PS-2 Keyboard & Mouse connectors (not connected)
    AT Power Supply Power Connectors (connected to custom Atari board)
    3 Volt Coin Battery
    74HCT32 (SOIC14)


    Custom Atari board number: ATARI GAMES INC. 5772-15642-02 JAMMIT

    Components list on custom Atari board, clockwise from top left.
    -------------------------------------
    DS1232 (SOIC8)
    74F244 (x4, SOIC20)
    3 PIN JUMPER for sound OUT (Set to MONO, alternative setting is STEREO)
    4 PIN POWER CONNECTOR for IDE Hard Drive
    15 PIN VGA CONNECTOR (DSUB15, plugged into MAIN board)
    25 PIN PARALLEL CONNECTOR (DSUB25, plugged into MAIN board)
    AD 813AR (x2, SOIC14)
    ST 084C P1S735 (x2, SOIC14)
    74F244 (SOIC20)
    7AAY2JK LS86A (SOIC14)
    74F07 (SOIC14)
    3 PIN JUMPER for SYNC (Set to -, alternative setting is +)
    OSC 14.31818MHz
    ACTEL A42MX09 (PLCC84, labelled 'JAMMINT U6 A-22505 (C)1996 ATARI')
    LS14 (SOIC14)
    74F244 (x2, SOIC20)
    ST ULN2064B (DIP16)

*/

#include "emu.h"
#include "pcshare.h"

#include "bus/centronics/atari_jammit.h"
#include "cpu/i386/i386.h"
#include "machine/cx5510.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "sound/dmadac.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define SPEEDUP_HACKS   1

class mediagx_state : public pcat_base_state
{
public:
	mediagx_state(const machine_config &mconfig, device_type type, const char *tag) :
		pcat_base_state(mconfig, type, tag),
		m_ide(*this, "ide"),
		m_ramdac(*this, "ramdac"),
		m_dmadac(*this, "dac%u", 0U),
		m_main_ram(*this, "main_ram"),
		m_cga_ram(*this, "cga_ram"),
		m_bios_ram(*this, "bios_ram"),
		m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_lpt0(*this, "lpt0"),
		m_guns(*this, "GUN%u", 0U),
		m_jammit_outputs(*this, "jammit_out%u", 0U),
		m_triggers(*this, "TRIGGERS")
	{ }

	void init_a51site4();
	void mediagx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	struct speedup_entry
	{
		uint32_t          offset;
		uint32_t          pc;
	};

	uint32_t disp_ctrl_r(offs_t offset);
	void disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t memory_ctrl_r(offs_t offset);
	void memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t biu_ctrl_r(offs_t offset);
	void biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t jammit_gun_r(offs_t player);
	void jammit_output_w(uint16_t data);
	uint32_t ad1847_r(offs_t offset);
	void ad1847_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	[[maybe_unused]] void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t io20_r(offs_t offset);
	void io20_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <offs_t N> uint32_t speedup_r(address_space &space) { return generic_speedup(space, N); }
	void draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y);
	void draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void ad1847_reg_write(int reg, uint8_t data);
	uint32_t ad1847_fifo_level(bool update = true);
	inline uint32_t generic_speedup(address_space &space, int idx);
	uint32_t frame_counter_r();
	void report_speedups();
	TIMER_CALLBACK_MEMBER(video_poll_end);
	void install_speedups(const speedup_entry *entries, int count);
	void init_mediagx();
	void mediagx_io(address_map &map) ATTR_COLD;
	void mediagx_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	required_device<ide_controller_32_device> m_ide;
	required_device<ramdac_device> m_ramdac;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint32_t> m_cga_ram;
	required_shared_ptr<uint32_t> m_bios_ram;
	required_shared_ptr<uint32_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pc_lpt_device> m_lpt0;
	uint8_t m_pal[768];

	required_ioport_array<4> m_guns;
	output_finder<14> m_jammit_outputs;
	required_ioport m_triggers;

	uint32_t m_disp_ctrl_reg[256/4]{};
	int m_frame_width = 0;
	int m_frame_height = 0;

	uint32_t m_memory_ctrl_reg[256/4]{};
	int m_pal_index = 0;

	uint32_t m_biu_ctrl_reg[256/4]{};

	uint8_t m_mediagx_config_reg_sel = 0;
	uint8_t m_mediagx_config_regs[256]{};

	uint8_t m_ad1847_regs[16]{};
	uint32_t m_audio_fifo_count = 0;
	uint64_t m_audio_fifo_sample_tick = 0;
	uint32_t m_ad1847_sample_rate = 0;

#if SPEEDUP_HACKS
	emu_timer *m_video_poll_timer = nullptr;
	uint32_t m_video_poll_pc = 0;
	uint32_t m_video_poll_vna = 0;
	const speedup_entry *m_speedup_table = nullptr;
	uint32_t m_speedup_hits[12]{};
	int m_speedup_count = 0;
#endif

	using speedup_handler = std::pair<uint32_t (mediagx_state::*)(address_space &), const char *>;
	static const speedup_handler s_speedup_handlers[];
	static const speedup_entry a51site4_speedups[];
};

// Display controller registers
#define DC_UNLOCK               0x00/4
#define DC_GENERAL_CFG          0x04/4
#define DC_TIMING_CFG           0x08/4
#define DC_OUTPUT_CFG           0x0c/4
#define DC_FB_ST_OFFSET         0x10/4
#define DC_CB_ST_OFFSET         0x14/4
#define DC_CUR_ST_OFFSET        0x18/4
#define DC_VID_ST_OFFSET        0x20/4
#define DC_LINE_DELTA           0x24/4
#define DC_BUF_SIZE             0x28/4
#define DC_H_TIMING_1           0x30/4
#define DC_H_TIMING_2           0x34/4
#define DC_H_TIMING_3           0x38/4
#define DC_FP_H_TIMING          0x3c/4
#define DC_V_TIMING_1           0x40/4
#define DC_V_TIMING_2           0x44/4
#define DC_V_TIMING_3           0x48/4
#define DC_FP_V_TIMING          0x4c/4
#define DC_CURSOR_X             0x50/4
#define DC_V_LINE_CNT           0x54/4
#define DC_CURSOR_Y             0x58/4
#define DC_SS_LINE_CMP          0x5c/4
#define DC_PAL_ADDRESS          0x70/4
#define DC_PAL_DATA             0x74/4
#define DC_DFIFO_DIAG           0x78/4
#define DC_CFIFO_DIAG           0x7c/4






static const rgb_t cga_palette[16] =
{
	rgb_t( 0x00, 0x00, 0x00 ), rgb_t( 0x00, 0x00, 0xaa ), rgb_t( 0x00, 0xaa, 0x00 ), rgb_t( 0x00, 0xaa, 0xaa ),
	rgb_t( 0xaa, 0x00, 0x00 ), rgb_t( 0xaa, 0x00, 0xaa ), rgb_t( 0xaa, 0x55, 0x00 ), rgb_t( 0xaa, 0xaa, 0xaa ),
	rgb_t( 0x55, 0x55, 0x55 ), rgb_t( 0x55, 0x55, 0xff ), rgb_t( 0x55, 0xff, 0x55 ), rgb_t( 0x55, 0xff, 0xff ),
	rgb_t( 0xff, 0x55, 0x55 ), rgb_t( 0xff, 0x55, 0xff ), rgb_t( 0xff, 0xff, 0x55 ), rgb_t( 0xff, 0xff, 0xff ),
};

void mediagx_state::video_start()
{
	int i;
	for (i=0; i < 16; i++)
	{
		m_palette->set_pen_color(i, cga_palette[i]);
	}
}

void mediagx_state::draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y)
{
	int index = 0;
	pen_t const *const pens = &m_palette->pen(0);

	uint8_t const *const dp = gfx->get_data(ch);

	for (int j=y; j < y+8; j++)
	{
		uint32_t *const p = &bitmap.pix(j);
		for (int i=x; i < x+8; i++)
		{
			uint8_t const pen = dp[index++];
			if (pen)
				p[i] = pens[gfx->colorbase() + (att & 0xf)];
			else
			{
				if (((att >> 4) & 7) > 0)
					p[i] = pens[gfx->colorbase() + ((att >> 4) & 0x7)];
			}
		}
	}
}

void mediagx_state::draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int const line_delta = (m_disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) * 4;

	int width = (m_disp_ctrl_reg[DC_H_TIMING_1] & 0x7ff) + 1;
	if (m_disp_ctrl_reg[DC_TIMING_CFG] & 0x8000)     // pixel double
	{
		width >>= 1;
	}
	width += 4;

	int height = (m_disp_ctrl_reg[DC_V_TIMING_1] & 0x7ff) + 1;

	if ( (width != m_frame_width || height != m_frame_height) &&
			(width > 1 && height > 1 && width <= 640 && height <= 480) )
	{
		rectangle visarea;

		m_frame_width = width;
		m_frame_height = height;

		visarea.set(0, width - 1, 0, height - 1);
		m_screen->configure(width, height * 262 / 240, visarea, m_screen->frame_period());
	}

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)        // 8-bit mode
	{
		uint8_t const *const framebuf = (uint8_t const *)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];
		uint8_t const *const pal = m_pal;

		for (int j=0; j < m_frame_height; j++)
		{
			uint32_t *const p = &bitmap.pix(j);
			uint8_t const *si = &framebuf[j * line_delta];
			for (int i=0; i < m_frame_width; i++)
			{
				int const c = *si++;
				int const r = pal[(c*3)+0] << 2;
				int const g = pal[(c*3)+1] << 2;
				int const b = pal[(c*3)+2] << 2;

				p[i] = r << 16 | g << 8 | b;
			}
		}
	}
	else            // 16-bit
	{
		uint16_t const *const framebuf = (uint16_t const *)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		// RGB 5-6-5 mode
		if ((m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x2) == 0)
		{
			for (int j=0; j < m_frame_height; j++)
			{
				uint32_t *const p = &bitmap.pix(j);
				uint16_t const *si = &framebuf[j * (line_delta/2)];
				for (int i=0; i < m_frame_width; i++)
				{
					uint16_t const c = *si++;
					int const r = ((c >> 11) & 0x1f) << 3;
					int const g = ((c >> 5) & 0x3f) << 2;
					int const b = (c & 0x1f) << 3;

					p[i] = r << 16 | g << 8 | b;
				}
			}
		}
		// RGB 5-5-5 mode
		else
		{
			for (int j=0; j < m_frame_height; j++)
			{
				uint32_t *const p = &bitmap.pix(j);
				uint16_t const *si = &framebuf[j * (line_delta/2)];
				for (int i=0; i < m_frame_width; i++)
				{
					uint16_t const c = *si++;
					int const r = ((c >> 10) & 0x1f) << 3;
					int const g = ((c >> 5) & 0x1f) << 3;
					int const b = (c & 0x1f) << 3;

					p[i] = r << 16 | g << 8 | b;
				}
			}
		}
	}
}

void mediagx_state::draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *const gfx = m_gfxdecode->gfx(0);
	uint32_t const *const cga = m_cga_ram;
	int index = 0;

	for (int j=0; j < 25; j++)
	{
		for (int i=0; i < 80; i+=2)
		{
			int const att0 = (cga[index] >> 8) & 0xff;
			int const ch0 = (cga[index] >> 0) & 0xff;
			int const att1 = (cga[index] >> 24) & 0xff;
			int const ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
		}
	}
}

uint32_t mediagx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_framebuffer( bitmap, cliprect);

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)   // don't show MDA text screen on 16-bit mode. this is basically a hack
	{
		draw_cga(bitmap, cliprect);
	}
	return 0;
}

uint32_t mediagx_state::disp_ctrl_r(offs_t offset)
{
	uint32_t r = m_disp_ctrl_reg[offset];

	switch (offset)
	{
		case DC_TIMING_CFG:
			// VNA is high outside the active display (MediaGX DC_TIMING_CFG).
			r = (r & ~0x40000000U) | (m_screen->vpos() >= m_frame_height ? 0x40000000U : 0);

#if SPEEDUP_HACKS
			if (!machine().side_effects_disabled())
			{
				const uint32_t pc = m_maincpu->pc();
				const uint32_t vna = r & 0x40000000U;
				// HD 2.01 polls both edges of VNA. A repeated read at the same
				// instruction with unchanged status means its loop is still waiting.
				// Leave first reads and successful edge reads unsuspended. Wake on
				// either an IRQ or the raster edge, without asserting a guest IRQ.
				if ((pc == 0x364d1 || pc == 0x364f8) && pc == m_video_poll_pc && vna == m_video_poll_vna &&
					m_main_ram[0x364d0 / 4] == 0x00002508 && m_main_ram[0x364f8 / 4] == 0x00000025 &&
					m_frame_height > 0 && m_frame_height < m_screen->height())
				{
					m_video_poll_timer->adjust(m_screen->time_until_pos(vna ? 0 : m_frame_height));
					m_maincpu->spin_until_interrupt();
				}
				m_video_poll_pc = pc;
				m_video_poll_vna = vna;
			}
#endif
			break;
		case DC_V_LINE_CNT:
			r = m_screen->vpos() & 0x7ff;
			break;
	}

	return r;
}

void mediagx_state::disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == DC_V_LINE_CNT)
		return;
//  printf("disp_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	COMBINE_DATA(m_disp_ctrl_reg + offset);
#if SPEEDUP_HACKS
	m_video_poll_pc = 0;
#endif
}


uint32_t mediagx_state::memory_ctrl_r(offs_t offset)
{
	return m_memory_ctrl_reg[offset];
}

void mediagx_state::memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("memory_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	if (offset == 0x20/4)
	{
		if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00e00000) == 0x00400000)
		{
			// guess: crtc params?
			// ...
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00000000)
		{
			m_pal_index = data;
			m_ramdac->index_w(data);
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00100000)
		{
			m_pal[m_pal_index] = data & 0xff;
			m_pal_index++;
			if (m_pal_index >= 768)
				m_pal_index = 0;

			m_ramdac->pal_w(data);
		}
	}
	else
	{
		COMBINE_DATA(m_memory_ctrl_reg + offset);
	}
}



uint32_t mediagx_state::biu_ctrl_r(offs_t offset)
{
	if (offset == 0)
	{
		return 0xffffff;
	}
	return m_biu_ctrl_reg[offset];
}

void mediagx_state::biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//osd_printf_debug("biu_ctrl_w %08X, %08X, %08X\n", data, offset, mem_mask);
	COMBINE_DATA(m_biu_ctrl_reg + offset);

	if (offset == 3)        // BC_XMAP_3 register
	{
		//osd_printf_debug("BC_XMAP_3: %08X, %08X, %08X\n", data, offset, mem_mask);
	}
}

void mediagx_state::bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

uint8_t mediagx_state::io20_r(offs_t offset)
{
	uint8_t r = 0;

	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
	}
	else if (offset == 0x01)
	{
		r = m_mediagx_config_regs[m_mediagx_config_reg_sel];
	}
	return r;
}

void mediagx_state::io20_w(offs_t offset, uint8_t data)
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
		m_mediagx_config_reg_sel = data;
	}
	else if (offset == 0x01)
	{
		m_mediagx_config_regs[m_mediagx_config_reg_sel] = data;
	}
}

void mediagx_state::jammit_output_w(uint16_t data)
{
	// Keep the existing cabinet output names at the machine root.
	for (unsigned bit = 0; bit < 14; ++bit)
		m_jammit_outputs[bit] = BIT(data, bit);
}

// Convert host aim to the existing JAMMIT beam-counter units.  The peripheral
// samples both players once at the beginning of each input packet.
uint32_t mediagx_state::jammit_gun_r(offs_t player)
{
	const uint8_t triggers = m_triggers->read();
	const unsigned htotal = ((m_disp_ctrl_reg[DC_H_TIMING_1] >> 16) & 0x7ff) + 8;
	// Horizontal gun counter is referenced to the leading edge of HSYNC.
	const unsigned hsyncstart = (m_disp_ctrl_reg[DC_H_TIMING_3] & 0x7ff) + 8;
	const unsigned backporch = htotal > hsyncstart ? htotal - hsyncstart : 0;
	const unsigned pixel_double = BIT(m_disp_ctrl_reg[DC_TIMING_CFG], 15) ? 2 : 1;
	const double counts_per_line = m_screen->scan_period().as_double() * 14'318'180;
	const unsigned x = m_guns[player * 2]->read();
	const unsigned y = m_guns[player * 2 + 1]->read();
	const bool offscreen = BIT(triggers, player + 4) || x == 0 || x == 255 || y == 0 || y == 255;
	const unsigned xpos = x * std::max(m_frame_width - 1, 0) / 255;
	const unsigned ypos = y * std::max(m_frame_height - 1, 0) / 255;
	const unsigned vertical = ~ypos & 0x0fff;
	const unsigned horizontal = std::min<unsigned>(1023, (backporch + xpos * pixel_double) * counts_per_line / htotal);
	return vertical | (horizontal << 12) | (BIT(triggers, player) << 24)
		| (BIT(triggers, player + 4) << 24) | (unsigned(offscreen) << 25);
}

/* Analog Devices AD1847 Stereo DAC */

uint32_t mediagx_state::ad1847_fifo_level(bool update)
{
	const uint64_t sample_tick = m_ad1847_sample_rate ? machine().time().as_ticks(m_ad1847_sample_rate) : 0;
	const uint64_t elapsed = sample_tick >= m_audio_fifo_sample_tick ? sample_tick - m_audio_fifo_sample_tick : 0;
	const uint32_t level = m_audio_fifo_count - std::min<uint64_t>(m_audio_fifo_count, elapsed);
	if (update)
	{
		m_audio_fifo_count = level;
		m_audio_fifo_sample_tick = sample_tick;
	}
	return level;
}

void mediagx_state::ad1847_reg_write(int reg, uint8_t data)
{
	static const int divide_factor[] = { 3072, 1536, 896, 768, 448, 384, 512, 2560 };

	switch (reg)
	{
		case 8:     // Data format register
		{
			ad1847_fifo_level();
			if (data & 0x1)
			{
				m_ad1847_sample_rate = 16934400 / divide_factor[(data >> 1) & 0x7];
			}
			else
			{
				m_ad1847_sample_rate = 24576000 / divide_factor[(data >> 1) & 0x7];
			}

			m_audio_fifo_sample_tick = machine().time().as_ticks(m_ad1847_sample_rate);
			m_dmadac[0]->set_frequency(m_ad1847_sample_rate);
			m_dmadac[1]->set_frequency(m_ad1847_sample_rate);

			if (data & 0x20)
			{
				fatalerror("AD1847: Companded data not supported\n");
			}
			if ((data & 0x40) == 0)
			{
				fatalerror("AD1847: 8-bit data not supported\n");
			}
			break;
		}

		default:
		{
			m_ad1847_regs[reg] = data;
			break;
		}
	}
}

uint32_t mediagx_state::ad1847_r(offs_t offset)
{
	switch (offset)
	{
		case 0x14/4:
			// Bits 8-13 report occupancy of the 32-entry stereo FIFO.
			// Advance on the native sample clock, retaining fractional periods.
			return ad1847_fifo_level(!machine().side_effects_disabled()) << 8;
	}
	return 0;
}

void mediagx_state::ad1847_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (!mem_mask || ad1847_fifo_level() >= 32)
			return;
		// Consume the previous FIFO contents up to this write before adding
		// new samples. A periodic 10 ms batch loses the FIFO's clock feedback.
		m_dmadac[0]->flush();
		m_dmadac[1]->flush();
		if (ACCESSING_BITS_16_31)
		{
			int16_t ldata = data >> 16;
			m_dmadac[0]->transfer(0, 0, 1, 1, &ldata);
		}
		if (ACCESSING_BITS_0_15)
		{
			int16_t rdata = data;
			m_dmadac[1]->transfer(0, 0, 1, 1, &rdata);
		}

		m_audio_fifo_count++;
	}
	else if (offset == 3)
	{
		int reg = (data >> 8) & 0xf;
		ad1847_reg_write(reg, data & 0xff);
	}
}


/*****************************************************************************/

void mediagx_state::mediagx_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram().share(m_main_ram);
	map(0x000a0000, 0x000affff).ram();
	map(0x000b0000, 0x000b7fff).ram().share(m_cga_ram);
	map(0x000c0000, 0x000fffff).ram().share(m_bios_ram);
	map(0x00100000, 0x00ffffff).ram();
	map(0x40008000, 0x400080ff).rw(FUNC(mediagx_state::biu_ctrl_r), FUNC(mediagx_state::biu_ctrl_w));
	map(0x40008300, 0x400083ff).rw(FUNC(mediagx_state::disp_ctrl_r), FUNC(mediagx_state::disp_ctrl_w));
	map(0x40008400, 0x400084ff).rw(FUNC(mediagx_state::memory_ctrl_r), FUNC(mediagx_state::memory_ctrl_w));
	map(0x40800000, 0x40bfffff).ram().share(m_vram);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void mediagx_state::mediagx_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x0022, 0x0023).rw(FUNC(mediagx_state::io20_r), FUNC(mediagx_state::io20_w));
	map(0x00e8, 0x00eb).noprw();     // I/O delay port
	map(0x01f0, 0x01f7).rw(m_ide, FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x0378, 0x037b).m("jammit_lpt", FUNC(pc_lpt_device::isa_map));
	map(0x03bc, 0x03bf).rw(m_lpt0, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write)).umask16(0x00ff);
	map(0x03f0, 0x03f7).rw(m_ide, FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x0400, 0x04ff).rw(FUNC(mediagx_state::ad1847_r), FUNC(mediagx_state::ad1847_w));
}

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,                    /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8,1*8,2*8,3*8,
		4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_cga )
/* Support up to four CGA fonts */
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout, 0, 256 )   /* Font 0 */
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout, 0, 256 )   /* Font 1 */
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout, 0, 256 )   /* Font 2 */
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout, 0, 256 )   /* Font 3*/
GFXDECODE_END

static INPUT_PORTS_START(mediagx)
	PORT_START("SYSTEM")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_COIN3)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_COIN4)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_TILT)
	PORT_SERVICE_NO_TOGGLE(0x020, IP_ACTIVE_HIGH) PORT_NAME("Test / Operator Menu")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_BILL1)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_SERVICE2)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN)
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_VOLUME_UP)

	PORT_START("STARTS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x0c, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TRIGGERS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Offscreen Shot")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Offscreen Shot")
	PORT_BIT(0xcc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("GUN0")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_X) PORT_MINMAX(0, 255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_PLAYER(1)
	PORT_START("GUN1")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_Y) PORT_MINMAX(0, 255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_PLAYER(1)
	PORT_START("GUN2")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_X) PORT_MINMAX(0, 255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_PLAYER(2)
	PORT_START("GUN3")
	PORT_BIT(0xff, 0x80, IPT_LIGHTGUN_Y) PORT_MINMAX(0, 255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_PLAYER(2)
INPUT_PORTS_END

void mediagx_state::machine_start()
{
#if SPEEDUP_HACKS
	m_video_poll_timer = timer_alloc(FUNC(mediagx_state::video_poll_end), this);
	save_item(NAME(m_video_poll_pc));
	save_item(NAME(m_video_poll_vna));
#endif
	save_item(NAME(m_audio_fifo_count));
	save_item(NAME(m_audio_fifo_sample_tick));
	save_item(NAME(m_ad1847_sample_rate));
}

void mediagx_state::machine_reset()
{
#if SPEEDUP_HACKS
	m_video_poll_pc = 0;
	m_video_poll_vna = 0;
	m_video_poll_timer->adjust(attotime::never);
#endif
	// HACK: pretend shadow RAM doesn't exist in a x86 BIOS ...
	uint8_t *rom = memregion("bios")->base();
	memcpy(m_bios_ram, rom, 0x40000);
	m_maincpu->reset();

	m_audio_fifo_count = 0;
	m_audio_fifo_sample_tick = m_ad1847_sample_rate ? machine().time().as_ticks(m_ad1847_sample_rate) : 0;
	m_dmadac[0]->enable(0);
	m_dmadac[1]->enable(0);
	m_dmadac[0]->enable(1);
	m_dmadac[1]->enable(1);
}

void mediagx_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void mediagx_state::mediagx(machine_config &config)
{
	/* basic machine hardware */
	MEDIAGX(config, m_maincpu, 166000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mediagx_state::mediagx_map);
	m_maincpu->set_addrmap(AS_IO, &mediagx_state::mediagx_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	// TODO: checked at POST, wants a debug device?
	PC_LPT(config, m_lpt0);
//  m_lpt0->irq_handler().set("mb:pic8259", FUNC(pic8259_device::ir7_w));

	// External Atari JAMMIT board on the mainboard's DB25 parallel connector.
	pc_lpt_device &jammit_lpt(PC_LPT(config, "jammit_lpt"));
	centronics_device &centronics(*jammit_lpt.subdevice<centronics_device>("centronics"));
	centronics.option_add_internal("atari_jammit", ATARI_JAMMIT);
	centronics.set_default_option("atari_jammit");
	centronics.set_fixed(true);
	centronics.set_option_machine_config("atari_jammit", [this] (device_t *device)
	{
		auto &jammit = downcast<atari_jammit_device &>(*device);
		jammit.system_callback().set_ioport(":SYSTEM");
		jammit.starts_callback().set_ioport(":STARTS");
		jammit.gun_callback().set(*this, FUNC(mediagx_state::jammit_gun_r));
		jammit.output_callback().set(*this, FUNC(mediagx_state::jammit_output_w));
	});

	// Cabinet PS/2 connectors are unused. Keep the 8042, but do not attach
	// a PC keyboard: its input ports put MAME in full keyboard mode and
	// disable the default UI shortcuts (including Tab).
	pcat_common_nokeyboard(config);

	PCI_ROOT(config, "pci");
	CX5510(config, "pci:12.0", m_maincpu);

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set(m_pic8259_2, FUNC(pic8259_device::ir6_w));

	RAMDAC(config, m_ramdac, m_palette);
	m_ramdac->set_addrmap(0, &mediagx_state::ramdac_map);

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 639, 0, 239);
	m_screen->set_screen_update(FUNC(mediagx_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cga);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}


void mediagx_state::init_mediagx()
{
	m_frame_width = m_frame_height = 1;
	std::fill(std::begin(m_disp_ctrl_reg), std::end(m_disp_ctrl_reg), 0);
	std::fill(std::begin(m_biu_ctrl_reg), std::end(m_biu_ctrl_reg), 0);
	std::fill(std::begin(m_speedup_hits), std::end(m_speedup_hits), 0);
}

TIMER_CALLBACK_MEMBER(mediagx_state::video_poll_end)
{
	// Release a polling wait only; this is not a hardware interrupt.
	m_maincpu->signal_interrupt_trigger();
}

#if SPEEDUP_HACKS

uint32_t mediagx_state::frame_counter_r()
{
	const uint32_t counter = m_main_ram[0x8c2ec / 4];
	const uint32_t pc = m_maincpu->pc();
	const uint32_t elapsed = counter - m_main_ram[0x6cfa0 / 4];
	// HD 2.01 waits for two/four vertical ticks before starting the next frame.
	// The tick is advanced by the game's interrupt handler. Yield only while
	// the observed unsigned loop condition is true, preserving IRQ execution.
	if (!machine().side_effects_disabled() &&
		m_main_ram[0x54c4 / 4] == 0x0006cfcc &&
		m_main_ram[0x54c8 / 4] == 0x290c408b &&
		((pc == 0x54cb && elapsed < 4) || (pc == 0x54b4 && elapsed < 2)))
		m_maincpu->spin_until_interrupt();
	return counter;
}

uint32_t mediagx_state::generic_speedup(address_space &space, int idx)
{
	const uint32_t value = m_main_ram[m_speedup_table[idx].offset / 4];
	// Only yield while the observed loop condition is still true.
	const bool waiting = m_speedup_table[idx].pc == 0x363e ? value == 0 : value != 0x65;
	if (!machine().side_effects_disabled() && waiting && m_maincpu->pc() == m_speedup_table[idx].pc)
	{
		m_speedup_hits[idx]++;
		space.device().execute().spin_until_interrupt();
	}
	return value;
}

const mediagx_state::speedup_handler mediagx_state::s_speedup_handlers[]
{
	{ FUNC(mediagx_state::speedup_r<0>) }, { FUNC(mediagx_state::speedup_r<1>) }, { FUNC(mediagx_state::speedup_r<2>)  }, { FUNC(mediagx_state::speedup_r<3>)  },
	{ FUNC(mediagx_state::speedup_r<4>) }, { FUNC(mediagx_state::speedup_r<5>) }, { FUNC(mediagx_state::speedup_r<6>)  }, { FUNC(mediagx_state::speedup_r<7>)  },
	{ FUNC(mediagx_state::speedup_r<8>) }, { FUNC(mediagx_state::speedup_r<9>) }, { FUNC(mediagx_state::speedup_r<10>) }, { FUNC(mediagx_state::speedup_r<11>) }
};

#ifdef MAME_DEBUG
void mediagx_state::report_speedups()
{
	for (int i = 0; i < m_speedup_count; i++)
		printf("Speedup %2d: offs=%06X pc=%06X hits=%d\n", i, m_speedup_table[i].offset, m_speedup_table[i].pc, m_speedup_hits[i]);
}
#endif

void mediagx_state::install_speedups(const speedup_entry *entries, int count)
{
	assert(count < std::size(s_speedup_handlers));

	m_speedup_table = entries;
	m_speedup_count = count;

	for (int i = 0; i < count; i++)
		m_maincpu->space(AS_PROGRAM).install_read_handler(entries[i].offset, entries[i].offset + 3, read32mo_delegate(*this, s_speedup_handlers[i].first, s_speedup_handlers[i].second));

#ifdef MAME_DEBUG
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&mediagx_state::report_speedups, this));
#endif
}

const mediagx_state::speedup_entry mediagx_state::a51site4_speedups[] =
{
	{ 0x5504c, 0x0363e },
	{ 0x5f11c, 0x0363e },
	{ 0x5cac8, 0x0363e },
	{ 0x560fc, 0x0363e },
	{ 0x55298, 0x0363e },
	{ 0x63a88, 0x049d9 },
	{ 0x5e01c, 0x049d9 },
	{ 0x5e3ec, 0x049d9 },
	{ 0x60504, 0x1c91d },
	{ 0x60440, 0x1c8c4 },
};

#endif

void mediagx_state::init_a51site4()
{
	init_mediagx();

#if SPEEDUP_HACKS
	install_speedups(a51site4_speedups, std::size(a51site4_speedups));
	if (!strcmp(machine().system().name, "a51site4"))
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8c2ec, 0x8c2ef, read32smo_delegate(*this, FUNC(mediagx_state::frame_counter_r)));
#endif
}

/*****************************************************************************/

ROM_START( a51site4 )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "new", "v1.0h" )
	ROMX_LOAD("a51s4_bios_09-15-98.u1", 0x00000, 0x40000, CRC(f8cd6a6b) SHA1(75f851ae21517b729a5596ce5e042ebfaac51778), ROM_BIOS(0)) /* Build date 09/15/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(1, "old", "v1.0f" )
	ROMX_LOAD("a51s4_bios_07-11-98.u1", 0x00000, 0x40000, CRC(5ee189cc) SHA1(0b0d9321a4c59b1deea6854923e655a4d8c4fcfe), ROM_BIOS(1)) /* Build date 07/11/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(2, "older", "v1.0d" ) /* doesn't work with the HDs currently available, shows "FOR EVALUATION ONLY" */
	ROMX_LOAD("a51s4_bios_04-22-98.u1", 0x00000, 0x40000, CRC(2008bfc6) SHA1(004bec8759fb04d375c6efc49d048693d1f871ee), ROM_BIOS(2)) /* Build date 04/22/98 string stored at 0x3fff5 */

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "a51site4-2_01", 0, SHA1(48496666d1613700ae9274f9a5361ea5bbaebea0) ) /* Happ replacement drive "A-22509", sticker on drive shows REV 2.01 and Test Mode screen shows the date September 7, 1998 */
ROM_END

ROM_START( a51site4a ) /* When dumped connected straight to IDE the cylinders were 4968 and heads were 16 */
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "new", "v1.0h" )
	ROMX_LOAD("a51s4_bios_09-15-98.u1", 0x00000, 0x40000, CRC(f8cd6a6b) SHA1(75f851ae21517b729a5596ce5e042ebfaac51778), ROM_BIOS(0)) /* Build date 09/15/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(1, "old", "v1.0f" )
	ROMX_LOAD("a51s4_bios_07-11-98.u1", 0x00000, 0x40000, CRC(5ee189cc) SHA1(0b0d9321a4c59b1deea6854923e655a4d8c4fcfe), ROM_BIOS(1)) /* Build date 07/11/98 string stored at 0x3fff5 */
	ROM_SYSTEM_BIOS(2, "older", "v1.0d" ) /* doesn't work with the HDs currently available, shows "FOR EVALUATION ONLY" */
	ROMX_LOAD("a51s4_bios_04-22-98.u1", 0x00000, 0x40000, CRC(2008bfc6) SHA1(004bec8759fb04d375c6efc49d048693d1f871ee), ROM_BIOS(2)) /* Build date 04/22/98 string stored at 0x3fff5, doesn't work */

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "a51site4-2_0", 0, SHA1(4de421e4d1708ecbdfb50730000814a1ea36a044) ) /* Stock drive, sticker on drive shows REV 2.0 and Test Mode screen shows the date September 11, 1998 */
ROM_END

} // Anonymous namespace


/*****************************************************************************/

GAME( 1998, a51site4, 0       , mediagx, mediagx, mediagx_state, init_a51site4,  ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.01, September 7, 1998)", MACHINE_NOT_WORKING )
GAME( 1998, a51site4a,a51site4, mediagx, mediagx, mediagx_state, init_a51site4,  ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.0, September 11, 1998)", MACHINE_NOT_WORKING )
