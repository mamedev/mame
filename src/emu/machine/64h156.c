// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64H156 Gate Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    http://personalpages.tds.net/~rcarlsen/cbm/1541/1541%20EARLY/1540-2.GIF

    - write circuitry
    - cycle exact VIA

    - get these running and we're golden
    	- Bounty Bob Strikes Back (aligned halftracks)
        - Quiwi (speed change within track)
        - Defender of the Crown (V-MAX! v2, density checks)
        - Test Drive / Cabal (HLS, sub-cycle jitter)
        - Galaxian (?, needs 100% accurate VIA)

*/

#include "64h156.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define ATN (m_atni ^ m_atna)

#define CYCLES_UNTIL_ANALOG_DESYNC      288 // 18 us



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64H156 = &device_creator<c64h156_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_atn_line -
//-------------------------------------------------

inline void c64h156_device::set_atn_line()
{
	m_write_atn(ATN);
}


//-------------------------------------------------
//  get_next_edge -
//-------------------------------------------------

inline void c64h156_device::get_next_edge(attotime when)
{
	m_edge = m_floppy->get_next_transition(when);
}


//-------------------------------------------------
//  receive_bit - receive bit
//-------------------------------------------------

inline void c64h156_device::receive_bit()
{
	attotime when = machine().time();
	attotime next = when + m_period;

	m_bit_sync = (m_edge.is_never() || m_edge >= next) ? 0 : 1;

	if (m_bit_sync) {
		m_zero_count = 0;
		m_cycles_until_random_flux = (rand() % 31) + 289;

		get_next_edge(when);
	} else {
		m_zero_count++;
	}

	if (m_zero_count >= m_cycles_until_random_flux) {
		m_bit_sync = 1;

		m_zero_count = 0;
		m_cycles_until_random_flux = (rand() % 367) + 33;
	}
}


//-------------------------------------------------
//  decode_bit -
//-------------------------------------------------

