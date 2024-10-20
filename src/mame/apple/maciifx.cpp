// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    maciifx.cpp
    Mac IIfx

    By R. Belmont

    This was the fastest 68030 Mac, with a 40 MHz clock speed and 2 65C02
    coprocessors plus DMA capability.  MacOS used almost none of the extra
    hardware so the machine never reached its full potential.  However, its
    DMA and I/O coprocessors were reused in the LaserWriter IIf/IIg and
    Quadra 900/950.

****************************************************************************/

#include "emu.h"

#include "macadb.h"
#include "macrtc.h"
#include "mactoolbox.h"
#include "scsidma.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/applepic.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/asc.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "softlist_dev.h"

namespace {

#define C15M    (15.6672_MHz_XTAL)
#define C7M     (C15M/2)

class maciifx_state : public driver_device
{
public:
	maciifx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_rtc(*this, "rtc"),
		m_scsidma(*this, "scsidma"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scc(*this, "scc"),
		m_asc(*this, "asc"),
		m_cur_floppy(nullptr),
		m_hdsel(0),
		m_last_taken_interrupt(-1),
		m_overlay(true),
		m_rom_ptr(nullptr),
		m_rom_size(0),
		m_adb_in(0)
	{
	}

	void maciifx(machine_config &config);
	void maciifx_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68030_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<rtc3430042_device> m_rtc;
	required_device<scsidma_device> m_scsidma;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<z80scc_device> m_scc;
	required_device<asc_device> m_asc;

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;

	uint8_t m_oss_regs[0x400]{};
	int m_last_taken_interrupt;
	emu_timer *m_6015_timer;

	bool m_overlay;
	u32 *m_rom_ptr;
	u32 m_rom_size;

	int m_adb_in;

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	void fdc_hdsel(int state);

	uint32_t biu_r(offs_t offset, uint32_t mem_mask = ~0);
	void biu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int N>
	void oss_interrupt(int state);
	TIMER_CALLBACK_MEMBER(oss_6015_tick);
	uint8_t oss_r(offs_t offset);
	void oss_w(offs_t offset, uint8_t data);
	uint32_t buserror_r();

	uint16_t via_r(offs_t offset);
	void via_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t via_in_a();
	uint8_t via_in_b();
	void via_out_a(uint8_t data);
	void via_out_b(uint8_t data);
	void via_sync();

	uint32_t rom_switch_r(offs_t offset);

	void set_adb_line(int linestate) { m_adb_in = (linestate == ASSERT_LINE) ? true : false; }
	int adbin_r() { return m_adb_in; }
};

void maciifx_state::machine_start()
{
	save_item(NAME(m_hdsel));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_adb_in));

