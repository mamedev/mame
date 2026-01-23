// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-07/-27 SASI interface

Original -07 is for 1st gen HW (clock from C-Bus like -26?)

TODO:
- Doesn't work for original pc9801f (wants -07 differences? Separate BIOS?);
- Hangs after msdos33 install for pc9801m2 after issuing a 08 00 00 00 02 00;
- pc88va2 doesn't do anything meaningful in dtc510 core (uses SC_ASSIGN_ALT_TRACK,
  for actual PC Engine OS differences?);
- configurable DRQ channel for built-in versions;
- BIOS subscribes two irqs, the other one is INT1 that manipulates DMA channel then
  reads from CMT ports;

NOTES:
- 40MB -> -chs 615,8,17
  https://stason.org/TULARC/pc/hard-drives-hdd/nec/D3146-40MB-3-5-HH-MFM-ST506.html
- 20MB -> -chs 615,4,17
  https://stason.org/TULARC/pc/hard-drives-hdd/nec/D5126-20MB-5-25-HH-MFM-ST506.html
  -chs 310,8,17
  https://stason.org/TULARC/pc/hard-drives-hdd/nec/D5244-20MB-5-25-FH-MFM-ST506.html
- 10MB -> -chs 309,4,17
  https://stason.org/TULARC/pc/hard-drives-hdd/nec/D5124-10MB-5-25-FH-MFM-ST506.html
- 5MB -> -chs 310,2,17
  https://stason.org/TULARC/pc/hard-drives-hdd/nec/D5104-5MB-5-25-HH-MFM-ST506.html

**************************************************************************************************/

#include "emu.h"
#include "pc9801_27.h"

#define LOG_STATE (1U << 1) // SASI state flags

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGSTATE(...)    LOGMASKED(LOG_STATE, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PC9801_27, pc9801_27_device, "pc9801_27", "NEC PC-9801-27 SASI interface")

pc9801_27_device::pc9801_27_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_27, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_sasibus(*this, "sasi")
//  , m_harddisk(*this, "sasi:0:harddisk")
	, m_sasi(*this, "sasi:7:sasicb")
	, m_bios(*this, "bios")
	, m_dsw(*this, "DSW")
{
}

static void sasi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_DTC510);
}


void pc9801_27_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_sasibus);
	NSCSI_CONNECTOR(config, "sasi:0", sasi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "sasi:1", sasi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "sasicb", true)
		.option_add_internal("sasicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(pc9801_27_device::sasi_req_w));
			downcast<nscsi_callback_device&>(*device).cd_callback().set(*this, FUNC(pc9801_27_device::sasi_cd_w));
			downcast<nscsi_callback_device&>(*device).io_callback().set(*this, FUNC(pc9801_27_device::sasi_io_w));
			downcast<nscsi_callback_device&>(*device).msg_callback().set(*this, FUNC(pc9801_27_device::sasi_msg_w));
			downcast<nscsi_callback_device&>(*device).ack_callback().set(*this, FUNC(pc9801_27_device::sasi_ack_w));
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(pc9801_27_device::sasi_bsy_w));
		});
}

ROM_START( pc9801_27 )
	ROM_REGION( 0x1000, "bios", ROMREGION_ERASEFF )
	// from CSCP package
	ROM_LOAD("sasi.rom", 0x0000, 0x1000, CRC(9652011b) SHA1(b607707d74b5a7d3ba211825de31a8f32aec8146) )
ROM_END

const tiny_rom_entry *pc9801_27_device::device_rom_region() const
{
	return ROM_NAME( pc9801_27 );
}

static INPUT_PORTS_START( pc9801_27 )
/*
 * read drive info NRDSW=0
 *
 * x--- ---- CT0 HDD #1 sector length (1=512, 0=256)
 * -x-- ---- CT1 HDD #2 sector length
 * --xx x--- DT02-DT01-DT00 HDD #1 capacity
 * --11 1--- <unconnected>
 * --11 0--- 40MB
 * --10 0--- 20MB
 * --00 1--- 10MB
 * --00 0--- 5MB
 * ---- -xxx DT12-DT11-DT10 HDD #2 capacity
 *
 * The HDD capacity is testable thru pc88va2:pceva2tb HDFORM tool.
 * Omitted settings just routes to unconnected again.
 *
 */
	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, "DT1x SASI-2 HDD capacity" )    PORT_DIPLOCATION("SW:6,7,8")
	PORT_DIPSETTING(    0x07, "unconnected" )
	PORT_DIPSETTING(    0x06, "40 MB" )
	PORT_DIPSETTING(    0x04, "20 MB" )
	PORT_DIPSETTING(    0x01, "10 MB" )
	PORT_DIPSETTING(    0x00, "5 MB" )
	PORT_DIPNAME( 0x38, 0x30, "DT0x SASI-1 HDD capacity" )    PORT_DIPLOCATION("SW:3,4,5")
	PORT_DIPSETTING(    0x38, "unconnected" )
	PORT_DIPSETTING(    0x30, "40 MB" )
	PORT_DIPSETTING(    0x20, "20 MB" )
	PORT_DIPSETTING(    0x08, "10 MB" )
	PORT_DIPSETTING(    0x00, "5 MB" )
	PORT_DIPNAME( 0x40, 0x40, "CT1 SASI-2 sector length" )    PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x40, "512 bytes" )
	PORT_DIPSETTING(    0x00, "256 bytes" )
	PORT_DIPNAME( 0x80, 0x80, "CT0 SASI-1 sector length" )    PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x80, "512 bytes" )
	PORT_DIPSETTING(    0x00, "256 bytes" )
INPUT_PORTS_END