inline void c64h156_device::decode_bit()
{
	// UE7

	bool ue7_tc = false;

	if (!m_last_bit_sync && m_bit_sync)
	{
		m_ue7 = m_ds;
	}
	else
	{
		m_ue7++;

		if (m_ue7 == 16)
		{
			m_ue7 = m_ds;
			ue7_tc = true;
		}
	}

	if (LOG) logerror("%s UE7 CTR %01x TC %u, ", machine().time().as_string(),m_ue7, ue7_tc);

	// UF4

	if (!m_last_bit_sync && m_bit_sync)
	{
		m_uf4 = 0;

		if (LOG) logerror("--");
	}
	else
	{
		if (!m_ue7_tc && ue7_tc)
		{
			m_uf4++;
			m_uf4 &= 0x0f;

			if (LOG) logerror("++");
		}
		else
		{
			if (LOG) logerror("  ");
		}
	}

	m_last_bit_sync = m_bit_sync;
	m_ue7_tc = ue7_tc;

	int uf4_qa = BIT(m_uf4, 0);
	int uf4_qb = BIT(m_uf4, 1);
	int uf4_qc = BIT(m_uf4, 2);
	int uf4_qd = BIT(m_uf4, 3);

	int ue5a = !(uf4_qc || uf4_qd);

	if (LOG) logerror("UF4 CTR %01x %u%u%u%u UE5A %u, ", m_uf4, uf4_qd, uf4_qc, uf4_qb, uf4_qa, ue5a);

	if (!m_uf4_qb && uf4_qb)
	{
		// shift bits thru flip-flops
		m_u4b = m_u4a;
		m_u4a = BIT(m_ud2, 7);

		// shift in data bit
		m_ud2 <<= 1;
		m_ud2 |= ue5a;

		if (LOG) logerror("<<");
	}
	else
	{
		if (LOG) logerror("  ");
	}

	if (LOG) logerror("UD2 %u%u%u%u%u%u%u%u%u%u : %02x (%02x), ", m_u4b, m_u4a, BIT(m_ud2, 7), BIT(m_ud2, 6), BIT(m_ud2, 5), BIT(m_ud2, 4), BIT(m_ud2, 3), BIT(m_ud2, 2), BIT(m_ud2, 1), BIT(m_ud2, 0), m_ud2, m_shift & 0xff);

	// UE3

	int block_sync = !(m_oe && (m_ud2 == 0xff) && m_u4a && m_u4b);

	if (LOG) logerror("SYNC %u, ", block_sync);

	if (!block_sync)
	{
		// load UE3
		m_ue3 = 8; // pin D is floating and TTL inputs float high

		if (LOG) logerror("--");
	}
	else
	{
		if (m_block_sync && !m_uf4_qb && uf4_qb)
		{
			// clock UE3
			m_ue3++;
			m_ue3 &= 0x0f;

			if (LOG) logerror("++");
		}
		else
		{
			if (LOG) logerror("  ");
		}
	}

	int ue3_qa = BIT(m_ue3, 0);
	int ue3_qb = BIT(m_ue3, 1);
	int ue3_qc = BIT(m_ue3, 2);

	int uf3a = !(ue3_qa && ue3_qb && ue3_qc);
	int uc1b = !uf3a; // schmitt trigger

	if (LOG) logerror("UE3 CTR %01x UF3A %u UC1B %u, ", m_ue3, uf3a, uc1b);

	int byte_sync = !(uc1b && m_soe && !uf4_qb);

	if (LOG) logerror("BYTE %u SOE %u\n", byte_sync, m_soe);

	// UD3
#ifdef WRITE_SUPPORTED
	int uf3b = !(uc1b && uf4_qa && uf4_qb);

	if (!uf3b)
	{
		m_ud3 = m_via_pa;
	}
	else if (!m_uf4_qb && uf4_qb)
	{
		// shift out bits
		int ud3_qh = BIT(m_ud3, 0);
		m_ud3 >>= 1;

		int uf5b = !(!uf4_qb && ud3_qh);

		if (!m_oe && m_floppy->wpt_r())
		{
			// TODO write bit to disk
			if (LOG) logerror("WRITE BIT %u\n", uf5b);
		}
	}
#endif
	// prepare for next cycle

	if (m_block_sync != block_sync)
	{
		m_block_sync = block_sync;
		m_write_sync(m_block_sync);
	}

	if (m_byte_sync != byte_sync)
	{
		m_byte_sync = byte_sync;

		if (m_accl)
		{
			if (!byte_sync)
			{
				m_accl_yb = m_ud2;
				m_accl_byte_sync = byte_sync;
				m_write_byte(m_accl_byte_sync);
			}
		}
		else
		{
			m_write_byte(m_byte_sync);
		}
	}

	m_uf4_qb = uf4_qb;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64h156_device - constructor
//-------------------------------------------------

c64h156_device::c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C64H156, "64H156", tag, owner, clock, "c64h156", __FILE__),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_write_atn(*this),
		m_write_sync(*this),
		m_write_byte(*this),
		m_floppy(NULL),
		m_period(attotime::from_hz(clock)),
		m_edge(attotime::never),
		m_shift(0),
		m_mtr(0),
		m_accl(0),
		m_stp(-1),
		m_ds(0),
		m_soe(0),
		m_oe(0),
		m_atni(1),
		m_atna(1),
		m_last_bit_sync(0),
		m_bit_sync(0),
		m_byte_sync(1),
		m_accl_byte_sync(1),
		m_block_sync(1),
		m_ue7(0),
		m_ue7_tc(0),
		m_uf4(0),
		m_uf4_qb(0),
		m_ud2(0),
		m_accl_yb(0),
		m_u4a(0),
		m_u4b(0),
		m_ue3(0),
		m_uc1b(0),
		m_zero_count(0),
		m_cycles_until_random_flux(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64h156_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_write_atn.resolve_safe();
	m_write_sync.resolve_safe();
	m_write_byte.resolve_safe();

	// register for state saving
	save_item(NAME(m_shift));
	save_item(NAME(m_mtr));
	save_item(NAME(m_accl));
	save_item(NAME(m_stp));
	save_item(NAME(m_ds));
	save_item(NAME(m_soe));
	save_item(NAME(m_oe));
	save_item(NAME(m_atni));
	save_item(NAME(m_atna));
	save_item(NAME(m_last_bit_sync));
	save_item(NAME(m_bit_sync));
	save_item(NAME(m_byte_sync));
	save_item(NAME(m_accl_byte_sync));
	save_item(NAME(m_block_sync));
	save_item(NAME(m_ue7));
	save_item(NAME(m_ue7_tc));
	save_item(NAME(m_uf4));
	save_item(NAME(m_uf4_qb));
	save_item(NAME(m_ud2));
	save_item(NAME(m_accl_yb));
	save_item(NAME(m_u4a));
	save_item(NAME(m_u4b));
	save_item(NAME(m_ue3));
	save_item(NAME(m_uc1b));
	save_item(NAME(m_via_pa));
	save_item(NAME(m_ud3));
	save_item(NAME(m_zero_count));
	save_item(NAME(m_cycles_until_random_flux));
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void c64h156_device::execute_run()
{
	do
	{
		if (m_mtr)
		{
			receive_bit();
		}

		decode_bit();
		
		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  yb_r -
//-------------------------------------------------

READ8_MEMBER( c64h156_device::yb_r )
{
	UINT8 data = 0;

	if (m_soe)
	{
		if (m_accl)
		{
			data = m_accl_yb;
		}
		else
		{
			data = m_ud2;
		}
	}

	if (LOG) logerror("%s YB read %02x:%02x\n", machine().describe_context(), m_ud2, data);

	return data;
}


//-------------------------------------------------
//  yb_w -
//-------------------------------------------------

WRITE8_MEMBER( c64h156_device::yb_w )
{
	m_via_pa = data;
}


//-------------------------------------------------
//  test_w - test write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::test_w )
{
}


//-------------------------------------------------
//  accl_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::accl_w )
{
	m_accl = state;
}


//-------------------------------------------------
//  sync_r - sync read
//-------------------------------------------------

READ_LINE_MEMBER( c64h156_device::sync_r )
{
	return m_block_sync;
}


//-------------------------------------------------
//  byte_r - byte ready read
//-------------------------------------------------

READ_LINE_MEMBER( c64h156_device::byte_r )
{
	int state = 1;

	if (m_accl)
	{
		state = m_accl_byte_sync;
	}
	else
	{
		state = m_byte_sync;
	}

	return state;
}


//-------------------------------------------------
//  ted_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::ted_w )
{
	if (m_accl && !m_accl_byte_sync && !state)
	{
		m_accl_byte_sync = 1;

		m_write_byte(m_accl_byte_sync);
	}
}


//-------------------------------------------------
//  mtr_w - motor write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::mtr_w )
{
	if (m_mtr != state)
	{
		if (LOG) logerror("MTR %u\n", state);

		m_floppy->mon_w(!state);

		m_mtr = state;

		get_next_edge(machine().time());
	}
}


//-------------------------------------------------
//  oe_w - output enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::oe_w )
{
	if (LOG) logerror("OE %u\n", state);

	m_oe = state;
}


//-------------------------------------------------
//  soe_w - SO enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::soe_w )
{
	if (LOG) logerror("SOE %u\n", state);

	m_soe = state;
}


//-------------------------------------------------
//  atn_r - serial attention read
//-------------------------------------------------

READ_LINE_MEMBER( c64h156_device::atn_r )
{
	return ATN;
}


//-------------------------------------------------
//  atni_w - serial attention input write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::atni_w )
{
	if (LOG) logerror("ATNI %u\n", state);

	m_atni = state;

	set_atn_line();
}


//-------------------------------------------------
//  atna_w - serial attention acknowledge write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::atna_w )
{
	if (LOG) logerror("ATNA %u\n", state);

	m_atna = state;

	set_atn_line();
}


//-------------------------------------------------
//  set_floppy -
//-------------------------------------------------

void c64h156_device::set_floppy(floppy_image_device *floppy)
{
	m_floppy = floppy;
}


//-------------------------------------------------
//  stp_w -
//-------------------------------------------------

void c64h156_device::stp_w(int stp)
{
	if (m_mtr)
	{
		if (m_stp != stp)
		{
			int track = m_floppy->get_cyl();
			int tracks = (stp - track) & 0x03;

			if (tracks == 3)
			{
				tracks = -1;
			}

			if (tracks == -1)
			{
				m_floppy->dir_w(1);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}
			else if (tracks == 1)
			{
				m_floppy->dir_w(0);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}
		}

		m_stp = stp;

		get_next_edge(machine().time());
	}
}


//-------------------------------------------------
//  ds_w - density select
//-------------------------------------------------

void c64h156_device::ds_w(int ds)
{
	if (LOG) logerror("DS %u\n", ds & 0x03);

	m_ds = ds & 0x03;
}
