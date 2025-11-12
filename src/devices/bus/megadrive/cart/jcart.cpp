// license: BSD-3-Clause
// copyright-holders: Angelo Salese, Fabio Priuli
/**************************************************************************************************

Codemasters J-Cart

Two extra ports mounted on cart

TODO:
- Starting at 3 with control ports doesn't work well for Teradrive (will map to P5/P6, before
  system ports)

**************************************************************************************************/

#include "emu.h"
#include "jcart.h"

#include "bus/sms_ctrl/controllers.h"

/*
 * Pete Sampras Tennis
 *
 * Maps J-Cart to 0x3f'fffe in place of 0x38'fffe of all the others.
 * sampras set will hang, sampras1/sampras2 won't but still won't give 4p option
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_SAMPRAS, megadrive_rom_jcart_sampras_device, "megadrive_rom_jcart_sampras", "Megadrive J-Cart Pete Sampras Tennis cart")

megadrive_rom_jcart_sampras_device::megadrive_rom_jcart_sampras_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, type, tag, owner, clock)
	, m_ctrl_ports(*this, "control%u", 3U)
{
}

megadrive_rom_jcart_sampras_device::megadrive_rom_jcart_sampras_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_jcart_sampras_device(mconfig, MEGADRIVE_ROM_JCART_SAMPRAS, tag, owner, clock)
{
}

void megadrive_rom_jcart_sampras_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_ctrl_ports[0], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ctrl_ports[0]->th_handler().set(FUNC(megadrive_rom_jcart_sampras_device::th_in<0>));

	SMS_CONTROL_PORT(config, m_ctrl_ports[1], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ctrl_ports[1]->th_handler().set(FUNC(megadrive_rom_jcart_sampras_device::th_in<1>));
}


void megadrive_rom_jcart_sampras_device::device_start()
{
	megadrive_rom_device::device_start();
	save_item(NAME(m_th_in));
	save_item(NAME(m_th_out));
}

void megadrive_rom_jcart_sampras_device::device_reset()
{
	megadrive_rom_device::device_reset();
	m_th_in[0] = m_th_in[1] = 0x40;
	m_th_out = 0x40;
}

template <unsigned N>
void megadrive_rom_jcart_sampras_device::th_in(int state)
{
	m_th_in[N] = state ? 0x40 : 0x00;
}

u16 megadrive_rom_jcart_sampras_device::jcart_r(offs_t offset, u16 mem_mask)
{
	u8 const ctrl3 = (m_ctrl_ports[0]->in_r() & 0x3f) | (m_th_in[0] & m_th_out) | 0x80;
	u8 const ctrl4 = (m_ctrl_ports[1]->in_r() & 0x3f) | (m_th_in[1] & m_th_out) | 0x80;
	return ctrl3 | (ctrl4 << 8);
}

void megadrive_rom_jcart_sampras_device::jcart_w(offs_t offset, u16 data, u16 mem_mask)
{
	// assume TH only actively driven low
	m_th_out = BIT(data, 0) << 6;
	m_ctrl_ports[0]->out_w(m_th_out | 0x3f, ~m_th_out & 0x40);
	m_ctrl_ports[1]->out_w(m_th_out | 0x3f, ~m_th_out & 0x40);
}

void megadrive_rom_jcart_sampras_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x3f'fffe, 0x3f'ffff).rw(FUNC(megadrive_rom_jcart_sampras_device::jcart_r), FUNC(megadrive_rom_jcart_sampras_device::jcart_w));
}

/*
 * Super Skidmarks / Pete Sampras Tennis '96
 *
 * Relocated J-Cart position
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_SSKID, megadrive_rom_jcart_sskid_device, "megadrive_rom_jcart_sskid", "Megadrive J-Cart Super Skidmarks cart")

megadrive_rom_jcart_sskid_device::megadrive_rom_jcart_sskid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_jcart_sampras_device(mconfig, MEGADRIVE_ROM_JCART_SSKID, tag, owner, clock)
{
}

void megadrive_rom_jcart_sskid_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x38'fffe, 0x38'ffff).rw(FUNC(megadrive_rom_jcart_sskid_device::jcart_r), FUNC(megadrive_rom_jcart_sskid_device::jcart_w));
}

/*
 * Micro Machines 2 / Micro Machines Military
 *
 * Adds an I2C to the bus
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_MICROMAC2, megadrive_rom_jcart_micromac2_device, "megadrive_rom_jcart_micromac2", "Megadrive J-Cart Micro Machines 2 cart")

megadrive_rom_jcart_micromac2_device::megadrive_rom_jcart_micromac2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_jcart_sampras_device(mconfig, type, tag, owner, clock)
	, m_i2cmem(*this, "i2cmem")
{
}

megadrive_rom_jcart_micromac2_device::megadrive_rom_jcart_micromac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_jcart_micromac2_device(mconfig, MEGADRIVE_ROM_JCART_MICROMAC2, tag, owner, clock)
{
}


void megadrive_rom_jcart_micromac2_device::device_add_mconfig(machine_config &config)
{
	megadrive_rom_jcart_sampras_device::device_add_mconfig(config);

	I2C_24C08(config, m_i2cmem);
}

void megadrive_rom_jcart_micromac2_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x30'0000, 0x30'0000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_scl(BIT(data, 1));
			m_i2cmem->write_sda(BIT(data, 0));
		})
	);
	map(0x38'0001, 0x38'0001).lr8(
		NAME([this] (offs_t offset) {
			return m_i2cmem->read_sda() << 7;
		})
	);
	map(0x38'fffe, 0x38'ffff).rw(FUNC(megadrive_rom_jcart_micromac2_device::jcart_r), FUNC(megadrive_rom_jcart_micromac2_device::jcart_w));
}

/*
 * Micro Machines Turbo Tournament 96
 *
 * Bumps I2C, otherwise same as 2
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_MICROMAC96, megadrive_rom_jcart_micromac96_device, "megadrive_rom_jcart_micromac96", "Megadrive J-Cart Micro Machines 96 cart")

megadrive_rom_jcart_micromac96_device::megadrive_rom_jcart_micromac96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_jcart_micromac2_device(mconfig, MEGADRIVE_ROM_JCART_MICROMAC96, tag, owner, clock)
{
}

void megadrive_rom_jcart_micromac96_device::device_add_mconfig(machine_config &config)
{
	megadrive_rom_jcart_micromac2_device::device_add_mconfig(config);

	I2C_24C16(config.replace(), m_i2cmem);
}

