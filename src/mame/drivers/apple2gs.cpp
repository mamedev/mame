// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2gs.cpp - Apple IIgs

    Next generation driver written June 2018 by R. Belmont.
    Thanks to the original Apple IIgs driver's authors: Nathan Woods and R. Belmont
    Thanks also to the Apple II Documentation Project/Antoine Vignau, Peter Ferrie, and Olivier Galibert.

    Unique hardware configurations:
    - ROM 00/01: original motherboard, 256K of RAM (banks 00/01/E0/E1 only), FPI chip manages fast/slow side
    - ROM 03: revised motherboard, 1M of RAM (banks 00/01/->0F/E0/E1), CYA chip replaces FPI
    - Expanded IIe: ROM 00/01 motherboard in a IIe case with a IIe keyboard rather than ADB

    Timing in terms of the 14M 14.3181818 MHz clock (1/2 of the 28.6363636 master clock):
    - 1 65816 cycle is 5 14M clocks
    - Every 50 14M clocks (10 65816 cycles) DRAM refresh occurs for 5 14M clocks
      * During this time, CPU accesses to ROM, Mega II side I/O, or banks E0/E1 are not penalized (but a side-sync penalty is incurred)
      * Accesses to banks 00-7F are penalized except for I/O in banks 0/1.
    - The Mega II 1 MHz side runs for 64 cycles at 14 14M clocks and every 65th is stretched to 16 14M clocks.
      This allows 8-bit Apple II raster demos to work.  Each scanline is (64*14) + 16 = 912 14M clocks.
      Due to this stretch, which does not occur on the fast side, the fast and 1 Mhz sides drift from each other
      and sync up every 22800 14M clocks (25 scan lines).

    One video line is: 6 cycles of right border, 13 cycles of hblank, 6 cycles of left border, and 40 cycles of active video

    640 + 96 + 96 = 832 (make borders each 38 pixels wider in A2 modes)

    FF6ACF is speed test in ROM
    Diags:
    A138 = scanline interrupt test (raster is too long to pass this)
    A179 = pass
    A17C = fail 1
    A0F1 = fail 2

    ZipGS notes:
    $C059 is the GS settings register
    bit 3: CPS Follow
    bit 4: Counter Delay
    bit 5: AppleTalk Delay
    bit 6: Joystick Delay (reverse logic: 0 = delay is ON)
    bit 7: C/D cache disable

    $C05D is the speed percentage:
    $F0 = 6%, $E0 = 12%, $D0 = 18%, $C0 = 25%, $B0 = 31%, $A0 = 37%, $90 = 43%, $80 = 50%,
    $70 = 56%, $60 = 62%, $50 = 68%, $40 = 75%, $30 = 81%, $20 = 87%, $10 = 93%, $00 = 100%

***************************************************************************/

#include "emu.h"
#include "video/apple2.h"

#define RUN_ADB_MICRO (0)       // use the ADB microcontroller for keyboard/mouse input
#define ADB_HLE (0)             // connect the ADB microcontroller to the macadb.cpp ADB bit-serial keyboard+mouse
#define LOG_ADB (0)             // log ADB activity in the old-style HLE simulation of the microcontroller and GLU

#include "screen.h"
#include "softlist_dev.h"
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
#include "machine/macadb.h"

#include "machine/applefdintf.h"
#include "machine/iwm.h"
#include "formats/ap2_dsk.h"
#include "formats/ap_dsk35.h"

#include "bus/rs232/rs232.h"

#include "emu.h"
#include "video/apple2.h"
#include "machine/apple2common.h"
//#include "machine/apple2host.h"

#include "bus/a2bus/cards.h"
#include "bus/a2gameio/gameio.h"

namespace {

// various timing standards
#define A2GS_MASTER_CLOCK (XTAL(28'636'363))
#define A2GS_14M    (A2GS_MASTER_CLOCK/2)
#define A2GS_7M     (A2GS_MASTER_CLOCK/4)
#define A2GS_1M     (A2GS_MASTER_CLOCK/28)

#define A2GS_UPPERBANK_TAG "inhbank"
#define A2GS_AUXUPPER_TAG "inhaux"
#define A2GS_00UPPER_TAG "inh00"
#define A2GS_01UPPER_TAG "inh01"

#define A2GS_C300_TAG "c3bank"
#define A2GS_LCBANK_TAG "lcbank"
#define A2GS_LCAUX_TAG "lcaux"
#define A2GS_LC00_TAG "lc00"
#define A2GS_LC01_TAG "lc01"
#define A2GS_B0CXXX_TAG "bnk0atc"
#define A2GS_B1CXXX_TAG "bnk1atc"
#define A2GS_B00000_TAG "b0r00bank"
#define A2GS_B00200_TAG "b0r02bank"
#define A2GS_B00400_TAG "b0r04bank"
#define A2GS_B00800_TAG "b0r08bank"
#define A2GS_B02000_TAG "b0r20bank"
#define A2GS_B04000_TAG "b0r40bank"

#define A2GS_KBD_SPEC_TAG "keyb_special"

class apple2gs_state : public driver_device
{
public:
	apple2gs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_screen(*this, "screen"),
		  m_scantimer(*this, "scantimer"),
		  m_acceltimer(*this, "acceltimer"),
		  m_adbmicro(*this, "adbmicro"),
#if ADB_HLE
		  m_macadb(*this, "macadb"),
#endif
		  m_ram(*this, "ram"),
		  m_rom(*this, "maincpu"),
		  m_docram(*this, "docram"),
		  m_nvram(*this, "nvram"),
		  m_video(*this, "a2video"),
		  m_a2bus(*this, "a2bus"),
		  m_a2common(*this, "a2common"),
		  //      m_a2host(*this, "a2host"),
		  m_gameio(*this, "gameio"),
		  m_speaker(*this, "speaker"),
		  m_upperbank(*this, A2GS_UPPERBANK_TAG),
		  m_upperaux(*this, A2GS_AUXUPPER_TAG),
		  m_upper00(*this, A2GS_00UPPER_TAG),
		  m_upper01(*this, A2GS_01UPPER_TAG),
		  m_c300bank(*this, A2GS_C300_TAG),
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
		  m_bank1_atc(*this, A2GS_B1CXXX_TAG),
		  m_scc(*this, "scc"),
		  m_doc(*this, "doc"),
		  m_iwm(*this, "fdc"),
		  m_floppy(*this, "fdc:%d", 0U),
		  m_kbd(*this, "Y%d", 0),
		  m_kbspecial(*this, A2GS_KBD_SPEC_TAG),
		  m_sysconfig(*this, "a2_config"),
		  m_ay3600(*this, "ay3600"),
		  m_kbdrom(*this, "keyboard"),
		  m_adb_mousex(*this, "adb_mouse_x"),
		  m_adb_mousey(*this, "adb_mouse_y")
	{
		m_cur_floppy = nullptr;
		m_devsel = 0;
		m_diskreg = 0;
	}

	void apple2gs(machine_config &config);
	void apple2gsr1(machine_config &config);

	void rom1_init() { m_is_rom3 = false; }
	void rom3_init() { m_is_rom3 = true; }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<g65816_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scantimer, m_acceltimer;
	required_device<m5074x_device> m_adbmicro;
#if ADB_HLE
	required_device<macadb_device> m_macadb;
#endif
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_docram;
	required_device<nvram_device> m_nvram;
	required_device<a2_video_device> m_video;
	required_device<a2bus_device> m_a2bus;
	required_device<apple2_common_device> m_a2common;
//  required_device<apple2_host_device> m_a2host;
	required_device<apple2_gameio_device> m_gameio;
	required_device<speaker_sound_device> m_speaker;
	memory_view m_upperbank, m_upperaux, m_upper00, m_upper01;
	required_device<address_map_bank_device> m_c300bank;
	memory_view m_b0_0000bank, m_b0_0200bank, m_b0_0400bank, m_b0_0800bank, m_b0_2000bank, m_b0_4000bank;
	memory_view m_lcbank, m_lcaux, m_lc00, m_lc01, m_bank0_atc, m_bank1_atc;
	required_device<z80scc_device> m_scc;
	required_device<es5503_device> m_doc;
	required_device<applefdintf_device> m_iwm;
	required_device_array<floppy_connector, 4> m_floppy;
	optional_ioport_array<10> m_kbd;
	optional_ioport m_kbspecial;
	required_ioport m_sysconfig;
	optional_device<ay3600_device> m_ay3600;
	required_memory_region m_kbdrom;
	optional_ioport m_adb_mousex, m_adb_mousey;

	static constexpr int CNXX_UNCLAIMED = -1;
	static constexpr int CNXX_INTROM = -2;

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


	static constexpr u8 KGS_ANY_KEY_DOWN = 0x01;
	static constexpr u8 KGS_KEYSTROBE    = 0x10;
	static constexpr u8 KGS_DATA_FULL    = 0x20;
	static constexpr u8 KGS_COMMAND_FULL = 0x40;
	static constexpr u8 KGS_MOUSEX_FULL  = 0x80;

	static constexpr u8 GLU_STATUS_CMDFULL  = 0x01;
	static constexpr u8 GLU_STATUS_MOUSEXY  = 0x02;
	static constexpr u8 GLU_STATUS_KEYDATIRQEN = 0x04;
	static constexpr u8 GLU_STATUS_KEYDATIRQ = 0x08;
	static constexpr u8 GLU_STATUS_DATAIRQEN = 0x10;
	static constexpr u8 GLU_STATUS_DATAIRQ  = 0x20;
	static constexpr u8 GLU_STATUS_MOUSEIRQEN = 0x40;
	static constexpr u8 GLU_STATUS_MOUSEIRQ = 0x080;

	static constexpr u8 SHAD_IOLC       = 0x40; // I/O and language card inhibit for banks 00/01
	static constexpr u8 SHAD_TXTPG2     = 0x20; // inhibits text-page 2 shadowing in both banks (ROM 03 h/w only)
	static constexpr u8 SHAD_AUXHIRES   = 0x10; // inhibits bank 01 hi-res region shadowing
	static constexpr u8 SHAD_SUPERHIRES = 0x08; // inhibits bank 01 super-hi-res region shadowing
	static constexpr u8 SHAD_HIRESPG2   = 0x04; // inhibits hi-res page 2 shadowing in both banks
	static constexpr u8 SHAD_HIRESPG1   = 0x02; // inhibits hi-res page 1 shadowing in both banks
	static constexpr u8 SHAD_TXTPG1     = 0x01; // inhibits text-page 1 shadowing in both banks

	static constexpr u8 SPEED_HIGH      = 0x80; // full 2.8 MHz speed when set, Apple II 1 MHz when clear
	[[maybe_unused]] static constexpr u8 SPEED_POWERON = 0x40; // ROM 03 only; indicates machine turned on by power switch (as opposed to ?)
	static constexpr u8 SPEED_ALLBANKS  = 0x10; // enables bank 0/1 shadowing in all banks (not supported)
	[[maybe_unused]] static constexpr u8 SPEED_DISKIISL7 = 0x08; // enable Disk II motor on detect for slot 7
	[[maybe_unused]] static constexpr u8 SPEED_DISKIISL6 = 0x04; // enable Disk II motor on detect for slot 6
	[[maybe_unused]] static constexpr u8 SPEED_DISKIISL5 = 0x02; // enable Disk II motor on detect for slot 5
	[[maybe_unused]] static constexpr u8 SPEED_DISKIISL4 = 0x01; // enable Disk II motor on detect for slot 4

	static constexpr u8 DISKREG_HDSEL = 7; // select signal for 3.5" Sony drives
	static constexpr u8 DISKREG_35SEL = 6; // 1 to enable 3.5" drives, 0 to chain through to 5.25"

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

	static constexpr u8 INTFLAG_IRQASSERTED = 0x01;
	[[maybe_unused]] static constexpr u8 INTFLAG_M2MOUSEMOVE = 0x02;
	[[maybe_unused]] static constexpr u8 INTFLAG_M2MOUSESW   = 0x04;
	static constexpr u8 INTFLAG_VBL         = 0x08;
	static constexpr u8 INTFLAG_QUARTER     = 0x10;
	static constexpr u8 INTFLAG_AN3         = 0x20;
	[[maybe_unused]] static constexpr u8 INTFLAG_MOUSEDOWNLAST = 0x40;
	[[maybe_unused]] static constexpr u8 INTFLAG_MOUSEDOWN   = 0x80;

