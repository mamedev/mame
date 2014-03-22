/*********************************************************

    Konami 053260 PCM Sound Chip

*********************************************************/

#include "emu.h"
#include "k053260.h"

/* 2004-02-28: Fixed PPCM decoding. Games sound much better now.*/

#define LOG 0

#define BASE_SHIFT  16


// device type definition
const device_type K053260 = &device_creator<k053260_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k053260_device - constructor
//-------------------------------------------------

k053260_device::k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053260, "K053260", tag, owner, clock, "k053260", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_mode(0),
		m_rom(NULL),
		m_rom_size(0),
		m_delta_table(NULL),
		m_intf(NULL)
{
	memset(m_regs, 0, sizeof(int)*0x30);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053260_device::device_start()
{
	static const k053260_interface defintrf = { 0 };
	int rate = clock() / 32;
	int i;

	/* Initialize our chip structure */
	m_intf = (static_config() != NULL) ? (const k053260_interface *)static_config() : &defintrf;

	m_mode = 0;

	memory_region *region = (m_intf->rgnoverride != NULL) ? memregion(m_intf->rgnoverride) : this->region();

	m_rom = *region;
	m_rom_size = region->bytes();

	device_reset();

	for ( i = 0; i < 0x30; i++ )
		m_regs[i] = 0;

	m_delta_table = auto_alloc_array( machine(), UINT32, 0x1000 );

	m_channel = stream_alloc( 0, 2, rate );

	InitDeltaTable( rate, clock() );

	/* register with the save state system */
	save_item(NAME(m_mode));
	save_item(NAME(m_regs));

	for ( i = 0; i < 4; i++ )
	{
		save_item(NAME(m_channels[i].rate), i);
		save_item(NAME(m_channels[i].size), i);
		save_item(NAME(m_channels[i].start), i);
		save_item(NAME(m_channels[i].bank), i);
		save_item(NAME(m_channels[i].volume), i);
		save_item(NAME(m_channels[i].play), i);
		save_item(NAME(m_channels[i].pan), i);
		save_item(NAME(m_channels[i].pos), i);
		save_item(NAME(m_channels[i].loop), i);
		save_item(NAME(m_channels[i].ppcm), i);
		save_item(NAME(m_channels[i].ppcm_data), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053260_device::device_reset()
{
	int i;

	for( i = 0; i < 4; i++ ) {
		m_channels[i].rate = 0;
		m_channels[i].size = 0;
		m_channels[i].start = 0;
		m_channels[i].bank = 0;
		m_channels[i].volume = 0;
		m_channels[i].play = 0;
		m_channels[i].pan = 0;
		m_channels[i].pos = 0;
		m_channels[i].loop = 0;
		m_channels[i].ppcm = 0;
		m_channels[i].ppcm_data = 0;
	}
}


INLINE int limit( int val, int max, int min )
{
	if ( val > max )
		val = max;
	else if ( val < min )
		val = min;

	return val;
}

#define MAXOUT 0x7fff
#define MINOUT -0x8000

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k053260_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	static const long dpcmcnv[] = { 0,1,2,4,8,16,32,64, -128, -64, -32, -16, -8, -4, -2, -1};

	int i, j, lvol[4], rvol[4], play[4], loop[4], ppcm_data[4], ppcm[4];
	unsigned char *rom[4];
	UINT32 delta[4], end[4], pos[4];
	int dataL, dataR;
	signed char d;

	/* precache some values */
	for ( i = 0; i < 4; i++ ) {
		rom[i]= &m_rom[m_channels[i].start + ( m_channels[i].bank << 16 )];
		delta[i] = m_delta_table[m_channels[i].rate];
		lvol[i] = m_channels[i].volume * m_channels[i].pan;
		rvol[i] = m_channels[i].volume * ( 8 - m_channels[i].pan );
		end[i] = m_channels[i].size;
		pos[i] = m_channels[i].pos;
		play[i] = m_channels[i].play;
		loop[i] = m_channels[i].loop;
		ppcm[i] = m_channels[i].ppcm;
		ppcm_data[i] = m_channels[i].ppcm_data;
		if ( ppcm[i] )
			delta[i] /= 2;
	}

		for ( j = 0; j < samples; j++ ) {
			dataL = dataR = 0;

			for ( i = 0; i < 4; i++ ) {
				/* see if the voice is on */
				if ( play[i] ) {
					/* see if we're done */
					if ( ( pos[i] >> BASE_SHIFT ) >= end[i] ) {
						ppcm_data[i] = 0;
						if ( loop[i] )
							pos[i] = 0;
						else {
							play[i] = 0;
							continue;
						}
					}

					if ( ppcm[i] ) { /* Packed PCM */
						/* we only update the signal if we're starting or a real sound sample has gone by */
						/* this is all due to the dynamic sample rate convertion */
						if ( pos[i] == 0 || ( ( pos[i] ^ ( pos[i] - delta[i] ) ) & 0x8000 ) == 0x8000 )

						{
							int newdata;
							if ( pos[i] & 0x8000 ){
								newdata = ((rom[i][pos[i] >> BASE_SHIFT]) >> 4) & 0x0f; /*high nybble*/
							}
							else{
								newdata = ( ( rom[i][pos[i] >> BASE_SHIFT] ) ) & 0x0f; /*low nybble*/
							}

							ppcm_data[i] = (( ( ppcm_data[i] * 62 ) >> 6 ) + dpcmcnv[newdata]);

							if ( ppcm_data[i] > 127 )
								ppcm_data[i] = 127;
							else
								if ( ppcm_data[i] < -128 )
									ppcm_data[i] = -128;
						}



						d = ppcm_data[i];

						pos[i] += delta[i];
					} else { /* PCM */
						d = rom[i][pos[i] >> BASE_SHIFT];

						pos[i] += delta[i];
					}

					if ( m_mode & 2 ) {
						dataL += ( d * lvol[i] ) >> 2;
						dataR += ( d * rvol[i] ) >> 2;
					}
				}
			}

			outputs[1][j] = limit( dataL, MAXOUT, MINOUT );
			outputs[0][j] = limit( dataR, MAXOUT, MINOUT );
		}

	/* update the regs now */
	for ( i = 0; i < 4; i++ ) {
		m_channels[i].pos = pos[i];
		m_channels[i].play = play[i];
		m_channels[i].ppcm_data = ppcm_data[i];
	}
}


void k053260_device::InitDeltaTable( int rate, int clock )
{
	int     i;
	double  base = ( double )rate;
	double  max = (double)(clock); /* Hz */
	UINT32 val;

	for( i = 0; i < 0x1000; i++ ) {
		double v = ( double )( 0x1000 - i );
		double target = (max) / v;
		double fixed = ( double )( 1 << BASE_SHIFT );

		if ( target && base ) {
			target = fixed / ( base / target );
			val = ( UINT32 )target;
			if ( val == 0 )
				val = 1;
		} else
			val = 1;

		m_delta_table[i] = val;
	}
}


void k053260_device::check_bounds( int channel )
{
	int channel_start = ( m_channels[channel].bank << 16 ) + m_channels[channel].start;
	int channel_end = channel_start + m_channels[channel].size - 1;

	if ( channel_start > m_rom_size ) {
		logerror("K53260: Attempting to start playing past the end of the ROM ( start = %06x, end = %06x ).\n", channel_start, channel_end );

		m_channels[channel].play = 0;

		return;
	}

	if ( channel_end > m_rom_size ) {
		logerror("K53260: Attempting to play past the end of the ROM ( start = %06x, end = %06x ).\n", channel_start, channel_end );

		m_channels[channel].size = m_rom_size - channel_start;
	}
	if (LOG) logerror("K053260: Sample Start = %06x, Sample End = %06x, Sample rate = %04x, PPCM = %s\n", channel_start, channel_end, m_channels[channel].rate, m_channels[channel].ppcm ? "yes" : "no" );
}


WRITE8_MEMBER( k053260_device::k053260_w )
{
	int i, t;
	int r = offset;
	int v = data;

	if ( r > 0x2f ) {
		logerror("K053260: Writing past registers\n" );
		return;
	}

		m_channel->update();

	/* before we update the regs, we need to check for a latched reg */
	if ( r == 0x28 ) {
		t = m_regs[r] ^ v;

		for ( i = 0; i < 4; i++ ) {
			if ( t & ( 1 << i ) ) {
				if ( v & ( 1 << i ) ) {
					m_channels[i].play = 1;
					m_channels[i].pos = 0;
					m_channels[i].ppcm_data = 0;
					check_bounds( i );
				} else
					m_channels[i].play = 0;
			}
		}

		m_regs[r] = v;
		return;
	}

	/* update regs */
	m_regs[r] = v;

	/* communication registers */
	if ( r < 8 )
		return;

	/* channel setup */
	if ( r < 0x28 ) {
		int channel = ( r - 8 ) / 8;

		switch ( ( r - 8 ) & 0x07 ) {
			case 0: /* sample rate low */
				m_channels[channel].rate &= 0x0f00;
				m_channels[channel].rate |= v;
			break;

			case 1: /* sample rate high */
				m_channels[channel].rate &= 0x00ff;
				m_channels[channel].rate |= ( v & 0x0f ) << 8;
			break;

			case 2: /* size low */
				m_channels[channel].size &= 0xff00;
				m_channels[channel].size |= v;
			break;

			case 3: /* size high */
				m_channels[channel].size &= 0x00ff;
				m_channels[channel].size |= v << 8;
			break;

			case 4: /* start low */
				m_channels[channel].start &= 0xff00;
				m_channels[channel].start |= v;
			break;

			case 5: /* start high */
				m_channels[channel].start &= 0x00ff;
				m_channels[channel].start |= v << 8;
			break;

			case 6: /* bank */
				m_channels[channel].bank = v & 0xff;
			break;

			case 7: /* volume is 7 bits. Convert to 8 bits now. */
				m_channels[channel].volume = ( ( v & 0x7f ) << 1 ) | ( v & 1 );
			break;
		}

		return;
	}

	switch( r ) {
		case 0x2a: /* loop, ppcm */
			for ( i = 0; i < 4; i++ )
				m_channels[i].loop = ( v & ( 1 << i ) ) != 0;

			for ( i = 4; i < 8; i++ )
				m_channels[i-4].ppcm = ( v & ( 1 << i ) ) != 0;
		break;

		case 0x2c: /* pan */
			m_channels[0].pan = v & 7;
			m_channels[1].pan = ( v >> 3 ) & 7;
		break;

		case 0x2d: /* more pan */
			m_channels[2].pan = v & 7;
			m_channels[3].pan = ( v >> 3 ) & 7;
		break;

		case 0x2f: /* control */
			m_mode = v & 7;
			/* bit 0 = read ROM */
			/* bit 1 = enable sound output */
			/* bit 2 = unknown */
		break;
	}
}

READ8_MEMBER( k053260_device::k053260_r )
{
	switch ( offset ) {
		case 0x29: /* channel status */
			{
				int i, status = 0;

				for ( i = 0; i < 4; i++ )
					status |= m_channels[i].play << i;

				return status;
			}
		break;

		case 0x2e: /* read ROM */
			if ( m_mode & 1 )
			{
				UINT32 offs = m_channels[0].start + ( m_channels[0].pos >> BASE_SHIFT ) + ( m_channels[0].bank << 16 );

				m_channels[0].pos += ( 1 << 16 );

				if ( offs > m_rom_size ) {
					logerror("%s: K53260: Attempting to read past ROM size in ROM Read Mode (offs = %06x, size = %06x).\n", machine().describe_context(),offs,m_rom_size );

					return 0;
				}

				return m_rom[offs];
			}
		break;
	}

	return m_regs[offset];
}
