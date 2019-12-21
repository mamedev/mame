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

#define VERBOSE             (LOG_GCM394 | LOG_GCM394_UNMAPPED | LOG_GCM394_SYSDMA)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(GCM394, sunplus_gcm394_device, "gcm394", "SunPlus GCM394 System-on-a-Chip")

sunplus_gcm394_device::sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sunplus_gcm394_base_device(mconfig, GCM394, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(GPAC800, generalplus_gpac800_device, "gpac800", "GeneralPlus GPAC800 System-on-a-Chip")

generalplus_gpac800_device::generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sunplus_gcm394_base_device(mconfig, GPAC800, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpac800_device::gpac800_internal_map), this))
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
	uint32_t source = m_dma_params[1] | (m_dma_params[4] << 16);
	uint32_t dest = m_dma_params[2] | (m_dma_params[5] << 16) ;
	uint32_t length = m_dma_params[3] | (m_dma_params[6] << 16);

	LOGMASKED(LOG_GCM394_SYSDMA, "%s:possible DMA operation (7abf) (trigger %04x) with params mode:%04x source:%08x (word offset) dest:%08x (word offset) length:%08x (words)\n", machine().describe_context(), data, mode, source, dest, length );

	if (source >= 0x20000)
		LOGMASKED(LOG_GCM394_SYSDMA, " likely transfer from ROM %08x - %08x\n", (source - 0x20000) * 2, (source - 0x20000) * 2 + (length * 2)- 1);

	// wrlshunt transfers ROM to RAM, all RAM write addresses have 0x800000 in the destination set

	// mode 0x0089 == no source inc, used for memory clear operations? (source usually points at stack value)
	// mode 0x0009 == regular copy? (smartfp does 2 copies like this after the initial clears, source definitely points at a correctly sized data structure)
	// what does having bit 0x4000 on mode set mean? (first transfer on wrlshunt - maybe an IRQ disable?)

	if ((mode == 0x0089) || (mode == 0x0009) || (mode == 0x4009))
	{
		for (int i = 0; i < length; i++)
		{
			uint16_t val = 0x0000;

			address_space& mem = this->space(AS_PROGRAM);

			if (source < 0x20000)
			{
				val = mem.read_word(source);
			}
			else
			{
				// maybe the -0x20000 here should be handled in external space handlers instead
				val = m_space_read_cb(space, source - 0x20000);
			}

			if (dest < 0x20000)
			{
				mem.write_word(dest, val);
			}
			else
			{
				// maybe the -0x20000 here should be handled in external space handlers instead
				m_space_write_cb(space, dest - 0x20000, val);
			}

			dest += 1;
			if ((mode&0x3fff) == 0x0009)
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


// this gets stored / modified / restored before certain memory accesses (in practical terms it just seems to flip from 0/1) maybe it changes the memory mapping (as 7820 seems to)
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7810_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7810_r\n", machine().describe_context());
	return m_7810;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7810_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7810_w %04x\n", machine().describe_context(), data);
	m_7810 = data;
}


WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7816_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7816_w %04x\n", machine().describe_context(), data); m_7816 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7817_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7817_w %04x\n", machine().describe_context(), data); m_7817 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7820_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7820_w (Rom Mapping control?) %04x\n", machine().describe_context(), data);
	m_7820 = data;
	m_mapping_write_cb(data);
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7821_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7821_w %04x\n", machine().describe_context(), data); m_7821 = data;
}
// these seem to be related to the above but don't change after startup.  maybe it's how different devices see memory?
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7822_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7822_w %04x\n", machine().describe_context(), data); m_7822 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7823_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7823_w %04x\n", machine().describe_context(), data); m_7823 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7824_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7824_w %04x\n", machine().describe_context(), data); m_7824 = data; }


WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7835_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7835_w %04x\n", machine().describe_context(), data); m_7835 = data; }

// IO here?

READ16_MEMBER(sunplus_gcm394_base_device::ioarea_7860_porta_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioarea_7860_porta_r\n", machine().describe_context());
	return m_porta_in();
}

WRITE16_MEMBER(sunplus_gcm394_base_device::ioarea_7860_porta_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioarea_7860_porta_w %04x\n", machine().describe_context(), data);
	m_porta_out(data);
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7861_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7861_r\n", machine().describe_context());
	return 0x0000;// 0xffff;// m_7861;
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7862_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7862_r\n", machine().describe_context());
	return 0x0000;// m_7862;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7862_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7862_w %04x\n", machine().describe_context(), data);
	m_7862 = data;
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7863_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7863_r\n", machine().describe_context());
	return 0x0000; //0xffff;// m_7863;
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7863_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7863_w %04x\n", machine().describe_context(), data);
	m_7863 = data;
}

