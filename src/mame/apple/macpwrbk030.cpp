// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    macpwrbk030.cpp
    Mac PowerBooks with a 68030 CPU and M50753 PMU
    By R. Belmont

    These are basically late-period Mac IIs without NuBus and with
    Egret/Cuda replaced by the PMU.

    Generation 1:
    PowerBook 140: 16 MHz 68030, 2 MiB RAM, no FPU, passive-matrix screen
    PowerBook 145: 140 with 25 MHz 68030
    PowerBook 145B: 140 with 25 MHz 68030 and 4 MiB of RAM standard
    PowerBook 170: 140, with 25 MHz 68030, 68881 FPU, active-matrix screen

    Generation 2: (all models include external monitor support)
    PowerBook 160: 25 MHz 68030, no FPU, passive-matrix screen with 2 bits per pixel grayscale
    PowerBook 180: 33 MHz 68030, 68881 FPU, active-matrix screen with 2 bits per pixel grayscale
    PowerBook 165: 160 with 33 MHz 68030
    PowerBook 165c: 160 with 33 MHz 68030, FPU, color 640x400 display
    PowerBook 180c: 165c with color 640x480 display

    Driver features:
    - Display, audio, floppy, SCSI, and ADB all work.  You can boot compatible System versions
      and run arbitrary software.
    - FPU presence and Jaws/Niagra CPU speed readback are supported so that Gestalt properly
      identifies all models (except the 145B is shown as a 145; Apple documents this as also
      occuring on hardware).
    - 165c/180c use of a VGA GPIO feature bit to determine the correct model is supported.
    - Sleep/suspend and wake-up works on all models.

    Driver TODOs:
    - External video interface on 160/165/165c/180/180c.  Need to make this a slot interface
      because MAME doesn't otherwise support optionally adding a screen.

    ============================================================================
    Technical info

    VIA 1 connections: (PowerBook 140/170, 160/180/180C are similar)
    Port A: 0: VIA_TEST
            1: CPU_ID0
            2: CPU_ID1
            3: MODEM
            4: CPU_ID2
            5: HDSEL (floppy head select)
            6: CPU_ID3
            7: SCC REQ

    Port B: 0: RTC DATA
            1: RTC CLOCK
            2: RTC
            3: LINK SEL
            4: N/C
            5: N/C
            6: N/C
            7: N/C

    CA1: 60 Hz clock
    CA2: 1 second clock
    CB1: PMU IRQ
    CB2: MODEM SND EN

    VIA 2 connections: (VIA2 is a pseudo-VIA inside the "Peripheral Glue" ASIC)
    Port A: bi-directional PMU data bus

    Port B: 0: SV1
            1: PMACK
            2: PMREQ
            3: SV0
            4: SV2
            5: HMMU
            6: N/C
            7: MODEM RESET

    PMU (M50753) connections:
    IN port: 0: BRITE SENSE
             1: A/D BATT     (battery charge level)
             2: SND CNTL
             3: MODEM BUSY
             4: PWR ON       (input, 1 = system power on)
             5: TEMP A/D     (battery temperature)
             6: NICAD SLA
             7: TABLE SEL    (A/D input, some kind of battery state)

    Port 0: 0: SWIM CNTL
            1: SCC CNTL
            2: HD PWR
            3: MODEM PWR
            4: N/C
            5: SOUND PWR
            6: MODEM PWROUT
            7: SYS_PWR

    Port 1: 0: CCFL PWR CNTL
            1: AKD              (input, works like the high bit of $C000 on the Apple II, except includes the modifiers)
            2: STOP CLK
            3: CHRG ON          (input, 1 = charger is on, 7.1.1 Battery applet shows charging symbol)
            4: KBD RST          (output, resets keyboard M50740)
            5: HICHG            (output)
            6: RING DETECT
            7: CHG OFF          (output)

    Port 2: bi-directional data bus, connected to VIA port A

    Port 3: 0: PWR OFF
            1: SYS RST
            2: VIA TEST
            3: SOUND OFF
            4: 1 SEC        (input from Mac Plus RTC/PRAM chip, never read by the PMU program)
            5: PMINT
            6: PMACK
            7: PMREQ

    Port 4: 0: PMGR_ADB (ADB out)
            1: ADB (ADB in)
            2: DISP BLANK
            3: MODEM_INS

    INT1: 60 Hz clock
    INT2: INT2 PULLUP (pulled up and otherwise N/C)

****************************************************************************/

#include "emu.h"

#include "dfac.h"
#include "gsc.h"
#include "macadb.h"
#include "macrtc.h"
#include "macscsi.h"
#include "mactoolbox.h"
#include "pseudovia.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m6502/m5074x.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/applefdintf.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"
#include "sound/asc.h"
#include "video/wd90c26.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"

