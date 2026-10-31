// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont, Chris Toshok, Vladimir Vukicevic
/*
 * apollo_dn300.cpp - Apollo DN300
 *
 * The Apollo DN300 (1983) is a Domain workstation of the SAU2 family: a
 * 68010 with Apollo's own MMU (translation through a page translation
 * table and a hashed page frame table), up to 1.5 MB of RAM, a 1024x800
 * monochrome display with a BLT engine, a Winchester/floppy controller
 * (ANSI X3.101 Winchesters, a uPD765A with an 8" floppy drive, and the
 * MSM5832 calendar on the same board), the Domain token ring network
 * controller, an HD63450 DMA controller, an MC6840 timer, an SCN2681
 * with the two serial lines (and the speaker on its output OP7) and an
 * MC6850 for the keyboard and mouse.
 *
 * Not emulated:
 *  - the ring network (the controller's registers and DMA are there, but
 *    a transmit goes nowhere and nothing is ever received)
 *  - memory parity errors
 *  - the Winchester controller's FORMAT_TRACK command
 *  - the RAM size: the map always has 1.5 MB
 *
 * Untested: the floppy.  The uPD765A, its DMA and an 8" drive are there
 * (the PROM and the operating system probe them), but no Apollo floppy
 * has been read or written.
 *
 * The DN320 (1984) is a DN300 with a floating-point unit (the PEB without
 * its cache); it waits for that unit's emulation.
 *
 *  see also:
 *  - Domain Series 3000/Series 4000 Hardware Architecture Handbook (Order No. 007861 Rev. 02)
 *  - Domain Personal Workstations and Servers Technical Reference (Apollo Order No. 008778-A01)
 *  - Servicing the Domain Personal Workstations and Servers (Apollo Order No. 007859-A01)
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/AEGIS_Internals_and_Data_Structures_Jan86.pdf
 *  - http://www.bitsavers.org/pdf/apollo/019411-A00_Addendum_to_Domain_Personal_Workstations_and_Servers_Hardware_Architecture_Handbook_1991.pdf
 *  - data sheets from Intel and Motorola
 */

#include "emu.h"

#include "apollo_dn300_disk.h"
#include "apollo_dn300_kbd.h"
#include "apollo_dn300_mmu.h"
#include "apollo_dn300_ring.h"
#include "apollo_dn300_v.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/hd63450.h"
#include "machine/mc68681.h"
#include "machine/ram.h"
#include "sound/beep.h"

#include "speaker.h"

#define LOG_BUSERR (1U << 1) // bus errors on the physical bus
#define LOG_DMA    (1U << 2) // DMA controller interrupt and end-of-transfer
#define LOG_REGS   (1U << 3) // the memory control/status registers and display 1

#define VERBOSE (0)
#include "logmacro.h"

#define LOGBUSERR(...) LOGMASKED(LOG_BUSERR, __VA_ARGS__)
#define LOGDMA(...)    LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGREGS(...)   LOGMASKED(LOG_REGS, __VA_ARGS__)