// similar read/write pattern to above, 2nd group?

READ16_MEMBER(sunplus_gcm394_base_device::ioarea_7870_portb_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioarea_7870_portb_r\n", machine().describe_context());
	return m_portb_in();
}

WRITE16_MEMBER(sunplus_gcm394_base_device::ioarea_7870_portb_w)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::ioarea_7870_portb_w %04x\n", machine().describe_context(), data);
	m_7870 = data;
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7871_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7871_r\n", machine().describe_context()); return 0xffff;// m_7871;
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7872_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7872_r\n", machine().describe_context()); return 0xffff;// m_7872;
}
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7872_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7872_w %04x\n", machine().describe_context(), data); m_7872 = data; }
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7873_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7873_r\n", machine().describe_context()); return 0xffff;// m_7873;
}
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7873_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7873_w %04x\n", machine().describe_context(), data); m_7873 = data; }



READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7882_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_r\n", machine().describe_context()); return 0xffff;// m_7882;
}
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7882_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7882_w %04x\n", machine().describe_context(), data); m_7882 = data; }
READ16_MEMBER(sunplus_gcm394_base_device::unkarea_7883_r) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_r\n", machine().describe_context()); return 0xffff;// m_7883;
}
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_7883_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_7883_w %04x\n", machine().describe_context(), data); m_7883 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a0_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a0_w %04x\n", machine().describe_context(), data); m_78a0 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_78a0_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a0_r\n", machine().describe_context());
	return machine().rand();
}

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_78a1_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a1_r\n", machine().describe_context());
	return 0xffff;// machine().rand();
}

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a4_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a4_w %04x\n", machine().describe_context(), data); m_78a4 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a5_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a5_w %04x\n", machine().describe_context(), data); m_78a5 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a6_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a6_w %04x\n", machine().describe_context(), data); m_78a6 = data; }

WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78a8_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78a8_w %04x\n", machine().describe_context(), data); m_78a8 = data; }


WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b0_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b0_w %04x\n", machine().describe_context(), data); m_78b0 = data; }
WRITE16_MEMBER(sunplus_gcm394_base_device::unkarea_78b1_w) { LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b1_w %04x\n", machine().describe_context(), data); m_78b1 = data; }

READ16_MEMBER(sunplus_gcm394_base_device::unkarea_78b2_r)
{
	LOGMASKED(LOG_GCM394, "%s:sunplus_gcm394_base_device::unkarea_78b2_r\n", machine().describe_context());
	return 0xffff;// m_78b2;
}

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
	//checkirq6();
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

void sunplus_gcm394_base_device::gcm394_internal_map(address_map &map)
{
	map(0x000000, 0x006fff).ram();
	map(0x007000, 0x007fff).rw(FUNC(sunplus_gcm394_base_device::unk_r), FUNC(sunplus_gcm394_base_device::unk_w)); // catch unhandled

	// ######################################################################################################################################################################################
	// 70xx region = video hardware
	// ######################################################################################################################################################################################

	// note, tilemaps are at the same address offsets in video device as spg2xx (but unknown devices are extra)

	map(0x007000, 0x007007).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid1_regs_w)); // written with other unknown_video_device1 LSB/MSB regs below (roz layer or line layer?)
	map(0x007008, 0x00700f).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid2_regs_w)); // written with other unknown_video_device2 LSB/MSB regs below (roz layer or line layer?)

	map(0x007010, 0x007015).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap0_regs_r), FUNC(gcm394_base_video_device::tmap0_regs_w));
	map(0x007016, 0x00701b).rw(m_spg_video, FUNC(gcm394_base_video_device::tmap1_regs_r), FUNC(gcm394_base_video_device::tmap1_regs_w));

	// tilebase LSBs
	map(0x007020, 0x007020).w(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_lsb_w));      // tilebase, written with other tmap0 regs
	map(0x007021, 0x007021).w(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_lsb_w));      // tilebase, written with other tmap1 regs
	map(0x007022, 0x007022).w(m_spg_video, FUNC(gcm394_base_video_device::sprite_7022_gfxbase_lsb_w)); // sprite tilebase written as 7022, 702d and 7042 group
	map(0x007023, 0x007023).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid1_gfxbase_lsb_w));    // written with other unknown_video_device1 regs (roz layer or line layer?)
	map(0x007024, 0x007024).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid2_gfxbase_lsb_w));    // written with other unknown_video_device2 regs (roz layer or line layer?)

	map(0x00702a, 0x00702a).w(m_spg_video, FUNC(gcm394_base_video_device::video_702a_w)); // blend level control

	// tilebase MSBs
	map(0x00702b, 0x00702b).w(m_spg_video, FUNC(gcm394_base_video_device::tmap0_tilebase_msb_w));      // written with other tmap0 regs
	map(0x00702c, 0x00702c).w(m_spg_video, FUNC(gcm394_base_video_device::tmap1_tilebase_msb_w));      // written with other tmap1 regs
	map(0x00702d, 0x00702d).w(m_spg_video, FUNC(gcm394_base_video_device::sprite_702d_gfxbase_msb_w)); // sprites, written as 7022, 702d and 7042 group
	map(0x00702e, 0x00702e).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid1_gfxbase_msb_w));    // written with other unknown_video_device1 regs (roz layer or line layer?)
	map(0x00702f, 0x00702f).w(m_spg_video, FUNC(gcm394_base_video_device::unk_vid2_gfxbase_msb_w));    // written with other unknown_video_device2 regs (roz layer or line layer?)

	map(0x007030, 0x007030).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7030_brightness_r), FUNC(gcm394_base_video_device::video_7030_brightness_w));
	map(0x007038, 0x007038).r(m_spg_video, FUNC(gcm394_base_video_device::video_curline_r));
	map(0x00703a, 0x00703a).rw(m_spg_video, FUNC(gcm394_base_video_device::video_703a_palettebank_r), FUNC(gcm394_base_video_device::video_703a_palettebank_w));
	map(0x00703c, 0x00703c).w(m_spg_video, FUNC(gcm394_base_video_device::video_703c_w)); // TV Control 1

	map(0x007042, 0x007042).rw(m_spg_video, FUNC(gcm394_base_video_device::sprite_7042_extra_r), FUNC(gcm394_base_video_device::sprite_7042_extra_w)); // maybe sprites,  written as 7022, 702d and 7042 group

	map(0x007051, 0x007051).r(m_spg_video, FUNC(gcm394_base_video_device::video_7051_r)); // wrlshunt checks this (doesn't exist on older SPG?)

	map(0x007062, 0x007062).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7062_r), FUNC(gcm394_base_video_device::video_7062_w));
	map(0x007063, 0x007063).rw(m_spg_video, FUNC(gcm394_base_video_device::video_7063_videoirq_source_r), FUNC(gcm394_base_video_device::video_7063_videoirq_source_ack_w));

	// note, 70 / 71 / 72 are the same offsets used for DMA as in spg2xx video device
	map(0x007070, 0x007070).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_source_w));                                                      // video dma, not system dma? (sets pointers to ram buffers)
	map(0x007071, 0x007071).w(m_spg_video, FUNC(gcm394_base_video_device::video_dma_dest_w));                                                        // sets pointers to 7300, 7400 ram areas below
	map(0x007072, 0x007072).rw(m_spg_video, FUNC(gcm394_base_video_device::video_dma_size_busy_r), FUNC(gcm394_base_video_device::video_dma_size_trigger_w));     //

	// these don't exist on older SPG
	map(0x00707c, 0x00707c).r(m_spg_video, FUNC(gcm394_base_video_device::video_707c_r)); // wrlshunt polls this waiting for 0x8000, is this some kind of manual port based data upload?

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

	map(0x007100, 0x0071ef).ram().share("unkram1"); // maybe a line table? (assuming DMA isn't writing to wrong place)
	map(0x007200, 0x0072ef).ram().share("unkram2"); // ^^

	map(0x007300, 0x0073ff).rw(m_spg_video, FUNC(gcm394_base_video_device::palette_r), FUNC(gcm394_base_video_device::palette_w));

	map(0x007400, 0x0077ff).rw(m_spg_video, FUNC(gcm394_base_video_device::spriteram_r), FUNC(gcm394_base_video_device::spriteram_w));

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

																				 // wrlshunt                                                               | smartfp
	map(0x007820, 0x007820).w(FUNC(sunplus_gcm394_base_device::unkarea_7820_w)); // 7f8a (7f8a before DMA from ROM to RAM, 008a after DMA from ROM to RAM) | 3f04
	map(0x007821, 0x007821).w(FUNC(sunplus_gcm394_base_device::unkarea_7821_w)); // 7f47                                                                   | 0044
	map(0x007822, 0x007822).w(FUNC(sunplus_gcm394_base_device::unkarea_7822_w)); // 0047                                                                   | 1f44
	map(0x007823, 0x007823).w(FUNC(sunplus_gcm394_base_device::unkarea_7823_w)); // 0047                                                                   | 0044
	map(0x007824, 0x007824).w(FUNC(sunplus_gcm394_base_device::unkarea_7824_w)); // 0047                                                                   | 0044

	map(0x00782d, 0x00782d).rw(FUNC(sunplus_gcm394_base_device::unkarea_782d_r), FUNC(sunplus_gcm394_base_device::unkarea_782d_w)); // on startup

	map(0x007835, 0x007835).w(FUNC(sunplus_gcm394_base_device::unkarea_7835_w));

	map(0x007860, 0x007860).rw(FUNC(sunplus_gcm394_base_device::ioarea_7860_porta_r), FUNC(sunplus_gcm394_base_device::ioarea_7860_porta_w));

	map(0x007861, 0x007861).r(FUNC(sunplus_gcm394_base_device::unkarea_7861_r));
	map(0x007862, 0x007862).rw(FUNC(sunplus_gcm394_base_device::unkarea_7862_r), FUNC(sunplus_gcm394_base_device::unkarea_7862_w));
	map(0x007863, 0x007863).rw(FUNC(sunplus_gcm394_base_device::unkarea_7863_r), FUNC(sunplus_gcm394_base_device::unkarea_7863_w));

	map(0x007868, 0x007868).r(FUNC(sunplus_gcm394_base_device::unkarea_7868_r)); // on startup

	map(0x007870, 0x007870).rw(FUNC(sunplus_gcm394_base_device::ioarea_7870_portb_r) ,FUNC(sunplus_gcm394_base_device::ioarea_7870_portb_w));

	map(0x007871, 0x007871).r(FUNC(sunplus_gcm394_base_device::unkarea_7871_r));
	map(0x007872, 0x007872).rw(FUNC(sunplus_gcm394_base_device::unkarea_7872_r), FUNC(sunplus_gcm394_base_device::unkarea_7872_w));
	map(0x007873, 0x007873).rw(FUNC(sunplus_gcm394_base_device::unkarea_7873_r), FUNC(sunplus_gcm394_base_device::unkarea_7873_w));

	map(0x007882, 0x007882).rw(FUNC(sunplus_gcm394_base_device::unkarea_7882_r), FUNC(sunplus_gcm394_base_device::unkarea_7882_w));
	map(0x007883, 0x007883).rw(FUNC(sunplus_gcm394_base_device::unkarea_7883_r), FUNC(sunplus_gcm394_base_device::unkarea_7883_w));

	map(0x0078a0, 0x0078a0).rw(FUNC(sunplus_gcm394_base_device::unkarea_78a0_r), FUNC(sunplus_gcm394_base_device::unkarea_78a0_w));

	map(0x0078a1, 0x0078a1).r(FUNC(sunplus_gcm394_base_device::unkarea_78a1_r));

	map(0x0078a4, 0x0078a4).w(FUNC(sunplus_gcm394_base_device::unkarea_78a4_w));
	map(0x0078a5, 0x0078a5).w(FUNC(sunplus_gcm394_base_device::unkarea_78a5_w));
	map(0x0078a6, 0x0078a6).w(FUNC(sunplus_gcm394_base_device::unkarea_78a6_w));

	map(0x0078a8, 0x0078a8).w(FUNC(sunplus_gcm394_base_device::unkarea_78a8_w));

	map(0x0078b0, 0x0078b0).w(FUNC(sunplus_gcm394_base_device::unkarea_78b0_w));
	map(0x0078b1, 0x0078b1).w(FUNC(sunplus_gcm394_base_device::unkarea_78b1_w));

	map(0x0078b2, 0x0078b2).r(FUNC(sunplus_gcm394_base_device::unkarea_78b2_r));
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

