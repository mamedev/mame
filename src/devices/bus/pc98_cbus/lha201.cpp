// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Logitec LHA-201/20B SCSI-2

PC-9801-92 derived?

https://98epjunk.shakunage.net/sw/ext_card/ext_card_doc/lha201.txt
Maximum -chs 13329,15,63 according to link above (for LHA-20B BIOS, TBD for LHA-201)

TODO:
- Can't IPL boot as standalone (requires a bootable IDE BIOS and moving of ROM window here),
  uses SC_MODE_SENSE_6 pages 0xc3 and 0xc4, then SC_READ_CAPACITY, finally keeps SC_TEST_UNIT_READY
  -> SC_REQUEST_SENSE with length = 0x16 trimmed internally with 18 (?).
  What it reads comes from initial device_reset sense request.
- Need data throttle between here, wd33c9x and/or NSCSI harddisk, otherwise executing anything will
  fail (should be fixed with pc98_hd option?);
- PIO mode (involves port $cc6 and a ready flag in port $37 bit 0)
- Remaining remap options;

**************************************************************************************************/

#include "emu.h"
#include "lha201.h"

DEFINE_DEVICE_TYPE(LHA201, lha201_device, "lha201", "Logitec LHA-201/20B SCSI interface")

lha201_device::lha201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_55_device(mconfig, LHA201, tag, owner, clock)
	, m_dsw3(*this, "DSW3")
{
	m_space_io_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(lha201_device::internal_map), this));
}

ROM_START( lha201 )
	ROM_REGION16_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD("lha-201_ver1.33.ic3", 0x0000, 0x8000, CRC(a5995180) SHA1(4316fd50ce03d1b8f522327c8caa79edc88ac55a) )
ROM_END

const tiny_rom_entry *lha201_device::device_rom_region() const
{
	return ROM_NAME( lha201 );
}

void lha201_device::device_add_mconfig(machine_config &config)
{
	pc9801_55_device::device_add_mconfig(config);
	// two xtals, 28 MHz (MX2) and 20 MHz (MX1)

	NSCSI_CONNECTOR(config.replace(), "scsi:7").option_set("wdc", WD33C93B).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &adapter = downcast<wd33c9x_base_device &>(*device);

			// FIXME: check frequency select in wd core
			adapter.set_clock(20'000'000 / 4);
			adapter.irq_cb().set(*this, FUNC(lha201_device::scsi_irq_w));
			adapter.drq_cb().set(*this, FUNC(lha201_device::scsi_drq_w));
		}
	);

	// 2x DS21S07A active terminators
	// uPD65641 gate array at IC1, chip labeled BMX-2 "REV 930701 9510WD005"
}

