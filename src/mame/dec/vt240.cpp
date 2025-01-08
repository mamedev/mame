// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

        DEC VT240

****************************************************************************/

#include "emu.h"
#include "lk201.h"

#include "ms7004.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "cpu/t11/t11.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/bankdev.h"
#include "machine/x2212.h"
#include "video/upd7220.h"
#include "emupal.h"
#include "screen.h"


namespace {

class vt240_state : public driver_device
{
public:
	vt240_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i8085(*this, "charcpu")
		, m_i8251(*this, "i8251")
		, m_duart(*this, "duart")
		, m_host(*this, "host")
		, m_hgdc(*this, "upd7220")
		, m_bank(*this, "bank")
		, m_nvram(*this, "x2212")
		, m_palette(*this, "palette")
		, m_rom(*this, "maincpu")
		, m_video_ram(*this, "vram")
		, m_monitor(*this, "monitor")
		, m_lk201(*this, "lk201")
	{
	}

	void mc7105(machine_config &config);
	void vt240(machine_config &config);

private:
	required_device<t11_device> m_maincpu;
	required_device<i8085a_cpu_device> m_i8085;
	required_device<i8251_device> m_i8251;
	required_device<scn2681_device> m_duart;
	required_device<rs232_port_device> m_host;
	required_device<upd7220_device> m_hgdc;
	required_device<address_map_bank_device> m_bank;
	required_device<x2212_device> m_nvram;
	required_device<palette_device> m_palette;
	required_region_ptr<uint16_t> m_rom;
	required_shared_ptr<uint16_t> m_video_ram;
	required_ioport m_monitor;
	optional_device<lk201_device> m_lk201;

	void write_keyboard_clock(int state);
	void i8085_rdy_w(int state);
	void lben_w(int state);
	void tx_w(int state);
	void t11_reset_w(int state);
	int i8085_sid_r();
	uint8_t i8085_comm_r(offs_t offset);
	void i8085_comm_w(offs_t offset, uint8_t data);
	uint8_t t11_comm_r();
	void t11_comm_w(uint8_t data);
	uint8_t duart_r(offs_t offset);
	void duart_w(offs_t offset, uint8_t data);
	void duartout_w(uint8_t data);
	uint8_t mem_map_cs_r(offs_t offset);
	void mem_map_cs_w(offs_t offset, uint8_t data);
	uint8_t ctrl_r();
	void mem_map_sel_w(uint8_t data);
	uint8_t char_buf_r();
	void char_buf_w(uint8_t data);
	void patmult_w(uint8_t data);
	void vpat_w(uint8_t data);
	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data);
	uint8_t vom_r(offs_t offset);
	void vom_w(offs_t offset, uint8_t data);
	uint8_t nvr_store_r();
	void nvr_store_w(uint8_t data);
	void mask_w(uint8_t data);
	void reg0_w(uint8_t data);
	void reg1_w(uint8_t data);
	void lu_w(uint8_t data);
	void hbscrl_w(uint8_t data);
	void lbscrl_w(uint8_t data);
	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void init_vt240();
	virtual void machine_reset() override ATTR_COLD;
	UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_draw);
	void irq_encoder(int irq, int state);
	void irq7_w(int state);
	void irq9_w(int state);
	void irq13_w(int state);

	uint8_t m_i8085_out, m_t11_out, m_i8085_rdy, m_t11;
	uint8_t m_mem_map[16];
	uint8_t m_mem_map_sel;
	uint8_t m_char_buf[16];
	uint8_t m_char_idx, m_mask, m_reg0, m_reg1, m_lu;
	uint8_t m_vom[16];
	uint8_t m_vpat, m_patmult, m_patcnt, m_patidx;
	uint16_t m_irqs;
	bool m_lb;
	uint16_t m_scrl;

	void bank_map(address_map &map) ATTR_COLD;
	void upd7220_map(address_map &map) ATTR_COLD;
	void vt240_char_io(address_map &map) ATTR_COLD;
	void vt240_char_mem(address_map &map) ATTR_COLD;
	void vt240_mem(address_map &map) ATTR_COLD;
};

