// license:LGPL-2.1+
// copyright-holders:R. Belmont, Brad Martin
/***************************************************************************

  s_smp.cpp

  File to handle the S-SMP emulation used in Nintendo Super NES.

  By R. Belmont, adapted from OpenSPC 0.3.99 by Brad Martin with permission.
  Thanks to Brad and also to Charles Bilyu? of SNeESe.

  OpenSPC's license terms (the LGPL) follow:

 ---------------------------------------------------------------------------

  Copyright Brad Martin.

  OpenSPC is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OpenSPC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

***************************************************************************/

#include "emu.h"
#include "s_smp.h"

/***************************************************************************
 CONSTANTS AND MACROS
***************************************************************************/


// Nintendo/Sony S-SMP internal ROM region
ROM_START( s_smp )
	ROM_REGION( 0x40, "sound_ipl", 0 )     /* IPL ROM */
	ROM_LOAD( "spc700.rom", 0, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) /* boot rom */
ROM_END


void s_smp_device::internal_map(address_map &map)
{
	map(0x0000, 0x00ef).lrw8(
		[this](offs_t offset) -> u8 { return data_read_byte(offset); }, "data_r",
		[this](offs_t offset, u8 data) { data_write_byte(offset, data); }, "data_w");
	map(0x00f0, 0x00ff).rw(FUNC(s_smp_device::io_r), FUNC(s_smp_device::io_w));
	map(0x0100, 0xffff).lrw8(
		[this](offs_t offset) -> u8 { return data_read_byte(offset + 0x100); }, "data_100_r",
		[this](offs_t offset, u8 data) { data_write_byte(offset + 0x100, data); }, "data_100_w");
}


DEFINE_DEVICE_TYPE(S_SMP, s_smp_device, "s_smp", "Nintendo/Sony S-SMP")


