// license:BSD-3-Clause
// copyright-holders:David Haywood, K.Wilkins

/*
TODO:
output support, Golly Ghost is currently hacking this based on DPRAM in the namcos2.cpp driver side!
some of this can likely be moved into the actual MCU core too

*/

#include "emu.h"
#include "machine/namco68.h"

DEFINE_DEVICE_TYPE(NAMCOC68, namcoc68_device, "namcoc68", "Namco C68 I/O")

namcoc68_device::namcoc68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOC68, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_in_pb_cb(*this),
	m_in_pb2_cb(*this),
	m_in_pc_cb(*this),
	m_in_ph_cb(*this),
	m_in_pdsw_cb(*this),
	m_port_analog_in_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}},
	m_port_dial_in_cb{{*this}, {*this}, {*this}, {*this}},
	m_dp_in(*this),
	m_dp_out(*this)
{
}

ROM_START( namcoc68 )
	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "c68.3d",  0x000000, 0x008000, CRC(ca64550a) SHA1(38d1ad1b1287cadef0c999aff9357927315f8e6b) )
ROM_END



READ8_MEMBER(namcoc68_device::c68_p5_r)
{
	return (m_player_mux) ? m_in_pb2_cb() : m_in_pb_cb();
}

WRITE8_MEMBER(namcoc68_device::c68_p3_w)
{
	m_player_mux = (data & 0x80) ? 1 : 0;
}

READ8_MEMBER(namcoc68_device::ack_mcu_vbl_r)
{
	m_mcu->set_input_line(m37450_device::M3745X_INT1_LINE, CLEAR_LINE);
	return 0;
}

READ8_MEMBER(namcoc68_device::dpram_byte_r)
{
	return m_dp_in(offset);
}

WRITE8_MEMBER(namcoc68_device::dpram_byte_w)
{
	m_dp_out(offset,data);
}

void namcoc68_device::c68_default_am(address_map &map)
{
	/* input ports and dips are mapped here */
	map(0x2000, 0x2000).r(FUNC(namcoc68_device::mcudsw_r));
	map(0x3000, 0x3000).r(FUNC(namcoc68_device::mcudi0_r));
	map(0x3001, 0x3001).r(FUNC(namcoc68_device::mcudi1_r));
	map(0x3002, 0x3002).r(FUNC(namcoc68_device::mcudi2_r));
	map(0x3003, 0x3003).r(FUNC(namcoc68_device::mcudi3_r));
	map(0x5000, 0x57ff).rw(FUNC(namcoc68_device::dpram_byte_r), FUNC(namcoc68_device::dpram_byte_w));
	map(0x6000, 0x6fff).r(FUNC(namcoc68_device::ack_mcu_vbl_r)); // VBL ack
	map(0x8000, 0xffff).rom().region("mcu", 0);
}

void namcoc68_device::device_add_mconfig(machine_config &config)
{
	m3745x_device* device = &M37450(config, m_mcu, DERIVED_CLOCK(1, 1)); // ugly, needs modernizing
	MCFG_M3745X_ADC14_CALLBACKS(READ8(*this, namcoc68_device, mcuan0_r), READ8(*this, namcoc68_device, mcuan1_r), READ8(*this, namcoc68_device, mcuan2_r), READ8(*this, namcoc68_device, mcuan3_r))
	MCFG_M3745X_ADC58_CALLBACKS(READ8(*this, namcoc68_device, mcuan4_r), READ8(*this, namcoc68_device, mcuan5_r), READ8(*this, namcoc68_device, mcuan6_r), READ8(*this, namcoc68_device, mcuan7_r))
	MCFG_M3745X_PORT3_CALLBACKS(READ8(*this, namcoc68_device, mcuh_r), WRITE8(*this, namcoc68_device, c68_p3_w))    // coins/test/service
	MCFG_M3745X_PORT5_CALLBACKS(READ8(*this, namcoc68_device, c68_p5_r), NOOP) // muxed player 1/2
	MCFG_M3745X_PORT6_CALLBACKS(READ8(*this, namcoc68_device, mcuc_r), NOOP) // unused in sgunner2
	MCFG_DEVICE_PROGRAM_MAP(c68_default_am)
}

void namcoc68_device::device_resolve_objects()
{
	m_in_pb_cb.resolve_safe(0xff);
	m_in_pb2_cb.resolve_safe(0xff);
	m_in_pc_cb.resolve_safe(0xff);
	m_in_ph_cb.resolve_safe(0xff);
	m_in_pdsw_cb.resolve_safe(0xff);

	for (auto &cb : m_port_analog_in_cb)
		cb.resolve_safe(0xff);

	for (auto &cb : m_port_dial_in_cb)
		cb.resolve_safe(0xff);

	m_dp_in.resolve_safe(0xff);
	m_dp_out.resolve_safe();
}

void namcoc68_device::device_start()
{
}

void namcoc68_device::device_reset()
{
	m_player_mux = 0;
}

const tiny_rom_entry *namcoc68_device::device_rom_region() const
{
	return ROM_NAME(namcoc68);
}