void vt240_state::irq_encoder(int irq, int state)
{
	if(state == ASSERT_LINE)
		m_irqs |= (1 << irq);
	else
		m_irqs &= ~(1 << irq);

	int i;
	for(i = 15; i > 0; i--)
	{
		if(m_irqs & (1 << i))
			break;
	}
	m_maincpu->set_input_line(t11_device::CP3_LINE, (i & 8) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP2_LINE, (i & 4) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, (i & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP0_LINE, (i & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void vt240_state::irq7_w(int state)
{
	irq_encoder(7, state);
}

void vt240_state::irq9_w(int state)
{
	irq_encoder(9, state);
}

void vt240_state::irq13_w(int state)
{
	irq_encoder(13, state);
}

void vt240_state::write_keyboard_clock(int state)
{
	m_i8251->write_txc(state);
	m_i8251->write_rxc(state);
}

void vt240_state::lben_w(int state)
{
	m_lb = state ? false : true;
}

void vt240_state::t11_reset_w(int state)
{
	if(state == ASSERT_LINE)
	{
		m_duart->reset();
		m_i8251->reset();
		m_nvram->recall(ASSERT_LINE);
		m_nvram->recall(CLEAR_LINE);
	}
}

void vt240_state::tx_w(int state)
{
	if(m_lb)
		m_i8251->write_rxd(state);
	else
		m_lk201->rx_w(state);
}

void vt240_state::i8085_rdy_w(int state)
{
	irq_encoder(3, state ? CLEAR_LINE : ASSERT_LINE);
	m_i8085_rdy = state;
}

int vt240_state::i8085_sid_r()
{
	return m_t11 ? CLEAR_LINE : ASSERT_LINE;
}

UPD7220_DISPLAY_PIXELS_MEMBER( vt240_state::hgdc_draw )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	if(!BIT(m_reg0, 7))
	{
		vram_w(address, 0);
		vram_w((0x10000 + address), 0);
	}

	int const gfx1 = m_video_ram[(address & 0x3fff)];
	int const gfx2 = m_video_ram[((address & 0x3fff) + 0x4000)];

	bool const color = m_monitor->read() ? true : false;
	for(int xi=0;xi<16;xi++)
	{
		uint8_t const vom = BIT(gfx1, xi) | (BIT(gfx2, xi) << 1) | ((m_reg0 & 3) << 2);
		bitmap.pix(y, x + xi) = palette[color ? (vom + 16) : vom];
	}
}

uint8_t vt240_state::t11_comm_r()
{
	m_t11 = 1;
	m_i8085->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
	return m_t11_out;
}

void vt240_state::t11_comm_w(uint8_t data)
{
	m_i8085_out = data;
}

uint8_t vt240_state::i8085_comm_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return m_i8085_out;
		case 2:
			m_i8085->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
			m_i8085->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			m_t11 = 1;
			break;
	}
	return 0xff;
}

void vt240_state::i8085_comm_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 1:
			m_t11_out = data;
			m_t11 = 0;
			m_i8085->set_input_line(I8085_RST65_LINE, ASSERT_LINE);
			break;
		case 2:
			m_i8085->set_input_line(I8085_RST65_LINE, CLEAR_LINE);
			m_i8085->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			m_t11 = 1;
			break;
	}
}

uint8_t vt240_state::duart_r(offs_t offset)
{
	if(!(offset & 1))
		return m_duart->read(offset >> 1);
	return 0;
}

void vt240_state::duart_w(offs_t offset, uint8_t data)
{
	if(offset & 1)
		m_duart->write(offset >> 1, data);
}

