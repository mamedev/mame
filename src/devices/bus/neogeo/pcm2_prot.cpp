// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#include "emu.h"
#include "pcm2_prot.h"



extern const device_type PCM2_PROT = &device_creator<pcm2_prot_device>;


pcm2_prot_device::pcm2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PCM2_PROT, "NeoGeo Protection (NEOPCM2)", tag, owner, clock, "pcm2_prot", __FILE__)
{
}


void pcm2_prot_device::device_start()
{
}

void pcm2_prot_device::device_reset()
{
}

/***************************************************************************

NeoGeo 'V' (PCM) ROM encryption
  NEOPCM2 chip

***************************************************************************/

/* Neo-Pcm2 Drivers for Encrypted V Roms */
void pcm2_prot_device::neo_pcm2_snk_1999(UINT8* ymrom, UINT32 ymsize, int value)
{   /* thanks to Elsemi for the NEO-PCM2 info */
	UINT16 *rom = (UINT16 *)ymrom;
	int size = ymsize;
	int i, j;

	if( rom != nullptr )
	{   /* swap address lines on the whole ROMs */
		std::vector<UINT16> buffer(value / 2);

		for( i = 0; i < size / 2; i += ( value / 2 ) )
		{
			memcpy( &buffer[0], &rom[ i ], value );
			for( j = 0; j < (value / 2); j++ )
			{
				rom[ i + j ] = buffer[ j ^ (value/4) ];
			}
		}
	}
}


/* the later PCM2 games have additional scrambling */
void pcm2_prot_device::neo_pcm2_swap(UINT8* ymrom, UINT32 ymsize, int value)
{
	static const UINT32 addrs[7][2]={
		{0x000000,0xa5000},
		{0xffce20,0x01000},
		{0xfe2cf6,0x4e001},
		{0xffac28,0xc2000},
		{0xfeb2c0,0x0a000},
		{0xff14ea,0xa7001},
		{0xffb440,0x02000}};
	static const UINT8 xordata[7][8]={
		{0xf9,0xe0,0x5d,0xf3,0xea,0x92,0xbe,0xef},
		{0xc4,0x83,0xa8,0x5f,0x21,0x27,0x64,0xaf},
		{0xc3,0xfd,0x81,0xac,0x6d,0xe7,0xbf,0x9e},
		{0xc3,0xfd,0x81,0xac,0x6d,0xe7,0xbf,0x9e},
		{0xcb,0x29,0x7d,0x43,0xd2,0x3a,0xc2,0xb4},
		{0x4b,0xa4,0x63,0x46,0xf0,0x91,0xea,0x62},
		{0x4b,0xa4,0x63,0x46,0xf0,0x91,0xea,0x62}};

	dynamic_buffer buf(0x1000000);
	int i, j, d;
	UINT8* src = ymrom;
	memcpy(&buf[0],src,0x1000000);
	for (i=0;i<0x1000000;i++)
	{
		j=BITSWAP24(i,23,22,21,20,19,18,17,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,16);
		j=j^addrs[value][1];
		d=((i+addrs[value][0])&0xffffff);
		src[j]=buf[d]^xordata[value][j&0x7];
	}
}
