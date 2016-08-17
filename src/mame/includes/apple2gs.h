// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/*****************************************************************************
 *
 * includes/apple2gs.h
 *
 * Apple IIgs
 *
 ****************************************************************************/

#ifndef APPLE2GS_H_
#define APPLE2GS_H_

#define RUN_ADB_MICRO (0)

#include "includes/apple2.h"
#include "sound/es5503.h"
#include "machine/nvram.h"
#include "cpu/g65816/g65816.h"
#include "cpu/m6502/m5074x.h"

#define ADBMICRO_TAG    "adbmicro"

// IIgs clocks as marked on the schematics
#define APPLE2GS_28M  (XTAL_28_63636MHz) // IIGS master clock
#define APPLE2GS_14M  (APPLE2GS_28M/2)
#define APPLE2GS_7M   (APPLE2GS_28M/4)

// screen dimensions
#define BORDER_LEFT (32)
#define BORDER_RIGHT    (32)
#define BORDER_TOP  (16)    // (plus bottom)

// these are numbered as seen from the MCU
enum glu_reg_names
{
	GLU_KEY_DATA = 0,   // MCU W
	GLU_COMMAND,        // MCU R
	GLU_MOUSEX,         // MCU W
	GLU_MOUSEY,         // MCU W
	GLU_KG_STATUS,      // MCU R
	GLU_ANY_KEY_DOWN,   // MCU W
	GLU_KEYMOD,         // MCU W
	GLU_DATA,           // MCU W

	GLU_C000,       // 816 R
	GLU_C010,       // 816 RW
	GLU_SYSSTAT     // 816 R/(limited) W
};

enum glu_kg_status
{
	KGS_ANY_KEY_DOWN = 0x01,
	KGS_KEYSTROBE    = 0x10,
	KGS_DATA_FULL    = 0x20,
	KGS_COMMAND_FULL = 0x40,
	KGS_MOUSEX_FULL  = 0x80
};

enum apple2gs_clock_mode
{
	CLOCKMODE_IDLE,
	CLOCKMODE_TIME,
	CLOCKMODE_INTERNALREGS,
	CLOCKMODE_BRAM1,
	CLOCKMODE_BRAM2
};


enum adbstate_t
{
	ADBSTATE_IDLE,
	ADBSTATE_INCOMMAND,
	ADBSTATE_INRESPONSE
};

#define IRQ_KBD_SRQ         0x01
#define IRQ_ADB_DATA        0x02
#define IRQ_ADB_MOUSE       0x04
#define IRQ_VGC_SCANLINE    0x08
#define IRQ_VGC_SECOND      0x10
#define IRQ_INTEN_QSECOND   0x20
#define IRQ_INTEN_VBL       0x40
#define IRQ_DOC             0x80
#define IRQ_SLOT            0x100

void apple2gs_add_irq(running_machine &machine, UINT16 irq_mask);
void apple2gs_remove_irq(running_machine &machine, UINT16 irq_mask);

class apple2gs_state : public apple2_state
{
public:
	apple2gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: apple2_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_es5503(*this, "es5503"),
		m_fdc(*this, "fdc"),
		#if RUN_ADB_MICRO
		m_adbmicro(*this, ADBMICRO_TAG),
		#endif
		m_adb_mousex(*this, "adb_mouse_x"),
		m_adb_mousey(*this, "adb_mouse_y"),
		m_palette(*this, "palette")
		{ }

	required_device<g65816_device> m_maincpu;
	required_device<es5503_device> m_es5503;
	required_device<applefdc_base_device> m_fdc;
	#if RUN_ADB_MICRO
	optional_device<m5074x_device> m_adbmicro;
	#endif

	required_ioport  m_adb_mousex, m_adb_mousey;
	required_device<palette_device> m_palette;

	std::unique_ptr<UINT8[]> m_slowmem;
	UINT8 m_newvideo;
	UINT16 m_bordercolor;
	UINT8 m_vgcint;
	UINT8 m_langsel;
	UINT8 m_sltromsel;
	UINT8 m_cyareg;
	UINT8 m_inten;
	UINT8 m_intflag;
	UINT8 m_shadow;
	UINT16 m_pending_irqs;
	UINT8 m_mouse_x;
	UINT8 m_mouse_y;
	INT8 m_mouse_dx;
	INT8 m_mouse_dy;
	device_t *m_cur_slot6_image;
	emu_timer *m_scanline_timer;
	emu_timer *m_clock_timer;
	emu_timer *m_qsecond_timer;
	UINT8 m_clock_data;
	UINT8 m_clock_control;
	UINT8 m_clock_read;
	UINT8 m_clock_reg1;
	apple2gs_clock_mode m_clock_mode;
	UINT32 m_clock_curtime;
	seconds_t m_clock_curtime_interval;
	UINT8 m_clock_bram[256];
	#if !RUN_ADB_MICRO
	adbstate_t m_adb_state;
	UINT8 m_adb_command;
	UINT8 m_adb_mode;
	UINT8 m_adb_kmstatus;
	UINT8 m_adb_latent_result;
	INT32 m_adb_command_length;
	INT32 m_adb_command_pos;
	UINT8 m_adb_command_bytes[8];
	UINT8 m_adb_response_bytes[8];
	UINT8 m_adb_response_length;
	INT32 m_adb_response_pos;
	UINT8 m_adb_memory[0x100];
	int m_adb_address_keyboard;
	int m_adb_address_mouse;
	#endif
	UINT8 m_sndglu_ctrl;
	int m_sndglu_addr;
	int m_sndglu_dummy_read;
	std::unique_ptr<bitmap_ind16> m_legacy_gfx;
	bool m_is_rom3;
	UINT8 m_echo_bank;
	UINT64 m_last_adb_time;
	int m_adb_dtime;
	UINT32 m_a2_palette[16];
	UINT32 m_shr_palette[256];

