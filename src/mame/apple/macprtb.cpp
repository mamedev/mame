// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macprtb.cpp
    Mac Portable / PowerBook 100 emulation
    By R. Belmont

    These are electrically identical 68000 machines in very different form factors.
    The Mac Portable came in a large, heavy, "luggable" form factor and didn't enjoy
    much success.

    The PowerBook 100 was much smaller and lighter, the result of
    Apple partnering with Sony to redesign the Portable motherboard to fit into
    a much smaller case.  The PowerBook 100 pioneered the modern convention of
    having the pointing device nearest the user with the keyboard pushed back
    towards the hinge.

    These are sort of an intermediate step between the SE and Mac II in terms
    of functional layout: ASC and SWIM are present, but there's only 1 VIA
    (CMDμ G65SC22PE-2, not the "6523" variant normally used in ADB Macs) and an
    M50753 microcontroller "PMU" handles power management, ADB, and clock/PRAM.

    VIA connections:
    Port A: 8-bit bidirectional data bus to the PMU
    Port B: 0: PMU REQ
            1: PMU ACK
            2: VIA_TEST
            3: MODEM
            4: N/C
            5: HDSEL (floppy head select)
            6: STEREO
            7: SCC REQ

    CA1: 60 Hz clock
    CA2: 1 second clock from PMU
    CB1: PMU IRQ
    CB2: SCSI IRQ

    PMU (M50753) connections:
    IN port: 0: PMGR_IN0
             1: A/D FILTER
             2: SOUND LATCH
             3: OFF HOOK
             4: ?
             5: Target Disk Mode flag (0 = TDM, 1 for normal boot)
             6: ?
             7: ?

    Port 0: 0: IWM_CNTRL
            1: N/C
            2: HD PWR/
            3: MODEM PWR/
            4: SERIAL PWR/
            5: SOUND PWR/
            6: -5 EN
            7: SYS_PWR/

    Port 1: 0: N/C
            1: Any Key Down
            2: STOP CLK
            3: CHRG ON/
            4: KBD RST/ (resets keyboard M50740)
            5: HICHG
            6: RING DETECT
            7: N/C

    Port 2: bi-directional data bus, connected to VIA port A

    Port 3: 0: RESET/
            1: SYS_RST/
            2: VIA_TEST
            3: SOUND OFF
            4: 1 SEC/
            5: PMINT/
            6: PMACK/
            7: PMREQ/

    Port 4: 0: PMGR_ADB (ADB out)
            1: ADB (ADB in)
            2: DISP BLANK/
            3: MODEM_INS/

    INT1: 60.15 Hz clock
    INT2: RESET

****************************************************************************/

#include "emu.h"

