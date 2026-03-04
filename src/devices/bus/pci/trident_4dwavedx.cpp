// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Trident 4DWAVE-DX

AC'97 v1.x (fixed 48 kHz)

TODO:
- Stub-ish, enough to install fine in win98se and nothing else;
- wavetsr.com can't find a free IRQ for SB emulation under DOS;
- Soundblaster, FM and MPU-401 are emulated by the 4dwave sound engine;

**************************************************************************************************/

#include "emu.h"
#include "trident_4dwavedx.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(TRIDENT_4DWAVEDX, trident_4dwavedx_device,   "trident_4dwavedx",   "Trident 4D Wave-DX sound card")


trident_4dwavedx_device::trident_4dwavedx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_joy(*this, "pc_joy")
{
}

trident_4dwavedx_device::trident_4dwavedx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: trident_4dwavedx_device(mconfig, TRIDENT_4DWAVEDX, tag, owner, clock)
{
	set_ids(0x10232000, 0x00, 0x040100, 0x10232000);
}

void trident_4dwavedx_device::device_add_mconfig(machine_config &config)
{
	// TODO: SigmaTel STAC9704T or STAC9700T
	// Trident suggests to use an AD1819A

	PC_JOY(config, m_joy);
}

void trident_4dwavedx_device::device_start()
{
	pci_card_device::device_start();

	add_map( 256,    M_IO,  FUNC(trident_4dwavedx_device::io_map));
	add_map( 4*1024, M_MEM, FUNC(trident_4dwavedx_device::mmio_map));

	// INTA#
	intr_pin = 1;
	intr_line = 0x05;

	// TODO: max_lat = 0x05, min_gnt = 0x02

	save_item(NAME(m_ddma_config));
	save_item(NAME(m_legacy_control));
	save_item(NAME(m_power_state));
	save_item(STRUCT_MEMBER(m_interrupt_snoop, enable));
	save_item(STRUCT_MEMBER(m_interrupt_snoop, vector));

	save_item(NAME(m_asr4));
	save_item(NAME(m_asr5));
	save_item(NAME(m_asr6));
}

void trident_4dwavedx_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 7;
	// Medium DEVSEL#, support cap list
	status = 0x0210;

	m_ddma_config = m_legacy_control = 0;
	m_power_state = 0;
	m_interrupt_snoop.enable = false;
	m_interrupt_snoop.vector = 0;

	m_asr4 = 0;
	m_asr5 = 0x04;
	m_asr6 = 0x02;

	remap_cb();
}

u8 trident_4dwavedx_device::capptr_r()
{
	return 0x48;
}

void trident_4dwavedx_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);

	// DDMA config
	map(0x40, 0x43).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			LOG("PCI 40h: DDMA config read (mask %08x)\n", mem_mask);
			return m_ddma_config;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_ddma_config);
			// knock off Legacy DMA Transfer Size Control
			m_ddma_config &= ~(3 << 1);
			LOG("PCI 40h: %08x & %08x\n", data, mem_mask);
			LOG("\tDDMA config write: address %08x Extended Address %d Slave Channel Access %d\n"
				, m_ddma_config >> 4, BIT(m_ddma_config, 3), BIT(m_ddma_config, 0));
		})
	);
//	map(0x44, 0x44) Legacy I/O Base
//	map(0x45, 0x45) Legacy DMA
//	map(0x46, 0x46) Legacy Control
	map(0x44, 0x47).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			LOG("PCI 44h: legacy control read (mask %08x)\n", mem_mask);
			return m_legacy_control;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_legacy_control);
			m_legacy_control &= 0x0006'03ff;
			LOG("PCI 44h: %08x & %08x\n", data, mem_mask);
			LOG("\tLegacy I/O base: %02x Legacy DMA: %02x Audio Engine Reset: %d Writable subsystem %d\n"
				, m_legacy_control & 0xff
				, (m_legacy_control >> 8) & 3
				, BIT(m_legacy_control, 18)
				, BIT(m_legacy_control, 17)
			);
			if (ACCESSING_BITS_0_7)
				remap_cb();
		})
	);

	// Power Management v1.0, D2 and D1 support, no PME#
	map(0x48, 0x4b).lr32(NAME([] () { return 0x0601'0001; }));
	map(0x4c, 0x4f).lrw32(
		NAME([this] (offs_t offset) { return m_power_state; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
			{
				LOG("PM Power State D%d\n", data & 3);
				m_power_state = data & 3;
			}
		})
	);

	map(0x50, 0x53).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) { return m_interrupt_snoop.enable | (m_interrupt_snoop.vector << 8); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("PCI 50h: %08x & %08x\n", data, mem_mask);
			if (ACCESSING_BITS_0_7)
			{
				m_interrupt_snoop.enable = !!BIT(data, 0);
				LOG("\tInterrupt snoop enable %d\n", m_interrupt_snoop.enable);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_interrupt_snoop.vector = (data >> 8) & 0xff;
				LOG("\tInterrupt snoop vector %02x\n", m_interrupt_snoop.vector);
			}
		})
	);
}

void trident_4dwavedx_device::io_map(address_map &map)
{
	map(0x5c, 0x5f).lrw32(
		NAME([this] (offs_t offset) {
			return m_asr4 | (m_asr5 << 16) | (m_asr6 << 24);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
				m_asr4 = data & 0xff;
			if (ACCESSING_BITS_16_23)
			{
				m_asr5 = (data >> 16) & 0xff;
				LOG("ASR5: SB ESP Version High %02x\n", m_asr5);
			}
			if (ACCESSING_BITS_24_31)
			{
				m_asr6 = (data >> 24) & 0xff;
				LOG("ASR6: SB ESP Version High %02x\n", m_asr6);
			}
		})
	);
}

void trident_4dwavedx_device::mmio_map(address_map &map)
{
}

void trident_4dwavedx_device::gameport_map(address_map &map)
{
	map(0x00, 0x07).rw(m_joy, FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
}

void trident_4dwavedx_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: legacy mapping
#if 0
	if (BIT(m_legacy_control, 7))
	{
		const u16 mpu401_port = BIT(m_legacy_control, 6) ? 0x0300 : 0x0330;
		io_space->install_device(mpu401_port, mpu401_port + 3, *this, &trident_4dwavedx_device::midi_map);
	}
#endif

	if (BIT(m_legacy_control, 5))
	{
		const u16 game_port = BIT(m_legacy_control, 4) ? 0x0208 : 0x0200;
		io_space->install_device(game_port, game_port + 7, *this, &trident_4dwavedx_device::gameport_map);
	}

#if 0
	if (BIT(m_legacy_control, 3))
	{
		const u16 fm_port = BIT(m_legacy_control, 2) ? 0x038c : 0x0388;
		io_space->install_device(fm_port, fm_port + 3, *this, &trident_4dwavedx_device::fm_map);
	}

	if (BIT(m_legacy_control, 1))
	{
		const u16 sb_port = BIT(m_legacy_control, 0) ? 0x0240 : 0x0220;
		io_space->install_device(sb_port, sb_port + 0xf, *this, &trident_4dwavedx_device::sb_map);
	}

#endif
}
