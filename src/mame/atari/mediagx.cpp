// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Atari MediaGX

    Driver by Ville Linde
*/

/*
    Main Board number: P5GX-LG REV:1.1
		A-22540 CPU Board Assembly

    Components list on MAIN board, clockwise from top left.
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
			Data Signal bus with the Custom Atari board
			* Only 17 pins used in manual, but appears to be a 25-pin connector

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
		04-11238 JAMMIT PCB Assembly

    Components list on custom Atari board, clockwise from top left.
    -------------------------------------
    DS1232 (SOIC8)
    74F244 (x4, SOIC20)
    3 PIN JUMPER for sound OUT (Set to MONO, alternative setting is STEREO)
    4 PIN POWER CONNECTOR for IDE Hard Drive
    15 PIN VGA CONNECTOR (DSUB15, plugged into MAIN board)
    25 PIN PARALLEL CONNECTOR (DSUB25, plugged into MAIN board)
			Data Signal bus with the CPU (Main) Board Assembly
			* Only 17 pins used in manual

    AD 813AR (x2, SOIC14)
    ST 084C P1S735 (x2, SOIC14)
    74F244 (SOIC20)
    7AAY2JK LS86A (SOIC14)
    74F07 (SOIC14)
    3 PIN JUMPER for SYNC (Set to -, alternative setting is +)
    OSC 14.31818MHz
    ACTEL A42MX09 (PLCC84, labelled 'JAMMINT U6 A-22505 (C)1996 ATARI')
			Designation: 	U6
			Part #: 			A-22505
			Function: 		Gun control and security
			Description: 	FPGA Assembly
    LS14 (SOIC14)
    74F244 (x2, SOIC20)
    ST ULN2064B (DIP16)

*/

#include "emu.h"
#include "pcshare.h"

#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "machine/timer.h"
#include "sound/dmadac.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define SPEEDUP_HACKS 1

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
		m_ports(*this, "IN%u", 0U)
	{ }

	void init_a51site4();
	void mediagx(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

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
	void mediagx_io(address_map &map);
	void mediagx_map(address_map &map);
	void ramdac_map(address_map &map);

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
	uint8_t m_pal[768];

	optional_ioport_array<11> m_ports; // but parallel_pointer takes values 0 -> 23

	uint32_t m_disp_ctrl_reg[256/4]{};
	int m_frame_width = 0;
	int m_frame_height = 0;

	uint32_t m_memory_ctrl_reg[256/4]{};
	int m_pal_index = 0;

	// Bus Interface Unit (BIU) Control Register
	uint32_t m_biu_ctrl_reg[256/4]{};

	uint8_t m_mediagx_config_reg_sel = 0;
	uint8_t m_mediagx_config_regs[256]{};

	// Parallel Port stuff
	// Points to a port in the list of m_ports, allows selection of coin, inputs, controls, etc.
	uint8_t m_parallel_pointer = 0;
	// This latches the value retrieved from m_ports[m_parallel_pointer], allowing us to read it later
	uint8_t m_parallel_latched = 0;
	// The parallel port itself, data is read/written from here
	uint32_t m_parport = 0;
	// simple control register for the parallel port
	// updates on writes, changing the state, and reflected in the subsequent reads
	uint8_t m_parport_control_reg = 0;
	//int m_control_num;
	//int m_control_num2;
	//int m_control_read;

	uint32_t m_cx5510_regs[256/4];

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
		int j8 = j*8;
		for (int i=0; i < 80; i+=2)
		{
			int i8 = i*8;
			int const att0 = (cga[index] >> 8) & 0xff;
			int const ch0 = (cga[index] >> 0) & 0xff;
			int const att1 = (cga[index] >> 24) & 0xff;
			int const ch1 = (cga[index] >> 16) & 0xff;

			draw_char(bitmap, cliprect, gfx, ch0, att0, i8, j8);
			draw_char(bitmap, cliprect, gfx, ch1, att1, i8+8, j8);
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


// Read from memory, generally done at start/setup
uint32_t mediagx_state::memory_ctrl_r(offs_t offset)
{
	return m_memory_ctrl_reg[offset];
}


// Write to memory, generally done at start/setup
void mediagx_state::memory_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// printf("memory_ctrl_w %08X, %08X, %08X\n", data, offset*4, mem_mask);
	if (offset == 0x20/4)
	{
		if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00e00000) == 0x00400000)
		{
			// guess: crtc params?
			// ...
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00000000)
		{
			// set index from data
			m_pal_index = data;
			m_ramdac->index_w(data);
		}
		else if((m_disp_ctrl_reg[DC_GENERAL_CFG] & 0x00f00000) == 0x00100000)
		{
			// write data at current index
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

/**
 * The I/O locations 0x22 and 0x23 are used for MediaGX processor configuration register access.
 * Reading occurs at 0x23, where a prior index was written on 0x22 to read from
 */
uint8_t mediagx_state::io20_r(offs_t offset)
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x01)
	{
		// read only occurs over 0x23, assumes prior 0x22 has set the register index correctly
		return m_mediagx_config_regs[m_mediagx_config_reg_sel];
	}
	return 0;
}


