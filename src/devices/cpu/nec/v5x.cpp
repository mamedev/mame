// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NEC V5x devices consist of a V3x CPU core plus integrated peripherals. The
 * CPU cores within each device are as follows:
 *
 *   Device            CPU
 *   V40 (µPD70208)    V20 (µPD70108)
 *   V50 (µPD70216)    V30 (µPD70116)
 *   V53 (µPD70236)    V33 (µPD70136)
 *   V53A (µPD70236A)  V33A (µPD70136A)
 *
 * The peripherals are nearly identical between all four devices:
 *
 *   Name  Description             Device
 *   TCU   Timer/Counter Unit      µPD71054/i8254 subset
 *   DMAU  DMA Control Unit        µPD71071 equivalent
 *   ICU   Interrupt control Unit  µPD71059/i8259 equivalent
 *   SCU   Serial Control Unit     µPD71051/i8251 subset (async only)
 *
 * The V53/V53A DMAU also supports a configurable µPD71037/i8237A mode.
 *
 * Sources:
 *
 *   http://www.chipfind.net/datasheet/pdf/nec/upd70236.pdf
 *   https://datasheet.datasheetarchive.com/originals/scans/Scans-107/DSASCANS15-59637.pdf
 *
 */
#include "emu.h"
#include "v5x.h"

#include "necpriv.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(V40,  v40_device,  "v40",  "NEC V40")
DEFINE_DEVICE_TYPE(V50,  v50_device,  "v50",  "NEC V50")
DEFINE_DEVICE_TYPE(V53,  v53_device,  "v53",  "NEC V53")
DEFINE_DEVICE_TYPE(V53A, v53a_device, "v53a", "NEC V53A")

void device_v5x_interface::SULA_w(u8 data)
{
	if (VERBOSE)
		device().logerror("SULA_w %02x\n", data);
	m_SULA = data;
	install_peripheral_io();
}

void device_v5x_interface::TULA_w(u8 data)
{
	if (VERBOSE)
		device().logerror("TULA_w %02x\n", data);
	m_TULA = data;
	install_peripheral_io();
}

void device_v5x_interface::IULA_w(u8 data)
{
	if (VERBOSE)
		device().logerror("IULA_w %02x\n", data);
	m_IULA = data;
	install_peripheral_io();
}

void device_v5x_interface::DULA_w(u8 data)
{
	if (VERBOSE)
		device().logerror("DULA_w %02x\n", data);
	m_DULA = data;
	install_peripheral_io();
}

void device_v5x_interface::OPHA_w(u8 data)
{
	if (VERBOSE)
	{
		device().logerror("OPHA_w %02x\n", data);
		if (data == 0xff)
			device().logerror("OPHA is mapped in system IO area!\n", data);
	}
	m_OPHA = data;
}

void device_v5x_interface::OPSEL_w(u8 data)
{
	if (VERBOSE)
		device().logerror("OPSEL_w %02x\n", data);
	m_OPSEL = data;
	install_peripheral_io();
}

void device_v5x_interface::TCKS_w(u8 data)
{
	m_TCKS = data;
	tcu_clock_update();
}

void device_v5x_interface::interface_clock_changed()
{
	tcu_clock_update();
}

void device_v5x_interface::tcu_clock_update()
{
	for (int i = 0; i < 3; i++)
		m_tcu->set_clockin(i, BIT(m_TCKS, i + 2) ? m_tclk : device().clock() / double(4 << (m_TCKS & 3)));
}

WRITE_LINE_MEMBER(device_v5x_interface::tclk_w)
{
	if (BIT(m_TCKS, 2))
		m_tcu->write_clk0(state);
	if (BIT(m_TCKS, 3))
		m_tcu->write_clk1(state);
	if (BIT(m_TCKS, 4))
		m_tcu->write_clk2(state);
}

void device_v5x_interface::interface_pre_reset()
{
	m_OPSEL= 0x00;

	// peripheral addresses
	m_SULA = 0x00;
	m_TULA = 0x00;
	m_IULA = 0x00;
	m_DULA = 0x00;
	m_OPHA = 0x00;

	m_TCKS = 0x00;
	tcu_clock_update();
}

