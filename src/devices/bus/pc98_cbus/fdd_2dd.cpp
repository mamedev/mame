// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FDD 2DD bridge for 1st gen HW

TODO:
- support for TMSK / TTRG (sorcer cares)

**************************************************************************************************/

#include "emu.h"
#include "fdd_2dd.h"

#include "formats/img_dsk.h"
#include "formats/pc98_dsk.h"
#include "formats/pc98fdi_dsk.h"
#include "formats/fdd_dsk.h"
#include "formats/dcp_dsk.h"
#include "formats/dip_dsk.h"
#include "formats/nfd_dsk.h"

DEFINE_DEVICE_TYPE(FDD_2DD_BRIDGE, fdd_2dd_bridge_device, "pc98_fdd_2dd", "NEC PC-98 2DD FDD bridge")

fdd_2dd_bridge_device::fdd_2dd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDD_2DD_BRIDGE, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

ROM_START( fdd_2dd )
	ROM_REGION( 0x8000, "bios", ROMREGION_ERASEFF )
	// from an onboard pc9801f
	ROM_LOAD16_BYTE( "urf01-01.bin", 0x00000, 0x4000, CRC(2f5ae147) SHA1(69eb264d520a8fc826310b4fce3c8323867520ee) )
	ROM_LOAD16_BYTE( "urf02-01.bin", 0x00001, 0x4000, CRC(62a86928) SHA1(4160a6db096dbeff18e50cbee98f5d5c1a29e2d1) )
ROM_END

const tiny_rom_entry *fdd_2dd_bridge_device::device_rom_region() const
{
	return ROM_NAME( fdd_2dd );
}

static void drives_2dd(device_slot_interface &device)
{
	device.option_add("525dd", TEAC_FD_55F);
}

static void floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PC98_FORMAT);
	fr.add(FLOPPY_PC98FDI_FORMAT);
	fr.add(FLOPPY_FDD_FORMAT);
	fr.add(FLOPPY_DCP_FORMAT);
	fr.add(FLOPPY_DIP_FORMAT);
	fr.add(FLOPPY_NFD_FORMAT);
	// *nix/FreeBSD may distribute with this
	fr.add(FLOPPY_IMG_FORMAT);
}

void fdd_2dd_bridge_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, 8'000'000, false, true);
	m_fdc->intrq_wr_callback().set([this] (int state) {
		//if (BIT(m_ctrl, 2))
		m_bus->int_w(7, state);
	});
	m_fdc->drq_wr_callback().set([this] (int state) { m_bus->drq_w(3, state); });
	FLOPPY_CONNECTOR(config, "fdc:0", drives_2dd, "525dd", floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", drives_2dd, "525dd", floppy_formats);
}


void fdd_2dd_bridge_device::device_start()
{
	m_bus->set_dma_channel(3, this, true);

	m_fdc->set_rate(250000);
	// TODO: set_rpm?

	save_item(NAME(m_ctrl));
}

void fdd_2dd_bridge_device::device_reset()
{
	m_ctrl = 0;
}

void fdd_2dd_bridge_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: any option to disconnect the ROM?
		logerror("map ROM at 0xd6000-0xd6fff\n");
		m_bus->space(AS_PROGRAM).install_rom(
			0xd6000,
			0xd6fff,
			m_bios->base()
		);

	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &fdd_2dd_bridge_device::io_map);
	}
}

void fdd_2dd_bridge_device::io_map(address_map &map)
{
	map(0x00c8, 0x00cb).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x00cc, 0x00cc).rw(FUNC(fdd_2dd_bridge_device::ctrl_r), FUNC(fdd_2dd_bridge_device::ctrl_w));
}

u8 fdd_2dd_bridge_device::dack_r(int line)
{
	u8 res = m_fdc->dma_r();
	return res;
}

void fdd_2dd_bridge_device::dack_w(int line, u8 data)
{
	m_fdc->dma_w(data);
}

void fdd_2dd_bridge_device::eop_w(int state)
{
	m_fdc->tc_w(state);
}

bool fdd_2dd_bridge_device::fdc_drive_ready_r(upd765a_device *fdc)
{
	floppy_image_device *floppy0 = fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = fdc->subdevice<floppy_connector>("1")->get_device();

	return (!floppy0->ready_r() || !floppy1->ready_r());
}

u8 fdd_2dd_bridge_device::ctrl_r()
{
	u8 ret = 0;

	// 2dd BIOS specifically tests if a disk is in any drive
	// (does not happen on 2HD standalone)
	ret |= fdc_drive_ready_r(m_fdc) << 4;

	//popmessage("%d %d %02x", floppy0->ready_r(), floppy1->ready_r(), ret);

	// TODO: dips et al.
	return ret | 0x40;
}

void fdd_2dd_bridge_device::ctrl_w(u8 data)
{
	logerror("%02x ctrl\n",data);
	m_fdc->reset_w(BIT(data, 7));

	m_ctrl = data;
	m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
	m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
}


