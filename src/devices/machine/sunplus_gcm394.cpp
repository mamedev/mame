// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#include "emu.h"
#include "sunplus_gcm394.h"



#define LOG_GCM394_SYSDMA         (1U << 2)
#define LOG_GCM394                (1U << 1)
#define LOG_GCM394_UNMAPPED       (1U << 0)

#define VERBOSE             (LOG_GCM394_UNMAPPED | LOG_GCM394_SYSDMA)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(GCM394, sunplus_gcm394_device, "gcm394", "SunPlus GCM394 System-on-a-Chip")

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}




// **************************************** SYSTEM DMA device *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::system_dma_status_r)
{
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::system_dma_status_r (7abf)\n", machine().describe_context());
	return 0x0001;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::system_dma_params_w)
{
	m_dma_params[offset] = data;
	LOGMASKED(LOG_GCM394_SYSDMA, "%s:sunplus_gcm394_base_device::sys_dma_params_w %01x %04x\n", machine().describe_context(), offset, data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::system_dma_trigger_w)
{
	uint16_t mode = m_dma_params[0];
	uint16_t sourcelow = m_dma_params[1];
	uint16_t dest = m_dma_params[2];
	uint16_t length = m_dma_params[3];
	uint16_t srchigh = m_dma_params[4];

	LOGMASKED(LOG_GCM394_SYSDMA, "%s:possible DMA operation (7abf) (trigger %04x) with params mode:%04x source:%04x dest:%04x length:%04x srchigh:%04x unk:%04x unk:%04x\n", machine().describe_context(), data, mode, sourcelow, dest, length, srchigh, m_dma_params[5], m_dma_params[6]);

	uint32_t source = sourcelow | (srchigh << 16);

	// wrlshunt uses the extra params, might be doing very large ROM -> RAM transfers with even more upper address bits?

	if (mode == 0x0089) // no source inc, used for memory clear operations? (source usually points at stack value)
	{
		for (int i = 0; i < length; i++)
		{
			address_space &mem = this->space(AS_PROGRAM);
			uint16_t val = mem.read_word(source);
			mem.write_word(dest, val);
			dest += 1;
		}
	}
	else if (mode == 0x0009) // regular copy? (smartfp does 2 copies like this after the initial clears, source definitely points at a correctly sized data structure)
	{
		for (int i = 0; i < length; i++)
		{
			address_space &mem = this->space(AS_PROGRAM);
			uint16_t val = mem.read_word(source);
			mem.write_word(dest, val);
			dest += 1;
			source += 1;
		}
	}
	else
	{
		LOGMASKED(LOG_GCM394_SYSDMA, "unhandled!\n");
	}

	m_dma_params[0] = m_dma_params[1] = m_dma_params[2] = m_dma_params[3] = m_dma_params[4] = m_dma_params[5] = m_dma_params[6] = 0x0000;
	//machine().debug_break();
}



// **************************************** 78xx region with some handling *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_780f_status_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_780f_status_r\n", machine().describe_context());
	return 0x0002;
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_78fb_status_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78fb_status_r\n", machine().describe_context());
	m_78fb ^= 0x0100; // status flag for something?
	return m_78fb;
}

// sets bit 0x0002 then expects it to have cleared
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7819_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7819_r\n", machine().describe_context()); return m_7819 & ~ 0x0002; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7819_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7819_w %04x\n", machine().describe_context(), data); m_7819 = data; }

// ****************************************  78xx region stubs *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7868_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7868_r\n", machine().describe_context()); return 0x0000; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_782d_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_782d_r\n", machine().describe_context()); return m_782d; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_782d_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_782d_w %04x\n", machine().describe_context(), data); m_782d = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7803_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7803_r\n", machine().describe_context()); return m_7803; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7803_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7803_w %04x\n", machine().describe_context(), data); m_7803 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7807_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7807_w %04x\n", machine().describe_context(), data); m_7807 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7810_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7810_r\n", machine().describe_context()); return m_7810; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7810_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7810_w %04x\n", machine().describe_context(), data); m_7810 = data; }


WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7816_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7816_w %04x\n", machine().describe_context(), data); m_7816 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7817_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7817_w %04x\n", machine().describe_context(), data); m_7817 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7820_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7820_w %04x\n", machine().describe_context(), data); m_7820 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7821_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7821_w %04x\n", machine().describe_context(), data); m_7821 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7822_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7822_w %04x\n", machine().describe_context(), data); m_7822 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7823_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7823_w %04x\n", machine().describe_context(), data); m_7823 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7824_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7824_w %04x\n", machine().describe_context(), data); m_7824 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7835_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7835_w %04x\n", machine().describe_context(), data); m_7835 = data; }

// IO here?

READ16_MEMBER(sunplus_gcm394_base_device::ioport_a_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioport_a_r\n", machine().describe_context()); return m_porta_in();  }
WRITE16_MEMBER(sunplus_gcm394_base_device::ioport_a_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioport_a_w %04x\n", machine().describe_context(), data); m_7860 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7861_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7861_r\n", machine().describe_context()); return m_7861; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7862_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7862_r\n", machine().describe_context()); return m_7862; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7862_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7862_w %04x\n", machine().describe_context(), data); m_7862 = data; }
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7863_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7863_r\n", machine().describe_context()); return m_7863; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7863_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7863_w %04x\n", machine().describe_context(), data); m_7863 = data; }

// similar read/write pattern to above, 2nd group?

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7870_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7870_r\n", machine().describe_context()); return m_portb_in(); }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7870_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7870_w %04x\n", machine().describe_context(), data); m_7870 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7871_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7871_r\n", machine().describe_context()); return m_7871; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7872_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7872_r\n", machine().describe_context()); return m_7872; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7872_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7872_w %04x\n", machine().describe_context(), data); m_7872 = data; }
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7873_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7873_r\n", machine().describe_context()); return m_7873; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7873_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7873_w %04x\n", machine().describe_context(), data); m_7873 = data; }



READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7882_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_r\n", machine().describe_context()); return m_7882; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7882_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_w %04x\n", machine().describe_context(), data); m_7882 = data; }
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7883_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_r\n", machine().describe_context()); return m_7883; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7883_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_w %04x\n", machine().describe_context(), data); m_7883 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a0_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a0_w %04x\n", machine().describe_context(), data); m_78a0 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a4_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a4_w %04x\n", machine().describe_context(), data); m_78a4 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a5_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a5_w %04x\n", machine().describe_context(), data); m_78a5 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a6_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a6_w %04x\n", machine().describe_context(), data); m_78a6 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a8_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a8_w %04x\n", machine().describe_context(), data); m_78a8 = data; }


WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b0_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b0_w %04x\n", machine().describe_context(), data); m_78b0 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b1_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b1_w %04x\n", machine().describe_context(), data); m_78b1 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b2_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b2_w %04x\n", machine().describe_context(), data); m_78b2 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b8_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b8_w %04x\n", machine().describe_context(), data); m_78b8 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78f0_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78f0_w %04x\n", machine().describe_context(), data); m_78f0 = data; }

// **************************************** 79xx region stubs *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7934_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7934_r\n", machine().describe_context()); return 0x0000; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7934_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7934_w %04x\n", machine().describe_context(), data); m_7934 = data; }

// value of 7935 is read then written in irq6, nothing happens unless bit 0x0100 was set, which could be some kind of irq source being acked?
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7935_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7935_r\n", machine().describe_context()); return m_7935; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7935_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7935_w %04x\n", machine().describe_context(), data);
	m_7935 &= ~data;
	checkirq6();
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7936_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7936_r\n", machine().describe_context()); return 0x0000; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7936_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7936_w %04x\n", machine().describe_context(), data); m_7936 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7960_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7960_w %04x\n", machine().describe_context(), data); m_7960 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7961_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_r\n", machine().describe_context()); return m_7961; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7961_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7961_w %04x\n", machine().describe_context(), data); m_7961 = data; }


// **************************************** fallthrough logger etc. *************************************************

