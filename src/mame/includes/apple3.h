// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
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
#include "machine/mos6551.h"
#include "machine/6522via.h"
#include "machine/kb3600.h"
#include "machine/mm58167.h"
#include "sound/speaker.h"
#include "sound/dac.h"
#include "machine/wozfdc.h"
#include "imagedev/floppy.h"
#include "formats/flopimg.h"

#define VAR_VM0         0x0001
#define VAR_VM1         0x0002
#define VAR_VM2         0x0004
#define VAR_VM3         0x0008
#define VAR_EXTA0       0x0010
#define VAR_EXTA1       0x0020
#define VAR_EXTPOWER    0x0040
#define VAR_EXTSIDE     0x0080

#define SPEAKER_TAG "a3spkr"
#define DAC_TAG     "a3dac"

class apple3_state : public driver_device
{
public:
	apple3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_via_0(*this, "via6522_0"),
		m_via_1(*this, "via6522_1"),
		m_acia(*this, "acia"),
		m_fdc(*this, "fdc"),
		m_ay3600(*this, "ay3600"),
		m_a2bus(*this, "a2bus"),
		m_rtc(*this, "rtc"),
		m_speaker(*this, SPEAKER_TAG),
		m_dac(*this, DAC_TAG),
		m_kbspecial(*this, "keyb_special"),
		m_palette(*this, "palette"),
		m_joy1x(*this, "joy_1_x"),
		m_joy1y(*this, "joy_1_y"),
		m_joy2x(*this, "joy_2_x"),
		m_joy2y(*this, "joy_2_y"),
		m_joybuttons(*this, "joy_buttons"),
		m_pdltimer(*this, "pdltimer"),
		floppy0(*this, "0"),
		floppy1(*this, "1"),
		floppy2(*this, "2"),
		floppy3(*this, "3")
	{
	}

	required_device<m6502_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_device<mos6551_device> m_acia;
	required_device<appleiii_fdc> m_fdc;
	required_device<ay3600_device> m_ay3600;
	required_device<a2bus_device> m_a2bus;
	required_device<mm58167_device> m_rtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<dac_device> m_dac;
	required_ioport m_kbspecial;
	required_device<palette_device> m_palette;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	required_device<timer_device> m_pdltimer;
	required_device<floppy_connector> floppy0;
	required_device<floppy_connector> floppy1;
	required_device<floppy_connector> floppy2;
	required_device<floppy_connector> floppy3;

	DECLARE_READ8_MEMBER(apple3_memory_r);
	DECLARE_WRITE8_MEMBER(apple3_memory_w);
	DECLARE_WRITE_LINE_MEMBER(apple3_sync_w);
	DECLARE_READ8_MEMBER(apple3_c0xx_r);
	DECLARE_WRITE8_MEMBER(apple3_c0xx_w);
	DECLARE_DRIVER_INIT(apple3);
	DECLARE_MACHINE_RESET(apple3);
	DECLARE_VIDEO_START(apple3);
	UINT32 screen_update_apple3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(apple3_interrupt);
	TIMER_CALLBACK_MEMBER(scanstart_cb);
	TIMER_CALLBACK_MEMBER(scanend_cb);
	DECLARE_WRITE_LINE_MEMBER(apple3_acia_irq_func);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_0_out_b);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_a);
	DECLARE_WRITE8_MEMBER(apple3_via_1_out_b);
	DECLARE_WRITE_LINE_MEMBER(apple3_via_0_irq_func);
	DECLARE_WRITE_LINE_MEMBER(apple3_via_1_irq_func);
	void apple3_write_charmem();
	void text40(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void text80(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_hgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_chgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_shgr(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void graphics_chires(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT8 *apple3_bankaddr(UINT16 bank, offs_t offset);
	UINT8 *apple3_get_zpa_addr(offs_t offset);
	void apple3_update_memory();
	void apple3_via_out(UINT8 *var, UINT8 data);
	UINT8 *apple3_get_indexed_addr(offs_t offset);
	TIMER_DEVICE_CALLBACK_MEMBER(apple3_c040_tick);
	DECLARE_PALETTE_INIT(apple3);
	void apple3_irq_update();
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	void apple3_postload();
	TIMER_DEVICE_CALLBACK_MEMBER(paddle_timer);
	void pdl_handler(int offset);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);

	// these need to be public for now
	UINT32 m_flags;
	int m_enable_mask;

private:
	int m_acia_irq;
	UINT8 m_via_0_a;
	UINT8 m_via_0_b;
	UINT8 m_via_1_a;
	UINT8 m_via_1_b;
	int m_via_0_irq;
	int m_via_1_irq;
	offs_t m_zpa;
	UINT8 m_last_n;
	UINT8 m_char_mem[0x800];
	std::unique_ptr<UINT32[]> m_hgr_map;

	bool m_sync;
	bool m_rom_has_been_disabled;
	int m_cnxx_slot;
	UINT8 m_indir_bank;

	UINT8 *m_bank2, *m_bank3, *m_bank4, *m_bank5, *m_bank8, *m_bank9;
	UINT8 *m_bank10, *m_bank11;
	UINT8 *m_bank6, *m_bank7rd, *m_bank7wr;
	int m_speaker_state;
	int m_c040_time;
	UINT16 m_lastchar, m_strobe;
	UINT8 m_transchar;

	emu_timer *m_scanstart, *m_scanend;

	int m_analog_sel;
	bool m_ramp_active;
	int m_pdl_charge;
	int m_va, m_vb, m_vc;
	int m_smoothscr;
};

#endif /* APPLE3_H_ */