void device_v5x_interface::interface_post_start()
{
	device().save_item(NAME(m_OPSEL));
	device().save_item(NAME(m_SULA));
	device().save_item(NAME(m_TULA));
	device().save_item(NAME(m_IULA));
	device().save_item(NAME(m_DULA));
	device().save_item(NAME(m_OPHA));
	device().save_item(NAME(m_TCKS));
}

void device_v5x_interface::interface_post_load()
{
	install_peripheral_io();
}

// the external interface provides no external access to the usual IRQ line of the V33, everything goes through the interrupt controller
void device_v5x_interface::v5x_set_input(int irqline, int state)
{
	switch (irqline)
	{
		case INPUT_LINE_IRQ0: m_icu->ir0_w(state); break;
		case INPUT_LINE_IRQ1: m_icu->ir1_w(state); break;
		case INPUT_LINE_IRQ2: m_icu->ir2_w(state); break;
		case INPUT_LINE_IRQ3: m_icu->ir3_w(state); break;
		case INPUT_LINE_IRQ4: m_icu->ir4_w(state); break;
		case INPUT_LINE_IRQ5: m_icu->ir5_w(state); break;
		case INPUT_LINE_IRQ6: m_icu->ir6_w(state); break;
		case INPUT_LINE_IRQ7: m_icu->ir7_w(state); break;

		case INPUT_LINE_NMI: downcast<nec_common_device &>(device()).set_nmi_line(state); break;
		case NEC_INPUT_LINE_POLL: downcast<nec_common_device &>(device()).set_poll_line(state); break;
	}
}

// for hooking the interrupt controller output up to the core
WRITE_LINE_MEMBER(device_v5x_interface::internal_irq_w)
{
	downcast<nec_common_device &>(device()).set_int_line(state);
}

void device_v5x_interface::v5x_add_mconfig(machine_config &config)
{
	PIT8254(config, m_tcu);

	V5X_DMAU(config, m_dmau, 4000000);

	V5X_ICU(config, m_icu, 0);
	m_icu->out_int_callback().set(FUNC(device_v5x_interface::internal_irq_w));
	m_icu->in_sp_callback().set_constant(1);
	m_icu->read_slave_ack_callback().set(FUNC(device_v5x_interface::get_pic_ack));

	V5X_SCU(config, m_scu, 0);
}

void device_v5x_interface::remappable_io_map(address_map &map)
{
	map(0, INTERNAL_IO_ADDR_MASK).rw(FUNC(device_v5x_interface::temp_io_byte_r), FUNC(device_v5x_interface::temp_io_byte_w));
}

device_v5x_interface::device_v5x_interface(const machine_config &mconfig, nec_common_device &device, bool is_16bit)
	: device_interface(device, "v5x")
	, m_tcu(device, "tcu")
	, m_dmau(device, "dmau")
	, m_icu(device, "icu")
	, m_scu(device, "scu")
	, m_internal_io_config("internal_io", ENDIANNESS_LITTLE, is_16bit ? 16 : 8, INTERNAL_IO_ADDR_WIDTH, 0, address_map_constructor(FUNC(device_v5x_interface::remappable_io_map), this))
	, m_tclk(0.0)
	, m_OPSEL(0)
	, m_SULA(0)
	, m_TULA(0)
	, m_IULA(0)
	, m_DULA(0)
	, m_OPHA(0)
	, m_TCKS(0)
{
}


u8 v50_base_device::io_read_byte(offs_t a)
{
	if (check_OPHA(a))
		return device_v5x_interface::internal_io_read_byte(a);
	else
		return nec_common_device::io_read_byte(a);
}

u16 v50_base_device::io_read_word(offs_t a)
{
	if (check_OPHA(a))
	{
		if ((a & INTERNAL_IO_ADDR_MASK) == INTERNAL_IO_ADDR_MASK)
		{
			return (device_v5x_interface::internal_io_read_byte(a) & 0x00ff)
				| ((nec_common_device::io_read_byte(a + 1) << 8) & 0xff00);
		}
		else
			return device_v5x_interface::internal_io_read_word(a);
	}
	else
		return nec_common_device::io_read_word(a);
}

void v50_base_device::io_write_byte(offs_t a, u8 v)
{
	if (check_OPHA(a))
	{
		device_v5x_interface::internal_io_write_byte(a, v);
	}
	else
		nec_common_device::io_write_byte(a, v);
}