void vt240_state::duartout_w(uint8_t data)
{
	m_host->write_rts(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	m_host->write_dtr(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	irq_encoder(15, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);
	irq_encoder(14, BIT(data, 5) ? CLEAR_LINE : ASSERT_LINE);
	irq_encoder(11, BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);
	irq_encoder(10, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

uint8_t vt240_state::mem_map_cs_r(offs_t offset)
{
	return ~m_mem_map[offset];
}

void vt240_state::mem_map_cs_w(offs_t offset, uint8_t data)
{
	m_mem_map[offset] = ~data;
}

uint8_t vt240_state::ctrl_r()
{
	return m_mem_map_sel | ((m_lb ? 0 : 1) << 3) | (m_i8085_rdy << 6) | (m_t11 << 7) | (1<<5); // no modem
}

void vt240_state::mem_map_sel_w(uint8_t data)
{
	m_mem_map_sel = data & 1;
}

uint16_t vt240_state::mem_r(offs_t offset, uint16_t mem_mask)
{
	if(m_mem_map_sel)
	{
		m_bank->set_bank(m_mem_map[(offset >> 11) & 0xf]);
		return m_bank->read16(offset & 0x7ff, mem_mask);
	}
	else
		return m_rom[offset];
}

void vt240_state::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(m_mem_map_sel)
	{
		m_bank->set_bank(m_mem_map[(offset >> 11) & 0xf]);
		m_bank->write16(offset & 0x7ff, data, mem_mask);
	}
}

uint8_t vt240_state::char_buf_r()
{
	m_char_idx = 0;
	return 0xff;
}

void vt240_state::char_buf_w(uint8_t data)
{
	m_char_buf[m_char_idx++] = bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
	m_char_idx &= 0xf;
}

void vt240_state::patmult_w(uint8_t data)
{
	m_patmult = data & 0xf;
}

void vt240_state::vpat_w(uint8_t data)
{
	m_vpat = data;
	m_patcnt = m_patmult;
	m_patidx = 7;
}

uint8_t vt240_state::vom_r(offs_t offset)
{
	if(!BIT(m_reg0, 2))
		return m_vom[offset];
	// this hack passes a self test, is not a useful value normally
	// when vom read mode is disabled, the read latch is set to whatever map is
	// enabled and color is currently drawn, the self test fills the screen with 0xff
	// and reads it
	return m_vom[((m_reg0 & 3) << 2) + 3];
}

void vt240_state::vom_w(offs_t offset, uint8_t data)
{
	if(!BIT(m_reg0, 2))
	{
		m_vom[offset] = data;
		data = ~bitswap<8>(data, 1, 0, 3, 2, 5, 4, 7, 6);
		m_palette->set_pen_color(offset, pal2bit(data >> 6), pal2bit(data >> 6), pal2bit(data >> 6));
		m_palette->set_pen_color((offset + 16), pal2bit(data >> 0), pal2bit(data >> 2), pal2bit(data >> 4));
	}
}

uint16_t vt240_state::vram_r(offs_t offset)
{
	if(!BIT(m_reg0, 3) || machine().side_effects_disabled())
	{
		offset = ((offset & 0x18000) >> 1) | (offset & 0x3fff);
		return m_video_ram[offset & 0x7fff];
	}
	return 0;
}

void vt240_state::vram_w(offs_t offset, uint16_t data)
{
	uint8_t *video_ram = (uint8_t *)(&m_video_ram[0]);
	offset <<= 1;
	offset = ((offset & 0x30000) >> 1) | (offset & 0x7fff);
	if(!BIT(m_reg0, 3))
		offset |= BIT(offset, 16);
	else
	{
		if(data & 0xff00)
		{
			data >>= 8;
			offset += 1;
		}
		else
			data &= 0xff;
	}
	offset &= 0xffff;
	uint8_t chr = data;

	if(BIT(m_reg0, 4))
	{
		if(BIT(m_reg0, 6))
		{
			chr = bitswap<8>(m_vpat, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx);
			if(m_patcnt-- == 0)
			{
				m_patcnt = m_patmult;
				if(m_patidx-- == 0)
					m_patidx = 7;
			}
		}
		else
		{
			chr = m_char_buf[m_char_idx++];
			m_char_idx &= 0xf;
		}
		int ps = ~m_lu & 3;
		for(int i = 0; i <= (ps >> 1); i++)
		{
			if(ps == 0)
				i++;
			uint8_t mem = video_ram[(offset & 0x7fff) + (0x8000 * i)];
			uint8_t out = 0, ifore = BIT(m_lu, (i ? 5 : 4)), iback = BIT(m_lu, (i ? 3 : 2));
			for(int j = 0; j < 8; j++)
				out |= BIT(chr, j) ? (ifore << j) : (iback << j);
			switch(m_lu >> 6)
			{
				case 0:
					break;
				case 1:
					out |= mem;
					break;
				case 2:
					logerror("invalid logic unit mode 2\n");
					break;
				case 3:
					out ^= ~mem;
					break;
			}
			if(!BIT(m_reg0, 3))
				out = (out & ~m_mask) | (mem & m_mask);
			else
				out = (out & data) | (mem & ~data);
			if(BIT(m_reg1, 3))
			{
				uint8_t out2 = out;
				if(BIT(m_reg1, 2))
				{
					out = video_ram[((offset & 0x7ffe) | 0) + (0x8000 * i)];
					out2 = video_ram[((offset & 0x7ffe) | 1) + (0x8000 * i)];
				}
				video_ram[((m_scrl << 1) | 0) + (0x8000 * i)] = out;
				video_ram[((m_scrl << 1) | 1) + (0x8000 * i)] = out2;
			}
			else
				video_ram[(offset & 0x7fff) + (0x8000 * i)] = out;
		}
		if(BIT(m_reg1, 3))
		{
			m_scrl += BIT(m_reg1, 1) ? -1 : 1;
			m_scrl &= 0x3fff;
		}
		return;
	}
	if(!BIT(m_reg0, 3))
		data = (chr & ~m_mask) | (video_ram[offset] & m_mask);
	else
		data = (chr & data) | (video_ram[offset] & ~data);
	if(BIT(m_reg1, 3))
	{
		uint8_t data2 = data;
		if(BIT(m_reg1, 2))
		{
			data = video_ram[(offset & ~1) | 0];
			data2 = video_ram[(offset & ~1) | 1];
		}
		video_ram[(offset & 0x8000) | (m_scrl << 1) | 0] = data;
		video_ram[(offset & 0x8000) | (m_scrl << 1) | 1] = data2;
		m_scrl += BIT(m_reg1, 1) ? -1 : 1;
		m_scrl &= 0x3fff;
	}
	else
		video_ram[offset] = data;
}

void vt240_state::mask_w(uint8_t data)
{
	m_mask = bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
}

uint8_t vt240_state::nvr_store_r()
{
	m_nvram->store(ASSERT_LINE);
	m_nvram->store(CLEAR_LINE);
	return 0;
}

void vt240_state::nvr_store_w(uint8_t data)
{
	m_nvram->store(ASSERT_LINE);
	m_nvram->store(CLEAR_LINE);
}

void vt240_state::reg0_w(uint8_t data)
{
	m_reg0 = data;
}

void vt240_state::reg1_w(uint8_t data)
{
	m_reg1 = data;
}

void vt240_state::lu_w(uint8_t data)
{
	m_lu = data;
}

void vt240_state::lbscrl_w(uint8_t data)
{
	m_scrl = (m_scrl & 0xff00) | data;
}

void vt240_state::hbscrl_w(uint8_t data)
{
	m_scrl = (m_scrl & 0xff) | ((data & 0x3f) << 8);
}

void vt240_state::bank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x80000, 0x87fff).ram();
}

// PDF page 78 (4-25)
void vt240_state::vt240_mem(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0167777).rw(FUNC(vt240_state::mem_r), FUNC(vt240_state::mem_w));
	map(0170000, 0170037).rw(FUNC(vt240_state::mem_map_cs_r), FUNC(vt240_state::mem_map_cs_w)).umask16(0x00ff);
	map(0170040, 0170040).w(FUNC(vt240_state::mem_map_sel_w));
	map(0170100, 0170100).r(FUNC(vt240_state::ctrl_r));
	map(0170140, 0170140).rw(FUNC(vt240_state::nvr_store_r), FUNC(vt240_state::nvr_store_w));
	map(0171000, 0171003).rw(m_i8251, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w)).umask16(0x00ff);
	map(0171004, 0171007).rw(m_i8251, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w)).umask16(0x00ff);
	map(0172000, 0172077).rw(FUNC(vt240_state::duart_r), FUNC(vt240_state::duart_w)).umask16(0x00ff);
	map(0173000, 0173003).r(m_hgdc, FUNC(upd7220_device::read)).umask16(0x00ff);
	map(0173040, 0173077).r(FUNC(vt240_state::vom_r)).umask16(0x00ff);
	map(0173140, 0173140).r(FUNC(vt240_state::char_buf_r));
	map(0174000, 0174003).w(m_hgdc, FUNC(upd7220_device::write)).umask16(0x00ff);
	map(0174040, 0174077).w(FUNC(vt240_state::vom_w)).umask16(0x00ff);
	map(0174140, 0174140).w(FUNC(vt240_state::char_buf_w));
	map(0174400, 0174400).w(FUNC(vt240_state::patmult_w));
	map(0174440, 0174440).w(FUNC(vt240_state::mask_w));
	map(0174500, 0174500).w(FUNC(vt240_state::vpat_w));
	map(0174540, 0174540).w(FUNC(vt240_state::lu_w));
	map(0174600, 0174600).w(FUNC(vt240_state::reg0_w));
	map(0174640, 0174640).w(FUNC(vt240_state::reg1_w));
	map(0174700, 0174700).w(FUNC(vt240_state::hbscrl_w));
	map(0174740, 0174740).w(FUNC(vt240_state::lbscrl_w));
	map(0175000, 0175005).rw(FUNC(vt240_state::i8085_comm_r), FUNC(vt240_state::i8085_comm_w)).umask16(0x00ff);
	map(0176000, 0176777).rw(m_nvram, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0x00ff);
	// 017700x System comm logic
}

