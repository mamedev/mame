/***************************************************************************

    tc8830f.c - Toshiba TC8830F, CMOS voice recording/reproducing LSI
    
    Very preliminary...

    TODO:
    - remaining commands
    - cpu manual mode
    - status read
    - RAM
    - recording

***************************************************************************/

#include "emu.h"
#include "tc8830f.h"


// device type definition
const device_type TC8830F = &device_creator<tc8830f_device>;

tc8830f_device::tc8830f_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC8830F, "TC8830F", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}


void tc8830f_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / 0x10, this);

	m_mem_base = (UINT8 *)device().machine().root_device().memregion(":tc8830f")->base();
	m_mem_mask = device().machine().root_device().memregion(":tc8830f")->bytes() - 1;
	
	reset();
}


void tc8830f_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		outputs[0][i] = 0;
	}
}


void tc8830f_device::reset()
{
	;
}


void tc8830f_device::write_p(UINT8 data)
{
	m_stream->update();
}