void v50_base_device::io_write_word(offs_t a, u16 v)
{
	if (check_OPHA(a))
	{
		if ((a & INTERNAL_IO_ADDR_MASK) == INTERNAL_IO_ADDR_MASK)
		{
			device_v5x_interface::internal_io_write_byte(a, v & 0xff);
			nec_common_device::io_write_byte(a + 1, (v >> 8) & 0xff);
		}
		else
		{
			device_v5x_interface::internal_io_write_word(a, v);
		}
	}
	else
		nec_common_device::io_write_word(a, v);
}


void v50_base_device::OPCN_w(u8 data)
{
	// bit 7: unused
	// bit 6: unused
	// bit 5: unused
	// bit 4: unused
	// bit 3: IRSW
	// bit 2: IRSW
	// bit 1: PF
	// bit 0: PF

	LOG("OPCN_w %02x\n", data);
	m_OPCN = data;
	install_peripheral_io();
}

void v50_base_device::device_reset()
{
	nec_common_device::device_reset();

	m_OPCN = 0;
}

void v50_base_device::device_start()
{
	nec_common_device::device_start();
	m_internal_io = &space(AS_INTERNAL_IO);

	set_irq_acknowledge_callback(*m_icu, FUNC(v5x_icu_device::inta_cb));

	save_item(NAME(m_OPCN));
}

void v40_device::install_peripheral_io()
{
	// unmap everything in I/O space up to the fixed position registers (we avoid overwriting them, it isn't a valid config)
	space(AS_INTERNAL_IO).unmap_readwrite(0, INTERNAL_IO_ADDR_MASK);
	space(AS_INTERNAL_IO).install_readwrite_handler(0, INTERNAL_IO_ADDR_MASK,
		read8sm_delegate(*this, FUNC(v40_device::temp_io_byte_r)),
		write8sm_delegate(*this, FUNC(v40_device::temp_io_byte_w)));

	if (m_OPSEL & OPSEL_DS)
	{
		u16 const base = m_DULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x0f, base | 0x0f);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x0f, base | 0x0f,
			read8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::read)),
			write8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::write)));
	}

	if (m_OPSEL & OPSEL_IS)
	{
		u16 const base = m_IULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x01, base | 0x01);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x01, base | 0x01,
			read8sm_delegate(*m_icu, FUNC(v5x_icu_device::read)),
			write8sm_delegate(*m_icu, FUNC(v5x_icu_device::write)));
	}

	if (m_OPSEL & OPSEL_TS)
	{
		u16 const base = m_TULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
			read8sm_delegate(*m_tcu, FUNC(pit8253_device::read)),
			write8sm_delegate(*m_tcu, FUNC(pit8253_device::write)));
	}

	if (m_OPSEL & OPSEL_SS)
	{
		u16 const base = m_SULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
			read8sm_delegate(*m_scu, FUNC(v5x_scu_device::read)),
			write8sm_delegate(*m_scu, FUNC(v5x_scu_device::write)));
	}
}

void v50_device::install_peripheral_io()
{
	// unmap everything in I/O space up to the fixed position registers (we avoid overwriting them, it isn't a valid config)
	space(AS_INTERNAL_IO).unmap_readwrite(0, INTERNAL_IO_ADDR_MASK);
	space(AS_INTERNAL_IO).install_readwrite_handler(0, INTERNAL_IO_ADDR_MASK,
		read8sm_delegate(*this, FUNC(v50_device::temp_io_byte_r)),
		write8sm_delegate(*this, FUNC(v50_device::temp_io_byte_w)));

	if (m_OPSEL & OPSEL_DS)
	{
		u16 const base = m_DULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x0f, base | 0x0f);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x0f, base | 0x0f,
			read8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::read)),
			write8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::write)), 0xffff);
	}

	if (m_OPSEL & OPSEL_IS)
	{
		u16 const base = m_IULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
			read8sm_delegate(*m_icu, FUNC(v5x_icu_device::read)),
			write8sm_delegate(*m_icu, FUNC(v5x_icu_device::write)), io_mask(base));
	}

	if (m_OPSEL & OPSEL_TS)
	{
		u16 const base = m_TULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x07, base | 0x07);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x07, base | 0x07,
			read8sm_delegate(*m_tcu, FUNC(pit8253_device::read)),
			write8sm_delegate(*m_tcu, FUNC(pit8253_device::write)), io_mask(base));
	}

	if (m_OPSEL & OPSEL_SS)
	{
		u16 const base = m_SULA & INTERNAL_IO_ADDR_MASK;

		space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x07, base | 0x07);
		space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x07, base | 0x07,
			read8sm_delegate(*m_scu, FUNC(v5x_scu_device::read)),
			write8sm_delegate(*m_scu, FUNC(v5x_scu_device::write)), io_mask(base));
	}
}

