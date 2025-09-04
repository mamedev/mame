// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC.
 *
 * Sources:
 *   - IBM RT PC Hardware Technical Reference Volume I, 75X0232, March 1987
 *   - http://www.cs.cmu.edu/afs/andrew.cmu.edu/usr/shadow/www/ibmrt.html
 *   - http://ps-2.kev009.com/ohlandl/6152/rt_index.html
 *
 * TODO:
 *   - finish refactoring iocc
 *   - additional machine variants
 *   - configurable RAM size
 *   - shared interrupts
 */
/*
 * https://www-01.ibm.com/common/ssi/ShowDoc.wss?docURL=/common/ssi/rep_ca/6/897/ENUS186-006/index.html
 * https://www-01.ibm.com/common/ssi/ShowDoc.wss?docURL=/common/ssi/rep_ca/1/897/ENUS187-021/index.html
 * https://www-01.ibm.com/common/ssi/ShowDoc.wss?docURL=/common/ssi/rep_ca/0/897/ENUS188-120/index.html
 *
 *   Model  Chassis  CPU  RAM      HDD  Release   Price    Notes
 *    010    6151    032  1M/4M    40M  Jan 1986  $11,700
 *    015    6151    032  2M/4M    70M  Nov 1986  $10,050
 *    020    6150    032  1M/4M    40M  Jan 1986  $14,945
 *    025    6150    032  2M/4M    70M  Jan 1986  $17,940
 *    A25    6150    032  2M/4M    70M  Jan 1986  $19,510  5080 attachment/no keyboard
 *    115    6151    Adv  4M/16M   70M  Feb 1987  $10,600  AFPA
 *    125    6150    Adv  4M/16M   70M  Feb 1987  $16,100  AFPA
 *    B25    6150    Adv  4M/16M   70M  Feb 1987  $17,670  AFPA, 5080 attachment/no keyboard
 *    130    6151    Enh  16M     114M  Jul 1988  $23,220  EAFPA
 *    135    6150    Enh  16M     114M  Jul 1988  $30,595  EAFPA
 *    B35    6150    Enh  16M     114M  Jul 1988  $32,165  EAFPA, 5080 attachment/no keyboard
 *
 * 032 (aka SGP), 170ns (23.5294 MHz crystal / 4 == 5.882350 MHz == 170ns), 1MB/2MB/4MB memory boards
 * Advanced, 100ns 4MB (6151) external (6150) (presume ~40MHz crystal/4), 4MB/8MB memory boards
 * Enhanced, 80ns, 16MB soldered, EAFPA standard, CMOS (49.400 MHz crystal/4 == 12.350MHz == 80.971ns)
 *
 * FPA is NS32081
 * AFPA is M68881 @ 20MHz
 * EAFPA is AD90221-2 ADSP-3210 (multiplier) + AD90222-2 ADSP-3221 (fp alu) + AD90220-2 ADSP-1401 (program sequencer)
 *
 * system processor real memory address map
 *   0000'0000-00ff'ffff 16MB memory management unit
 *   0100'0000-efff'ffff -- not defined
 *
 * system board I/O addresses (all in segment F)
 *   f000'0000-f0ff'ffff 16MB i/o channel i/o map
 *   f100'0000-f3ff'ffff 48MB reserved
 *   f400'0000-f4ff'ffff 16MB i/o channel memory map
 *   f500'0000-f7ff'ffff reserved?
 *   f800'0000-fbff'ffff reserved
 *   fc00'0000-fdff'ffff mc68881
 *   fe00'0000-feff'ffff fpa2
 *   ff00'0000-ffff'ffff 16MB floating point accelerator
 *
 * system processor i/o address map
 *   00'0000-7f'ffff 8MB not defined
 *   80'0000-80'ffff 64KB processor channel device initialization
 *   81'0000-81'ffff 64KB memory management unit
 *   82'0000-ff'ffff 8064KB not defined
 *
 * advanced processor
 *   80'0400 i/o interface 1 i/o base
 *   80'0100 i/o interface 2 i/o base
 *
 */
 /*
  * https://ardent-tool.com/615x/rt_loadable_post.html
  *
  * WIP
  *  - boots to vrm install disk menu
  *  - requires improved hard disk controller emulation
  */

