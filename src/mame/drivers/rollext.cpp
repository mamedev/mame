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


    Texture ROM decode:

      {ic45}     {ic47}     {ic58}     {ic60}
    [2,0][0,0] [2,1][0,1] [3,0][1,0] [3,1][1,1]

*/

#include "emu.h"
#include "cpu/tms32082/tms32082.h"
#include "video/poly.h"

struct rollext_polydata
{
	uint32_t tex_bottom;
	uint32_t tex_left;
	uint32_t pal;
};

class rollext_renderer : public poly_manager<float, rollext_polydata, 4, 10000>
{
public:
	rollext_renderer(screen_device &screen)
		: poly_manager<float, rollext_polydata, 4, 10000>(screen)
	{
		m_fb = std::make_unique<bitmap_rgb32>(1024, 1024);
	}

	void render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid);

	void set_texture_ram(uint8_t* texture_ram);
	void set_palette_ram(uint16_t* palette_ram);
	void process_display_list(uint32_t* dispram);

	void clear_fb();
	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);
private:
	std::unique_ptr<bitmap_rgb32> m_fb;

	uint8_t *m_texture_ram;
	uint16_t *m_palette_ram;
};

void rollext_renderer::set_texture_ram(uint8_t* texture_ram)
{
	m_texture_ram = texture_ram;
}

void rollext_renderer::set_palette_ram(uint16_t* palette_ram)
{
	m_palette_ram = palette_ram;
}

void rollext_renderer::render_texture_scan(int32_t scanline, const extent_t &extent, const rollext_polydata &extradata, int threadid)
{
	float u = extent.param[0].start;
	float v = extent.param[1].start;
	float du = extent.param[0].dpdx;
	float dv = extent.param[1].dpdx;

	uint32_t *fb = &m_fb->pix32(scanline);

	uint32_t texbot = extradata.tex_bottom;
	uint32_t texleft = extradata.tex_left;

	int palnum = extradata.pal;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu = (int)(u * 29.0f);
		int iv = (int)(v * 29.0f);

		uint8_t p = m_texture_ram[((texbot - iv) * 2048) + texleft + iu];

		uint16_t texel = m_palette_ram[(palnum * 256) + BYTE_XOR_BE(p)];
		int r = ((texel >> 10) & 0x1f) << 3;
		int g = ((texel >> 5) & 0x1f) << 3;
		int b = (texel & 0x1f) << 3;

		fb[x] = 0xff000000 | (r << 16) | (g << 8) | b;

		u += du;
		v += dv;
	}
}

