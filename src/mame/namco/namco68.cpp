// license:BSD-3-Clause
// copyright-holders:David Haywood, K.Wilkins

/*

TODO:
output support

*/

#include "emu.h"
#include "namco68.h"

DEFINE_DEVICE_TYPE(NAMCOC68, namcoc68_device, "namcoc68", "Namco C68 I/O")

namcoc68_device::namcoc68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOC68, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_in_pb_cb(*this),
	m_in_pc_cb(*this),
	m_in_ph_cb(*this),
	m_in_pdsw_cb(*this),
	m_port_analog_in_cb(*this),
	m_port_dial_in_cb(*this),
	m_dp_in(*this),
	m_dp_out(*this)
{
}

ROM_START( namcoc68 )
	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "c68.bin",  0x000000, 0x008000, CRC(ca64550a) SHA1(38d1ad1b1287cadef0c999aff9357927315f8e6b) ) // usually position 3d, but device really needs a way to specify it
ROM_END


// C68 looks like a drop-in replacement for C65, so the port mappings must be scrambled in one instance, since this already has a multiplex on the port, assume here
uint8_t namcoc68_device::c68_p5_r()
{
	uint16_t ret = (m_in_ph_cb() << 8) | m_in_pb_cb();

	if (m_player_mux)
	{
		return bitswap<8>(ret, 6, 8, 10, 12, 4, 2, 0, 14);
	}
	else
	{
		return bitswap<8>(ret, 7, 9, 11, 13, 5, 3, 1, 15);
	}
}

uint8_t namcoc68_device::mcuc_r()
{
	uint8_t ret = m_in_pc_cb();
	ret = bitswap<8>(ret, 3, 2, 1, 0, 7, 6, 5, 4);
	return ret;
}

void namcoc68_device::c68_p3_w(uint8_t data)
{
	m_player_mux = (data & 0x80) ? 1 : 0;
}

uint8_t namcoc68_device::ack_mcu_vbl_r()
{
	m_mcu->set_input_line(m37450_device::M3745X_INT1_LINE, CLEAR_LINE);
	return 0;
}

uint8_t namcoc68_device::dpram_byte_r(offs_t offset)
{
	return m_dp_in(offset);
}

void namcoc68_device::dpram_byte_w(offs_t offset, uint8_t data)
{
	m_dp_out(offset,data);
}

uint8_t namcoc68_device::unk_r()
{
	return 0x00;
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
	M37450(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->read_ad<0>().set(FUNC(namcoc68_device::mcuan0_r));
	m_mcu->read_ad<1>().set(FUNC(namcoc68_device::mcuan1_r));
	m_mcu->read_ad<2>().set(FUNC(namcoc68_device::mcuan2_r));
	m_mcu->read_ad<3>().set(FUNC(namcoc68_device::mcuan3_r));
	m_mcu->read_ad<4>().set(FUNC(namcoc68_device::mcuan4_r));
	m_mcu->read_ad<5>().set(FUNC(namcoc68_device::mcuan5_r));
	m_mcu->read_ad<6>().set(FUNC(namcoc68_device::mcuan6_r));
	m_mcu->read_ad<7>().set(FUNC(namcoc68_device::mcuan7_r));
	m_mcu->read_p<3>().set(FUNC(namcoc68_device::mcuc_r)); // coins/test/service
	m_mcu->write_p<3>().set(FUNC(namcoc68_device::c68_p3_w));
	m_mcu->read_p<5>().set(FUNC(namcoc68_device::c68_p5_r)); // muxed player 1/2
	m_mcu->write_p<5>().set_nop();
	m_mcu->read_p<6>().set(FUNC(namcoc68_device::unk_r)); // unused in sgunner2
	m_mcu->write_p<6>().set_nop();
	m_mcu->set_addrmap(AS_PROGRAM, &namcoc68_device::c68_default_am);
}

void namcoc68_device::device_resolve_objects()
{
	m_in_pb_cb.resolve_safe(0xff);
	m_in_pc_cb.resolve_safe(0xff);
	m_in_ph_cb.resolve_safe(0xff);
	m_in_pdsw_cb.resolve_safe(0xff);

	m_port_analog_in_cb.resolve_all_safe(0xff);
	m_port_dial_in_cb.resolve_all_safe(0xff);

	m_dp_in.resolve_safe(0xff);
	m_dp_out.resolve_safe();
}

void namcoc68_device::device_start()
{
	save_item(NAME(m_player_mux));
}

void namcoc68_device::device_reset()
{
	m_player_mux = 0;
}

const tiny_rom_entry *namcoc68_device::device_rom_region() const
{
	return ROM_NAME(namcoc68);
}
