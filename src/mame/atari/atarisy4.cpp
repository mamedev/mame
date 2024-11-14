// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Atari System IV hardware

    preliminary driver by Phil Bennett

    Games supported:
        * The Last Starfighter (prototype)
        * Air Race (prototype)

    To do:
        * Finish video implementation
        * Find missing HUD/text data for both games

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "video/poly.h"
#include "emupal.h"
#include "screen.h"


namespace {

class atarisy4_state : public driver_device
{
public:
	atarisy4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dsp0(*this, "dsp0"),
		m_dsp0_bank1(*this, "dsp0_bank1"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_m68k_ram(*this, "m68k_ram"),
		m_screen_ram(*this, "screen_ram"),
		m_stick(*this, "STICK%c", 'X')
	{
	}

	void atarisy4(machine_config &config);

	void init_laststar();

protected:
	struct atarisy4_polydata
	{
		uint16_t color = 0;
		uint16_t *screen_ram = nullptr;
	};

	class atarisy4_renderer : public poly_manager<float, atarisy4_polydata, 2, POLY_FLAG_NO_WORK_QUEUE>
	{
	public:
		atarisy4_renderer(atarisy4_state &state, screen_device &screen);
		~atarisy4_renderer() {}

		void draw_scanline(int32_t scanline, const extent_t &extent, const atarisy4_polydata &extradata, int threadid);
		void draw_polygon(uint16_t color);

	protected:
		atarisy4_state &m_state;
	};

	struct gpu
	{
		uint32_t xy_to_screen_addr(uint32_t x, uint32_t y) const;

		/* Memory-mapped registers */
		uint16_t gr[8]{};   /* Command parameters */

		uint16_t bcrw = 0;    /* Screen buffer W control */
		uint16_t bcrx = 0;    /* Screen buffer X control */
		uint16_t bcry = 0;    /* Screen buffer Y control */
		uint16_t bcrz = 0;    /* Screen buffer Z control */
		uint16_t psrw = 0;
		uint16_t psrx = 0;
		uint16_t psry = 0;
		uint16_t psrz = 0;

		uint16_t dpr = 0;
		uint16_t ctr = 0;
		uint16_t lfr = 0;
		uint16_t ifr = 0;
		uint16_t ecr = 0;     /* Execute command register */
		uint16_t far = 0;
		uint16_t mcr = 0;     /* Interrupt control */
		uint16_t qlr = 0;
		uint16_t qar = 0;

		uint16_t dhr = 0;     /* Scanline counter */
		uint16_t dlr = 0;

		/* Others */
		uint16_t idr = 0;
		uint16_t icd = 0;

		uint8_t  transpose = 0;
		uint8_t  vblank_wait = 0;

		/* Polygon points */
		struct
		{
			int16_t x = 0;
			int16_t y = 0;
		} points[16];

		uint16_t pt_idx = 0;
		bool   poly_open = false;

		uint16_t clip_min_x = 0;
		uint16_t clip_max_x = 0;
		uint16_t clip_min_y = 0;
		uint16_t clip_max_y = 0;
	};

	void gpu_w(offs_t offset, uint16_t data);
	uint16_t gpu_r(offs_t offset);
	uint16_t m68k_shared_0_r(offs_t offset);
	void m68k_shared_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68k_shared_1_r(offs_t offset);
	void m68k_shared_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp0_status_r();
	void dsp0_control_w(uint16_t data);
	int dsp0_bio_r();
	void dsp0_bank_w(uint16_t data);
	uint16_t analog_r();

	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	uint32_t screen_update_atarisy4(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_int);

	void image_mem_to_screen( bool clip);
	void execute_gpu_command();
	inline uint8_t hex_to_ascii(uint8_t in);
	void load_ldafile(address_space &space, const uint8_t *file);
	void load_hexfile(address_space &space, const uint8_t *file);

	void main_map(address_map &map) ATTR_COLD;
	void dsp0_map(address_map &map) ATTR_COLD;
	void dsp0_io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<tms32010_device> m_dsp0;

	required_memory_bank m_dsp0_bank1;

	uint16_t m_dsp_bank[2]{};
	uint8_t m_csr[2]{};
	std::unique_ptr<uint16_t[]> m_shared_ram[2];

private:
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_m68k_ram;
	required_shared_ptr<uint16_t> m_screen_ram;

	required_ioport_array<2> m_stick;

	std::unique_ptr<atarisy4_renderer> m_renderer;

	gpu m_gpu;

	uint8_t m_r_color_table[256]{};
	uint8_t m_g_color_table[256]{};
	uint8_t m_b_color_table[256]{};
};


class airrace_state : public atarisy4_state
{
public:
	airrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarisy4_state(mconfig, type, tag),
		m_dsp1(*this, "dsp1"),
		m_dsp1_bank1(*this, "dsp1_bank1")
	{
	}