#include "emu.h"

#include "rosetta.h"
#include "rtpc_iocc.h"
#include "rtpc_kls.h"

// cpu and memory
#include "cpu/romp/romp.h"

// various hardware
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/z80scc.h"
#include "sound/spkrdev.h"

// busses and connectors
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/amgda.h"
#include "bus/isa/ega.h"
#include "bus/isa/fdc.h"
#include "bus/isa/ide.h"
#include "bus/isa/mda.h"
#include "bus/rs232/rs232.h"

#include "rtpc.lh"

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class rtpc_state : public driver_device
{
public:
	rtpc_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_mmu(*this, "mmu")
		, m_iocc(*this, "iocc")
		, m_dma(*this, "dma%u", 0U)
		, m_pic(*this, "pic%u", 0U)
		, m_kls(*this, "kls")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_isa(*this, "isa")
		, m_slot(*this, "isa%u", 1U)
		, m_ipl(*this, "ipl")
	{
	}

	void ibm6150(machine_config &config);
	void ibm6151(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void iocc_mem_map(address_map &map);
	template <bool SCC> void iocc_pio_map(address_map &map) ATTR_COLD;

	void common(machine_config &config);

	void crra_w(u8 data);
	void crrb_w(u8 data);
	void dia_w(u8 data);

	// devices
	required_device<romp_device> m_cpu;
	required_device<rosetta_device> m_mmu;
	required_device<rtpc_iocc_device> m_iocc;

	required_device_array<am9517a_device, 2> m_dma;
	required_device_array<pic8259_device, 2> m_pic;
	required_device<rtpc_kls_device> m_kls;
	required_device<mc146818_device> m_rtc;
	optional_device<z80scc_device> m_scc;

	required_device<isa16_device> m_isa;
	optional_device_array<device_t, 8> m_slot;

	required_region_ptr<u32> m_ipl;

	u8 m_ch8er;  // dma channel 8 enable register
	u8 m_crra;   // component reset register a
	u8 m_crrb;   // component reset register b
	u8 m_ext[2]; // external serial registers
};

void rtpc_state::machine_start()
{
	m_ch8er = 0;
	m_crra = 0xff;
	m_crrb = 0xff;

	m_ext[0] = 0;
	m_ext[1] = 0;
}

void rtpc_state::device_reset()
{
	offs_t const mask = m_ipl.bytes() - 1;

	m_cpu->space(AS_PROGRAM).install_rom(0, mask, 0x00ff'ffff & ~mask, m_ipl);
}

void rtpc_state::iocc_mem_map(address_map &map)
{
	map.unmap_value_high();

	// HACK: temporarily silence noisy probing for absent hardware
	map(0x0b'8000, 0x0b'8fff).noprw();
	map(0xe0'0000, 0xe1'ffff).noprw();
}

template <bool SCC> void rtpc_state::iocc_pio_map(address_map &map)
{
	// HACK: temporarily silence noisy probing for absent hardware
	map(0x00'0110, 0x00'0113).noprw();
	map(0x00'01e8, 0x00'01ef).noprw();

	// interrupt level enables (shared interrupts)
	map(0x00'02f0, 0x00'02f7).nopw(); // irq 0..7
	map(0x00'06f0, 0x00'06f7).nopw(); // irq 8..15 (excluding 13)

	if (SCC)
	{
		map(0x00'8000, 0x00'8003).rw(m_scc, FUNC(z80scc_device::dc_ab_r), FUNC(z80scc_device::dc_ab_w));
		map(0x00'8020, 0x00'8020).lrw8([this]() { return m_ext[0]; }, "ext_a_r", [this](u8 data) { m_ext[0] = data; }, "ext_a_w");
		map(0x00'8040, 0x00'8040).lrw8([this]() { return m_ext[1]; }, "ext_b_r", [this](u8 data) { m_ext[1] = data; }, "ext_b_w");
		map(0x00'8060, 0x00'8060).lr8([this]() { return u8(m_scc->m1_r()); }, "intack");
	}

	// delay 1µs per byte written
	map(0x00'80e0, 0x00'80e3).nopr(); // TODO: does read also generate a delay?
	map(0x00'80e0, 0x00'80e3).lw8([this](u8 data) { m_cpu->eat_cycles(m_cpu->clock() / 1000000 + 1); }, "io_delay");

	map(0x00'8400, 0x00'8407).m(m_kls, FUNC(rtpc_kls_device::map));

	map(0x00'8800, 0x00'883f).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x00'8840, 0x00'884f).mirror(0x10).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x00'8860, 0x00'887f).umask16(0x00ff).rw(m_dma[1], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x00'8880, 0x00'8881).mirror(0x1e).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00'88a0, 0x00'88a1).mirror(0x1e).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00'88c0, 0x00'88c0).mirror(0x1f).rw(m_iocc, FUNC(rtpc_iocc_device::dbr_r), FUNC(rtpc_iocc_device::dbr_w));
	map(0x00'88e0, 0x00'88e0).mirror(0x1f).rw(m_iocc, FUNC(rtpc_iocc_device::dmr_r), FUNC(rtpc_iocc_device::dmr_w));

	map(0x00'8c00, 0x00'8c00).mirror(0x03).lrw8([this]() { return m_ch8er; }, "ch8er_r", [this](u8 data) { m_ch8er = data; }, "ch8er_w");
	map(0x00'8c20, 0x00'8c20).mirror(0x03).rw(m_iocc, FUNC(rtpc_iocc_device::ccr_r), FUNC(rtpc_iocc_device::ccr_w));
	map(0x00'8c40, 0x00'8c40).mirror(0x03).lr8([this]() { return m_crra; }, "crra_r");
	map(0x00'8c40, 0x00'8c40).mirror(0x03).w(FUNC(rtpc_state::crra_w));
	map(0x00'8c60, 0x00'8c60).mirror(0x03).lr8([this]() { return m_crrb; }, "crrb_r");
	map(0x00'8c60, 0x00'8c60).mirror(0x03).w(FUNC(rtpc_state::crrb_w));

	// memory config reg (cc=2x8M, dd=2x2M, 88=2x4M)
	map(0x00'8c80, 0x00'8c80).mirror(0x03).lr8([]() { return 0xf8; }, "mcr");
	map(0x00'8ca0, 0x00'8ca0).mirror(0x03).w(FUNC(rtpc_state::dia_w));

	// 8c82 diag dma mode?
	// 8c84 diag dma exit?
	map(0x00'8ce0, 0x00'8ce1).nopw(); // FIXME: hex display register?

	map(0x01'1000, 0x01'20ff).noprw(); // silence something
}

