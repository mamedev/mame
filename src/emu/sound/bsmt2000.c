/***************************************************************************

    bsmt2000.c

    BSMT2000 device emulator.

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

****************************************************************************

    Chip is actually a TMS320C15 DSP with embedded mask rom
    Trivia: BSMT stands for "Brian Schmidt's Mouse Trap"

***************************************************************************/

#include "emu.h"
#include "bsmt2000.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type BSMT2000 = bsmt2000_device_config::static_alloc_device_config;


// program map for the DSP (points to internal ROM)
static ADDRESS_MAP_START( tms_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END


// I/O map for the DSP
static ADDRESS_MAP_START( tms_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0, 0) AM_DEVREADWRITE_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_register_r, tms_rom_addr_w)
	AM_RANGE(1, 1) AM_DEVREADWRITE_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_data_r, tms_rom_bank_w)
	AM_RANGE(2, 2) AM_DEVREAD_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_rom_r)
	AM_RANGE(3, 3) AM_DEVWRITE_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_left_w)
	AM_RANGE(7, 7) AM_DEVWRITE_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_right_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_DEVREAD_MODERN(DEVICE_SELF_OWNER, bsmt2000_device, tms_write_pending_r)
ADDRESS_MAP_END


// machine fragment
static MACHINE_CONFIG_FRAGMENT( bsmt2000 )
	MCFG_CPU_ADD("bsmt2000", TMS32015, DERIVED_CLOCK(1,1))
	MCFG_CPU_PROGRAM_MAP(tms_program_map)
	// data map is internal to the CPU
	MCFG_CPU_IO_MAP(tms_io_map)
MACHINE_CONFIG_END


// default address map for the external memory interface
// the BSMT can address a full 32 bits but typically only 24 are used
static ADDRESS_MAP_START( bsmt2000, 0, 8 )
	AM_RANGE(0x00000, 0xffffff) AM_ROM
ADDRESS_MAP_END


// ROM definition for the BSMT2000 program ROM
ROM_START( bsmt2000 )
	ROM_REGION( 0x2000, "bsmt2000", ROMREGION_LOADBYNAME )
	ROM_LOAD16_WORD_SWAP( "bsmt2000.bin", 0x0000, 0x2000, CRC(c2a265af) SHA1(6ec9eb038fb8eb842c5482aebe1d149daf49f2e6) )
ROM_END




//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  bsmt2000_device_config - constructor
//-------------------------------------------------

bsmt2000_device_config::bsmt2000_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "BSMT2000", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("samples", ENDIANNESS_LITTLE, 8, 32, 0, NULL, *ADDRESS_MAP_NAME(bsmt2000)),
	  m_ready_callback(NULL)
{
	m_shortname = "bsmt2000";
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *bsmt2000_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(bsmt2000_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *bsmt2000_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, bsmt2000_device(machine, *this));
}


//-------------------------------------------------
//  static_set_ready_callback - configuration
//  helper to set the ready callback
//-------------------------------------------------

