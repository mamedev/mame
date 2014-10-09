/**********************************************************************

    Sega 315-5296 I/O chip
    
    Sega 100-pin QFP, with 8 bidirectional I/O ports, and 3 output pins.
    Commonly used from the late 80s up until Sega Model 2.

**********************************************************************/

#include "machine/315-5296.h"


const device_type SEGA_315_5296 = &device_creator<sega_315_5296_device>;

//-------------------------------------------------
//  sega_315_5296_device - constructor
//-------------------------------------------------


sega_315_5296_device::sega_315_5296_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_315_5296, "Sega 315-5296", tag, owner, clock, "315-5296", __FILE__),
	m_in_pa_cb(*this),
	m_in_pb_cb(*this),
	m_in_pc_cb(*this),
	m_in_pd_cb(*this),
	m_in_pe_cb(*this),
	m_in_pf_cb(*this),
	m_in_pg_cb(*this),
	m_in_ph_cb(*this),
	m_out_pa_cb(*this),
	m_out_pb_cb(*this),
	m_out_pc_cb(*this),
	m_out_pd_cb(*this),
	m_out_pe_cb(*this),
	m_out_pf_cb(*this),
	m_out_pg_cb(*this),
	m_out_ph_cb(*this),
	m_out_cnt0_cb(*this),
	m_out_cnt1_cb(*this),
	m_out_cnt2_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5296_device::device_start()
{
	// resolve callbacks
	m_in_pa_cb.resolve_safe(0xff); m_in_port_cb[0] = &m_in_pa_cb;
	m_in_pb_cb.resolve_safe(0xff); m_in_port_cb[1] = &m_in_pb_cb;
	m_in_pc_cb.resolve_safe(0xff); m_in_port_cb[2] = &m_in_pc_cb;
	m_in_pd_cb.resolve_safe(0xff); m_in_port_cb[3] = &m_in_pd_cb;
	m_in_pe_cb.resolve_safe(0xff); m_in_port_cb[4] = &m_in_pe_cb;
	m_in_pf_cb.resolve_safe(0xff); m_in_port_cb[5] = &m_in_pf_cb;
	m_in_pg_cb.resolve_safe(0xff); m_in_port_cb[6] = &m_in_pg_cb;
	m_in_ph_cb.resolve_safe(0xff); m_in_port_cb[7] = &m_in_ph_cb;
	
	m_out_pa_cb.resolve_safe(); m_out_port_cb[0] = &m_out_pa_cb;
	m_out_pb_cb.resolve_safe(); m_out_port_cb[1] = &m_out_pb_cb;
	m_out_pc_cb.resolve_safe(); m_out_port_cb[2] = &m_out_pc_cb;
	m_out_pd_cb.resolve_safe(); m_out_port_cb[3] = &m_out_pd_cb;
	m_out_pe_cb.resolve_safe(); m_out_port_cb[4] = &m_out_pe_cb;
	m_out_pf_cb.resolve_safe(); m_out_port_cb[5] = &m_out_pf_cb;
	m_out_pg_cb.resolve_safe(); m_out_port_cb[6] = &m_out_pg_cb;
	m_out_ph_cb.resolve_safe(); m_out_port_cb[7] = &m_out_ph_cb;
	
	m_out_cnt0_cb.resolve_safe(); m_out_cnt_cb[0] = &m_out_cnt0_cb;
	m_out_cnt1_cb.resolve_safe(); m_out_cnt_cb[1] = &m_out_cnt1_cb;
	m_out_cnt2_cb.resolve_safe(); m_out_cnt_cb[2] = &m_out_cnt2_cb;
	
	// register for savestates
	save_item(NAME(m_output_latch));
	save_item(NAME(m_cnt));
	save_item(NAME(m_dir));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5296_device::device_reset()
{
	// all ports are set to input
	m_dir = 0;
	
	// clear output ports
	memset(m_output_latch, 0, sizeof(m_output_latch));
	m_cnt = 0;

	for (int i = 0; i < 8; i++)
		(*m_out_port_cb[i])((offs_t)i, 0);
	for (int i = 0; i < 3; i++)
		(*m_out_cnt_cb[i])(0);
}


//-------------------------------------------------

READ8_MEMBER( sega_315_5296_device::read )
{
	offset &= 0xf;
	
	switch (offset)
	{
		// port A to H
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			// if the port is configured as an output, return the last thing written
			if (m_dir & 1 << offset)
				return m_output_latch[offset];

			// otherwise, return an input port
			return (*m_in_port_cb[offset])(offset);
		
		// 'SEGA' protection
		case 0x8:
			return 'S';
		case 0x9:
			return 'E';
		case 0xa:
			return 'G';
		case 0xb:
			return 'A';

		// CNT register & mirror
		case 0xc: case 0xe:
			return m_cnt;

		// port direction register & mirror
		case 0xd: case 0xf:
			return m_dir;
	}
	
	return 0xff;
}


WRITE8_MEMBER( sega_315_5296_device::write )
{
	offset &= 0xf;

	switch (offset)
	{
		// port A to H
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			// if the port is configured as an output, write it
			if (m_dir & 1 << offset)
				(*m_out_port_cb[offset])(offset, data);

			m_output_latch[offset] = data;
			break;
		
		// CNT register
		case 0xe:
			// d0-2: CNT0-2, other bits: ?
			for (int i = 0; i < 3; i++)
				(*m_out_cnt_cb[i])(data >> i & 1);
			
			m_cnt = data;
			break;

		// port direction register
		case 0xf:
			// refresh output ports if the direction changed
			for (int i = 0; i < 8; i++)
			{
				if ((m_dir ^ data) & (1 << i))
					(*m_out_port_cb[i])((offs_t)i, (data & 1 << i) ? m_output_latch[i] : 0);
			}
			
			m_dir = data;
			break;
		
		default:
			break;
	}
}
