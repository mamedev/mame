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
    - "Mark Twain" prototype: ROM 3 hardware, SWIM1 instead of IWM, built-in floppy, integrated High-Speed SCSI Card
      and internal SCSI HDD, 2 SIMM slots for RAM expansion instead of the proprietary memory slot of the previous IIgs.
      Only 5 slots: slots 5 and 7 are missing (5 for the SuperDrive, 7 for the SCSI).

    Timing in terms of the 14M 14.3181818 MHz clock (1/2 of the 28.6363636 master clock):
    - 1 2.8 MHz 65816 cycle is 5 14M clocks.
    - The Mega II 1 MHz side runs for 64 cycles at 14 14M clocks and every 65th is stretched to 16 14M clocks.
      This allows 8-bit Apple II raster demos to work.  Each scanline is (64*14) + 16 = 912 14M clocks.
      Due to this stretch, which does not occur on the fast side, the fast and 1 Mhz sides drift from each other
      and sync up every 22800 14M clocks (25 scan lines).
    - Accesses to the 1 MHz side incur a side-sync penalty (waiting for the start of the next 1 MHz cycle).
    - Every 50 14M clocks (10 65816 cycles) DRAM refresh occurs for 5 14M clocks
      * During this time, CPU accesses to ROM, Mega II side I/O, or banks E0/E1 are not penalized (but a side-sync penalty is incurred for the 1 MHz side)
      * Accesses to banks 00-7F are penalized except for I/O in banks 0/1.
    - ROM accesses always run at full speed.

    One video line is: 6 cycles of right border, 13 cycles of hblank, 6 cycles of left border, and 40 cycles of active video

    ((6*14)*2) + 560 = 728 (total for A2 modes)  htotal = 910 (65 * 14)
    ((6*16)*2) + 640 = 832 (total for SHR)       htotal = 1040 (65 * 16)

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

#include "apple2video.h"

#include "apple2common.h"
// #include "machine/apple2host.h"
#include "macadb.h"
#include "macrtc.h"

#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/cards.h"
#include "bus/a2gameio/gameio.h"
#include "bus/rs232/rs232.h"
#include "cpu/g65816/g65816.h"
#include "cpu/m6502/m5074x.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/es5503.h"
#include "sound/spkrdev.h"

