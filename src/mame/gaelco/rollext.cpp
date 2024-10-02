// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Rolling Extreme
    Gaelco, 1999

    PCB Layout
    ----------

    REF.991015
      |--------------------------------------------------|
      |TL074 TDA1543  LP61256         ROE.45     ROE.58  |
      |                                                  |
      |TL074 TDA1543  LP61256         ROE.47     ROE.60  |-|
      |                                                  | |DB9
      |              |--------| KM4216C258  KM4216C258   |-|
      |              |ALTERA  |                          |
      |      LP61256 |FLEX    | KM4216C258  KM4216C258   |
      |      LP61256 |EPF10K50|                          |
    |-|      LP61256 |--------| KM4216C258  KM4216C258   |
    | |2444CS           CY2308                           |
    | |           |--------|    KM4216C258  KM4216C258   |
    | |  L4955    |ALTERA  |                             |
    | |           |FLEX    |    |------------|           |
    | |LED1       |EPF10K100    |CHIP EXPRESS|KM4216C258 |
    | |LED2 60MHz |--------|    |D3PLUS-4    |           |
    |-|    |----------|         |T2032-01    |KM4216C258 |
      |    |DSP       |         |N9G1554     |           |
      |    |TMS320C82 |         |9939 CA TWN |           |
      |    |GGP60     |         |------------|           |
      |    |1997 TI   |      93C66                       |
      |    |----------|         |-------|                |-|
      |          ROE.17         |ALTERA |         90MHz  | |DB9
      |K4S643232-TC80           |MAX    |KM718V895T-72   |-|
      |          ROE.18   ROE.38|EPM7160|                |
      |K4S643232-TC80           |-------|                |
      |--------------------------------------------------|
*/

/*

    MP Interrupts:

        External Interrupt 1:       0x4003ef78
        External Interrupt 2:       -
        External Interrupt 3:       0x40041d30
        Memory Fault:               0x40043ae8

    PP0 Interrupts:
        Task:                       0x400010a0 / 0x400001a0

    PP1 Interrupts:
        Task:                       0x4004c6e8


    Memory locations:

        [0x00000084] PP0 busy flag?
        [0x00000090] PP0 fifo write pointer?
        [0x00000094] PP0 fifo read pointer?
        [0x00000320] 2000000-TCOUNT in XINT3 handler
        [0x01010668] copied from (word)0xb0000004 in XINT3 handler

        ROM [0xc0050000] 0x10000 floats copied to [0x40180000]

        ROM [0xff800000 - 0xff80171f] sound data headers, 32 bytes each
            Word 0: sound data pointer (offset from 0xff800000)
            Word 1: uncompressed length?
            Word 2: ?
            Word 3: same as word 1?
            Word 4: ?
            Word 5: ?
            Word 6: ?
            Word 7: 0

        ROM [0xff801720 -> ] compressed audio data

        [0x0000100c] bitmask of active sound channels (max 16 channels?)
        [0x00001018 -> ] sound channel data, 64 bytes each
            +0x00: sound data pointer in ROM
            +0x04: 0
            +0x08: uncompressed length
            +0x0c: uncompressed length
            +0x10: 0?
            +0x14: 0 - sample rate?
            +0x18: 0?
            +0x1c: sample rate? (halfword)
            +0x1e: ?? (halfword)
            +0x20: ?
            +0x24: ?
            +0x28: ?
            +0x2c: ?
            +0x30: ?
            +0x34: ?
            +0x36: ? (halfword)
            +0x38: ?
            +0x3c: ?



    Texture ROM decode:

      {ic45}     {ic47}     {ic58}     {ic60}
    [2,0][0,0] [2,1][0,1] [3,0][1,0] [3,1][1,1]



    TODO:

    - TMS320C82 parallel processors are not emulated
      * PP0 transfers polygon data from a software FIFO to the graphics processor. This is currently HLE'd.
      * PP1 most likely does sound mixing. This is currently not emulated.
    - Alpha blending (probably based on palette index like on gaelco3d)
    - Minor Z-buffer issues
    - Wrong textures in a few places (could be a CPU core bug)
    - Networking

*/

