// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2gs.cpp - Apple IIgs

    Next generation driver written June 2018 by R. Belmont.
    Thanks to the original Apple IIgs driver's authors: Nathan Woods, and R. Belmont
    Thanks also to the Apple II Documentation Project/Antoine Vignau, Peter Ferrie, and Olivier Galibert.

    Unique hardware configurations:
    - ROM 00/01: original motherboard, 256K of RAM (banks 00/01/E0/E1 only), FPI chip manages fast/slow side
    - ROM 03: revised motherboard, 1M of RAM (banks 00/01/->0F/E0/E1), CYA chip replaces FPI
    - Expanded IIe: ROM 00/01 motherboard in a IIe case with a IIe keyboard rather than ADB

    FF6ACF is speed test in ROM
    Diags:
    A138 = scanline interrupt test (raster is too long to pass this)
    A179 = pass
    A17C = fail 1
    A0F1 = fail 2

***************************************************************************/

#define RUN_ADB_MICRO (0)
#define LOG_ADB (0)

#include "emu.h"
#include "video/apple2.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "cpu/g65816/g65816.h"
#include "cpu/m6502/m5074x.h"
#include "sound/spkrdev.h"
#include "sound/es5503.h"
#include "machine/bankdev.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/ram.h"
#include "machine/kb3600.h"
#include "machine/nvram.h"

#include "machine/applefdc.h"
#include "machine/sonydriv.h"
#include "machine/appldriv.h"
#include "imagedev/flopdrv.h"
#include "formats/ap2_dsk.h"
#include "formats/ap_dsk35.h"

#include "bus/rs232/rs232.h"

#include "emu.h"
#include "video/apple2.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/ramcard16k.h"
#include "bus/a2bus/a2diskiing.h"
#include "bus/a2bus/a2mockingboard.h"
#include "bus/a2bus/a2cffa.h"
#include "bus/a2bus/a2memexp.h"
#include "bus/a2bus/a2scsi.h"
#include "bus/a2bus/a2thunderclock.h"
#include "bus/a2bus/a2softcard.h"
#include "bus/a2bus/a2videoterm.h"
#include "bus/a2bus/a2ssc.h"
#include "bus/a2bus/a2swyft.h"
#include "bus/a2bus/a2themill.h"
#include "bus/a2bus/a2sam.h"
#include "bus/a2bus/a2alfam2.h"
#include "bus/a2bus/laser128.h"
#include "bus/a2bus/a2echoii.h"
#include "bus/a2bus/a2arcadebd.h"
#include "bus/a2bus/a2midi.h"
#include "bus/a2bus/a2zipdrive.h"
#include "bus/a2bus/a2applicard.h"
#include "bus/a2bus/a2ultraterm.h"
#include "bus/a2bus/a2pic.h"
#include "bus/a2bus/a2corvus.h"
#include "bus/a2bus/a2mcms.h"
#include "bus/a2bus/a2dx1.h"
#include "bus/a2bus/timemasterho.h"
#include "bus/a2bus/mouse.h"
#include "bus/a2bus/ezcgi.h"
//#include "bus/a2bus/pc_xporter.h"

// various timing standards
#define A2GS_MASTER_CLOCK (XTAL(28'636'363))
#define A2GS_14M    (A2GS_MASTER_CLOCK/2)
#define A2GS_7M     (A2GS_MASTER_CLOCK/4)
#define A2GS_1M     (A2GS_MASTER_CLOCK/28)

#define A2GS_CPU_TAG "maincpu"
#define A2GS_ADBMCU_TAG    "adbmicro"
#define A2GS_KBDC_TAG "ay3600"
#define A2GS_BUS_TAG "a2bus"
#define A2GS_SPEAKER_TAG "speaker"
#define A2GS_CASSETTE_TAG "tape"
#define A2GS_UPPERBANK_TAG "inhbank"
#define A2GS_AUXUPPER_TAG "inhaux"
#define A2GS_00UPPER_TAG "inh00"
#define A2GS_01UPPER_TAG "inh01"
#define A2GS_IWM_TAG    "fdc"   // must be "fdc" or sonydriv pukes
#define A2GS_DOC_TAG    "doc"
#define A2GS_VIDEO_TAG "a2video"
#define SCC_TAG    "scc"
#define RS232A_TAG "printer"
#define RS232B_TAG "modem"

#define A2GS_C100_TAG "c1bank"
#define A2GS_C300_TAG "c3bank"
#define A2GS_C400_TAG "c4bank"
#define A2GS_C800_TAG "c8bank"
#define A2GS_LCBANK_TAG "lcbank"
#define A2GS_LCAUX_TAG "lcaux"
#define A2GS_LC00_TAG "lc00"
#define A2GS_LC01_TAG "lc01"
#define A2GS_B0CXXX_TAG "bnk0atc"
#define A2GS_B01_TAG    "bnk1at0"
#define A2GS_B1CXXX_TAG "bnk1atc"
#define A2GS_B00000_TAG "b0r00bank"
#define A2GS_B00200_TAG "b0r02bank"
#define A2GS_B00400_TAG "b0r04bank"
#define A2GS_B00800_TAG "b0r08bank"
#define A2GS_B02000_TAG "b0r20bank"
#define A2GS_B04000_TAG "b0r40bank"

#define A2GS_KBD_Y0_TAG "Y0"
#define A2GS_KBD_Y1_TAG "Y1"
#define A2GS_KBD_Y2_TAG "Y2"
#define A2GS_KBD_Y3_TAG "Y3"
#define A2GS_KBD_Y4_TAG "Y4"
#define A2GS_KBD_Y5_TAG "Y5"
#define A2GS_KBD_Y6_TAG "Y6"
#define A2GS_KBD_Y7_TAG "Y7"
#define A2GS_KBD_Y8_TAG "Y8"
#define A2GS_KBD_Y9_TAG "Y9"
#define A2GS_KBD_SPEC_TAG "keyb_special"

#define CNXX_UNCLAIMED  -1
#define CNXX_INTROM     -2

