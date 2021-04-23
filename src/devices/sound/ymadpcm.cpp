// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymadpcm.h"

//#define VERBOSE 1
#define LOG_OUTPUT_FUNC osd_printf_verbose
#include "logmacro.h"


//*********************************************************
//  DEBUGGING
//*********************************************************

// set this to only play certain channels: bits 0-5 are ADPCM-A
// channels and bit 0x80 is the ADPCM-B channel
constexpr u8 global_chanmask = 0xff;



//*********************************************************
//  MACROS
//*********************************************************

// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace
#define ADPCM_A_NAME(x) x, "adpcma." #x
#define ADPCM_B_NAME(x) x, "adpcmb." #x



//*********************************************************
// ADPCM "A" REGISTERS
//*********************************************************

//-------------------------------------------------
//  ymadpcm_a_registers - constructor
//-------------------------------------------------

void ymadpcm_a_registers::save(device_t &device)
{
	device.save_item(ADPCM_A_NAME(m_regdata));
}


//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void ymadpcm_a_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// initialize the pans to on by default, and max instrument volume;
	// some neogeo homebrews (for example ffeast) rely on this
	m_regdata[0x08] = m_regdata[0x09] = m_regdata[0x0a] =
	m_regdata[0x0b] = m_regdata[0x0c] = m_regdata[0x0d] = 0xdf;
}


//*********************************************************
// ADPCM "A" CHANNEL
//*********************************************************

//-------------------------------------------------
//  ymadpcm_a_channel - constructor
//-------------------------------------------------