/**
 * I/O locations 0x22 and 0x23 are used for MediaGX processor configuration register access.
 * A write to 0x22 sets the index of the configuration register.
 * A subsequent read on 0x23 should use this index
 * Each operation on 0x23 should be proceeded by a valid index set on 0x22
 */
void mediagx_state::io20_w(offs_t offset, uint8_t data)
{
	// 0x22, 0x23, Cyrix configuration registers
	if (offset == 0x00)
	{
		// Index of the configuration register is received on 0x22
		m_mediagx_config_reg_sel = data;
	}
	else if (offset == 0x01)
	{
		// Data written through 0x23, assumes prior index set via write over 0x22
		m_mediagx_config_regs[m_mediagx_config_reg_sel] = data;
	}
}

// Takes an 8-bit input val, and converts it into an
// appropriate output val for reading from IO 0x379 on the par port
// Only interested in the low 4-bits
uint16_t mk_parport_outval(uint8_t v) {
	if(v >= 0x10) {
		printf("[WARN] mk_parport_outval given too large a value: %08x\n", v);
	}

	uint16_t v2 = v << 3;
	if(v2 & 0x40) {
		// high bit is set, leave inverted bit low
		// clear high bit
		v2 ^= 0x40;
	} else {
		// high bit is not set, set inverted bit high
		v2 |= 0x80;

	}
	return v2 << 8;
}

int par_pointer_out_val = 0x5;

uint8_t previous_parport_state = 0;

// used to control whether 2-bits for P2 jgun X should be read
// same value usually triggers system menu, so tracking when this is okay to read to avoid the overlap
bool should_read_p2x_highbits = false;