class apple2gs_state : public driver_device
{
public:
	apple2gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, A2GS_CPU_TAG),
		m_screen(*this, "screen"),
		m_scantimer(*this, "scantimer"),
		m_adbmicro(*this, A2GS_ADBMCU_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, "maincpu"),
		m_docram(*this, "docram"),
		m_nvram(*this, "nvram"),
		m_video(*this, A2GS_VIDEO_TAG),
		m_a2bus(*this, A2GS_BUS_TAG),
		m_joy1x(*this, "joystick_1_x"),
		m_joy1y(*this, "joystick_1_y"),
		m_joy2x(*this, "joystick_2_x"),
		m_joy2y(*this, "joystick_2_y"),
		m_joybuttons(*this, "joystick_buttons"),
		m_speaker(*this, A2GS_SPEAKER_TAG),
		m_upperbank(*this, A2GS_UPPERBANK_TAG),
		m_upperaux(*this, A2GS_AUXUPPER_TAG),
		m_upper00(*this, A2GS_00UPPER_TAG),
		m_upper01(*this, A2GS_01UPPER_TAG),
		m_c100bank(*this, A2GS_C100_TAG),
		m_c300bank(*this, A2GS_C300_TAG),
		m_c400bank(*this, A2GS_C400_TAG),
		m_c800bank(*this, A2GS_C800_TAG),
		m_b0_0000bank(*this, A2GS_B00000_TAG),
		m_b0_0200bank(*this, A2GS_B00200_TAG),
		m_b0_0400bank(*this, A2GS_B00400_TAG),
		m_b0_0800bank(*this, A2GS_B00800_TAG),
		m_b0_2000bank(*this, A2GS_B02000_TAG),
		m_b0_4000bank(*this, A2GS_B04000_TAG),
		m_lcbank(*this, A2GS_LCBANK_TAG),
		m_lcaux(*this, A2GS_LCAUX_TAG),
		m_lc00(*this, A2GS_LC00_TAG),
		m_lc01(*this, A2GS_LC01_TAG),
		m_bank0_atc(*this, A2GS_B0CXXX_TAG),
		m_bank1_at0(*this, A2GS_B01_TAG),
		m_bank1_atc(*this, A2GS_B1CXXX_TAG),
		m_scc(*this, SCC_TAG),
		m_doc(*this, A2GS_DOC_TAG),
		m_iwm(*this, A2GS_IWM_TAG),
		m_ky0(*this, A2GS_KBD_Y0_TAG),
		m_ky1(*this, A2GS_KBD_Y1_TAG),
		m_ky2(*this, A2GS_KBD_Y2_TAG),
		m_ky3(*this, A2GS_KBD_Y3_TAG),
		m_ky4(*this, A2GS_KBD_Y4_TAG),
		m_ky5(*this, A2GS_KBD_Y5_TAG),
		m_ky6(*this, A2GS_KBD_Y6_TAG),
		m_ky7(*this, A2GS_KBD_Y7_TAG),
		m_ky8(*this, A2GS_KBD_Y8_TAG),
		m_ky9(*this, A2GS_KBD_Y9_TAG),
		m_kbspecial(*this, A2GS_KBD_SPEC_TAG),
		m_ay3600(*this, "ay3600"),
		m_kbdrom(*this, "keyboard"),
		m_adb_mousex(*this, "adb_mouse_x"),
		m_adb_mousey(*this, "adb_mouse_y")
	{ }

	required_device<g65816_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scantimer;
	required_device<m5074x_device> m_adbmicro;
	required_device<ram_device> m_ram;
	required_region_ptr<uint8_t> m_rom;
	required_shared_ptr<uint8_t> m_docram;
	required_device<nvram_device> m_nvram;
	required_device<a2_video_device> m_video;
	required_device<a2bus_device> m_a2bus;
	required_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	required_device<speaker_sound_device> m_speaker;
	required_device<address_map_bank_device> m_upperbank, m_upperaux, m_upper00, m_upper01;
	required_device<address_map_bank_device> m_c100bank, m_c300bank, m_c400bank, m_c800bank;
	required_device<address_map_bank_device> m_b0_0000bank, m_b0_0200bank, m_b0_0400bank, m_b0_0800bank, m_b0_2000bank, m_b0_4000bank;
	required_device<address_map_bank_device> m_lcbank, m_lcaux, m_lc00, m_lc01, m_bank0_atc, m_bank1_at0, m_bank1_atc;
	required_device<z80scc_device> m_scc;
	required_device<es5503_device> m_doc;
	required_device<applefdc_base_device> m_iwm;
	optional_ioport m_ky0, m_ky1, m_ky2, m_ky3, m_ky4, m_ky5, m_ky6, m_ky7, m_ky8, m_ky9;
	required_ioport m_kbspecial;
	optional_device<ay3600_device> m_ay3600;
	required_memory_region m_kbdrom;
	required_ioport m_adb_mousex, m_adb_mousey;

	enum glu_reg_names
	{
		// these are the MCU-visible registers
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

	enum glu_sys_status
	{
		GLU_STATUS_CMDFULL  = 0x01,
		GLU_STATUS_MOUSEXY  = 0x02,
		GLU_STATUS_KEYDATIRQEN = 0x04,
		GLU_STATUS_KEYDATIRQ = 0x08,
		GLU_STATUS_DATAIRQEN = 0x10,
		GLU_STATUS_DATAIRQ  = 0x20,
		GLU_STATUS_MOUSEIRQEN = 0x40,
		GLU_STATUS_MOUSEIRQ = 0x080
	};

	enum shadow_reg_bits
	{
		SHAD_IOLC       = 0x40, // I/O and language card inhibit for banks 00/01
		SHAD_TXTPG2     = 0x20, // inhibits text-page 2 shadowing in both banks (ROM 03 h/w only)
		SHAD_AUXHIRES   = 0x10, // inhibits bank 01 hi-res region shadowing
		SHAD_SUPERHIRES = 0x08, // inhibits bank 01 super-hi-res region shadowing
		SHAD_HIRESPG2   = 0x04, // inhibits hi-res page 2 shadowing in both banks
		SHAD_HIRESPG1   = 0x02, // inhibits hi-res page 1 shadowing in both banks
		SHAD_TXTPG1     = 0x01  // inhibits text-page 1 shadowing in both banks
	};

	enum speed_reg_bits
	{
		SPEED_HIGH      = 0x80, // full 2.8 MHz speed when set, Apple II 1 MHz when clear
		SPEED_POWERON   = 0x40, // ROM 03 only; indicates machine turned on by power switch (as opposed to ?)
		SPEED_ALLBANKS  = 0x10, // enables bank 0/1 shadowing in all banks (not supported)
		SPEED_DISKIISL7 = 0x08, // enable Disk II motor on detect for slot 7
		SPEED_DISKIISL6 = 0x04, // enable Disk II motor on detect for slot 6
		SPEED_DISKIISL5 = 0x02, // enable Disk II motor on detect for slot 5
		SPEED_DISKIISL4 = 0x01  // enable Disk II motor on detect for slot 4
	};

	enum disk_reg_bits
	{
		DISKREG_35HEADSEL   = 0x80, // head select for 3.5" "dumb" Sony drives
		DISKREG_35ENABLE    = 0x40  // 1 to enable 3.5" drives, 0 to chain through to 5.25"
	};

	enum irq_sources
	{
		IRQS_DOC        = 0,
		IRQS_SCAN       = 1,
		IRQS_ADB        = 2,
		IRQS_VBL        = 3,
		IRQS_SECOND     = 4,
		IRQS_QTRSEC     = 5,
		IRQS_SLOT       = 6,
		IRQS_SCC        = 7
	};

	enum intflag_bits
	{
		INTFLAG_IRQASSERTED = 0x01,
		INTFLAG_M2MOUSEMOVE = 0x02,
		INTFLAG_M2MOUSESW   = 0x04,
		INTFLAG_VBL         = 0x08,
		INTFLAG_QUARTER     = 0x10,
		INTFLAG_AN3         = 0x20,
		INTFLAG_MOUSEDOWNLAST = 0x40,
		INTFLAG_MOUSEDOWN   = 0x80
	};

	enum vgcint_bits
	{
		VGCINT_EXTERNALEN   = 0x01,
		VGCINT_SCANLINEEN   = 0x02,
		VGCINT_SECONDENABLE = 0x04,
		VGCINT_EXTERNAL     = 0x10,
		VGCINT_SCANLINE     = 0x20,
		VGCINT_SECOND       = 0x40,
		VGCINT_ANYVGCINT    = 0x80
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

	uint64_t m_last_adb_time;
	int m_adb_dtime;
	bool m_adb_line;

	address_space *m_maincpu_space;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void palette_init(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void apple2gs(machine_config &config);
	void apple2gsr1(machine_config &config);

	void apple2gs_map(address_map &map);
	void vectors_map(address_map &map);
	void c100bank_map(address_map &map);
	void c300bank_map(address_map &map);
	void c400bank_map(address_map &map);
	void c800bank_map(address_map &map);
	void inhbank_map(address_map &map);
	void inhaux_map(address_map &map);
	void inh00_map(address_map &map);
	void inh01_map(address_map &map);
	void lcbank_map(address_map &map);
	void lcaux_map(address_map &map);
	void lc00_map(address_map &map);
	void lc01_map(address_map &map);
	void bank0_iolc_map(address_map &map);
	void bank1_lower48_map(address_map &map);
	void bank1_iolc_map(address_map &map);
	void rb0000bank_map(address_map &map);
	void rb0200bank_map(address_map &map);
	void rb0400bank_map(address_map &map);
	void rb0800bank_map(address_map &map);
	void rb2000bank_map(address_map &map);
	void rb4000bank_map(address_map &map);
	void a2gs_es5503_map(address_map &map);

	// temp old IWM hookup
	int apple2_fdc_has_35();
	int apple2_fdc_has_525();
	void apple2_iwm_setdiskreg(uint8_t data);

	uint8_t m_diskreg;  // move into private when we can

	void rom1_init() { m_is_rom3 = false; }
	void rom3_init() { m_is_rom3 = true; }

private:
	DECLARE_READ8_MEMBER(ram0000_r);
	DECLARE_WRITE8_MEMBER(ram0000_w);
	DECLARE_READ8_MEMBER(auxram0000_r);
	DECLARE_WRITE8_MEMBER(auxram0000_w);
	DECLARE_READ8_MEMBER(b0ram0000_r);
	DECLARE_WRITE8_MEMBER(b0ram0000_w);
	DECLARE_READ8_MEMBER(b0ram0200_r);
	DECLARE_WRITE8_MEMBER(b0ram0200_w);
	DECLARE_READ8_MEMBER(b0ram0400_r);
	DECLARE_WRITE8_MEMBER(b0ram0400_w);
	DECLARE_READ8_MEMBER(b0ram0800_r);
	DECLARE_WRITE8_MEMBER(b0ram0800_w);
	DECLARE_READ8_MEMBER(b0ram2000_r);
	DECLARE_WRITE8_MEMBER(b0ram2000_w);
	DECLARE_READ8_MEMBER(b0ram4000_r);
	DECLARE_WRITE8_MEMBER(b0ram4000_w);
	DECLARE_READ8_MEMBER(b1ram0000_r);
	DECLARE_WRITE8_MEMBER(b1ram0000_w);
	DECLARE_READ8_MEMBER(b1ram0200_r);
	DECLARE_WRITE8_MEMBER(b1ram0200_w);
	DECLARE_READ8_MEMBER(b1ram0400_r);
	DECLARE_WRITE8_MEMBER(b1ram0400_w);
	DECLARE_READ8_MEMBER(b1ram0800_r);
	DECLARE_WRITE8_MEMBER(b1ram0800_w);
	DECLARE_READ8_MEMBER(b1ram2000_r);
	DECLARE_WRITE8_MEMBER(b1ram2000_w);
	DECLARE_READ8_MEMBER(b1ram4000_r);
	DECLARE_WRITE8_MEMBER(b1ram4000_w);
	DECLARE_READ8_MEMBER(c000_r);
	DECLARE_WRITE8_MEMBER(c000_w);
	DECLARE_READ8_MEMBER(c080_r);
	DECLARE_WRITE8_MEMBER(c080_w);
	DECLARE_READ8_MEMBER(c100_r);
	DECLARE_READ8_MEMBER(c100_int_r);
	DECLARE_WRITE8_MEMBER(c100_w);
	DECLARE_READ8_MEMBER(c300_r);
	DECLARE_READ8_MEMBER(c300_int_r);
	DECLARE_WRITE8_MEMBER(c300_w);
	DECLARE_READ8_MEMBER(c400_r);
	DECLARE_READ8_MEMBER(c400_int_r);
	DECLARE_WRITE8_MEMBER(c400_w);
	DECLARE_READ8_MEMBER(c800_r);
	DECLARE_READ8_MEMBER(c800_int_r);
	DECLARE_WRITE8_MEMBER(c800_w);
	DECLARE_READ8_MEMBER(inh_r);
	DECLARE_WRITE8_MEMBER(inh_w);
	DECLARE_READ8_MEMBER(lc_r);
	DECLARE_WRITE8_MEMBER(lc_w);
	DECLARE_READ8_MEMBER(lc_aux_r);
	DECLARE_WRITE8_MEMBER(lc_aux_w);
	DECLARE_READ8_MEMBER(lc_00_r);
	DECLARE_WRITE8_MEMBER(lc_00_w);
	DECLARE_READ8_MEMBER(lc_01_r);
	DECLARE_WRITE8_MEMBER(lc_01_w);
	DECLARE_READ8_MEMBER(bank0_c000_r);
	DECLARE_WRITE8_MEMBER(bank0_c000_w);
	DECLARE_READ8_MEMBER(bank1_0000_r);
	DECLARE_WRITE8_MEMBER(bank1_0000_w);
	DECLARE_WRITE8_MEMBER(bank1_0000_sh_w);
	DECLARE_READ8_MEMBER(bank1_c000_r);
	DECLARE_WRITE8_MEMBER(bank1_c000_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_WRITE_LINE_MEMBER(doc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(scc_irq_w);
	DECLARE_READ8_MEMBER(doc_adc_read);
	DECLARE_READ8_MEMBER(apple2gs_read_vector);

#if !RUN_ADB_MICRO
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	DECLARE_WRITE_LINE_MEMBER(ay3600_ako_w);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);
#endif

	uint8_t keyglu_mcu_read(uint8_t offset);
	void keyglu_mcu_write(uint8_t offset, uint8_t data);
	uint8_t keyglu_816_read(uint8_t offset);
	void keyglu_816_write(uint8_t offset, uint8_t data);
	void keyglu_regen_irqs();

	DECLARE_READ8_MEMBER(adbmicro_p0_in);
	DECLARE_READ8_MEMBER(adbmicro_p1_in);
	DECLARE_READ8_MEMBER(adbmicro_p2_in);
	DECLARE_READ8_MEMBER(adbmicro_p3_in);
	DECLARE_WRITE8_MEMBER(adbmicro_p0_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p1_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p2_out);
	DECLARE_WRITE8_MEMBER(adbmicro_p3_out);

private:
	bool m_is_rom3;
	int m_speaker_state;

	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;

	int m_inh_slot;
	int m_cnxx_slot;

	bool m_page2;
	bool m_an0, m_an1, m_an2, m_an3;

	bool m_vbl;

	int m_irqmask;

	bool m_intcxrom;
	bool m_80store;
	bool m_slotc3rom;
	bool m_altzp;
	bool m_ramrd, m_ramwrt;
	bool m_lcram, m_lcram2, m_lcprewrite, m_lcwriteenable;
	bool m_ioudis;

	uint8_t m_shadow, m_speed, m_textcol;
	uint8_t m_motors_active, m_slotromsel, m_intflag, m_vgcint, m_inten;

	bool m_last_speed;

	// Sound GLU variables
	uint8_t m_sndglu_ctrl;
	int m_sndglu_addr;
	int m_sndglu_dummy_read;

	// Key GLU variables
	uint8_t m_glu_regs[12], m_glu_bus, m_glu_sysstat;
	bool m_glu_mcu_read_kgs, m_glu_816_read_dstat, m_glu_mouse_read_stat;
	int m_glu_kbd_y;

	uint8_t *m_ram_ptr;
	int m_ram_size;

	int m_inh_bank;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];

	uint32_t m_slow_counter;

	// clock/BRAM
	uint8_t m_clkdata, m_clock_control, m_clock_read, m_clock_reg1;
	apple2gs_clock_mode m_clock_mode;
	uint32_t m_clock_curtime;
	seconds_t m_clock_curtime_interval;
	uint8_t m_clock_bram[256];
	int m_clock_frame;

	// ADB simulation
	#if !RUN_ADB_MICRO
	adbstate_t m_adb_state;
	uint8_t m_adb_command;
	uint8_t m_adb_mode;
	uint8_t m_adb_kmstatus;
	uint8_t m_adb_latent_result;
	int32_t m_adb_command_length;
	int32_t m_adb_command_pos;
	uint8_t m_adb_response_length;
	int32_t m_adb_response_pos;
	uint8_t m_adb_command_bytes[8];
	uint8_t m_adb_response_bytes[8];
	uint8_t m_adb_memory[0x100];
	int m_adb_address_keyboard;
	int m_adb_address_mouse;

	uint16_t m_lastchar, m_strobe;
	uint8_t m_transchar;
	bool m_anykeydown;
	int m_repeatdelay;

	uint8_t adb_read_datareg();
	uint8_t adb_read_kmstatus();
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
	#endif

	uint8_t m_mouse_x;
	uint8_t m_mouse_y;
	int8_t m_mouse_dx;
	int8_t m_mouse_dy;

	void do_io(address_space &space, int offset);
	uint8_t read_floatingbus();
	void update_slotrom_banks();
	void lc_update(int offset, bool writing);
	uint8_t read_slot_rom(address_space &space, int slotbias, int offset);
	void write_slot_rom(address_space &space, int slotbias, int offset, uint8_t data);
	uint8_t read_int_rom(address_space &space, int slotbias, int offset);
	void auxbank_update();
	void raise_irq(int irq);
	void lower_irq(int irq);
	void update_speed();
	int get_vpos();
	void process_clock();
};

// FF6ACF is speed test routine in ROM 3

#define slow_cycle() \
{   \
	if (m_last_speed) \
	{\
		m_slow_counter += 0x0003cccc; \
		int cycles = (m_slow_counter >> 16) & 0xffff; \
		m_slow_counter &= 0xffff; \
		m_maincpu->adjust_icount(-cycles); \
	} \
}


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

WRITE_LINE_MEMBER(apple2gs_state::a2bus_irq_w)
{
	if (state == ASSERT_LINE)
	{
		raise_irq(IRQS_SLOT);
	}
	else
	{
		lower_irq(IRQS_SLOT);
	}
}

WRITE_LINE_MEMBER(apple2gs_state::a2bus_nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// TODO: this assumes /INH only on ROM, needs expansion to support e.g. phantom-slotting cards and etc.
WRITE_LINE_MEMBER(apple2gs_state::a2bus_inh_w)
{
	if (state == ASSERT_LINE)
	{
		// assume no cards are pulling /INH
		m_inh_slot = -1;

		// scan the slots to figure out which card(s) are INHibiting stuff
		for (int i = 0; i <= 7; i++)
		{
			if (m_slotdevice[i])
			{
				// this driver only can inhibit from 0xd000-0xffff
				if ((m_slotdevice[i]->inh_start() == 0xd000) &&
					(m_slotdevice[i]->inh_end() == 0xffff))
				{
					if ((m_slotdevice[i]->inh_type() & INH_READ) == INH_READ)
					{
						if (m_inh_bank != 1)
						{
							m_upperbank->set_bank(1);
							m_upperaux->set_bank(1);
							m_upper00->set_bank(1);
							m_upper01->set_bank(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank->set_bank(0);
							m_upperaux->set_bank(0);
							m_upper00->set_bank(0);
							m_upper01->set_bank(0);
							m_inh_bank = 0;
						}
					}

					m_inh_slot = i;
					break;
				}
			}
		}

		// if no slots are inhibiting, make sure ROM is fully switched in
		if ((m_inh_slot == -1) && (m_inh_bank != 0))
		{
			m_upperbank->set_bank(0);
			m_upperaux->set_bank(0);
			m_upper00->set_bank(0);
			m_upper01->set_bank(0);
			m_inh_bank = 0;
		}
	}
}

// FPI/CYA chip is connected to the VPB output of the 65816.
// this facilitates the documented behavior from the Firmware Reference.
READ8_MEMBER(apple2gs_state::apple2gs_read_vector)
{
	// when IOLC shadowing is enabled, vector fetches always go to ROM,
	// regardless of the language card config.
	if (!(m_shadow & SHAD_IOLC))
	{
		return m_maincpu->space(AS_PROGRAM).read_byte(offset | 0xFFFFE0);
	}
	else    // else vector fetches from bank 0 RAM
	{
		return m_maincpu->space(AS_PROGRAM).read_byte((offset & 0xffff) | 0xFFE0);
	}
}

/***************************************************************************
    ADB MCU simulation
***************************************************************************/
#if !RUN_ADB_MICRO
READ_LINE_MEMBER(apple2gs_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

READ_LINE_MEMBER(apple2gs_state::ay3600_control_r)
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

WRITE_LINE_MEMBER(apple2gs_state::ay3600_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		uint8_t *decode = m_kbdrom->base();
		uint16_t trans;

		m_lastchar = m_ay3600->b_r();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place
		trans <<= 2;                    // 4 entries per key
		trans |= (m_kbspecial->read() & 0x06) ? 0x00 : 0x01;    // shift is bit 1 (active low)
		trans |= (m_kbspecial->read() & 0x08) ? 0x00 : 0x02;    // control is bit 2 (active low)
		trans |= (m_kbspecial->read() & 0x01) ? 0x0000 : 0x0200;    // caps lock is bit 9 (active low)

		m_transchar = decode[trans];
		m_strobe = 0x80;

		//printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);
	}
}

WRITE_LINE_MEMBER(apple2gs_state::ay3600_ako_w)
{
	m_anykeydown = (state == ASSERT_LINE) ? true : false;

	if (m_anykeydown)
	{
		m_repeatdelay = 10;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple2gs_state::ay3600_repeat)
{
	// is the key still down?
	if (m_anykeydown)
	{
		if (m_repeatdelay)
		{
			m_repeatdelay--;
		}
		else
		{
			m_strobe = 0x80;
		}
	}
}

uint8_t apple2gs_state::adb_read_memory(uint32_t address)
{
	if (address < ARRAY_LENGTH(m_adb_memory))
		return m_adb_memory[address];
	else
		return 0x00;
}

void apple2gs_state::adb_write_memory(uint32_t address, uint8_t data)
{
	if (address < ARRAY_LENGTH(m_adb_memory))
		m_adb_memory[address] = data;
}

void apple2gs_state::adb_set_mode(uint8_t mode)
{
	m_adb_mode = mode;
}

void apple2gs_state::adb_set_config(uint8_t b1, uint8_t b2, uint8_t b3)
{
	/* ignore for now */
}

void apple2gs_state::adb_post_response(const uint8_t *bytes, size_t length)
{
	assert(length < ARRAY_LENGTH(m_adb_response_bytes));
	memcpy(m_adb_response_bytes, bytes, length);

	m_adb_state = ADBSTATE_INRESPONSE;
	m_adb_response_length = length;
	m_adb_response_pos = 0;
}

void apple2gs_state::adb_post_response_1(uint8_t b)
{
	adb_post_response(&b, 1);
}

void apple2gs_state::adb_post_response_2(uint8_t b1, uint8_t b2)
{
	uint8_t b[2];
	b[0] = b1;
	b[1] = b2;
	adb_post_response(b, 2);
}

void apple2gs_state::adb_do_command()
{
	int device;
	uint32_t address;
	uint8_t val;

	m_adb_state = ADBSTATE_IDLE;
	if (LOG_ADB)
		logerror("adb_do_command(): adb_command=0x%02x\n", m_adb_command);

	switch(m_adb_command)
	{
		case 0x00:  /* ??? */
			break;

		case 0x03:  /* flush keyboard buffer */
			break;

		case 0x04:  /* set modes */
			adb_set_mode(m_adb_mode | m_adb_command_bytes[0]);
			break;

		case 0x05:  /* clear modes */
			adb_set_mode(m_adb_mode & ~m_adb_command_bytes[0]);
			break;

		case 0x06:  /* set config */
			adb_set_config(m_adb_command_bytes[0], m_adb_command_bytes[1], m_adb_command_bytes[2]);
			break;

		case 0x07:  /* synchronize */
			adb_set_mode(m_adb_command_bytes[0]);
			adb_set_config(m_adb_command_bytes[1], m_adb_command_bytes[2], m_adb_command_bytes[3]);
			break;

		case 0x08:  /* write memory */
			address = m_adb_command_bytes[0];
			val = m_adb_command_bytes[1];
			adb_write_memory(address, val);
			break;

		case 0x09:  /* read memory */
			address = (m_adb_command_bytes[1] << 8) | m_adb_command_bytes[0];
			adb_post_response_1(adb_read_memory(address));
			break;

		case 0x0a: /* ??? */
		case 0x0b: /* ??? */
			break;

		case 0x0d:  /* get version */
			adb_post_response_1(0x06);
			break;

		case 0x0e:  /* read available charsets */
			adb_post_response_2(0x01, 0x00);
			break;

		case 0x0f:  /* read available layouts */
			adb_post_response_2(0x01, 0x00);
			break;

		case 0x12:  /* mystery command 0x12 */
		case 0x13:  /* mystery command 0x13 */
			break;

		case 0x84:  // ACS demo disk #2 has a bug and writes this accidentally to $C026
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			/* send data to device */
			device = m_adb_command & 0x0f;
			if (device == m_adb_address_keyboard)
			{
			}
			else if (device == m_adb_address_mouse)
			{
			}
			break;

		case 0xf2:
			break;

		default:
			fatalerror("ADB command 0x%02x unimplemented\n", m_adb_command);
	}
	m_adb_kmstatus |= 0x20;
}

uint8_t apple2gs_state::adb_read_datareg()
{
	uint8_t result;

	switch(m_adb_state)
	{
		case ADBSTATE_INRESPONSE:
			result = m_adb_response_bytes[m_adb_response_pos++];
			if (m_adb_response_pos >= m_adb_response_length)
			{
				m_adb_state = ADBSTATE_IDLE;
				m_adb_latent_result = result;
				m_adb_kmstatus &= ~0x20;
			}
			break;

		default:
			result = 0; //m_adb_latent_result & 0x7f;
			break;
	}

	if (LOG_ADB)
		logerror("adb_read_datareg(): result=0x%02x\n", result);

	return result;
}

void apple2gs_state::adb_write_datareg(uint8_t data)
{
	if (LOG_ADB)
		logerror("adb_write_datareg(): data=0x%02x\n", data);

	switch(m_adb_state)
	{
		case ADBSTATE_IDLE:
			m_adb_command = data;
			m_adb_command_length = 0;
			m_adb_command_pos = 0;

//          printf("ADB command %02x\n", data);
			switch(data)
			{
				case 0x00:  /* ??? */
				case 0x01:  /* abort */
					/* do nothing for now */
					break;

				case 0x03:  /* flush keyboard buffer */
					m_adb_command_length = 0;
					break;

				case 0x04:  /* set modes */
				case 0x05:  /* clear modes */
					m_adb_command_length = 1;
					break;

				case 0x06:  /* set config */
					m_adb_command_length = 3;
					break;

				case 0x07:  /* synchronize */
					if (m_is_rom3)
						m_adb_command_length = 8;    // ROM 3 has 8 bytes: mode byte, 3 config bytes, kbd/mouse params, disk eject options
					else
						m_adb_command_length = 4;    // ROM 0/1 has 4 bytes sync
					break;

				case 0x08:  /* write memory */
				case 0x09:  /* read memory */
					m_adb_command_length = 2;
					break;

				case 0x0a:  /* ??? */
				case 0x0b:  /* ??? */
					m_adb_command_length = 0;
					break;

				case 0x0d:  /* get version */
					m_adb_command_length = 0;
					break;

				case 0x0e:  /* read available charsets */
					m_adb_command_length = 0;
					m_adb_state = ADBSTATE_INCOMMAND;    /* HACK */
					break;

				case 0x0f:  /* read available layouts */
					m_adb_command_length = 0;
					m_adb_state = ADBSTATE_INCOMMAND;    /* HACK */
					break;

				case 0x12:  /* mystery command 0x12 */
				case 0x13:  /* mystery command 0x13 */
					m_adb_command_length = 2;
					break;

				case 0x70:  /* disable SRQ device 0 */
				case 0x71:  /* disable SRQ device 1 */
				case 0x72:  /* disable SRQ device 2 */
				case 0x73:  /* disable SRQ device 3 */
					/* ignore for now */
					break;

				case 0x84:  // ACS demo disk #2 has a bug and writes this accidentally to $C026
					break;

				case 0xb0: case 0xb1: case 0xb2: case 0xb3:
				case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				case 0xb8: case 0xb9: case 0xba: case 0xbb:
				case 0xbc: case 0xbd: case 0xbe: case 0xbf:
					/* send data to device */
					m_adb_command_length = 2;
					break;

				case 0xe2:  // Jam Session sends this when starting a song
					break;

				case 0xf2:
					break;

				default:
					fatalerror("ADB command 0x%02x unimplemented\n", data);
			}

			if (m_adb_command_length > 0)
			{
				m_adb_state = ADBSTATE_INCOMMAND;
				if (LOG_ADB)
					logerror("adb_write_datareg(): in command length %u\n", (unsigned) m_adb_command_length);
			}
			break;

		case ADBSTATE_INCOMMAND:
			assert(m_adb_command_pos < ARRAY_LENGTH(m_adb_command_bytes));
//          printf("ADB param %02x\n", data);
			m_adb_command_bytes[m_adb_command_pos++] = data;
			break;

		case ADBSTATE_INRESPONSE:
			m_adb_state = ADBSTATE_IDLE;
			break;
	}

	/* do command if necessary */
	if ((m_adb_state == ADBSTATE_INCOMMAND) && (m_adb_command_pos >= m_adb_command_length))
		adb_do_command();
}

// real rom 3 h/w reads 0x90 when idle, 0x98 when key pressed
// current MESS reads back 0xb0 when idle
uint8_t apple2gs_state::adb_read_kmstatus()
{
	return m_adb_kmstatus;
}


void apple2gs_state::adb_write_kmstatus(uint8_t data)
{
	m_adb_kmstatus &= ~0x54;
	m_adb_kmstatus |= data & 0x54;
}



uint8_t apple2gs_state::adb_read_mousedata()
{
	uint8_t result = 0x00;
	uint8_t absolute;
	int8_t delta;

	if (m_adb_kmstatus & 0x80)   // mouse register full
	{
		if (m_adb_kmstatus & 0x02)   // H/V mouse data select
		{
			absolute = m_mouse_y;
			delta = m_mouse_dy;
			m_adb_kmstatus &= ~0x82;
			lower_irq(IRQS_ADB);
		}
		else
		{
			absolute = m_mouse_x;
			delta = m_mouse_dx;
			m_adb_kmstatus |= 0x02;
		}

		if (delta > 63)
			delta = 63;
		else if (delta < -64)
			delta = -64;

		result = (absolute & 0x80) | (delta & 0x7F);
	}
	return result;
}


int8_t apple2gs_state::seven_bit_diff(uint8_t v1, uint8_t v2)
{
	v1 -= v2;
	if (v1 & 0x40)
		v1 |= 0x80;
	else
		v1 &= ~0x80;
	return v1;
}



void apple2gs_state::adb_check_mouse()
{
	uint8_t new_mouse_x, new_mouse_y;

	/* read mouse values */
	if ((m_adb_kmstatus & 0x80) == 0x00)
	{
		new_mouse_x = m_adb_mousex->read();
		new_mouse_y = m_adb_mousey->read();

		if ((m_mouse_x != new_mouse_x) || (m_mouse_y != new_mouse_y))
		{
			m_mouse_dx = seven_bit_diff(new_mouse_x, m_mouse_x);
			m_mouse_dy = seven_bit_diff(new_mouse_y, m_mouse_y);
			m_mouse_x = new_mouse_x;
			m_mouse_y = new_mouse_y;

			m_adb_kmstatus |= 0x80;
			m_adb_kmstatus &= ~0x02;
			if (m_adb_kmstatus & 0x40)
			{
				//raise_irq(IRQS_ADB);
			}
		}
	}
}
#endif

/***************************************************************************
    START/RESET
***************************************************************************/

void apple2gs_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);
	m_upperbank->set_bank(0);
	m_upperaux->set_bank(0);
	m_upper00->set_bank(0);
	m_upper01->set_bank(0);
	m_lcbank->set_bank(0);
	m_lcaux->set_bank(0);
	m_lc00->set_bank(0);
	m_lc01->set_bank(0);
	m_b0_0000bank->set_bank(0);
	m_b0_0200bank->set_bank(0);
	m_b0_0400bank->set_bank(0);
	m_b0_0800bank->set_bank(0);
	m_b0_2000bank->set_bank(0);
	m_b0_4000bank->set_bank(0);
	m_inh_bank = 0;

	m_nvram->set_base(m_clock_bram, sizeof(m_clock_bram));

	// setup speaker toggle volumes.  this should be done mathematically probably,
	// but these ad-hoc values aren't too bad.
	static const int16_t lvlTable[16] =
	{
		0x0000, 0x03ff, 0x04ff, 0x05ff, 0x06ff, 0x07ff, 0x08ff, 0x09ff,
		0x0aff, 0x0bff, 0x0cff, 0x0fff, 0x1fff, 0x3fff, 0x5fff, 0x7fff
	};
	m_speaker->set_levels(16, lvlTable);

	// precalculate joystick time constants
	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();

	// cache slot devices
	for (int i = 0; i <= 7; i++)
	{
		m_slotdevice[i] = m_a2bus->get_a2bus_card(i);
	}

	// setup video pointers
	m_video->m_ram_ptr = m_ram_ptr;
	m_video->m_aux_ptr = &m_ram_ptr[0x10000];
	m_video->m_char_ptr = memregion("gfx1")->base();
	m_video->m_char_size = memregion("gfx1")->bytes();
	m_video->m_8bit_graphics = std::make_unique<bitmap_ind16>(560, 192);

	m_inh_slot = -1;
	m_cnxx_slot = CNXX_UNCLAIMED;

	// install memory beyond 256K/1M
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int ramsize = m_ram_size - 0x40000;

	// ROM 00/01 hardware: the quoted "256K" for a base machine *does* include banks e0/e1.
	space.install_readwrite_bank(0x020000, ramsize - 1 + 0x20000, "bank1");
	membank("bank1")->set_base(m_ram_ptr + 0x040000);

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_page2));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_intcxrom));
	save_item(NAME(m_80store));
	save_item(NAME(m_slotc3rom));
	save_item(NAME(m_altzp));
	save_item(NAME(m_ramrd));
	save_item(NAME(m_ramwrt));
	save_item(NAME(m_ioudis));
	save_item(NAME(m_vbl));
	save_item(NAME(m_irqmask));
	save_item(NAME(m_lcram));
	save_item(NAME(m_lcram2));
	save_item(NAME(m_lcprewrite));
	save_item(NAME(m_lcwriteenable));
	save_item(NAME(m_shadow));
	save_item(NAME(m_speed));
	save_item(NAME(m_textcol));
	save_item(NAME(m_clock_control));
	save_item(NAME(m_clkdata));
	save_item(NAME(m_motors_active));
	save_item(NAME(m_slotromsel));
	save_item(NAME(m_diskreg));
	save_item(NAME(m_sndglu_ctrl));
	save_item(NAME(m_sndglu_addr));
	save_item(NAME(m_sndglu_dummy_read));
	save_item(NAME(m_last_speed));
	save_item(NAME(m_glu_regs));
	save_item(NAME(m_glu_bus));
	save_item(NAME(m_glu_sysstat));
	save_item(NAME(m_glu_mcu_read_kgs));
	save_item(NAME(m_glu_816_read_dstat));
	save_item(NAME(m_glu_mouse_read_stat));
	save_item(NAME(m_glu_kbd_y));
	save_item(NAME(m_intflag));
	save_item(NAME(m_vgcint));
	save_item(NAME(m_inten));
	save_item(m_clkdata, "CLKDATA");
	save_item(m_clock_control, "CLKCTRL");
	save_item(m_clock_read, "CLKRD");
	save_item(m_clock_reg1, "CLKREG1");
	save_item(m_clock_curtime, "CLKCURTIME");
	save_item(m_clock_curtime_interval, "CLKCURTIMEINT");
