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
    CA2: 1 second clock
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
            2: HD PWR
            3: MODEM PWR
            4: SERIAL PWR
            5: SOUND PWR
            6: -5 EN
            7: SYS_PWR

    Port 1: 0: N/C
            1: AKD
            2: STOP CLK
            3: CHRG ON
            4: KBD RST (resets keyboard M50740)
            5: HICHG
            6: RING DETECT
            7: N/C

    Port 2: bi-directional data bus, connected to VIA port A

    Port 3: 0: RESET
            1: SYS_RST
            2: VIA_TEST
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
    INT2: RESET

****************************************************************************/

#include "emu.h"

#include "machine/macrtc.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m5074x.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/applefdintf.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/macadb.h"
#include "machine/macscsi.h"
#include "machine/ncr5380n.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"
#include "sound/asc.h"
#include "formats/ap_dsk35.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define C32M (31.3344_MHz_XTAL)
#define C15M (C32M/2)
#define C7M (C32M/4)

class macportable_state : public driver_device
{
public:
	macportable_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
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
		m_hdsel(0)
	{
	}

	void macprtb(machine_config &config);
	void macprtb_map(address_map &map);

	void init_macprtb();

private:
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
	required_shared_ptr<uint16_t> m_vram;

	floppy_image_device *m_cur_floppy;
	int m_hdsel;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size, m_rom_size;

	emu_timer *m_6015_timer;

	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t mac_via_in_a();
	uint8_t mac_via_in_b();
	void mac_via_out_a(uint8_t data);
	void mac_via_out_b(uint8_t data);
	void field_interrupts();
	DECLARE_WRITE_LINE_MEMBER(via_irq_w);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	int m_via_cycles, m_via_interrupt, m_scc_interrupt, m_asc_interrupt, m_last_taken_interrupt;
	int m_ca1_data;

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);

	uint16_t rom_switch_r(offs_t offset);
	bool m_overlay;

	uint16_t scsi_r(offs_t offset, uint16_t mem_mask);
	void scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void scsi_berr_w(uint8_t data);

	uint16_t mac_scc_r(offs_t offset)
	{
		uint16_t result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, uint16_t data) { m_scc->dc_ab_w(offset, data >> 8); }

	uint16_t mac_iwm_r(offs_t offset, uint16_t mem_mask)
	{
		uint16_t result = m_swim->read((offset >> 8) & 0xf);
		return (result << 8) | result;
	}
	void mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_swim->write((offset >> 8) & 0xf, data & 0xff);
		else
			m_swim->write((offset >> 8) & 0xf, data>>8);
	}

	uint16_t mac_autovector_r(offs_t offset) { return 0; }
	void mac_autovector_w(offs_t offset, uint16_t data) {};

	// returns nonzero if no PDS RAM expansion, 0 if present
	uint16_t mac_config_r() { return 0xffff; }

	DECLARE_WRITE_LINE_MEMBER(asc_irq_w)
	{
		m_asc_interrupt = state;
		field_interrupts();
	}

	u8 m_pmu_to_via, m_pmu_from_via, m_pmu_ack, m_pmu_req;
	u8 pmu_data_r() { return m_pmu_from_via; }
	void pmu_data_w(u8 data) { m_pmu_to_via = data; }
	u8 pmu_comms_r() { return (m_pmu_req<<7); }
	void pmu_comms_w(u8 data)
	{
		if (BIT(data, 1))   // start the 68K if it's not already
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		}

		m_via1->write_ca2(BIT(data, 4));
		m_via1->write_cb1(BIT(data, 5));

			m_pmu_ack = BIT(data, 6);
		//printf("PMU ACK = %d\n", m_pmu_ack);
	}
	int m_adb_line;
	void set_adb_line(int state) { m_adb_line = state; }
	u8 pmu_adb_r() { return (m_adb_line<<1); }
	void pmu_adb_w(u8 data) { m_macadb->adb_linechange_w((data & 1) ^ 1); }
	u8 pmu_in_r() { return 0x20; }  // bit 5 is 0 if the Target Disk Mode should be enabled on the PB100
	u8 ad_in_r() { return 0xff; }
};

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
	m_via_interrupt = m_scc_interrupt = m_asc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_ca1_data = 0;
	m_pmu_to_via = 0;
	m_pmu_from_via = 0;
	m_pmu_ack = m_pmu_req = 0;
	m_adb_line = 1;

	m_6015_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(macportable_state::mac_6015_tick),this));
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

	// as with Egret/Cuda, the PMU starts the 68K when it's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void macportable_state::init_macprtb()
{
}

