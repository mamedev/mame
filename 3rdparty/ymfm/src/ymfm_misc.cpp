// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ymfm_misc.h"

namespace ymfm
{

//*********************************************************
//  YM2149
//*********************************************************

//-------------------------------------------------
//  ym2149 - constructor
//-------------------------------------------------

ym2149::ym2149(ymfm_interface &intf) :
	m_address(0),
	m_ssg(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2149::reset()
{
	// reset the engines
	m_ssg.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2149::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_ssg.save_restore(state);
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ym2149::read_data()
{
	return m_ssg.read(m_address & 0x0f);
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2149::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 3)	// BC2,BC1
	{
		case 0: // inactive
			break;

		case 1: // address
			break;

		case 2: // inactive
			break;

		case 3: // read
			result = read_data();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2149::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2149::write_data(uint8_t data)
{
	m_ssg.write(m_address & 0x0f, data);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2149::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)	// BC2,BC1
	{
		case 0: // address
			write_address(data);
			break;

		case 1: // inactive
			break;

		case 2: // write
			write_data(data);
			break;

		case 3: // address
			write_address(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate samples of SSG sound
//-------------------------------------------------

void ym2149::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the SSG
		m_ssg.clock();

		// YM2149 keeps the three SSG outputs independent
		m_ssg.output(*output);
	}
}

}
