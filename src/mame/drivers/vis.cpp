// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "machine/at.h"

#include "bus/isa/isa_cards.h"
#include "cpu/i86/i286.h"
#include "machine/8042kbdc.h"
#include "machine/ds6417.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "video/pc_vga.h"

#include "softlist_dev.h"
#include "speaker.h"


class vis_audio_device : public device_t,
						 public device_isa16_card_interface
{
public:
	vis_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t pcm_r(offs_t offset);
	void pcm_w(offs_t offset, uint8_t data);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void dack16_w(int line, uint16_t data) override;
	virtual void device_add_mconfig(machine_config &config) override;
private:
	required_device<dac_16bit_r2r_device> m_rdac;
	required_device<dac_16bit_r2r_device> m_ldac;
	uint16_t m_count;
	uint16_t m_curcount;
	uint16_t m_sample[2];
	uint8_t m_index[2]; // unknown indexed registers, volume?
	uint8_t m_data[2][16];
	uint8_t m_mode;
	uint8_t m_ctrl;
	unsigned int m_sample_byte;
	unsigned int m_samples;
	emu_timer *m_pcm;
};

DEFINE_DEVICE_TYPE(VIS_AUDIO, vis_audio_device, "vis_pcm", "vis_pcm")

vis_audio_device::vis_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIS_AUDIO, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_rdac(*this, "rdac"),
	m_ldac(*this, "ldac")
{
}

void vis_audio_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(7, this, false);
	m_isa->install_device(0x0220, 0x022f, read8sm_delegate(*this, FUNC(vis_audio_device::pcm_r)), write8sm_delegate(*this, FUNC(vis_audio_device::pcm_w)));
	m_isa->install_device(0x0388, 0x038b, read8sm_delegate(*subdevice<ymf262_device>("ymf262"), FUNC(ymf262_device::read)), write8sm_delegate(*subdevice<ymf262_device>("ymf262"), FUNC(ymf262_device::write)));
	m_pcm = timer_alloc();
	m_pcm->adjust(attotime::never);
}

void vis_audio_device::device_reset()
{
	m_count = 0;
	m_curcount = 0;
	m_sample_byte = 0;
	m_samples = 0;
	m_mode = 0;
	m_index[0] = m_index[1] = 0;
}

void vis_audio_device::dack16_w(int line, uint16_t data)
{
	m_sample[m_samples++] = data;
	m_curcount++;
	if((m_samples >= 2) || !(m_mode & 0x8))
		m_isa->drq7_w(CLEAR_LINE);
}

void vis_audio_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if(((m_samples < 2) && (m_mode & 8)) || !m_samples)
		return;
	switch(m_mode & 0x88)
	{
		case 0x80: // 8bit mono
		{
			uint8_t sample = m_sample[m_sample_byte >> 1] >> ((m_sample_byte & 1) * 8);
			m_ldac->write(sample << 8);
			m_rdac->write(sample << 8);
			m_sample_byte++;
			break;
		}
		case 0x00: // 8bit stereo
			m_ldac->write(m_sample[m_sample_byte >> 1] << 8);
			m_rdac->write(m_sample[m_sample_byte >> 1] & 0xff00);
			m_sample_byte += 2;
			break;
		case 0x88: // 16bit mono
			m_ldac->write(m_sample[m_sample_byte >> 1] ^ 0x8000);
			m_rdac->write(m_sample[m_sample_byte >> 1] ^ 0x8000);
			m_sample_byte += 2;
			break;
		case 0x08: // 16bit stereo
			m_ldac->write(m_sample[0] ^ 0x8000);
			m_rdac->write(m_sample[1] ^ 0x8000);
			m_sample_byte += 4;
			break;
	}

	if(m_sample_byte >= (m_mode & 8 ? 4 : 2))
	{
		m_sample_byte = 0;
		m_samples = 0;
		m_isa->drq7_w(ASSERT_LINE);
		if(m_curcount >= m_count)
		{
			m_curcount = 0;
			m_ctrl |= 4;
			if(BIT(m_ctrl, 1))
				m_isa->irq7_w(ASSERT_LINE);
		}
	}
}

