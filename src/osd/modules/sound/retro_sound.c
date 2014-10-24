
/***************************************************************************

    retro_sound.c

    Dummy sound interface.

*******************************************************************c********/


#include "retro_sound.h"

#include "libretro.h"

static int  attenuation = 0;

extern int pauseg;
extern retro_audio_sample_batch_t audio_batch_cb;

const osd_sound_type OSD_SOUND_RETRO = &osd_sound_creator<sound_retro>;


//-------------------------------------------------
//  sound_none - constructor
//-------------------------------------------------
sound_retro::sound_retro(const osd_interface &osd)
	: osd_sound_interface(osd)
{
	set_mastervolume(attenuation);
}


//============================================================
//  update_audio_stream
//============================================================
void sound_retro::update_audio_stream(const INT16 *buffer, int samples_this_frame)
//void retro_osd_interface::update_audio_stream(const INT16 *buffer, int samples_this_frame) 
{
	if(pauseg!=-1)audio_batch_cb(buffer, samples_this_frame);
}
  

//============================================================
//  set_mastervolume
//============================================================
void sound_retro::set_mastervolume(int attenuation)
//void retro_osd_interface::set_mastervolume(int attenuation)
{
	// if we had actual sound output, we would adjust the global
	// volume in response to this function
}