void v50_base_device::internal_port_map(address_map &map)
{
	map(0xfff0, 0xfff0).w(FUNC(v50_base_device::TCKS_w));

	map(0xfff2, 0xfff2).w(FUNC(v50_base_device::RFC_w));

	map(0xfff4, 0xfff4).w(FUNC(v50_base_device::WMB0_w)); // actually WMB on V50
	map(0xfff5, 0xfff5).w(FUNC(v50_base_device::WCY1_w));
	map(0xfff6, 0xfff6).w(FUNC(v50_base_device::WCY2_w));

	map(0xfff8, 0xfff8).w(FUNC(v50_base_device::SULA_w));
	map(0xfff9, 0xfff9).w(FUNC(v50_base_device::TULA_w));
	map(0xfffa, 0xfffa).w(FUNC(v50_base_device::IULA_w));
	map(0xfffb, 0xfffb).w(FUNC(v50_base_device::DULA_w));
	map(0xfffc, 0xfffc).w(FUNC(v50_base_device::OPHA_w));
	map(0xfffd, 0xfffd).w(FUNC(v50_base_device::OPSEL_w));
	map(0xfffe, 0xfffe).w(FUNC(v50_base_device::OPCN_w));
}

void v50_base_device::execute_set_input(int irqline, int state)
{
	v5x_set_input(irqline, state);
}

void v50_base_device::device_add_mconfig(machine_config &config)
{
	v5x_add_mconfig(config);

	// V50 timer 0 is internally connected to INT0
	m_tcu->out_handler<0>().set(m_icu, FUNC(pic8259_device::ir0_w));
}

device_memory_interface::space_config_vector v50_base_device::memory_space_config() const
{
	space_config_vector spaces = {
			std::make_pair(AS_PROGRAM,     &m_program_config),
			std::make_pair(AS_IO,          &m_io_config),
			std::make_pair(AS_INTERNAL_IO, &m_internal_io_config)
		};
	return spaces;
}

v50_base_device::v50_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool is_16bit, u8 prefetch_size, u8 prefetch_cycles, u32 chip_type)
	: nec_common_device(mconfig, type, tag, owner, clock, is_16bit, prefetch_size, prefetch_cycles, chip_type, address_map_constructor(FUNC(v50_base_device::internal_port_map), this))
	, device_v5x_interface(mconfig, *this, is_16bit)
{
}

v40_device::v40_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: v50_base_device(mconfig, V40, tag, owner, clock, false, 4, 4, V20_TYPE)
{
}

v50_device::v50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: v50_base_device(mconfig, V50, tag, owner, clock, true, 6, 2, V30_TYPE)
{
}

u8 v53_device::io_read_byte(offs_t a)
{
	if (check_OPHA(a))
		return device_v5x_interface::internal_io_read_byte(a);
	else
		return nec_common_device::io_read_byte(a);
}

u16 v53_device::io_read_word(offs_t a)
{
	if (check_OPHA(a))
	{
		if ((a & INTERNAL_IO_ADDR_MASK) == INTERNAL_IO_ADDR_MASK)
		{
			return (device_v5x_interface::internal_io_read_byte(a) & 0x00ff)
				| ((nec_common_device::io_read_byte(a + 1) << 8) & 0xff00);
		}
		else
			return device_v5x_interface::internal_io_read_word(a);
	}
	else
		return nec_common_device::io_read_word(a);
}

void v53_device::io_write_byte(offs_t a, u8 v)
{
	if (check_OPHA(a))
	{
		device_v5x_interface::internal_io_write_byte(a, v);
	}
	else
		nec_common_device::io_write_byte(a, v);
}