void rollext_renderer::process_display_list(uint32_t* disp_ram)
{
	const rectangle& visarea = screen().visible_area();

	render_delegate rd = render_delegate(&rollext_renderer::render_texture_scan, this);

	int num = disp_ram[0xffffc/4];

	for (int i=0; i < num; i++)
	{
		int ii = i * 0x60;

		vertex_t vert[4];

		//int x[4];
		//int y[4];

		// Poly data:
		// Word 0:   xxxxxxxx -------- -------- --------   Command? 0xFC for quads
		//           -------- -------- xxxxxxxx --------   Palette?
		//           -------- -------- -------- xxxxxxxx   Number of verts? (4 for quads)

		// Word 1:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 1 X

		// Word 2:   xxxxxxxx xxxxx--- -------- --------   Texture Origin Bottom
		//           -------- -----xxx xxxxxxxx --------   Texture Origin Left

		// Word 3:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 1 Y

		// Word 4:   -------- -------- xxxxxxxx xxxxxxxx   ?

		// Word 5:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 2 X

		// Word 6:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Unknown float

		// Word 7:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 2 Y

		// Word 8:   -------- -------- -------- --------   ?

		// Word 9:   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 3 X

		// Word 10:  -------- -------- -------- --------   ?

		// Word 11:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 3 Y

		// Word 12:  -------- -------- -------- --------   ?

		// Word 13:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 4 X

		// Word 14:  -------- -------- -------- --------   ?

		// Word 15:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Vertex 4 Y

		// Word 16:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Unknown float

		// Word 17:  -------- -------- -------- --------   ?

		// Word 18:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Unknown float

		// Word 19:  -------- -------- -------- --------   ?

		// Word 20:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Unknown float

		// Word 21:  -------- -------- -------- --------   ?

		// Word 22:  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Unknown float

		// Word 23:  -------- -------- -------- --------   ?

		for (int j=0; j < 4; j++)
		{
			uint32_t ix = disp_ram[(ii + (j*0x10) + 0x4) / 4];
			uint32_t iy = disp_ram[(ii + (j*0x10) + 0xc) / 4];

			vert[j].x = (int)((u2f(ix) / 2.0f) + 256.0f);
			vert[j].y = (int)((u2f(iy) / 2.0f) + 192.0f);
		}

		vert[0].p[0] = 0.0f;        vert[0].p[1] = 1.0f;
		vert[1].p[0] = 0.0f;        vert[1].p[1] = 0.0f;
		vert[2].p[0] = 1.0f;        vert[2].p[1] = 0.0f;
		vert[3].p[0] = 1.0f;        vert[3].p[1] = 1.0f;

		rollext_polydata &extra = object_data_alloc();

		extra.tex_bottom = (disp_ram[(ii + 8) / 4] >> 19) & 0x1fff;
		extra.tex_left = (disp_ram[(ii + 8) / 4] >> 8) & 0x7ff;
		extra.pal = (disp_ram[(ii + 0) / 4] >> 8) & 0x1f;

#if 0
		printf("P%d\n", i);
		for (int j=0; j < 6; j++)
		{
			printf("   %08X %08X %08X %08X", disp_ram[(ii + (j*0x10) + 0) / 4], disp_ram[(ii + (j*0x10) + 4) / 4], disp_ram[(ii + (j*0x10) + 8) / 4], disp_ram[(ii + (j*0x10) + 12) / 4]);
			printf("   %f %f %f %f\n", u2f(disp_ram[(ii + (j*0x10) + 0) / 4]), u2f(disp_ram[(ii + (j*0x10) + 4) / 4]), u2f(disp_ram[(ii + (j*0x10) + 8) / 4]), u2f(disp_ram[(ii + (j*0x10) + 12) / 4]));
		}
#endif

		render_triangle(visarea, rd, 4, vert[0], vert[1], vert[2]);
		render_triangle(visarea, rd, 4, vert[0], vert[2], vert[3]);

	}

	wait();
}

void rollext_renderer::clear_fb()
{
	rectangle visarea;
	visarea.min_x = 0;
	visarea.max_x = 511;
	visarea.min_y = 0;
	visarea.max_y = 383;

	m_fb->fill(0xff000000, visarea);

}

void rollext_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	copybitmap_trans(*bitmap, *m_fb, 0, 0, 0, 0, cliprect, 0);
}





class rollext_state : public driver_device
{
public:
	rollext_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette_ram(*this, "palette_ram"),
		m_texture_mask(*this, "texture_mask"),
		m_disp_ram(*this, "disp_ram"),
		m_screen(*this, "screen")
	{
	}

	void rollext(machine_config &config);

	void init_rollext();

private:
	required_device<tms32082_mp_device> m_maincpu;
	required_shared_ptr<uint32_t> m_palette_ram;
	required_shared_ptr<uint32_t> m_texture_mask;
	required_shared_ptr<uint32_t> m_disp_ram;
	required_device<screen_device> m_screen;

	DECLARE_READ32_MEMBER(a0000000_r);
	DECLARE_WRITE32_MEMBER(a0000000_w);
	DECLARE_READ32_MEMBER(b0000000_r);

	DECLARE_WRITE32_MEMBER(cmd_callback);

	std::unique_ptr<uint8_t[]> m_texture;
	std::unique_ptr<rollext_renderer> m_renderer;

	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void preprocess_texture_data();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void memmap(address_map &map);
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
	m_renderer->set_palette_ram((uint16_t*)&m_palette_ram[0]);
}

uint32_t rollext_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#if 0
	uint16_t *pal = (uint16_t*)&m_palette_ram[0];

	int palnum = 31;
	// 24,25,31 for basic font
	// 29 = trees

	int ii=0;
	for (int j=0; j < 384; j++)
	{
		uint32_t *fb = &bitmap.pix32(j);
		for (int i=0; i < 512; i++)
		{
			uint8_t p = m_texture[ii++];

			uint16_t rgb = pal[(palnum * 256) + BYTE_XOR_BE(p)];
			int r = ((rgb >> 10) & 0x1f) << 3;
			int g = ((rgb >> 5) & 0x1f) << 3;
			int b = (rgb & 0x1f) << 3;

			fb[i] = 0xff000000 | (r << 16) | (g << 8) | b;
		}
		ii += 1536;
	}
