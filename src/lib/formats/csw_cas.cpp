// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes
/*
CSW format
----------
Header Description

Offset  Value   Type    Description
0x00    (note)  ASCII   22 bytes "Compressed Square Wave"  signature
0x16    0x1A    BYTE    Terminator code
0x17    0x02    BYTE    CSW major revision number
0x18    0x00    BYTE    CSW minor revision number
0x19            DWORD   Sample rate
0x1D            DWORD   Total number of pulses (after decompression)
0x21            BYTE    Compression type  0x01: RLE    0x02: Z-RLE
0x22            BYTE    Flags   b0: initial polarity: if set, the signal starts at logical high
0x23    HDR     BYTE    Header extension length in bytes (0x00)
0x24            ASCII   16 bytes free use
0x34            BYTE    Start of Header is HDR>0



*/

#include <cstring>

#include <zlib.h>
#include <cassert>
#include "uef_cas.h"
#include "csw_cas.h"


#define CSW_WAV_FREQUENCY   44100

static const uint8_t CSW_HEADER[] = { "Compressed Square Wave" };

static uint32_t get_leuint32(const void *ptr)
{
	uint32_t value;
	memcpy(&value, ptr, sizeof(value));
	return little_endianize_int32(value);
}

static int mycaslen;

static int csw_cas_to_wav_size( const uint8_t *casdata, int caslen )
{
	uint8_t  MajorRevision;
	uint8_t  MinorRevision;
	uint8_t  HeaderExtensionLength;
	std::vector<uint8_t> gz_ptr;

	int         total_size;
	z_stream    d_stream;
	int         err;
	uint8_t       *in_ptr;
	int         bsize=0;

	if ( memcmp( casdata, CSW_HEADER, sizeof(CSW_HEADER)-1 ) )
	{
		LOG_FORMATS( "csw_cas_to_wav_size: cassette image has incompatible header\n" );
		goto cleanup;
	}

	if (casdata[0x16]!=0x1a)
	{
		LOG_FORMATS( "csw_cas_to_wav_size: Terminator Code Not Found\n" );
		goto cleanup;
	}

	MajorRevision=casdata[0x17];
	MinorRevision=casdata[0x18];

	LOG_FORMATS("Version %d : %d\n",MajorRevision,MinorRevision);

	if (casdata[0x17]!=2)
	{
		LOG_FORMATS( "csw_cas_to_wav_size: Unsuported Major Version\n" );
		goto cleanup;
	}

	HeaderExtensionLength=casdata[0x23];

	mycaslen=caslen;
	//from here on down for now I am assuming it is compressed csw file.
	in_ptr = (uint8_t*) casdata+0x34+HeaderExtensionLength;

	gz_ptr.resize( 8 );

	d_stream.next_in = (unsigned char *)in_ptr;
	d_stream.avail_in = caslen - ( in_ptr - casdata );
	d_stream.total_in=0;

	d_stream.next_out = &gz_ptr[0];
	d_stream.avail_out = 1;
	d_stream.total_out=0;

	d_stream.zalloc = nullptr;
	d_stream.zfree = nullptr;
	d_stream.opaque = nullptr;
	d_stream.data_type=0;

	err = inflateInit( &d_stream );
	if ( err != Z_OK )
	{
		LOG_FORMATS( "inflateInit2 error: %d\n", err );
		goto cleanup;
	}


	total_size=1;
	do
	{
		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out=1;
		err=inflate( &d_stream, Z_SYNC_FLUSH );
		if (err==Z_OK)
		{
			bsize=gz_ptr[0];
			if (bsize==0)
			{
				d_stream.avail_out=4;
				d_stream.next_out = &gz_ptr[0];
				err=inflate( &d_stream, Z_SYNC_FLUSH );
				bsize=get_leuint32(&gz_ptr[0]);
			}
			total_size=total_size+bsize;
		}
	}
	while (err==Z_OK);

	if ( err != Z_STREAM_END )
	{
		LOG_FORMATS( "inflate error: %d\n", err );
		goto cleanup;
	}

	err = inflateEnd( &d_stream );
	if ( err != Z_OK )
	{
		LOG_FORMATS( "inflateEnd error: %d\n", err );
		goto cleanup;
	}

	return total_size;

cleanup:
	return -1;
}

