// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Philips BO "Videosynthesizer Prototype" skeleton

*******************************************************************************/

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "imagedev/floppy.h"
#include "machine/am79c90.h"
#include "machine/hd63450.h"
#include "machine/mc68681.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"

#define VERBOSE         (1)
#include "logmacro.h"

namespace {

class pbo_state : public driver_device
{
public:
	pbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_netcpu(*this, "netcpu")
		, m_scc(*this, "scc")
		, m_rs232(*this, "rs232%u", 0U)
		, m_fdc(*this, "fdc")
		, m_hdc(*this, "scsi:7:ncr5380")
		, m_dmac(*this, "dmac")
		, m_lance(*this, "lance")
		, m_main_ram_share(*this, "main_ram")
		, m_net_ram_share(*this, "net_ram")
	{ }

	void pbo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
	void net_map(address_map &map) ATTR_COLD;

	required_device<m68010_device> m_maincpu;
	required_device<m68010_device> m_netcpu;
	required_device<scc85c30_device> m_scc;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_device<mb8877_device> m_fdc;
	required_device<ncr5380_device> m_hdc;
	required_device<hd63450_device> m_dmac;
	required_device<am7990_device> m_lance;
	required_shared_ptr<uint16_t> m_main_ram_share;
	required_shared_ptr<uint16_t> m_net_ram_share;

	uint16_t berr_r(offs_t offset);
	void berr_w(offs_t offset, uint16_t data);
	void scsi_irq_w(int state);
	void dma_irq_w(int state);
	void fdc_irq_w(int state);
	void scc_irq_w(int state);
	void net_irq_w(int state);
	void floppy_select_w(uint8_t data);
	uint8_t fa0101_read();
	uint8_t net_fe00e1_read();
};

void pbo_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share(m_main_ram_share);
	map(0x080000, 0xbfffff).rw(FUNC(pbo_state::berr_r), FUNC(pbo_state::berr_w));
	map(0xf80000, 0xf8ffff).rom().region("maincpu", 0);
	map(0xfa0000, 0xfa0007).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write)).umask16(0x00ff);
	map(0xfa0011, 0xfa0011).w(FUNC(pbo_state::floppy_select_w));
	map(0xfa0030, 0xfa0037).rw(m_scc, FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w)).umask16(0x00ff);
	map(0xfa8400, 0xfa840f).rw(m_hdc, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
	map(0xfa0100, 0xfa0101).r(FUNC(pbo_state::fa0101_read)).umask16(0x00ff);
}