	void init_airrace();

	void airrace(machine_config &config);

protected:
	uint16_t dsp1_status_r();
	void dsp1_control_w(uint16_t data);
	int dsp1_bio_r();
	void dsp1_bank_w(uint16_t data);

	virtual void machine_reset() override ATTR_COLD;

	void airrace_map(address_map &map) ATTR_COLD;
	void dsp1_map(address_map &map) ATTR_COLD;
	void dsp1_io_map(address_map &map) ATTR_COLD;

private:
	required_device<tms32010_device> m_dsp1;
	required_memory_bank m_dsp1_bank1;
};



/*************************************
 *
 *  Video hardware
 *
 *************************************/

atarisy4_state::atarisy4_renderer::atarisy4_renderer(atarisy4_state &state, screen_device &screen) :
	poly_manager<float, atarisy4_polydata, 2, POLY_FLAG_NO_WORK_QUEUE>(screen.machine()),
	m_state(state)
{
}

void atarisy4_state::video_start()
{
	m_renderer = std::make_unique<atarisy4_renderer>(*this, *m_screen);
}

void atarisy4_state::video_reset()
{
	m_gpu.vblank_wait = 0;
}

uint32_t atarisy4_state::screen_update_atarisy4(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t offset = 0;

	if (m_gpu.bcrw & 0x80)
	{
		offset = 0;
	}
	else if (m_gpu.bcrx & 0x80)
	{
		offset = 0x10 << 5;
	}

	//uint32_t offset = m_gpu.dpr << 5;

	for (int y = cliprect.top(); y <= cliprect.bottom(); ++y)
	{
		uint16_t const *src = &m_screen_ram[(offset + (4096 * y)) / 2];
		uint32_t *dest = &bitmap.pix(y, cliprect.left());

		for (int x = cliprect.left(); x < cliprect.right(); x += 2)
		{
			uint16_t const data = *src++;

			*dest++ = m_palette->pen(data & 0xff);
			*dest++ = m_palette->pen(data >> 8);
		}
	}
	return 0;
}

inline uint32_t atarisy4_state::gpu::xy_to_screen_addr(uint32_t x, uint32_t y) const
{
//  uint32_t offset = ((mcr >> 4) & 3) << 9;
	uint32_t offset = 0;

	if (~bcrw & 0x80)
	{
		offset = 0;
	}
	else if (~bcrx & 0x80)
	{
		offset = 0x10 << 5;
	}

	return (y * 4096) + offset + x;
}

void atarisy4_state::image_mem_to_screen(bool clip)
{
	int16_t y = m_gpu.gr[1] - 0x200;
	uint16_t h = m_gpu.gr[3];

	if (h & 0x8000)
		h = -h;

	/* Not 100% sure of this */
	while (h--)
	{
		uint16_t w = m_gpu.gr[2];
		int16_t x = m_gpu.gr[0] - 0x400;

		if (w & 0x8000)
			w = -w;

		if (y >= 0 && y <= 511)
		{
			while (w--)
			{
				if (x >= 0 && x <= 511)
				{
					uint16_t pix = m_screen_ram[m_gpu.xy_to_screen_addr(x,y) >> 1];

					if (x & 1)
						pix = (pix & (0x00ff)) | m_gpu.idr << 8;
					else
						pix = (pix & (0xff00)) | m_gpu.idr;

					m_screen_ram[m_gpu.xy_to_screen_addr(x,y) >> 1] = pix;
				}
				++x;
			}
		}
		++y;
	}
}

void atarisy4_state::atarisy4_renderer::draw_scanline(int32_t scanline, const extent_t &extent, const atarisy4_polydata &extradata, int threadid)
{
	uint16_t color = extradata.color;
	int x;

	for (x = extent.startx; x < extent.stopx; ++x)
	{
		uint32_t addr = m_state.m_gpu.xy_to_screen_addr(x, scanline);
		uint16_t pix = extradata.screen_ram[addr >> 1];

		if (x & 1)
			pix = (pix & (0x00ff)) | color << 8;
		else
			pix = (pix & (0xff00)) | color;

		extradata.screen_ram[addr >> 1] = pix;
	}
}

void atarisy4_state::atarisy4_renderer::draw_polygon(uint16_t color)
{
	rectangle clip;
	vertex_t v1{}, v2{}, v3{};
	atarisy4_polydata &extradata = object_data().next();
	render_delegate rd_scan = render_delegate(&atarisy4_renderer::draw_scanline, this);

	clip.set(0, 511, 0, 511);

	extradata.color = color;
	extradata.screen_ram = m_state.m_screen_ram;

	v1.x = m_state.m_gpu.points[0].x;
	v1.y = m_state.m_gpu.points[0].y;

	v2.x = m_state.m_gpu.points[1].x;
	v2.y = m_state.m_gpu.points[1].y;

	/* Draw a triangle fan */
	for (int i = 2; i <= m_state.m_gpu.pt_idx; ++i)
	{
		v3.x = m_state.m_gpu.points[i].x;
		v3.y = m_state.m_gpu.points[i].y;

		render_triangle<1>(clip, rd_scan, v1, v2, v3);
		v2 = v3;
	}
}

/*
    GPU commands

    REXP    0x00    Reset expansion
    SEXP    0x01    Set expansion
    ROFF    0x02    Reset offset on update
    SOFF    0x03    Set offset
    RTPS    0x04    Reset transposition
    STPS    0x05    Set transposition
    SIMD    0x06    Load IDR register
    SICD    0x07    Load partial transfer compare value
    RCW     0x08    Reset clipping window
    SCW     0x09    Set clipping window
    WVB     0x0B    Wait for VBlank
    FXOT    0x10    Fill x offset table
    FYOT    0x11    Fill y offset table
    FERT    0x14    Fill expansion table
    FCT     0x15    Fill color table
    SMCT    0x16    Screen memory to color table
    ITSN    0x20    Image memory to screen memory transfer
    ITSC    0x21    Image memory to screen memory transfer with clip
    PPA     0x24    Plot point absolute
    PFOA    0x28    Polygon open absolute
    PFOR    0x29    Polygon open relative
    PFVA    0x2A    Polygon vector absolute
    PFVR    0x2B    Polygon vector relative
    PFC     0x2C    Polygon close
*/
void atarisy4_state::execute_gpu_command()
{
	switch (m_gpu.ecr)
	{
		case 0x04:
		{
			m_gpu.transpose = 0;
			break;
		}
		case 0x05:
		{
			m_gpu.transpose = 1;
			break;
		}
		case 0x06:
		{
			m_gpu.idr = m_gpu.gr[0];
			break;
		}
		case 0x07:
		{
			m_gpu.icd = m_gpu.gr[0];
			break;
		}
		case 0x09:
		{
			m_gpu.clip_max_x = m_gpu.gr[0];
			m_gpu.clip_min_x = m_gpu.gr[1];
			m_gpu.clip_max_y = m_gpu.gr[2];
			m_gpu.clip_min_y = m_gpu.gr[3];
			break;
		}
		case 0x0b:
		{
			// Wait for VBLANK and swap buffers?
			m_gpu.vblank_wait = 1;
			break;
		}
		case 0x16:
		{
			/*
			    Copy screen RAM to color RAM
			    GR0 : Color start X
			    GR1 : Color start Y
			    GR2 : Color table offset
			    GR3 : Size
			    GR4 : Channels to set (R: 0x10, G: 0x20, B: 0x40)
			*/
			int i;
			int offset = m_gpu.xy_to_screen_addr(m_gpu.gr[0] - 0x400, m_gpu.gr[1] - 0x200);
			int table_offs = m_gpu.gr[2];

			for (i = 0; i < m_gpu.gr[3]; ++i)
			{
				uint16_t val = m_screen_ram[offset >> 1];
				val >>= (~offset & 1) << 3;

				if (m_gpu.gr[4] & 0x10)
					m_r_color_table[table_offs] = val;
				if (m_gpu.gr[4] & 0x20)
					m_g_color_table[table_offs] = val;
				if (m_gpu.gr[4] & 0x40)
					m_b_color_table[table_offs] = val;

				/* Update */
				m_palette->set_pen_color(table_offs, rgb_t(m_r_color_table[table_offs], m_g_color_table[table_offs], m_b_color_table[table_offs]));

				++table_offs;
				++offset;
			}

			break;
		}
		case 0x20:
		{
			image_mem_to_screen(false);
			break;
		}
		case 0x21:
		{
			image_mem_to_screen(true);
			break;
		}
		case 0x28:
		{
			m_gpu.points[0].x = m_gpu.gr[0] - 0x400;
			m_gpu.points[0].y = m_gpu.gr[1] - 0x200;
			m_gpu.pt_idx = 0;
			break;
		}
		case 0x29:
		{
			m_gpu.points[0].x = m_gpu.points[m_gpu.pt_idx].x + m_gpu.gr[0];
			m_gpu.points[0].y = m_gpu.points[m_gpu.pt_idx].y + m_gpu.gr[1];
			m_gpu.pt_idx = 0;
			break;
		}
		case 0x2a:
		{
			++m_gpu.pt_idx;
			m_gpu.points[m_gpu.pt_idx].x = m_gpu.gr[0] - 0x400;
			m_gpu.points[m_gpu.pt_idx].y = m_gpu.gr[1] - 0x200;
			break;
		}
		case 0x2b:
		{
			uint16_t x = m_gpu.points[m_gpu.pt_idx].x + m_gpu.gr[0];
			uint16_t y = m_gpu.points[m_gpu.pt_idx].y + m_gpu.gr[1];
			++m_gpu.pt_idx;
			m_gpu.points[m_gpu.pt_idx].x = x;
			m_gpu.points[m_gpu.pt_idx].y = y;
			break;
		}
		case 0x2c:
		{
			m_renderer->draw_polygon(m_gpu.gr[2]);
			m_renderer->wait();
			break;
		}
		default:
			logerror("GPU COMMAND: %x\n", m_gpu.ecr);
	}
}

void atarisy4_state::gpu_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_gpu.gr[0] = data;   break;
		case 0x01:  m_gpu.gr[1] = data;   break;
		case 0x02:  m_gpu.gr[2] = data;   break;
		case 0x03:  m_gpu.gr[3] = data;   break;
		case 0x04:  m_gpu.gr[4] = data;   break;
		case 0x05:  m_gpu.gr[5] = data;   break;
		case 0x06:  m_gpu.gr[6] = data;   break;
		case 0x07:  m_gpu.gr[7] = data;   break;