void vis_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymf262_device &ymf262(YMF262(config, "ymf262", XTAL(14'318'181)));
	ymf262.add_route(0, "lspeaker", 1.00);
	ymf262.add_route(1, "rspeaker", 1.00);
	ymf262.add_route(2, "lspeaker", 1.00);
	ymf262.add_route(3, "rspeaker", 1.00);

	DAC_16BIT_R2R(config, m_ldac, 0);
	DAC_16BIT_R2R(config, m_rdac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "lspeaker", 1.0); // sanyo lc7883k
	m_rdac->add_route(ALL_OUTPUTS, "rspeaker", 1.0); // sanyo lc7883k
}

uint8_t vis_audio_device::pcm_r(offs_t offset)
{
	switch(offset)
	{
		case 0x00:
			m_isa->irq7_w(CLEAR_LINE);
			m_ctrl &= ~4;
			return m_mode;
		case 0x02:
			return m_data[0][m_index[0]];
		case 0x04:
			return m_data[1][m_index[1]];
		case 0x09:
		{
			u8 ret = m_ctrl;
			m_isa->irq7_w(CLEAR_LINE);
			m_ctrl &= ~4;
			return ret;
		}
		case 0x0c:
			return m_count & 0xff;
		case 0x0e:
			return m_count >> 8;
		case 0x0f:
			//cdrom related?
			break;
		default:
			logerror("unknown pcm read %04x\n", offset);
			break;
	}
	return 0;
}

void vis_audio_device::pcm_w(offs_t offset, uint8_t data)
{
	u8 oldmode = m_mode;
	switch(offset)
	{
		case 0x00:
			m_mode = data;
			break;
		case 0x02:
			m_data[0][m_index[0] & 0xf] = data;
			return;
		case 0x03:
			m_index[0] = data;
			return;
		case 0x04:
			m_data[1][m_index[1] & 0xf] = data;
			return;
		case 0x05:
			m_index[1] = data;
			return;
		case 0x09:
			m_ctrl = data;
			return;
		case 0x0c:
			m_count = (m_count & 0xff00) | data;
			m_curcount = 0;
			break;
		case 0x0e:
			m_count = (m_count & 0xff) | (data << 8);
			m_curcount = 0;
			break;
		case 0x0f:
			//cdrom related?
			return;
		default:
			logerror("unknown pcm write %04x %02x\n", offset, data);
			return;
	}
	if((m_mode & 0x10) && (m_mode ^ oldmode))
	{
		m_samples = 0;
		m_sample_byte = 0;
		m_isa->drq7_w(ASSERT_LINE);
		attotime rate = attotime::from_ticks(1 << ((m_mode >> 5) & 3), 44100); // TODO : Unknown clock
		m_pcm->adjust(rate, 0, rate);
	}
	else if(!(m_mode & 0x10))
		m_pcm->adjust(attotime::never);
}

