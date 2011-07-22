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

#define MAX_TRACK_BYTES			12500
#define ACTUAL_TRACK_BYTES		11968
#define GAP_TRACK_BYTES			( MAX_TRACK_BYTES - ACTUAL_TRACK_BYTES )
#define ONE_SECTOR_BYTES		(544*2)
#define ONE_REV_TIME			200 /* ms */
#define MAX_WORDS_PER_DMA_CYCLE	32 /* 64 bytes per dma cycle */
#define DISK_DETECT_DELAY		1
#define MAX_TRACKS				160
#define MAX_MFM_TRACK_LEN		16384

bool adf_format::load(floppy_image *image)
{
	UINT8 *mfm = NULL;
	UINT8	temp_cyl[512*11];
	UINT16 sector_len = 512*11;
	image->set_meta_data(80,2,300,(UINT16)253360);	
	for(int track=0; track < 80; track++) {
		for(int side=0; side < 2; side++) {
			UINT16 offset = ((track<<1)+side);
			mfm = image->get_buffer(track,side);					
			image->set_track_size(track, side, MAX_MFM_TRACK_LEN);			
			image->image_read(temp_cyl, offset*sector_len, sector_len);		
			memset( &mfm[ONE_SECTOR_BYTES*11], 0xaa, GAP_TRACK_BYTES );

			for (int sector = 0; sector < 11; sector++ ) {
				int x;
				UINT8 *dest = ( &mfm[(ONE_SECTOR_BYTES*sector)] );
				UINT8 *src = &temp_cyl[sector*512];
				UINT32 tmp;
				UINT32 even, odd;
				UINT32 hck = 0, dck = 0;

				/* Preamble and sync */
				*(dest + 0) = 0xaa;
				*(dest + 1) = 0xaa;
				*(dest + 2) = 0xaa;
				*(dest + 3) = 0xaa;
				*(dest + 4) = 0x44;
				*(dest + 5) = 0x89;
				*(dest + 6) = 0x44;
				*(dest + 7) = 0x89;

				/* Track and sector info */

				tmp = 0xff000000 | (offset<<16) | (sector<<8) | (11 - sector);
				odd = (tmp & 0x55555555) | 0xaaaaaaaa;
				even = ( ( tmp >> 1 ) & 0x55555555 ) | 0xaaaaaaaa;
				*(dest +  8) = (UINT8) ((even & 0xff000000)>>24);
				*(dest +  9) = (UINT8) ((even & 0xff0000)>>16);
				*(dest + 10) = (UINT8) ((even & 0xff00)>>8);
				*(dest + 11) = (UINT8) ((even & 0xff));
				*(dest + 12) = (UINT8) ((odd & 0xff000000)>>24);
				*(dest + 13) = (UINT8) ((odd & 0xff0000)>>16);
				*(dest + 14) = (UINT8) ((odd & 0xff00)>>8);
				*(dest + 15) = (UINT8) ((odd & 0xff));

				/* Fill unused space */

				for (x = 16 ; x < 48; x++)
					*(dest + x) = 0xaa;

				/* Encode data section of sector */

				for (x = 64 ; x < 576; x++)
				{
					tmp = *(src + x - 64);
					odd = (tmp & 0x55);
					even = (tmp>>1) & 0x55;
					*(dest + x) = (UINT8) (even | 0xaa);
					*(dest + x + 512) = (UINT8) (odd | 0xaa);
				}

				/* Calculate checksum for unused space */

				for(x = 8; x < 48; x += 4)
					hck ^= (((UINT32) *(dest + x))<<24) | (((UINT32) *(dest + x + 1))<<16) |
						   (((UINT32) *(dest + x + 2))<<8) | ((UINT32) *(dest + x + 3));

				even = odd = hck;
				odd >>= 1;
				even |= 0xaaaaaaaa;
				odd |= 0xaaaaaaaa;

				*(dest + 48) = (UINT8) ((odd & 0xff000000)>>24);
				*(dest + 49) = (UINT8) ((odd & 0xff0000)>>16);
				*(dest + 50) = (UINT8) ((odd & 0xff00)>>8);
				*(dest + 51) = (UINT8) (odd & 0xff);
				*(dest + 52) = (UINT8) ((even & 0xff000000)>>24);
				*(dest + 53) = (UINT8) ((even & 0xff0000)>>16);
				*(dest + 54) = (UINT8) ((even & 0xff00)>>8);
				*(dest + 55) = (UINT8) (even & 0xff);

				/* Calculate checksum for data section */

				for(x = 64; x < 1088; x += 4)
					dck ^= (((UINT32) *(dest + x))<<24) | (((UINT32) *(dest + x + 1))<<16) |
						   (((UINT32) *(dest + x + 2))<< 8) |  ((UINT32) *(dest + x + 3));
				even = odd = dck;
				odd >>= 1;
				even |= 0xaaaaaaaa;
				odd |= 0xaaaaaaaa;
				*(dest + 56) = (UINT8) ((odd & 0xff000000)>>24);
				*(dest + 57) = (UINT8) ((odd & 0xff0000)>>16);
				*(dest + 58) = (UINT8) ((odd & 0xff00)>>8);
				*(dest + 59) = (UINT8) (odd & 0xff);
				*(dest + 60) = (UINT8) ((even & 0xff000000)>>24);
				*(dest + 61) = (UINT8) ((even & 0xff0000)>>16);
				*(dest + 62) = (UINT8) ((even & 0xff00)>>8);
				*(dest + 63) = (UINT8) (even & 0xff);
			}

			// update MFM data with proper CLK signal
			int lastbit= 0;
			for(int i=0;i<MAX_TRACK_BYTES ;i++)
			{
				UINT8 c=mfm[i];
				UINT8 dat = 0;
				for(int j=0;j<8;j=j+2)
				{
					UINT8 c1=(c>>(6-j))&0x3;
					if(c1&0x1)
					{
						dat |= (0x01<<(6-j));
						lastbit=1;
					}
					else
					{
						if(lastbit==0 && (c1&0x2))
						{
							dat |= (0x02<<(6-j));
						}
						else
						{
							dat |= (0x00<<(6-j));
						}
						lastbit=0;
					}
				}
				mfm[i] = dat;
			}
		}
	}
	return TRUE;
}

const floppy_format_type FLOPPY_ADF_FORMAT = &floppy_image_format_creator<adf_format>;
