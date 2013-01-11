/***************************************************************************

    harddisk.h

    Generic MAME hard disk implementation, with differencing files

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __HARDDISK_H__
#define __HARDDISK_H__

#include "osdcore.h"
#include "chd.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct hard_disk_file;

struct hard_disk_info
{
	UINT32          cylinders;
	UINT32          heads;
	UINT32          sectors;
	UINT32          sectorbytes;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

hard_disk_file *hard_disk_open(chd_file *chd);
void hard_disk_close(hard_disk_file *file);

chd_file *hard_disk_get_chd(hard_disk_file *file);
hard_disk_info *hard_disk_get_info(hard_disk_file *file);

UINT32 hard_disk_read(hard_disk_file *file, UINT32 lbasector, void *buffer);
UINT32 hard_disk_write(hard_disk_file *file, UINT32 lbasector, const void *buffer);

#endif  /* __HARDDISK_H__ */
