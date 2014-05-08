// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.c

    Dummy sound interface.

*******************************************************************c********/


#include "none.h"

//-------------------------------------------------
//  sound_none - constructor
//-------------------------------------------------
sound_none::sound_none(const osd_interface &osd)
	: osd_sound_interface(osd)
{
}


const osd_sound_type OSD_SOUND_NONE = &osd_sound_creator<sound_none>;