namespace {

#define MAINCPU "maincpu"

// 1536 KB of RAM
#define DN300_RAM_BASE 0x100000
#define DN300_RAM_END  0x27ffff

// interrupt levels
#define APOLLO_DN300_IRQ_SIO1 1
#define APOLLO_DN300_IRQ_KBD 2
#define APOLLO_DN300_IRQ_RING 3
#define APOLLO_DN300_IRQ_DISPLAY 4
#define APOLLO_DN300_IRQ_DISK 5
#define APOLLO_DN300_IRQ_PTM 6
#define APOLLO_DN300_IRQ_PARITY_ERROR 7

// channels for our DMAC
#define APOLLO_DN300_DMA_RING_RCVHEADER 0
#define APOLLO_DN300_DMA_RING_RCVDATA 1
#define APOLLO_DN300_DMA_RING_XMIT 2
#define APOLLO_DN300_DMA_DISK 3

// memory status register
#define APOLLO_DN300_MCSR_SR_PARITY_TRAP_ENABLED     0x0001
#define APOLLO_DN300_MCSR_SR_RIGHT_PARITY_ERROR      0x0002
#define APOLLO_DN300_MCSR_SR_LEFT_PARITY_ERROR       0x0004
#define APOLLO_DN300_MCSR_SR_PARITY_DURING_DMA_CYCLE 0x0008
#define APOLLO_DN300_MCSR_SR_FAILING_PPN_MASK        0xff00

// memory control register
#define APOLLO_DN300_MCSR_CR_PARITY_TRAP_ENABLE      0x01
#define APOLLO_DN300_MCSR_CR_FORCE_RIGHT_BYTE_PARITY 0x02
#define APOLLO_DN300_MCSR_CR_FORCE_LEFT_BYTE_PARITY  0x04

class apollo_dn300_state : public driver_device
{
public:
	apollo_dn300_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU),
		m_ram(*this, RAM_TAG),
		m_messram_ptr(*this, RAM_TAG),
		m_dmac(*this, "dma63450_1"),
		m_ptm(*this, "ptm"),
		m_sio(*this, "sio"),
		m_beep(*this, "beep"),
		m_acia(*this, "acia"),
		m_graphics(*this, "apollo_dn300_screen"),
		m_keyboard(*this, "keyboard"),
		m_mmu(*this, "apollo_dn300_mmu"),
		m_disk(*this, "apollo_dn300_disk"),
		m_ring(*this, "apollo_dn300_ring"),
		m_internal_leds(*this, "internal_led_%u", 1U)
	{ }

	void dn300(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint16_t> m_messram_ptr;

	required_device<hd63450_device> m_dmac;
	required_device<ptm6840_device> m_ptm;
	required_device<scn2681_device> m_sio;
	required_device<beep_device> m_beep;
	required_device<acia6850_device> m_acia;
	required_device<apollo_dn300_graphics> m_graphics;
	optional_device<apollo_dn300_kbd_device> m_keyboard;
	required_device<apollo_dn300_mmu_device> m_mmu;
	required_device<apollo_dn300_disk_ctrlr_device> m_disk;
	required_device<apollo_dn300_ring_ctrlr_device> m_ring;
	output_finder<4> m_internal_leds;

	uint16_t m_mem_status_register = 0;
	uint8_t m_mem_control_register = 0;

	void apollo_timers_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_timers_r(offs_t offset, uint16_t mem_mask = ~0);

	void apollo_display_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t apollo_display_r(offs_t offset, uint8_t mem_mask = ~0);

	void apollo_dn300_mcsr_status_register_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apollo_dn300_mcsr_status_register_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_dn300_mcsr_control_register_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t apollo_dn300_mcsr_control_register_r(offs_t offset, uint8_t mem_mask = ~0);

	uint16_t apollo_unmapped_r(offs_t offset, uint16_t mem_mask = ~0);
	void apollo_unmapped_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void apollo_bus_error(offs_t fault_addr, u8 rw);

	void dma_irq(int state);
	void dma_end(offs_t offset, uint8_t data);

	void apollo_reset_instr_callback(int state);

	void dn300_common(machine_config &config) ATTR_COLD;

	// hook a DMA channel to a device that moves bytes: 16- and 32-bit
	// operands are 2 or 4 byte cycles at ascending addresses, MSB first
	template <int Ch, typename R, typename W> void dma_channel(R rd, W wr)
	{
		m_dmac->dma8_read<Ch>().set(rd);
		m_dmac->dma8_write<Ch>().set(wr);
		m_dmac->dma16_read<Ch>().set([rd] (offs_t o) -> u16 { u16 const hi = rd(o); return (hi << 8) | rd(o + 1); });
		m_dmac->dma16_write<Ch>().set([wr] (offs_t o, u16 d) { wr(o, d >> 8); wr(o + 1, d & 0xff); });
		m_dmac->dma32_read<Ch>().set([rd] (offs_t o) -> u32 {
			u32 d = rd(o);
			d = (d << 8) | rd(o + 1);
			d = (d << 8) | rd(o + 2);
			return (d << 8) | rd(o + 3);
		});
		m_dmac->dma32_write<Ch>().set([wr] (offs_t o, u32 d) { wr(o, d >> 24); wr(o + 1, (d >> 16) & 0xff); wr(o + 2, (d >> 8) & 0xff); wr(o + 3, d & 0xff); });
	}

	void dn300_virtual_map(address_map &map) ATTR_COLD;
	void dn300_physical_map(address_map &map) ATTR_COLD;
};

/***************************************************************************
 apollo bus error
 ***************************************************************************/

void apollo_dn300_state::apollo_bus_error(offs_t fault_addr, u8 rw)
{
	if (machine().side_effects_disabled()) {
		return;
	}

	LOGBUSERR("%s: bus error at %06x (%s)\n", machine().describe_context(), fault_addr, rw ? "read" : "write");

	// a non-restartable bus error
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
}

void apollo_dn300_state::dma_irq(int state)
{
	LOGDMA("dma_irq: state=%d\n", state);
}

void apollo_dn300_state::dma_end(offs_t offset, uint8_t data)
{
	LOGDMA("dma_end: offset=%x data=%x\n", offset, data);
}

/***************************************************************************
 unmapped memory
 ***************************************************************************/

uint16_t apollo_dn300_state::apollo_unmapped_r(offs_t offset, uint16_t mem_mask)
{
	offs_t const address = (offset << 1) | (mem_mask == 0x00ff ? 1 : 0);

	/* unmapped; access causes a bus error */
	apollo_bus_error(address, 1);
	return 0xffff;
}

void apollo_dn300_state::apollo_unmapped_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offs_t const address = (offset << 1) | (mem_mask == 0x00ff ? 1 : 0);

