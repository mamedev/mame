// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Applied Engineering Quadralink 4-port serial card

***************************************************************************/

#include "emu.h"
#include "quadralink.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
#include "machine/z80scc.h"
#include "screen.h"

namespace {
class nubus_quadralink_device : public device_t,
								public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_quadralink_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_quadralink_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<z80scc_device> m_scc1, m_scc2;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u32 dev_r(offs_t offset, u32 mem_mask = ~0);
	void dev_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};

static void ql_com(device_slot_interface &device)
{
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
}

ROM_START( quadralink )
	ROM_REGION(0x4000, "declrom", 0)
	ROM_LOAD( "ql_2.2.bin",   0x000000, 0x004000, CRC(88c323b8) SHA1(d70bc32ad50ceb5cb75c6251293cacd65d8313aa) )
ROM_END

void nubus_quadralink_device::device_add_mconfig(machine_config &config)
{
	SCC8530(config, m_scc1, 3.6864_MHz_XTAL);
	m_scc1->out_txda_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	m_scc1->out_txdb_callback().set("serport1", FUNC(rs232_port_device::write_txd));

	SCC8530(config, m_scc2, 3.6864_MHz_XTAL);
	m_scc2->out_txda_callback().set("serport2", FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set("serport3", FUNC(rs232_port_device::write_txd));

	rs232_port_device &serport0(RS232_PORT(config, "serport0", ql_com, nullptr));
	serport0.rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));
	serport0.dcd_handler().set(m_scc1, FUNC(z80scc_device::dcda_w));
	serport0.cts_handler().set(m_scc1, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", ql_com, nullptr));
	serport1.rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));
	serport1.dcd_handler().set(m_scc1, FUNC(z80scc_device::dcdb_w));
	serport1.cts_handler().set(m_scc1, FUNC(z80scc_device::ctsb_w));

	rs232_port_device &serport2(RS232_PORT(config, "serport2", ql_com, nullptr));
	serport2.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	serport2.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	serport2.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &serport3(RS232_PORT(config, "serport3", ql_com, nullptr));
	serport3.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	serport3.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	serport3.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));
}

const tiny_rom_entry *nubus_quadralink_device::device_rom_region() const
{
	return ROM_NAME( quadralink );
}

nubus_quadralink_device::nubus_quadralink_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_quadralink_device(mconfig, NUBUS_QUADRALINK, tag, owner, clock)
{
}

nubus_quadralink_device::nubus_quadralink_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_scc1(*this, "scc1"),
	m_scc2(*this, "scc2")
{
}

void nubus_quadralink_device::device_start()
{
	u32 slotspace;

	install_declaration_rom("declrom");

	slotspace = get_slotspace();

	nubus().install_device(slotspace, slotspace+0xefffff, read32s_delegate(*this, FUNC(nubus_quadralink_device::dev_r)), write32s_delegate(*this, FUNC(nubus_quadralink_device::dev_w)));
}

void nubus_quadralink_device::dev_w(offs_t offset, u32 data, u32 mem_mask)
{
	//printf("write %x to QL space @ %x, mask %08x\n", data, offset, mem_mask);
	switch (offset)
	{
		case 0x0000:    // SCC 2 A control
			m_scc2->ca_w(0, data & 0xff);
			break;

		case 0x0002:    // SCC 2 A data
			m_scc2->da_w(0, data & 0xff);
			break;

		case 0x0004:    // SCC 2 B control
			m_scc2->cb_w(0, data & 0xff);
			break;

		case 0x0006:    // SCC 2 B data
			m_scc2->db_w(0, data & 0xff);
			break;

		case 0x10000:   // SCC 1 A control
			m_scc1->ca_w(0, data & 0xff);
			break;

		case 0x10002:   // SCC 1 A data
			m_scc1->da_w(0, data & 0xff);
			break;

		case 0x10004:   // SCC 1 B control
			m_scc1->cb_w(0, data & 0xff);
			break;

		case 0x10006:   // SCC 1 B data
			m_scc1->db_w(0, data & 0xff);
			break;
	}
}

u32 nubus_quadralink_device::dev_r(offs_t offset, u32 mem_mask)
{
	//printf("read QL space @ %x, mask %08x\n", offset, mem_mask);
	switch (offset)
	{
		case 0x0000:
			return m_scc2->ca_r(0);

		case 0x0002:
			return m_scc2->da_r(0);

		case 0x0004:
			return m_scc2->cb_r(0);

		case 0x0006:
			return m_scc2->db_r(0);

		case 0x10000:
			return m_scc1->ca_r(0);

		case 0x10002:
			return m_scc1->da_r(0);

		case 0x10004:
			return m_scc1->cb_r(0);

		case 0x10006:
			return m_scc1->db_r(0);
	}
	return 0xffffffff;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_QUADRALINK, device_nubus_card_interface, nubus_quadralink_device, "nb_qdlink", "Applied Engineering Quadralink serial card")
