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

#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "machine/timer.h"
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
		m_ports(*this, "IN%u", 0U)
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
	uint32_t parallel_port_r(offs_t offset, uint32_t mem_mask = ~0);
	void parallel_port_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ad1847_r(offs_t offset);
	void ad1847_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	[[maybe_unused]] void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t io20_r(offs_t offset);
	void io20_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <offs_t N> uint32_t speedup_r(address_space &space) { return generic_speedup(space, N); }
	TIMER_DEVICE_CALLBACK_MEMBER(sound_timer_callback);
	void draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y);
	void draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void ad1847_reg_write(int reg, uint8_t data);
	inline uint32_t generic_speedup(address_space &space, int idx);
	void report_speedups();
	void install_speedups(const speedup_entry *entries, int count);
	void init_mediagx();
	void mediagx_io(address_map &map) ATTR_COLD;
	void mediagx_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	uint32_t cx5510_pci_r(int function, int reg, uint32_t mem_mask)
	{
		//osd_printf_debug("CX5510: PCI read %d, %02X, %08X\n", function, reg, mem_mask);
		switch (reg)
		{
			case 0:     return 0x00001078;
		}

		return m_cx5510_regs[reg/4];
	}

	void cx5510_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
	{
		//osd_printf_debug("CX5510: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
		COMBINE_DATA(&m_cx5510_regs[reg/4]);
	}

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

	optional_ioport_array<9> m_ports;   // but parallel_pointer takes values 0 -> 23

	uint32_t m_disp_ctrl_reg[256/4]{};
	int m_frame_width = 0;
	int m_frame_height = 0;

	uint32_t m_memory_ctrl_reg[256/4]{};
	int m_pal_index = 0;

	uint32_t m_biu_ctrl_reg[256/4]{};

	uint8_t m_mediagx_config_reg_sel = 0;
	uint8_t m_mediagx_config_regs[256]{};

	//uint8_t m_controls_data = 0;
	uint8_t m_parallel_pointer = 0;
	uint8_t m_parallel_latched = 0;
	uint32_t m_parport = 0;
	//int m_control_num = 0;
	//int m_control_num2 = 0;
	//int m_control_read = 0;

	uint32_t m_cx5510_regs[256/4]{};

	std::unique_ptr<int16_t[]> m_dacl;
	std::unique_ptr<int16_t[]> m_dacr;
	int m_dacl_ptr = 0;
	int m_dacr_ptr = 0;

	uint8_t m_ad1847_regs[16]{};
	uint32_t m_ad1847_sample_counter = 0;
	uint32_t m_ad1847_sample_rate = 0;

#if SPEEDUP_HACKS
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
		m_screen->configure(width, height * 262 / 240, visarea, m_screen->frame_period().attoseconds());
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
			r |= 0x40000000;

			if (m_screen->vpos() >= m_frame_height)
				r &= ~0x40000000;

#if SPEEDUP_HACKS
			// wait for vblank speedup
			m_maincpu->spin_until_interrupt();
#endif
			break;
	}

	return r;
}

void mediagx_state::disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("disp_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	COMBINE_DATA(m_disp_ctrl_reg + offset);
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

uint32_t mediagx_state::parallel_port_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_8_15)
	{
		uint8_t nibble = m_parallel_latched;
		r |= ((~nibble & 0x08) << 12) | ((nibble & 0x07) << 11);
		logerror("%08X:parallel_port_r()\n", m_maincpu->pc());
#if 0
		if (m_controls_data == 0x18)
		{
			r |= m_ports[0]->read() << 8;
		}
		else if (m_controls_data == 0x60)
		{
			r |= m_ports[1]->read() << 8;
		}
		else if (m_controls_data == 0xff || m_controls_data == 0x50)
		{
			r |= m_ports[2]->read() << 8;
		}

		//r |= m_control_read << 8;
#endif
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= m_parport & 0xff0000;
	}

	return r;
}

void mediagx_state::parallel_port_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA( &m_parport );

	if (ACCESSING_BITS_0_7)
	{
		/*
		    Controls:

		        18 = reset internal pointer to 0
		        19 = reset internal pointer to 1
		        1a = reset internal pointer to 2
		        1b = reset internal pointer to 3
		        2x = set low 4 bits of general purpose output to 'x'
		        3x = set high 4 bits of general purpose output to 'x'
		        4x = control up to 4 coin counters; each bit of 'x' controls one
		        5x = control up to 2 watchdogged outputs (kickers); bits D0-D1 control each one
		        6x = watchdog reset
		        7x..ff = advance pointer
		*/

		logerror("%08X:", m_maincpu->pc());

		m_parallel_latched = (m_ports[m_parallel_pointer / 3].read_safe(0) >> (4 * (m_parallel_pointer % 3))) & 15;
		//parallel_pointer++;
		//logerror("[%02X] Advance pointer to %d\n", data, parallel_pointer);
		switch (data & 0xfc)
		{
			case 0x18:
				m_parallel_pointer = data & 3;
				logerror("[%02X] Reset pointer to %d\n", data, m_parallel_pointer);
				break;

			case 0x20:
			case 0x24:
			case 0x28:
			case 0x2c:
				logerror("[%02X] General purpose output = x%X\n", data, data & 0x0f);
				break;

			case 0x30:
			case 0x34:
			case 0x38:
			case 0x3c:
				logerror("[%02X] General purpose output = %Xx\n", data, data & 0x0f);
				break;

			case 0x40:
			case 0x44:
			case 0x48:
			case 0x4c:
				logerror("[%02X] Coin counters = %d%d%d%d\n", data, (data >> 3) & 1, (data >> 2) & 1, (data >> 1) & 1, data & 1);
				break;

			case 0x50:
			case 0x54:
			case 0x58:
			case 0x5c:
				logerror("[%02X] Kickers = %d%d\n", data, (data >> 1) & 1, data & 1);
				break;

			case 0x60:
			case 0x64:
			case 0x68:
			case 0x6c:
				logerror("[%02X] Watchdog reset\n", data);
				break;

			default:
				if (data >= 0x70)
				{
					m_parallel_pointer++;
					logerror("[%02X] Advance pointer to %d\n", data, m_parallel_pointer);
				}
				else
					logerror("[%02X] Unknown write\n", data);
				break;
		}
	}
}

