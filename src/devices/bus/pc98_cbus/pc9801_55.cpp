// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-55/-55U/-55L

SCSI interface, running on WD33C93A
HDDs apparently needs to be with 8 heads and 25 cylinders only

https://www7b.biglobe.ne.jp/~drachen6jp/98scsi.html

TODO:
- hangs on wdc core with a Negate ACK command when not initiator (cfr. issue #14532);
- Wants to read "NEC" around PC=dc632, currently reads " SE" (from nscsi/hd.cpp " SEAGATE" inquiry)
\- Wants specifically -ss 256 in chdman, DOS will throw a "run-time error R6003 otherwise".
\- Should really require a nscsi/hd.cpp subclass, seems to detect 130 MB hdd from mode sense 6 pages
   no matter the actual HDD options ...
- Throws Unhandled command LOCATE/POSITION_TO_ELEMENT/SEEK_10 (10): 2b 00 ff ff ff ff 00 00 00 00
  (non fatal?)
- Manages to install msdos622 after all above but doesn't mark HDD as bootable;
- Sometimes BIOS fails to boot entirely, why?
- PC-9801-55 also runs on this except with vanilla WD33C93 instead;

**************************************************************************************************/

#include "emu.h"
#include "pc9801_55.h"

DEFINE_DEVICE_TYPE(PC9801_55U, pc9801_55u_device, "pc9801_55u", "NEC PC-9801-55U SCSI interface")
DEFINE_DEVICE_TYPE(PC9801_55L, pc9801_55l_device, "pc9801_55l", "NEC PC-9801-55L SCSI interface")

pc9801_55_device::pc9801_55_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_scsi_bus(*this, "scsi")
	, m_wdc(*this, "scsi:7:wdc")
//  , m_space_io_config("io_regs", ENDIANNESS_LITTLE, 8, 8, 0, amap)
	, m_bios(*this, "bios")
	, m_dsw1(*this, "DSW1")
	, m_dsw2(*this, "DSW2")
{
}

pc9801_55u_device::pc9801_55u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_55_device(mconfig, PC9801_55U, tag, owner, clock)
{
	m_space_io_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pc9801_55u_device::internal_map), this));

}

pc9801_55l_device::pc9801_55l_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_55_device(mconfig, PC9801_55L, tag, owner, clock)
{
	m_space_io_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pc9801_55l_device::internal_map), this));
}


ROM_START( pc9801_55u )
	ROM_REGION16_LE( 0x10000, "bios", ROMREGION_ERASEFF )
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
	ROM_REGION16_LE( 0x10000, "bios", ROMREGION_ERASEFF )
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
	m_bus->int_w(m_int_line, BIT(m_port30, 2) && state);
}

void pc9801_55_device::scsi_drq_w(int state)
{
	m_bus->drq_w(0, state);
}

u8 pc9801_55_device::dack_r(int line)
{
	//if (!m_dma_enable)
	//  return 0xff;
	const u8 res = m_wdc->dma_r();
	return res;
}

void pc9801_55_device::dack_w(int line, u8 data)
{
	//if (!m_dma_enable)
	//  return;

	m_wdc->dma_w(data);
}

// opt-in, works with specific options only
static void pc98_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_PC98_HD);
	// TODO: at least a CD-ROM option
}


void pc9801_55_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsi_bus);
	// TODO: currently returning default_scsi_devices, checkout if true for PC-98
	NSCSI_CONNECTOR(config, "scsi:0", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", pc98_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wdc", WD33C93A).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &adapter = downcast<wd33c9x_base_device &>(*device);

			// 33C93 @ 8 MHz
			// 33C93A @ 10 MHz
			// 33C93B @ 20 MHz
			adapter.set_clock(10'000'000);
			adapter.irq_cb().set(*this, FUNC(pc9801_55_device::scsi_irq_w));
			adapter.drq_cb().set(*this, FUNC(pc9801_55_device::scsi_drq_w));
		}
	);
}

static INPUT_PORTS_START( pc9801_55 )
	PORT_START("DSW1")
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

	PORT_START("DSW2")
	// TODO: understand all valid possible settings of this
	PORT_DIPNAME( 0x7f, 0x66, "PC-9801-55: machine ID and ROM base address") PORT_DIPLOCATION("SCSI_SW2:!1,!2,!3,!4,!5,!6,!7")
	PORT_DIPSETTING(    0x66, "i386, 0xdc000-0xdcfff")
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


device_memory_interface::space_config_vector pc9801_55_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_io_config)
	};
}


void pc9801_55_device::device_validity_check(validity_checker &valid) const
{
}