	/* unmapped; access causes a bus error */
	apollo_bus_error(address, 0);
}

void apollo_dn300_state::apollo_timers_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// note: 16-bit offset gets used directly, as the LSB of each 16-bit word is
	// what is actually sent to the PTM in the appropriate same-offset register
	m_ptm->write(offset, data);
}

uint16_t apollo_dn300_state::apollo_timers_r(offs_t offset, uint16_t mem_mask)
{
	// note: 16-bit offset gets used directly, as the LSB of each 16-bit word is
	// what is actually sent to the PTM in the appropriate same-offset register
	return m_ptm->read(offset);
}

void apollo_dn300_state::apollo_display_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGREGS("writing display at offset %02x = %02x\n", offset, data);
}

uint8_t apollo_dn300_state::apollo_display_r(offs_t offset, uint8_t mem_mask)
{
	LOGREGS("reading display at offset %02x & %08x\n", offset, mem_mask);
	return 0;
}

/*-------------------------------------------------
 DN300/DN320 Memory Control Register at 0x8005
 -------------------------------------------------*/

void apollo_dn300_state::apollo_dn300_mcsr_control_register_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_mem_control_register);

	// the four LEDs, active low
	for (int i = 0; i < 4; i++)
		m_internal_leds[i] = BIT(m_mem_control_register, 7 - i);

	int const leds = ((m_mem_control_register >> 4) & 0xf) ^ 0xf;

	LOGREGS("writing Memory Control Register at offset %X = %04x & %04x (%04x - %d%d%d%d)\n",
			offset, data, mem_mask, m_mem_control_register,
			(leds >> 3) & 1, (leds >> 2) & 1, (leds >> 1) & 1, (leds >> 0) & 1);
}

uint8_t apollo_dn300_state::apollo_dn300_mcsr_control_register_r(offs_t offset, uint8_t mem_mask)
{
	LOGREGS("reading Memory Control Register at offset %X = %04x & %04x\n", offset, m_mem_control_register, mem_mask);
	return m_mem_control_register & mem_mask;
}

