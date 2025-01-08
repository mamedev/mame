// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    Mac Centris 610 ("WLCD")
    Mac Centris 650 ("Wombat 25")
    Mac Quadra 610 ("Speedbump 610")
    Mac Quadra 650 ("Speedbump 650")
    Mac Quadra 800 ("Wombat 33")

    By R. Belmont

    These second-generation 68040 machines shrunk the huge mass of separate
    chips found in the Quadra 700 down to a pair of ASICs, djMEMC (memory controller
    plus revised DAFB video) and IOSB (I/O bus adaptor with integrated VIAs,
    audio, "Turbo SCSI", and SWIM2 floppy).

****************************************************************************/

#include "emu.h"

#include "adbmodem.h"
#include "dfac.h"
#include "djmemc.h"
#include "iosb.h"
#include "macadb.h"
#include "mactoolbox.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68040.h"
#include "machine/dp83932c.h"
#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80scc.h"

#include "softlist_dev.h"

#define C32M 31.3344_MHz_XTAL
#define C15M (C32M/2)
#define C7M (C32M/4)

namespace {

class quadra800_state : public driver_device
{
public:
	quadra800_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_djmemc(*this, "djmemc"),
		m_iosb(*this, "iosb"),
		m_dfac(*this, "dfac"),
		m_macadb(*this, "macadb"),
		m_adbmodem(*this, "adbmodem"),
		m_scc(*this, "scc"),
		m_ram(*this, RAM_TAG),
		m_scsibus(*this, "scsi"),
		m_ncr1(*this, "scsi:7:ncr53c96"),
		m_sonic(*this, "sonic")
	{
	}

	void macqd800(machine_config &config);
	void macct610(machine_config &config);
	void macct650(machine_config &config);
	void macqd610(machine_config &config);
	void macqd650(machine_config &config);

	void quadra800_map(address_map &map) ATTR_COLD;

	void init_macqd800();

private:
	required_device<m68040_device> m_maincpu;
	required_device<djmemc_device> m_djmemc;
	required_device<iosb_device> m_iosb;
	required_device<dfac_device> m_dfac;
	required_device<macadb_device> m_macadb;
	required_device<adbmodem_device> m_adbmodem;
	required_device<z80scc_device> m_scc;
	required_device<ram_device> m_ram;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c96_device> m_ncr1;
	required_device<dp83932c_device> m_sonic;

	u8 m_mac[6];

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 mac_scc_r(offs_t offset)
	{
		m_iosb->via_sync();
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, u16 data) { m_iosb->via_sync(); m_scc->dc_ab_w(offset, data >> 8); }

	u8 ethernet_mac_r(offs_t offset);
};

void quadra800_state::machine_start()
{
	m_djmemc->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());

