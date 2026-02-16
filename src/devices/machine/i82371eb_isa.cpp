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

void i82371eb_isa_device::device_add_mconfig(machine_config &config)
{
	i82371sb_isa_device::device_add_mconfig(config);

	// TODO: bump to DS12885EXT
//	DS12885EXT(config, m_rtc, XTAL(32'768));
	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set(m_pic8259_slave, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}

void i82371eb_isa_device::device_start()
{
	i82371sb_isa_device::device_start();

	save_item(NAME(m_serirqc));
	save_item(NAME(m_pdmacfg));
	save_item(NAME(m_pdma_ch));
	save_item(NAME(m_ddmabp));
	save_item(NAME(m_gencfg));
	save_item(NAME(m_rtccfg));
}

void i82371eb_isa_device::device_reset()
{
	i82371sb_isa_device::device_reset();

	m_serirqc = 0x10;
	std::fill(std::begin(m_pdmacfg), std::end(m_pdmacfg), 0);
	std::fill(std::begin(m_pdma_ch), std::end(m_pdma_ch), 0);
	std::fill(std::begin(m_ddmabp), std::end(m_ddmabp), 0);
	std::fill(std::begin(m_gencfg), std::end(m_gencfg), 0);
	m_rtccfg = 0x21;
}


void i82371eb_isa_device::config_map(address_map &map)
{
	i82371sb_isa_device::config_map(map);
//	map(0x4c, 0x4c) IORT (from PIIX3)
//	map(0x4e, 0x4f) XBCS (from PIIX3)
//	map(0x60, 0x63) PIRQC (from PIIX3)
	// Serial IRQ Control
	map(0x64, 0x64).lrw8(
		NAME([this] () { return m_serirqc; }),
		NAME([this] (offs_t offset, u8 data) {
			m_serirqc = data;
			LOGIO("SERIRQC %02x\n", data);
			LOGIO("\tIRQ Enable %d IRQ Mode Select %s Frame Size %d Start Frame Pulse Width %d\n"
				, BIT(data, 7)
				, BIT(data, 6) ? "Continuous" : "Quiet"
				// NOTE: PIIX4 allegedly supports 0100b mode only (frame size 17 + 4)
				, (data >> 2) & 0xf
				// NOTE: 11b <reserved>
				, (((data >> 0) & 3) * 2) + 4
			);
		})
	);
//	map(0x69, 0x69) TOM (from PIIX3)
//	map(0x6a, 0x6b) MSTAT (from PIIX3)
//	map(0x70, 0x71) MBDMA (from PIIX3)
//	map(0x78, 0x79).unmaprw();
//	map(0x80, 0x80) APICBASE (from PIIX3)
//	map(0x82, 0x82) DLC (from PIIX3)

	// PCI DMA Configuration
	map(0x90, 0x91).rw(FUNC(i82371eb_isa_device::pdmacfg_r), FUNC(i82371eb_isa_device::pdmacfg_w));
	// Distributed DMA Slave Base Pointer
	map(0x92, 0x93).lrw8(
		NAME([this] (offs_t offset) { return m_ddmabp[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_ddmabp[offset] = data;
			LOGIO("DDMABP%d (%s) %02x\n", offset, offset ? "CH5-7" : "CH0-3", data);
		})
	);
	// TODO: the SMI stuff is moved in ACPI core
//	map(0xa0, 0xaf).unmaprw();
	// General Configuration
	map(0xb0, 0xb3).lrw8(
		// TODO: bits 2 and 3 are r/o thru CONFIG pins
		NAME([this] (offs_t offset) { return m_gencfg[offset] | (1 << 2); }),
		NAME([this] (offs_t offset, u8 data) {
			m_gencfg[offset] = data;
			LOGIO("GENCFG%d %02x\n", offset, data);
			switch(offset)
			{
				case 0:
					LOGIO("\tPnP Address Decode %d Alt Access Mode %d IDE Signal %s Decode %s Select %s\n"
						, BIT(data, 6)
						, BIT(data, 5)
						, BIT(data, 4) ? "Primary 0/1" : "Primary/Secondary"
						, BIT(data, 1) ? "Positive" : "Subtractive"
						, BIT(data, 0) ? "ISA" : "EIO"
					);
					break;
				case 1:
					LOGIO("\t%s %s Secondary IDE %d Primary IDE %d %s %s %s\n"
						, BIT(data, 7) ? "GPI11" : "SMBALERT#"
						, BIT(data, 6) ? "IRQ8#" : "GPI6"
						//, BIT(data, 5) <reserved>
						, !BIT(data, 4)
						, !BIT(data, 3)
						, BIT(data, 2) ? "PCI REQC/GNTC" : "GPI4/GPO11"
						, BIT(data, 1) ? "REQB/GNTB" : "GPI3/GPO10"
						, BIT(data, 0) ? "REQA/GNTA" : "GPI2/GPO9"
					);
					break;
				case 2:
					LOGIO("\t%s %s %s %s %s %s %s %s\n"
						, BIT(data, 7) ? "GPI8" : "THRM#"
						, BIT(data, 6) ? "GPO21" : "SUS_STAT2#"
						, BIT(data, 5) ? "GPO20" : "SUS_STAT1#"
						, BIT(data, 4) ? "GPO19" : "ZZ"
						, BIT(data, 3) ? "PCI_STP#" : "GPO18"
						, BIT(data, 2) ? "GPO17" : "CPU_STP#"
						, BIT(data, 1) ? "GPO15/GPO16" : "SUSB#/SUSC#"
						, BIT(data, 0) ? "SERIRQ" : "GPI7"
					);
					break;
				case 3:
					LOGIO("\t%s %s %s %s %s %s %s\n"
						, BIT(data, 7) ? "GPO26" : "KBCCS#"
						, BIT(data, 6) ? "GPO25" : "RTCALE"
						, BIT(data, 5) ? "GPO2" : "RTCCS#"
						, BIT(data, 4) ? "GPO23/GPO22" : "XOE#/XDIR#"
						, BIT(data, 3) ? "GPI12" : "RI#"
						//, BIT(data, 2) <reserved>
						, BIT(data, 1) ? "GPI10" : "LID"
						, BIT(data, 0) ? "GPI9" : "BATLOW#"
					);
					break;
			}
		})
	);
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

u8 i82371eb_isa_device::pdmacfg_r(offs_t offset)
{
	return m_pdmacfg[offset];
}

void i82371eb_isa_device::pdmacfg_w(offs_t offset, u8 data)
{
	m_pdmacfg[offset] = data;
	LOGIO("PDMACFG%d %02x\n", offset, data);
	// optimize
	const u8 base = offset * 4;
	const char* dma_modes[] = {"Normal ISA DMA", "PC/PCI DMA", "Distributed DMA", "<reserved>"};

	// skip CH4 (reserved)
	for (int i = offset; i < 4; i++)
	{
		m_pdma_ch[i + base] = (data >> (i * 2)) & 3;
		LOGIO("\tCH%d %s\n", i + base, dma_modes[m_pdma_ch[i + base]]);
	}
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

	// NOTE: ISA don't care about PCI command bit 0

	// TODO: unknown enable for the internal RTC, use a config option for the time being
	// - savquest/pciagp/silvrball: uses the Super I/O one (activates dev[8])
	// - xtom3d uses some MB integrated non-DS12885 version
	// - comebaby/se440bx2/midqslvr.cpp has no RTC from Super I/O
	// - quakeat/ez2d assumed to use this version
	// - thinkpad uses a DS17485 MB resource
	if (m_rtccfg & 5 && m_has_internal_rtc)
	{
		// 11 $70-$73
		// 10 $72-$73
		// 01 $70-$71, with aliases at $72/$74/$76
		// 00 unmapped
		const bool base_rtc = !!BIT(m_rtccfg, 0);
		if (base_rtc)
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
		else if (base_rtc)
		{
			// comebaby BIOS checks often at $74
			for (u16 port = 0x72; port < 0x78; port += 2)
			{
				io_space->install_readwrite_handler(port | 0, port | 0,
					read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_r<0>)),
					write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_index_w<0>))
				);
				io_space->install_readwrite_handler(port | 1, port | 1,
					read8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_r<0>)),
					write8sm_delegate(*this, FUNC(i82371eb_isa_device::rtc_data_w<0>))
				);
			}
		}
	}
}