void pc9801_55_device::device_start()
{
	save_item(NAME(m_ar));
	save_item(NAME(m_port30));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_pkg_id));
	save_item(NAME(m_dma_enable));

	m_bus->set_dma_channel(0, this, false);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_55_device::device_reset()
{
	m_int_line = 3;

	m_rom_bank = 0;

	m_pkg_id = 0xfd;
	m_port30 = 0x00;
	m_ar = 0;
	m_dma_enable = false;
}

void pc9801_55_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: move base to device_reset
		// incredibly verbose for LHA-201
		// logerror("map ROM at 0x000dc000-0x000dcfff (bank %d)\n", m_rom_bank);
		m_bus->space(AS_PROGRAM).install_rom(
			0xdc000,
			0xdcfff,
			m_bios->base() + m_rom_bank * 0x1000
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x3fff, *this, &pc9801_55_device::io_map);
	}
}

// required by MS-DOS to actually detect the disk size for format.
// Mimic wd33c9x core here
void pc9801_55_device::increment_addr()
{
	if (machine().side_effects_disabled()) return;

	if (m_ar <= 0x19)
		m_ar ++;
}

void pc9801_55_device::io_map(address_map &map)
{
	map(0x0cc0, 0x0cc0).lrw8(
		NAME([this] (offs_t offset) { return m_wdc->status_r(); }),
		NAME([this] (offs_t offset, u8 data) { m_ar = data; })
	);
	map(0x0cc2, 0x0cc2).lrw8(
		NAME([this] (offs_t offset) {
			const u8 reg = m_ar;
			increment_addr();
			return space(0).read_byte(reg);
		}),
		NAME([this] (offs_t offset, u8 data) {
			space(0).write_byte(m_ar, data);
			increment_addr();
		})
	);
	map(0x0cc4, 0x0cc4).lrw8(
		// -x-- ---- Interrupt by TCI DMA TCO signal
		// ---- --xx DMA channel switch status
		NAME([this] (offs_t offset) {
			logerror("Read Name Status I/O\n");
			return 0;
		}),
		// ---x ---- TCIR reset TC interrupt
		// ---- x--- TCMR reset TC interrupt mask
		// ---- -x-- TCMS set TC interrupt mask
		// ---- --x- DMER reset DMA enable
		// ---- ---x DMES set DMA enable
		// TODO: who wins if both bits of a couple are enabled?
		NAME([this] (offs_t offset, u8 data) {
			logerror("Write Interrupt Control I/O %02x\n", data);
			if (BIT(data, 1))
				m_dma_enable = false;
			if (BIT(data, 0))
				m_dma_enable = true;
			if ((data & 3) == 3)
				popmessage("pc9801_55.cpp: DMA enable undocumented write %02x", data);
		})
	);
}

void pc9801_55_device::internal_map(address_map &map)
{
	map(0x00, 0x1f).rw(m_wdc, FUNC(wd33c9x_base_device::dir_r), FUNC(wd33c9x_base_device::dir_w));

	// xx-- ---- ROM bank
	// ---- x--- MEM1 allow memory access (DMA?)
	// ---- -x-- IRE1 allow interrupts
	// ---- --x- WRS1 SCSI bus RST (1 -> 0)
	map(0x30, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_port30;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if ((m_port30 & 0x3f) != (data & 0x3f))
				logerror("$30 Memory Bank %02x\n", data);

			if (BIT(m_port30, 1) && !(BIT(data, 1)))
			{
				m_wdc->reset_w(1);
				m_wdc->reset_w(0);
			}

			if ((data & 0xc0) != (m_port30 & 0xc0))
			{
				m_rom_bank = data >> 6;
				remap(AS_PROGRAM, 0xc0000, 0xdffff);
			}
			m_port30 = data;
		})
	);

	// 0xfd = External SCSI
	// 0xfe = Built-in SCSI
	map(0x32, 0x32).lrw8(
		NAME([this] (offs_t offset) {
			return m_pkg_id;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("$32 PkgIdRegister %02x\n", data);
			m_pkg_id = data;
		})
	);

	// write <prohibited>, NEC reserved
	// read:
	// x--- ---- SCSI RST signal on SCSI bus (active low, drives low when read)
	// --xx xxxx DSW1 INT and SCSI ID
	map(0x33, 0x33).lr8(
		NAME([this] (offs_t offset) {
			u8 scsi_reset = (m_scsi_bus->ctrl_r() & nscsi_device::S_RST) ? 0x80 : 0;
			if (!machine().side_effects_disabled())
				m_scsi_bus->ctrl_w(7, 0, nscsi_device::S_RST);
			return (m_dsw1->read() & 0x3f) | scsi_reset;
		})
	);
}


