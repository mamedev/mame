// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

    Akiko

    ASIC used in the Amiga CD32. Commodore Part number 391563-01.

    - CD-ROM controller
    - Builtin 1KB NVRAM
    - Chunky to planar converter
    - 2x CIA chips

***************************************************************************/

#include "akiko.h"
#include "includes/amiga.h"
#include "imagedev/chd_cd.h"
#include "coreutil.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG_AKIKO       0
#define LOG_AKIKO_CD    0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type AKIKO = &device_creator<akiko_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( akiko )
MACHINE_CONFIG_END

machine_config_constructor akiko_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( akiko );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  akiko_device - constructor
//-------------------------------------------------

akiko_device::akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AKIKO, "CBM AKIKO", tag, owner, clock, "akiko", __FILE__),
	m_c2p_input_index(0),
	m_c2p_output_index(0),
	m_i2c_scl_out(0),
	m_i2c_scl_dir(0),
	m_i2c_sda_out(0),
	m_i2c_sda_dir(0),
	m_cdrom_track_index(0),
	m_cdrom_lba_start(0),
	m_cdrom_lba_end(0),
	m_cdrom_lba_cur(0),
	m_cdrom_readmask(0),
	m_cdrom_readreqmask(0),
	m_cdrom_dmacontrol(0),
	m_cdrom_numtracks(0),
	m_cdrom_speed(0),
	m_cdrom_cmd_start(0),
	m_cdrom_cmd_end(0),
	m_cdrom_cmd_resp(0),
	m_cdda(nullptr),
	m_cdrom(nullptr),
	m_cdrom_toc(nullptr),
	m_dma_timer(nullptr),
	m_frame_timer(nullptr),
	m_cdrom_is_device(0),
	m_scl_w(*this),
	m_sda_r(*this),
	m_sda_w(*this)
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
//  set_cputag - set cpu tag for cpu we working on
//-------------------------------------------------