class vis_vga_device : public svga_device,
					   public device_isa16_card_interface
{
public:
	vis_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t vga_r(offs_t offset);
	void vga_w(offs_t offset, uint8_t data);
	uint8_t visvgamem_r(offs_t offset);
	void visvgamem_w(offs_t offset, uint8_t data);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void recompute_params() override;
private:
	void vga_vh_yuv8(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vga_vh_yuv422(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	rgb_t yuv_to_rgb(int y, int u, int v) const;
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	int m_extcnt;
	uint8_t m_extreg;
	uint8_t m_interlace;
	uint16_t m_wina, m_winb;
	uint8_t m_shift256, m_dw, m_8bit_640;
	uint8_t m_crtc_regs[0x32];
};

DEFINE_DEVICE_TYPE(VIS_VGA, vis_vga_device, "vis_vga", "vis_vga")

vis_vga_device::vis_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	svga_device(mconfig, VIS_VGA, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this)
{
	set_screen(*this, "screen");
	set_vram_size(0x100000);
}

void vis_vga_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(FUNC(vis_vga_device::screen_update));
}

void vis_vga_device::recompute_params()
{
	int vblank_period,hblank_period;
	attoseconds_t refresh;
	uint8_t hclock_m = (!vga.gc.alpha_dis) ? (vga.sequencer.data[1]&1)?8:9 : 8;
	int pixel_clock;
	const XTAL base_xtal = XTAL(14'318'181);
	const XTAL xtal = (vga.miscellaneous_output & 0xc) ? base_xtal*2 : base_xtal*1.75;
	int divisor = 1; // divisor is 2 for 15/16 bit rgb/yuv modes and 3 for 24bit

	/* safety check */
	if(!vga.crtc.horz_disp_end || !vga.crtc.vert_disp_end || !vga.crtc.horz_total || !vga.crtc.vert_total)
		return;

	rectangle visarea(0, ((vga.crtc.horz_disp_end + 1) * ((float)(hclock_m)/divisor))-1, 0, vga.crtc.vert_disp_end);

	vblank_period = (vga.crtc.vert_total + 2);
	hblank_period = ((vga.crtc.horz_total + 5) * ((float)(hclock_m)/divisor));

	/* TODO: 10b and 11b settings aren't known */
	pixel_clock = xtal.value() / (((vga.sequencer.data[1]&8) >> 3) + 1);

	refresh  = HZ_TO_ATTOSECONDS(pixel_clock) * (hblank_period) * vblank_period;
	screen().configure((hblank_period), (vblank_period), visarea, refresh );
	//popmessage("%d %d\n",vga.crtc.horz_total * 8,vga.crtc.vert_total);
	m_vblank_timer->adjust( screen().time_until_pos((vga.crtc.vert_blank_start + vga.crtc.vert_blank_end)) );
}

rgb_t vis_vga_device::yuv_to_rgb(int y, int u, int v) const
{
	u -= 128;
	v -= 128;
	double r = y + v * 1.371;
	double g = y - u * 0.337 - v * 0.698;
	double b = y + u * 1.733;

	return rgb_t(rgb_t::clamp(r), rgb_t::clamp(g), rgb_t::clamp(b));
}

void vis_vga_device::vga_vh_yuv8(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint32_t IV = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int curr_addr = 0;
	const uint8_t decode_tbl[] = {0, 1, 2, 3, 4, 5, 6, 9, 12, 17, 22, 29, 38, 50, 66, 91, 128, 165, 190,
		206, 218, 227, 234, 239, 244, 247, 250, 251, 252, 253, 254, 255};

	for (int addr = vga.crtc.start_addr, line=0; line<(vga.crtc.vert_disp_end+1); line+=height, addr+=offset(), curr_addr+=offset())
	{
		for(int yi = 0;yi < height; yi++)
		{
			uint8_t ydelta = 0, ua = 0, va = 0;
			if((line + yi) < (vga.crtc.line_compare & 0x3ff))
				curr_addr = addr;
			if((line + yi) == (vga.crtc.line_compare & 0x3ff))
				curr_addr = 0;
			for (int pos=curr_addr, col=0, column=0; column<(vga.crtc.horz_disp_end+1); column++, col+=8, pos+=8)
			{
				if(pos + 0x08 > 0x80000)
					return;

				for(int xi=0;xi<8;xi+=4)
				{
					if(!screen().visible_area().contains(col+xi, line + yi))
						continue;
					uint8_t a = vga.memory[pos + xi], b = vga.memory[pos + xi + 1];
					uint8_t c = vga.memory[pos + xi + 2], d = vga.memory[pos + xi + 3];
					uint8_t y[4], ub, vb;
					if(col || xi)
					{
						y[0] = decode_tbl[a & 0x1f] + ydelta;
						ub = decode_tbl[((a >> 5) & 3) | ((b >> 3) & 0x1c)] + ua;
						vb = decode_tbl[((c >> 5) & 3) | ((d >> 3) & 0x1c)] + va;
					}
					else
					{
						y[0] = (a & 0x1f) << 3;
						ua = ub = ((a >> 2) & 0x18) | (b & 0xe0);
						va = vb = ((c >> 2) & 0x18) | (d & 0xe0);
					}
					y[1] = decode_tbl[b & 0x1f] + y[0];
					y[2] = decode_tbl[c & 0x1f] + y[1];
					y[3] = decode_tbl[d & 0x1f] + y[2];
					uint8_t trans = (a >> 7) | ((c >> 6) & 2);
					uint16_t u = ua;
					uint16_t v = va;
					for(int i = 0; i < 4; i++)
					{
						if(i == trans)
						{
							u = (ua + ub) >> 1;
							v = (va + vb) >> 1;
						}
						else if(i == (trans + 1))
						{
							u = ub;
							v = vb;
						}
						bitmap.pix(line + yi, col + xi + i) = IV | (uint32_t)yuv_to_rgb(y[i], u, v);
					}
					ua = ub;
					va = vb;
					ydelta = y[3];
				}
			}
		}
	}
}

void vis_vga_device::vga_vh_yuv422(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint32_t IV = 0xff000000;
	int height = vga.crtc.maximum_scan_line * (vga.crtc.scan_doubling + 1);
	int curr_addr = 0;

	for (int addr = vga.crtc.start_addr, line=0; line<(vga.crtc.vert_disp_end+1); line+=height, addr+=offset(), curr_addr+=offset())
	{
		for(int yi = 0;yi < height; yi++)
		{
			uint8_t ua = 0, va = 0;
			if((line + yi) < (vga.crtc.line_compare & 0x3ff))
				curr_addr = addr;
			if((line + yi) == (vga.crtc.line_compare & 0x3ff))
				curr_addr = 0;
			for (int pos=curr_addr, col=0, column=0; column<(vga.crtc.horz_disp_end+1); column++, col+=8,  pos+=8)
			{
				if(pos + 0x08 > 0x80000)
					return;
				for(int xi=0;xi<8;xi+=4)
				{
					if(!screen().visible_area().contains(col+xi, line + yi))
						continue;
					uint8_t y0 = vga.memory[pos + xi + 0], ub = vga.memory[pos + xi + 1];
					uint8_t y1 = vga.memory[pos + xi + 2], vb = vga.memory[pos + xi + 3];
					uint16_t u, v;
					if(col)
					{
						u = (ua + ub) >> 1;
						v = (va + vb) >> 1;
					}
					else
					{
						u = ub;
						v = vb;
					}
					ua = ub; va = vb;
					// this reads one byte per clock so it'll be one pixel for every 2 clocks
					bitmap.pix(line + yi, col + xi + 0) = IV | (uint32_t)yuv_to_rgb(y0, u, v);
					bitmap.pix(line + yi, col + xi + 1) = IV | (uint32_t)yuv_to_rgb(y0, u, v);
					bitmap.pix(line + yi, col + xi + 2) = IV | (uint32_t)yuv_to_rgb(y1, ub, vb);
					bitmap.pix(line + yi, col + xi + 3) = IV | (uint32_t)yuv_to_rgb(y1, ub, vb);
				}
			}
		}
	}
}

void vis_vga_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03b0, 0x03df, read8sm_delegate(*this, FUNC(vis_vga_device::vga_r)), write8sm_delegate(*this, FUNC(vis_vga_device::vga_w)));
	m_isa->install_memory(0x0a0000, 0x0bffff, read8sm_delegate(*this, FUNC(vis_vga_device::visvgamem_r)), write8sm_delegate(*this, FUNC(vis_vga_device::visvgamem_w)));
	svga_device::device_start();
	vga.svga_intf.seq_regcount = 0x2d;
	save_pointer(m_crtc_regs,"VIS CRTC",0x32);
}

