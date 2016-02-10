// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/********************************************************************

    Support for Videoton TVC .cas files

    http://tvc.homeserver.hu/html/konvertformatum.html

********************************************************************/

#include <assert.h>

#include "tvc_cas.h"

#define TVC64_BIT0_FREQ     1812
#define TVC64_BIT1_FREQ     2577
#define TVC64_PRE_FREQ      2128
#define TVC64_SYNC_FREQ     1359

#define WAVE_AMPLITUDE      0x3fffffff
#define TVC64_HEADER_BYTES  0x90
#define TVC64_HEADER_BYTES  0x90

static void tvc64_emit_level(cassette_image *cass, double &time, int freq, int level)
{
	double period = 1.0 / freq;
	cassette_put_sample(cass, 0, time, period, level * WAVE_AMPLITUDE);
	time += period;
}

static casserr_t tvc64_output_byte(cassette_image *cass, double &time, UINT8 byte)
{
	for (int i=0; i<8; i++)
	{
		if ((byte>>i) & 0x01)
		{
			tvc64_emit_level(cass, time, TVC64_BIT1_FREQ*2, +1);
			tvc64_emit_level(cass, time, TVC64_BIT1_FREQ*2, -1);
		}
		else
		{
			tvc64_emit_level(cass, time, TVC64_BIT0_FREQ*2, +1);
			tvc64_emit_level(cass, time, TVC64_BIT0_FREQ*2, -1);
		}
	}

	return CASSETTE_ERROR_SUCCESS;
}

static int tvc64_output_predata(cassette_image *cass, double &time, int number)
{
	for (int i=0; i<number; i++)
	{
		tvc64_emit_level(cass, time, TVC64_PRE_FREQ*2, +1);
		tvc64_emit_level(cass, time, TVC64_PRE_FREQ*2, -1);
	}

	return CASSETTE_ERROR_SUCCESS;
}

static UINT16 tvc64_calc_crc(const UINT8 *bytes, int size)
{
	UINT16 crc = 0;

	for (int i=0; i<size; i++)
	{
		for (int b=0; b<8; b++)
		{
			UINT8 al = (bytes[i] & (1<<b)) ? 0x80 : 0x00;

			al ^= ((crc>>8) & 0xff);

			if (al & 0x80)
				crc ^= 0x0810;

			crc <<= 1;

			if (al & 0x80) crc += 1;
		}
	}

	return crc;
}

