// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Intel 82378ZB (SIO) & 82379AB (SIO.A) southbridges

Has a X-Bus device that maps the various optional devices in place of a Super I/O.
ECSADDR[2:0] connects thru two 74F138 decoders, page 112~115 diagrams for x86 based systems.
Flash BIOS, keyboard, RTC, IDE, FDC, LPT, COMs and configuration RAM maps there

BeBox likely maps just a subset of this given that it has an actual Super I/O (i82091aa)
for LPT, COMs and FDC. This is translated in clients to be responsible about mapping the
individual devices, as sub-device of this.

**************************************************************************************************/

#include "emu.h"
#include "i82378zb_sio.h"

#include "speaker.h"

#define LOG_IRQ      (1U << 1) // log line state

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#define LOGIRQ(...)    LOGMASKED(LOG_IRQ, __VA_ARGS__)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82378ZB_SIO, i82378zb_sio_device, "i82378zb_sio", "Intel 82378ZB System I/O (SIO)")
//DEFINE_DEVICE_TYPE(I82379AB_SIO, i82379ab_sio_device, "i82379ab_sio", "Intel 82379AB System I/O (SIO.A)")


i82378zb_sio_device::i82378zb_sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_pic(*this, "pic%u", 0U)
	, m_dma(*this, "dma%u", 0U)
	, m_pit(*this, "pit")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
	, m_xbus_flash(*this, "xbus_flash")
	, m_xbus_keybc(*this, "xbus_keybc")
//  , m_xbus_rtc(*this, "xbus_rtc")
	, m_xbus_ide(*this, "xbus_ide%u", 0U)
	, m_boot_state_hook(*this)
	, m_write_a20m(*this)
	, m_write_cpureset(*this)
//  , m_write_pcirst(*this)
	, m_rtcale(*this)
	, m_rtccs_read(*this, 0xff)
	, m_rtccs_write(*this)
{
}

i82378zb_sio_device::i82378zb_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82378zb_sio_device(mconfig, I82378ZB_SIO, tag, owner, clock)
{
	// 0x00 for 82378IB (as per PCI.exe)
	// 0x03 for 82378ZB A0-Stepping
	// 0x88 for 82379AB A0-Stepping
	// No class code
	set_ids(0x80860484, 0x03, 0x000000, 0x00000000);
}