#include "emu.h"
#include "cpu/tms32082/tms32082.h"
#include "video/poly.h"
#include "video/rgbutil.h"
#include "machine/eepromser.h"
#include "screen.h"


namespace {

#define BILINEAR 1

struct rollext_polydata
{
	uint32_t tex_origin_y;
	uint32_t tex_origin_x;
	uint32_t pal;
	float uoz_dx;
	float uoz_dy;
	float voz_dx;
	float voz_dy;
	float ooz_dx;
	float ooz_dy;
	float baseu;
	float basev;
	float basez;
	uint32_t zmul;
};

class rollext_renderer : public poly_manager<float, rollext_polydata, 4>
{
	friend class rollext_state;

public:
	rollext_renderer(screen_device &screen)
		: poly_manager<float, rollext_polydata, 4>(screen.machine())
	{
		m_fb[0] = std::make_unique<bitmap_rgb32>(1024, 1024);
		m_fb[1] = std::make_unique<bitmap_rgb32>(1024, 1024);

		m_zb = std::make_unique<bitmap_ind32>(1024, 1024);


		m_palette = std::make_unique<rgb_t[]>(32768);
		m_texture_mask = std::make_unique<uint8_t[]>(2048 * 16384);

		m_fb_current = 0;
	}

	template<bool UseZ>
	void render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid);

	void set_texture_ram(uint8_t* texture_ram) { m_texture_ram = texture_ram; }
	void process_display_list(screen_device &screen, uint32_t* dispram);

	void palette_write(int index, uint16_t data);
	void texture_mask_write(int index, uint8_t data);

	void clear_fb();
	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);
private:
	std::unique_ptr<bitmap_rgb32> m_fb[2];
	std::unique_ptr<rgb_t[]> m_palette;
	std::unique_ptr<uint8_t[]> m_texture_mask;

	std::unique_ptr<bitmap_ind32> m_zb;

	int m_fb_current;

	uint8_t *m_texture_ram = nullptr;
};