void v53_device::io_write_word(offs_t a, u16 v)
{
	if (check_OPHA(a))
	{
		if ((a & INTERNAL_IO_ADDR_MASK) == INTERNAL_IO_ADDR_MASK)
		{
			device_v5x_interface::internal_io_write_byte(a, v & 0xff);
			nec_common_device::io_write_byte(a + 1, (v >> 8) & 0xff);
		}
		else
		{
			device_v5x_interface::internal_io_write_word(a, v);
		}
	}
	else
		nec_common_device::io_write_word(a, v);
}


void v53_device::SCTL_w(u8 data)
{
	// bit 7: unused
	// bit 6: unused
	// bit 5: unused
	// bit 4: SCU input clock source
	// bit 3: uPD71037 DMA mode - Carry A20
	// bit 2: uPD71037 DMA mode - Carry A16
	// bit 1: uPD71037 DMA mode enable (otherwise in uPD71071 mode)
	// bit 0: Onboard pripheral I/O maps to 8-bit boundaries? (otherwise 16-bit)

	LOG("SCTL_w %02x\n", data);
	m_SCTL = data;
	install_peripheral_io();
}

void v53_device::device_reset()
{
	v33_base_device::device_reset();

	m_SCTL = 0x00;
}

void v53_device::device_start()
{
	v33_base_device::device_start();
	m_internal_io = &space(AS_INTERNAL_IO);

	set_irq_acknowledge_callback(*m_icu, FUNC(v5x_icu_device::inta_cb));

	save_item(NAME(m_SCTL));
}

void v53_device::install_peripheral_io()
{
	// unmap everything in I/O space up to the fixed position registers (we avoid overwriting them, it isn't a valid config)
	space(AS_INTERNAL_IO).unmap_readwrite(0, INTERNAL_IO_ADDR_MASK);
	space(AS_INTERNAL_IO).install_readwrite_handler(0, INTERNAL_IO_ADDR_MASK,
		read8sm_delegate(*this, FUNC(v53_device::temp_io_byte_r)),
		write8sm_delegate(*this, FUNC(v53_device::temp_io_byte_w)));

	// IOAG determines if the handlers used 8-bit or 16-bit access
	// the hng64.cpp games first set everything up in 8-bit mode, then
	// do the procedure again in 16-bit mode before using them?!

	bool const IOAG = m_SCTL & 1;

	if (m_OPSEL & OPSEL_DS)
	{
		u16 const base = m_DULA & INTERNAL_IO_ADDR_MASK;

		if (m_SCTL & 0x02) // uPD71037 mode
		{
			if (IOAG) // 8-bit
			{
				space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x0f, base | 0x0f);
			}
			else
			{
				space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x1f, base | 0x1f);
			}
		}
		else // uPD71071 mode
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x0f, base | 0x0f);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x0f, base | 0x0f,
				read8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::read)),
				write8sm_delegate(*m_dmau, FUNC(v5x_dmau_device::write)), 0xffff);
		}
	}

	if (m_OPSEL & OPSEL_IS)
	{
		u16 const base = m_IULA & INTERNAL_IO_ADDR_MASK;

		if (IOAG) // 8-bit
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x01, base | 0x01);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x01, base | 0x01,
				read8sm_delegate(*m_icu, FUNC(v5x_icu_device::read)),
				write8sm_delegate(*m_icu, FUNC(v5x_icu_device::write)), 0xffff);
		}
		else
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
				read8sm_delegate(*m_icu, FUNC(v5x_icu_device::read)),
				write8sm_delegate(*m_icu, FUNC(v5x_icu_device::write)), io_mask(base));
		}
	}

	if (m_OPSEL & OPSEL_TS)
	{
		u16 const base = m_TULA & INTERNAL_IO_ADDR_MASK;

		if (IOAG) // 8-bit
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
				read8sm_delegate(*m_tcu, FUNC(pit8253_device::read)),
				write8sm_delegate(*m_tcu, FUNC(pit8253_device::write)), 0xffff);
		}
		else
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x07, base | 0x07);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x07, base | 0x07,
				read8sm_delegate(*m_tcu, FUNC(pit8253_device::read)),
				write8sm_delegate(*m_tcu, FUNC(pit8253_device::write)), io_mask(base));
		}
	}

	if (m_OPSEL & OPSEL_SS)
	{
		u16 const base = m_SULA & INTERNAL_IO_ADDR_MASK;

		if (IOAG) // 8-bit
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x03, base | 0x03);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x03, base | 0x03,
				read8sm_delegate(*m_scu, FUNC(v5x_scu_device::read)),
				write8sm_delegate(*m_scu, FUNC(v5x_scu_device::write)), 0xffff);
		}
		else
		{
			space(AS_INTERNAL_IO).unmap_readwrite(base & ~0x07, base | 0x07);
			space(AS_INTERNAL_IO).install_readwrite_handler(base & ~0x07, base | 0x07,
				read8sm_delegate(*m_scu, FUNC(v5x_scu_device::read)),
				write8sm_delegate(*m_scu, FUNC(v5x_scu_device::write)), io_mask(base));
		}
	}
}

