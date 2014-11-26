// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    K230 Internal 8088 module without interrupt controller
    K231 External 8088 module without interrupt controller
    K234 External 68008 module
    K235 Internal 8088 module with interrupt controller

***************************************************************************/

#include "emu.h"
#include "k230.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( dmv_k230 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD( "dmv_int_8088_32167.bin", 0x0000, 0x1000, CRC(f4a58880) SHA1(4f50ef25008851ae6f0c670f19d63f4e61249581))
ROM_END

ROM_START( dmv_k231 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD( "dmv_ext_8088_32167.bin", 0x0000, 0x1000, CRC(f4a58880) SHA1(4f50ef25008851ae6f0c670f19d63f4e61249581))
ROM_END

ROM_START( dmv_k235 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD( "dmv_int_8088_pic_33473.bin", 0x0000, 0x1000, CRC(104195dc) SHA1(08d48ca3b84ab26c1a764792e04ec4def7dad2ad))
ROM_END

static ADDRESS_MAP_START(k230_mem, AS_PROGRAM, 8, dmv_k230_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00000, 0x7ffff ) AM_READWRITE(program_r, program_w)
	AM_RANGE( 0x80000, 0xfffff ) AM_READ(rom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(k230_io, AS_IO, 8, dmv_k230_device)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0xff ) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(k234_mem, AS_PROGRAM, 8, dmv_k230_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00000, 0x7ffff ) AM_READWRITE(program_r, program_w)
	AM_RANGE( 0xfff00, 0xfffff ) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(k235_io, AS_IO, 8, dmv_k230_device)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x90, 0x91 ) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE( 0x00, 0xff ) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( dmv_k230 )
	MCFG_CPU_ADD("maincpu", I8088, XTAL_15MHz / 3)
	MCFG_CPU_PROGRAM_MAP(k230_mem)
	MCFG_CPU_IO_MAP(k230_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( dmv_k234 )
	MCFG_CPU_ADD("maincpu", M68008, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(k234_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( dmv_k235 )
	MCFG_CPU_ADD("maincpu", V20, XTAL_15MHz / 3)
	MCFG_CPU_PROGRAM_MAP(k230_mem)
	MCFG_CPU_IO_MAP(k235_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NULL)
MACHINE_CONFIG_END


static INPUT_PORTS_START( dmv_k235 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "K235 INT7" )  PORT_DIPLOCATION("S:1")
	PORT_DIPSETTING( 0x00, "Slot 5" )
	PORT_DIPSETTING( 0x01, "Slot 6" )
	PORT_DIPNAME( 0x02, 0x00, "K235 INT5" )  PORT_DIPLOCATION("S:2")
	PORT_DIPSETTING( 0x00, "Slot 2a" )
	PORT_DIPSETTING( 0x02, "Slot 2" )
INPUT_PORTS_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DMV_K230 = &device_creator<dmv_k230_device>;
const device_type DMV_K231 = &device_creator<dmv_k231_device>;
const device_type DMV_K234 = &device_creator<dmv_k234_device>;
const device_type DMV_K235 = &device_creator<dmv_k235_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k230_device - constructor
//-------------------------------------------------

dmv_k230_device::dmv_k230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, DMV_K230, "K230 8088 without interrupt controller", tag, owner, clock, "dmv_k230", __FILE__),
		device_dmvslot_interface( mconfig, *this ),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "rom")
{
}

dmv_k230_device::dmv_k230_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_dmvslot_interface( mconfig, *this ),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "rom")
{
}

//-------------------------------------------------
//  dmv_k231_device - constructor
//-------------------------------------------------

dmv_k231_device::dmv_k231_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k230_device(mconfig, DMV_K231, "K231 8088 without interrupt controller", tag, owner, clock, "dmv_k231", __FILE__)
{
}

//-------------------------------------------------
//  dmv_k234_device - constructor
//-------------------------------------------------

dmv_k234_device::dmv_k234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k230_device(mconfig, DMV_K234, "K234 68008", tag, owner, clock, "dmv_k234", __FILE__)
{
}

//-------------------------------------------------
//  dmv_k235_device - constructor
//-------------------------------------------------

dmv_k235_device::dmv_k235_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k230_device(mconfig, DMV_K235, "K235 8088 with interrupt controller", tag, owner, clock, "dmv_k235", __FILE__),
		m_pic(*this, "pic8259"),
		m_dsw(*this, "DSW")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k230_device::device_start()
{
	m_bus = static_cast<dmvcart_slot_device*>(owner());
	m_io = &machine().device<cpu_device>("maincpu")->space(AS_IO);
}

void dmv_k234_device::device_start()
{
	dmv_k230_device::device_start();
	m_io->install_readwrite_handler(0xd8, 0xdf, 0, 0, read8_delegate(FUNC(dmv_k234_device::snr_r), this), write8_delegate(FUNC(dmv_k234_device::snr_w), this), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k230_device::device_reset()
{
	m_switch16 = 0;
	m_hold = 0;
}

void dmv_k234_device::device_reset()
{
	dmv_k230_device::device_reset();
	m_snr = 0;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dmv_k230_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k230 );
}

machine_config_constructor dmv_k234_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k234 );
}

machine_config_constructor dmv_k235_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k235 );
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *dmv_k230_device::device_rom_region() const
{
	return ROM_NAME( dmv_k230 );
}

const rom_entry *dmv_k231_device::device_rom_region() const
{
	return ROM_NAME( dmv_k231 );
}

const rom_entry *dmv_k234_device::device_rom_region() const
{
	return NULL;
}

const rom_entry *dmv_k235_device::device_rom_region() const
{
	return ROM_NAME( dmv_k235 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k235_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k235 );
}

bool dmv_k230_device::av16bit()
{
	return true;
}

void dmv_k230_device::hold_w(int state)
{
	m_hold = state;
	m_maincpu->set_input_line(INPUT_LINE_HALT, (m_hold || !m_switch16) ? ASSERT_LINE : CLEAR_LINE);
}

void dmv_k230_device::switch16_w(int state)
{
	m_switch16 = state;
	m_maincpu->set_input_line(INPUT_LINE_HALT, (m_hold || !m_switch16) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(dmv_k230_device::rom_r)
{
	return m_rom->base()[offset & 0x0fff];
}

READ8_MEMBER( dmv_k230_device::io_r )
{
	return m_io->read_byte(offset);
}

WRITE8_MEMBER( dmv_k230_device::io_w )
{
	m_io->write_byte(offset, data);
}

READ8_MEMBER( dmv_k230_device::program_r )
{
	return m_bus->m_prog_read_cb(space, offset);
}

WRITE8_MEMBER( dmv_k230_device::program_w )
{
	m_bus->m_prog_write_cb(space, offset, data);
}

void dmv_k234_device::hold_w(int state)
{
	m_hold = state;
	m_maincpu->set_input_line(INPUT_LINE_HALT, (m_hold || !m_snr) ? ASSERT_LINE : CLEAR_LINE);
}

void dmv_k234_device::switch16_w(int state)
{
	if (m_switch16 != state)
	{
		m_snr = CLEAR_LINE;
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_bus->m_out_thold_cb(CLEAR_LINE);
		m_switch16 = state;
	}
}

READ8_MEMBER( dmv_k234_device::snr_r )
{
	m_snr = ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_maincpu->reset();
	m_bus->m_out_thold_cb(ASSERT_LINE);

	return 0xff;
}

WRITE8_MEMBER( dmv_k234_device::snr_w )
{
	m_snr = ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_maincpu->reset();
	m_bus->m_out_thold_cb(ASSERT_LINE);
}
