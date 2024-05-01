// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *   Data East Pinball Dot Matrix Display
 *
 *    Type 1: 128x16
 *    Z80 @ 4MHz
 */

#include "emu.h"
#include "decodmd1.h"

#include "screen.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(DECODMD1, decodmd_type1_device, "decodmd1", "Data East Pinball Dot Matrix Display Type 1")

uint8_t decodmd_type1_device::latch_r()
{
	return 0;
}

void decodmd_type1_device::data_w(uint8_t data)
{
	m_latch = data;
}

uint8_t decodmd_type1_device::busy_r()
{
	return m_status;
}


void decodmd_type1_device::ctrl_w(uint8_t data)
{
	if((data | m_ctrl) & 0x01)
	{
		m_command = m_latch;
		set_busy(B_CLK,data & 0x01);
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_rombank->set_entry(0);
		set_busy(B_SET,0);
		m_rowselect = 0;
		m_blank = 0;
		m_frameswap = false;
		m_status = 0;
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
	m_ctrl = data;
}

uint8_t decodmd_type1_device::ctrl_r()
{
	return m_ctrl;
}

uint8_t decodmd_type1_device::status_r()
{
	return (m_busy & 0x01) | (m_status << 1);
}

// Z80 I/O ports not fully decoded.
// if bit 7 = 0, then when bit 2 is 0 selects COCLK, and when bit 2 is 1 selects CLATCH
uint8_t decodmd_type1_device::dmd_port_r(offs_t offset)
{
	if((offset & 0x84) == 0x80)
	{
		if(!machine().side_effects_disabled())
		{
			// IDAT (read only)
			//m_ctrl &= ~0x01;
			set_busy(B_CLR,0);
			set_busy(B_CLR,1);
		}
		return m_command;
	}
	return 0xff;
}

void decodmd_type1_device::dmd_port_w(offs_t offset, uint8_t data)
{
	uint8_t bit;

	switch(offset & 0x84)
	{
	case 0x00:  // COCLK
		bit = (data >> ((offset & 0x03)*2));  // selects pair of bits depending on port used (0x00-0x03)
		m_pxdata1 = (m_pxdata1 >> 1) | ((bit & 0x01) ? 0x80000000 : 0x00000000);
		m_pxdata2 = (m_pxdata2 >> 1) | ((bit & 0x02) ? 0x80000000 : 0x00000000);
		break;
	case 0x04:  // CLATCH
		m_pxdata1_latched = m_pxdata1;
		m_pxdata2_latched = m_pxdata2;
		if(m_blank)
			output_data();
		break;
	case 0x80:  // IDAT (ignored)
		break;
	case 0x84:
		bit = data & 0x01;
		m_bitlatch->write_bit((offset & 0x40) >> 4 | (offset & 0x18) >> 3, bit);
		break;
	}
}

void decodmd_type1_device::blank_w(int state)
{
	m_blank = state;
	if (state)
		output_data();
}

void decodmd_type1_device::status_w(int state)
{
	m_status = state;
}

void decodmd_type1_device::rowdata_w(int state)
{
	m_rowdata = state;
}

void decodmd_type1_device::rowclock_w(int state)
{
	if (!state && m_rowclock)  // on negative edge
		m_rowselect = (m_rowselect << 1) | m_rowdata;
	m_rowclock = state;
}

void decodmd_type1_device::test_w(int state)
{
	set_busy(B_SET, state);
}

void decodmd_type1_device::output_data()
{
	uint8_t ptr = 0;
	uint32_t row = m_rowselect;

	if(row == 0)
		m_frameswap = !m_frameswap;

	if(!m_frameswap)
		ptr = 0x80;

	while(row != 0)
	{
		if(row & 0x01)
		{
			m_pixels[ptr] = m_pxdata2_latched;
			m_pixels[ptr+1] = m_pxdata1_latched;
			if(m_prevrow != m_rowselect)
			{
				m_pixels[ptr+2] = m_pixels[ptr];
				m_pixels[ptr+3] = m_pixels[ptr+1];
			}
		}
		ptr += 4;
		row >>= 1;
	}
	m_prevrow = m_rowselect;
}

void decodmd_type1_device::set_busy(uint8_t input, uint8_t val)
{
	uint8_t const newval = (m_busy_lines & ~input) | (val ? input : 0);

	if(~newval & m_busy_lines & B_CLR)
		m_busy = 0;
	else if (~newval & m_busy_lines & B_SET)
		m_busy = 1;
	else if ((newval & (B_CLR|B_SET)) == (B_CLR|B_SET))
	{
		if(newval & ~m_busy_lines & B_CLK)
			m_busy = 1;
	}

	m_busy_lines = newval;

	m_cpu->set_input_line(INPUT_LINE_IRQ0,m_busy ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(decodmd_type1_device::dmd_nmi)
{
	m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void decodmd_type1_device::decodmd1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(DEVICE_SELF, 0x1c000); // last 16k of ROM
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0x9fff).ram().share(m_ram);
}

void decodmd_type1_device::decodmd1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(decodmd_type1_device::dmd_port_r), FUNC(decodmd_type1_device::dmd_port_w));
}

