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

	void altos486(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(read_rmx_ack);

	DECLARE_READ16_MEMBER(mmu_ram_r);
	DECLARE_READ16_MEMBER(mmu_io_r);
	DECLARE_WRITE16_MEMBER(mmu_ram_w);
	DECLARE_WRITE16_MEMBER(mmu_io_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void altos486_io(address_map &map);
	void altos486_mem(address_map &map);
	void altos486_z80_io(address_map &map);
	void altos486_z80_mem(address_map &map);

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;
	required_memory_region m_rom;

	bool m_sys_mode;
	uint8_t m_prot[256];
};

READ8_MEMBER(altos486_state::read_rmx_ack)
{
	if(offset == 4)
		return m_maincpu->int_callback(*this, 0);

	return 0;
}

READ16_MEMBER(altos486_state::mmu_ram_r)
{
	if (offset < 0x7e000)
		return m_ram[offset]; // TODO
	else
		return m_rom->as_u16(offset - 0x7e000);
}

READ16_MEMBER(altos486_state::mmu_io_r)
{
	if (!m_sys_mode)
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
	//uint16_t entry = m_prot[offset >> 11];
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

static void altos486_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void altos486_state::altos486_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xfffff).rw(this, FUNC(altos486_state::mmu_ram_r), FUNC(altos486_state::mmu_ram_w)).share("main_ram");
}

void altos486_state::altos486_io(address_map &map)
{
	map(0x0000, 0xffff).rw(this, FUNC(altos486_state::mmu_io_r), FUNC(altos486_state::mmu_io_w));
}

void altos486_state::altos486_z80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("iocpu", 0);
	map(0x2000, 0x27ff).ram();
	//AM_RANGE(0x8000, 0xffff) AM_READWRITE(z80_shared_r, z80_shared_w)
}

void altos486_state::altos486_z80_io(address_map &map)
{
	//AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("sio0", z80sio0_device, read, write)
	//AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("sio1", z80sio0_device, read, write)
	//AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("sio2", z80sio0_device, read, write)
}

MACHINE_CONFIG_START(altos486_state::altos486)
	MCFG_DEVICE_ADD("maincpu", I80186, XTAL(8'000'000))
	MCFG_DEVICE_PROGRAM_MAP(altos486_mem)
	MCFG_DEVICE_IO_MAP(altos486_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb) // yes, really

	MCFG_DEVICE_ADD("iocpu", Z80, XTAL(8'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(altos486_z80_mem)
	MCFG_DEVICE_IO_MAP(altos486_z80_io)

	MCFG_DEVICE_ADD("pic8259", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(WRITELINE("maincpu", i80186_cpu_device, int0_w))
	MCFG_PIC8259_IN_SP_CB(VCC)
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(*this, altos486_state, read_rmx_ack))

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)

	MCFG_UPD765A_ADD("fdc", false, false)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", altos486_floppies, "525qd", altos486_state::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_DEVICE_ADD("sio0", Z80SIO0, 4000000)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE("rs232a", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE("rs232a", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE("rs232b", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE("rs232b", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(*this, altos486_state, sio_interrupt))

	MCFG_DEVICE_ADD("sio1", Z80SIO0, 4000000)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE("rs232c", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE("rs232c", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE("rs232c", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE("rs232d", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE("rs232d", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE("rs232d", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(*this, altos486_state, sio_interrupt))

	MCFG_DEVICE_ADD("sio2", Z80SIO0, 4000000)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE("rs232_lp", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE("rs232_lp", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE("rs232_lp", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(*this, altos486_state, sio_interrupt))

	MCFG_DEVICE_ADD("i8274", I8274, XTAL(16'000'000)/4)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE("rs422_wn", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE("rs422_wn", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE("rs422_wn", rs232_port_device, write_rts))
	//MCFG_Z80DART_OUT_INT_CB(WRITELINE(*this, altos486_state, sio_interrupt))

	MCFG_DEVICE_ADD("rs232a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio0", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio0", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio0", z80dart_device, ctsa_w))
	//MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", altos486_terminal)

	MCFG_DEVICE_ADD("rs232b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio0", z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio0", z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio0", z80dart_device, ctsb_w))

	MCFG_DEVICE_ADD("rs232c", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80dart_device, ctsa_w))

	MCFG_DEVICE_ADD("rs232d", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80dart_device, ctsb_w))

	MCFG_DEVICE_ADD("rs232_lp", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio2", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio2", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio2", z80dart_device, ctsa_w))

	MCFG_DEVICE_ADD("rs422_wn", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("i8274", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("i8274", z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("i8274", z80dart_device, ctsa_w))

	MCFG_DEVICE_ADD("pit0", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(22'118'400)/18)
	MCFG_PIT8253_CLK1(XTAL(22'118'400)/144)
	MCFG_PIT8253_CLK2(XTAL(22'118'400)/18)
	MCFG_DEVICE_ADD("pit1", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(22'118'400)/18)
	MCFG_PIT8253_CLK1(XTAL(22'118'400)/144)
	MCFG_PIT8253_CLK2(XTAL(22'118'400)/18)
MACHINE_CONFIG_END


ROM_START( altos486 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v11", "Altos 486 v1.1")
	ROMX_LOAD("16577_lo_v1.1.bin",   0x0000, 0x2000, CRC(65a9db18) SHA1(3ac2b87f1fc0b28ed4907c9b4091aaa170609674), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("16576_hi_v1.1.bin",   0x0001, 0x2000, CRC(cea4cd8d) SHA1(f9f49828bd5e3281bd7cc34d4460ca1b677530b0), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v12", "Altos 486 v1.2")
	ROMX_LOAD("16577-003_4d_v1.2.bin",   0x0000, 0x2000, CRC(e2ac806b) SHA1(9b358246e26b3e85a6dff418899a180370884537), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("16576-003_3d_v1.2.bin",   0x0001, 0x2000, CRC(912f4c12) SHA1(df5088e8610513b577926b0c752e3b54bc880167), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION( 0x1000, "iocpu", 0 )
	ROM_LOAD("16019_z80.bin", 0x0000, 0x1000, CRC(68b1b2e1) SHA1(5d83609a465029212d5e3f72ac9c520b3dbed838))
ROM_END

COMP( 1984, altos486, 0, 0, altos486, 0, altos486_state, empty_init, "Altos Computer Systems", "Altos 486", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
