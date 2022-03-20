// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

    Akiko

    ASIC used in the Amiga CD32. Commodore Part number 391563-01.

    - CD-ROM controller
    - Builtin 1KB NVRAM
    - Chunky to planar converter
    - 2x CIA chips

	TODO:
    - Reportedly the CD drive should be a Sony KSM-2101BAM,
	  schematics shows Akiko connected to a laconic "26-pin CD connector"
	- NVRAM needs inheriting from i2c_24c08_device;
	- Handle tray open/close events, needed at very least by:
	  \- cdtv:cdremix2 load sequences;
	  \- kangfu on cd32 as "out of memory" workaround;

***************************************************************************/

#include "emu.h"
#include "akiko.h"
#include "coreutil.h"
#include "romload.h"

#define LOG_WARN        (1U << 1) // Show warnings
#define LOG_REGS        (1U << 2) // Show register r/w
#define LOG_CD          (1U << 3) // Show CD interactions and commands

#define VERBOSE (LOG_WARN)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGWARN(...)       LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGREGS(...)       LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGCD(...)         LOGMASKED(LOG_CD, __VA_ARGS__)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AKIKO, akiko_device, "akiko", "CBM AKIKO")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void akiko_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  akiko_device - constructor
//-------------------------------------------------

akiko_device::akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AKIKO, tag, owner, clock)
	, m_c2p_input_index(0)
	, m_c2p_output_index(0)
	, m_i2c_scl_out(0)
	, m_i2c_scl_dir(0)
	, m_i2c_sda_out(0)
	, m_i2c_sda_dir(0)
	, m_cdrom_track_index(0)
	, m_cdrom_lba_start(0)
	, m_cdrom_lba_end(0)
	, m_cdrom_lba_cur(0)
	, m_cdrom_readmask(0)
	, m_cdrom_readreqmask(0)
	, m_cdrom_dmacontrol(0)
	, m_cdrom_numtracks(0)
	, m_cdrom_speed(0)
	, m_cdrom_cmd_start(0)
	, m_cdrom_cmd_end(0)
	, m_cdrom_cmd_resp(0)
	, m_cdda(*this, "^cdda")
	, m_cddevice(*this, "^cdrom")
	, m_cdrom(nullptr)
	, m_cdrom_toc(nullptr)
	, m_dma_timer(nullptr)
	, m_frame_timer(nullptr)
	, m_mem_r(*this), m_mem_w(*this), m_int_w(*this)
	, m_scl_w(*this), m_sda_r(*this), m_sda_w(*this)
{
	for (int i = 0; i < 8; i++)
	{
		m_c2p_input_buffer[i] = 0;
		m_c2p_output_buffer[i] = 0;
	}

	for (int i = 0; i < 2; i++)
	{
		m_cdrom_status[i] = 0;
		m_cdrom_address[i] = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void akiko_device::device_start()
{
	// resolve callbacks
	m_mem_r.resolve_safe(0xffff);
	m_mem_w.resolve_safe();
	m_int_w.resolve_safe();
	m_scl_w.resolve_safe();
	m_sda_r.resolve_safe(1);
	m_sda_w.resolve_safe();

	m_c2p_input_index = 0;
	m_c2p_output_index = 0;

	m_i2c_scl_out = 0;
	m_i2c_scl_dir = 0;
	m_i2c_sda_out = 0;
	m_i2c_sda_dir = 0;

	m_cdrom_status[0] = m_cdrom_status[1] = 0;
	m_cdrom_address[0] = m_cdrom_address[1] = 0;
	m_cdrom_track_index = 0;
	m_cdrom_lba_start = 0;
	m_cdrom_lba_end = 0;
	m_cdrom_lba_cur = 0;
	m_cdrom_readmask = 0;
	m_cdrom_readreqmask = 0;
	m_cdrom_dmacontrol = 0;
	m_cdrom_numtracks = 0;
	m_cdrom_speed = 0;
	m_cdrom_cmd_start = 0;
	m_cdrom_cmd_end = 0;
	m_cdrom_cmd_resp = 0;

	m_cdrom_toc = nullptr;
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(akiko_device::dma_proc), this));
	m_frame_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(akiko_device::frame_proc), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void akiko_device::device_reset()
{
	if (m_cddevice.found())
	{
		// CD32 case
		m_cdrom = m_cddevice->get_cdrom_file();
	}
	else
	{
		// Arcade case
		m_cdrom = cdrom_open(machine().rom_load().get_disk_handle(":cdrom"));
	}

	/* create the TOC table */
	if ( m_cdrom != nullptr && cdrom_get_last_track(m_cdrom) )
	{
		uint8_t *p;
		int     i, addrctrl = cdrom_get_adr_control( m_cdrom, 0 );
		uint32_t  discend;

		discend = cdrom_get_track_start(m_cdrom,cdrom_get_last_track(m_cdrom)-1);
		discend += cdrom_get_toc(m_cdrom)->tracks[cdrom_get_last_track(m_cdrom)-1].frames;
		discend = lba_to_msf(discend);

		m_cdrom_numtracks = cdrom_get_last_track(m_cdrom)+3;

		m_cdrom_toc = std::make_unique<uint8_t[]>(13*m_cdrom_numtracks);
		memset( m_cdrom_toc.get(), 0, 13*m_cdrom_numtracks);

		p = m_cdrom_toc.get();
		p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
		p[3] = 0xa0; /* first track */
		p[8] = 1;
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa1; /* last track */
		p[8] = cdrom_get_last_track(m_cdrom);
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa2; /* disc end */
		p[8] = (discend >> 16 ) & 0xff;
		p[9] = (discend >> 8 ) & 0xff;
		p[10] = discend & 0xff;
		p += 13;

		for( i = 0; i < cdrom_get_last_track(m_cdrom); i++ )
		{
			uint32_t  trackpos = cdrom_get_track_start(m_cdrom,i);

			trackpos = lba_to_msf(trackpos);
			addrctrl = cdrom_get_adr_control( m_cdrom, i );

			p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
			p[3] = dec_2_bcd( i+1 );
			p[8] = (trackpos >> 16 ) & 0xff;
			p[9] = (trackpos >> 8 ) & 0xff;
			p[10] = trackpos & 0xff;

			p += 13;
		}
	}

}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void akiko_device::device_stop()
{
	if (!m_cddevice.found())
	{
		if( m_cdrom )
		{
			cdrom_close(m_cdrom);
			m_cdrom = (cdrom_file *)nullptr;
		}
	}
}

void akiko_device::nvram_write(uint32_t data)
{
	m_i2c_scl_out = BIT(data, 31);
	m_i2c_sda_out = BIT(data, 30);
	m_i2c_scl_dir = BIT(data, 15);
	m_i2c_sda_dir = BIT(data, 14);

	m_scl_w(m_i2c_scl_out);
	m_sda_w(m_i2c_sda_out);
}

uint32_t akiko_device::nvram_read()
{
	uint32_t v = 0;

	if (m_i2c_scl_dir)
		v |= m_i2c_scl_out << 31;

	if (m_i2c_sda_dir)
		v |= m_i2c_sda_out << 30;
	else
		v |= m_sda_r() << 30;

	v |= m_i2c_scl_dir << 15;
	v |= m_i2c_sda_dir << 14;

	return v;
}

uint8_t akiko_device::mem_r8(offs_t offset)
{
	int shift = (offset & 1) ? 0 : 8;
	return m_mem_r(offset, 0xff << shift) >> shift;
}

void akiko_device::mem_w8(offs_t offset, uint8_t data)
{
	int shift = (offset & 1) ? 0 : 8;
	m_mem_w(offset, data << shift, 0xff << shift);
}


/*************************************
 *
 * Akiko Chunky to Planar converter
 *
 ************************************/

void akiko_device::c2p_write(uint32_t data)
{
	m_c2p_input_buffer[m_c2p_input_index] = data;
	m_c2p_input_index++;
	m_c2p_input_index &= 7;
	m_c2p_output_index = 0;
}

uint32_t akiko_device::c2p_read()
{
	uint32_t val;

	if ( m_c2p_output_index == 0 )
	{
		int i;

		for ( i = 0; i < 8; i++ )
			m_c2p_output_buffer[i] = 0;

		for (i = 0; i < 8 * 32; i++)
		{
			if (m_c2p_input_buffer[7 - (i >> 5)] & (1 << (i & 31)))
				m_c2p_output_buffer[i & 7] |= 1 << (i >> 3);
		}
	}
	m_c2p_input_index = 0;
	val = m_c2p_output_buffer[m_c2p_output_index];
	m_c2p_output_index++;
	m_c2p_output_index &= 7;
	return val;
}

static const char *const akiko_reg_names[] =
{
	/*0*/   "ID",
	/*1*/   "CDROM STATUS 1",
	/*2*/   "CDROM_STATUS 2",
	/*3*/   "???",
	/*4*/   "CDROM ADDRESS 1",
	/*5*/   "CDROM ADDRESS 2",
	/*6*/   "CDROM COMMAND 1",
	/*7*/   "CDROM COMMAND 2",
	/*8*/   "CDROM READMASK",
	/*9*/   "CDROM DMACONTROL",
	/*A*/   "???",
	/*B*/   "???",
	/*C*/   "NVRAM",
	/*D*/   "???",
	/*E*/   "C2P"
};

static const char* get_akiko_reg_name(int reg)
{
	if (reg < 0xf )
	{
		return akiko_reg_names[reg];
	}
	else
	{
		return "???";
	}
}

/*************************************
 *
 * Akiko CDROM Controller
 *
 ************************************/

void akiko_device::cdda_stop()
{
	if (m_cdda != nullptr)
	{
		m_cdda->stop_audio();
		m_frame_timer->reset();
	}
}

void akiko_device::cdda_play(uint32_t lba, uint32_t num_blocks)
{
	if (m_cdda != nullptr)
	{
		m_cdda->start_audio(lba, num_blocks);
		m_frame_timer->adjust( attotime::from_hz( 75 ) );
	}
}

void akiko_device::cdda_pause(int pause)
{
	if (m_cdda != nullptr)
	{
		if (m_cdda->audio_active() && m_cdda->audio_paused() != pause )
		{
			m_cdda->pause_audio(pause);

			if ( pause )
			{
				m_frame_timer->reset(  );
			}
			else
			{
				m_frame_timer->adjust( attotime::from_hz( 75 ) );
			}
		}
	}
}

uint8_t akiko_device::cdda_getstatus(uint32_t *lba)
{
	if ( lba ) *lba = 0;

	if (m_cdda != nullptr)
	{
		if (m_cdda->audio_active())
		{
			if ( lba ) *lba = m_cdda->get_audio_lba();

			if (m_cdda->audio_paused())
			{
				return 0x12;    /* audio paused */
			}
			else
			{
				return 0x11;    /* audio in progress */
			}
		}
		else if (m_cdda->audio_ended())
		{
			return 0x13;    /* audio ended */
		}
	}

	return 0x15;    /* no audio status */
}

void akiko_device::set_cd_status(uint32_t status)
{
	m_cdrom_status[0] |= status;

	if ( m_cdrom_status[0] & m_cdrom_status[1] )
	{
		LOGCD("Akiko CD IRQ\n");

		m_int_w(1);
	}
}

TIMER_CALLBACK_MEMBER(akiko_device::frame_proc)
{
	(void)param;

	if (m_cdda != nullptr)
	{
		uint8_t   s = cdda_getstatus(nullptr);

		if ( s == 0x11 )
		{
			set_cd_status(0x80000000); /* subcode ready */
		}

		m_frame_timer->adjust( attotime::from_hz( 75 ) );
	}
}

static uint32_t lba_from_triplet( uint8_t *triplet )
{
	uint32_t  r;

	r = bcd_2_dec(triplet[0]) * (60*75);
	r += bcd_2_dec(triplet[1]) * 75;
	r += bcd_2_dec(triplet[2]);

	return r;
}

TIMER_CALLBACK_MEMBER(akiko_device::dma_proc)
{
	uint8_t   buf[2352];
	int     index;

	if ( (m_cdrom_dmacontrol & 0x04000000) == 0 )
		return;

	if ( m_cdrom_readreqmask == 0 )
		return;

	index = (m_cdrom_lba_cur - m_cdrom_lba_start) & 0x0f;

	if ( m_cdrom_readreqmask & ( 1 << index ) )
	{
		uint32_t  track = cdrom_get_track( m_cdrom, m_cdrom_lba_cur );
		uint32_t  datasize;// = cdrom_get_toc(m_cdrom)->tracks[track].datasize;
		uint32_t  subsize = cdrom_get_toc( m_cdrom )->tracks[track].subsize;

		uint32_t  curmsf = lba_to_msf( m_cdrom_lba_cur );
		memset( buf, 0, 16 );

		buf[3] = m_cdrom_lba_cur - m_cdrom_lba_start;
		memset( &buf[4], 0xff, 8 );

		buf[12] = (curmsf >> 16) & 0xff;
		buf[13] = (curmsf >> 8) & 0xff;
		buf[14] = curmsf & 0xff;
		buf[15] = 0x01; /* mode1 */

		datasize = 2048;
		if ( !cdrom_read_data( m_cdrom, m_cdrom_lba_cur, &buf[16], CD_TRACK_MODE1 ) )
		{
			LOGWARN( "AKIKO: Read error trying to read sector %08x!\n", m_cdrom_lba_cur );
			return;
		}

		if ( subsize )
		{
			if ( !cdrom_read_subcode( m_cdrom, m_cdrom_lba_cur, &buf[16+datasize] ) )
			{
				LOGWARN( "AKIKO: Read error trying to read subcode for sector %08x!\n", m_cdrom_lba_cur );
				return;
			}
		}

		LOGCD( "DMA: sector %d - address %08x\n", m_cdrom_lba_cur, m_cdrom_address[0] + (index*4096) );

		// write sector data to host memory
		for (int i = 0; i < 2352; i++)
			mem_w8(m_cdrom_address[0] + (index*4096) + i, buf[i]);

		m_cdrom_readmask |= ( 1 << index );
		m_cdrom_readreqmask &= ~( 1 << index );
		m_cdrom_lba_cur++;
	}

	if ( m_cdrom_readreqmask == 0 )
		set_cd_status(0x04000000);
	else
		m_dma_timer->adjust( attotime::from_usec( CD_SECTOR_TIME / m_cdrom_speed ) );
}

void akiko_device::start_dma()
{
	if ( m_cdrom_readreqmask == 0 )
		return;

	if ( m_cdrom_lba_start > m_cdrom_lba_end )
		return;

	if ( m_cdrom_speed == 0 )
		return;

	m_cdrom_lba_cur = m_cdrom_lba_start;

	m_dma_timer->adjust( attotime::from_usec( CD_SECTOR_TIME / m_cdrom_speed ) );
}

void akiko_device::setup_response( int len, uint8_t *r1 )
{
	int     resp_addr = m_cdrom_address[1];
	uint8_t   resp_csum = 0xff;
	uint8_t   resp_buffer[32];
	int     i;

	memset( resp_buffer, 0, sizeof( resp_buffer ) );

	for( i = 0; i < len; i++ )
	{
		resp_buffer[i] = r1[i];
		resp_csum -= resp_buffer[i];
	}

	resp_buffer[len++] = resp_csum;

	for( i = 0; i < len; i++ )
	{
		offs_t addr = resp_addr + ((m_cdrom_cmd_resp + i) & 0xff);
		mem_w8(addr, resp_buffer[i]);
	}

	m_cdrom_cmd_resp = (m_cdrom_cmd_resp+len) & 0xff;

	set_cd_status(0x10000000); /* new data available */
}

TIMER_CALLBACK_MEMBER( akiko_device::cd_delayed_cmd )
{
	uint8_t   resp[32];
	uint8_t   cddastatus;

	if ( m_cdrom_status[0] & 0x10000000 )
		return;

	cddastatus = cdda_getstatus(nullptr);

	if ( cddastatus == 0x11 || cddastatus == 0x12 )
		return;

	memset( resp, 0, sizeof( resp ) );
	resp[0] = param;

	param &= 0x0f;

	if ( param == 0x05 )
	{
		LOGCD( "AKIKO: Completing Command %d\n", param );

		resp[0] = 0x06;

		if ( m_cdrom == nullptr || m_cdrom_numtracks == 0 )
		{
			resp[1] = 0x80;
			setup_response( 15, resp );
		}
		else
		{
			resp[1] = 0x00;
			memcpy( &resp[2], &m_cdrom_toc[13*m_cdrom_track_index], 13 );

			m_cdrom_track_index = ( m_cdrom_track_index + 1 ) % m_cdrom_numtracks;

			setup_response( 15, resp );
		}
	}
}

void akiko_device::update_cdrom()
{
	uint8_t   resp[32], cmdbuf[32];

	if ( m_cdrom_status[0] & 0x10000000 )
		return;

	while ( m_cdrom_cmd_start != m_cdrom_cmd_end )
	{
		uint32_t  cmd_addr = m_cdrom_address[1] + 0x200 + m_cdrom_cmd_start;
		uint8_t cmd = mem_r8(cmd_addr);

		memset( resp, 0, sizeof( resp ) );
		resp[0] = cmd;

		cmd &= 0x0f;

		LOGCD( "CDROM command: %02X\n", cmd );

		if ( cmd == 0x02 ) /* pause audio */
		{
			resp[1] = 0x00;

			if ( cdda_getstatus(nullptr) == 0x11 )
				resp[1] = 0x08;

			cdda_pause(1);

			m_cdrom_cmd_start = (m_cdrom_cmd_start + 2) & 0xff;

			setup_response( 2, resp );
		}
		else if ( cmd == 0x03 ) /* unpause audio (and check audiocd playing status) */
		{
			resp[1] = 0x00;

			if ( cdda_getstatus(nullptr) == 0x11 )
				resp[1] = 0x08;

			cdda_pause(0);

			m_cdrom_cmd_start = (m_cdrom_cmd_start + 2) & 0xff;

			setup_response( 2, resp );
		}
		else if ( cmd == 0x04 ) /* seek/read/play cd multi command */
		{
			int i;
			uint32_t  startpos, endpos;

			for( i = 0; i < 13; i++ )
			{
				cmdbuf[i] = mem_r8(cmd_addr);
				cmd_addr &= 0xffffff00;
				cmd_addr += ( m_cdrom_cmd_start + i + 1 ) & 0xff;
			}

			m_cdrom_cmd_start = (m_cdrom_cmd_start + 13) & 0xff;

			if ( m_cdrom == nullptr || m_cdrom_numtracks == 0 )
			{
				resp[1] = 0x80;
				setup_response( 2, resp );
			}
			else
			{
				startpos = lba_from_triplet( &cmdbuf[1] );
				endpos = lba_from_triplet( &cmdbuf[4] );

				cdda_stop();

				resp[1] = 0x00;

				if ( cmdbuf[7] == 0x80 )
				{
					m_cdrom_speed = (cmdbuf[8] & 0x40) ? 2 : 1;
					LOGCD("AKIKO CD: Data read - start lba: %08x - end lba: %08x - divider speed: %d\n", startpos, endpos, m_cdrom_speed );
					m_cdrom_lba_start = startpos;
					m_cdrom_lba_end = endpos;

					resp[1] = 0x02;
				}
				else if ( cmdbuf[10] & 0x04 )
				{
					LOGCD("AKIKO CD: Audio Play - start lba: %08x - end lba: %08x\n", startpos, endpos );
					cdda_play(startpos, endpos - startpos);
					resp[1] = 0x08;
				}
				else
				{
					LOGCD("AKIKO CD: Seek - start lba: %08x - end lba: %08x\n", startpos, endpos );
					m_cdrom_track_index = 0;

					for( i = 0; i < cdrom_get_last_track(m_cdrom); i++ )
					{
						if ( startpos <= cdrom_get_track_start( m_cdrom, i ) )
						{
							/* reset to 0 */
							m_cdrom_track_index = i + 2;
							m_cdrom_track_index %= m_cdrom_numtracks;
							break;
						}
					}
				}

				setup_response( 2, resp );
			}
		}
		else if ( cmd == 0x05 ) /* read toc */
		{
			m_cdrom_cmd_start = (m_cdrom_cmd_start + 3) & 0xff;

			machine().scheduler().timer_set( attotime::from_msec(1), timer_expired_delegate(FUNC(akiko_device::cd_delayed_cmd ), this), resp[0]);

			break;
		}
		else if ( cmd == 0x06 ) /* read subq */
		{
			uint32_t  lba;

			resp[1] = 0x00;

			(void)cdda_getstatus(&lba);

			if ( lba > 0 )
			{
				uint32_t  disk_pos;
				uint32_t  track_pos;
				uint32_t  track;
				int     addrctrl;

				track = cdrom_get_track(m_cdrom, lba);
				addrctrl = cdrom_get_adr_control(m_cdrom, track);

				resp[2] = 0x00;
				resp[3] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
				resp[4] = dec_2_bcd(track+1);
				resp[5] = 0; /* index */

				disk_pos = lba_to_msf(lba);
				track_pos = lba_to_msf(lba - cdrom_get_track_start(m_cdrom, track));

				/* track position */
				resp[6] = (track_pos >> 16) & 0xff;
				resp[7] = (track_pos >> 8) & 0xff;
				resp[8] = track_pos & 0xff;

				/* disk position */
				resp[9] = (disk_pos >> 24) & 0xff;
				resp[10] = (disk_pos >> 16) & 0xff;
				resp[11] = (disk_pos >> 8) & 0xff;
				resp[12] = disk_pos & 0xff;
			}
			else
			{
				resp[1] = 0x80;
			}
			
			// needed by cdtv:defcrown (would otherwise hardlock emulation)
			m_cdrom_cmd_start = (m_cdrom_cmd_start + 2) & 0xff;

			setup_response( 15, resp );
		}
		else if ( cmd == 0x07 ) /* check door status */
		{
			resp[1] = 0x01;

			m_cdrom_cmd_start = (m_cdrom_cmd_start + 2) & 0xff;

			if ( m_cdrom == nullptr || m_cdrom_numtracks == 0 )
				resp[1] = 0x80;

			setup_response( 20, resp );
			break;
		}
		else
		{
			break;
		}
	}
}

uint32_t akiko_device::read(offs_t offset)
{
	uint32_t      retval;

	if ( offset < (0x30/4) )
		LOGREGS("Reading AKIKO reg %0x [%s] at %s\n", offset, get_akiko_reg_name(offset), machine().describe_context());

	switch( offset )
	{
		case 0x00/4:    /* ID */
			if ( m_cdrom != nullptr ) m_cdda->set_cdrom(m_cdrom);
			return 0x0000cafe;

		case 0x04/4:    /* CDROM STATUS 1 */
			return m_cdrom_status[0];

		case 0x08/4:    /* CDROM STATUS 2 */
			return m_cdrom_status[1];

		case 0x10/4:    /* CDROM ADDRESS 1 */
			return m_cdrom_address[0];

		case 0x14/4:    /* CDROM ADDRESS 2 */
			return m_cdrom_address[1];

		case 0x18/4:    /* CDROM COMMAND 1 */
			update_cdrom();
			retval = m_cdrom_cmd_start;
			retval <<= 8;
			retval |= m_cdrom_cmd_resp;
			retval <<= 8;
			return retval;

		case 0x1C/4:    /* CDROM COMMAND 2 */
			update_cdrom();
			retval = m_cdrom_cmd_end;
			retval <<= 16;
			return retval;

		case 0x20/4:    /* CDROM DMA SECTOR READ MASK */
			retval = m_cdrom_readmask << 16;
			return retval;

		case 0x24/4:    /* CDROM DMA ENABLE? */
			retval = m_cdrom_dmacontrol;
			return retval;

		case 0x30/4:    /* NVRAM */
			return nvram_read();

		case 0x38/4:    /* C2P */
			return c2p_read();

		default:
			break;
	}

	return 0;
}

void akiko_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if ( offset < (0x30/4) )
		LOGREGS("Writing AKIKO reg %0x [%s] with %08x at %s\n", offset, get_akiko_reg_name(offset), data, machine().describe_context());

	switch( offset )
	{
		case 0x04/4:    /* CDROM STATUS 1 */
			m_cdrom_status[0] = data;
			break;

		case 0x08/4:    /* CDROM STATUS 2 */
			m_cdrom_status[1] = data;
			m_cdrom_status[0] &= data;
			break;

		case 0x10/4:    /* CDROM ADDRESS 1 */
			m_cdrom_address[0] = data;
			break;

		case 0x14/4:    /* CDROM ADDRESS 2 */
			m_cdrom_address[1] = data;
			break;

		case 0x18/4:    /* CDROM COMMAND 1 */
			if ( ACCESSING_BITS_16_23 )
				m_cdrom_cmd_start = ( data >> 16 ) & 0xff;

			if ( ACCESSING_BITS_8_15 )
				m_cdrom_cmd_resp = ( data >> 8 ) & 0xff;

			update_cdrom();
			break;

		case 0x1C/4:    /* CDROM COMMAND 2 */
			if ( ACCESSING_BITS_16_23 )
				m_cdrom_cmd_end = ( data >> 16 ) & 0xff;

			update_cdrom();
			break;

		case 0x20/4:    /* CDROM DMA SECTOR READ REQUEST WRITE */
			LOGCD( "Read Req mask W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ACCESSING_BITS_16_31 )
			{
				m_cdrom_readreqmask = (data >> 16);
				m_cdrom_readmask = 0;
			}
			break;

		case 0x24/4:    /* CDROM DMA ENABLE? */
			LOGCD( "DMA enable W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ( m_cdrom_dmacontrol ^ data ) & 0x04000000 )
			{
				if ( data & 0x04000000 )
					start_dma();
			}
			m_cdrom_dmacontrol = data;
			break;

		case 0x30/4:
			nvram_write(data);
			break;

		case 0x38/4:
			c2p_write(data);
			break;

		default:
			break;
	}
}
