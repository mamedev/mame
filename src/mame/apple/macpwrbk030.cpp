// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macpwrbk030.cpp
    Mac PowerBooks with a 68030 CPU and M50753 PMU
    By R. Belmont

    These are basically late-period Mac IIs without NuBus and with
    Egret/Cuda replaced with the PMU.

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

    VIA 2 connections: (VIA2 is inside the "Peripheral Glue" ASIC)
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
             1: A/D BATT
             2: SND CNTL
             3: MODEM BUSY
             4: PWR ON
             5: TEMP A/D
             6: NICAD SLA
             7: TABLE SEL

    Port 0: 0: SWIM CNTL
            1: SCC CNTL
            2: HD PWR
            3: MODEM PWR
            4: N/C
            5: SOUND PWR
            6: MODEM PWROUT
            7: SYS_PWR

    Port 1: 0: CCFL PWR CNTL
            1: AKD
            2: STOP CLK
            3: CHRG ON
            4: KBD RST (resets keyboard M50740)
            5: HICHG
            6: RING DETECT
            7: CHG OFF

    Port 2: bi-directional data bus, connected to VIA port A

    Port 3: 0: PWR OFF
            1: SYS RST
            2: VIA TEST
            3: SOUND OFF
            4: 1 SEC
            5: PMINT
            6: PMACK
            7: PMREQ

    Port 4: 0: PMGR_ADB (ADB out)
            1: ADB (ADB in)
            2: DISP BLANK
            3: MODEM_INS

    INT1: 60 Hz clock
    INT2: INT2 PULLUP (pulled up and otherwise N/C)

    PG&E (68HC05 PMU) version spotting:
    (find the text "BORG" in the system ROM, the next 32768 bytes are the PG&E image.
     offset +4 in the image is the version byte).
    01 - PowerBook Duo 210/230/250
    02 - PowerBook 540c, PBDuo 270C, PBDuo 280/280C
    03 - PowerBook 150
    08 - PB190cs, PowerBook 540c PPC update, all PowerPC PowerBooks through WallStreet G3s

****************************************************************************/

#include "emu.h"

#include "macadb.h"
#include "macrtc.h"
#include "macscsi.h"
#include "mactoolbox.h"

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

#define C32M (31.3344_MHz_XTAL)
#define C15M (C32M/2)
#define C7M (C32M/4)

class macpb030_state : public driver_device
{
public:
	macpb030_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pmu(*this, "pmu"),
		m_via1(*this, "via1"),
		m_via2(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_rtc(*this, "rtc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_asc(*this, "asc"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram"),
		m_vga(*this, "vga")
	{
	}

	void macpb140(machine_config &config);
	void macpb145(machine_config &config);
	void macpb160(machine_config &config);
	void macpb170(machine_config &config);
	void macpb180(machine_config &config);
	void macpb180c(machine_config &config);
	void macpd210(machine_config &config);
	void macpb140_map(address_map &map) ATTR_COLD;
	void macpb160_map(address_map &map) ATTR_COLD;
	void macpb165c_map(address_map &map) ATTR_COLD;
	void macpd210_map(address_map &map) ATTR_COLD;

	void init_macpb140();
	void init_macpb160();

private:
	required_device<m68030_device> m_maincpu;
	required_device<m50753_device> m_pmu;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<macadb_device> m_macadb;
	required_device<ncr53c80_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rtc3430042_device> m_rtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<asc_device> m_asc;
	required_device<z80scc_device> m_scc;
	optional_shared_ptr<u32> m_vram;
	optional_device<wd90c26_vga_device> m_vga;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u32 *m_ram_ptr = nullptr, *m_rom_ptr = nullptr;
	u32 m_ram_mask = 0, m_ram_size = 0, m_rom_size = 0;

	emu_timer *m_6015_timer = nullptr;

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, u16 data, u16 mem_mask);
	u8 mac_via_in_a();
	u8 mac_via_in_b();
	void mac_via_out_a(u8 data);
	void mac_via_out_b(u8 data);
	u8 mac_via2_in_a();
	u8 mac_via2_in_b();
	void mac_via2_out_a(u8 data);
	void mac_via2_out_b(u8 data);
	void field_interrupts();
	void mac_via_sync();
	void via_irq_w(int state);
	void via2_irq_w(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	int m_via_interrupt = 0, m_via2_interrupt = 0, m_scc_interrupt = 0, m_asc_interrupt = 0, m_last_taken_interrupt = 0;
	int m_ca1_data = 0, m_via2_ca1_hack = 0;

	u32 rom_switch_r(offs_t offset);
	bool m_overlay = false;

	u16 scsi_r(offs_t offset, u16 mem_mask);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask);
	uint32_t scsi_drq_r(offs_t offset, uint32_t mem_mask = ~0);
	void scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void scsi_berr_w(uint8_t data);
	u16 mac_scc_r(offs_t offset)
	{
		mac_via_sync();
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, u16 data) { m_scc->dc_ab_w(offset, data >> 8); }

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel = 0;

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);

	u16 swim_r(offs_t offset, u16 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-5);
		}

		u16 result = m_swim->read((offset >> 8) & 0xf);
		return result << 8;
	}
	void swim_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_swim->write((offset >> 8) & 0xf, data & 0xff);
		else
			m_swim->write((offset >> 8) & 0xf, data >> 8);
	}

	u32 buserror_r();

	void asc_irq_w(int state)
	{
		m_asc_interrupt = state;
		field_interrupts();
	}

	// ID for PowerBook Duo 210
	u32 pd210_id_r() { return 0xa55a1004; }

	uint8_t mac_gsc_r(offs_t offset);
	void mac_gsc_w(uint8_t data);
	void macgsc_palette(palette_device &palette) const;

	u8 m_pmu_from_via = 0, m_pmu_to_via = 0, m_pmu_ack = 0, m_pmu_req = 0;

	u8 pmu_p1_r() { return 0; }
	u8 pmu_data_r() { return m_pmu_from_via; }
	void pmu_data_w(u8 data)
	{
		m_pmu_to_via = data;
	}
	u8 pmu_comms_r() { return (m_pmu_req<<7); }
	void pmu_comms_w(u8 data)
	{
		m_via1->write_cb1(BIT(data, 5) ^ 1);
		if (m_pmu_ack != BIT(data, 6))
		{
			m_pmu_ack = BIT(data, 6);
			machine().scheduler().synchronize();
		}
	}
	int m_adb_line = 0;
	void set_adb_line(int state) { m_adb_line = state; }
	u8 pmu_adb_r() { return (m_adb_line<<1); }
	void pmu_adb_w(u8 data)
	{
		m_adb_line = (data & 1) ^ 1;
		m_macadb->adb_linechange_w((data & 1) ^ 1);
	}

	u8 pmu_in_r() { return 0x20; }  // bit 5 is 0 if the Target Disk Mode should be enabled

	u8 battery_r() { return 0x7f; }
};