void vis_vga_device::device_reset()
{
	m_extcnt = 0;
	m_extreg = 0;
	m_wina = 0;
	m_winb = 0;
	m_interlace = 0;
	m_shift256 = 0;
	m_dw = 0;
	m_8bit_640 = 0;
}

uint32_t vis_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(!BIT(m_extreg, 7))
		return svga_device::screen_update(screen, bitmap, cliprect);

	if((m_extreg & 0xc0) == 0x80)
	{
		svga_vh_rgb15(bitmap, cliprect);
		return 0;
	}

	switch(m_extreg & 0xc7)
	{
		case 0xc0:
		case 0xc1:
			svga_vh_rgb16(bitmap, cliprect);
			break;
		case 0xc2:
			popmessage("Border encoded 8-bit mode");
			break;
		case 0xc3:
			vga_vh_yuv422(bitmap, cliprect);
			break;
		case 0xc4:
			vga_vh_yuv8(bitmap, cliprect);
			break;
		case 0xc5:
			svga_vh_rgb24(bitmap, cliprect);
			break;
		case 0xc6:
			popmessage("DAC off");
			break;
	}
	return 0;
}

uint8_t vis_vga_device::visvgamem_r(offs_t offset)
{
	if(!(vga.sequencer.data[0x25] & 0x40))
		return mem_r(offset);
	u16 win = (vga.sequencer.data[0x1e] & 0x0f) == 3 ? m_wina : m_winb; // this doesn't seem quite right
	return mem_linear_r((offset + (win * 64)) & 0x3ffff);
}

void vis_vga_device::visvgamem_w(offs_t offset, uint8_t data)
{
	if(!(vga.sequencer.data[0x25] & 0x40))
		return mem_w(offset, data);
	return mem_linear_w((offset + (m_wina * 64)) & 0x3ffff, data);
}