// Reads from the parallel port
// 'offset' seems to be nearly constant in this case
// the 'mem_mask' changes between (L) 0000FF00 and (H) 00FF0000
uint32_t mediagx_state::parallel_port_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_8_15)
	{
		uint32_t mp0 = 0;
		uint8_t upper_reg = m_parport_control_reg >> 4;

		if(m_parport_control_reg == 0x10) {
			r = mk_parport_outval(0);

		} else if(m_parport_control_reg == 0x18) {
			// Check for F2 to open the system menu
			// TEST MODE switch, only works here on 0x18
			mp0 = m_ports[0].read_safe(0);

			// high 2-bits of P2X (controls thirds of X access)
			int16_t p2x = m_ports[9].read_safe(0) * 3;
			if ((previous_parport_state >> 4) == 0xF) {

				if(mp0 & 1) {
					// allow opening the System Menu (TEST)
					r = mk_parport_outval(2);

				} else if (p2x > 512 && should_read_p2x_highbits) {
					// P2 X 3/3, only when we're allowed to read this (always a bit after a 0x5 has been seen)
					r = mk_parport_outval(2);

				} else if (p2x > 256) {
					// P2 X 2/3
					r = mk_parport_outval(1);

				} else {
					// regular response
					r = mk_parport_outval(0);
				}

				// clear this flag
				should_read_p2x_highbits = false;

			} else {
				r = mk_parport_outval(0);
			}

		} else if(upper_reg == 0x2) {
			// Tilt, Test, unused, Bill... 3rd option is unused, Bill is inverted by the way
			uint8_t mp2 = m_ports[2].read_safe(0);
			r = mk_parport_outval(mp2);

		} else if(upper_reg == 0x3) {
			// Coins
			mp0 = m_ports[0].read_safe(0);
			// @montymxb This toggles the appropriate coin switches 1-4, but does not actually cause a credit to show up.
			// Instead we rely on 0x6 to actually put coins in, and this to toggle the switch correctly...not sure why yet
			r = mk_parport_outval((mp0 & 0xf0) >> 4 ^ 0xe);

		} else if(upper_reg == 0x4) {
			// SVC (service credits 1 + 2), and Audio controls
			uint32_t mp1 = m_ports[1].read_safe(0);
			// Service Credits 1 + 2, along with Volume controls.
			// Bit 4 has to be inverted (0x8), active low, done in inputs already
			r = mk_parport_outval(mp1);

		} else if(upper_reg == 0x5) {
			// Start buttons
			// P1 - P4 start buttons
			// All start buttons lead to jgun movements being registered, but in a different fashion?
			mp0 = m_ports[0].read_safe(0);
			r = mk_parport_outval(mp0 >> 0x8);

			// set flag that next time control reg is 0x18,
			// we should read P2 X high bits
			should_read_p2x_highbits = true;

		} else if(m_parport_control_reg == 0x60) {
			// Reset watchdogs?
			// coins & coin tracker only tripped together under 0x60
			mp0 = m_ports[0].read_safe(0);

			// keeping 0xf0 as the bitmask pushes the coin insertion to keys 5-8, which is desirable
			if(mp0 & 0xf0) {
				// this changes coin insertion to show up on 1-5, but it doesn't block the mode interestingly enough
				// drops a coin in, but does not toggle the 'coin' switch itself
				r = mk_parport_outval(0x1);
			}

		} else if(upper_reg == 0xF) {
			// TODO Internal pointer advance?
			mp0 = m_ports[0].read_safe(0);

			r = mk_parport_outval(0);

			int16_t mp5;
			int8_t mp6; // TODO should be converted to 16-bit
			int16_t p2y;
			int16_t p2x;

			uint8_t mp_gen;

			// JGun Analog Controls
			switch (m_parallel_pointer) {
				case 10:
					// Related to enabling P2 Start, P1 start, and allows some basic controls as such
					r = mk_parport_outval(par_pointer_out_val);
					break;
				case 11:
					// TRIGGERS (1 & 2)
					// 0x1 = P1 Trigger
					// 0x2 = P2 Trigger
					// TODO @montymxb Oct. 30th, 2022: Muzzle 'flash' shows up sporadically when shooting, random values of even 0/1 will trigger it as well.
					// Seems to be connected to some other state machine? Unsure.
					mp_gen = m_ports[7].read_safe(0);
					r = mk_parport_outval(mp_gen);
					break;
				case 12:
					// P1 control select (but P2 as well?)
					// 0x1 allows P1 gun (but P2 already works?)
					// 0x2, nothing?
					// 0x4, nothing?
					// 0x8, nothing?
					r = mk_parport_outval(0x1);
					break;
				case 13:
					// P1, Y (LOW)
					// 0x1 = 1
					// 0x2 = 2
					// 0x4 = 4
					// 0x8 = 8
					// convert mouse Y
					mp6 = m_ports[6].read_safe(0);
					mp6 = 128 - mp6;
					r = mk_parport_outval(mp6 & 0xf);
					break;
				case 14:
					// P1, Y, (HIGH)
					// 0x1 = 16
					// 0x2 = 32
					// 0x4 = -64 (using this inverted approach, needs to be fixed?)
					// 0x8 = +128
					mp6 = m_ports[6].read_safe(0);
					mp6 = 128 - mp6;
					if (mp6 >= 0 && mp6 < 64) {
						// 2/4
						r = mk_parport_outval(((mp6 >> 4) & 0x3));

					} else if (mp6 < -64) {
						// 4/4
						r = mk_parport_outval(((mp6 >> 4) & 0x3) | 0x8);

					} else if (mp6 < 0) {
						// 3/4
						r = mk_parport_outval(((mp6 >> 4) & 0x3) | 0x4);

					} else if (mp6 >= 64) {
						// 1/4
						r = mk_parport_outval(((mp6 >> 4) & 0x3) | 12);

					}
					break;
				case 15:
					// P1, Y (single high bit)
					// 0x1 = -256
					// 0x2 = ???
					// 0x4 = ???
					// 0x8 = ???
					mp6 = m_ports[6].read_safe(0);
					// mp6 = 124 - mp6;
					if (mp6 >= 0 || mp6 < -64) {
						r = mk_parport_outval(0x1);
					}
					break;
				case 16:
					// P1 X (LOW)
					// 0x1 sets 0x2 as 1, otherwise 0x4 is 1? ** important to set
					// 0x2 == 1
					// 0x4 == 2
					// 0x8 == 4
					mp5 = int(float(m_ports[5].read_safe(0)) * 3) % 256;
					r = mk_parport_outval(mp5 & 0xf);
					break;
				case 17:
					// P1 X (Low + High)
					// 0x1 == 8
					// 0x2 == 16
					// 0x4 == 32
					// 0x8 == 63 ** (not 64 for some reason?)
					mp5 = int(float(m_ports[5].read_safe(0)) * 3) % 256;
					r = mk_parport_outval((mp5 >> 4) & 0xf);
					break;
				case 18:
					// P1, X (HH)
					// 0x1 = 2nd 3rd
					// 0x2 = last 3rd
					// 0x4 nothing..
					// 0x8 nothing..
					mp5 = int(float(m_ports[5].read_safe(0)) * 3);
					if (mp5 > 512) {
						r = mk_parport_outval(0x2);
					} else if (mp5 > 256) {
						r = mk_parport_outval(0x1);
					}
					break;
				case 19:
					// P2, Y (low 4 bits)
					p2y = 256 - m_ports[10].read_safe(0);
					r = mk_parport_outval(p2y & 0xf);
					break;
				case 20:
					// P2, Y (high 4 bits)
					p2y = 256 - m_ports[10].read_safe(0);
					r = mk_parport_outval((p2y >> 4) & 0xf);
					break;
				case 21:
					// P2, Y (high single bit)
					// 0x1 = -256
					// 0x2 = ??? (nothing)
					// 0x4 = ??? (nothing)
					// 0x8 = ??? (nothing)
					r = mk_parport_outval(0x1);
					break;
				case 22:
					// P2, X (lower nibble)
					// 0x1 = 1
					// 0x2 = 2
					// 0x4 = 4
					// 0x8 = 8
					p2x = m_ports[9].read_safe(0) * 3 % 256;
					r = mk_parport_outval(p2x & 0xf);
					break;
				case 23:
					// P2, X (upper nibble)
					// 0x1 = 16
					// 0x2 = 32
					// 0x4 = 64
					// 0x8 = 128
					p2x = m_ports[9].read_safe(0) * 3 % 256;
					r = mk_parport_outval((p2x >> 4) & 0xf);
					break;
			}


		} else if(upper_reg == 0x0) {
			// TODO some empty data written on start

		}

		// record prior control register state for parport
		// (used to help with distinguishing Sys Menu cmd from P2 X high 2-bit inputs)
		previous_parport_state = m_parport_control_reg;

	}
	else if (ACCESSING_BITS_16_23)
	{
		// Reading CONTROL
		// 0x01 causes a busy spin
		// 0x02 causes a slight shift in the overall access? (unsure what this really pertains to)
		// negate bits 0,1, and 3, all are hardware inverted

		// standard ctrl read, return what was set before
		r = (m_parport & 0xff0000) ^ 0x0b0000;

	} else if(ACCESSING_BITS_0_7) {
		// Reading DATA
		// only happens on boot, just return what was written to the parport before, if anything
		r = m_parport & 0xff;

	}

	return r;
}