//  save_item(m_clock_mode, "CLKMODE");
	save_item(NAME(m_clock_bram));
	save_item(NAME(m_clock_frame));
#if !RUN_ADB_MICRO
	save_item(NAME(m_adb_memory));
	save_item(NAME(m_adb_command_bytes));
	save_item(NAME(m_adb_response_bytes));
//  save_item(m_adb_state, "ADB/m_adb_state");
	save_item(m_adb_command, "ADB/m_adb_command");
	save_item(m_adb_mode, "ADB/m_adb_mode");
	save_item(m_adb_kmstatus, "ADB/m_adb_kmstatus");
	save_item(m_adb_latent_result, "ADB/m_adb_latent_result");
	save_item(m_adb_command_length, "ADB/m_adb_command_length");
	save_item(m_adb_command_pos, "ADB/m_adb_command_pos");
	save_item(m_adb_response_length, "ADB/m_adb_response_length");
	save_item(m_adb_response_pos, "ADB/m_adb_response_pos");
	save_item(m_adb_address_keyboard, "ADB/m_adb_address_keyboard");
	save_item(m_adb_address_mouse, "ADB/m_adb_address_mouse");
	save_item(NAME(m_lastchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_transchar));
#endif
	save_item(m_mouse_x, "MX");
	save_item(m_mouse_y, "MY");
	save_item(m_mouse_dx, "MDX");
	save_item(m_mouse_dy, "MDY");
}

void apple2gs_state::machine_reset()
{
	m_page2 = false;
	m_video->m_page2 = false;
	m_an0 = m_an1 = m_an2 = m_an3 = false;
	m_vbl = false;
	m_slotc3rom = false;
	m_irqmask = 0;
	m_intcxrom = false;
	m_80store = false;
	m_video->m_80store = false;
	m_altzp = false;
	m_ramrd = false;
	m_ramwrt = false;
	m_ioudis = true;
	m_clock_frame = 0;
	m_mouse_x = 0x00;
	m_mouse_y = 0x00;
	m_mouse_dx = 0x00;
	m_mouse_dy = 0x00;

	#if !RUN_ADB_MICRO
	m_adb_state = ADBSTATE_IDLE;
	m_adb_kmstatus = 0x00;
	m_adb_command = 0;
	m_adb_mode = 0;
	m_adb_latent_result = 0;
	m_adb_command_length = 0;
	m_adb_command_pos = 0;
	m_adb_response_length = 0;
	m_adb_response_pos = 0;
	memset(m_adb_command_bytes, 0, sizeof(m_adb_command_bytes));
	memset(m_adb_response_bytes, 0, sizeof(m_adb_response_bytes));
	memset(m_adb_memory, 0, sizeof(m_adb_memory));
	m_adb_address_keyboard = 2;
	m_adb_address_mouse = 3;
	#endif

	/* init time */
	m_clkdata = 0;
	m_clock_control =0;
	m_clock_read = 0;
	m_clock_reg1 = 0;
	m_clock_mode = CLOCKMODE_IDLE;
	m_clock_curtime_interval = 0;

	// seed the clock with real time
	struct tm cur_time, mac_reference;
	system_time curtime;
	machine().current_datetime(curtime);

	cur_time.tm_sec = curtime.local_time.second;
	cur_time.tm_min = curtime.local_time.minute;
	cur_time.tm_hour = curtime.local_time.hour;
	cur_time.tm_mday = curtime.local_time.mday;
	cur_time.tm_mon = curtime.local_time.month;
	cur_time.tm_year = curtime.local_time.year-1900;
	cur_time.tm_isdst = 0;

	/* The count starts on 1st January 1904 */
	mac_reference.tm_sec = 0;
	mac_reference.tm_min = 0;
	mac_reference.tm_hour = 0;
	mac_reference.tm_mday = 1;
	mac_reference.tm_mon = 0;
	mac_reference.tm_year = 4;
	mac_reference.tm_isdst = 0;

	m_clock_curtime = difftime(mktime(&cur_time), mktime(&mac_reference));

	m_shadow = 0x00;
	m_speed = 0x80;
	m_motors_active = 0;
	m_diskreg = 0;
	m_intflag = 0;
	m_vgcint = 0;
	m_inten = 0;

	m_slow_counter = 0;

	// always assert full speed on reset
	m_maincpu->set_unscaled_clock(A2GS_14M/5);
	m_last_speed = true;

	m_sndglu_ctrl = 0;
	m_sndglu_addr = 0;
	m_sndglu_dummy_read = 0;

	m_maincpu_space = &m_maincpu->space(AS_PROGRAM);

	m_b0_0000bank->set_bank(0);
	m_b0_0200bank->set_bank(0);
	m_b0_0400bank->set_bank(0);
	m_b0_0800bank->set_bank(0);
	m_b0_2000bank->set_bank(0);
	m_b0_4000bank->set_bank(0);
	m_bank0_atc->set_bank(1);
	m_bank1_at0->set_bank(1);
	m_bank1_atc->set_bank(1);

	// LC default state: read ROM, write enabled, Dxxx bank 2
	m_lcram = false;
	m_lcram2 = true;
	m_lcprewrite = false;
	m_lcwriteenable = true;

	// sync up the banking with the variables.
	// RESEARCH: how does RESET affect LC state and aux banking states?
	auxbank_update();
	update_slotrom_banks();

	// with all the banking reset, now reset the CPU
	m_maincpu->reset();
}

void apple2gs_state::raise_irq(int irq)
{
	if (!(m_irqmask & (1 << irq)))
	{
		m_irqmask |= (1 << irq);

		//printf("raise IRQ %d (mask %x)\n", irq, m_irqmask);

		if (m_irqmask)
		{
			m_intflag |= INTFLAG_IRQASSERTED;
			m_maincpu->set_input_line(G65816_LINE_IRQ, ASSERT_LINE);
		}
	}
}


void apple2gs_state::lower_irq(int irq)
{
	if (m_irqmask & (1 << irq))
	{
		m_irqmask &= ~(1 << irq);

		//printf("lower IRQ %d (mask %x)\n", irq, m_irqmask);

		if (!m_irqmask)
		{
			m_intflag &= ~INTFLAG_IRQASSERTED;
			m_maincpu->set_input_line(G65816_LINE_IRQ, CLEAR_LINE);
		}
		else
		{
			m_maincpu->set_input_line(G65816_LINE_IRQ, ASSERT_LINE);
		}
	}
}

void apple2gs_state::update_speed()
{
	bool isfast = false;

	if (m_speed & SPEED_HIGH)
	{
		isfast = true;
	}

	if ((m_motors_active & (m_speed & 0x0f)) != 0)
	{
		isfast = false;
	}

	// prevent unnecessary reschedules by only setting this if it changed
	if (isfast != m_last_speed)
	{
		m_maincpu->set_unscaled_clock(isfast ? A2GS_14M/5 : A2GS_1M);
		m_last_speed = isfast;
	}
}

/***************************************************************************
    VIDEO
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(apple2gs_state::apple2_interrupt)
{
	int scanline = m_screen->vpos();
	m_screen->update_partial(scanline);

	if ((scanline % 8) == 0)
	{
		#if !RUN_ADB_MICRO
		adb_check_mouse();
		#endif
	}

	/* check scanline interrupt bits if we're in super hi-res and the current scanline is within the active display area */
	if ((m_video->m_newvideo & 0x80) && (scanline >= (BORDER_TOP-1)) && (scanline < (200+BORDER_TOP-1)))
	{
		uint8_t scb;

		scb = m_ram_ptr[0x19d00 + scanline - BORDER_TOP + 1];

		if (scb & 0x40)
		{
			// scanline int flag is set even when the actual interrupt is disabled
			m_vgcint |= VGCINT_SCANLINE;

			// see if the interrupt is also enabled and trigger it if so
			if (m_vgcint & VGCINT_SCANLINEEN)
			{
				m_vgcint |= VGCINT_ANYVGCINT;
				raise_irq(IRQS_SCAN);
			}
		}
	}

	if (scanline == (192+BORDER_TOP))
	{
		m_vbl = true;

		/* VBL interrupt */
		if ((m_inten & 0x08) && !(m_intflag & INTFLAG_VBL))
		{
			m_intflag |= INTFLAG_VBL;
			raise_irq(IRQS_VBL);
		}

		m_adbmicro->set_input_line(0, ASSERT_LINE);
		m_video->m_sysconfig = 0;

		m_clock_frame++;

		// quarter second?
		if ((m_clock_frame % 15) == 0)
		{
			if ((m_inten & 0x10) && !(m_intflag & INTFLAG_QUARTER))
			{
				m_intflag |= INTFLAG_QUARTER;
				raise_irq(IRQS_QTRSEC);
			}
		}

		// one second
		if (m_clock_frame >= 60)
		{
			//printf("one sec, vgcint = %02x\n", m_vgcint);
			m_clock_frame = 0;
			m_clock_curtime++;

			if ((m_vgcint & VGCINT_SECONDENABLE) && !(m_vgcint & VGCINT_SECOND))
			{
				m_vgcint |= (VGCINT_SECOND|VGCINT_ANYVGCINT);
				raise_irq(IRQS_SECOND);
			}
		}
	}
	else if (scanline == (192+BORDER_TOP+1))
	{
		m_adbmicro->set_input_line(1, ASSERT_LINE);
	}
}

void apple2gs_state::palette_init(palette_device &palette)
{
	static const unsigned char apple2gs_palette[] =
	{
		0x0, 0x0, 0x0,  /* Black         $0              $0000 */
		0xD, 0x0, 0x3,  /* Deep Red      $1              $0D03 */
		0x0, 0x0, 0x9,  /* Dark Blue     $2              $0009 */
		0xD, 0x2, 0xD,  /* Purple        $3              $0D2D */
		0x0, 0x7, 0x2,  /* Dark Green    $4              $0072 */
		0x5, 0x5, 0x5,  /* Dark Gray     $5              $0555 */
		0x2, 0x2, 0xF,  /* Medium Blue   $6              $022F */
		0x6, 0xA, 0xF,  /* Light Blue    $7              $06AF */
		0x8, 0x5, 0x0,  /* Brown         $8              $0850 */
		0xF, 0x6, 0x0,  /* Orange        $9              $0F60 */
		0xA, 0xA, 0xA,  /* Light Gray    $A              $0AAA */
		0xF, 0x9, 0x8,  /* Pink          $B              $0F98 */
		0x1, 0xD, 0x0,  /* Light Green   $C              $01D0 */
		0xF, 0xF, 0x0,  /* Yellow        $D              $0FF0 */
		0x4, 0xF, 0x9,  /* Aquamarine    $E              $04F9 */
		0xF, 0xF, 0xF   /* White         $F              $0FFF */
	};

	for (int i = 0; i < 16; i++)
	{
		palette.set_pen_color(i,
			apple2gs_palette[(3*i)]*17,
			apple2gs_palette[(3*i)+1]*17,
			apple2gs_palette[(3*i)+2]*17);

		m_video->m_GSborder_colors[i] = rgb_t(apple2gs_palette[(3*i)]*17, apple2gs_palette[(3*i)+1]*17, apple2gs_palette[(3*i)+2]*17);
	}
}

uint32_t apple2gs_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_video->screen_update_GS(screen, bitmap, cliprect);
}

/***************************************************************************
    I/O
***************************************************************************/
void apple2gs_state::auxbank_update()
{
	int ramwr = (m_ramrd ? 1 : 0) | (m_ramwrt ? 2 : 0);

	m_b0_0000bank->set_bank(m_altzp ? 1 : 0);
	m_b0_0200bank->set_bank(ramwr);

	if (m_80store)
	{
		if (m_page2)
		{
			m_b0_0400bank->set_bank(3);
		}
		else
		{
			m_b0_0400bank->set_bank(0);
		}
	}
	else
	{
		m_b0_0400bank->set_bank(ramwr);
	}

	m_b0_0800bank->set_bank(ramwr);

	if ((m_80store) && (m_video->m_hires))
	{
		if (m_page2)
		{
			m_b0_2000bank->set_bank(3);
		}
		else
		{
			m_b0_2000bank->set_bank(0);
		}
	}
	else
	{
		m_b0_2000bank->set_bank(ramwr);
	}

	m_b0_4000bank->set_bank(ramwr);
}

