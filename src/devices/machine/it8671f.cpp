// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

ITE IT8671F Giga I/O
ITE IT8673F Advanced I/O
ITE IT8680F Super AT I/O Chipset (& IT8680RF)

TODO:
- Emulate the other flavours here (probably more of them with some digging);
- IT8687R looks to be a transceiver chip, tied with 8671 and 8680;
- GPIO;
- Keyboard A20 doesn't work right (most likely needs own BIOS);
- SMI source;
- ISA PnP mode;

**************************************************************************************************/

#include "emu.h"
#include "it8671f.h"

#include "bus/pc_kbd/keyboards.h"

#include "formats/naslite_dsk.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(IT8671F, it8671f_device, "it8671f", "ITE IT8671F Giga I/O")

it8671f_device::it8671f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IT8671F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(it8671f_device::config_map), this))
	, m_fdc(*this, "fdc")
	, m_com(*this, "uart%d", 0U)
	, m_lpt(*this, "lpta")
	, m_keybc(*this, "keybc")
	, m_ps2_con(*this, "ps2_con")
	, m_aux_con(*this, "aux_con")
	, m_logical_view(*this, "logical_view")
	, m_krst_callback(*this)
	, m_ga20_callback(*this)
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, m_index(0)
	, m_logical_index(0)
	, m_lock_sequence_index(0)
{
	std::fill(std::begin(m_activate), std::end(m_activate), false);
}

it8671f_device::~it8671f_device()
{
}

ALLOW_SAVE_TYPE(it8671f_device::config_phase_t);

void it8671f_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(0, this, true);
	m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	m_isa->set_dma_channel(3, this, true);
	save_item(NAME(m_activate));
	save_item(NAME(m_port_select_index));
	save_item(NAME(m_port_select_data));
	save_item(NAME(m_config_phase));
	save_item(NAME(m_port_config));
	save_item(NAME(m_lock_port_index));
	save_item(NAME(m_lock_sequence_index));

	save_item(NAME(m_fdc_irq_line));
	save_item(NAME(m_fdc_drq_line));
	save_item(NAME(m_com_irq_line));
	save_item(NAME(m_lpt_irq_line));
	save_item(NAME(m_lpt_drq_line));
	save_item(NAME(m_key_irq_line));
	save_item(NAME(m_aux_irq_line));
}

void it8671f_device::device_reset()
{
	m_index = 0;
	m_lock_sequence_index = m_lock_port_index = 0;
	m_config_phase = config_phase_t::WAIT_FOR_KEY;
	// irrelevant, should really remap with port_select
	m_port_select_index = 0x3f0;
	m_port_select_data = 0x3f1;
	std::fill(std::begin(m_activate), std::end(m_activate), false);
	// TODO: from UIF4 pin default (comebaby never explicitly enables it)
	m_activate[5] = true;

	m_fdc_irq_line = 6;
	m_fdc_drq_line = 2;
//  m_fdc_mode = ;
	m_fdc_address = 0x03f0;

	m_lpt_address = 0x0378;
	m_lpt_irq_line = 7;
	m_lpt_drq_line = 4; // disabled
//  m_lpt_mode = 0x3f;

	m_fdc_f0 = 0;
	m_fdc_f1 = 0;
	m_fdc->set_mode(upd765_family_device::mode_t::AT);
	m_fdc->set_rate(500000);

	m_last_dma_line = -1;

	m_key_irq_line = 1;
	m_aux_irq_line = 0xc;

	remap(AS_IO, 0, 0x400);
}

device_memory_interface::space_config_vector it8671f_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void it8671f_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}