template<bool UseZ>
void rollext_renderer::render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid)
{
	uint32_t *fb = &m_fb[m_fb_current]->pix(scanline);
	uint32_t* const zb = &m_zb->pix(scanline);

	uint32_t tex_origin_y = extradata.tex_origin_y;
	uint32_t tex_origin_x = extradata.tex_origin_x;

	float baseu = extradata.baseu;
	float basev = extradata.basev;
	float basez = extradata.basez;
	float uoz_dx = extradata.uoz_dx;
	float uoz_dy = extradata.uoz_dy;
	float voz_dx = extradata.voz_dx;
	float voz_dy = extradata.voz_dy;
	float ooz_dx = extradata.ooz_dx;
	float ooz_dy = extradata.ooz_dy;

	uint32_t zmul = extradata.zmul;

	int palnum = extradata.pal << 8;

	float uoz = baseu + (uoz_dx * (extent.startx - 256)) + (-uoz_dy * (scanline - 192));
	float voz = basev + (voz_dx * (extent.startx - 256)) + (-voz_dy * (scanline - 192));
	float ooz = basez + (ooz_dx * (extent.startx - 256)) + (-ooz_dy * (scanline - 192));

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		float z = recip_approx(ooz);
		uint32_t zbufval = (int)(z * zmul);

		if (zbufval <= zb[x] || !UseZ)
		{
			float u = uoz * z;
			float v = voz * z;

			int tx = tex_origin_x + (int)(u);
			int ty = tex_origin_y + (int)(v);


#if BILINEAR
			float intu, intv;

			int fracu = modff(u, &intu) * 255.0f;
			int fracv = modff(v, &intv) * 255.0f;

			uint32_t mask00 = m_texture_mask[((ty & 0x3fff) * 2048) + (tx & 0x7ff)];
			uint32_t mask01 = m_texture_mask[((ty & 0x3fff) * 2048) + ((tx+1) & 0x7ff)];
			uint32_t mask10 = m_texture_mask[(((ty+1) & 0x3fff) * 2048) + (tx & 0x7ff)];
			uint32_t mask11 = m_texture_mask[(((ty+1) & 0x3fff) * 2048) + ((tx+1) & 0x7ff)];
			const uint32_t mask_level = rgbaint_t::bilinear_filter(mask00, mask01, mask10, mask11, fracu, fracv);

			if (mask_level < 0xff)
			{
				uint32_t tex00 = m_palette[palnum + BYTE_XOR_BE(m_texture_ram[((ty) & 0x3fff) * 2048 + ((tx) & 0x7ff)])];
				uint32_t tex01 = m_palette[palnum + BYTE_XOR_BE(m_texture_ram[((ty) & 0x3fff) * 2048 + ((tx + 1) & 0x7ff)])];
				uint32_t tex10 = m_palette[palnum + BYTE_XOR_BE(m_texture_ram[((ty + 1) & 0x3fff) * 2048 + ((tx) & 0x7ff)])];
				uint32_t tex11 = m_palette[palnum + BYTE_XOR_BE(m_texture_ram[((ty + 1) & 0x3fff) * 2048 + ((tx + 1) & 0x7ff)])];

				const uint32_t texsam = rgbaint_t::bilinear_filter(tex00, tex01, tex10, tex11, fracu, fracv);

				rgbaint_t texel_color(texsam);
				rgbaint_t fb_color(fb[x]);
				texel_color.blend(fb_color, 255-mask_level);

				fb[x] = texel_color.to_rgba_clamp();
				if (UseZ)
					zb[x] = zbufval;
			}
#else
			uint8_t mask = m_texture_mask[((ty & 0x3fff) * 2048) + (tx & 0x7ff)];
			if (mask == 0)
			{
				uint8_t pen = m_texture_ram[((ty & 0x3fff) * 2048) + (tx & 0x7ff)];
				uint32_t texel = m_palette[palnum + BYTE_XOR_BE(pen)];
				fb[x] = texel;
				if (UseZ)
					zb[x] = zbufval;
			}
#endif
		}

		uoz += uoz_dx;
		voz += voz_dx;
		ooz += ooz_dx;
	}
}