#include "machine/applefdintf.h"
#include "machine/iwm.h"
#include "machine/swim1.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"


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
		  m_macadb(*this, "macadb"),
		  m_ram(*this, "ram"),
		  m_rom(*this, "maincpu"),
		  m_docram(*this, "docram"),
		  m_video(*this, "a2video"),
		  m_rtc(*this, "rtc"),
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
		  m_e0_0000bank(*this, "e0_0000_bank"),
		  m_e0_0200bank(*this, "e0_0200_bank"),
		  m_e0_0400bank(*this, "e0_0400_bank"),
		  m_e0_0800bank(*this, "e0_0800_bank"),
		  m_e0_2000bank(*this, "e0_2000_bank"),
		  m_e0_4000bank(*this, "e0_4000_bank"),
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
		  m_sysconfig(*this, "a2_config")

	{
		m_cur_floppy = nullptr;
		m_devsel = 0;
		m_diskreg = 0;
	}

	void apple2gs(machine_config &config);
	void apple2gsr1(machine_config &config);
	void apple2gsmt(machine_config &config);

	void rom1_init() { m_is_rom3 = false; }
	void rom3_init() { m_is_rom3 = true; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<g65816_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scantimer, m_acceltimer;
	required_device<m5074x_device> m_adbmicro;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_docram;
	required_device<a2_video_device> m_video;
	required_device<rtc3430042_device> m_rtc;
	required_device<a2bus_device> m_a2bus;
	required_device<apple2_common_device> m_a2common;
//  required_device<apple2_host_device> m_a2host;
	required_device<apple2_gameio_device> m_gameio;
	required_device<speaker_sound_device> m_speaker;
	memory_view m_upperbank, m_upperaux, m_upper00, m_upper01;
	required_device<address_map_bank_device> m_c300bank;
	memory_view m_b0_0000bank, m_b0_0200bank, m_b0_0400bank, m_b0_0800bank, m_b0_2000bank, m_b0_4000bank;
	memory_view m_e0_0000bank, m_e0_0200bank, m_e0_0400bank, m_e0_0800bank, m_e0_2000bank, m_e0_4000bank;
	memory_view m_lcbank, m_lcaux, m_lc00, m_lc01, m_bank0_atc, m_bank1_atc;
	required_device<z80scc_device> m_scc;
	required_device<es5503_device> m_doc;
	required_device<applefdintf_device> m_iwm;
	required_device_array<floppy_connector, 4> m_floppy;
	required_ioport m_sysconfig;

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

	enum adbstate_t
	{
		ADBSTATE_IDLE,
		ADBSTATE_INCOMMAND,
		ADBSTATE_INRESPONSE
	};

	bool m_adb_line = false;

	address_space *m_maincpu_space = nullptr;

	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(accel_timer);

	void palette_init(palette_device &palette);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void apple2gs_map(address_map &map) ATTR_COLD;
	void vectors_map(address_map &map) ATTR_COLD;
	void a2gs_es5503_map(address_map &map) ATTR_COLD;
	void c300bank_map(address_map &map) ATTR_COLD;

	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);

	floppy_image_device *m_cur_floppy = nullptr;
	int m_devsel = 0;
	u8 m_diskreg = 0;

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

	template <int Addr> u8 e0ram_r(offs_t offset);
	template <int Addr> void e0ram_w(offs_t offset, u8 data);
	template <int Addr> u8 e1ram_r(offs_t offset);
	template <int Addr> void e1ram_w(offs_t offset, u8 data);

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
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void a2bus_inh_w(int state);
	void doc_irq_w(int state);
	void scc_irq_w(int state);
	u8 doc_adc_read();
	u8 apple2gs_read_vector(offs_t offset);

	u8 keyglu_mcu_read(u8 offset);
	void keyglu_mcu_write(u8 offset, u8 data);
	u8 keyglu_816_read(u8 offset);
	void keyglu_816_write(u8 offset, u8 data);

	u8 m_adb_p2_last, m_adb_p3_last;
	int m_adb_reset_freeze = 0;
	void keyglu_regen_irqs();

	u8 adbmicro_p0_in();
	u8 adbmicro_p1_in();
	u8 adbmicro_p2_in();
	u8 adbmicro_p3_in();
	void adbmicro_p0_out(u8 data);
	void adbmicro_p1_out(u8 data);
	void adbmicro_p2_out(u8 data);
	void adbmicro_p3_out(u8 data);
	void set_adb_line(int linestate);

	offs_t dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
	void wdm_trampoline(offs_t offset, u8 data) { }; //m_a2host->wdm_w(space, offset, data); }

	bool m_is_rom3 = false;
	int m_speaker_state = 0;

	double m_joystick_x1_time = 0, m_joystick_y1_time = 0, m_joystick_x2_time = 0, m_joystick_y2_time = 0;

	int m_inh_slot = 0, m_cnxx_slot = 0;
	int m_motoroff_time = 0;

	bool m_romswitch = false;

	bool m_an0 = false, m_an1 = false, m_an2 = false, m_an3 = false;

	bool m_vbl = false;

	int m_irqmask = 0;

	bool m_intcxrom = false;
	bool m_slotc3rom = false;
	bool m_altzp = false;
	bool m_ramrd = false, m_ramwrt = false;
	bool m_lcram = false, m_lcram2 = false, m_lcprewrite = false, m_lcwriteenable = false;
	bool m_ioudis = false;
	bool m_rombank = false;

	u8 m_shadow = 0, m_speed = 0, m_textcol = 0;
	u8 m_motors_active = 0, m_slotromsel = 0, m_intflag = 0, m_vgcint = 0, m_inten = 0;

	bool m_last_speed = false;

	// Sound GLU variables
	u8 m_sndglu_ctrl = 0;
	int m_sndglu_addr = 0;
	int m_sndglu_dummy_read = 0;

	// Key GLU variables
	u8 m_glu_regs[12]{}, m_glu_bus = 0;
	bool m_glu_mcu_read_kgs = false, m_glu_816_read_dstat = false, m_glu_mouse_read_stat = false;
	int m_glu_kbd_y = 0;

	u8 *m_ram_ptr = nullptr;
	int m_ram_size = 0;
	u8 m_megaii_ram[0x20000]{};  // 128K of "slow RAM" at $E0/0000

	int m_inh_bank = 0;

	bool m_slot_irq = false;

	double m_x_calibration = 0, m_y_calibration = 0;

	device_a2bus_card_interface *m_slotdevice[8]{};

	u32 m_slow_counter = 0;

	// clock/BRAM
	u8 m_clkdata = 0, m_clock_control = 0;
	u8 m_clock_frame = 0;

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
	bool m_accel_unlocked = false;
	bool m_accel_fast = false;
	bool m_accel_present = false;
	bool m_accel_temp_slowdown = false;
	int m_accel_stage = 0;
	u32 m_accel_speed = 0;
	u8 m_accel_slotspk = 0, m_accel_gsxsettings = 0, m_accel_percent = 0;

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

// slow_cycle() - take a 1 MHz cycle.  Theory: a 2.8 MHz cycle is 14M / 5.
// 1 MHz is 14M / 14.  14/5 = 2.8 * 65536 (16.16 fixed point) = 0x2cccd.
#define slow_cycle() \
{   \
	if (!machine().side_effects_disabled() && m_last_speed) \
	{\
		m_slow_counter += 0x0002cccd; \
		int cycles = (m_slow_counter >> 16) & 0xffff; \
		m_slow_counter &= 0xffff; \
		m_maincpu->adjust_icount(-cycles); \
	} \
}


offs_t apple2gs_state::dasm_trampoline(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return m_a2common->dasm_override_GS(stream, pc, opcodes, params);
}

void apple2gs_state::a2bus_irq_w(int state)
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

void apple2gs_state::a2bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