	[[maybe_unused]]  static constexpr u8 VGCINT_EXTERNALEN = 0x01;
	static constexpr u8 VGCINT_SCANLINEEN   = 0x02;
	static constexpr u8 VGCINT_SECONDENABLE = 0x04;
	[[maybe_unused]] static constexpr u8 VGCINT_EXTERNAL     = 0x10;
	static constexpr u8 VGCINT_SCANLINE     = 0x20;
	static constexpr u8 VGCINT_SECOND       = 0x40;
	static constexpr u8 VGCINT_ANYVGCINT    = 0x80;

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

	bool m_adb_line;

	address_space *m_maincpu_space;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(accel_timer);

	void palette_init(palette_device &palette);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void apple2gs_map(address_map &map);
	void vectors_map(address_map &map);
	void a2gs_es5503_map(address_map &map);
	void c300bank_map(address_map &map);

	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);

	floppy_image_device *m_cur_floppy;
	int m_devsel;
	u8 m_diskreg;

	u8 ram0000_r(offs_t offset);
	void ram0000_w(offs_t offset, u8 data);
	u8 auxram0000_r(offs_t offset);
	void auxram0000_w(offs_t offset, u8 data);
	u8 b0ram0000_r(offs_t offset);
	void b0ram0000_w(offs_t offset, u8 data);
	u8 b0ram0200_r(offs_t offset);
	void b0ram0200_w(offs_t offset, u8 data);
	u8 b0ram0400_r(offs_t offset);
	void b0ram0400_w(offs_t offset, u8 data);
	u8 b0ram0800_r(offs_t offset);
	void b0ram0800_w(offs_t offset, u8 data);
	u8 b0ram2000_r(offs_t offset);
	void b0ram2000_w(offs_t offset, u8 data);
	u8 b0ram4000_r(offs_t offset);
	void b0ram4000_w(offs_t offset, u8 data);
	u8 b1ram0000_r(offs_t offset);
	void b1ram0000_w(offs_t offset, u8 data);
	u8 b1ram0200_r(offs_t offset);
	void b1ram0200_w(offs_t offset, u8 data);
	u8 b1ram0400_r(offs_t offset);
	void b1ram0400_w(offs_t offset, u8 data);
	u8 b1ram0800_r(offs_t offset);
	void b1ram0800_w(offs_t offset, u8 data);
	u8 b1ram2000_r(offs_t offset);
	void b1ram2000_w(offs_t offset, u8 data);
	u8 b1ram4000_r(offs_t offset);
	void b1ram4000_w(offs_t offset, u8 data);
	u8 c000_r(offs_t offset);
	void c000_w(offs_t offset, u8 data);
	u8 c080_r(offs_t offset);
	void c080_w(offs_t offset, u8 data);
	u8 c100_r(offs_t offset);
	void c100_w(offs_t offset, u8 data);
	u8 c300_r(offs_t offset);
	u8 c300_int_r(offs_t offset);
	void c300_w(offs_t offset, u8 data);
	u8 c400_r(offs_t offset);
	void c400_w(offs_t offset, u8 data);
	u8 c800_r(offs_t offset);
	void c800_w(offs_t offset, u8 data);
	u8 inh_r(offs_t offset);
	void inh_w(offs_t offset, u8 data);
	u8 lc_r(offs_t offset);
	void lc_w(offs_t offset, u8 data);
	u8 lc_aux_r(offs_t offset);
	void lc_aux_w(offs_t offset, u8 data);
	u8 lc_00_r(offs_t offset);
	void lc_00_w(offs_t offset, u8 data);
	u8 lc_01_r(offs_t offset);
	void lc_01_w(offs_t offset, u8 data);
	u8 bank0_c000_r(offs_t offset);
	void bank0_c000_w(offs_t offset, u8 data);
	u8 bank1_0000_r(offs_t offset);
	void bank1_0000_sh_w(offs_t offset, u8 data);
	u8 bank1_c000_r(offs_t offset);
	void bank1_c000_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_WRITE_LINE_MEMBER(doc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(scc_irq_w);
	u8 doc_adc_read();
	u8 apple2gs_read_vector(offs_t offset);

#if !RUN_ADB_MICRO
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	DECLARE_WRITE_LINE_MEMBER(ay3600_ako_w);
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);
#endif

	u8 keyglu_mcu_read(u8 offset);
	void keyglu_mcu_write(u8 offset, u8 data);
#if RUN_ADB_MICRO
	u8 keyglu_816_read(u8 offset);
	void keyglu_816_write(u8 offset, u8 data);
#endif
	void keyglu_regen_irqs();

	u8 adbmicro_p0_in();
	u8 adbmicro_p1_in();
	u8 adbmicro_p2_in();
	u8 adbmicro_p3_in();
	void adbmicro_p0_out(u8 data);
	void adbmicro_p1_out(u8 data);
	void adbmicro_p2_out(u8 data);
	void adbmicro_p3_out(u8 data);
#if ADB_HLE
	void set_adb_line(int linestate);
#endif

	offs_t dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
	void wdm_trampoline(offs_t offset, u8 data) { }; //m_a2host->wdm_w(space, offset, data); }

	bool m_is_rom3;
	int m_speaker_state;

	double m_joystick_x1_time, m_joystick_y1_time, m_joystick_x2_time, m_joystick_y2_time;

	int m_inh_slot, m_cnxx_slot;
	int m_motoroff_time;

	bool m_romswitch;

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
	bool m_rombank;

	u8 m_shadow, m_speed, m_textcol;
	u8 m_motors_active, m_slotromsel, m_intflag, m_vgcint, m_inten, m_newvideo;

	bool m_last_speed;

	// Sound GLU variables
	u8 m_sndglu_ctrl;
	int m_sndglu_addr;
	int m_sndglu_dummy_read;

	// Key GLU variables
	u8 m_glu_regs[12], m_glu_bus;
	bool m_glu_mcu_read_kgs, m_glu_816_read_dstat, m_glu_mouse_read_stat;
	int m_glu_kbd_y;

	u8 *m_ram_ptr;
	int m_ram_size;
	u8 m_megaii_ram[0x20000];  // 128K of "slow RAM" at $E0/0000

	int m_inh_bank;

	bool m_slot_irq;

	double m_x_calibration, m_y_calibration;

	device_a2bus_card_interface *m_slotdevice[8];

	u32 m_slow_counter;

	// clock/BRAM
	u8 m_clkdata, m_clock_control, m_clock_read, m_clock_reg1;
	apple2gs_clock_mode m_clock_mode;
	u32 m_clock_curtime;
	seconds_t m_clock_curtime_interval;
	u8 m_clock_bram[256];
	int m_clock_frame;

	// ADB simulation
	#if !RUN_ADB_MICRO
	adbstate_t m_adb_state;
	u8 m_adb_command;
	u8 m_adb_mode;
	u8 m_adb_kmstatus;
	u8 m_adb_pending_status;
	u8 m_adb_latent_result;
	s32 m_adb_command_length;
	s32 m_adb_command_pos;
	u8 m_adb_response_length;
	s32 m_adb_response_pos;
	u8 m_adb_command_bytes[8];
	u8 m_adb_response_bytes[8];
	u8 m_adb_memory[0x100];
	int m_adb_address_keyboard;
	int m_adb_address_mouse;

	u16 m_lastchar, m_strobe;
	u8 m_transchar;
	bool m_anykeydown;
	int m_repeatdelay;

	u8 adb_read_datareg();
	u8 adb_read_kmstatus();
	u8 adb_read_memory(u32 address);
	void adb_write_memory(u32 address, u8 data);
	void adb_set_mode(u8 mode);
	void adb_set_config(u8 b1, u8 b2, u8 b3);
	void adb_post_response(const u8 *bytes, size_t length);
	void adb_post_response_1(u8 b);
	void adb_post_response_2(u8 b1, u8 b2);
	void adb_post_response_3(u8 b1, u8 b2, u8 b3);
	void adb_do_command();
	void adb_write_datareg(u8 data);
	void adb_write_kmstatus(u8 data);
	u8 adb_read_mousedata();
	s8 seven_bit_diff(u8 v1, u8 v2);
	void adb_check_mouse();
	#endif

	u8 m_mouse_x;
	u8 m_mouse_y;
	s8 m_mouse_dx;
	s8 m_mouse_dy;

	void do_io(int offset);
	u8 read_floatingbus();
	void update_slotrom_banks();
	void lc_update(int offset, bool writing);
	u8 read_slot_rom(int slotbias, int offset);
	void write_slot_rom(int slotbias, int offset, u8 data);
	u8 read_int_rom(int slotbias, int offset);
	void auxbank_update();
	void raise_irq(int irq);
	void lower_irq(int irq);
	void update_speed();
	int get_vpos();
	void process_clock();

	// ZipGS stuff
	bool m_accel_unlocked;
	bool m_accel_fast;
	bool m_accel_present;
	bool m_accel_temp_slowdown;
	int m_accel_stage;
	u32 m_accel_speed;
	u8 m_accel_slotspk, m_accel_gsxsettings, m_accel_percent;

	void accel_full_speed()
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

		if (isfast)
		{
			m_maincpu->set_unscaled_clock(m_accel_speed);
		}
		else
		{
			m_maincpu->set_unscaled_clock(1021800);
		}
	}

	void accel_normal_speed()
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

		if (isfast)
		{
			m_maincpu->set_unscaled_clock(A2GS_14M/5);
		}
		else
		{
			m_maincpu->set_unscaled_clock(1021800);
		}
	}

	void accel_slot(int slot);
};

// FF6ACF is speed test routine in ROM 3

#define slow_cycle() \
{   \
	if (!machine().side_effects_disabled() && m_last_speed) \
	{\
		m_slow_counter += 0x0001999a; \
		int cycles = (m_slow_counter >> 16) & 0xffff; \
		m_slow_counter &= 0xffff; \
		m_maincpu->adjust_icount(-cycles); \
	} \
}


offs_t apple2gs_state::dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return m_a2common->dasm_override_GS(stream, pc, opcodes, params);
}

WRITE_LINE_MEMBER(apple2gs_state::a2bus_irq_w)
{
	if (state == ASSERT_LINE)
	{
		raise_irq(IRQS_SLOT);
		m_slot_irq = true;
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
							m_upperbank.select(1);
							m_upperaux.select(1);
							m_upper00.select(1);
							m_upper01.select(1);
							m_inh_bank = 1;
						}
					}
					else
					{
						if (m_inh_bank != 0)
						{
							m_upperbank.select(0);
							m_upperaux.select(0);
							m_upper00.select(0);
							m_upper01.select(0);
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
			m_upperbank.select(0);
			m_upperaux.select(0);
			m_upper00.select(0);
			m_upper01.select(0);
			m_inh_bank = 0;
		}
	}
}

// FPI/CYA chip is connected to the VPB output of the 65816.
// this facilitates the documented behavior from the Firmware Reference.
u8 apple2gs_state::apple2gs_read_vector(offs_t offset)
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
		u8 *decode = m_kbdrom->base();
		u16 trans;

		// if the user presses a valid key to start the driver from the info screen,
		// we will see that key.  ignore keys in the first 25,000 cycles (in my tests,
		// the unwanted key shows up at 17030 cycles)
		if (m_sysconfig->read() & 0x01)
		{ // bump the cycle count way up for a 16 Mhz ZipGS
			if (m_maincpu->total_cycles() < 700000)
			{
				return;
			}
		}
		else
		{
			if (m_maincpu->total_cycles() < 25000)
			{
				return;
			}
		}
		m_lastchar = m_ay3600->b_r();

		u8 special = m_kbspecial->read();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place
		trans <<= 2;                    // 4 entries per key
		trans |= (special & 0x06) ? 0x00 : 0x01;    // shift is bit 1 (active low)
		trans |= (special & 0x08) ? 0x00 : 0x02;    // control is bit 2 (active low)
		trans |= (special & 0x01) ? 0x0000 : 0x0200;    // caps lock is bit 9 (active low)

		// hack in keypad equals because we can't find it in the IIe keymap (Sather doesn't show it in the matrix, but it's clearly on real platinum IIes)
		if (m_lastchar == 0x106)
		{
			m_transchar = '=';
		}
		else
		{
			m_transchar = decode[trans];
		}
		m_strobe = 0x80;

//      printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);

		/* check for command-control-esc and command-control delete */
		if ((special & 0x18) == 0x18)
		{
			if (m_transchar == 0x1b)
			{
				m_adb_pending_status |= 0x20;
			}
			else if (m_transchar == 0x7f)
			{
				m_adb_pending_status |= 0x10;
			}
			if (m_adb_pending_status && m_adb_state == ADBSTATE_IDLE)
			{
				adb_post_response_1(m_adb_pending_status);
				m_adb_pending_status = 0;
			}
		}
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
#endif
#if !RUN_ADB_MICRO
u8 apple2gs_state::adb_read_memory(u32 address)
{
	if (address < std::size(m_adb_memory))
		return m_adb_memory[address];
	else
		return 0x00;
}

