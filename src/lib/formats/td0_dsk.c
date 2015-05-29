// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/td0_dsk.c

    TD0 disk images

*********************************************************************/
/*
 * Based on Japanese version 29-NOV-1988
 * LZSS coded by Haruhiko OKUMURA
 * Adaptive Huffman Coding coded by Haruyasu YOSHIZAKI
 * Edited and translated to English by Kenji RIKITAKE
 */

#include <string.h>
#include <assert.h>
#include "pool.h"
#include "flopimg.h"

#define BUFSZ           512     // new input buffer

/* LZSS Parameters */

#define N        4096    /* Size of string buffer */
#define F        60    /* Size of look-ahead buffer */
#define THRESHOLD    2
#define NIL        N    /* End of tree's node  */


/* Huffman coding parameters */

#define N_CHAR      (256 - THRESHOLD + F)
				/* character code (= 0..N_CHAR-1) */
#define T         (N_CHAR * 2 - 1)    /* Size of table */
#define R         (T - 1)            /* root position */
#define MAX_FREQ    0x8000
					/* update when cumulative frequency */
					/* reaches to this value */

struct td0dsk_tag
{
	int heads;
	int tracks;
	int sector_size;
	UINT64 track_offsets[84*2]; /* offset within data for each track */
	UINT8 *data;
};

struct tdlzhuf {
	UINT16 r,
					bufcnt,bufndx,bufpos,  // string buffer
				// the following to allow block reads from input in next_word()
					ibufcnt,ibufndx; // input buffer counters
	UINT8  inbuf[BUFSZ];    // input buffer
};


struct td0dsk_t
{
	io_generic *floppy_file;
	UINT64 floppy_file_offset;

	struct tdlzhuf tdctl;
	UINT8 text_buf[N + F - 1];
	UINT16 freq[T + 1];    /* cumulative freq table */

/*
 * pointing parent nodes.
 * area [T..(T + N_CHAR - 1)] are pointers for leaves
 */
	INT16 prnt[T + N_CHAR];

	/* pointing children nodes (son[], son[] + 1)*/
	INT16 son[T];

	UINT16 getbuf;
	UINT8 getlen;

	int data_read(UINT8 *buf, UINT16 size);
	int next_word();
	int GetBit();
	int GetByte();
	void StartHuff();
	void reconst();
	void update(int c);
	INT16 DecodeChar();
	INT16 DecodePosition();
	void init_Decode();
	int Decode(UINT8 *buf, int len);
};

//static td0dsk_t td0dsk;

struct floppy_image_legacy
{
	struct io_generic io;

	const struct FloppyFormat *floppy_option;
	struct FloppyCallbacks format;

	/* loaded track stuff */
	int loaded_track_head;
	int loaded_track_index;
	UINT32 loaded_track_size;
	void *loaded_track_data;
	UINT8 loaded_track_status;
	UINT8 flags;

	/* tagging system */
	object_pool *tags;
	void *tag_data;
};


static struct td0dsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct td0dsk_tag *tag;
	tag = (td0dsk_tag *)floppy_tag((floppy_image_legacy *)floppy);
	return tag;
}



