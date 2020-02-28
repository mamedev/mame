// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
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

void dmv_k230_device::k230_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).rw(FUNC(dmv_k230_device::program_r), FUNC(dmv_k230_device::program_w));
	map(0x80000, 0xfffff).r(FUNC(dmv_k230_device::rom_r));
}

void dmv_k230_device::k230_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(dmv_k230_device::io_r), FUNC(dmv_k230_device::io_w));
}

void dmv_k234_device::k234_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).rw(FUNC(dmv_k234_device::program_r), FUNC(dmv_k234_device::program_w));
	map(0xfff00, 0xfffff).rw(FUNC(dmv_k234_device::io_r), FUNC(dmv_k234_device::io_w));
}

void dmv_k235_device::k235_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(dmv_k235_device::io_r), FUNC(dmv_k235_device::io_w));
	map(0x90, 0x91).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
}

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

DEFINE_DEVICE_TYPE(DMV_K230, dmv_k230_device, "dmv_k230", "K230 8088 without interrupt controller")
DEFINE_DEVICE_TYPE(DMV_K231, dmv_k231_device, "dmv_k231", "K231 8088 without interrupt controller")
DEFINE_DEVICE_TYPE(DMV_K234, dmv_k234_device, "dmv_k234", "K234 68008")
DEFINE_DEVICE_TYPE(DMV_K235, dmv_k235_device, "dmv_k235", "K235 8088 with interrupt controller")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k230_device - constructor
//-------------------------------------------------

dmv_k230_device::dmv_k230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k230_device(mconfig, DMV_K230, tag, owner, clock)
{
}

dmv_k230_device::dmv_k230_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_dmvslot_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_rom(*this, "rom")
	, m_switch16(0)
	, m_hold(0)
{
}

//-------------------------------------------------
//  dmv_k231_device - constructor
//-------------------------------------------------

dmv_k231_device::dmv_k231_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k230_device(mconfig, DMV_K231, tag, owner, clock)
{
}

//-------------------------------------------------
//  dmv_k234_device - constructor
//-------------------------------------------------

dmv_k234_device::dmv_k234_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k230_device(mconfig, DMV_K234, tag, owner, clock), m_snr(0)
{
}

//-------------------------------------------------
//  dmv_k235_device - constructor
//-------------------------------------------------

dmv_k235_device::dmv_k235_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k230_device(mconfig, DMV_K235, tag, owner, clock), m_pic(*this, "pic8259"), m_dsw(*this, "DSW")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k230_device::device_start()
{
	// register for state saving
	save_item(NAME(m_switch16));
	save_item(NAME(m_hold));
}

void dmv_k234_device::device_start()
{
	dmv_k230_device::device_start();
	iospace().install_readwrite_handler(0xd8, 0xdf, read8_delegate(*this, FUNC(dmv_k234_device::snr_r)), write8_delegate(*this, FUNC(dmv_k234_device::snr_w)), 0);

	// register for state saving
	save_item(NAME(m_snr));
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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmv_k230_device::device_add_mconfig(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmv_k230_device::k230_mem);
	m_maincpu->set_addrmap(AS_IO, &dmv_k230_device::k230_io);
}

void dmv_k234_device::device_add_mconfig(machine_config &config)
{
	M68008(config, m_maincpu, XTAL(16'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmv_k234_device::k234_mem);
}

void dmv_k235_device::device_add_mconfig(machine_config &config)
{
	V20(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmv_k235_device::k230_mem);
	m_maincpu->set_addrmap(AS_IO, &dmv_k235_device::k235_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *dmv_k230_device::device_rom_region() const
{
	return ROM_NAME( dmv_k230 );
}

const tiny_rom_entry *dmv_k231_device::device_rom_region() const
{
	return ROM_NAME( dmv_k231 );
}

const tiny_rom_entry *dmv_k234_device::device_rom_region() const
{
	return nullptr;
}

const tiny_rom_entry *dmv_k235_device::device_rom_region() const
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
	return iospace().read_byte(offset);
}

WRITE8_MEMBER( dmv_k230_device::io_w )
{
	iospace().write_byte(offset, data);
}

READ8_MEMBER( dmv_k230_device::program_r )
{
	return prog_read(space, offset);
}

WRITE8_MEMBER( dmv_k230_device::program_w )
{
	prog_write(space, offset, data);
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
		out_thold(CLEAR_LINE);
		m_switch16 = state;
	}
}

READ8_MEMBER( dmv_k234_device::snr_r )
{
	m_snr = ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_maincpu->reset();
	out_thold(ASSERT_LINE);

	return 0xff;
}

WRITE8_MEMBER( dmv_k234_device::snr_w )
{
	m_snr = ASSERT_LINE;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_maincpu->reset();
	out_thold(ASSERT_LINE);
}
