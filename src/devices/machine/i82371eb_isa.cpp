// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

    PIIX4E ISA interface

    TODO:
    - i82371ab PIIX4 / i82371mb PIIX4M dispatches
    - pinpoint actual differences wrt i82371sb (definitely EISA, then ...?)

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_isa.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(I82371EB_ISA, i82371eb_isa_device, "i82371eb_isa", "Intel 82371EB PIIX4E PCI to ISA/EIO southbridge")

i82371eb_isa_device::i82371eb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82371sb_isa_device(mconfig, I82371EB_ISA, tag, owner, clock)
	, m_rtc(*this, "rtc")
{
	// 0x060100 - Bridge device, PCI-to-ISA bridge
	// TODO: above can change to 0x068000 if positive decode is used.
	// rev 0x00 PIIX4 A-0 / A-1
	// rev 0x01 PIIX4 B-0
	// rev 0x02 for PIIX4E A-0 / PIIX4M A-0
	set_ids(0x80867110, 0x02, 0x060100, 0x00);
}

void i82371eb_isa_device::device_start()
{
	i82371sb_isa_device::device_start();
	save_item(NAME(m_rtccfg));
}

void i82371eb_isa_device::device_reset()
{
	i82371sb_isa_device::device_reset();
	m_rtccfg = 0x21;
}

void i82371eb_isa_device::device_add_mconfig(machine_config &config)
{
	i82371sb_isa_device::device_add_mconfig(config);

	// TODO: should be a 256 bytes compatible MC146818B
	// however xtom3d crashes during startup, with rtccfg = 0x21 vs. 0x25 of midqslvr.cpp
//	DS12885EXT(config, m_rtc, XTAL(32'768));
	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set(m_pic8259_slave, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}

void i82371eb_isa_device::config_map(address_map &map)
{
	i82371sb_isa_device::config_map(map);
//  map(0x90, 0x91) PDMACFG
//  map(0xb0, 0xb0) GENCFG
	map(0xcb, 0xcb).lrw8(
		NAME([this] () { return m_rtccfg; }),
		NAME([this] (offs_t offset, u8 data) {
			LOGIO("RTCCFG %02x\n", data);
			m_rtccfg = data;
			remap_cb();
		})
	);
}

void i82371eb_isa_device::internal_io_map(address_map &map)
{
	i82371sb_isa_device::internal_io_map(map);
	map(0x00eb, 0x00eb).lw8(NAME([] (offs_t offset, u8 data) { }));
}

// TODO: readable thru RTCIREN in config register $ff
// cfr. Specification Update section 17 "RTC Index Register Read"
template <unsigned E> u8 i82371eb_isa_device::rtc_index_r(offs_t offset)
{
	u8 res = m_rtc_index;
	if (!E)
		res |= (m_nmi_enabled << 7);
	return res;
}

template <unsigned E> void i82371eb_isa_device::rtc_index_w(offs_t offset, u8 data)
{
	m_rtc_index = data & 0x7f;
	// TODO: documentation claims reversed meaning compared to any other southbridge
	// i.e. NMI enable with bit 7 = 0, mistake or really the way it is?
	if (!E)
		m_nmi_enabled = BIT(data, 7);
}

template <unsigned E> u8 i82371eb_isa_device::rtc_data_r(offs_t offset)
{
	const u8 rtc_address = m_rtc_index | (E << 7);
	return m_rtc->read_direct(rtc_address);
}

template <unsigned E> void i82371eb_isa_device::rtc_data_w(offs_t offset, u8 data)
{
	const u8 rtc_address = m_rtc_index | (E << 7);
	m_rtc->write_direct(rtc_address, data);
}


void i82371eb_isa_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	i82371sb_isa_device::map_extra(memory_window_start, memory_window_end, memory_offset, memory_space, io_window_start, io_window_end, io_offset, io_space);

	if (BIT(command, 0) && m_rtccfg & 5)
	{
		// 11 $70-$73
		// 10 $72-$73
		// 01 $70-$71, with aliases at $72/$74/$76
		// 00 unmapped
		if (BIT(m_rtccfg, 0))
		{
			io_space->install_readwrite_handler(0x70, 0x70,
				read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_r<0>)),
				write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_w<0>))
			);
			io_space->install_readwrite_handler(0x71, 0x71,
				read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_r<0>)),
				write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_w<0>))
			);
		}

		// TODO: as above
		if (BIT(m_rtccfg, 2))
		{
			//io_space->install_readwrite_handler(0x72, 0x72,
			//	read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_r<1>)),
			//	write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_w<1>))
			//);
			//io_space->install_readwrite_handler(0x73, 0x73,
			//	read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_r<1>)),
			//	write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_w<1>))
			//);
		}
	}
}