// Writes to m_parport and m_parallel_latched
// Updates the m_parallel_pointer to the next port, but used only internally here
// Uses a mask of 0x00FF0000 and 0x000000FF only, alternating
void mediagx_state::parallel_port_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// update parport registers together, DATA, STATUS, and CONTROL
	// Can be seen like so: 0xCC SS DD
	// 		where C = Control nibble, S = Status nibble, D = Data nibble
	COMBINE_DATA( &m_parport );

	if (ACCESSING_BITS_0_7)
	{
		// Writing to DATA
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

		//logerror("%08X:", m_maincpu->pc());

		//parallel_pointer++;
		//logerror("[%02X] Advance pointer to %d\n", data, parallel_pointer);

		// update the control register by masking off the low bit
		// this controls our parallel reads
		m_parport_control_reg = data & 0xfc;

		switch (m_parport_control_reg)
		{
			case 0x18:
			//case 0x19: dropped anyways by 0xfc...
			case 0x1a:
			case 0x1b:
				// 18 = reset internal pointer to 0
				m_parallel_pointer = data & 3;
				//printf("[%02X] Reset pointer to %d\n", data, m_parallel_pointer);
				//logerror("[%02X] Reset pointer to %d\n", data, m_parallel_pointer);
				break;

			case 0x20:
			case 0x24:
			case 0x28:
			case 0x2c:
				//printf("[%02X] General purpose output (0x2) = x%X\n", data, data & 0x0f);
				//logerror("[%02X] General purpose output = x%X\n", data, data & 0x0f);
				break;

			case 0x30:
			case 0x34:
			case 0x38:
			case 0x3c:
				//printf("[%02X] General purpose output (0x3) = x%X\n", data, data & 0x0f);
				//logerror("[%02X] General purpose output = %Xx\n", data, data & 0x0f);
				break;

			case 0x40:
			case 0x44:
			case 0x48:
			case 0x4c:
				//printf("[%02X] Coin counters = %d%d%d%d\n", data, (data >> 3) & 1, (data >> 2) & 1, (data >> 1) & 1, data & 1);
				//logerror("[%02X] Coin counters = %d%d%d%d\n", data, (data >> 3) & 1, (data >> 2) & 1, (data >> 1) & 1, data & 1);
				break;

			case 0x50:
			case 0x54:
			case 0x58:
			case 0x5c:
				//logerror("[%02X] Kickers = %d%d\n", data, (data >> 1) & 1, data & 1);
				break;

			case 0x60:
			case 0x64:
			case 0x68:
			case 0x6c:
				//logerror("[%02X] Watchdog reset\n", data);
				break;

			default:
				if (data >= 0x70)
				{
					m_parallel_pointer++;
					//logerror("[%02X] Advance pointer to %d\n", data, m_parallel_pointer);
				}
				else
				{
					//logerror("[%02X] Unknown write\n", data);
				}
				break;
		}
	} else if(ACCESSING_BITS_16_23) {
		// Writing to CONTROL

	} else if(ACCESSING_BITS_8_15) {
		// Writing to STATUS
		// this should never happen...

	}
}