// 4-level grayscale
void macpb030_state::macgsc_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xff, 0xff, 0xff);
	palette.set_pen_color(1, 0x7f, 0x7f, 0x7f);
	palette.set_pen_color(2, 0x3f, 0x3f, 0x3f);
	palette.set_pen_color(3, 0x00, 0x00, 0x00);
}

u32 macpb030_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
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

void macpb030_state::mac_via_sync()
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

void macpb030_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macpb030_state::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_swim->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

void macpb030_state::machine_start()
{
	m_ram_ptr = (u32*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u32*)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = m_asc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;

	m_6015_timer = timer_alloc(FUNC(macpb030_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);
}

void macpb030_state::machine_reset()
{
	m_overlay = true;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = m_asc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;
	m_via2_ca1_hack = 0;

	m_cur_floppy = nullptr;
	m_hdsel = 0;

	// put ROM mirror at 0
	address_space& space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

void macpb030_state::init_macpb140()
{
}

void macpb030_state::init_macpb160()
{
}

u32 macpb030_state::screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 const *const video_ram = (const uint16_t *)m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u16 *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			uint16_t const word = video_ram[((y * 640) / 16) + ((x / 16) ^ 1)];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

u32 macpb030_state::screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const vram8 = (uint8_t *)m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u16 *line = &bitmap.pix(y);

		for (int x = 0; x < 640/4; x++)
		{
			static const u16 palette[4] = { 0, 1, 2, 3 };
			u8 const pixels = vram8[(y * 640/4) + (BYTE4_XOR_BE(x))];
			*line++ = palette[((pixels >> 6) & 3)];
			*line++ = palette[((pixels >> 4) & 3)];
			*line++ = palette[((pixels >> 2) & 3)];
			*line++ = palette[(pixels & 3)];
		}
	}
	return 0;
}

u16 macpb030_state::mac_via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via1->read(offset);

	if (!machine().side_effects_disabled())
		mac_via_sync();

	return (data & 0xff) | (data << 8);
}

void macpb030_state::mac_via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);

	mac_via_sync();
}

uint16_t macpb030_state::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	data = m_via2->read(offset);
	return (data & 0xff) | (data << 8);
}

void macpb030_state::mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	mac_via_sync();

	//printf("%02x to VIA2 @ %x\n", data & 0xff, offset);

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