#endif

	m_renderer->display(&bitmap, cliprect);

	//m_renderer->clear_fb();

	//m_disp_ram[0xffffc/4] = 0;


	return 0;
}


READ32_MEMBER(rollext_state::a0000000_r)
{
	switch (offset)
	{
		case 0:         // ??
		{
			uint32_t data = 0x20200;

			//data |= ioport("INPUTS1")->read();
			//data |= 0xfff7fff7;

			return data;
		}

		case 1:
			return 0xffffffff;
	}

	return 0xffffffff;
}

WRITE32_MEMBER(rollext_state::a0000000_w)
{
	// FPGA interface?
}

READ32_MEMBER(rollext_state::b0000000_r)
{
	switch (offset)
	{
		case 0:     // ??
			return 0x0000ffff;
		case 1:     // ??
			return 0;
	}

	return 0;
}

WRITE32_MEMBER(rollext_state::cmd_callback)
{
	uint32_t command = data;

	// PP0
	if (command & 1)
	{
		if (command & 0x00004000)
		{
			// simulate PP behavior for now...
			space.write_dword(0x00000084, 3);

			uint32_t num = space.read_dword(0x90);

			int consume_num = num;
			if (consume_num > 32)
				consume_num = 32;

			printf("PP num %d\n", num);
			printf("0x00000084 = %08X\n", space.read_dword(0x84));


			uint32_t ra = 0x1000280;

			/*
			printf("FIFO push:\n");

			for (int i=0; i < consume_num; i++)
			{
			    printf("Entry %d:\n", i);
			    for (int k=0; k < 6; k++)
			    {
			        for (int l=0; l < 4; l++)
			        {
			            uint32_t dd = m_program->read_dword(ra);
			            ra += 4;

			            printf("%08X(%f) ", dd, u2f(dd));
			        }
			        printf("\n");
			    }
			    printf("\n");
			}
			*/

			ra = 0x1000280;

			int oldnum = space.read_dword(0x600ffffc);
			uint32_t rb = 0x60000000 + (oldnum * 0x60);

			for (int i=0; i < consume_num; i++)
			{
				for (int k=0; k < 24; k++)
				{
					uint32_t dd = space.read_dword(ra);
					ra += 4;

					space.write_dword(rb, dd);
					rb += 4;
				}
			}
			space.write_dword(0x600ffffc, oldnum+consume_num);

			m_renderer->process_display_list(m_disp_ram);

			space.write_dword(0x600ffffc, 0);

			space.write_dword(0x00000090, 0);
			space.write_dword(0x00000094, 0);

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


// Master Processor memory map
void rollext_state::memmap(address_map &map)
{
	map(0x40000000, 0x40ffffff).ram().share("main_ram");
	map(0x60000000, 0x600fffff).ram().share("disp_ram");
	map(0x80000000, 0x8000ffff).ram().share("palette_ram");
	map(0x90000000, 0x9007ffff).ram().share("texture_mask");
	map(0xa0000000, 0xa00000ff).rw(FUNC(rollext_state::a0000000_r), FUNC(rollext_state::a0000000_w));
	map(0xb0000000, 0xb0000007).r(FUNC(rollext_state::b0000000_r));
	map(0xc0000000, 0xc03fffff).rom().region("rom1", 0);
	map(0xff000000, 0xffffffff).ram().region("rom0", 0);
}


static INPUT_PORTS_START(rollext)

	PORT_START("INPUTS1")
	PORT_BIT( 0xfff7fff7, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00080008, IP_ACTIVE_LOW) /* Test Button */

INPUT_PORTS_END


void rollext_state::machine_reset()
{
}

void rollext_state::machine_start()
{
}


void rollext_state::rollext(machine_config &config)
{
	TMS32082_MP(config, m_maincpu, 60000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rollext_state::memmap);
	//m_maincpu->set_vblank_int("screen", FUNC(rollext_state::vblank_interrupt));
	m_maincpu->set_periodic_int(FUNC(rollext_state::irq1_line_assert), attotime::from_hz(60));
	//m_maincpu->set_periodic_int(FUNC(rollext_state::irq3_line_assert), attotime::from_hz(500));

	tms32082_pp_device &pp0(TMS32082_PP(config, "pp0", 60000000));
	pp0.set_addrmap(AS_PROGRAM, &rollext_state::memmap);

	config.set_maximum_quantum(attotime::from_hz(100));

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


GAME( 1999, rollext, 0, rollext, rollext, rollext_state, init_rollext, ROT0, "Gaelco", "ROLLing eX.tre.me", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
