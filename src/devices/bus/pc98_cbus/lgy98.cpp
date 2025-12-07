// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Melco/Buffalo LGY-98 NIC (NE2000 based)

LANMAN.DOS setup described here:
https://pokug.net/entry/2022/01/22/221536

TODO:
- Has no dipswitches on board, the port and INT lines are PnP configured thru LGYSETUP.
  Once LANMAN is setup it will try $00d0 base even if EEPROM says otherwise at CONFIG.SYS boot,
  and even if PROTOCOL.INI is explicitly  $10d0, definitely needs a BIOS.
  wpset 0xa294,2,w,wpdata==0xd0
- Wants CR bit 5 high for enough time on DMA transfers at CONFIG.SYS loading
- Untested beyond this.

**************************************************************************************************/

#include "emu.h"
#include "lgy98.h"

#include "multibyte.h"

DEFINE_DEVICE_TYPE(LGY98, lgy98_device, "lgy98", "Melco LGY-98 NIC interface")

lgy98_device::lgy98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LGY98, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_nic(*this, "nic")
	, m_eeprom(*this, "eeprom")
	, m_space_mem_config("internal_memory", ENDIANNESS_LITTLE, 8, 15, 0, address_map_constructor(FUNC(lgy98_device::memory_map), this))
{
}

void lgy98_device::device_add_mconfig(machine_config &config)
{
	DP8390D(config, m_nic, 20'000'000);
	m_nic->irq_callback().set([this](int state) { m_bus->int_w(0, state); });
	m_nic->mem_read_callback().set([this] (offs_t offset) { return space(0).read_byte(offset); });
	m_nic->mem_write_callback().set([this] (offs_t offset, u8 data) { space(0).write_byte(offset, data); });

	// ATMEL829 93C46 PC
	EEPROM_93C46_16BIT(config, m_eeprom);
}

void lgy98_device::device_start()
{
	// NOTE: identical to isa/ne2000.cpp
	uint8_t mac[6];
	uint32_t num = machine().rand();
	memset(m_prom, 0x57, 16);
	mac[2] = 0x1b;
	put_u24be(mac+3, num);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_nic->set_mac(mac);
}

void lgy98_device::device_reset()
{
	// TODO: this clears the PROM (?)
	//memcpy(m_prom, &m_nic->get_mac()[0], 6);
}

device_memory_interface::space_config_vector lgy98_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_mem_config)
	};
}

ROM_START( lgy98 )
	// either 0x2000 or 0x4000 in size
	ROM_REGION16_LE( 0x2000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD("bios.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

const tiny_rom_entry *lgy98_device::device_rom_region() const
{
	return ROM_NAME( lgy98 );
}


void lgy98_device::remap(int space_id, offs_t start, offs_t end)
{
	// TODO: seems to employ an (undumped) ROM socket too
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &lgy98_device::io_map);
	}
}

void lgy98_device::io_map(address_map &map)
{
	// 0*d* ~ 7*d*
	const u16 base_io = 0x10d0;

	map(base_io, base_io + 0xf).rw(m_nic, FUNC(dp8390_device::cs_read), FUNC(dp8390_device::cs_write));

	// TODO: +0x18 reset (on reads?)

	map(base_io + 0x200, base_io + 0x201).rw(m_nic, FUNC(dp8390_device::remote_read), FUNC(dp8390_device::remote_write));

	// identification flags
	map(base_io + 0x30a, base_io + 0x30a).lr8(NAME([] () { return 0x00; }));
	map(base_io + 0x30b, base_io + 0x30b).lr8(NAME([] () { return 0x40; }));
	map(base_io + 0x30c, base_io + 0x30c).lr8(NAME([] () { return 0x26; }));
	map(base_io + 0x30d, base_io + 0x30d).lrw8(
		NAME([this] () {
			// TODO: bit 5 required low during ID sequence
			return 0x0a | (m_eeprom->do_read());
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: other bits
			m_eeprom->di_write(BIT(data, 1));
			m_eeprom->cs_write(BIT(data, 2));
			m_eeprom->clk_write(BIT(data, 3));
		})
	);
}

// NOTE: again identical to isa/ne2000.cpp
void lgy98_device::memory_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x001f).lr8(NAME([this] (offs_t offset) { return m_prom[offset >> 1]; }));
	map(0x4000, 0x7fff).ram();
}
