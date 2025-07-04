// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    bsmt2000.cpp

    BSMT2000 device emulator.

****************************************************************************

    Chip is actually a TMS320C15 DSP with embedded mask rom
    Trivia: BSMT stands for "Brian Schmidt's Mouse Trap"

***************************************************************************/

#include "emu.h"
#include "bsmt2000.h"


// device type definition
DEFINE_DEVICE_TYPE(BSMT2000, bsmt2000_device, "bsmt2000", "BSMT2000")


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// program map for the DSP (points to internal ROM)
void bsmt2000_device::tms_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0xfff).rom();
}


// I/O map for the DSP
void bsmt2000_device::tms_io_map(address_map &map)
{
	map(0, 0).rw(FUNC(bsmt2000_device::tms_register_r), FUNC(bsmt2000_device::tms_rom_addr_w));
	map(1, 1).rw(FUNC(bsmt2000_device::tms_data_r), FUNC(bsmt2000_device::tms_rom_bank_w));
	map(2, 2).r(FUNC(bsmt2000_device::tms_rom_r));
	map(3, 3).w(FUNC(bsmt2000_device::tms_left_w));
	map(7, 7).w(FUNC(bsmt2000_device::tms_right_w));
}


// ROM definition for the BSMT2000 program ROM
ROM_START( bsmt2000 )
	ROM_REGION( 0x2000, "bsmt2000", 0 )
	ROM_LOAD16_WORD( "bsmt2000.bin", 0x0000, 0x2000, CRC(c2a265af) SHA1(6ec9eb038fb8eb842c5482aebe1d149daf49f2e6) )
ROM_END




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bsmt2000_device - constructor
//-------------------------------------------------

bsmt2000_device::bsmt2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BSMT2000, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_ready_callback(*this)
	, m_stream(nullptr)
	, m_cpu(*this, "bsmt2000")
	, m_register_select(0)
	, m_write_data(0)
	, m_rom_address(0)
	, m_rom_bank(0)
	, m_left_data(0)
	, m_right_data(0)
	, m_write_pending(false)
{
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *bsmt2000_device::device_rom_region() const
{
	return ROM_NAME( bsmt2000 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bsmt2000_device::device_add_mconfig(machine_config &config)
{
	tms32015_device &tms(TMS32015(config, "bsmt2000", DERIVED_CLOCK(1,1)));
	tms.set_addrmap(AS_PROGRAM, &bsmt2000_device::tms_program_map);
	// data map is internal to the CPU
	tms.set_addrmap(AS_IO, &bsmt2000_device::tms_io_map);
	tms.bio().set(FUNC(bsmt2000_device::tms_write_pending_r));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bsmt2000_device::device_start()
{
	m_ready_callback.resolve_safe();

	// create the stream; BSMT typically runs at 24MHz and writes to a DAC, so
	// in theory we should generate a 24MHz stream, but that's certainly overkill
	// internally at 24MHz the max output sample rate is 32kHz
	// divided by 128 gives us 6x the max output rate which is plenty for oversampling
	m_stream = stream_alloc(0, 2, clock() / 128);

	// register for save states
	save_item(NAME(m_register_select));
	save_item(NAME(m_write_data));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_left_data));
	save_item(NAME(m_right_data));
	save_item(NAME(m_write_pending));

	// allocate timers
	m_deferred_reset = timer_alloc(FUNC(bsmt2000_device::deferred_reset), this);
	m_deferred_reg_write = timer_alloc(FUNC(bsmt2000_device::deferred_reg_write), this);
	m_deferred_data_write = timer_alloc(FUNC(bsmt2000_device::deferred_data_write), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bsmt2000_device::device_reset()
{
	m_deferred_reset->adjust(attotime::zero);
}



//-------------------------------------------------
//  deferred_reset -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(bsmt2000_device::deferred_reset)
{
	m_stream->update();
	m_cpu->reset();
}



//-------------------------------------------------
//  deferred_reg_write -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(bsmt2000_device::deferred_reg_write)
{
	m_register_select = param & 0xffff;
}



//-------------------------------------------------
//  deferred_data_write -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(bsmt2000_device::deferred_data_write)
{
	m_write_data = param & 0xffff;
	if (m_write_pending) logerror("BSMT2000: Missed data\n");
	m_write_pending = true;
}



//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void bsmt2000_device::sound_stream_update(sound_stream &stream)
{
	// just fill with current left/right values
	constexpr sound_stream::sample_t sample_scale = 1.0 / 32768.0;
	stream.fill(0, sound_stream::sample_t(m_left_data) * sample_scale);
	stream.fill(1, sound_stream::sample_t(m_right_data) * sample_scale);
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void bsmt2000_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  read_status - return the write pending status
//-------------------------------------------------

uint16_t bsmt2000_device::read_status()
{
	return m_write_pending ? 0 : 1;
}


//-------------------------------------------------
//  write_reg - handle writes to the BSMT2000
//  register select interface
//-------------------------------------------------

void bsmt2000_device::write_reg(uint16_t data)
{
	m_deferred_reg_write->adjust(attotime::zero, data);
}


//-------------------------------------------------
//  write_data - handle writes to the BSMT2000
//  data port
//-------------------------------------------------

void bsmt2000_device::write_data(uint16_t data)
{
	m_deferred_data_write->adjust(attotime::zero, data);

	// boost the interleave on a write so that the caller detects the status more accurately
	machine().scheduler().add_quantum(attotime::from_usec(1), attotime::from_usec(10));
}


//-------------------------------------------------
//  tms_register_r - return the value written to
//  the register select port
//-------------------------------------------------

uint16_t bsmt2000_device::tms_register_r()
{
	return m_register_select;
}


//-------------------------------------------------
//  tms_data_r - return the value written to the
//  data port
//-------------------------------------------------

uint16_t bsmt2000_device::tms_data_r()
{
	// also implicitly clear the write pending flag
	m_write_pending = false;
	m_ready_callback();
	return m_write_data;
}


//-------------------------------------------------
//  tms_rom_r - read a byte from the currently
//  selected ROM bank and address
//-------------------------------------------------

uint16_t bsmt2000_device::tms_rom_r()
{
	// DSP code expects a 16-bit value with the data in the high byte
	return (int16_t)(read_byte((m_rom_bank << 16) + m_rom_address) << 8);
}


//-------------------------------------------------
//  tms_rom_addr_w - selects which byte within the
//  current ROM bank to access
//-------------------------------------------------

void bsmt2000_device::tms_rom_addr_w(uint16_t data)
{
	m_rom_address = data;
}


//-------------------------------------------------
//  tms_rom_bank_w - selects which bank of ROM to
//  access
//-------------------------------------------------

void bsmt2000_device::tms_rom_bank_w(uint16_t data)
{
	m_rom_bank = data;
}


//-------------------------------------------------
//  tms_left_w - handle writes to the left channel
//  DAC
//-------------------------------------------------

void bsmt2000_device::tms_left_w(uint16_t data)
{
	m_stream->update();
	m_left_data = data;
}


//-------------------------------------------------
//  tms_right_w - handle writes to the right
//  channel DAC
//-------------------------------------------------

void bsmt2000_device::tms_right_w(uint16_t data)
{
	m_stream->update();
	m_right_data = data;
}


//-------------------------------------------------
//  tms_write_pending_r - return whether a write
//  is pending; this data is fed into the BIO line
//  on the TMS32015
//-------------------------------------------------

int bsmt2000_device::tms_write_pending_r()
{
	return m_write_pending ? 1 : 0;
}