void apple2gs_state::adb_write_memory(u32 address, u8 data)
{
	if (address < std::size(m_adb_memory))
		m_adb_memory[address] = data;
}

void apple2gs_state::adb_set_mode(u8 mode)
{
	m_adb_mode = mode;
}

void apple2gs_state::adb_set_config(u8 b1, u8 b2, u8 b3)
{
	/* ignore for now */
}

void apple2gs_state::adb_post_response(const u8 *bytes, size_t length)
{
	assert(length < std::size(m_adb_response_bytes));
	memcpy(m_adb_response_bytes, bytes, length);

	m_adb_state = ADBSTATE_INRESPONSE;
	m_adb_response_length = length;
	m_adb_response_pos = 0;
	m_adb_kmstatus |= 0x20;
	if (m_adb_kmstatus & 0x10)
		raise_irq(IRQS_ADB);
}

void apple2gs_state::adb_post_response_1(u8 b)
{
	adb_post_response(&b, 1);
}

void apple2gs_state::adb_post_response_2(u8 b1, u8 b2)
{
	u8 b[2];
	b[0] = b1;
	b[1] = b2;
	adb_post_response(b, 2);
}

void apple2gs_state::adb_post_response_3(u8 b1, u8 b2, u8 b3)
{
	u8 b[3];
	b[0] = b1;
	b[1] = b2;
	b[2] = b3;
	adb_post_response(b, 3);
}