// TODO: this assumes /INH only on ROM, needs expansion to support e.g. phantom-slotting cards and etc.
void apple2gs_state::a2bus_inh_w(int state)
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
	m_e0_0000bank.select(0);
	m_b0_0200bank.select(0);
	m_e0_0200bank.select(0);
	m_b0_0400bank.select(0);
	m_e0_0400bank.select(0);
	m_b0_0800bank.select(0);
	m_e0_0800bank.select(0);
	m_b0_2000bank.select(0);
	m_e0_2000bank.select(0);
	m_b0_4000bank.select(0);
	m_e0_4000bank.select(0);
	m_inh_bank = 0;
	std::fill(std::begin(m_megaii_ram), std::end(m_megaii_ram), 0);

	std::fill(std::begin(m_glu_regs), std::end(m_glu_regs), 0);

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
	m_video->set_ram_pointers(m_megaii_ram, &m_megaii_ram[0x10000]);
	m_video->set_char_pointer(memregion("gfx1")->base(), memregion("gfx1")->bytes());
	m_video->setup_GS_graphics();

	m_textcol = 0xf2;
	m_video->set_GS_foreground((m_textcol >> 4) & 0xf);
	m_video->set_GS_background(m_textcol & 0xf);

	m_inh_slot = -1;
	m_cnxx_slot = CNXX_UNCLAIMED;


	// install memory beyond 256K
	int ramsize = m_ram_size;
	if (!m_is_rom3 && m_ram_size <= 1280 * 1024)
	{
		ramsize -= 0x40000; // subtract 256k for banks 0, 1, e0, e1
	}
	else if (m_is_rom3 || m_ram_size == 1024 * 1024 * 8)
	{
		ramsize -= 0x20000; // subtract 128K for banks 0 and 1, which are handled specially
	}

	if (ramsize)
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		// RAM sizes for both classes of machine no longer include the Mega II RAM
		space.install_ram(0x020000, ramsize - 1 + 0x20000, m_ram_ptr + 0x020000);
	}

	// setup save states
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_joystick_x1_time));
	save_item(NAME(m_joystick_y1_time));
	save_item(NAME(m_joystick_x2_time));
	save_item(NAME(m_joystick_y2_time));
	save_item(NAME(m_inh_slot));
	save_item(NAME(m_inh_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_romswitch));
	save_item(NAME(m_an0));
	save_item(NAME(m_an1));
	save_item(NAME(m_an2));
	save_item(NAME(m_an3));
	save_item(NAME(m_intcxrom));
	save_item(NAME(m_rombank));
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
	save_item(NAME(m_clock_frame));
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
	save_item(NAME(m_adb_p2_last));
	save_item(NAME(m_adb_p3_last));
	save_item(NAME(m_adb_reset_freeze));
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
}