uint8_t vis_vga_device::vga_r(offs_t offset)
{
	switch(offset)
	{
		case 0x16:
			if(m_extcnt == 4)
			{
				m_extcnt = 0;
				return m_extreg;
			}
			m_extcnt++;
			break;
		case 0x05:
		case 0x25:
			return m_crtc_regs[vga.crtc.index];
	}
	if(offset < 0x10)
		return port_03b0_r(offset);
	else if(offset < 0x20)
		return port_03c0_r(offset - 0x10);
	else
		return port_03d0_r(offset - 0x20);
}

void vis_vga_device::vga_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x15:
			switch(vga.sequencer.index)
			{
				case 0x01:
					m_8bit_640 = (data & 8) ? 0 : 1;
					if(m_8bit_640 && (vga.sequencer.data[0x1f] & 0x10))
						svga.rgb8_en = 1;
					else
						svga.rgb8_en = 0;
					break;
				case 0x19:
					m_wina = (m_wina & 0xff00) | data;
					break;
				case 0x18:
					m_wina = (m_wina & 0xff) | (data << 8);
					break;
				case 0x1d:
					m_winb = (m_winb & 0xff00) | data;
					break;
				case 0x1c:
					m_winb = (m_winb & 0xff) | (data << 8);
					break;
				case 0x1f:
					if(data & 0x10)
					{
						vga.gc.shift256 = 1;
						vga.crtc.dw = 1;
						vga.crtc.no_wrap = 1;
						if(m_8bit_640)
							svga.rgb8_en = 1;
						else
							svga.rgb8_en = 0;
					}
					else
					{
						vga.gc.shift256 = m_shift256;
						vga.crtc.dw = m_dw;
						vga.crtc.no_wrap = 0;
						svga.rgb8_en = 0;
					}
					break;
			}
			break;
		case 0x16:
			if(m_extcnt == 4)
			{
				if((data & 0xc7) != 0xc7)
					m_extreg = data;
				m_extcnt = 0;
				return;
			}
			break;
		case 0x1f:
			if(vga.gc.index == 0x05)
			{
				m_shift256 = data & 0x40 ? 1 : 0;
				if(vga.sequencer.data[0x1f] & 0x10)
					data |= 0x40;
			}
			break;
		case 0x05:
		case 0x25:
			assert(vga.crtc.index < std::size(m_crtc_regs));
			m_crtc_regs[vga.crtc.index] = data;
			switch(vga.crtc.index)
			{
				case 0x00:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_total = data / (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1);
					recompute_params();
					return;
				case 0x01:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_disp_end = (data / (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1)) | 1;
					recompute_params();
					return;
				case 0x02:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_blank_start = data / (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1);
					return;
				case 0x03:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_blank_end = (m_crtc_regs[0x05] & 0x80) >> 2;
					vga.crtc.horz_blank_end |= data & 0x1f;
					vga.crtc.horz_blank_end /= (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1);
					vga.crtc.disp_enable_skew = (data & 0x60) >> 5;
					vga.crtc.evra = (data & 0x80) >> 7;
					return;
				case 0x04:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_retrace_start = data / (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1);
					return;
				case 0x05:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.horz_blank_end = m_crtc_regs[0x05] & 0x1f;
					vga.crtc.horz_blank_end |= ((data & 0x80) >> 2);
					vga.crtc.horz_blank_end /= (m_interlace && !(vga.sequencer.data[0x25] & 0x20) ? 2 : 1);
					vga.crtc.horz_retrace_skew = ((data & 0x60) >> 5);
					vga.crtc.horz_retrace_end = data & 0x1f;
					return;
				case 0x06:
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.vert_total = ((m_crtc_regs[0x07] & 0x20) << (9-6)) | ((m_crtc_regs[0x07] & 0x01) << (8-0));
					vga.crtc.vert_total |= data;
					vga.crtc.vert_total *= (m_interlace ? 2 : 1);
					recompute_params();
					return;
				case 0x07: // Overflow Register
					vga.crtc.line_compare       &= ~0x100;
					vga.crtc.line_compare       |= ((data & 0x10) << (8-4));
					if(vga.crtc.protect_enable)
						return;
					vga.crtc.vert_total         = m_crtc_regs[0x05];
					vga.crtc.vert_retrace_start = m_crtc_regs[0x10];
					vga.crtc.vert_disp_end      = m_crtc_regs[0x12];
					vga.crtc.vert_blank_start   = m_crtc_regs[0x15] | ((m_crtc_regs[0x09] & 0x20) << (9-5));
					vga.crtc.vert_retrace_start |= ((data & 0x80) << (9-7));
					vga.crtc.vert_disp_end      |= ((data & 0x40) << (9-6));
					vga.crtc.vert_total         |= ((data & 0x20) << (9-5));
					vga.crtc.vert_blank_start   |= ((data & 0x08) << (8-3));
					vga.crtc.vert_retrace_start |= ((data & 0x04) << (8-2));
					vga.crtc.vert_disp_end      |= ((data & 0x02) << (8-1));
					vga.crtc.vert_total         |= ((data & 0x01) << (8-0));
					vga.crtc.vert_total *= (m_interlace ? 2 : 1);
					vga.crtc.vert_blank_start *= (m_interlace ? 2 : 1);
					vga.crtc.vert_retrace_start *= (m_interlace ? 2 : 1);
					vga.crtc.vert_disp_end *= (m_interlace ? 2 : 1);
					recompute_params();
					return;
				case 0x09: // Maximum Scan Line Register
					vga.crtc.line_compare      &= ~0x200;
					vga.crtc.vert_blank_start  = m_crtc_regs[0x15] | ((m_crtc_regs[0x07] & 0x02) << (8-3));
					vga.crtc.scan_doubling      = ((data & 0x80) >> 7);
					vga.crtc.line_compare      |= ((data & 0x40) << (9-6));
					vga.crtc.vert_blank_start  |= ((data & 0x20) << (9-5));
					vga.crtc.maximum_scan_line  = (data & 0x1f) + 1;
					vga.crtc.vert_blank_start *= (m_interlace ? 2 : 1);
					return;
				case 0x10:
					vga.crtc.vert_retrace_start = ((m_crtc_regs[0x07] & 0x80) << (9-7)) | ((m_crtc_regs[0x07] & 0x40) << (8-2));
					vga.crtc.vert_retrace_start |= data;
					vga.crtc.vert_retrace_start *= (m_interlace ? 2 : 1);
					return;
				case 0x12:
					vga.crtc.vert_disp_end = ((m_crtc_regs[0x07] & 0x40) << (9-6)) | ((m_crtc_regs[0x07] & 0x02) << (8-1));
					vga.crtc.vert_disp_end |= data;
					vga.crtc.vert_disp_end *= (m_interlace ? 2 : 1);
					recompute_params();
					return;
				case 0x15:
					vga.crtc.vert_blank_start = ((m_crtc_regs[0x09] & 0x20) << (9-5)) | ((m_crtc_regs[0x07] & 0x02) << (8-3));
					vga.crtc.vert_blank_start |= data;
					vga.crtc.vert_blank_start *= (m_interlace ? 2 : 1);
					return;
				case 0x16:
					vga.crtc.vert_blank_end = (data & 0x7f) * (m_interlace ? 2 : 1);
					return;
				case 0x14:
					m_dw = data & 0x40 ? 1 : 0;
					if(vga.sequencer.data[0x1f] & 0x10)
						data |= 0x40;
					break;
				case 0x30:
					if(data && !m_interlace)
					{
						if(!(vga.sequencer.data[0x25] & 0x20))
						{
							vga.crtc.horz_total /= 2;
							vga.crtc.horz_disp_end = (vga.crtc.horz_disp_end / 2) | 1;
							vga.crtc.horz_blank_end /= 2;
							vga.crtc.horz_retrace_start /= 2;
							vga.crtc.horz_retrace_end /= 2;
						}
						vga.crtc.vert_total *= 2;
						vga.crtc.vert_retrace_start *= 2;
						vga.crtc.vert_disp_end *= 2;
						vga.crtc.vert_blank_start *= 2;
						vga.crtc.vert_blank_end *= 2;
						recompute_params();
					}
					else if(!data && m_interlace)
					{
						if(!(vga.sequencer.data[0x25] & 0x20))
						{
							vga.crtc.horz_total *= 2;
							vga.crtc.horz_disp_end  = (vga.crtc.horz_disp_end * 2) | 1;
							vga.crtc.horz_blank_end *= 2;
							vga.crtc.horz_retrace_start *= 2;
							vga.crtc.horz_retrace_end *= 2;
						}
						vga.crtc.vert_total /= 2;
						vga.crtc.vert_retrace_start /= 2;
						vga.crtc.vert_disp_end /= 2;
						vga.crtc.vert_blank_start /= 2;
						vga.crtc.vert_blank_end /= 2;
						recompute_params();
					}
					m_interlace = data;
					return;
			}
			break;
	}
	if(offset < 0x10)
		port_03b0_w(offset, data);
	else if(offset < 0x20)
		port_03c0_w(offset - 0x10, data);
	else
		port_03d0_w(offset - 0x20, data);
}

