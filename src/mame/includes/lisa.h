// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 * includes/lisa.h
 *
 * Lisa driver declarations
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_LISA_H
#define MAME_INCLUDES_LISA_H

#include "cpu/m6502/m6504.h"
#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/6522via.h"
#include "machine/6522via.h"
#include "machine/8530scc.h"
#include "machine/applefdc.h"
#include "machine/nvram.h"
#include "machine/sonydriv.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"

/* lisa MMU segment regs */
struct real_mmu_entry
{
	uint16_t sorg;
	uint16_t slim;
};

/* MMU regs translated into a more efficient format */
enum mmu_entry_t { RAM_stack_r, RAM_r, RAM_stack_rw, RAM_rw, IO, invalid, special_IO };

struct mmu_entry
{
	offs_t sorg;    /* (real_sorg & 0x0fff) << 9 */
	mmu_entry_t type;   /* <-> (real_slim & 0x0f00) */
	int slim;   /* (~ ((real_slim & 0x00ff) << 9)) & 0x01ffff */
};

enum floppy_hardware_t
{
	twiggy,         /* twiggy drives (Lisa 1) */
	sony_lisa2,     /* 3.5'' drive with LisaLite adapter (Lisa 2) */
	sony_lisa210    /* 3.5'' drive with modified fdc hardware (Lisa 2/10, Mac XL) */
};

enum clock_mode_t
{
	clock_timer_disable = 0,
	timer_disable = 1,
	timer_interrupt = 2,    /* timer underflow generates interrupt */
	timer_power_on = 3      /* timer underflow turns system on if it is off and gens interrupt */
};          /* clock mode */

/* clock registers */
struct clock_regs_t
{
	long alarm;     /* alarm (20-bit binary) */
	int years;      /* years (4-bit binary ) */
	int days1;      /* days (BCD : 1-366) */
	int days2;
	int days3;
	int hours1;     /* hours (BCD : 0-23) */
	int hours2;
	int minutes1;   /* minutes (BCD : 0-59) */
	int minutes2;
	int seconds1;   /* seconds (BCD : 0-59) */
	int seconds2;
	int tenths;     /* tenths of second (BCD : 0-9) */

	int clock_write_ptr;    /* clock byte to be written next (-1 if clock write disabled) */

	enum clock_mode_t clock_mode;
};

struct lisa_features_t
{
	unsigned int has_fast_timers : 1;   /* I/O board VIAs are clocked at 1.25 MHz (?) instead of .5 MHz (?) (Lisa 2/10, Mac XL) */
										/* Note that the beep routine in boot ROMs implies that
										VIA clock is 1.25 times faster with fast timers than with
										slow timers.  I read the schematics again and again, and
										I simply don't understand : in one case the VIA is
										connected to the 68k E clock, which is CPUCK/10, and in
										another case, to a generated PH2 clock which is CPUCK/4,
										with additional logic to keep it in phase with the 68k
										memory cycle.  After hearing the beep when MacWorks XL
										boots, I bet the correct values are .625 MHz and .5 MHz.
										Maybe the schematics are wrong, and PH2 is CPUCK/8.
										Maybe the board uses a 6522 variant with different
										timings. */
	floppy_hardware_t floppy_hardware;
	unsigned int has_double_sided_floppy : 1;   /* true on lisa 1 and *hacked* lisa 2/10 / Mac XL */
	unsigned int has_mac_xl_video : 1;  /* modified video for MacXL */
};