	READ8_MEMBER( apple2gs_c0xx_r );
	WRITE8_MEMBER( apple2gs_c0xx_w );
	WRITE8_MEMBER( apple2gs_main0400_w );
	WRITE8_MEMBER( apple2gs_aux0400_w );
	WRITE8_MEMBER( apple2gs_main2000_w );
	WRITE8_MEMBER( apple2gs_aux2000_w );
	WRITE8_MEMBER( apple2gs_main4000_w );
	WRITE8_MEMBER( apple2gs_aux4000_w );

	UINT8 adb_read_datareg();
	UINT8 adb_read_kmstatus();

	void apple2gs_refresh_delegates();

	write8_delegate write_delegates_2gs0400[2];
	write8_delegate write_delegates_2gs2000[2];
	write8_delegate write_delegates_2gs4000[2];
	DECLARE_MACHINE_START(apple2gs);
	DECLARE_MACHINE_RESET(apple2gs);
	DECLARE_VIDEO_START(apple2gs);
	DECLARE_PALETTE_INIT(apple2gs);
	DECLARE_MACHINE_START(apple2gsr1);
	DECLARE_MACHINE_START(apple2gscommon);
	UINT32 screen_update_apple2gs(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(apple2gs_clock_tick);
	TIMER_CALLBACK_MEMBER(apple2gs_qsecond_tick);
	TIMER_CALLBACK_MEMBER(apple2gs_scanline_tick);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_READ8_MEMBER(apple2gs_read_vector);

	// ADB MCU and ADB GLU stuff
	#if RUN_ADB_MICRO
	UINT8 m_glu_regs[8], m_glu_bus, m_glu_sysstat;
	bool m_glu_mcu_read_kgs, m_glu_816_read_dstat, m_glu_mouse_read_stat, m_adb_line;

	UINT8 keyglu_mcu_read(UINT8 offset);
	void keyglu_mcu_write(UINT8 offset, UINT8 data);
	UINT8 keyglu_816_read(UINT8 offset);
	void keyglu_816_write(UINT8 offset, UINT8 data);

	DECLARE_READ8_MEMBER(adbmicro_p0_in);
	DECLARE_READ8_MEMBER(adbmicro_p1_in);
	DECLARE_READ8_MEMBER(adbmicro_p2_in);
	DECLARE_READ8_MEMBER(adbmicro_p3_in);
	DECLARE_WRITE8_MEMBER(adbmicro_p0_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p1_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p2_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p3_out);
	#endif
	void process_clock();
	const char *apple2gs_irq_name(UINT16 irq_mask);
	void apple2gs_add_irq(UINT16 irq_mask);
	void apple2gs_remove_irq(UINT16 irq_mask);
	UINT8 adb_read_memory(UINT32 address);
	void adb_write_memory(UINT32 address, UINT8 data);
	void adb_set_mode(UINT8 mode);
	void adb_set_config(UINT8 b1, UINT8 b2, UINT8 b3);
	void adb_post_response(const UINT8 *bytes, size_t length);
	void adb_post_response_1(UINT8 b);
	void adb_post_response_2(UINT8 b1, UINT8 b2);
	void adb_do_command();
	void adb_write_datareg(UINT8 data);
	void adb_write_kmstatus(UINT8 data);
	UINT8 adb_read_mousedata();
	INT8 seven_bit_diff(UINT8 v1, UINT8 v2);
	void adb_check_mouse();
	void apple2gs_set_scanint(UINT8 data);
	int apple2gs_get_vpos();
	UINT8 *apple2gs_getslotmem(offs_t address);
	UINT8 apple2gs_xxCxxx_r(address_space &space, offs_t address);
	void apple2gs_xxCxxx_w(address_space &space, offs_t address, UINT8 data);
	void apple2gs_setup_memory();

	DECLARE_READ8_MEMBER( gssnd_r );
	DECLARE_WRITE8_MEMBER( gssnd_w );
	DECLARE_READ8_MEMBER( apple2gs_00Cxxx_r );
	DECLARE_READ8_MEMBER( apple2gs_01Cxxx_r );
	DECLARE_READ8_MEMBER( apple2gs_E0Cxxx_r );
	DECLARE_READ8_MEMBER( apple2gs_E1Cxxx_r );
	DECLARE_WRITE8_MEMBER( apple2gs_00Cxxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_01Cxxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E0Cxxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E1Cxxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_Exxxxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E004xx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E02xxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E104xx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_E12xxx_w );
	DECLARE_WRITE8_MEMBER( apple2gs_slowmem_w );
	DECLARE_READ8_MEMBER(apple2gs_bank_echo_r);
	DECLARE_WRITE_LINE_MEMBER( apple2gs_doc_irq);
	DECLARE_READ8_MEMBER(apple2gs_adc_read);

};

#endif /* APPLE2GS_H_ */
