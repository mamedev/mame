// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#include "emu.h"
#include "sunplus_gcm394.h"

DEFINE_DEVICE_TYPE(GCM394, sunplus_gcm394_device, "gcm394", "SunPlus GCM394 System-on-a-Chip")

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}

// **************************************** TILEMAP 0 *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::tmap0_regs_r) { return tmap0_regs[offset]; }

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap0_regs_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	tmap0_regs[offset] = data;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap0_unk0_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap0_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap0_unk1_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap0_unk0_w %04x\n", machine().describe_context(), data);
}

// **************************************** TILEMAP 1 *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::tmap1_regs_r) { return tmap1_regs[offset]; }

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap1_regs_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	tmap1_regs[offset] = data;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap1_unk0_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap0_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::tmap1_unk1_w)
{
	logerror("%s:sunplus_gcm394_base_device::tmap0_unk0_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 0 (another tilemap? sprite layer?) *************************************************

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device0_regs_w)
{
	// offsets 0,1,4,5,6,7 used in main IRQ code
	// offsets 2,3 only cleared on startup

	logerror("%s:sunplus_gcm394_base_device::unknown_video_device0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device0_unk0_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device0_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device0_unk1_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device0_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 1 (another tilemap? sprite layer?) *************************************************

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device1_regs_w)
{
	// offsets 0,1,4,5,6,7 used in main IRQ code
	// offsets 2,3 only cleared on startup

	logerror("%s:sunplus_gcm394_base_device::unknown_video_device1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device1_unk0_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device1_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device1_unk1_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device1_unk1_w %04x\n", machine().describe_context(), data);
}

// **************************************** unknown video device 2 (sprite control?) *************************************************

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device2_unk0_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device2_unk0_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device2_unk1_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device2_unk1_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unknown_video_device2_unk2_w)
{
	logerror("%s:sunplus_gcm394_base_device::unknown_video_device2_unk2_w %04x\n", machine().describe_context(), data);
}

// **************************************** video DMA device *************************************************

WRITE16_MEMBER(sunplus_gcm394_base_device::video_dma_source_w)
{
	logerror("%s:sunplus_gcm394_base_device::video_dma_source_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::video_dma_dest_w)
{
	logerror("%s:sunplus_gcm394_base_device::video_dma_dest_w %04x\n", machine().describe_context(), data);
}

READ16_MEMBER(sunplus_gcm394_base_device::video_dma_size_r)
{
	logerror("%s:sunplus_gcm394_base_device::video_dma_size_r\n", machine().describe_context());
	return 0x0000;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::video_dma_size_w)
{
	logerror("%s:sunplus_gcm394_base_device::video_dma_size_w %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::video_dma_unk_w)
{
	logerror("%s:sunplus_gcm394_base_device::video_dma_unk_w %04x\n", machine().describe_context(), data);
}


// ***********************************************************************************


// **************************************** SYSTEM DMA device *************************************************

WRITE16_MEMBER(sunplus_gcm394_base_device::system_dma_params_w)
{
	m_dma_params[offset] = data;
	logerror("%s:sunplus_gcm394_base_device::sys_dma_params_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::system_dma_trigger_w)
{
	uint16_t mode = m_dma_params[0];
	uint16_t sourcelow = m_dma_params[1];
	uint16_t dest = m_dma_params[2];
	uint16_t length = m_dma_params[3];
	uint16_t srchigh = m_dma_params[4];

	logerror("%s:possible DMA operation (7abf) (trigger %04x) with params mode:%04x source:%04x dest:%04x length:%04x srchigh:%04x unk:%04x unk:%04x\n", machine().describe_context(), data, mode, sourcelow, dest, length, srchigh, m_dma_params[5], m_dma_params[6]);

	uint32_t source = sourcelow | (srchigh << 16);

	// wrlshunt uses the extra params, might be doing very large ROM -> RAM transfers with even more upper address bits?

	if (mode == 0x0089) // no source inc, used for memory clear operations? (source usually points at stack value)
	{
		for (int i = 0; i < length; i++)
		{
			address_space& mem = m_cpu->space(AS_PROGRAM);
			uint16_t val = mem.read_word(source);
			mem.write_word(dest, val);
			dest += 1;
		}
	}
	else if (mode == 0x0009) // regular copy? (smartfp does 2 copies like this after the initial clears, source definitely points at a correctly sized data structure)
	{
		for (int i = 0; i < length; i++)
		{
			address_space& mem = m_cpu->space(AS_PROGRAM);
			uint16_t val = mem.read_word(source);
			mem.write_word(dest, val);
			dest += 1;
			source += 1;
		}
	}
	else
	{
		logerror("unhandled!\n");
	}

	m_dma_params[0] = m_dma_params[1] = m_dma_params[2] = m_dma_params[3] = m_dma_params[4] = m_dma_params[5] = m_dma_params[6] = 0x0000;
	//machine().debug_break();
}


// ***********************************************************************************

// **************************************** fallthrough logger etc. *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::unk_r)
{
	switch (offset)
	{

	case 0x80f:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0002;

	case 0x8fb:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		m_78fb ^= 0x0100; // status flag for something?
		return m_78fb;

	case 0xabf:
		logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0001;

	}

	logerror("%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
	return 0x0000;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unk_w)
{

	switch (offset)
	{
	default:
		logerror("%s:sunplus_gcm394_base_device::unk_w @ 0x%04x (data 0x%04x)\n", machine().describe_context(), offset + 0x7000, data);
		break;
	}
}

void sunplus_gcm394_base_device::map(address_map &map)
{
	map(0x000000, 0x006fff).ram();
	map(0x007000, 0x007fff).rw(FUNC(sunplus_gcm394_base_device::unk_r), FUNC(sunplus_gcm394_base_device::unk_w)); // catch unhandled

	// note, tilemaps are at the same address offsets in video device as spg2xx (but unknown devices are extra)

	map(0x007000, 0x007007).w(FUNC(sunplus_gcm394_base_device::unknown_video_device0_regs_w)); // gcm394_video_device::
	map(0x007008, 0x00700f).w(FUNC(sunplus_gcm394_base_device::unknown_video_device1_regs_w)); // gcm394_video_device::

	map(0x007010, 0x007015).rw(FUNC(sunplus_gcm394_base_device::tmap0_regs_r), FUNC(sunplus_gcm394_base_device::tmap0_regs_w)); // gcm394_video_device::
	map(0x007016, 0x00701b).rw(FUNC(sunplus_gcm394_base_device::tmap1_regs_r), FUNC(sunplus_gcm394_base_device::tmap1_regs_w)); // gcm394_video_device::

	map(0x007020, 0x007020).w(FUNC(sunplus_gcm394_base_device::tmap0_unk0_w));                 // gcm394_video_device::  probably tilebase, written with other tmap0 regs
	map(0x007021, 0x007021).w(FUNC(sunplus_gcm394_base_device::tmap1_unk0_w));                 // gcm394_video_device::  probably tilebase, written with other tmap1 regs
	map(0x007022, 0x007022).w(FUNC(sunplus_gcm394_base_device::unknown_video_device2_unk0_w)); // gcm394_video_device::  another tilebase? maybe sprites? written as 7022, 702d and 7042 group
	map(0x007023, 0x007023).w(FUNC(sunplus_gcm394_base_device::unknown_video_device0_unk0_w)); // gcm394_video_device::  written with other unknown_video_device0 regs
	map(0x007024, 0x007024).w(FUNC(sunplus_gcm394_base_device::unknown_video_device1_unk0_w)); // gcm394_video_device::  written with other unknown_video_device1 regs

	map(0x00702b, 0x00702b).w(FUNC(sunplus_gcm394_base_device::tmap0_unk1_w));                 // gcm394_video_device::   written with other tmap0 regs
	map(0x00702c, 0x00702c).w(FUNC(sunplus_gcm394_base_device::tmap1_unk1_w));                 // gcm394_video_device::   written with other tmap1 regs
	map(0x00702d, 0x00702d).w(FUNC(sunplus_gcm394_base_device::unknown_video_device2_unk1_w)); // gcm394_video_device::  maybe sprites?  written as 7022, 702d and 7042 group
	map(0x00702e, 0x00702e).w(FUNC(sunplus_gcm394_base_device::unknown_video_device0_unk1_w)); // gcm394_video_device::  written with other unknown_video_device0 regs
	map(0x00702f, 0x00702f).w(FUNC(sunplus_gcm394_base_device::unknown_video_device1_unk1_w)); // gcm394_video_device::  written with other unknown_video_device1 regs

	map(0x007042, 0x007042).w(FUNC(sunplus_gcm394_base_device::unknown_video_device2_unk2_w)); // gcm394_video_device::  maybe sprites?  written as 7022, 702d and 7042 group

	// note, 70 / 71 / 72 are the same offsets used for DMA as in spg2xx video device
	map(0x007070, 0x007070).w(FUNC(sunplus_gcm394_base_device::video_dma_source_w));                                                      // gcm394_video_device::  video dma, not system dma? (sets pointers to ram buffers)
	map(0x007071, 0x007071).w(FUNC(sunplus_gcm394_base_device::video_dma_dest_w));                                                        // gcm394_video_device::  sets pointers to 7300, 7400 ram areas below
	map(0x007072, 0x007072).rw(FUNC(sunplus_gcm394_base_device::video_dma_size_r), FUNC(sunplus_gcm394_base_device::video_dma_size_w));   // gcm394_video_device:: 
	
	map(0x00707e, 0x00707e).w(FUNC(sunplus_gcm394_base_device::video_dma_unk_w));                                                         // gcm394_video_device::  written around same time as DMA, seems related

	map(0x007300, 0x0073ff).ram();
	map(0x007400, 0x0074ff).ram();
	map(0x007500, 0x0075ff).ram();
	map(0x007600, 0x0076ff).ram();
	map(0x007700, 0x0077ff).ram();

	map(0x007a80, 0x007a86).w(FUNC(sunplus_gcm394_base_device::system_dma_params_w));
	map(0x007abf, 0x007abf).w(FUNC(sunplus_gcm394_base_device::system_dma_trigger_w));	

	map(0x007c00, 0x007cff).ram();
	map(0x007d00, 0x007dff).ram();
	map(0x007e00, 0x007eff).ram();
	map(0x007f00, 0x007fff).ram();
}

void sunplus_gcm394_base_device::device_start()
{
}

void sunplus_gcm394_base_device::device_reset()
{
	m_78fb = 0x0000;

	for (int i = 0; i < 7; i++)
	{
		m_dma_params[i] = 0x0000;
	}
}

void sunplus_gcm394_device::device_add_mconfig(machine_config &config)
{
	//SUNPLUS_GCM394_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	//m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	//m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);
}