void rtpc_state::crra_w(u8 data)
{
	LOG("crra_w 0x%02x (%s)\n", data, machine().describe_context());

	// selectively reset i/o slots 1..8
	for (unsigned i = 0; i < 8; i++)
	{
		if (BIT(data, i) && !BIT(m_crra, i) && m_slot[i].found())
		{
			LOG("reset slot %u\n", i);
			m_slot[i]->reset();
		}
	}

	m_crra = data;
}

void rtpc_state::crrb_w(u8 data)
{
	LOG("crrb_w 0x%02x (%s)\n", data, machine().describe_context());

	// bit  function
	//  0   8530
	//  1   rs232 interface
	//  2   8051
	//  3   dmac1
	//  4   dmac2
	//  5   arbitor
	//  6   reserved
	//  7   reserved

	if (m_scc && BIT(data, 0))
		m_scc->reset();

	// TODO: rs232 if

	m_kls->mcu_reset_w(!BIT(data, 2));

	// TODO: dmac !ready
	// TODO: arbitor

	m_crrb = data;
}

void rtpc_state::dia_w(u8 data)
{
	bool const state = BIT(data, 0);

	LOG("dia_w 0x%02x (%s)\n", data, machine().describe_context());

	m_pic[0]->ir0_w(state);
	m_pic[0]->ir1_w(state);
	m_pic[0]->ir2_w(state);
	m_pic[0]->ir3_w(state);
	m_pic[0]->ir4_w(state);
	m_pic[0]->ir5_w(state);
	m_pic[0]->ir6_w(state);
	m_pic[0]->ir7_w(state);

	m_pic[1]->ir0_w(state);
	m_pic[1]->ir1_w(state);
	m_pic[1]->ir2_w(state);
	m_pic[1]->ir3_w(state);
	m_pic[1]->ir4_w(state);
	m_pic[1]->ir5_w(state);
	m_pic[1]->ir6_w(state);
	m_pic[1]->ir7_w(state);
}