namespace {
class macpb030_state : public driver_device
{
public:
	macpb030_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pmu(*this, "pmu"),
		m_via1(*this, "via1"),
		m_pseudovia(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_rtc(*this, "rtc"),
		m_gsc(*this, "gsc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_asc(*this, "asc"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram"),
		m_ext_vram(*this, "ext_vram"),
		m_vga(*this, "vga"),
		m_ram_ptr(nullptr),
		m_rom_ptr(nullptr),
		m_ram_mask(0),
		m_ram_size(0),
		m_rom_size(0),
		m_via_interrupt(0),
		m_via2_interrupt(0),
		m_scc_interrupt(0),
		m_last_taken_interrupt(0),
		m_ca1_data(0),
		m_adb_line(0),
		m_overlay(false),
		m_cur_floppy(nullptr),
		m_hdsel(0),
		m_pmu_blank_display(false),
		m_pmu_from_via(0),
		m_pmu_to_via(0),
		m_pmu_ack(0),
		m_pmu_req(0),
		m_pangola_data(0x1e),
		m_ponti_modem_ctl(0),
		m_ponti_snd_ctl(0),
		m_ponti_SPI_SR(0),
		m_ponti_backlight_ctl(0)
	{
	}

	void macpb140(machine_config &config);
	void macpb145(machine_config &config);
	void macpb145b(machine_config &config);
	void macpb160(machine_config &config);
	void macpb165(machine_config &config);
	void macpb165c(machine_config &config);
	void macpb170(machine_config &config);
	void macpb180(machine_config &config);
	void macpb180c(machine_config &config);
	void macpb140_map(address_map &map) ATTR_COLD;
	void macpb160_map(address_map &map) ATTR_COLD;
	void macpb165c_map(address_map &map) ATTR_COLD;

private:
	required_device<m68030_device> m_maincpu;
	required_device<m50753_device> m_pmu;
	required_device<via6522_device> m_via1;
	required_device<pseudovia_device> m_pseudovia;
	required_device<macadb_device> m_macadb;
	required_device<ncr53c80_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rtc3430042_device> m_rtc;
	optional_device<gsc_device> m_gsc;
	optional_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<asc_device> m_asc;
	required_device<z80scc_device> m_scc;
	optional_shared_ptr<u32> m_vram, m_ext_vram;
	optional_device<wd90c26_vga_device> m_vga;

	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size, m_rom_size;

	emu_timer *m_6015_timer;

	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	int m_ca1_data;
	int m_adb_line, m_adb_akd;

	bool m_overlay;

	floppy_image_device *m_cur_floppy;
	int m_hdsel;

	bool m_pmu_blank_display;
	u8 m_pmu_from_via, m_pmu_to_via, m_pmu_ack, m_pmu_req;

	u16 m_pangola_data;

	u8 m_ponti_modem_ctl, m_ponti_snd_ctl, m_ponti_SPI_SR, m_ponti_backlight_ctl;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update_ddc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_vga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 via_r(offs_t offset);
	void via_w(offs_t offset, u16 data, u16 mem_mask);
	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	void field_interrupts();
	void via_sync();
	void scc_irq_w(int state);
	void via_irq_w(int state);
	u8 via2_r(offs_t offset);
	void via2_w(offs_t offset, u8 data);
	u8 via2_in_a();
	u8 via2_in_b();
	void via2_out_a(u8 data);
	void via2_out_b(u8 data);
	void via2_irq_w(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	u32 rom_switch_r(offs_t offset);
	u16 scsi_r(offs_t offset, u16 mem_mask);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask);
	u32 scsi_drq_r(offs_t offset, u32 mem_mask = ~0);
	void scsi_drq_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void scsi_berr_w(u8 data);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	void phases_w(u8 phases);
	void devsel_w(u8 devsel);
	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
	u32 buserror_r();

	u32 jaws_r(offs_t offset, u32 mem_mask);
	void jaws_w(offs_t offset, u32 data, u32 mem_mask);
	u32 niagra_r(offs_t offset, u32 mem_mask);
	void niagra_w(offs_t offset, u32 data, u32 mem_mask);

	u16 pangola_r();
	void pangola_w(u16 data);
	u8 pangola_vram_r(offs_t offset);
	void pangola_vram_w(offs_t offset, u8 data);

	u8 ext_video_r(offs_t offset);
	void ext_video_w(offs_t offset, u8 data);

	u8 pmu_p1_r();
	u8 pmu_data_r();
	void pmu_data_w(u8 data);
	u8 pmu_comms_r();
	void pmu_comms_w(u8 data);
	void set_adb_line(int state);
	void set_adb_anykeydown(int state);
	u8 pmu_p4_r();
	void pmu_p4_w(u8 data);
	u8 pmu_in_r();
	u8 battery_r();
	u8 battery2_r();
	u8 battery3_r();
	u8 brightness_r();
};

void macpb030_state::machine_start()
{
	m_ram_ptr = (u32 *)m_ram->pointer();
	m_ram_size = m_ram->size() >> 1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u32 *)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;

