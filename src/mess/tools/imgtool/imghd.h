// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    imghd.h

    Bridge between Imgtool and CHD hard disk images

****************************************************************************/

#ifndef IMGHD_H
#define IMGHD_H

#include "harddisk.h"

struct mess_hard_disk_file
{
	imgtool_stream *stream;
	hard_disk_file *hard_disk;
	chd_file *chd;
};


/* create a new hard disk */
imgtoolerr_t imghd_create(imgtool_stream *stream, UINT32 blocksize, UINT32 cylinders, UINT32 heads, UINT32 sectors, UINT32 seclen);

/* opens a hard disk given an Imgtool stream */
imgtoolerr_t imghd_open(imgtool_stream *stream, struct mess_hard_disk_file *hard_disk);

/* close a hard disk */
void imghd_close(struct mess_hard_disk_file *disk);

/* reads data from a hard disk */
imgtoolerr_t imghd_read(struct mess_hard_disk_file *disk, UINT32 lbasector, void *buffer);

/* writes data to a hard disk */
imgtoolerr_t imghd_write(struct mess_hard_disk_file *disk, UINT32 lbasector, const void *buffer);

/* gets the header from a hard disk */
const hard_disk_info *imghd_get_header(struct mess_hard_disk_file *disk);

#endif /* IMGHD_H */