/*-------------------------------------------------
 DN300/DN320 Memory Status Register at 0x8006
 -------------------------------------------------*/

void apollo_dn300_state::apollo_dn300_mcsr_status_register_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// To clear parity error condition, write to the status register.
	// This register is readonly aside from this
	m_mem_status_register &= ~(APOLLO_DN300_MCSR_SR_RIGHT_PARITY_ERROR | APOLLO_DN300_MCSR_SR_LEFT_PARITY_ERROR);
	LOGREGS("writing Memory Status Register at offset %X = %04x & %04x (%04x)\n", offset, data, mem_mask, m_mem_status_register);
}

uint16_t apollo_dn300_state::apollo_dn300_mcsr_status_register_r(offs_t offset, uint16_t mem_mask)
{
	LOGREGS("reading Memory Status Register at offset %X = %04x & %04x\n", offset, m_mem_status_register, mem_mask);
	return m_mem_status_register & mem_mask;
}

/***************************************************************************
 ADDRESS MAPS
 ***************************************************************************/

// the CPU's program space: every cycle is translated by the MMU
void apollo_dn300_state::dn300_virtual_map(address_map &map)
{
	map(0x000000, 0xffffff).m(m_mmu, FUNC(apollo_dn300_mmu_device::virtual_map));
}

// the physical bus, owned by the MMU (its space 0)
void apollo_dn300_state::dn300_physical_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(apollo_dn300_state::apollo_unmapped_r), FUNC(apollo_dn300_state::apollo_unmapped_w));
	map(0x000000, 0x003fff).nopw().rom().region(MAINCPU, 0); /* boot ROM  */

	map(0x008000, 0x0083ff).rw(m_mmu, FUNC(apollo_dn300_mmu_device::unk_r), FUNC(apollo_dn300_mmu_device::unk_w));
	map(0x008000, 0x008001).rw(m_mmu, FUNC(apollo_dn300_mmu_device::pid_priv_power_r), FUNC(apollo_dn300_mmu_device::pid_priv_power_w));
	map(0x008002, 0x008002).rw(m_mmu, FUNC(apollo_dn300_mmu_device::fpu_owner_r), FUNC(apollo_dn300_mmu_device::fpu_owner_w));
	map(0x008003, 0x008003).rw(m_mmu, FUNC(apollo_dn300_mmu_device::status_r), FUNC(apollo_dn300_mmu_device::status_w));
	map(0x004000, 0x007fff).rw(m_mmu, FUNC(apollo_dn300_mmu_device::pft_r), FUNC(apollo_dn300_mmu_device::pft_w));
	map(0x700000, 0x7fffff).rw(m_mmu, FUNC(apollo_dn300_mmu_device::ptt_r), FUNC(apollo_dn300_mmu_device::ptt_w));

	// the memory control and status registers, beside the MMU's
	map(0x008005, 0x008005).rw(FUNC(apollo_dn300_state::apollo_dn300_mcsr_control_register_r), FUNC(apollo_dn300_state::apollo_dn300_mcsr_control_register_w));
	map(0x008006, 0x008007).rw(FUNC(apollo_dn300_state::apollo_dn300_mcsr_status_register_r), FUNC(apollo_dn300_state::apollo_dn300_mcsr_status_register_w));

	// both serial chips are on the low byte lane: register n of the 2681 at
	// 0x8401 + 2n, the 6850's status and control at 0x8421, its data at 0x8423
	map(0x008400, 0x00841f).rw(m_sio, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x008420, 0x008421).rw(m_acia, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).umask16(0x00ff);
	map(0x008422, 0x008423).rw(m_acia, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).umask16(0x00ff);

	map(0x008800, 0x008bff).rw(FUNC(apollo_dn300_state::apollo_timers_r), FUNC(apollo_dn300_state::apollo_timers_w));

	map(0x009000, 0x0093ff).rw(m_dmac, FUNC(hd63450_device::read), FUNC(hd63450_device::write)); // docs make it seem like this is just 0x9000 - 0x90ff
	map(0x009400, 0x0097ff).rw(FUNC(apollo_dn300_state::apollo_display_r), FUNC(apollo_dn300_state::apollo_display_w)); // docs call this "display 1"

	map(0x009400, 0x00940f).rw(m_graphics, FUNC(apollo_dn300_graphics::reg_r), FUNC(apollo_dn300_graphics::reg_w));

	map(0x009800, 0x009bff).m(m_ring, FUNC(apollo_dn300_ring_ctrlr_device::map));
	map(0x009c00, 0x009fff).m(m_disk, FUNC(apollo_dn300_disk_ctrlr_device::map));

	map(0x020000, 0x03ffff).rw(m_graphics, FUNC(apollo_dn300_graphics::mem_r), FUNC(apollo_dn300_graphics::mem_w)); // docs call this "disp1 mem"

	map(DN300_RAM_BASE, DN300_RAM_END).ram().share(RAM_TAG);
}

