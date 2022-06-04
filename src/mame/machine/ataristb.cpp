// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert

#include "emu.h"
#include "machine/ataristb.h"



//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

DEFINE_DEVICE_TYPE(ST_BLITTER, st_blitter_device, "st_blitter", "Atari ST Blitter")

static const int BLITTER_NOPS[16][4] =
{
	{ 1, 1, 1, 1 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 1, 1 }
};


//**************************************************************************
//  BLITTER
//**************************************************************************

//-------------------------------------------------
//  blitter_source -
//-------------------------------------------------

void st_blitter_device::blitter_source()
{
	uint16_t data = m_space->read_word(m_src);

	if (m_src_inc_x < 0)
	{
		m_srcbuf = (data << 16) | (m_srcbuf >> 16);
	}
	else
	{
		m_srcbuf = (m_srcbuf << 16) | data;
	}
}


//-------------------------------------------------
//  blitter_hop -
//-------------------------------------------------

uint16_t st_blitter_device::blitter_hop()
{
	uint16_t source = m_srcbuf >> (m_skew & 0x0f);
	uint16_t halftone = m_halftone[m_ctrl & 0x0f];

	if (m_ctrl & CTRL_SMUDGE)
	{
		halftone = m_halftone[source & 0x0f];
	}

	switch (m_hop)
	{
	case 0:
		return 0xffff;
	case 1:
		return halftone;
	case 2:
		return source;
	case 3:
		return source & halftone;
	}

	return 0;
}


//-------------------------------------------------
//  blitter_op -
//-------------------------------------------------

void st_blitter_device::blitter_op(uint16_t s, uint32_t dstaddr, uint16_t mask)
{
	uint16_t d = m_space->read_word(dstaddr);
	uint16_t result = 0;

	if (m_op & 0x08) result = (~s & ~d);
	if (m_op & 0x04) result |= (~s & d);
	if (m_op & 0x02) result |= (s & ~d);
	if (m_op & 0x01) result |= (s & d);

	m_space->write_word(dstaddr, result);
}


//-------------------------------------------------
//  blitter_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(st_blitter_device::blitter_tick)
{
	do
	{
		if (m_skew & SKEW_FXSR)
		{
			blitter_source();
			m_src += m_src_inc_x;
		}

		blitter_source();
		blitter_op(blitter_hop(), m_dst, m_endmask1);
		m_xcount--;

		while (m_xcount > 0)
		{
			m_src += m_src_inc_x;
			m_dst += m_dst_inc_x;

			if (m_xcount == 1)
			{
				if (!(m_skew & SKEW_NFSR))
				{
					blitter_source();
				}

				blitter_op(blitter_hop(), m_dst, m_endmask3);
			}
			else
			{
				blitter_source();
				blitter_op(blitter_hop(), m_dst, m_endmask2);
			}

			m_xcount--;
		}

		m_src += m_src_inc_y;
		m_dst += m_dst_inc_y;

		if (m_dst_inc_y < 0)
		{
			m_ctrl = (m_ctrl & 0xf0) | (((m_ctrl & 0x0f) - 1) & 0x0f);
		}
		else
		{
			m_ctrl = (m_ctrl & 0xf0) | (((m_ctrl & 0x0f) + 1) & 0x0f);
		}

		m_xcount = m_xcountl;
		m_ycount--;
	}
	while (m_ycount > 0);

	m_ctrl &= 0x7f;

	m_int_callback(0); // active low
}


//-------------------------------------------------
//  halftone_r -
//-------------------------------------------------

uint16_t st_blitter_device::halftone_r(offs_t offset)
{
	return m_halftone[offset];
}


//-------------------------------------------------
//  src_inc_x_r -
//-------------------------------------------------

uint16_t st_blitter_device::src_inc_x_r()
{
	return m_src_inc_x;
}


//-------------------------------------------------
//  src_inc_y_r -
//-------------------------------------------------

uint16_t st_blitter_device::src_inc_y_r()
{
	return m_src_inc_y;
}


//-------------------------------------------------
//  src_r -
//-------------------------------------------------

uint16_t st_blitter_device::src_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return (m_src >> 16) & 0xff;
	case 1:
		return m_src & 0xfffe;
	}

	return 0;
}


//-------------------------------------------------
//  end_mask_r -
//-------------------------------------------------

uint16_t st_blitter_device::end_mask_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return m_endmask1;
	case 1:
		return m_endmask2;
	case 2:
		return m_endmask3;
	}

	return 0;
}


//-------------------------------------------------
//  dst_inc_x_r -
//-------------------------------------------------

uint16_t st_blitter_device::dst_inc_x_r()
{
	return m_dst_inc_x;
}


//-------------------------------------------------
//  dst_inc_y_r -
//-------------------------------------------------

uint16_t st_blitter_device::dst_inc_y_r()
{
	return m_dst_inc_y;
}


//-------------------------------------------------
//  blitter_dst_r -
//-------------------------------------------------

