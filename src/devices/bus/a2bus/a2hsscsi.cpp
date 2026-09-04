// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2hsscsi.cpp

    Implementation of the Apple II High Speed SCSI Card

    This uses an ASIC called "Sandwich II"; the card itself is
    sometimes known as "Cocoon".

    Notes:
    C0n0-C0n7 = NCR5380 registers in normal order
    C0n8 = DMA address low
    C0n9 = DMA address high
    C0nA = DMA count low
    C0nB = DMA count high
    C0nC = DMA control
    C0nD = Enable DMA / reset 5380
    C0nE = SW1-3, the card's SCSI ID (read bits 5-7, inverted) /
           Fire watchdog (write bit 7) / ROM bank (read/write bits 0-4)
    C0nF = DMA speed (bit 7 = 0 for fast, 1 for slow) / RAM bank (write bits 0-3)

    DMA control register (C0nC):
    0x01 = pseudo-DMA enable
    0x02 = DMA enable
    0x04 = DMA stopped, address rolled over (read only)
    0x08 = disable stop-DMA-on-IRQ
    0x10 = watchdog still running (read only)
    0x20 = 5380 IRQ enable
    0x40 = SW4, true DMA available (read only)
    0x80 = DMA stopped due to IRQ

    Enable DMA / reset 5380 register (C0nD):
    0x01 = Resume DMA after rollover or IRQ
    0x02 = Reset the 5380
    0x40 = Clear test mode
    0x80 = Set test mode

*********************************************************************/

#include "emu.h"
#include "a2hsscsi.h"

#include "bus/nscsi/devices.h"
#include "machine/ncr5380.h"


namespace {

ROM_START( hsscsi )
	ROM_REGION(0x8000, "scsi_rom", 0)
	ROM_LOAD( "341-0803.bin", 0x0000, 0x8000, CRC(2c15618b) SHA1(7d32227299933bfc1b7f8bc2062906fdfe530674) )
ROM_END

// SW1-3 are the card's own SCSI ID, read back inverted in bits 5-7 of C0nE.
// SW4 enables true DMA, read back in bit 6 of C0nC.
static INPUT_PORTS_START( hsscsi )
	PORT_START("DSW")
	PORT_DIPNAME(0x07, 0x07, "SCSI ID") PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPNAME(0x08, 0x08, "Transfer mode") PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(0x00, "Pseudo-DMA only")
	PORT_DIPSETTING(0x08, "True DMA")
INPUT_PORTS_END

class a2bus_hsscsi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_hsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void drq_w(int state);

protected:
	a2bus_hsscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;
	virtual bool take_c800() const override { return true; }
	virtual void reset_from_bus() override;

	required_device<ncr53c80_device> m_ncr5380;
	required_device<nscsi_bus_device> m_scsibus;
	required_region_ptr<u8> m_rom;
	required_ioport m_dsw;

private:
	static constexpr attotime WATCHDOG_PERIOD = attotime::from_msec(250);

	bool watchdog_running() const { return machine().time() < m_watchdog_expire; }
	bool true_dma_enabled() const { return BIT(m_dsw->read(), 3); }
	void dma_step();

	uint8_t m_ram[8192];
	int m_rambank, m_rombank;
	uint8_t m_drq;
	uint8_t m_bank;
	uint8_t m_dma_control;
	uint8_t m_c0ne, m_c0nf;
	uint16_t m_dma_addr, m_dma_size;
	bool m_dma_active;      // card's DMA sequencer is answering DRQ
	bool m_dma_in;          // true = SCSI to memory, false = memory to SCSI
	attotime m_watchdog_expire;
};

void a2bus_hsscsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", default_scsi_devices, "harddisk", false);

	NCR53C80(config, m_ncr5380);
	m_scsibus->set_external_device(7, m_ncr5380);
	m_ncr5380->drq_handler().set(DEVICE_SELF, FUNC(a2bus_hsscsi_device::drq_w));
}

const tiny_rom_entry *a2bus_hsscsi_device::device_rom_region() const
{
	return ROM_NAME( hsscsi );
}

ioport_constructor a2bus_hsscsi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hsscsi );
}

a2bus_hsscsi_device::a2bus_hsscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, "ncr5380"),
	m_scsibus(*this, "scsibus"),
	m_rom(*this, "scsi_rom"),
	m_dsw(*this, "DSW"),
	m_rambank(0), m_rombank(0), m_drq(0), m_bank(0), m_dma_control(0), m_c0ne(0), m_c0nf(0),
	m_dma_addr(0), m_dma_size(0), m_dma_active(false), m_dma_in(false)
{
}

a2bus_hsscsi_device::a2bus_hsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_hsscsi_device(mconfig, A2BUS_HSSCSI, tag, owner, clock)
{
}

void a2bus_hsscsi_device::device_start()
{
	memset(m_ram, 0, 8192);

	save_item(NAME(m_ram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_rombank));
	save_item(NAME(m_bank));
	save_item(NAME(m_drq));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_c0ne));
	save_item(NAME(m_c0nf));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_dma_size));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_dma_in));
	save_item(NAME(m_watchdog_expire));
}

void a2bus_hsscsi_device::device_reset()
{
	reset_from_bus();
}

void a2bus_hsscsi_device::reset_from_bus()
{
	m_rambank = 0;
	m_rombank = 0;
	m_dma_addr = m_dma_size = 0;
	m_dma_control = 0;
	m_dma_active = false;
	m_dma_in = false;
	m_c0ne = m_c0nf = 0;
	m_watchdog_expire = attotime::zero;

	m_ncr5380->reset();
}