ioport_constructor pc9801_27_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_27 );
}

void pc9801_27_device::device_start()
{
	m_bus->set_dma_channel(0, this, true);

	save_item(NAME(m_control));
	save_item(NAME(m_sasi_ack));
	save_item(NAME(m_sasi_req));
	save_item(NAME(m_sasi_msg));
	save_item(NAME(m_sasi_io));
	save_item(NAME(m_sasi_cd));
}

void pc9801_27_device::device_reset()
{
	m_control = 0;
	m_sasi_ack = m_sasi_req = m_sasi_msg = m_sasi_io = m_sasi_cd = 0;
}

void pc9801_27_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		const u32 start_address = 0xd7000;
		const u32 end_address = start_address + 0xfff;
		logerror("map ROM at 0x%08x-0x%08x\n", start_address, end_address);
		m_bus->space(AS_PROGRAM).install_rom(
			start_address,
			end_address,
			m_bios->base()
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &pc9801_27_device::io_map);
	}
}



void pc9801_27_device::io_map(address_map &map)
{
	map(0x0080, 0x0080).rw(FUNC(pc9801_27_device::data_r), FUNC(pc9801_27_device::data_w));

/*
 * read status when NRDSW=1
 * x--- ---- REQ
 * -x-- ---- ACK
 * --x- ---- BSY
 * ---x ---- MSG
 * ---- x--- CD
 * ---- -x-- IO
 * ---- --x- DMAE
 * ---- ---x INT
 *
 */
	map(0x0082, 0x0082).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = 0;
			if (BIT(m_control, 6))
			{
				res = (
					m_sasi->req_r() << 7 |
					//m_sasi->ack_r() << 6 |
					m_sasi->bsy_r() << 5 |
					m_sasi->msg_r() << 4 |
					m_sasi->cd_r() << 3 |
					m_sasi->io_r() << 2 |
					// DMAE should be write only
					m_irq_state << 0
				);
			}
			else
			{
				res = m_dsw->read();
			}
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(m_control, 3) && !BIT(data, 3))
				m_sasibus->reset();

			//m_sasi->sel_w(BIT(data, 5));
			if (!BIT(m_control, 5) && BIT(data, 5))
			{
				m_sasi->sel_w(1);
			}
			else if (BIT(m_control, 5) && !BIT(data, 5))
			{
				m_sasi->sel_w(0);
			}

			if (BIT(m_control, 1) != BIT(data, 1))
				update_drq();

			if (BIT(m_control, 0) != BIT(data, 0))
				update_irq();

			m_control = data;
		})
	);
}

u8 pc9801_27_device::data_r()
{
	u8 res = m_sasi->read();

	if (!machine().side_effects_disabled() && m_sasi_req)
	{
		m_sasi->ack_w(1);
	}

	return res;
}

void pc9801_27_device::data_w(u8 data)
{
	m_sasi_data = data;

	if (!m_sasi_io)
	{
		m_sasi->write(data);
		if (m_sasi_req)
			m_sasi->ack_w(1);
	}
}

u8 pc9801_27_device::dack_r(int line)
{
	u8 res = data_r();
	return res;
}

void pc9801_27_device::dack_w(int line, u8 data)
{
	data_w(data);
}

void pc9801_27_device::eop_w(int state)
{
	if (state)
	{
		m_control &= 0xfd;
		update_drq();
	}
}

void pc9801_27_device::sasi_req_w(int state)
{
	LOGSTATE("REQ %d -> %d\n", m_sasi_req, state);
	if (!state)
	{
		m_sasi->ack_w(0);
	}
	if (m_sasi_req != state)
	{
		update_irq();
		update_drq();
	}

	m_sasi_req = state;
}

void pc9801_27_device::sasi_cd_w(int state)
{
	LOGSTATE("CD %d -> %d\n", m_sasi_cd, state);
	if (m_sasi_cd != state)
	{
		update_irq();
		update_drq();
	}

	m_sasi_cd = state;
}

void pc9801_27_device::sasi_io_w(int state)
{
	LOGSTATE("IO %d -> %d\n", m_sasi_io, state);
	if (m_sasi_io != state)
		update_irq();
	m_sasi_io = state;
	m_sasi->write(state ? 0 : m_sasi_data);
}

void pc9801_27_device::sasi_msg_w(int state)
{
	LOGSTATE("MSG %d -> %d\n", m_sasi_msg, state);
	if (m_sasi_msg != state)
		update_irq();

	m_sasi_msg = state;
}

void pc9801_27_device::sasi_ack_w(int state)
{
	LOGSTATE("ACK %d -> %d\n", m_sasi_ack, state);
	m_sasi_ack = state;
}

void pc9801_27_device::sasi_bsy_w(int state)
{
	if (state)
		m_sasi->sel_w(0);
}

void pc9801_27_device::update_irq()
{
//  printf("%d %d %d %d\n", m_sasi_req, m_sasi_msg, m_sasi_cd, m_sasi_io);
	//if (m_sasi_req && !m_sasi_msg && m_sasi_cd && m_sasi_io)
	if (m_sasi->req_r() && !m_sasi->msg_r() && m_sasi->cd_r() && m_sasi->io_r())
	{
		m_irq_state = BIT(m_control, 0);
	}
	else
	{
		m_irq_state = 0;
	}
	m_bus->int_w(3, m_irq_state);
}

void pc9801_27_device::update_drq()
{
//  m_bus->drq_w(0, !(m_sasi_req && !(m_sasi_cd) && BIT(m_control, 1)));
	m_bus->drq_w(0, m_sasi->req_r() && !m_sasi->cd_r() && BIT(m_control, 1));
}
