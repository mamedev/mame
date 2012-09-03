#include "emu.h"
#include "cdrom.h"
#include "coreutil.h"
#include "sound/cdda.h"
#include "machine/i2cmem.h"
#include "imagedev/chd_cd.h"
#include "includes/cd32.h"


/*********************************************************************************

    Akiko custom chip emulation

The Akiko chip has:
- built in 1KB NVRAM
- chunky to planar converter
- custom CDROM controller

TODO: Add CDDA support

*********************************************************************************/

#define LOG_AKIKO		0
#define LOG_AKIKO_CD	0

#define CD_SECTOR_TIME		(1000/((150*1024)/2048))	/* 1X CDROM sector time in msec (300KBps) */


class akiko_state
{
public:
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	void set_machine(running_machine &machine) { m_machine = &machine; }

	address_space *m_space;

	/* chunky to planar converter */
	UINT32	m_c2p_input_buffer[8];
	UINT32	m_c2p_output_buffer[8];
	UINT32	m_c2p_input_index;
	UINT32	m_c2p_output_index;

	/* i2c bus */
	int		m_i2c_scl_out;
	int		m_i2c_scl_dir;
	int		m_i2c_sda_out;
	int		m_i2c_sda_dir;

	/* cdrom */
	UINT32	m_cdrom_status[2];
	UINT32	m_cdrom_address[2];
	UINT32	m_cdrom_track_index;
	UINT32	m_cdrom_lba_start;
	UINT32	m_cdrom_lba_end;
	UINT32	m_cdrom_lba_cur;
	UINT16	m_cdrom_readmask;
	UINT16	m_cdrom_readreqmask;
	UINT32	m_cdrom_dmacontrol;
	UINT32	m_cdrom_numtracks;
	UINT8	m_cdrom_speed;
	UINT8	m_cdrom_cmd_start;
	UINT8	m_cdrom_cmd_end;
	UINT8	m_cdrom_cmd_resp;
	cdrom_file *m_cdrom;
	UINT8 *	m_cdrom_toc;
	emu_timer *m_dma_timer;
	emu_timer *m_frame_timer;
	device_t *m_i2cmem;

	int m_cdrom_is_device;

private:
	running_machine *m_machine;
};

static TIMER_CALLBACK(akiko_dma_proc);
static TIMER_CALLBACK(akiko_frame_proc);

INLINE akiko_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == AKIKO);

	return (akiko_state *)downcast<akiko_device *>(device)->token();
}

static DEVICE_STOP( akiko )
{
	akiko_state *state = get_safe_token(device);
	if (!state->m_cdrom_is_device)
	{
		if( state->m_cdrom )
		{
			cdrom_close(state->m_cdrom);
			state->m_cdrom = (cdrom_file *)NULL;
		}
	}
}

static DEVICE_RESET( akiko )
{
	running_machine &machine = device->machine();
	akiko_state *state = get_safe_token(device);

	cdrom_image_device *cddevice = machine.device<cdrom_image_device>("cdrom");
	if (cddevice!=NULL)
	{
		// MESS case
		state->m_cdrom = cddevice->get_cdrom_file();
		state->m_cdrom_is_device = 1;
	}
	else
	{
		// MAME case
		state->m_cdrom = cdrom_open(get_disk_handle(machine, ":cdrom"));
		state->m_cdrom_is_device = 0;
	}


	/* create the TOC table */
	if ( state->m_cdrom != NULL && cdrom_get_last_track(state->m_cdrom) )
	{
		UINT8 *p;
		int		i, addrctrl = cdrom_get_adr_control( state->m_cdrom, 0 );
		UINT32	discend;

		discend = cdrom_get_track_start(state->m_cdrom,cdrom_get_last_track(state->m_cdrom)-1);
		discend += cdrom_get_toc(state->m_cdrom)->tracks[cdrom_get_last_track(state->m_cdrom)-1].frames;
		discend = lba_to_msf(discend);

		state->m_cdrom_numtracks = cdrom_get_last_track(state->m_cdrom)+3;

		state->m_cdrom_toc = auto_alloc_array(machine, UINT8, 13*state->m_cdrom_numtracks);
		memset( state->m_cdrom_toc, 0, 13*state->m_cdrom_numtracks);

		p = state->m_cdrom_toc;
		p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
		p[3] = 0xa0; /* first track */
		p[8] = 1;
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa1; /* last track */
		p[8] = cdrom_get_last_track(state->m_cdrom);
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa2; /* disc end */
		p[8] = (discend >> 16 ) & 0xff;
		p[9] = (discend >> 8 ) & 0xff;
		p[10] = discend & 0xff;
		p += 13;

		for( i = 0; i < cdrom_get_last_track(state->m_cdrom); i++ )
		{
			UINT32	trackpos = cdrom_get_track_start(state->m_cdrom,i);

			trackpos = lba_to_msf(trackpos);
			addrctrl = cdrom_get_adr_control( state->m_cdrom, i );

			p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
			p[3] = dec_2_bcd( i+1 );
			p[8] = (trackpos >> 16 ) & 0xff;
			p[9] = (trackpos >> 8 ) & 0xff;
			p[10] = trackpos & 0xff;

			p += 13;
		}
	}

}