void apple2gs_state::adb_do_command()
{
	int device;
	u32 address;
	u8 val;

	m_adb_state = ADBSTATE_IDLE;
	if (LOG_ADB)
		logerror("adb_do_command(): adb_command=0x%02x\n", m_adb_command);

	switch(m_adb_command)
	{
		case 0x00:  /* ??? */
			break;

		case 0x01: /* abort */
			break;

		case 0x02: /* reset keyboard uC */
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

		case 0x0a: /* read modes */
			adb_post_response_1(m_adb_mode);
			break;

		case 0x0b: /* read config */
			adb_post_response_3(0, 0, 0); /* ignored for now */
			break;

		case 0x0c: /* read adb error */
			adb_post_response_1(0);
			break;

		case 0x0d:  /* get version */
			adb_post_response_1(m_is_rom3 ? 0x06 : 0x05); /* rom0 0x04 */
			break;

		case 0x0e:  /* read available charsets */
			adb_post_response_2(0x01, 0x00);
			break;

		case 0x0f:  /* read available layouts */
			adb_post_response_2(0x01, 0x00);
			break;

		case 0x10: /* reset system */
			break;

		case 0x11: /* send adb keycode */
			break;

		case 0x12:  /* mystery command 0x12 - mouse key parameters */
		case 0x13:  /* mystery command 0x13 - disk key parameters */
			break;

		case 0x40: /* reset adb */
			break;

		case 0x49: case 0x4a: case 0x4b: case 0x4c:
		case 0x4d: case 0x4e: case 0x4f:
			/* transmit adb bytes. first byte is adb device command */
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
}

u8 apple2gs_state::adb_read_datareg()
{
	u8 result;

	switch(m_adb_state)
	{
		case ADBSTATE_INRESPONSE:
			result = m_adb_response_bytes[m_adb_response_pos++];
			if (m_adb_response_pos >= m_adb_response_length)
			{
				m_adb_state = ADBSTATE_IDLE;
				m_adb_latent_result = result;

				m_adb_kmstatus &= ~0x20;
				if (((m_adb_kmstatus & (m_adb_kmstatus >> 1)) & 0x54) == 0)
					lower_irq(IRQS_ADB);
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

void apple2gs_state::adb_write_datareg(u8 data)
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
				case 0x02:  /* reset keyboard uC */
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

				case 0x0a:  /* read modes */
				case 0x0b:  /* read config */
				case 0x0c:  /* read adb error */
				case 0x0d:  /* get version */
				case 0x0e:  /* read available charsets */
				case 0x0f:  /* read available layouts */
					m_adb_command_length = 0;
					m_adb_state = ADBSTATE_INCOMMAND;    /* HACK */
					break;

				case 0x10: /* reset system */
					/* do nothing for now */
					break;

				case 0x11: /* send adb keycode */
					m_adb_command_length = 1;
					break;

				case 0x12:  /* mystery command 0x12 - mouse key parameters? */
				case 0x13:  /* mystery command 0x13 - disk eject parameters? */
					m_adb_command_length = 2;
					break;

				case 0x40: /* reset ADB */
					break;

				/* 0x49 - 0x4f - transmit ADB bytes where length = command & 0xf - 6*/
				case 0x49: case 0x4a: case 0x4b: case 0x4c:
				case 0x4d: case 0x4e: case 0x4f:
					m_adb_command_length = (m_adb_command & 0x0f) - 6;
					break;

				case 0x50:  /* enable SRQ device 0 */
				case 0x51:  /* enable SRQ device 1 */
				case 0x52:  /* enable SRQ device 2 */
				case 0x53:  /* enable SRQ device 3 */
					/* ignore for now */
					break;

				case 0x60:  /* flush adb buffer device 0 */
				case 0x61:  /* flush adb buffer device 1 */
				case 0x62:  /* flush adb buffer device 2 */
				case 0x63:  /* flush adb buffer device 3 */
					/* ignore for now */
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
					adb_post_response_1(0x80);
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
			assert(m_adb_command_pos < std::size(m_adb_command_bytes));
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
// current MAME reads back 0xb0 when idle
u8 apple2gs_state::adb_read_kmstatus()
{
	return m_adb_kmstatus;
}


void apple2gs_state::adb_write_kmstatus(u8 data)
{
	m_adb_kmstatus &= ~0x54;
	m_adb_kmstatus |= data & 0x54;

	if (((m_adb_kmstatus & (m_adb_kmstatus >> 1)) & 0x54))
		raise_irq(IRQS_ADB);
	else
		lower_irq(IRQS_ADB);
}



u8 apple2gs_state::adb_read_mousedata()
{
	u8 result = 0x00;
	u8 absolute;
	s8 delta;

	if (m_adb_kmstatus & 0x80)   // mouse register full
	{
		if (m_adb_kmstatus & 0x02)   // H/V mouse data select
		{
			absolute = m_mouse_y;
			delta = m_mouse_dy;
			m_adb_kmstatus &= ~0x82;
			if (((m_adb_kmstatus & (m_adb_kmstatus >> 1)) & 0x54) == 0)
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
	else
	{
		// no mouse axis data, so just return the button status.  used by some 3200 viewers.
		result = m_adb_mousey->read() & 0x80;
	}

	return result;
}


s8 apple2gs_state::seven_bit_diff(u8 v1, u8 v2)
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
	u8 new_mouse_x, new_mouse_y;

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
				raise_irq(IRQS_ADB);
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
	m_upperbank.select(0);
	m_upperaux.select(0);
	m_upper00.select(0);
	m_upper01.select(0);
	m_lcbank.select(0);
	m_lcaux.select(0);
	m_lc00.select(0);
	m_lc01.select(0);
	m_b0_0000bank.select(0);
	m_b0_0200bank.select(0);
	m_b0_0400bank.select(0);
	m_b0_0800bank.select(0);
	m_b0_2000bank.select(0);
	m_b0_4000bank.select(0);
	m_inh_bank = 0;
#if !RUN_ADB_MICRO
	m_transchar = 0;
	m_anykeydown = false;
#endif
	std::fill(std::begin(m_megaii_ram), std::end(m_megaii_ram), 0);

	m_nvram->set_base(m_clock_bram, sizeof(m_clock_bram));

	// setup speaker toggle volumes.  this should be done mathematically probably,
	// but these ad-hoc values aren't too bad.
#define LVL(x) (double(x) / 32768.0)
	static const double lvlTable[16] =
	{
		LVL(0x0000), LVL(0x03ff), LVL(0x04ff), LVL(0x05ff), LVL(0x06ff), LVL(0x07ff), LVL(0x08ff), LVL(0x09ff),
		LVL(0x0aff), LVL(0x0bff), LVL(0x0cff), LVL(0x0fff), LVL(0x1fff), LVL(0x3fff), LVL(0x5fff), LVL(0x7fff)
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
	m_video->m_ram_ptr = m_megaii_ram;
	m_video->m_aux_ptr = &m_megaii_ram[0x10000];
	m_video->m_char_ptr = memregion("gfx1")->base();
	m_video->m_char_size = memregion("gfx1")->bytes();
	m_video->m_8bit_graphics = std::make_unique<bitmap_ind16>(560, 192);

	m_textcol = 0xf2;
	m_video->m_GSfg = (m_textcol >> 4) & 0xf;
	m_video->m_GSbg = m_textcol & 0xf;

	m_inh_slot = -1;
	m_cnxx_slot = CNXX_UNCLAIMED;

	// install memory beyond 256K/1M
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int ramsize = m_ram_size - 0x20000; // subtract 128K for banks 0 and 1, which are handled specially

	// RAM sizes for both classes of machine no longer include the Mega II RAM
	space.install_ram(0x020000, ramsize - 1 + 0x20000, m_ram_ptr + 0x020000);

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
	save_item(NAME(m_romswitch));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_intcxrom));
	save_item(NAME(m_rombank));
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
	save_item(NAME(m_glu_mcu_read_kgs));
	save_item(NAME(m_glu_816_read_dstat));
	save_item(NAME(m_glu_mouse_read_stat));
	save_item(NAME(m_glu_kbd_y));
	save_item(NAME(m_intflag));
	save_item(NAME(m_vgcint));
	save_item(NAME(m_inten));
	save_item(NAME(m_slot_irq));
	save_item(NAME(m_slow_counter));
	save_item(NAME(m_megaii_ram));
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
	save_item(m_adb_pending_status, "ADB/m_adb_pending_status");
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
	save_item(NAME(m_anykeydown));
	save_item(NAME(m_repeatdelay));
#endif
	save_item(m_mouse_x, "MX");
	save_item(m_mouse_y, "MY");
	save_item(m_mouse_dx, "MDX");
	save_item(m_mouse_dy, "MDY");
	save_item(NAME(m_accel_unlocked));
	save_item(NAME(m_accel_stage));
	save_item(NAME(m_accel_fast));
	save_item(NAME(m_accel_present));
	save_item(NAME(m_accel_slotspk));
	save_item(NAME(m_accel_gsxsettings));
	save_item(NAME(m_accel_percent));
	save_item(NAME(m_accel_temp_slowdown));
	save_item(NAME(m_accel_speed));
	save_item(NAME(m_motoroff_time));
	save_item(NAME(m_newvideo));
}

void apple2gs_state::machine_reset()
{
	m_page2 = false;
	m_romswitch = false;
	m_video->m_page2 = false;
	m_video->m_GSborder = 0x02;
	m_video->m_GSbg = 0x02;
	m_video->m_GSfg = 0x0f;
	m_an0 = m_an1 = m_an2 = m_an3 = false;
	m_gameio->an0_w(0);
	m_gameio->an1_w(0);
	m_gameio->an2_w(0);
	m_gameio->an3_w(0);
	m_vbl = false;
	m_slotc3rom = false;
	m_irqmask = 0;
	m_intcxrom = false;
	m_rombank = false;
	m_80store = false;
	m_video->m_80store = false;
	m_altzp = false;
	m_ramrd = false;
	m_ramwrt = false;
	m_ioudis = true;
	m_newvideo = 0x01;      // verified on ROM03 hardware
	m_clock_frame = 0;
	m_mouse_x = 0x00;
	m_mouse_y = 0x00;
	m_mouse_dx = 0x00;
	m_mouse_dy = 0x00;

	m_slot_irq = false;

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

	m_motoroff_time = 0;

	m_slow_counter = 0;

	// always assert full speed on reset
	m_maincpu->set_unscaled_clock(A2GS_14M/5);
	m_last_speed = true;

	m_sndglu_ctrl = 0;
	m_sndglu_addr = 0;
	m_sndglu_dummy_read = 0;

	m_maincpu_space = &m_maincpu->space(AS_PROGRAM);

	m_b0_0000bank.select(0);
	m_b0_0200bank.select(0);
	m_b0_0400bank.select(0);
	m_b0_0800bank.select(0);
	m_b0_2000bank.select(0);
	m_b0_4000bank.select(0);
	m_bank0_atc.select(1);
	m_bank1_atc.select(1);

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

	// Setup ZipGS
	m_accel_unlocked = false;
	m_accel_stage = 0;
	m_accel_slotspk = 0x41; // speaker and slot 6 slow
	m_accel_gsxsettings = 0;
	m_accel_percent = 0;    // 100% speed
	m_accel_present = false;
	m_accel_temp_slowdown = false;
	m_accel_fast = false;

	// is Zip enabled?
	if (m_sysconfig->read() & 0x01)
	{
		static const int speeds[4] = { 7000000, 8000000, 12000000, 16000000 };
		m_accel_present = true;
		int idxSpeed = (m_sysconfig->read() >> 1);
		m_accel_speed = speeds[idxSpeed];
		m_accel_fast = true;
		accel_full_speed();
	}
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
		if ((m_accel_present) && (isfast))
		{
			accel_full_speed();
		}
		else
		{
			m_maincpu->set_unscaled_clock(isfast ? A2GS_14M / 5 : A2GS_1M);
		}
		m_last_speed = isfast;
	}
}

void apple2gs_state::accel_slot(int slot)
{
	if ((m_accel_present) && (m_accel_slotspk & (1 << slot)))
	{
		m_accel_temp_slowdown = true;
		m_acceltimer->adjust(attotime::from_msec(52));
		accel_normal_speed();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple2gs_state::accel_timer)
{
	if (m_accel_fast)
	{
		accel_full_speed();
	}
	m_accel_temp_slowdown = false;
	m_acceltimer->adjust(attotime::never);
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

		if (m_adb_state == ADBSTATE_IDLE && m_adb_pending_status)
		{
			adb_post_response_1(m_adb_pending_status);
			m_adb_pending_status = 0;
		}
		#endif
	}

	/* check scanline interrupt bits if we're in super hi-res and the current scanline is within the active display area */
	if ((m_video->m_newvideo & 0x80) && (scanline >= (BORDER_TOP-1)) && (scanline < (200+BORDER_TOP-1)))
	{
		u8 scb;
		const int shrline = scanline - BORDER_TOP + 1;

		if (shrline & 1)
		{
			scb = m_megaii_ram[0x19e80 + (shrline >> 1)];
		}
		else
		{
			scb = m_megaii_ram[0x15e80 + (shrline >> 1)];
		}

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
#if ADB_HLE
		m_macadb->adb_vblank();
#endif
		m_vbl = true;

		// check for ctrl-reset
#if !RUN_ADB_MICRO
		if ((m_kbspecial->read() & 0x88) == 0x88)
		{
			m_maincpu->reset();
		}
#endif
		// VBL interrupt
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

		// 3.5 motor off timeout
		if (m_motoroff_time > 0)
		{
			m_motoroff_time--;
			if (m_motoroff_time == 0)
			{
				if (m_floppy[2]->get_device()) m_floppy[2]->get_device()->tfsel_w(0);
				if (m_floppy[3]->get_device()) m_floppy[3]->get_device()->tfsel_w(0);
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

u32 apple2gs_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_video->screen_update_GS(screen, bitmap, cliprect);
}

/***************************************************************************
    I/O
***************************************************************************/
void apple2gs_state::auxbank_update()
{
	int ramwr = (m_ramrd ? 1 : 0) | (m_ramwrt ? 2 : 0);

	m_b0_0000bank.select(m_altzp ? 1 : 0);
	m_b0_0200bank.select(ramwr);

	if (m_80store)
	{
		if (m_page2)
		{
			m_b0_0400bank.select(3);
		}
		else
		{
			m_b0_0400bank.select(0);
		}
	}
	else
	{
		m_b0_0400bank.select(ramwr);
	}

	m_b0_0800bank.select(ramwr);

	if ((m_80store) && (m_video->m_hires))
	{
		if (m_page2)
		{
			m_b0_2000bank.select(3);
		}
		else
		{
			m_b0_2000bank.select(0);
		}
	}
	else
	{
		m_b0_2000bank.select(ramwr);
	}

	m_b0_4000bank.select(ramwr);
}

void apple2gs_state::update_slotrom_banks()
{
	//printf("update_slotrom_banks: intcxrom %d cnxx_slot %d SLOT %02x\n", m_intcxrom, m_cnxx_slot, m_slotromsel);

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
			m_lcbank.select(1);
			m_lcaux.select(1);
			m_lc00.select(1 + (m_romswitch ? 2 : 0));
			m_lc01.select(1);
		}
		else
		{
			m_lcbank.select(0);
			m_lcaux.select(0);
			m_lc00.select(0 + (m_romswitch ? 2 : 0));
			m_lc01.select(0);
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
void apple2gs_state::do_io(int offset)
{
	if(machine().side_effects_disabled()) return;

	if (m_ioudis)
	{
		switch (offset)
		{
			case 0x5e:  // SETDHIRES
				m_video->dhires_w(0);
				return;

			case 0x5f:  // CLRDHIRES
				m_video->dhires_w(1);
				return;
		}
	}

	switch (offset)
	{
		case 0x20:
			break;

		case 0x28:  //  ROMSWITCH - not used by the IIgs firmware or SSW, but does exist at least on ROM 0/1 (need to test on ROM 3 hw)
			if (!m_is_rom3)
			{
				m_romswitch = !m_romswitch;
				if (m_lcram)
				{
					m_lc00.select(1 + (m_romswitch ? 2 : 0));
				}
				else
				{
					m_lc00.select(0 + (m_romswitch ? 2 : 0));
				}
			}
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

			if ((m_accel_present) && (m_accel_slotspk & 1))
			{
				m_accel_temp_slowdown = true;
				m_acceltimer->adjust(attotime::from_msec(5));
				accel_normal_speed();
			}
			break;

		case 0x50:  // graphics mode
			m_video->txt_w(0);
			break;

		case 0x51:  // text mode
			m_video->txt_w(1);
			break;

		case 0x52:  // no mix
			m_video->mix_w(0);
			break;

		case 0x53:  // mixed mode
			m_video->mix_w(1);
			break;

		case 0x54:  // set page 1
			m_page2 = false;
			m_video->scr_w(0);
			auxbank_update();
			break;

		case 0x55:  // set page 2
			m_page2 = true;
			m_video->scr_w(1);
			auxbank_update();
			break;

		case 0x56: // select lo-res
			m_video->res_w(0);
			auxbank_update();
			break;

		case 0x57: // select hi-res
			m_video->res_w(1);
			auxbank_update();
			break;

		case 0x58: // AN0 off
			m_an0 = false;
			m_gameio->an0_w(0);
			break;

		case 0x59: // AN0 on
			m_an0 = true;
			m_gameio->an0_w(1);
			break;

		case 0x5a: // AN1 off
			m_an1 = false;
			m_gameio->an1_w(0);
			break;

		case 0x5b: // AN1 on
			m_an1 = true;
			m_gameio->an1_w(1);
			break;

		case 0x5c: // AN2 off
			m_an2 = false;
			m_gameio->an2_w(0);
			break;

		case 0x5d: // AN2 on
			m_an2 = true;
			m_gameio->an2_w(1);
			break;

		case 0x5e: // AN3 off
			m_an3 = false;
			m_gameio->an3_w(0);
			break;

		case 0x5f: // AN3 on
			m_an3 = true;
			m_gameio->an3_w(1);
			break;

		case 0x68:  // STATE
			break;

		// trigger joypad read
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			// Zip paddle slowdown (does ZipGS also use the old Zip flag?)
			if ((m_accel_present) && !BIT(m_accel_gsxsettings, 6))
			{
				m_accel_temp_slowdown = true;
				m_acceltimer->adjust(attotime::from_msec(5));
				accel_normal_speed();
			}

			// 558 monostable one-shot timers; a running timer cannot be restarted
			if (machine().time().as_double() >= m_joystick_x1_time)
			{
				m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
			}
			if (machine().time().as_double() >= m_joystick_y1_time)
			{
				m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
			}
			if (machine().time().as_double() >= m_joystick_x2_time)
			{
				m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
			}
			if (machine().time().as_double() >= m_joystick_y2_time)
			{
				m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();
			}
			break;

		default:
			logerror("do_io: unknown switch $C0%02X\n", offset);
			break;
	}
}

// apple2gs_get_vpos - return the correct vertical counter value for the current scanline.
int apple2gs_state::get_vpos()
{
	// as per IIgs Tech Note #39, this is simply scanline + 250 on NTSC (262 lines),
	// or scanline + 200 on PAL (312 lines)
	int vpos = m_screen->vpos() + (511 - BORDER_TOP + 6);
	if (vpos > 511)
	{
		vpos -= (511 - 250);
	}
	return vpos;
}

void apple2gs_state::process_clock()
{
	u8 operation;

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

u8 apple2gs_state::c000_r(offs_t offset)
{
	u8 ret;

	// allow ROM window at C07x to be debugger-visible
	if ((offset < 0x70) || (offset > 0x7f))
	{
		if(machine().side_effects_disabled()) return read_floatingbus();
	}

	slow_cycle();
	u8 uFloatingBus7 = read_floatingbus() & 0x7f;
#if RUN_ADB_MICRO
	u8 uKeyboard = keyglu_816_read(GLU_C000);
	u8 uKeyboardC010 = keyglu_816_read(GLU_C010);
#else
	u8 uKeyboard = m_transchar | m_strobe;
	u8 uKeyboardC010 = m_transchar;
#endif

	switch (offset)
	{
		case 0x00:  // keyboard latch
			return uKeyboard;

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
#if !RUN_ADB_MICRO
			m_strobe = 0;
			return uKeyboardC010 | (m_anykeydown ? 0x80 : 0x00);
#else
			keyglu_816_write(GLU_C010, 0);
			return uKeyboardC010;
#endif
		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return uKeyboardC010 | (m_lcram2 ? 0x80 : 0x00);

		case 0x12:  // read LCRAM (is LC readable?)
			return uKeyboardC010 | (m_lcram ? 0x80 : 0x00);

		case 0x13:  // read RAMRD
			return uKeyboardC010 | (m_ramrd ? 0x80 : 0x00);

		case 0x14:  // read RAMWRT
			return uKeyboardC010 | (m_ramwrt ? 0x80 : 0x00);

		case 0x15:  // read INTCXROM
			return uKeyboardC010 | (m_intcxrom ? 0x80 : 0x00);

		case 0x16:  // read ALTZP
			return uKeyboardC010 | (m_altzp ? 0x80 : 0x00);

		case 0x17:  // read SLOTC3ROM
			return uKeyboardC010 | (m_slotc3rom ? 0x80 : 0x00);

		case 0x18:  // read 80STORE
			return uKeyboardC010 | (m_80store ? 0x80 : 0x00);

		case 0x19:  // read VBLBAR
			return uKeyboardC010 | (m_screen->vblank() ? 0x00 : 0x80);

		case 0x1a:  // read TEXT
			return uKeyboardC010 | (m_video->m_graphics ? 0x00 : 0x80);

		case 0x1b:  // read MIXED
			return uKeyboardC010 | (m_video->m_mix ? 0x80 : 0x00);

		case 0x1c:  // read PAGE2
			return uKeyboardC010 | (m_page2 ? 0x80 : 0x00);

		case 0x1d:  // read HIRES
			return uKeyboardC010 | (m_video->m_hires ? 0x80 : 0x00);

		case 0x1e:  // read ALTCHARSET
			return uKeyboardC010 | (m_video->m_altcharset ? 0x80 : 0x00);

		case 0x1f:  // read 80COL
			return uKeyboardC010 | (m_video->m_80col ? 0x80 : 0x00);

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
				u8 temp = m_kbspecial->read();
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
					ret |= 0x80;
				}
				if (temp & 0x20)    // option
				{
					ret |= 0x40;
				}
				// keypad is a little rough right now
				if (m_lastchar >= 0x28 && m_lastchar <= 0x2d)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x32 && m_lastchar <= 0x37)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x3c && m_lastchar <= 0x3f)
				{
					ret |= 0x10;
				}
				else if (m_lastchar >= 0x100 && m_lastchar <= 0x101)
				{
					ret |= 0x10;
				}
				else if ((m_lastchar >= 0x109 && m_lastchar <= 0x10a) || (m_lastchar == 0x106))
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
			// secondary hack for slot IRQs
			if (m_slot_irq)
			{
				m_slot_irq = false;
				return 0;
			}

			return adb_read_kmstatus();
#endif

		case 0x29:  // NEWVIDEO
			return m_newvideo;

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

		case 0x32: // VGCINTCLEAR
			return 0;

		case 0x33: // CLOCKDATA
			return m_clkdata;

		case 0x34:  // BORDERCOL
			return (m_clock_control & 0xf0) | (m_video->m_GSborder & 0xf);

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
				m_sndglu_dummy_read = m_doc->read(m_sndglu_addr);
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

		case 0x47:  // CLRVBLINT
			m_intflag &= ~(INTFLAG_VBL|INTFLAG_QUARTER);
			lower_irq(IRQS_VBL);
			lower_irq(IRQS_QTRSEC);
			return read_floatingbus();

		case 0x60: // button 3 on IIgs
			return m_gameio->sw3_r() | uFloatingBus7;
#if RUN_ADB_MICRO
		case 0x61: // button 0 or Open Apple
			return m_gameio->sw0_r() | uFloatingBus7;

		case 0x62: // button 1 or Option
			return m_gameio->sw1_r() | uFloatingBus7;

		case 0x63: // button 2 or SHIFT key
			return m_gameio->sw2_r() | uFloatingBus7;
#else
		case 0x61:  // button 0 or Open Apple
			return ((m_gameio->sw0_r() || (m_kbspecial->read() & 0x10)) ? 0x80 : 0) | uFloatingBus7;

		case 0x62:  // button 1 or Option
			return ((m_gameio->sw1_r() || (m_kbspecial->read() & 0x20)) ? 0x80 : 0) | uFloatingBus7;

		case 0x63:  // button 2 or SHIFT key
			return ((m_gameio->sw2_r() || (m_kbspecial->read() & 0x06)) ? 0x80 : 0) | uFloatingBus7;
#endif
		case 0x64:  // joy 1 X axis
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x65:  // joy 1 Y axis
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x66: // joy 2 X axis
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x67: // joy 2 Y axis
			if (!m_gameio->is_device_connected()) return 0x80 | uFloatingBus7;
			return ((machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0) | uFloatingBus7;

		case 0x68: // STATEREG, synthesizes all the IIe state regs
			return  (m_altzp ? 0x80 : 0x00) |
					(m_page2 ? 0x40 : 0x00) |
					(m_ramrd ? 0x20 : 0x00) |
					(m_ramwrt ? 0x10 : 0x00) |
					(m_lcram ? 0x00 : 0x08) |
					(m_lcram2 ? 0x04 : 0x00) |
					(m_rombank ? 0x02 : 0x00) |
					(m_intcxrom ? 0x01 : 0x00);

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			// todo: does reading these on the IIgs also trigger the joysticks?
			if (!machine().side_effects_disabled())
			{
				// Zip paddle slowdown (does ZipGS also use the old Zip flag?)
				if ((m_accel_present) && !BIT(m_accel_gsxsettings, 6))
				{
					m_accel_temp_slowdown = true;
					m_acceltimer->adjust(attotime::from_msec(5));
					accel_normal_speed();
				}

				// 558 monostable one-shot timers; a running timer cannot be restarted
				if (machine().time().as_double() >= m_joystick_x1_time)
				{
					m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
				}
				if (machine().time().as_double() >= m_joystick_y1_time)
				{
					m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
				}
				if (machine().time().as_double() >= m_joystick_x2_time)
				{
					m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
				}
				if (machine().time().as_double() >= m_joystick_y2_time)
				{
					m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();
				}

			}

			return m_rom[offset + 0x3c000];
			break;

		default:
			do_io(offset);

			if (m_accel_unlocked)
			{
				if (offset == 0x59)
				{
					return m_accel_gsxsettings;
				}
				else if (offset == 0x5a)
				{
					return m_accel_percent | 0x0f;
				}
				else if (offset == 0x5b)
				{
					// bit 7 is a 1.0035 millisecond clock; the value changes every 0.50175 milliseconds
					const int time = machine().time().as_ticks(1.0f / 0.00050175f);
					if (time & 1)
					{
						return 0x03;
					}
					else
					{
						return 0x83;
					}
				}
				else if (offset == 0x5c)
				{
					return m_accel_slotspk;
				}
			}
			break;
	}

	// assume all $C00X returns the keyboard, like on the IIe
	if ((offset & 0xf0) == 0x00)
	{
		return uKeyboard;
	}

	return read_floatingbus();
}

void apple2gs_state::c000_w(offs_t offset, u8 data)
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
			if (m_textcol != data)
			{
				m_screen->update_now();
			}
			m_textcol = data;
			m_video->m_GSfg = (data >> 4) & 0xf;
			m_video->m_GSbg = data & 0xf;
			break;

		case 0x23:  // VGCINT
			if ((m_vgcint & VGCINT_SECOND) && !(data & VGCINT_SECONDENABLE))
			{
				lower_irq(IRQS_SECOND);
				m_vgcint &= ~(VGCINT_SECOND);
			}
			if ((m_vgcint & VGCINT_SCANLINE) && !(data & VGCINT_SCANLINEEN))
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
			m_video->m_newvideo = m_newvideo = data;
			break;

		case 0x2d:  // SLOTROMSEL
			m_slotromsel = data;
			break;
		case 0x31:  //
			if (!BIT(data, DISKREG_35SEL))
			{
				m_motoroff_time = 30;
			}

			if ((m_cur_floppy) && (BIT(data, DISKREG_35SEL)))
			{
				m_cur_floppy->ss_w(BIT(data, DISKREG_HDSEL));
			}
			m_diskreg = data;
			break;

		case 0x32:  // VGCINTCLEAR
			//printf("%02x to VGCINTCLEAR\n", data);
			// one second
			if (m_vgcint & VGCINT_SECOND)
			{
				lower_irq(IRQS_SECOND);
				m_vgcint &= ~(VGCINT_SECOND|VGCINT_ANYVGCINT);
			}

			// scanline
			if (m_vgcint & VGCINT_SCANLINE)
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
			if ((data & 0xf) != (m_video->m_GSborder & 0xf))
			{
				m_screen->update_now();
			}
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
				m_bank0_atc.select(0);
				m_bank1_atc.select(0);
			}
			else
			{
				m_bank0_atc.select(1);
				m_bank1_atc.select(1);

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
				m_doc->write(m_sndglu_addr, data);
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
			m_intflag &= ~(INTFLAG_VBL|INTFLAG_QUARTER);
			lower_irq(IRQS_VBL);
			lower_irq(IRQS_QTRSEC);
			break;

		case 0x59: // Zip GS-specific settings
			if (m_accel_unlocked)
			{
				m_accel_gsxsettings = data & 0xf8;
				m_accel_gsxsettings |= 0x01;    // indicate this is a GS
			}
			break;

		case 0x5a: // Zip accelerator unlock
			if (m_sysconfig->read() & 0x01)
			{
				if (data == 0x5a)
				{
					m_accel_stage++;
					if (m_accel_stage == 4)
					{
						m_accel_unlocked = true;
					}
				}
				else if (data == 0xa5)
				{
					// lock
					m_accel_unlocked = false;
					m_accel_stage = 0;
				}
				else if (m_accel_unlocked)
				{
					// disable acceleration
					accel_normal_speed();
					m_accel_unlocked = false;
					m_accel_stage = 0;
				}
			}
			do_io(offset);
			break;

		case 0x5b: // Zip full speed
			if (m_accel_unlocked)
			{
				accel_full_speed();
			}
			do_io(offset);
			break;

		case 0x5c: // Zip slot/speaker flags
			if (m_accel_unlocked)
			{
				m_accel_slotspk = data;
			}
			do_io(offset);
			break;

		case 0x5d: // Zip speed percentage
			if (m_accel_unlocked)
			{
				m_accel_percent = data;
			}
			break;

		case 0x68: // STATEREG
			m_altzp = (data & 0x80);
			m_page2 = (data & 0x40);
			m_ramrd = (data & 0x20);
			m_ramwrt = (data & 0x10);
			m_lcram = (data & 0x08) ? false : true;
			m_lcram2 = (data & 0x04);
			m_rombank = (data & 0x02);
			m_intcxrom = (data & 0x01);

			// update the aux state
			auxbank_update();

			// update LC state
			if (m_lcram)
			{
				m_lcbank.select(1);
				m_lcaux.select(1);
				m_lc00.select(1);
				m_lc01.select(1);
			}
			else
			{
				m_lcbank.select(0);
				m_lcaux.select(0);
				m_lc00.select(0);
				m_lc01.select(0);
			}
			break;

		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
			do_io(offset); // make sure it also side-effect resets the paddles as documented
			break;

		case 0x7e: // SETIOUDIS
			m_ioudis = true;
			break;

		case 0x7f: // CLRIOUDIS
			m_ioudis = false;
			break;

		default:
			do_io(offset);
			break;
		}
}

u8 apple2gs_state::c080_r(offs_t offset)
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
			if (m_accel_present)
			{
				accel_slot(slot);
			}

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

void apple2gs_state::c080_w(offs_t offset, u8 data)
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

u8 apple2gs_state::read_slot_rom(int slotbias, int offset)
{
	const int slotnum = ((offset>>8) & 0xf) + slotbias;

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

void apple2gs_state::write_slot_rom(int slotbias, int offset, u8 data)
{
	const int slotnum = ((offset>>8) & 0xf) + slotbias;

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

u8 apple2gs_state::read_int_rom(int slotbias, int offset)
{
	if ((m_cnxx_slot == CNXX_UNCLAIMED) && (!machine().side_effects_disabled()))
	{
		m_cnxx_slot = CNXX_INTROM;
		update_slotrom_banks();
	}

	return m_rom[slotbias + offset];
}

u8 apple2gs_state::c100_r(offs_t offset)
{
	const int slot = ((offset>>8) & 0xf) + 1;

	slow_cycle();

	accel_slot(slot);

	// SETSLOTCXROM is disabled, so the $C02D SLOT register controls what's in each slot
	if (!(m_slotromsel & (1 << slot)))
	{
		return read_int_rom(0x3c100, offset);
	}

	return read_slot_rom(1, offset);
}

void apple2gs_state::c100_w(offs_t offset, u8 data)
{
	const int slot = ((offset>>8) & 0xf) + 1;

	accel_slot(slot);

	slow_cycle();

	if ((m_slotromsel & (1 << slot)))
	{
		write_slot_rom(1, offset, data);
	}
}

u8 apple2gs_state::c300_int_r(offs_t offset)  { accel_slot(3 + ((offset >> 8) & 0x7)); slow_cycle(); return read_int_rom(0x3c300, offset); }
u8 apple2gs_state::c300_r(offs_t offset)  { accel_slot(3 + ((offset >> 8) & 0x7)); slow_cycle(); return read_slot_rom(3, offset); }
void apple2gs_state::c300_w(offs_t offset, u8 data) { accel_slot(3 + ((offset >> 8) & 0x7)); slow_cycle(); write_slot_rom(3, offset, data); }

u8 apple2gs_state::c400_r(offs_t offset)
{
	const int slot = ((offset>>8) & 0xf) + 4;

	accel_slot(slot);

	slow_cycle();

	if (!(m_slotromsel & (1 << slot)))
	{
		return read_int_rom(0x3c400, offset);
	}

	return read_slot_rom(4, offset);
}

void apple2gs_state::c400_w(offs_t offset, u8 data)
{
	const int slot = ((offset>>8) & 0xf) + 4;

	accel_slot(slot);

	slow_cycle();

	if ((m_slotromsel & (1 << slot)))
	{
		write_slot_rom(4, offset, data);
	}
}

u8 apple2gs_state::c800_r(offs_t offset)
{
	slow_cycle();

	if ((offset == 0x7ff) && !machine().side_effects_disabled())
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return 0xff;
	}

	if (m_cnxx_slot == CNXX_INTROM)
	{
		return m_rom[offset + 0x3c800];
	}

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		return m_slotdevice[m_cnxx_slot]->read_c800(offset&0xfff);
	}

	return 0xff;
}

void apple2gs_state::c800_w(offs_t offset, u8 data)
{
	slow_cycle();

	if ((m_cnxx_slot > 0) && (m_slotdevice[m_cnxx_slot] != nullptr))
	{
		m_slotdevice[m_cnxx_slot]->write_c800(offset&0xfff, data);
	}

	if (offset == 0x7ff)
	{
		m_cnxx_slot = CNXX_UNCLAIMED;
		update_slotrom_banks();
		return;
	}
}

u8 apple2gs_state::inh_r(offs_t offset)
{
	if (m_inh_slot != -1)
	{
		return m_slotdevice[m_inh_slot]->read_inh_rom(offset + 0xd000);
	}

	assert(0);  // hitting inh_r with invalid m_inh_slot should not be possible
	return read_floatingbus();
}

void apple2gs_state::inh_w(offs_t offset, u8 data)
{
	if (m_inh_slot != -1)
	{
		m_slotdevice[m_inh_slot]->write_inh_rom(offset + 0xd000, data);
	}
}

u8 apple2gs_state::lc_r(offs_t offset)
{
	slow_cycle();
	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			return m_megaii_ram[(offset & 0xfff) + 0xc000];
		}
		else
		{
			return m_megaii_ram[(offset & 0xfff) + 0xd000];
		}
	}

	return m_megaii_ram[(offset & 0x1fff) + 0xe000];
}

void apple2gs_state::lc_w(offs_t offset, u8 data)
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
			m_megaii_ram[(offset & 0xfff) + 0xc000] = data;
		}
		else
		{
			m_megaii_ram[(offset & 0xfff) + 0xd000] = data;
		}
		return;
	}

	m_megaii_ram[(offset & 0x1fff) + 0xe000] = data;
}