void pbo_state::net_map(address_map &map)
{
	map(0x000000, 0x000fff).ram().share(m_net_ram_share); // Unknown RAM size
	map(0xf00000, 0xf1ffff).ram();
	map(0xf80000, 0xf83fff).rom().region("netcpu", 0);
	map(0xfe00e1, 0xfe00e1).r(FUNC(pbo_state::net_fe00e1_read));
	map(0xfe0100, 0xfe011f).rw("duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
}

uint16_t pbo_state::berr_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x080000 + offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

void pbo_state::berr_w(offs_t offset, uint16_t data)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0x080000 + offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

void pbo_state::scsi_irq_w(int state)
{
	LOG("SCSI IRQ: %d\n", state);
	m_maincpu->set_input_line(M68K_IRQ_5, state);
}

void pbo_state::dma_irq_w(int state)
{
	LOG("DMA IRQ: %d\n", state);
}

void pbo_state::fdc_irq_w(int state)
{
	LOG("FDC IRQ: %d\n", state);
}

void pbo_state::scc_irq_w(int state)
{
	LOG("DMA IRQ: %d\n", state);
}

void pbo_state::net_irq_w(int state)
{
	LOG("LANCE IRQ: %d\n", state);
}

void pbo_state::floppy_select_w(uint8_t data)
{
	LOG("Floppy select: %02X\n", data);
}

// It's unclear what hardware this location corresponds to on the actual board.
// If it's unmapped, which returns 0 by default, the system hangs with very little
//   meaningful external access.
// If bit 7 is set, the system defaults to using the serial console.
uint8_t pbo_state::fa0101_read()
{
	LOG("Read from FA0101: 80\n");
	return 0x80;
}

uint8_t pbo_state::net_fe00e1_read()
{
	return 0;
}

static INPUT_PORTS_START( pbo )
INPUT_PORTS_END

void pbo_state::machine_start()
{
}

void pbo_state::machine_reset()
{
	uint16_t *src = (uint16_t*)memregion("maincpu")->base();
	uint16_t *dst = &m_main_ram_share[0];
	memcpy(dst, src, 8);

	src = (uint16_t*)memregion("netcpu")->base();
	dst = &m_net_ram_share[0];
	memcpy(dst, src, 8);
}

static void pbo_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

static void pbo_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void pbo_state::pbo(machine_config &config)
{
	M68010(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pbo_state::main_map);

	SCC85C30(config, m_scc, 10_MHz_XTAL); // Unknown PCLK
	m_scc->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	m_scc->out_txda_callback().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set(m_rs232[0], FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set(m_rs232[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_int_callback().set(FUNC(pbo_state::scc_irq_w));

	RS232_PORT(config, m_rs232[0], default_rs232_devices, "terminal");
	m_rs232[0]->cts_handler().set(m_scc[0], FUNC(scc85c30_device::ctsa_w));
	m_rs232[0]->dcd_handler().set(m_scc[0], FUNC(scc85c30_device::dcda_w));
	m_rs232[0]->rxd_handler().set(m_scc[0], FUNC(scc85c30_device::rxa_w));

	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->cts_handler().set(m_scc[0], FUNC(scc85c30_device::ctsb_w));
	m_rs232[1]->dcd_handler().set(m_scc[0], FUNC(scc85c30_device::dcdb_w));
	m_rs232[1]->rxd_handler().set(m_scc[0], FUNC(scc85c30_device::rxb_w));

	HD63450(config, m_dmac, 8_MHz_XTAL, m_maincpu); // MC68450 compatible
	m_dmac->set_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2)); // Guesses
	m_dmac->set_burst_clocks(attotime::from_usec(32), attotime::from_nsec(450), attotime::from_nsec(50), attotime::from_nsec(50)); // Guesses
	m_dmac->irq_callback().set(FUNC(pbo_state::dma_irq_w));
	//m_dmac->dma_read<0>().set(m_hdc, FUNC(ncr5380_device::dma_r));
	//m_dmac->dma_write<0>().set(m_hdc, FUNC(ncr5380_device::dma_w));

	MB8877(config, m_fdc, 8_MHz_XTAL / 8); // Unknown clock
	//m_fdc->set_force_ready(true);
	m_fdc->intrq_wr_callback().set(FUNC(pbo_state::fdc_irq_w));
	//m_fdc->drq_wr_callback().set(FUNC(pbo_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pbo_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);

	NSCSI_BUS(config, "scsi");

	NSCSI_CONNECTOR(config, "scsi:0", pbo_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", pbo_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", pbo_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", pbo_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", pbo_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", pbo_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", pbo_scsi_devices, nullptr);

	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380).machine_config(
		[this](device_t *device)
		{
			ncr5380_device &adapter = downcast<ncr5380_device &>(*device);

			adapter.irq_handler().set(*this, FUNC(pbo_state::scsi_irq_w));
		});

	M68010(config, m_netcpu, 20_MHz_XTAL / 2); // Confirmed
	m_netcpu->set_addrmap(AS_PROGRAM, &pbo_state::net_map);

	AM7990(config, m_lance);
	m_lance->intr_out().set(FUNC(pbo_state::net_irq_w));

	MC68681(config, "duart", 3'686'400);
}

ROM_START( pbo )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE( "even_4.2.bin", 0x0000, 0x8000, CRC(4a10eac1) SHA1(77ef3b17565cd58ccf86df753b30e4ed6212d729) )
	ROM_LOAD16_BYTE( "odd_4.2.bin", 0x0001, 0x8000, CRC(bed19ea4) SHA1(f69264c192965f65ecc6e39130f3a4861a475f92) )

	ROM_REGION(0x4000, "netcpu", 0)
	ROM_LOAD16_BYTE( "knlrom10_4.2_h.bin", 0x0000, 0x2000, CRC(28ecadca) SHA1(0eca5b407c7d4fc1312b96ccb486952206d627a9) )
	ROM_LOAD16_BYTE( "knlrom10_4.2_l.bin", 0x0001, 0x2000, CRC(40bba894) SHA1(924979ea7e383cb76bc68c785933bb5595446eae) )
ROM_END

} // Anonymous namespace

COMP( 1987, pbo,      0,      0,      pbo,      pbo,      pbo_state, empty_init, "Philips",    "BO (Videosynthesizer Prototype)", MACHINE_IS_SKELETON )