void rollext_renderer::process_display_list(screen_device &screen, uint32_t* disp_ram)
{
	rectangle visarea = rectangle(0, 511, 0, 383);

	int num = disp_ram[0xffffc/4];

	for (int i=0; i < num; i++)
	{
		int ii = i * 0x18;

		vertex_t vert[5];

		// Poly data:
		// Word 0:   x------- -------- -------- --------   ?
		//           -x------ -------- -------- --------   ? set to 1 for polygons, 0 on command 0x9c (viewport setup command?)
		//           --x----- -------- -------- --------   ? set to 1 for no perspective
		//           ---x---- -------- -------- --------   ? always 1?
		//           ----x--- -------- -------- --------   ? always 1?
		//           -----x-- -------- -------- --------   the PP transfer code checks for 1
		//           ------x- -------- -------- --------   always 0?
		//           -------x -------- -------- --------   Texture page
		//           -------- -------- -xxxxxxx --------   Palette
		//           -------- -------- -------- -----xxx   Number of verts (3,4,5 used)
		// Word 1:   (float) Vertex 1 X
		// Word 2:   xxxxxxxx xxxxx--- -------- --------   Texture Origin Bottom
		//           -------- -----xxx xxxxxxxx --------   Texture Origin Left
		// Word 3:   (float) Vertex 1 Y
		// Word 4:   xxxxxxxx xxxxxxxx xxxxxxxx x-------   Z buffer multiplier (int)
		// Word 5:   (float) Vertex 2 X
		// Word 6:   (float) U/Z per X pixel increment
		// Word 7:   (float) Vertex 2 Y
		// Word 8:   (float) U/Z per Y pixel increment
		// Word 9:   (float) Vertex 3 X
		// Word 10:  (float) 1/Z per X pixel increment
		// Word 11:  (float) Vertex 3 Y
		// Word 12:  (float) 1/Z per Y pixel increment
		// Word 13:  (float) Vertex 4 X (if quad)
		// Word 14:  (float) V/Z per X pixel increment
		// Word 15:  (float) Vertex 4 Y (if quad)
		// Word 16:  (float) V/Z per Y pixel increment
		// Word 17:  (float) Vertex 5 X (if 5 verts)
		// Word 18:  (float) Base U coordinate
		// Word 19:  (float) Vertex 5 Y (if 5 verts)
		// Word 20:  (float) Base Z coordinate
		// Word 21:  (float) Unused? PP code checks for 0 (validity check?)
		// Word 22:  (float) Base V coordinate
		// Word 23:  (float) ? Seems to be a copy of the last X coordinate

		for (int j=0; j < 5; j++)
		{
			float fx = u2f(disp_ram[ii + (j * 4) + 1]);
			float fy = u2f(disp_ram[ii + (j * 4) + 3]);

			vert[j].x = (int)(fx + 256.0f);
			vert[j].y = (int)(-fy + 192.0f);
		}

		rollext_polydata& extra = object_data().next();

		extra.tex_origin_y = (disp_ram[ii + 2] >> 19) & 0x1fff;
		extra.tex_origin_x = (disp_ram[ii + 2] >> 8) & 0x7ff;
		extra.pal = (disp_ram[ii + 0] >> 8) & 0x7f;

		extra.tex_origin_y |= (disp_ram[ii + 0] & 0x01000000) ? 0x2000 : 0;

		extra.uoz_dx = u2f(disp_ram[ii + 6]);
		extra.uoz_dy = u2f(disp_ram[ii + 8]);
		extra.voz_dx = u2f(disp_ram[ii + 14]);
		extra.voz_dy = u2f(disp_ram[ii + 16]);
		extra.ooz_dx = u2f(disp_ram[ii + 10]);
		extra.ooz_dy = u2f(disp_ram[ii + 12]);
		extra.baseu = u2f(disp_ram[ii + 18]);
		extra.basev = u2f(disp_ram[ii + 22]);
		extra.basez = u2f(disp_ram[ii + 20]);

		extra.zmul = (uint32_t)(disp_ram[ii + 4]) >> 7;

		int num_verts = (disp_ram[ii + 0] & 0x7);

		if (disp_ram[ii + 0] & 0x40000000)
		{
			if (num_verts == 4)
				render_polygon<4, 0>(visarea, render_delegate(&rollext_renderer::render_texture_scan<true>, this), vert);
			else if (num_verts == 3)
				render_triangle<0>(visarea, render_delegate(&rollext_renderer::render_texture_scan<true>, this), vert[0], vert[1], vert[2]);
			else if (num_verts == 5)
				render_polygon<5, 0>(visarea, render_delegate(&rollext_renderer::render_texture_scan<true>, this), vert);
		}

	}
}

void rollext_renderer::clear_fb()
{
	rectangle visarea(0, 511, 0, 383);

	m_zb->fill(0xffffffff, visarea);
	m_fb[m_fb_current]->fill(0xff000000, visarea);
}

void rollext_renderer::palette_write(int index, uint16_t data)
{
	m_palette[index] = rgb_t(0xff, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data));
}

void rollext_renderer::texture_mask_write(int index, uint8_t data)
{
	int x = index & 0xff;
	int y = index >> 8;

	// expand to bytes for easier access
	for (auto j = 0; j < 8; j++)
	{
		m_texture_mask[(y * 2048) + (x * 8) + j] = (data & (1 << j)) ? 0xff : 0x00;
	}
}

