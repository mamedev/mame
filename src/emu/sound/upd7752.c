/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "sound/upd7752.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type UPD7752 = &device_creator<upd7752_device>;

static ADDRESS_MAP_START( upd7752_ram, AS_0, 8, upd7752_device )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
ADDRESS_MAP_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7752_device - constructor
//-------------------------------------------------

upd7752_device::upd7752_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7752, "uPD7752", tag, owner, clock, "upd7752", __FILE__),
	  device_sound_interface(mconfig, *this),
	  device_memory_interface(mconfig, *this),
	  m_space_config("ram", ENDIANNESS_LITTLE, 8, 18, 0, NULL, *ADDRESS_MAP_NAME(upd7752_ram))
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *upd7752_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7752_device::device_start()
{
	/* TODO: clock */
	m_stream = stream_alloc(0, 1, clock() / 64);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7752_device::device_reset()
{
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void upd7752_device::device_stop()
{

}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd7752_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( upd7752_device::read )
{
	switch(offset & 3)
	{
		//[0x00]: status register
		//x--- ---- BSY busy status (1) processing (0) stopped
		//-x-- ---- REQ audio parameter (1) input request (0) prohibited (???)
		//--x- ---- ~INT / EXT message data (1) Outside (0) Inside
		//---x ---- ERR error flag
		case 0x00: return 0x60;
		//[0x02]: port 0xe2 latch?
		case 0x02: return 0xff;
		//[0x03]: port 0xe3 latch?
		case 0x03: return 0xff;
	}
	return 0xff;
}

WRITE8_MEMBER( upd7752_device::write )
{
	switch(offset & 3)
	{
		// [0x00]: audio parameter transfer

		// [0x02]: mode set
		// ---- -x-- Frame periodic analysis (0) 10 ms / frame (1) 20 ms / frame
		// ---- --xx Utterance (tempo?) speed
		//        00 : NORMAL SPEED
		//        01 : SLOW SPEED
		//        10 : FAST SPEED
		//        11 : Setting prohibited

		// case 0x02:

		// case 0x03: command set
	}
}