static DEVICE_START( akiko )
{
	running_machine &machine = device->machine();
	akiko_state *state = get_safe_token(device);

	state->set_machine(machine);
	state->m_space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->m_c2p_input_index = 0;
	state->m_c2p_output_index = 0;

	state->m_i2c_scl_out = 0;
	state->m_i2c_scl_dir = 0;
	state->m_i2c_sda_out = 0;
	state->m_i2c_sda_dir = 0;

	state->m_cdrom_status[0] = state->m_cdrom_status[1] = 0;
	state->m_cdrom_address[0] = state->m_cdrom_address[1] = 0;
	state->m_cdrom_track_index = 0;
	state->m_cdrom_lba_start = 0;
	state->m_cdrom_lba_end = 0;
	state->m_cdrom_lba_cur = 0;
	state->m_cdrom_readmask = 0;
	state->m_cdrom_readreqmask = 0;
	state->m_cdrom_dmacontrol = 0;
	state->m_cdrom_numtracks = 0;
	state->m_cdrom_speed = 0;
	state->m_cdrom_cmd_start = 0;
	state->m_cdrom_cmd_end = 0;
	state->m_cdrom_cmd_resp = 0;

	state->m_cdrom_toc = NULL;
	state->m_dma_timer = machine.scheduler().timer_alloc(FUNC(akiko_dma_proc), state);
	state->m_frame_timer = machine.scheduler().timer_alloc(FUNC(akiko_frame_proc), state);
	state->m_i2cmem = machine.device("i2cmem");


}

static void akiko_nvram_write(akiko_state *state, UINT32 data)
{
	state->m_i2c_scl_out = BIT(data,31);
	state->m_i2c_sda_out = BIT(data,30);
	state->m_i2c_scl_dir = BIT(data,15);
	state->m_i2c_sda_dir = BIT(data,14);

	i2cmem_scl_write( state->m_i2cmem, state->m_i2c_scl_out );
	i2cmem_sda_write( state->m_i2cmem, state->m_i2c_sda_out );
}

static UINT32 akiko_nvram_read(akiko_state *state)
{
	UINT32	v = 0;

	if ( state->m_i2c_scl_dir )
	{
		v |= state->m_i2c_scl_out << 31;
	}
	else
	{
		v |= 0 << 31;
	}

	if ( state->m_i2c_sda_dir )
	{
		v |= state->m_i2c_sda_out << 30;
	}
	else
	{
		v |= i2cmem_sda_read( state->m_i2cmem ) << 30;
	}

	v |= state->m_i2c_scl_dir << 15;
	v |= state->m_i2c_sda_dir << 14;

	return v;
}

/*************************************
 *
 * Akiko Chunky to Planar converter
 *
 ************************************/

static void akiko_c2p_write(akiko_state *state, UINT32 data)
{
	state->m_c2p_input_buffer[state->m_c2p_input_index] = data;
	state->m_c2p_input_index++;
	state->m_c2p_input_index &= 7;
	state->m_c2p_output_index = 0;
}