s_smp_device::s_smp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: spc700_device(mconfig, S_SMP, tag, owner, clock, address_map_constructor(FUNC(s_smp_device::internal_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16)
	, m_ipl_region(*this, "sound_ipl")
	, m_dsp_io_r_cb(*this)
	, m_dsp_io_w_cb(*this)
{
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *s_smp_device::device_rom_region() const
{
	return ROM_NAME( s_smp );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s_smp_device::device_start()
{
	m_dsp_io_r_cb.resolve_safe(0);
	m_dsp_io_w_cb.resolve_safe();

	space(AS_DATA).specific(m_data);
	space(AS_DATA).cache(m_dcache);

	m_tick_timer = timer_alloc(FUNC(s_smp_device::update_timers), this);

	save_item(NAME(m_timer_enabled));
	save_item(NAME(m_subcounter));
	save_item(NAME(m_counter));
	save_item(NAME(m_port_in));
	save_item(NAME(m_port_out));
	save_item(NAME(m_test));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_counter_reg));

	save_item(NAME(m_TnDIV));
	spc700_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s_smp_device::device_reset()
{
	int i;
	/* default to ROM visible */
	m_ctrl = 0x80;

	/* Sort out the ports */
	for (i = 0; i < 4; i++)
	{
		m_port_in[i] = 0;
		m_port_out[i] = 0;
	}

	for (i = 0; i < 3; i++)
	{
		m_timer_enabled[i] = false;
		m_TnDIV[i] = 256;
		m_counter[i] = 0;
		m_subcounter[i] = 0;
	}

	attotime period = attotime::from_ticks(32, clock());
	m_tick_timer->adjust(period, 0, period);
	spc700_device::device_reset();
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void s_smp_device::device_clock_changed()
{
	attotime period = attotime::from_ticks(32, clock());
	m_tick_timer->adjust(period, 0, period);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector s_smp_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

inline void s_smp_device::update_timer_tick(u8 which)
{
	if (m_timer_enabled[which] == false)
		return;

	m_subcounter[which]++;

	// if timer channel is 0 or 1 we update at 64000/8
	if (m_subcounter[which] >= 8 || which == 2)
	{
		m_subcounter[which] = 0;
		m_counter[which]++;
		if (m_counter[which] >= m_TnDIV[which] ) // minus =
		{
			m_counter[which] = 0;
			m_counter_reg[which]++;
			m_counter_reg[which] &= 0x0f;
		}
	}
}

TIMER_CALLBACK_MEMBER(s_smp_device::update_timers)
{
	for (int ch = 0; ch < 3; ch++)
		update_timer_tick(ch);
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/


/***************************
        I/O for S-SMP
 ***************************/

u8 s_smp_device::io_r(offs_t offset)
{
	switch (offset) /* Offset is from 0x00f0 */
	{
		case 0x0: //FIXME: Super Bomberman PBW reads from there, is it really write-only?
			return 0;
		case 0x1:
			return 0; //Super Kick Boxing reads port 1 and wants it to be zero.
		case 0x2:       /* Register address */
		case 0x3:       /* Register data */
			return m_dsp_io_r_cb(offset - 0x2);
		case 0x4:       /* Port 0 */
		case 0x5:       /* Port 1 */
		case 0x6:       /* Port 2 */
		case 0x7:       /* Port 3 */
			// osd_printf_debug("%s SPC: rd %02x @ %d\n", machine().describe_context(), m_port_in[offset - 4], offset - 4);
			return m_port_in[offset - 4];
		case 0x8: //normal RAM, can be read even if the ram disabled flag ($f0 bit 1) is active
		case 0x9:
			return data_read_byte(0xf0 + offset);
		case 0xa:       /* Timer 0 */
		case 0xb:       /* Timer 1 */
		case 0xc:       /* Timer 2 */
			break;
		case 0xd:       /* Counter 0 */
		case 0xe:       /* Counter 1 */
		case 0xf:       /* Counter 2 */
		{
			u8 value = m_counter_reg[offset - 0xd] & 0x0f;
			if (!machine().side_effects_disabled())
				m_counter_reg[offset - 0xd] = 0;
			return value;
		}
	}

	return 0;
}

void s_smp_device::io_w(offs_t offset, u8 data)
{
	switch (offset) /* Offset is from 0x00f0 */
	{
		case 0x0:
			m_test = data;
			logerror("Warning: write to SOUND TEST register with data %02x!\n", data);
			break;
		case 0x1:       /* Control */
			m_ctrl = data;
			for (int i = 0; i < 3; i++)
			{
				if (BIT(data, i) && m_timer_enabled[i] == false)
				{
					m_subcounter[i] = 0;
					m_counter[i] = 0;
					m_counter_reg[i] = 0;
				}

				m_timer_enabled[i] = BIT(data, i);
				//m_timer[i]->enable(m_timer_enabled[i]);
			}

			if (BIT(data, 4))
			{
				m_port_in[0] = 0;
				m_port_in[1] = 0;
			}

			if (BIT(data, 5))
			{
				m_port_in[2] = 0;
				m_port_in[3] = 0;
			}

			/* bit 7 = IPL ROM enable */
			break;
		case 0x2:       /* Register address */
		case 0x3:       /* Register data - 0x80-0xff is a read-only mirror of 0x00-0x7f */
			m_dsp_io_w_cb(offset - 0x2, data);
			break;
		case 0x4:       /* Port 0 */
		case 0x5:       /* Port 1 */
		case 0x6:       /* Port 2 */
		case 0x7:       /* Port 3 */
			// osd_printf_debug("%s SPC: %02x to APU @ %d\n", machine().describe_context(), data, offset & 3);
			m_port_out[offset - 4] = data;
			// Unneeded, we already run at perfect_interleave
			// machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(20));
			break;
		case 0xa:       /* Timer 0 */
		case 0xb:       /* Timer 1 */
		case 0xc:       /* Timer 2 */
			// if 0 then TnDiv is divided by 256, otherwise it's divided by 1 to 255
			if (data == 0)
				m_TnDIV[offset - 0xa] = 256;
			else
				m_TnDIV[offset - 0xa] = data;
			break;
		case 0xd:       /* Counter 0 */
		case 0xe:       /* Counter 1 */
		case 0xf:       /* Counter 2 */
			return;
	}

	data_write_byte(0xf0 + offset, data);
}


u8 s_smp_device::spc_port_out_r(offs_t offset)
{
	assert(offset < 4);

	return m_port_out[offset];
}

void s_smp_device::spc_port_in_w(offs_t offset, u8 data)
{
	assert(offset < 4);

	m_port_in[offset] = data;
}