void macpb030_state::via_irq_w(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void macpb030_state::via2_irq_w(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

u32 macpb030_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	//printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

TIMER_CALLBACK_MEMBER(macpb030_state::mac_6015_tick)
{
	/* signal VBlank on CA1 input on the VIA */
	m_ca1_data ^= 1;
	m_via1->write_ca1(m_ca1_data);

	m_pmu->set_input_line(m50753_device::M50753_INT1_LINE, ASSERT_LINE);
}

u16 macpb030_state::scsi_r(offs_t offset, u16 mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//printf("scsi_r: offset %x mask %x\n", offset, mem_mask);

	bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void macpb030_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//printf("scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data>>8);
}

uint32_t macpb030_state::scsi_drq_r(offs_t offset, uint32_t mem_mask)
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

void macpb030_state::scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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

void macpb030_state::scsi_berr_w(uint8_t data)
{
	m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
}

uint8_t macpb030_state::mac_gsc_r(offs_t offset)
{
	if (offset == 1)
	{
		return 5;
	}

	return 0;
}

void macpb030_state::mac_gsc_w(uint8_t data)
{
}

/***************************************************************************
    ADDRESS MAPS
****************************************************************************/

// ROM detects the "Jaws" ASIC by checking for I/O space mirrored at 0x01000000 boundries
void macpb030_state::macpb140_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w)).mirror(0x01f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w)).mirror(0x01f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w)).mirror(0x01f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w)).mirror(0x01f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w)).mirror(0x01f00000);
	map(0x50012060, 0x50012063).r(FUNC(macpb030_state::scsi_drq_r)).mirror(0x01f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x01f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w)).mirror(0x01f00000);
	map(0x50024000, 0x50027fff).r(FUNC(macpb030_state::buserror_r)).mirror(0x01f00000); // bus error here to make sure we aren't mistaken for another decoder

	map(0xfee08000, 0xfeffffff).ram().share("vram");
}

void macpb030_state::macpb160_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(macpb030_state::mac_gsc_r), FUNC(macpb030_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

void macpb030_state::macpb165c_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w));
	map(0x50f20000, 0x50f21fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to detect we're not the grayscale 160/165/180
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	// on-board color video on 165c/180c
	map(0xfc000000, 0xfc07ffff).rw(m_vga, FUNC(wd90c26_vga_device::mem_linear_r), FUNC(wd90c26_vga_device::mem_linear_w)).mirror(0x00380000); // 512k of VRAM
//  map(0xfc400000, 0xfc7fffff).rw(FUNC(macpb030_state::macwd_r), FUNC(macpb030_state::macwd_w));
	map(0xfc4003b0, 0xfc4003df).m(m_vga, FUNC(wd90c26_vga_device::io_map));
	// something else video related? is at fc800000
	map(0xfcff8000, 0xfcffffff).rom().region("vrom", 0x0000);
}

void macpb030_state::macpd210_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::swim_r), FUNC(macpb030_state::swim_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(macpb030_state::mac_gsc_r), FUNC(macpb030_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	map(0x5ffffffc, 0x5fffffff).r(FUNC(macpb030_state::pd210_id_r));

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

u8 macpb030_state::mac_via_in_a()
{
	return 0x81 | 0x12; // ID for 140/160
}

u8 macpb030_state::mac_via_in_b()
{
	return 0x08 | m_rtc->data_r();    // flag indicating no Target Disk Mode
}

void macpb030_state::mac_via_out_a(u8 data)
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

void macpb030_state::mac_via_out_b(u8 data)
{
	m_rtc->ce_w(BIT(data, 2));
	m_rtc->data_w(BIT(data, 0));
	m_rtc->clk_w(BIT(data, 1));
}

u8 macpb030_state::mac_via2_in_a()
{
	return m_pmu_to_via;
}

u8 macpb030_state::mac_via2_in_b()
{
	return ((m_pmu_ack & 1) << 1);
}

void macpb030_state::mac_via2_out_a(u8 data)
{
	m_pmu_from_via = data;
}

void macpb030_state::mac_via2_out_b(u8 data)
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
	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb140_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	M50753(config, m_pmu, 3.93216_MHz_XTAL);
	m_pmu->read_p<1>().set(FUNC(macpb030_state::pmu_p1_r));
	m_pmu->read_p<2>().set(FUNC(macpb030_state::pmu_data_r));
	m_pmu->write_p<2>().set(FUNC(macpb030_state::pmu_data_w));
	m_pmu->read_p<3>().set(FUNC(macpb030_state::pmu_comms_r));
	m_pmu->write_p<3>().set(FUNC(macpb030_state::pmu_comms_w));
	m_pmu->read_p<4>().set(FUNC(macpb030_state::pmu_adb_r));
	m_pmu->write_p<4>().set(FUNC(macpb030_state::pmu_adb_w));
	m_pmu->read_in_p().set(FUNC(macpb030_state::pmu_in_r));
	m_pmu->ad_in<1>().set(FUNC(macpb030_state::battery_r));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_macpb140));

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(FUNC(macpb030_state::set_adb_line));

	RTC3430042(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_swim, C15M);
	m_swim->phases_cb().set(FUNC(macpb030_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macpb030_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macpb030_state::scsi_berr_w));

	SCC85C30(config, m_scc, C7M);