// PDF page 134 (6-9)
void vt240_state::vt240_char_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("charcpu", 0);
	map(0x4000, 0x5fff).rom().region("charcpu", 0x8000);
	map(0x8000, 0x87ff).ram();
}

void vt240_state::vt240_char_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0x10, 0x1f).rw(FUNC(vt240_state::vom_r), FUNC(vt240_state::vom_w));
	map(0x20, 0x20).rw(FUNC(vt240_state::t11_comm_r), FUNC(vt240_state::t11_comm_w));
	map(0x30, 0x30).rw(FUNC(vt240_state::char_buf_r), FUNC(vt240_state::char_buf_w));
	map(0x80, 0x80).w(FUNC(vt240_state::patmult_w));
	map(0x90, 0x90).w(FUNC(vt240_state::mask_w));
	map(0xa0, 0xa0).w(FUNC(vt240_state::vpat_w));
	map(0xb0, 0xb0).w(FUNC(vt240_state::lu_w));
	map(0xc0, 0xc0).w(FUNC(vt240_state::reg0_w));
	map(0xd0, 0xd0).w(FUNC(vt240_state::reg1_w));
	map(0xe0, 0xe0).w(FUNC(vt240_state::hbscrl_w));
	map(0xf0, 0xf0).w(FUNC(vt240_state::lbscrl_w));
}

