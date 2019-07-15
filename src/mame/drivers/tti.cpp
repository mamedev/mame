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
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/mc68901.h"
#include "machine/ncr5390.h"

class tti_state : public driver_device
{
public:
	tti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mfp(*this, "mfp")
		, m_asc(*this, "scsibus:7:asc")
		, m_dma_address(0)
	{ }

	void tti(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	u8 asc_r(offs_t offset);
	void asc_w(offs_t offset, u8 data);
	void dma_address_w(offs_t offset, u8 data);
	u8 io_status_r();
	void channel_w(u8 data);

	void asc_config(device_t *device);

	IRQ_CALLBACK_MEMBER(intack);

	void prg_map(address_map &map);
	void fc7_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
	required_device<ncr53c90a_device> m_asc;

	u32 m_dma_address;
};

void tti_state::machine_start()
{
	save_item(NAME(m_dma_address));
}

IRQ_CALLBACK_MEMBER(tti_state::intack)
{
	return m_mfp->get_vector();
}

static INPUT_PORTS_START( tti )
INPUT_PORTS_END

u8 tti_state::asc_r(offs_t offset)
{
	return m_asc->read(offset ^ 1);
}

void tti_state::asc_w(offs_t offset, u8 data)
{
	m_asc->write(offset ^ 1, data);
}

void tti_state::dma_address_w(offs_t offset, u8 data)
{
	m_dma_address &= ~(0xff000000 >> (offset * 8));
	m_dma_address |= u32(data) << (24 - offset * 8);
}

u8 tti_state::io_status_r()
{
	return 0;
}

void tti_state::channel_w(u8 data)
{
	switch (data & 0x03)
	{
	case 0:
		m_mfp->i4_w(0);
		break;

	case 1:
		m_mfp->i3_w(0);
		break;

	case 3:
		m_mfp->i5_w(0);
		break;
	}
}

void tti_state::prg_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("maincpu", 0);
	map(0x7e000, 0x7ffff).ram();
	map(0x80000, 0x80017).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0x80018, 0x8001f).ram();
	map(0x80020, 0x8002b).rw(FUNC(tti_state::asc_r), FUNC(tti_state::asc_w));
	map(0x80070, 0x80077).w("bitlatch", FUNC(ls259_device::write_d0));
	map(0x80078, 0x8007b).w(FUNC(tti_state::dma_address_w));
	map(0x8007c, 0x8007c).r(FUNC(tti_state::io_status_r));
	map(0x8007d, 0x8007d).w(FUNC(tti_state::channel_w));
}

void tti_state::fc7_map(address_map &map)
{
	map(0xffff5, 0xffff5).r(m_mfp, FUNC(mc68901_device::get_vector));
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
	m_maincpu->set_addrmap(m68008_device::AS_CPU_SPACE, &tti_state::fc7_map);

	MC68901(config, m_mfp, 20_MHz_XTAL / 2); // guess
	m_mfp->set_timer_clock(20_MHz_XTAL / 2); // guess
	m_mfp->set_rx_clock(9600); // for testing (FIXME: actually 16x)
	m_mfp->set_tx_clock(9600); // for testing (FIXME: actually 16x)
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_irq_cb().set_inputline("maincpu", M68K_IRQ_2); // probably

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