void i82378zb_sio_device::device_add_mconfig(machine_config &config)
{
	// 82C54
	PIT8254(config, m_pit);
	m_pit->set_clk<0>(4772720 / 4); // heartbeat IRQ
	m_pit->out_handler<0>().set(FUNC(i82378zb_sio_device::at_pit8254_out0_changed));
	m_pit->set_clk<1>(4772720 / 4); // DRAM refresh
	m_pit->out_handler<1>().set(FUNC(i82378zb_sio_device::at_pit8254_out1_changed));
	m_pit->set_clk<2>(4772720 / 4); // PIO port C pin 4, and speaker polling enough
	m_pit->out_handler<2>().set(FUNC(i82378zb_sio_device::at_pit8254_out2_changed));

	// 82C37A
	AM9517A(config, m_dma[0], XTAL(14'318'181) / 3);
	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
	m_dma[0]->out_eop_callback().set(FUNC(i82378zb_sio_device::at_dma8237_out_eop));
	m_dma[0]->in_memr_callback().set(FUNC(i82378zb_sio_device::pc_dma_read_byte));
	m_dma[0]->out_memw_callback().set(FUNC(i82378zb_sio_device::pc_dma_write_byte));
	m_dma[0]->in_ior_callback<0>().set(FUNC(i82378zb_sio_device::pc_dma8237_0_dack_r));
	m_dma[0]->in_ior_callback<1>().set(FUNC(i82378zb_sio_device::pc_dma8237_1_dack_r));
	m_dma[0]->in_ior_callback<2>().set(FUNC(i82378zb_sio_device::pc_dma8237_2_dack_r));
	m_dma[0]->in_ior_callback<3>().set(FUNC(i82378zb_sio_device::pc_dma8237_3_dack_r));
	m_dma[0]->out_iow_callback<0>().set(FUNC(i82378zb_sio_device::pc_dma8237_0_dack_w));
	m_dma[0]->out_iow_callback<1>().set(FUNC(i82378zb_sio_device::pc_dma8237_1_dack_w));
	m_dma[0]->out_iow_callback<2>().set(FUNC(i82378zb_sio_device::pc_dma8237_2_dack_w));
	m_dma[0]->out_iow_callback<3>().set(FUNC(i82378zb_sio_device::pc_dma8237_3_dack_w));
	m_dma[0]->out_dack_callback<0>().set(FUNC(i82378zb_sio_device::pc_dack0_w));
	m_dma[0]->out_dack_callback<1>().set(FUNC(i82378zb_sio_device::pc_dack1_w));
	m_dma[0]->out_dack_callback<2>().set(FUNC(i82378zb_sio_device::pc_dack2_w));
	m_dma[0]->out_dack_callback<3>().set(FUNC(i82378zb_sio_device::pc_dack3_w));

	AM9517A(config, m_dma[1], XTAL(14'318'181) / 3);
	m_dma[1]->out_hreq_callback().set(FUNC(i82378zb_sio_device::pc_dma_hrq_changed));
	m_dma[1]->in_memr_callback().set(FUNC(i82378zb_sio_device::pc_dma_read_word));
	m_dma[1]->out_memw_callback().set(FUNC(i82378zb_sio_device::pc_dma_write_word));
	m_dma[1]->in_ior_callback<1>().set(FUNC(i82378zb_sio_device::pc_dma8237_5_dack_r));
	m_dma[1]->in_ior_callback<2>().set(FUNC(i82378zb_sio_device::pc_dma8237_6_dack_r));
	m_dma[1]->in_ior_callback<3>().set(FUNC(i82378zb_sio_device::pc_dma8237_7_dack_r));
	m_dma[1]->out_iow_callback<1>().set(FUNC(i82378zb_sio_device::pc_dma8237_5_dack_w));
	m_dma[1]->out_iow_callback<2>().set(FUNC(i82378zb_sio_device::pc_dma8237_6_dack_w));
	m_dma[1]->out_iow_callback<3>().set(FUNC(i82378zb_sio_device::pc_dma8237_7_dack_w));
	m_dma[1]->out_dack_callback<0>().set(FUNC(i82378zb_sio_device::pc_dack4_w));
	m_dma[1]->out_dack_callback<1>().set(FUNC(i82378zb_sio_device::pc_dack5_w));
	m_dma[1]->out_dack_callback<2>().set(FUNC(i82378zb_sio_device::pc_dack6_w));
	m_dma[1]->out_dack_callback<3>().set(FUNC(i82378zb_sio_device::pc_dack7_w));

	// 8259A-equivalent
	PIC8259(config, m_pic[0]);
	m_pic[0]->out_int_callback().set_inputline(m_host_cpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(i82378zb_sio_device::get_slave_ack));

	PIC8259(config, m_pic[1]);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	// TODO: ISA bus runs at 6 to 8.33 MHz
	ISA16(config, m_isabus);
	m_isabus->irq3_callback().set(FUNC(i82378zb_sio_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(i82378zb_sio_device::pc_irq4_w));
	m_isabus->irq5_callback().set(FUNC(i82378zb_sio_device::pc_irq5_w));
	m_isabus->irq6_callback().set(FUNC(i82378zb_sio_device::pc_irq6_w));
	m_isabus->irq7_callback().set(FUNC(i82378zb_sio_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(i82378zb_sio_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(i82378zb_sio_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(i82378zb_sio_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(i82378zb_sio_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(i82378zb_sio_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(i82378zb_sio_device::pc_irq15_w));
	m_isabus->drq0_callback().set(m_dma[0], FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma[0], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma[0], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma[0], FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma[1], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma[1], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma[1], FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(i82378zb_sio_device::iochck_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void i82378zb_sio_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_host_cpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_host_cpu, AS_IO);

	pci_device::device_config_complete();
}

void i82378zb_sio_device::device_start()
{
	pci_device::device_start();

//  m_pci_root->set_pin_mapper(pci_pin_mapper(*this, FUNC(i82378zb_sio_device::pin_mapper)));
//  m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(i82378zb_sio_device::irq_handler)));

	save_item(NAME(m_pcicon));
	save_item(NAME(m_pac));
	save_item(NAME(m_papc));
	save_item(NAME(m_arbprix));
	save_item(NAME(m_mcscon));
	save_item(NAME(m_mcsboh));
	save_item(NAME(m_mcstoh));
	save_item(NAME(m_mcstom));
	save_item(NAME(m_iadcon));
	save_item(NAME(m_iadrbe));
	save_item(NAME(m_iadboh));
	save_item(NAME(m_iadtoh));
	save_item(NAME(m_icrt));
	save_item(NAME(m_icd));
	save_item(NAME(m_ubcsa));
	save_item(NAME(m_ubcsb));
	save_item(NAME(m_mar));
	save_item(NAME(m_pirq));
	save_item(NAME(m_bios_timer_base));
	save_item(NAME(m_smicntl));
	save_item(NAME(m_smien));
	save_item(NAME(m_see));
	save_item(NAME(m_ftmr));
	save_item(NAME(m_smireq));
	save_item(NAME(m_ctltmr));
	save_item(NAME(m_ctltmrh));

	save_item(NAME(m_port92));

	save_item(NAME(m_ext_gatea20));
	save_item(NAME(m_fast_gatea20));

	// X-Bus config optimizers, for checking presence of objects
	m_has_xbus.keyboard = m_xbus_keybc != nullptr;
	m_has_xbus.flash_bios = m_xbus_flash != nullptr;
	// we expect both IDE slots to be filled for now (X-Bus decodes from 1 bit alone)
	m_has_xbus.ide = m_xbus_ide[0] != nullptr && m_xbus_ide[1] != nullptr;

	LOG("X-Bus configuration:\n");
	LOG("\tKeyboard %d\n", m_has_xbus.keyboard);
	LOG("\tFlash BIOS %d\n", m_has_xbus.flash_bios);
	LOG("\tIDE %d\n", m_has_xbus.ide);
}

void i82378zb_sio_device::device_reset()
{
	pci_device::device_reset();

	// supports Special Cycle Enable, hardwires bits 2-0 to '1'
	// (Bus Master, Memory and I/O)
	command = 0x0007;
	command_mask = 0x0008;
	// medium DEVSEL#
	// Supports Master-Abort, Received Target-Abort and Signaled Target-Abort (r/o)
	// statuses, bits 13-11
	status = 0x0200;

	m_pcicon = 0x20;
	m_pac = 0x00;
	m_papc = 0x04;
	m_arbprix = 0;
	m_mcscon = 0x00;
	m_mcsboh = 0x10;
	m_mcstoh = 0x0f;
	m_mcstom = 0x00;
	m_iadcon = 0x01;
	m_iadrbe = 0x00;
	m_iadboh = 0x10;
	m_iadtoh = 0x0f;
	m_icrt = 0x56;
	m_icd = 0x40;
	m_ubcsa = 0x07;
	m_ubcsb = 0x4f;
	std::fill(std::begin(m_mar), std::end(m_mar), 0U);
	std::fill(std::begin(m_pirq), std::end(m_pirq), 0x80U);
	m_bios_timer_base = 0x0078;
	m_smicntl = 0x08;
	m_smien = 0x0000;
	m_see = 0x00000000;
	m_ftmr = 0x0f;
	m_smireq = 0x0000;
	m_ctltmr = 0x00;
	m_ctltmrh = 0x00;

	m_ext_gatea20 = 0;
	m_fast_gatea20 = 0;

	remap_cb();
}

void i82378zb_sio_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// reserved range on this controller (starts at 0x09 really, but need revision being mapped)
	map(0x10, 0x3f).lr8(NAME([] () { return 0; }));

	map(0x40, 0x40).lrw8(
		NAME([this] (offs_t offset) {
			return m_pcicon;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("40h: PCICON PCI Control %02x\n", data);
			m_pcicon = data & 0x7f;
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] (offs_t offset) {
			return m_pac;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("41h: PAC PCI Arbiter Control %02x\n", data);
			m_pac = data & 0x1f;
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] (offs_t offset) {
			return m_papc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("42h: PAPC PCI Arbiter Priority Control %02x\n", data);
			m_papc = data;
		})
	);
	map(0x43, 0x43).lrw8(
		NAME([this] (offs_t offset) {
			return m_arbprix;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("43h: ARBPRIX PCI Arbiter Priority Control %02x\n", data);
			m_arbprix = BIT(data, 0);
		})
	);
	map(0x44, 0x44).lrw8(
		NAME([this] (offs_t offset) {
			return m_mcscon;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("44h: MCSCON MEMCS# Control %02x\n", data);
			m_mcscon = data & 0x1f;
		})
	);
	map(0x45, 0x45).lrw8(
		NAME([this] (offs_t offset) {
			return m_mcsboh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("45h: MCSBOH MEMCS# Bottom Of Hole %02x\n", data);
			m_mcsboh = data;
		})
	);
	map(0x46, 0x46).lrw8(
		NAME([this] (offs_t offset) {
			return m_mcstoh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("45h: MCSTOH MEMCS# Top Of Hole %02x\n", data);
			m_mcstoh = data;
		})
	);
	map(0x47, 0x47).lrw8(
		NAME([this] (offs_t offset) {
			return m_mcstom;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("47h: MCSTOM MEMCS# Top Of Memory %02x\n", data);
			m_mcstom = data;
		})
	);
	map(0x48, 0x48).lrw8(
		NAME([this] (offs_t offset) {
			return m_iadcon;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("48h: IADCON ISA Address Decoder Control %02x\n", data);
			m_iadcon = data;
		})
	);
	map(0x49, 0x49).lrw8(
		NAME([this] (offs_t offset) {
			return m_iadrbe;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("49h: IADRBE ISA Address Decoder ROM Block Enable %02x\n", data);
			m_iadrbe = data;
		})
	);
	map(0x4a, 0x4a).lrw8(
		NAME([this] (offs_t offset) {
			return m_iadboh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Ah: IADBOH ISA Address Decoder Bottom Of Hole %02x\n", data);
			m_iadboh = data;
		})
	);
	map(0x4b, 0x4b).lrw8(
		NAME([this] (offs_t offset) {
			return m_iadtoh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Bh: IADTOH ISA Address Decoder Top Of Hole %02x\n", data);
			m_iadtoh = data;
		})
	);
	map(0x4c, 0x4c).lrw8(
		NAME([this] (offs_t offset) {
			return m_icrt;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Ch: ICRT ISA Controller Recovery Timer %02x\n", data);
			m_icrt = data & 0x7f;
		})
	);
	// -x-- ---- Positive decode of $xxxf0000 BIOS enable
	// --x- ---- Coprocessor Error Enable
	// ---x ---- IRQ12/M mouse function enable
	// ---- x--- RSTDRV enable (for resetting ISA bus clock below)
	// ---- -xxx PCICLK-to-ISA SYSCLK divisor (unlisted combinations <reserved>)
	// ---- -000 4 (33 MHz) -> 8.33 MHz
	// ---- -001 3 (25 Mhz) -> 8.33 MHz
	map(0x4d, 0x4d).lrw8(
		NAME([this] (offs_t offset) {
			return m_icd;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4Dh: ICD ISA Clock Divisor %02x\n", data);
			m_icd = data & 0x7f;
		})
	);

	map(0x4e, 0x4e).lrw8(
		NAME([this] (offs_t offset) {
			return m_ubcsa;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4eh: UBCSA Utility Bus Chip Select A %02x\n", data);
			m_ubcsa = data;
			remap_cb();
		})
	);
	map(0x4f, 0x4f).lrw8(
		NAME([this] (offs_t offset) {
			return m_ubcsb;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("4fh: UBCSB Utility Bus Chip Select B %02x\n", data);
			m_ubcsb = data;
			remap_cb();
		})
	);

	map(0x54, 0x56).lrw8(
		NAME([this] (offs_t offset) {
			return m_mar[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("%02Xh: MAR%d MEMCS# Attribute %02x\n", offset + 0x54, offset + 1, data);
			m_mar[offset] = data;
			// TODO: r/w enable for PCI/ISA ROM segments
		})
	);

	map(0x60, 0x63).lrw8(
		NAME([this] (offs_t offset) {
			return m_pirq[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("%02Xh: PIRQ%d PIRQ Route Control %02x\n", offset + 0x60, offset, data);
			m_pirq[offset] = data & 0x8f;
		})
	);

	// map(0x70, 0x70) PACC, 'AB only
	// map(0x71, 0x71) APICBASE, 'AB only

	map(0x80, 0x81).lrw16(
		NAME([this] (offs_t offset) {
			return m_bios_timer_base;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("80h: BIOS Timer Base Address %04x\n", data, mem_mask);
			COMBINE_DATA(&m_bios_timer_base);
			m_bios_timer_base &= ~2;
			if (BIT(m_bios_timer_base, 0))
				LOG("\tWarning: enabled\n");
		})
	);

	map(0xa0, 0xa0).lrw8(
		NAME([this] (offs_t offset) {
			return m_smicntl;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("A0h: SMICNTL SMI Control %02x\n", data);
			// TODO: bits 6 writable in 'AB
			m_smicntl = data & 0x0f;
		})
	);

	map(0xa2, 0xa3).lrw16(
		NAME([this] (offs_t offset) {
			return m_smien;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("A2h: SMIEN SMI Enable %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_smien);
			m_smien &= 0x00ff;
		})
	);
	map(0xa4, 0xa7).lrw32(
		NAME([this] (offs_t offset) {
			return m_see;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOG("A4h: SEE System Event Enable %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_see);
			// TODO: bits 30, 27~24 writable on 'AB
			// m_see &= 0xef00fffb;
			m_see &= 0xa000fffb;
		})
	);
	map(0xa8, 0xa8).lrw8(
		NAME([this] (offs_t offset) {
			return m_ftmr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("A8h: FTMR Fast Off Timer %02x\n", data);
			m_ftmr = data;
		})
	);
	map(0xaa, 0xab).lrw16(
		NAME([this] (offs_t offset) {
			return m_smireq;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("AAh: SMIREQ SMI Request %04x & %04x\n", data, mem_mask);
			COMBINE_DATA(&m_smireq);
			m_smireq &= 0x00ff;
		})
	);

	map(0xac, 0xac).lrw8(
		NAME([this] (offs_t offset) {
			return m_ctltmr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("ACh: CTLTMR Clock Scale STPCLK# Low Timer %02x\n", data);
			m_ctltmr = data;
		})
	);
	map(0xae, 0xae).lrw8(
		NAME([this] (offs_t offset) {
			return m_ctltmrh;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("AEh: CTLTMRH Clock Scale STPCLK# High Timer %02x\n", data);
			m_ctltmrh = data;
		})
	);

}

void i82378zb_sio_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	// map(0x0060, 0x0060) keyboard
	map(0x0061, 0x0061).rw(FUNC(i82378zb_sio_device::at_portb_r), FUNC(i82378zb_sio_device::at_portb_w));
	// map(0x0064, 0x0064) keyboard

	//  map(0x0070, 0x0071) RTC
	map(0x0080, 0x008f).rw(FUNC(i82378zb_sio_device::at_page8_r), FUNC(i82378zb_sio_device::at_page8_w));
//  map(0x0092, 0x0092).rw(FUNC(i82378zb_sio_device::port92_r), FUNC(i82378zb_sio_device::port92_w));
	map(0x00a0, 0x00a1).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(FUNC(i82378zb_sio_device::at_dma8237_2_r), FUNC(i82378zb_sio_device::at_dma8237_2_w));

	// timestamp for this southbridge?
	map(0x00e1, 0x00e1).lw8(NAME([] (offs_t offset, u8 data) { }));
}

/*
 * Remapping functions
 */

void i82378zb_sio_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void i82378zb_sio_device::rtc_index_w(offs_t offset, u8 data)
{
	m_nmi_enabled = BIT(data, 7);
	m_rtcale(data);
}

u8 i82378zb_sio_device::rtc_data_r(offs_t offset) { return m_rtccs_read(); }
void i82378zb_sio_device::rtc_data_w(offs_t offset, u8 data) { m_rtccs_write(data); }

void i82378zb_sio_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	// assume that map_extra of the southbridge is called before the one of the northbridge
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	// TODO: bits 7-6 in UBCSA for BIOS enable control, retrieve region size from flash type
	if (m_has_xbus.flash_bios)
	{
		memory_space->install_readwrite_handler(0xfffe0000, 0xffffffff, read8sm_delegate(*m_xbus_flash, FUNC(intelfsh8_device::read)), write8sm_delegate(*m_xbus_flash, FUNC(intelfsh8_device::write)));
		memory_space->install_readwrite_handler(0x000e0000, 0x000fffff, read8sm_delegate(*m_xbus_flash, FUNC(intelfsh8_device::read)), write8sm_delegate(*m_xbus_flash, FUNC(intelfsh8_device::write)));
	}
	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0xffff, *this, &i82378zb_sio_device::internal_io_map);

	// TODO: UBCSA bits 3,2 for FDC + bit 5 for location address

	// decodes IDE signals after FDC (i.e. overrides $3f6 / $376)
	// ga586ip & sy029c2 initializes UBCSA with 0xc3, which implies both aren't on X-Bus
	if (BIT(m_ubcsa, 4) && m_has_xbus.ide)
	{
		io_space->install_readwrite_handler(0x1f0, 0x1f7, read32s_delegate(*m_xbus_ide[0], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_xbus_ide[0], FUNC(ide_controller_32_device::cs0_w)));
		io_space->install_readwrite_handler(0x3f0, 0x3f7, read32s_delegate(*m_xbus_ide[0], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_xbus_ide[0], FUNC(ide_controller_32_device::cs1_w)));

		io_space->install_readwrite_handler(0x170, 0x177, read32s_delegate(*m_xbus_ide[1], FUNC(ide_controller_32_device::cs0_r)), write32s_delegate(*m_xbus_ide[1], FUNC(ide_controller_32_device::cs0_w)));
		io_space->install_readwrite_handler(0x370, 0x377, read32s_delegate(*m_xbus_ide[1], FUNC(ide_controller_32_device::cs1_r)), write32s_delegate(*m_xbus_ide[1], FUNC(ide_controller_32_device::cs1_w)));
	}

	if (BIT(m_ubcsa, 1) && m_has_xbus.keyboard)
	{
		io_space->install_readwrite_handler(0x60, 0x60, read8smo_delegate(*m_xbus_keybc, FUNC(at_keyboard_controller_device::data_r)), write8smo_delegate(*m_xbus_keybc, FUNC(at_keyboard_controller_device::data_w)));
		io_space->install_readwrite_handler(0x64, 0x64, read8smo_delegate(*m_xbus_keybc, FUNC(at_keyboard_controller_device::status_r)), write8smo_delegate(*m_xbus_keybc, FUNC(at_keyboard_controller_device::command_w)));
	}

	// This maps thru X-Bus but we don't know yet the type it expects so using devcb for now.
	if (BIT(m_ubcsa, 0))
	{
		// no read support at $70
		io_space->install_write_handler(0x70, 0x70,
			write8sm_delegate(*this, FUNC(i82378zb_sio_device::rtc_index_w))
		);
		io_space->install_readwrite_handler(0x71, 0x71,
			read8sm_delegate(*this, FUNC(i82378zb_sio_device::rtc_data_r)),
			write8sm_delegate(*this, FUNC(i82378zb_sio_device::rtc_data_w))
		);
	}

	// BIT(m_ubcsb, 7) configuration RAM decode enable

	// TODO: does it decode on BeBox?
	if (BIT(m_ubcsb, 6))
	{
		io_space->install_readwrite_handler(0x92, 0x92,
			read8sm_delegate(*this, FUNC(i82378zb_sio_device::port92_r)),
			write8sm_delegate(*this, FUNC(i82378zb_sio_device::port92_w))
		);
	}

	// TODO: all of these enabled by ga586ip
	// BIT(m_ubcsb, 5:4) LPT enable
	// BIT(m_ubcsb, 3:2) COM2 enable
	// BIT(m_ubcsb, 1:0) COM1 enable
}

/*
 * Southbridge common stuff
 */

void i82378zb_sio_device::pc_irq1_w(int state)   { m_pic[0]->ir1_w(state); }
void i82378zb_sio_device::pc_irq3_w(int state)   { m_pic[0]->ir3_w(state); }
void i82378zb_sio_device::pc_irq4_w(int state)   { m_pic[0]->ir4_w(state); }
void i82378zb_sio_device::pc_irq5_w(int state)   { m_pic[0]->ir5_w(state); }
void i82378zb_sio_device::pc_irq6_w(int state)   { m_pic[0]->ir6_w(state); }
void i82378zb_sio_device::pc_irq7_w(int state)   { m_pic[0]->ir7_w(state); }
void i82378zb_sio_device::pc_irq8n_w(int state)  { m_pic[1]->ir0_w(state); }
void i82378zb_sio_device::pc_irq9_w(int state)   { m_pic[1]->ir1_w(state); }
void i82378zb_sio_device::pc_irq10_w(int state)  { m_pic[1]->ir2_w(state); }
void i82378zb_sio_device::pc_irq11_w(int state)  { m_pic[1]->ir3_w(state); }
void i82378zb_sio_device::pc_irq12m_w(int state) { m_pic[1]->ir4_w(state); }
void i82378zb_sio_device::pc_irq14_w(int state)  { m_pic[1]->ir6_w(state); }
void i82378zb_sio_device::pc_irq15_w(int state)  { m_pic[1]->ir7_w(state); }

uint8_t i82378zb_sio_device::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return m_pic[1]->acknowledge();

	return 0x00;
}

void i82378zb_sio_device::at_pit8254_out0_changed(int state)
{
	if (m_pic[0])
		m_pic[0]->ir0_w(state);
}

void i82378zb_sio_device::at_pit8254_out1_changed(int state)
{
	if (state)
		m_refresh = !m_refresh;
}

void i82378zb_sio_device::at_pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

void i82378zb_sio_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

uint8_t i82378zb_sio_device::at_portb_r()
{
	uint8_t data = m_at_speaker;

	data &= ~0xd0; /* AT BIOS don't likes this being set */
	/* 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us. */
	data |= m_refresh ? 0x10 : 0;
	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

void i82378zb_sio_device::at_portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void i82378zb_sio_device::iochck_w(int state)
{
	if (!state && !m_channel_check && m_nmi_enabled)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t i82378zb_sio_device::at_page8_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch (offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}
	return data;
}

void i82378zb_sio_device::at_page8_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch (offset % 8)
	{
	case 0:
		m_boot_state_hook((offs_t)0, data);
		break;
	case 1:
		m_dma_offset[BIT(offset, 3)][2] = data;
		break;
	case 2:
		m_dma_offset[BIT(offset, 3)][3] = data;
		break;
	case 3:
		m_dma_offset[BIT(offset, 3)][1] = data;
		break;
	case 7:
		m_dma_offset[BIT(offset, 3)][0] = data;
		break;
	}
}

void i82378zb_sio_device::pc_dma_hrq_changed(int state)
{
	m_host_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma[1]->hack_w(state);
}

uint8_t i82378zb_sio_device::pc_dma_read_byte(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}

void i82378zb_sio_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t i82378zb_sio_device::pc_dma_read_word(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}

void i82378zb_sio_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

uint8_t i82378zb_sio_device::pc_dma8237_0_dack_r() { return m_isabus->dack_r(0); }
uint8_t i82378zb_sio_device::pc_dma8237_1_dack_r() { return m_isabus->dack_r(1); }
uint8_t i82378zb_sio_device::pc_dma8237_2_dack_r() { return m_isabus->dack_r(2); }
uint8_t i82378zb_sio_device::pc_dma8237_3_dack_r() { return m_isabus->dack_r(3); }
uint8_t i82378zb_sio_device::pc_dma8237_5_dack_r() { return m_isabus->dack_r(5); }
uint8_t i82378zb_sio_device::pc_dma8237_6_dack_r() { return m_isabus->dack_r(6); }
uint8_t i82378zb_sio_device::pc_dma8237_7_dack_r() { return m_isabus->dack_r(7); }

void i82378zb_sio_device::pc_dma8237_0_dack_w(uint8_t data) { m_isabus->dack_w(0, data); }
void i82378zb_sio_device::pc_dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }
void i82378zb_sio_device::pc_dma8237_2_dack_w(uint8_t data) { m_isabus->dack_w(2, data); }
void i82378zb_sio_device::pc_dma8237_3_dack_w(uint8_t data) { m_isabus->dack_w(3, data); }
void i82378zb_sio_device::pc_dma8237_5_dack_w(uint8_t data) { m_isabus->dack_w(5, data); }
void i82378zb_sio_device::pc_dma8237_6_dack_w(uint8_t data) { m_isabus->dack_w(6, data); }
void i82378zb_sio_device::pc_dma8237_7_dack_w(uint8_t data) { m_isabus->dack_w(7, data); }

void i82378zb_sio_device::at_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if (m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE);
}

void i82378zb_sio_device::pc_select_dma_channel(int channel, bool state)
{
	m_isabus->dack_line_w(channel, state);

	if (!state)
	{
		m_dma_channel = channel;
		if (m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE);
	}
	else if (m_dma_channel == channel)
	{
		m_dma_channel = -1;
		if (m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE);
	}
}

void i82378zb_sio_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void i82378zb_sio_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void i82378zb_sio_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void i82378zb_sio_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }
void i82378zb_sio_device::pc_dack4_w(int state) { m_dma[0]->hack_w( state ? 0 : 1); } // it's inverted
void i82378zb_sio_device::pc_dack5_w(int state) { pc_select_dma_channel(5, state); }
void i82378zb_sio_device::pc_dack6_w(int state) { pc_select_dma_channel(6, state); }
void i82378zb_sio_device::pc_dack7_w(int state) { pc_select_dma_channel(7, state); }

uint8_t i82378zb_sio_device::at_dma8237_2_r(offs_t offset)
{
	return m_dma[1]->read(offset / 2);
}

void i82378zb_sio_device::at_dma8237_2_w(offs_t offset, uint8_t data)
{
	m_dma[1]->write(offset / 2, data);
}

void i82378zb_sio_device::cpu_a20_w(int state)
{
	m_ext_gatea20 = state;
	m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
}

void i82378zb_sio_device::fast_gatea20(int state)
{
	m_fast_gatea20 = state;
	m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
}

void i82378zb_sio_device::cpu_reset_w(int state)
{
	m_write_cpureset(state);
}

u8 i82378zb_sio_device::port92_r(offs_t offset)
{
	return 0x24 | m_port92;
}

void i82378zb_sio_device::port92_w(offs_t offset, u8 data)
{
	fast_gatea20(BIT(data, 1));

	// TODO: alt system reset
//  if (!BIT(m_port92, 0) && BIT(data, 0))
//  {
//      // pulse reset line
//      m_write_cpureset(1);
//      m_write_cpureset(0);
//  }

	m_port92 = data & 3;
}