void akiko_device::set_cputag(device_t &device, const char *tag)
{
	akiko_device &akiko = downcast<akiko_device &>(device);
	akiko.m_cputag = tag;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void akiko_device::device_start()
{
	// resolve callbacks
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

	device_t *cpu = machine().device(m_cputag);
	m_space = &cpu->memory().space(AS_PROGRAM);

	m_cdrom_toc = nullptr;
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(akiko_device::dma_proc), this));
	m_frame_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(akiko_device::frame_proc), this));
	m_cdda = machine().device<cdda_device>("cdda");
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void akiko_device::device_reset()
{
	cdrom_image_device *cddevice = machine().device<cdrom_image_device>("cdrom");

	if (cddevice != nullptr)
	{
		// MESS case
		m_cdrom = cddevice->get_cdrom_file();
		m_cdrom_is_device = 1;
	}
	else
	{
		// MAME case
		m_cdrom = cdrom_open(machine().rom_load().get_disk_handle(":cdrom"));
		m_cdrom_is_device = 0;
	}

	/* create the TOC table */
	if ( m_cdrom != nullptr && cdrom_get_last_track(m_cdrom) )
	{
		UINT8 *p;
		int     i, addrctrl = cdrom_get_adr_control( m_cdrom, 0 );
		UINT32  discend;

		discend = cdrom_get_track_start(m_cdrom,cdrom_get_last_track(m_cdrom)-1);
		discend += cdrom_get_toc(m_cdrom)->tracks[cdrom_get_last_track(m_cdrom)-1].frames;
		discend = lba_to_msf(discend);

		m_cdrom_numtracks = cdrom_get_last_track(m_cdrom)+3;

		m_cdrom_toc = std::make_unique<UINT8[]>(13*m_cdrom_numtracks);
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
			UINT32  trackpos = cdrom_get_track_start(m_cdrom,i);

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
	if (!m_cdrom_is_device)
	{
		if( m_cdrom )
		{
			cdrom_close(m_cdrom);
			m_cdrom = (cdrom_file *)nullptr;
		}
	}
}

void akiko_device::nvram_write(UINT32 data)
{
	m_i2c_scl_out = BIT(data, 31);
	m_i2c_sda_out = BIT(data, 30);
	m_i2c_scl_dir = BIT(data, 15);
	m_i2c_sda_dir = BIT(data, 14);

	m_scl_w(m_i2c_scl_out);
	m_sda_w(m_i2c_sda_out);
}

UINT32 akiko_device::nvram_read()
{
	UINT32 v = 0;

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

/*************************************
 *
 * Akiko Chunky to Planar converter
 *
 ************************************/

void akiko_device::c2p_write(UINT32 data)
{
	m_c2p_input_buffer[m_c2p_input_index] = data;
	m_c2p_input_index++;
	m_c2p_input_index &= 7;
	m_c2p_output_index = 0;
}

UINT32 akiko_device::c2p_read()
{
	UINT32 val;

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
		m_frame_timer->reset(  );
	}
}

void akiko_device::cdda_play(UINT32 lba, UINT32 num_blocks)
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

UINT8 akiko_device::cdda_getstatus(UINT32 *lba)
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

void akiko_device::set_cd_status(UINT32 status)
{
	amiga_state *amiga = machine().driver_data<amiga_state>();

	m_cdrom_status[0] |= status;

	if ( m_cdrom_status[0] & m_cdrom_status[1] )
	{
		if (LOG_AKIKO_CD)
			logerror("Akiko CD IRQ\n");

		amiga->custom_chip_w(REG_INTREQ, INTENA_SETCLR | INTENA_PORTS);
	}
}

TIMER_CALLBACK_MEMBER(akiko_device::frame_proc)
{
	(void)param;

	if (m_cdda != nullptr)
	{
		UINT8   s = cdda_getstatus(nullptr);

		if ( s == 0x11 )
		{
			set_cd_status(0x80000000); /* subcode ready */
		}

		m_frame_timer->adjust( attotime::from_hz( 75 ) );
	}
}

static UINT32 lba_from_triplet( UINT8 *triplet )
{
	UINT32  r;

	r = bcd_2_dec(triplet[0]) * (60*75);
	r += bcd_2_dec(triplet[1]) * 75;
	r += bcd_2_dec(triplet[2]);

	return r;
}

TIMER_CALLBACK_MEMBER(akiko_device::dma_proc)
{
	UINT8   buf[2352];
	int     index;

	if ( (m_cdrom_dmacontrol & 0x04000000) == 0 )
		return;

	if ( m_cdrom_readreqmask == 0 )
		return;

	index = (m_cdrom_lba_cur - m_cdrom_lba_start) & 0x0f;

	if ( m_cdrom_readreqmask & ( 1 << index ) )
	{
		amiga_state *amiga = machine().driver_data<amiga_state>();
		UINT32  track = cdrom_get_track( m_cdrom, m_cdrom_lba_cur );
		UINT32  datasize;// = cdrom_get_toc(m_cdrom)->tracks[track].datasize;
		UINT32  subsize = cdrom_get_toc( m_cdrom )->tracks[track].subsize;
		int     i;

		UINT32  curmsf = lba_to_msf( m_cdrom_lba_cur );
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
			logerror( "AKIKO: Read error trying to read sector %08x!\n", m_cdrom_lba_cur );
			return;
		}

		if ( subsize )
		{
			if ( !cdrom_read_subcode( m_cdrom, m_cdrom_lba_cur, &buf[16+datasize] ) )
			{
				logerror( "AKIKO: Read error trying to read subcode for sector %08x!\n", m_cdrom_lba_cur );
				return;
			}
		}

		if (LOG_AKIKO_CD) logerror( "DMA: sector %d - address %08x\n", m_cdrom_lba_cur, m_cdrom_address[0] + (index*4096) );

		for( i = 0; i < 2352; i += 2 )
		{
			UINT16  data;

			data = buf[i];
			data <<= 8;
			data |= buf[i+1];

			amiga->chip_ram_w(m_cdrom_address[0] + (index*4096) + i, data );
		}

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

void akiko_device::setup_response( int len, UINT8 *r1 )
{
	int     resp_addr = m_cdrom_address[1];
	UINT8   resp_csum = 0xff;
	UINT8   resp_buffer[32];
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
		m_space->write_byte( resp_addr + ((m_cdrom_cmd_resp + i) & 0xff), resp_buffer[i] );
	}

	m_cdrom_cmd_resp = (m_cdrom_cmd_resp+len) & 0xff;

	set_cd_status(0x10000000); /* new data available */
}

TIMER_CALLBACK_MEMBER( akiko_device::cd_delayed_cmd )
{
	UINT8   resp[32];
	UINT8   cddastatus;

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
		if (LOG_AKIKO_CD) logerror( "AKIKO: Completing Command %d\n", param );

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
	UINT8   resp[32], cmdbuf[32];

	if ( m_cdrom_status[0] & 0x10000000 )
		return;

	while ( m_cdrom_cmd_start != m_cdrom_cmd_end )
	{
		UINT32  cmd_addr = m_cdrom_address[1] + 0x200 + m_cdrom_cmd_start;
		int     cmd = m_space->read_byte( cmd_addr );

		memset( resp, 0, sizeof( resp ) );
		resp[0] = cmd;

		cmd &= 0x0f;

		if (LOG_AKIKO_CD) logerror( "CDROM command: %02X\n", cmd );

		if ( cmd == 0x02 ) /* pause audio */
		{
			resp[1] = 0x00;

			if ( cdda_getstatus(nullptr) == 0x11 )
				resp[1] = 0x08;

			cdda_pause(1);

			m_cdrom_cmd_start = (m_cdrom_cmd_start+2) & 0xff;

			setup_response( 2, resp );
		}
		else if ( cmd == 0x03 ) /* unpause audio (and check audiocd playing status) */
		{
			resp[1] = 0x00;

			if ( cdda_getstatus(nullptr) == 0x11 )
				resp[1] = 0x08;

			cdda_pause(0);

			m_cdrom_cmd_start = (m_cdrom_cmd_start+2) & 0xff;

			setup_response( 2, resp );
		}
		else if ( cmd == 0x04 ) /* seek/read/play cd multi command */
		{
			int i;
			UINT32  startpos, endpos;

			for( i = 0; i < 13; i++ )
			{
				cmdbuf[i] = m_space->read_byte( cmd_addr );
				cmd_addr &= 0xffffff00;
				cmd_addr += ( m_cdrom_cmd_start + i + 1 ) & 0xff;
			}

			m_cdrom_cmd_start = (m_cdrom_cmd_start+13) & 0xff;

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
					if (LOG_AKIKO_CD) logerror( "%s:AKIKO CD: Data read - start lba: %08x - end lba: %08x\n", machine().describe_context(), startpos, endpos );
					m_cdrom_speed = (cmdbuf[8] & 0x40) ? 2 : 1;
					m_cdrom_lba_start = startpos;
					m_cdrom_lba_end = endpos;

					resp[1] = 0x02;
				}
				else if ( cmdbuf[10] & 0x04 )
				{
					logerror( "AKIKO CD: Audio Play - start lba: %08x - end lba: %08x\n", startpos, endpos );
					cdda_play(startpos, endpos - startpos);
					resp[1] = 0x08;
				}
				else
				{
					if (LOG_AKIKO_CD) logerror( "AKIKO CD: Seek - start lba: %08x - end lba: %08x\n", startpos, endpos );
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
			m_cdrom_cmd_start = (m_cdrom_cmd_start+3) & 0xff;

			machine().scheduler().timer_set( attotime::from_msec(1), timer_expired_delegate(FUNC(akiko_device::cd_delayed_cmd ), this), resp[0]);

			break;
		}
		else if ( cmd == 0x06 ) /* read subq */
		{
			UINT32  lba;

			resp[1] = 0x00;

			(void)cdda_getstatus(&lba);

			if ( lba > 0 )
			{
				UINT32  disk_pos;
				UINT32  track_pos;
				UINT32  track;
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

			setup_response( 15, resp );
		}
		else if ( cmd == 0x07 ) /* check door status */
		{
			resp[1] = 0x01;

			m_cdrom_cmd_start = (m_cdrom_cmd_start+2) & 0xff;

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

READ32_MEMBER( akiko_device::read )
{
	UINT32      retval;

	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Reading AKIKO reg %0x [%s] at PC=%06x\n", offset, get_akiko_reg_name(offset), m_space->device().safe_pc() );
	}

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

WRITE32_MEMBER( akiko_device::write )
{
	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Writing AKIKO reg %0x [%s] with %08x at PC=%06x\n", offset, get_akiko_reg_name(offset), data, m_space->device().safe_pc() );
	}

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
			if (LOG_AKIKO_CD) logerror( "Read Req mask W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ACCESSING_BITS_16_31 )
			{
				m_cdrom_readreqmask = (data >> 16);
				m_cdrom_readmask = 0;
			}
			break;

		case 0x24/4:    /* CDROM DMA ENABLE? */
			if (LOG_AKIKO_CD) logerror( "DMA enable W: data %08x - mem mask %08x\n", data, mem_mask );
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