/***************************************************************************
 Machine Reset
 ***************************************************************************/

void apollo_dn300_state::apollo_reset_instr_callback(int state)
{
	// reset the CPU board devices
	m_graphics->reset();
	m_keyboard->reset();
}

/***************************************************************************
 Machine Start
 ***************************************************************************/

void apollo_dn300_state::machine_start()
{
	// clear ram
	memset(m_messram_ptr.target(), 0x00, m_messram_ptr.bytes());

	save_item(NAME(m_mem_status_register));
	save_item(NAME(m_mem_control_register));
}

/***************************************************************************
 MACHINE DRIVERS
 ***************************************************************************/

void apollo_dn300_state::dn300_common(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, 16000000); /* 16 MHz 68010 with an external mmu */
	// the CPU's program space is the virtual view through the MMU, which owns the physical bus
	m_maincpu->set_addrmap(AS_PROGRAM, &apollo_dn300_state::dn300_virtual_map);

	config.set_maximum_quantum(attotime::from_hz(60));

	PTM6840(config, m_ptm, 0);
	m_ptm->set_external_clocks(250000, 125000, 62500);
	m_ptm->irq_callback().set_inputline(m_maincpu, APOLLO_DN300_IRQ_PTM);

	// the cpu board has a 4.9152 MHz oscillator can which passes through 2 /16
	// dividers, resulting in 19200.
	ACIA6850(config, m_acia, 0);
	clock_device &acia_clock(CLOCK(config, "acia_clock", 19200));
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	// 9000-903F - ring receive header
	// 9040-907F - ring receive data
	// 9080-90BF - ring transmit
	// 90CO-90FF - winchester/floppy
	HD63450(config, m_dmac, 8'000'000, m_mmu, 0); // DMA is on the physical bus
	m_dmac->set_clocks(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2));
	// the DN300 programs burst mode with external request (DCR 0x28, OCR 0x12/0x92):
	// transfers follow DRQ, repeating at the channel clock while DRQ stays asserted
	m_dmac->set_burst_clocks(attotime::from_usec(2), attotime::from_nsec(450), attotime::from_usec(4), attotime::from_hz(15625/2));
	m_dmac->irq_callback().set(FUNC(apollo_dn300_state::dma_irq));
	m_dmac->dma_end().set(FUNC(apollo_dn300_state::dma_end));
	// the devices move bytes; word and long operands are that many byte
	// cycles at consecutive addresses, most significant byte first
	dma_channel<APOLLO_DN300_DMA_RING_RCVHEADER>(
			[this] (offs_t o) { return m_ring->rcv_header_read_byte(o); },
			[this] (offs_t o, u8 d) { m_ring->rcv_header_write_byte(o, d); });
	dma_channel<APOLLO_DN300_DMA_RING_RCVDATA>(
			[this] (offs_t o) { return m_ring->rcv_data_read_byte(o); },
			[this] (offs_t o, u8 d) { m_ring->rcv_data_write_byte(o, d); });
	dma_channel<APOLLO_DN300_DMA_RING_XMIT>(
			[this] (offs_t o) { return m_ring->transmit_read_byte(o); },
			[this] (offs_t o, u8 d) { m_ring->transmit_write_byte(o, d); });
	dma_channel<APOLLO_DN300_DMA_DISK>(
			[this] (offs_t o) { return m_disk->dma_read_byte(o); },
			[this] (offs_t o, u8 d) { m_disk->dma_write_byte(o, d); });

	SCN2681(config, m_sio, 3.6864_MHz_XTAL);
	m_sio->irq_cb().set_inputline(m_maincpu, APOLLO_DN300_IRQ_SIO1);

	// the two serial lines
	m_sio->a_tx_cb().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_sio->b_tx_cb().set("rs232b", FUNC(rs232_port_device::write_txd));
	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(scn2681_device::rx_a_w));
	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(scn2681_device::rx_b_w));

	// the speaker: the tone sounds while output OP7 is high
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.50);
	m_sio->outport_cb().set([this] (u8 data) { m_beep->set_state(BIT(data, 7)); });

	APOLLO_DN300_KBD(config, m_keyboard, 0);
	m_keyboard->tx_cb().set(m_acia, FUNC(acia6850_device::write_rxd));
	m_acia->irq_handler().set_inputline(m_maincpu, APOLLO_DN300_IRQ_KBD);

	APOLLO_DN300_GRAPHICS(config, m_graphics, 0);
	m_graphics->irq_callback().set_inputline(m_maincpu, APOLLO_DN300_IRQ_DISPLAY);

	APOLLO_DN300_MMU(config, m_mmu, 0);
	m_mmu->set_cpu(m_maincpu);
	m_mmu->set_addrmap(0, &apollo_dn300_state::dn300_physical_map);

	APOLLO_DN300_DISK_CTRLR(config, m_disk, 0);
	m_disk->irq_callback().set_inputline(m_maincpu, APOLLO_DN300_IRQ_DISK);
	m_disk->drq_wr_callback().set(m_dmac, FUNC(hd63450_device::drq3_w));

	APOLLO_DN300_RING_CTRLR(config, m_ring, 0);
	m_ring->irq_callback().set_inputline(m_maincpu, APOLLO_DN300_IRQ_RING);
	m_ring->rcv_header_drq_wr_callback().set(m_dmac, FUNC(hd63450_device::drq0_w));
	m_ring->rcv_data_drq_wr_callback().set(m_dmac, FUNC(hd63450_device::drq1_w));
	m_ring->transmit_data_drq_wr_callback().set(m_dmac, FUNC(hd63450_device::drq2_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1536K").set_extra_options("512K,1M,1536K");
}

void apollo_dn300_state::dn300(machine_config &config)
{
	dn300_common(config);

	// the RESET instruction resets the display and the keyboard
	m_maincpu->reset_cb().set(FUNC(apollo_dn300_state::apollo_reset_instr_callback));
}

/***************************************************************************
 ROM Definitions
 ***************************************************************************/

ROM_START( dn300 )
	ROM_REGION( 0x04000, MAINCPU, 0 ) /* 68000 code */

	ROM_SYSTEM_BIOS( 0, "md-rev-5", "MD REV 5, 1983/08/31" )
	ROMX_LOAD( "300_boot.bin", 0x00000, 0x04000, CRC(0b8d94e7) SHA1(38085f10590f285f8378dfca4fd13d18fa476a74), ROM_BIOS(0) )
ROM_END

} // anonymous namespace

/***************************************************************************
 GAME DRIVERS
 ***************************************************************************/

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS               INIT        COMPANY   FULLNAME        FLAGS */
COMP( 1983, dn300, 0,      0,      dn300,   0,     apollo_dn300_state, empty_init, "Apollo", "Apollo DN300", 0 )