WRITE_LINE_MEMBER(v53_device::hack_w)
{
	if (!(m_SCTL & 0x02))
		m_dmau->hack_w(state);
	else
		LOG("hack_w not in 71071mode\n");
}

void v53_device::internal_port_map(address_map &map)
{
	v33_internal_port_map(map);

	map(0xffe0, 0xffe0).w(FUNC(v53_device::BSEL_w));  // uPD71037 DMA mode bank selection register
	map(0xffe1, 0xffe1).w(FUNC(v53_device::BADR_w));  // uPD71037 DMA mode bank register peripheral mapping (also uses OPHA)
	// 0xffe2-0xffe9 reserved
	map(0xffe9, 0xffe9).w(FUNC(v53_device::BRC_w));   // baud rate counter (used for serial peripheral)
	map(0xffea, 0xffea).w(FUNC(v53_device::WMB0_w));  // waitstate control
	map(0xffeb, 0xffeb).w(FUNC(v53_device::WCY1_w));  // waitstate control
	map(0xffec, 0xffec).w(FUNC(v53_device::WCY0_w));  // waitstate control
	map(0xffed, 0xffed).w(FUNC(v53_device::WAC_w));   // waitstate control
	// 0xffee-0xffef reserved
	map(0xfff0, 0xfff0).w(FUNC(v53_device::TCKS_w));  // timer clocks
	map(0xfff1, 0xfff1).w(FUNC(v53_device::SBCR_w));  // internal clock divider, halt behavior etc.
	map(0xfff2, 0xfff2).w(FUNC(v53_device::RFC_w));   // ram refresh control
	map(0xfff3, 0xfff3).w(FUNC(v53_device::WMB1_w));  // waitstate control
	map(0xfff4, 0xfff4).w(FUNC(v53_device::WCY2_w));  // waitstate control
	map(0xfff5, 0xfff5).w(FUNC(v53_device::WCY3_w));  // waitstate control
	map(0xfff6, 0xfff6).w(FUNC(v53_device::WCY4_w));  // waitstate control
	// 0xfff6 reserved
	map(0xfff8, 0xfff8).w(FUNC(v53_device::SULA_w));  // scu mapping
	map(0xfff9, 0xfff9).w(FUNC(v53_device::TULA_w));  // tcu mapping
	map(0xfffa, 0xfffa).w(FUNC(v53_device::IULA_w));  // icu mapping
	map(0xfffb, 0xfffb).w(FUNC(v53_device::DULA_w));  // dmau mapping
	map(0xfffc, 0xfffc).w(FUNC(v53_device::OPHA_w));  // peripheral mapping (upper bits, common)
	map(0xfffd, 0xfffd).w(FUNC(v53_device::OPSEL_w)); // peripheral enabling
	map(0xfffe, 0xfffe).w(FUNC(v53_device::SCTL_w));  // peripheral configuration (& byte / word mapping)
	// 0xffff reserved
}

void v53_device::execute_set_input(int irqline, int state)
{
	v5x_set_input(irqline, state);
}

void v53_device::device_add_mconfig(machine_config &config)
{
	v5x_add_mconfig(config);
}

device_memory_interface::space_config_vector v53_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM,     &m_program_config),
		std::make_pair(AS_IO,          &m_io_config),
		std::make_pair(AS_INTERNAL_IO, &m_internal_io_config)
	};
}

v53_device::v53_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: v33_base_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(v53_device::internal_port_map), this))
	, device_v5x_interface(mconfig, *this, true)
{
}

v53_device::v53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: v53_device(mconfig, V53, tag, owner, clock)
{
}

v53a_device::v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: v53_device(mconfig, V53A, tag, owner, clock)
{
}