	// MAC PROM is stored with a bit swizzle and must match one of 2
	// Apple-assigned OUI blocks 00:05:02 or 08:00:07
	const std::array<u8, 6> &MAC = m_sonic->get_mac();
	m_mac[0] = bitswap<8>(0x08, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[1] = bitswap<8>(0x00, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[2] = bitswap<8>(0x07, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[3] = bitswap<8>(MAC[3], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[4] = bitswap<8>(MAC[4], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[5] = bitswap<8>(MAC[5], 0, 1, 2, 3, 7, 6, 5, 4);
	m_sonic->set_mac(&m_mac[0]);
}

void quadra800_state::machine_reset()
{
}

void quadra800_state::init_macqd800()
{
}

u8 quadra800_state::ethernet_mac_r(offs_t offset)
{
	if (offset < 6)
	{
		return m_mac[offset];
	}
	else if (offset == 7)
	{
		u8 xor_total = 0;

		for (int i = 0; i < 6; i++)
		{
			xor_total ^= (u8)m_mac[i];
		}

		return xor_total ^ 0xff;
	}

	return 0;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void quadra800_state::quadra800_map(address_map &map)
{
	map(0x00000000, 0xffffffff).m(m_djmemc, FUNC(djmemc_device::map));
	map(0x50000000, 0x5fffffff).m(m_iosb, FUNC(iosb_device::map));

	map(0x50008000, 0x50008007).r(FUNC(quadra800_state::ethernet_mac_r)).mirror(0x00fc0000);
	map(0x5000a000, 0x5000b0ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0x0000ffff).mirror(0x00fc0000);
	map(0x5000c000, 0x5000dfff).rw(FUNC(quadra800_state::mac_scc_r), FUNC(quadra800_state::mac_scc_2_w)).mirror(0x00fc0000);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void quadra800_state::macqd800(machine_config &config)
{
	M68040(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra800_state::quadra800_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	DJMEMC(config, m_djmemc, 33_MHz_XTAL);
	m_djmemc->set_maincpu_tag("maincpu");
	m_djmemc->set_rom_tag("bootrom");
	m_djmemc->write_irq().set(m_iosb, FUNC(iosb_device::via2_irq_w<0x40>));

	IOSB(config, m_iosb, 33_MHz_XTAL);
	m_iosb->set_maincpu_tag("maincpu");
	m_iosb->set_scsi_tag("scsi:7:ncr53c96");
	m_iosb->write_adb_st().set(m_adbmodem, FUNC(adbmodem_device::set_via_state));

	// Quadra 800 ID is 0x12
	m_iosb->read_pa1().set_constant(1);
	m_iosb->read_pa2().set_constant(0);
	m_iosb->read_pa4().set_constant(1);
	m_iosb->read_pa6().set_constant(0);

	APPLE_DFAC(config, m_dfac, 22257);
	m_iosb->write_dfac_clock().set(m_dfac, FUNC(dfac_device::clock_write));
	m_iosb->write_dfac_data().set(m_dfac, FUNC(dfac_device::data_write));
	m_iosb->write_dfac_latch().set(m_dfac, FUNC(dfac_device::latch_write));

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(m_iosb, FUNC(iosb_device::scc_irq_w));
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

	// SCSI bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^iosb:lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^iosb:rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c96", NCR53C96).clock(40_MHz_XTAL).machine_config(
		[this] (device_t *device)
		{
			ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

			adapter.set_busmd(ncr53c96_device::BUSMD_1);
			adapter.irq_handler_cb().set(m_iosb, FUNC(iosb_device::scsi_irq_w));
			adapter.drq_handler_cb().set(m_iosb, FUNC(iosb_device::scsi_drq_w));
		});

	DP83932C(config, m_sonic, 40_MHz_XTAL / 2); // clock is C20M on the schematics
	m_sonic->set_bus(m_maincpu, 0);
	m_sonic->out_int_cb().set(m_iosb, FUNC(iosb_device::via2_irq_w<0x01>));

	nubus_device &nubus(NUBUS(config, "nubus", 40_MHz_XTAL / 4));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqc_callback().set(m_iosb, FUNC(iosb_device::via2_irq_w<0x08>));
	nubus.out_irqd_callback().set(m_iosb, FUNC(iosb_device::via2_irq_w<0x10>));
	nubus.out_irqe_callback().set(m_iosb, FUNC(iosb_device::via2_irq_w<0x20>));
	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	ADBMODEM(config, m_adbmodem, C7M);
	m_adbmodem->via_clock_callback().set(m_iosb, FUNC(iosb_device::cb1_w));
	m_adbmodem->via_data_callback().set(m_iosb, FUNC(iosb_device::cb2_w));
	m_adbmodem->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_adbmodem->irq_callback().set(m_iosb, FUNC(iosb_device::pb3_w));
	m_iosb->write_cb2().set(m_adbmodem, FUNC(adbmodem_device::set_via_data));
	config.set_perfect_quantum(m_maincpu);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(m_adbmodem, FUNC(adbmodem_device::set_adb_line));

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,32M,64M,96M,128M,192M,256M,320M,384M,512M,640M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");
	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

void quadra800_state::macct610(machine_config &config)
{
	macqd800(config);

	M68040(config.replace(), m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra800_state::quadra800_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	// Centris 610 ID is 0x40
	m_iosb->read_pa1().set_constant(0);
	m_iosb->read_pa2().set_constant(0);
	m_iosb->read_pa4().set_constant(0);
	m_iosb->read_pa6().set_constant(1);
}

void quadra800_state::macct650(machine_config &config)
{
	macqd800(config);

	M68040(config.replace(), m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra800_state::quadra800_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	// Centris 650 ID is 0x46
	m_iosb->read_pa1().set_constant(1);
	m_iosb->read_pa2().set_constant(1);
	m_iosb->read_pa4().set_constant(0);
	m_iosb->read_pa6().set_constant(1);
}

void quadra800_state::macqd610(machine_config &config)
{
	macqd800(config);

	M68040(config.replace(), m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra800_state::quadra800_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	// Quadra 610 ID is 0x44
	m_iosb->read_pa1().set_constant(0);
	m_iosb->read_pa2().set_constant(1);
	m_iosb->read_pa4().set_constant(0);
	m_iosb->read_pa6().set_constant(1);
}

void quadra800_state::macqd650(machine_config &config)
{
	macqd800(config);

	M68040(config.replace(), m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra800_state::quadra800_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	// Quadra 650 ID is 0x52
	m_iosb->read_pa1().set_constant(1);
	m_iosb->read_pa2().set_constant(0);
	m_iosb->read_pa4().set_constant(1);
	m_iosb->read_pa6().set_constant(1);
}

ROM_START( macqd800 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "Version 23F2")
	ROMX_LOAD( "f1acad13.rom", 0x000000, 0x100000, CRC(4e70e3c0) SHA1(f2a9ce387019bf272c6e3459d961b30f28942ac5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "original", "Version 23F1")
	ROMX_LOAD( "f1a6f343.rom", 0x000000, 0x100000, CRC(3318a935) SHA1(031f13bfd726a70cfe4c73c5967861bc77297a79), ROM_BIOS(1) )
ROM_END

#define rom_macct610 rom_macqd800
#define rom_macct650 rom_macqd800
#define rom_macqd610 rom_macqd800
#define rom_macqd650 rom_macqd800

} // anonymous namespace

COMP( 1993, macqd800, 0, 0, macqd800, macadb, quadra800_state, init_macqd800,  "Apple Computer", "Macintosh Quadra 800", MACHINE_SUPPORTS_SAVE)
COMP( 1993, macct610, macqd800, 0, macct610, macadb, quadra800_state, init_macqd800,  "Apple Computer", "Macintosh Centris 610", MACHINE_SUPPORTS_SAVE)
COMP( 1993, macct650, macqd800, 0, macct650, macadb, quadra800_state, init_macqd800,  "Apple Computer", "Macintosh Centris 650", MACHINE_SUPPORTS_SAVE)
COMP( 1993, macqd610, macqd800, 0, macqd610, macadb, quadra800_state, init_macqd800,  "Apple Computer", "Macintosh Quadra 610", MACHINE_SUPPORTS_SAVE)
COMP( 1993, macqd650, macqd800, 0, macqd650, macadb, quadra800_state, init_macqd800,  "Apple Computer", "Macintosh Quadra 650", MACHINE_SUPPORTS_SAVE)