READ16_MEMBER(generalplus_gpac800_device::unkarea_7850_r)
{
	return machine().rand();
}

READ16_MEMBER(generalplus_gpac800_device::unkarea_7854_r)
{
	m_testval ^= 1;

	logerror("%s:sunplus_gcm394_base_device::unkarea_7854_r\n", machine().describe_context());

	// jak_tsm code looks for various 'magic values'

	if (m_testval == 1)
		return 0x98;
	else
		return 0x79;

}

// 7998

WRITE16_MEMBER(generalplus_gpac800_device::flash_addr_low_w)
{
	//logerror("%s:sunplus_gcm394_base_device::flash_addr_low_w %04x\n", machine().describe_context(), data);
	m_flash_addr_low = data;
}

WRITE16_MEMBER(generalplus_gpac800_device::flash_addr_high_w)
{
	//logerror("%s:sunplus_gcm394_base_device::flash_addr_high_w %04x\n", machine().describe_context(), data);
	m_flash_addr_high = data;

	uint32_t address = (m_flash_addr_high << 16) | m_flash_addr_low;

	logerror("%s: flash address is now %08x\n", machine().describe_context(), address);
}


// all tilemap registers etc. appear to be in the same place as the above system, including the 'extra' ones not on the earlier models
// so it's likely this is built on top of that just with NAND support
void generalplus_gpac800_device::gpac800_internal_map(address_map& map)
{
	sunplus_gcm394_base_device::gcm394_internal_map(map);

	// there is an extra command-based device at 785x, what it returns is important to code flow
	// code is littered with NOPs so clearly the device can't accept commands too quickly and doesn't return data immediately
	//
	// this should be the NAND device, as the games attempt to do a DMA operation with '7854' as the source, and the target
	// as the RAM location where code needs to end up before jumping to it
	map(0x007850, 0x007850).r(FUNC(generalplus_gpac800_device::unkarea_7850_r)); // 'device ready' status flag?

	map(0x007852, 0x007852).w(FUNC(generalplus_gpac800_device::flash_addr_low_w));
	map(0x007853, 0x007853).w(FUNC(generalplus_gpac800_device::flash_addr_high_w));


	map(0x007854, 0x007854).r(FUNC(generalplus_gpac800_device::unkarea_7854_r)); // data read port (timing appears to be important)
}