void it8671f_device::device_add_mconfig(machine_config &config)
{
	// 82077 compatible
	N82077AA(config, m_fdc, XTAL(24'000'000), upd765_family_device::mode_t::AT);
	m_fdc->intrq_wr_callback().set(FUNC(it8671f_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(it8671f_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", it8671f_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", it8671f_device::floppy_formats);

	NS16550(config, m_com[0], XTAL(24'000'000) / 13);
	m_com[0]->out_int_callback().set(FUNC(it8671f_device::irq_serial1_w));
	m_com[0]->out_tx_callback().set(FUNC(it8671f_device::txd_serial1_w));
	m_com[0]->out_dtr_callback().set(FUNC(it8671f_device::dtr_serial1_w));
	m_com[0]->out_rts_callback().set(FUNC(it8671f_device::rts_serial1_w));

	NS16550(config, m_com[1], XTAL(24'000'000) / 13);
	m_com[1]->out_int_callback().set(FUNC(it8671f_device::irq_serial2_w));
	m_com[1]->out_tx_callback().set(FUNC(it8671f_device::txd_serial2_w));
	m_com[1]->out_dtr_callback().set(FUNC(it8671f_device::dtr_serial2_w));
	m_com[1]->out_rts_callback().set(FUNC(it8671f_device::rts_serial2_w));

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(it8671f_device::irq_parallel_w));

	PS2_KEYBOARD_CONTROLLER(config, m_keybc, XTAL(8'000'000));
	// didn't tried, assume non-working with ibm BIOS
	m_keybc->set_default_bios_tag("compaq");
	m_keybc->hot_res().set(FUNC(it8671f_device::cpu_reset_w));
	m_keybc->gate_a20().set(FUNC(it8671f_device::cpu_a20_w));
	m_keybc->kbd_irq().set(FUNC(it8671f_device::irq_keyboard_w));
	m_keybc->kbd_clk().set(m_ps2_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_ps2_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_keybc->aux_irq().set(FUNC(it8671f_device::irq_mouse_w));
	m_keybc->aux_clk().set(m_aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->aux_data().set(m_aux_con, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_ps2_con, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_ps2_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	m_ps2_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// TODO: verify if it doesn't give problems later on
	PC_KBDC(config, m_aux_con, ps2_mice, STR_HLE_PS2_MOUSE);
	m_aux_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
	m_aux_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_data_w));
}


void it8671f_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		if (m_config_phase == config_phase_t::WAIT_FOR_KEY)
			m_isa->install_device(0x0000, 0x03ff, *this, &it8671f_device::port_map);
		else
		{
			m_isa->install_device(m_port_select_index, m_port_select_data, read8sm_delegate(*this, FUNC(it8671f_device::read)), write8sm_delegate(*this, FUNC(it8671f_device::write)));
		}

		if (m_activate[0])
		{
			m_isa->install_device(m_fdc_address, m_fdc_address + 7, *m_fdc, &n82077aa_device::map);
		}

		for (int i = 0; i < 2; i++)
		{
			if (m_activate[i + 1])
			{
				const u16 uart_addr = m_com_address[i];
				m_isa->install_device(uart_addr, uart_addr + 7, read8sm_delegate(*m_com[i], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[i], FUNC(ns16450_device::ins8250_w)));
			}
		}

		// can't map below 0x100
		if (m_activate[3] & 1 && m_lpt_address & 0xf00)
		{
			m_isa->install_device(m_lpt_address, m_lpt_address + 3, read8sm_delegate(*m_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_lpt, FUNC(pc_lpt_device::write)));
		}

		if (m_activate[5] || m_activate[6])
		{
			m_isa->install_device(0x60, 0x60, read8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::data_r)), write8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::data_w)));
			m_isa->install_device(0x64, 0x64, read8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::status_r)), write8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::command_w)));
		}
	}
}

// Goofy, but it's the only way that we can install a byte-wide write only I/O handler in ISA bus
void it8671f_device::port_map(address_map &map)
{
	map(0x0279, 0x0279).w(FUNC(it8671f_device::port_select_w));
}

void it8671f_device::port_select_w(offs_t offset, uint8_t data)
{
	if (m_config_phase == config_phase_t::WAIT_FOR_KEY)
	{
		m_port_config[m_lock_port_index++] = data;
		// TODO: simplified, may really discard oldest result
		if (m_lock_port_index == 4)
		{
			bool valid = false;
			switch((m_port_config[2] << 8) | m_port_config[3])
			{
				case 0x5555:
					m_port_select_index = 0x3f0;
					m_port_select_data = 0x3f1;
					valid = true;
					break;
				case 0x55aa:
					// FIXME: this setup may not mount correctly
					m_port_select_index = 0x3bd;
					m_port_select_data = 0x3bf;
					valid = true;
					break;
				case 0xaa55:
					m_port_select_index = 0x370;
					m_port_select_data = 0x371;
					valid = true;
					break;
				default:
					// invalid, reset lock sequence
					valid = false;
					LOG("Invalid lock port sequence 2-3 %02x %02x\n", m_port_config[2], m_port_config[3]);
					break;
			}
			if (m_port_config[0] != 0x86 || m_port_config[1] != 0x80)
			{
				LOG("Invalid lock port sequence 0-1 %02x %02x\n", m_port_config[0], m_port_config[1]);
				valid = false;
			}
			m_lock_port_index = 0;
			if (valid)
			{
				LOG("Unlock PnP phase at %04x %04x\n", m_port_select_index, m_port_select_data);
				m_config_phase = config_phase_t::UNLOCK_PNP;
				remap(AS_IO, 0, 0x400);
				m_lock_sequence_index = 0;
			}
		}
	}
}