/* Analog Devices AD1847 Stereo DAC */

TIMER_DEVICE_CALLBACK_MEMBER(mediagx_state::sound_timer_callback)
{
	m_ad1847_sample_counter = 0;
	timer.adjust(attotime::from_msec(10));

	m_dmadac[0]->transfer(1, 0, 1, m_dacl_ptr, m_dacl.get());
	m_dmadac[1]->transfer(1, 0, 1, m_dacr_ptr, m_dacr.get());

	m_dacl_ptr = 0;
	m_dacr_ptr = 0;
}

void mediagx_state::ad1847_reg_write(int reg, uint8_t data)
{
	static const int divide_factor[] = { 3072, 1536, 896, 768, 448, 384, 512, 2560 };

	switch (reg)
	{
		case 8:     // Data format register
		{
			if (data & 0x1)
			{
				m_ad1847_sample_rate = 16934400 / divide_factor[(data >> 1) & 0x7];
			}
			else
			{
				m_ad1847_sample_rate = 24576000 / divide_factor[(data >> 1) & 0x7];
			}

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
			return ((m_ad1847_sample_rate) / 100) - m_ad1847_sample_counter;
	}
	return 0;
}

void mediagx_state::ad1847_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_16_31)
		{
			uint16_t ldata = (data >> 16) & 0xffff;
			m_dacl[m_dacl_ptr++] = ldata;
		}
		if (ACCESSING_BITS_0_15)
		{
			uint16_t rdata = data & 0xffff;
			m_dacr[m_dacr_ptr++] = rdata;
		}

		m_ad1847_sample_counter++;
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
	map(0x0378, 0x037b).rw(FUNC(mediagx_state::parallel_port_r), FUNC(mediagx_state::parallel_port_w));
	map(0x03bc, 0x03bf).rw(m_lpt0, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write)).umask16(0x00ff);
	map(0x03f0, 0x03f7).rw(m_ide, FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x0400, 0x04ff).rw(FUNC(mediagx_state::ad1847_r), FUNC(mediagx_state::ad1847_w));
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
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
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x001, IP_ACTIVE_HIGH )
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x800, IP_ACTIVE_HIGH, IPT_START4 )

	PORT_START("IN1")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("IN2")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON6 )

	PORT_START("IN3")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON8 )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON9 )

	PORT_START("IN4")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN5")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)

	PORT_START("IN6")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN7")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN8")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
INPUT_PORTS_END

void mediagx_state::machine_start()
{
	m_dacl = std::make_unique<int16_t[]>(65536);
	m_dacr = std::make_unique<int16_t[]>(65536);
}

void mediagx_state::machine_reset()
{
	uint8_t *rom = memregion("bios")->base();
	memcpy(m_bios_ram, rom, 0x40000);
	m_maincpu->reset();

	timer_device *sound_timer = subdevice<timer_device>("sound_timer");
	sound_timer->adjust(attotime::from_msec(10));

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

	pcat_common(config);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device(18, FUNC(mediagx_state::cx5510_pci_r), FUNC(mediagx_state::cx5510_pci_w));

	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set(m_pic8259_2, FUNC(pic8259_device::ir6_w));

	TIMER(config, "sound_timer").configure_generic(FUNC(mediagx_state::sound_timer_callback));

	RAMDAC(config, m_ramdac, 0, m_palette);
	m_ramdac->set_addrmap(0, &mediagx_state::ramdac_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
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
	m_parallel_pointer = 0;
	std::fill(std::begin(m_disp_ctrl_reg), std::end(m_disp_ctrl_reg), 0);
	std::fill(std::begin(m_biu_ctrl_reg), std::end(m_biu_ctrl_reg), 0);
	std::fill(std::begin(m_speedup_hits), std::end(m_speedup_hits), 0);
}

#if SPEEDUP_HACKS

uint32_t mediagx_state::generic_speedup(address_space &space, int idx)
{
	if (m_maincpu->pc() == m_speedup_table[idx].pc)
	{
		m_speedup_hits[idx]++;
		space.device().execute().spin_until_interrupt();
	}
	return m_main_ram[m_speedup_table[idx].offset/4];
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
