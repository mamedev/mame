/***************************************************************************

    PSX SPU

    CXD2922BQ/CXD2925Q

    preliminary version by smf.

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "psx.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

#define MAX_CHANNEL ( 24 )
#define SPU_RAM_SIZE ( 512 * 1024 )
#define SAMPLES_PER_BLOCK ( 28 )
#define PITCH_SHIFT ( 12 )

typedef enum { e_attack = 0, e_decay, e_sustain, e_sustainEnd, e_release, e_releaseend } sound_envstate;

struct psxinfo
{
	const psx_spu_interface *intf;
	device_t *device;

	UINT32 *m_p_n_psxram;
	UINT16 m_n_mainvolumeleft;
	UINT16 m_n_mainvolumeright;
	UINT16 m_n_reverberationdepthleft;
	UINT16 m_n_reverberationdepthright;
	UINT32 m_n_voiceon;
	UINT32 m_n_voiceoff;
	UINT32 m_n_modulationmode;
	UINT32 m_n_noisemode;
	UINT32 m_n_reverbmode;
	UINT32 m_n_channelonoff;
	UINT16 m_n_reverbworkareastart;
	UINT16 m_n_irqaddress;
	UINT32 m_n_spuoffset;
	UINT16 m_n_spudata;
	UINT16 m_n_spucontrol;
	UINT32 m_n_spustatus;
	UINT16 m_n_cdvolumeleft;
	UINT16 m_n_cdvolumeright;
	UINT16 m_n_externalvolumeleft;
	UINT16 m_n_externalvolumeright;
	UINT16 m_p_n_volumeleft[ MAX_CHANNEL ];
	UINT16 m_p_n_volumeright[ MAX_CHANNEL ];
	UINT16 m_p_n_pitch[ MAX_CHANNEL ];
	UINT16 m_p_n_address[ MAX_CHANNEL ];
	UINT16 m_p_n_envelopestate[ MAX_CHANNEL ];
	sound_envstate m_envstate;
	UINT16 m_p_n_attackdecaysustain[ MAX_CHANNEL ];
	UINT16 m_p_n_sustainrelease[ MAX_CHANNEL ];
	UINT16 m_p_n_adsrvolume[ MAX_CHANNEL ];
	UINT16 m_p_n_repeataddress[ MAX_CHANNEL ];
	UINT32 m_p_n_effect[ 16 ];
	UINT16 *m_p_n_spuram;
	UINT32 m_p_n_blockaddress[ MAX_CHANNEL ];
	UINT32 m_p_n_blockoffset[ MAX_CHANNEL ];
	UINT32 m_p_n_blockstatus[ MAX_CHANNEL ];
	INT16 m_p_n_blockbuffer[ MAX_CHANNEL * SAMPLES_PER_BLOCK ];
	INT16 m_p_n_s1[ MAX_CHANNEL ];
	INT16 m_p_n_s2[ MAX_CHANNEL ];
	UINT32 m_n_loop[ MAX_CHANNEL ];

	sound_stream *stream;
	int installHack;
	UINT32 m_n_spu_delay;
};

INLINE struct psxinfo *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PSXSPU);
	return (struct psxinfo *)downcast<legacy_device_base *>(device)->token();
}


#define SPU_REG( a ) ( ( a - 0xc00 ) / 4 )
#define SPU_CHANNEL_REG( a ) ( a / 4 )

INLINE int volume( UINT16 n_volume )
{
	if( ( n_volume & 0x8000 ) != 0 )
	{
		n_volume = ( n_volume & 0x7f ) * 0x80;
	}
	else if( ( n_volume & 0x4000 ) != 0 )
	{
		n_volume = -( n_volume & 0x3fff );
	}
	return n_volume;
}

INLINE int limit( int v )
{
	if( v < -32768 )
	{
		return -32768;
	}
	else if( v > 32767 )
	{
		return 32767;
	}
	return v;
}

static STREAM_UPDATE( PSXSPU_update )
{
	struct psxinfo *chip = (struct psxinfo *)param;
	int v;
	int voll;
	int volr;
	int n_channel;
	int n_sample;
	int n_word;
	int n_shift;
	int n_predict;
	int n_flags;
	int n_nibble;
	int n_packed;
	int n_unpacked;

	static const int f[ 5 ][ 2 ] =
	{
		{   0,   0  },
		{  60,   0  },
		{ 115, -52 },
		{  98, -55 },
		{ 122, -60 }
	};

	memset( outputs[ 0 ], 0, samples * sizeof( *outputs[ 0 ] ));
	memset( outputs[ 1 ], 0, samples * sizeof( *outputs[ 1 ] ));

	for( n_channel = 0; n_channel < MAX_CHANNEL; n_channel++ )
	{
		// hack, if the envelope is in release state, silence it
		// - the envelopes aren't currently emulated!
		// - and this prevents audiable sounds looping forever
		if (chip->m_p_n_envelopestate[ n_channel ] == e_release )
		{
			voll = 0;
			volr = 0;
		}
		else
		{
			voll = volume( chip->m_p_n_volumeleft[ n_channel ] );
			volr = volume( chip->m_p_n_volumeright[ n_channel ] );
		}

		for( n_sample = 0; n_sample < samples; n_sample++ )
		{
			if( chip->m_p_n_blockoffset[ n_channel ] >= ( SAMPLES_PER_BLOCK << PITCH_SHIFT ) )
			{
				if( !chip->m_p_n_blockstatus[ n_channel ] )
				{
					break;
				}
				if( ( chip->m_n_spucontrol & 0x40 ) != 0 &&
					( chip->m_n_irqaddress * 4 ) >= chip->m_p_n_blockaddress[ n_channel ] &&
					( chip->m_n_irqaddress * 4 ) <= chip->m_p_n_blockaddress[ n_channel ] + 7 )
				{
					chip->intf->irq_set( chip->device, 0x0200 );
				}

				n_shift =   ( chip->m_p_n_spuram[ chip->m_p_n_blockaddress[ n_channel ] ] >> 0 ) & 0x0f;
				n_predict = ( chip->m_p_n_spuram[ chip->m_p_n_blockaddress[ n_channel ] ] >> 4 ) & 0x0f;
				n_flags =   ( chip->m_p_n_spuram[ chip->m_p_n_blockaddress[ n_channel ] ] >> 8 ) & 0xff;
				if( ( n_flags & 4 ) != 0 )
				{
					chip->m_n_loop[ n_channel ] = chip->m_p_n_blockaddress[ n_channel ];
				}
				chip->m_p_n_blockaddress[ n_channel ]++;
				chip->m_p_n_blockaddress[ n_channel ] %= ( SPU_RAM_SIZE / 2 );

				for( n_word = 0; n_word < 7; n_word++ )
				{
					n_packed = chip->m_p_n_spuram[ chip->m_p_n_blockaddress[ n_channel ] ];
					chip->m_p_n_blockaddress[ n_channel ]++;
					chip->m_p_n_blockaddress[ n_channel ] %= ( SPU_RAM_SIZE / 2 );

					for( n_nibble = 0; n_nibble < 4; n_nibble++ )
					{
						n_unpacked = ( ( n_packed & 0xf ) << 12 );
						if( ( n_unpacked & 0x8000 ) != 0 )
						{
							n_unpacked |= 0xffff0000;
						}
						n_unpacked = ( n_unpacked >> n_shift ) +
							( ( chip->m_p_n_s1[ n_channel ] * f[ n_predict ][ 0 ] ) >> 6 ) +
							( ( chip->m_p_n_s2[ n_channel ] * f[ n_predict ][ 1 ] ) >> 6 );
						chip->m_p_n_s2[ n_channel ] = chip->m_p_n_s1[ n_channel ];
						chip->m_p_n_s1[ n_channel ] = n_unpacked;

						chip->m_p_n_blockbuffer[ ( n_channel * SAMPLES_PER_BLOCK ) + ( n_word * 4 ) + n_nibble ] = n_unpacked;
						n_packed >>= 4;
					}
				}
				if( ( n_flags & 1 ) != 0 )
				{
					if( n_flags != 3 )
					{
						chip->m_p_n_blockstatus[ n_channel ] = 0;
					}
					else
					{
						chip->m_p_n_blockaddress[ n_channel ] = chip->m_n_loop[ n_channel ];
					}
				}
				chip->m_p_n_blockoffset[ n_channel ] %= ( SAMPLES_PER_BLOCK << PITCH_SHIFT );
			}
			v = chip->m_p_n_blockbuffer[ ( n_channel * SAMPLES_PER_BLOCK ) + ( chip->m_p_n_blockoffset[ n_channel ] >> PITCH_SHIFT ) ];
			chip->m_p_n_blockoffset[ n_channel ] += chip->m_p_n_pitch[ n_channel ];
			outputs[ 0 ][ n_sample ] = limit( outputs[ 0 ][ n_sample ] + ( ( v * voll ) / 0x4000 ) );
			outputs[ 1 ][ n_sample ] = limit( outputs[ 1 ][ n_sample ] + ( ( v * volr ) / 0x4000 ) );
		}
	}
}

static void spu_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	struct psxinfo *chip = get_safe_token(machine->device("spu"));
	verboselog( machine, 1, "spu_read( %08x, %08x )\n", n_address, n_size );

	while( n_size > 0 )
	{
		chip->m_p_n_psxram[ n_address / 4 ] =
			( chip->m_p_n_spuram[ chip->m_n_spuoffset + 0 ] << 0 ) |
			( chip->m_p_n_spuram[ chip->m_n_spuoffset + 1 ] << 16 );
		verboselog( machine, 2, "%08x > %04x\n", chip->m_n_spuoffset + 0, chip->m_p_n_spuram[ chip->m_n_spuoffset + 0 ] );
		verboselog( machine, 2, "%08x > %04x\n", chip->m_n_spuoffset + 1, chip->m_p_n_spuram[ chip->m_n_spuoffset + 1 ] );
		chip->m_n_spuoffset += 2;
		chip->m_n_spuoffset %= ( SPU_RAM_SIZE / 2 );
		n_address += 4;
		n_size--;
	}
}

static void spu_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	struct psxinfo *chip = get_safe_token(machine->device("spu"));
	verboselog( machine, 1, "spu_write( %08x, %08x )\n", n_address, n_size );

	while( n_size > 0 )
	{
		chip->m_p_n_spuram[ chip->m_n_spuoffset + 0 ] = ( chip->m_p_n_psxram[ n_address / 4 ] >> 0 );
		chip->m_p_n_spuram[ chip->m_n_spuoffset + 1 ] = ( chip->m_p_n_psxram[ n_address / 4 ] >> 16 );
		verboselog( machine, 2, "%08x < %04x\n", chip->m_n_spuoffset + 0, chip->m_p_n_spuram[ chip->m_n_spuoffset + 0 ] );
		verboselog( machine, 2, "%08x < %04x\n", chip->m_n_spuoffset + 1, chip->m_p_n_spuram[ chip->m_n_spuoffset + 1 ] );
		chip->m_n_spuoffset += 2;
		chip->m_n_spuoffset %= ( SPU_RAM_SIZE / 2 );
		n_address += 4;
		n_size--;
	}
}

static DEVICE_START( psxspu )
{
	struct psxinfo *chip = get_safe_token(device);
	int n_effect;
	int n_channel;

	chip->intf = (const psx_spu_interface *)device->baseconfig().static_config();
	chip->device = device;
	chip->m_p_n_psxram = (UINT32 *)memory_get_shared(*device->machine, "share1");

	chip->m_n_mainvolumeleft = 0;
	chip->m_n_mainvolumeright = 0;
	chip->m_n_reverberationdepthleft = 0;
	chip->m_n_reverberationdepthright = 0;
	chip->m_n_voiceon = 0;
	chip->m_n_voiceoff = 0;
	chip->m_n_modulationmode = 0;
	chip->m_n_noisemode = 0;
	chip->m_n_reverbmode = 0;
	chip->m_n_channelonoff = 0;
	chip->m_n_reverbworkareastart = 0;
	chip->m_n_irqaddress = 0;
	chip->m_n_spuoffset = 0;
	chip->m_n_spudata = 0;
	chip->m_n_spucontrol = 0;
	chip->m_n_spustatus = 0;
	chip->m_n_cdvolumeleft = 0;
	chip->m_n_cdvolumeright = 0;
	chip->m_n_externalvolumeleft = 0;
	chip->m_n_externalvolumeright = 0;

	for( n_channel = 0; n_channel < MAX_CHANNEL; n_channel++ )
	{
		chip->m_p_n_volumeleft[ n_channel ] = 0;
		chip->m_p_n_volumeright[ n_channel ] = 0;
		chip->m_p_n_pitch[ n_channel ] = 0;
		chip->m_p_n_address[ n_channel ] = 0;
		chip->m_p_n_attackdecaysustain[ n_channel ] = 0;
		chip->m_p_n_sustainrelease[ n_channel ] = 0;
		chip->m_p_n_adsrvolume[ n_channel ] = 0;
		chip->m_p_n_repeataddress[ n_channel ] = 0;
		chip->m_p_n_blockaddress[ n_channel ] = 0;
		chip->m_p_n_blockoffset[ n_channel ] = 0;
		chip->m_p_n_blockstatus[ n_channel ] = 0;
		{
			char s[ 1024 ];
			sprintf( s, "SPU%d", n_channel );
		}
	}

	for( n_effect = 0; n_effect < 16; n_effect++ )
	{
		chip->m_p_n_effect[ n_effect ] = 0;
	}

	chip->m_p_n_spuram = auto_alloc_array( device->machine, UINT16, SPU_RAM_SIZE/2 );

	state_save_register_device_item( device, 0, chip->m_n_mainvolumeleft );
	state_save_register_device_item( device, 0, chip->m_n_mainvolumeright );
	state_save_register_device_item( device, 0, chip->m_n_reverberationdepthleft );
	state_save_register_device_item( device, 0, chip->m_n_reverberationdepthright );
	state_save_register_device_item( device, 0, chip->m_n_voiceon );
	state_save_register_device_item( device, 0, chip->m_n_voiceoff );
	state_save_register_device_item( device, 0, chip->m_n_modulationmode );
	state_save_register_device_item( device, 0, chip->m_n_noisemode );
	state_save_register_device_item( device, 0, chip->m_n_reverbmode );
	state_save_register_device_item( device, 0, chip->m_n_channelonoff );
	state_save_register_device_item( device, 0, chip->m_n_reverbworkareastart );
	state_save_register_device_item( device, 0, chip->m_n_irqaddress );
	state_save_register_device_item( device, 0, chip->m_n_spuoffset );
	state_save_register_device_item( device, 0, chip->m_n_spudata );
	state_save_register_device_item( device, 0, chip->m_n_spucontrol );
	state_save_register_device_item( device, 0, chip->m_n_spustatus );
	state_save_register_device_item( device, 0, chip->m_n_cdvolumeleft );
	state_save_register_device_item( device, 0, chip->m_n_cdvolumeright );
	state_save_register_device_item( device, 0, chip->m_n_externalvolumeleft );
	state_save_register_device_item( device, 0, chip->m_n_externalvolumeright );
	state_save_register_device_item_array( device, 0, chip->m_p_n_volumeleft );
	state_save_register_device_item_array( device, 0, chip->m_p_n_volumeright );
	state_save_register_device_item_array( device, 0, chip->m_p_n_pitch );
	state_save_register_device_item_array( device, 0, chip->m_p_n_address );
	state_save_register_device_item_array( device, 0, chip->m_p_n_envelopestate );
	state_save_register_device_item_array( device, 0, chip->m_p_n_attackdecaysustain );
	state_save_register_device_item_array( device, 0, chip->m_p_n_sustainrelease );
	state_save_register_device_item_array( device, 0, chip->m_p_n_adsrvolume );
	state_save_register_device_item_array( device, 0, chip->m_p_n_repeataddress );
	state_save_register_device_item_array( device, 0, chip->m_p_n_effect );
	state_save_register_device_item_pointer( device, 0, chip->m_p_n_spuram, SPU_RAM_SIZE/2 );
	state_save_register_device_item_array( device, 0, chip->m_p_n_blockaddress );
	state_save_register_device_item_array( device, 0, chip->m_p_n_blockoffset );
	state_save_register_device_item_array( device, 0, chip->m_p_n_blockstatus );
	state_save_register_device_item_array( device, 0, chip->m_p_n_blockbuffer );
	state_save_register_device_item_array( device, 0, chip->m_p_n_s1 );
	state_save_register_device_item_array( device, 0, chip->m_p_n_s2 );
	state_save_register_device_item_array( device, 0, chip->m_n_loop );

	chip->installHack = 0;

	chip->stream = stream_create( device, 0, 2, 44100, chip, PSXSPU_update );
}


WRITE32_DEVICE_HANDLER( psx_spu_delay_w )
{
	struct psxinfo *chip = get_safe_token(device);

	COMBINE_DATA( &chip->m_n_spu_delay );
	verboselog( device->machine, 1, "psx_spu_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_DEVICE_HANDLER( psx_spu_delay_r )
{
	struct psxinfo *chip = get_safe_token(device);

	verboselog( device->machine, 1, "psx_spu_delay_r( %08x )\n", mem_mask );
	return chip->m_n_spu_delay;
}

READ32_DEVICE_HANDLER( psx_spu_r )
{
	struct psxinfo *chip = get_safe_token(device);
	running_machine *machine = device->machine;
	int n_channel;
	n_channel = offset / 4;
	if( n_channel < MAX_CHANNEL )
	{
		switch( offset % 4 )
		{
		case SPU_CHANNEL_REG( 0x8 ):
			if( ACCESSING_BITS_0_15 )
			{
				verboselog( machine, 1, "psx_spu_r() channel %d attack/decay/sustain = %04x\n", n_channel, chip->m_p_n_attackdecaysustain[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				verboselog( machine, 1, "psx_spu_r() channel %d sustain/release = %04x\n", n_channel, chip->m_p_n_sustainrelease[ n_channel ] );
			}
			return ( chip->m_p_n_sustainrelease[ n_channel ] << 16 ) | chip->m_p_n_attackdecaysustain[ n_channel ];
		case SPU_CHANNEL_REG( 0xc ):
			if( ACCESSING_BITS_0_15 )
			{
				verboselog( machine, 1, "psx_spu_r() channel %d adsr volume = %04x\n", n_channel, chip->m_p_n_adsrvolume[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				verboselog( machine, 1, "psx_spu_r() channel %d repeat address = %04x\n", n_channel, chip->m_p_n_repeataddress[ n_channel ] );
			}
			return ( chip->m_p_n_repeataddress[ n_channel ] << 16 ) | chip->m_p_n_adsrvolume[ n_channel ];
		default:
			verboselog( machine, 0, "psx_spu_r( %08x, %08x ) channel %d reg %d\n", offset, mem_mask, n_channel, offset % 4 );
			return 0;
		}
	}
	else
	{
		switch( offset )
		{
		case SPU_REG( 0xd88 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) voice on = %08x\n", mem_mask, chip->m_n_voiceon );
			return chip->m_n_voiceon;
		case SPU_REG( 0xd8c ):
			verboselog( machine, 1, "psx_spu_r( %08x ) voice off = %08x\n", mem_mask, chip->m_n_voiceoff );
			return chip->m_n_voiceoff;
		case SPU_REG( 0xd90 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) modulation mode = %08x\n", mem_mask, chip->m_n_modulationmode );
			return chip->m_n_modulationmode;
		case SPU_REG( 0xd94 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) noise mode = %08x\n", mem_mask, chip->m_n_noisemode );
			return chip->m_n_noisemode;
		case SPU_REG( 0xd98 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) reverb mode = %08x\n", mem_mask, chip->m_n_reverbmode );
			return chip->m_n_reverbmode;
		case SPU_REG( 0xda4 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) dma/irq address = %08x\n", mem_mask, ( ( chip->m_n_spuoffset / 4 ) << 16 ) | chip->m_n_irqaddress );
			return ( ( chip->m_n_spuoffset / 4 ) << 16 ) | chip->m_n_irqaddress;
		case SPU_REG( 0xda8 ):
			verboselog( machine, 1, "psx_spu_r( %08x ) spu control/data = %08x\n", mem_mask, ( chip->m_n_spucontrol << 16 ) | chip->m_n_spudata );
			return chip->m_n_spudata | ( chip->m_n_spucontrol << 16 );
		case SPU_REG( 0xdac ):
			verboselog( machine, 1, "psx_spu_r( %08x ) spu status = %08x\n", mem_mask, chip->m_n_spustatus );
			return chip->m_n_spustatus;
		default:
			verboselog( machine, 0, "psx_spu_r( %08x, %08x ) %08x\n", offset, mem_mask, 0xc00 + ( offset * 4 ) );
			return 0;
		}
	}
}

WRITE32_DEVICE_HANDLER( psx_spu_w )
{
	struct psxinfo *chip = get_safe_token(device);
	running_machine *machine = device->machine;
	int n_channel;
	n_channel = offset / 4;

	if( !chip->installHack )
	{
		chip->intf->spu_install_read_handler( device->machine, 4, spu_read );
		chip->intf->spu_install_write_handler( device->machine, 4, spu_write );

		chip->installHack = 1;
	}

	if( n_channel < MAX_CHANNEL )
	{
		switch( offset % 4 )
		{
		case SPU_CHANNEL_REG( 0x0 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_p_n_volumeleft[ n_channel ] = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() channel %d volume left = %04x\n", n_channel, chip->m_p_n_volumeleft[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_p_n_volumeright[ n_channel ] = data >> 16;
				verboselog( machine, 1, "psx_spu_w() channel %d volume right = %04x\n", n_channel, chip->m_p_n_volumeright[ n_channel ] );
			}
			break;
		case SPU_CHANNEL_REG( 0x4 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_p_n_pitch[ n_channel ] = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() channel %d pitch = %04x\n", n_channel, chip->m_p_n_pitch[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_p_n_address[ n_channel ] = data >> 16;
				verboselog( machine, 1, "psx_spu_w() channel %d address = %04x\n", n_channel, chip->m_p_n_address[ n_channel ] );
			}
			break;
		case SPU_CHANNEL_REG( 0x8 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_p_n_attackdecaysustain[ n_channel ] = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() channel %d attack/decay/sustain = %04x\n", n_channel, chip->m_p_n_attackdecaysustain[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_p_n_sustainrelease[ n_channel ] = data >> 16;
				verboselog( machine, 1, "psx_spu_w() channel %d sustain/release = %04x\n", n_channel, chip->m_p_n_sustainrelease[ n_channel ] );
			}
			break;
		case SPU_CHANNEL_REG( 0xc ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_p_n_adsrvolume[ n_channel ] = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() channel %d adsr volume = %04x\n", n_channel, chip->m_p_n_adsrvolume[ n_channel ] );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_p_n_repeataddress[ n_channel ] = data >> 16;
				verboselog( machine, 1, "psx_spu_w() channel %d repeat address = %04x\n", n_channel, chip->m_p_n_repeataddress[ n_channel ] );
			}
			break;
		default:
			verboselog( machine, 0, "psx_spu_w( %08x, %08x, %08x ) channel %d reg %d\n", offset, mem_mask, data, n_channel, offset % 4 );
			break;
		}
	}
	else
	{
		switch( offset )
		{
		case SPU_REG( 0xd80 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_mainvolumeleft = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() main volume left = %04x\n", chip->m_n_mainvolumeleft );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_mainvolumeright = data >> 16;
				verboselog( machine, 1, "psx_spu_w() main volume right = %04x\n", chip->m_n_mainvolumeright );
			}
			break;
		case SPU_REG( 0xd84 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_reverberationdepthleft = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() reverberation depth left = %04x\n", chip->m_n_reverberationdepthleft );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_reverberationdepthright = data >> 16;
				verboselog( machine, 1, "psx_spu_w() reverberation depth right = %04x\n", chip->m_n_reverberationdepthright );
			}
			break;
		case SPU_REG( 0xd88 ):
			chip->m_n_voiceon = 0;
			COMBINE_DATA( &chip->m_n_voiceon );
			verboselog( machine, 1, "psx_spu_w() voice on = %08x\n", chip->m_n_voiceon );
			for( n_channel = 0; n_channel < MAX_CHANNEL; n_channel++ )
			{
				if( ( chip->m_n_voiceon & ( 1 << n_channel ) ) != 0 )
				{
					chip->m_p_n_blockaddress[ n_channel ] = ( chip->m_p_n_address[ n_channel ] * 4 ) % ( SPU_RAM_SIZE / 2 );
					chip->m_p_n_blockoffset[ n_channel ] = ( SAMPLES_PER_BLOCK << PITCH_SHIFT );
					chip->m_p_n_s1[ n_channel ] = 0;
					chip->m_p_n_s2[ n_channel ] = 0;
					chip->m_p_n_blockstatus[ n_channel ] = 1;

					chip->m_p_n_envelopestate[ n_channel ] = e_attack;
				}
			}
			break;
		case SPU_REG( 0xd8c ):
			chip->m_n_voiceoff = 0;
			COMBINE_DATA( &chip->m_n_voiceoff );
			verboselog( machine, 1, "psx_spu_w() voice off = %08x\n", chip->m_n_voiceoff );

			for( n_channel = 0; n_channel < MAX_CHANNEL; n_channel++ )
			{
				if( ( chip->m_n_voiceoff & ( 1 << n_channel ) ) != 0 )
				{
					// keyoff advances the envelope to release state
					if (chip->m_p_n_envelopestate[ n_channel ] < e_release)
						chip->m_p_n_envelopestate[ n_channel ] = e_release;
				}
			}

			break;
		case SPU_REG( 0xd90 ):
			COMBINE_DATA( &chip->m_n_modulationmode );
			verboselog( machine, 1, "psx_spu_w() modulation mode = %08x\n", chip->m_n_modulationmode );
			break;
		case SPU_REG( 0xd94 ):
			COMBINE_DATA( &chip->m_n_noisemode );
			verboselog( machine, 1, "psx_spu_w() noise mode = %08x\n", chip->m_n_noisemode );
			break;
		case SPU_REG( 0xd98 ):
			COMBINE_DATA( &chip->m_n_reverbmode );
			verboselog( machine, 1, "psx_spu_w() reverb mode = %08x\n", chip->m_n_reverbmode );
			break;
		case SPU_REG( 0xd9c ):
			COMBINE_DATA( &chip->m_n_channelonoff );
			verboselog( machine, 1, "psx_spu_w() channel on/off = %08x\n", chip->m_n_channelonoff );
			break;
		case SPU_REG( 0xda0 ):
			if( ACCESSING_BITS_0_15 )
			{
				verboselog( machine, 0, "psx_spu_w( %08x, %08x, %08x ) %08x\n", offset, mem_mask, data, 0xc00 + ( offset * 4 ) );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_reverbworkareastart = data >> 16;
				verboselog( machine, 1, "psx_spu_w() reverb work area start = %04x\n", chip->m_n_reverbworkareastart );
			}
			break;
		case SPU_REG( 0xda4 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_irqaddress = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() irq address = %04x\n", chip->m_n_irqaddress );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_spuoffset = ( data >> 16 ) * 4;
				chip->m_n_spuoffset %= ( SPU_RAM_SIZE / 2 );
				verboselog( machine, 1, "psx_spu_w() spu offset = %04x\n", chip->m_n_spuoffset );
			}
			break;
		case SPU_REG( 0xda8 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_spudata = data & 0xffff;
				chip->m_p_n_spuram[ chip->m_n_spuoffset++ ] = chip->m_n_spudata;
				chip->m_n_spuoffset %= ( SPU_RAM_SIZE / 2 );
				verboselog( machine, 1, "psx_spu_w() spu data = %04x\n", chip->m_n_spudata );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_spucontrol = data >> 16;
				verboselog( machine, 1, "psx_spu_w() spu control = %04x\n", chip->m_n_spucontrol );
			}
			break;
		case SPU_REG( 0xdac ):
			COMBINE_DATA( &chip->m_n_spustatus );
			chip->m_n_spustatus &= 0xf800ffff;
			verboselog( machine, 1, "psx_spu_w() spu status = %08x\n", chip->m_n_spustatus );
			break;
		case SPU_REG( 0xdb0 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_cdvolumeleft = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() cd volume left = %04x\n", chip->m_n_cdvolumeleft );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_cdvolumeright = data >> 16;
				verboselog( machine, 1, "psx_spu_w() cd volume right = %04x\n", chip->m_n_cdvolumeright );
			}
			break;
		case SPU_REG( 0xdb4 ):
			if( ACCESSING_BITS_0_15 )
			{
				chip->m_n_externalvolumeleft = data & 0xffff;
				verboselog( machine, 1, "psx_spu_w() external volume left = %04x\n", chip->m_n_externalvolumeleft );
			}
			if( ACCESSING_BITS_16_31 )
			{
				chip->m_n_externalvolumeright = data >> 16;
				verboselog( machine, 1, "psx_spu_w() external volume right = %04x\n", chip->m_n_externalvolumeright );
			}
			break;
		case SPU_REG( 0xdc0 ):
		case SPU_REG( 0xdc4 ):
		case SPU_REG( 0xdc8 ):
		case SPU_REG( 0xdcc ):
		case SPU_REG( 0xdd0 ):
		case SPU_REG( 0xdd4 ):
		case SPU_REG( 0xdd8 ):
		case SPU_REG( 0xddc ):
		case SPU_REG( 0xde0 ):
		case SPU_REG( 0xde4 ):
		case SPU_REG( 0xde8 ):
		case SPU_REG( 0xdec ):
		case SPU_REG( 0xdf0 ):
		case SPU_REG( 0xdf4 ):
		case SPU_REG( 0xdf8 ):
		case SPU_REG( 0xdfc ):
			COMBINE_DATA( &chip->m_p_n_effect[ offset & 0x0f ] );
			verboselog( machine, 1, "psx_spu_w() effect %d = %04x\n", offset & 0x0f, chip->m_p_n_effect[ offset & 0x0f ] );
			break;
		default:
			verboselog( machine, 0, "psx_spu_w( %08x, %08x, %08x ) %08x\n", offset, mem_mask, data, 0xc00 + ( offset * 4 ) );
			break;
		}
	}
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( psxspu )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct psxinfo);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( psxspu );			break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SPU");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Sony custom");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(PSXSPU, psxspu);