	m_6015_timer = timer_alloc(FUNC(macpb030_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	/*
	   HACK-ish: There is an uninitialized variable in the PMU code that can
	   cause the charger connected state to not be checked until after
	   the 68K startup code reads it and makes decisions.  On hardware this
	   likely is ameliorated by the PMU always having power unless the
	   battery goes 100% dead.

	   In normal operation this location counts down and when it reaches zero,
	   the charger and battery states are refreshed.  The variable is then set to
	   either 0x3c or 0x5b depending on battery state.  The important part is that
	   in normal operation it never is allowed to wrap to 0xff, which was happening
	   in MAME due to RAM being initialized to zero.
	*/
	m_pmu->space(AS_PROGRAM).write_byte(0x25, 0x5b);

	m_maincpu->space(AS_PROGRAM).install_write_tap(0x50f14000, 0x50f15fff, "snd_latch_mon", [this](offs_t offset, u32 &data, u32 mem_mask)                                         {
		if (!machine().side_effects_disabled())
		{
			this->m_ponti_snd_ctl |= 0x08;          // indicate sound chip write so power management knows not to sleep
		}
	});

	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_ca1_data));
	save_item(NAME(m_adb_line));
	save_item(NAME(m_adb_akd));
	save_item(NAME(m_overlay));
	save_item(NAME(m_hdsel));
	save_item(NAME(m_pmu_blank_display));
	save_item(NAME(m_pmu_from_via));
	save_item(NAME(m_pmu_to_via));
	save_item(NAME(m_pmu_ack));
	save_item(NAME(m_pmu_req));
	save_item(NAME(m_pangola_data));
	save_item(NAME(m_ponti_modem_ctl));
	save_item(NAME(m_ponti_snd_ctl));
	save_item(NAME(m_ponti_SPI_SR));
	save_item(NAME(m_ponti_backlight_ctl));
}

void macpb030_state::machine_reset()
{
	m_overlay = true;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;

	m_cur_floppy = nullptr;
	m_hdsel = 0;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	// main cpu shouldn't start until PMU wakes it up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

// Jaws memory controller for the PowerBook 140/170 and 145/145B.
u32 macpb030_state::jaws_r(offs_t offset, u32 mem_mask)
{
	switch (offset >> 10)
	{
		case 0x00:      // RAM wait state control
		case 0x04:      // Econo-Mode register
		case 0x06:      // ROM wait state control
		case 0x10:      // RAM bank A config
		case 0x12:      // RAM bank B config
		case 0x14:      // RAM bank C config
		case 0x20:      // CPU power off control
		case 0x22:      // Set CPU clock
		case 0x30:      // Select plain SCC vs. 85C80 "Combo" SCC + 53C80 SCSI
		case 0x32:      // Puts RAM into self-refresh mode
			break;

		case 0x34: // Get CPU clock
			if (m_maincpu->clock() == 25'000'000)
			{
				return 1<<24;
			}
			break;
	}
	return 0;
}

void macpb030_state::jaws_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> 10)
	{
		case 0x00:      // RAM wait state control
		case 0x04:      // Econo-Mode register
		case 0x06:      // ROM wait state control
		case 0x10:      // RAM bank A config
		case 0x12:      // RAM bank B config
		case 0x14:      // RAM bank C config
		case 0x22:      // Set CPU clock
		case 0x30:      // Select plain SCC vs. 85C80 "Combo" SCC + 53C80 SCSI
		case 0x32:      // Puts RAM into self-refresh mode
		case 0x34:      // Get CPU clock
			break;

		case 0x20:      // CPU power down control, expects the CPU to reset after a short delay to resume
						// We make that no delay, since we don't care about saving power.
			m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			break;
	}
}