		case 0x08:  m_gpu.bcrw = data;    break;
		case 0x09:  m_gpu.bcrx = data;    break;
		case 0x0a:  m_gpu.bcry = data;    break;
		case 0x0b:  m_gpu.bcrz = data;    break;
		case 0x0c:  m_gpu.psrw = data;    break;
		case 0x0d:  m_gpu.psrx = data;    break;
		case 0x0e:  m_gpu.psry = data;    break;
		case 0x0f:  m_gpu.psrz = data;    break;

		case 0x14:  m_gpu.dpr = data;     break;
		case 0x15:  m_gpu.ctr = data;     break;
		case 0x16:  m_gpu.ifr = data;     break;
		case 0x17:
		{
			m_gpu.ecr = data;
			execute_gpu_command();
			break;
		}
		case 0x1a:  m_gpu.far = data;     break;
		case 0x20:
		{
			m_gpu.mcr = data;

			if (~data & 0x08)
				m_maincpu->set_input_line(6, CLEAR_LINE);

			break;
		}

		case 0x21:  m_gpu.qlr = data;     break;
		case 0x22:  m_gpu.qar = data;     break;
	}
}

uint16_t atarisy4_state::gpu_r(offs_t offset)
{
	uint16_t res = 0;

	switch (offset)
	{
		case 0x08:  res = m_gpu.bcrw;     break;
		case 0x09:  res = m_gpu.bcrx;     break;
		case 0x0a:  res = m_gpu.bcry;     break;
		case 0x0b:  res = m_gpu.bcrz;     break;

		case 0x20:  res = m_gpu.mcr;      break;

		case 0x400: res = 5;            break; // TODO!
		case 0x420: res = 5;            break;

		default:    logerror("GPU_R[%x]\n", offset);
	}

	return res;
}