static INPUT_PORTS_START( lha201 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "LHA-201: SCSI board ID") PORT_DIPLOCATION("SCSI_SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "0 (prohibited)" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x38, 0x18, "LHA-201: Interrupt level") PORT_DIPLOCATION("SCSI_SW1:!4,!5,!6")
	PORT_DIPSETTING(    0x00, "INT0" )
	PORT_DIPSETTING(    0x08, "INT1" )
	PORT_DIPSETTING(    0x10, "INT2" )
	PORT_DIPSETTING(    0x18, "INT3" )
	PORT_DIPSETTING(    0x20, "INT5" )
	PORT_DIPSETTING(    0x28, "INT6" )
	PORT_DIPSETTING(    0x30, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x38, DEF_STR( Unknown ) )
	PORT_DIPNAME( 0xc0, 0x00, "LHA-201: DMA channel") PORT_DIPLOCATION("SCSI_SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1 (prohibited)" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x06, "LHA-201: ROM address") PORT_DIPLOCATION("SCSI_SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "0xd0000-0xd0fff" )
	PORT_DIPSETTING(    0x01, "0xd2000-0xd2fff" )
	PORT_DIPSETTING(    0x02, "0xd4000-0xd4fff" )
	PORT_DIPSETTING(    0x03, "0xd6000-0xd6fff" )
	PORT_DIPSETTING(    0x04, "0xd8000-0xd8fff" )
	PORT_DIPSETTING(    0x05, "0xda000-0xdafff" )
	PORT_DIPSETTING(    0x06, "0xdc000-0xdcfff" )
	PORT_DIPSETTING(    0x07, "0xde000-0xdefff" )
	PORT_DIPNAME( 0x18, 0x10, "LHA-201: ROM address high reso") PORT_DIPLOCATION("SCSI_SW2:!4,!5")
	PORT_DIPSETTING(    0x00, "0xe8000-0xe8fff" )
	PORT_DIPSETTING(    0x08, "0xea000-0xeafff" )
	PORT_DIPSETTING(    0x10, "0xec000-0xecfff" )
	PORT_DIPSETTING(    0x18, "0xee000-0xeefff" )
	PORT_DIPNAME( 0x60, 0x60, "LHA-201: Model") PORT_DIPLOCATION("SCSI_SW2:!6,!7")
	PORT_DIPSETTING(    0x00, "98XA model" )
	PORT_DIPSETTING(    0x20, "V30 model" )
	PORT_DIPSETTING(    0x40, "98XL,XL^2,RL model" )
	PORT_DIPSETTING(    0x60, "286,386,486 model" )
	PORT_DIPNAME( 0x80, 0x80, "LHA-201: ROM accessibility at Power-On") PORT_DIPLOCATION("SCSI_SW2:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x00, DEF_STR( No ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "LHA-201: I/O base address") PORT_DIPLOCATION("SCSI_SW3:!1,!2")
	PORT_DIPSETTING(   0x00, "0xcc0")
	PORT_DIPSETTING(   0x01, "0xcd0")
	PORT_DIPSETTING(   0x02, "0xce0")
	PORT_DIPSETTING(   0x03, "0xcf0")
	PORT_DIPNAME( 0x04, 0x00, "LHA-201: Transfer mode setting") PORT_DIPLOCATION("SCSI_SW3:!3")
	PORT_DIPSETTING(    0x00, "Automatic selection (bus master)" )
	PORT_DIPSETTING(    0x04, "I/O transfer fixed")
	// following are all "system use"
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SCSI_SW3:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SCSI_SW3:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SCSI_SW3:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SCSI_SW3:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SCSI_SW3:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// TODO: SW5 jumper setting

	// TODO: SW6 setting of main unit model
INPUT_PORTS_END

ioport_constructor lha201_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( lha201 );
}


//void lha201_device::device_validity_check(validity_checker &valid) const
//{
//}
//

void lha201_device::device_start()
{
	pc9801_55_device::device_start();
	save_item(NAME(m_port34));
	save_item(NAME(m_port36));

	m_bus->set_dma_channel(0, this, false);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lha201_device::device_reset()
{
	pc9801_55_device::device_reset();
	// TODO: INT5/INT6
	m_int_line = (m_dsw1->read() & 0x18) >> 3;
	m_port34 = 0;
	m_port36 = 0;
}

void lha201_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: move base in device_reset
		const u32 m_memory_base = 0x000d0000 + ((m_dsw2->read() & 7) * 0x2000);
		m_bus->space(AS_PROGRAM).install_rom(
			m_memory_base,
			m_memory_base + 0xfff,
			m_bios->base() + m_rom_bank * 0x1000
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x3fff, *this, &lha201_device::io_map);
	}
}

void lha201_device::io_map(address_map &map)
{
	pc9801_55_device::io_map(map);
//  0xcc6 direct FIFO read/write in PIO mode
}

void lha201_device::internal_map(address_map &map)
{
	pc9801_55_device::internal_map(map);

	// FIFO int guard on NEC, unknown here
	map(0x34, 0x34).lrw8(
		NAME([this] (offs_t offset) {
			return m_port34;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Port $34 write %02x\n", data);
			m_port34 = data;
		})
	);
	// unknown
	map(0x36, 0x36).lrw8(
		NAME([this] (offs_t offset) {
			return m_port36;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Port $36 write %02x\n", data);
			m_port36 = data;
		})
	);
	map(0x37, 0x37).lr8(
		NAME([this] (offs_t offset) {
			// TODO: not all of it, bit 0 at least is for a status ready in PIO mode (FFE?)
			return m_dsw3->read();
		})
	);
}