	m_6015_timer = timer_alloc(FUNC(maciifx_state::oss_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	m_rom_ptr = (u32 *)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();

	m_last_taken_interrupt = -1;
	m_adb_in = 0;
}

void maciifx_state::machine_reset()
{
	// put ROM mirror at 0
	address_space& space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
	m_overlay = true;

	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

uint32_t maciifx_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	// printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void maciifx_state::maciifx_map(address_map &map)
{
	map(0x40000000, 0x4007ffff).r(FUNC(maciifx_state::rom_switch_r)).mirror(0x0ff80000);

	map(0x50000000, 0x50001fff).rw(FUNC(maciifx_state::via_r), FUNC(maciifx_state::via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw("sccpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x50004000, 0x50005fff).rw("sccpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);
	map(0x50008000, 0x50009fff).m(m_scsidma, FUNC(scsidma_device::map)).mirror(0x0ff80000);
	map(0x50010000, 0x50011fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw("swimpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x50012000, 0x50013fff).rw("swimpic", FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);
	map(0x50018000, 0x50019fff).rw(FUNC(maciifx_state::biu_r), FUNC(maciifx_state::biu_w)).mirror(0x00f00000);
	map(0x5001a000, 0x5001bfff).rw(FUNC(maciifx_state::oss_r), FUNC(maciifx_state::oss_w)).mirror(0x00f00000);
	map(0x50024000, 0x50027fff).r(FUNC(maciifx_state::buserror_r)).mirror(0x00f00000); // must bus error on access here so ROM can determine we're an FMC
	map(0x50040000, 0x50041fff).rw(FUNC(maciifx_state::via_r), FUNC(maciifx_state::via_w)).mirror(0x00f00000);
}

void maciifx_state::via_sync()
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

uint16_t maciifx_state::via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void maciifx_state::via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

uint8_t maciifx_state::via_in_a()
{
	return 0xd3;    // PA6 | PA4 | PA1
}

uint8_t maciifx_state::via_in_b()
{
	return  m_rtc->data_r();
}

void maciifx_state::via_out_a(uint8_t data)
{
}

void maciifx_state::via_out_b(uint8_t data)
{
	m_rtc->ce_w((data & 0x04) >> 2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

void maciifx_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void maciifx_state::devsel_w(uint8_t devsel)
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

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(m_hdsel);
	}
}

void maciifx_state::fdc_hdsel(int state)
{
	if (state != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(state);
		}
	}
	m_hdsel = state;
}

uint32_t maciifx_state::biu_r(offs_t offset, uint32_t mem_mask)
{
	return 0;
}

void maciifx_state::biu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

template <int N>
void maciifx_state::oss_interrupt(int state)
{
	if (state == ASSERT_LINE)
	{
		m_oss_regs[N >= 8 ? 0x202 : 0x203] |= 1 << (N & 7);
	}
	else
	{
		m_oss_regs[N >= 8 ? 0x202 : 0x203] &= ~(1 << (N & 7));
	}

	int take_interrupt = 0;
	for (int n = 0; n < 8; n++)
	{
		if (BIT(m_oss_regs[0x203], n) && take_interrupt < m_oss_regs[n])
		{
			take_interrupt = m_oss_regs[n];
		}
		if (BIT(m_oss_regs[0x202], n) && take_interrupt < m_oss_regs[8 + n])
		{
			take_interrupt = m_oss_regs[8 + n];
		}
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
		m_oss_regs[0x200] &= 0x7f;
	}

	if (take_interrupt > 0)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
		m_oss_regs[0x200] |= 0x80;
	}
}

TIMER_CALLBACK_MEMBER(maciifx_state::oss_6015_tick)
{
	m_via1->write_ca1(0);
	m_via1->write_ca1(1);
	oss_interrupt<10>(ASSERT_LINE);
}

uint8_t maciifx_state::oss_r(offs_t offset)
{
	if (offset < std::size(m_oss_regs))
	{
		return m_oss_regs[offset];
	}
	else
	{
		return 0;
	}
}

void maciifx_state::oss_w(offs_t offset, uint8_t data)
{
	if (offset == 0x207)
	{
		oss_interrupt<10>(CLEAR_LINE);
	}
	else if (offset < std::size(m_oss_regs))
	{
		m_oss_regs[offset] = data;
	}
}

uint32_t maciifx_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( maciifx )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/
void maciifx_state::maciifx(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 40000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciifx_state::maciifx_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(maciifx_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(maciifx_state::phases_w));
	m_fdc->hdsel_cb().set(FUNC(maciifx_state::fdc_hdsel));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
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

	SCSIDMA(config, m_scsidma, C15M);
	m_scsidma->set_maincpu_tag("maincpu");
	m_scsidma->write_irq().set(FUNC(maciifx_state::oss_interrupt<9>));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);
	m_asc->irqf_callback().set(FUNC(maciifx_state::oss_interrupt<8>));

	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(maciifx_state::via_in_a));
	m_via1->readpb_handler().set(FUNC(maciifx_state::via_in_b));
	m_via1->writepa_handler().set(FUNC(maciifx_state::via_out_a));
	m_via1->writepb_handler().set(FUNC(maciifx_state::via_out_b));
	m_via1->irq_handler().set(FUNC(maciifx_state::oss_interrupt<11>));

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(FUNC(maciifx_state::set_adb_line));

	applepic_device &sccpic(APPLEPIC(config, "sccpic", C15M));
	sccpic.prd_callback().set(m_scc, FUNC(z80scc_device::dc_ab_r));
	sccpic.pwr_callback().set(m_scc, FUNC(z80scc_device::dc_ab_w));
	sccpic.hint_callback().set(FUNC(maciifx_state::oss_interrupt<7>));

	m_scc->out_int_callback().set("sccpic", FUNC(applepic_device::pint_w));
	m_scc->out_wreqa_callback().set("sccpic", FUNC(applepic_device::reqa_w));
	m_scc->out_wreqb_callback().set("sccpic", FUNC(applepic_device::reqb_w));

	applepic_device &swimpic(APPLEPIC(config, "swimpic", C15M));
	swimpic.prd_callback().set(m_fdc, FUNC(applefdintf_device::read));
	swimpic.pwr_callback().set(m_fdc, FUNC(applefdintf_device::write));
	swimpic.hint_callback().set(FUNC(maciifx_state::oss_interrupt<6>));
	swimpic.gpout0_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w)).invert();
	swimpic.gpin_callback().set(FUNC(maciifx_state::adbin_r));

	m_fdc->dat1byte_cb().set("swimpic", FUNC(applepic_device::reqa_w));

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(maciifx_state::oss_interrupt<0>));
	nubus.out_irqa_callback().set(FUNC(maciifx_state::oss_interrupt<1>));
	nubus.out_irqb_callback().set(FUNC(maciifx_state::oss_interrupt<2>));
	nubus.out_irqc_callback().set(FUNC(maciifx_state::oss_interrupt<3>));
	nubus.out_irqd_callback().set(FUNC(maciifx_state::oss_interrupt<4>));
	nubus.out_irqe_callback().set(FUNC(maciifx_state::oss_interrupt<5>));

	NUBUS_SLOT(config, "nb9", "nubus", mac_nubus_cards, "mdc824");
	NUBUS_SLOT(config, "nba", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbb", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);
}

ROM_START(maciifx)
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD("4147dd77.rom", 0x000000, 0x080000, CRC(ef441bbd) SHA1(9fba3d4f672a630745d65788b1d1119afa2c6728))
ROM_END

}   // anonymous namespace

COMP(1990, maciifx, 0, 0, maciifx, maciifx, maciifx_state, empty_init, "Apple Computer", "Macintosh IIfx", MACHINE_SUPPORTS_SAVE)
