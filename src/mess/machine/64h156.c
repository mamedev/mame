/**********************************************************************

    Commodore 64H156 Gate Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    http://staff.washington.edu/rrcc/uwweb/1541early/1540-2.GIF

    - model disk rotation for proper track alignment
    - write circuitry
    - cycle exact M6502
    - cycle exact VIA
    - refactor to use modern floppy system

    - get these running and we're golden
        - Quiwi (speed change within track)
        - Defender of the Crown (V-MAX! v2, density checks)
        - Test Drive / Cabal (HLS, sub-cycle jitter)
        - Galaxian (?, needs 100% accurate VIA)

*/

#include "emu.h"
#include "64h156.h"
#include "formats/g64_dsk.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define ATN (m_atni ^ m_atna)

#define CYCLES_UNTIL_ANALOG_DESYNC		288 // 18 us



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64H156 = &device_creator<c64h156_device>;

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void c64h156_device::device_config_complete()
{
	// inherit a copy of the static data
	const c64h156_interface *intf = reinterpret_cast<const c64h156_interface *>(static_config());
	if (intf != NULL)
		*static_cast<c64h156_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_atn_cb, 0, sizeof(m_out_atn_cb));
		memset(&m_out_sync_cb, 0, sizeof(m_out_sync_cb));
		memset(&m_out_byte_cb, 0, sizeof(m_out_byte_cb));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_atn_line -
//-------------------------------------------------

inline void c64h156_device::set_atn_line()
{
	m_out_atn_func(ATN);
}


//-------------------------------------------------
//  read_current_track -
//-------------------------------------------------

inline void c64h156_device::read_current_track()
{
	int track_length = G64_BUFFER_SIZE;

	// read track data
	floppy_drive_read_track_data_info_buffer(m_image, m_side, m_track_buffer, &track_length);

	// extract track length
	m_track_len = floppy_drive_get_current_track_size(m_image, m_side);

	// set bit pointer to track start
	m_buffer_pos = 0;
	m_bit_pos = 7;
	m_bit_count = 0;
	m_zero_count = 0;

	if (m_track_len)
	{
		memcpy(m_speed_buffer, m_track_buffer + m_track_len, G64_SPEED_BLOCK_SIZE);

		update_cycles_until_next_bit();
	}
}


//-------------------------------------------------
//  update_cycles_until_next_bit -
//-------------------------------------------------

inline void c64h156_device::update_cycles_until_next_bit()
{
	int speed_offset = m_buffer_pos / 4;
	int speed_shift = (3 - (m_buffer_pos % 4)) * 2;

	UINT8 speed = m_speed_buffer[speed_offset];

	int ds = (speed >> speed_shift) & 0x03;

	m_cycles_until_next_bit = (16 - ds) * 4;

	if (LOG) logerror("buff %04x:%u speed %04x shift %u ds %u cycles %u\n", m_buffer_pos, m_bit_pos, speed_offset, speed_shift, ds, m_cycles_until_next_bit);
}


//-------------------------------------------------
//  receive_bit - receive bit
//-------------------------------------------------