uint8_t it8671f_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_config_phase != config_phase_t::MB_PNP_MODE)
	{
		LOG("read(%d): reading while not in MB PnP mode (%d)\n", offset, m_config_phase);
		return 0;
	}

	if (!offset)
		return m_index;

	return space().read_byte(m_index);
}


void it8671f_device::write(offs_t offset, u8 data)
{
	if (!offset)
	{
		if (m_config_phase == config_phase_t::UNLOCK_PNP)
		{
			m_unlock_sequence[m_lock_sequence_index] = data;
			m_lock_sequence_index++;

			if (m_lock_sequence_index == 32)
			{
				m_lock_sequence_index = 0;
				const u8 lock_seq[32] = {
					0x6a, 0xb5, 0xda, 0xed, 0xf6, 0xfb, 0x7d, 0xbe,
					0xdf, 0x6f, 0x37, 0x1b, 0x0d, 0x86, 0xc3, 0x61,
					0xb0, 0x58, 0x2c, 0x16, 0x8b, 0x45, 0xa2, 0xd1,
					0xe8, 0x74, 0x3a, 0x9d, 0xce, 0xe7, 0x73, 0x39
				};
				bool valid = true;
				// TODO: as above
				for (int i = 0; i < 32; i++)
				{
					if (m_unlock_sequence[i] != lock_seq[i])
						valid = false;
				}
				if (valid)
				{
					LOG("Unlock MB PnP phase\n");
					m_config_phase = config_phase_t::MB_PNP_MODE;
					remap(AS_IO, 0, 0x400);
				}
			}
		}
		else if (m_config_phase == config_phase_t::MB_PNP_MODE)
		{
			m_index = data;
		}
	}
	else
	{
		if (m_config_phase == config_phase_t::MB_PNP_MODE)
			space().write_byte(m_index, data);
		else
			LOG("data_w: writing %02x while not in MB PnP mode (%d)\n", data, m_config_phase);
	}
}


void it8671f_device::config_map(address_map &map)
{
//	map(0x00, 0x00) (ISA PnP) set RD_DATA Port
//	map(0x01, 0x01) (ISA PnP) Serial isolation
	map(0x02, 0x02).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 1))
			{
				// TODO: throws several phase drifts with the theoretically more correct WAIT_FOR_KEY
				// Notice however that doing so it will BSoD in comebaby, missing setting?
				m_config_phase = config_phase_t::UNLOCK_PNP;
				remap(AS_IO, 0, 0x400);
				LOG("Exit setup mode\n");
			}
			// TODO: bit 0 for global reset
		})
	);
//	map(0x03, 0x03) (ISA PnP) Wake[CSN]
//	map(0x04, 0x04) (ISA PnP) Resource Data
//	map(0x05, 0x05) (ISA PnP) Status
//	map(0x06, 0x06) (ISA PnP) Card Select Number
	map(0x07, 0x07).lr8(NAME([this] () { return m_logical_index; })).w(FUNC(it8671f_device::logical_device_select_w));
	map(0x20, 0x20).lr8(NAME([] () { return 0x86; })); // chip ID byte 1
	map(0x21, 0x21).lr8(NAME([] () { return 0x81; })); // chip ID byte 2
	map(0x22, 0x22).lr8(NAME([] () { return 0x00; })); // version
