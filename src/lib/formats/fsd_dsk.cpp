// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro 8271 protected disk image format

    Disk image format

***************************************************************************/

#include "emu.h"
#include "fsd_dsk.h"


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

int fsd_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[3];

	io_generic_read(io, h, 0, 3);
	if (memcmp(h, "FSD", 3) == 0) {
		return 100;
	}
	return 0;
}

bool fsd_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT64 size = io_generic_size(io);
	dynamic_buffer img(size);
	io_generic_read(io, &img[0], 0, size);

	UINT64 pos;
	std::string title;
	for(pos=8; pos < size && img[pos] != '\0'; pos++)
		title += char(img[pos]);
	pos++;

	if(pos >= size)
		return false;

	//popmessage("Loading image of '%s'\n", title);

	desc_pc_sector sects[10];
	UINT8 total_tracks = img[pos++];
	UINT8 tnum, hnum, snum, ssize, error;

	hnum = 0;
	//osd_printf_verbose("%d Tracks\n", total_tracks+1);
	//osd_printf_verbose("Tr.#  No.S  Sec.# Tr.ID Head# SecID IDsiz REsiz Error\n");
	for(int curr_track=0; curr_track <= total_tracks; curr_track++)
	{
		UINT8 track = img[pos++];
		UINT8 spt = img[pos++];
		//osd_printf_verbose("%x    %x\n", track, spt);
		if (spt > 0) // formatted
		{
			UINT8 readable = img[pos++];
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
					sects[i].actual_size = 1 << (img[pos++] + 7);
					error = img[pos++];
					sects[i].deleted = (error & 0x20) == 0x20;
					sects[i].bad_crc = (error & 0x0e) == 0x0e;
					sects[i].data = &img[pos];
					pos += sects[i].actual_size;
					//osd_printf_verbose("Read        %x    %x    %x    %x    %x  %x  %x\n", i, sects[i].track, sects[i].head, sects[i].sector, sects[i].size, sects[i].actual_size, error);
				}
				else
				{
					throw emu_fatalerror("fsd_format: Unsupported unreadable sector on track %d sector %d head %d", track, i, hnum);

					// Unreadable sectors not supported!!
					//sects[i].track = track;
					//sects[i].head = 0;
					//sects[i].sector = i;
					//sects[i].size = 0;
					//sects[i].actual_size = 0;
					//sects[i].deleted = false;
					//sects[i].bad_crc = false;
					//sects[i].data = NULL;
					//osd_printf_verbose("Unread      %x    %x    %x    %x    %x  %x  %x\n", i, sects[i].track, sects[i].head, sects[i].sector, sects[i].size, sects[i].actual_size, 0);
				}
			}
		}
		else // unformatted
		{
			sects[0].track = curr_track;
			sects[0].head = hnum;
			sects[0].sector = 0;
			sects[0].size = 0;
			//osd_printf_verbose("Unform      %x    %x    %x    %x    %x  %x  %x\n", 0, sects[0].track, sects[0].head, sects[0].sector, sects[0].size, sects[0].actual_size, 0);
		}
		build_wd_track_fm(curr_track, hnum, image, 50000, spt, sects, 10, 40, 10);
	}
	return true;
}

const floppy_format_type FLOPPY_FSD_FORMAT = &floppy_image_format_creator<fsd_format>;
