/*********************************************************************

    formats/ami_dsk.c

    Amiga disk images

*********************************************************************/

#include "formats/ami_dsk.h"

adf_format::adf_format(const char *name,const char *extensions,const char *description,const char *param_guidelines) :
	floppy_image_format_t(name,extensions,description,param_guidelines)
{
}

int adf_format::identify(floppy_image *image)
{
	UINT64 size = image->image_size();
	if ((size == 901120) || (size == 1802240))
	{
		return 100;
	}
	return 0;
}

const floppy_image_format_t::desc_e adf_format::desc[] = {
	{ SECTOR_LOOP_START, 0, 10 },
	{   MFM, 0x00, 2 },
	{   RAW, 0x4489, 2 },
	{   CRC_AMIGA_START, 1 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_O },
	{     SECTOR_ID_O },
	{     REMAIN_O, 11 },
	{     MFMBITS, 0xf, 4 },
	{     OFFSET_ID_E },
	{     SECTOR_ID_E },
	{     REMAIN_E, 11 },
	{     MFM, 0x00, 16 },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   CRC, 2 },
	{   CRC_AMIGA_START, 2 },
	{     SECTOR_DATA_O, -1 },
	{     SECTOR_DATA_E, -1 },
	{   CRC_END, 2 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x00, 266 },
	{ END }
};

bool adf_format::load(floppy_image *image)
{
	desc_s sectors[11];
	UINT8 sectdata[512*11];
	for(int i=0; i<11; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
	}

	UINT8 *mfm = NULL;
	image->set_meta_data(80,2,300,(UINT16)253360);
	for(int track=0; track < 80; track++) {
		for(int side=0; side < 2; side++) {
			mfm = image->get_buffer(track,side);
			image->set_track_size(track, side, 16384);
			image->image_read(sectdata, (track*2 + side)*512*11, 512*11);
			generate_track(desc, track, side, sectors, 11, 100000, mfm);
		}
	}

	return TRUE;
}

const floppy_format_type FLOPPY_ADF_FORMAT = &floppy_image_format_creator<adf_format>;
