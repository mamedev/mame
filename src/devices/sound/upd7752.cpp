// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

NEC uPD7752 Voice Synthesizing LSI

skeleton device

***************************************************************************/

#include "emu.h"
#include "sound/upd7752.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type UPD7752 = &device_creator<upd7752_device>;

/* TODO: unknown exact size */
static ADDRESS_MAP_START( upd7752_ram, AS_0, 8, upd7752_device )
//  AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x0000, 0xffff) AM_RAM
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
		device_memory_interface(mconfig, *this), m_stream(nullptr),
		m_space_config("ram", ENDIANNESS_LITTLE, 8, 16, 0, nullptr, *ADDRESS_MAP_NAME(upd7752_ram)), m_status(0), m_ram_addr(0), m_mode(0)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *upd7752_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7752_device::device_start()
{
	/* TODO: clock */
	m_stream = stream_alloc(0, 1, clock() / 64);

	m_status = 0;
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
//  INLINE HELPERS
//**************************************************************************

inline UINT8 upd7752_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void upd7752_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void upd7752_device::status_change(UINT8 flag,bool type)
{
	if(type == true)
		m_status |= flag;
	else
		m_status &= ~flag;
}

READ8_MEMBER( upd7752_device::read )
{
	switch(offset & 3)
	{
		//[0x00]: status register
		//x--- ---- BSY busy status (1) processing (0) stopped
		//-x-- ---- REQ audio parameter (1) input request (0) prohibited (???)
		//--x- ---- ~INT / EXT message data (1) Outside (0) Inside
		//---x ---- ERR error flag
		case 0x00: return m_status;
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
		case 0x00:
			if(m_status & EXT)
			{
				/*
				[0] xxxx x--- number of frames (times) to apply next table (N1)
				    ---- -x-- Quantized Magnification Data (QMAG)
				    ---- --x- Selective Interpolation Data (SI)
				    ---- ---x Voicing/Unvoicing Data (VU)
				[1] xxxx ---- amp Voice source amplitude
				    ---- x--- Fricative Voice data
				    ---- -xxx Pitch
				    (repeat for N1 times)
				if [0] & 0xf8 == 0 then command stop
				*/
				writebyte(m_ram_addr++,data);
			}
			//else
			// ...

		break;

		// [0x02]: mode set
		// ---- -x-- Frame periodic analysis (0) 10 ms / frame (1) 20 ms / frame
		// ---- --xx Utterance (tempo?) speed
		//        00 : NORMAL SPEED
		//        01 : SLOW SPEED
		//        10 : FAST SPEED
		//        11 : Setting prohibited

		case 0x02:
			m_mode = data & 7;
			break;

		case 0x03: //command set
			switch(data)
			{
				case 0xfe: // external message select cmd
					status_change(EXT,true);
					status_change(REQ,true);
					//TODO: BSY flag too
					m_ram_addr = 0;
					break;
			}

			break;

	}
}