uint8_t a2bus_hsscsi_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 7:
			return m_ncr5380->read(offset);

		case 6:
			if (m_dma_control & 1)  // pseudo-DMA?
			{
				return m_ncr5380->dma_r();
			}
			else
			{
				return m_ncr5380->read(offset);
			}

		case 8: // DMA address low
			return m_dma_addr & 0xff;

		case 9: // DMA address high
			return (m_dma_addr >> 8) & 0xff;

		case 0xa:   // DMA size low
			return m_dma_size & 0xff;

		case 0xb:   // DMA size high
			return (m_dma_size >> 8) & 0xff;

		case 0xc:   // DMA control
		{
			// bit 4 reads as 1 while the watchdog one-shot is still running,
			// bit 6 is SW4: set means true DMA is available
			const uint8_t result = m_dma_control | (watchdog_running() ? 0x10 : 0x00) | (true_dma_enabled() ? 0x40 : 0x00);

			if (!machine().side_effects_disabled())
			{
				m_dma_control &= ~0x04;
			}
			return result;
		}

		case 0xe:   // code at cf32 wants to RMW this without killing the ROM bank
			return (m_c0ne & 0x1f) | ((~m_dsw->read() & 0x07) << 5);

		case 0xf:
			return m_c0nf;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context().c_str());
			break;
	}

	return 0xff;
}

void a2bus_hsscsi_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // data out register; in PDMA mode, it's assumed this goes to DMA as reads do
			if (m_dma_control & 1)
			{
				m_ncr5380->dma_w(data);
			}
			else
			{
				m_ncr5380->write(offset, data);
			}
			break;

		case 5: // start DMA send: 5380 takes data from us
		case 6: // start DMA target receive: 5380 hands data to us
		case 7: // start DMA initiator receive: 5380 hands data to us
			m_dma_in = (offset != 5);
			if (m_dma_control & 0x02)
			{
				m_dma_active = true;
			}
			m_ncr5380->write(offset, data);
			break;

		case 1:
		case 2:
		case 3:
		case 4:
			m_ncr5380->write(offset, data);
			break;

		case 8: // DMA address low
			m_dma_addr &= 0xff00;
			m_dma_addr |= data;
			break;

		case 9: // DMA address high
			m_dma_addr &= 0x00ff;
			m_dma_addr |= (data << 8);
			break;

		case 0xa: // DMA count low
			m_dma_size &= 0xff00;
			m_dma_size |= data;
			break;

		case 0xb: // DMA count high
			m_dma_size &= 0x00ff;
			m_dma_size |= (data << 8);
			break;

		case 0xc:   // DMA control
			m_dma_control &= ~0x2b; // clear the read/write bits
			m_dma_control |= (data & 0x2b);
			if (!(m_dma_control & 0x02))
			{
				m_dma_active = false;
			}
			break;

		case 0xd:   // DMA enable / reset
			//printf("%02x to DMA enable/reset\n", data);
			if ((data & 0x1) && (m_dma_control & 0x02) && m_dma_size && !(m_dma_control & 0x04))
			{
				// Restart a transfer that stopped on an address rollover or IRQ
				m_dma_control &= ~0x80;
				m_dma_active = true;
				dma_step();
			}
			if (data & 0x2)
			{
				m_ncr5380->reset();
			}
			break;

		case 0xe:
			// pulsing bit 7 low resets the watchdog
			if ((m_c0ne & 0x80) && !(data & 0x80))
			{
				m_watchdog_expire = machine().time() + WATCHDOG_PERIOD;
			}
			m_c0ne = data;
			m_rombank = (data & 0x1f) * 0x400;
			break;

		case 0xf:
			m_c0nf = data;
			m_rambank = (data & 0x7) * 0x400;
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context().c_str());
			break;
	}
}

uint8_t a2bus_hsscsi_device::read_cnxx(uint8_t offset)
{
	// one slot image at the start of the ROM, it appears
	return m_rom[offset];
}

void a2bus_hsscsi_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

uint8_t a2bus_hsscsi_device::read_c800(uint16_t offset)
{
	// bankswitched RAM at c800-cbff
	// bankswitched ROM at cc00-cfff
	if (offset < 0x400)
	{
		return m_ram[offset + m_rambank];
	}
	else
	{
		return m_rom[(offset-0x400) + m_rombank];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_hsscsi_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset < 0x400)
	{
		m_ram[offset + m_rambank] = data;
	}
}

void a2bus_hsscsi_device::drq_w(int state)
{
	m_drq = (state ? 0x80 : 0x00);

	dma_step();
}

void a2bus_hsscsi_device::dma_step()
{
	if (!m_dma_active || !m_drq)
	{
		return;
	}

	if (m_dma_in)
	{
		slot_dma_write(m_dma_addr, m_ncr5380->dma_r());
	}
	else
	{
		m_ncr5380->dma_w(slot_dma_read(m_dma_addr));
	}

	m_dma_addr++;
	m_dma_size++;

	if (!m_dma_size)
	{
		m_dma_active = false;
		m_ncr5380->eop_w(1);
		m_ncr5380->eop_w(0);
	}
	else if (!m_dma_addr)
	{
		// bank wrap, need to halt and ask for software to bump the IIgs DMAREG
		m_dma_active = false;
		m_dma_control |= 0x04;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_HSSCSI, device_a2bus_card_interface, a2bus_hsscsi_device, "a2hsscsi", "Apple II High-Speed SCSI Card")