// Niagra (Jaws derivative) memory controller for the PowerBook 160/180 and 165c/180c.
u32 macpb030_state::niagra_r(offs_t offset, u32 mem_mask)
{
	switch (offset >> 10)
	{
		case 0x02:      // Video count options
		case 0x22:      // Video accesses through 64
		case 0x24:      // Video accesses through 512
		case 0x26:      // Video accesses through 2K
		case 0x30:      // Enable flash through Niagra
			break;

		case 0x34: // FPU access detected
			return 0xff000000;

		case 0x36: // CPU speed register
			if (m_maincpu->clock() == 25'000'000)
			{
				return 2<<24;
			}
			else if (m_maincpu->clock() == 33'000'000)
			{
				return 3<<24;
			}
			return 0;

		case 0x16:      // "Ponti registers"
			switch ((offset >> 8) & 3)
			{
				case 0:     // SPI modem control
					return m_ponti_modem_ctl << 24;

				case 1:     // Sound control
					return m_ponti_snd_ctl << 24;

				case 2:     // SPI shift register
					return m_ponti_SPI_SR << 24;

				case 3:     // Backlight control
					return m_ponti_backlight_ctl << 24;
			}
			break;

		default:
			return jaws_r(offset, mem_mask);
	}

	return 0;
}

void macpb030_state::niagra_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> 10)
	{
		case 0x02:      // Video count options
		case 0x22:      // Video accesses through 64
		case 0x24:      // Video accesses through 512
		case 0x26:      // Video accesses through 2K
		case 0x30:      // Enable flash through Niagra
		case 0x34:      // FPU access detected
		case 0x36:      // CPU speed register
			break;

		case 0x16: // "Ponti registers"
			data >>= 24;
			switch ((offset >> 8) & 3)
			{
			case 0: // SPI modem control
				m_ponti_modem_ctl = data;
				break;

			case 1: // Sound control
				// if the sound latch clear is asserted, clear the latch
				if (BIT(data, 2) && !BIT(m_ponti_snd_ctl, 2))
				{
					m_ponti_snd_ctl &= ~0x08;
				}

				// preserve the value of the sound latch output
				m_ponti_snd_ctl &= 0x08;
				m_ponti_snd_ctl |= data & ~0x08;
				break;

			case 2: // SPI shift register
				m_ponti_SPI_SR = data;
				break;

			case 3: // Backlight control
				m_ponti_backlight_ctl = data;
				break;
			}
			break;

		default:
			jaws_w(offset, data, mem_mask);
			break;
	}
}

/*
    Color PowerBooks used a stock Western Digital SVGA chipset that could drive LCDs,
    but in order to maintain compatibility with all Mac video depths, the Pangola chip was
    inserted in between.  The SVGA chip is always run at 8 bits per pixel, while Pangola
    sits in between and does the necessary conversion between 1, 2, and 4 bpp on the Mac
    end and 8 bpp on QuickDraw's end.  In 8bpp mode it's just a passthrough.
*/
u16 macpb030_state::pangola_r()
{
	return m_pangola_data;
	// TODO: trace pins, 0x13 -> 0x17 -> 0x16 sequence written before waking up VGA core
}

void macpb030_state::pangola_w(u16 data)
{
	m_pangola_data = data;
}

u8 macpb030_state::pangola_vram_r(offs_t offset)
{
	switch ((m_pangola_data >> 5) & 3)
	{
		case 0: // 8 bpp, do the default thing
			break;

		case 1: // 4bpp
			offset <<= 1;
			return (m_vga->mem_linear_r(offset) << 4) | (m_vga->mem_linear_r(offset+1) & 0xf);

		case 2: // 2bpp
			offset <<= 2;
			return (m_vga->mem_linear_r(offset) << 6) |
				   (m_vga->mem_linear_r(offset + 1) & 0x3) << 4 |
				   (m_vga->mem_linear_r(offset + 2) & 0x3) << 2 |
				   (m_vga->mem_linear_r(offset + 3) & 0x3);

		case 3: // 1bpp
			offset <<= 3;
			return (m_vga->mem_linear_r(offset) << 7) |
				   (m_vga->mem_linear_r(offset + 1) & 0x1) << 6 |
				   (m_vga->mem_linear_r(offset + 2) & 0x1) << 5 |
				   (m_vga->mem_linear_r(offset + 3) & 0x1) << 4 |
				   (m_vga->mem_linear_r(offset + 4) & 0x1) << 3 |
				   (m_vga->mem_linear_r(offset + 5) & 0x1) << 2 |
				   (m_vga->mem_linear_r(offset + 6) & 0x1) << 1 |
				   (m_vga->mem_linear_r(offset + 7) & 0x1);
	}

	return m_vga->mem_linear_r(offset);
}

