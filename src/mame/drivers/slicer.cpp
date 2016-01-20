// license:BSD-3-Clause
// copyright-holders:Carl

// Slicer Computers Slicer 80186 SBC
// The bios makefile refers to a "exe3bin" utility, this can be substituted with FreeDOS exe2bin and the /l=0xf800 option
// which will fixup the relocations

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/wd_fdc.h"
#include "machine/mc68681.h"
#include "bus/rs232/rs232.h"
#include "bus/isa/isa.h"
#include "bus/scsi/scsi.h"

class slicer_state : public driver_device
{
public:
	slicer_state(const machine_config &mconfig, device_type type, std::string tag) :
	driver_device(mconfig, type, tag),
	m_fdc(*this, "fdc"),
	m_sasi(*this, "sasi")
	{}

	required_device<fd1797_t> m_fdc;
	required_device<SCSI_PORT_DEVICE> m_sasi;

	DECLARE_WRITE8_MEMBER(sio_out_w);
	DECLARE_WRITE8_MEMBER(drive_sel_w);
};

WRITE8_MEMBER(slicer_state::sio_out_w)
{
	floppy_image_device *floppy;
	int state = (data & 0x80) ? 0 : 1;
	char devname[8];

	for(int i = 0; i < 4; i++)
	{
		sprintf(devname, "%d", i);
		floppy = m_fdc->subdevice<floppy_connector>(devname)->get_device();
		if(floppy)
			floppy->mon_w(state);
	}
}

WRITE8_MEMBER(slicer_state::drive_sel_w)
{
	data &= 1;
	switch(offset)
	{
		case 0:
			m_sasi->write_sel(data);
			break;
		case 1:
			m_sasi->write_rst(data);
			break;
		case 7:
			m_fdc->dden_w(data);
			break;

		default:
		{
			floppy_image_device *floppy;
			char devname[8];
			unsigned int drive = 3 - (offset - 2);
			if((drive > 3) || !data)
				break;

			sprintf(devname, "%d", drive);
			floppy = m_fdc->subdevice<floppy_connector>(devname)->get_device();
			m_fdc->set_floppy(floppy);
			break;
		}
	}
}

static ADDRESS_MAP_START( slicer_map, AS_PROGRAM, 16, slicer_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM // fixed 256k for now
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slicer_io, AS_IO, 16, slicer_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_DEVREADWRITE8("fdc", fd1797_t, read, write, 0x00ff) //PCS0
	AM_RANGE(0x0080, 0x00ff) AM_DEVREADWRITE8("sc2681", mc68681_device, read, write, 0x00ff) //PCS1
	AM_RANGE(0x0100, 0x017f) AM_WRITE8(drive_sel_w, 0x00ff) //PCS2
	// TODO: 0x180 sets ack
	AM_RANGE(0x0180, 0x0181) AM_DEVREAD8("sasi_data_in", input_buffer_device, read, 0x00ff) AM_DEVWRITE8("sasi_data_out", output_latch_device, write, 0x00ff) //PCS3
	AM_RANGE(0x0180, 0x0181) AM_DEVREAD8("sasi_ctrl_in", input_buffer_device, read, 0xff00)
	AM_RANGE(0x0184, 0x0185) AM_DEVREAD8("sasi_data_in", input_buffer_device, read, 0x00ff) AM_DEVWRITE8("sasi_data_out", output_latch_device, write, 0x00ff)
	AM_RANGE(0x0184, 0x0185) AM_DEVREAD8("sasi_ctrl_in", input_buffer_device, read, 0xff00)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( slicer_floppies )
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
	SLOT_INTERFACE("8dsdd", FLOPPY_8_DSDD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( slicer, slicer_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(slicer_map)
	MCFG_CPU_IO_MAP(slicer_io)

	MCFG_MC68681_ADD("sc2681", XTAL_3_6864MHz)
	MCFG_MC68681_IRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, int0_w))
	MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE("rs232_1", rs232_port_device, write_txd))
	MCFG_MC68681_B_TX_CALLBACK(DEVWRITELINE("rs232_2", rs232_port_device, write_txd))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(slicer_state, sio_out_w))

	MCFG_RS232_PORT_ADD("rs232_1", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sc2681", mc68681_device, rx_a_w))
	MCFG_RS232_PORT_ADD("rs232_2", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sc2681", mc68681_device, rx_b_w))

	MCFG_FD1797_ADD("fdc", XTAL_16MHz/2/8)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, int1_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq0_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", slicer_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", slicer_floppies, nullptr, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", slicer_floppies, nullptr, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", slicer_floppies, nullptr, floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("sasi", SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("sasi_data_in")
	MCFG_SCSI_BSY_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit3))
	MCFG_SCSI_MSG_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit4))
	MCFG_SCSI_CD_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit5))
	MCFG_SCSI_REQ_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit6))
	MCFG_SCSI_IO_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit7))

	MCFG_SCSI_OUTPUT_LATCH_ADD("sasi_data_out", "sasi")
	MCFG_DEVICE_ADD("sasi_data_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("sasi_ctrl_in", INPUT_BUFFER, 0)
MACHINE_CONFIG_END

ROM_START( slicer )
	ROM_REGION(0x8001, "bios", 0)
	// built from sources, reset.asm adds an extra byte
	ROM_LOAD("epbios.bin", 0x0000, 0x8001, CRC(96fe9dd4) SHA1(5fc43454fe7d51f2ae97aef822155dcd28eb7f23))
ROM_END

COMP( 1983, slicer, 0, 0, slicer, 0, driver_device, 0, "Slicer Computers", "Slicer", MACHINE_NO_SOUND)