/* Analog Devices AD1847 Stereo DAC */
#define AD1847_SAMPLE_DELAY 10
#define AD1847_SAMPLE_SIZE 1000 / AD1847_SAMPLE_DELAY

// TODO: What else uses this callback for sound, might find a good example to make the audio more stable across devices
TIMER_DEVICE_CALLBACK_MEMBER(mediagx_state::sound_timer_callback)
{
	m_ad1847_sample_counter = 0;
	timer.adjust(attotime::from_msec(AD1847_SAMPLE_DELAY));

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

// >> ANALOG DEVICES AD1847JP 'SOUNDPORT' (PLCC44, surface mounted)
uint32_t mediagx_state::ad1847_r(offs_t offset)
{
	switch (offset)
	{
		case 0x14/4:
			return ((m_ad1847_sample_rate) / (AD1847_SAMPLE_SIZE)) - m_ad1847_sample_counter; // 1000 / 10, usually, so 100
	}
	return 0;
}

// >> ANALOG DEVICES AD1847JP 'SOUNDPORT' (PLCC44, surface mounted)
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
	// region for the main ram
	map(0x00000000, 0x0009ffff).ram().share(m_main_ram);
	//  additional ram region, pad?
	map(0x000a0000, 0x000affff).ram();
	// CGA ram follows
	map(0x000b0000, 0x000b7fff).ram().share(m_cga_ram);
	// bios RAM follows after
	map(0x000c0000, 0x000fffff).ram().share(m_bios_ram);

	map(0x00100000, 0x00ffffff).ram();
	// 256-bit region for BIU Control
	map(0x40008000, 0x400080ff).rw(FUNC(mediagx_state::biu_ctrl_r), FUNC(mediagx_state::biu_ctrl_w));
	// 256-bit region for Diplsay control
	map(0x40008300, 0x400083ff).rw(FUNC(mediagx_state::disp_ctrl_r), FUNC(mediagx_state::disp_ctrl_w));
	// 256-bit region for Memory Control
	// Handles R/W to mem, loaded & written at boot
	map(0x40008400, 0x400084ff).rw(FUNC(mediagx_state::memory_ctrl_r), FUNC(mediagx_state::memory_ctrl_w));
	// Larger chunk for VRAM (Video Ram)
	map(0x40800000, 0x40bfffff).ram().share(m_vram);
	// BIOS itself lives at the end
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

// Maps address ranges to I/O Functions
// These IO functions are defined for the MediaGX device specifically
// Larger IO port list:				https://bochs.sourceforge.io/techspec/PORTS.LST
// Programmaable Interrupt Timer:	https://wiki.osdev.org/PIT#I.2F0_Ports
// Typical port usages: 			https://wiki.osdev.org/I/O_Ports
void mediagx_state::mediagx_io(address_map &map)
{
	pcat32_io_common(map);

	// Access to the registers specific to Cyrix processors
	// 2-bits
	map(0x0022, 0x0023).rw(FUNC(mediagx_state::io20_r), FUNC(mediagx_state::io20_w));

	// 0x40 -> 0x41 does something special but not sure?
	// seems related to interrupts, fetching the result of something that was pressed perhaps?
	// 0x40 programmable interrupt timer?
	// See: https://wiki.osdev.org/PIT#I.2F0_Ports

	// 4-bits for the I/O delay port
	map(0x00e8, 0x00eb).noprw();

	// IDE is an interface for connecting an HDD w/ the computer
	// Primary Parallel ATA Hard Disk Controller
	// 8-bits
	map(0x01f0, 0x01f7).rw(m_ide, FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));

	// 4-bytes
	// Maps parallel port R/W to an address range in this system, https://en.wikipedia.org/wiki/Parallel_port
	// Was at 0x037b, but think it should go to 37f for full range?
	// but keeping at 0x37b so it's the same as it was before, not sure how much it matters here
	map(0x0378, 0x037b).rw(FUNC(mediagx_state::parallel_port_r), FUNC(mediagx_state::parallel_port_w));

	// More Integrated Drive Electronics (IDE) controller stuff, for interfacing w/ external storage
	// 8-bits
	// Primary IDE controller
	map(0x03f0, 0x03f7).rw(m_ide, FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));

	// AD1847 Audio stuff
	// 256-bits
	map(0x0400, 0x04ff).rw(FUNC(mediagx_state::ad1847_r), FUNC(mediagx_state::ad1847_w));

	// PCI Configuration Space, https://en.wikipedia.org/wiki/PCI_configuration_space
	// specifically for the Intel Pentium motherboard ("Neptune" chipset)
	// 8-bits
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
}

