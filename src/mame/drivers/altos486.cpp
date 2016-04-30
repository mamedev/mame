// License: BSD-3-Clause
// copyright-holders:Carl

// Altos 486, very incomplete

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"

class altos486_state : public driver_device
{
public:
	altos486_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "main_ram"),
		m_rom(*this, "bios")
	{ }

	required_device<i80186_cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_ram;
	required_memory_region m_rom;

	DECLARE_READ8_MEMBER(read_rmx_ack);

	DECLARE_READ16_MEMBER(mmu_ram_r);
	DECLARE_READ16_MEMBER(mmu_io_r);
	DECLARE_WRITE16_MEMBER(mmu_ram_w);
	DECLARE_WRITE16_MEMBER(mmu_io_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	bool m_sys_mode;
	UINT8 m_prot[256];
	UINT16 m_viol[16];
};

READ8_MEMBER(altos486_state::read_rmx_ack)
{
	if(offset == 4)
		return m_maincpu->int_callback(*this, 0);

	return 0;
}

READ16_MEMBER(altos486_state::mmu_ram_r)
{
	if(offset < 0x7e000)
		return m_ram[offset]; // TODO
	else
		return m_rom->u16(offset - 0x7e000);
}

READ16_MEMBER(altos486_state::mmu_io_r)
{
	if(!m_sys_mode)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		return 0;
	}
	if(offset < 0x100)
		return m_prot[offset];
	return 0; // TODO
}

WRITE16_MEMBER(altos486_state::mmu_ram_w)
{
	//UINT16 entry = m_prot[offset >> 11];
	//if(!m_sys_mode)
}

WRITE16_MEMBER(altos486_state::mmu_io_w)
{
	if(!m_sys_mode)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		return;
	}
	if(offset < 0x100)
	{
		if(mem_mask != 0xff00)
			m_prot[offset] = data & 0xf;
	}
}

FLOPPY_FORMATS_MEMBER( altos486_state::floppy_formats )
	FLOPPY_TD0_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( altos486_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START(altos486_mem, AS_PROGRAM, 16, altos486_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(mmu_ram_r, mmu_ram_w) AM_SHARE("main_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos486_io, AS_IO, 16, altos486_state)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mmu_io_r, mmu_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos486_z80_mem, AS_PROGRAM, 8, altos486_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("iocpu", 0)
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	//AM_RANGE(0x8000, 0xffff) AM_READWRITE(z80_shared_r, z80_shared_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos486_z80_io, AS_IO, 8, altos486_state)
	//AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("sio0", z80sio0_device, read, write)
	//AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("sio1", z80sio0_device, read, write)
	//AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("sio2", z80sio0_device, read, write)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( altos486, altos486_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(altos486_mem)
	MCFG_CPU_IO_MAP(altos486_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb) // yes, really

	MCFG_CPU_ADD("iocpu", Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(altos486_z80_mem)
	MCFG_CPU_IO_MAP(altos486_z80_io)

	MCFG_PIC8259_ADD("pic8259", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, READ8(altos486_state, read_rmx_ack))
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)

	MCFG_UPD765A_ADD("fdc", false, false)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", altos486_floppies, "525qd", altos486_state::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_Z80SIO0_ADD("sio0", 4000000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(altos486_state, sio_interrupt))
	MCFG_Z80SIO0_ADD("sio1", 4000000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232c", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232c", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232c", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("rs232d", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("rs232d", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE("rs232d", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(altos486_state, sio_interrupt))
	MCFG_Z80SIO0_ADD("sio2", 4000000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232_lp", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232_lp", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232_lp", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(altos486_state, sio_interrupt))

	MCFG_I8274_ADD("i8274", XTAL_16MHz/4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs422_wn", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs422_wn", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs422_wn", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(altos486_state, sio_interrupt))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio0", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio0", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio0", z80dart_device, ctsa_w))
	//MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", altos486_terminal)

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio0", z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio0", z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio0", z80dart_device, ctsb_w))

	MCFG_RS232_PORT_ADD("rs232c", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1", z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD("rs232d", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1", z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1", z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1", z80dart_device, ctsb_w))

	MCFG_RS232_PORT_ADD("rs232_lp", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio2", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio2", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio2", z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD("rs422_wn", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8274", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("i8274", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("i8274", z80dart_device, ctsa_w))

	MCFG_DEVICE_ADD("pit0", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_22_1184MHz/18)
	MCFG_PIT8253_CLK1(XTAL_22_1184MHz/144)
	MCFG_PIT8253_CLK2(XTAL_22_1184MHz/18)
	MCFG_DEVICE_ADD("pit1", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_22_1184MHz/18)
	MCFG_PIT8253_CLK1(XTAL_22_1184MHz/144)
	MCFG_PIT8253_CLK2(XTAL_22_1184MHz/18)
MACHINE_CONFIG_END


ROM_START( altos486 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v11", "Altos 486 v1.1")
	ROMX_LOAD("16577_lo_v1.1.bin",   0x0000, 0x2000, CRC(65a9db18) SHA1(3ac2b87f1fc0b28ed4907c9b4091aaa170609674), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("16576_hi_v1.1.bin",   0x0001, 0x2000, CRC(cea4cd8d) SHA1(f9f49828bd5e3281bd7cc34d4460ca1b677530b0), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v12", "Altos 486 v1.2")
	ROMX_LOAD("16577-003_4D_v1.2.bin",   0x0000, 0x2000, CRC(e2ac806b) SHA1(9b358246e26b3e85a6dff418899a180370884537), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("16576-003_3D_v1.2.bin",   0x0001, 0x2000, CRC(912f4c12) SHA1(df5088e8610513b577926b0c752e3b54bc880167), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION( 0x1000, "iocpu", 0 )
	ROM_LOAD("16019_z80.bin", 0x0000, 0x1000, CRC(68b1b2e1) SHA1(5d83609a465029212d5e3f72ac9c520b3dbed838))
ROM_END

COMP( 1984, altos486, 0, 0, altos486, 0, driver_device, 0, "Altos Computer Systems", "Altos 486",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