INTERRUPT_GEN_MEMBER(atarisy4_state::vblank_int)
{
	if (m_gpu.mcr & 0x08)
		m_maincpu->set_input_line(6, ASSERT_LINE);
}


/*************************************
 *
 *  68000<->TMS comms
 *
 *************************************/

uint16_t atarisy4_state::m68k_shared_0_r(offs_t offset)
{
	if (!BIT(m_csr[0], 3))
		return (m_shared_ram[0][offset]);
	else
		return 0xffff;
}

void atarisy4_state::m68k_shared_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!BIT(m_csr[0], 3))
		COMBINE_DATA(&m_shared_ram[0][offset]);
}

uint16_t atarisy4_state::m68k_shared_1_r(offs_t offset)
{
	if (!BIT(m_csr[1], 3))
		return (m_shared_ram[1][offset]);
	else
		return 0xffff;
}

void atarisy4_state::m68k_shared_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!BIT(m_csr[1], 3))
		COMBINE_DATA(&m_shared_ram[1][offset]);
}

uint16_t atarisy4_state::dsp0_status_r()
{
	return m_csr[0];
}

void atarisy4_state::dsp0_control_w(uint16_t data)
{
	m_dsp0->set_input_line(INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	m_dsp0->set_input_line(0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	m_csr[0] = data;
}

int atarisy4_state::dsp0_bio_r()
{
	return BIT(m_csr[0], 2);
}

void atarisy4_state::dsp0_bank_w(uint16_t data)
{
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		m_csr[0] |= 0x10;

		if (BIT(m_csr[0], 5) == 1)
			fatalerror("68000 interrupt enable was set!\n");
	}

	data &= 0x3800;
	m_dsp0_bank1->set_base(&m_shared_ram[0][data]);
	m_dsp_bank[0] = data;
}

uint16_t airrace_state::dsp1_status_r()
{
	return m_csr[1];
}

void airrace_state::dsp1_control_w(uint16_t data)
{
	m_dsp1->set_input_line(INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	m_dsp1->set_input_line(0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	m_csr[1] = data;
}

int airrace_state::dsp1_bio_r()
{
	return BIT(m_csr[1], 2);
}

void airrace_state::dsp1_bank_w(uint16_t data)
{
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		m_csr[1] |= 0x10;

		if (BIT(m_csr[1], 5) == 1)
			fatalerror("68000 interrupt enable was set!\n");
	}

	data &= 0x3800;
	m_dsp1_bank1->set_base(&m_shared_ram[1][data]);
	m_dsp_bank[1] = data;
}


/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void atarisy4_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("m68k_ram");
	map(0x010000, 0x01ffff).ram();
	map(0x580000, 0x580001).portr("JOYSTICK");
	map(0x588000, 0x588001).r(FUNC(atarisy4_state::analog_r));
	map(0x598000, 0x598001).noprw(); /* Sound board */
	map(0x7c0000, 0x7c4fff).rw(FUNC(atarisy4_state::m68k_shared_1_r), FUNC(atarisy4_state::m68k_shared_1_w));
	map(0x7f0000, 0x7f4fff).rw(FUNC(atarisy4_state::m68k_shared_0_r), FUNC(atarisy4_state::m68k_shared_0_w));
	map(0x7f6000, 0x7f6001).rw(FUNC(atarisy4_state::dsp0_status_r), FUNC(atarisy4_state::dsp0_control_w));
	map(0xa00400, 0xbfffff).ram().share("screen_ram");
	map(0xff8000, 0xff8fff).rw(FUNC(atarisy4_state::gpu_r), FUNC(atarisy4_state::gpu_w));
}

void airrace_state::airrace_map(address_map &map)
{
	main_map(map);

	map(0x7c6000, 0x7c6001).rw(FUNC(airrace_state::dsp1_status_r), FUNC(airrace_state::dsp1_control_w));
}


/*************************************
 *
 *  Mathbox DSP memory map
 *
 *************************************/

void atarisy4_state::dsp0_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x07ff).bankrw("dsp0_bank0");
	map(0x0800, 0x0fff).bankrw("dsp0_bank1");
}