void vt240_state::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(vt240_state::vram_r), FUNC(vt240_state::vram_w)).share("vram");
}


void vt240_state::machine_reset()
{
	m_i8251->write_cts(0);
	m_nvram->recall(ASSERT_LINE);
	m_nvram->recall(CLEAR_LINE);
	m_mem_map_sel = 0;
	m_t11 = 1;
	m_i8085_rdy = 1;
	m_char_idx = 0;
	m_patcnt = 0;
	m_patidx = 0;
	m_reg0 = 0x80;
	m_irqs = 0;
}

static const gfx_layout vt240_chars_8x10 =
{
	8,10,
	0x240,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*10
};

static GFXDECODE_START( gfx_vt240 )
	GFXDECODE_ENTRY( "charcpu", 0x338*10-3, vt240_chars_8x10, 0, 8 )
GFXDECODE_END

static INPUT_PORTS_START( vt240 )
	PORT_START("monitor")
	PORT_CONFNAME(0x01, 0x01, "Monitor Type")
	PORT_CONFSETTING(0x00, "Monochrome")
	PORT_CONFSETTING(0x01, "Color")
INPUT_PORTS_END

void vt240_state::vt240(machine_config &config)
{
	T11(config, m_maincpu, XTAL(7'372'800)); // confirm
	m_maincpu->set_addrmap(AS_PROGRAM, &vt240_state::vt240_mem);
	m_maincpu->set_initial_mode(5 << 13);
	m_maincpu->out_reset().set(FUNC(vt240_state::t11_reset_w));

	I8085A(config, m_i8085, XTAL(16'097'280) / 2);
	m_i8085->set_addrmap(AS_PROGRAM, &vt240_state::vt240_char_mem);
	m_i8085->set_addrmap(AS_IO, &vt240_state::vt240_char_io);
	m_i8085->out_sod_func().set(FUNC(vt240_state::i8085_rdy_w));
	m_i8085->in_sid_func().set(FUNC(vt240_state::i8085_sid_r));

	ADDRESS_MAP_BANK(config, "bank").set_map(&vt240_state::bank_map).set_options(ENDIANNESS_LITTLE, 16, 20, 0x1000);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(16'097'280), 1024, 0, 800, 629, 0, 480);
	screen.set_screen_update("upd7220", FUNC(upd7220_device::screen_update));

	PALETTE(config, m_palette).set_entries(32);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_vt240);

	UPD7220(config, m_hgdc, XTAL(16'097'280) / 16); // actually /8?
	m_hgdc->set_addrmap(0, &vt240_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(vt240_state::hgdc_draw));
	m_hgdc->vsync_wr_callback().set_inputline(m_i8085, I8085_RST75_LINE);
	m_hgdc->blank_wr_callback().set_inputline(m_i8085, I8085_RST55_LINE);
	m_hgdc->set_screen("screen");

	SCN2681(config, m_duart, XTAL(7'372'800) / 2);
	m_duart->irq_cb().set(FUNC(vt240_state::irq13_w));
	m_duart->a_tx_cb().set(m_host, FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set("printer", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set(FUNC(vt240_state::duartout_w));

	I8251(config, m_i8251, 0);
	m_i8251->txd_handler().set(FUNC(vt240_state::tx_w));
	m_i8251->dtr_handler().set(FUNC(vt240_state::lben_w));
	m_i8251->rxrdy_handler().set(FUNC(vt240_state::irq9_w));
	m_i8251->txrdy_handler().set(FUNC(vt240_state::irq7_w));

	LK201(config, m_lk201, 0);
	m_lk201->tx_handler().set(m_i8251, FUNC(i8251_device::write_rxd));

	CLOCK(config, "keyboard_clock", 4800 * 64).signal_handler().set(FUNC(vt240_state::write_keyboard_clock)); // 8251 is set to /64 on the clock input

	RS232_PORT(config, m_host, default_rs232_devices, "null_modem");
	m_host->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_host->dsr_handler().set(m_duart, FUNC(scn2681_device::ip5_w));
	m_host->cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	printer.dsr_handler().set(m_duart, FUNC(scn2681_device::ip1_w));

	X2212(config, "x2212");
}

void vt240_state::mc7105(machine_config &config)
{
	vt240(config);

	config.device_remove("lk201");

	ms7004_device &ms7004(MS7004(config, "ms7004", 0));
	ms7004.tx_handler().set(m_i8251, FUNC(i8251_device::write_rxd));

	m_i8251->txd_handler().set_nop();
	//m_i8251->txd_handler().set("ms7004", FUNC(ms7004_device::rx_w));

	// baud rate is supposed to be 4800 but keyboard is slightly faster
	CLOCK(config.replace(), "keyboard_clock", 4960*64).signal_handler().set(FUNC(vt240_state::write_keyboard_clock));
}

/* ROM definition */
ROM_START( mc7105 )
	ROM_REGION( 0x10000, "charcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "027.bin", 0x8000, 0x8000, CRC(a159b412) SHA1(956097ccc2652d494258b3682498cfd3096d7d4f))
	ROM_LOAD( "028.bin", 0x0000, 0x8000, CRC(b253151f) SHA1(22ffeef8eb5df3c38bfe91266f26d1e7822cdb53))

	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "029.bin", 0x00000, 0x8000, CRC(4a6db217) SHA1(47637325609ea19ffab61fe31e2700d72fa50729))
	ROM_LOAD16_BYTE( "031.bin", 0x00001, 0x8000, CRC(47129579) SHA1(39de9e2e26f90c5da5e72a09ff361c1a94b9008a))
	ROM_LOAD16_BYTE( "030.bin", 0x10000, 0x8000, CRC(05fd7b75) SHA1(2ad8c14e76accfa1b9b8748c58e9ebbc28844a47))
	ROM_LOAD16_BYTE( "032.bin", 0x10001, 0x8000, CRC(e81d93c4) SHA1(982412a7a6e65d6f6b4f66bd093e54ee16f31384))
ROM_END

/* ROM definition */
ROM_START( vt240 )
	ROM_REGION( 0x10000, "charcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-008e6-00.e100", 0x0000, 0x4000, CRC(ebc8a2fe) SHA1(70838175f8302fdc0dee79b2403fa95e6d989206))
	ROM_CONTINUE(0x8000, 0x4000)

	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "vt240" )
	// according to the schematics an even older set exists, variation 'E1' with roms:
	// e100/8085: 23-003e6
	// e20: 23-001e6
	// e22: 23-002e6
	// e19: 23-048e5
	// e21: 23-049e5
	// but according to the Field Change Order below, the initial release is V2.1, so the above must be a prototype.
	// DOL for v2.1 to v2.2 change: http://web.archive.org/web/20060905145200/http://cmcnabb.cc.vt.edu/dec94mds/vt240dol.txt
	ROM_SYSTEM_BIOS( 0, "vt240v21", "VT240 V2.1" ) // initial factory release, FCO says this was 8 Feburary 1985
	ROMX_LOAD( "23-006e6-00.e20", 0x00000, 0x8000, CRC(79c11d82) SHA1(5a6fe5b75b6504a161f2c9b148c0fe9f19770837), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-004e6-00.e22", 0x00001, 0x8000, CRC(eba10fef) SHA1(c0ee4d8e4eeb70066f03f3d17a7e2f2bd0b5f8ad), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-007e6-00.e19", 0x10000, 0x8000, CRC(d18a2ab8) SHA1(37f448a332fc50298007ed39c8bf1ab1eb6d4cae), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-005e6-00.e21", 0x10001, 0x8000, CRC(558d0285) SHA1(e96a49bf9d55d8ab879d9b39aa380368c5c9ade0), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "vt240", "VT240 V2.2" ) // Revised version, December 1985
	ROMX_LOAD( "23-058e6.e20", 0x00000, 0x8000, CRC(d2a56b90) SHA1(39cbb26134d7d8ba308df3a93228918a5945b45f), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-056e6.e22", 0x00001, 0x8000, CRC(c46e13c3) SHA1(0f2801fa7483d1f97708143cd81ae0816bf9a435), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-059e6.e19", 0x10000, 0x8000, CRC(f8393346) SHA1(1e28daf1b7f2bdabc47ce2f6fa99ef038b275a29), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-057e6.e21", 0x10001, 0x8000, CRC(7ce9dce9) SHA1(5a105e5bdca13910b3b79cc23567ce2dc36b844d), ROM_SKIP(1) | ROM_BIOS(1))
	// E39, E85, E131 are empty.

	ROM_REGION( 0x1000, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "23-351a1.e149", 0x0000, 0x0020, NO_DUMP) // 82s123; DRAM RAS/CAS Timing PROM
	ROM_LOAD( "23-352a1.e187", 0x0020, 0x0020, NO_DUMP) // 82s123; "CT0" Timing PROM
	ROM_LOAD( "23-369a1.e53", 0x0040, 0x0020, NO_DUMP) // 82s123; ROM and RAM mapping PROM
	ROM_LOAD( "23-370a1.e188", 0x0060, 0x0020, NO_DUMP) // 82s123; "CT1" Timing PROM
	ROM_LOAD( "23-994a9.e74", 0x0100, 0x0200, NO_DUMP) // 82s131; T11 Interrupt Encoder PROM

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD( "23-087j5.e182.e183.jed", 0x0000, 0x1000, NO_DUMP ) // PAL16L8ACN; "Logic Unit" Character Pattern Related

	ROM_REGION( 0x100, "x2212", 0 ) // default nvram to avoid error 10
	ROM_LOAD( "x2212", 0x000, 0x100, CRC(31c90c64) SHA1(21a0f1d4eec1ced04b85923151783bf23d18bfbd) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                          FULLNAME  FLAGS */
COMP( 1983, vt240,  0,      0,      vt240,   vt240, vt240_state, empty_init, "Digital Equipment Corporation", "VT240",  MACHINE_IMPERFECT_GRAPHICS )
//COMP( 1983, vt241,  0,      0,      vt220,   vt220, vt240_state, empty_init, "Digital Equipment Corporation", "VT241",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
// NOTE: the only difference between VT240 and VT241 is the latter comes with a VR241 Color monitor, while the former comes with a mono display; the ROMs and operation are identical.
COMP( 1983, mc7105, 0,      0,      mc7105,  vt240, vt240_state, empty_init, "Elektronika",                   "MC7105", MACHINE_IMPERFECT_GRAPHICS )