void apple2gs_state::update_slotrom_banks()
{
	int cxswitch = 0;

	if (m_intcxrom)
	{
		cxswitch = 1;
	}

	m_c100bank->set_bank(cxswitch);
	m_c400bank->set_bank(cxswitch);

	//printf("update_slotrom_banks: intcxrom %d cnxx_slot %d SLOT %02x\n", m_intcxrom, m_cnxx_slot, m_slotromsel);
	if ((m_intcxrom) || (m_cnxx_slot < 0))
	{
		m_c800bank->set_bank(1);
	}
	else
	{
		m_c800bank->set_bank(0);
	}

	// slot 3 ROM is controlled exclusively by SLOTC3ROM
	if (!m_slotc3rom)
	{
		m_c300bank->set_bank(1);
	}
	else
	{
		m_c300bank->set_bank(0);
	}
}

void apple2gs_state::lc_update(int offset, bool writing)
{
	bool old_lcram = m_lcram;

	//any even access disables pre-write and writing
	if ((offset & 1) == 0)
	{
		m_lcprewrite = false;
		m_lcwriteenable = false;
	}

	//any write disables pre-write
	//has no effect on write-enable if writing was enabled already
	if (writing == true)
	{
		m_lcprewrite = false;
	}
	//first odd read enables pre-write, second one enables writing
	else if ((offset & 1) == 1)
	{
		if (m_lcprewrite == false)
		{
			m_lcprewrite = true;
		}
		else
		{
			m_lcwriteenable = true;
		}
	}

	switch (offset & 3)
	{
		case 0:
		case 3:
		{
			m_lcram = true;
			break;
		}

		case 1:
		case 2:
		{
			m_lcram = false;
			break;
		}
	}

	m_lcram2 = false;

	if (!(offset & 8))
	{
		m_lcram2 = true;
	}

	if (m_lcram != old_lcram)
	{
		if (m_lcram)
		{
			m_lcbank->set_bank(1);
			m_lcaux->set_bank(1);
			m_lc00->set_bank(1);
			m_lc01->set_bank(1);
		}
		else
		{
			m_lcbank->set_bank(0);
			m_lcaux->set_bank(0);
			m_lc00->set_bank(0);
			m_lc01->set_bank(0);
		}
	}

	#if 0
	printf("LC: new state %c%c dxxx=%04x altzp=%d\n",
			m_lcram ? 'R' : 'x',
			m_lcwriteenable ? 'W' : 'x',
			m_lcram2 ? 0x1000 : 0x0000,
			m_altzp);
	#endif
}

// most softswitches don't care about read vs write, so handle them here
void apple2gs_state::do_io(address_space &space, int offset)
{
	if(machine().side_effects_disabled()) return;

	if (m_ioudis)
	{
		switch (offset)
		{
			case 0x5e:  // SETDHIRES
				m_video->m_dhires = true;
				return;

			case 0x5f:  // CLRDHIRES
				m_video->m_dhires = false;
				return;
		}
	}

	switch (offset)
	{
		case 0x20:
			break;

		case 0x28:
			break;

		case 0x30:
			m_speaker_state ^= 1;
			if (m_speaker_state)
			{
				m_speaker->level_w(m_sndglu_ctrl & 0xf);
			}
			else
			{
				m_speaker->level_w(0);
			}
			break;

		case 0x50:  // graphics mode
			m_video->m_graphics = true;
			break;

		case 0x51:  // text mode
			m_video->m_graphics = false;
			break;

		case 0x52:  // no mix
			m_video->m_mix = false;
			break;

		case 0x53:  // mixed mode
			m_video->m_mix = true;
			break;

		case 0x54:  // set page 1
			m_page2 = false;
			m_video->m_page2 = false;
			auxbank_update();
			break;

		case 0x55:  // set page 2
			m_page2 = true;
			m_video->m_page2 = true;
			auxbank_update();
			break;

		case 0x56: // select lo-res
			m_video->m_hires = false;
			auxbank_update();
			break;

		case 0x57: // select hi-res
			m_video->m_hires = true;
			auxbank_update();
			break;

		case 0x58: // AN0 off
			m_an0 = false; break;

		case 0x59: // AN0 on
			m_an0 = true; break;

		case 0x5a: // AN1 off
			m_an1 = false; break;

		case 0x5b: // AN1 on
			m_an1 = true; break;

		case 0x5c: // AN2 off
			m_an2 = false; break;

		case 0x5d: // AN2 on
			m_an2 = true; break;

		case 0x5e: // AN3 off
			m_an3 = false; break;

		case 0x5f: // AN3 on
			m_an3 = true; break;

		case 0x68:  // STATE
			break;

		// trigger joypad read
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_joy1x->read();
			m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_joy1y->read();
			m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_joy2x->read();
			m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_joy2y->read();
			break;

		default:
			logerror("do_io: unknown switch $C0%02X\n", offset);
			break;
	}
}

// apple2gs_get_vpos - return the correct vertical counter value for the current scanline,
// keeping borders in mind.

int apple2gs_state::get_vpos()
{
	int result, scan;
	static const uint8_t top_border_vert[BORDER_TOP] =
	{
		0xfa, 0xfa, 0xfa, 0xfa, 0xfb, 0xfb, 0xfb, 0xfb,
		0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfe, 0xfe, 0xff,

	};

	scan = m_screen->vpos();

	if (scan < BORDER_TOP)
	{
		result = top_border_vert[scan];
	}
	else
	{
		result = scan - BORDER_TOP + 0x100 + 1;
	}

	return result;
}

void apple2gs_state::process_clock()
{
	uint8_t operation;

	switch(m_clock_mode)
	{
		case CLOCKMODE_IDLE:
			m_clock_read = (m_clkdata >> 7);
			m_clock_reg1 = (m_clkdata >> 2) & 0x03;
			operation = (m_clkdata >> 4) & 0x07;
			if ((m_clkdata & 0x40) == 0x00)
			{
				switch(operation)
				{
					case 0x00:
						/* read/write seconds register */
						m_clock_mode = CLOCKMODE_TIME;
						break;

					case 0x03:
						/* internal registers */
						if (m_clock_reg1 & 0x02)
						{
							m_clock_mode = CLOCKMODE_BRAM2;
							m_clock_reg1 = (m_clkdata & 0x07) << 5;
						}
						else
						{
							m_clock_mode = CLOCKMODE_INTERNALREGS;
						}
						break;

					default:
						//fatalerror("NYI\n");
						break;
				}
			}
			break;

		case CLOCKMODE_BRAM1:
			if (m_clock_read)
			{
				m_clkdata = m_clock_bram[m_clock_reg1];
				//printf("Read BRAM %02x = %02x\n", m_clock_reg1, m_clkdata);
			}
			else
			{
				//printf("Write BRAM %02x = %02x\n", m_clock_reg1, m_clkdata);
				m_clock_bram[m_clock_reg1] = m_clkdata;
			}
			m_clock_mode = CLOCKMODE_IDLE;
			break;

		case CLOCKMODE_BRAM2:
			m_clock_reg1 |= (m_clkdata >> 2) & 0x1F;
			m_clock_mode = CLOCKMODE_BRAM1;
			break;

		case CLOCKMODE_INTERNALREGS:
			//printf("internalregs, reg %d, read %x\n", m_clock_reg1, m_clock_read);
			switch (m_clock_reg1)
			{
				case 0x00:
					/* test register */
					break;

				case 0x01:
					/* write protect register */
					break;
			}
			m_clock_mode = CLOCKMODE_IDLE;
			break;

		case CLOCKMODE_TIME:
			if (m_clock_control & 0x40)
			{
				m_clkdata = m_clock_curtime >> (m_clock_reg1 * 8);
				//printf("Read time reg %x = %x\n", m_clock_reg1, m_clkdata);
			}
			else
			{
				m_clock_curtime &= ~(0xFF << (m_clock_reg1 * 8));
				m_clock_curtime |= m_clkdata << (m_clock_reg1 * 8);
				//printf("Write time reg %x = %x\n", m_clock_reg1, m_clkdata);
			}
			m_clock_mode = CLOCKMODE_IDLE;
			break;

		default:
			//fatalerror("NYI\n");
			break;
	}
}

READ8_MEMBER(apple2gs_state::c000_r)
{
	uint8_t ret;

	// allow ROM window at C07x to be debugger-visible
	if ((offset < 0x70) || (offset > 0x7f))
	{
		if(machine().side_effects_disabled()) return read_floatingbus();
	}

	slow_cycle();
	u8 uFloatingBus7 = read_floatingbus();

	switch (offset)
	{
		case 0x00:  // keyboard latch
		#if RUN_ADB_MICRO
			return keyglu_816_read(GLU_C000);
		#else
			return m_transchar | m_strobe;
		#endif

		case 0x02:  // RAMRDOFF
			m_ramrd = false;
			auxbank_update();
			break;

		case 0x03:  // RAMRDON
			m_ramrd = true;
			auxbank_update();
			break;

		case 0x04:  // RAMWRTOFF
			m_ramwrt = false;
			auxbank_update();
			break;

		case 0x05:  // RAMWRTON
			m_ramwrt = true;
			auxbank_update();
			break;

		case 0x10:  // read any key down, reset keyboard strobe
		#if RUN_ADB_MICRO
			return keyglu_816_read(GLU_C010);
		#else
			ret = m_transchar | (m_anykeydown ? 0x80 : 0x00);
			m_strobe = 0;
			return ret;
		#endif

		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return m_lcram2 ? 0x80 : 0x00;

		case 0x12:  // read LCRAM (is LC readable?)
			return m_lcram ? 0x80 : 0x00;

		case 0x13:  // read RAMRD
			return m_ramrd ? 0x80 : 0x00;

		case 0x14:  // read RAMWRT
			return m_ramwrt ? 0x80 : 0x00;

		case 0x15:  // read INTCXROM
			return m_intcxrom ? 0x80 : 0x00;

		case 0x16:  // read ALTZP
			return m_altzp ? 0x80 : 0x00;

		case 0x17:  // read SLOTC3ROM
			return m_slotc3rom ? 0x80 : 0x00;

		case 0x18:  // read 80STORE
			return m_80store ? 0x80 : 0x00;

		case 0x19:  // read VBLBAR
			return m_screen->vblank() ? 0x00 : 0x80;

		case 0x1a:  // read TEXT
			return m_video->m_graphics ? 0x00 : 0x80;

		case 0x1b:  // read MIXED
			return m_video->m_mix ? 0x80 : 0x00;

		case 0x1c:  // read PAGE2
			return m_page2 ? 0x80 : 0x00;

		case 0x1d:  // read HIRES
			return m_video->m_hires ? 0x80 : 0x00;

		case 0x1e:  // read ALTCHARSET
			return m_video->m_altcharset ? 0x80 : 0x00;

		case 0x1f:  // read 80COL
			return m_video->m_80col ? 0x80 : 0x00;

		case 0x22:  // TEXTCOL
			return m_textcol;

		case 0x23:  // VGCINT
			return m_vgcint;
#if RUN_ADB_MICRO
		case 0x24:  // MOUSEDATA */
			return keyglu_816_read(GLU_MOUSEX);

		case 0x25:  // KEYMODREG
			return keyglu_816_read(GLU_KEYMOD);

		case 0x26:  // DATAREG
			return keyglu_816_read(GLU_DATA);

		case 0x27:  // KMSTATUS
			return keyglu_816_read(GLU_SYSSTAT);
#else
		case 0x24:  // MOUSEDATA */
			return adb_read_mousedata();

		case 0x25:  // KEYMODREG
			ret = 0;
			{
				uint8_t temp = m_kbspecial->read();
				if (temp & 1)   // capslock
				{
					ret |= 4;
				}
				if (temp & 6)   // shift
				{
					ret |= 1;
				}
				if (temp & 8)   // control
				{
					ret |= 2;
				}
				if (temp & 0x10)    // open apple/command
				{
					ret |= 0x40;
				}
				if (temp & 0x20)    // option
				{
					ret |= 0x80;
				}
				// keypad is a little rough right now
				if (m_lastchar >= 0x28 && m_lastchar <= 0x2d)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x32 && m_lastchar <= 0x3f)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x100 && m_lastchar <= 0x101)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x109 && m_lastchar <= 0x10a)
				{
					ret |= 0x10;
				}
			}
			return ret;

		case 0x26:  // DATAREG
			return adb_read_datareg();

		case 0x27:  // KMSTATUS
		// hack to let one-second IRQs get through in Nucleus
			if (m_vgcint & VGCINT_SECOND)
				return 0;

			return adb_read_kmstatus();