void rtpc_state::common(machine_config &config)
{
	/*
	 * irq  source
	 *  0   mcu system attention
	 *  1   rtc interrupt
	 *  2   mmu program check, iocc error
	 *  3   pic 0 interrupt
	 *  4   pic 1 interrupt
	 *  5   (not connected, software interrupt?)
	 *  6   (not connected, software interrupt?)
	 * nmi  mmu machine check, early power off
	 */
	ROMP(config, m_cpu, 23'529'400 / 4);
	m_cpu->set_mmu(m_mmu);
	m_cpu->set_iou(m_iocc);

	input_merger_device &reqi2(INPUT_MERGER_ANY_LOW(config, "reqi2"));
	reqi2.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ2).invert();

	// TODO: hole for 1M/4M memory boards
	// 2/2 rams 4M
	// 2/1 rams 4M
	// 1/1 rams 4M hole 1M
	// 4/0 rams 4M
	// 4/4 rams 16M hole 4M
	ROSETTA(config, m_mmu, m_cpu->clock(), rosetta_device::RAM_4M);
	m_mmu->set_mem(m_cpu, AS_PROGRAM);
	m_mmu->set_rom("ipl");
	m_mmu->out_pchk().set(reqi2, FUNC(input_merger_device::in_w<0>));
	m_mmu->out_mchk().set_inputline(m_cpu, INPUT_LINE_NMI);

	RTPC_IOCC(config, m_iocc, 0);
	m_iocc->set_addrmap(0, &rtpc_state::iocc_mem_map);
	m_iocc->out_int().set(reqi2, FUNC(input_merger_device::in_w<1>));
	m_iocc->out_rst().set_inputline(m_dma[0], INPUT_LINE_RESET);
	m_iocc->out_rst().append_inputline(m_dma[1], INPUT_LINE_RESET);

	// ISA bus
	ISA16(config, m_isa, 14'318'180 / 3);
	m_isa->set_memspace(m_iocc, AS_PROGRAM);
	m_isa->set_iospace(m_iocc, AS_IO);
	//m_isa->iochck_callback().set(FUNC(at_mb_device::iochck_w));

	// NEC
	// D8237AC-5
	// 8903HV101
	AM9517A(config, m_dma[0], m_isa->clock());

	// dma0 channel 0 == isa channel 2 (diskette drive, serial port A)
	m_isa->drq2_callback().set(m_dma[0], FUNC(am9517a_device::dreq_w<0>));
	m_dma[0]->in_ior_callback<0>().set([this]() { return m_isa->dack_r(2); });
	m_dma[0]->out_iow_callback<0>().set([this](u8 data) { m_isa->dack_w(2, data); });
	m_dma[0]->out_dack_callback<0>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<2>));

	// dma0 channel 1 == isa channel 1 (serial port B)
	m_isa->drq1_callback().set(m_dma[0], FUNC(am9517a_device::dreq_w<1>));
	m_dma[0]->in_ior_callback<1>().set([this]() { return m_isa->dack_r(1); });
	m_dma[0]->out_iow_callback<1>().set([this](u8 data) { m_isa->dack_w(1, data); });
	m_dma[0]->out_dack_callback<1>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<1>));

	// dma0 channel 2 == isa channel 0 (serial port A)
	m_isa->drq0_callback().set(m_dma[0], FUNC(am9517a_device::dreq_w<2>));
	m_dma[0]->in_ior_callback<2>().set([this]() { return m_isa->dack_r(0); });
	m_dma[0]->out_iow_callback<2>().set([this](u8 data) { m_isa->dack_w(0, data); });
	m_dma[0]->out_dack_callback<2>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<0>));

	// dma0 channel 3 == isa channel 3 (pc network, serial port B)
	m_isa->drq3_callback().set(m_dma[0], FUNC(am9517a_device::dreq_w<3>));
	m_dma[0]->in_ior_callback<3>().set([this]() { return m_isa->dack_r(3); });
	m_dma[0]->out_iow_callback<3>().set([this](u8 data) { m_isa->dack_w(3, data); });
	m_dma[0]->out_dack_callback<3>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<3>));

	m_dma[0]->out_hreq_callback().set(m_dma[0], FUNC(am9517a_device::hack_w));
	m_dma[0]->in_memr_callback().set(m_iocc, FUNC(rtpc_iocc_device::dma_b_r));
	m_dma[0]->out_memw_callback().set(m_iocc, FUNC(rtpc_iocc_device::dma_b_w));
	m_dma[0]->out_eop_callback().set(m_pic[0], FUNC(pic8259_device::ir0_w));

	// FIXME: eop should be asserted on the bus and tested by the device
	// when transferring data, not routed to a specific card like this
	m_dma[0]->out_eop_callback().append([this](int state) { m_isa->eop_w(m_iocc->adc_r(), state); });

	// NEC
	// D8237AC-5
	// 8903HV101
	AM9517A(config, m_dma[1], m_isa->clock());

	// dma1 channel 0 == coprocessor channel 8?
	m_dma[1]->out_dack_callback<0>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<4>));

	// dma1 channel 1 == isa channel 5
	m_isa->drq5_callback().set(m_dma[1], FUNC(am9517a_device::dreq_w<1>));
	m_dma[1]->out_dack_callback<1>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<5>));

	// dma1 channel 2 == isa channel 6
	m_isa->drq6_callback().set(m_dma[1], FUNC(am9517a_device::dreq_w<2>));
	m_dma[1]->out_dack_callback<2>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<6>));

	// dma1 channel 3 == isa channel 7
	m_isa->drq7_callback().set(m_dma[1], FUNC(am9517a_device::dreq_w<3>));
	m_dma[1]->out_dack_callback<3>().set(m_iocc, FUNC(rtpc_iocc_device::dack_w<7>));

	m_dma[1]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::hack_w));
	m_dma[1]->in_memr_callback().set(m_iocc, FUNC(rtpc_iocc_device::dma_w_r));
	m_dma[1]->out_memw_callback().set(m_iocc, FUNC(rtpc_iocc_device::dma_w_w));


	// TODO: system board irq 8, 13

	// NEC
	// D8259AC
	PIC8259(config, m_pic[0]);
	m_pic[0]->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ3).invert();
	// irq  source
	//  0   iocc irq 0: dma tc
	//  1   iocc irq 10: (multiport async)
	//  2   iocc irq 9: pc network, (multiport async), enhanced color graphics adapter, 3278/79 emulation adapter
	//  3   iocc irq 3: serial port 2, pc network
	//  4   iocc irq 4: serial port 1
	//  5   iocc irq 1: kbd
	//  6   iocc irq 2: 8530
	//  7   iocc irq 7: parallel port 1, monochrome/printer
	m_isa->irq10_callback().set(m_pic[0], FUNC(pic8259_device::ir1_w));
	m_isa->irq2_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_isa->irq3_callback().set(m_pic[0], FUNC(pic8259_device::ir3_w));
	m_isa->irq4_callback().set(m_pic[0], FUNC(pic8259_device::ir4_w));
	m_isa->irq7_callback().set(m_pic[0], FUNC(pic8259_device::ir7_w));

	// NEC
	// D8259AC
	PIC8259(config, m_pic[1]);
	m_pic[1]->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ4).invert();
	// irq  source
	//  0   iocc irq 8: reserved
	//  1   iocc irq 11: (advanced monochrome graphics display) (multiport async)
	//  2   iocc irq 14: fixed disk
	//  3   iocc irq 12: (ibm rt pc streaming tape drive adapter)
	//  4   iocc irq 6: diskette drive
	//  5   iocc irq 5: parallel port 2
	//  6   iocc irq 15: (286 coprocessor)
	//  7   iocc irq 13: serial port ex ct1 irpt
	m_isa->irq11_callback().set(m_pic[1], FUNC(pic8259_device::ir1_w));
	m_isa->irq14_callback().set(m_pic[1], FUNC(pic8259_device::ir2_w));
	m_isa->irq12_callback().set(m_pic[1], FUNC(pic8259_device::ir3_w));
	m_isa->irq6_callback().set(m_pic[1], FUNC(pic8259_device::ir4_w));
	m_isa->irq5_callback().set(m_pic[1], FUNC(pic8259_device::ir5_w));
	m_isa->irq15_callback().set(m_pic[1], FUNC(pic8259_device::ir6_w));

	RTPC_KLS(config, m_kls);
	m_kls->atn().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_kls->irq().set(m_pic[0], FUNC(pic8259_device::ir5_w));

	// MC146818AP
	// IL 0A46D8729
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->sqw().set(m_cpu, FUNC(romp_device::clk_w));
	m_rtc->sqw().append(m_kls, FUNC(rtpc_kls_device::rtc_sqw_w));
	m_rtc->irq().set_inputline(m_cpu, INPUT_LINE_IRQ1).invert();
	m_rtc->irq().append(m_kls, FUNC(rtpc_kls_device::rtc_irq_w)).invert();

	config.set_default_layout(layout_rtpc);
}

