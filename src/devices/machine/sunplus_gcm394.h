// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#ifndef MAME_MACHINE_SUNPLUS_GCM394_H
#define MAME_MACHINE_SUNPLUS_GCM394_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "screen.h"

class sunplus_gcm394_base_device : public device_t, public device_mixer_interface
{
public:
	sunplus_gcm394_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	{
	}

	void map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(vblank) { /*m_spg_video->vblank(state);*/ }
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect) { return 0; /* m_spg_video->screen_update(screen, bitmap, cliprect);*/ }

protected:

	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	
	// video 70xx
	uint16_t tmap0_regs[0x6];
	uint16_t tmap1_regs[0x6];

	uint16_t m_dma_params[7];
	uint16_t m_707f;
	uint16_t m_703a;
	uint16_t m_7062;
	uint16_t m_7063;

	uint16_t m_702a;
	uint16_t m_7030;
	uint16_t m_703c;


	uint16_t m_7080;
	uint16_t m_7081;
	uint16_t m_7082;
	uint16_t m_7083;
	uint16_t m_7084;
	uint16_t m_7085;
	uint16_t m_7086;
	uint16_t m_7087;
	uint16_t m_7088;

	// unk 78xx
	uint16_t m_7803;

	uint16_t m_7807;

	uint16_t m_7810;

	uint16_t m_7816;
	uint16_t m_7817;


	uint16_t m_7819;

	uint16_t m_7820;
	uint16_t m_7821;
	uint16_t m_7822;
	uint16_t m_7823;
	uint16_t m_7824;

	uint16_t m_782d;

	uint16_t m_7835;

	uint16_t m_7860;

	uint16_t m_7861;

	uint16_t m_7862;
	uint16_t m_7863;

	uint16_t m_7870;

	uint16_t m_7871;

	uint16_t m_7872;
	uint16_t m_7873;

	uint16_t m_7882;
	uint16_t m_7883;

	uint16_t m_78a0;

	uint16_t m_78a4;
	uint16_t m_78a5;
	uint16_t m_78a6;

	uint16_t m_78a8;

	uint16_t m_78b0;
	uint16_t m_78b1;
	uint16_t m_78b2;

	uint16_t m_78b8;

	uint16_t m_78f0;

	uint16_t m_78fb;

	// unk 79xx
	uint16_t m_7934;
	uint16_t m_7936;