static casserr_t tvc64_cassette_load(cassette_image *cassette)
{
	UINT8 tmp_buff[512];
	int buff_idx = 0;
	double time = 0.0;

	UINT8 header[TVC64_HEADER_BYTES];
	cassette_image_read(cassette, header, 0, TVC64_HEADER_BYTES);
	UINT16 cas_size = (header[0x83]<<8) | header[0x82];

	// tape header
	tmp_buff[buff_idx++] = 0x00;
	tmp_buff[buff_idx++] = 0x6a;
	tmp_buff[buff_idx++] = 0xff;            // head sector
	tmp_buff[buff_idx++] = 0x11;            // not puffered
	tmp_buff[buff_idx++] = 0x00;            // not write protected
	tmp_buff[buff_idx++] = 0x01;            // 1 sector
	tmp_buff[buff_idx++] = 0x00;            // sector number
	tmp_buff[buff_idx++] = 0x12;            // head size
	tmp_buff[buff_idx++] = 0x01;            // name size
	tmp_buff[buff_idx++] = ' ';             // name
	tmp_buff[buff_idx++] = 0x00;
	tmp_buff[buff_idx++] = header[0x81];    // type
	tmp_buff[buff_idx++] = header[0x82];    // size LSB
	tmp_buff[buff_idx++] = header[0x83];    // size MSB
	tmp_buff[buff_idx++] = header[0x84];    // autostart

		// sector fill
	for (int i=0; i<10 ; i++)
		tmp_buff[buff_idx++] = 0x00;

	tmp_buff[buff_idx++] = 0x00;    // file version
	tmp_buff[buff_idx++] = 0x00;    // no last sector

	// updates the header CRC
	UINT16 crc = tvc64_calc_crc(tmp_buff, buff_idx);
	tmp_buff[buff_idx++] = crc & 0xff;
	tmp_buff[buff_idx++] = (crc>>8) & 0xff;

	// 2 sec silence
	tvc64_emit_level(cassette, time, 2, 0);

	// 10240 pre data cycles
	tvc64_output_predata(cassette, time, 10240);

	// 1 synchro cycle
	tvc64_emit_level(cassette, time, TVC64_SYNC_FREQ*2, +1);
	tvc64_emit_level(cassette, time, TVC64_SYNC_FREQ*2, -1);

	// header data
	for (int i=0; i<buff_idx; i++)
		tvc64_output_byte(cassette, time, tmp_buff[i]);

	// 5 post data cycles
	tvc64_output_predata(cassette, time, 5);

	// 1 sec silence
	tvc64_emit_level(cassette, time, 1, 0);

	// 5120 pre data cycles
	tvc64_output_predata(cassette, time, 5120);

	// 1 synchro cycle
	tvc64_emit_level(cassette, time, TVC64_SYNC_FREQ*2, +1);
	tvc64_emit_level(cassette, time, TVC64_SYNC_FREQ*2, -1);

	// first data sector contain the data header
	buff_idx = 0;
	tmp_buff[buff_idx++] = 0x00;
	tmp_buff[buff_idx++] = 0x6a;
	tmp_buff[buff_idx++] = 0x00;        // data sector
	tmp_buff[buff_idx++] = 0x11;        // not puffered
	tmp_buff[buff_idx++] = 0x00;        // not write protected
	tmp_buff[buff_idx++] = (UINT8)((cas_size / 256) + ((cas_size % 256) > 0 ? 1 : 0));  // number of sectors

	UINT8 sect_num = 1;
	int sector_num = cas_size / 256;
	for (int i=0; i<=sector_num; i++)
	{
		tmp_buff[buff_idx++] = sect_num++;      // sector number

		// sector size
		if (i == sector_num)
			tmp_buff[buff_idx++] = cas_size % 256;
		else
			tmp_buff[buff_idx++] = 0x00;

		// sector data
		int sector_size = (i == sector_num) ? (cas_size % 256) : 256;
		for (int z=0; z < sector_size; z++)
			cassette_image_read(cassette, &tmp_buff[buff_idx++], TVC64_HEADER_BYTES + i*256 + z, 1);

		if (i == sector_num || ((i+1) == sector_num && (cas_size % 256 ) == 0))
			tmp_buff[buff_idx++] = 0xff;    // last sector
		else
			tmp_buff[buff_idx++] = 0x00;    // no last sector

		// sector crc
		crc = tvc64_calc_crc(tmp_buff, buff_idx);
		tmp_buff[buff_idx++] = crc & 0xff;
		tmp_buff[buff_idx++] = (crc>>8) & 0xff;

		// output the sector
		for (int z=0; z<buff_idx; z++)
			tvc64_output_byte(cassette, time, tmp_buff[z]);

		buff_idx = 0;
	}

	// 5 post data cycles
	tvc64_output_predata(cassette, time, 5);

	// 1 sec silence
	tvc64_emit_level(cassette, time, 1, 0);

	return CASSETTE_ERROR_SUCCESS;
}

static casserr_t tvc64_cassette_identify(cassette_image *cassette, struct CassetteOptions *opts)
{
	UINT8 byte;
	cassette_image_read(cassette, &byte, 0, 1);

	if (byte == 0x11)
	{
		opts->bits_per_sample = 16;
		opts->channels = 1;
		opts->sample_frequency = 44100;
		return CASSETTE_ERROR_SUCCESS;
	}

	return CASSETTE_ERROR_INVALIDIMAGE;
}

static const struct CassetteFormat tvc64_cassette_image_format =
{
	"cas",
	tvc64_cassette_identify,
	tvc64_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(tvc64_cassette_formats)
	CASSETTE_FORMAT(tvc64_cassette_image_format)
CASSETTE_FORMATLIST_END