static UINT32 akiko_c2p_read(akiko_state *state)
{
	UINT32 val;

	if ( state->m_c2p_output_index == 0 )
	{
		int i;

		for ( i = 0; i < 8; i++ )
			state->m_c2p_output_buffer[i] = 0;

		for (i = 0; i < 8 * 32; i++)
		{
			if (state->m_c2p_input_buffer[7 - (i >> 5)] & (1 << (i & 31)))
				state->m_c2p_output_buffer[i & 7] |= 1 << (i >> 3);
		}
	}
	state->m_c2p_input_index = 0;
	val = state->m_c2p_output_buffer[state->m_c2p_output_index];
	state->m_c2p_output_index++;
	state->m_c2p_output_index &= 7;
	return val;
}

static const char *const akiko_reg_names[] =
{
	/*0*/	"ID",
	/*1*/	"CDROM STATUS 1",
	/*2*/	"CDROM_STATUS 2",
	/*3*/	"???",
	/*4*/	"CDROM ADDRESS 1",
	/*5*/	"CDROM ADDRESS 2",
	/*6*/	"CDROM COMMAND 1",
	/*7*/	"CDROM COMMAND 2",
	/*8*/	"CDROM READMASK",
	/*9*/	"CDROM DMACONTROL",
	/*A*/	"???",
	/*B*/	"???",
	/*C*/	"NVRAM",
	/*D*/	"???",
	/*E*/	"C2P"
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

static void akiko_cdda_stop(akiko_state *state)
{
	device_t *cdda = cdda_from_cdrom(state->machine(), state->m_cdrom);

	if (cdda != NULL)
	{
		cdda_stop_audio(cdda);
		state->m_frame_timer->reset(  );
	}
}

static void akiko_cdda_play(akiko_state *state, UINT32 lba, UINT32 num_blocks)
{
	device_t *cdda = cdda_from_cdrom(state->machine(), state->m_cdrom);
	if (cdda != NULL)
	{
		cdda_start_audio(cdda, lba, num_blocks);
		state->m_frame_timer->adjust( attotime::from_hz( 75 ) );
	}
}

static void akiko_cdda_pause(akiko_state *state, int pause)
{
	device_t *cdda = cdda_from_cdrom(state->machine(), state->m_cdrom);
	if (cdda != NULL)
	{
		if (cdda_audio_active(cdda) && cdda_audio_paused(cdda) != pause )
		{
			cdda_pause_audio(cdda, pause);

			if ( pause )
			{
				state->m_frame_timer->reset(  );
			}
			else
			{
				state->m_frame_timer->adjust( attotime::from_hz( 75 ) );
			}
		}
	}
}

static UINT8 akiko_cdda_getstatus(akiko_state *state, UINT32 *lba)
{
	device_t *cdda = cdda_from_cdrom(state->machine(), state->m_cdrom);

	if ( lba ) *lba = 0;

	if (cdda != NULL)
	{
		if (cdda_audio_active(cdda))
		{
			if ( lba ) *lba = cdda_get_audio_lba(cdda);

			if (cdda_audio_paused(cdda))
			{
				return 0x12;	/* audio paused */
			}
			else
			{
				return 0x11;	/* audio in progress */
			}
		}
		else if (cdda_audio_ended(cdda))
		{
			return 0x13;	/* audio ended */
		}
	}

	return 0x15;	/* no audio status */
}

static void akiko_set_cd_status(akiko_state *state, UINT32 status)
{
	state->m_cdrom_status[0] |= status;

	if ( state->m_cdrom_status[0] & state->m_cdrom_status[1] )
	{
		if (LOG_AKIKO_CD) logerror( "Akiko CD IRQ\n" );
		amiga_custom_w(state->m_space, REG_INTREQ, 0x8000 | INTENA_PORTS, 0xffff);
	}
}

static TIMER_CALLBACK(akiko_frame_proc)
{
	akiko_state *state = (akiko_state *)ptr;
	device_t *cdda = cdda_from_cdrom(machine, state->m_cdrom);

	(void)param;

	if (cdda != NULL)
	{
		UINT8	s = akiko_cdda_getstatus(state, NULL);

		if ( s == 0x11 )
		{
			akiko_set_cd_status(state, 0x80000000);	/* subcode ready */
		}

		state->m_frame_timer->adjust( attotime::from_hz( 75 ) );
	}
}

static UINT32 lba_from_triplet( UINT8 *triplet )
{
	UINT32	r;

	r = bcd_2_dec(triplet[0]) * (60*75);
	r += bcd_2_dec(triplet[1]) * 75;
	r += bcd_2_dec(triplet[2]);

	return r;
}

static TIMER_CALLBACK(akiko_dma_proc)
{
	akiko_state *state = (akiko_state *)ptr;
	UINT8	buf[2352];
	int		index;

	if ( (state->m_cdrom_dmacontrol & 0x04000000) == 0 )
		return;

	if ( state->m_cdrom_readreqmask == 0 )
		return;

	index = (state->m_cdrom_lba_cur - state->m_cdrom_lba_start) & 0x0f;

	if ( state->m_cdrom_readreqmask & ( 1 << index ) )
	{
		amiga_state *amiga = machine.driver_data<amiga_state>();
		UINT32	track = cdrom_get_track( state->m_cdrom, state->m_cdrom_lba_cur );
		UINT32	datasize = cdrom_get_toc( state->m_cdrom )->tracks[track].datasize;
		UINT32	subsize = cdrom_get_toc( state->m_cdrom )->tracks[track].subsize;
		int		i;

		UINT32	curmsf = lba_to_msf( state->m_cdrom_lba_cur );
		memset( buf, 0, 16 );

		buf[3] = state->m_cdrom_lba_cur - state->m_cdrom_lba_start;
		memset( &buf[4], 0xff, 8 );

		buf[12] = (curmsf >> 16) & 0xff;
		buf[13] = (curmsf >> 8) & 0xff;
		buf[14] = curmsf & 0xff;
		buf[15] = 0x01; /* mode1 */

		datasize = 2048;
		if ( !cdrom_read_data( state->m_cdrom, state->m_cdrom_lba_cur, &buf[16], CD_TRACK_MODE1 ) )
		{
			logerror( "AKIKO: Read error trying to read sector %08x!\n", state->m_cdrom_lba_cur );
			return;
		}

		if ( subsize )
		{
			if ( !cdrom_read_subcode( state->m_cdrom, state->m_cdrom_lba_cur, &buf[16+datasize] ) )
			{
				logerror( "AKIKO: Read error trying to read subcode for sector %08x!\n", state->m_cdrom_lba_cur );
				return;
			}
		}

		if (LOG_AKIKO_CD) logerror( "DMA: sector %d - address %08x\n", state->m_cdrom_lba_cur, state->m_cdrom_address[0] + (index*4096) );

		for( i = 0; i < 2352; i += 2 )
		{
			UINT16	data;

			data = buf[i];
			data <<= 8;
			data |= buf[i+1];

			(*amiga->m_chip_ram_w)( amiga, state->m_cdrom_address[0] + (index*4096) + i, data );
		}

		state->m_cdrom_readmask |= ( 1 << index );
		state->m_cdrom_readreqmask &= ~( 1 << index );
		state->m_cdrom_lba_cur++;
	}

	if ( state->m_cdrom_readreqmask == 0 )
		akiko_set_cd_status(state, 0x04000000);
	else
		state->m_dma_timer->adjust( attotime::from_usec( CD_SECTOR_TIME / state->m_cdrom_speed ) );
}

static void akiko_start_dma(akiko_state *state)
{
	if ( state->m_cdrom_readreqmask == 0 )
		return;

	if ( state->m_cdrom_lba_start > state->m_cdrom_lba_end )
		return;

	if ( state->m_cdrom_speed == 0 )
		return;

	state->m_cdrom_lba_cur = state->m_cdrom_lba_start;

	state->m_dma_timer->adjust( attotime::from_usec( CD_SECTOR_TIME / state->m_cdrom_speed ) );
}

static void akiko_setup_response( akiko_state *state, int len, UINT8 *r1 )
{
	int		resp_addr = state->m_cdrom_address[1];
	UINT8	resp_csum = 0xff;
	UINT8	resp_buffer[32];
	int		i;

	memset( resp_buffer, 0, sizeof( resp_buffer ) );

	for( i = 0; i < len; i++ )
	{
		resp_buffer[i] = r1[i];
		resp_csum -= resp_buffer[i];
	}

	resp_buffer[len++] = resp_csum;

	for( i = 0; i < len; i++ )
	{
		state->m_space->write_byte( resp_addr + ((state->m_cdrom_cmd_resp + i) & 0xff), resp_buffer[i] );
	}

	state->m_cdrom_cmd_resp = (state->m_cdrom_cmd_resp+len) & 0xff;

	akiko_set_cd_status(state, 0x10000000); /* new data available */
}

static TIMER_CALLBACK( akiko_cd_delayed_cmd )
{
	akiko_state *state = (akiko_state *)ptr;
	UINT8	resp[32];
	UINT8	cddastatus;

	if ( state->m_cdrom_status[0] & 0x10000000 )
		return;

	cddastatus = akiko_cdda_getstatus(state, NULL);

	if ( cddastatus == 0x11 || cddastatus == 0x12 )
		return;

	memset( resp, 0, sizeof( resp ) );
	resp[0] = param;

	param &= 0x0f;

	if ( param == 0x05 )
	{
		if (LOG_AKIKO_CD) logerror( "AKIKO: Completing Command %d\n", param );

		resp[0] = 0x06;

		if ( state->m_cdrom == NULL || state->m_cdrom_numtracks == 0 )
		{
			resp[1] = 0x80;
			akiko_setup_response( state, 15, resp );
		}
		else
		{
			resp[1] = 0x00;
			memcpy( &resp[2], &state->m_cdrom_toc[13*state->m_cdrom_track_index], 13 );

			state->m_cdrom_track_index = ( state->m_cdrom_track_index + 1 ) % state->m_cdrom_numtracks;

			akiko_setup_response( state, 15, resp );
		}
	}
}

static void akiko_update_cdrom(akiko_state *state)
{
	UINT8	resp[32], cmdbuf[32];

	if ( state->m_cdrom_status[0] & 0x10000000 )
		return;

	while ( state->m_cdrom_cmd_start != state->m_cdrom_cmd_end )
	{
		UINT32	cmd_addr = state->m_cdrom_address[1] + 0x200 + state->m_cdrom_cmd_start;
		int		cmd = state->m_space->read_byte( cmd_addr );

		memset( resp, 0, sizeof( resp ) );
		resp[0] = cmd;

		cmd &= 0x0f;

		if (LOG_AKIKO_CD) logerror( "CDROM command: %02X\n", cmd );

		if ( cmd == 0x02 ) /* pause audio */
		{
			resp[1] = 0x00;

			if ( akiko_cdda_getstatus(state, NULL) == 0x11 )
				resp[1] = 0x08;

			akiko_cdda_pause(state, 1);

			state->m_cdrom_cmd_start = (state->m_cdrom_cmd_start+2) & 0xff;

			akiko_setup_response( state, 2, resp );
		}
		else if ( cmd == 0x03 ) /* unpause audio (and check audiocd playing status) */
		{
			resp[1] = 0x00;

			if ( akiko_cdda_getstatus(state, NULL) == 0x11 )
				resp[1] = 0x08;

			akiko_cdda_pause(state, 0);

			state->m_cdrom_cmd_start = (state->m_cdrom_cmd_start+2) & 0xff;

			akiko_setup_response( state, 2, resp );
		}
		else if ( cmd == 0x04 ) /* seek/read/play cd multi command */
		{
			int	i;
			UINT32	startpos, endpos;

			for( i = 0; i < 13; i++ )
			{
				cmdbuf[i] = state->m_space->read_byte( cmd_addr );
				cmd_addr &= 0xffffff00;
				cmd_addr += ( state->m_cdrom_cmd_start + i + 1 ) & 0xff;
			}

			state->m_cdrom_cmd_start = (state->m_cdrom_cmd_start+13) & 0xff;

			if ( state->m_cdrom == NULL || state->m_cdrom_numtracks == 0 )
			{
				resp[1] = 0x80;
				akiko_setup_response( state, 2, resp );
			}
			else
			{
				startpos = lba_from_triplet( &cmdbuf[1] );
				endpos = lba_from_triplet( &cmdbuf[4] );

				akiko_cdda_stop(state);

				resp[1] = 0x00;

				if ( cmdbuf[7] == 0x80 )
				{
					if (LOG_AKIKO_CD) logerror( "%s:AKIKO CD: Data read - start lba: %08x - end lba: %08x\n", state->machine().describe_context(), startpos, endpos );
					state->m_cdrom_speed = (cmdbuf[8] & 0x40) ? 2 : 1;
					state->m_cdrom_lba_start = startpos;
					state->m_cdrom_lba_end = endpos;

					resp[1] = 0x02;
				}
				else if ( cmdbuf[10] & 0x04 )
				{
					logerror( "AKIKO CD: Audio Play - start lba: %08x - end lba: %08x\n", startpos, endpos );
					akiko_cdda_play(state, startpos, endpos - startpos);
					resp[1] = 0x08;
				}
				else
				{
					if (LOG_AKIKO_CD) logerror( "AKIKO CD: Seek - start lba: %08x - end lba: %08x\n", startpos, endpos );
					state->m_cdrom_track_index = 0;

					for( i = 0; i < cdrom_get_last_track(state->m_cdrom); i++ )
					{
						if ( startpos <= cdrom_get_track_start( state->m_cdrom, i ) )
						{
							/* reset to 0 */
							state->m_cdrom_track_index = i + 2;
							state->m_cdrom_track_index %= state->m_cdrom_numtracks;
							break;
						}
					}
				}

				akiko_setup_response( state, 2, resp );
			}
		}
		else if ( cmd == 0x05 ) /* read toc */
		{
			state->m_cdrom_cmd_start = (state->m_cdrom_cmd_start+3) & 0xff;

			state->machine().scheduler().timer_set( attotime::from_msec(1), FUNC(akiko_cd_delayed_cmd ), resp[0], state);

			break;
		}
		else if ( cmd == 0x06 ) /* read subq */
		{
			UINT32	lba;

			resp[1] = 0x00;

			(void)akiko_cdda_getstatus(state, &lba);

			if ( lba > 0 )
			{
				UINT32	disk_pos;
				UINT32	track_pos;
				UINT32	track;
				int		addrctrl;

				track = cdrom_get_track(state->m_cdrom, lba);
				addrctrl = cdrom_get_adr_control(state->m_cdrom, track);

				resp[2] = 0x00;
				resp[3] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
				resp[4] = dec_2_bcd(track+1);
				resp[5] = 0; /* index */

				disk_pos = lba_to_msf(lba);
				track_pos = lba_to_msf(lba - cdrom_get_track_start(state->m_cdrom, track));

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

			akiko_setup_response( state, 15, resp );
		}
		else if ( cmd == 0x07 )	/* check door status */
		{
			resp[1] = 0x01;

			state->m_cdrom_cmd_start = (state->m_cdrom_cmd_start+2) & 0xff;

			if ( state->m_cdrom == NULL || state->m_cdrom_numtracks == 0 )
				resp[1] = 0x80;

			akiko_setup_response( state, 20, resp );
			break;
		}
		else
		{
			break;
		}
	}
}

READ32_DEVICE_HANDLER( amiga_akiko32_r )
{
	akiko_state *state = get_safe_token(device);
	address_space *space = state->m_space;
	UINT32		retval;

	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Reading AKIKO reg %0x [%s] at PC=%06x\n", offset, get_akiko_reg_name(offset), cpu_get_pc(&space->device()) );
	}

	switch( offset )
	{
		case 0x00/4:	/* ID */
			if ( state->m_cdrom != NULL ) cdda_set_cdrom(space->machine().device("cdda"), state->m_cdrom);
			return 0x0000cafe;

		case 0x04/4:	/* CDROM STATUS 1 */
			return state->m_cdrom_status[0];

		case 0x08/4:	/* CDROM STATUS 2 */
			return state->m_cdrom_status[1];

		case 0x10/4:	/* CDROM ADDRESS 1 */
			return state->m_cdrom_address[0];

		case 0x14/4:	/* CDROM ADDRESS 2 */
			return state->m_cdrom_address[1];

		case 0x18/4:	/* CDROM COMMAND 1 */
			akiko_update_cdrom(state);
			retval = state->m_cdrom_cmd_start;
			retval <<= 8;
			retval |= state->m_cdrom_cmd_resp;
			retval <<= 8;
			return retval;

		case 0x1C/4:	/* CDROM COMMAND 2 */
			akiko_update_cdrom(state);
			retval = state->m_cdrom_cmd_end;
			retval <<= 16;
			return retval;

		case 0x20/4:	/* CDROM DMA SECTOR READ MASK */
			retval = state->m_cdrom_readmask << 16;
			return retval;

		case 0x24/4:	/* CDROM DMA ENABLE? */
			retval = state->m_cdrom_dmacontrol;
			return retval;

		case 0x30/4:	/* NVRAM */
			return akiko_nvram_read(state);

		case 0x38/4:	/* C2P */
			return akiko_c2p_read(state);

		default:
			break;
	}

	return 0;
}

WRITE32_DEVICE_HANDLER( amiga_akiko32_w )
{
	akiko_state *state = get_safe_token(device);
	address_space *space = state->m_space;

	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Writing AKIKO reg %0x [%s] with %08x at PC=%06x\n", offset, get_akiko_reg_name(offset), data, cpu_get_pc(&space->device()) );
	}

	switch( offset )
	{
		case 0x04/4:	/* CDROM STATUS 1 */
			state->m_cdrom_status[0] = data;
			break;

		case 0x08/4:	/* CDROM STATUS 2 */
			state->m_cdrom_status[1] = data;
			state->m_cdrom_status[0] &= data;
			break;

		case 0x10/4:	/* CDROM ADDRESS 1 */
			state->m_cdrom_address[0] = data;
			break;

		case 0x14/4:	/* CDROM ADDRESS 2 */
			state->m_cdrom_address[1] = data;
			break;

		case 0x18/4:	/* CDROM COMMAND 1 */
			if ( ACCESSING_BITS_16_23 )
				state->m_cdrom_cmd_start = ( data >> 16 ) & 0xff;

			if ( ACCESSING_BITS_8_15 )
				state->m_cdrom_cmd_resp = ( data >> 8 ) & 0xff;

			akiko_update_cdrom(state);
			break;

		case 0x1C/4:	/* CDROM COMMAND 2 */
			if ( ACCESSING_BITS_16_23 )
				state->m_cdrom_cmd_end = ( data >> 16 ) & 0xff;

			akiko_update_cdrom(state);
			break;

		case 0x20/4:	/* CDROM DMA SECTOR READ REQUEST WRITE */
			if (LOG_AKIKO_CD) logerror( "Read Req mask W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ACCESSING_BITS_16_31 )
			{
				state->m_cdrom_readreqmask = (data >> 16);
				state->m_cdrom_readmask = 0;
			}
			break;

		case 0x24/4:	/* CDROM DMA ENABLE? */
			if (LOG_AKIKO_CD) logerror( "DMA enable W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ( state->m_cdrom_dmacontrol ^ data ) & 0x04000000 )
			{
				if ( data & 0x04000000 )
					akiko_start_dma(state);
			}
			state->m_cdrom_dmacontrol = data;
			break;

		case 0x30/4:
			akiko_nvram_write(state, data);
			break;

		case 0x38/4:
			akiko_c2p_write(state, data);
			break;

		default:
			break;
	}
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

DEVICE_GET_INFO(akiko)
{
 switch (state)
 {
  case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(akiko_state); break;

  case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(akiko); break;

  case DEVINFO_FCT_RESET: info->reset = DEVICE_RESET_NAME(akiko); break;

  case DEVINFO_FCT_STOP: info->stop = DEVICE_STOP_NAME(akiko); break;

  case DEVINFO_STR_NAME: strcpy(info->s, "Akiko"); break;
 }
}

const device_type AKIKO = &device_creator<akiko_device>;

akiko_device::akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AKIKO, "Akiko", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(akiko_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void akiko_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void akiko_device::device_start()
{
	DEVICE_START_NAME( akiko )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void akiko_device::device_reset()
{
	DEVICE_RESET_NAME( akiko )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void akiko_device::device_stop()
{
	DEVICE_STOP_NAME( akiko )(this);
}