uint16_t st_blitter_device::dst_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return (m_dst >> 16) & 0xff;
	case 1:
		return m_dst & 0xfffe;
	}

	return 0;
}


//-------------------------------------------------
//  count_x_r -
//-------------------------------------------------

uint16_t st_blitter_device::count_x_r()
{
	return m_xcount;
}


//-------------------------------------------------
//  count_y_r -
//-------------------------------------------------

uint16_t st_blitter_device::count_y_r()
{
	return m_ycount;
}


//-------------------------------------------------
//  op_r -
//-------------------------------------------------

uint16_t st_blitter_device::op_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_hop;
	}
	else
	{
		return m_op;
	}
}


//-------------------------------------------------
//  ctrl_r -
//-------------------------------------------------

uint16_t st_blitter_device::ctrl_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_ctrl;
	}
	else
	{
		return m_skew;
	}
}


//-------------------------------------------------
//  halftone_w -
//-------------------------------------------------

void st_blitter_device::halftone_w(offs_t offset, uint16_t data)
{
	m_halftone[offset] = data;
}


//-------------------------------------------------
//  src_inc_x_w -
//-------------------------------------------------

void st_blitter_device::src_inc_x_w(uint16_t data)
{
	m_src_inc_x = data & 0xfffe;
}


//-------------------------------------------------
//  src_inc_y_w -
//-------------------------------------------------

void st_blitter_device::src_inc_y_w(uint16_t data)
{
	m_src_inc_y = data & 0xfffe;
}


//-------------------------------------------------
//  src_w -
//-------------------------------------------------

void st_blitter_device::src_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
		m_src = (data & 0xff) | (m_src & 0xfffe);
		break;

	case 1:
		m_src = (m_src & 0xff0000) | (data & 0xfffe);
		break;
	}
}


//-------------------------------------------------
//  end_mask_w -
//-------------------------------------------------

void st_blitter_device::end_mask_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
		m_endmask1 = data;
		break;

	case 1:
		m_endmask2 = data;
		break;

	case 2:
		m_endmask3 = data;
		break;
	}
}


//-------------------------------------------------
//  dst_inc_x_w -
//-------------------------------------------------

void st_blitter_device::dst_inc_x_w(uint16_t data)
{
	m_dst_inc_x = data & 0xfffe;
}


//-------------------------------------------------
//  dst_inc_y_w -
//-------------------------------------------------

void st_blitter_device::dst_inc_y_w(uint16_t data)
{
	m_dst_inc_y = data & 0xfffe;
}


//-------------------------------------------------
//  dst_w -
//-------------------------------------------------

void st_blitter_device::dst_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
		m_dst = (data & 0xff) | (m_dst & 0xfffe);
		break;

	case 1:
		m_dst = (m_dst & 0xff0000) | (data & 0xfffe);
		break;
	}
}


//-------------------------------------------------
//  count_x_w -
//-------------------------------------------------

void st_blitter_device::count_x_w(uint16_t data)
{
	m_xcount = data;
}


//-------------------------------------------------
//  count_y_w -
//-------------------------------------------------

void st_blitter_device::count_y_w(uint16_t data)
{
	m_ycount = data;
}


//-------------------------------------------------
//  op_w -
//-------------------------------------------------

void st_blitter_device::op_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_hop = (data >> 8) & 0x03;
	}
	else
	{
		m_op = data & 0x0f;
	}
}


//-------------------------------------------------
//  ctrl_w -
//-------------------------------------------------

void st_blitter_device::ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_ctrl = (data >> 8) & 0xef;

		if (!(m_ctrl & CTRL_BUSY))
		{
			if ((data >> 8) & CTRL_BUSY)
			{
				m_int_callback(1);

				int nops = BLITTER_NOPS[m_op][m_hop]; // each NOP takes 4 cycles
				m_blitter_timer->adjust(clocks_to_attotime(4*nops));
			}
		}
	}
	else
	{
		m_skew = data & 0xcf;
	}
}



//**************************************************************************
//  VIDEO
//**************************************************************************

st_blitter_device::st_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ST_BLITTER, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, 0)
	, m_int_callback(*this)
	, m_blitter_timer(nullptr)
{
}


void st_blitter_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
}


void st_blitter_device::device_start()
{
	m_blitter_timer = timer_alloc(FUNC(st_blitter_device::blitter_tick), this);

	// register for state saving
	save_item(NAME(m_halftone));
	save_item(NAME(m_src_inc_x));
	save_item(NAME(m_src_inc_y));
	save_item(NAME(m_dst_inc_x));
	save_item(NAME(m_dst_inc_y));
	save_item(NAME(m_src));
	save_item(NAME(m_dst));
	save_item(NAME(m_endmask1));
	save_item(NAME(m_endmask2));
	save_item(NAME(m_endmask3));
	save_item(NAME(m_xcount));
	save_item(NAME(m_ycount));
	save_item(NAME(m_xcountl));
	save_item(NAME(m_hop));
	save_item(NAME(m_op));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_skew));
}


void st_blitter_device::device_reset()
{
}