#include "macadb.h"
#include "macscsi.h"
#include "mactoolbox.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m5074x.h"
#include "formats/ap_dsk35.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/applefdintf.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "sound/asc.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {
class macportable_state : public driver_device, public device_nvram_interface
{
public:
	macportable_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		device_nvram_interface(mconfig, *this),
		m_maincpu(*this, "maincpu"),
		m_pmu(*this, "pmu"),
		m_via1(*this, "via1"),
		m_macadb(*this, "macadb"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_screen(*this, "screen"),
		m_asc(*this, "asc"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram"),
		m_cur_floppy(nullptr),
		m_hdsel(0),
		m_ram_ptr(nullptr),
		m_rom_ptr(nullptr),
		m_ram_mask(0),
		m_ram_size(0),
		m_rom_size(0),
		m_6015_timer(nullptr),
		m_via_cycles(0),
		m_via_interrupt(0),
		m_scc_interrupt(0),
		m_asc_interrupt(0),
		m_last_taken_interrupt(-1),
		m_ca1_data(0),
		m_overlay(false),
		m_pmu_to_via(0),
		m_pmu_from_via(0),
		m_pmu_ack(0),
		m_pmu_req(0),
		m_pmu_p0(0x80),
		m_adb_line(1)
	{
	}

	void macprtb(machine_config &config);
	void macprtb_map(address_map &map) ATTR_COLD;

	void init_macprtb();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 via_r(offs_t offset);
	void via_w(offs_t offset, u16 data, u16 mem_mask);
	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	void field_interrupts();
	void via_irq_w(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	void phases_w(u8 phases);
	void devsel_w(u8 devsel);

	u16 rom_switch_r(offs_t offset);

	u16 scsi_r(offs_t offset, u16 mem_mask);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask);
	void scsi_berr_w(u8 data);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	u16 iwm_r(offs_t offset, u16 mem_mask);
	void iwm_w(offs_t offset, u16 data, u16 mem_mask);
	u16 autovector_r(offs_t offset);
	void autovector_w(offs_t offset, u16 data);
	void pmu_p0_w(u8 data);
	u8 pmu_p1_r();
	u8 pmu_data_r();
	void pmu_data_w(u8 data);
	u8 pmu_comms_r();
	void pmu_comms_w(u8 data);
	void set_adb_line(int state);
	u8 pmu_adb_r();
	void pmu_adb_w(u8 data);
	u8 pmu_in_r();
	u8 ad_in_r();
	void asc_irq_w(int state);
	void scc_irq_w(int state);
	u16 config_r();

	required_device<m68000_device> m_maincpu;
	required_device<m50753_device> m_pmu;
	required_device<via6522_device> m_via1;
	required_device<macadb_device> m_macadb;
	required_device<ncr53c80_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<screen_device> m_screen;
	required_device<asc_device> m_asc;
	required_device<z80scc_device> m_scc;
	required_shared_ptr<u16> m_vram;

	floppy_image_device *m_cur_floppy;
	int m_hdsel;

	u16 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size, m_rom_size;

	emu_timer *m_6015_timer;

	s32 m_via_cycles, m_via_interrupt, m_scc_interrupt, m_asc_interrupt, m_last_taken_interrupt;
	s32 m_ca1_data;

	bool m_overlay;

	u8 m_pmu_to_via, m_pmu_from_via, m_pmu_ack, m_pmu_req, m_pmu_p0;
	s32 m_adb_line;
};

void macportable_state::nvram_default()
{
}

bool macportable_state::nvram_read(util::read_stream &file)
{
	// Unlike Egret and Cuda, this PMU doesn't assume RAM contents
	// are bad on boot.  It checksums the PRAM and RTC and if the checksum
	// passes it doesn't re-initialize.
	u8 nvram[0xc0];
	auto const [err, actual] = read(file, nvram, 0xc0);
	if (!err && (actual == 0xc0))
	{
		for (int i = 0; i < 0xc0; i++)
		{
			m_pmu->space(AS_PROGRAM).write_byte(i, nvram[i]);
		}
		return true;
	}
	return false;
}

bool macportable_state::nvram_write(util::write_stream &file)
{
	u8 nvram[0xc0];
	for (int i = 0; i < 0xc0; i++)
	{
		nvram[i] = m_pmu->space(AS_PROGRAM).read_byte(i);
	}

	auto const [err, actual] = write(file, nvram, 0xc0);
	return !err;
}

u16 macportable_state::scc_r(offs_t offset)
{
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void macportable_state::scc_w(offs_t offset, u16 data)
{
	m_scc->dc_ab_w(offset, data >> 8);
}

u16 macportable_state::iwm_r(offs_t offset, u16 mem_mask)
{
	u16 result = m_swim->read((offset >> 8) & 0xf);
	return (result << 8) | result;
}

void macportable_state::iwm_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_swim->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_swim->write((offset >> 8) & 0xf, data >> 8);
}

u16 macportable_state::autovector_r(offs_t offset)
{
	return 0;
}

void macportable_state::autovector_w(offs_t offset, u16 data)
{
};

// returns nonzero if no PDS RAM expansion, 0 if present
u16 macportable_state::config_r()
{
	if (m_ram->size() < (6*1024*1024))
	{
		return 0xffff;
	}

	return 0;
}

void macportable_state::asc_irq_w(int state)
{
	m_asc_interrupt = state;
	field_interrupts();
}

void macportable_state::scc_irq_w(int state)
{
	m_scc_interrupt = state;
	field_interrupts();
}

void macportable_state::pmu_p0_w(u8 data)
{
	if ((!BIT(data, 7)) && (BIT(m_pmu_p0, 7)))
	{
		system_time systime;
		struct tm cur_time, macref;
		machine().current_datetime(systime);

		cur_time.tm_sec = systime.local_time.second;
		cur_time.tm_min = systime.local_time.minute;
		cur_time.tm_hour = systime.local_time.hour;
		cur_time.tm_mday = systime.local_time.mday;
		cur_time.tm_mon = systime.local_time.month;
		cur_time.tm_year = systime.local_time.year - 1900;
		cur_time.tm_isdst = 0;

		macref.tm_sec = 0;
		macref.tm_min = 0;
		macref.tm_hour = 0;
		macref.tm_mday = 1;
		macref.tm_mon = 0;
		macref.tm_year = 4;
		macref.tm_isdst = 0;
		const u32 ref = (u32)mktime(&macref);

		const u32 seconds = (u32)((u32)mktime(&cur_time) - ref);
		m_pmu->space(AS_PROGRAM).write_byte(0x28, seconds & 0xff);
		m_pmu->space(AS_PROGRAM).write_byte(0x27, (seconds >> 8) & 0xff);
		m_pmu->space(AS_PROGRAM).write_byte(0x26, (seconds >> 16) & 0xff);
		m_pmu->space(AS_PROGRAM).write_byte(0x25, (seconds >> 24) & 0xff);
	}

	m_pmu_p0 = data;
}

u8 macportable_state::pmu_p1_r()
{
	return 0x08;        // indicate on charger power
}

u8 macportable_state::pmu_data_r()
{
	return m_pmu_from_via;
}

void macportable_state::pmu_data_w(u8 data)
{
	m_pmu_to_via = data;
}

u8 macportable_state::pmu_comms_r()
{
	return (m_pmu_req << 7);
}

void macportable_state::pmu_comms_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);