void bsmt2000_device_config::static_set_ready_callback(device_config *device, ready_callback callback)
{
	bsmt2000_device_config *bsmt = downcast<bsmt2000_device_config *>(device);
	bsmt->m_ready_callback = callback;
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const rom_entry *bsmt2000_device_config::rom_region() const
{
	return ROM_NAME( bsmt2000 );
}


//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor bsmt2000_device_config::machine_config_additions() const
{
	return MACHINE_CONFIG_NAME( bsmt2000 );
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *bsmt2000_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bsmt2000_device - constructor
//-------------------------------------------------

bsmt2000_device::bsmt2000_device(running_machine &_machine, const bsmt2000_device_config &config)
	: device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  device_memory_interface(_machine, config, *this),
	  m_config(config),
	  m_stream(NULL),
	  m_direct(NULL),
	  m_cpu(NULL),
	  m_register_select(0),
	  m_write_data(0),
	  m_rom_address(0),
	  m_rom_bank(0),
	  m_left_data(0),
	  m_right_data(0),
	  m_write_pending(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bsmt2000_device::device_start()
{
	// find our CPU
	m_cpu = downcast<tms32015_device*>(subdevice("bsmt2000"));

	// find our direct access
	m_direct = &space()->direct();

	// create the stream; BSMT typically runs at 24MHz and writes to a DAC, so
	// in theory we should generate a 24MHz stream, but that's certainly overkill
	// internally at 24MHz the max output sample rate is 32kHz
	// divided by 128 gives us 6x the max output rate which is plenty for oversampling
	m_stream = m_machine.sound().stream_alloc(*this, 0, 2, clock() / 128);

	// register for save states
	save_item(NAME(m_register_select));
	save_item(NAME(m_write_data));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_left_data));
	save_item(NAME(m_right_data));
	save_item(NAME(m_write_pending));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bsmt2000_device::device_reset()
{
	synchronize(TIMER_ID_RESET);
}


//-------------------------------------------------
//  device_timer - handle deferred writes and
//  resets as a timer callback
//-------------------------------------------------

void bsmt2000_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// deferred reset
		case TIMER_ID_RESET:
			m_stream->update();
			m_cpu->reset();
			break;

		// deferred register write
		case TIMER_ID_REG_WRITE:
			m_register_select = param & 0xffff;
			break;

		// deferred data write
		case TIMER_ID_DATA_WRITE:
			m_write_data = param & 0xffff;
			if (m_write_pending) logerror("BSMT2000: Missed data\n");
			m_write_pending = true;
			break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void bsmt2000_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// just fill with current left/right values
	for (int samp = 0; samp < samples; samp++)
	{
		outputs[0][samp] = m_left_data * 16;
		outputs[1][samp] = m_right_data * 16;
	}
}


//-------------------------------------------------
//  read_status - return the write pending status
//-------------------------------------------------

UINT16 bsmt2000_device::read_status()
{
	return m_write_pending ? 0 : 1;
}


//-------------------------------------------------
//  write_reg - handle writes to the BSMT2000
//  register select interface
//-------------------------------------------------

void bsmt2000_device::write_reg(UINT16 data)
{
	synchronize(TIMER_ID_REG_WRITE, data);
}


//-------------------------------------------------
//  write_data - handle writes to the BSMT2000
//  data port
//-------------------------------------------------

void bsmt2000_device::write_data(UINT16 data)
{
	synchronize(TIMER_ID_DATA_WRITE, data);

	// boost the interleave on a write so that the caller detects the status more accurately
	m_machine.scheduler().boost_interleave(attotime::from_usec(1), attotime::from_usec(10));
}


//-------------------------------------------------
//  tms_register_r - return the value written to
//  the register select port
//-------------------------------------------------

READ16_MEMBER( bsmt2000_device::tms_register_r )
{
	return m_register_select;
}


//-------------------------------------------------
//  tms_data_r - return the value written to the
//  data port
//-------------------------------------------------

READ16_MEMBER( bsmt2000_device::tms_data_r )
{
	// also implicitly clear the write pending flag
	m_write_pending = false;
	if (m_config.m_ready_callback != NULL)
		(*m_config.m_ready_callback)(*this);
	return m_write_data;
}


//-------------------------------------------------
//  tms_rom_r - read a byte from the currently
//  selected ROM bank and address
//-------------------------------------------------

READ16_MEMBER( bsmt2000_device::tms_rom_r )
{
	// underlying logic assumes this is a sign-extended value
	return (INT8)m_direct->read_raw_byte((m_rom_bank << 16) + m_rom_address);
}


//-------------------------------------------------
//  tms_rom_addr_w - selects which byte within the
//  current ROM bank to access
//-------------------------------------------------

WRITE16_MEMBER( bsmt2000_device::tms_rom_addr_w )
{
	m_rom_address = data;
}


//-------------------------------------------------
//  tms_rom_bank_w - selects which bank of ROM to
//  access
//-------------------------------------------------

WRITE16_MEMBER( bsmt2000_device::tms_rom_bank_w )
{
	m_rom_bank = data;
}


//-------------------------------------------------
//  tms_left_w - handle writes to the left channel
//  DAC
//-------------------------------------------------

WRITE16_MEMBER( bsmt2000_device::tms_left_w )
{
	m_stream->update();
	m_left_data = data;
}


//-------------------------------------------------
//  tms_right_w - handle writes to the right
//  channel DAC
//-------------------------------------------------

WRITE16_MEMBER( bsmt2000_device::tms_right_w )
{
	m_stream->update();
	m_right_data = data;
}


//-------------------------------------------------
//  tms_write_pending_r - return whether a write
//  is pending; this data is fed into the BIO line
//  on the TMS32015
//-------------------------------------------------

READ16_MEMBER( bsmt2000_device::tms_write_pending_r )
{
	return m_write_pending ? 1 : 0;
}