#endif

		case 0x29:  // NEWVIDEO
			return m_video->m_newvideo;

		case 0x2d:  // SLOTROMSEL
			return m_slotromsel;

		case 0x2e:  // VERTCNT
			return get_vpos() >> 1;

		case 0x2f:  // HORIZCNT
			ret = m_screen->hpos() / 11;
			if (ret > 0)
			{
				ret += 0x40;
			}

			if (get_vpos() & 1)
			{
				ret |= 0x80;
			}
			return ret;

		case 0x31:  // DISKREG
			return m_diskreg;

		case 0x33: // CLOCKDATA
			return m_clkdata;

		case 0x34:  // BORDERCOL
			return m_clock_control;

		case 0x35:  // SHADOW
			return m_shadow;

		case 0x36:  // SPEED/CYAREG
			return m_speed;

		case 0x38:  // SCCBREG
			return m_scc->cb_r(0);

		case 0x39:  // SCCAREG
			return m_scc->ca_r(0);

		case 0x3a:  // SCCBDATA
			return m_scc->db_r(0);

		case 0x3b:  // SCCADATA
			return m_scc->da_r(0);

		case 0x3c:  // SOUNDCTL
			return m_sndglu_ctrl;

		case 0x3d:  // SOUNDDATA
			ret = m_sndglu_dummy_read;

			if (m_sndglu_ctrl & 0x40)    // docram access
			{
				m_sndglu_dummy_read = m_docram[m_sndglu_addr];
			}
			else
			{
				m_sndglu_dummy_read = m_doc->read(space, m_sndglu_addr);
			}

			if (m_sndglu_ctrl & 0x20)    // auto-increment
			{
				m_sndglu_addr++;
			}
			return ret;

		case 0x3e:  // SOUNDADRL
			return m_sndglu_addr & 0xff;

		case 0x3f:  // SOUNDADRH
			return (m_sndglu_addr >> 8) & 0xff;

		case 0x41:  // INTEN
			return m_inten;

		case 0x46:  // INTFLAG
			return (m_an3 ? INTFLAG_AN3 : 0x00) | m_intflag;

		case 0x60: // button 3 on IIgs
			return (m_joybuttons->read() & 0x80) | uFloatingBus7;

		case 0x61:  // button 0 or Open Apple
			return (((m_joybuttons->read() & 0x10) || (m_kbspecial->read() & 0x10)) ? 0x80 : 0) | uFloatingBus7;

		case 0x62:  // button 1 or Solid Apple
			return (((m_joybuttons->read() & 0x20) || (m_kbspecial->read() & 0x20)) ? 0x80 : 0) | uFloatingBus7;

		case 0x63:  // button 2 or SHIFT key
			return (((m_joybuttons->read() & 0x40) || (m_kbspecial->read() & 0x06)) ? 0x80 : 0) | uFloatingBus7;

		case 0x64:  // joy 1 X axis
			return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x65:  // joy 1 Y axis
			return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x66: // joy 2 X axis
			return ((machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x67: // joy 2 Y axis
			return ((machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x68: // STATEREG, synthesizes all the IIe state regs
			return  (m_altzp ? 0x80 : 0x00) |
					(m_page2 ? 0x40 : 0x00) |
					(m_ramrd ? 0x20 : 0x00) |
					(m_ramwrt ? 0x10 : 0x00) |
					(m_lcram ? 0x00 : 0x08) |
					(m_lcram2 ? 0x04 : 0x00) |
					(m_intcxrom ? 0x01 : 0x00);

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			// todo: does reading these on the IIgs also trigger the joysticks?
			if (!machine().side_effects_disabled())
			{
				m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_joy1x->read();
				m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_joy1y->read();
				m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_joy2x->read();
				m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_joy2y->read();
			}

			return m_rom[offset + 0x3c000];
			break;

		default:
			do_io(space, offset);
			break;
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2gs_state::c000_w)
{
	if(machine().side_effects_disabled()) return;

	slow_cycle();

	switch (offset)
	{
		case 0x00:  // 80STOREOFF
			m_80store = false;
			m_video->m_80store = false;
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_80store = true;
			m_video->m_80store = true;
			auxbank_update();
			break;

		case 0x02:  // RAMRDOFF
			m_ramrd = false;
			auxbank_update();
			break;

		case 0x03:  // RAMRDON
			m_ramrd = true;
			auxbank_update();
			break;

		case 0x04:  // RAMWRTOFF
			m_ramwrt = false;
			auxbank_update();
			break;

		case 0x05:  // RAMWRTON
			m_ramwrt = true;
			auxbank_update();
			break;

		case 0x06:  // INTCXROMOFF
			m_intcxrom = false;
			update_slotrom_banks();
			break;

		case 0x07:  // INTCXROMON
			m_intcxrom = true;
			update_slotrom_banks();
			break;

		case 0x08:  // ALTZPOFF
			m_altzp = false;
			auxbank_update();
			break;

		case 0x09:  // ALTZPON
			m_altzp = true;
			auxbank_update();
			break;

		case 0x0a:  // SETINTC3ROM
			m_slotc3rom = false;
			update_slotrom_banks();
			break;

		case 0x0b:  // SETSLOTC3ROM
			m_slotc3rom = true;
			update_slotrom_banks();
			break;

		case 0x0c:  // 80COLOFF
			m_video->m_80col = false;
			break;

		case 0x0d:  // 80COLON
			m_video->m_80col = true;
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->m_altcharset = false;
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->m_altcharset = true;
			break;

		case 0x10:  // clear keyboard latch
		#if RUN_ADB_MICRO
			keyglu_816_write(GLU_C010, data);
			break;
		#else
			m_strobe = 0;
			break;
		#endif

		case 0x20:
			break;

		case 0x21:  // MONOCHROME
			m_video->m_monochrome = data;
			break;

		case 0x22:  // TEXTCOL
			m_textcol = data;
			m_video->m_GSfg = (data >> 4) & 0xf;
			m_video->m_GSbg = data & 0xf;
			break;

		case 0x23:  // VGCINT
			if ((m_vgcint & VGCINT_SECOND) && !(data & VGCINT_SECOND))
			{
				lower_irq(IRQS_SECOND);
				m_vgcint &= ~(VGCINT_SECOND);
			}
			if ((m_vgcint & VGCINT_SCANLINE) && !(data & VGCINT_SCANLINE))
			{
				lower_irq(IRQS_SCAN);
				m_vgcint &= ~(VGCINT_SCANLINE);
			}

			if (!(m_vgcint & (VGCINT_SECOND|VGCINT_SCANLINE)))
			{
				m_vgcint &= ~(VGCINT_ANYVGCINT);
			}

			m_vgcint &= 0xf0;
			m_vgcint |= (data & 0x0f);
			//printf("%02x to VGCINT, now %02x\n", data, m_vgcint);
			break;

#if RUN_ADB_MICRO
		case 0x26:  // DATAREG
			keyglu_816_write(GLU_COMMAND, data);
			break;

		case 0x27:  // KMSTATUS
			keyglu_816_write(GLU_SYSSTAT, data);
			break;
#else
		case 0x26:  // DATAREG
			adb_write_datareg(data);
			break;

		case 0x27:  // KMSTATUS
			adb_write_kmstatus(data);
			break;
#endif

		case 0x29:  // NEWVIDEO
			m_video->m_newvideo = data;
			break;

		case 0x2d:  // SLOTROMSEL
			m_slotromsel = data;
			break;

		case 0x31:  // DISKREG
			m_diskreg = data;
			apple2_iwm_setdiskreg(m_diskreg);
			break;

		case 0x32:  // VGCINTCLEAR
			//printf("%02x to VGCINTCLEAR\n", data);
			// one second
			if ((m_vgcint & VGCINT_SECOND) && !(data & VGCINT_SECOND))
			{
				lower_irq(IRQS_SECOND);
				m_vgcint &= ~(VGCINT_SECOND|VGCINT_ANYVGCINT);
			}

			// scanline
			if ((m_vgcint & VGCINT_SCANLINE) && !(data & VGCINT_SCANLINE))
			{
				lower_irq(IRQS_SCAN);
				m_vgcint &= ~(VGCINT_SCANLINE|VGCINT_ANYVGCINT);
			}

			if (m_irqmask & ((1<<IRQS_SECOND) | (1<<IRQS_SCAN)))
			{
				m_vgcint |= VGCINT_ANYVGCINT;
			}
			break;

		case 0x33:  // CLOCKDATA
			m_clkdata = data;
			break;

		case 0x34:  // CLOCKCTL
			m_clock_control = data & 0x7f;
			m_video->m_GSborder = data & 0xf;
			if (data & 0x80)
			{
				process_clock();
			}
			break;

		case 0x35:  // SHADOW
			m_shadow = data;

			// handle I/O and language card inhibit bits here
			if (m_shadow & SHAD_IOLC)
			{
				m_bank0_atc->set_bank(0);
				m_bank1_atc->set_bank(0);
			}
			else
			{
				m_bank0_atc->set_bank(1);
				m_bank1_atc->set_bank(1);
			}
			break;

		case 0x36:  // SPEED
			m_speed = data;

			if (m_speed & SPEED_ALLBANKS)
			{
				logerror("apple2gs: Driver does not support shadowing in all banks\n");
			}
			update_speed();
			break;

		case 0x38:  // SCCBREG
			m_scc->cb_w(0, data);
			break;

		case 0x39:  // SCCAREG
			m_scc->ca_w(0, data);
			break;

		case 0x3a:  // SCCBDATA
			m_scc->db_w(0, data);
			break;

		case 0x3b:  // SCCADATA
			m_scc->da_w(0, data);
			break;

		case 0x3c:  // SOUNDCTL
			m_sndglu_ctrl = data & 0x7f; // make sure DOC is never busy
			if (!(m_sndglu_ctrl & 0x40)) // clear hi byte of address pointer on DOC access
			{
				m_sndglu_addr &= 0xff;
			}
			break;

		case 0x3d:  // SOUNDDATA
			if (m_sndglu_ctrl & 0x40)    // docram access
			{
				m_docram[m_sndglu_addr] = data;
			}
			else
			{
				m_doc->write(space, m_sndglu_addr, data);
			}

			if (m_sndglu_ctrl & 0x20)    // auto-increment
			{
				m_sndglu_addr++;
			}
			break;

		case 0x3e:  // SOUNDADRL
			m_sndglu_addr &= 0xff00;
			m_sndglu_addr |= data;
			break;

		case 0x3f:  // SOUNDADRH
			m_sndglu_addr &= 0x00ff;
			m_sndglu_addr |= data<<8;
			break;

		case 0x41:  // INTEN
			m_inten = data & 0x1f;
			if (!(data & 0x10))
			{
				lower_irq(IRQS_QTRSEC);
			}
			if (!(data & 0x08))
			{
				lower_irq(IRQS_VBL);
			}
			//printf("%02x to INTEN, now %02x\n", data, m_vgcint);
			break;

		case 0x47:  // CLRVBLINT
			m_intflag &= ~INTFLAG_VBL;
			lower_irq(IRQS_VBL);
			lower_irq(IRQS_QTRSEC);
			break;

		case 0x68:  // STATEREG
			m_altzp = (data & 0x80);
			m_page2 = (data & 0x40);
			m_ramrd = (data & 0x20);
			m_ramwrt = (data & 0x10);
			m_lcram = (data & 0x08) ? false : true;
			m_lcram2 = (data & 0x04);
			m_intcxrom = (data & 0x01);

			// update the aux state
			auxbank_update();

			// update LC state
			if (m_lcram)
			{
				m_lcbank->set_bank(1);
				m_lcaux->set_bank(1);
				m_lc00->set_bank(1);
				m_lc01->set_bank(1);
			}
			else
			{
				m_lcbank->set_bank(0);
				m_lcaux->set_bank(0);
				m_lc00->set_bank(0);
				m_lc01->set_bank(0);
			}
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d:
			do_io(space, offset);    // make sure it also side-effect resets the paddles as documented
			break;

		case 0x7e:  // SETIOUDIS
			m_ioudis = true; break;

		case 0x7f:  // CLRIOUDIS
			m_ioudis = false; break;

		default:
			do_io(space, offset);
			break;
	}
}

READ8_MEMBER(apple2gs_state::c080_r)
{
	if (!machine().side_effects_disabled())
	{
		int slot;

		slow_cycle();

		offset &= 0x7F;
		slot = offset / 0x10;

		if (slot == 0)
		{
			lc_update(offset & 0xf, false);
		}
		else
		{
			if (slot >= 4)
			{
				if ((offset & 0xf) == 0x9)
				{
					m_motors_active |= (1 << (slot - 4));
				}
				else if ((offset & 0xf) == 8)
				{
					m_motors_active &= ~(1 << (slot - 4));
				}

				update_speed();
			}

			// slot 3 always has I/O go to the external card
			if ((slot != 3) && ((m_slotromsel & (1 << slot)) == 0))
			{
				if (slot == 6)
				{
					return m_iwm->read(offset & 0xf);
				}
			}
			else
			{
				if (m_slotdevice[slot] != nullptr)
				{
					return m_slotdevice[slot]->read_c0nx(offset % 0x10);
				}
			}
		}
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2gs_state::c080_w)
{
	int slot;

	slow_cycle();

	offset &= 0x7F;
	slot = offset / 0x10;

	if (slot == 0)
	{
		lc_update(offset & 0xf, true);
	}
	else
	{
		if (slot >= 4)
		{
			if ((offset & 0xf) == 0x9)
			{
				m_motors_active |= (1 << (slot - 4));
			}
			else if ((offset & 0xf) == 8)
			{
				m_motors_active &= ~(1 << (slot - 4));
			}

			update_speed();
		}

		// slot 3 always has I/O go to the external card
		if ((slot != 3) && ((m_slotromsel & (1 << slot)) == 0))
		{
			if (slot == 6)
			{
				m_iwm->write(offset & 0xf, data);
			}
		}
		else
		{
			if (m_slotdevice[slot] != nullptr)
			{
				m_slotdevice[slot]->write_c0nx(offset % 0x10, data);
			}
		}
	}
}

uint8_t apple2gs_state::read_slot_rom(address_space &space, int slotbias, int offset)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

//  printf("read_slot_rom: sl %d offs %x, cnxx_slot %d\n", slotnum, offset, m_cnxx_slot);

	if (m_slotdevice[slotnum] != nullptr)
	{
//      printf("slotdevice is not null\n");
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		return m_slotdevice[slotnum]->read_cnxx(offset&0xff);
	}

	return read_floatingbus();
}

void apple2gs_state::write_slot_rom(address_space &space, int slotbias, int offset, uint8_t data)
{
	int slotnum = ((offset>>8) & 0xf) + slotbias;

	slow_cycle();

	if (m_slotdevice[slotnum] != nullptr)
	{
		if ((m_cnxx_slot == CNXX_UNCLAIMED) && (m_slotdevice[slotnum]->take_c800()) && (!machine().side_effects_disabled()))
		{
			m_cnxx_slot = slotnum;
			update_slotrom_banks();
		}

		m_slotdevice[slotnum]->write_cnxx(offset&0xff, data);
	}
}

uint8_t apple2gs_state::read_int_rom(address_space &space, int slotbias, int offset)
{
	if ((m_cnxx_slot == CNXX_UNCLAIMED) && (!machine().side_effects_disabled()))
	{
		m_cnxx_slot = CNXX_INTROM;
		update_slotrom_banks();
	}

	return m_rom[slotbias + offset];
}

READ8_MEMBER(apple2gs_state::c100_r)
{
	int slot = ((offset>>8) & 0xf) + 1;

	slow_cycle();

	// SETSLOTCXROM is disabled, so the $C02D SLOT register controls what's in each slot
	if (!(m_slotromsel & (1 << slot)))
	{
		return read_int_rom(space, 0x3c100, offset);
	}

	return read_slot_rom(space, 1, offset);
}

WRITE8_MEMBER(apple2gs_state::c100_w)
{
	int slot = ((offset>>8) & 0xf) + 1;

	slow_cycle();

	if ((m_slotromsel & (1 << slot)))
	{
		write_slot_rom(space, 1, offset, data);
	}
}

READ8_MEMBER(apple2gs_state::c100_int_r)  { slow_cycle(); return read_int_rom(space, 0x3c100, offset); }
READ8_MEMBER(apple2gs_state::c300_int_r)  { slow_cycle(); return read_int_rom(space, 0x3c300, offset); }
READ8_MEMBER(apple2gs_state::c400_int_r)  { slow_cycle(); return read_int_rom(space, 0x3c400, offset); }
READ8_MEMBER(apple2gs_state::c300_r)  { slow_cycle(); return read_slot_rom(space, 3, offset); }
WRITE8_MEMBER(apple2gs_state::c300_w) { slow_cycle(); write_slot_rom(space, 3, offset, data); }

READ8_MEMBER(apple2gs_state::c400_r)
{
	int slot = ((offset>>8) & 0xf) + 4;

	slow_cycle();

	if (!(m_slotromsel & (1 << slot)))
	{
		return read_int_rom(space, 0x3c400, offset);
	}

	return read_slot_rom(space, 4, offset);
}

WRITE8_MEMBER(apple2gs_state::c400_w)
{
	int slot = ((offset>>8) & 0xf) + 1;

	slow_cycle();

	if ((m_slotromsel & (1 << slot)))
	{
		write_slot_rom(space, 4, offset, data);
	}
}

READ8_MEMBER(apple2gs_state::c800_r)
{
	slow_cycle();

	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return 0xff;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
	}

	return read_floatingbus();
}

READ8_MEMBER(apple2gs_state::c800_int_r)
{
	slow_cycle();

	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return m_rom[offset + 0x3c800];
	}

	if (m_cnxx_slot == CNXX_INTROM)
	{
		return m_rom[offset + 0x3c800];
	}

	return read_floatingbus();
}

WRITE8_MEMBER(apple2gs_state::c800_w)
{
	slow_cycle();

	if (offset == 0x7ff)
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return;
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(offset&0xfff, data);
	}
}

READ8_MEMBER(apple2gs_state::inh_r)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

WRITE8_MEMBER(apple2gs_state::inh_w)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(offset + 0xd000, data);
	}
}

READ8_MEMBER(apple2gs_state::lc_r)
{
	slow_cycle();
	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			return m_ram_ptr[(offset & 0xfff) + 0xc000];
		}
		else
		{
			return m_ram_ptr[(offset & 0xfff) + 0xd000];
		}
	}

	return m_ram_ptr[(offset & 0x1fff) + 0xe000];
}

WRITE8_MEMBER(apple2gs_state::lc_w)
{
	slow_cycle();
	if (!m_lcwriteenable)
	{
		return;
	}

	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			m_ram_ptr[(offset & 0xfff) + 0xc000] = data;
		}
		else
		{
			m_ram_ptr[(offset & 0xfff) + 0xd000] = data;
		}
		return;
	}

	m_ram_ptr[(offset & 0x1fff) + 0xe000] = data;
}

READ8_MEMBER(apple2gs_state::lc_aux_r)
{
	slow_cycle();
	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			return m_ram_ptr[(offset & 0xfff) + 0x1c000];
		}
		else
		{
			return m_ram_ptr[(offset & 0xfff) + 0x1d000];
		}
	}

	return m_ram_ptr[(offset & 0x1fff) + 0x1e000];
}

WRITE8_MEMBER(apple2gs_state::lc_aux_w)
{
	slow_cycle();
	if (!m_lcwriteenable)
	{
		return;
	}

	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			m_ram_ptr[(offset & 0xfff) + 0x1c000] = data;
		}
		else
		{
			m_ram_ptr[(offset & 0xfff) + 0x1d000] = data;
		}
		return;
	}

	m_ram_ptr[(offset & 0x1fff) + 0x1e000] = data;
}

READ8_MEMBER(apple2gs_state::lc_00_r)
{
	if (m_altzp)
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				return m_ram_ptr[(offset & 0xfff) + 0x3c000];
			}
			else
			{
				return m_ram_ptr[(offset & 0xfff) + 0x3d000];
			}
		}

		return m_ram_ptr[(offset & 0x1fff) + 0x3e000];
	}
	else
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				return m_ram_ptr[(offset & 0xfff) + 0x2c000];
			}
			else
			{
				return m_ram_ptr[(offset & 0xfff) + 0x2d000];
			}
		}

		return m_ram_ptr[(offset & 0x1fff) + 0x2e000];
	}
}

WRITE8_MEMBER(apple2gs_state::lc_00_w)
{
	if (!m_lcwriteenable)
	{
		return;
	}

	if (m_altzp)
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				m_ram_ptr[(offset & 0xfff) + 0x3c000] = data;
			}
			else
			{
				m_ram_ptr[(offset & 0xfff) + 0x3d000] = data;
			}
			return;
		}

		m_ram_ptr[(offset & 0x1fff) + 0x3e000] = data;
	}
	else
	{
		if (offset < 0x1000)
		{
			if (m_lcram2)
			{
				m_ram_ptr[(offset & 0xfff) + 0x2c000] = data;
			}
			else
			{
				m_ram_ptr[(offset & 0xfff) + 0x2d000] = data;
			}
			return;
		}

		m_ram_ptr[(offset & 0x1fff) + 0x2e000] = data;
	}
}

READ8_MEMBER(apple2gs_state::lc_01_r)
{
	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			return m_ram_ptr[(offset & 0xfff) + 0x3c000];
		}
		else
		{
			return m_ram_ptr[(offset & 0xfff) + 0x3d000];
		}
	}

	return m_ram_ptr[(offset & 0x1fff) + 0x3e000];
}

