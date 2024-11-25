// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Oak Solutions SCSI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Oak_SCSI.html

    Podule space :
     ROM: ROM  a0..11 from podule A2..13,
          ROM a12..15 come from latch.
    A14,15 are inverted to allow any size from 2764 to 512 to be used without
    link changes. Data is via D0..7


    MEMC space :
    All reads/writes set the latch from b8..11 of the address used,
    unless b5,6 = 11 (eeprom read)

    Address    Address bit

               2,3,4 : 5380 register select :

    &000                000 SCSI Data read/write
    &004                001 Initiator command register
    &008                010 Mode register
    &00C                011 Target command register
    &010                100 SCSI status/select enable
    &014                101 Bus & status register /start dma send
    &018                110 input data/start dma target receive
    &01C                111 reset IRQ/start DMA init receive

                  5,6 : Main address decode :
    &000                00 Read/write 5380
    &020                01 Read/write 5380 DMA data (uses DACK instead of CS)
                          (DMA mode bit determines 8 or 16 bit DMA mode)
    &040                10 d8 = byte flag, d9 = irq state, d10 = DRQ state
    &060                11 d0 = EEprom data

    &100            8 : EEProm CS
    &200            9 : ROM A12, 16 bit DMA enable
    &400           10 : ROM A13, DMA mode select (1=write), EEProm data in
    &800           11 : ROM !A14, EEProm Clock
    &1000          12 : ROM !A15

    When 16 bit DMA is enabled, and b5,6 = 01, the i/o cycle is stretched until
    2 bytes of data have been transferred from SCSI. There is a hardware timeout
    of 125 microseconds. This period can be changed via a link. All other
    accesses are fast MEMC cycles (250ns).

    The latch and 5380 are reset after a hardware reset.

**********************************************************************/

#include "emu.h"
#include "scsi_oak.h"
#include "machine/eepromser.h"
#include "machine/ncr5380.h"
#include "bus/nscsi/devices.h"


namespace {

class arc_scsi_oak_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_oak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_device<ncr5380_device> m_ncr5380;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_region m_podule_rom;

	u8 m_rom_page;
	int m_irq_state;
	int m_drq_state;
};


void arc_scsi_oak_device::ioc_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | (((m_rom_page ^ 0x0c) << 12) & 0xf000)]; })).umask32(0x000000ff);
}

void arc_scsi_oak_device::memc_map(address_map &map)
{
	map(0x0000, 0x3fff).lrw16(
		NAME([this](offs_t offset)
			{
				u16 data = 0xffff;

				offset <<= 2;

				if ((offset & 0xe0) != 0x60)
					m_rom_page = (offset >> 9) & 0x0f;

				m_eeprom->di_write(BIT(offset, 11));
				m_eeprom->clk_write(BIT(offset, 10));
				m_eeprom->cs_write(BIT(offset, 8));

				switch (offset & 0xe0)
				{
				case 0x00:
					data = m_ncr5380->read(offset >> 2);
					break;

				case 0x20:
					data = m_ncr5380->dma_r();

					if (offset & 0x200)
						data |= m_ncr5380->dma_r() << 8;
					break;

				case 0x40:
					data = 0x0000;
					data |= m_irq_state << 9;
					data |= m_drq_state << 8;
					break;

				case 0x60:
					data = m_eeprom->do_read();
					break;
				}

				return data;
			}),
		NAME([this](offs_t offset, u32 data)
			{
				offset <<= 2;

				if ((offset & 0xe0) != 0x60)
					m_rom_page = (offset >> 9) & 0x0f;

				switch (offset & 0xe0)
				{
				case 0x00:
					m_ncr5380->write(offset >> 2, data & 0xff);
					break;

				case 0x20:
					m_ncr5380->dma_w(data & 0xff);

					if (offset & 0x200)
						m_ncr5380->dma_w(data >> 8);
					break;
				}
			})).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( scsi_oak )
//-------------------------------------------------

ROM_START( scsi_oak )
	ROM_REGION(0x10000, "podule_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "136", "Oak SCSI driver 1.36 (29 Nov 1993)")
	ROMX_LOAD("scsi_rom_1.36.rom", 0x0000, 0x10000, CRC(9571ac99) SHA1(bf41e422135ceb8048b04efae10cea32eb4bb864), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "116", "Oak SCSI driver 1.16 (04 Oct 1990)")
	ROMX_LOAD("scsi_rom_1.16.rom", 0xc000, 0x04000, CRC(f05c414c) SHA1(de7f6cc27ff09b34b132d7ced5123926005be18d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "137", "Oak SCSI driver 1.37 (27 Jul 1994)")
	ROMX_LOAD("scsi_rom_1.37.rom", 0x0000, 0x10000, CRC(c1e12d48) SHA1(cba365e35aa84d6a9ca98d678fa61fe3a7a7ac3c), ROM_BIOS(2))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASEFF)
	ROM_LOAD("93c06n", 0x00, 0x20, CRC(45badabc) SHA1(c102d90e4f5c16fedde1ba4637170a39ed7ab371))
ROM_END

const tiny_rom_entry *arc_scsi_oak_device::device_rom_region() const
{
	return ROM_NAME( scsi_oak );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_oak_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380)
		.machine_config([this](device_t *device)
		{
			downcast<ncr5380_device&>(*device).irq_handler().set([this](int state) { m_irq_state = state; });
			downcast<ncr5380_device &>(*device).drq_handler().set([this](int state) { m_drq_state = state; });
		});

	EEPROM_93C06_16BIT(config, m_eeprom);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_oak_device - constructor
//-------------------------------------------------

arc_scsi_oak_device::arc_scsi_oak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCSI_OAK, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_ncr5380(*this, "scsi:7:ncr5380")
	, m_eeprom(*this, "eeprom")
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
	, m_irq_state(0)
	, m_drq_state(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_oak_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scsi_oak_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_OAK, device_archimedes_podule_interface, arc_scsi_oak_device, "arc_scsi_oak", "Oak Solutions 16 bit SCSI Interface")