void apple2gs_state::machine_reset()
{
	m_adb_p2_last = m_adb_p3_last = 0;
	m_adb_reset_freeze = 0;
	m_romswitch = false;
	m_video->page2_w(false);
	m_video->set_GS_border(0x02);
	m_video->set_GS_background(0x02);
	m_video->set_GS_foreground(0x0f);
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
	m_video->a80store_w(false);
	m_altzp = false;
	m_ramrd = false;
	m_ramwrt = false;
	m_ioudis = true;
	m_video->set_newvideo(0x01); // verified on ROM03 hardware
	m_clock_frame = 0;
	m_slot_irq = false;
	m_clkdata = 0;
	m_clock_control = 0;

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
	m_e0_0000bank.select(0);
	m_b0_0200bank.select(0);
	m_e0_0200bank.select(0);
	m_b0_0400bank.select(0);
	m_e0_0400bank.select(0);
	m_b0_0800bank.select(0);
	m_e0_0800bank.select(0);
	m_b0_2000bank.select(0);
	m_e0_2000bank.select(0);
	m_b0_4000bank.select(0);
	m_e0_4000bank.select(0);
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

	// reset the slots
	m_a2bus->reset_bus();

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

	/* check scanline interrupt bits if we're in super hi-res and the current scanline is within the active display area */
	if ((m_video->get_newvideo() & 0x80) && (scanline >= BORDER_TOP) && (scanline < (200+BORDER_TOP)))
	{
		u8 scb;
		const int shrline = scanline - BORDER_TOP;

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
		m_vbl = true;

		// VBL interrupt
		if ((m_inten & 0x08) && !(m_intflag & INTFLAG_VBL))
		{
			m_intflag |= INTFLAG_VBL;
			raise_irq(IRQS_VBL);
		}

		m_adbmicro->set_input_line(0, ASSERT_LINE);

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

		m_video->set_GS_border_color(i, rgb_t(apple2gs_palette[(3*i)]*17, apple2gs_palette[(3*i)+1]*17, apple2gs_palette[(3*i)+2]*17));
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
	m_e0_0000bank.select(m_altzp ? 1 : 0);
	m_b0_0200bank.select(ramwr);
	m_e0_0200bank.select(ramwr);

	if (m_video->get_80store())
	{
		if (m_video->get_page2())
		{
			m_b0_0400bank.select(3);
			m_e0_0400bank.select(3);
		}
		else
		{
			m_b0_0400bank.select(0);
			m_e0_0400bank.select(0);
		}
	}
	else
	{
		m_b0_0400bank.select(ramwr);
		m_e0_0400bank.select(ramwr);
	}

	m_b0_0800bank.select(ramwr);
	m_e0_0800bank.select(ramwr);

	if ((m_video->get_80store()) && (m_video->get_hires()))
	{
		if (m_video->get_page2())
		{
			m_b0_2000bank.select(3);
			m_e0_2000bank.select(3);
		}
		else
		{
			m_b0_2000bank.select(0);
			m_e0_2000bank.select(0);
		}
	}
	else
	{
		m_b0_2000bank.select(ramwr);
		m_e0_2000bank.select(ramwr);
	}

	m_b0_4000bank.select(ramwr);
	m_e0_4000bank.select(ramwr);
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

	// any even access disables pre-write and writing
	if ((offset & 1) == 0)
	{
		m_lcprewrite = false;
		m_lcwriteenable = false;
	}

	// any write disables pre-write
	// has no effect on write-enable if writing was enabled already
	if (writing == true)
	{
		m_lcprewrite = false;
	}
	// first odd read enables pre-write, second one enables writing
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

		case 0x28:  // ROMSWITCH - not used by the IIgs firmware or SSW, but does exist at least on ROM 0/1 (need to test on ROM 3 hw)
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
			m_video->page2_w(false);
			m_video->scr_w(0);
			auxbank_update();
			break;

		case 0x55:  // set page 2
			m_video->page2_w(true);
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
	for (int i = 0; i < 8; i++)
	{
		m_rtc->clk_w(ASSERT_LINE);
		if (!BIT(m_clock_control, 6))
		{
			m_rtc->data_w(BIT(m_clkdata, 7-i));
			m_rtc->clk_w(CLEAR_LINE);
		}
		else
		{
			m_rtc->clk_w(CLEAR_LINE);
			m_clkdata <<= 1;
			m_clkdata |= m_rtc->data_r() & 1;
		}
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
	u8 uKeyboard = keyglu_816_read(GLU_C000);
	u8 uKeyboardC010 = keyglu_816_read(GLU_C010);

	switch (offset)
	{
		// keyboard latch
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
		case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d:
		case 0x0e: case 0x0f:
			return uKeyboard;

		case 0x10:  // read any key down, reset keyboard strobe
			keyglu_816_write(GLU_C010, 0);
			return uKeyboardC010 | (uKeyboard & 0x7f);

		case 0x11:  // read LCRAM2 (LC Dxxx bank)
			return (uKeyboardC010 & 0x7f) | (m_lcram2 ? 0x80 : 0x00);

		case 0x12:  // read LCRAM (is LC readable?)
			return (uKeyboardC010 & 0x7f) | (m_lcram ? 0x80 : 0x00);

		case 0x13:  // read RAMRD
			return (uKeyboardC010 & 0x7f) | (m_ramrd ? 0x80 : 0x00);

		case 0x14:  // read RAMWRT
			return (uKeyboardC010 & 0x7f) | (m_ramwrt ? 0x80 : 0x00);

		case 0x15:  // read INTCXROM
			return (uKeyboardC010 & 0x7f) | (m_intcxrom ? 0x80 : 0x00);

		case 0x16:  // read ALTZP
			return (uKeyboardC010 & 0x7f) | (m_altzp ? 0x80 : 0x00);

		case 0x17:  // read SLOTC3ROM
			return (uKeyboardC010 & 0x7f) | (m_slotc3rom ? 0x80 : 0x00);

		case 0x18:  // read 80STORE
			return (uKeyboardC010 & 0x7f) | (m_video->get_80store() ? 0x80 : 0x00);

		case 0x19:  // read VBLBAR
			return (uKeyboardC010 & 0x7f) | (m_screen->vblank() ? 0x00 : 0x80);

		case 0x1a:  // read TEXT
			return (uKeyboardC010 & 0x7f) | (m_video->get_graphics() ? 0x00 : 0x80);

		case 0x1b:  // read MIXED
			return (uKeyboardC010 & 0x7f) | (m_video->get_mix() ? 0x80 : 0x00);

		case 0x1c:  // read PAGE2
			return (uKeyboardC010 & 0x7f) | (m_video->get_page2() ? 0x80 : 0x00);

		case 0x1d:  // read HIRES
			return (uKeyboardC010 & 0x7f) | (m_video->get_hires() ? 0x80 : 0x00);

		case 0x1e:  // read ALTCHARSET
			return (uKeyboardC010 & 0x7f) | (m_video->get_altcharset() ? 0x80 : 0x00);

		case 0x1f:  // read 80COL
			return (uKeyboardC010 & 0x7f) | (m_video->get_80col() ? 0x80 : 0x00);

		case 0x22:  // TEXTCOL
			return m_textcol;

		case 0x23:  // VGCINT
			return m_vgcint;

		case 0x24:  // MOUSEDATA */
			return keyglu_816_read(GLU_MOUSEX);

		case 0x25:  // KEYMODREG
			return keyglu_816_read(GLU_KEYMOD);

		case 0x26:  // DATAREG
			return keyglu_816_read(GLU_DATA);

		case 0x27:  // KMSTATUS
			return keyglu_816_read(GLU_SYSSTAT);

		case 0x29:  // NEWVIDEO
			return m_video->get_newvideo();

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
			return (m_clock_control & 0xf0) | (m_video->get_GS_border() & 0xf);

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
			return (m_gameio->sw3_r() ? 0x80 : 0x00) | uFloatingBus7;

		case 0x61: // button 0 or Open Apple
			// HACK/TODO: the 65816 loses a race to the microcontroller on reset
			if (m_adb_reset_freeze > 0) m_adb_reset_freeze--;
			return ((m_gameio->sw0_r() || (m_adb_p3_last & 0x20)) ? 0x80 : 0) | uFloatingBus7;

		case 0x62: // button 1 or Option
			return ((m_gameio->sw1_r() || (m_adb_p3_last & 0x10)) ? 0x80 : 0) | uFloatingBus7;

		case 0x63: // button 2 or SHIFT key
			return (m_gameio->sw2_r() ? 0x80 : 0x00) | uFloatingBus7;

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
					(m_video->get_page2() ? 0x40 : 0x00) |
					(m_ramrd ? 0x20 : 0x00) |
					(m_ramwrt ? 0x10 : 0x00) |
					(m_lcram ? 0x00 : 0x08) |
					(m_lcram2 ? 0x04 : 0x00) |
					(m_rombank ? 0x02 : 0x00) |
					(m_intcxrom ? 0x01 : 0x00);

		case 0x70:  // PTRIG - triggers paddles on read or write
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

		// The ROM IRQ vectors point here
		case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			return m_rom[offset + 0x3c000];

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
			m_video->a80store_w(false);
			auxbank_update();
			break;

		case 0x01:  // 80STOREON
			m_video->a80store_w(true);
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
			m_video->a80col_w(false);
			break;

		case 0x0d:  // 80COLON
			m_video->a80col_w(true);
			break;

		case 0x0e:  // ALTCHARSETOFF
			m_video->altcharset_w(false);
			break;

		case 0x0f:  // ALTCHARSETON
			m_video->altcharset_w(true);
			break;

		case 0x10:  // clear keyboard latch
			keyglu_816_write(GLU_C010, data);
			break;

		case 0x20:
			break;

		case 0x21:  // MONOCHROME
			m_video->set_GS_monochrome(data);
			break;

		case 0x22:  // TEXTCOL
			if (m_textcol != data)
			{
				m_screen->update_now();
			}
			m_textcol = data;
			m_video->set_GS_foreground((data >> 4) & 0xf);
			m_video->set_GS_background(data & 0xf);
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

		case 0x26:  // DATAREG
			// allow ADBuC sync if the Zip is on
			if (m_accel_present)
			{
				m_accel_temp_slowdown = true;
				m_acceltimer->adjust(attotime::from_msec(30));
				accel_normal_speed();
			}
			keyglu_816_write(GLU_COMMAND, data);
			break;

		case 0x27:  // KMSTATUS
			keyglu_816_write(GLU_SYSSTAT, data);
			break;

		case 0x29:  // NEWVIDEO
			m_video->set_newvideo(data);
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
			if ((data & 0xf) != (m_video->get_GS_border() & 0xf))
			{
				m_screen->update_now();
			}
			m_clock_control = data & 0x7f;
			m_video->set_GS_border(data & 0xf);
			m_rtc->ce_w(BIT(data, 7) ^ 1);
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
			m_video->page2_w(data & 0x40);
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

		offset &= 0x7f;
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

	offset &= 0x7f;
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
	if (m_altzp)
	{
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
	else
	{
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
}

void apple2gs_state::lc_w(offs_t offset, u8 data)
{
	slow_cycle();
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
	else
	{
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
	Hires    = (m_video->get_hires() && m_video->get_graphics()) ? 1 : 0;
	Mixed    = m_video->get_mix() ? 1 : 0;
	Page2    = m_video->get_page2() ? 1 : 0;
	_80Store = m_video->get_80store() ? 1 : 0;

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

template <int Addr>
u8 apple2gs_state::e0ram_r(offs_t offset) { slow_cycle(); return m_megaii_ram[offset + Addr]; }

template <int Addr>
void apple2gs_state::e0ram_w(offs_t offset, u8 data) { slow_cycle(); m_megaii_ram[offset + Addr] = data; }

template <int Addr>
u8 apple2gs_state::e1ram_r(offs_t offset) { slow_cycle(); return m_megaii_ram[offset + Addr + 0x10000]; }

template <int Addr>
void apple2gs_state::e1ram_w(offs_t offset, u8 data) { slow_cycle(); m_megaii_ram[offset + Addr + 0x10000] = data; }

template u8 apple2gs_state::e0ram_r<0x0000>(offs_t offset);
template u8 apple2gs_state::e0ram_r<0x0200>(offs_t offset);
template u8 apple2gs_state::e0ram_r<0x0400>(offs_t offset);
template u8 apple2gs_state::e0ram_r<0x0800>(offs_t offset);
template u8 apple2gs_state::e0ram_r<0x2000>(offs_t offset);
template u8 apple2gs_state::e0ram_r<0x4000>(offs_t offset);

template u8 apple2gs_state::e1ram_r<0x0000>(offs_t offset);
template u8 apple2gs_state::e1ram_r<0x0200>(offs_t offset);
template u8 apple2gs_state::e1ram_r<0x0400>(offs_t offset);
template u8 apple2gs_state::e1ram_r<0x0800>(offs_t offset);
template u8 apple2gs_state::e1ram_r<0x2000>(offs_t offset);
template u8 apple2gs_state::e1ram_r<0x4000>(offs_t offset);

template void apple2gs_state::e0ram_w<0x0000>(offs_t offset, u8 data);
template void apple2gs_state::e0ram_w<0x0200>(offs_t offset, u8 data);
template void apple2gs_state::e0ram_w<0x0400>(offs_t offset, u8 data);
template void apple2gs_state::e0ram_w<0x0800>(offs_t offset, u8 data);
template void apple2gs_state::e0ram_w<0x2000>(offs_t offset, u8 data);
template void apple2gs_state::e0ram_w<0x4000>(offs_t offset, u8 data);

template void apple2gs_state::e1ram_w<0x0000>(offs_t offset, u8 data);
template void apple2gs_state::e1ram_w<0x0200>(offs_t offset, u8 data);
template void apple2gs_state::e1ram_w<0x0400>(offs_t offset, u8 data);
template void apple2gs_state::e1ram_w<0x0800>(offs_t offset, u8 data);
template void apple2gs_state::e1ram_w<0x2000>(offs_t offset, u8 data);
template void apple2gs_state::e1ram_w<0x4000>(offs_t offset, u8 data);

u8 apple2gs_state::auxram0000_r(offs_t offset)
{
	slow_cycle();
	if ((offset >= 0x2000) && (offset < 0xa000) && ((m_video->get_newvideo() & 0xc0) != 0))
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

	if ((offset >= 0x2000) && (offset < 0xa000) && ((m_video->get_newvideo() & 0xc0) != 0))
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
		m_video->set_SHR_color(color, rgb_t(
			((m_megaii_ram[0x19f00 + color] >> 0) & 0x0f) * 17,
			((m_megaii_ram[0x15f00 + color] >> 4) & 0x0f) * 17,
			((m_megaii_ram[0x15f00 + color] >> 0) & 0x0f) * 17));
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
	m_b0_0400bank[1](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b1ram0400_r), FUNC(apple2gs_state::b0ram0400_w)); // wr 0 rd 1
	m_b0_0400bank[2](0x0400, 0x07ff).rw(FUNC(apple2gs_state::b0ram0400_r), FUNC(apple2gs_state::b1ram0400_w)); // wr 1 rd 0
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
	map(0xe00000, 0xe001ff).view(m_e0_0000bank);
	m_e0_0000bank[0](0x0000, 0x01ff).rw(FUNC(apple2gs_state::e0ram_r<0x0000>), FUNC(apple2gs_state::e0ram_w<0x0000>));
	m_e0_0000bank[1](0x0000, 0x01ff).rw(FUNC(apple2gs_state::e1ram_r<0x0000>), FUNC(apple2gs_state::e1ram_w<0x0000>));

	map(0xe00200, 0xe003ff).view(m_e0_0200bank);
	m_e0_0200bank[0](0x0200, 0x03ff).rw(FUNC(apple2gs_state::e0ram_r<0x0200>), FUNC(apple2gs_state::e0ram_w<0x0200>)); // wr 0 rd 0
	m_e0_0200bank[1](0x0200, 0x03ff).rw(FUNC(apple2gs_state::e1ram_r<0x0200>), FUNC(apple2gs_state::e0ram_w<0x0200>)); // wr 0 rd 1
	m_e0_0200bank[2](0x0200, 0x03ff).rw(FUNC(apple2gs_state::e0ram_r<0x0200>), FUNC(apple2gs_state::e1ram_w<0x0200>)); // wr 1 rd 0
	m_e0_0200bank[3](0x0200, 0x03ff).rw(FUNC(apple2gs_state::e1ram_r<0x0200>), FUNC(apple2gs_state::e1ram_w<0x0200>)); // wr 1 rd 1

	map(0xe00400, 0xe007ff).view(m_e0_0400bank);
	m_e0_0400bank[0](0x0400, 0x07ff).rw(FUNC(apple2gs_state::e0ram_r<0x0400>), FUNC(apple2gs_state::e0ram_w<0x0400>)); // wr 0 rd 0
	m_e0_0400bank[1](0x0400, 0x07ff).rw(FUNC(apple2gs_state::e1ram_r<0x0400>), FUNC(apple2gs_state::e0ram_w<0x0400>)); // wr 0 rd 1
	m_e0_0400bank[2](0x0400, 0x07ff).rw(FUNC(apple2gs_state::e0ram_r<0x0400>), FUNC(apple2gs_state::e1ram_w<0x0400>)); // wr 1 rd 0
	m_e0_0400bank[3](0x0400, 0x07ff).rw(FUNC(apple2gs_state::e1ram_r<0x0400>), FUNC(apple2gs_state::e1ram_w<0x0400>)); // wr 1 rd 1

	map(0xe00800, 0xe01fff).view(m_e0_0800bank);
	m_e0_0800bank[0](0x0800, 0x1fff).rw(FUNC(apple2gs_state::e0ram_r<0x0800>), FUNC(apple2gs_state::e0ram_w<0x0800>));
	m_e0_0800bank[1](0x0800, 0x1fff).rw(FUNC(apple2gs_state::e1ram_r<0x0800>), FUNC(apple2gs_state::e0ram_w<0x0800>));
	m_e0_0800bank[2](0x0800, 0x1fff).rw(FUNC(apple2gs_state::e0ram_r<0x0800>), FUNC(apple2gs_state::e1ram_w<0x0800>));
	m_e0_0800bank[3](0x0800, 0x1fff).rw(FUNC(apple2gs_state::e1ram_r<0x0800>), FUNC(apple2gs_state::e1ram_w<0x0800>));

	map(0xe02000, 0xe03fff).view(m_e0_2000bank);
	m_e0_2000bank[0](0x2000, 0x3fff).rw(FUNC(apple2gs_state::e0ram_r<0x2000>), FUNC(apple2gs_state::e0ram_w<0x2000>));
	m_e0_2000bank[1](0x2000, 0x3fff).rw(FUNC(apple2gs_state::e1ram_r<0x2000>), FUNC(apple2gs_state::e0ram_w<0x2000>));
	m_e0_2000bank[2](0x2000, 0x3fff).rw(FUNC(apple2gs_state::e0ram_r<0x2000>), FUNC(apple2gs_state::e1ram_w<0x2000>));
	m_e0_2000bank[3](0x2000, 0x3fff).rw(FUNC(apple2gs_state::e1ram_r<0x2000>), FUNC(apple2gs_state::e1ram_w<0x2000>));

	map(0xe04000, 0xe0bfff).view(m_e0_4000bank);
	m_e0_4000bank[0](0x4000, 0xbfff).rw(FUNC(apple2gs_state::e0ram_r<0x4000>), FUNC(apple2gs_state::e0ram_w<0x4000>));
	m_e0_4000bank[1](0x4000, 0xbfff).rw(FUNC(apple2gs_state::e1ram_r<0x4000>), FUNC(apple2gs_state::e0ram_w<0x4000>));
	m_e0_4000bank[2](0x4000, 0xbfff).rw(FUNC(apple2gs_state::e0ram_r<0x4000>), FUNC(apple2gs_state::e1ram_w<0x4000>));
	m_e0_4000bank[3](0x4000, 0xbfff).rw(FUNC(apple2gs_state::e1ram_r<0x4000>), FUNC(apple2gs_state::e1ram_w<0x4000>));

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
	if (m_is_rom3)
	{
		// Check the jumper to remove the Control Panel from the Control-Open Apple-Esc CDA menu
		if (m_sysconfig->read() & 0x08)
		{
			return 0x40;
		}
		return 0x00;
	}
	else
	{
		return 0x07;
	}
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
	if (!BIT(data, 5) && BIT(m_adb_p2_last, 5))
	{
		m_adb_reset_freeze = 2;
		m_a2bus->reset_bus();
		m_maincpu->reset();
		m_video->set_newvideo(0x41);

		m_lcram = false;
		m_lcram2 = true;
		m_lcprewrite = false;
		m_lcwriteenable = true;
		m_intcxrom = false;
		m_slotc3rom = false;
		m_video->a80store_w(false);
		m_altzp = false;
		m_ramrd = false;
		m_ramwrt = false;
		m_altzp = false;
		m_video->page2_w(false);
		m_video->res_w(0);

		auxbank_update();
		update_slotrom_banks();
	}

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

	m_adb_p2_last = data;
}

void apple2gs_state::adbmicro_p3_out(u8 data)
{
	if (((data & 0x08) == 0x08) != m_adb_line)
	{
		m_adb_line = (data & 0x8) ? true : false;
		m_macadb->adb_linechange_w(!m_adb_line);
	}

	if (m_adb_reset_freeze == 0)
	{
		m_adb_p3_last = data;
	}
}

void apple2gs_state::set_adb_line(int linestate)
{
	m_adb_line = (linestate == CLEAR_LINE);
}

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
			m_glu_regs[GLU_KG_STATUS] |= KGS_MOUSEX_FULL;
			m_glu_regs[GLU_SYSSTAT] |= GLU_STATUS_MOUSEIRQ;
			keyglu_regen_irqs();
			m_glu_mouse_read_stat = false;  // signal next read will be mouse X
			break;

		case GLU_MOUSEY:
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
				m_glu_mouse_read_stat = true;
				return m_glu_regs[GLU_MOUSEX];
			}
			m_glu_mouse_read_stat = false;
			m_glu_regs[GLU_KG_STATUS] &= ~KGS_MOUSEX_FULL;
			m_glu_regs[GLU_SYSSTAT] &= ~GLU_STATUS_MOUSEIRQ;
			keyglu_regen_irqs();
			return m_glu_regs[GLU_MOUSEY];

		case GLU_SYSSTAT:
			{
				// regenerate sysstat bits
				u8 sysstat = m_glu_regs[GLU_SYSSTAT] & ~0xab;
				if (m_glu_regs[GLU_KG_STATUS] & KGS_COMMAND_FULL)
				{
					sysstat |= GLU_STATUS_CMDFULL;
				}
				if (m_glu_mouse_read_stat)
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

void apple2gs_state::keyglu_regen_irqs()
{
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
}

void apple2gs_state::scc_irq_w(int state)
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
void apple2gs_state::doc_irq_w(int state)
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

INPUT_PORTS_START( apple2gs )
	PORT_START("a2_config")
	PORT_CONFNAME(0x07, 0x00, "CPU type")
	PORT_CONFSETTING(0x00, "Standard")
	PORT_CONFSETTING(0x01, "7 MHz ZipGS")
	PORT_CONFSETTING(0x03, "8 MHz ZipGS")
	PORT_CONFSETTING(0x05, "12 MHz ZipGS")
	PORT_CONFSETTING(0x07, "16 MHz ZipGS")
INPUT_PORTS_END

INPUT_PORTS_START( apple2gsrom3 )
	PORT_INCLUDE( apple2gs )

	PORT_MODIFY("a2_config")
	PORT_CONFNAME(0x08, 0x00, "Disable CDA Control Panel")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x08, DEF_STR(Yes))
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
	m_adbmicro->set_pullups<2>(0x20);
	m_adbmicro->read_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_in));
	m_adbmicro->write_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_out));
	m_adbmicro->read_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_in));
	m_adbmicro->write_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_out));
	m_adbmicro->read_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_in));
	m_adbmicro->write_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_out));
	m_adbmicro->read_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_in));
	m_adbmicro->write_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_out));

	MACADB(config, m_macadb, A2GS_MASTER_CLOCK/8);
	m_macadb->adb_data_callback().set(FUNC(apple2gs_state::set_adb_line));

	RTC3430042(config, m_rtc, XTAL(32'768));

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

	IWM(config, m_iwm, A2GS_7M, A2GS_MASTER_CLOCK/14);
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
	m_adbmicro->set_pullups<2>(0x20);
	m_adbmicro->read_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_in));
	m_adbmicro->write_p<0>().set(FUNC(apple2gs_state::adbmicro_p0_out));
	m_adbmicro->read_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_in));
	m_adbmicro->write_p<1>().set(FUNC(apple2gs_state::adbmicro_p1_out));
	m_adbmicro->read_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_in));
	m_adbmicro->write_p<2>().set(FUNC(apple2gs_state::adbmicro_p2_out));
	m_adbmicro->read_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_in));
	m_adbmicro->write_p<3>().set(FUNC(apple2gs_state::adbmicro_p3_out));
}