WRITE8_MEMBER(apple2gs_state::lc_01_w)
{
	if (!m_lcwriteenable)
	{
		return;
	}

	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			m_ram_ptr[(offset & 0xfff) + 0x3c000] = data;
		}
		else
		{
			m_ram_ptr[(offset & 0xfff) + 0x3d000] = data;
		}
		return;
	}

	m_ram_ptr[(offset & 0x1fff) + 0x3e000] = data;
}

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
uint8_t apple2gs_state::read_floatingbus()
{
	enum
	{
		// scanner types
		kScannerNone = 0, kScannerApple2, kScannerApple2e,

		// scanner constants
		kHBurstClock      =    53, // clock when Color Burst starts
		kHBurstClocks     =     4, // clocks per Color Burst duration
		kHClock0State     =  0x18, // H[543210] = 011000
		kHClocks          =    65, // clocks per horizontal scan (including HBL)
		kHPEClock         =    40, // clock when HPE (horizontal preset enable) goes low
		kHPresetClock     =    41, // clock when H state presets
		kHSyncClock       =    49, // clock when HSync starts
		kHSyncClocks      =     4, // clocks per HSync duration
		kNTSCScanLines    =   262, // total scan lines including VBL (NTSC)
		kNTSCVSyncLine    =   224, // line when VSync starts (NTSC)
		kPALScanLines     =   312, // total scan lines including VBL (PAL)
		kPALVSyncLine     =   264, // line when VSync starts (PAL)
		kVLine0State      = 0x100, // V[543210CBA] = 100000000
		kVPresetLine      =   256, // line when V state presets
		kVSyncLines       =     4, // lines per VSync duration
		kClocksPerVSync   = kHClocks * kNTSCScanLines // FIX: NTSC only?
	};

	// vars
	//
	int i, Hires, Mixed, Page2, _80Store, ScanLines, /* VSyncLine, ScanCycles,*/
		h_clock, h_state, h_0, h_1, h_2, h_3, h_4, h_5,
		v_line, v_state, v_A, v_B, v_C, v_0, v_1, v_2, v_3, v_4, /* v_5, */
		_hires, addend0, addend1, addend2, sum, address;

	// video scanner data
	//
	i = m_maincpu->total_cycles() % kClocksPerVSync; // cycles into this VSync

	// machine state switches
	//
	Hires    = (m_video->m_hires && m_video->m_graphics) ? 1 : 0;
	Mixed    = m_video->m_mix ? 1 : 0;
	Page2    = m_page2 ? 1 : 0;
	_80Store = m_80store ? 1 : 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	//
	h_clock = (i + kHPEClock) % kHClocks; // which horizontal scanning clock
	h_state = kHClock0State + h_clock; // H state bits
	if (h_clock >= kHPresetClock) // check for horizontal preset
	{
		h_state -= 1; // correct for state preset (two 0 states)
	}
	h_0 = (h_state >> 0) & 1; // get horizontal state bits
	h_1 = (h_state >> 1) & 1;
	h_2 = (h_state >> 2) & 1;
	h_3 = (h_state >> 3) & 1;
	h_4 = (h_state >> 4) & 1;
	h_5 = (h_state >> 5) & 1;

	// calculate vertical scanning state
	//
	v_line  = i / kHClocks; // which vertical scanning line
	v_state = kVLine0State + v_line; // V state bits
	if ((v_line >= kVPresetLine)) // check for previous vertical state preset
	{
		v_state -= ScanLines; // compensate for preset
	}
	v_A = (v_state >> 0) & 1; // get vertical state bits
	v_B = (v_state >> 1) & 1;
	v_C = (v_state >> 2) & 1;
	v_0 = (v_state >> 3) & 1;
	v_1 = (v_state >> 4) & 1;
	v_2 = (v_state >> 5) & 1;
	v_3 = (v_state >> 6) & 1;
	v_4 = (v_state >> 7) & 1;
	//v_5 = (v_state >> 8) & 1;

	// calculate scanning memory address
	//
	_hires = Hires;
	if (Hires && Mixed && (v_4 & v_2))
	{
		_hires = 0; // (address is in text memory)
	}

	addend0 = 0x68; // 1            1            0            1
	addend1 =              (h_5 << 5) | (h_4 << 4) | (h_3 << 3);
	addend2 = (v_4 << 6) | (v_3 << 5) | (v_4 << 4) | (v_3 << 3);
	sum     = (addend0 + addend1 + addend2) & (0x0F << 3);

	address = 0;
	address |= h_0 << 0; // a0
	address |= h_1 << 1; // a1
	address |= h_2 << 2; // a2
	address |= sum;      // a3 - aa6
	address |= v_0 << 7; // a7
	address |= v_1 << 8; // a8
	address |= v_2 << 9; // a9
	address |= ((_hires) ? v_A : (1 ^ (Page2 & (1 ^ _80Store)))) << 10; // a10
	address |= ((_hires) ? v_B : (Page2 & (1 ^ _80Store))) << 11; // a11
	if (_hires) // hires?
	{
		// Y: insert hires only address bits
		//
		address |= v_C << 12; // a12
		address |= (1 ^ (Page2 & (1 ^ _80Store))) << 13; // a13
		address |= (Page2 & (1 ^ _80Store)) << 14; // a14
	}
	else
	{
		// N: text, so no higher address bits unless Apple ][, not Apple //e
		//
		if ((1) && // Apple ][? // FIX: check for Apple ][? (FB is most useful in old games)
			(kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	return m_ram_ptr[address % m_ram_size]; // FIX: this seems to work, but is it right!?
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

READ8_MEMBER(apple2gs_state::ram0000_r)  { slow_cycle(); return m_ram_ptr[offset]; }
WRITE8_MEMBER(apple2gs_state::ram0000_w) { slow_cycle(); m_ram_ptr[offset] = data; }
READ8_MEMBER(apple2gs_state::auxram0000_r)  { slow_cycle(); return m_ram_ptr[offset+0x10000]; }
WRITE8_MEMBER(apple2gs_state::auxram0000_w)
{
	slow_cycle();
	m_ram_ptr[offset+0x10000] = data;

	if ((offset >= 0x9e00) && (offset <= 0x9fff))
	{
		int color = (offset - 0x9e00) >> 1;

		m_video->m_shr_palette[color] = rgb_t(
			((m_ram_ptr[0x19E00 + (color * 2) + 1] >> 0) & 0x0F) * 17,
			((m_ram_ptr[0x19E00 + (color * 2) + 0] >> 4) & 0x0F) * 17,
			((m_ram_ptr[0x19E00 + (color * 2) + 0] >> 0) & 0x0F) * 17);
	}
}

READ8_MEMBER( apple2gs_state::b0ram0000_r)  { return m_ram_ptr[offset+0x20000]; }
WRITE8_MEMBER(apple2gs_state::b0ram0000_w) { m_ram_ptr[offset+0x20000] = data; }
READ8_MEMBER( apple2gs_state::b0ram0200_r)  { return m_ram_ptr[offset+0x20200]; }
WRITE8_MEMBER(apple2gs_state::b0ram0200_w) { m_ram_ptr[offset+0x20200] = data; }
READ8_MEMBER( apple2gs_state::b0ram0400_r)  { return m_ram_ptr[offset+0x20400]; }
WRITE8_MEMBER(apple2gs_state::b0ram0400_w)
{
	m_ram_ptr[offset+0x20400] = data;
	if (!(m_shadow & SHAD_TXTPG1))
	{
		slow_cycle();
		m_ram_ptr[offset+0x0400] = data;
	}
}
READ8_MEMBER( apple2gs_state::b0ram0800_r)  { return m_ram_ptr[offset+0x20800]; }
WRITE8_MEMBER(apple2gs_state::b0ram0800_w)
{
	m_ram_ptr[offset+0x20800] = data;

	if (offset < 0x400) // TODO: ROM 03
	{
		if ((!(m_shadow & SHAD_TXTPG2)) && (m_is_rom3))
		{
			slow_cycle();
			m_ram_ptr[offset+0x800] = data;
		}
	}
}
READ8_MEMBER( apple2gs_state::b0ram2000_r)  { return m_ram_ptr[offset+0x22000]; }
WRITE8_MEMBER(apple2gs_state::b0ram2000_w)
{
	m_ram_ptr[offset+0x22000] = data;
	if (!(m_shadow & SHAD_HIRESPG1))
	{
		slow_cycle();
		m_ram_ptr[offset+0x2000] = data;
	}
}
READ8_MEMBER( apple2gs_state::b0ram4000_r)  { return m_ram_ptr[offset+0x24000]; }
WRITE8_MEMBER(apple2gs_state::b0ram4000_w)
{
	m_ram_ptr[offset+0x24000] = data;
	if (offset < 0x2000)
	{
		if (!(m_shadow & SHAD_HIRESPG2))
		{
			slow_cycle();
			m_ram_ptr[offset+0x4000] = data;
		}
	}
}

READ8_MEMBER( apple2gs_state::b1ram0000_r)  { return m_ram_ptr[offset+0x30000]; }
WRITE8_MEMBER(apple2gs_state::b1ram0000_w) { m_ram_ptr[offset+0x30000] = data; }
READ8_MEMBER( apple2gs_state::b1ram0200_r)  { return m_ram_ptr[offset+0x30200]; }
WRITE8_MEMBER(apple2gs_state::b1ram0200_w) { m_ram_ptr[offset+0x30200] = data; }
READ8_MEMBER( apple2gs_state::b1ram0400_r)  { return m_ram_ptr[offset+0x30400]; }
WRITE8_MEMBER(apple2gs_state::b1ram0400_w)
{
	m_ram_ptr[offset+0x30400] = data;
	if (!(m_shadow & SHAD_TXTPG1))
	{
		slow_cycle();
		m_ram_ptr[offset+0x10400] = data;
	}
}
READ8_MEMBER( apple2gs_state::b1ram0800_r) { return m_ram_ptr[offset+0x30800]; }
WRITE8_MEMBER(apple2gs_state::b1ram0800_w)
{
	m_ram_ptr[offset+0x30800] = data;
	if (offset < 0x400)
	{
		slow_cycle();
		m_ram_ptr[offset+0x10800] = data;
	}
}
READ8_MEMBER( apple2gs_state::b1ram2000_r)  { return m_ram_ptr[offset+0x32000]; }
WRITE8_MEMBER(apple2gs_state::b1ram2000_w)
{
	m_ram_ptr[offset+0x32000] = data;
	if ((!(m_shadow & SHAD_HIRESPG1) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
	{
		auxram0000_w(space, offset+0x2000, data);
	}
}
READ8_MEMBER( apple2gs_state::b1ram4000_r)  { return m_ram_ptr[offset+0x34000]; }
WRITE8_MEMBER(apple2gs_state::b1ram4000_w)
{
	m_ram_ptr[offset+0x34000] = data;
	if (offset < 0x2000)
	{
		if (!(m_shadow & SHAD_HIRESPG2) && !(m_shadow & SHAD_AUXHIRES))
		{
			auxram0000_w(space, offset+0x4000, data);
		}
	}
	else if ((offset >= 0x2000) && (offset <= 0x5fff))
	{
		if (!(m_shadow & SHAD_SUPERHIRES))
		{
			auxram0000_w(space, offset+0x4000, data);
		}
	}
}

READ8_MEMBER(apple2gs_state::bank0_c000_r)
{
	if (m_ramrd)
	{
		return m_ram_ptr[offset + 0x3c000];
	}

	return m_ram_ptr[offset + 0x2c000];
}

WRITE8_MEMBER(apple2gs_state::bank0_c000_w)
{
	if (m_ramwrt)
	{
		m_ram_ptr[offset + 0x3c000] = data;
		return;
	}

	m_ram_ptr[offset + 0x2c000] = data;
}

READ8_MEMBER(apple2gs_state::bank1_0000_r) { return m_ram_ptr[offset + 0x30000]; }
WRITE8_MEMBER(apple2gs_state::bank1_0000_w) { m_ram_ptr[offset + 0x30000] = data; }
READ8_MEMBER(apple2gs_state::bank1_c000_r) { return m_ram_ptr[offset + 0x3c000]; }
WRITE8_MEMBER(apple2gs_state::bank1_c000_w) { m_ram_ptr[offset + 0x3c000] = data; }
WRITE8_MEMBER(apple2gs_state::bank1_0000_sh_w)
{
	m_ram_ptr[offset + 0x30000] = data;

	switch (offset>>8)
	{
		case 0x04:  // text page 1
		case 0x05:
		case 0x06:
		case 0x07:
			if (!(m_shadow & SHAD_TXTPG1))
			{
				slow_cycle();
				m_ram_ptr[offset + 0x10000] = data;
			}
			break;

		case 0x08:  // text page 2 (only shadowable on ROM 03)
		case 0x09:
		case 0x0a:
		case 0x0b:
			if ((!(m_shadow & SHAD_TXTPG2)) && (m_is_rom3))
			{
				slow_cycle();
				m_ram_ptr[offset + 0x10000] = data;
			}
			break;

			// hi-res page 1
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			if ((!(m_shadow & SHAD_HIRESPG1) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
			{
				slow_cycle();
				m_ram_ptr[offset + 0x10000] = data;
			}
			break;

			// hi-res page 2
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			if ((!(m_shadow & SHAD_HIRESPG2) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
			{
				slow_cycle();
				m_ram_ptr[offset + 0x10000] = data;
			}
			break;

		default:
			if ((offset >= 0x6000) && (offset <= 0x9fff))
			{
				if (!(m_shadow & SHAD_SUPERHIRES))
				{
					auxram0000_w(space, offset, data);
				}
			}
			break;
	}
}

void apple2gs_state::apple2gs_map(address_map &map)
{
	/* "fast side" - runs 2.8 MHz minus RAM refresh, banks 00 and 01 usually have writes shadowed to E0/E1 where I/O lives */
	/* Banks 00 and 01 also have their own independent language cards if shadowing is disabled */
	map(0x000000, 0x0001ff).m(m_b0_0000bank, FUNC(address_map_bank_device::amap8));
	map(0x000200, 0x0003ff).m(m_b0_0200bank, FUNC(address_map_bank_device::amap8));
	map(0x000400, 0x0007ff).m(m_b0_0400bank, FUNC(address_map_bank_device::amap8));
	map(0x000800, 0x001fff).m(m_b0_0800bank, FUNC(address_map_bank_device::amap8));
	map(0x002000, 0x003fff).m(m_b0_2000bank, FUNC(address_map_bank_device::amap8));
	map(0x004000, 0x00bfff).m(m_b0_4000bank, FUNC(address_map_bank_device::amap8));
	map(0x00c000, 0x00ffff).m(m_bank0_atc, FUNC(address_map_bank_device::amap8));
	map(0x010000, 0x01bfff).m(m_bank1_at0, FUNC(address_map_bank_device::amap8));
	map(0x01c000, 0x01ffff).m(m_bank1_atc, FUNC(address_map_bank_device::amap8));

	/* "Mega II side" - this is basically a 128K IIe on a chip that runs merrily at 1 MHz */
	/* Unfortunately all I/O happens here, including new IIgs-specific stuff */
	map(0xe00000, 0xe0bfff).rw(FUNC(apple2gs_state::ram0000_r), FUNC(apple2gs_state::ram0000_w));
	map(0xe0c000, 0xe0c07f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0xe0c080, 0xe0c0ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0xe0c100, 0xe0c2ff).m(m_c100bank, FUNC(address_map_bank_device::amap8));
	map(0xe0c300, 0xe0c3ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0xe0c400, 0xe0c7ff).m(m_c400bank, FUNC(address_map_bank_device::amap8));
	map(0xe0c800, 0xe0cfff).m(m_c800bank, FUNC(address_map_bank_device::amap8));
	map(0xe0d000, 0xe0ffff).m(A2GS_UPPERBANK_TAG, FUNC(address_map_bank_device::amap8));

	map(0xe10000, 0xe1bfff).rw(FUNC(apple2gs_state::auxram0000_r), FUNC(apple2gs_state::auxram0000_w));
	map(0xe1c000, 0xe1c07f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0xe1c080, 0xe1c0ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0xe1c100, 0xe1c2ff).m(m_c100bank, FUNC(address_map_bank_device::amap8));
	map(0xe1c300, 0xe1c3ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0xe1c400, 0xe1c7ff).m(m_c400bank, FUNC(address_map_bank_device::amap8));
	map(0xe1c800, 0xe1cfff).m(m_c800bank, FUNC(address_map_bank_device::amap8));
	map(0xe1d000, 0xe1ffff).m(m_upperaux, FUNC(address_map_bank_device::amap8));

	map(0xfc0000, 0xffffff).rom().region("maincpu", 0x00000);
}

void apple2gs_state::vectors_map(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(apple2gs_state::apple2gs_read_vector));
}

void apple2gs_state::c100bank_map(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(apple2gs_state::c100_r), FUNC(apple2gs_state::c100_w));
	map(0x0200, 0x03ff).r(FUNC(apple2gs_state::c100_int_r)).nopw();
}

void apple2gs_state::c300bank_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(apple2gs_state::c300_r), FUNC(apple2gs_state::c300_w));
	map(0x0100, 0x01ff).r(FUNC(apple2gs_state::c300_int_r)).nopw();
}

void apple2gs_state::c400bank_map(address_map &map)
{
	map(0x0000, 0x03ff).rw(FUNC(apple2gs_state::c400_r), FUNC(apple2gs_state::c400_w));
	map(0x0400, 0x07ff).rw(FUNC(apple2gs_state::c400_int_r), FUNC(apple2gs_state::c400_w));
}

void apple2gs_state::c800bank_map(address_map &map)
{
	map(0x0000, 0x07ff).rw(FUNC(apple2gs_state::c800_r), FUNC(apple2gs_state::c800_w));
	map(0x0800, 0x0fff).rw(FUNC(apple2gs_state::c800_int_r), FUNC(apple2gs_state::c800_w));
}

void apple2gs_state::inhbank_map(address_map &map)
{
	map(0x0000, 0x2fff).m(m_lcbank, FUNC(address_map_bank_device::amap8));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));
}

void apple2gs_state::inhaux_map(address_map &map)
{
	map(0x0000, 0x2fff).m(m_lcaux, FUNC(address_map_bank_device::amap8));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));
}

void apple2gs_state::inh00_map(address_map &map)
{
	map(0x0000, 0x2fff).m(m_lc00, FUNC(address_map_bank_device::amap8));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));
}

void apple2gs_state::inh01_map(address_map &map)
{
	map(0x0000, 0x2fff).m(m_lc01, FUNC(address_map_bank_device::amap8));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));
}

void apple2gs_state::lcbank_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_w));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::lc_r), FUNC(apple2gs_state::lc_w));
}

void apple2gs_state::lcaux_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_aux_w));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::lc_aux_r), FUNC(apple2gs_state::lc_aux_w));
}

void apple2gs_state::lc00_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_00_w));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::lc_00_r), FUNC(apple2gs_state::lc_00_w));
}

void apple2gs_state::lc01_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_01_w));
	map(0x3000, 0x5fff).rw(FUNC(apple2gs_state::lc_01_r), FUNC(apple2gs_state::lc_01_w));
}

void apple2gs_state::bank0_iolc_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(apple2gs_state::bank0_c000_r), FUNC(apple2gs_state::bank0_c000_w));
	map(0x4000, 0x407f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0x4080, 0x40ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0x4100, 0x42ff).m(m_c100bank, FUNC(address_map_bank_device::amap8));
	map(0x4300, 0x43ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0x4400, 0x47ff).m(m_c400bank, FUNC(address_map_bank_device::amap8));
	map(0x4800, 0x4fff).m(m_c800bank, FUNC(address_map_bank_device::amap8));
	map(0x5000, 0x7fff).m(m_upper00,  FUNC(address_map_bank_device::amap8));
}

void apple2gs_state::bank1_lower48_map(address_map &map)
{
	map(0x0000, 0x0bfff).rw(FUNC(apple2gs_state::bank1_0000_r), FUNC(apple2gs_state::bank1_0000_w));
	map(0xc000, 0x17fff).rw(FUNC(apple2gs_state::bank1_0000_r), FUNC(apple2gs_state::bank1_0000_sh_w));
}