void sunplus_gcm394_base_device::device_start()
{
	unsp_20_device::device_start();

	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);

	m_porta_out.resolve();


	m_space_read_cb.resolve_safe(0);
	m_space_write_cb.resolve();
	m_mapping_write_cb.resolve();


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

void generalplus_gpac800_device::device_reset()
{
	sunplus_gcm394_base_device::device_reset();

	m_flash_addr_low = 0x0000;
	m_flash_addr_high = 0x0000;
}

IRQ_CALLBACK_MEMBER(sunplus_gcm394_base_device::irq_vector_cb)
{
	//logerror("irq_vector_cb %d\n", irqline);

	if (irqline == UNSP_IRQ6_LINE)
		set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);

	if (irqline == UNSP_IRQ4_LINE)
		set_state_unsynced(UNSP_IRQ4_LINE, CLEAR_LINE);

	return 0;
}


void sunplus_gcm394_base_device::checkirq6()
{
/*
    if (m_7935 & 0x0100)
        set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
    else
        set_state_unsynced(UNSP_IRQ6_LINE, CLEAR_LINE);
*/
}

/* the IRQ6 interrupt on Wrlshunt
   reads 78a1, checks bit 0400
   if it IS set, just increases value in RAM at 1a2e  (no ack)
   if it ISN'T set, then read/write 78b2 (ack something?) then increase value in RAM  at 1a2e
   (smartfp doesn't touch these addresses? but instead reads/writes 7935, alt ack?)

   wrlshunt also has an IRQ4
   it always reads/writes 78c0 before executing payload (ack?)
   payload is a lot of manipulation of values in RAM, no registers touched

   wrlshunt also has FIQ
   no ack mechanism, for sound timer maybe (as it appears to be on spg110)


   ----

   IRQ5 is video IRQ
   in both games writing 0x0001 to 7063 seems to be an ack mechanism
   wrlshunt also checks bit 0x0040 of 7863 and will ack that too with alt code paths

   7863 is therefore some kind of 'video irq source' ?

*/


void sunplus_gcm394_base_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case 0:
	{
		m_7935 |= 0x0100;
		set_state_unsynced(UNSP_IRQ6_LINE, ASSERT_LINE);
		//set_state_unsynced(UNSP_IRQ4_LINE, ASSERT_LINE);

	//  checkirq6();
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
//  uint16_t b = m_gfxregion[(offset * 2) & (m_gfxregionsize - 1)] | (m_gfxregion[(offset * 2 + 1) & (m_gfxregionsize - 1)] << 8);
//  return b;
	address_space& mem = this->space(AS_PROGRAM);
	uint16_t retdata = mem.read_word(offset);
	return retdata;
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


