// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie
/*****************************************************************************************************************
PINBALL
Williams Pinball 2000

Skeleton by R. Belmont, based on mediagx.c by Ville Linde

Hardware:
- Cyrix MediaGX processor/VGA (northbridge)
- Cyrix CX5520 (southbridge)
- VS9824AG SuperI/O standard PC I/O chip
- 1 ISA, 2 PCI slots, 2 IDE headers
- "Prism" PCI card with PLX PCI9052 PCI-to-random stuff bridge
   Card also contains DCS2 Stereo sound system with ADSP-2104

Games:
- Star Wars Episode 1 (#50069)
- Revenge from Mars (#50070)
- Wizard Blocks (#50072) (cancelled)
- Playboy (cancelled)

Status:
- Skeletons

TODO:
- Everything!
- MediaGX features should be moved out to machine/ and shared with mediagx.c once we know what these games need

****************************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/idectrl.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/pcshare.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pinball2k_state : public pcat_base_state
{
public:
	pinball2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_main_ram(*this, "main_ram"),
		m_cga_ram(*this, "cga_ram"),
		m_bios_ram(*this, "bios_ram"),
		m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_ramdac(*this, "ramdac"),
		m_palette(*this, "palette") { }

	void mediagx(machine_config &config);

	void init_mediagx();
	void init_pinball2k();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint32_t> m_cga_ram;
	required_shared_ptr<uint32_t> m_bios_ram;
	required_shared_ptr<uint32_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<ramdac_device> m_ramdac;
	required_device<palette_device> m_palette;
	uint8_t m_pal[768]{};


	uint32_t m_disp_ctrl_reg[256/4]{};
	int m_frame_width = 0;
	int m_frame_height = 0;

	uint32_t m_memory_ctrl_reg[256/4]{};
	int m_pal_index = 0;

	uint32_t m_biu_ctrl_reg[256/4]{};

	uint8_t m_mediagx_config_reg_sel = 0;
	uint8_t m_mediagx_config_regs[256]{};

	//uint8_t m_controls_data = 0U;
	//uint8_t m_parallel_pointer = 0U;
	//uint8_t m_parallel_latched = 0U;
	//uint32_t m_parport = 0U;
	//int m_control_num = 0;
	//int m_control_num2 = 0;
	//int m_control_read = 0;

	uint32_t m_cx5510_regs[256/4]{};

	uint32_t disp_ctrl_r(offs_t offset);
	void disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t memory_ctrl_r(offs_t offset);
	void memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t biu_ctrl_r(offs_t offset);
	void biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	[[maybe_unused]] void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t parallel_port_r();
	void parallel_port_w(uint32_t data);
	uint8_t io20_r(offs_t offset);
	void io20_w(offs_t offset, uint8_t data);
	uint32_t port400_r();
	void port400_w(uint32_t data);
	uint32_t port800_r();
	void port800_w(uint32_t data);

	uint32_t screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y);
	void draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mediagx_io(address_map &map);
	void mediagx_map(address_map &map);
	void ramdac_map(address_map &map);

	uint32_t cx5510_pci_r(int function, int reg, uint32_t mem_mask);
	void cx5510_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
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

void pinball2k_state::video_start()
{
	for (u8 i=0; i < 16; i++)
		m_palette->set_pen_color(i, cga_palette[i]);
}

void pinball2k_state::draw_char(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, int ch, int att, int x, int y)
{
	int index = 0;
	pen_t const *const pens = m_palette->pens();

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

void pinball2k_state::draw_framebuffer(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int line_delta = (m_disp_ctrl_reg[DC_LINE_DELTA] & 0x3ff) * 4;

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
		uint8_t const *const framebuf = (uint8_t*)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];
		uint8_t const *const pal = m_pal;

		for (int j=0; j < m_frame_height; j++)
		{
			uint32_t *const p = &bitmap.pix(j);
			uint8_t const *si = &framebuf[j * line_delta];
			for (int i=0; i < m_frame_width; i++)
			{
				int c = *si++;
				int r = pal[(c*3)+0] << 2;
				int g = pal[(c*3)+1] << 2;
				int b = pal[(c*3)+2] << 2;

				p[i] = r << 16 | g << 8 | b;
			}
		}
	}
	else            // 16-bit
	{
		uint16_t const *const framebuf = (uint16_t*)&m_vram[m_disp_ctrl_reg[DC_FB_ST_OFFSET]/4];

		// RGB 5-6-5 mode
		if ((m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x2) == 0)
		{
			for (int j=0; j < m_frame_height; j++)
			{
				uint32_t *const p = &bitmap.pix(j);
				uint16_t const *si = &framebuf[j * (line_delta/2)];
				for (int i=0; i < m_frame_width; i++)
				{
					uint16_t c = *si++;
					int r = ((c >> 11) & 0x1f) << 3;
					int g = ((c >> 5) & 0x3f) << 2;
					int b = (c & 0x1f) << 3;

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
					uint16_t c = *si++;
					int r = ((c >> 10) & 0x1f) << 3;
					int g = ((c >> 5) & 0x1f) << 3;
					int b = (c & 0x1f) << 3;

					p[i] = r << 16 | g << 8 | b;
				}
			}
		}
	}
}

void pinball2k_state::draw_cga(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint32_t const *const cga = m_cga_ram;
	int index = 0;

	for (int j=0; j < 25; j++)
	{
		for (int i=0; i < 80; i+=2)
		{
			int att0 = (cga[index] >> 8) & 0xff;
			int ch0 = (cga[index] >> 0) & 0xff;
			int att1 = (cga[index] >> 24) & 0xff;
			int ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i*8, j*8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, (i*8)+8, j*8);
			index++;
		}
	}
}

uint32_t pinball2k_state::screen_update_mediagx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_framebuffer( bitmap, cliprect);

	if (m_disp_ctrl_reg[DC_OUTPUT_CFG] & 0x1)   // don't show MDA text screen on 16-bit mode. this is basically a hack
	{
		draw_cga(bitmap, cliprect);
	}
	return 0;
}

uint32_t pinball2k_state::disp_ctrl_r(offs_t offset)
{
	uint32_t r = m_disp_ctrl_reg[offset];

	switch (offset)
	{
		case DC_TIMING_CFG:
			r |= 0x40000000;

			if (m_screen->vpos() >= m_frame_height)
				r &= ~0x40000000;
			break;
	}

	return r;
}

void pinball2k_state::disp_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("disp_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	COMBINE_DATA(m_disp_ctrl_reg + offset);
}


uint32_t pinball2k_state::memory_ctrl_r(offs_t offset)
{
	return m_memory_ctrl_reg[offset];
}

void pinball2k_state::memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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
			{
				m_pal_index = 0;
			}
			m_ramdac->pal_w(data);
		}
	}
	else
	{
		COMBINE_DATA(m_memory_ctrl_reg + offset);
	}
}



uint32_t pinball2k_state::biu_ctrl_r(offs_t offset)
{
	if (offset == 0)
	{
		return 0xffffff;
	}
	return m_biu_ctrl_reg[offset];
}

void pinball2k_state::biu_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//osd_printf_debug("biu_ctrl_w %08X, %08X, %08X\n", data, offset, mem_mask);
	COMBINE_DATA(m_biu_ctrl_reg + offset);

	if (offset == 3)        // BC_XMAP_3 register
	{
		//osd_printf_debug("BC_XMAP_3: %08X, %08X, %08X\n", data, offset, mem_mask);
	}
}

void pinball2k_state::bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

uint8_t pinball2k_state::io20_r(offs_t offset)
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

void pinball2k_state::io20_w(offs_t offset, uint8_t data)
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

uint32_t pinball2k_state::port400_r()
{
	return 0x8000;
}

void pinball2k_state::port400_w(uint32_t data)
{
}

uint32_t pinball2k_state::port800_r()
{
	return 0x80;
}

void pinball2k_state::port800_w(uint32_t data)
{
}

uint32_t pinball2k_state::parallel_port_r()
{
	uint32_t r = 0;

	return r;
}

void pinball2k_state::parallel_port_w(uint32_t data)
{
}

uint32_t pinball2k_state::cx5510_pci_r(int function, int reg, uint32_t mem_mask)
{
	//osd_printf_debug("CX5510: PCI read %d, %02X, %08X\n", function, reg, mem_mask);
	switch (reg)
	{
		case 0:     return 0x00001078;
	}

	return m_cx5510_regs[reg/4];
}

void pinball2k_state::cx5510_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	//osd_printf_debug("CX5510: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(&m_cx5510_regs[reg/4]);
}

/*****************************************************************************/