void apple2gs_state::bank1_iolc_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(apple2gs_state::bank1_c000_r), FUNC(apple2gs_state::bank1_c000_w));
	map(0x4000, 0x407f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0x4080, 0x40ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0x4100, 0x42ff).m(m_c100bank, FUNC(address_map_bank_device::amap8));
	map(0x4300, 0x43ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0x4400, 0x47ff).m(m_c400bank, FUNC(address_map_bank_device::amap8));
	map(0x4800, 0x4fff).m(m_c800bank, FUNC(address_map_bank_device::amap8));
	map(0x5000, 0x7fff).m(m_upper01,  FUNC(address_map_bank_device::amap8));
}

void apple2gs_state::rb0000bank_map(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(apple2gs_state::b0ram0000_r), FUNC(apple2gs_state::b0ram0000_w));
	map(0x0200, 0x03ff).rw(FUNC(apple2gs_state::b1ram0000_r), FUNC(apple2gs_state::b1ram0000_w));
}

void apple2gs_state::rb0200bank_map(address_map &map)
{
	map(0x0000, 0x01ff).rw(FUNC(apple2gs_state::b0ram0200_r), FUNC(apple2gs_state::b0ram0200_w)); // wr 0 rd 0
	map(0x0200, 0x03ff).rw(FUNC(apple2gs_state::b1ram0200_r), FUNC(apple2gs_state::b0ram0200_w)); // wr 0 rd 1
	map(0x0400, 0x05ff).rw(FUNC(apple2gs_state::b0ram0200_r), FUNC(apple2gs_state::b1ram0200_w)); // wr 1 rd 0
	map(0x0600, 0x07ff).rw(FUNC(apple2gs_state::b1ram0200_r), FUNC(apple2gs_state::b1ram0200_w)); // wr 1 rd 1
}

void apple2gs_state::rb0400bank_map(address_map &map)
{
	map(0x0000, 0x03ff).rw(FUNC(apple2gs_state::b0ram0400_r), FUNC(apple2gs_state::b0ram0400_w)); // wr 0 rd 0
	map(0x0400, 0x07ff).rw(FUNC(apple2gs_state::b1ram0400_r), FUNC(apple2gs_state::b0ram0400_w));  // wr 0 rd 1
	map(0x0800, 0x0bff).rw(FUNC(apple2gs_state::b0ram0400_r), FUNC(apple2gs_state::b1ram0400_w));  // wr 1 rd 0
	map(0x0c00, 0x0fff).rw(FUNC(apple2gs_state::b1ram0400_r), FUNC(apple2gs_state::b1ram0400_w)); // wr 1 rd 1
}

void apple2gs_state::rb0800bank_map(address_map &map)
{
	map(0x0000, 0x17ff).rw(FUNC(apple2gs_state::b0ram0800_r), FUNC(apple2gs_state::b0ram0800_w));
	map(0x2000, 0x37ff).rw(FUNC(apple2gs_state::b1ram0800_r), FUNC(apple2gs_state::b0ram0800_w));
	map(0x4000, 0x57ff).rw(FUNC(apple2gs_state::b0ram0800_r), FUNC(apple2gs_state::b1ram0800_w));
	map(0x6000, 0x77ff).rw(FUNC(apple2gs_state::b1ram0800_r), FUNC(apple2gs_state::b1ram0800_w));
}

void apple2gs_state::rb2000bank_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(apple2gs_state::b0ram2000_r), FUNC(apple2gs_state::b0ram2000_w));
	map(0x2000, 0x3fff).rw(FUNC(apple2gs_state::b1ram2000_r), FUNC(apple2gs_state::b0ram2000_w));
	map(0x4000, 0x5fff).rw(FUNC(apple2gs_state::b0ram2000_r), FUNC(apple2gs_state::b1ram2000_w));
	map(0x6000, 0x7fff).rw(FUNC(apple2gs_state::b1ram2000_r), FUNC(apple2gs_state::b1ram2000_w));
}

void apple2gs_state::rb4000bank_map(address_map &map)
{
	map(0x00000, 0x07fff).rw(FUNC(apple2gs_state::b0ram4000_r), FUNC(apple2gs_state::b0ram4000_w));
	map(0x08000, 0x0ffff).rw(FUNC(apple2gs_state::b1ram4000_r), FUNC(apple2gs_state::b0ram4000_w));
	map(0x10000, 0x17fff).rw(FUNC(apple2gs_state::b0ram4000_r), FUNC(apple2gs_state::b1ram4000_w));
	map(0x18000, 0x1ffff).rw(FUNC(apple2gs_state::b1ram4000_r), FUNC(apple2gs_state::b1ram4000_w));
}

void apple2gs_state::a2gs_es5503_map(address_map &map)
{
	map(0x00000, 0x0ffff).mirror(0x10000).readonly().share("docram"); // IIgs only has 64K, top bank mirrors lower bank
}

/***************************************************************************
    ADB microcontroller + KEYGLU emulation

    Huge thanks to Neil Parker's writeup on the ADB microcontroller!
    http://www.llx.com/~nparker/a2/adb.html
***************************************************************************/

READ8_MEMBER(apple2gs_state::adbmicro_p0_in)
{
	return m_glu_bus;
}

READ8_MEMBER(apple2gs_state::adbmicro_p1_in)
{
#if RUN_ADB_MICRO
	switch (m_glu_kbd_y)
	{
		case 0:
			return m_ky0->read();
		case 1:
			return m_ky1->read();
		case 2:
			return m_ky2->read();
		case 3:
			return m_ky3->read();
		case 4:
			return m_ky4->read();
		case 5:
			return m_ky5->read();
		case 6:
			return m_ky6->read();
		case 7:
			return m_ky7->read();
		case 8:
			return m_ky8->read();
		case 9:
			return m_ky9->read();
	}
#endif
	return 0xff;
}

READ8_MEMBER(apple2gs_state::adbmicro_p2_in)
{
	uint8_t rv = 0;

	rv |= 0x40;     // no reset
	rv |= (m_adb_line) ? 0x00 : 0x80;

	return rv;
}

READ8_MEMBER(apple2gs_state::adbmicro_p3_in)
{
	uint8_t rv = 0;
#if RUN_ADB_MICRO
	uint8_t special = m_kbspecial->read();

	rv |= (special & 0x06) ? 0x00 : 0x01;
	rv |= (special & 0x08) ? 0x00 : 0x02;
	rv |= (special & 0x01) ? 0x00 : 0x04;
	rv |= (special & 0x10) ? 0x00 : 0x80;
	rv |= (special & 0x20) ? 0x00 : 0x40;
#else
	rv = 0xc7;
#endif
	return rv;
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p0_out)
{
	m_glu_bus = data;
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p1_out)
{
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p2_out)
{
	if (!(data & 0x10))
	{
		if (m_adbmicro->are_port_bits_output(0, 0xff))
		{
			keyglu_mcu_write(data & 7, m_glu_bus);
		}
		else    // read GLU
		{
			m_glu_bus = keyglu_mcu_read(data & 7);
		}
	}
	else
	{
		m_glu_kbd_y = data & 0xf;
	}
}

WRITE8_MEMBER(apple2gs_state::adbmicro_p3_out)
{
	if (((data & 0x08) == 0x08) != m_adb_line)
	{
//      m_adb_dtime = (int)(machine().time().as_ticks(XTAL(3'579'545)*2) - m_last_adb_time);
//      printf("ADB change to %d (dtime %d)\n", (data>>3) & 1, m_adb_dtime);
//      m_last_adb_time = machine().time().as_ticks(XTAL(3'579'545)*2);
		m_adb_line = (data & 0x8) ? true : false;
	}
}

uint8_t apple2gs_state::keyglu_mcu_read(uint8_t offset)
{
	uint8_t rv = m_glu_regs[offset];

//  printf("MCU reads reg %x\n", offset);

	// the command full flag is cleared by the MCU reading
	// first the KGS register and then the command register
	if ((offset == GLU_COMMAND) && (m_glu_mcu_read_kgs))
	{
		m_glu_regs[GLU_KG_STATUS] &= ~KGS_COMMAND_FULL;
		m_glu_mcu_read_kgs = false;
//      printf("MCU reads COMMAND = %02x (drop command full)\n", rv);
	}

	// prime for the next command register read to clear the command full flag
	if (offset == GLU_KG_STATUS)
	{
		m_glu_mcu_read_kgs = true;
	}

	return rv;
}

void  apple2gs_state::keyglu_mcu_write(uint8_t offset, uint8_t data)
{
	m_glu_regs[offset] = data;

//  printf("MCU writes %02x to reg %x\n", data, offset);

	switch (offset)
	{
		case GLU_KEY_DATA:
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_KEYDATIRQ;
			if (m_glu_regs[GLU_SYSSTAT] & GLU_STATUS_KEYDATIRQEN)
			{
				raise_irq(IRQS_ADB);
			}
			break;

		case GLU_MOUSEX:
		case GLU_MOUSEY:
			m_glu_regs[GLU_KG_STATUS] |= KGS_MOUSEX_FULL;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_MOUSEIRQ;
			if (m_glu_regs[GLU_SYSSTAT] & GLU_STATUS_MOUSEIRQEN)
			{
				raise_irq(IRQS_ADB);
			}
			m_glu_mouse_read_stat = false;  // signal next read will be mouse X
			break;

		case GLU_ANY_KEY_DOWN:  // bit 7 is the actual flag here; both MCU programs write either 0x7f or 0xff
//          printf("%d to ANY_KEY_DOWN (PC=%x)\n", data, m_adbmicro->pc());
			if (data & 0x80)
			{
				m_glu_regs[GLU_KG_STATUS] |= KGS_ANY_KEY_DOWN | KGS_KEYSTROBE;
			}
			break;

		case GLU_DATA:
			m_glu_regs[GLU_DATA] = data;
			m_glu_regs[GLU_KG_STATUS] |= KGS_DATA_FULL;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_DATAIRQ;
#if RUN_ADB_MICRO
			if (m_glu_regs[GLU_SYSSTAT] & GLU_STATUS_DATAIRQEN)
			{
				raise_irq(IRQS_ADB);
			}
#endif
			m_glu_816_read_dstat = false;
//          printf("MCU writes %02x to DATA\n", data);
			break;
	}
}

/*
   Keyglu registers map as follows on the 816:

   C000           = key data + any key down, clears strobe
   C010           = clears keystrobe
   C024 MOUSEDATA = reads GLU mouseX and mouseY
   C025 KEYMODREG = reads GLU keymod register
   C026 DATAREG   = writes from the 816 go to COMMAND, reads from DATA
   C027 KMSTATUS  = GLU system status register
*/

uint8_t apple2gs_state::keyglu_816_read(uint8_t offset)
{
	switch (offset)
	{
		case GLU_C000:
			{
				uint8_t rv;
				rv = m_glu_regs[GLU_KEY_DATA] & 0x7f;
				if (m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE)
				{
					rv |= 0x80;
				}
				return rv;
			}
			break;

		case GLU_C010:
			{
				uint8_t rv;
				rv = m_glu_regs[GLU_KEY_DATA] & 0x7f;
				if (m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE)
				{
					rv |= 0x80;
				}
				m_glu_regs[GLU_KG_STATUS] &= ~KGS_KEYSTROBE;
				return rv;
			}
			break;

		case GLU_MOUSEX:
		case GLU_MOUSEY:
			if (!m_glu_mouse_read_stat)
			{
				m_glu_mouse_read_stat = 1;
				return m_glu_regs[GLU_MOUSEY];
			}
			return m_glu_regs[GLU_MOUSEX];

		case GLU_SYSSTAT:
			// regenerate sysstat bits
			m_glu_sysstat &= ~0xab; // mask off read/write bits
			if (m_glu_regs[GLU_KG_STATUS] & KGS_COMMAND_FULL)
			{
				m_glu_sysstat |= 1;
			}
			if (m_glu_regs[GLU_KG_STATUS] & m_glu_mouse_read_stat)
			{
				m_glu_sysstat |= 2;
			}
			if (m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE)
			{
				m_glu_sysstat |= 8;
			}
			if (m_glu_regs[GLU_KG_STATUS] & KGS_DATA_FULL)
			{
				m_glu_sysstat |= 0x20;
			}
			if (m_glu_regs[GLU_KG_STATUS] & KGS_MOUSEX_FULL)
			{
				m_glu_sysstat |= 0x80;
			}
			m_glu_816_read_dstat = true;
//        printf("816 gets %02x in sysstat (data avail %02x)\n", m_glu_sysstat, m_glu_sysstat & 0x20);
			return m_glu_sysstat;

		case GLU_DATA:
			if (m_glu_816_read_dstat)
			{
				m_glu_816_read_dstat = false;
				m_glu_regs[GLU_KG_STATUS] &= ~KGS_DATA_FULL;
//              keyglu_regen_irqs();
//              printf("816 reads %02x from DATA\n", m_glu_regs[GLU_DATA]);
			}
			return m_glu_regs[GLU_DATA];

		default:
			return m_glu_regs[offset];
			break;
	}

	return 0xff;
}

void apple2gs_state::keyglu_816_write(uint8_t offset, uint8_t data)
{
	if (offset < GLU_C000)
	{
		m_glu_regs[offset&7] = data;
	}

	switch (offset)
	{
		case GLU_C010:
			m_glu_regs[GLU_KG_STATUS] &= ~KGS_KEYSTROBE;
			break;

		case GLU_COMMAND:
//          printf("816 sets COMMAND to %02x (raise command full)\n", data);
			m_glu_regs[GLU_KG_STATUS] |= KGS_COMMAND_FULL;
			break;

		case GLU_SYSSTAT:
			m_glu_sysstat &= 0xab;  // clear the non-read-only fields
			m_glu_sysstat |= (data & ~0xab);

			if (m_glu_sysstat)
			{
			}
			break;
	}
}

WRITE_LINE_MEMBER(apple2gs_state::scc_irq_w)
{
	if (state)
	{
		raise_irq(IRQS_SCC);
	}
	else
	{
		lower_irq(IRQS_SCC);
	}
}

/* Sound - DOC */
WRITE_LINE_MEMBER(apple2gs_state::doc_irq_w)
{
	if (state)
	{
		raise_irq(IRQS_DOC);
	}
	else
	{
		lower_irq(IRQS_DOC);
	}
}

READ8_MEMBER(apple2gs_state::doc_adc_read)
{
	return 0x80;
}

// temporary hookup of old IWM

int apple2gs_state::apple2_fdc_has_35()
{
	return (floppy_get_count(machine())); // - apple525_get_count(machine)) > 0;
}

int apple2gs_state::apple2_fdc_has_525()
{
	return 1; //apple525_get_count(machine) > 0;
}

static void apple2_fdc_set_lines(device_t *device, uint8_t lines)
{
	apple2gs_state *state = device->machine().driver_data<apple2gs_state>();
	if (state->m_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			sony_set_lines(device,lines);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			apple525_set_lines(device,lines);
		}
	}
}

static void apple2_fdc_set_enable_lines(device_t *device,int enable_mask)
{
	apple2gs_state *state = device->machine().driver_data<apple2gs_state>();
	int slot5_enable_mask = 0;
	int slot6_enable_mask = 0;

	if (state->m_diskreg & 0x40)
		slot5_enable_mask = enable_mask;
	else
		slot6_enable_mask = enable_mask;

	if (state->apple2_fdc_has_35())
	{
		/* set the 3.5" enable lines */
		sony_set_enable_lines(device,slot5_enable_mask);
	}

	if (state->apple2_fdc_has_525())
	{
		/* set the 5.25" enable lines */
		apple525_set_enable_lines(device,slot6_enable_mask);
	}
}

static uint8_t apple2_fdc_read_data(device_t *device)
{
	apple2gs_state *state = device->machine().driver_data<apple2gs_state>();
	uint8_t result = 0x00;

	if (state->m_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			result = sony_read_data(device);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			result = apple525_read_data(device);
		}
	}
	return result;
}

static void apple2_fdc_write_data(device_t *device, uint8_t data)
{
	apple2gs_state *state = device->machine().driver_data<apple2gs_state>();
	if (state->m_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			sony_write_data(device,data);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			apple525_write_data(device,data);
		}
	}
}

static int apple2_fdc_read_status(device_t *device)
{
	apple2gs_state *state = device->machine().driver_data<apple2gs_state>();
	int result = 0;

	if (state->m_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			result = sony_read_status(device);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			result = apple525_read_status(device);
		}
	}
	return result;
}

void apple2gs_state::apple2_iwm_setdiskreg(uint8_t data)
{
	if (apple2_fdc_has_35())
	{
		sony_set_sel_line(m_iwm, m_diskreg & 0x80);
	}
}

const applefdc_interface apple2_fdc_interface =
{
	apple2_fdc_set_lines,           /* set_lines */
	apple2_fdc_set_enable_lines,    /* set_enable_lines */

	apple2_fdc_read_data,           /* read_data */
	apple2_fdc_write_data,          /* write_data */
	apple2_fdc_read_status          /* read_status */
};

static const floppy_interface apple2gs_floppy35_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_iigs),
	"floppy_3_5"
};