/*****************************************************************************/

static const gfx_layout CGA_charlayout =
{
	8,8,                    /* 8 x 16 characters .... but says 8 intead? */
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
	// activates the debug service menu
	PORT_SERVICE_NO_TOGGLE( 0x1, IP_ACTIVE_HIGH )

	// Coins
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_COIN4 )

	// Start buttons
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x800, IP_ACTIVE_HIGH, IPT_START4 )

	// 'Light' Gun X & Y, idea pulled from Carnevil's implementation using the Seattle Driver
	PORT_START("JGUN0_X") // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
	PORT_START("JGUN0_Y") // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("JGUN1_X") // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("JGUN1_Y") // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("JGUN") // fake switches
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Trigger")

	// Service 1 + 2 & Volume controls
	PORT_START("IN1")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN )
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_VOLUME_UP )

	// Tilt, Test, UNUSED, Bill
	PORT_START("IN2")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_TILT);
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_SERVICE); // is this 'test' switch service mode?
	PORT_BIT(0x4, IP_ACTIVE_HIGH, IPT_UNUSED); // unused
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_BILL1);

	// ZX CV, for debugging
	PORT_START("IN3")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON8 )

	// BN M, for debugging
	PORT_START("IN4")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON11 )
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON12 )

	// JGun0 X
	PORT_START("IN5")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	// JGun0 Y
	PORT_START("IN6")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	// JGun Triggers
	PORT_START("IN7")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Trigger")

	PORT_START("IN8")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0f0, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0xf00, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)

	// JGun1 X
	PORT_START("IN9")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)

	// JGun1 Y
	PORT_START("IN10")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)

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
	sound_timer->adjust(attotime::from_msec(AD1847_SAMPLE_DELAY));

	m_dmadac[0]->enable(1);
	m_dmadac[1]->enable(1);
}

