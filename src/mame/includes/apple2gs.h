// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/*****************************************************************************
 *
 * includes/apple2gs.h
 *
 * Apple IIgs
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_APPLE2GS_H
#define MAME_INCLUDES_APPLE2GS_H

#define RUN_ADB_MICRO (0)

#include "includes/apple2.h"
#include "sound/es5503.h"
#include "machine/nvram.h"
#include "cpu/g65816/g65816.h"
#include "cpu/m6502/m5074x.h"
#include "machine/z80scc.h"

#define ADBMICRO_TAG    "adbmicro"
#define SCC_TAG     "scc"
#define RS232A_TAG  "printer"
#define RS232B_TAG  "modem"

// IIgs clocks as marked on the schematics
#define APPLE2GS_28M  (XTAL(28'636'363)) // IIGS master clock
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

void apple2gs_add_irq(running_machine &machine, uint16_t irq_mask);
void apple2gs_remove_irq(running_machine &machine, uint16_t irq_mask);

class apple2gs_state : public apple2_state
{
public:
	apple2gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: apple2_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_es5503(*this, "es5503"),
		m_fdc(*this, "fdc"),
		m_scc(*this, "scc"),
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
	required_device<z80scc_device> m_scc;
	#if RUN_ADB_MICRO
	optional_device<m5074x_device> m_adbmicro;
	#endif

	required_ioport  m_adb_mousex, m_adb_mousey;
	required_device<palette_device> m_palette;

	std::unique_ptr<uint8_t[]> m_slowmem;
	uint8_t m_newvideo;
	uint16_t m_bordercolor;
	uint8_t m_vgcint;
	uint8_t m_langsel;
	uint8_t m_sltromsel;
	uint8_t m_cyareg;
	uint8_t m_inten;
	uint8_t m_intflag;
	uint8_t m_shadow;
	uint16_t m_pending_irqs;
	uint8_t m_mouse_x;
	uint8_t m_mouse_y;
	int8_t m_mouse_dx;
	int8_t m_mouse_dy;
	device_t *m_cur_slot6_image;
	emu_timer *m_scanline_timer;
	emu_timer *m_clock_timer;
	emu_timer *m_qsecond_timer;
	uint8_t m_clock_data;
	uint8_t m_clock_control;
	uint8_t m_clock_read;
	uint8_t m_clock_reg1;
	apple2gs_clock_mode m_clock_mode;
	uint32_t m_clock_curtime;
	seconds_t m_clock_curtime_interval;
	uint8_t m_clock_bram[256];
	#if !RUN_ADB_MICRO
	adbstate_t m_adb_state;
	uint8_t m_adb_command;
	uint8_t m_adb_mode;
	uint8_t m_adb_kmstatus;
	uint8_t m_adb_latent_result;
	int32_t m_adb_command_length;
	int32_t m_adb_command_pos;
	uint8_t m_adb_command_bytes[8];
	uint8_t m_adb_response_bytes[8];
	uint8_t m_adb_response_length;
	int32_t m_adb_response_pos;
	uint8_t m_adb_memory[0x100];
	int m_adb_address_keyboard;
	int m_adb_address_mouse;
	#endif
	uint8_t m_sndglu_ctrl;
	int m_sndglu_addr;
	int m_sndglu_dummy_read;
	std::unique_ptr<bitmap_ind16> m_legacy_gfx;
	bool m_is_rom3;
	uint8_t m_echo_bank;
	uint64_t m_last_adb_time;
	int m_adb_dtime;
	uint32_t m_a2_palette[16];
	uint32_t m_shr_palette[256];

	READ8_MEMBER( apple2gs_c0xx_r );
	WRITE8_MEMBER( apple2gs_c0xx_w );
	WRITE8_MEMBER( apple2gs_main0400_w );
	WRITE8_MEMBER( apple2gs_aux0400_w );
	WRITE8_MEMBER( apple2gs_main2000_w );
	WRITE8_MEMBER( apple2gs_aux2000_w );
	WRITE8_MEMBER( apple2gs_main4000_w );
	WRITE8_MEMBER( apple2gs_aux4000_w );

	uint8_t adb_read_datareg();
	uint8_t adb_read_kmstatus();

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
	uint32_t screen_update_apple2gs(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(apple2gs_clock_tick);
	TIMER_CALLBACK_MEMBER(apple2gs_qsecond_tick);
	TIMER_CALLBACK_MEMBER(apple2gs_scanline_tick);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_READ8_MEMBER(apple2gs_read_vector);

	// ADB MCU and ADB GLU stuff
	#if RUN_ADB_MICRO
	uint8_t m_glu_regs[8], m_glu_bus, m_glu_sysstat;
	bool m_glu_mcu_read_kgs, m_glu_816_read_dstat, m_glu_mouse_read_stat, m_adb_line;

	uint8_t keyglu_mcu_read(uint8_t offset);
	void keyglu_mcu_write(uint8_t offset, uint8_t data);
	uint8_t keyglu_816_read(uint8_t offset);
	void keyglu_816_write(uint8_t offset, uint8_t data);

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
	const char *apple2gs_irq_name(uint16_t irq_mask);
	void apple2gs_add_irq(uint16_t irq_mask);
	void apple2gs_remove_irq(uint16_t irq_mask);
	uint8_t adb_read_memory(uint32_t address);
	void adb_write_memory(uint32_t address, uint8_t data);
	void adb_set_mode(uint8_t mode);
	void adb_set_config(uint8_t b1, uint8_t b2, uint8_t b3);
	void adb_post_response(const uint8_t *bytes, size_t length);
	void adb_post_response_1(uint8_t b);
	void adb_post_response_2(uint8_t b1, uint8_t b2);
	void adb_do_command();
	void adb_write_datareg(uint8_t data);
	void adb_write_kmstatus(uint8_t data);
	uint8_t adb_read_mousedata();
	int8_t seven_bit_diff(uint8_t v1, uint8_t v2);
	void adb_check_mouse();
	void apple2gs_set_scanint(uint8_t data);
	int apple2gs_get_vpos();
	uint8_t *apple2gs_getslotmem(offs_t address);
	uint8_t apple2gs_xxCxxx_r(address_space &space, offs_t address);
	void apple2gs_xxCxxx_w(address_space &space, offs_t address, uint8_t data);
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

	void apple2gs(machine_config &config);
	void apple2gsr1(machine_config &config);
	void apple2gs_map(address_map &map);
	void vectors_map(address_map &map);
};

#endif // MAME_INCLUDES_APPLE2GS_H