private:
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE16_MEMBER(unk_w);

	DECLARE_READ16_MEMBER(tmap0_regs_r);
	DECLARE_WRITE16_MEMBER(tmap0_regs_w);
	DECLARE_WRITE16_MEMBER(tmap0_unk0_w);
	DECLARE_WRITE16_MEMBER(tmap0_unk1_w);

	DECLARE_READ16_MEMBER(tmap1_regs_r);
	DECLARE_WRITE16_MEMBER(tmap1_regs_w);
	DECLARE_WRITE16_MEMBER(tmap1_unk0_w);
	DECLARE_WRITE16_MEMBER(tmap1_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device0_regs_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device0_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device0_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device1_regs_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device1_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device1_unk1_w);

	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk0_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk1_w);
	DECLARE_WRITE16_MEMBER(unknown_video_device2_unk2_w);

	DECLARE_WRITE16_MEMBER(video_dma_source_w);
	DECLARE_WRITE16_MEMBER(video_dma_dest_w);
	DECLARE_READ16_MEMBER(video_dma_size_r);
	DECLARE_WRITE16_MEMBER(video_dma_size_w);
	DECLARE_WRITE16_MEMBER(video_dma_unk_w);

	DECLARE_READ16_MEMBER(video_703a_r);
	DECLARE_WRITE16_MEMBER(video_703a_w);

	DECLARE_READ16_MEMBER(video_7062_r);
	DECLARE_WRITE16_MEMBER(video_7062_w);

	DECLARE_WRITE16_MEMBER(video_7063_w);

	DECLARE_WRITE16_MEMBER(video_702a_w);
	DECLARE_WRITE16_MEMBER(video_7030_w);
	DECLARE_WRITE16_MEMBER(video_703c_w);

	DECLARE_READ16_MEMBER(video_707f_r);
	DECLARE_WRITE16_MEMBER(video_707f_w);

	DECLARE_WRITE16_MEMBER(video_7080_w);
	DECLARE_WRITE16_MEMBER(video_7081_w);
	DECLARE_WRITE16_MEMBER(video_7082_w);
	DECLARE_WRITE16_MEMBER(video_7083_w);
	DECLARE_WRITE16_MEMBER(video_7084_w);
	DECLARE_WRITE16_MEMBER(video_7085_w);
	DECLARE_WRITE16_MEMBER(video_7086_w);
	DECLARE_WRITE16_MEMBER(video_7087_w);
	DECLARE_WRITE16_MEMBER(video_7088_w);

	DECLARE_READ16_MEMBER(video_7083_r);

	DECLARE_WRITE16_MEMBER(system_dma_params_w);
	DECLARE_WRITE16_MEMBER(system_dma_trigger_w);
	DECLARE_READ16_MEMBER(system_dma_status_r);

	DECLARE_READ16_MEMBER(unkarea_780f_status_r);
	DECLARE_READ16_MEMBER(unkarea_78fb_status_r);

	DECLARE_READ16_MEMBER(unkarea_7803_r);
	DECLARE_WRITE16_MEMBER(unkarea_7803_w);

	DECLARE_WRITE16_MEMBER(unkarea_7807_w);

	DECLARE_READ16_MEMBER(unkarea_7810_r);
	DECLARE_WRITE16_MEMBER(unkarea_7810_w);

	DECLARE_WRITE16_MEMBER(unkarea_7816_w);
	DECLARE_WRITE16_MEMBER(unkarea_7817_w);

	DECLARE_READ16_MEMBER(unkarea_7819_r);
	DECLARE_WRITE16_MEMBER(unkarea_7819_w);

	DECLARE_WRITE16_MEMBER(unkarea_7820_w);
	DECLARE_WRITE16_MEMBER(unkarea_7821_w);
	DECLARE_WRITE16_MEMBER(unkarea_7822_w);
	DECLARE_WRITE16_MEMBER(unkarea_7823_w);
	DECLARE_WRITE16_MEMBER(unkarea_7824_w);

	DECLARE_WRITE16_MEMBER(unkarea_7835_w);

	DECLARE_READ16_MEMBER(unkarea_7868_r);

	DECLARE_READ16_MEMBER(unkarea_782d_r);
	DECLARE_WRITE16_MEMBER(unkarea_782d_w);

	DECLARE_READ16_MEMBER(unkarea_7860_r);
	DECLARE_WRITE16_MEMBER(unkarea_7860_w);

	DECLARE_READ16_MEMBER(unkarea_7861_r);

	DECLARE_READ16_MEMBER(unkarea_7862_r);
	DECLARE_WRITE16_MEMBER(unkarea_7862_w);
	DECLARE_READ16_MEMBER(unkarea_7863_r);
	DECLARE_WRITE16_MEMBER(unkarea_7863_w);

	DECLARE_READ16_MEMBER(unkarea_7870_r);
	DECLARE_WRITE16_MEMBER(unkarea_7870_w);

	DECLARE_READ16_MEMBER(unkarea_7871_r);

	DECLARE_READ16_MEMBER(unkarea_7872_r);
	DECLARE_WRITE16_MEMBER(unkarea_7872_w);
	DECLARE_READ16_MEMBER(unkarea_7873_r);
	DECLARE_WRITE16_MEMBER(unkarea_7873_w);

	DECLARE_READ16_MEMBER(unkarea_7882_r);
	DECLARE_WRITE16_MEMBER(unkarea_7882_w);
	DECLARE_READ16_MEMBER(unkarea_7883_r);
	DECLARE_WRITE16_MEMBER(unkarea_7883_w);

	DECLARE_WRITE16_MEMBER(unkarea_78a0_w);

	DECLARE_WRITE16_MEMBER(unkarea_78a4_w);
	DECLARE_WRITE16_MEMBER(unkarea_78a5_w);
	DECLARE_WRITE16_MEMBER(unkarea_78a6_w);

	DECLARE_WRITE16_MEMBER(unkarea_78a8_w);

	DECLARE_WRITE16_MEMBER(unkarea_78b0_w);
	DECLARE_WRITE16_MEMBER(unkarea_78b1_w);
	DECLARE_WRITE16_MEMBER(unkarea_78b2_w);

	DECLARE_WRITE16_MEMBER(unkarea_78b8_w);

	DECLARE_WRITE16_MEMBER(unkarea_78f0_w);

	DECLARE_READ16_MEMBER(unkarea_7934_r);
	DECLARE_WRITE16_MEMBER(unkarea_7934_w);
	DECLARE_READ16_MEMBER(unkarea_7936_r);
	DECLARE_WRITE16_MEMBER(unkarea_7936_w);



};

class sunplus_gcm394_device : public sunplus_gcm394_base_device
{
public:
	template <typename T, typename U>
	sunplus_gcm394_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, T&& cpu_tag, U&& screen_tag)
		: sunplus_gcm394_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	sunplus_gcm394_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(GCM394, sunplus_gcm394_device)

#endif // MAME_MACHINE_SUNPLUS_GCM394_H