void macpb030_state::pangola_vram_w(offs_t offset, u8 data)
{
	switch ((m_pangola_data >> 5) & 3)
	{
		case 0: // 8 bpp, passthrough
			m_vga->mem_linear_w(offset, data);
			break;

		case 1: // 4bpp
			offset <<= 1;
			m_vga->mem_linear_w(offset, data>>4);
			m_vga->mem_linear_w(offset+1, data & 0xf);
			break;

		case 2: // 2bpp
			offset <<= 2;
			m_vga->mem_linear_w(offset, data >> 6);
			m_vga->mem_linear_w(offset + 1, (data >> 4) & 0x3);
			m_vga->mem_linear_w(offset + 2, (data >> 2) & 0x3);
			m_vga->mem_linear_w(offset + 3, data & 0x3);
			break;

		case 3: // 1bpp
			offset <<= 3;
			m_vga->mem_linear_w(offset, data >> 7);
			m_vga->mem_linear_w(offset + 1, (data >> 6) & 0x1);
			m_vga->mem_linear_w(offset + 2, (data >> 5) & 0x1);
			m_vga->mem_linear_w(offset + 3, (data >> 4) & 0x1);
			m_vga->mem_linear_w(offset + 4, (data >> 3) & 0x1);
			m_vga->mem_linear_w(offset + 5, (data >> 2) & 0x1);
			m_vga->mem_linear_w(offset + 6, (data >> 1) & 0x1);
			m_vga->mem_linear_w(offset + 7, data & 0x1);
			break;
	}
}

/*
    PB160/180 external video (stub for now)
*/
u8 macpb030_state::ext_video_r(offs_t offset)
{
	switch (offset)
	{
		case 4:             // Monitor ID in bits 4-6.  Return 7 (no connection) for now.
			return 0x70;    // No monitor for now
	}

	return 0;
}

void macpb030_state::ext_video_w(offs_t offset, u8 data)
{
	// 0 = DAC color number
	// 1 = DAC color write (write R, then G, then B, like usual)
	// 8 = depth (0=1bpp, 1=2bpp, 2=4bpp, 3=8bpp, 4=16bpp)
	// 60+61 = visible vertical area (LSB in 60, MSB in 61)
}

u8 macpb030_state::pmu_in_r()
{
	// power on, no target disk mode
	return 0x30;
}

u8 macpb030_state::brightness_r()
{
	return 0x7f;
}

// ADC 1 - battery level
u8 macpb030_state::battery_r()
{
	return 0xff;
}

// ADC 5 - battery temperature
u8 macpb030_state::battery2_r()
{
	return 0x40;
}

// ADC 7 - "TABLE SEL" on the schematic
u8 macpb030_state::battery3_r()
{
	return 0x10;
}

void macpb030_state::set_adb_line(int state)
{
	m_adb_line = state;
}

void macpb030_state::set_adb_anykeydown(int state)
{
	m_adb_akd = state;
}

u8 macpb030_state::pmu_p1_r()
{
	if (m_adb_akd)
	{
		return 0x88 | 0x02;
	}

	return 0x88;
}

u8 macpb030_state::pmu_data_r()
{
	return m_pmu_from_via;
}

void macpb030_state::pmu_data_w(u8 data)
{
	m_pmu_to_via = data;
}

u8 macpb030_state::pmu_comms_r()
{
	return (m_pmu_req << 7);
}

void macpb030_state::pmu_comms_w(u8 data)
{
	if (!BIT(data, 1))
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
		const u32 memory_end = memory_size - 1;
		offs_t memory_mirror = memory_end & ~(memory_size - 1);

		space.unmap_write(0x00000000, memory_end);
		space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
		m_overlay = true;
	}

	m_maincpu->set_input_line(INPUT_LINE_HALT, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);

	m_via1->write_cb1(BIT(data, 5) ^ 1);
	if (m_pmu_ack != BIT(data, 6))
	{
		m_pmu_ack = BIT(data, 6);
		machine().scheduler().synchronize();
	}
}

u8 macpb030_state::pmu_p4_r()
{
	return (m_adb_line << 1);
}

void macpb030_state::pmu_p4_w(u8 data)
{
	m_macadb->adb_linechange_w((data & 1) ^ 1);
	m_pmu_blank_display = BIT(data, 2) ^ 1;
	if (m_gsc)
	{
		m_gsc->set_pmu_blank(m_pmu_blank_display);
	}
}

u32 macpb030_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

u16 macpb030_state::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_swim->read((offset >> 8) & 0xf);
	return result << 8;
}

void macpb030_state::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_swim->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_swim->write((offset >> 8) & 0xf, data >> 8);
}

u16 macpb030_state::scc_r(offs_t offset)
{
	via_sync();
	const u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void macpb030_state::scc_w(offs_t offset, u16 data)
{
	m_scc->dc_ab_w(offset, data >> 8);
}

void macpb030_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void macpb030_state::via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

void macpb030_state::phases_w(u8 phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macpb030_state::devsel_w(u8 devsel)
{
	if (devsel == 1)
	{
		m_cur_floppy = m_floppy[0]->get_device();
	}
	else if (devsel == 2)
	{
		m_cur_floppy = m_floppy[1]->get_device();
	}
	else
	{
		m_cur_floppy = nullptr;
	}
	m_swim->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(m_hdsel);
	}
}

u32 macpb030_state::screen_update_ddc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// is the display enabled?
	if (m_pmu_blank_display)
	{
		bitmap.fill(1, cliprect);
		return 0;
	}

	u16 const *const video_ram = (const u16 *)m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u16 *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			u16 const word = video_ram[((y * 640) / 16) + ((x / 16) ^ 1)];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

u32 macpb030_state::screen_update_vga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_pmu_blank_display)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	return m_vga->screen_update(screen, bitmap, cliprect);
}