class vis_state : public driver_device
{
public:
	vis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic1(*this, "mb:pic8259_master"),
		m_pic2(*this, "mb:pic8259_slave"),
		m_card(*this, "card"),
		m_pad(*this, "PAD")
		{ }

	void vis(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(update);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<ds6417_device> m_card;
	required_ioport m_pad;

	uint8_t sysctl_r();
	void sysctl_w(uint8_t data);
	uint8_t unk_r(offs_t offset);
	void unk_w(offs_t offset, uint8_t data);
	uint8_t unk2_r();
	uint8_t memcard_r(offs_t offset);
	void memcard_w(offs_t offset, uint8_t data);
	uint16_t pad_r(offs_t offset);
	void pad_w(offs_t offset, uint16_t data);
	uint8_t unk1_r(offs_t offset);
	void unk1_w(offs_t offset, uint8_t data);
	void io_map(address_map &map);
	void main_map(address_map &map);

	void machine_reset() override;

	uint8_t m_sysctl;
	uint8_t m_unkidx;
	uint8_t m_unk[16];
	uint8_t m_unk1[4];
	uint8_t m_cardreg, m_cardval, m_cardcnt;
	uint16_t m_padctl, m_padstat;
	bool m_padsel;
};

void vis_state::machine_reset()
{
	m_sysctl = 0;
	m_padctl = 0;
	m_padsel = false;
}

INPUT_CHANGED_MEMBER(vis_state::update)
{
	m_pic1->ir3_w(ASSERT_LINE);
	m_padstat = 0x80;
	m_padsel = false;
}

//chipset registers?
uint8_t vis_state::unk_r(offs_t offset)
{
	if(offset)
		return m_unk[m_unkidx];
	return 0;
}

void vis_state::unk_w(offs_t offset, uint8_t data)
{
	if(offset)
		m_unk[m_unkidx] = data;
	else
		m_unkidx = data & 0xf;
}

uint8_t vis_state::unk2_r()
{
	return 0x40;
}

uint8_t vis_state::memcard_r(offs_t offset)
{
	if(offset)
	{
		if(m_cardreg & 0x10)
		{
			if(m_cardcnt == 8)
				return 0;
			if(m_cardreg & 8)
			{
				m_card->clock_w(1);
				m_card->clock_w(0);
				m_cardval = (m_cardval >> 1) | (m_card->data_r() ? 0x80 : 0);
			}
			else
			{
				m_card->clock_w(0);
				m_card->data_w(BIT(m_cardval, 0));
				m_card->clock_w(1);
				m_cardval >>= 1;
			}
			m_cardcnt++;
			return 0x80;
		}
	}
	else
	{
		m_cardcnt = 0;
		return m_cardval;
	}
	return 0;
}

void vis_state::memcard_w(offs_t offset, uint8_t data)
{
	if(offset)
	{
		if(!(data & 0x10) && !(m_cardreg & 0x10))
		{
			m_card->data_w(BIT(data, 1));
			m_card->clock_w(BIT(data, 0));
			m_card->reset_w(!BIT(data, 2));
		}
		m_cardreg = data;
		m_cardcnt = data & 8 ? 0 : 8;
	}
	else
	{
		m_cardcnt = 0;
		m_cardval = data;
	}
}

uint16_t vis_state::pad_r(offs_t offset)
{
	uint16_t ret = 0;
	switch(offset)
	{
		case 0:
			if(!m_padsel)
			{
				ret = m_pad->read();
				m_padstat = 0;
				m_padsel = true;
			}
			else
			{
				ret = 0x400; // this is probably for the second controller
				m_padsel = false;
			}
			m_pic1->ir3_w(CLEAR_LINE);
			break;
		case 1:
			ret = m_padstat;
	}
	return ret;
}

void vis_state::pad_w(offs_t offset, uint16_t data)
{
	switch(offset)
	{
		case 1:
			m_padctl = data;
			break;
	}
}

uint8_t vis_state::unk1_r(offs_t offset)
{
	if(offset == 2)
		return 0xde;
	return 0;
}

void vis_state::unk1_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 1:
			if(data == 0x10)
				m_pic2->ir1_w(CLEAR_LINE);
			else if(data == 0x16)
				m_pic2->ir1_w(ASSERT_LINE);
	}
	m_unk1[offset] = data;
}