void decodmd_type1_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_cpu, XTAL(8'000'000) / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &decodmd_type1_device::decodmd1_map);
	m_cpu->set_addrmap(AS_IO, &decodmd_type1_device::decodmd1_io_map);

	config.set_maximum_quantum(attotime::from_hz(50));

	TIMER(config, "nmi_timer").configure_periodic(FUNC(decodmd_type1_device::dmd_nmi), attotime::from_hz(2000));  // seems a lot

	screen_device &dmd(SCREEN(config, "dmd", SCREEN_TYPE_LCD));
	dmd.set_size(128, 16);
	dmd.set_visarea(0, 128-1, 0, 16-1);
	dmd.set_screen_update(FUNC(decodmd_type1_device::screen_update));
	dmd.set_refresh_hz(50);

	HC259(config, m_bitlatch); // U4
	m_bitlatch->parallel_out_cb().set_membank(m_rombank).mask(0x07).invert();
	m_bitlatch->q_out_cb<3>().set(FUNC(decodmd_type1_device::blank_w));
	m_bitlatch->q_out_cb<4>().set(FUNC(decodmd_type1_device::status_w));
	m_bitlatch->q_out_cb<5>().set(FUNC(decodmd_type1_device::rowdata_w));
	m_bitlatch->q_out_cb<6>().set(FUNC(decodmd_type1_device::rowclock_w));
	m_bitlatch->q_out_cb<7>().set(FUNC(decodmd_type1_device::test_w));
}


decodmd_type1_device::decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECODMD1, tag, owner, clock)
	, m_cpu(*this, "dmdcpu")
	, m_rombank(*this, "dmdbank")
	, m_ram(*this, "dmdram")
	, m_bitlatch(*this, "bitlatch")
	, m_rom(*this, DEVICE_SELF)
	, m_latch(0)
	, m_status(0)
	, m_ctrl(0)
	, m_busy(0)
	, m_command(0)
	, m_rowclock(0)
	, m_rowdata(0)
	, m_rowselect(0)
	, m_blank(0)
	, m_pxdata1(0)
	, m_pxdata2(0)
	, m_pxdata1_latched(0)
	, m_pxdata2_latched(0)
	, m_frameswap(false)
	, m_busy_lines(0)
	, m_prevrow(0)
{
	std::fill(std::begin(m_pixels), std::end(m_pixels), 0);
}

void decodmd_type1_device::device_start()
{
	m_rombank->configure_entries(0, 8, &m_rom[0x0000], 0x4000);

	save_item(NAME(m_latch));
	save_item(NAME(m_status));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_busy));
	save_item(NAME(m_command));
	save_item(NAME(m_rowclock));
	save_item(NAME(m_rowdata));
	save_item(NAME(m_rowselect));
	save_item(NAME(m_blank));
	save_item(NAME(m_pxdata1));
	save_item(NAME(m_pxdata2));
	save_item(NAME(m_pxdata1_latched));
	save_item(NAME(m_pxdata2_latched));
	save_item(NAME(m_frameswap));
	save_item(NAME(m_pixels));
	save_item(NAME(m_busy_lines));
	save_item(NAME(m_prevrow));
}

void decodmd_type1_device::device_reset()
{
	memset(m_ram,0,0x2000);
	memset(m_pixels,0,0x200*sizeof(uint32_t));

	m_rombank->set_entry(0);
	m_status = 0;
	m_busy = 0;
	set_busy(B_CLR|B_SET,0);
	m_rowselect = 0;
	m_blank = 0;
	m_frameswap = false;
}

uint32_t decodmd_type1_device::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	uint8_t ptr = 0;

	if(m_frameswap)
		ptr = 0x80;

	for(uint8_t y=0;y<16;y++)  // scanline
	{
		for(uint8_t x=0;x<128;x+=64)
		{
			uint32_t data1 = m_pixels[ptr];
			uint32_t data2 = m_pixels[ptr+1];
			uint32_t data3 = m_pixels[ptr+2];
			uint32_t data4 = m_pixels[ptr+3];
			for(uint8_t dot=0;dot<64;dot+=2)
			{
				uint32_t col;
				if((data1 & 0x01) != (data3 & 0x01))
					col = rgb_t(0x7f,0x55,0x00);
				else if (data1 & 0x01) // both are the same, so either high intensity or none at all
					col = rgb_t(0xff,0xaa,0x00);
				else
					col = rgb_t::black();
				bitmap.pix(y,x+dot) = col;
				if((data2 & 0x01) != (data4 & 0x01))
					col = rgb_t(0x7f,0x55,0x00);
				else if (data2 & 0x01) // both are the same, so either high intensity or none at all
					col = rgb_t(0xff,0xaa,0x00);
				else
					col = rgb_t::black();
				bitmap.pix(y,x+dot+1) = col;
				data1 >>= 1;
				data2 >>= 1;
				data3 >>= 1;
				data4 >>= 1;
			}
			ptr+=4;
		}
	}

	return 0;
}