u8 apple2gs_state::lc_aux_r(offs_t offset)
{
	slow_cycle();
	if (offset < 0x1000)
	{
		if (m_lcram2)
		{
			return m_megaii_ram[(offset & 0xfff) + 0x1c000];
		}
		else
		{
			return m_megaii_ram[(offset & 0xfff) + 0x1d000];
		}
	}

	return m_megaii_ram[(offset & 0x1fff) + 0x1e000];
}

void apple2gs_state::lc_aux_w(offs_t offset, u8 data)
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
			m_megaii_ram[(offset & 0xfff) + 0x1c000] = data;
		}
		else
		{
			m_megaii_ram[(offset & 0xfff) + 0x1d000] = data;
		}
		return;
	}

	m_megaii_ram[(offset & 0x1fff) + 0x1e000] = data;
}

u8 apple2gs_state::lc_00_r(offs_t offset)
{
	if (m_altzp)
	{
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
	else
	{
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
}

void apple2gs_state::lc_00_w(offs_t offset, u8 data)
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
	else
	{
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
}

u8 apple2gs_state::lc_01_r(offs_t offset)
{
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

void apple2gs_state::lc_01_w(offs_t offset, u8 data)
{
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

// floating bus code from old machine/apple2: needs to be reworked based on real beam position to enable e.g. Bob Bishop's screen splitter
u8 apple2gs_state::read_floatingbus()
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

	return m_megaii_ram[address % m_ram_size]; // FIX: this seems to work, but is it right!?
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

u8 apple2gs_state::ram0000_r(offs_t offset)  { slow_cycle(); return m_megaii_ram[offset]; }
void apple2gs_state::ram0000_w(offs_t offset, u8 data) { slow_cycle(); m_megaii_ram[offset] = data; }
u8 apple2gs_state::auxram0000_r(offs_t offset)
{
	slow_cycle();
	if ((offset >= 0x2000) && (offset < 0xa000) && ((m_newvideo & 0xc0) != 0))
	{
		if (offset & 1)
		{
			offset = ((offset - 0x2000) >> 1) + 0x6000;
		}
		else
		{
			offset = ((offset - 0x2000) >> 1) + 0x2000;
		}
	}
	return m_megaii_ram[offset+0x10000];
}

void apple2gs_state::auxram0000_w(offs_t offset, u8 data)
{
	u16 orig_addr = offset;

	slow_cycle();

	if ((offset >= 0x2000) && (offset < 0xa000) && ((m_newvideo & 0xc0) != 0))
	{
		if (offset & 1)
		{
			offset = ((offset - 0x2000) >> 1) + 0x6000;
		}
		else
		{
			offset = ((offset - 0x2000) >> 1) + 0x2000;
		}
	}

	m_megaii_ram[offset+0x10000] = data;

	if ((orig_addr >= 0x9e00) && (orig_addr <= 0x9fff))
	{
		int color = (orig_addr - 0x9e00) >> 1;

		m_video->m_shr_palette[color] = rgb_t(
			((m_megaii_ram[0x19f00 + color] >> 0) & 0x0f) * 17,
			((m_megaii_ram[0x15f00 + color] >> 4) & 0x0f) * 17,
			((m_megaii_ram[0x15f00 + color] >> 0) & 0x0f) * 17);
	}
}

u8 apple2gs_state::b0ram0000_r(offs_t offset)  { return m_ram_ptr[offset]; }
void apple2gs_state::b0ram0000_w(offs_t offset, u8 data) { m_ram_ptr[offset] = data; }
u8 apple2gs_state::b0ram0200_r(offs_t offset)  { return m_ram_ptr[offset+0x200]; }
void apple2gs_state::b0ram0200_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x200] = data; }
u8 apple2gs_state::b0ram0400_r(offs_t offset)  { return m_ram_ptr[offset+0x400]; }
void apple2gs_state::b0ram0400_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x400] = data;
	if (!(m_shadow & SHAD_TXTPG1))
	{
		slow_cycle();
		m_megaii_ram[offset+0x0400] = data;
	}
}
u8 apple2gs_state::b0ram0800_r(offs_t offset)  { return m_ram_ptr[offset+0x800]; }
void apple2gs_state::b0ram0800_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x800] = data;

	if (offset < 0x400)
	{
		if ((!(m_shadow & SHAD_TXTPG2)) && (m_is_rom3))
		{
			slow_cycle();
			m_megaii_ram[offset+0x800] = data;
		}
	}
}
u8 apple2gs_state::b0ram2000_r(offs_t offset)  { return m_ram_ptr[offset+0x2000]; }
void apple2gs_state::b0ram2000_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x2000] = data;
	if (!(m_shadow & SHAD_HIRESPG1))
	{
		slow_cycle();
		m_megaii_ram[offset+0x2000] = data;
	}
}
u8 apple2gs_state::b0ram4000_r(offs_t offset)  { return m_ram_ptr[offset+0x4000]; }
void apple2gs_state::b0ram4000_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x4000] = data;
	if (offset < 0x2000)
	{
		if (!(m_shadow & SHAD_HIRESPG2))
		{
			slow_cycle();
			m_megaii_ram[offset+0x4000] = data;
		}
	}
}