void apple2gs_state::apple2gsmt(machine_config &config)
{
	apple2gs(config);

	// SWIM1 344S0061 confirmed on two different Mark Twain boards, with 15.6672 MHz oscillator
	SWIM1(config.replace(), m_iwm, 15.6672_MHz_XTAL);
	m_iwm->phases_cb().set(FUNC(apple2gs_state::phases_w));
	m_iwm->sel35_cb().set(FUNC(apple2gs_state::sel35_w));
	m_iwm->devsel_cb().set(FUNC(apple2gs_state::devsel_w));

	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);
	applefdintf_device::add_35_hd(config, m_floppy[2]);
	applefdintf_device::add_35_hd(config, m_floppy[3]);
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

	ROM_REGION(0x80000,"maincpu",0)
	// 341-0728 is the MASK rom version while 341-0737 is the EPROM version - SAME data.
	ROM_LOAD("341-0728", 0x00000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27) ) /* 341-0728: IIgs ROM03 FC-FD */
	// 341-0748 is the MASK rom version while 341-0749 is the EPROM version - SAME data.
	ROM_LOAD("341-0748", 0x30000, 0x10000, CRC(18190283) SHA1(c70576869deec92ca82c78438b1d5c686eac7480) ) /* 341-0748: IIgs ROM03 FE-FF */
	ROM_CONTINUE ( 0x20000, 0x10000) /* high address line is inverted on PCB? */