//  map(0x23, 0x23) (MB PnP) Logical Device Enable
//  map(0x24, 0x24) (MB PnP) Software Suspend
//  map(0x25, 0x26) (MB PnP) GPIO Function Enable
//	map(0x2e, 0x2e) (MB PnP) <reserved>
//  map(0x2f, 0x2f) (MB PnP) LDN=F4 Test Enable

	map(0x30, 0xff).view(m_logical_view);
	// FDC
	m_logical_view[0](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<0>), FUNC(it8671f_device::activate_w<0>));
	m_logical_view[0](0x60, 0x61).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fdc_address >> (offset * 8)) & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			m_fdc_address &= 0xff << shift;
			m_fdc_address |= data << (shift ^ 8);
			m_fdc_address &= ~0xf007;
			LOG("LDN0 (FDC): remap %04x ([%d] %02x)\n", m_fdc_address, offset, data);

			remap(AS_IO, 0, 0x400);
		})
	);
	m_logical_view[0](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_fdc_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_irq_line = data & 0xf;
			LOG("LDN0 (FDC): irq routed to %02x\n", m_fdc_irq_line);
		})
	);
	m_logical_view[0](0x74, 0x74).lrw8(
		NAME([this] () {
			return m_fdc_drq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_drq_line = data & 0x7;
			LOG("LDN0 (FDC): drq %s (%02x)\n", BIT(m_fdc_drq_line, 2) ? "disabled" : "enabled", data);
			update_dreq_mapping(m_fdc_drq_line, 0);
		})
	);
	m_logical_view[0](0xf0, 0xf0).lrw8(
		NAME([this] () {
			LOG("LDN0 (FDC): special config 1 (F0h) read\n");
			return m_fdc_f0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("LDN0 (FDC): special config 1 (F0h) %02x\n", data);
			m_fdc_f0 = data;
		})
	);
	m_logical_view[0](0xf1, 0xf1).lrw8(
		NAME([this] () {
			LOG("LDN0 (FDC): special config 2 (F1h) read\n");
			return m_fdc_f1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: sets up bit 7 at POST, undocumented
			LOG("LDN0 (FDC): special config 2 (F1h) %02x\n", data);
			m_fdc_f1 = data;
		})
	);

	// UART1
	m_logical_view[1](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<1>), FUNC(it8671f_device::activate_w<1>));
	m_logical_view[1](0x60, 0x61).rw(FUNC(it8671f_device::uart_address_r<0>), FUNC(it8671f_device::uart_address_w<0>));
	m_logical_view[1](0x70, 0x70).rw(FUNC(it8671f_device::uart_irq_r<0>), FUNC(it8671f_device::uart_irq_w<0>));
	m_logical_view[1](0xf0, 0xf0).rw(FUNC(it8671f_device::uart_config_r<0>), FUNC(it8671f_device::uart_config_w<0>));

	// UART2
	m_logical_view[2](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<2>), FUNC(it8671f_device::activate_w<2>));
	m_logical_view[2](0x60, 0x61).rw(FUNC(it8671f_device::uart_address_r<1>), FUNC(it8671f_device::uart_address_w<1>));
	m_logical_view[2](0x70, 0x70).rw(FUNC(it8671f_device::uart_irq_r<1>), FUNC(it8671f_device::uart_irq_w<1>));
	m_logical_view[2](0xf0, 0xf0).rw(FUNC(it8671f_device::uart_config_r<1>), FUNC(it8671f_device::uart_config_w<1>));

	// LPT
	m_logical_view[3](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<3>), FUNC(it8671f_device::activate_w<3>));
	m_logical_view[3](0x60, 0x61).lrw8(
		NAME([this] (offs_t offset) {
			return (m_lpt_address >> (offset * 8)) & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			m_lpt_address &= 0xff << shift;
			m_lpt_address |= data << (shift ^ 8);
			m_lpt_address &= ~0xf003;
			LOG("LDN3 (LPT): remap %04x ([%d] %02x)\n", m_lpt_address, offset, data);

			remap(AS_IO, 0, 0x400);
		})
	);
	//m_logical_view[3](0x62, 0x63) secondary base address
	//m_logical_view[3](0x64, 0x65) POST data port base address
	m_logical_view[3](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_lpt_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lpt_irq_line = data & 0xf;
			LOG("LDN3 (LPT): irq routed to %02x\n", m_lpt_irq_line);
		})
	);
	m_logical_view[3](0x74, 0x74).lrw8(
		NAME([this] () {
			return m_lpt_drq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_lpt_drq_line = data & 0x7;
			LOG("LDN3 (LPT): drq %s (%02x)\n", BIT(m_lpt_drq_line, 2) ? "disabled" : "enabled", data);
			update_dreq_mapping(m_lpt_drq_line, 3);
		})
	);

	// APC Configuration
	m_logical_view[4](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<4>), FUNC(it8671f_device::activate_w<4>));
	m_logical_view[4](0x31, 0xff).unmaprw();

	// Keyboard
	m_logical_view[5](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<5>), FUNC(it8671f_device::activate_w<5>));
	m_logical_view[5](0x31, 0x6f).unmaprw();
	m_logical_view[5](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_key_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_key_irq_line = data & 0xf;
			LOG("LDN5 (KEYB): irq routed to %02x\n", m_key_irq_line);
		})
	);
	m_logical_view[5](0x71, 0xff).unmaprw();

	// Mouse
	m_logical_view[6](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<6>), FUNC(it8671f_device::activate_w<6>));
	m_logical_view[6](0x31, 0x6f).unmaprw();
	m_logical_view[6](0x70, 0x70).lrw8(
		NAME([this] () {
			return m_aux_irq_line;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_aux_irq_line = data & 0xf;
			LOG("LDN6 (AUX): irq routed to %02x\n", m_aux_irq_line);
		})
	);
	m_logical_view[6](0x71, 0xff).unmaprw();

	// GPIO & Alternate Function
	m_logical_view[7](0x30, 0x30).rw(FUNC(it8671f_device::activate_r<7>), FUNC(it8671f_device::activate_w<7>));
	m_logical_view[7](0x31, 0xff).unmaprw();
}