u8 apple2gs_state::b1ram0000_r(offs_t offset)  { return m_ram_ptr[offset+0x10000]; }
void apple2gs_state::b1ram0000_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x10000] = data; }
u8 apple2gs_state::b1ram0200_r(offs_t offset)  { return m_ram_ptr[offset+0x10200]; }
void apple2gs_state::b1ram0200_w(offs_t offset, u8 data) { m_ram_ptr[offset+0x10200] = data; }
u8 apple2gs_state::b1ram0400_r(offs_t offset)  { return m_ram_ptr[offset+0x10400]; }
void apple2gs_state::b1ram0400_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x10400] = data;
	if (!(m_shadow & SHAD_TXTPG1))
	{
		slow_cycle();
		m_megaii_ram[offset+0x10400] = data;
	}
}
u8 apple2gs_state::b1ram0800_r(offs_t offset) { return m_ram_ptr[offset+0x10800]; }
void apple2gs_state::b1ram0800_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x10800] = data;
	if (offset < 0x400)
	{
		slow_cycle();
		m_megaii_ram[offset+0x10800] = data;
	}
}
u8 apple2gs_state::b1ram2000_r(offs_t offset)  { return m_ram_ptr[offset+0x12000]; }
void apple2gs_state::b1ram2000_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x12000] = data;
	if ((!(m_shadow & SHAD_HIRESPG1) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
	{
		auxram0000_w(offset+0x2000, data);
	}
}
u8 apple2gs_state::b1ram4000_r(offs_t offset)  { return m_ram_ptr[offset+0x14000]; }
void apple2gs_state::b1ram4000_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset+0x14000] = data;
	if (offset < 0x2000)
	{
		if ((!(m_shadow & SHAD_HIRESPG2) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
		{
			auxram0000_w(offset+0x4000, data);
		}
	}
	else if ((offset >= 0x2000) && (offset <= 0x5fff))
	{
		if (!(m_shadow & SHAD_SUPERHIRES))
		{
			auxram0000_w(offset+0x4000, data);
		}
	}
}

u8 apple2gs_state::bank0_c000_r(offs_t offset)
{
	if (offset & 0x2000)
	{
		offset ^= 0x1000;
	}

	if (m_ramrd)
	{
		return m_ram_ptr[offset + 0x1c000];
	}

	return m_ram_ptr[offset + 0xc000];
}

void apple2gs_state::bank0_c000_w(offs_t offset, u8 data)
{
	if (offset & 0x2000)
	{
		offset ^= 0x1000;
	}

	if (m_ramwrt)
	{
		m_ram_ptr[offset + 0x1c000] = data;
		return;
	}

	m_ram_ptr[offset + 0xc000] = data;
}

u8 apple2gs_state::bank1_0000_r(offs_t offset) { return m_ram_ptr[offset + 0x10000]; }
u8 apple2gs_state::bank1_c000_r(offs_t offset) { if (offset & 0x2000) offset ^= 0x1000; return m_ram_ptr[offset + 0x1c000]; }
void apple2gs_state::bank1_c000_w(offs_t offset, u8 data) { if (offset & 0x2000) offset ^= 0x1000; m_ram_ptr[offset + 0x1c000] = data; }
void apple2gs_state::bank1_0000_sh_w(offs_t offset, u8 data)
{
	m_ram_ptr[offset + 0x10000] = data;

	switch (offset>>8)
	{
		case 0x04:  // text page 1
		case 0x05:
		case 0x06:
		case 0x07:
			if (!(m_shadow & SHAD_TXTPG1))
			{
				slow_cycle();
				m_megaii_ram[offset + 0x10000] = data;
			}
			break;

		case 0x08:  // text page 2 (only shadowable on ROM 03)
		case 0x09:
		case 0x0a:
		case 0x0b:
			if ((!(m_shadow & SHAD_TXTPG2)) && (m_is_rom3))
			{
				slow_cycle();
				m_megaii_ram[offset + 0x10000] = data;
			}
			break;

			// hi-res page 1
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			if ((!(m_shadow & SHAD_HIRESPG1) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
			{
				auxram0000_w(offset, data);
			}
			break;

			// hi-res page 2
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			if ((!(m_shadow & SHAD_HIRESPG2) && !(m_shadow & SHAD_AUXHIRES)) || (!(m_shadow & SHAD_SUPERHIRES)))
			{
				auxram0000_w(offset, data);
			}
			break;

		default:
			if ((offset >= 0x6000) && (offset <= 0x9fff))
			{
				if (!(m_shadow & SHAD_SUPERHIRES))
				{
					auxram0000_w(offset, data);
				}
			}
			break;
	}
}

void apple2gs_state::apple2gs_map(address_map &map)
{
	/* "fast side" - runs 2.8 MHz minus RAM refresh, banks 00 and 01 usually have writes shadowed to E0/E1 where I/O lives */
	/* Banks 00 and 01 also have their own independent language cards which are NOT shadowed. */
	map(0x000000, 0x0001ff).view(m_b0_0000bank);
	m_b0_0000bank[0](0x0000, 0x01ff).rw(FUNC(apple2gs_state::b0ram0000_r), FUNC(apple2gs_state::b0ram0000_w));
	m_b0_0000bank[1](0x0000, 0x01ff).rw(FUNC(apple2gs_state::b1ram0000_r), FUNC(apple2gs_state::b1ram0000_w));

	map(0x000200, 0x0003ff).view(m_b0_0200bank);
	m_b0_0200bank[0](0x0200, 0x03ff).rw(FUNC(apple2gs_state::b0ram0200_r), FUNC(apple2gs_state::b0ram0200_w)); // wr 0 rd 0
	m_b0_0200bank[1](0x0200, 0x03ff).rw(FUNC(apple2gs_state::b1ram0200_r), FUNC(apple2gs_state::b0ram0200_w)); // wr 0 rd 1
	m_b0_0200bank[2](0x0200, 0x03ff).rw(FUNC(apple2gs_state::b0ram0200_r), FUNC(apple2gs_state::b1ram0200_w)); // wr 1 rd 0
	m_b0_0200bank[3](0x0200, 0x03ff).rw(FUNC(apple2gs_state::b1ram0200_r), FUNC(apple2gs_state::b1ram0200_w)); // wr 1 rd 1

	map(0x000400, 0x0007ff).view(m_b0_0400bank);
	m_b0_0400bank[0](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b0ram0400_r), FUNC(apple2gs_state::b0ram0400_w)); // wr 0 rd 0
	m_b0_0400bank[1](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b1ram0400_r), FUNC(apple2gs_state::b0ram0400_w));  // wr 0 rd 1
	m_b0_0400bank[2](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b0ram0400_r), FUNC(apple2gs_state::b1ram0400_w));  // wr 1 rd 0
	m_b0_0400bank[3](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b1ram0400_r), FUNC(apple2gs_state::b1ram0400_w)); // wr 1 rd 1

	map(0x000800, 0x001fff).view(m_b0_0800bank);
	m_b0_0800bank[0](0x0800, 0x1fff).rw(FUNC(apple2gs_state::b0ram0800_r), FUNC(apple2gs_state::b0ram0800_w));
	m_b0_0800bank[1](0x0800, 0x1fff).rw(FUNC(apple2gs_state::b1ram0800_r), FUNC(apple2gs_state::b0ram0800_w));
	m_b0_0800bank[2](0x0800, 0x1fff).rw(FUNC(apple2gs_state::b0ram0800_r), FUNC(apple2gs_state::b1ram0800_w));
	m_b0_0800bank[3](0x0800, 0x1fff).rw(FUNC(apple2gs_state::b1ram0800_r), FUNC(apple2gs_state::b1ram0800_w));

	map(0x002000, 0x003fff).view(m_b0_2000bank);
	m_b0_2000bank[0](0x2000, 0x3fff).rw(FUNC(apple2gs_state::b0ram2000_r), FUNC(apple2gs_state::b0ram2000_w));
	m_b0_2000bank[1](0x2000, 0x3fff).rw(FUNC(apple2gs_state::b1ram2000_r), FUNC(apple2gs_state::b0ram2000_w));
	m_b0_2000bank[2](0x2000, 0x3fff).rw(FUNC(apple2gs_state::b0ram2000_r), FUNC(apple2gs_state::b1ram2000_w));
	m_b0_2000bank[3](0x2000, 0x3fff).rw(FUNC(apple2gs_state::b1ram2000_r), FUNC(apple2gs_state::b1ram2000_w));

	map(0x004000, 0x00bfff).view(m_b0_4000bank);
	m_b0_4000bank[0](0x4000, 0xbfff).rw(FUNC(apple2gs_state::b0ram4000_r), FUNC(apple2gs_state::b0ram4000_w));
	m_b0_4000bank[1](0x4000, 0xbfff).rw(FUNC(apple2gs_state::b1ram4000_r), FUNC(apple2gs_state::b0ram4000_w));
	m_b0_4000bank[2](0x4000, 0xbfff).rw(FUNC(apple2gs_state::b0ram4000_r), FUNC(apple2gs_state::b1ram4000_w));
	m_b0_4000bank[3](0x4000, 0xbfff).rw(FUNC(apple2gs_state::b1ram4000_r), FUNC(apple2gs_state::b1ram4000_w));

	map(0x00c000, 0x00ffff).view(m_bank0_atc);
	m_bank0_atc[0](0xc000, 0xffff).rw(FUNC(apple2gs_state::bank0_c000_r), FUNC(apple2gs_state::bank0_c000_w));
	m_bank0_atc[1](0xc000, 0xffff).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	m_bank0_atc[1](0xc080, 0xffff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	m_bank0_atc[1](0xc100, 0xffff).rw(FUNC(apple2gs_state::c100_r), FUNC(apple2gs_state::c100_w));
	m_bank0_atc[1](0xc300, 0xffff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	m_bank0_atc[1](0xc400, 0xffff).rw(FUNC(apple2gs_state::c400_r), FUNC(apple2gs_state::c400_w));
	m_bank0_atc[1](0xc800, 0xffff).rw(FUNC(apple2gs_state::c800_r), FUNC(apple2gs_state::c800_w));
	m_bank0_atc[1](0xd000, 0xffff).view(m_upper00);

	m_upper00[0](0xd000, 0xffff).view(m_lc00);
	m_upper00[1](0xd000, 0xffff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));

	m_lc00[0](0xd000, 0xffff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_00_w));
	m_lc00[1](0xd000, 0xffff).rw(FUNC(apple2gs_state::lc_00_r), FUNC(apple2gs_state::lc_00_w));
	m_lc00[2](0xd000, 0xffff).rom().region("maincpu", 0x39000).w(FUNC(apple2gs_state::lc_00_w));
	m_lc00[3](0xd000, 0xffff).rw(FUNC(apple2gs_state::lc_00_r), FUNC(apple2gs_state::lc_00_w));

	map(0x010000, 0x01bfff).rw(FUNC(apple2gs_state::bank1_0000_r), FUNC(apple2gs_state::bank1_0000_sh_w));

	map(0x01c000, 0x01ffff).view(m_bank1_atc);
	m_bank1_atc[0](0x1c000, 0x1ffff).rw(FUNC(apple2gs_state::bank1_c000_r), FUNC(apple2gs_state::bank1_c000_w));
	m_bank1_atc[1](0x1c000, 0x1c07f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	m_bank1_atc[1](0x1c080, 0x1c0ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	m_bank1_atc[1](0x1c100, 0x1c2ff).rw(FUNC(apple2gs_state::c100_r), FUNC(apple2gs_state::c100_w));
	m_bank1_atc[1](0x1c300, 0x1c3ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	m_bank1_atc[1](0x1c400, 0x1c7ff).rw(FUNC(apple2gs_state::c400_r), FUNC(apple2gs_state::c400_w));
	m_bank1_atc[1](0x1c800, 0x1cfff).rw(FUNC(apple2gs_state::c800_r), FUNC(apple2gs_state::c800_w));

	m_bank1_atc[1](0x1d000, 0x1ffff).view(m_upper01);
	m_upper01[0](0x1d000, 0x1ffff).view(m_lc01);
	m_upper01[1](0x1d000, 0x1ffff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));

	m_lc01[0](0x1d000, 0x1ffff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_01_w));
	m_lc01[1](0x1d000, 0x1ffff).rw(FUNC(apple2gs_state::lc_01_r), FUNC(apple2gs_state::lc_01_w));

	/* "Mega II side" - this is basically a 128K IIe on a chip that runs merrily at 1 MHz */
	/* Unfortunately all I/O happens here, including new IIgs-specific stuff */
	map(0xe00000, 0xe0bfff).rw(FUNC(apple2gs_state::ram0000_r), FUNC(apple2gs_state::ram0000_w));
	map(0xe0c000, 0xe0c07f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0xe0c080, 0xe0c0ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0xe0c100, 0xe0c2ff).rw(FUNC(apple2gs_state::c100_r), FUNC(apple2gs_state::c100_w));
	map(0xe0c300, 0xe0c3ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0xe0c400, 0xe0c7ff).rw(FUNC(apple2gs_state::c400_r), FUNC(apple2gs_state::c400_w));
	map(0xe0c800, 0xe0cfff).rw(FUNC(apple2gs_state::c800_r), FUNC(apple2gs_state::c800_w));

	map(0xe0d000, 0xe0ffff).view(m_upperbank);
	m_upperbank[0](0xe0d000, 0xe0ffff).view(m_lcbank);
	m_upperbank[1](0xe0d000, 0xe0ffff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));

	m_lcbank[0](0xe0d000, 0xe0ffff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_w));
	m_lcbank[1](0xe0d000, 0xe0ffff).rw(FUNC(apple2gs_state::lc_r), FUNC(apple2gs_state::lc_w));

	map(0xe10000, 0xe1bfff).rw(FUNC(apple2gs_state::auxram0000_r), FUNC(apple2gs_state::auxram0000_w));
	map(0xe1c000, 0xe1c07f).rw(FUNC(apple2gs_state::c000_r), FUNC(apple2gs_state::c000_w));
	map(0xe1c080, 0xe1c0ff).rw(FUNC(apple2gs_state::c080_r), FUNC(apple2gs_state::c080_w));
	map(0xe1c100, 0xe1c2ff).rw(FUNC(apple2gs_state::c100_r), FUNC(apple2gs_state::c100_w));
	map(0xe1c300, 0xe1c3ff).m(m_c300bank, FUNC(address_map_bank_device::amap8));
	map(0xe1c400, 0xe1c7ff).rw(FUNC(apple2gs_state::c400_r), FUNC(apple2gs_state::c400_w));
	map(0xe1c800, 0xe1cfff).rw(FUNC(apple2gs_state::c800_r), FUNC(apple2gs_state::c800_w));

	map(0xe1d000, 0xe1ffff).view(m_upperaux);
	m_upperaux[0](0xe1d000, 0xe1ffff).view(m_lcaux);
	m_upperaux[1](0xe1d000, 0xe1ffff).rw(FUNC(apple2gs_state::inh_r), FUNC(apple2gs_state::inh_w));

	m_lcaux[0](0xe1d000, 0xe1ffff).rom().region("maincpu", 0x3d000).w(FUNC(apple2gs_state::lc_aux_w));
	m_lcaux[1](0xe1d000, 0xe1ffff).rw(FUNC(apple2gs_state::lc_aux_r), FUNC(apple2gs_state::lc_aux_w));

	map(0xfc0000, 0xffffff).rom().region("maincpu", 0x00000);
}