ROM_END

ROM_START(apple2gsr3p)
	ROM_REGION(0x1000, "adbmicro", 0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x80000,"maincpu",0)
	ROM_LOAD("341-0728", 0x00000, 0x20000, CRC(8d410067) SHA1(c0f4704233ead14cb8e1e8a68fbd7063c56afd27) ) /* 341-0728: IIgs ROM03 prototype FC-FD - 28 pin MASK rom */
	ROM_LOAD("341-0729", 0x20000, 0x20000, NO_DUMP) /* 341-0729: IIgs ROM03 prototype FE-FF */
ROM_END

ROM_START(apple2gsr1)
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89)) /* need label/part number */

	ROM_REGION(0x80000,"maincpu",0)
	ROM_LOAD("342-0077-b", 0x20000, 0x20000, CRC(42f124b0) SHA1(e4fc7560b69d062cb2da5b1ffbe11cd1ca03cc37)) /* 342-0077-B: IIgs ROM01 */
ROM_END

ROM_START(apple2gsr0)
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x80000,"maincpu",0)
	ROM_LOAD("342-0077-a", 0x20000, 0x20000, CRC(dfbdd97b) SHA1(ff0c245dd0732ec4413a934fd80efc2defd8a8e3) ) /* 342-0077-A: IIgs ROM00 */
