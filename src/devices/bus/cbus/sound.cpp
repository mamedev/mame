// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* Internal sound slot options */

#include "emu.h"
#include "sound.h"

/*
 * Early CanBe (-86 based)
 * PC-98GS/9821 Multi/Ap/As/Ae/Ce/Af/Ap2/As2/Cs2/Ce2/An/Ap3/As3/Cf/Cx/Cb/Cx2/Cb2
 *
 * References:
 * io_sound.txt
 *
 */

DEFINE_DEVICE_TYPE(SOUND_PC9821CE, sound_pc9821ce_device, "sound_pc9821ce", "NEC PC-9821Ce built-in sound")

sound_pc9821ce_device::sound_pc9821ce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, SOUND_PC9821CE, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_io_config("pnp_io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sound_pc9821ce_device::pnp_io_map), this))
	, m_eeprom(*this, "eeprom")
{
}

device_memory_interface::space_config_vector sound_pc9821ce_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_io_config)
	};
}

const tiny_rom_entry *sound_pc9821ce_device::device_rom_region() const
{
	return nullptr;
}

void sound_pc9821ce_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
	EEPROM_93C06_16BIT(config, m_eeprom);
}


void sound_pc9821ce_device::device_start()
{
	pc9801_86_device::device_start();

	m_bus->install_device(0xac6c, 0xac6f, *this, &sound_pc9821ce_device::pnp_map);
}

void sound_pc9821ce_device::device_reset()
{
	pc9801_86_device::device_reset();
}

// expects writes with 0x54xx for index, 0x41xx for data
void sound_pc9821ce_device::pnp_map(address_map &map)
{
	map(0x00, 0x01).lrw16(
		NAME([this] (offs_t offset) {
			return 0x5400 | m_index;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (mem_mask != 0xffff || (data & 0xff00) != 0x5400)
				logerror("sound_pc9821ce: warning access index with %04x & %04x (blocked?)\n", data, mem_mask);
			m_index = data & 0xff;
		})
	);
	map(0x02, 0x03).lrw16(
		NAME([this] (offs_t offset) {
			return 0x4100 | this->space(0).read_byte(m_index);
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (mem_mask != 0xffff || (data & 0xff00) != 0x4100)
				logerror("sound_pc9821ce: warning access data with %04x & %04x (blocked?)\n", data, mem_mask);
			this->space(0).write_byte(m_index, data & 0xff);
		})
	);
}

// applies to PC-9821 Multi and PC-9821Ce
// other models (PC-98GS, A Mate, other early CanBe) maps some stuff differently
void sound_pc9821ce_device::pnp_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			return m_eeprom->do_read() << 0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_eeprom->cs_write(BIT(data, 2));
			m_eeprom->clk_write(BIT(data, 1));
			m_eeprom->di_write(BIT(data, 0));
		})
	);

	// ??-? -??- <unknown>
	// --x- ---- using SCSI (mapping or mounting?)
	// ---- x--- Sound ports PnP
//	map(0x10, 0x10)

	// ???? ??-- <unknown>
	// ---- --xx SCSI I/O port
//	map(0x20, 0x20)

	// ???? ?--- <unknown>
	// ---- -111 SCSI not used
	// ---- -000 SCSI used
//	map(0x21, 0x21)

//	map(0x30, 0x30) Sound irq related
//	map(0x40, 0x40) <unknown>

	// CPU flag
	// -100 High
	// -010 Middle
	// -001 Low
	// TODO: repeated in Setup menu, where this comes from?
	map(0x50, 0x50).lr8(
		NAME([] () { return 4; })
	);

//	map(0x52, 0x52) <unknown>

//	map(0xf0, 0xf0) <unknown>
}

/*
 * Late CanBe (-118 based)
 * Cf/Cx/Cb/Cx2/Cb2/Cx3/Cb3/Na12/Na9/Na7/Nb7/Nx
 *
 * References:
 * - io_canbe.txt
 *
 */

DEFINE_DEVICE_TYPE(SOUND_PC9821CX3, sound_pc9821cx3_device, "sound_pc9821cx3", "NEC PC-9821Cx3 built-in sound")

sound_pc9821cx3_device::sound_pc9821cx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_118_device(mconfig, SOUND_PC9821CX3, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_io_config("pnp_io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sound_pc9821cx3_device::pnp_io_map), this))
{
}

device_memory_interface::space_config_vector sound_pc9821cx3_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_io_config)
	};
}

const tiny_rom_entry *sound_pc9821cx3_device::device_rom_region() const
{
	return nullptr;
}

void sound_pc9821cx3_device::device_add_mconfig(machine_config &config)
{
	pc9801_118_device::device_add_mconfig(config);
}


void sound_pc9821cx3_device::device_start()
{
	pc9801_118_device::device_start();

	m_bus->install_io(0x0f4a, 0x0f4b, read8sm_delegate(*this, FUNC(sound_pc9821cx3_device::control_r)), write8sm_delegate(*this, FUNC(sound_pc9821cx3_device::control_w)));
}

void sound_pc9821cx3_device::device_reset()
{
	pc9801_118_device::device_reset();
}

u8 sound_pc9821cx3_device::control_r(offs_t offset)
{
	if (!offset)
	{
		logerror("sound_pc9821cx3: Warning control_r from index (confirm me)\n");
		return m_index;
	}

	return this->space(0).read_byte(m_index);
}

void sound_pc9821cx3_device::control_w(offs_t offset, u8 data)
{
	if (!offset)
		m_index = data & 0xff;
	else
		this->space(0).write_byte(m_index, data & 0xff);
}

/*
 * [0x00] <unknown>
 * [0x01] Windows Sound System related
 * ---- xxxx irq select
 * ---- 1000 INT8
 * ---- 0011 INT0
 * ---- 0010 INT41
 * ---- 0000 INT5
 * [0x02] <unknown>
 * [0x03] Remote control reset
 * ---- -x-- (1) Mic through (loopback?)
 * ---- ---x (1) Remote control reset
 * [0x04] Mute control
 * ---- ---x (Global?) sound mute
 * [0x10] Remote control data status
 * ---- --x- (1) device ready (0) busy
 * ---- ---x (1) received data available
 * [0x11] Remote control code
 * <returns the button pressed in 2 bytes form, 2nd byte is the XOR-ed version of 1st>
 * [0x12] <unknown>
 * [0x13] Power control
 * <succession of bytes to power on/off>
 * [0x14] remote irq control
 * ---- -x-- irq enable
 * ---- --xx irq select
 * ---- --11 INT41
 * ---- --10 INT1
 * ---- --01 INT2
 * ---- --00 INT0
 * [0x30] VOL1,2 YMF288 Left sound output
 * [0x31] VOL1,2 YMF288 Right sound output
 * [0x32] VOL3,4 Line Left input
 * [0x33] VOL3,4 Line Right input
 * [0x34] <unknown>
 * [0x35] <unknown>
 */
void sound_pc9821cx3_device::pnp_io_map(address_map &map)
{
	map.unmap_value_high();
	// throws MICOM ERROR otherwise
	map(0x10, 0x10).lr8(NAME([] () { return 2; }));
}