void rtpc_isa8_cards(device_slot_interface &device)
{
	device.option_add("mda", ISA8_MDA);
	device.option_add("ega", ISA8_EGA);
	device.option_add("fdc", ISA8_FDC_AT);
}

void rtpc_isa16_cards(device_slot_interface &device)
{
	device.option_add("amgda", ISA16_AMGDA);
	device.option_add("ide", ISA16_IDE);

	rtpc_isa8_cards(device);
}

void rtpc_state::ibm6150(machine_config &config)
{
	common(config);
	m_iocc->set_addrmap(2, &rtpc_state::iocc_pio_map<true>);

	SCC8530(config, m_scc, 3'580'000);
	m_scc->configure_channels(3'072'000, 3'072'000, 3'072'000, 3'072'000);
	m_scc->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir6_w));

	// port: TXD, DTR, RTS, RI, RXD, DSR, CTS, DCD
	// external registers: DTR, DSR, RI
	// scc: TXD, RTS, RXD, CTS, DCD
	// scc: DTR/REQ, SYNC not connected

	rs232_port_device &port0(RS232_PORT(config, "serial0", default_rs232_devices, nullptr));
	port0.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	port0.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	port0.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(port0, FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(port0, FUNC(rs232_port_device::write_txd));

	rs232_port_device &port1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	// TXD, DTR, RTS, RI, RXD, DSR, CTS, DCD
	port1.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	port1.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	port1.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(port1, FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(port1, FUNC(rs232_port_device::write_txd));

	// ISA slots
	ISA16_SLOT(config, m_slot[0], 0, m_isa, rtpc_isa16_cards, "fdc",   false); // slot 1: disk/diskette adapter
	ISA16_SLOT(config, m_slot[1], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 2: option
	ISA8_SLOT(config,  m_slot[2], 0, m_isa, rtpc_isa8_cards,  "mda",   false); // slot 3: option
	ISA16_SLOT(config, m_slot[3], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 4: option
	ISA16_SLOT(config, m_slot[4], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 5: option
	ISA8_SLOT(config,  m_slot[5], 0, m_isa, rtpc_isa8_cards,  nullptr, false); // slot 6: option
	ISA16_SLOT(config, m_slot[6], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 7: option
	ISA16_SLOT(config, m_slot[7], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 8: coprocessor/option
}

void rtpc_state::ibm6151(machine_config &config)
{
	common(config);
	m_iocc->set_addrmap(2, &rtpc_state::iocc_pio_map<false>);

	// ISA slots
	ISA8_SLOT(config,  m_slot[0], 0, m_isa, rtpc_isa8_cards,  "mda",   false); // slot 1: option
	ISA16_SLOT(config, m_slot[1], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 2: option
	ISA16_SLOT(config, m_slot[2], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 2: option
	ISA16_SLOT(config, m_slot[3], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 4: option
	ISA16_SLOT(config, m_slot[4], 0, m_isa, rtpc_isa16_cards, nullptr, false); // slot 5: coprocessor/option
	ISA16_SLOT(config, m_slot[5], 0, m_isa, rtpc_isa16_cards, "fdc",   false); // slot 6: disk/diskette adapter
}

ROM_START(ibm6150)
	ROM_REGION32_BE(0x10000, "ipl", 0)
	ROM_SYSTEM_BIOS(0, "ipl", "IPL")
	ROMX_LOAD("79x3456.bin", 0x00000, 0x4000, CRC(0a45a9ba) SHA1(02ca637c6a871c180dbfebf2ec68d8ec5a998c76), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3458.bin", 0x00001, 0x4000, CRC(7bd08ab6) SHA1(aabcfbb8fa1a5f8a08fb5cfd90ca6fe05258fde9), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3460.bin", 0x00002, 0x4000, CRC(897586e0) SHA1(528772635903f27235ebba2622b03386b84e4e17), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3462.bin", 0x00003, 0x4000, CRC(12aca906) SHA1(58f95b95768ef131d8d9d552506a9fe9c9c6077d), ROM_BIOS(0) | ROM_SKIP(3))
ROM_END

ROM_START(ibm6151)
	ROM_REGION32_BE(0x10000, "ipl", 0)
	ROM_SYSTEM_BIOS(0, "ipl", "IPL")
	ROMX_LOAD("79x3456.bin", 0x00000, 0x4000, CRC(0a45a9ba) SHA1(02ca637c6a871c180dbfebf2ec68d8ec5a998c76), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3458.bin", 0x00001, 0x4000, CRC(7bd08ab6) SHA1(aabcfbb8fa1a5f8a08fb5cfd90ca6fe05258fde9), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3460.bin", 0x00002, 0x4000, CRC(897586e0) SHA1(528772635903f27235ebba2622b03386b84e4e17), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("79x3462.bin", 0x00003, 0x4000, CRC(12aca906) SHA1(58f95b95768ef131d8d9d552506a9fe9c9c6077d), ROM_BIOS(0) | ROM_SKIP(3))
ROM_END

#define rom_rtpc010 rom_ibm6151
#define rom_rtpc015 rom_ibm6151
#define rom_rtpc020 rom_ibm6150
#define rom_rtpc025 rom_ibm6150
#define rom_rtpca25 rom_ibm6150

} // anonymous namespace

/*   YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT         COMPANY                             FULLNAME               FLAGS */
COMP(1986,  rtpc010,  0,      0,      ibm6151,  0,     rtpc_state,  empty_init,  "International Business Machines",  "IBM RT PC Model 010", MACHINE_NOT_WORKING)
COMP(1986,  rtpc015,  0,      0,      ibm6151,  0,     rtpc_state,  empty_init,  "International Business Machines",  "IBM RT PC Model 015", MACHINE_NOT_WORKING)
COMP(1986,  rtpc020,  0,      0,      ibm6150,  0,     rtpc_state,  empty_init,  "International Business Machines",  "IBM RT PC Model 020", MACHINE_NOT_WORKING)
COMP(1986,  rtpc025,  0,      0,      ibm6150,  0,     rtpc_state,  empty_init,  "International Business Machines",  "IBM RT PC Model 025", MACHINE_NOT_WORKING)
COMP(1986,  rtpca25,  0,      0,      ibm6150,  0,     rtpc_state,  empty_init,  "International Business Machines",  "IBM RT PC Model A25", MACHINE_NOT_WORKING)