READ16_MEMBER(sunplus_gcm394_base_device::unk_r)
{
	switch (offset)
	{
	default:
		LOGMASKED(LOG_GCM394_UNMAPPED, "%s:sunplus_gcm394_base_device::unk_r @ 0x%04x\n", machine().describe_context(), offset + 0x7000);
		return 0x0000;
	}

	return 0x0000;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unk_w)
{

	switch (offset)
	{
	default:
		LOGMASKED(LOG_GCM394_UNMAPPED, "%s:sunplus_gcm394_base_device::unk_w @ 0x%04x (data 0x%04x)\n", machine().describe_context(), offset + 0x7000, data);
		break;
	}
}

void sunplus_gcm394_base_device::internal_map(address_map &map)
{
	map(0x000000, 0x006fff).ram();
	map(0x007000, 0x007fff).rw(FUNC(sunplus_gcm394_base_device::unk_r), FUNC(sunplus_gcm394_base_device::unk_w)); // catch unhandled

	// ######################################################################################################################################################################################
	// 70xx region = video hardware
	// ######################################################################################################################################################################################

	// note, tilemaps are at the same address offsets in video device as spg2xx (but unknown devices are extra)

	map(0x007000, 0x007007).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device0_regs_w));
	map(0x007008, 0x00700f).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device1_regs_w));

	map(0x007010, 0x007015).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_regs_r), FUNC(gcm394_base_video_device::tmap0_regs_w));
	map(0x007016, 0x00701b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_regs_r), FUNC(gcm394_base_video_device::tmap1_regs_w));

	map(0x007020, 0x007020).w(m_spg_video, FUNC(gcm394_base_video_device::tmap0_unk0_w));                 // probably tilebase, written with other tmap0 regs
	map(0x007021, 0x007021).w(m_spg_video, FUNC(gcm394_base_video_device::tmap1_unk0_w));                 // probably tilebase, written with other tmap1 regs
	map(0x007022, 0x007022).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device2_unk0_w)); // another tilebase? maybe sprites? written as 7022, 702d and 7042 group
	map(0x007023, 0x007023).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device0_unk0_w)); // written with other unknown_video_device0 regs
	map(0x007024, 0x007024).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device1_unk0_w)); // written with other unknown_video_device1 regs

	map(0x00702a, 0x00702a).w(m_spg_video, FUNC(gcm394_base_video_device::video_702a_w));

	map(0x00702b, 0x00702b).w(m_spg_video, FUNC(gcm394_base_video_device::tmap0_unk1_w));                 // written with other tmap0 regs
	map(0x00702c, 0x00702c).w(m_spg_video, FUNC(gcm394_base_video_device::tmap1_unk1_w));                 // written with other tmap1 regs
	map(0x00702d, 0x00702d).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device2_unk1_w)); // maybe sprites?  written as 7022, 702d and 7042 group
	map(0x00702e, 0x00702e).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device0_unk1_w)); // written with other unknown_video_device0 regs
	map(0x00702f, 0x00702f).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device1_unk1_w)); // written with other unknown_video_device1 regs

	map(0x007030, 0x007030).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7030_r), FUNC(gcm394_base_video_device::video_7030_w));
	map(0x00703a, 0x00703a).rw(m_spg_video, FUNC(gcm394_base_video_device::video_703a_r), FUNC(gcm394_base_video_device::video_703a_w));
	map(0x00703c, 0x00703c).w(m_spg_video, FUNC(gcm394_base_video_device::video_703c_w));

	map(0x007042, 0x007042).w(m_spg_video, FUNC(gcm394_base_video_device::unknown_video_device2_unk2_w)); // maybe sprites?  written as 7022, 702d and 7042 group

	map(0x007062, 0x007062).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7062_r), FUNC(gcm394_base_video_device::video_7062_w));
	map(0x007063, 0x007063).w(m_spg_video, FUNC(gcm394_base_video_device::video_7063_w));

	// note, 70 / 71 / 72 are the same offsets used for DMA as in spg2xx video device
	map(0x007070, 0x007070).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_source_w));                                                      // video dma, not system dma? (sets pointers to ram buffers)
	map(0x007071, 0x007071).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_dest_w));                                                        // sets pointers to 7300, 7400 ram areas below
	map(0x007072, 0x007072).rw(m_spg_video, FUNC(gcm394_base_video_device::video_dma_size_r), FUNC(gcm394_base_video_device::video_dma_size_w));     //
	map(0x00707e, 0x00707e).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_unk_w));                                                         // written around same time as DMA, seems to select alt sprite bank

	map(0x00707f, 0x00707f).rw(m_spg_video, FUNC(gcm394_base_video_device::video_707f_r), FUNC(gcm394_base_video_device::video_707f_w));

	// another set of registers for something?
	map(0x007080, 0x007080).w(m_spg_video, FUNC(gcm394_base_video_device::video_7080_w));
	map(0x007081, 0x007081).w(m_spg_video, FUNC(gcm394_base_video_device::video_7081_w));
	map(0x007082, 0x007082).w(m_spg_video, FUNC(gcm394_base_video_device::video_7082_w));
	map(0x007083, 0x007083).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7083_r), FUNC(gcm394_base_video_device::video_7083_w));
	map(0x007084, 0x007084).w(m_spg_video, FUNC(gcm394_base_video_device::video_7084_w));
	map(0x007085, 0x007085).w(m_spg_video, FUNC(gcm394_base_video_device::video_7085_w));
	map(0x007086, 0x007086).w(m_spg_video, FUNC(gcm394_base_video_device::video_7086_w));
	map(0x007087, 0x007087).w(m_spg_video, FUNC(gcm394_base_video_device::video_7087_w));
	map(0x007088, 0x007088).w(m_spg_video, FUNC(gcm394_base_video_device::video_7088_w));

	// ######################################################################################################################################################################################
	// 73xx-77xx = video ram
	// ######################################################################################################################################################################################

	map(0x007300, 0x0073ff).rw(m_spg_video, FUNC(gcm394_base_video_device::palette_r), FUNC(gcm394_base_video_device::palette_w));

	map(0x007400, 0x0077ff).ram().share("spriteram");

	// ######################################################################################################################################################################################
	// 78xx region = io + other?
	// ######################################################################################################################################################################################

	map(0x007803, 0x007803).rw(FUNC(sunplus_gcm394_base_device::unkarea_7803_r), FUNC(sunplus_gcm394_base_device::unkarea_7803_w));

	map(0x007807, 0x007807).w(FUNC(sunplus_gcm394_base_device::unkarea_7807_w));

	map(0x00780f, 0x00780f).r(FUNC(sunplus_gcm394_base_device::unkarea_780f_status_r));

	map(0x007810, 0x007810).rw(FUNC(sunplus_gcm394_base_device::unkarea_7810_r), FUNC(sunplus_gcm394_base_device::unkarea_7810_w));

	map(0x007819, 0x007819).rw(FUNC(sunplus_gcm394_base_device::unkarea_7819_r), FUNC(sunplus_gcm394_base_device::unkarea_7819_w));

	map(0x007816, 0x007816).w(FUNC(sunplus_gcm394_base_device::unkarea_7816_w));
	map(0x007817, 0x007817).w(FUNC(sunplus_gcm394_base_device::unkarea_7817_w));


	map(0x007820, 0x007820).w(FUNC(sunplus_gcm394_base_device::unkarea_7820_w));
	map(0x007821, 0x007821).w(FUNC(sunplus_gcm394_base_device::unkarea_7821_w));
	map(0x007822, 0x007822).w(FUNC(sunplus_gcm394_base_device::unkarea_7822_w));
	map(0x007823, 0x007823).w(FUNC(sunplus_gcm394_base_device::unkarea_7823_w));
	map(0x007824, 0x007824).w(FUNC(sunplus_gcm394_base_device::unkarea_7824_w));

	map(0x00782d, 0x00782d).rw(FUNC(sunplus_gcm394_base_device::unkarea_782d_r), FUNC(sunplus_gcm394_base_device::unkarea_782d_w)); // on startup

	map(0x007835, 0x007835).w(FUNC(sunplus_gcm394_base_device::unkarea_7835_w));

	map(0x007860, 0x007860).rw(FUNC(sunplus_gcm394_base_device::ioport_a_r), FUNC(sunplus_gcm394_base_device::ioport_a_w));

	map(0x007861, 0x007861).r(FUNC(sunplus_gcm394_base_device::unkarea_7861_r));
	map(0x007862, 0x007862).rw(FUNC(sunplus_gcm394_base_device::unkarea_7862_r), FUNC(sunplus_gcm394_base_device::unkarea_7862_w));
	map(0x007863, 0x007863).rw(FUNC(sunplus_gcm394_base_device::unkarea_7863_r), FUNC(sunplus_gcm394_base_device::unkarea_7863_w));

	map(0x007868, 0x007868).r(FUNC(sunplus_gcm394_base_device::unkarea_7868_r)); // on startup

	map(0x007870, 0x007870).rw(FUNC(sunplus_gcm394_base_device::unkarea_7870_r) ,FUNC(sunplus_gcm394_base_device::unkarea_7870_w));

	map(0x007871, 0x007871).r(FUNC(sunplus_gcm394_base_device::unkarea_7871_r));
	map(0x007872, 0x007872).rw(FUNC(sunplus_gcm394_base_device::unkarea_7872_r), FUNC(sunplus_gcm394_base_device::unkarea_7872_w));
	map(0x007873, 0x007873).rw(FUNC(sunplus_gcm394_base_device::unkarea_7873_r), FUNC(sunplus_gcm394_base_device::unkarea_7873_w));

	map(0x007882, 0x007882).rw(FUNC(sunplus_gcm394_base_device::unkarea_7882_r), FUNC(sunplus_gcm394_base_device::unkarea_7882_w));
	map(0x007883, 0x007883).rw(FUNC(sunplus_gcm394_base_device::unkarea_7883_r), FUNC(sunplus_gcm394_base_device::unkarea_7883_w));

	map(0x0078a0, 0x0078a0).w(FUNC(sunplus_gcm394_base_device::unkarea_78a0_w));

	map(0x0078a4, 0x0078a4).w(FUNC(sunplus_gcm394_base_device::unkarea_78a4_w));
	map(0x0078a5, 0x0078a5).w(FUNC(sunplus_gcm394_base_device::unkarea_78a5_w));
	map(0x0078a6, 0x0078a6).w(FUNC(sunplus_gcm394_base_device::unkarea_78a6_w));

	map(0x0078a8, 0x0078a8).w(FUNC(sunplus_gcm394_base_device::unkarea_78a8_w));

	map(0x0078b0, 0x0078b0).w(FUNC(sunplus_gcm394_base_device::unkarea_78b0_w));
	map(0x0078b1, 0x0078b1).w(FUNC(sunplus_gcm394_base_device::unkarea_78b1_w));
	map(0x0078b2, 0x0078b2).w(FUNC(sunplus_gcm394_base_device::unkarea_78b2_w));

	map(0x0078b8, 0x0078b8).w(FUNC(sunplus_gcm394_base_device::unkarea_78b8_w));

	map(0x0078f0, 0x0078f0).w(FUNC(sunplus_gcm394_base_device::unkarea_78f0_w));

	map(0x0078fb, 0x0078fb).r(FUNC(sunplus_gcm394_base_device::unkarea_78fb_status_r));

	// ######################################################################################################################################################################################
	// 79xx region = timers?
	// ######################################################################################################################################################################################

	map(0x007934, 0x007934).rw(FUNC(sunplus_gcm394_base_device::unkarea_7934_r), FUNC(sunplus_gcm394_base_device::unkarea_7934_w));
	map(0x007935, 0x007935).rw(FUNC(sunplus_gcm394_base_device::unkarea_7935_r), FUNC(sunplus_gcm394_base_device::unkarea_7935_w));
	map(0x007936, 0x007936).rw(FUNC(sunplus_gcm394_base_device::unkarea_7936_r), FUNC(sunplus_gcm394_base_device::unkarea_7936_w));

	map(0x007960, 0x007960).w(FUNC(sunplus_gcm394_base_device::unkarea_7960_w));
	map(0x007961, 0x007961).rw(FUNC(sunplus_gcm394_base_device::unkarea_7961_r), FUNC(sunplus_gcm394_base_device::unkarea_7961_w));

	// ######################################################################################################################################################################################
	// 7axx region = system (including dma)
	// ######################################################################################################################################################################################

	map(0x007a80, 0x007a86).w(FUNC(sunplus_gcm394_base_device::system_dma_params_w));
	map(0x007abf, 0x007abf).rw(FUNC(sunplus_gcm394_base_device::system_dma_status_r), FUNC(sunplus_gcm394_base_device::system_dma_trigger_w));

	// ######################################################################################################################################################################################
	// 7bxx-7fxx = audio
	// ######################################################################################################################################################################################

	map(0x007b80, 0x007bbf).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::control_r), FUNC(sunplus_gcm394_audio_device::control_w));
	map(0x007c00, 0x007dff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_r), FUNC(sunplus_gcm394_audio_device::audio_w));
	map(0x007e00, 0x007fff).rw(m_spg_audio, FUNC(sunplus_gcm394_audio_device::audio_phase_r), FUNC(sunplus_gcm394_audio_device::audio_phase_w));
}