uint8_t vis_state::sysctl_r()
{
	return m_sysctl;
}

void vis_state::sysctl_w(uint8_t data)
{
	if(BIT(data, 0) && !BIT(m_sysctl, 0))
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	//m_maincpu->set_input_line(INPUT_LINE_A20, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
	m_sysctl = data;
}

void vis_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).ram();
	map(0x0d8000, 0x0fffff).rom().region("bios", 0xd8000);
	map(0x100000, 0x15ffff).ram();
	map(0x300000, 0x3fffff).rom().region("bios", 0);
	map(0xff0000, 0xffffff).rom().region("bios", 0xf0000);
}

void vis_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x001f).rw("mb:dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x003f).rw(m_pic1, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0026, 0x0027).rw(FUNC(vis_state::unk_r), FUNC(vis_state::unk_w));
	map(0x0040, 0x005f).rw("mb:pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0065).rw("kbdc", FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x0061, 0x0061).rw("mb", FUNC(at_mb_device::portb_r), FUNC(at_mb_device::portb_w));
	map(0x006a, 0x006a).r(FUNC(vis_state::unk2_r));
	map(0x0080, 0x009f).rw("mb", FUNC(at_mb_device::page8_r), FUNC(at_mb_device::page8_w));
	map(0x0092, 0x0092).rw(FUNC(vis_state::sysctl_r), FUNC(vis_state::sysctl_w));
	map(0x00a0, 0x00bf).rw(m_pic2, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw("mb:dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	map(0x00e0, 0x00e1).noprw();
	map(0x023c, 0x023f).rw(FUNC(vis_state::unk1_r), FUNC(vis_state::unk1_w));
	map(0x0268, 0x026f).rw(FUNC(vis_state::pad_r), FUNC(vis_state::pad_w));
	map(0x0318, 0x031a).rw(FUNC(vis_state::memcard_r), FUNC(vis_state::memcard_w)).umask16(0x00ff);
}

static void vis_cards(device_slot_interface &device)
{
	device.option_add("visaudio", VIS_AUDIO);
	device.option_add("visvga", VIS_VGA);
}

// TODO: other buttons
static INPUT_PORTS_START(vis)
	PORT_START("PAD")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("1") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("2") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("4") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("3") PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, vis_state, update, 0)
INPUT_PORTS_END

void vis_state::vis(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, "maincpu", XTAL(12'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &vis_state::main_map);
	maincpu.set_addrmap(AS_IO, &vis_state::io_map);
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, "mb");
	// the vis doesn't have a real keyboard controller
	config.device_remove("mb:keybc");

	kbdc8042_device &kbdc(KBDC8042(config, "kbdc"));
	kbdc.set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	kbdc.system_reset_callback().set_inputline("maincpu", INPUT_LINE_RESET);
	kbdc.gate_a20_callback().set_inputline("maincpu", INPUT_LINE_A20);
	kbdc.input_buffer_full_callback().set("mb:pic8259_master", FUNC(pic8259_device::ir1_w));

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "mcd",      0, "mb:isabus", pc_isa16_cards, "mcd",      true);
	ISA16_SLOT(config, "visaudio", 0, "mb:isabus", vis_cards,      "visaudio", true);
	ISA16_SLOT(config, "visvga",   0, "mb:isabus", vis_cards,      "visvga",   true);

	SOFTWARE_LIST(config, "cd_list").set_original("vis");

	DS6417(config, m_card, 0);
}

ROM_START(vis)
	ROM_REGION16_LE(0x100000,"bios", 0)
	ROM_LOAD( "p513bk0b.bin", 0x00000, 0x80000, CRC(364e3f74) SHA1(04260ef1e65e482c9c49d25ace40e22487d6aab9))
	ROM_LOAD( "p513bk1b.bin", 0x80000, 0x80000, CRC(e18239c4) SHA1(a0262109e10a07a11eca43371be9978fff060bc5))
ROM_END

COMP( 1992, vis, 0, 0, vis, vis, vis_state, empty_init, "Tandy/Memorex", "Video Information System MD-2500", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