void rollext_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	wait();
	copybitmap_trans(*bitmap, *m_fb[m_fb_current], 0, 0, 0, 0, cliprect, 0);
}





class rollext_state : public driver_device
{
public:
	rollext_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_disp_ram(*this, "disp_ram"),
		m_screen(*this, "screen"),
		m_in(*this, "INPUTS%u", 1U),
		m_analog(*this, "ANALOG%u", 1U),
		m_eeprom_in(*this, "EEPROMIN"),
		m_eeprom_out(*this, "EEPROMOUT")
	{
	}

	void rollext(machine_config &config);

	void init_rollext();

private:
	required_device<tms32082_mp_device> m_maincpu;
	required_shared_ptr<uint32_t> m_disp_ram;
	required_device<screen_device> m_screen;

	uint32_t a0000000_r(offs_t offset, uint32_t mem_mask = ~0);
	void a0000000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t b0000000_r(offs_t offset);

	void palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void texture_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void cmd_callback(address_space &space, uint32_t data);

	uint32_t fifo_ptr_r(offs_t offset, uint32_t mem_mask = ~0);
	void fifo_ptr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	std::unique_ptr<uint8_t[]> m_texture;
	std::unique_ptr<rollext_renderer> m_renderer;

	uint32_t m_fifo_ptr;

	uint8_t m_adc_reg;
	uint8_t m_adc_input;
	uint8_t m_adc_readbit;

	required_ioport_array<3> m_in;
	required_ioport_array<1> m_analog;
	required_ioport m_eeprom_in;
	required_ioport m_eeprom_out;

	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void preprocess_texture_data();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void memmap(address_map &map) ATTR_COLD;
};

void rollext_state::preprocess_texture_data()
{
	uint8_t *rom = (uint8_t*)memregion("texture")->base();

	for (int j=0; j < 16384; j+=2)
	{
		for (int i=0; i < 2048; i+=4)
		{
			m_texture[((j+0) * 2048) + i + 1] = *rom++;
			m_texture[((j+1) * 2048) + i + 1] = *rom++;
			m_texture[((j+0) * 2048) + i + 0] = *rom++;
			m_texture[((j+1) * 2048) + i + 0] = *rom++;
			m_texture[((j+0) * 2048) + i + 3] = *rom++;
			m_texture[((j+1) * 2048) + i + 3] = *rom++;
			m_texture[((j+0) * 2048) + i + 2] = *rom++;
			m_texture[((j+1) * 2048) + i + 2] = *rom++;
		}
	}
}

void rollext_state::video_start()
{
	m_texture = std::make_unique<uint8_t[]>(0x2000000);

	preprocess_texture_data();

	m_renderer = std::make_unique<rollext_renderer>(*m_screen);
	m_renderer->set_texture_ram(m_texture.get());
}

uint32_t rollext_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_renderer->display(&bitmap, cliprect);
	return 0;
}


uint32_t rollext_state::a0000000_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0:         // inputs
		{
			uint32_t data = 0;

			if (ACCESSING_BITS_16_23)
			{
				// -------- ---x---- -------- -------- ADC channel 0
				// -------- --x----- -------- -------- ADC channel 1?
				// -------- -x------ -------- -------- ADC channel 2?
				// -------- x------- -------- -------- ADC channel 3?

				data |= (m_adc_readbit & 1) ? 0x100000 : 0;

				data |= m_in[0]->read() << 16;
			}
			if (ACCESSING_BITS_8_15)
			{
				data |= 0x200;      // 0 causes inf loop
				data |= m_eeprom_in->read() << 8;
			}
			if (ACCESSING_BITS_0_7)
			{
				data |= m_in[1]->read();
			}

			return data;
		}

		case 1:
			uint32_t data = 0;
			data |= m_in[2]->read();
			return data;
	}

	return 0xffffffff;
}