	m_via1->write_ca2(BIT(data, 4)); // 1 second interrupt
	m_via1->write_cb1(BIT(data, 5)); // PMINT/
	m_pmu_ack = BIT(data, 6);
}

void macportable_state::set_adb_line(int state)
{
	m_adb_line = state;
}

u8 macportable_state::pmu_adb_r()
{
	return (m_adb_line << 1);
}

void macportable_state::pmu_adb_w(u8 data)
{
	m_macadb->adb_linechange_w((data & 1) ^ 1);
}

u8 macportable_state::pmu_in_r()
{
	return 0x20;
} // bit 5 is 0 if the Target Disk Mode should be enabled on the PB100

u8 macportable_state::ad_in_r()
{
	return 0xff;
}

void macportable_state::field_interrupts()
{
	int take_interrupt = -1;

	if ((m_scc_interrupt) || (m_asc_interrupt))
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

void macportable_state::machine_start()
{
	m_ram_ptr = (u16*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u16*)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_cycles = -50;

	save_item(NAME(m_via_cycles));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_asc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_ca1_data));
	save_item(NAME(m_overlay));
	save_item(NAME(m_pmu_to_via));

	m_6015_timer = timer_alloc(FUNC(macportable_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);
}

void macportable_state::machine_reset()
{
	m_overlay = true;
	m_via_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;

	m_pmu_ack = 1;

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

void macportable_state::init_macprtb()
{
}

u32 macportable_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 const *const video_ram = (const u16 *) m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u32 *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			u16 const word = video_ram[((y * 640)/16) + ((x/16))];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = ((word >> (15 - b)) & 0x0001) ? 0 : 0xffffffff;
			}
		}
	}
	return 0;
}

u16 macportable_state::via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via1->read(offset);

	m_maincpu->adjust_icount(m_via_cycles);

	return (data & 0xff) | (data << 8);
}

void macportable_state::via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

void macportable_state::via_irq_w(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

u16 macportable_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (!machine().side_effects_disabled())
	{
		if ((m_overlay) && (offset == 0x67f))
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			const u32 memory_end = m_ram->size() - 1;
			void *memory_data = m_ram->pointer();
			offs_t memory_mirror = memory_end & ~memory_end;

			space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
			m_overlay = false;
		}
	}

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

TIMER_CALLBACK_MEMBER(macportable_state::mac_6015_tick)
{
	/* signal VBlank on CA1 input on the VIA */
	m_ca1_data ^= 1;
	m_via1->write_ca1(m_ca1_data);

	m_pmu->set_input_line(m50753_device::M50753_INT1_LINE, ASSERT_LINE);
	m_macadb->adb_vblank();
}

u16 macportable_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset >= 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void macportable_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset >= 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data);
}

void macportable_state::scsi_berr_w(u8 data)
{
	m_maincpu->trigger_bus_error();
}

void macportable_state::macprtb_map(address_map &map)
{
	map(0x000000, 0x1fffff).r(FUNC(macportable_state::rom_switch_r));
	map(0x900000, 0x93ffff).rom().region("bootrom", 0).mirror(0x0c0000);
	map(0xf60000, 0xf6ffff).rw(FUNC(macportable_state::iwm_r), FUNC(macportable_state::iwm_w));
	map(0xf70000, 0xf7ffff).rw(FUNC(macportable_state::via_r), FUNC(macportable_state::via_w));
	map(0xf90000, 0xf9ffff).rw(FUNC(macportable_state::scsi_r), FUNC(macportable_state::scsi_w));
	map(0xfa8000, 0xfaffff).ram().share("vram"); // VRAM
	map(0xfb0000, 0xfbffff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0xfc0000, 0xfcffff).r(FUNC(macportable_state::config_r));
	map(0xfd0000, 0xfdffff).rw(FUNC(macportable_state::scc_r), FUNC(macportable_state::scc_w));
	map(0xfe0000, 0xfe0001).noprw();
	map(0xfffff0, 0xffffff).rw(FUNC(macportable_state::autovector_r), FUNC(macportable_state::autovector_w));
}

u8 macportable_state::via_in_a()
{
	return m_pmu_to_via;
}

u8 macportable_state::via_in_b()
{
	return 0x80 | 0x04 | ((m_pmu_ack & 1)<<1);
}