//  m_scc->intrq_callback().set(FUNC(macpb030_state::set_scc_interrupt));

	R65C22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(macpb030_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macpb030_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macpb030_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macpb030_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macpb030_state::via_irq_w));

	R65NC22(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(macpb030_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macpb030_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macpb030_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macpb030_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macpb030_state::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, 22.5792_MHz_XTAL, asc_device::asc_type::EASC);
	m_asc->irqf_callback().set(FUNC(macpb030_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
}

// PowerBook 145 = 140 @ 25 MHz (still 2MB RAM - the 145B upped that to 4MB)
void macpb030_state::macpb145(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

// PowerBook 170 = 140 @ 25 MHz with an active-matrix LCD (140/145/145B were passive)
void macpb030_state::macpb170(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

void macpb030_state::macpb160(machine_config &config)
{
	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb160_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	M50753(config, m_pmu, 3.93216_MHz_XTAL);
	m_pmu->read_p<2>().set(FUNC(macpb030_state::pmu_data_r));
	m_pmu->write_p<2>().set(FUNC(macpb030_state::pmu_data_w));
	m_pmu->read_p<3>().set(FUNC(macpb030_state::pmu_comms_r));
	m_pmu->write_p<3>().set(FUNC(macpb030_state::pmu_comms_w));
	m_pmu->read_p<4>().set(FUNC(macpb030_state::pmu_adb_r));
	m_pmu->write_p<4>().set(FUNC(macpb030_state::pmu_adb_w));
	m_pmu->read_in_p().set(FUNC(macpb030_state::pmu_in_r));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_macpb160));

	PALETTE(config, m_palette, FUNC(macpb030_state::macgsc_palette), 16);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(FUNC(macpb030_state::set_adb_line));

	RTC3430042(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_swim, C15M);
	m_swim->phases_cb().set(FUNC(macpb030_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macpb030_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macpb030_state::scsi_berr_w));

	SCC85C30(config, m_scc, C7M);
	//  m_scc->intrq_callback().set(FUNC(macpb030_state::set_scc_interrupt));

	R65C22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(macpb030_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macpb030_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macpb030_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macpb030_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macpb030_state::via_irq_w));

	R65NC22(config, m_via2, C7M / 10);
	m_via2->readpa_handler().set(FUNC(macpb030_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macpb030_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macpb030_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macpb030_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macpb030_state::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, 22.5792_MHz_XTAL, asc_device::asc_type::EASC);
	m_asc->irqf_callback().set(FUNC(macpb030_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
}

void macpb030_state::macpb180(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
}

void macpb030_state::macpb180c(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb165c_map);

	m_screen->set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	m_screen->set_screen_update("vga", FUNC(wd90c26_vga_device::screen_update));
	m_screen->set_no_palette();

	WD90C26(config, m_vga, 0);
	m_vga->set_screen(m_screen);
	// 512KB
	m_vga->set_vram_size(0x80000);
}

void macpb030_state::macpd210(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpd210_map);

	M50753(config.replace(), m_pmu, 3.93216_MHz_XTAL).set_disable();

	m_ram->set_extra_options("8M,12M,16M,20M,24M");
}

ROM_START(macpb140)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb145)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb145b)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb170)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb160)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpb180)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpb180c)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION32_BE(0x8000, "vrom", 0)
	ROM_LOAD("pb180cvrom.bin", 0x0000, 0x8000, CRC(810c75ad) SHA1(3a936e97dee5ceeb25e50197ef504e514ae689a4))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpd210)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("ecfa989b.rom", 0x000000, 0x100000, CRC(b86ed854) SHA1(ed1371c97117a5884da4a6605ecfc5abed48ae5a))

	ROM_REGION(0x1800, "pmu", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


COMP(1991, macpb140, 0, 0, macpb140, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 140", MACHINE_NOT_WORKING)
COMP(1991, macpb170, macpb140, 0, macpb170, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 170", MACHINE_NOT_WORKING)
COMP(1992, macpb145, macpb140, 0, macpb145, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 145", MACHINE_NOT_WORKING)
COMP(1992, macpb145b, macpb140, 0, macpb170, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 145B", MACHINE_NOT_WORKING)
COMP(1992, macpb160, 0, 0, macpb160, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 160", MACHINE_NOT_WORKING)
COMP(1992, macpb180, macpb160, 0, macpb180, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 180", MACHINE_NOT_WORKING)
COMP(1992, macpb180c, macpb160, 0, macpb180c, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 180c", MACHINE_NOT_WORKING)

// PowerBook Duos (may or may not belong in this driver ultimately)
COMP(1992, macpd210, 0, 0, macpd210, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook Duo 210", MACHINE_NOT_WORKING)