FLOPPY_IDENTIFY( td0_dsk_identify )
{
	UINT8 header[2];

	floppy_image_read(floppy, header, 0, 2);
	if (header[0]=='T' && header[1]=='D') {
		*vote = 100;
	} else if (header[0]=='t' && header[1]=='d') {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int td0_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int td0_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static UINT64 td0_get_track_offset(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	UINT64 offs;
	UINT8 *header;
	UINT8 sectors_per_track;
	int i;

	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	// position on beginning of track data
	offs = td0_get_track_offset(floppy, head, track);

	// read track header
	header = get_tag(floppy)->data + offs - 4;

	// take number of sectors per track
	sectors_per_track = header[0];

	if (!sector_is_index) {
		// when taking ID's return seek error if number is over counter
		if (sector > sectors_per_track) {
			return FLOPPY_ERROR_SEEKERROR;
		}
	}

	// move trought sectors
	for(i=0;i < sector-1;i++) {
		header = get_tag(floppy)->data + offs;
		offs+= 6;
		if ((header[4] & 0x30)==0) {
			offs+= 2;
			offs+= header[6] + (header[7]<<8);
		}
	}
	// read size of sector
	header = get_tag(floppy)->data + offs;
	get_tag(floppy)->sector_size = 1 << (header[3] + 7);

	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_td0_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;
	UINT8 *header;
	int size,realsize,i;
	int buff_pos;
	int data_pos;
	UINT8 *data;
	UINT8 *buf;

	buf = (UINT8*)buffer;
	// take sector offset
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	// read sector header
	header = get_tag(floppy)->data + offset;
	offset+=6;
	// if there is no date just jump out
	if ((header[4] & 0x30)!=0) return FLOPPY_ERROR_SUCCESS;

	offset+=3;
	// take data size
	size = header[6] + (header[7]<<8)-1;
	// take real sector size
	realsize =  1 << (header[3] + 7);

	// read sector data
	data = get_tag(floppy)->data + offset;
	buff_pos = 0;
	data_pos = 0;

	switch(header[8]) {
		case 0:
				// encoding type 0
				//  - plain data
				memcpy(buffer,data,size);
				break;
		case 1:
				// encoding type 1
				//  - 2 bytes size
				//  - 2 bytes of data
				//  data is reapeted specified number of times
				while(buff_pos<realsize) {
					for (i=0;i<data[data_pos]+(data[data_pos+1] << 8);i++) {
						buf[buff_pos] = data[data_pos+2];buff_pos++;
						buf[buff_pos] = data[data_pos+3];buff_pos++;
					}
					data_pos+=4;
				}
				break;
		case 2:
				// encoding type 2
				//  - if first byte is zero next byte represent size of
				//      plain data after it
				//  - if different then zero when multiply by 2 represent
				//      size of data that should be reapeted next byte times
				while(buff_pos<realsize) {
					if (data[data_pos]==0x00) {
						int size_ = data[data_pos+1];
						memcpy(buf+buff_pos,data + data_pos + 2,size_);
						data_pos += 2 + size_;
						buff_pos += size_;
					} else {
						int size_  = 2*data[data_pos];
						int repeat = data[data_pos+1];
						data_pos+=2;

						for (i=0;i<repeat;i++) {
							memcpy(buf + buff_pos,data + data_pos,size_);
							buff_pos += size_;
						}
						data_pos += size_;
					}
				}
				break;
		default:
				return FLOPPY_ERROR_INTERNAL;
	}
	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t td0_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_td0_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t td0_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_td0_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t td0_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t td0_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	floperr_t retVal;
	UINT64 offset = 0;
	UINT8 *sector_info;

	retVal = get_offset(floppy, head, track, sector_index, FALSE, &offset);
	sector_info = get_tag(floppy)->data + offset;
	if (cylinder)
		*cylinder = sector_info[0];
	if (side)
		*side = sector_info[1];
	if (sector)
		*sector = sector_info[2];
	if (sector_length) {
		*sector_length = 1 << (sector_info[3] + 7);
	}
	if (flags) {
		*flags = 0;
		if (sector_info[4] & 0x02) *flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
		if (sector_info[4] & 0x04) *flags |= ID_FLAG_DELETED_DATA;
	}

	return retVal;
}

int td0dsk_t::data_read(UINT8 *buf, UINT16 size)
{
	UINT64 image_size = io_generic_size(floppy_file);
	if (size > image_size - floppy_file_offset) {
		size = image_size - floppy_file_offset;
	}
	io_generic_read(floppy_file,buf,floppy_file_offset,size);
	floppy_file_offset += size;
	return size;
}


/*
 * Tables for encoding/decoding upper 6 bits of
 * sliding dictionary pointer
 */

/* decoder table */
static const UINT8 d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
	0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
	0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
	0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static const UINT8 d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

int td0dsk_t::next_word()
{
	if(tdctl.ibufndx >= tdctl.ibufcnt)
	{
		tdctl.ibufndx = 0;
		tdctl.ibufcnt = data_read(tdctl.inbuf,BUFSZ);
		if(tdctl.ibufcnt <= 0)
			return(-1);
	}
	while (getlen <= 8) { // typically reads a word at a time
		getbuf |= tdctl.inbuf[tdctl.ibufndx++] << (8 - getlen);
		getlen += 8;
	}
	return(0);
}


int td0dsk_t::GetBit()    /* get one bit */
{
	INT16 i;
	if(next_word() < 0)
		return(-1);
	i = getbuf;
	getbuf <<= 1;
	getlen--;
		if(i < 0)
		return(1);
	else
		return(0);
}

int td0dsk_t::GetByte()    /* get a byte */
{
	UINT16 i;
	if(next_word() != 0)
		return(-1);
	i = getbuf;
	getbuf <<= 8;
	getlen -= 8;
	i = i >> 8;
	return((int) i);
}



/* initialize freq tree */

void td0dsk_t::StartHuff()
{
	int i, j;

	for (i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}
	i = 0; j = N_CHAR;
	while (j <= R) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[T] = 0xffff;
	prnt[R] = 0;
}


/* reconstruct freq tree */

void td0dsk_t::reconst()
{
	INT16 i, j, k;
	UINT16 f, l;

	/* halven cumulative freq for leaf nodes */
	j = 0;
	for (i = 0; i < T; i++) {
		if (son[i] >= T) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
	/* make a tree : first, connect children nodes */
	for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for (k = j - 1; f < freq[k]; k--);
		k++;
		l = (j - k) * 2;

		/* movmem() is Turbo-C dependent
		   rewritten to memmove() by Kenji */

		/* movmem(&freq[k], &freq[k + 1], l); */
		(void)memmove(&freq[k + 1], &freq[k], l);
		freq[k] = f;
		/* movmem(&son[k], &son[k + 1], l); */
		(void)memmove(&son[k + 1], &son[k], l);
		son[k] = i;
	}
	/* connect parent nodes */
	for (i = 0; i < T; i++) {
		if ((k = son[i]) >= T) {
			prnt[k] = i;
		} else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}


/* update freq tree */

void td0dsk_t::update(int c)
{
	int i, j, k, l;

	if (freq[R] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + T];
	do {
		k = ++freq[c];

		/* swap nodes to keep the tree freq-ordered */
		if (k > freq[l = c + 1]) {
			while (k > freq[++l]);
			l--;
			freq[c] = freq[l];
			freq[l] = k;

			i = son[c];
			prnt[i] = l;
			if (i < T) prnt[i + 1] = l;

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < T) prnt[j + 1] = c;
			son[c] = j;

			c = l;
		}
	} while ((c = prnt[c]) != 0);    /* do it until reaching the root */
}


INT16 td0dsk_t::DecodeChar()
{
	int ret;
	UINT16 c;

	c = son[R];

	/*
	 * start searching tree from the root to leaves.
	 * choose node #(son[]) if input bit == 0
	 * else choose #(son[]+1) (input bit == 1)
	 */
	while (c < T) {
		if((ret = GetBit()) < 0)
			return(-1);
		c += (unsigned) ret;
		c = son[c];
	}
	c -= T;
	update(c);
	return c;
}

INT16 td0dsk_t::DecodePosition()
{
	INT16 bit;
	UINT16 i, j, c;

	/* decode upper 6 bits from given table */
	if((bit=GetByte()) < 0)
		return(-1);
	i = (UINT16) bit;
	c = (UINT16)d_code[i] << 6;
	j = d_len[i];

	/* input lower 6 bits directly */
	j -= 2;
	while (j--) {
		if((bit = GetBit()) < 0)
			return(-1);
		i = (i << 1) + bit;
	}
	return(c | (i & 0x3f));
}

/* DeCompression

split out initialization code to init_Decode()

*/

void td0dsk_t::init_Decode()
{
	int i;
	getbuf = 0;
	getlen = 0;
	tdctl.ibufcnt= tdctl.ibufndx = 0; // input buffer is empty
	tdctl.bufcnt = 0;
	StartHuff();
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';
	tdctl.r = N - F;
}


int td0dsk_t::Decode(UINT8 *buf, int len)  /* Decoding/Uncompressing */
{
	INT16 c,pos;
	int  count;  // was an unsigned long, seems unnecessary
	for (count = 0; count < len; ) {
			if(tdctl.bufcnt == 0) {
				if((c = DecodeChar()) < 0)
					return(count); // fatal error
				if (c < 256) {
					*(buf++) = c;
					text_buf[tdctl.r++] = c;
					tdctl.r &= (N - 1);
					count++;
				}
				else {
					if((pos = DecodePosition()) < 0)
							return(count); // fatal error
					tdctl.bufpos = (tdctl.r - pos - 1) & (N - 1);
					tdctl.bufcnt = c - 255 + THRESHOLD;
					tdctl.bufndx = 0;
				}
			}
			else { // still chars from last string
				while( tdctl.bufndx < tdctl.bufcnt && count < len ) {
					c = text_buf[(tdctl.bufpos + tdctl.bufndx) & (N - 1)];
					*(buf++) = c;
					tdctl.bufndx++;
					text_buf[tdctl.r++] = c;
					tdctl.r &= (N - 1);
					count++;
				}
				// reset bufcnt after copy string from text_buf[]
				if(tdctl.bufndx >= tdctl.bufcnt)
					tdctl.bufndx = tdctl.bufcnt = 0;
			}
	}
	return(count); // count == len, success
}


FLOPPY_CONSTRUCT( td0_dsk_construct )
{
	td0dsk_t state;
	struct FloppyCallbacks *callbacks;
	struct td0dsk_tag *tag;
	UINT8 *header;
	int number_of_sectors;
	int position;
	int i;
	int track;

	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct td0dsk_tag *) floppy_create_tag(floppy, sizeof(struct td0dsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->data = (UINT8*)malloc(floppy_image_size(floppy));
	if (tag->data==NULL) {
		return FLOPPY_ERROR_OUTOFMEMORY;
	}
	floppy_image_read(floppy, tag->data, 0, floppy_image_size(floppy));
	header = tag->data;

	if (header[0]=='t') {
		UINT8 obuf[BUFSZ];
		int rd;
		int off = 12;
		int size = 0;
		state.floppy_file = &(floppy->io);
		state.init_Decode();
		state.floppy_file_offset = 12;
		do
		{
			if((rd = state.Decode(obuf, BUFSZ)) > 0) size += rd;
		} while(rd == BUFSZ);
		memcpy(obuf,tag->data,12);
		free(tag->data);
		tag->data = (UINT8*)malloc(size+12);
		if (tag->data==NULL) {
			return FLOPPY_ERROR_OUTOFMEMORY;
		}
		memcpy(tag->data,obuf,12);
		state.floppy_file_offset = 12;
		state.init_Decode();
		do
		{
			if((rd = state.Decode(obuf, BUFSZ)) > 0) {
				memcpy(tag->data+off,obuf,rd);
				off += rd;
			}
		}  while(rd == BUFSZ);
	}
	header = tag->data;
	tag->heads   = header[9];
	if (tag->heads > 1) {
		tag->heads  = 2;
	}

	//  header len + comment header + comment len
	position = 12;
	if (header[7] & 0x80) {
		position += 10 + header[14] + (header[15]<<8);
	}
	tag->tracks = 0;
	do {
		// read track header
		header = tag->data + position;
		track = header[1];
		number_of_sectors = header[0];
		if (number_of_sectors!=0xff){
			position+=4;
			tag->track_offsets[(track<<1) + (header[2] & 1)] = position;
			for(i=0;i<number_of_sectors;i++) {
				// read sector header
				header = tag->data + position;
				position+=6;
				// read sector size
				if ((header[4] & 0x30)==0) {
					// if there is sector data
					header = tag->data + position;
					position+=2;
					// skip sector data
					position+= header[0] + (header[1]<<8);
				}
			}
			tag->tracks++;
		}
	} while(number_of_sectors!=0xff);
	tag->tracks++;

	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = td0_read_sector;
	callbacks->read_indexed_sector = td0_read_indexed_sector;
	callbacks->get_sector_length = td0_get_sector_length;
	callbacks->get_heads_per_disk = td0_get_heads_per_disk;
	callbacks->get_tracks_per_disk = td0_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = td0_get_indexed_sector_info;
	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_DESTRUCT( td0_dsk_destruct )
{
	struct td0dsk_tag *tag = get_tag(floppy);
	free(tag->data);
	tag->data = NULL;
	return FLOPPY_ERROR_SUCCESS;
}

/*********************************************************************

    formats/td0_dsk.h

    Teledisk disk images

*********************************************************************/

#include "td0_dsk.h"

td0_format::td0_format()
{
}

const char *td0_format::name() const
{
	return "td0";
}

const char *td0_format::description() const
{
	return "Teledisk disk image";
}

const char *td0_format::extensions() const
{
	return "td0";
}

int td0_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[7];

	io_generic_read(io, h, 0, 7);
	if(((h[0] == 'T') && (h[1] == 'D')) || ((h[0] == 't') && (h[1] == 'd')))
	{
			return 100;
	}
	return 0;
}

bool td0_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int track_count = 0;
	int head_count = 0;
	int track_spt;
	int offset = 0;
	const int max_size = 4*1024*1024; // 4MB ought to be large enough for any floppy
	dynamic_buffer imagebuf(max_size);
	UINT8 header[12];

	io_generic_read(io, header, 0, 12);
	head_count = header[9];

	if(header[0] == 't')
	{
		td0dsk_t disk_decode;

		disk_decode.floppy_file = io;
		disk_decode.init_Decode();
		disk_decode.floppy_file_offset = 12;
		disk_decode.Decode(&imagebuf[0], max_size);
	}
	else
		io_generic_read(io, &imagebuf[0], 12, io_generic_size(io));

	if(header[7] & 0x80)
		offset = 10 + imagebuf[2] + (imagebuf[3] << 8);

	track_spt = imagebuf[offset];
	if(track_spt == 255) // Empty file?
		return false;

	switch(header[6])
	{
		case 2:
			if((imagebuf[offset + 2] & 0x7f) == 2) // ?
			{
				if(head_count == 2)
					image->set_variant(floppy_image::DSHD);
				else
					return false; // single side hd?
				break;
			}
			/* no break; could be qd, won't know until tracks are counted */
		case 1:
			if(head_count == 2)
				image->set_variant(floppy_image::DSDD);
			else
				image->set_variant(floppy_image::SSDD);
			break;
		case 4:
			if((imagebuf[offset + 2] & 0x7f) == 2) // ?
			{
				if(head_count == 2)
					image->set_variant(floppy_image::DSHD);
				else
					return false; // single side 3.5?
				break;
			}
			/* no break */
		case 3:
			if(head_count == 2)
			{
				if(form_factor == floppy_image::FF_525)
					image->set_variant(floppy_image::DSQD);
				else
					image->set_variant(floppy_image::DSDD);
			}
			else
			{
				if(form_factor == floppy_image::FF_525)
					image->set_variant(floppy_image::SSQD);
				else
					return false; // single side 3.5?
			}
			break;
	}

	static const int rates[3] = { 250000, 300000, 500000 };
	int rate = (header[5] & 0x7f) >= 3 ? 500000 : rates[header[5] & 0x7f];
	int rpm = form_factor == floppy_image::FF_8 || (form_factor == floppy_image::FF_525 && rate >= 300000) ? 360 : 300;
	int base_cell_count = rate*60/rpm;

	while(track_spt != 255)
	{
		desc_pc_sector sects[256];
		UINT8 sect_data[65536];
		int sdatapos = 0;
		int track = imagebuf[offset + 1];
		int head = imagebuf[offset + 2] & 1;
		bool fm = (header[5] & 0x80) || (imagebuf[offset + 2] & 0x80); // ?
		offset += 4;
		for(int i = 0; i < track_spt; i++)
		{
			UINT8 *hs = &imagebuf[offset];
			UINT16 size;
			offset += 6;

			sects[i].track       = hs[0];
			sects[i].head        = hs[1];
			sects[i].sector      = hs[2];
			sects[i].size        = hs[3];
			sects[i].deleted     = (hs[4] & 4) == 4;
			sects[i].bad_crc     = (hs[4] & 2) == 2;

			if(hs[4] & 0x30)
				size = 0;
			else
			{
				offset += 3;
				size = 128 << hs[3];
				int j, k;
				switch(hs[8])
				{
					default:
						return false;
					case 0:
						memcpy(&sect_data[sdatapos], &imagebuf[offset], size);
						offset += size;
						break;
					case 1:
						offset += 4;
						k = (hs[9] + (hs[10] << 8)) * 2;
						k = (k <= size) ? k : size;
						for(j = 0; j < k; j += 2)
						{
							sect_data[sdatapos + j] = hs[11];
							sect_data[sdatapos + j + 1] = hs[12];
						}
						if(k < size)
							memset(&sect_data[sdatapos + k], '\0', size - k);
						break;
					case 2:
						k = 0;
						while(k < size)
						{
							UINT16 len = imagebuf[offset];
							UINT16 rep = imagebuf[offset + 1];
							offset += 2;
							if(!len)
							{
								memcpy(&sect_data[sdatapos + k], &imagebuf[offset], rep);
								offset += rep;
								k += rep;
							}
							else
							{
								len = (1 << len);
								rep = len * rep;
								rep = ((rep + k) <= size) ? rep : (size - k);
								for(j = 0; j < rep; j += len)
									memcpy(&sect_data[sdatapos + j + k], &imagebuf[offset], len);
								k += rep;
								offset += len;
							}
						}
						break;
				}
			}

			sects[i].actual_size = size;

			if(size)
			{
				sects[i].data = &sect_data[sdatapos];
				sdatapos += size;
			}
			else
				sects[i].data = NULL;
		}
		track_count = track;

		if(fm)
			build_pc_track_fm(track, head, image, base_cell_count, track_spt, sects, calc_default_pc_gap3_size(form_factor, sects[0].actual_size));
		else
			build_pc_track_mfm(track, head, image, base_cell_count*2, track_spt, sects, calc_default_pc_gap3_size(form_factor, sects[0].actual_size));

		track_spt = imagebuf[offset];
	}
	if((track_count > 50) && (form_factor == floppy_image::FF_525)) // ?
	{
		if(image->get_variant() == floppy_image::DSDD)
			image->set_variant(floppy_image::DSQD);
		else if(image->get_variant() == floppy_image::SSDD)
			image->set_variant(floppy_image::SSQD);
	}
	return true;
}


bool td0_format::save(io_generic *io, floppy_image *image)
{
	return false;
}

bool td0_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_TD0_FORMAT = &floppy_image_format_creator<td0_format>;