static int csw_cas_fill_wave( int16_t *buffer, int length, uint8_t *bytes )
{
	uint32_t SampleRate;
	uint32_t NumberOfPulses;
	uint8_t  CompressionType;
	uint8_t  Flags;
	uint8_t  HeaderExtensionLength;
	int8_t   Bit;

	std::vector<uint8_t> gz_ptr;
	int         total_size;
	z_stream    d_stream;
	int         err;
	uint8_t       *in_ptr;
	int         bsize=0;
	int     i;

	LOG_FORMATS("Length %d\n",length);

	SampleRate=get_leuint32(bytes+0x19);
	LOG_FORMATS("Sample rate %u\n",SampleRate);

	NumberOfPulses=get_leuint32(bytes+0x1d);
	LOG_FORMATS("Number Of Pulses %u\n",NumberOfPulses);

	CompressionType=bytes[0x21];
	Flags=bytes[0x22];
	HeaderExtensionLength=bytes[0x23];

	Bit = (Flags & 1) ? 100 : -100;

	LOG_FORMATS("CompressionType %u   Flags %u   HeaderExtensionLength %u\n",CompressionType,Flags,HeaderExtensionLength);

	LOG_FORMATS("Encoder: ");
	for (i = 0; i < 16; i++)
		LOG_FORMATS("%c", bytes[0x24 + i]);
	LOG_FORMATS("\n");

	LOG_FORMATS("Header: ");
	for (i = 0; i < HeaderExtensionLength; i++)
		LOG_FORMATS("%c", bytes[0x34 + i]);
	LOG_FORMATS("\n");

	//from here on down for now I am assuming it is compressed csw file.
	in_ptr = (uint8_t*) bytes+0x34+HeaderExtensionLength;

	gz_ptr.resize( 8 );

	d_stream.next_in = (unsigned char *)in_ptr;
	d_stream.avail_in = mycaslen - ( in_ptr - bytes );
	d_stream.total_in=0;

	d_stream.next_out = &gz_ptr[0];
	d_stream.avail_out = 1;
	d_stream.total_out=0;

	d_stream.zalloc = nullptr;
	d_stream.zfree = nullptr;
	d_stream.opaque = nullptr;
	d_stream.data_type=0;

	err = inflateInit( &d_stream );
	if ( err != Z_OK )
	{
		LOG_FORMATS( "inflateInit2 error: %d\n", err );
		goto cleanup;
	}

	total_size=0;

	do
	{
		d_stream.next_out = &gz_ptr[0];
		d_stream.avail_out=1;
		err=inflate( &d_stream, Z_SYNC_FLUSH );
		if (err==Z_OK)
		{
			bsize=gz_ptr[0];
			if (bsize==0)
			{
				d_stream.avail_out=4;
				d_stream.next_out = &gz_ptr[0];
				err=inflate( &d_stream, Z_SYNC_FLUSH );
				bsize=get_leuint32(&gz_ptr[0]);
			}
			for (i=0;i<bsize;i++)
			{
				buffer[total_size++]=Bit;
			}
			Bit=-Bit;
		}
	}
	while (err==Z_OK);

	if ( err != Z_STREAM_END )
	{
		LOG_FORMATS( "inflate error: %d\n", err );
		goto cleanup;
	}

	err = inflateEnd( &d_stream );
	if ( err != Z_OK )
	{
		LOG_FORMATS( "inflateEnd error: %d\n", err );
		goto cleanup;
	}

	return length;

cleanup:
	return -1;
}


static const struct CassetteLegacyWaveFiller csw_legacy_fill_wave = {
	csw_cas_fill_wave,      /* fill_wave */
	-1,                     /* chunk_size */
	0,                      /* chunk_samples */
	csw_cas_to_wav_size,    /* chunk_sample_calc */
	CSW_WAV_FREQUENCY,      /* sample_frequency */
	0,                      /* header_samples */
	0                       /* trailer_samples */
};

static cassette_image::error csw_cassette_identify( cassette_image *cassette, struct CassetteOptions *opts )
{
	uint8_t header[22];

	cassette_image_read(cassette, header, 0, sizeof(header));
	if (memcmp(&header[0], CSW_HEADER, sizeof(CSW_HEADER) - 1)) {
		return cassette_image::error::INVALID_IMAGE;
	}
	return cassette_legacy_identify( cassette, opts, &csw_legacy_fill_wave );
}

static cassette_image::error csw_cassette_load( cassette_image *cassette )
{
	return cassette_legacy_construct( cassette, &csw_legacy_fill_wave );
}

const struct CassetteFormat csw_cassette_format = {
	"csw",
	csw_cassette_identify,
	csw_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(csw_cassette_formats)
	CASSETTE_FORMAT(csw_cassette_format)
CASSETTE_FORMATLIST_END

CASSETTE_FORMATLIST_START(bbc_cassette_formats)
	CASSETTE_FORMAT(csw_cassette_format)
	CASSETTE_FORMAT(uef_cassette_format)
CASSETTE_FORMATLIST_END