void macportable_state::via_out_a(u8 data)
{
	m_pmu_from_via = data;
}

void macportable_state::via_out_b(u8 data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
		m_hdsel = hdsel;
	}
	m_pmu_req = (data & 1);
}

void macportable_state::phases_w(u8 phases)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->seek_phase_w(phases);
	}
}

void macportable_state::devsel_w(u8 devsel)
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

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

void macportable_state::macprtb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 15.6672_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &macportable_state::macprtb_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	M50753(config, m_pmu, 3.93216_MHz_XTAL);
	m_pmu->write_p<0>().set(FUNC(macportable_state::pmu_p0_w));
	m_pmu->read_p<1>().set(FUNC(macportable_state::pmu_p1_r));
	m_pmu->read_p<2>().set(FUNC(macportable_state::pmu_data_r));
	m_pmu->write_p<2>().set(FUNC(macportable_state::pmu_data_w));
	m_pmu->set_pullups<2>(0xff); // internal pullup option?
	m_pmu->read_p<3>().set(FUNC(macportable_state::pmu_comms_r));
	m_pmu->write_p<3>().set(FUNC(macportable_state::pmu_comms_w));
	m_pmu->read_p<4>().set(FUNC(macportable_state::pmu_adb_r));
	m_pmu->write_p<4>().set(FUNC(macportable_state::pmu_adb_w));
	m_pmu->set_pullups<4>(0x0f); // external pullups
	m_pmu->read_in_p().set(FUNC(macportable_state::pmu_in_r));
	m_pmu->ad_in<0>().set(FUNC(macportable_state::ad_in_r));
	m_pmu->ad_in<1>().set(FUNC(macportable_state::ad_in_r));

	M50740(config, "kybd", 3.93216_MHz_XTAL).set_disable();

	config.set_perfect_quantum(m_maincpu);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_screen_update(FUNC(macportable_state::screen_update));

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);
	m_macadb->adb_data_callback().set(FUNC(macportable_state::set_adb_line));

	SWIM1(config, m_swim, 15.6672_MHz_XTAL);
	m_swim->phases_cb().set(FUNC(macportable_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macportable_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
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
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(m_via1, FUNC(r65c22_device::write_cb2));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
		adapter.drq_handler().append(m_via1, FUNC(r65c22_device::write_ca1));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macportable_state::scsi_berr_w));

	SCC85C30(config, m_scc, 15.6672_MHz_XTAL /2 );
	m_scc->out_int_callback().set(FUNC(macportable_state::scc_irq_w));

	R65C22(config, m_via1, 15.6672_MHz_XTAL / 20);
	m_via1->readpa_handler().set(FUNC(macportable_state::via_in_a));
	m_via1->readpb_handler().set(FUNC(macportable_state::via_in_b));
	m_via1->writepa_handler().set(FUNC(macportable_state::via_out_a));
	m_via1->writepb_handler().set(FUNC(macportable_state::via_out_b));
	m_via1->irq_handler().set(FUNC(macportable_state::via_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, 15.6672_MHz_XTAL, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(FUNC(macportable_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	RAM(config, m_ram);
	m_ram->set_default_size("1M");
	m_ram->set_extra_options("2M,4M,5M,6M,7M,8M,9M");

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
}

ROM_START(macprtb)
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD("93ca3846.rom", 0x000000, 0x040000, CRC(497348f8) SHA1(79b468b33fc53f11e87e2e4b195aac981bf0c0a6))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv1.bin", 0x000000, 0x001800, CRC(01dae148) SHA1(29d2fca7426c31f2b9334832ed3d257974a61bb1))

	ROM_REGION(0xc00, "kybd", 0)
	ROM_LOAD("342s0740-2.12l", 0x000, 0xc00, NO_DUMP)
ROM_END

ROM_START(macpb100)
	ROM_REGION16_BE(0x40000, "bootrom", 0)
	ROM_LOAD16_WORD("96645f9c.rom", 0x000000, 0x040000, CRC(29ac7ee9) SHA1(7f3acf40b1f63612de2314a2e9fcfeafca0711fc))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv1.bin", 0x000000, 0x001800, CRC(01dae148) SHA1(29d2fca7426c31f2b9334832ed3d257974a61bb1))

	ROM_REGION(0xc00, "kybd", 0)
	ROM_LOAD("342s0743-1.u29", 0x000, 0xc00, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(1989, macprtb,  0, 0, macprtb, macadb, macportable_state, init_macprtb, "Apple Computer", "Macintosh Portable", MACHINE_SUPPORTS_SAVE )
COMP(1991, macpb100, 0, 0, macprtb, macadb, macportable_state, init_macprtb, "Apple Computer", "Macintosh PowerBook 100", MACHINE_SUPPORTS_SAVE )
