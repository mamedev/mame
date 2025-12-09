// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

NEC uPD7752 Voice Synthesizing LSI

skeleton device

***************************************************************************/

#include "emu.h"
#include "upd7752.h"


/* status flags */
#define BSY 1<<7
#define REQ 1<<6
#define EXT 1<<5
#define ERR 1<<4


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(UPD7752, upd7752_device, "upd7752", "NEC uPD7752")


/* TODO: unknown exact size */
void upd7752_device::upd7752_ram(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0xffff).ram();
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7752_device - constructor
//-------------------------------------------------

upd7752_device::upd7752_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD7752, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this), m_stream(nullptr),
		m_space_config("ram", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(upd7752_device::upd7752_ram), this)), m_status(0), m_ram_addr(0), m_mode(0)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd7752_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
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

void upd7752_device::sound_stream_update(sound_stream &stream)
{
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline uint8_t upd7752_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}

//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void upd7752_device::writebyte(offs_t address, uint8_t data)
{
	space().write_byte(address, data);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void upd7752_device::status_change(uint8_t flag,bool type)
{
	if(type == true)
		m_status |= flag;
	else
		m_status &= ~flag;
}

uint8_t upd7752_device::read(offs_t offset)
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

void upd7752_device::write(offs_t offset, uint8_t data)
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