void atarisy4_state::dsp0_io_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(atarisy4_state::dsp0_bank_w));
}


/*************************************
 *
 *  Mathbox DSP memory map
 *
 *************************************/

void airrace_state::dsp1_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x07ff).bankrw("dsp1_bank0");
	map(0x0800, 0x0fff).bankrw("dsp1_bank1");
}

void airrace_state::dsp1_io_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(airrace_state::dsp1_bank_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

uint16_t atarisy4_state::analog_r()
{
	return (m_stick[0]->read() << 8) | m_stick[1]->read();
}

static INPUT_PORTS_START( atarisy4 )
	PORT_START("JOYSTICK")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atarisy4_state::atarisy4(machine_config &config)
{
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &atarisy4_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(atarisy4_state::vblank_int));

	TMS32010(config, m_dsp0, 16000000);
	m_dsp0->set_addrmap(AS_PROGRAM, &atarisy4_state::dsp0_map);
	m_dsp0->set_addrmap(AS_IO, &atarisy4_state::dsp0_io_map);
	m_dsp0->bio().set(FUNC(atarisy4_state::dsp0_bio_r));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(32000000/2, 660, 0, 512, 404, 0, 384);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_screen_update(FUNC(atarisy4_state::screen_update_atarisy4));

	PALETTE(config, m_palette).set_entries(256);
}

