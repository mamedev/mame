// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro 8271 protected disk image format

    Disk image format

***************************************************************************/

#include "fsd_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"

#include <cstring>


/*********************************************************************

    BBC Micro 8271 protected disk image format

    Header:
    =======
    Identifier: "FSD"  string literal 3 bytes
       Creator:        5 bytes; date of creation/author
         Title:        Character string (unlimited length; may contain any but null)
     Title_End: 0x00   byte literal 1 byte
        Num_Tr: xx     1 byte, number of tracks

    For each track
    ==============
       track_num: 1 byte
         num_sec: 1 byte (00 == unformatted)
        readable: 1 byte (00 == unreadable, ff ==readable)

       If readable:
         For each sector
         ===============
               Track_ID: 1 byte
            Head_number: 1 byte
              Sector_ID: 1 byte
          reported_size: 1 byte (2^{7+x}; 0 ==>128, 1 ==> 256, 2==>512, 3==>1024 etc)
              real_size: 1 byte (2^{7+x})
             error_code: 1 byte (0==No Error, &20==Deleted Data; &0E = Data CRC Error)
                   data: <real_size> bytes
       Note that error_code matches the OSWORD &7F result byte

       If unreadable:
               Track_ID: 1 byte
            Head_number: 1 byte
              Sector_ID: 1 byte
          reported_size: 1 byte

    Decoding of the "creator" 5 byte field:
    =======================================
       (byte1 byte2 byte3 byte4 byte5)

           Date_DD = (byte1 AND &F8)/8
           Date_MM = (byte3 AND &0F)
         Date_YYYY = (byte1 AND &07)*256+byte2
        Creator_ID = (byte3 AND &F0)/16
       Release_num = ((byte5 AND &C0)/64)*256 + byte4

*********************************************************************/

fsd_format::fsd_format()
{
}

const char *fsd_format::name() const
{
	return "fsd";
}

const char *fsd_format::description() const
{
	return "BBC Micro 8271 protected disk image";
}

const char *fsd_format::extensions() const
{
	return "fsd";
}

bool fsd_format::supports_save() const
{
	return false;
}

int fsd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t h[3];

	size_t actual;
	io.read_at(0, h, 3, actual);
	if (memcmp(h, "FSD", 3) == 0) {
		return 100;
	}
	LOG_FORMATS("fsd: no match\n");
	return 0;
}

bool fsd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	const char* result[255];
	result[0x00] = "OK";
	result[0x0e] = "Data CRC Error";
	result[0x20] = "Deleted Data";

	uint64_t size;
	if(io.length(size))
		return false;
	std::vector<uint8_t> img(size);
	size_t actual;
	io.read_at(0, &img[0], size, actual);

	uint64_t pos;
	std::string title;
	for(pos=8; pos < size && img[pos] != '\0'; pos++)
		title.append(1, img[pos]);
	pos++;

	if(pos >= size)
		return false;

	LOG_FORMATS("FSD Title: %s\n", title.c_str());
	LOG_FORMATS("FSD Created: %02d/%02d/%4d\n", (img[3] & 0xf8) / 8, img[5] & 0x0f, (img[3] & 0x07) * 256 + img[4]);
	LOG_FORMATS("FSD Creator: %d\n", (img[5] & 0xf0) / 16);
	LOG_FORMATS("FSD Release: %d\n", ((img[7] & 0xc0) / 64) * 256 + img[6]);

	desc_pc_sector sects[256];
	uint8_t total_tracks = img[pos++];
	uint8_t tnum, hnum, snum, ssize, error;

	hnum = 0;
	LOG_FORMATS("FSD Tracks: %02d\n", total_tracks+1);
	LOG_FORMATS("Tr.#  No.S  Sec.# Tr.ID Head# SecID IDsiz REsiz Error\n");
	for(int curr_track=0; curr_track <= total_tracks; curr_track++)
	{
		uint8_t track = img[pos++];
		uint8_t spt = img[pos++];

		if (spt > 0) // formatted
		{
			LOG_FORMATS("%02X    %02X\n", track, spt);
			uint8_t readable = img[pos++];
			for (int i = 0; i < spt; i++)
			{
				tnum = img[pos++];  // logical track
				hnum = img[pos++];  // head number
				snum = img[pos++];  // logical sector
				ssize = img[pos++]; // reported size
				sects[i].track = tnum;
				sects[i].head = hnum;
				sects[i].sector = snum;
				sects[i].size = ssize;
				if (readable == 0xff)
				{
					sects[i].actual_size = 128 << img[pos++];
					error = img[pos++];
					sects[i].deleted = (error & 0x20) == 0x20;
					sects[i].bad_crc = (error & 0x0e) == 0x0e;
					sects[i].data = &img[pos];
					pos += sects[i].actual_size;
					LOG_FORMATS("Read        %02X    %02X    %02X    %02X    %04X  %04X  %s\n", i, sects[i].track, sects[i].head, sects[i].sector, 128 << sects[i].size, sects[i].actual_size, result[error]);
				}
				else
				{
					LOG_FORMATS("Unreadable sector on track %02d sector %02X head %02d\n", track, i, hnum);
					sects[i].actual_size = 0;
					sects[i].deleted = false;
					sects[i].bad_crc = false;
					sects[i].data = nullptr;
					LOG_FORMATS("Unread      %02X    %02X    %02X    %02X    %04X  %04X  %s\n", i, sects[i].track, sects[i].head, sects[i].sector, sects[i].size, sects[i].actual_size, "Unreadable");
					//return false;
				}
			}
		}
		else // unformatted
		{
			sects[0].track = curr_track;
			sects[0].head = hnum;
			sects[0].sector = 0;
			sects[0].size = 0;
			LOG_FORMATS("Unform      %02X    %02X    %02X    %02X    %04X  %04X  %s\n", 0, sects[0].track, sects[0].head, sects[0].sector, sects[0].size, sects[0].actual_size, "Unformatted");
		}
		build_wd_track_fm(curr_track, hnum, image, 50000, spt, sects, 10, 40, 10);
	}
	return true;
}

const floppy_format_type FLOPPY_FSD_FORMAT = &floppy_image_format_creator<fsd_format>;
