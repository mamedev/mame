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
#include "rendlay.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(DECODMD1, decodmd_type1_device, "decodmd1", "Data East Pinball Dot Matrix Display Type 1")

READ8_MEMBER( decodmd_type1_device::latch_r )
{
	return 0;
}

WRITE8_MEMBER( decodmd_type1_device::data_w )
{
	m_latch = data;
}

READ8_MEMBER( decodmd_type1_device::busy_r )
{
	return m_status;
}


WRITE8_MEMBER( decodmd_type1_device::ctrl_w )
{
	if((data | m_ctrl) & 0x01)
	{
		m_command = m_latch;
		set_busy(B_CLK,data & 0x01);
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_rombank1->set_entry(0);
		set_busy(B_SET,0);
		m_rowselect = 0;
		m_blank = 0;
		m_frameswap = false;
		m_status = 0;
		m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
	m_ctrl = data;
}

READ8_MEMBER( decodmd_type1_device::ctrl_r )
{
	return m_ctrl;
}

READ8_MEMBER( decodmd_type1_device::status_r )
{
	return (m_busy & 0x01) | (m_status << 1);
}

WRITE8_MEMBER( decodmd_type1_device::status_w )
{
	m_status = data;
}

// Z80 I/O ports not fully decoded.
// if bit 7 = 0, then when bit 2 is 0 selects COCLK, and when bit 2 is 1 selects CLATCH
READ8_MEMBER( decodmd_type1_device::dmd_port_r )
{
	if((offset & 0x84) == 0x80)
	{
		// IDAT (read only)
		//m_ctrl &= ~0x01;
		set_busy(B_CLR,0);
		set_busy(B_CLR,1);
		return m_command;
	}
	return 0xff;
}

WRITE8_MEMBER( decodmd_type1_device::dmd_port_w )
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

WRITE_LINE_MEMBER(decodmd_type1_device::blank_w)
{
	m_blank = state;
	if (state)
		output_data();
}

WRITE_LINE_MEMBER(decodmd_type1_device::status_w)
{
	m_status = state;
}

WRITE_LINE_MEMBER(decodmd_type1_device::rowdata_w)
{
	m_rowdata = state;
}

WRITE_LINE_MEMBER(decodmd_type1_device::rowclock_w)
{
	if (!state && m_rowclock)  // on negative edge
		m_rowselect = (m_rowselect << 1) | m_rowdata;
	m_rowclock = state;
}

WRITE_LINE_MEMBER(decodmd_type1_device::test_w)
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
	uint8_t newval = (m_busy_lines & ~input) | (val ? input : 0);

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
	map(0x0000, 0x3fff).bankr("dmdbank2"); // last 16k of ROM
	map(0x4000, 0x7fff).bankr("dmdbank1");
	map(0x8000, 0x9fff).bankrw("dmdram");
}

void decodmd_type1_device::decodmd1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(this, FUNC(decodmd_type1_device::dmd_port_r), FUNC(decodmd_type1_device::dmd_port_w));
}

MACHINE_CONFIG_START(decodmd_type1_device::device_add_mconfig)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("dmdcpu", Z80, XTAL(8'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(decodmd1_map)
	MCFG_DEVICE_IO_MAP(decodmd1_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", decodmd_type1_device, dmd_nmi, attotime::from_hz(2000))  // seems a lot

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_ADD("dmd",LCD)
	MCFG_SCREEN_SIZE(128, 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 128-1, 0, 16-1)
	MCFG_SCREEN_UPDATE_DRIVER(decodmd_type1_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")

	MCFG_DEVICE_ADD("bitlatch", HC259, 0) // U4
	MCFG_ADDRESSABLE_LATCH_PARALLEL_OUT_CB(MEMBANK("dmdbank1")) MCFG_DEVCB_MASK(0x07) MCFG_DEVCB_INVERT
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(*this, decodmd_type1_device, blank_w))
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(*this, decodmd_type1_device, status_w))
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(*this, decodmd_type1_device, rowdata_w))
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(*this, decodmd_type1_device, rowclock_w))
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(WRITELINE(*this, decodmd_type1_device, test_w))
MACHINE_CONFIG_END


decodmd_type1_device::decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECODMD1, tag, owner, clock),
		m_cpu(*this, "dmdcpu"),
		m_rombank1(*this, "dmdbank1"),
		m_rombank2(*this, "dmdbank2"),
		m_ram(*this, RAM_TAG),
		m_bitlatch(*this, "bitlatch")
{}

void decodmd_type1_device::device_start()
{
	save_pointer(m_pixels,"DMD Video data",0x100);
}

void decodmd_type1_device::device_reset()
{
	uint8_t* ROM;
	uint8_t* RAM = m_ram->pointer();
	m_rom = memregion(m_gfxtag);

	memset(RAM,0,0x2000);
	memset(m_pixels,0,0x200*sizeof(uint32_t));

	ROM = m_rom->base();
	m_rombank1->configure_entries(0, 8, &ROM[0x0000], 0x4000);
	m_rombank2->configure_entry(0, &ROM[0x1c000]);
	m_rombank1->set_entry(0);
	m_rombank2->set_entry(0);
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
	uint8_t x,y,dot;
	uint32_t data1,data2,data3,data4;
	uint32_t col;

	if(m_frameswap)
		ptr = 0x80;

	for(y=0;y<16;y++)  // scanline
	{
		for(x=0;x<128;x+=64)
		{
			data1 = m_pixels[ptr];
			data2 = m_pixels[ptr+1];
			data3 = m_pixels[ptr+2];
			data4 = m_pixels[ptr+3];
			for(dot=0;dot<64;dot+=2)
			{
				if((data1 & 0x01) != (data3 & 0x01))
					col = rgb_t(0x7f,0x55,0x00);
				else if (data1 & 0x01) // both are the same, so either high intensity or none at all
					col = rgb_t(0xff,0xaa,0x00);
				else
					col = rgb_t::black();
				bitmap.pix32(y,x+dot) = col;
				if((data2 & 0x01) != (data4 & 0x01))
					col = rgb_t(0x7f,0x55,0x00);
				else if (data2 & 0x01) // both are the same, so either high intensity or none at all
					col = rgb_t(0xff,0xaa,0x00);
				else
					col = rgb_t::black();
				bitmap.pix32(y,x+dot+1) = col;
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