ROM_END

ROM_START(apple2gsr0p)  // 6/19/1986 Cortland prototype
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x80000,"maincpu",0)
	ROM_LOAD( "rombf.bin",    0x020000, 0x020000, CRC(ab04fedf) SHA1(977589a17553956d583a21020080a39dd396df5c) )
ROM_END

ROM_START(apple2gsr0p2)  // 3/10/1986 Cortland prototype, boots as "Apple //'ing - Alpha 2.0"
	ROM_REGION(0xc00, "adbmicro", 0)
	ROM_LOAD( "341s0345.bin", 0x000000, 0x000c00, CRC(48cd5779) SHA1(97e421f5247c00a0ca34cd08b6209df573101480) )

	ROM_REGION(0x1000,"gfx1",0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x80000,"maincpu",0)
	ROM_LOAD( "apple iigs alpha rom 2.0 19860310.bin", 0x020000, 0x020000, CRC(a47d275f) SHA1(c5836adcfc8be69c7351b84afa94c814e8d92b81) )
ROM_END

ROM_START(apple2gsmt)
	// 50741 ADB MCU inside the IIgs system unit
	ROM_REGION(0x1000, "adbmicro", 0)
	ROM_LOAD( "341s0632-2.bin", 0x000000, 0x001000, CRC(e1c11fb0) SHA1(141d18c36a617ab9dce668445440d34354be0672) )

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD ( "apple2gs.chr", 0x0000, 0x1000, CRC(91e53cd8) SHA1(34e2443e2ef960a36c047a09ed5a93f471797f89))

	ROM_REGION(0x80000, "maincpu", 0)
	// The Mark Twain ROM is 512K, with address bit 16 inverted (same as ROM 3).
	// The first 256K is filled with 64K of 0xF0, then 0xF1, 0xF2, and 0xF3.  I'm guessing this was meant to be
	// a small ROM disk at $F00000, like the Mac Classic has.  The second 256K is the firmware we all know from
	// the "System 6.0.1" source leak.
	ROM_LOAD( "mtrom.bin", 0x040000, 0x040000, CRC(d75414c5) SHA1(7054915f5e5f9f3bb2cbecf6830d4f80793694a6) )
	ROM_CONTINUE(0x10000, 0x10000)
	ROM_CONTINUE(0x00000, 0x10000)
	ROM_CONTINUE(0x30000, 0x10000)
	ROM_CONTINUE(0x20000, 0x10000)

	// The firmware for the built-in High-Speed SCSI Card
	ROM_REGION(0x8000, "hsscsi", 0)
	ROM_LOAD( "mtscsi.bin",   0x000000, 0x008000, CRC(7426c880) SHA1(1c16310e5c180701a05089d69c6e72e9dc7434f6) )