u16 macpb030_state::via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via1->read(offset);

	if (!machine().side_effects_disabled())
		via_sync();

	return (data & 0xff) | (data << 8);
}

void macpb030_state::via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);

	via_sync();
}

void macpb030_state::scc_irq_w(int state)
{
	m_scc_interrupt = state;
	field_interrupts();
}

void macpb030_state::via_irq_w(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

u8 macpb030_state::via2_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		via_sync();
	}

	return m_pseudovia->read(offset);
}

void macpb030_state::via2_w(offs_t offset, u8 data)
{
	via_sync();
	m_pseudovia->write(offset, data);
}

void macpb030_state::via2_irq_w(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

u32 macpb030_state::rom_switch_r(offs_t offset)
{
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

TIMER_CALLBACK_MEMBER(macpb030_state::mac_6015_tick)
{
	/* signal VBlank on CA1 input on the VIA */
	m_ca1_data ^= 1;
	m_via1->write_ca1(m_ca1_data);

	m_pmu->set_input_line(m50753_device::M50753_INT1_LINE, ASSERT_LINE);
	m_macadb->portable_update_keyboard();
}

u16 macpb030_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset >= 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void macpb030_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset >= 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data>>8);
}

u32 macpb030_state::scsi_drq_r(offs_t offset, u32 mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			return m_scsihelp->read_wrapper(true, 6)<<24;

		case 0xffff0000:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16);

		case 0xffffffff:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16) | (m_scsihelp->read_wrapper(true, 6)<<8) | m_scsihelp->read_wrapper(true, 6);

		default:
			logerror("scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

void macpb030_state::scsi_drq_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			break;

		case 0xffff0000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			break;

		case 0xffffffff:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			m_scsihelp->write_wrapper(true, 0, data>>8);
			m_scsihelp->write_wrapper(true, 0, data&0xff);
			break;

		default:
			logerror("scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
			break;
	}
}

void macpb030_state::scsi_berr_w(u8 data)
{
	m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
}

/***************************************************************************
    ADDRESS MAPS
****************************************************************************/

// ROM detects the "Jaws" ASIC by checking for I/O space mirrored at 0x01000000 boundries
void macpb030_state::macpb140_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(macpb030_state::via_r), FUNC(macpb030_state::via_w)).mirror(0x01f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(macpb030_state::via2_r), FUNC(macpb030_state::via2_w)).mirror(0x01f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(macpb030_state::scc_r), FUNC(macpb030_state::scc_w)).mirror(0x01f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w)).mirror(0x01f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w)).mirror(0x01f00000);
	map(0x50012060, 0x50012063).r(FUNC(macpb030_state::scsi_drq_r)).mirror(0x01f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x01f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w)).mirror(0x01f00000);
	map(0x50024000, 0x50027fff).r(FUNC(macpb030_state::buserror_r)).mirror(0x01f00000); // bus error here to make sure we aren't mistaken for another decoder
	map(0x50080000, 0x500bffff).rw(FUNC(macpb030_state::jaws_r), FUNC(macpb030_state::jaws_w)).mirror(0x01f00000);

	// Video uses the mirror at fee08000, but the Power Manager stashes some sleep data in the
	// lower 32K, so this *must* be mirrored
	map(0xfee00000, 0xfee07fff).ram().share("vram").mirror(0x00008000);
}

void macpb030_state::macpb160_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x6fffffff).m(m_gsc, FUNC(gsc_device::map));

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::via_r), FUNC(macpb030_state::via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::via2_r), FUNC(macpb030_state::via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::scc_r), FUNC(macpb030_state::scc_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder
	map(0x50f80000, 0x50fbffff).rw(FUNC(macpb030_state::niagra_r), FUNC(macpb030_state::niagra_w));

	// external video on 160/180
	map(0xfe0fe000, 0xfe0fe0ff).rw(FUNC(macpb030_state::ext_video_r), FUNC(macpb030_state::ext_video_w));
	map(0xfe100000, 0xfe17ffff).ram().share("ext_vram");
}