void sunplus_gcm394_base_device::device_start()
{
	unsp_20_device::device_start();

	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);

	m_unk_timer = timer_alloc(0);
	m_unk_timer->adjust(attotime::never);
}

void sunplus_gcm394_base_device::device_reset()
{
	unsp_20_device::device_reset();

	for (int i = 0; i < 7; i++)
	{
		m_dma_params[i] = 0x0000;
	}

	// 78xx unknown

	m_78fb = 0x0000;
	m_782d = 0x0000;

	m_7807 = 0x0000;

	m_7810 = 0x0000;

	m_7816 = 0x0000;
	m_7817 = 0x0000;

	m_7819 = 0x0000;

	m_7820 = 0x0000;
	m_7821 = 0x0000;
	m_7822 = 0x0000;
	m_7823 = 0x0000;
	m_7824 = 0x0000;

	m_7835 = 0x0000;

	m_7860 = 0x0000;

	m_7861 = 0x0000;

	m_7862 = 0x0000;
	m_7863 = 0x0000;

	m_7870 = 0x0000;

	m_7871 = 0x0000;

	m_7872 = 0x0000;
	m_7873 = 0x0000;

	m_7882 = 0x0000;
	m_7883 = 0x0000;

	m_78a0 = 0x0000;

	m_78a4 = 0x0000;
	m_78a5 = 0x0000;
	m_78a6 = 0x0000;

	m_78a8 = 0x0000;

	m_78b0 = 0x0000;
	m_78b1 = 0x0000;
	m_78b2 = 0x0000;

	m_78b8 = 0x0000;
	m_78f0 = 0x0000;

	// 79xx unknown

	m_7934 = 0x0000;
	m_7935 = 0x0000;
	m_7936 = 0x0000;

	m_7960 = 0x0000;
	m_7961 = 0x0000;


	m_unk_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));

	m_gfxregion = memregion(":maincpu")->base();
	m_gfxregionsize = memregion(":maincpu")->bytes();

}