void apple2gs_state::vectors_map(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(apple2gs_state::apple2gs_read_vector));
}

void apple2gs_state::c300bank_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(apple2gs_state::c300_r), FUNC(apple2gs_state::c300_w));
	map(0x0100, 0x01ff).r(FUNC(apple2gs_state::c300_int_r)).nopw();
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

u8 apple2gs_state::adbmicro_p0_in()
{
	return m_glu_bus;
}

u8 apple2gs_state::adbmicro_p1_in()
{
	if (!m_is_rom3)
	{
		return 0xff;
	}
	else
	{
		return 0x06;
	}
}

u8 apple2gs_state::adbmicro_p2_in()
{
	u8 rv = 0;

	if (!m_is_rom3)
	{
		rv |= 0x40;     // no reset, must be 0 on ROM 3
	}
	rv |= (m_adb_line) ? 0x00 : 0x80;

	return rv;
}

u8 apple2gs_state::adbmicro_p3_in()
{
	return 0xc7;
}

void apple2gs_state::adbmicro_p0_out(u8 data)
{
	m_glu_bus = data;
}

void apple2gs_state::adbmicro_p1_out(u8 data)
{
}

void apple2gs_state::adbmicro_p2_out(u8 data)
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

void apple2gs_state::adbmicro_p3_out(u8 data)
{
	if (((data & 0x08) == 0x08) != m_adb_line)
	{
		m_adb_line = (data & 0x8) ? true : false;
#if ADB_HLE
		m_macadb->adb_linechange_w(!m_adb_line);
#endif
	}
}
#if ADB_HLE
void apple2gs_state::set_adb_line(int linestate)
{
	m_adb_line = (linestate == CLEAR_LINE);
}
#endif
u8 apple2gs_state::keyglu_mcu_read(u8 offset)
{
	u8 rv = m_glu_regs[offset];

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

void  apple2gs_state::keyglu_mcu_write(u8 offset, u8 data)
{
	// eat ADB SRQ notices - this shouldn't be necessary and breaks ROM 0/1 :(
	//if ((offset == GLU_DATA) && (data == 0x08))
	//{
	//  return;
	//}

//  if (m_glu_regs[offset] != data)
//  {
//      printf("MCU writes %02x to GLU reg %x (PC=%x)\n", data, offset, m_adbmicro->pc());
//  }
	m_glu_regs[offset] = data;

	switch (offset)
	{
		case GLU_KEY_DATA:
			// if this is the first key pressed within a certain time frame, don't raise the strobe
			// so that the emulated system doesn't see the keypress used to start the emulation.
			if (m_sysconfig->read() & 0x01)
			{ // bump the cycle count way up for a 16 Mhz ZipGS
				if (m_maincpu->total_cycles() < 700000)
				{
					return;
				}
			}
			else
			{
				if (m_maincpu->total_cycles() < 25000)
				{
					return;
				}
			}

			m_glu_regs[GLU_KG_STATUS] |= KGS_KEYSTROBE;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_KEYDATIRQ;
			keyglu_regen_irqs();
			break;

		case GLU_MOUSEX:
		case GLU_MOUSEY:
			m_glu_regs[GLU_KG_STATUS] |= KGS_MOUSEX_FULL;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_MOUSEIRQ;
			if (offset == GLU_MOUSEX)
			{
				keyglu_regen_irqs();
			}
			m_glu_mouse_read_stat = false;  // signal next read will be mouse X
			break;

		case GLU_ANY_KEY_DOWN:  // bit 7 is the actual flag here
			if (data & 0x80)
			{
				m_glu_regs[GLU_KG_STATUS] |= KGS_ANY_KEY_DOWN;
			}
			break;

		case GLU_DATA:
			m_glu_regs[GLU_DATA] = data;
			m_glu_regs[GLU_KG_STATUS] |= KGS_DATA_FULL;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_DATAIRQ;
			keyglu_regen_irqs();

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

#if RUN_ADB_MICRO
u8 apple2gs_state::keyglu_816_read(u8 offset)
{
	switch (offset)
	{
		case GLU_C000:
			if (m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE)
			{
				return m_glu_regs[GLU_KEY_DATA] | 0x80;
			}
			return m_glu_regs[GLU_KEY_DATA];

		case GLU_C010:
			return (m_glu_regs[GLU_ANY_KEY_DOWN] & 0x80);

		case GLU_KEYMOD:
			return m_glu_regs[GLU_KEYMOD];

		case GLU_MOUSEX:
		case GLU_MOUSEY:
			if (!m_glu_mouse_read_stat)
			{
				m_glu_mouse_read_stat = 1;
				m_glu_regs[GLU_KG_STATUS] &= ~KGS_MOUSEX_FULL;
				keyglu_regen_irqs();
				return m_glu_regs[GLU_MOUSEX];
			}
			return m_glu_regs[GLU_MOUSEY];

		case GLU_SYSSTAT:
			{
				// regenerate sysstat bits
				u8 sysstat = m_glu_regs[GLU_SYSSTAT] & ~0xab;
				if (m_glu_regs[GLU_KG_STATUS] & KGS_COMMAND_FULL)
				{
					sysstat |= GLU_STATUS_CMDFULL;
				}
				if (m_glu_regs[GLU_KG_STATUS] & m_glu_mouse_read_stat)
				{
					sysstat |= GLU_STATUS_MOUSEXY;
				}
				if (m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE)
				{
					sysstat |= GLU_STATUS_KEYDATIRQ;
				}
				if (m_glu_regs[GLU_KG_STATUS] & KGS_DATA_FULL)
				{
					sysstat |= GLU_STATUS_DATAIRQ;
				}
				if (m_glu_regs[GLU_KG_STATUS] & KGS_MOUSEX_FULL)
				{
					sysstat |= GLU_STATUS_MOUSEIRQ;
				}
				m_glu_816_read_dstat = true;
				//printf("Read SYSSTAT %02x (PC=%x)\n", sysstat, m_maincpu->pc());
				return sysstat;
			}

		case GLU_DATA:
			if (m_glu_816_read_dstat)
			{
				m_glu_816_read_dstat = false;
				m_glu_regs[GLU_KG_STATUS] &= ~KGS_DATA_FULL;
				keyglu_regen_irqs();
			}
			return m_glu_regs[GLU_DATA];

		default:
			return 0xff;
			break;
	}

	return 0xff;
}

void apple2gs_state::keyglu_816_write(u8 offset, u8 data)
{
	if (offset < GLU_C000)
	{
		m_glu_regs[offset&7] = data;
	}

	switch (offset)
	{
		case GLU_C010:
			m_glu_regs[GLU_SYSSTAT] &= ~GLU_STATUS_KEYDATIRQ;
			m_glu_regs[GLU_KG_STATUS] &= ~KGS_KEYSTROBE;
			keyglu_regen_irqs();
			break;

		case GLU_COMMAND:
			m_glu_regs[GLU_KG_STATUS] |= KGS_COMMAND_FULL;
			break;

		case GLU_SYSSTAT:
			m_glu_regs[GLU_SYSSTAT] &= 0xab;  // clear the non-read-only fields
			m_glu_regs[GLU_SYSSTAT] |= (data & ~0xab);
			break;
	}
}
#endif

void apple2gs_state::keyglu_regen_irqs()
{
#if RUN_ADB_MICRO
	bool bIRQ = false;

	if ((m_glu_regs[GLU_KG_STATUS] & KGS_DATA_FULL) && (m_glu_regs[GLU_SYSSTAT] & GLU_STATUS_DATAIRQEN))
	{
		bIRQ = true;
	}

	if ((m_glu_regs[GLU_KG_STATUS] & KGS_KEYSTROBE) && (m_glu_regs[GLU_SYSSTAT] & GLU_STATUS_KEYDATIRQEN))
	{
		bIRQ = true;
	}

	if (bIRQ)
	{
		raise_irq(IRQS_ADB);
	}
	else
	{
		lower_irq(IRQS_ADB);
	}
#endif
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

u8 apple2gs_state::doc_adc_read()
{
	return 0x80;
}

void apple2gs_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->seek_phase_w(phases);
	}
}

void apple2gs_state::devsel_w(uint8_t devsel)
{
	m_devsel = devsel;
	if (m_devsel == 1)
	{
		if (!BIT(m_diskreg, DISKREG_35SEL))
		{
			m_cur_floppy = m_floppy[0]->get_device();
		}
		else
		{
			m_cur_floppy = m_floppy[2]->get_device();
			m_motoroff_time = 0;
		}
	}
	else if (m_devsel == 2)
	{
		if (!BIT(m_diskreg, DISKREG_35SEL))
		{
			m_cur_floppy = m_floppy[1]->get_device();
		}
		else
		{
			m_cur_floppy = m_floppy[3]->get_device();
			m_motoroff_time = 0;
		}
	}
	else
	{
		m_cur_floppy = nullptr;
	}
	m_iwm->set_floppy(m_cur_floppy);

	if ((m_cur_floppy) && (BIT(m_diskreg, DISKREG_35SEL)))
	{
		m_cur_floppy->ss_w(BIT(m_diskreg, DISKREG_HDSEL));
	}
}

void apple2gs_state::sel35_w(int sel35)
{
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

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
#if !RUN_ADB_MICRO
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
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
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

	PORT_START(A2GS_KBD_SPEC_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)

	PORT_START("adb_mouse_x")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 1")

	PORT_START("adb_mouse_y")
	PORT_BIT( 0x7f, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 0")
#endif

	PORT_START("a2_config")
	PORT_CONFNAME(0x07, 0x00, "CPU type")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x01, "7 MHz ZipGS")
	PORT_CONFSETTING(0x03, "8 MHz ZipGS")
	PORT_CONFSETTING(0x05, "12 MHz ZipGS")
	PORT_CONFSETTING(0x07, "16 MHz ZipGS")
INPUT_PORTS_END

void apple2gs_state::apple2gs(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, A2GS_MASTER_CLOCK/10);
	m_maincpu->set_addrmap(AS_PROGRAM, &apple2gs_state::apple2gs_map);
	m_maincpu->set_addrmap(g65816_device::AS_VECTORS, &apple2gs_state::vectors_map);
	m_maincpu->set_dasm_override(FUNC(apple2gs_state::dasm_trampoline));
	m_maincpu->wdm_handler().set(FUNC(apple2gs_state::wdm_trampoline));
	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(apple2gs_state::apple2_interrupt), "screen", 0, 1);

	TIMER(config, m_acceltimer, 0).configure_generic(FUNC(apple2gs_state::accel_timer));

	config.set_maximum_quantum(attotime::from_hz(60));

	M50741(config, m_adbmicro, A2GS_MASTER_CLOCK/8);
	m_adbmicro->read_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_in));
	m_adbmicro->write_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_out));
	m_adbmicro->read_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_in));
	m_adbmicro->write_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_out));
	m_adbmicro->read_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_in));
	m_adbmicro->write_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_out));
	m_adbmicro->read_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_in));
	m_adbmicro->write_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_out));

