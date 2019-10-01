// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Hitachi HD647180X MCU (Micro Controller Unit)

    This expandable 8-bit microcontroller architecturally extends the
    on-chip I/O capabilities of the Hitachi HD64180/Zilog Z80180 CPU
    with parallel ports and more. It also contains a 16-kilobyte
    internal programmable ROM (which can be disabled by strapping) and
    512 bytes of internal RAM, the latter being software-remappable to
    the end of any 64K block. The MP pins configure the MCU for either
    single-chip mode, one of two expanded modes or PROM writing/
    verification. (Hitachi also had the HD643180X, which uses a 16 KB
    mask ROM instead of the PROM, and HD641180X, which offers neither
    ROM nor PROM and therefore must be used in ROMless mode.)

    TODO: the current emulation is incomplete, implementing mostly
    the internal memory and parallel ports. Timer 2 (which is very
    similar to the additional timer of the HD6301) is not emulated at
    all. Programs trying to execute from internal RAM will also fail,
    though this likely capability is merely theoretical so far.

**********************************************************************/

#include "emu.h"
#include "hd647180x.h"

#define VERBOSE 0
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(HD647180X, hd647180x_device, "hd647180x", "Hitachi HD647180X MCU")

hd647180x_device::hd647180x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180_device(mconfig, HD647180X, tag, owner, clock, true, address_map_constructor(FUNC(hd647180x_device::prom_map), this))
	, m_port_input_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_port_output_cb{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 9, 0, address_map_constructor(FUNC(hd647180x_device::ram_map), this))
{
	// arbitrary initial states
	m_ccsr = 0;
	std::fill(std::begin(m_odr), std::end(m_odr), 0);
}

void hd647180x_device::prom_map(address_map &map)
{
	map(0x00000, 0x03fff).rom().region(DEVICE_SELF, 0); // 16 KB internal PROM (not used in mode 1)
}

void hd647180x_device::ram_map(address_map &map)
{
	map(0x000, 0x1ff).ram(); // 512 bytes remappable internal RAM (available in all modes)
}

device_memory_interface::space_config_vector hd647180x_device::memory_space_config() const
{
	auto spaces = z180_device::memory_space_config();
	spaces.emplace_back(AS_DATA, &m_data_config);
	return spaces;
}

uint8_t hd647180x_device::z180_read_memory(offs_t addr)
{
	if ((addr & 0xffe00) == (offs_t(m_rmcr) << 12 | 0x0fe00))
		return m_data->read_byte(addr & 0x1ff);
	else
		return z180_device::z180_read_memory(addr);
}

void hd647180x_device::z180_write_memory(offs_t addr, uint8_t data)
{
	if ((addr & 0xffe00) == (offs_t(m_rmcr) << 12 | 0x0fe00))
		m_data->write_byte(addr & 0x1ff, data);
	else
		z180_device::z180_write_memory(addr, data);
}

uint8_t hd647180x_device::z180_internal_port_read(uint8_t port)
{
	uint8_t data = 0xff;

	switch (port)
	{
	case 0x40:
		data = m_t2frc.b.l;
		LOG("HD647180X T2FRCL rd $%02x\n", data);
		break;

	case 0x41:
		data = m_t2frc.b.h;
		LOG("HD647180X T2FRCH rd $%02x\n", data);
		break;

	case 0x42:
		data = m_t2ocr[0].b.l;
		LOG("HD647180X T2OCR1L rd $%02x\n", data);
		break;

	case 0x43:
		data = m_t2ocr[0].b.h;
		LOG("HD647180X T2OCR1H rd $%02x\n", data);
		break;

	case 0x44:
		data = m_t2ocr[1].b.l;
		LOG("HD647180X T2OCR2L rd $%02x\n", data);
		break;

	case 0x45:
		data = m_t2ocr[1].b.h;
		LOG("HD647180X T2OCR2H rd $%02x\n", data);
		break;

	case 0x46:
		data = m_t2icr.b.l;
		LOG("HD647180X T2ICRL rd $%02x\n", data);
		break;

	case 0x47:
		data = m_t2icr.b.h;
		LOG("HD647180X T2ICRH rd $%02x\n", data);
		break;

	case 0x48:
		data = m_t2csr[0];
		LOG("HD647180X T2CSR1 rd $%02x\n", data);
		break;

	case 0x49:
		data = m_t2csr[1] | 0x10;
		LOG("HD647180X T2CSR2 rd $%02x\n", data);
		break;

	case 0x50:
		data = m_ccsr | 0x40;
		LOG("HD647180X CCSR rd $%02x\n", data);
		break;

	case 0x51:
		data = m_rmcr | 0x0f;
		LOG("HD647180X RMCR rd $%02x\n", data);
		break;

	case 0x53:
		data = m_dera;
		LOG("HD647180X DERA rd $%02x\n", data);
		break;

	case 0x60:
	case 0x61:
	case 0x62:
	case 0x65:
		data = m_odr[port - 0x60] & m_ddr[port - 0x60];
		if (m_ddr[port - 0x60] != 0xff)
			data |= m_port_input_cb[port - 0x60](0, ~m_ddr[port - 0x60]) & ~m_ddr[port - 0x60];
		LOG("HD647180X IDR%c rd $%02x\n", port - 0x60 + 'A', data);
		break;

	case 0x63: // ODRD is write-only
		data = m_ddr[3];
		if (m_ddr[3] != 0xff)
			data |= m_port_input_cb[3](0, ~m_ddr[3]) & ~m_ddr[3];
		LOG("HD647180X IDRD rd $%02x\n", data);
		break;

	case 0x64: // lower half of ODRE is write-only
		data = (m_odr[port - 0x60] | 0x0f) & m_ddr[3];
		if (m_ddr[3] != 0xff)
			data |= m_port_input_cb[3](0, ~m_ddr[3]) & ~m_ddr[3];
		LOG("HD647180X IDRE rd $%02x\n", data);
		break;

	case 0x66: // Port G is read-only and only has 6 bits
		data = m_port_input_cb[6](0, 0x3f) | 0xc0;
		LOG("HD647180X IDRG rd $%02x\n", data);
		break;

	default:
		data = z180_device::z180_internal_port_read(port);
		break;
	}

	return data;
}