void macpb030_state::macpb165c_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::via_r), FUNC(macpb030_state::via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::via2_r), FUNC(macpb030_state::via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::scc_r), FUNC(macpb030_state::scc_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w));
	map(0x50f20000, 0x50f21fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to detect we're not the grayscale 160/165/180
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder
	map(0x50f80000, 0x50fbffff).rw(FUNC(macpb030_state::niagra_r), FUNC(macpb030_state::niagra_w));

	// on-board color video on 165c/180c, presumably under ISA bus
	map(0xfc000000, 0xfc07ffff).rw(FUNC(macpb030_state::pangola_vram_r), FUNC(macpb030_state::pangola_vram_w)).mirror(0x00380000);
	//map(0xfc400102, 0xfc400102).w(wd90c26_vga_device::wakeup_w));
	map(0xfc4003b0, 0xfc4003df).m(m_vga, FUNC(wd90c26_vga_device::io_map));
	// TODO: trace $3d0 writes (doesn't belong to WD90C26 core, RAMDAC overlay?)
	map(0xfc4046e8, 0xfc4046e8).mirror(0x3000).w(m_vga, FUNC(wd90c26_vga_device::mode_setup_w));

	map(0xfc800000, 0xfc800003).rw(FUNC(macpb030_state::pangola_r), FUNC(macpb030_state::pangola_w));
	map(0xfcff8000, 0xfcffffff).rom().region("vrom", 0x0000);

	// external video on 165c/180c
	map(0xfe0fe000, 0xfe0fe0ff).rw(FUNC(macpb030_state::ext_video_r), FUNC(macpb030_state::ext_video_w));
	map(0xfe100000, 0xfe17ffff).ram().share("ext_vram");
}

u8 macpb030_state::via_in_a()
{
	return 0x81 | 0x12; // ID for 140/160
}

u8 macpb030_state::via_in_b()
{
	return 0x08 | m_rtc->data_r();    // flag indicating no Target Disk Mode
}

void macpb030_state::via_out_a(u8 data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void macpb030_state::via_out_b(u8 data)
{
	m_rtc->ce_w(BIT(data, 2));
	m_rtc->data_w(BIT(data, 0));
	m_rtc->clk_w(BIT(data, 1));
}

u8 macpb030_state::via2_in_a()
{
	return m_pmu_to_via;
}

u8 macpb030_state::via2_in_b()
{
	// Must also return the pmu_req state here or bset/bclr operations on other
	// bits in this port will accidentally clear pmu_req and cause CPU/PMU comms
	// problems!  The ROM code for sleeping on all of these machines does that.
	return ((m_pmu_ack & 1) << 1) | (m_pmu_req << 2);
}

void macpb030_state::via2_out_a(u8 data)
{
	m_pmu_from_via = data;
}

void macpb030_state::via2_out_b(u8 data)
{
	if (m_pmu_req != BIT(data, 2))
	{
		m_pmu_req = BIT(data, 2);
		machine().scheduler().synchronize();
	}
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macpb030_state::macpb140(machine_config &config)
{
	M68030(config, m_maincpu, 31.3344_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb140_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");
	m_maincpu->set_fpu_enable(false);

	M50753(config, m_pmu, 3.93216_MHz_XTAL);
	m_pmu->read_p<1>().set(FUNC(macpb030_state::pmu_p1_r));
	m_pmu->read_p<2>().set(FUNC(macpb030_state::pmu_data_r));
	m_pmu->write_p<2>().set(FUNC(macpb030_state::pmu_data_w));
	m_pmu->read_p<3>().set(FUNC(macpb030_state::pmu_comms_r));
	m_pmu->write_p<3>().set(FUNC(macpb030_state::pmu_comms_w));
	m_pmu->read_p<4>().set(FUNC(macpb030_state::pmu_p4_r));
	m_pmu->write_p<4>().set(FUNC(macpb030_state::pmu_p4_w));
	m_pmu->read_in_p().set(FUNC(macpb030_state::pmu_in_r));
	m_pmu->ad_in<0>().set(FUNC(macpb030_state::brightness_r));
	m_pmu->ad_in<1>().set(FUNC(macpb030_state::battery_r));
	m_pmu->ad_in<5>().set(FUNC(macpb030_state::battery2_r));
	m_pmu->ad_in<8>().set(FUNC(macpb030_state::battery3_r));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_ddc));

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MACADB(config, m_macadb, 31.3344_MHz_XTAL/2);
	m_macadb->adb_data_callback().set(FUNC(macpb030_state::set_adb_line));
	m_macadb->adb_akd_callback().set(FUNC(macpb030_state::set_adb_anykeydown));

	RTC3430042(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_swim, 31.3344_MHz_XTAL / 2);
	m_swim->phases_cb().set(FUNC(macpb030_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macpb030_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(m_pseudovia, FUNC(pseudovia_device::scsi_irq_w));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macpb030_state::scsi_berr_w));

	SCC85C30(config, m_scc, 31.3344_MHz_XTAL / 4);
	m_scc->out_int_callback().set(FUNC(macpb030_state::scc_irq_w));

	R65C22(config, m_via1, 31.3344_MHz_XTAL / 20);
	m_via1->readpa_handler().set(FUNC(macpb030_state::via_in_a));
	m_via1->readpb_handler().set(FUNC(macpb030_state::via_in_b));
	m_via1->writepa_handler().set(FUNC(macpb030_state::via_out_a));
	m_via1->writepb_handler().set(FUNC(macpb030_state::via_out_b));
	m_via1->irq_handler().set(FUNC(macpb030_state::via_irq_w));

	APPLE_PSEUDOVIA(config, m_pseudovia, 31.3344_MHz_XTAL / 20);
	m_pseudovia->readpa_handler().set(FUNC(macpb030_state::via2_in_a));
	m_pseudovia->readpb_handler().set(FUNC(macpb030_state::via2_in_b));
	m_pseudovia->writepa_handler().set(FUNC(macpb030_state::via2_out_a));
	m_pseudovia->writepb_handler().set(FUNC(macpb030_state::via2_out_b));
	m_pseudovia->irq_callback().set(FUNC(macpb030_state::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, 22.5792_MHz_XTAL, asc_device::asc_type::EASC);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

// PowerBook 145 = 140 @ 25 MHz (still 2MB RAM)
void macpb030_state::macpb145(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25_MHz_XTAL);
}

// PowerBook 145B = 140 @ 25 MHz with 4MB RAM
void macpb030_state::macpb145b(machine_config &config)
{
	macpb145(config);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

// PowerBook 170 = 140 @ 25 MHz with an active-matrix LCD (140/145/145B were passive)
void macpb030_state::macpb170(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25_MHz_XTAL);
	m_maincpu->set_fpu_enable(true);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

void macpb030_state::macpb160(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb160_map);
	m_maincpu->set_fpu_enable(false);

	config.device_remove("screen");
	config.device_remove("palette");

	GSC(config, m_gsc, 31.3344_MHz_XTAL);
	m_gsc->set_panel_id(5);

	m_ram->set_extra_options("4M,6M,8M,12M,14M");
}

void macpb030_state::macpb165(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33_MHz_XTAL);
}

void macpb030_state::macpb180(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33_MHz_XTAL);
	m_maincpu->set_fpu_enable(true);
}