#if ADB_HLE
	MACADB(config, m_macadb, A2GS_MASTER_CLOCK/8);
	m_macadb->set_mcu_mode(true);
	m_macadb->adb_data_callback().set(FUNC(apple2gs_state::set_adb_line));
#endif

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

	APPLE2_VIDEO(config, m_video, A2GS_14M).set_screen(m_screen);

	APPLE2_COMMON(config, m_a2common, A2GS_14M).set_GS_cputag(m_maincpu);

//  APPLE2_HOST(config, m_a2host, A2GS_14M);
//  m_a2host->set_cputag(m_maincpu);
//  m_a2host->set_space(m_maincpu, AS_PROGRAM);

	APPLE2_GAMEIO(config, m_gameio, apple2_gameio_device::default_options, nullptr);

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
	// IIgs Tech Node #19 says even channels are right, odd are left, and 80s/90s stereo cards followed that.
	m_doc->add_route(0, "rspeaker", 1.0);
	m_doc->add_route(1, "lspeaker", 1.0);

	/* RAM */
	RAM(config, m_ram).set_default_size("2M").set_extra_options("1M,3M,4M,5M,6M,7M,8M").set_default_value(0x00);

	/* C300 banking */
	ADDRESS_MAP_BANK(config, A2GS_C300_TAG).set_map(&apple2gs_state::c300bank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x100);

	/* serial */
	SCC85C30(config, m_scc, A2GS_14M / 2);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(apple2gs_state::scc_irq_w));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
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
	A2BUS_SLOT(config, "sl1", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl2", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl3", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl4", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl5", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl6", m_a2bus, apple2gs_cards, nullptr);
	A2BUS_SLOT(config, "sl7", m_a2bus, apple2gs_cards, nullptr);

	IWM(config, m_iwm, A2GS_7M, 1021800*2);
	m_iwm->phases_cb().set(FUNC(apple2gs_state::phases_w));
	m_iwm->sel35_cb().set(FUNC(apple2gs_state::sel35_w));
	m_iwm->devsel_cb().set(FUNC(apple2gs_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);
	applefdintf_device::add_35(config, m_floppy[2]);
	applefdintf_device::add_35(config, m_floppy[3]);

	SOFTWARE_LIST(config, "flop_gs_clean").set_original("apple2gs_flop_clcracked"); // GS-specific cleanly cracked disks
	SOFTWARE_LIST(config, "flop_gs_orig").set_compatible("apple2gs_flop_orig"); // Original disks for GS
	SOFTWARE_LIST(config, "flop_gs_misc").set_compatible("apple2gs_flop_misc"); // Legacy software list pre-June 2021 and defaced cracks
	SOFTWARE_LIST(config, "flop_a2_clean").set_compatible("apple2_flop_clcracked"); // Apple II series cleanly cracked
	SOFTWARE_LIST(config, "flop_a2_orig").set_compatible("apple2_flop_orig").set_filter("A2GS");  // Filter list to compatible disks for this machine.
	SOFTWARE_LIST(config, "flop_a2_misc").set_compatible("apple2_flop_misc");
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
	ROM_REGION(0x1000, "adbmicro", 0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

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
	ROM_REGION(0x1000, "adbmicro", 0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

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
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("342-0077-b", 0x20000, 0x20000, CRC(42f124b0) SHA1(e4fc7560b69d062cb2da5b1ffbe11cd1ca03cc37)) /* 342-0077-B: IIgs ROM01 */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0)
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD("342-0077-a", 0x20000, 0x20000, CRC(dfbdd97b) SHA1(ff0c245dd0732ec4413a934fd80efc2defd8a8e3) ) /* 342-0077-A: IIgs ROM00 */

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p)  // 6/19/1986 Cortland prototype
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD( "rombf.bin",    0x020000, 0x020000, CRC(ab04fedf) SHA1(977589a17553956d583a21020080a39dd396df5c) )

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2gsr0p2)  // 3/10/1986 Cortland prototype, boots as "Apple //'ing - Alpha 2.0"
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x40000,"maincpu",0)
	ROM_LOAD( "apple iigs alpha rom 2.0 19860310.bin", 0x020000, 0x020000, CRC(a47d275f) SHA1(c5836adcfc8be69c7351b84afa94c814e8d92b81) )

	// temporary: use IIe enhanced keyboard decode ROM
	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

} // Anonymous namespace


/*    YEAR  NAME            PARENT    COMPAT    MACHINE     INPUT     CLASS        INIT  COMPANY           FULLNAME */
COMP( 1989, apple2gs,     0,        apple2, apple2gs,   apple2gs, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03)", MACHINE_SUPPORTS_SAVE )
COMP( 198?, apple2gsr3p,  apple2gs, 0,      apple2gs,   apple2gs, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03 prototype)", MACHINE_NOT_WORKING )
COMP( 1987, apple2gsr1,   apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM01)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0,   apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p,  apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 6/19/1986)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p2, apple2gs, 0,      apple2gsr1, apple2gs, apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 3/10/1986)", MACHINE_SUPPORTS_SAVE )
