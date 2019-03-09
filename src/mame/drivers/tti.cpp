// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

2017-11-02 Skeleton

Transitional Technology Inc. SCSI host adapter, possibly part of the QTx (Q-Bus) series or UTx (Unibus) series.
Model number not known, zipfile was named "TTI_10012000.zip"

Chips: NCR 53C90A, Motorola MC68901P, Fujitsu 8464A-10L (8KB static ram), Xicor X24C44P (16x16 serial NOVRAM), and 14 undumped
Lattice PLDs.

Other: LED, 20MHz crystal. Next to the MC68901P is another chip just as large (48 pin DIL), with a huge "MFG. UNDER LICENSE FROM
       DIGITAL EQUIPMENT CORP." sticker covering all details. Assumed to be a Motorola MC68008 CPU.

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/mc68901.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

class tti_state : public driver_device
{
public:
	tti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mfp(*this, "mfp")
	{ }

	void tti(machine_config &config);

private:
	void asc_config(device_t *device);

	IRQ_CALLBACK_MEMBER(intack);

	void prg_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
};

IRQ_CALLBACK_MEMBER(tti_state::intack)
{
	return m_mfp->get_vector();
}

static INPUT_PORTS_START( tti )
INPUT_PORTS_END

void tti_state::prg_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("maincpu", 0);
	map(0x7e000, 0x7ffff).ram();
	map(0x80000, 0x80017).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0x80020, 0x8002b).m("scsibus:7:asc", FUNC(ncr53c90a_device::map));
	map(0x80070, 0x80077).w("bitlatch", FUNC(ls259_device::write_d0));
}

static void tti_scsi_devices(device_slot_interface &device)
{
	// FIXME: these device options are placeholders
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("asc", NCR53C90A);
}

void tti_state::asc_config(device_t *device)
{
	ncr53c90a_device &adapter = downcast<ncr53c90a_device &>(*device);

	adapter.set_clock(20_MHz_XTAL);

	adapter.irq_handler_cb().set(m_mfp, FUNC(mc68901_device::i7_w)).invert();
	//adapter.drq_handler_cb().set(?);
}

void tti_state::tti(machine_config &config)
{
	M68008(config, m_maincpu, 20_MHz_XTAL / 2); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &tti_state::prg_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(tti_state::intack));

	MC68901(config, m_mfp, 20_MHz_XTAL / 2); // guess
	m_mfp->set_timer_clock(20_MHz_XTAL / 2); // guess
	m_mfp->set_rx_clock(9600); // for testing (FIXME: actually 16x)
	m_mfp->set_tx_clock(9600); // for testing (FIXME: actually 16x)
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_irq_cb().set_inputline("maincpu", M68K_IRQ_5);

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:1", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:2", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", tti_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:7", tti_scsi_devices, "asc", true).set_option_machine_config("asc", [this] (device_t *device) { asc_config(device); });

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_mfp, FUNC(mc68901_device::write_rx));

	EEPROM_X24C44_16BIT(config, "novram").do_callback().set("mfp", FUNC(mc68901_device::i0_w));

	ls259_device &bitlatch(LS259(config, "bitlatch")); // U17
	bitlatch.q_out_cb<0>().set("novram", FUNC(eeprom_serial_x24c44_device::di_write));
	bitlatch.q_out_cb<1>().set("novram", FUNC(eeprom_serial_x24c44_device::clk_write));
	bitlatch.q_out_cb<2>().set("novram", FUNC(eeprom_serial_x24c44_device::cs_write));
}

ROM_START( tti )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tti_10012000_rev2.3.bin", 0x0000, 0x8000, CRC(95a5bce8) SHA1(46d7c99e37ca5598aec2062dfd9759853a237c14) )
	ROM_LOAD( "tti_10012000_rev1.7.bin", 0x0000, 0x8000, CRC(6660c059) SHA1(05d97009b5b8034dda520f655c73c474da97f822) )
ROM_END

COMP( 1989, tti, 0, 0, tti, tti, tti_state, empty_init, "Transitional Technology Inc", "unknown TTI SCSI host adapter", MACHINE_IS_SKELETON )