void mediagx_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void mediagx_state::mediagx(machine_config &config)
{
	MEDIAGX(config, m_maincpu, 166000000);

	// Program mapping
	m_maincpu->set_addrmap(AS_PROGRAM, &mediagx_state::mediagx_map);
	// Map address regions to I/O
	m_maincpu->set_addrmap(AS_IO, &mediagx_state::mediagx_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

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
	m_screen->set_refresh_hz(61);

	// TODO should probably set this via set_raw instead
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 639, 0, 239);

	m_screen->set_screen_update(FUNC(mediagx_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cga);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}


void mediagx_state::init_mediagx()
{
	m_frame_width = m_frame_height = 1;
	m_parallel_pointer = 0;
	// set these ranges to 0, keeps registers all zeroed out as well
	// clears the display control register
	std::fill(std::begin(m_disp_ctrl_reg), std::end(m_disp_ctrl_reg), 0);
	// clears the 'BIU' control register
	std::fill(std::begin(m_biu_ctrl_reg), std::end(m_biu_ctrl_reg), 0);

	#if SPEEDUP_HACKS
	// clears the 'speedup hits' registers, just a cache of some form
	std::fill(std::begin(m_speedup_hits), std::end(m_speedup_hits), 0);
	#endif
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

GAME( 1998, a51site4, 0       , mediagx, mediagx, mediagx_state, init_a51site4,  ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.01, September 7, 1998)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
GAME( 1998, a51site4a,a51site4, mediagx, mediagx, mediagx_state, init_a51site4,  ROT0,   "Atari Games",  "Area 51: Site 4 (HD Rev 2.0, September 11, 1998)", MACHINE_NOT_WORKING )