void rollext_state::a0000000_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_8_15)
		{
			m_eeprom_out->write(data >> 8, 0xff);
		}
	}
	else if (offset == 1)
	{
		if (ACCESSING_BITS_16_23)
		{
			uint8_t newdata = data >> 16;
			if ((newdata & 0x20) == 0 && (m_adc_reg & 0x20) != 0)
			{
				m_adc_input = m_analog[0]->read();
			}

			if (newdata & 0x10)
			{
				m_adc_readbit = (m_adc_input >> 7) & 1;
				m_adc_input <<= 1;
			}

			m_adc_reg = newdata;
		}
	}
}

uint32_t rollext_state::b0000000_r(offs_t offset)
{
	switch (offset)
	{
		case 0:     // ??
			return 0x0000ffff;
		case 1:     // ??
			return 0;
	}

	return 0xffffffff;
}

void rollext_state::cmd_callback(address_space &space, uint32_t data)
{
	uint32_t command = data;

	// PP0
	if (command & 1)
	{
		if (command & 0x00004000)
		{
			// simulate PP behavior for now...
			space.write_dword(0x00000084, 3);

			m_renderer->m_fb_current ^= 1;
			m_renderer->clear_fb();

			m_renderer->process_display_list(*m_screen, m_disp_ram);
			m_maincpu->space().write_dword(0x600ffffc, 0);
		}
	}
	// PP1
	if (command & 2)
	{
		if (command & 0x00004000)
		{
			// simulate PP behavior for now...
			space.write_dword(0x00001014, 3);
		}
	}
}

void rollext_state::palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		m_renderer->palette_write((offset * 2) + 1, data >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		m_renderer->palette_write(offset * 2, data & 0xffff);
	}
}

void rollext_state::texture_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		m_renderer->texture_mask_write((offset * 4) + 1, data >> 24);
	}
	if (ACCESSING_BITS_16_23)
	{
		m_renderer->texture_mask_write((offset * 4) + 0, data >> 16);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_renderer->texture_mask_write((offset * 4) + 3, data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_renderer->texture_mask_write((offset * 4) + 2, data & 0xff);
	}
}




// Master Processor memory map
void rollext_state::memmap(address_map &map)
{
	map(0x40000000, 0x40ffffff).ram().share("main_ram");
	map(0x60000000, 0x600fffff).ram().share("disp_ram");
	map(0x80000000, 0x8000ffff).w(FUNC(rollext_state::palette_w));
	map(0x90000000, 0x9007ffff).w(FUNC(rollext_state::texture_mask_w));
	map(0xa0000000, 0xa00000ff).rw(FUNC(rollext_state::a0000000_r), FUNC(rollext_state::a0000000_w));
	map(0xb0000000, 0xb0000007).r(FUNC(rollext_state::b0000000_r));
	map(0xc0000000, 0xc03fffff).rom().region("rom1", 0);
	map(0xff000000, 0xffffffff).rom().region("rom0", 0);
}


static INPUT_PORTS_START(rollext)
	PORT_START("INPUTS1")
	PORT_SERVICE_NO_TOGGLE(0x8, IP_ACTIVE_LOW)      // test button
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_VOLUME_DOWN)
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_VOLUME_UP)

	PORT_START("INPUTS2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("View Change")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Right Brake")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Left Brake")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Left Smash")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Right Smash")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUTS3")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("EEPROMIN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("EEPROMOUT")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START("ANALOG1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_NAME("Seat Tilt")
INPUT_PORTS_END


uint32_t rollext_state::fifo_ptr_r(offs_t offset, uint32_t mem_mask)
{
	return m_fifo_ptr;
}

void rollext_state::fifo_ptr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_fifo_ptr = data;
	if (m_fifo_ptr > 0)
	{
		// simulate PP behavior for now...
		uint32_t num = m_fifo_ptr;

		int consume_num = num;
		if (consume_num > 32)
			consume_num = 32;

		uint32_t ra = 0x1000280;

		int oldnum = m_maincpu->space().read_dword(0x600ffffc);
		uint32_t rb = 0x60000000 + (oldnum * 0x60);

		for (int i = 0; i < consume_num; i++)
		{
			for (int k = 0; k < 24; k++)
			{
				uint32_t dd = m_maincpu->space().read_dword(ra);
				ra += 4;

				m_maincpu->space().write_dword(rb, dd);
				rb += 4;
			}
		}
		m_maincpu->space().write_dword(0x600ffffc, oldnum + consume_num);
		m_maincpu->space().write_dword(0x00000090, 0);
	}
}