uint32_t macportable_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const video_ram = (const uint16_t *) m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		uint32_t *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			uint16_t const word = video_ram[((y * 640)/16) + ((x/16))];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = ((word >> (15 - b)) & 0x0001) ? 0 : 0xffffffff;
			}
		}
	}
	return 0;
}

uint16_t macportable_state::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via1->read(offset);

	m_maincpu->adjust_icount(m_via_cycles);

	return (data & 0xff) | (data << 8);
}

void macportable_state::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

WRITE_LINE_MEMBER(macportable_state::via_irq_w)
{
	m_via_interrupt = state;
	field_interrupts();
}

uint16_t macportable_state::rom_switch_r(offs_t offset)
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

uint16_t macportable_state::scsi_r(offs_t offset, uint16_t mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void macportable_state::scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data);
}

void macportable_state::scsi_berr_w(uint8_t data)
{
	m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void macportable_state::macprtb_map(address_map &map)
{
	map(0x000000, 0x1fffff).r(FUNC(macportable_state::rom_switch_r));
	map(0x900000, 0x93ffff).rom().region("bootrom", 0).mirror(0x0c0000);
	map(0xf60000, 0xf6ffff).rw(FUNC(macportable_state::mac_iwm_r), FUNC(macportable_state::mac_iwm_w));
	map(0xf70000, 0xf7ffff).rw(FUNC(macportable_state::mac_via_r), FUNC(macportable_state::mac_via_w));
	map(0xf90000, 0xf9ffff).rw(FUNC(macportable_state::scsi_r), FUNC(macportable_state::scsi_w));
	map(0xfa8000, 0xfaffff).ram().share("vram"); // VRAM
	map(0xfb0000, 0xfbffff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0xfc0000, 0xfcffff).r(FUNC(macportable_state::mac_config_r));
	map(0xfd0000, 0xfdffff).rw(FUNC(macportable_state::mac_scc_r), FUNC(macportable_state::mac_scc_2_w));
	map(0xfe0000, 0xfe0001).noprw();
	map(0xfffff0, 0xffffff).rw(FUNC(macportable_state::mac_autovector_r), FUNC(macportable_state::mac_autovector_w));
}

uint8_t macportable_state::mac_via_in_a()
{
	return m_pmu_to_via;
}

uint8_t macportable_state::mac_via_in_b()
{
	return 0x80 | 0x04 | ((m_pmu_ack & 1)<<1);
}

void macportable_state::mac_via_out_a(uint8_t data)
{
	m_pmu_from_via = data;
}

void macportable_state::mac_via_out_b(uint8_t data)
{
	//printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);
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

void macportable_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->seek_phase_w(phases);
	}
}

void macportable_state::devsel_w(uint8_t devsel)
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

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macportable_state::macprtb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &macportable_state::macprtb_map);

	M50753(config, m_pmu, 3.93216_MHz_XTAL);
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

	MACADB(config, m_macadb, C15M);
	m_macadb->set_mcu_mode(true);
	m_macadb->adb_data_callback().set(FUNC(macportable_state::set_adb_line));

	SWIM1(config, m_swim, C15M);
	m_swim->phases_cb().set(FUNC(macportable_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macportable_state::devsel_w));

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
		adapter.irq_handler().set(m_via1, FUNC(r65c22_device::write_cb2));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macportable_state::scsi_berr_w));

	SCC85C30(config, m_scc, C7M);
//  m_scc->intrq_callback().set(FUNC(macportable_state::set_scc_interrupt));

	R65C22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(macportable_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macportable_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macportable_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macportable_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macportable_state::via_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(FUNC(macportable_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("1M");
	m_ram->set_extra_options("1M,3M,5M,7M,9M");

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

COMP(1989, macprtb,  0, 0, macprtb, macadb, macportable_state, init_macprtb, "Apple Computer", "Macintosh Portable", MACHINE_NOT_WORKING)
COMP(1991, macpb100, 0, 0, macprtb, macadb, macportable_state, init_macprtb, "Apple Computer", "Macintosh PowerBook 100", MACHINE_NOT_WORKING )
