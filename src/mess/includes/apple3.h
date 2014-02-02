/*****************************************************************************
 *
 * includes/apple3.h
 *
 * Apple ///
 *
 ****************************************************************************/

#ifndef APPLE3_H_
#define APPLE3_H_

#include "includes/apple2.h"
#include "machine/applefdc.h"
#include "machine/6522via.h"

#define VAR_VM0         0x0001
#define VAR_VM1         0x0002
#define VAR_VM2         0x0004
#define VAR_VM3         0x0008
#define VAR_EXTA0       0x0010
#define VAR_EXTA1       0x0020
#define VAR_EXTPOWER    0x0040
#define VAR_EXTSIDE     0x0080


class apple3_state : public apple2_state
{
public:
	apple3_state(const machine_config &mconfig, device_type type, const char *tag)
		: apple2_state(mconfig, type, tag),
		m_via_0(*this, "via6522_0"),
		m_via_1(*this, "via6522_1")
	{
	}

	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;

	UINT32 m_flags;
	UINT8 m_via_0_a;
	UINT8 m_via_0_b;
	UINT8 m_via_1_a;
	UINT8 m_via_1_b;
	int m_via_1_irq;
	int m_enable_mask;
	offs_t m_zpa;
	int m_profile_lastaddr;
	UINT8 m_profile_gotstrobe;
	UINT8 m_profile_readdata;
	UINT8 m_profile_busycount;
	UINT8 m_profile_busy;
	UINT8 m_profile_online;
	UINT8 m_profile_writedata;
	UINT8 m_last_n;
	UINT8 *m_char_mem;
	UINT32 *m_hgr_map;
	DECLARE_READ8_MEMBER(apple3_c0xx_r);
	DECLARE_WRITE8_MEMBER(apple3_c0xx_w);
	DECLARE_READ8_MEMBER(apple3_00xx_r);
	DECLARE_WRITE8_MEMBER(apple3_00xx_w);
	DECLARE_READ8_MEMBER(apple3_indexed_read);
	DECLARE_WRITE8_MEMBER(apple3_indexed_write);
	UINT8 apple3_profile_r(offs_t offset);
	void apple3_profile_w(offs_t offset, UINT8 data);
	DECLARE_DIRECT_UPDATE_MEMBER(apple3_opbase);
	DECLARE_DRIVER_INIT(apple3);
	DECLARE_MACHINE_RESET(apple3);
	DECLARE_VIDEO_START(apple3);
	UINT32 screen_update_apple3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(apple3_interrupt);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_b);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_b);
	DECLARE_WRITE_LINE_MEMBER(apple2_via_1_irq_func);
	void apple3_write_charmem();
	void apple3_video_text40(bitmap_ind16 &bitmap);
	void apple3_video_text80(bitmap_ind16 &bitmap);
	void apple3_video_graphics_hgr(bitmap_ind16 &bitmap);
	UINT8 swap_bits(UINT8 b);
	void apple3_video_graphics_chgr(bitmap_ind16 &bitmap);
	void apple3_video_graphics_shgr(bitmap_ind16 &bitmap);
	void apple3_video_graphics_chires(bitmap_ind16 &bitmap);
	void apple3_profile_init(void);
	void apple3_profile_statemachine(void);
	UINT8 *apple3_bankaddr(UINT16 bank, offs_t offset);
	void apple3_setbank(const char *mame_bank, UINT16 bank, offs_t offset);
	UINT8 *apple3_get_zpa_addr(offs_t offset);
	void apple3_update_memory();
	void apple3_via_out(UINT8 *var, UINT8 data);
	UINT8 *apple3_get_indexed_addr(offs_t offset);
};


/*----------- defined in machine/apple3.c -----------*/

extern const applefdc_interface apple3_fdc_interface;

#endif /* APPLE3_H_ */
