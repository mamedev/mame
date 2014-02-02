/*****************************************************************************
 *
 * includes/apple3.h
 *
 * Apple ///
 *
 ****************************************************************************/

#ifndef APPLE3_H_
#define APPLE3_H_

#include "cpu/m6502/m6502.h"
#include "includes/apple2.h"
#include "machine/ram.h"
#include "bus/a2bus/a2bus.h"
#include "machine/applefdc.h"
#include "machine/mos6551.h"
#include "machine/6522via.h"
#include "sound/speaker.h"
#include "sound/dac.h"

#define VAR_VM0         0x0001
#define VAR_VM1         0x0002
#define VAR_VM2         0x0004
#define VAR_VM3         0x0008
#define VAR_EXTA0       0x0010
#define VAR_EXTA1       0x0020
#define VAR_EXTPOWER    0x0040
#define VAR_EXTSIDE     0x0080

#define SPEAKER_TAG	"a3spkr"
#define DAC_TAG		"a3dac"

class apple3_state : public apple2_state
{
public:
	apple3_state(const machine_config &mconfig, device_type type, const char *tag)
		: apple2_state(mconfig, type, tag),
		m_via_0(*this, "via6522_0"),
		m_via_1(*this, "via6522_1"),
		m_speaker(*this, SPEAKER_TAG),
		m_dac(*this, DAC_TAG)
	{
	}

	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_device<speaker_sound_device> m_speaker;
	required_device<dac_device> m_dac;

	UINT32 m_flags;
	UINT8 m_via_0_a;
	UINT8 m_via_0_b;
	UINT8 m_via_1_a;
	UINT8 m_via_1_b;
	int m_via_0_irq;
	int m_via_1_irq;
	int m_enable_mask;
	offs_t m_zpa;
	UINT8 m_last_n;
	UINT8 *m_char_mem;
	UINT32 *m_hgr_map;
	DECLARE_READ8_MEMBER(apple3_memory_r);
	DECLARE_WRITE8_MEMBER(apple3_memory_w);
	DECLARE_WRITE_LINE_MEMBER(apple3_sync_w);
	DECLARE_READ8_MEMBER(apple3_c0xx_r);
	DECLARE_WRITE8_MEMBER(apple3_c0xx_w);
	DECLARE_READ8_MEMBER(apple3_00xx_r);
	DECLARE_WRITE8_MEMBER(apple3_00xx_w);
	DECLARE_READ8_MEMBER(apple3_indexed_read);
	DECLARE_WRITE8_MEMBER(apple3_indexed_write);
	DECLARE_DRIVER_INIT(apple3);
	DECLARE_MACHINE_RESET(apple3);
	DECLARE_VIDEO_START(apple3);
	UINT32 screen_update_apple3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(apple3_interrupt);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_b);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_b);
	DECLARE_WRITE_LINE_MEMBER(apple3_via_0_irq_func);
	DECLARE_WRITE_LINE_MEMBER(apple3_via_1_irq_func);
	void apple3_write_charmem();
	void apple3_video_text40(bitmap_ind16 &bitmap);
	void apple3_video_text80(bitmap_ind16 &bitmap);
	void apple3_video_graphics_hgr(bitmap_ind16 &bitmap);
	UINT8 swap_bits(UINT8 b);
	void apple3_video_graphics_chgr(bitmap_ind16 &bitmap);
	void apple3_video_graphics_shgr(bitmap_ind16 &bitmap);
	void apple3_video_graphics_chires(bitmap_ind16 &bitmap);
	UINT8 *apple3_bankaddr(UINT16 bank, offs_t offset);
	UINT8 *apple3_get_zpa_addr(offs_t offset);
	void apple3_update_memory();
	void apple3_via_out(UINT8 *var, UINT8 data);
	UINT8 *apple3_get_indexed_addr(offs_t offset);
	TIMER_DEVICE_CALLBACK_MEMBER(apple3_c040_tick);
	DECLARE_PALETTE_INIT(apple3);

	bool m_sync;
	UINT8 m_indir_opcode;
	int m_indir_count;

	UINT8 *m_bank2, *m_bank3, *m_bank4, *m_bank5, *m_bank8, *m_bank9;
	UINT8 *m_bank10, *m_bank11;
	UINT8 *m_bank6, *m_bank7;
	int m_speaker_state;
	int m_c040_time;
};


/*----------- defined in machine/apple3.c -----------*/

extern const applefdc_interface apple3_fdc_interface;

#endif /* APPLE3_H_ */
