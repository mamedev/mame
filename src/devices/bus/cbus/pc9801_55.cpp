// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-55/-55U/-55L

SCSI interface, running on WD33C93A

TODO:
- Accesses registers that doesn't exist in wd33c9x core, NEC overlay?
- DIP is never taken (definitely lies at vector 0x2c -> PC=0xdc01e);
- DMA / DRQ;
- PC-9801-55 also runs on this except with vanilla WD33C93 instead;

**************************************************************************************************/

#include "emu.h"
#include "bus/cbus/pc9801_55.h"

DEFINE_DEVICE_TYPE(PC9801_55U, pc9801_55u_device, "pc9801_55u", "NEC PC-9801-55U")
DEFINE_DEVICE_TYPE(PC9801_55L, pc9801_55l_device, "pc9801_55l", "NEC PC-9801-55L")

ROM_START( pc9801_55u )
	ROM_REGION16_LE( 0x10000, "scsi_bios", ROMREGION_ERASEFF )
	// JNC2B_00.BIN                                    BADADDR         ---xxxxxxxxxxxx
	// JNC3B_00.BIN                                    BADADDR         ---xxxxxxxxxxxx
	ROM_LOAD16_BYTE( "jnc2b_00.bin", 0x000000, 0x008000, CRC(ddace1b7) SHA1(614569be28a90bd385cf8abc193e629e568125b7) )
	ROM_LOAD16_BYTE( "jnc3b_00.bin", 0x000001, 0x008000, CRC(b8a8a49e) SHA1(7781dab492df889148e070a7da7ead207e18ed04) )
ROM_END

const tiny_rom_entry *pc9801_55u_device::device_rom_region() const
{
	return ROM_NAME( pc9801_55u );
}

ROM_START( pc9801_55l )
	ROM_REGION16_LE( 0x10000, "scsi_bios", ROMREGION_ERASEFF )
	// ETA1B_00.BIN                                    BADADDR         ---xxxxxxxxxxxx
	// ETA3B_00.BIN                                    BADADDR         ---xxxxxxxxxxxx
	ROM_LOAD16_BYTE( "eta1b_00.bin", 0x000000, 0x008000, CRC(300ff6c1) SHA1(6cdee535b77535fe6c4dda4427aeb803fcdea0b8) )
	ROM_LOAD16_BYTE( "eta3b_00.bin", 0x000001, 0x008000, CRC(44477512) SHA1(182bb45ba0da7a4f9113e268e04ffca8403cf164) )
ROM_END

const tiny_rom_entry *pc9801_55l_device::device_rom_region() const
{
	return ROM_NAME( pc9801_55l );
}

void pc9801_55_device::scsi_irq_w(int state)
{
	// TODO: should be INT3, but BIOS configures as INT0 somewhere (unhandled dip reading?)
	m_bus->int_w<0>(state);
}

void pc9801_55_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsi_bus);
	// TODO: currently returning default_scsi_devices, checkout if true for PC-98
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wdc", WD33C93A).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &adapter = downcast<wd33c9x_base_device &>(*device);

			// TODO: unknown clock
			adapter.set_clock(10'000'000);
			adapter.irq_cb().set(*this, FUNC(pc9801_55_device::scsi_irq_w));
			// TODO: DRQ on C-bus
			//adapter.drq_cb().set(*this, FUNC(pc9801_55_device::scsi_drq));
		}
	);
}

static INPUT_PORTS_START( pc9801_55 )
	PORT_START("SCSI_DSW1")
	PORT_DIPNAME( 0x07, 0x07, "PC-9801-55: SCSI board ID") PORT_DIPLOCATION("SCSI_SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x38, 0x18, "PC-9801-55: Interrupt level") PORT_DIPLOCATION("SCSI_SW1:!4,!5,!6")
	PORT_DIPSETTING(    0x00, "INT0" )
	PORT_DIPSETTING(    0x08, "INT1" )
	PORT_DIPSETTING(    0x10, "INT2" )
	PORT_DIPSETTING(    0x18, "INT3" )
	PORT_DIPSETTING(    0x20, "INT5" )
	PORT_DIPSETTING(    0x28, "INT6" )
	PORT_DIPSETTING(    0x30, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x38, DEF_STR( Unknown ) )
	PORT_DIPNAME( 0xc0, 0x00, "PC-9801-55: DMA channel") PORT_DIPLOCATION("SCSI_SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1 (prohibited)" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("SCSI_DSW2")
	// TODO: understand all valid possible settings of this
	PORT_DIPNAME( 0x7f, 0x66, "PC-9801-55: machine ID and ROM base address") PORT_DIPLOCATION("SCSI_SW2:!1,!2,!3,!4,!5,!6,!7")
	PORT_DIPSETTING(    0x66, "i386, 0xdc000-0xddfff")
	// ...
	PORT_DIPNAME( 0x80, 0x80, "PC-9801-55: ROM accessibility at Power-On") PORT_DIPLOCATION("SCSI_SW2:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x00, DEF_STR( No ))

	PORT_START("SCSI_JP")
	// SW3 and SW4 Jumper settings
	PORT_CONFNAME( 0x03, 0x00, "PC-9801-55: I/O base address")
	PORT_CONFSETTING(    0x00, "0xcc0") // 01-02 01 02
	PORT_CONFSETTING(    0x01, "0xcd0")
	PORT_CONFSETTING(    0x02, "0xce0")
	PORT_CONFSETTING(    0x03, "0xcf0")
INPUT_PORTS_END

ioport_constructor pc9801_55_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_55 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_55u_device - constructor
//-------------------------------------------------

pc9801_55_device::pc9801_55_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_scsi_bus(*this, "scsi")
	, m_wdc(*this, "scsi:7:wdc")
{
}

pc9801_55u_device::pc9801_55u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_55_device(mconfig, PC9801_55U, tag, owner, clock)
{

}

pc9801_55l_device::pc9801_55l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_55_device(mconfig, PC9801_55L, tag, owner, clock)
{

}

void pc9801_55_device::device_validity_check(validity_checker &valid) const
{
}

void pc9801_55_device::device_start()
{
	m_bus->program_space().install_rom(
		0xdc000,
		0xddfff,
		memregion(this->subtag("scsi_bios").c_str())->base()
	);

	// TODO: docs hints that this has mirrors at 0xcd*, 0xce*, 0xcf*
	m_bus->install_io(
		0xcc0,
		0xcc5,
		read8sm_delegate(*this, FUNC(pc9801_55_device::comms_r)),
		write8sm_delegate(*this, FUNC(pc9801_55_device::comms_w))
	);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_55_device::device_reset()
{

}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 pc9801_55_device::comms_r(offs_t offset)
{
	if((offset & 1) == 0)
	{
		offs_t addr = offset >> 1;
		if (addr & 2)
		{
			logerror("%s: Read to status port [%02x]\n", machine().describe_context(), offset + 0xcc0);

			return 0;
		}

		return m_wdc->indir_r(addr);
	}
	// odd

	logerror("%s: Read to undefined port [%02x]\n", machine().describe_context(), offset + 0xcc0);

	return 0xff;
}

void pc9801_55_device::comms_w(offs_t offset, u8 data)
{
	if((offset & 1) == 0)
	{
		offs_t addr = offset >> 1;
		if (addr & 2)
		{
			logerror("%s: Write to command port [%02x] %02x\n", machine().describe_context(), offset + 0xcc0, data);

			return;
		}

		m_wdc->indir_w(addr, data);
		return;
	}

	// odd
	logerror("%s: Write to undefined port [%02x] %02x\n", machine().describe_context(), offset + 0xcc0, data);
}