ymadpcm_a_channel::ymadpcm_a_channel(ymadpcm_a_engine &owner, u32 choffs, read8sm_delegate reader, u32 addrshift) :
	m_choffs(choffs),
	m_address_shift(addrshift),
	m_playing(0),
	m_curnibble(0),
	m_curbyte(0),
	m_curaddress(0),
	m_accumulator(0),
	m_step_index(0),
	m_reader(std::move(reader)),
	m_regs(owner.regs())
{
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

void ymadpcm_a_channel::save(device_t &device, u32 index)
{
	device.save_item(ADPCM_A_NAME(m_playing), index);
	device.save_item(ADPCM_A_NAME(m_curnibble), index);
	device.save_item(ADPCM_A_NAME(m_curbyte), index);
	device.save_item(ADPCM_A_NAME(m_curaddress), index);
	device.save_item(ADPCM_A_NAME(m_accumulator), index);
	device.save_item(ADPCM_A_NAME(m_step_index), index);
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

void ymadpcm_a_channel::reset()
{
	m_playing = 0;
	m_curnibble = 0;
	m_curbyte = 0;
	m_curaddress = 0;
	m_accumulator = 0;
	m_step_index = 0;
}


//-------------------------------------------------
//  keyonoff - signal key on/off
//-------------------------------------------------

void ymadpcm_a_channel::keyonoff(bool on)
{
	// QUESTION: repeated key ons restart the sample?
	m_playing = on;
	if (m_playing)
	{
		m_curaddress = m_regs.ch_start(m_choffs) << m_address_shift;
		m_curnibble = 0;
		m_curbyte = 0;
		m_accumulator = 0;
		m_step_index = 0;

		// don't log masked channels
		if (((global_chanmask >> m_choffs) & 1) != 0)
			LOG("KeyOn ADPCM-A%d: pan=%d%d start=%04X end=%04X level=%02X\n",
				m_choffs,
				m_regs.ch_pan_left(m_choffs),
				m_regs.ch_pan_right(m_choffs),
				m_regs.ch_start(m_choffs),
				m_regs.ch_end(m_choffs),
				m_regs.ch_instrument_level(m_choffs));
	}
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

bool ymadpcm_a_channel::clock()
{
	// if not playing, just output 0
	if (m_playing == 0)
	{
		m_accumulator = 0;
		return false;
	}

	// stop when we hit the end address; apparently only low 20 bits are used for
	// comparison on the YM2610: this affects sample playback in some games, for
	// example twinspri character select screen music will skip some samples if
	// this is not correct
	if (((m_curaddress ^ (m_regs.ch_end(m_choffs) << m_address_shift)) & 0xfffff) == 0)
	{
		m_playing = m_accumulator = 0;
		return true;
	}

	// if we're about to read nibble 0, fetch the data
	u8 data;
	if (m_curnibble == 0)
	{
		m_curbyte = m_reader(m_curaddress++);
		data = m_curbyte >> 4;
		m_curnibble = 1;
	}

	// otherwise just extract from the previosuly-fetched byte
	else
	{
		data = m_curbyte & 0xf;
		m_curnibble = 0;
	}

	// compute the ADPCM delta
	static u16 const s_steps[49] =
	{
		 16,  17,   19,   21,   23,   25,   28,
		 31,  34,   37,   41,   45,   50,   55,
		 60,  66,   73,   80,   88,   97,  107,
		118, 130,  143,  157,  173,  190,  209,
		230, 253,  279,  307,  337,  371,  408,
		449, 494,  544,  598,  658,  724,  796,
		876, 963, 1060, 1166, 1282, 1411, 1552
	};
	s32 delta = (2 * BIT(data, 0, 3) + 1) * s_steps[m_step_index] / 8;
	if (BIT(data, 3))
		delta = -delta;

	// the 12-bit accumulator wraps on the ym2610 and ym2608 (like the msm5205)
	m_accumulator = (m_accumulator + delta) & 0xfff;

	// adjust ADPCM step
	static s8 const s_step_inc[8] = { -1, -1, -1, -1, 2, 5, 7, 9 };
	m_step_index = std::clamp(m_step_index + s_step_inc[BIT(data, 0, 3)], 0, 48);

	return false;
}


//-------------------------------------------------
//  output - return the computed output value, with
//  panning applied
//-------------------------------------------------

void ymadpcm_a_channel::output(s32 outputs[2]) const
{
	// volume combined instrument and total levels
	int vol = (m_regs.ch_instrument_level(m_choffs) ^ 0x1f) + (m_regs.total_level() ^ 0x3f);

	// if combined is maximum, don't add to outputs
	if (vol >= 63)
		return;

	// convert into a shift and a multiplier
	// QUESTION: verify this from other sources
	s8 mul = 15 - (vol & 7);
	u8 shift = 4 + 1 + (vol >> 3);

	// m_accumulator is a 12-bit value; shift up to sign-extend;
	// the downshift is incorporated into 'shift'
	s16 value = ((s16(m_accumulator << 4) * mul) >> shift) & ~3;

	// apply to left/right as appropriate
	if (m_regs.ch_pan_left(m_choffs))
		outputs[0] += value;
	if (m_regs.ch_pan_right(m_choffs))
		outputs[1] += value;
}



//*********************************************************
// ADPCM "A" ENGINE
//*********************************************************

//-------------------------------------------------
//  ymadpcm_a_engine - constructor
//-------------------------------------------------

ymadpcm_a_engine::ymadpcm_a_engine(device_t &device, read8sm_delegate reader, u32 addrshift)
{
	// create the channels
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		m_channel[chnum] = std::make_unique<ymadpcm_a_channel>(*this, chnum, reader, addrshift);
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

void ymadpcm_a_engine::save(device_t &device)
{
	// save register state
	m_regs.save(device);

	// save channel state
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		m_channel[chnum]->save(device, chnum);
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void ymadpcm_a_engine::reset()
{
	// reset register state
	m_regs.reset();

	// reset each channel
	for (auto &chan : m_channel)
		chan->reset();
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

u32 ymadpcm_a_engine::clock(u32 chanmask)
{
	// clock each channel, setting a bit in result if it finished
	u32 result = 0;
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		if (BIT(chanmask, chnum))
			if (m_channel[chnum]->clock())
				result |= 1 << chnum;

	// return the bitmask of completed samples
	return result;
}


//-------------------------------------------------
//  update - master update function
//-------------------------------------------------

void ymadpcm_a_engine::output(s32 outputs[2], u32 chanmask)
{
	// mask out some channels for debug purposes
	chanmask &= global_chanmask;

	// compute the output of each channel
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->output(outputs);
}


//-------------------------------------------------
//  write - handle writes to the ADPCM-A registers
//-------------------------------------------------

void ymadpcm_a_engine::write(u32 regnum, u8 data)
{
	// store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// actively handle writes to the control register
	if (regnum == 0x00)
		for (int chnum = 0; chnum < std::size(m_channel); chnum++)
			if (BIT(data, chnum))
				m_channel[chnum]->keyonoff(BIT(~data, 7));
}



//*********************************************************
// ADPCM "B" REGISTERS
//*********************************************************

//-------------------------------------------------
//  ymadpcm_b_registers - constructor
//-------------------------------------------------

void ymadpcm_b_registers::save(device_t &device)
{
	device.save_item(ADPCM_B_NAME(m_regdata));
}


//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void ymadpcm_b_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// default limit to wide open
	m_regdata[0x0c] = m_regdata[0x0d] = 0xff;
}


//*********************************************************
// ADPCM "B" CHANNEL
//*********************************************************

//-------------------------------------------------
//  ymadpcm_b_channel - constructor
//-------------------------------------------------

ymadpcm_b_channel::ymadpcm_b_channel(ymadpcm_b_engine &owner, read8sm_delegate reader, write8sm_delegate writer, u32 addrshift) :
	m_address_shift(addrshift),
	m_status(STATUS_BRDY),
	m_curnibble(0),
	m_curbyte(0),
	m_dummy_read(0),
	m_position(0),
	m_curaddress(0),
	m_accumulator(0),
	m_prev_accum(0),
	m_adpcm_step(STEP_MIN),
	m_reader(reader),
	m_writer(writer),
	m_regs(owner.regs())
{
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

void ymadpcm_b_channel::save(device_t &device, u32 index)
{
	device.save_item(ADPCM_B_NAME(m_status), index);
	device.save_item(ADPCM_B_NAME(m_curnibble), index);
	device.save_item(ADPCM_B_NAME(m_curbyte), index);
	device.save_item(ADPCM_B_NAME(m_dummy_read), index);
	device.save_item(ADPCM_B_NAME(m_position), index);
	device.save_item(ADPCM_B_NAME(m_curaddress), index);
	device.save_item(ADPCM_B_NAME(m_accumulator), index);
	device.save_item(ADPCM_B_NAME(m_prev_accum), index);
	device.save_item(ADPCM_B_NAME(m_adpcm_step), index);
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

void ymadpcm_b_channel::reset()
{
	m_status = STATUS_BRDY;
	m_curnibble = 0;
	m_curbyte = 0;
	m_dummy_read = 0;
	m_position = 0;
	m_curaddress = 0;
	m_accumulator = 0;
	m_prev_accum = 0;
	m_adpcm_step = STEP_MIN;
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void ymadpcm_b_channel::clock()
{
	// only process if active and not recording (which we don't support)
	if (!m_regs.execute() || m_regs.record() || (m_status & STATUS_PLAYING) == 0)
	{
		m_status &= ~STATUS_PLAYING;
		return;
	}

	// otherwise, advance the step
	u32 position = m_position + m_regs.delta_n();
	m_position = u16(position);
	if (position < 0x10000)
		return;

	// if playing from RAM/ROM, check the end address and process
	if (m_regs.external())
	{
		// wrap at the limit address
		if (at_limit())
			m_curaddress = 0;

		// handle the sample end, either repeating or stopping
		if (at_end())
		{
			// if repeating, go back to the start
			if (m_regs.repeat())
				load_start();

			// otherwise, done; set the EOS bit and return
			else
			{
				m_accumulator = 0;
				m_prev_accum = 0;
				m_status = (m_status & ~STATUS_PLAYING) | STATUS_EOS;
				LOG("ADPCM EOS\n");
				return;
			}
		}

		// if we're about to process nibble 0, fetch and increment
		if (m_curnibble == 0)
		{
			m_curbyte = m_reader(m_curaddress++);
			m_curaddress &= 0xffffff;
		}
	}

	// extract the nibble from our current byte
	u8 data = u8(m_curbyte << (4 * m_curnibble)) >> 4;
	m_curnibble ^= 1;

	// if CPU-driven and we just processed the last nibble, copy the next byte and request more
	if (m_curnibble == 0 && !m_regs.external())
	{
		m_curbyte = m_regs.cpudata();
		m_status |= STATUS_BRDY;
	}

	// remember previous value for interpolation
	m_prev_accum = m_accumulator;

	// forecast to next forecast: 1/8, 3/8, 5/8, 7/8, 9/8, 11/8, 13/8, 15/8
	s32 delta = (2 * BIT(data, 0, 3) + 1) * m_adpcm_step / 8;
	if (BIT(data, 3))
		delta = -delta;

	// add and clamp to 16 bits
	m_accumulator = std::clamp(m_accumulator + delta, -32768, 32767);

	// scale the ADPCM step: 0.9, 0.9, 0.9, 0.9, 1.2, 1.6, 2.0, 2.4
	static u8 const s_step_scale[8] = { 57, 57, 57, 57, 77, 102, 128, 153 };
	m_adpcm_step = std::clamp((m_adpcm_step * s_step_scale[BIT(data, 0, 3)]) / 64, STEP_MIN, STEP_MAX);
}


//-------------------------------------------------
//  output - return the computed output value, with
//  panning applied
//-------------------------------------------------

void ymadpcm_b_channel::output(s32 outputs[2], u32 rshift) const
{
	// mask out some channels for debug purposes
	if ((global_chanmask & 0x80) == 0)
		return;

	// do a linear interpolation between samples
	s32 result = (m_prev_accum * s32((m_position ^ 0xffff) + 1) + m_accumulator * s32(m_position)) >> 16;

	// apply volume (level) in a linear fashion and reduce
	result = (result * s32(m_regs.level())) >> (8 + rshift);

	// apply to left/right
	if (m_regs.pan_left())
		outputs[0] += result;
	if (m_regs.pan_right())
		outputs[1] += result;
}


//-------------------------------------------------
//  read - handle special register reads
//-------------------------------------------------

u8 ymadpcm_b_channel::read(u32 regnum)
{
	u8 result = 0;

	// register 8 reads over the bus under some conditions
	if (regnum == 0x08 && !m_regs.execute() && !m_regs.record() && m_regs.external())
	{
		// two dummy reads are consumed first
		if (m_dummy_read != 0)
		{
			load_start();
			m_dummy_read--;
		}

		// did we hit the end? if so, signal EOS
		if (at_end())
		{
			m_status = STATUS_EOS | STATUS_BRDY;
			LOG("ADPCM EOS\n");
		}

		// otherwise, write the data and signal ready
		else
		{
			result = m_reader(m_curaddress++);
			m_status = STATUS_BRDY;
		}
	}
	return result;
}


//-------------------------------------------------
//  write - handle special register writes
//-------------------------------------------------

void ymadpcm_b_channel::write(u32 regnum, u8 value)
{
	// register 0 can do a reset; also use writes here to reset the
	// dummy read counter
	if (regnum == 0x00)
	{
		if (m_regs.execute())
		{
			load_start();

			// don't log masked channels
			if ((global_chanmask & 0x80) != 0)
				LOG("KeyOn ADPCM-B: rep=%d spk=%d pan=%d%d dac=%d 8b=%d rom=%d ext=%d rec=%d start=%04X end=%04X pre=%04X dn=%04X lvl=%02X lim=%04X\n",
					m_regs.repeat(),
					m_regs.speaker(),
					m_regs.pan_left(),
					m_regs.pan_right(),
					m_regs.dac_enable(),
					m_regs.dram_8bit(),
					m_regs.rom_ram(),
					m_regs.external(),
					m_regs.record(),
					m_regs.start(),
					m_regs.end(),
					m_regs.prescale(),
					m_regs.delta_n(),
					m_regs.level(),
					m_regs.limit());
		}
		else
			m_status &= ~STATUS_EOS;
		if (m_regs.resetflag())
			reset();
		if (m_regs.external())
			m_dummy_read = 2;
	}

	// register 8 writes over the bus under some conditions
	else if (regnum == 0x08)
	{
		// if writing from the CPU during execute, clear the ready flag
		if (m_regs.execute() && !m_regs.record() && !m_regs.external())
			m_status &= ~STATUS_BRDY;

		// if writing during "record", pass through as data
		else if (!m_regs.execute() && m_regs.record() && m_regs.external())
		{
			// clear out dummy reads and set start address
			if (m_dummy_read != 0)
			{
				load_start();
				m_dummy_read = 0;
			}

			// did we hit the end? if so, signal EOS
			if (at_end())
			{
				LOG("ADPCM EOS\n");
				m_status = STATUS_EOS | STATUS_BRDY;
			}

			// otherwise, write the data and signal ready
			else
			{
				m_writer(m_curaddress++, value);
				m_status = STATUS_BRDY;
			}
		}
	}
}


//-------------------------------------------------
//  address_shift - compute the current address
//  shift amount based on register settings
//-------------------------------------------------

u32 ymadpcm_b_channel::address_shift() const
{
	// if a constant address shift, just provide that
	if (m_address_shift != 0)
		return m_address_shift;

	// if ROM or 8-bit DRAM, shift is 5 bits
	if (m_regs.rom_ram())
		return 5;
	if (m_regs.dram_8bit())
		return 5;

	// otherwise, shift is 2 bits
	return 2;
}


//-------------------------------------------------
//  load_start - load the start address and
//  initialize the state
//-------------------------------------------------

void ymadpcm_b_channel::load_start()
{
	m_status = (m_status & ~STATUS_EOS) | STATUS_PLAYING;
	m_curaddress = m_regs.external() ? (m_regs.start() << address_shift()) : 0;
	m_curnibble = 0;
	m_curbyte = 0;
	m_position = 0;
	m_accumulator = 0;
	m_prev_accum = 0;
	m_adpcm_step = STEP_MIN;
}



//*********************************************************
// ADPCM "B" ENGINE
//*********************************************************

//-------------------------------------------------
//  ymadpcm_b_engine - constructor
//-------------------------------------------------

ymadpcm_b_engine::ymadpcm_b_engine(device_t &device, read8sm_delegate reader, write8sm_delegate writer, u32 addrshift)
{
	// create the channel (only one supported for now, but leaving possibilities open)
	m_channel[0] = std::make_unique<ymadpcm_b_channel>(*this, reader, writer, addrshift);
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

void ymadpcm_b_engine::save(device_t &device)
{
	// save our state
	m_regs.save(device);

	// save channel state
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		m_channel[chnum]->save(device, chnum);
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void ymadpcm_b_engine::reset()
{
	// reset registers
	m_regs.reset();

	// reset each channel
	for (auto &chan : m_channel)
		chan->reset();
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void ymadpcm_b_engine::clock(u32 chanmask)
{
	// clock each channel, setting a bit in result if it finished
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->clock();
}


//-------------------------------------------------
//  output - master output function
//-------------------------------------------------

void ymadpcm_b_engine::output(s32 outputs[2], u32 rshift, u32 chanmask)
{
	// compute the output of each channel
	for (int chnum = 0; chnum < std::size(m_channel); chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->output(outputs, rshift);
}


//-------------------------------------------------
//  write - handle writes to the ADPCM-B registers
//-------------------------------------------------

void ymadpcm_b_engine::write(u32 regnum, u8 data)
{
	// store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// let the channel handle any special writes
	m_channel[0]->write(regnum, data);
}