void pinball2k_state::mediagx_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram().share("main_ram");
	map(0x000a0000, 0x000affff).ram();
	map(0x000b0000, 0x000b7fff).ram().share("cga_ram");
	map(0x000c0000, 0x000fffff).ram().share("bios_ram");
	map(0x00100000, 0x00ffffff).ram();
	map(0x40008000, 0x400080ff).rw(FUNC(pinball2k_state::biu_ctrl_r), FUNC(pinball2k_state::biu_ctrl_w));
	map(0x40008300, 0x400083ff).rw(FUNC(pinball2k_state::disp_ctrl_r), FUNC(pinball2k_state::disp_ctrl_w));
	map(0x40008400, 0x400084ff).rw(FUNC(pinball2k_state::memory_ctrl_r), FUNC(pinball2k_state::memory_ctrl_w));
	map(0x40800000, 0x40bfffff).ram().share("vram");
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void pinball2k_state::mediagx_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x0022, 0x0023).rw(FUNC(pinball2k_state::io20_r), FUNC(pinball2k_state::io20_w));
	map(0x00e8, 0x00eb).noprw();     // I/O delay port
	map(0x0378, 0x037b).rw(FUNC(pinball2k_state::parallel_port_r), FUNC(pinball2k_state::parallel_port_w));
	map(0x0400, 0x0403).rw(FUNC(pinball2k_state::port400_r), FUNC(pinball2k_state::port400_w));
	map(0x0800, 0x0803).rw(FUNC(pinball2k_state::port800_r), FUNC(pinball2k_state::port800_w));
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
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8                     /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_cga )
	// Support up to four CGA fonts
	GFXDECODE_ENTRY( "gfx1", 0x0000, CGA_charlayout, 0, 256 )   // Font 0
	GFXDECODE_ENTRY( "gfx1", 0x0800, CGA_charlayout, 0, 256 )   // Font 1
	GFXDECODE_ENTRY( "gfx1", 0x1000, CGA_charlayout, 0, 256 )   // Font 2
	GFXDECODE_ENTRY( "gfx1", 0x1800, CGA_charlayout, 0, 256 )   // Font 3
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

