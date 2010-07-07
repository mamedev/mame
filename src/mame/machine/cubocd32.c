#include "emu.h"
#include "includes/amiga.h"
#include "cdrom.h"
#include "coreutil.h"
#include "sound/cdda.h"
#include "machine/i2cmem.h"
#include "includes/cubocd32.h"



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


static struct akiko_def
{
	/* chunky to planar converter */
	UINT32	c2p_input_buffer[8];
	UINT32	c2p_output_buffer[8];
	UINT32	c2p_input_index;
	UINT32	c2p_output_index;

	/* i2c bus */
	int		i2c_scl_out;
	int		i2c_scl_dir;
	int		i2c_sda_out;
	int		i2c_sda_dir;

	/* cdrom */
	UINT32	cdrom_status[2];
	UINT32	cdrom_address[2];
	UINT32	cdrom_track_index;
	UINT32	cdrom_lba_start;
	UINT32	cdrom_lba_end;
	UINT32	cdrom_lba_cur;
	UINT16	cdrom_readmask;
	UINT16	cdrom_readreqmask;
	UINT32	cdrom_dmacontrol;
	UINT32	cdrom_numtracks;
	UINT8	cdrom_speed;
	UINT8	cdrom_cmd_start;
	UINT8	cdrom_cmd_end;
	UINT8	cdrom_cmd_resp;
	cdrom_file *cdrom;
	UINT8 *	cdrom_toc;
	emu_timer *dma_timer;
	emu_timer *frame_timer;
	running_device *i2cmem;
} akiko;

static TIMER_CALLBACK(akiko_dma_proc);
static TIMER_CALLBACK(akiko_frame_proc);

static void amiga_akiko_exit(running_machine& machine)
{
	if( akiko.cdrom ) {
		cdrom_close(akiko.cdrom);
		akiko.cdrom = (cdrom_file *)NULL;
	}
}

void amiga_akiko_init(running_machine* machine)
{
	akiko.c2p_input_index = 0;
	akiko.c2p_output_index = 0;

	akiko.i2c_scl_out = 0;
	akiko.i2c_scl_dir = 0;
	akiko.i2c_sda_out = 0;
	akiko.i2c_sda_dir = 0;

	akiko.cdrom_status[0] = akiko.cdrom_status[1] = 0;
	akiko.cdrom_address[0] = akiko.cdrom_address[1] = 0;
	akiko.cdrom_track_index = 0;
	akiko.cdrom_lba_start = 0;
	akiko.cdrom_lba_end = 0;
	akiko.cdrom_lba_cur = 0;
	akiko.cdrom_readmask = 0;
	akiko.cdrom_readreqmask = 0;
	akiko.cdrom_dmacontrol = 0;
	akiko.cdrom_numtracks = 0;
	akiko.cdrom_speed = 0;
	akiko.cdrom_cmd_start = 0;
	akiko.cdrom_cmd_end = 0;
	akiko.cdrom_cmd_resp = 0;
	akiko.cdrom = cdrom_open(get_disk_handle(machine, "cdrom"));
	akiko.cdrom_toc = NULL;
	akiko.dma_timer = timer_alloc(machine, akiko_dma_proc, NULL);
	akiko.frame_timer = timer_alloc(machine, akiko_frame_proc, NULL);
	akiko.i2cmem = machine->device("i2cmem");

	machine->add_notifier(MACHINE_NOTIFY_EXIT, amiga_akiko_exit);

	/* create the TOC table */
	if ( akiko.cdrom != NULL && cdrom_get_last_track(akiko.cdrom) )
	{
		UINT8 *p;
		int		i, addrctrl = cdrom_get_adr_control( akiko.cdrom, 0 );
		UINT32	discend;

		discend = cdrom_get_track_start(akiko.cdrom,cdrom_get_last_track(akiko.cdrom)-1);
		discend += cdrom_get_toc(akiko.cdrom)->tracks[cdrom_get_last_track(akiko.cdrom)-1].frames;
		discend = lba_to_msf(discend);

		akiko.cdrom_numtracks = cdrom_get_last_track(akiko.cdrom)+3;

		akiko.cdrom_toc = auto_alloc_array(machine, UINT8, 13*akiko.cdrom_numtracks);
		memset( akiko.cdrom_toc, 0, 13*akiko.cdrom_numtracks);

		p = akiko.cdrom_toc;
		p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
		p[3] = 0xa0; /* first track */
		p[8] = 1;
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa1; /* last track */
		p[8] = cdrom_get_last_track(akiko.cdrom);
		p += 13;
		p[1] = 0x01;
		p[3] = 0xa2; /* disc end */
		p[8] = (discend >> 16 ) & 0xff;
		p[9] = (discend >> 8 ) & 0xff;
		p[10] = discend & 0xff;
		p += 13;

		for( i = 0; i < cdrom_get_last_track(akiko.cdrom); i++ )
		{
			UINT32	trackpos = cdrom_get_track_start(akiko.cdrom,i);

			trackpos = lba_to_msf(trackpos);
			addrctrl = cdrom_get_adr_control( akiko.cdrom, i );

			p[1] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
			p[3] = dec_2_bcd( i+1 );
			p[8] = (trackpos >> 16 ) & 0xff;
			p[9] = (trackpos >> 8 ) & 0xff;
			p[10] = trackpos & 0xff;

			p += 13;
		}
	}
}

static void akiko_nvram_write(running_machine *machine, UINT32 data)
{
	akiko.i2c_scl_out = BIT(data,31);
	akiko.i2c_sda_out = BIT(data,30);
	akiko.i2c_scl_dir = BIT(data,15);
	akiko.i2c_sda_dir = BIT(data,14);

	i2cmem_scl_write( akiko.i2cmem, akiko.i2c_scl_out );
	i2cmem_sda_write( akiko.i2cmem, akiko.i2c_sda_out );
}

static UINT32 akiko_nvram_read(running_machine *machine)
{
	UINT32	v = 0;

	if ( akiko.i2c_scl_dir )
	{
		v |= akiko.i2c_scl_out << 31;
	}
	else
	{
		v |= 0 << 31;
	}

	if ( akiko.i2c_sda_dir )
	{
		v |= akiko.i2c_sda_out << 30;
	}
	else
	{
		v |= i2cmem_sda_read( akiko.i2cmem ) << 30;
	}

	v |= akiko.i2c_scl_dir << 15;
	v |= akiko.i2c_sda_dir << 14;

	return v;
}

/*************************************
 *
 * Akiko Chunky to Planar converter
 *
 ************************************/

static void akiko_c2p_write(UINT32 data)
{
	akiko.c2p_input_buffer[akiko.c2p_input_index] = data;
	akiko.c2p_input_index++;
	akiko.c2p_input_index &= 7;
	akiko.c2p_output_index = 0;
}