void macpb030_state::macpb165c(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(33_MHz_XTAL);
	m_maincpu->set_fpu_enable(true);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb165c_map);

	m_screen->set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_vga));
	m_screen->set_no_palette();

	WD90C26(config, m_vga, 0);
	m_vga->set_screen(m_screen);
	// 512KB
	m_vga->set_vram_size(0x80000);
	// model ID: 0 = 180c, 1 = 165c
	m_vga->read_cnf15_callback().set_constant(1);
}

void macpb030_state::macpb180c(machine_config &config)
{
	macpb165c(config);
	m_vga->read_cnf15_callback().set_constant(0);
}

ROM_START(macpb140)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

#define rom_macpb145 rom_macpb140
#define rom_macpb145b rom_macpb140
#define rom_macpb170 rom_macpb140

ROM_START(macpb160)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

#define rom_macpb165 rom_macpb160
#define rom_macpb180 rom_macpb160

ROM_START(macpb180c)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION32_BE(0x8000, "vrom", 0)
	ROM_LOAD("pb180cvrom.bin", 0x0000, 0x8000, CRC(810c75ad) SHA1(3a936e97dee5ceeb25e50197ef504e514ae689a4))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

#define rom_macpb165c rom_macpb180c

} // anonymous namespace


COMP(1991, macpb140, 0, 0, macpb140, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 140", MACHINE_SUPPORTS_SAVE)
COMP(1991, macpb170, macpb140, 0, macpb170, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 170", MACHINE_SUPPORTS_SAVE)
COMP(1992, macpb145, macpb140, 0, macpb145, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 145", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpb145b, macpb140, 0, macpb145b, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 145B", MACHINE_SUPPORTS_SAVE)
COMP(1992, macpb160, 0, 0, macpb160, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 160", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpb165, macpb160, 0, macpb165, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 165", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpb165c, macpb180c, 0, macpb165c, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 165c", MACHINE_SUPPORTS_SAVE)
COMP(1992, macpb180, macpb160, 0, macpb180, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 180", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpb180c, 0, 0, macpb180c, macadb, macpb030_state, empty_init, "Apple Computer", "Macintosh PowerBook 180c", MACHINE_SUPPORTS_SAVE)