void airrace_state::airrace(machine_config &config)
{
	atarisy4(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &airrace_state::airrace_map);

	TMS32010(config, m_dsp1, 16000000);
	m_dsp1->set_addrmap(AS_PROGRAM, &airrace_state::dsp1_map);
	m_dsp1->set_addrmap(AS_IO, &airrace_state::dsp1_io_map);
	m_dsp1->bio().set(FUNC(airrace_state::dsp1_bio_r));
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( laststar )
	ROM_REGION( 0x20000, "code", 0 )
	ROM_LOAD( "demo.hex",  0x00000, 0x1c2c4, CRC(7f9e344e) SHA1(ff1462f4f3fa01c47b74d365c240b1c3fdd36755) )

	ROM_REGION( 0x20000, "data", 0 )
	ROM_LOAD( "data1.hex", 0x00000, 0x077cb, CRC(0e75efc0) SHA1(badfcadd92625637bb3223bee986a29428c8d35a) )

	ROM_REGION( 0x20000, "dsp", 0 )
	ROM_LOAD( "mathbs.lda", 0x00000, 0x01e00, CRC(4378739a) SHA1(7c360da99a1366a3315f4846244ed3b2514ef7ec) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "2d_data", 0x00000, 0x077cb, NO_DUMP )
ROM_END


ROM_START( airrace )
	ROM_REGION( 0x30000, "code", 0 )
	ROM_LOAD( "demo.hex",  0x00000, 0x25d23, CRC(937fa025) SHA1(c393a05645ad5df9268f03fffad294477e6f090b) )

	ROM_REGION( 0x20000, "dsp", 0 )
	ROM_LOAD( "mathbs.lda", 0x00000, 0x01e00, CRC(d215acd4) SHA1(853e792b343dc6bcb5b11d455af478a79edfd740) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "2d_data", 0x00000, 0x077cb, NO_DUMP )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

uint8_t atarisy4_state::hex_to_ascii(uint8_t in)
{
	if (in < 0x3a)
		return in - 0x30;
	else if (in < 0x47)
		return in - 0x37;
	else
		return in;
}

void atarisy4_state::load_ldafile(address_space &space, const uint8_t *file)
{
#define READ_CHAR()     file[i++]
	int i = 0;

	while (true)
	{
		uint8_t c;
		uint8_t sum = 1;
		uint16_t len;
		uint16_t addr;
		uint8_t pval = 0;

		if (READ_CHAR() != 0x01)
			fatalerror("Bad .LDA file\n");

		if (READ_CHAR() != 0x00)
			fatalerror("Bad .LDA file\n");

		len = READ_CHAR();
		sum += len;

		c = READ_CHAR();
		len |= c << 8;
		sum += c;

		/* End of file */
		if (len == 6)
			break;

		addr = READ_CHAR();
		sum += addr;

		c = READ_CHAR();
		addr |= c << 8;
		sum += c;

		len -= 6;

		addr <<= 1;
		do
		{
			uint8_t data = READ_CHAR();
			sum += data;
			if (addr & 1)
				space.write_word(addr >> 1, (pval << 8) | data);
			else
				pval = data;
			addr ++;
		} while (--len);

		sum += READ_CHAR();

		if (sum != 0)
			fatalerror(".LDA checksum failure\n");
	}
}

/* Load memory space with data from a Tektronix-Extended HEX file */
void atarisy4_state::load_hexfile(address_space &space, const uint8_t *file)
{
#define READ_HEX_CHAR()     hex_to_ascii(file[i++])

	uint32_t i = 0;
	uint32_t line = 1;
	bool end = false;

	while (true)
	{
		uint8_t len;
		uint8_t record;
		uint8_t checksum;
		uint8_t sum = 0;
		uint8_t addr_len;
		uint32_t addr = 0;

		/* Ignore blank lines */
		while (file[i] == '\n') i++;

		/* First character of each line should be a '%' */
		if (file[i++] != '%')
			fatalerror("Error on line %d - invalid line start character\n", line);

		/* Get the line length */
		len = READ_HEX_CHAR() << 4;
		len |= READ_HEX_CHAR();

		sum += len & 0xf;
		sum += len >> 4;

		/* Get the record type */
		record = READ_HEX_CHAR();
		sum += record;

		/* Record type */
		if (record == 8)
		{
			end = true;
		}
		else if (record == 3)
		{
			/* Ignore this type */
			i += len - 2;
			goto next_line;
		}
		else if (record != 6)
		{
			fatalerror("Error on line %d - Invalid record type %d\n", line, record);
		}

		/* Get the checksum for this line */
		checksum = READ_HEX_CHAR() << 4;
		checksum |= READ_HEX_CHAR();

		/* Get the number of address digits */
		addr_len = READ_HEX_CHAR();
		sum += addr_len;

		len = len - 6 - addr_len;

		/* Form the address */
		while (addr_len)
		{
			uint8_t digit;

			addr <<= 4;
			digit = READ_HEX_CHAR();
			sum += digit;
			addr |= digit;
			--addr_len;
		}

		/* Now read the data */
		while (len)
		{
			uint8_t data;

			data = READ_HEX_CHAR() << 4;
			data |= READ_HEX_CHAR();
			sum += data >> 4;
			sum += data & 0xf;

			if (record == 6)
				space.write_byte(addr++, data);

			len -= 2;
		}

		/* Eat the carriage return */
		++i;

		if (sum != checksum)
			fatalerror("Checksum mismatch on line %d (Found 0x%.2x but expected 0x%.2x)\n", line, sum, checksum);

next_line:
		++line;

		if (end == true)
			break;
	}
}

void atarisy4_state::init_laststar()
{
	address_space &main = m_maincpu->space(AS_PROGRAM);

	/* Allocate 16kB of shared RAM */
	m_shared_ram[0] = make_unique_clear<uint16_t[]>(0x2000);

	/* Populate the 68000 address space with data from the HEX files */
	load_hexfile(main, memregion("code")->base());
	load_hexfile(main, memregion("data")->base());

	/* Set up the DSP */
	membank("dsp0_bank0")->set_base(m_shared_ram[0].get());
	m_dsp0_bank1->set_base(&m_shared_ram[0][0x800]);
	load_ldafile(m_dsp0->space(AS_PROGRAM), memregion("dsp")->base());
}

void airrace_state::init_airrace()
{
	/* Allocate two sets of 32kB shared RAM */
	m_shared_ram[0] = make_unique_clear<uint16_t[]>(0x4000);
	m_shared_ram[1] = make_unique_clear<uint16_t[]>(0x4000);

	/* Populate RAM with data from the HEX files */
	load_hexfile(m_maincpu->space(AS_PROGRAM), memregion("code")->base());

	/* Set up the first DSP */
	membank("dsp0_bank0")->set_base(m_shared_ram[0].get());
	m_dsp0_bank1->set_base(&m_shared_ram[0][0x800]);
	load_ldafile(m_dsp0->space(AS_PROGRAM), memregion("dsp")->base());

	/* Set up the second DSP */
	membank("dsp1_bank0")->set_base(m_shared_ram[1].get());
	m_dsp1_bank1->set_base(&m_shared_ram[1][0x800]);
	load_ldafile(m_dsp1->space(AS_PROGRAM), memregion("dsp")->base());
}

void atarisy4_state::machine_reset()
{
	m_dsp0->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void airrace_state::machine_reset()
{
	atarisy4_state::machine_reset();

	m_dsp1->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, laststar, 0, atarisy4, atarisy4, atarisy4_state, init_laststar, ROT0, "Atari Games", "The Last Starfighter (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
GAME( 1985, airrace,  0, airrace,  atarisy4, airrace_state,  init_airrace,  ROT0, "Atari Games", "Air Race (prototype)",             MACHINE_IS_INCOMPLETE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW )
