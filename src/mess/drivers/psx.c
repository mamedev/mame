/***************************************************************************

  Sony Playstaion
  ===============
  Preliminary driver by smf
  Additional development by pSXAuthor and R. Belmont

***************************************************************************/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "imagedev/snapquik.h"
#include "imagedev/chd_cd.h"
#include "includes/psx.h"
#include "sound/spu.h"
#include "debugger.h"
#include "zlib.h"
#include "machine/psxcd.h"
#include "machine/psxcard.h"

typedef struct
{
	UINT8 n_shiftin;
	UINT8 n_shiftout;
	int n_bits;
	int n_state;
	int n_byte;
	int b_lastclock;
	int b_ack;
} pad_t;


class psx1_state : public psx_state
{
public:
	psx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: psx_state(mconfig, type, tag) { }

	UINT8 *m_exe_buffer;
	int m_exe_size;
	pad_t m_pad[ 2 ];
	int m_cd_param_p;
	int m_cd_result_p;
	int m_cd_result_c;
	int m_cd_result_ready;
	int m_cd_reset;
	UINT8 m_cd_stat;
	UINT8 m_cd_io_status;
	UINT8 m_cd_param[8];
	UINT8 m_cd_result[8];
	DECLARE_READ8_MEMBER(psx_cd_r);
	DECLARE_WRITE8_MEMBER(psx_cd_w);
	DECLARE_DIRECT_UPDATE_MEMBER(psx_default);
	DECLARE_DIRECT_UPDATE_MEMBER(psx_setopbase);
	DECLARE_DRIVER_INIT(psx);
};


#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}


static void psxexe_conv32( UINT32 *p_uint32 )
{
	UINT8 *p_uint8;

	p_uint8 = (UINT8 *)p_uint32;

	*( p_uint32 ) = p_uint8[ 0 ] |
		( p_uint8[ 1 ] << 8 ) |
		( p_uint8[ 2 ] << 16 ) |
		( p_uint8[ 3 ] << 24 );
}

static int load_psxexe( device_t *cpu, unsigned char *p_n_file, int n_len )
{
	psx_state *state = cpu->machine().driver_data<psx_state>();
	struct PSXEXE_HEADER
	{
		UINT8 id[ 8 ];
		UINT32 text;	/* SCE only */
		UINT32 data;	/* SCE only */
		UINT32 pc0;
		UINT32 gp0;		/* SCE only */
		UINT32 t_addr;
		UINT32 t_size;
		UINT32 d_addr;	/* SCE only */
		UINT32 d_size;	/* SCE only */
		UINT32 b_addr;	/* SCE only */
		UINT32 b_size;	/* SCE only */
		UINT32 s_addr;
		UINT32 s_size;
		UINT32 SavedSP;
		UINT32 SavedFP;
		UINT32 SavedGP;
		UINT32 SavedRA;
		UINT32 SavedS0;
		UINT8 dummy[ 0x800 - 76 ];
	};

	struct PSXEXE_HEADER *psxexe_header = (struct PSXEXE_HEADER *)p_n_file;

	if( n_len >= sizeof( struct PSXEXE_HEADER ) &&
		memcmp( psxexe_header->id, "PS-X EXE", 8 ) == 0 )
	{
		UINT8 *p_ram;
		UINT8 *p_psxexe;
		UINT32 n_stack;
		UINT32 n_ram;
		UINT32 n_address;
		UINT32 n_size;

		psxexe_conv32( &psxexe_header->text );
		psxexe_conv32( &psxexe_header->data );
		psxexe_conv32( &psxexe_header->pc0 );
		psxexe_conv32( &psxexe_header->gp0 );
		psxexe_conv32( &psxexe_header->t_addr );
		psxexe_conv32( &psxexe_header->t_size );
		psxexe_conv32( &psxexe_header->d_addr );
		psxexe_conv32( &psxexe_header->d_size );
		psxexe_conv32( &psxexe_header->b_addr );
		psxexe_conv32( &psxexe_header->b_size );
		psxexe_conv32( &psxexe_header->s_addr );
		psxexe_conv32( &psxexe_header->s_size );
		psxexe_conv32( &psxexe_header->SavedSP );
		psxexe_conv32( &psxexe_header->SavedFP );
		psxexe_conv32( &psxexe_header->SavedGP );
		psxexe_conv32( &psxexe_header->SavedRA );
		psxexe_conv32( &psxexe_header->SavedS0 );

		/* todo: check size.. */

		logerror( "psx_exe_load: pc    %08x\n", psxexe_header->pc0 );
		logerror( "psx_exe_load: org   %08x\n", psxexe_header->t_addr );
		logerror( "psx_exe_load: len   %08x\n", psxexe_header->t_size );
		logerror( "psx_exe_load: sp    %08x\n", psxexe_header->s_addr );
		logerror( "psx_exe_load: len   %08x\n", psxexe_header->s_size );

		p_ram = (UINT8 *)state->m_p_n_psxram;
		n_ram = state->m_n_psxramsize;

		p_psxexe = p_n_file + sizeof( struct PSXEXE_HEADER );

		n_address = psxexe_header->t_addr;
		n_size = psxexe_header->t_size;
		while( n_size != 0 )
		{
			p_ram[ BYTE4_XOR_LE( n_address ) % n_ram ] = *( p_psxexe );
			n_address++;
			p_psxexe++;
			n_size--;
		}

		cpu->state().set_state_int( PSXCPU_PC, psxexe_header->pc0 );
		cpu->state().set_state_int( PSXCPU_R28, psxexe_header->gp0 );
		n_stack = psxexe_header->s_addr + psxexe_header->s_size;
		if( n_stack != 0 )
		{
			cpu->state().set_state_int( PSXCPU_R29, n_stack );
			cpu->state().set_state_int( PSXCPU_R30, n_stack );
		}

		return 1;
	}
	return 0;
}

static void cpe_set_register( device_t *cpu, int n_reg, int n_value )
{
	if( n_reg < 0x80 && ( n_reg % 4 ) == 0 )
	{
		logerror( "psx_exe_load: r%-2d   %08x\n", n_reg / 4, n_value );
		cpu->state().set_state_int( PSXCPU_R0 + ( n_reg / 4 ), n_value );
	}
	else if( n_reg == 0x80 )
	{
		logerror( "psx_exe_load: lo    %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_LO, n_value );
	}
	else if( n_reg == 0x84 )
	{
		logerror( "psx_exe_load: hi    %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_HI, n_value );
	}
	else if( n_reg == 0x88 )
	{
		logerror( "psx_exe_load: sr    %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_CP0R12, n_value );
	}
	else if( n_reg == 0x8c )
	{
		logerror( "psx_exe_load: cause %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_CP0R13, n_value );
	}
	else if( n_reg == 0x90 )
	{
		logerror( "psx_exe_load: pc    %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_PC, n_value );
	}
	else if( n_reg == 0x94 )
	{
		logerror( "psx_exe_load: prid  %08x\n", n_value );
		cpu->state().set_state_int( PSXCPU_CP0R15, n_value );
	}
	else
	{
		logerror( "psx_exe_load: invalid register %04x/%08x\n", n_reg, n_value );
	}
}

static int load_cpe( device_t *cpu, unsigned char *p_n_file, int n_len )
{
	psx_state *state = cpu->machine().driver_data<psx_state>();
	if( n_len >= 4 &&
		memcmp( p_n_file, "CPE\001", 4 ) == 0 )
	{
		int n_offset = 4;

		for( ;; )
		{
			if( n_offset >= n_len || p_n_file[ n_offset ] > 8 )
			{
				break;
			}

			switch( p_n_file[ n_offset++ ] )
			{
			case 0:
				/* end of file */
				return 1;
			case 1:
				/* read bytes */
				{
					int n_address = ( (int)p_n_file[ n_offset + 0 ] << 0 ) |
						( (int)p_n_file[ n_offset + 1 ] << 8 ) |
						( (int)p_n_file[ n_offset + 2 ] << 16 ) |
						( (int)p_n_file[ n_offset + 3 ] << 24 );
					int n_size = ( (int)p_n_file[ n_offset + 4 ] << 0 ) |
						( (int)p_n_file[ n_offset + 5 ] << 8 ) |
						( (int)p_n_file[ n_offset + 6 ] << 16 ) |
						( (int)p_n_file[ n_offset + 7 ] << 24 );

					UINT8 *p_ram = (UINT8 *)state->m_p_n_psxram;
					UINT32 n_ram = state->m_n_psxramsize;

					n_offset += 8;

					logerror( "psx_exe_load: org   %08x\n", n_address );
					logerror( "psx_exe_load: len   %08x\n", n_size );

					while( n_size > 0 )
					{
						p_ram[ BYTE4_XOR_LE( n_address ) % n_ram ] = p_n_file[ n_offset++ ];
						n_address++;
						n_size--;
					}
					break;
				}
			case 2:
				/* run address: not tested */
				{
					int n_value = ( (int)p_n_file[ n_offset + 2 ] << 0 ) |
						( (int)p_n_file[ n_offset + 3 ] << 8 ) |
						( (int)p_n_file[ n_offset + 4 ] << 16 ) |
						( (int)p_n_file[ n_offset + 5 ] << 24 );

					n_offset += 4;

					cpe_set_register( cpu, 0x90, n_value );
					break;
				}
			case 3:
				/* set reg to longword */
				{
					int n_reg = ( (int)p_n_file[ n_offset + 0 ] << 0 ) |
						( (int)p_n_file[ n_offset + 1 ] << 8 );
					int n_value = ( (int)p_n_file[ n_offset + 2 ] << 0 ) |
						( (int)p_n_file[ n_offset + 3 ] << 8 ) |
						( (int)p_n_file[ n_offset + 4 ] << 16 ) |
						( (int)p_n_file[ n_offset + 5 ] << 24 );

					n_offset += 6;

					cpe_set_register( cpu, n_reg, n_value );
					break;
				}
			case 4:
				/* set reg to word: not tested */
				{
					int n_reg = ( (int)p_n_file[ n_offset + 0 ] << 0 ) |
						( (int)p_n_file[ n_offset + 1 ] << 8 );
					int n_value = ( (int)p_n_file[ n_offset + 2 ] << 0 ) |
						( (int)p_n_file[ n_offset + 3 ] << 8 );

					n_offset += 4;

					cpe_set_register( cpu, n_reg, n_value );
					break;
				}
			case 5:
				/* set reg to byte: not tested */
				{
					int n_reg = ( (int)p_n_file[ n_offset + 0 ] << 0 ) |
						( (int)p_n_file[ n_offset + 1 ] << 8 );
					int n_value = ( (int)p_n_file[ n_offset + 2 ] << 0 );

					n_offset += 3;

					cpe_set_register( cpu, n_reg, n_value );
					break;
				}
			case 6:
				/* set reg to 3-byte: not tested */
				{
					int n_reg = ( (int)p_n_file[ n_offset + 0 ] << 0 ) |
						( (int)p_n_file[ n_offset + 1 ] << 8 );
					int n_value = ( (int)p_n_file[ n_offset + 2 ] << 0 ) |
						( (int)p_n_file[ n_offset + 3 ] << 8 ) |
						( (int)p_n_file[ n_offset + 4 ] << 16 );

					n_offset += 5;

					cpe_set_register( cpu, n_reg, n_value );
					break;
				}
			case 7:
				/* workspace: not tested */
				n_offset += 4;
				break;
			case 8:
				/* unit */
				{
					int n_unit = p_n_file[ n_offset + 0 ];

					n_offset++;

					logerror( "psx_exe_load: unit  %08x\n", n_unit );
				}
				break;
			}
		}
	}
	return 0;
}

static int load_psf( device_t *cpu, unsigned char *p_n_file, int n_len )
{
	int n_return;
	unsigned long n_crc;
	unsigned long n_compressed;
	unsigned char *p_n_compressed;
	unsigned long n_uncompressed;
	unsigned char *p_n_uncompressed;

	struct PSF_HEADER
	{
		unsigned char id[ 4 ];
		UINT32 reserved_size;
		UINT32 exe_size;
		UINT32 exe_crc;
	};

	struct PSF_HEADER *psf_header = (struct PSF_HEADER *)p_n_file;

	n_return = 0;

	if( n_len >= sizeof( struct PSF_HEADER ) &&
		memcmp( p_n_file, "PSF", 3 ) == 0 )
	{
		psxexe_conv32( &psf_header->reserved_size );
		psxexe_conv32( &psf_header->exe_size );
		psxexe_conv32( &psf_header->exe_crc );

		logerror( "psx_exe_load: reserved_size %08x\n", psf_header->reserved_size );
		logerror( "psx_exe_load: exe_size      %08x\n", psf_header->exe_size );
		logerror( "psx_exe_load: exe_crc       %08x\n", psf_header->exe_crc );

		n_compressed = psf_header->exe_size;
		p_n_compressed = p_n_file + sizeof( struct PSF_HEADER ) + psf_header->reserved_size;

		n_crc = crc32( crc32( 0L, Z_NULL, 0 ), p_n_compressed, n_compressed );
		if( n_crc != psf_header->exe_crc )
		{
			logerror( "psx_exe_load: psf invalid crc\n" );
			return 0;
		}

		n_uncompressed = 0x200000;
		p_n_uncompressed = (unsigned char *)malloc( n_uncompressed );

		if( uncompress( p_n_uncompressed, &n_uncompressed, p_n_compressed, n_compressed ) != Z_OK )
		{
			logerror( "psx_exe_load: psf uncompress failed\n" );
		}
		else if( !load_psxexe( cpu, p_n_uncompressed, n_uncompressed ) )
		{
			logerror( "psx_exe_load: psf load failed\n" );
		}
		else
		{
			n_return = 1;
		}

		free( p_n_uncompressed );
	}
	return n_return;
}

DIRECT_UPDATE_MEMBER(psx1_state::psx_default)
{
	return address;
}

DIRECT_UPDATE_MEMBER(psx1_state::psx_setopbase)
{
	if( address == 0x80030000 )
	{
		device_t *cpu = machine().device("maincpu");

		machine().device("maincpu")->memory().space(AS_PROGRAM)->set_direct_update_handler(direct_update_delegate(FUNC(psx1_state::psx_default), this));

		if( load_psxexe( cpu, m_exe_buffer, m_exe_size ) ||
			load_cpe( cpu, m_exe_buffer, m_exe_size ) ||
			load_psf( cpu, m_exe_buffer, m_exe_size ) )
		{
/*          DEBUGGER_BREAK; */

			address = cpu->state().state_int( PSXCPU_PC );
		}
		else
		{
			logerror( "psx_exe_load: invalid exe\n" );
		}

		m_exe_size = 0;
		free( m_exe_buffer );
	}
	return address;
}

static QUICKLOAD_LOAD( psx_exe_load )
{
	psx1_state *state = image.device().machine().driver_data<psx1_state>();
	address_space *space = image.device().machine().device( "maincpu")->memory().space( AS_PROGRAM );

	state->m_exe_size = 0;
	state->m_exe_buffer = (UINT8*)malloc( quickload_size );
	if( state->m_exe_buffer == NULL )
	{
		logerror( "psx_exe_load: out of memory\n" );
		return IMAGE_INIT_FAIL;
	}
	if( image.fread( state->m_exe_buffer, quickload_size ) != quickload_size )
	{
		free( state->m_exe_buffer );
		return IMAGE_INIT_FAIL;
	}
	state->m_exe_size = quickload_size;
	space->set_direct_update_handler(direct_update_delegate(FUNC(psx1_state::psx_setopbase), state));

	return IMAGE_INIT_PASS;
}

/* PAD emulation */

#define PAD_STATE_IDLE ( 0 )
#define PAD_STATE_LISTEN ( 1 )
#define PAD_STATE_ACTIVE ( 2 )
#define PAD_STATE_READ ( 3 )
#define PAD_STATE_UNLISTEN ( 4 )
#define PAD_STATE_MEMCARD ( 5 )

#define PAD_TYPE_STANDARD ( 4 )
#define PAD_BYTES_STANDARD ( 2 )

#define PAD_CMD_START ( 0x01 )
#define PAD_CMD_READ  ( 0x42 ) /* B */

#define PAD_DATA_OK   ( 0x5a ) /* Z */
#define PAD_DATA_IDLE ( 0xff )

static TIMER_CALLBACK(psx_pad_ack)
{
	psx1_state *state = machine.driver_data<psx1_state>();
	int n_port = param;
	pad_t *pad = &state->m_pad[ n_port ];

	if( pad->n_state != PAD_STATE_IDLE )
	{
		psx_sio_input( machine, 0, PSX_SIO_IN_DSR, pad->b_ack * PSX_SIO_IN_DSR );
		if( !pad->b_ack )
		{
			pad->b_ack = 1;
			machine.scheduler().timer_set(attotime::from_usec( 2 ), FUNC(psx_pad_ack) , n_port);
		}
	}
}

static void psx_pad( running_machine &machine, int n_port, int n_data )
{
	psx1_state *state = machine.driver_data<psx1_state>();
	pad_t *pad = &state->m_pad[ n_port ];
	int b_sel;
	int b_clock;
	int b_data;
	int b_ack;
	int b_ready;
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };
	psxcard_device *psxcard = NULL;

	if (n_port == 0)
	{
		psxcard = machine.device<psxcard_device>("card1");
	}
	else
	{
		psxcard = machine.device<psxcard_device>("card2");
	}

	b_sel = ( n_data & PSX_SIO_OUT_DTR ) / PSX_SIO_OUT_DTR;
	b_clock = ( n_data & PSX_SIO_OUT_CLOCK ) / PSX_SIO_OUT_CLOCK;
	b_data = ( n_data & PSX_SIO_OUT_DATA ) / PSX_SIO_OUT_DATA;
	b_ready = 0;
	b_ack = 0;

	if( b_sel )
	{
		pad->n_state = PAD_STATE_IDLE;
	}

	switch( pad->n_state )
	{
	case PAD_STATE_LISTEN:
	case PAD_STATE_ACTIVE:
	case PAD_STATE_READ:
	case PAD_STATE_MEMCARD:
		if( pad->b_lastclock && !b_clock )
		{
			psx_sio_input( machine, 0, PSX_SIO_IN_DATA, ( pad->n_shiftout & 1 ) * PSX_SIO_IN_DATA );
			pad->n_shiftout >>= 1;
		}
		if( !pad->b_lastclock && b_clock )
		{
			pad->n_shiftin >>= 1;
			pad->n_shiftin |= b_data << 7;
			pad->n_bits++;

			if( pad->n_bits == 8 )
			{
				pad->n_bits = 0;
				b_ready = 1;
			}
		}
		break;
	}

	pad->b_lastclock = b_clock;

	switch( pad->n_state )
	{
	case PAD_STATE_IDLE:
		if( !b_sel )
		{
			pad->n_state = PAD_STATE_LISTEN;
			pad->n_shiftout = PAD_DATA_IDLE;
			pad->n_bits = 0;
		}
		break;
	case PAD_STATE_LISTEN:
		if( b_ready )
		{
			if( pad->n_shiftin == PAD_CMD_START )
			{
				pad->n_state = PAD_STATE_ACTIVE;
				pad->n_shiftout = ( PAD_TYPE_STANDARD << 4 ) | ( PAD_BYTES_STANDARD >> 1 );
				b_ack = 1;
			}
			else if( psxcard->transfer(pad->n_shiftin, &pad->n_shiftout) )
			{
				pad->n_state = PAD_STATE_MEMCARD;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_UNLISTEN;
			}
		}
		break;
	case PAD_STATE_MEMCARD:
		if( b_ready )
		{
			if( psxcard->transfer(pad->n_shiftin, &pad->n_shiftout) )
			{
				b_ack = 1;
			}
			else
			{
				b_ack = 0;
				pad->n_state = PAD_STATE_IDLE;
			}
		}
		break;
	case PAD_STATE_ACTIVE:
		if( b_ready )
		{
			if( pad->n_shiftin == PAD_CMD_READ )
			{
				pad->n_state = PAD_STATE_READ;
				pad->n_shiftout = PAD_DATA_OK;
				pad->n_byte = 0;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_UNLISTEN;
			}
		}
		break;
	case PAD_STATE_READ:
		if( b_ready )
		{
			if( pad->n_byte < PAD_BYTES_STANDARD )
			{
				pad->n_shiftout = machine.root_device().ioport(portnames[pad->n_byte + ( n_port * PAD_BYTES_STANDARD )])->read();
				pad->n_byte++;
				b_ack = 1;
			}
			else
			{
				pad->n_state = PAD_STATE_LISTEN;
			}
		}
		break;
	}

	if( b_ack )
	{
		pad->b_ack = 0;
		machine.scheduler().timer_set(attotime::from_usec( 10 ), FUNC(psx_pad_ack), n_port);
	}
}

static void psx_sio0( running_machine &machine, int n_data )
{
	/* todo: raise data & ack when nothing is driving it low */
	psx_pad( machine, 0, n_data );
	psx_pad( machine, 1, n_data ^ PSX_SIO_OUT_DTR );
}

/* ----------------------------------------------------------------------- */

static void cd_dma_read( psxcd_device *psxcd, UINT32 n_address, INT32 n_size )
{
	UINT8 *psxram = (UINT8 *)psxcd->machine().root_device().memshare("share1")->ptr();

	psxcd->start_dma(psxram + n_address, n_size*4);
}

static void cd_dma_write( psxcd_device *psxcd, UINT32 n_address, INT32 n_size )
{
	printf("cd_dma_write?!: addr %x, size %x\n", n_address, n_size);
}

READ8_MEMBER(psx1_state::psx_cd_r)
{
	psxcd_device *psxcd = machine().device<psxcd_device>(PSXCD_TAG);

	return psxcd->read_byte(offset);
}

WRITE8_MEMBER(psx1_state::psx_cd_w)
{
	psxcd_device *psxcd = machine().device<psxcd_device>(PSXCD_TAG);

	psxcd->write_byte(offset, data);
}

static ADDRESS_MAP_START( psx_map, AS_PROGRAM, 32, psx1_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f801800, 0x1f801803) AM_READWRITE8(psx_cd_r, psx_cd_w, 0xffffffff)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x801fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x80600000, 0x807fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa01fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END


static MACHINE_RESET( psx )
{
	psx_sio_install_handler( machine, 0, psx_sio0 );
}

DRIVER_INIT_MEMBER(psx1_state,psx)
{
	psx_driver_init(machine());
}

static INPUT_PORTS_START( psx )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Square")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Cross")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Circle")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Triangle")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 R1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 L1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 R2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 L2")

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SELECT ) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Square")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Cross")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Circle")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Triangle")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 R1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 L1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 R2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 L2")
INPUT_PORTS_END

static void spu_irq(device_t *device, UINT32 data)
{
	if (data)
	{
		psx_irq_set(device->machine(), 1<<9);
	}
}

struct cdrom_interface psx_cdrom =
{
	"psx_cdrom",
	NULL
};

static MACHINE_CONFIG_START( psxntsc, psx1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( psx_map )

	MCFG_MACHINE_RESET( psx )

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x100000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", psx_exe_load, "cpe,exe,psf,psx", 0)

	MCFG_CDROM_ADD("cdrom",psx_cdrom)
	MCFG_SOFTWARE_LIST_ADD("cd_list","psx")

	MCFG_PSXCD_ADD("cdrom")
	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 3, psx_dma_read_delegate( FUNC( cd_dma_read ), (psxcd_device *) device ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 3, psx_dma_write_delegate( FUNC( cd_dma_write ), (psxcd_device *) device ) )

	MCFG_PSXCARD_ADD("card1")
	MCFG_PSXCARD_ADD("card2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( psxpal, psx1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530AQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( psx_map)

	MCFG_MACHINE_RESET( psx )

	/* video hardware */
	/* TODO: visible area and refresh rate */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x100000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", psx_exe_load, "cpe,exe,psf,psx", 0)

	MCFG_CDROM_ADD("cdrom",psx_cdrom)
	MCFG_SOFTWARE_LIST_ADD("cd_list","psx")

	MCFG_PSXCD_ADD("cdrom")
	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 3, psx_dma_read_delegate( FUNC( cd_dma_read ), (psxcd_device *) device ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 3, psx_dma_write_delegate( FUNC( cd_dma_write ), (psxcd_device *) device ) )

	MCFG_PSXCARD_ADD("card1")
	MCFG_PSXCARD_ADD("card2")
MACHINE_CONFIG_END

ROM_START( psj )
	ROM_REGION32_LE( 0x080000, "user1", 0 )

	ROM_SYSTEM_BIOS( 0, "1.0J", "SCPH-1000/DTL-H1000" ) // 22091994
	ROMX_LOAD( "ps-10j.bin",    0x0000000, 0x080000, CRC(3b601fc8) SHA1(343883a7b555646da8cee54aadd2795b6e7dd070), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "1.1J", "SCPH-3000/DTL-H1000H (Version 1.1 01/22/95)" ) // 22091994
	ROMX_LOAD( "ps-11j.bin",    0x0000000, 0x080000, CRC(3539def6) SHA1(b06f4a861f74270be819aa2a07db8d0563a7cc4e), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "2.1J", "SCPH-3500 (Version 2.1 07/17/95 J)" ) // 22091994
	ROMX_LOAD( "ps-21j.bin",    0x0000000, 0x080000, CRC(bc190209) SHA1(e38466a4ba8005fba7e9e3c7b9efeba7205bee3f), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "2.2J", "SCPH-5000/DTL-H1200 (Version 2.2 12/04/95 J)" ) // 04121995
/*  ROMX_LOAD( "ps-22j.bad",    0x0000000, 0x080000, BAD_DUMP CRC(8c93a399) SHA1(e340db2696274dda5fdc25e434a914db71e8b02b), ROM_BIOS(4) ) */
	ROMX_LOAD( "ps-22j.bin",    0x0000000, 0x080000, CRC(24fc7e17) SHA1(ffa7f9a7fb19d773a0c3985a541c8e5623d2c30d), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "2.2D", "DTL-H1100 (Version 2.2 03/06/96 D)" ) // 04121995
	ROMX_LOAD( "ps-22d.bin",    0x0000000, 0x080000, CRC(decb22f5) SHA1(73107d468fc7cb1d2c5b18b269715dd889ecef06), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 5, "3.0J", "SCPH-5500 (Version 3.0 09/09/96 J)" ) // 04121995
	ROMX_LOAD( "ps-30j.bin",    0x0000000, 0x080000, CRC(ff3eeb8c) SHA1(b05def971d8ec59f346f2d9ac21fb742e3eb6917), ROM_BIOS(6) )

	ROM_SYSTEM_BIOS( 6, "4.0J", "SCPH-7000/SCPH-9000 (Version 4.0 08/18/97 J)" ) // 29051997
	ROMX_LOAD( "ps-40j.bin",    0x0000000, 0x080000, CRC(ec541cd0) SHA1(77b10118d21ac7ffa9b35f9c4fd814da240eb3e9), ROM_BIOS(7) )

	ROM_SYSTEM_BIOS( 7, "4.3J", "SCPH-100 (Version 4.3 03/11/00 J)" ) // 04121995
	ROMX_LOAD( "psone-43j.bin", 0x0000000, 0x080000, CRC(f2af798b) SHA1(339a48f4fcf63e10b5b867b8c93cfd40945faf6c), ROM_BIOS(8) )
ROM_END

ROM_START( psu )
	ROM_REGION32_LE( 0x080000, "user1", 0 )

	ROM_SYSTEM_BIOS( 0, "2.0A", "DTL-H1001 (Version 2.0 05/07/95 A)" ) // 22091994
	ROMX_LOAD( "ps-20a.bin",    0x0000000, 0x080000, CRC(55847d8c) SHA1(649895efd79d14790eabb362e94eb0622093dfb9), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "2.1A", "DTL-H1101 (Version 2.1 07/17/95 A)" ) // 22091994
	ROMX_LOAD( "ps-21a.bin",    0x0000000, 0x080000, CRC(aff00f2f) SHA1(ca7af30b50d9756cbd764640126c454cff658479), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "2.2A", "SCPH-1001/DTL-H1201/DTL-H3001 (Version 2.2 12/04/95 A)" ) // 04121995
	ROMX_LOAD( "ps-22a.bin",    0x0000000, 0x080000, CRC(37157331) SHA1(10155d8d6e6e832d6ea66db9bc098321fb5e8ebf), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "3.0A", "SCPH-5501/SCPH-7003 (Version 3.0 11/18/96 A)" ) // 04121995
	ROMX_LOAD( "ps-30a.bin",    0x0000000, 0x080000, CRC(8d8cb7e4) SHA1(0555c6fae8906f3f09baf5988f00e55f88e9f30b), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "4.1A", "SCPH-7001/SCPH-7501/SCPH-7503/SCPH-9001 (Version 4.1 12/16/97 A)" ) // 04121995
	ROMX_LOAD( "ps-41a.bin",    0x0000000, 0x080000, CRC(502224b6) SHA1(14df4f6c1e367ce097c11deae21566b4fe5647a9), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 5, "4.5A", "SCPH-101 (Version 4.5 05/25/00 A)" ) // 04121995
	ROMX_LOAD( "psone-45a.bin", 0x0000000, 0x080000, CRC(171bdcec) SHA1(dcffe16bd90a723499ad46c641424981338d8378), ROM_BIOS(6) )
ROM_END

ROM_START( pse )
	ROM_REGION32_LE( 0x080000, "user1", 0 )

	ROM_SYSTEM_BIOS( 0, "2.0E", "DTL-H1002/SCPH-1002 (Version 2.0 05/10/95 E)" ) // 22091994
	ROMX_LOAD( "ps-20e.bin",    0x0000000, 0x080000, CRC(9bb87c4b) SHA1(20b98f3d80f11cbf5a7bfd0779b0e63760ecc62c), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "2.1E", "SCPH-1002/DTL-H1102 (Version 2.1 07/17/95 E)" ) // 22091994
	ROMX_LOAD( "ps-21e.bin",    0x0000000, 0x080000, CRC(86c30531) SHA1(76cf6b1b2a7c571a6ad07f2bac0db6cd8f71e2cc), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "2.2E", "SCPH-1002/DTL-H1202/DTL-H3002 (Version 2.2 12/04/95 E)" ) // 04121995
	ROMX_LOAD( "ps-22e.bin",    0x0000000, 0x080000, CRC(1e26792f) SHA1(b6a11579caef3875504fcf3831b8e3922746df2c), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "3.0E", "SCPH-5502/SCPH-5552 (Version 3.0 01/06/97 E)" ) // 04121995
/*  ROMX_LOAD( "ps-30e.bad",    0x0000000, 0x080000, BAD_DUMP CRC(4d9e7c86) SHA1(f8de9325fc36fcfa4b29124d291c9251094f2e54), ROM_BIOS(4) ) */
	ROMX_LOAD( "ps-30e.bin",    0x0000000, 0x080000, CRC(d786f0b9) SHA1(f6bc2d1f5eb6593de7d089c425ac681d6fffd3f0), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "4.1E", "SCPH-7002/SCPH-7502/SCPH-9002 (Version 4.1 12/16/97 E)" ) // 04121995
	ROMX_LOAD( "ps-41e.bin",    0x0000000, 0x080000, CRC(318178bf) SHA1(8d5de56a79954f29e9006929ba3fed9b6a418c1d), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 5, "4.4E", "SCPH-102 (Version 4.4 03/24/00 E)" ) // 04121995
	ROMX_LOAD( "psone-44e.bin", 0x0000000, 0x080000, CRC(0bad7ea9) SHA1(beb0ac693c0dc26daf5665b3314db81480fa5c7c), ROM_BIOS(6) )

	ROM_SYSTEM_BIOS( 6, "4.5E", "SCPH-102 (Version 4.5 05/25/00 E)" ) // 04121995
	ROMX_LOAD( "psone-45e.bin", 0x0000000, 0x080000, CRC(76b880e5) SHA1(dbc7339e5d85827c095764fc077b41f78fd2ecae), ROM_BIOS(7) )
ROM_END

ROM_START( psa )
	ROM_REGION32_LE( 0x080000, "user1", 0 )

	ROM_SYSTEM_BIOS( 0, "3.0A", "SCPH-5501/SCPH-7003 (Version 3.0 11/18/96 A)" ) // 04121995
	ROMX_LOAD( "ps-30a.bin",    0x0000000, 0x080000, CRC(8d8cb7e4) SHA1(0555c6fae8906f3f09baf5988f00e55f88e9f30b), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "4.1A", "SCPH-7001/SCPH-7501/SCPH-7503/SCPH-9001 (Version 4.1 12/16/97 A)" ) // 04121995
	ROMX_LOAD( "ps-41a.bin",    0x0000000, 0x080000, CRC(502224b6) SHA1(14df4f6c1e367ce097c11deae21566b4fe5647a9), ROM_BIOS(2) )
ROM_END

/*
The version number & release date is stored in ascii text at the end of every bios, except for scph1000.
There is also a BCD encoded date at offset 0x100, but this is set to 22091994 for versions prior to 2.2
and 04121995 for all versions from 2.2 ( except Version 4.0J which is 29051997 ).

Consoles not dumped:

DTL-H1001H
DTL-H3000
SCPH-5001
SCPH-5002
SCPH-5003
SCPH-5503
SCPH-7500
SCPH-9003
SCPH-103

Holes in version numbers:

Version 2.0 J
Version 4.1 J
Version 4.2 J
Version 4.4 J
Version 4.5 J
Version 4.0 A
Version 4.2 A
Version 4.3 A
Version 4.4 A
Version 4.0 E
Version 4.2 E
Version 4.3 E

*/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY                             FULLNAME                            FLAGS */
CONS( 1994, psj,    0,      0,      psxntsc,    psx, psx1_state,    psx,    "Sony Computer Entertainment Inc", "Sony PlayStation (Japan)",         GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
CONS( 1995, pse,    psj,    0,      psxpal,     psx, psx1_state,    psx,    "Sony Computer Entertainment Inc", "Sony PlayStation (Europe)",        GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
CONS( 1995, psu,    psj,    0,      psxntsc,    psx, psx1_state,    psx,    "Sony Computer Entertainment Inc", "Sony PlayStation (USA)",           GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
CONS( 1995, psa,    psj,    0,      psxntsc,    psx, psx1_state,    psx,    "Sony Computer Entertainment Inc", "Sony PlayStation (Asia-Pacific)",  GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