inline void c64h156_device::receive_bit()
{
	if (!m_track_len)
	{
		m_bit_sync = 0;
	}
	else
	{
		if (m_cycles_until_next_bit == 0)
		{
			m_bit_sync = BIT(m_track_buffer[m_buffer_pos], m_bit_pos);

			if (m_bit_sync)
			{
				m_zero_count = 0;
				m_cycles_until_random_flux = (rand() % 31) + 289;
			}
			else
			{
				m_zero_count++;
			}

			if (m_zero_count >= m_cycles_until_random_flux)
			{
				// receive phantom bit
				if (LOG) logerror("PHANTOM BIT SYNC 1 after %u cycles\n", m_zero_count);

				m_bit_sync = 1;

				m_zero_count = 0;
				m_cycles_until_random_flux = (rand() % 367) + 33;
			}
			else
			{
				if (LOG) logerror("BIT SYNC %u\n", m_bit_sync);
			}

			m_shift <<= 1;
			m_shift |= m_bit_sync;

			m_bit_pos--;
			m_bit_count++;

			if (m_bit_pos < 0)
			{
				m_bit_pos = 7;
				m_buffer_pos++;

				if (m_buffer_pos >= m_track_len)
				{
					// loop to the start of the track
					m_buffer_pos = 0;
				}
			}

			update_cycles_until_next_bit();
		}

		m_cycles_until_next_bit--;
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

	if (LOG) logerror("UE7 CTR %01x TC %u, ", m_ue7, ue7_tc);

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

			if (m_ue3 == 16)
			{
				m_ue3 = 0;
			}

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

	if (LOG) logerror("BYTE %u SOE %u\n", m_byte_sync, m_soe);

	// UD3

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

		if (!m_oe && m_wp)
		{
			// TODO write bit to disk
			if (LOG) logerror("WRITE BIT %u\n", uf5b);
		}
	}

	// prepare for next cycle

	if (m_block_sync != block_sync)
	{
		m_block_sync = block_sync;
		m_out_sync_func(m_block_sync);
	}

	if (m_byte_sync != byte_sync)
	{
		m_byte_sync = byte_sync;
		m_out_byte_func(m_byte_sync);
	}

	m_uf4_qb = uf4_qb;

	m_bit_sync = 0;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64h156_device - constructor
//-------------------------------------------------

c64h156_device::c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, C64H156, "64H156", tag, owner, clock),
      device_execute_interface(mconfig, *this),
      m_icount(0),
	  m_image(*this->owner(), FLOPPY_0),
	  m_side(0),
	  m_track_buffer(NULL),
	  m_track_len(0),
	  m_buffer_pos(0),
	  m_bit_pos(0),
	  m_bit_count(0),
	  m_stp(-1),
	  m_mtr(0),
	  m_accl(1),
	  m_ds(0),
	  m_soe(0),
	  m_oe(0),
	  m_atni(1),
	  m_atna(1),
	  m_last_bit_sync(0),
	  m_bit_sync(0),
	  m_byte_sync(1),
	  m_block_sync(1),
	  m_ue7(0),
	  m_ue7_tc(0),
	  m_uf4(0),
	  m_uf4_qb(0),
	  m_ud2(0),
	  m_u4a(0),
	  m_u4b(0),
	  m_ue3(0),
	  m_uc1b(0),
	  m_wp(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64h156_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// allocate track buffer
	m_track_buffer = auto_alloc_array(machine(), UINT8, G64_BUFFER_SIZE);
	m_speed_buffer = auto_alloc_array(machine(), UINT8, G64_SPEED_BLOCK_SIZE);

	// resolve callbacks
	m_out_atn_func.resolve(m_out_atn_cb, *this);
	m_out_sync_func.resolve(m_out_sync_cb, *this);
	m_out_byte_func.resolve(m_out_byte_cb, *this);

	// register for state saving
	save_item(NAME(m_shift));
	save_item(NAME(m_side));
	save_pointer(NAME(m_track_buffer), G64_BUFFER_SIZE);
	save_pointer(NAME(m_speed_buffer), G64_SPEED_BLOCK_SIZE);
	save_item(NAME(m_track_len));
	save_item(NAME(m_buffer_pos));
	save_item(NAME(m_bit_pos));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_cycles_until_next_bit));
	save_item(NAME(m_zero_count));
	save_item(NAME(m_cycles_until_random_flux));
	save_item(NAME(m_stp));
	save_item(NAME(m_mtr));
	save_item(NAME(m_accl));
	save_item(NAME(m_ds));
	save_item(NAME(m_soe));
	save_item(NAME(m_oe));
	save_item(NAME(m_atni));
	save_item(NAME(m_atna));
	save_item(NAME(m_last_bit_sync));
	save_item(NAME(m_bit_sync));
	save_item(NAME(m_byte_sync));
	save_item(NAME(m_block_sync));
	save_item(NAME(m_ue7));
	save_item(NAME(m_ue7_tc));
	save_item(NAME(m_uf4));
	save_item(NAME(m_uf4_qb));
	save_item(NAME(m_ud2));
	save_item(NAME(m_u4a));
	save_item(NAME(m_u4b));
	save_item(NAME(m_ue3));
	save_item(NAME(m_uc1b));
	save_item(NAME(m_via_pa));
	save_item(NAME(m_ud3));
	save_item(NAME(m_wp));
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
		data = m_ud2;
	}

	if (LOG) logerror("YB read %02x:%02x\n", m_ud2, data);

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
	return m_byte_sync;
}


//-------------------------------------------------
//  mtr_w - motor write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::mtr_w )
{
	if (m_mtr != state)
	{
		if (LOG) logerror("MTR %u\n", state);

		if (state)
		{
			// read track data
			read_current_track();
		}

		floppy_mon_w(m_image, !state);

		m_mtr = state;
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
//  stp_w -
//-------------------------------------------------

void c64h156_device::stp_w(int data)
{
	if (m_mtr && (m_stp != data))
	{
		int tracks = 0;

		switch (m_stp)
		{
		case 0:	if (data == 1) tracks++; else if (data == 3) tracks--; break;
		case 1:	if (data == 2) tracks++; else if (data == 0) tracks--; break;
		case 2: if (data == 3) tracks++; else if (data == 1) tracks--; break;
		case 3: if (data == 0) tracks++; else if (data == 2) tracks--; break;
		}

		if (tracks != 0)
		{
			// step read/write head
			floppy_drive_seek(m_image, tracks);

			// read new track data
			read_current_track();
		}

		m_stp = data;
	}
}


//-------------------------------------------------
//  ds_w - density select
//-------------------------------------------------

void c64h156_device::ds_w(int data)
{
	m_ds = data;
}


//-------------------------------------------------
//  on_disk_changed -
//-------------------------------------------------

void c64h156_device::on_disk_changed(int wp)
{
	m_wp = wp;

	read_current_track();
}


//-------------------------------------------------
//  set_side -
//-------------------------------------------------

void c64h156_device::set_side(int side)
{
	m_side = side;
}