void hd647180x_device::z180_internal_port_write(uint8_t port, uint8_t data)
{
	switch (port)
	{
	case 0x40:
		LOG("HD647180X T2FRCL wr $%02x\n", data);
		m_t2frc.b.l = data;
		break;

	case 0x41:
		LOG("HD647180X T2FRCH wr $%02x\n", data);
		m_t2frc.b.h = data;
		break;

	case 0x42:
		LOG("HD647180X T2OCR1L wr $%02x\n", data);
		m_t2ocr[0].b.l = data;
		break;

	case 0x43:
		LOG("HD647180X T2OCR1H wr $%02x\n", data);
		m_t2ocr[0].b.h = data;
		break;

	case 0x44:
		LOG("HD647180X T2OCR2L wr $%02x\n", data);
		m_t2ocr[1].b.l = data;
		break;

	case 0x45:
		LOG("HD647180X T2OCR2H wr $%02x\n", data);
		m_t2ocr[1].b.h = data;
		break;

	case 0x48:
		LOG("HD647180X T2CSR1 wr $%02x\n", data);
		m_t2csr[0] = data;
		break;

	case 0x49:
		LOG("HD647180X T2CSR2 wr $%02x\n", data);
		m_t2csr[1] = data & 0xef;
		break;

	case 0x50:
		LOG("HD647180X CCSR wr $%02x\n", data);
		m_ccsr = (m_ccsr & 0x80) | (data & 0x3f);
		break;

	case 0x51:
		LOG("HD647180X RMCR wr $%02x\n", data);
		m_rmcr = data & 0xf0;
		break;

	case 0x53:
		LOG("HD647180X DERA wr $%02x\n", data);
		m_dera = data;
		break;

	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
		LOG("HD647180X ODR%c wr $%02x\n", port - 0x60 + 'A', data);
		if ((data & m_ddr[port - 0x60]) != (m_odr[port - 0x60] & m_ddr[port - 0x60]))
			m_port_output_cb[port - 0x60](0, data | ~m_ddr[port - 0x60], m_ddr[port - 0x60]);
		m_odr[port - 0x60] = data;
		break;

	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
		LOG("HD647180X DDR%c wr $%02x\n", port - 0x70 + 'A', data);
		if ((data & ~m_ddr[port - 0x70]) != 0)
			m_port_output_cb[port - 0x70](0, m_odr[port - 0x70] | ~data, data);
		m_ddr[port - 0x70] = data;
		break;

	default:
		z180_device::z180_internal_port_write(port, data);
		break;
	}
}

void hd647180x_device::device_resolve_objects()
{
	for (auto &cb : m_port_input_cb)
		cb.resolve_safe(0xff);
	for (auto &cb : m_port_output_cb)
		cb.resolve_safe();
}

void hd647180x_device::device_start()
{
	z180_device::device_start();

	m_data = &space(AS_DATA);

	state_add(HD647180X_T2FRC, "T2FRC", m_t2frc.w);
	state_add(HD647180X_T2OCR1, "T2OCR1", m_t2ocr[0].w);
	state_add(HD647180X_T2OCR2, "T2OCR2", m_t2ocr[1].w);
	state_add(HD647180X_T2ICR, "T2ICR", m_t2icr.w);
	state_add(HD647180X_T2CSR1, "T2CSR1", m_t2csr[0]);
	state_add(HD647180X_T2CSR2, "T2CSR2", m_t2csr[1]).mask(0xef);
	state_add(HD647180X_CCSR, "CCSR", m_ccsr).mask(0xbf);
	state_add(HD647180X_RMCR, "RMCR", m_rmcr).mask(0xf0);
	state_add(HD647180X_DERA, "DERA", m_dera);
	for (int i = 0; i < 6; i++)
	{
		state_add(HD647180X_ODRA + i, string_format("ODR%c", i + 'A').c_str(), m_odr[i]);
		state_add(HD647180X_DDRA + i, string_format("DDR%c", i + 'A').c_str(), m_ddr[i]);
	}

	save_item(NAME(m_t2frc.w));
	save_item(NAME(m_t2ocr[0].w));
	save_item(NAME(m_t2ocr[1].w));
	save_item(NAME(m_t2icr.w));
	save_item(NAME(m_t2csr));
	save_item(NAME(m_ccsr));
	save_item(NAME(m_rmcr));
	save_item(NAME(m_dera));
	save_item(NAME(m_odr));
	save_item(NAME(m_ddr));
}

void hd647180x_device::device_reset()
{
	z180_device::device_reset();

	m_t2frc.w = 0;
	m_t2ocr[0].w = m_t2ocr[1].w = 0xffff;
	m_t2icr.w = 0;
	m_t2csr[0] = 0x00;
	m_t2csr[1] = 0x00;
	m_ccsr = (m_ccsr & 0x80) | 0x2c;
	m_rmcr = 0;
	m_dera = 0;
	std::fill(std::begin(m_ddr), std::end(m_ddr), 0);
}