class lisa_state : public driver_device
{
public:
	lisa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via0(*this, "via6522_0"),
		m_via1(*this, "via6522_1"),
		m_fdc(*this, "fdc"),
		m_scc(*this, "scc"),
		m_speaker(*this, "speaker"),
		m_nvram(*this, "nvram"),
		m_latch(*this, "latch"),
		m_fdc_cpu(*this,"fdccpu"),
		m_fdc_rom(*this,"fdc_rom"),
		m_fdc_ram(*this,"fdc_ram"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_mouse_x(*this, "MOUSE_X"),
		m_io_mouse_y(*this, "MOUSE_Y"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void lisa(machine_config &config);
	void lisa210(machine_config &config);
	void macxl(machine_config &config);

	void init_lisa210();
	void init_mac_xl();
	void init_lisa2();

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	optional_device<applefdc_base_device> m_fdc;
	required_device<scc8530_legacy_device> m_scc;
	required_device<speaker_sound_device> m_speaker;
	required_device<nvram_device> m_nvram;
	required_device<ls259_device> m_latch;
	required_device<m6504_device> m_fdc_cpu;

	required_shared_ptr<uint8_t> m_fdc_rom;
	required_shared_ptr<uint8_t> m_fdc_ram;

	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	uint8_t *m_ram_ptr;
	uint8_t *m_rom_ptr;
	uint8_t *m_videoROM_ptr;
	int m_setup;
	int m_seg;
	real_mmu_entry m_real_mmu_regs[4][128];
	mmu_entry m_mmu_regs[4][128];
	int m_diag2;
	int m_test_parity;
	uint16_t m_mem_err_addr_latch;
	int m_parity_error_pending;
	int m_bad_parity_count;
	std::unique_ptr<uint8_t[]> m_bad_parity_table;
	int m_VTMSK;
	int m_VTIR;
	uint16_t m_video_address_latch;
	uint16_t *m_videoram_ptr;
	int m_KBIR;
	int m_FDIR;
	int m_DISK_DIAG;
	int m_MT1;
	int m_PWM_floppy_motor_speed;
	int m_model;
	lisa_features_t m_features;
	int m_COPS_Ready;
	int m_COPS_command;
	int m_fifo_data[8];
	int m_fifo_size;
	int m_fifo_head;
	int m_fifo_tail;
	int m_mouse_data_offset;
	int m_COPS_force_unplug;
	emu_timer *m_mouse_timer;
	emu_timer *m_cops_ready_timer;
	int m_hold_COPS_data;
	int m_NMIcode;
	clock_regs_t m_clock_regs;
	int m_key_matrix[8];
	int m_last_mx;
	int m_last_my;
	int m_frame_count;
	int m_videoROM_address;
	DECLARE_READ8_MEMBER(lisa_fdc_io_r);
	DECLARE_WRITE8_MEMBER(lisa_fdc_io_w);
	DECLARE_READ16_MEMBER(lisa_r);
	DECLARE_WRITE16_MEMBER(lisa_w);
	DECLARE_READ16_MEMBER(lisa_IO_r);
	DECLARE_WRITE16_MEMBER(lisa_IO_w);
	DECLARE_WRITE_LINE_MEMBER(diag1_w);
	DECLARE_WRITE_LINE_MEMBER(diag2_w);
	DECLARE_WRITE_LINE_MEMBER(seg1_w);
	DECLARE_WRITE_LINE_MEMBER(seg2_w);
	DECLARE_WRITE_LINE_MEMBER(setup_w);
	DECLARE_WRITE_LINE_MEMBER(vtmsk_w);
	DECLARE_WRITE_LINE_MEMBER(sfmsk_w);
	DECLARE_WRITE_LINE_MEMBER(hdmsk_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void nvram_init(nvram_device &nvram, void *data, size_t size);
	uint32_t screen_update_lisa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lisa_interrupt);
	TIMER_CALLBACK_MEMBER(handle_mouse);
	TIMER_CALLBACK_MEMBER(read_COPS_command);
	TIMER_CALLBACK_MEMBER(set_COPS_ready);
	DECLARE_WRITE8_MEMBER(COPS_via_out_a);
	DECLARE_WRITE_LINE_MEMBER(COPS_via_out_ca2);
	DECLARE_WRITE8_MEMBER(COPS_via_out_b);
	DECLARE_WRITE_LINE_MEMBER(COPS_via_out_cb2);

	void field_interrupts();
	void set_parity_error_pending(int value);
	void set_VTIR(int value);
	void cpu_board_control_access(offs_t offset);
	void init_COPS();
	void reset_COPS();
	void lisa_fdc_ttl_glue_access(offs_t offset);
	void COPS_send_data_if_possible();
	void COPS_queue_data(const uint8_t *data, int len);
	void COPS_via_irq_func(int val);
	void scan_keyboard();
	void unplug_keyboard();
	void plug_keyboard();
	void lisa210_fdc_map(address_map &map);
	void lisa_fdc_map(address_map &map);
	void lisa_map(address_map &map);
};

#endif // MAME_INCLUDES_LISA_H