static const floppy_interface apple2gs_floppy525_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple2),
	"floppy_5_25"
};


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( apple2gs_gameport )
	PORT_START("joystick_1_x")      /* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P1 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_4_PAD)    PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_1_y")      /* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P1 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_8_PAD)    PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_2_x")      /* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P2 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_2_y")      /* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P2 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(1) PORT_CODE(KEYCODE_0_PAD)     PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(1) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON2)
INPUT_PORTS_END

	/*
	  Apple IIe platinum and IIgs upgrade key matrix

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  | ESC |  1  |  2  |  3  |  4  |  6  |  5  |  7  |  8  |  9  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  Q  |  W  |  E  |  R  |  Y  |  T  |  U  |  I  |  O  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  A  |  D  |  S  |  H  |  F  |  G  |  J  |  K  | ;:  |  L  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  M  |  N  | ,<  | .>  |  /? |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  | KP/ |     | KP0 | KP1 | KP2 | KP3 | \|  | +=  |  0  | -_  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |     |KPEsc| KP4 | KP5 | KP6 | KP7 | `~  |  P  | [{  | ]}  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  | KP* |     | KP8 | KP9 | KP. | KP+ |RETRN| UP  | SPC | '"  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |     |     | KP- |KPENT|     | DEL |DOWN |LEFT |RIGHT|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

INPUT_PORTS_START( apple2gs )
#if RUN_ADB_MICRO
	PORT_START(A2GS_KBD_Y0_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START(A2GS_KBD_Y1_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Esc") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START(A2GS_KBD_Y2_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)   PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)   PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START(A2GS_KBD_Y3_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)   PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)   PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))

	PORT_START(A2GS_KBD_Y4_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)   PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)   PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START(A2GS_KBD_Y5_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)   PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)   PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START(A2GS_KBD_Y6_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)

	PORT_START(A2GS_KBD_Y7_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)

	PORT_START(A2GS_KBD_Y8_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)

	PORT_START(A2GS_KBD_Y9_TAG)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
#else
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)   PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)   PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)   PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)   PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Esc") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)   PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)   PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)   PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)   PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)
#endif

	PORT_START(A2GS_KBD_SPEC_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_INCLUDE(apple2gs_gameport)

	PORT_START("adb_mouse_x")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 1")

	PORT_START("adb_mouse_y")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 0")
INPUT_PORTS_END

static void apple2_cards(device_slot_interface &device)
{
	device.option_add("diskiing", A2BUS_DISKIING);  /* Disk II Controller Card, cycle-accurate version */
	device.option_add("mockingboard", A2BUS_MOCKINGBOARD);  /* Sweet Micro Systems Mockingboard */
	device.option_add("phasor", A2BUS_PHASOR);  /* Applied Engineering Phasor */
	device.option_add("cffa2", A2BUS_CFFA2);  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware */
	device.option_add("cffa202", A2BUS_CFFA2_6502);  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware */
	device.option_add("memexp", A2BUS_MEMEXP);  /* Apple II Memory Expansion Card */
	device.option_add("ramfactor", A2BUS_RAMFACTOR);    /* Applied Engineering RamFactor */
	device.option_add("thclock", A2BUS_THUNDERCLOCK);    /* ThunderWare ThunderClock Plus */
	device.option_add("softcard", A2BUS_SOFTCARD);  /* Microsoft SoftCard */
	device.option_add("videoterm", A2BUS_VIDEOTERM);    /* Videx VideoTerm */
	device.option_add("ssc", A2BUS_SSC);    /* Apple Super Serial Card */
	device.option_add("swyft", A2BUS_SWYFT);    /* IAI SwyftCard */
	device.option_add("themill", A2BUS_THEMILL);    /* Stellation Two The Mill (6809 card) */
	device.option_add("sam", A2BUS_SAM);    /* SAM Software Automated Mouth (8-bit DAC + speaker) */
	device.option_add("alfam2", A2BUS_ALFAM2);    /* ALF Apple Music II */
	device.option_add("echoii", A2BUS_ECHOII);    /* Street Electronics Echo II */
	device.option_add("ap16", A2BUS_IBSAP16);    /* IBS AP16 (German VideoTerm clone) */
	device.option_add("ap16alt", A2BUS_IBSAP16ALT);    /* IBS AP16 (German VideoTerm clone), alternate revision */
	device.option_add("vtc1", A2BUS_VTC1);    /* Unknown VideoTerm clone #1 */
	device.option_add("vtc2", A2BUS_VTC2);    /* Unknown VideoTerm clone #2 */
	device.option_add("arcbd", A2BUS_ARCADEBOARD);    /* Third Millenium Engineering Arcade Board */
	device.option_add("midi", A2BUS_MIDI);  /* Generic 6840+6850 MIDI board */
	device.option_add("zipdrive", A2BUS_ZIPDRIVE);  /* ZIP Technologies IDE card */
	device.option_add("echoiiplus", A2BUS_ECHOPLUS);    /* Street Electronics Echo Plus (Echo II + Mockingboard clone) */
	device.option_add("scsi", A2BUS_SCSI);  /* Apple II SCSI Card */
	device.option_add("applicard", A2BUS_APPLICARD);    /* PCPI Applicard */
	device.option_add("aesms", A2BUS_AESMS);    /* Applied Engineering Super Music Synthesizer */
	device.option_add("ultraterm", A2BUS_ULTRATERM);    /* Videx UltraTerm (original) */
	device.option_add("ultratermenh", A2BUS_ULTRATERMENH);    /* Videx UltraTerm (enhanced //e) */
	device.option_add("aevm80", A2BUS_VTC2);    /* Applied Engineering ViewMaster 80 */
	device.option_add("parallel", A2BUS_PIC);   /* Apple Parallel Interface Card */
	device.option_add("corvus", A2BUS_CORVUS);  /* Corvus flat-cable HDD interface (see notes in a2corvus.c) */
	device.option_add("mcms1", A2BUS_MCMS1);  /* Mountain Computer Music System, card 1 of 2 */
	device.option_add("mcms2", A2BUS_MCMS2);  /* Mountain Computer Music System, card 2 of 2.  must be in card 1's slot + 1! */
	device.option_add("dx1", A2BUS_DX1);    /* Decillonix DX-1 sampler card */
	device.option_add("tm2ho", A2BUS_TIMEMASTERHO); /* Applied Engineering TimeMaster II H.O. */
	device.option_add("mouse", A2BUS_MOUSE);    /* Apple II Mouse Card */
	device.option_add("ezcgi", A2BUS_EZCGI);    /* E-Z Color Graphics Interface */
	device.option_add("ezcgi9938", A2BUS_EZCGI_9938);   /* E-Z Color Graphics Interface (TMS9938) */
	device.option_add("ezcgi9958", A2BUS_EZCGI_9958);   /* E-Z Color Graphics Interface (TMS9958) */
//  device.option_add("magicmusician", A2BUS_MAGICMUSICIAN);    /* Magic Musician Card */
//  device.option_add("pcxport", A2BUS_PCXPORTER); /* Applied Engineering PC Transporter */
}

void apple2gs_state::apple2gs(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, A2GS_MASTER_CLOCK/10);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2gs_state::apple2gs_map);
	m_maincpu->set_addrmap(g65816_device::AS_VECTORS, &apple2gs_state::vectors_map);
	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(apple2gs_state::apple2_interrupt), "screen", 0, 1);
	config.m_minimum_quantum = attotime::from_hz(60);

	M50741(config, m_adbmicro, A2GS_MASTER_CLOCK/8);
	m_adbmicro->read_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_in));
	m_adbmicro->write_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_out));
	m_adbmicro->read_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_in));
	m_adbmicro->write_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_out));
	m_adbmicro->read_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_in));
	m_adbmicro->write_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_out));
	m_adbmicro->read_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_in));
	m_adbmicro->write_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_out));

#if !RUN_ADB_MICRO
	/* keyboard controller */
	AY3600(config, m_ay3600, 0);
	m_ay3600->x0().set_ioport("X0");
	m_ay3600->x1().set_ioport("X1");
	m_ay3600->x2().set_ioport("X2");
	m_ay3600->x3().set_ioport("X3");
	m_ay3600->x4().set_ioport("X4");
	m_ay3600->x5().set_ioport("X5");
	m_ay3600->x6().set_ioport("X6");
	m_ay3600->x7().set_ioport("X7");
	m_ay3600->x8().set_ioport("X8");
	m_ay3600->shift().set(FUNC(apple2gs_state::ay3600_shift_r));
	m_ay3600->control().set(FUNC(apple2gs_state::ay3600_control_r));
	m_ay3600->data_ready().set(FUNC(apple2gs_state::ay3600_data_ready_w));
	m_ay3600->ako().set(FUNC(apple2gs_state::ay3600_ako_w));

	/* repeat timer.  15 Hz from page 7-15 of "Understanding the Apple IIe" */
	TIMER(config, "repttmr").configure_periodic(FUNC(apple2gs_state::ay3600_repeat), attotime::from_hz(15));
#endif

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	APPLE2_VIDEO(config, m_video, A2GS_14M);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(704, 262);  // 640+32+32 for the borders
	m_screen->set_visarea(0,703,0,230);
	m_screen->set_screen_update(FUNC(apple2gs_state::screen_update));

	PALETTE(config, "palette", FUNC(apple2gs_state::palette_init), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ES5503(config, m_doc, A2GS_7M);
	m_doc->set_channels(2);
	m_doc->set_addrmap(0, &apple2gs_state::a2gs_es5503_map);
	m_doc->irq_func().set(FUNC(apple2gs_state::doc_irq_w));
	m_doc->adc_func().set(FUNC(apple2gs_state::doc_adc_read));
	m_doc->add_route(0, "lspeaker", 1.0);
	m_doc->add_route(1, "rspeaker", 1.0);

	/* RAM */
	RAM(config, m_ram).set_default_size("2M").set_extra_options("1M,3M,4M,5M,6M,7M,8M").set_default_value(0x00);

	/* C100 banking */
	ADDRESS_MAP_BANK(config, A2GS_C100_TAG).set_map(&apple2gs_state::c100bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x200);

	/* C300 banking */
	ADDRESS_MAP_BANK(config, A2GS_C300_TAG).set_map(&apple2gs_state::c300bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x100);

	/* C400 banking */
	ADDRESS_MAP_BANK(config, A2GS_C400_TAG).set_map(&apple2gs_state::c400bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x400);

	/* C800 banking */
	ADDRESS_MAP_BANK(config, A2GS_C800_TAG).set_map(&apple2gs_state::c800bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x800);

	/* built-in language card emulation */
	ADDRESS_MAP_BANK(config, A2GS_LCBANK_TAG).set_map(&apple2gs_state::lcbank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* aux bank language card emulation */
	ADDRESS_MAP_BANK(config, A2GS_LCAUX_TAG).set_map(&apple2gs_state::lcaux_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* bank 00 language card emulation */
	ADDRESS_MAP_BANK(config, A2GS_LC00_TAG).set_map(&apple2gs_state::lc00_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* bank 01 language card emulation */
	ADDRESS_MAP_BANK(config, A2GS_LC01_TAG).set_map(&apple2gs_state::lc01_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* /INH banking */
	ADDRESS_MAP_BANK(config, A2GS_UPPERBANK_TAG).set_map(&apple2gs_state::inhbank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* /INH banking - aux bank */
	ADDRESS_MAP_BANK(config, A2GS_AUXUPPER_TAG).set_map(&apple2gs_state::inhaux_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* /INH banking - bank 00 */
	ADDRESS_MAP_BANK(config, A2GS_00UPPER_TAG).set_map(&apple2gs_state::inh00_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* /INH banking - bank 01 */
	ADDRESS_MAP_BANK(config, A2GS_01UPPER_TAG).set_map(&apple2gs_state::inh01_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x3000);

	/* Bank 0 - I/O and LC area */
	ADDRESS_MAP_BANK(config, A2GS_B0CXXX_TAG).set_map(&apple2gs_state::bank0_iolc_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	/* Bank 1 - lower 48K */
	ADDRESS_MAP_BANK(config, A2GS_B01_TAG).set_map(&apple2gs_state::bank1_lower48_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0xc000);

	/* Bank 1 - I/O and LC area */
	ADDRESS_MAP_BANK(config, A2GS_B1CXXX_TAG).set_map(&apple2gs_state::bank1_iolc_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	/* Bank 0 0000 banking */
	ADDRESS_MAP_BANK(config, A2GS_B00000_TAG).set_map(&apple2gs_state::rb0000bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x200);

	/* Bank 0 0200 banking */
	ADDRESS_MAP_BANK(config, A2GS_B00200_TAG).set_map(&apple2gs_state::rb0200bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x200);

	/* Bank 0 0400 banking */
	ADDRESS_MAP_BANK(config, A2GS_B00400_TAG).set_map(&apple2gs_state::rb0400bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x400);

	/* Bank 0 0800 banking */
	ADDRESS_MAP_BANK(config, A2GS_B00800_TAG).set_map(&apple2gs_state::rb0800bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);

	/* Bank 0 2000 banking */
	ADDRESS_MAP_BANK(config, A2GS_B02000_TAG).set_map(&apple2gs_state::rb2000bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);

	/* Bank 0 4000 banking */
	ADDRESS_MAP_BANK(config, A2GS_B04000_TAG).set_map(&apple2gs_state::rb4000bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x8000);

	/* serial */
	SCC85C30(config, m_scc, A2GS_14M/2);
	m_scc->out_int_callback().set(FUNC(apple2gs_state::scc_irq_w));
	m_scc->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	/* slot devices */
	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(apple2gs_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(apple2gs_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(apple2gs_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	A2BUS_SLOT(config, "sl1", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl2", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl3", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl4", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl6", m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl7", m_a2bus, apple2_cards, nullptr);

	IWM(config, m_iwm, &apple2_fdc_interface);

	FLOPPY_APPLE(config, FLOPPY_0, &apple2gs_floppy525_floppy_interface, 15, 16);
	FLOPPY_APPLE(config, FLOPPY_1, &apple2gs_floppy525_floppy_interface, 15, 16);

	FLOPPY_SONY(config, FLOPPY_2, &apple2gs_floppy35_floppy_interface);
	FLOPPY_SONY(config, FLOPPY_3, &apple2gs_floppy35_floppy_interface);

	SOFTWARE_LIST(config, "flop35_list").set_original("apple2gs");
	SOFTWARE_LIST(config, "flop525_clean").set_compatible("apple2_flop_clcracked"); // No filter on clean cracks yet.
	// As WOZ images won't load in the 2GS driver yet, comment out the softlist entry.
	//SOFTWARE_LIST(config, "flop525_orig").set_compatible("apple2_flop_orig").set_filter("A2GS");  // Filter list to compatible disks for this machine.
	SOFTWARE_LIST(config, "flop525_misc").set_compatible("apple2_misc");
}

void apple2gs_state::apple2gsr1(machine_config &config)
{
	apple2gs(config);

	// 256K on board + 1M in the expansion slot was common for ROM 01
	m_ram->set_default_size("1280K").set_extra_options("256K,512K,768K,1M,2M,3M,4M,5M,6M,7M,8M").set_default_value(0x00);

	M50740(config.replace(), m_adbmicro, A2GS_MASTER_CLOCK/8);
	m_adbmicro->read_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_in));
	m_adbmicro->write_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_out));
	m_adbmicro->read_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_in));
	m_adbmicro->write_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_out));
	m_adbmicro->read_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_in));
	m_adbmicro->write_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_out));
	m_adbmicro->read_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_in));
	m_adbmicro->write_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_out));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START(apple2gs)
	// M50740/50741 ADB MCU inside the IIgs system unit
	ROM_REGION(0x1000,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	// i8048 microcontroller inside the IIgs ADB Standard Keyboard
	ROM_REGION(0x400, "kmcu", 0)
	// from early-production ROM 00 Woz Limited Edition IIgs.  keyboard "Part Number 658-4081  825-1301-A"
	// ROM is marked "NEC Japan  8626XD 341-0232A  543" so 26th week of 1986
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	// from later non-Woz ROM 01.  keyboard "Model A9M0330"
	// ROM is marked "NEC Japan 8806HD  8048HC610  341-0124-A  (c) APPLE 87" so 6th week of 1988
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	// 341-0728 is the MASK rom version while 341-0737 is the EPROM version - SAME data.
	ROM_LOAD("341-0728", 0x00000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27) ) /* 341-0728: IIgs ROM03 FC-FD */
	// 341-0748 is the MASK rom version while 341-0749 is the EPROM version - SAME data.
	ROM_LOAD("341-0748", 0x30000, 0x10000, CRC(18190283) SHA1(c70576869deec92ca82c78438b1d5c686eac7480) ) /* 341-0748: IIgs ROM03 FE-FF */
	ROM_CONTINUE ( 0x20000, 0x10000) /* high address line is inverted on PCB? */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr3p)
	ROM_REGION(0x1000,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("341-0728", 0x00000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27) ) /* 341-0728: IIgs ROM03 prototype FC-FD - 28 pin MASK rom */
	ROM_LOAD("341-0729", 0x20000, 0x20000, NO_DUMP) /* 341-0729: IIgs ROM03 prototype FE-FF */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr1)
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("342-0077-b", 0x20000, 0x20000, CRC(42f124b0) SHA1(e4fc7560b69d062cb2da5b1ffbe11cd1ca03cc37)) /* 342-0077-B: IIgs ROM01 */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0)
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("342-0077-a", 0x20000, 0x20000, CRC(dfbdd97b) SHA1(ff0c245dd0732ec4413a934fd80efc2defd8a8e3) ) /* 342-0077-A: IIgs ROM00 */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p)  // 6/19/1986 Cortland prototype
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD( "rombf.bin",    0x020000, 0x020000, CRC(ab04fedf) SHA1(977589a17553956d583a21020080a39dd396df5c) )

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p2)  // 3/10/1986 Cortland prototype, boots as "Apple //'ing - Alpha 2.0"
	ROM_REGION(0xc00,M5074X_INTERNAL_ROM(A2GS_ADBMCU_TAG),0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x400, "kmcu", 0)
	ROM_LOAD( "341-0232a.bin", 0x000000, 0x000400, CRC(6a158b9f) SHA1(e8744180075182849d431fd8023a52a062a6da76) )
	ROM_LOAD( "341-0124a.bin", 0x000000, 0x000400, CRC(2a3576bf) SHA1(58fbf770d3801a02d0944039829f9241b5279013) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD( "apple iigs alpha rom 2.0 19860310.bin", 0x020000, 0x020000, CRC(a47d275f) SHA1(c5836adcfc8be69c7351b84afa94c814e8d92b81) )

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

/*    YEAR  NAME            PARENT    COMPAT    MACHINE     INPUT     CLASS        INIT  COMPANY           FULLNAME */
COMP( 1989, apple2gs,     0,        apple2, apple2gs,   apple2gs, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03)", MACHINE_SUPPORTS_SAVE )
COMP( 198?, apple2gsr3p,  apple2gs, 0,      apple2gs,   apple2gs, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03 prototype)", MACHINE_NOT_WORKING )
COMP( 1987, apple2gsr1,   apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM01)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0,   apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p,  apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 6/19/1986)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p2, apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 3/10/1986)", MACHINE_SUPPORTS_SAVE )