/*
 * Global register space
 */

void it8671f_device::logical_device_select_w(offs_t offset, u8 data)
{
	m_logical_index = data;
	if (m_logical_index <= 0x7)
		m_logical_view.select(m_logical_index);
	else
		LOG("Attempt to select an unmapped device with %02x\n", data);
}

template <unsigned N> u8 it8671f_device::activate_r(offs_t offset)
{
	return m_activate[N];
}

template <unsigned N> void it8671f_device::activate_w(offs_t offset, u8 data)
{
	m_activate[N] = data & 1;
	LOG("LDN%d Device %s\n", N, data & 1 ? "enabled" : "disabled");
	remap(AS_IO, 0, 0x400);
}

void it8671f_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

void it8671f_device::request_dma(int dreq, int state)
{
	switch (dreq)
	{
	case 0:
		m_isa->drq0_w(state);
		break;
	case 1:
		m_isa->drq1_w(state);
		break;
	case 2:
		m_isa->drq2_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	}
}

/*
 * Device #0 (FDC)
 */

void it8671f_device::irq_floppy_w(int state)
{
	if (!m_activate[0])
		return;
	request_irq(m_fdc_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

void it8671f_device::drq_floppy_w(int state)
{
	if (!m_activate[0] || BIT(m_fdc_drq_line, 2))
		return;
	request_dma(m_fdc_drq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Device #1/#2 (UART)
 */

template <unsigned N> u8 it8671f_device::uart_address_r(offs_t offset)
{
	return (m_com_address[N] >> (offset * 8)) & 0xff;
}

template <unsigned N> void it8671f_device::uart_address_w(offs_t offset, u8 data)
{
	const u8 shift = offset * 8;
	m_com_address[N] &= 0xff << shift;
	m_com_address[N] |= data << (shift ^ 8);
	m_com_address[N] &= ~0xf007;
	LOG("LDN%d (COM%d): remap %04x ([%d] %02x)\n", N + 1, N + 1, m_com_address[N], offset, data);

	remap(AS_IO, 0, 0x400);
}

template <unsigned N> u8 it8671f_device::uart_irq_r(offs_t offset)
{
	return m_com_irq_line[N];
}

template <unsigned N> void it8671f_device::uart_irq_w(offs_t offset, u8 data)
{
	m_com_irq_line[N] = data & 0xf;
	LOG("LDN%d (UART): irq routed to %02x\n", N, m_com_irq_line[N]);
}

template <unsigned N> u8 it8671f_device::uart_config_r(offs_t offset)
{
	return m_com_control[N];
}

/*
 * ---- -xx- Clock Source
 * ---- -00- 24 MHz/13
 * ---- -??- <reserved>
 * ---- ---x IRQ sharing enable
 */
template <unsigned N> void it8671f_device::uart_config_w(offs_t offset, u8 data)
{
	m_com_control[N] = data;
	LOG("LDN%d (UART): control %02x\n", N, m_com_control[N]);
}

void it8671f_device::irq_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	request_irq(m_com_irq_line[0], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8671f_device::irq_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	request_irq(m_com_irq_line[1], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8671f_device::txd_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_txd1_callback(state);
}

void it8671f_device::txd_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_txd2_callback(state);
}

void it8671f_device::dtr_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_ndtr1_callback(state);
}

void it8671f_device::dtr_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_ndtr2_callback(state);
}

void it8671f_device::rts_serial1_w(int state)
{
	if (!m_activate[1])
		return;
	m_nrts1_callback(state);
}

void it8671f_device::rts_serial2_w(int state)
{
	if (!m_activate[2])
		return;
	m_nrts2_callback(state);
}

void it8671f_device::rxd1_w(int state)
{
	m_com[0]->rx_w(state);
}

void it8671f_device::ndcd1_w(int state)
{
	m_com[0]->dcd_w(state);
}

void it8671f_device::ndsr1_w(int state)
{
	m_com[0]->dsr_w(state);
}

void it8671f_device::nri1_w(int state)
{
	m_com[0]->ri_w(state);
}

void it8671f_device::ncts1_w(int state)
{
	m_com[0]->cts_w(state);
}

void it8671f_device::rxd2_w(int state)
{
	m_com[1]->rx_w(state);
}

void it8671f_device::ndcd2_w(int state)
{
	m_com[1]->dcd_w(state);
}

void it8671f_device::ndsr2_w(int state)
{
	m_com[1]->dsr_w(state);
}

void it8671f_device::nri2_w(int state)
{
	m_com[1]->ri_w(state);
}

void it8671f_device::ncts2_w(int state)
{
	m_com[1]->cts_w(state);
}

/*
 * Device #3 (Parallel)
 */

void it8671f_device::irq_parallel_w(int state)
{
	if (m_activate[3] == false)
		return;
	request_irq(m_lpt_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Device #5 Keyboard
 */

void it8671f_device::cpu_a20_w(int state)
{
	if (m_activate[5] == false)
		return;
	// TODO: causes Windows 98 boot issues in comebaby (disabled elsewhere?)
	//m_ga20_callback(state);
}

void it8671f_device::cpu_reset_w(int state)
{
	if (m_activate[5] == false)
		return;
	m_krst_callback(state);
}

void it8671f_device::irq_keyboard_w(int state)
{
	if (m_activate[5] == false)
		return;
	request_irq(m_key_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}


/*
 * Device #6 Mouse
 */

void it8671f_device::irq_mouse_w(int state)
{
	if (m_activate[6] == false)
		return;
	request_irq(m_aux_irq_line, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * DMA
 */

void it8671f_device::update_dreq_mapping(int dreq, int logical)
{
	if ((dreq < 0) || (dreq >= 4))
		return;
	for (int n = 0; n < 4; n++)
		if (m_dreq_mapping[n] == logical)
			m_dreq_mapping[n] = -1;
	m_dreq_mapping[dreq] = logical;
}

void it8671f_device::eop_w(int state)
{
	// dma transfer finished
	if (m_last_dma_line < 0)
		return;
	switch (m_dreq_mapping[m_last_dma_line])
	{
	case 0:
		m_fdc->tc_w(state == ASSERT_LINE);
		break;
	default:
		break;
	}
	//m_last_dma_line = -1;
}

// TODO: LPT bindings
uint8_t it8671f_device::dack_r(int line)
{
	// transferring data from device to memory using dma
	// read one byte from device
	m_last_dma_line = line;
	switch (m_dreq_mapping[line])
	{
	case 0:
		return m_fdc->dma_r();
	default:
		break;
	}
	return 0;
}

void it8671f_device::dack_w(int line, uint8_t data)
{
	// transferring data from memory to device using dma
	// write one byte to device
	m_last_dma_line = line;
	switch (m_dreq_mapping[line])
	{
	case 0:
		m_fdc->dma_w(data);
		break;
	default:
		break;
	}
}