void sunplus_gcm394_base_device::checkirq6()
{
	if (m_7935 & 0x0100)
		set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
	else
		set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);
}

void sunplus_gcm394_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case 0:
	{
		m_7935 |= 0x0100;
		checkirq6();
		break;
	}
	}
}


WRITE_LINE_MEMBER(sunplus_gcm394_base_device::audioirq_w)
{
	//set_state_unsynced(UNSP_IRQ5_LINE, state);
}

WRITE_LINE_MEMBER(sunplus_gcm394_base_device::videoirq_w)
{
	set_state_unsynced(UNSP_IRQ5_LINE, state);
}

uint16_t sunplus_gcm394_base_device::read_space(uint32_t offset)
{
	uint16_t b = m_gfxregion[(offset * 2) & (m_gfxregionsize - 1)] | (m_gfxregion[(offset * 2 + 1) & (m_gfxregionsize - 1)] << 8);
	return b;
}


void sunplus_gcm394_base_device::device_add_mconfig(machine_config &config)
{
	SUNPLUS_GCM394_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(sunplus_gcm394_base_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(sunplus_gcm394_base_device::read_space));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

	GCM394_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	m_spg_video->write_video_irq_callback().set(FUNC(sunplus_gcm394_base_device::videoirq_w));
	m_spg_video->space_read_callback().set(FUNC(sunplus_gcm394_base_device::read_space));
}