static UINT32 akiko_c2p_read(void)
{
	UINT32 val;

	if ( akiko.c2p_output_index == 0 )
	{
		int i;

		for ( i = 0; i < 8; i++ )
			akiko.c2p_output_buffer[i] = 0;

		for (i = 0; i < 8 * 32; i++) {
			if (akiko.c2p_input_buffer[7 - (i >> 5)] & (1 << (i & 31)))
				akiko.c2p_output_buffer[i & 7] |= 1 << (i >> 3);
		}
	}
	akiko.c2p_input_index = 0;
	val = akiko.c2p_output_buffer[akiko.c2p_output_index];
	akiko.c2p_output_index++;
	akiko.c2p_output_index &= 7;
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

static void akiko_cdda_stop( running_machine *machine )
{
	running_device *cdda = cdda_from_cdrom(machine, akiko.cdrom);

	if (cdda != NULL)
	{
		cdda_stop_audio(cdda);
		timer_reset( akiko.frame_timer, attotime_never );
	}
}

static void akiko_cdda_play( running_machine *machine, UINT32 lba, UINT32 num_blocks )
{
	running_device *cdda = cdda_from_cdrom(machine, akiko.cdrom);
	if (cdda != NULL)
	{
		cdda_start_audio(cdda, lba, num_blocks);
		timer_adjust_oneshot( akiko.frame_timer, ATTOTIME_IN_HZ( 75 ), 0 );
	}
}

static void akiko_cdda_pause( running_machine *machine, int pause )
{
	running_device *cdda = cdda_from_cdrom(machine, akiko.cdrom);
	if (cdda != NULL)
	{
		if (cdda_audio_active(cdda) && cdda_audio_paused(cdda) != pause )
		{
			cdda_pause_audio(cdda, pause);

			if ( pause )
			{
				timer_reset( akiko.frame_timer, attotime_never );
			}
			else
			{
				timer_adjust_oneshot( akiko.frame_timer, ATTOTIME_IN_HZ( 75 ), 0 );
			}
		}
	}
}

static UINT8 akiko_cdda_getstatus( running_machine *machine, UINT32 *lba )
{
	running_device *cdda = cdda_from_cdrom(machine, akiko.cdrom);

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

static void akiko_set_cd_status( running_machine *machine, UINT32 status )
{
	akiko.cdrom_status[0] |= status;

	if ( akiko.cdrom_status[0] & akiko.cdrom_status[1] )
	{
		if (LOG_AKIKO_CD) logerror( "Akiko CD IRQ\n" );
		amiga_custom_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, 0x8000 | INTENA_PORTS, 0xffff);
	}
}

static TIMER_CALLBACK(akiko_frame_proc)
{
	running_device *cdda = cdda_from_cdrom(machine, akiko.cdrom);

	(void)param;

	if (cdda != NULL)
	{
		UINT8	s = akiko_cdda_getstatus(machine, NULL);

		if ( s == 0x11 )
		{
			akiko_set_cd_status( machine, 0x80000000 );	/* subcode ready */
		}

		timer_adjust_oneshot( akiko.frame_timer, ATTOTIME_IN_HZ( 75 ), 0 );
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
	UINT8	buf[2352];
	int		index;

	if ( (akiko.cdrom_dmacontrol & 0x04000000) == 0 )
		return;

	if ( akiko.cdrom_readreqmask == 0 )
		return;

	index = (akiko.cdrom_lba_cur - akiko.cdrom_lba_start) & 0x0f;

	if ( akiko.cdrom_readreqmask & ( 1 << index ) )
	{
		UINT32	track = cdrom_get_track( akiko.cdrom, akiko.cdrom_lba_cur );
		UINT32	datasize = cdrom_get_toc( akiko.cdrom )->tracks[track].datasize;
		UINT32	subsize = cdrom_get_toc( akiko.cdrom )->tracks[track].subsize;
		int		i;

		UINT32	curmsf = lba_to_msf( akiko.cdrom_lba_cur );
		memset( buf, 0, 16 );

		buf[3] = akiko.cdrom_lba_cur - akiko.cdrom_lba_start;
		memset( &buf[4], 0xff, 8 );

		buf[12] = (curmsf >> 16) & 0xff;
		buf[13] = (curmsf >> 8) & 0xff;
		buf[14] = curmsf & 0xff;
		buf[15] = 0x01; /* mode1 */

		datasize = 2048;
		if ( !cdrom_read_data( akiko.cdrom, akiko.cdrom_lba_cur, &buf[16], CD_TRACK_MODE1 ) )
		{
			logerror( "AKIKO: Read error trying to read sector %08x!\n", akiko.cdrom_lba_cur );
			return;
		}

		if ( subsize )
		{
			if ( !cdrom_read_subcode( akiko.cdrom, akiko.cdrom_lba_cur, &buf[16+datasize] ) )
			{
				logerror( "AKIKO: Read error trying to read subcode for sector %08x!\n", akiko.cdrom_lba_cur );
				return;
			}
		}

		if (LOG_AKIKO_CD) logerror( "DMA: sector %d - address %08x\n", akiko.cdrom_lba_cur, akiko.cdrom_address[0] + (index*4096) );

		for( i = 0; i < 2352; i += 2 )
		{
			UINT16	data;

			data = buf[i];
			data <<= 8;
			data |= buf[i+1];

			amiga_chip_ram_w( akiko.cdrom_address[0] + (index*4096) + i, data );
		}

		akiko.cdrom_readmask |= ( 1 << index );
		akiko.cdrom_readreqmask &= ~( 1 << index );
		akiko.cdrom_lba_cur++;
	}

	if ( akiko.cdrom_readreqmask == 0 )
		akiko_set_cd_status(machine, 0x04000000);
	else
		timer_adjust_oneshot( akiko.dma_timer, ATTOTIME_IN_USEC( CD_SECTOR_TIME / akiko.cdrom_speed ), 0 );
}

static void akiko_start_dma( void )
{
	if ( akiko.cdrom_readreqmask == 0 )
		return;

	if ( akiko.cdrom_lba_start > akiko.cdrom_lba_end )
		return;

	if ( akiko.cdrom_speed == 0 )
		return;

	akiko.cdrom_lba_cur = akiko.cdrom_lba_start;

	timer_adjust_oneshot( akiko.dma_timer, ATTOTIME_IN_USEC( CD_SECTOR_TIME / akiko.cdrom_speed ), 0 );
}

static void akiko_setup_response( const address_space *space, int len, UINT8 *r1 )
{
	int		resp_addr = akiko.cdrom_address[1];
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
		memory_write_byte( space, resp_addr + ((akiko.cdrom_cmd_resp + i) & 0xff), resp_buffer[i] );
	}

	akiko.cdrom_cmd_resp = (akiko.cdrom_cmd_resp+len) & 0xff;

	akiko_set_cd_status( space->machine, 0x10000000 ); /* new data available */
}

static TIMER_CALLBACK( akiko_cd_delayed_cmd )
{
	UINT8	resp[32];
	UINT8	cddastatus;

	if ( akiko.cdrom_status[0] & 0x10000000 )
		return;

	cddastatus = akiko_cdda_getstatus(machine, NULL);

	if ( cddastatus == 0x11 || cddastatus == 0x12 )
		return;

	memset( resp, 0, sizeof( resp ) );
	resp[0] = param;

	param &= 0x0f;

	if ( param == 0x05 )
	{
		const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		if (LOG_AKIKO_CD) logerror( "AKIKO: Completing Command %d\n", param );

		resp[0] = 0x06;

		if ( akiko.cdrom == NULL || akiko.cdrom_numtracks == 0 )
		{
			resp[1] = 0x80;
			akiko_setup_response( space, 15, resp );
		}
		else
		{
			resp[1] = 0x00;
			memcpy( &resp[2], &akiko.cdrom_toc[13*akiko.cdrom_track_index], 13 );

			akiko.cdrom_track_index = ( akiko.cdrom_track_index + 1 ) % akiko.cdrom_numtracks;

			akiko_setup_response( space, 15, resp );
		}
	}
}

static void akiko_update_cdrom(const address_space *space)
{
	UINT8	resp[32], cmdbuf[32];

	if ( akiko.cdrom_status[0] & 0x10000000 )
		return;

	while ( akiko.cdrom_cmd_start != akiko.cdrom_cmd_end )
	{
		UINT32	cmd_addr = akiko.cdrom_address[1] + 0x200 + akiko.cdrom_cmd_start;
		int		cmd = memory_read_byte( space, cmd_addr );

		memset( resp, 0, sizeof( resp ) );
		resp[0] = cmd;

		cmd &= 0x0f;

		if (LOG_AKIKO_CD) logerror( "CDROM command: %02X\n", cmd );

		if ( cmd == 0x02 ) /* pause audio */
		{
			resp[1] = 0x00;

			if ( akiko_cdda_getstatus(space->machine, NULL) == 0x11 )
				resp[1] = 0x08;

			akiko_cdda_pause( space->machine, 1 );

			akiko.cdrom_cmd_start = (akiko.cdrom_cmd_start+2) & 0xff;

			akiko_setup_response( space, 2, resp );
		}
		else if ( cmd == 0x03 ) /* unpause audio (and check audiocd playing status) */
		{
			resp[1] = 0x00;

			if ( akiko_cdda_getstatus(space->machine, NULL) == 0x11 )
				resp[1] = 0x08;

			akiko_cdda_pause( space->machine, 0 );

			akiko.cdrom_cmd_start = (akiko.cdrom_cmd_start+2) & 0xff;

			akiko_setup_response( space, 2, resp );
		}
		else if ( cmd == 0x04 ) /* seek/read/play cd multi command */
		{
			int	i;
			UINT32	startpos, endpos;

			for( i = 0; i < 13; i++ )
			{
				cmdbuf[i] = memory_read_byte( space, cmd_addr );
				cmd_addr &= 0xffffff00;
				cmd_addr += ( akiko.cdrom_cmd_start + i + 1 ) & 0xff;
			}

			akiko.cdrom_cmd_start = (akiko.cdrom_cmd_start+13) & 0xff;

			if ( akiko.cdrom == NULL || akiko.cdrom_numtracks == 0 )
			{
				resp[1] = 0x80;
				akiko_setup_response( space, 2, resp );
			}
			else
			{
				startpos = lba_from_triplet( &cmdbuf[1] );
				endpos = lba_from_triplet( &cmdbuf[4] );

				akiko_cdda_stop(space->machine);

				resp[1] = 0x00;

				if ( cmdbuf[7] == 0x80 )
				{
					if (LOG_AKIKO_CD) logerror( "%s:AKIKO CD: Data read - start lba: %08x - end lba: %08x\n", cpuexec_describe_context(space->machine), startpos, endpos );
					akiko.cdrom_speed = (cmdbuf[8] & 0x40) ? 2 : 1;
					akiko.cdrom_lba_start = startpos;
					akiko.cdrom_lba_end = endpos;

					resp[1] = 0x02;
				}
				else if ( cmdbuf[10] & 0x04 )
				{
					logerror( "AKIKO CD: Audio Play - start lba: %08x - end lba: %08x\n", startpos, endpos );
					akiko_cdda_play( space->machine, startpos, endpos - startpos );
					resp[1] = 0x08;
				}
				else
				{
					if (LOG_AKIKO_CD) logerror( "AKIKO CD: Seek - start lba: %08x - end lba: %08x\n", startpos, endpos );
					akiko.cdrom_track_index = 0;

					for( i = 0; i < cdrom_get_last_track(akiko.cdrom); i++ )
					{
						if ( startpos <= cdrom_get_track_start( akiko.cdrom, i ) )
						{
							/* reset to 0 */
							akiko.cdrom_track_index = i + 2;
							akiko.cdrom_track_index %= akiko.cdrom_numtracks;
							break;
						}
					}
				}

				akiko_setup_response( space, 2, resp );
			}
		}
		else if ( cmd == 0x05 ) /* read toc */
		{
			akiko.cdrom_cmd_start = (akiko.cdrom_cmd_start+3) & 0xff;

			timer_set( space->machine, ATTOTIME_IN_MSEC(1), NULL, resp[0], akiko_cd_delayed_cmd );

			break;
		}
		else if ( cmd == 0x06 ) /* read subq */
		{
			UINT32	lba;

			resp[1] = 0x00;

			(void)akiko_cdda_getstatus( space->machine, &lba );

			if ( lba > 0 )
			{
				UINT32	disk_pos;
				UINT32	track_pos;
				UINT32	track;
				int		addrctrl;

				track = cdrom_get_track(akiko.cdrom, lba);
				addrctrl = cdrom_get_adr_control(akiko.cdrom, track);

				resp[2] = 0x00;
				resp[3] = ((addrctrl & 0x0f) << 4) | ((addrctrl & 0xf0) >> 4);
				resp[4] = dec_2_bcd(track+1);
				resp[5] = 0; /* index */

				disk_pos = lba_to_msf(lba);
				track_pos = lba_to_msf(lba - cdrom_get_track_start(akiko.cdrom, track));

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

			akiko_setup_response( space, 15, resp );
		}
		else if ( cmd == 0x07 )	/* check door status */
		{
			resp[1] = 0x01;

			akiko.cdrom_cmd_start = (akiko.cdrom_cmd_start+2) & 0xff;

			if ( akiko.cdrom == NULL || akiko.cdrom_numtracks == 0 )
				resp[1] = 0x80;

			akiko_setup_response( space, 20, resp );
			break;
		}
		else
		{
			break;
		}
	}
}

READ32_HANDLER(amiga_akiko32_r)
{
	UINT32		retval;

	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Reading AKIKO reg %0x [%s] at PC=%06x\n", offset, get_akiko_reg_name(offset), cpu_get_pc(space->cpu) );
	}

	switch( offset )
	{
		case 0x00/4:	/* ID */
			if ( akiko.cdrom != NULL ) cdda_set_cdrom(space->machine->device("cdda"), akiko.cdrom);
			return 0x0000cafe;

		case 0x04/4:	/* CDROM STATUS 1 */
			return akiko.cdrom_status[0];

		case 0x08/4:	/* CDROM STATUS 2 */
			return akiko.cdrom_status[1];

		case 0x10/4:	/* CDROM ADDRESS 1 */
			return akiko.cdrom_address[0];

		case 0x14/4:	/* CDROM ADDRESS 2 */
			return akiko.cdrom_address[1];

		case 0x18/4:	/* CDROM COMMAND 1 */
			akiko_update_cdrom(space);
			retval = akiko.cdrom_cmd_start;
			retval <<= 8;
			retval |= akiko.cdrom_cmd_resp;
			retval <<= 8;
			return retval;

		case 0x1C/4:	/* CDROM COMMAND 2 */
			akiko_update_cdrom(space);
			retval = akiko.cdrom_cmd_end;
			retval <<= 16;
			return retval;

		case 0x20/4:	/* CDROM DMA SECTOR READ MASK */
			retval = akiko.cdrom_readmask << 16;
			return retval;

		case 0x24/4:	/* CDROM DMA ENABLE? */
			retval = akiko.cdrom_dmacontrol;
			return retval;

		case 0x30/4:	/* NVRAM */
			return akiko_nvram_read(space->machine);

		case 0x38/4:	/* C2P */
			return akiko_c2p_read();

		default:
			break;
	}

	return 0;
}

WRITE32_HANDLER(amiga_akiko32_w)
{
	if ( LOG_AKIKO && offset < (0x30/4) )
	{
		logerror( "Writing AKIKO reg %0x [%s] with %08x at PC=%06x\n", offset, get_akiko_reg_name(offset), data, cpu_get_pc(space->cpu) );
	}

	switch( offset )
	{
		case 0x04/4:	/* CDROM STATUS 1 */
			akiko.cdrom_status[0] = data;
			break;

		case 0x08/4:	/* CDROM STATUS 2 */
			akiko.cdrom_status[1] = data;
			akiko.cdrom_status[0] &= data;
			break;

		case 0x10/4:	/* CDROM ADDRESS 1 */
			akiko.cdrom_address[0] = data;
			break;

		case 0x14/4:	/* CDROM ADDRESS 2 */
			akiko.cdrom_address[1] = data;
			break;

		case 0x18/4:	/* CDROM COMMAND 1 */
			if ( ACCESSING_BITS_16_23 )
				akiko.cdrom_cmd_start = ( data >> 16 ) & 0xff;

			if ( ACCESSING_BITS_8_15 )
				akiko.cdrom_cmd_resp = ( data >> 8 ) & 0xff;

			akiko_update_cdrom(space);
			break;

		case 0x1C/4:	/* CDROM COMMAND 2 */
			if ( ACCESSING_BITS_16_23 )
				akiko.cdrom_cmd_end = ( data >> 16 ) & 0xff;

			akiko_update_cdrom(space);
			break;

		case 0x20/4:	/* CDROM DMA SECTOR READ REQUEST WRITE */
			if (LOG_AKIKO_CD) logerror( "Read Req mask W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ACCESSING_BITS_16_31 )
			{
				akiko.cdrom_readreqmask = (data >> 16);
				akiko.cdrom_readmask = 0;
			}
			break;

		case 0x24/4:	/* CDROM DMA ENABLE? */
			if (LOG_AKIKO_CD) logerror( "DMA enable W: data %08x - mem mask %08x\n", data, mem_mask );
			if ( ( akiko.cdrom_dmacontrol ^ data ) & 0x04000000 )
			{
				if ( data & 0x04000000 )
					akiko_start_dma();
			}
			akiko.cdrom_dmacontrol = data;
			break;

		case 0x30/4:
			akiko_nvram_write(space->machine, data);
			break;

		case 0x38/4:
			akiko_c2p_write(data);
			break;

		default:
			break;
	}
}