void pinball2k_state::machine_start()
{
	std::fill(std::begin(m_disp_ctrl_reg), std::end(m_disp_ctrl_reg), 0);
	std::fill(std::begin(m_biu_ctrl_reg), std::end(m_biu_ctrl_reg), 0);
}

void pinball2k_state::machine_reset()
{
	uint8_t *rom = memregion("bios")->base();

	memcpy(m_bios_ram, rom, 0x40000);
	m_maincpu->reset();
}

void pinball2k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void pinball2k_state::mediagx(machine_config &config)
{
	/* basic machine hardware */
	MEDIAGX(config, m_maincpu, 166000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pinball2k_state::mediagx_map);
	m_maincpu->set_addrmap(AS_IO, &pinball2k_state::mediagx_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device(18, FUNC(pinball2k_state::cx5510_pci_r), FUNC(pinball2k_state::cx5510_pci_w));

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	RAMDAC(config, m_ramdac, 0, m_palette);
	m_ramdac->set_addrmap(0, &pinball2k_state::ramdac_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 639, 0, 239);
	m_screen->set_screen_update(FUNC(pinball2k_state::screen_update_mediagx));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cga);
	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


void pinball2k_state::init_mediagx()
{
	m_frame_width = m_frame_height = 1;
}

void pinball2k_state::init_pinball2k()
{
	init_mediagx();
}

/*****************************************************************************/

/*------------------------------
/ Star Wars Episode 1 (#50069)
/------------------------------*/
ROM_START( swe1pb )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "awdbios.bin",     0x000000, 0x040000, CRC(854ce8c6) SHA1(7826de74026e052dacce8516382f664004c327ad) )

	ROM_REGION32_LE(0x4000000, "prism", 0)
	// bank 0
	ROM_LOAD( "swe1_u100.rom",   0x0000000, 0x800000, CRC(db2c9709) SHA1(14e8db2c0b09c4da6306a4a1f7fe54b2a334c5ed) )
	ROM_LOAD( "swe1_u101.rom",   0x0800000, 0x800000, CRC(a039e80d) SHA1(8f63e8ab83e043232fc17ed3dff1f251396a178a) )
	// bank 1
	ROM_LOAD( "swe1_u102.rom",   0x1000000, 0x800000, CRC(c9feb7bc) SHA1(a34acd34c3f91f082b67e385b1f4da2e5b6e5087) )
	ROM_LOAD( "swe1_u103.rom",   0x1800000, 0x800000, CRC(7a692466) SHA1(9adf5ae9c12bd5b6314913f6c01d4566ee453fe1) )
	// bank 2
	ROM_LOAD( "swe1_u104.rom",   0x2000000, 0x800000, CRC(76e2dd7e) SHA1(9bc20a1423b11c46eb2f5a514e985151defb5651) )
	ROM_LOAD( "swe1_u105.rom",   0x2800000, 0x800000, CRC(87f2460c) SHA1(cdc05e017367f61280e3d5682096e67e4c200150) )
	// bank 3
	ROM_LOAD( "swe1_u106.rom",   0x3000000, 0x800000, CRC(84877e2f) SHA1(6dd8c761b2e26313ae9e159690b3a4a170cb3bd8) )
	ROM_LOAD( "swe1_u107.rom",   0x3800000, 0x800000, CRC(dc433c89) SHA1(9f1273debc9168c04202078503cfc4f1ca8cb30b) )

	ROM_REGION(0xc00000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD( "28f800.bin",      0x000000, 0x100000, CRC(5fc1fd2c) SHA1(0967db9b6e82d386d3a8415bbef40bcab5a06654) )
	ROM_LOAD( "swe1_u109.rom",   0x400000, 0x400000, CRC(cc08936b) SHA1(fc428393e8a0cf37b800dd475fd293a1a98c4bcf) )
	ROM_LOAD( "swe1_u110.rom",   0x800000, 0x400000, CRC(6011ecd9) SHA1(8575958c8942a6cbcb2ac18f291fcada6f8cbc09) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",          0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END


/*----------------------------
/ Revenge from Mars (#50070)
/----------------------------*/
ROM_START( rfmpb )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "awdbios.bin",     0x000000, 0x040000, CRC(854ce8c6) SHA1(7826de74026e052dacce8516382f664004c327ad) )

	ROM_REGION32_LE(0x4000000, "prism", 0)
	// bank 0
	ROM_LOAD( "rfm_u100.rom",    0x0000000, 0x800000, CRC(b3548b1b) SHA1(874a16282bb778886cea2567d68ec7024dc5ed22) )
	ROM_LOAD( "rfm_u101.rom",    0x0800000, 0x800000, CRC(8bef301d) SHA1(2eade00b1a4cd3f5e98ebe8ed8f549e328188e77) )
	// bank 1
	ROM_LOAD( "rfm_u102.rom",    0x1000000, 0x800000, CRC(749f5c59) SHA1(2d8850e7f8ea3e07e8b444d7dd4dc4195a547ae7) )
	ROM_LOAD( "rfm_u103.rom",    0x1800000, 0x800000, CRC(a9ec5e97) SHA1(ce7c38dcbf34ce10d6e204a3176cd2c7a83b525a) )
	// bank 2
	ROM_LOAD( "rfm_u104.rom",    0x2000000, 0x800000, CRC(0a1acd70) SHA1(dcca4de92eadeb82ac776953326410a9687838cb) )
	ROM_LOAD( "rfm_u105.rom",    0x2800000, 0x800000, CRC(1ef31684) SHA1(141900a7426ad483384606cddb018d186952f439) )
	// bank 3
	ROM_LOAD( "rfm_u106.rom",    0x3000000, 0x800000, CRC(daf4e1dc) SHA1(0612495468fb962b833057e50f620c5f69cd5840) )
	ROM_LOAD( "rfm_u107.rom",    0x3800000, 0x800000, CRC(e737ab39) SHA1(0e978923db19e2893fdb4aae69d6ed3c3f664a31) )

	ROM_REGION(0xc00000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD( "28f800.bin",      0x000000, 0x100000, CRC(a57c55ad) SHA1(60ee230b8978b7c5f1482b1b587d1c6db5fdd20e) )
	ROM_LOAD( "rfm_u109.rom",    0x400000, 0x400000, CRC(385f1255) SHA1(0a3be261cd35cd153eff95335597bca46b760568) )
	ROM_LOAD( "rfm_u110.rom",    0x800000, 0x400000, CRC(2258dbde) SHA1(0c9e62e45fa7cc03aedd43a6e06fee28b2f288a5) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",          0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END

ROM_START( rfmpbr2 )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "awdbios.bin",     0x000000, 0x040000, CRC(854ce8c6) SHA1(7826de74026e052dacce8516382f664004c327ad) )

	ROM_REGION32_LE(0x4000000, "prism", 0)
	// bank 0
	ROM_LOAD( "rfm_u100r2.rom",  0x0000000, 0x800000, CRC(d4278a9b) SHA1(ec07b97190acb6b34b9ed6cda505ee8fefd66fec) )
	ROM_LOAD( "rfm_u101r2.rom",  0x0800000, 0x800000, CRC(e5d4c0ed) SHA1(cfc7d9d2324cc02c9eaf53fd674f7db24736699c) )
	// bank 1
	ROM_LOAD( "rfm_u102.rom",    0x1000000, 0x800000, CRC(749f5c59) SHA1(2d8850e7f8ea3e07e8b444d7dd4dc4195a547ae7) )
	ROM_LOAD( "rfm_u103.rom",    0x1800000, 0x800000, CRC(a9ec5e97) SHA1(ce7c38dcbf34ce10d6e204a3176cd2c7a83b525a) )
	// bank 2
	ROM_LOAD( "rfm_u104.rom",    0x2000000, 0x800000, CRC(0a1acd70) SHA1(dcca4de92eadeb82ac776953326410a9687838cb) )
	ROM_LOAD( "rfm_u105.rom",    0x2800000, 0x800000, CRC(1ef31684) SHA1(141900a7426ad483384606cddb018d186952f439) )
	// bank 3
	ROM_LOAD( "rfm_u106.rom",    0x3000000, 0x800000, CRC(daf4e1dc) SHA1(0612495468fb962b833057e50f620c5f69cd5840) )
	ROM_LOAD( "rfm_u107.rom",    0x3800000, 0x800000, CRC(e737ab39) SHA1(0e978923db19e2893fdb4aae69d6ed3c3f664a31) )

	ROM_REGION(0xc00000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD( "28f800.bin",      0x000000, 0x100000, CRC(5fc1fd2c) SHA1(0967db9b6e82d386d3a8415bbef40bcab5a06654) )
	ROM_LOAD( "rfm_u109.rom",    0x400000, 0x400000, CRC(385f1255) SHA1(0a3be261cd35cd153eff95335597bca46b760568) )
	ROM_LOAD( "rfm_u110.rom",    0x800000, 0x400000, CRC(2258dbde) SHA1(0c9e62e45fa7cc03aedd43a6e06fee28b2f288a5) )

	ROM_REGION(0x08100, "gfx1", 0)
	ROM_LOAD("cga.chr",          0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd))
ROM_END

} // Anonymous namespace

/*****************************************************************************/

GAME( 1999, swe1pb,   0,     mediagx, mediagx, pinball2k_state, init_pinball2k, ROT0, "Midway",  "Pinball 2000: Star Wars Episode 1", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1999, rfmpb,    0,     mediagx, mediagx, pinball2k_state, init_pinball2k, ROT0, "Midway",  "Pinball 2000: Revenge From Mars (rev. 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1999, rfmpbr2,  rfmpb, mediagx, mediagx, pinball2k_state, init_pinball2k, ROT0, "Midway",  "Pinball 2000: Revenge From Mars (rev. 2)", MACHINE_IS_SKELETON_MECHANICAL )