ROM_END

} // Anonymous namespace


/*    YEAR  NAME          PARENT    COMPAT  MACHINE     INPUT         CLASS           INIT       COMPANY           FULLNAME */
COMP( 1989, apple2gs,     0,        apple2, apple2gs,   apple2gsrom3, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03)", MACHINE_SUPPORTS_SAVE )
COMP( 198?, apple2gsr3p,  apple2gs, 0,      apple2gs,   apple2gsrom3, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (ROM03 prototype)", MACHINE_NOT_WORKING )
COMP( 1987, apple2gsr1,   apple2gs, 0,      apple2gsr1, apple2gs,     apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM01)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0,   apple2gs, 0,      apple2gsr1, apple2gs,     apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p,  apple2gs, 0,      apple2gsr1, apple2gs,     apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 6/19/1986)", MACHINE_SUPPORTS_SAVE )
COMP( 1986, apple2gsr0p2, apple2gs, 0,      apple2gsr1, apple2gs,     apple2gs_state, rom1_init, "Apple Computer", "Apple IIgs (ROM00 prototype 3/10/1986)", MACHINE_SUPPORTS_SAVE )
COMP( 1991, apple2gsmt,   apple2gs, 0,      apple2gsmt, apple2gsrom3, apple2gs_state, rom3_init, "Apple Computer", "Apple IIgs (1991 Mark Twain prototype)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