void rollext_state::machine_reset()
{
}

void rollext_state::machine_start()
{
	// hook to fifo pointer for simulating PP0
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x90, 0x93, read32s_delegate(*this, FUNC(rollext_state::fifo_ptr_r)), write32s_delegate(*this, FUNC(rollext_state::fifo_ptr_w)));
}


void rollext_state::rollext(machine_config &config)
{
	TMS32082_MP(config, m_maincpu, 60000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rollext_state::memmap);
	m_maincpu->set_vblank_int("screen", FUNC(rollext_state::vblank_interrupt));
	//m_maincpu->set_periodic_int(FUNC(rollext_state::irq3_line_assert), attotime::from_hz(500));

	tms32082_pp_device &pp0(TMS32082_PP(config, "pp0", 60000000));
	pp0.set_addrmap(AS_PROGRAM, &rollext_state::memmap);

	config.set_maximum_quantum(attotime::from_hz(100));

	EEPROM_93C66_16BIT(config, "eeprom");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 384);
	m_screen->set_visarea(0, 511, 0, 383);
	m_screen->set_screen_update(FUNC(rollext_state::screen_update));
}


INTERRUPT_GEN_MEMBER(rollext_state::vblank_interrupt)
{
	m_maincpu->set_input_line(tms32082_mp_device::INPUT_X1, ASSERT_LINE);
}

void rollext_state::init_rollext()
{
	m_maincpu->set_command_callback(*this, FUNC(rollext_state::cmd_callback));
}


ROM_START(rollext)
	ROM_REGION32_BE(0x1000000, "rom0", 0)
	ROM_LOAD64_DWORD_SWAP("roe.ic17", 0x000000, 0x800000, CRC(a52bf5a1) SHA1(5c35740e14978368ab1c72931a767fcb6f217edf))
	ROM_LOAD64_DWORD_SWAP("roe.ic18", 0x000004, 0x800000, CRC(1a4e65d6) SHA1(69fc7747d8e64d0b397c676bebaedc9e6122fe89))

	ROM_REGION32_BE(0x400000, "rom1", 0)
	ROM_LOAD32_DWORD("roe.ic38", 0x000000, 0x400000, CRC(d9bcfb7c) SHA1(eda9870881732d4dc7cdacb65c6d40af3451dc9d))

	ROM_REGION32_BE(0x2000000, "texture", 0)            // Texture ROMs
	ROM_LOAD32_BYTE("roe.ic45", 0x000000, 0x800000, CRC(0f7fe365) SHA1(ed50fd2b76840eac6ce394a0c748109f615b775a))
	ROM_LOAD32_BYTE("roe.ic47", 0x000001, 0x800000, CRC(44d7ccee) SHA1(2fec682396e4cca704bd1237016acec0e7b4b428))
	ROM_LOAD32_BYTE("roe.ic58", 0x000002, 0x800000, CRC(67ad4561) SHA1(56f41b4ebd827fec49902f377c5ed054c02d9e6c))
	ROM_LOAD32_BYTE("roe.ic60", 0x000003, 0x800000, CRC(a64524af) SHA1(31bef17656ab025f90cd222d3d6d0cb62dee29ee))
ROM_END

} // anonymous namespace


GAME( 1999, rollext, 0, rollext, rollext, rollext_state, init_rollext, ROT0, "Gaelco (Namco America license)", "ROLLing eX.tre.me (US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
