// license:BSD-3-Clause
// copyright-holders:Brice Onken
// thanks-to:Patrick Mackinlay,Olivier Galibert,Tsubai Masanari

/*
 * Sony NEWS R4000/4400 APbus-based workstations
 *
 * Sources and more information:
 *   - http://ozuma.o.oo7.jp/nws5000x.htm
 *   - https://katsu.watanabe.name/doc/sonynews/
 *   - https://web.archive.org/web/20170202100940/www3.videa.or.jp/NEWS/
 *   - https://github.com/NetBSD/src/tree/trunk/sys/arch/newsmips
 *   - https://web.archive.org/web/19970713173157/http%3A%2F%2Fwww1.sony.co.jp%2FOCMP%2F
 *   - https://github.com/briceonk/news-os
 *
 *  CPU configuration:
 *  - CPU card has a 75MHz crystal, multiplier (if any) TBD
 *  - PRId = 0x450
 *  - config register = 0x1081E4BF
 *      [31]    CM = 0 (MC mode off)
 *      [30:28] EC = 001 (clock frequency divided by 3)
 *      [27:24] EP = 00000 (doubleword every cycle)
 *      [23:22] SB = 10 (16 word scache line size)
 *      [21]    SS = 0 (unified scache)
 *      [20]    SW = 0 (128-bit data path to scache)
 *      [19:18] EW = 0 (64-bit system port width)
 *      [17]    SC = 0 (scache present)
 *      [16]    SM = 1 (Dirty Shared state disabled)
 *      [15]    BE = 1 (Big endian)
 *      [14]    EM = 1 (Parity mode)
 *      [13]    EB = 1 (Sub-block ordering)
 *      [12]    Reserved (0)
 *      [11:9]  IC = 2 (16 KByte icache)
 *      [8:6]   DC = 2 (16 KByte dcache)
 *      Per page 90 of the R4400 user guide, the following bits ([5:0]) are mutable by software at runtime.
 *      [5]     IB = 1 (32 byte icache line)
 *      [4]     DB = 1 (32 byte icache line)
 *      [3]     CU = 1 (SC uses cacheable coherent update on write)
 *      [2:0]   K0 = 7 (cache coherency algo, 7 is reserved)
 *  - Known R4400 config differences between this driver and the physical platform:
 *      - emulated R4400 sets revision to 40 instead of 50. The user manual warns against using the revision field of PRId
 *        in software, so hopefully that won't cause any deltas in behavior before that can be configured.
 *      - emulated SM (Dirty Shared state) is on by default - however, is SM actually being emulated?
 *      - emulated CU and K0 are all 0 instead of all 1 like on the physical platform. Unlike SM, software can set these.
 *      - In general, the secondary cache isn't emulated, which might influence bits 3:0 of the config register.
 *
 *  General Emulation Status (major chips only, there are additional smaller chips including CPLDs on the boards)
 *  CPU card:
 *   - MIPS R4400: emulated, with the caveats above
 *   - 10x Motorola MCM67A618FN12 SRAMs (secondary cache): partially emulated
 *     - Tag manipulation is implemented. This seems to be enough for NEWS-OS 4 to work.
 *  Motherboard:
 *   - Sony CXD8490G, CXD8491G, CXD8492G, CXD8489G (unknown ASICs): not emulated
 *   - Main memory: partially emulated (memory controller is unknown and barely emulated - memory configurations other than 64MB don't work at the moment)
 *  I/O board:
 *   - Sony CXD8409Q Parallel Interface: not emulated
 *   - National Semi PC8477B Floppy Controller: emulated (-A differences unemulated but seem to be unneeded)
 *   - Zilog Z8523010VSC ESCC serial interface: emulated (see following)
 *   - Sony CXD8421Q WSC-ESCC1 serial APbus interface controller: skeleton (ESCC connections, probably DMA, APbus interface, etc. handled by this chip)
 *   - 2x Sony CXD8442Q WSC-FIFO APbus FIFO/interface chips: partially emulated (handles APbus connections and DMA for sound, floppy, etc.)
 *   - National Semi DP83932B-VF SONIC Ethernet controller: emulated
 *   - Sony CXD8452AQ WSC-SONIC3 SONIC Ethernet APbus interface controller: mostly(?) emulated
 *   - Sony CXD8418Q WSC-PARK3: not fully emulated, but some of the general platform functions may come from this chip (most likely a gate array based on what the PARK2 was in older gen NEWS systems)
 *   - Sony CXD8403Q DMAC3Q DMA controller: Partially emulated
 *   - 2x HP 1TV3-0302 SPIFI3 SCSI controllers: Partially emulated, only works with the monitor ROM and NEWS-OS 4 right now.
 *   - ST Micro M58T02-150PC1 Timekeeper RAM: emulated
 *  DSC-39 XB Framebuffer/video card:
 *   - Sony CXD8486Q XB: not emulated (most likely APbus interface)
 *   - 16x NEC D482235G5 Dual Port Graphics Buffers: not emulated
 *   - Brooktree Bt468KG220 RAMDAC: not emulated
 *
 *  Known issues:
 *  - Monitor ROM command `ss -r` doesn't show most register values. Not sure what mechanism the APmonitor uses to get the register dump.
 *    (TLB dump is also broken, but unlike the register dump, the TLB dump is broken on the real NWS-5000X too.
 *     Use the `mp` monitor command instead for a working version that shows the TLB correctly, both on real hardware and in emulation)
 *  - Reboot/halt+boot fails because the ESCC FIFO isn't reset properly. For now, the emulator must be hard reset.
 *    It prints out a message (`esccf0: ai->ai_addr points AProm`) and then tries to allocate and use a different FIFO (which fails)
 *  - The NetBSD installer has some weird issues (selects SCSI ID 0 regardless of the ID, some serial weirdness that NEWS-OS doesn't have, etc)
 *
 *  TODO (in no particular order):
 *  - More complete floppy support (only supports floppy boot at the moment)
 *  - NetBSD SCSI support (mostly/entirely changes needed in the SPIFI driver)
 *  - Sound (lots of undocumented hardware)
 *  - Parallel I/O
 *  - Framebuffer (and remaining kb/ms support)
 *  - APbus expansion slots
 *  - Triage and fix the known issues mentioned above
 *  - Full save state support
 */

#include "emu.h"

#include "cxd8442q.h"
#include "cxd8452aq.h"
#include "dmac3.h"
#include "news_hid.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "cpu/mips/r4000.h"
#include "imagedev/floppy.h"
#include "machine/dp83932c.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/spifi3.h"
#include "machine/timekpr.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"

#define LOG_INTERRUPT (1U << 1)
#define LOG_ALL_INTERRUPT (1U << 2)
#define LOG_LED (1U << 3)
#define LOG_MEMORY (1U << 4)
#define LOG_APBUS (1U << 5)
#define LOG_ESCC (1U << 6)

#define NEWS_R4K_INFO (LOG_LED)
#define NEWS_R4K_DEBUG (NEWS_R4K_INFO | LOG_GENERAL | LOG_INTERRUPT)
#define NEWS_R4K_TRACE (NEWS_R4K_DEBUG | LOG_ALL_INTERRUPT | LOG_APBUS)
#define NEWS_R4K_MAX (NEWS_R4K_TRACE | LOG_MEMORY | LOG_ESCC)

#define VERBOSE NEWS_R4K_INFO

#include "logmacro.h"

namespace {

class news_r4k_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	news_r4k_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, "cpu"),
		m_ram(*this, "ram"),
		m_dma_ram(*this, "dmaram"),
		m_net_ram(*this, "netram"),
		m_rtc(*this, "rtc"),
		m_escc(*this, "escc"),
		m_serial(*this, "serial%u", 0U),
		m_fifo0(*this, "apfifo0"),
		m_fifo1(*this, "apfifo1"),
		m_sonic3(*this, "sonic3"),
		m_sonic(*this, "sonic"),
		m_fdc(*this, "fdc"),
		m_hid(*this, "hid"),
		m_dmac(*this, "dmac"),
		m_scsi0(*this, "scsi0:7:spifi3"),
		m_scsi1(*this, "scsi1:7:spifi3"),
		m_scsibus0(*this, "scsi0"),
		m_scsibus1(*this, "scsi1"),
		m_dip_switch(*this, "FRONT_PANEL"),
		m_led(*this, "led%u", 0U)
	{
	}

	void nws5000x(machine_config &config);

protected:

	enum irq0_number : uint32_t
	{
		DMAC = 0x01,
		SONIC = 0x02,
		FDC = 0x10
	};

	enum irq1_number : uint32_t
	{
		KBD = 0x01,
		ESCC = 0x02,
		AUDIO0 = 0x04,
		AUDIO1 = 0x08,
		PARALLEL = 0x20,
		FB = 0x80
	};

	enum irq2_number : uint32_t
	{
		TIMER0 = 0x01,
		TIMER1 = 0x02
	};

	enum irq4_number : uint32_t
	{
		APBUS = 0x01
	};

	enum escc_channel
	{
		CHA,
		CHB
	};

	enum APBUS_PTE_MASKS
	{
		ENTRY_VALID = 0x80000000,
		ENTRY_COHERENT = 0x40000000,
		ENTRY_PAD = 0x3FF00000,
		ENTRY_PAD_SHIFT = 20,
		ENTRY_PFNUM = 0xFFFFF
	};

	// APbus DMA page table entry structure
	// See https://github.com/NetBSD/src/blob/faa527952534272f8f4737347d9fc8c38e070c9c/sys/arch/newsmips/apbus/dmac3reg.h#L71
	struct apbus_pte
	{
		// Unused
		uint32_t pad;
		// Entry is OK to use
		bool valid;
		// DMA coherence enabled (don't care for emulation??)
		bool coherent;
		// Unused
		uint32_t pad2;
		// Page number (upper 20 bits of physical address)
		uint32_t pfnum;
	};

	// Interrupts and other platform state
	bool m_ram_map_shift = false;
	bool m_int_state[6] = {false, false, false, false, false, false};
	uint32_t m_inten[6] = {0, 0, 0, 0, 0, 0};
	uint32_t m_intst[6] = {0, 0, 0, 0, 0, 0};
	uint32_t escc1_int_status = 0;
	memory_access<64, 3, 0, ENDIANNESS_BIG>::cache m_main_cache;

	// Freerun timer (1us period)
	// NetBSD source code corroborates the period (https://github.com/NetBSD/src/blob/229cf3aa2cda57ba5f0c244a75ae83090e59c716/sys/arch/newsmips/newsmips/news5000.c#L259)
	emu_timer *m_freerun_timer;
	uint32_t m_freerun_timer_val;

	// Hardware timer TIMER0 (10ms period)
	// TODO: Is this programmable somehow? 100Hz is what the NetBSD kernel expects.
	emu_timer *m_timer0_timer;

	// MIPS R4400 CPU
	required_device<r4400_device> m_cpu;

	// Memory
	required_device<ram_device> m_ram;
	required_device<ram_device> m_dma_ram;
	required_device<ram_device> m_net_ram;

	// ST Micro M48T02 Timekeeper NVRAM + RTC
	required_device<m48t02_device> m_rtc;

	// Zilog ESCC and Sony CXD8421Q ESCC1 serial controller
	required_device<z80scc_device> m_escc;
	required_device_array<rs232_port_device, 2> m_serial;

	// Sony CXD8442Q APbus FIFOs
	required_device<cxd8442q_device> m_fifo0;
	required_device<cxd8442q_device> m_fifo1;

	// Sony CXD8452AQ WSC-SONIC3 APbus interface
	required_device<cxd8452aq_device> m_sonic3;

	// SONIC ethernet controller
	required_device<dp83932c_device> m_sonic;

	// National Semiconductor PC8477B floppy controller
	required_device<pc8477b_device> m_fdc;

	// NEWS keyboard and mouse
	required_device<news_hid_hle_device> m_hid;

	// DMAC3 DMA controller
	required_device<dmac3_device> m_dmac;

	// HP SPIFI3 SCSI controllers
	required_device<spifi3_device> m_scsi0;
	required_device<spifi3_device> m_scsi1;
	required_device<nscsi_bus_device> m_scsibus0;
	required_device<nscsi_bus_device> m_scsibus1;

	// Front panel
	required_ioport m_dip_switch;
	output_finder<6> m_led;

	// Constants
	static constexpr uint32_t ICACHE_SIZE = 0x4000;
	static constexpr uint32_t DCACHE_SIZE = 0x4000;
	static constexpr uint32_t SCACHE_SIZE = 0x100000;
	static constexpr int FREERUN_FREQUENCY = 1000000; // Hz
	static constexpr int TIMER0_FREQUENCY = 100;      // Hz
	static constexpr uint32_t APBUS_DMA_MAP_ADDRESS = 0x14c20000;
	static constexpr uint32_t APBUS_DMA_MAP_RAM_SIZE = 0x20000; // 128 kibibytes
	static constexpr const char* MAIN_MEMORY_DEFAULT = "64M";
	static constexpr int interrupt_map[6] = {0, 1, 2, 3, 4, 5};

	static constexpr std::string_view LED_MAP[6] = {"LED_POWER", "LED_DISK", "LED_FLOPPY", "LED_SEC", "LED_NET", "LED_CD"};
	std::map<offs_t, std::string_view> m_apbus_cmd_decoder
	{
		{0x0c, "INTMSK"},   // Interrupt mask
		{0x14, "INSTST"},   // Interrupt status
		{0x1c, "BER_ADDR"}, // Bus error address
		{0x34, "CFG_CTRL"}, // config control
		{0x5c, "DER_A"},    // DMA error address
		{0x6c, "DER_S"},    // DMA error slot
		{0x84, "DMA"}       // unmapped DMA coherency (?)
	};

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void machine_common(machine_config &config);

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;
	void sonic3_map(address_map &map) ATTR_COLD;
	void cpu_map_main_memory(address_map &map) ATTR_COLD;
	void cpu_map_debug(address_map &map) ATTR_COLD;

	// Interrupts
	// See news5000 section of https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/include/adrsmap.h
	void inten_w(offs_t offset, uint32_t data);
	uint32_t inten_r(offs_t offset);
	uint32_t intst_r(offs_t offset);
	void intclr_w(offs_t offset, uint32_t data);
	void generic_irq_w(uint32_t irq, uint32_t mask, int state);

	template <irq0_number Number>
	void irq_w(int state) { generic_irq_w(0, Number, state); }
	template <irq1_number Number>
	void irq_w(int state) { generic_irq_w(1, Number, state); }
	template <irq2_number Number>
	void irq_w(int state) { generic_irq_w(2, Number, state); }
	template <irq4_number Number>
	void irq_w(int state) { generic_irq_w(4, Number, state); }
	void int_check();

	void led_state_w(offs_t offset, uint32_t data);
	uint64_t front_panel_r(offs_t offset);

	TIMER_CALLBACK_MEMBER(freerun_clock);
	uint32_t freerun_r(offs_t offset);
	void freerun_w(offs_t offset, uint32_t data);

	TIMER_CALLBACK_MEMBER(timer0_clock);
	void timer0_w(offs_t offset, uint32_t data);

	// APbus control (should be split into a device eventually)
	uint8_t apbus_cmd_r(offs_t offset);
	void apbus_cmd_w(offs_t offset, uint32_t data);
	uint32_t apbus_virt_to_phys(uint32_t v_address);

	// RAM accessors
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	// CXD8421Q glue logic and secondary control for the ESCC
	// This belongs in a separate device if the other features are ever implemented.
	uint32_t escc1_int_status_r(offs_t offset);
	void escc1_int_status_w(offs_t offset, uint32_t data);
	uint32_t escc_ch_read(escc_channel channel, offs_t offset);
	void escc_ch_write(escc_channel channel, offs_t offset, uint32_t data);
	template <escc_channel Channel>
	uint32_t escc_ch_read(offs_t offset) { return escc_ch_read(Channel, offset); }
	template <escc_channel Channel>
	void escc_ch_write(offs_t offset, uint32_t data) { escc_ch_write(Channel, offset, data); }
};

/*
 * news_scsi_devices
 *
 * Declares the SCSI devices supported by this driver.
 */
static void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM_NEWS);
}

/*
 * machine_common
 *
 * Creates an emulated R4400-based NEWS platform.
 */
void news_r4k_state::machine_common(machine_config &config)
{
	// TODO: clock divisor support (config[30:28] = 0xb001) in r4000.cpp
	R4400(config, m_cpu, 75_MHz_XTAL, true, SCACHE_SIZE, 0x40);
	m_cpu->set_addrmap(AS_PROGRAM, &news_r4k_state::cpu_map);

	RAM(config, m_ram);
	m_ram->set_default_size(MAIN_MEMORY_DEFAULT);

	RAM(config, m_dma_ram);
	m_dma_ram->set_default_size("128KB");

	RAM(config, m_net_ram);
	m_net_ram->set_default_size("32KB");

	M48T02(config, m_rtc);

	SCC85230(config, m_escc, 9.8304_MHz_XTAL);

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxa_w));
	m_escc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_escc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_escc->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxb_w));
	m_escc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_escc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_escc->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_escc->out_int_callback().set(FUNC(news_r4k_state::irq_w<ESCC>));

	CXD8442Q(config, m_fifo0, 0);
	CXD8442Q(config, m_fifo1, 0);

	// Reverse polarity for ESCC DMA signals
	m_escc->out_dtra_callback().set(
			[this] (int status)
			{ m_fifo1->drq_w<cxd8442q_device::fifo_channel_number::CH2>(!status); });
	m_escc->out_wreqa_callback().set(
			[this] (int status)
			{ m_fifo1->drq_w<cxd8442q_device::fifo_channel_number::CH3>(!status); });
	m_fifo1->dma_w_cb<cxd8442q_device::fifo_channel_number::CH2>().set(
			[this] (uint32_t data)
			{ m_escc->da_w(0, data); });
	m_fifo1->dma_r_cb<cxd8442q_device::fifo_channel_number::CH3>().set(
			[this] ()
			{ return m_escc->da_r(0); });
	m_fifo1->out_int_callback().set(
			[this] (int status)
			{
				// Not sure if this is the actual mapping, or if sending any level 0 interrupt causes
				// NEWS-OS to scan the ESCC and FIFO registers. It seems to work OK, but should be
				// revisited if the exact details can be worked out.
				generic_irq_w(0, 0x4, status);
				escc1_int_status = status ? 0x8 : 0x0; // guess
			});

	CXD8452AQ(config, m_sonic3, 0);
	m_sonic3->set_addrmap(0, &news_r4k_state::sonic3_map);
	m_sonic3->irq_out().set(FUNC(news_r4k_state::irq_w<irq0_number::SONIC>));
	m_sonic3->set_bus(m_cpu, 0);
	m_sonic3->set_apbus_address_translator(FUNC(news_r4k_state::apbus_virt_to_phys));

	// TODO: actual clock frequency - there is a 20MHz crystal nearby on the board, but traces need to be confirmed
	DP83932C(config, m_sonic, 20'000'000);
	m_sonic->out_int_cb().set(m_sonic3, FUNC(cxd8452aq_device::irq_w));
	m_sonic->set_bus(m_sonic3, 1);

	// Use promiscuous mode to force network driver to accept all packets, since SONIC has its own filter (CAM table)
	// Not sure if needing to use this means something else isn't set up correctly.
	m_sonic->set_promisc(true);

	// Unlike 68k and R3000 NEWS machines, the keyboard and mouse seem to share an interrupt
	// See https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/ms_ap.c#L103
	// where the mouse interrupt handler is initialized using the Keyboard interrupt.
	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(FUNC(news_r4k_state::irq_w<KBD>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(FUNC(news_r4k_state::irq_w<KBD>));

	PC8477B(config, m_fdc, 24_MHz_XTAL, pc8477b_device::mode_t::PS2);
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// Note - FDC IRQ might go through the FIFO first. Might need to be updated in the future.
	m_fdc->intrq_wr_callback().set(FUNC(news_r4k_state::irq_w<irq0_number::FDC>));
	m_fdc->drq_wr_callback().set(
			[this] (int status)
			{ m_fifo0->drq_w<cxd8442q_device::fifo_channel_number::CH2>(status); });
	m_fifo0->dma_r_cb<cxd8442q_device::fifo_channel_number::CH2>().set(
			[this] ()
			{ return uint32_t(m_fdc->dma_r()); });

	DMAC3(config, m_dmac, 0);
	m_dmac->set_apbus_address_translator(FUNC(news_r4k_state::apbus_virt_to_phys));
	m_dmac->set_bus(m_cpu, 0);
	m_dmac->irq_out().set(FUNC(news_r4k_state::irq_w<DMAC>));

	NSCSI_BUS(config, m_scsibus0);
	NSCSI_BUS(config, m_scsibus1);

	NSCSI_CONNECTOR(config, "scsi0:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", news_scsi_devices, "cdrom");

	NSCSI_CONNECTOR(config, "scsi1:0", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", news_scsi_devices, nullptr);

	// TODO: Actual SPIFI3 clock frequency
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("spifi3", SPIFI3)
		.clock(16'000'000)
		.machine_config(
			[this](device_t *device)
			{
				spifi3_device &adapter = dynamic_cast<spifi3_device &>(*device);
				adapter.irq_handler_cb().set(m_dmac, FUNC(dmac3_device::irq_w<dmac3_device::CTRL0>));
				adapter.drq_handler_cb().set(m_dmac, FUNC(dmac3_device::drq_w<dmac3_device::CTRL0>));
			});
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("spifi3", SPIFI3)
		.clock(16'000'000)
		.machine_config(
			[this](device_t *device)
			{
				spifi3_device &adapter = dynamic_cast<spifi3_device &>(*device);
				adapter.irq_handler_cb().set(m_dmac, FUNC(dmac3_device::irq_w<dmac3_device::CTRL1>));
				adapter.drq_handler_cb().set(m_dmac, FUNC(dmac3_device::drq_w<dmac3_device::CTRL1>));
			});

	m_dmac->dma_r_cb<dmac3_device::CTRL0>().set(m_scsi0, FUNC(spifi3_device::dma_r));
	m_dmac->dma_w_cb<dmac3_device::CTRL0>().set(m_scsi0, FUNC(spifi3_device::dma_w));
	m_dmac->dma_r_cb<dmac3_device::CTRL1>().set(m_scsi1, FUNC(spifi3_device::dma_r));
	m_dmac->dma_w_cb<dmac3_device::CTRL1>().set(m_scsi1, FUNC(spifi3_device::dma_w));

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("RISC,NWS5000");
}

void news_r4k_state::nws5000x(machine_config &config) { machine_common(config); }

/*
 * cpu_map
 *
 * Assign the address map for the CPU
 * References:
 *  - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/include/adrsmap.h
 *  - APmonitor device table (Run APmonitor command `ss -d` for details)
 */
void news_r4k_state::cpu_map(address_map &map)
{
	// Unmapping to high causes the APmonitor to wait a huge amount of time in between
	// printing serial characters - what is weird is that manually setting the unmapped
	// reads to return zero while unmapping everything high doesn't fix this issue, so
	// there must be a side effect caused by unmapping high somewhere.
	map.unmap_value_low();

	cpu_map_main_memory(map);

	// NEWS firmware
	map(0x1fc00000, 0x1fc3ffff).rom().region("mrom", 0);
	map(0x1f3c0000, 0x1f3c03ff).rom().region("idrom", 0);
	map(0x1e600000, 0x1e6003ff).rom().region("macrom", 0);

	// Front panel DIP switches
	map(0x1f3d0000, 0x1f3d0007).r(FUNC(news_r4k_state::front_panel_r));

	// Hardware timers
	map(0x1f800000, 0x1f800003).w(FUNC(news_r4k_state::timer0_w));
	map(0x1f840000, 0x1f840003).rw(FUNC(news_r4k_state::freerun_r), FUNC(news_r4k_state::freerun_w));

	// Timekeeper NVRAM and RTC
	map(0x1f880000, 0x1f881fff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask32(0x000000ff);

	// Interrupt ports
	map(0x1f4e0000, 0x1f4e0017).w(FUNC(news_r4k_state::intclr_w));
	map(0x1fa00000, 0x1fa00017).rw(FUNC(news_r4k_state::inten_r), FUNC(news_r4k_state::inten_w));
	map(0x1fa00020, 0x1fa00037).r(FUNC(news_r4k_state::intst_r));

	// LEDs
	map(0x1f3f0000, 0x1f3f0017).w(FUNC(news_r4k_state::led_state_w));

	// APbus
	map(0x14c00000, 0x14c0008f).rw(FUNC(news_r4k_state::apbus_cmd_r), FUNC(news_r4k_state::apbus_cmd_w));
	map(0x14c20000, 0x14c3ffff).lrw8(NAME([this](offs_t offset)
										  { return m_dma_ram->read(offset); }),
									 NAME([this](offs_t offset, uint8_t data)
										  {
											  LOGMASKED(LOG_MEMORY, "Write to DMA map @ offset 0x%x: 0x%x (%s)\n", offset, data, machine().describe_context());
											  m_dma_ram->write(offset, data);
										  }));

	// ESCC + WSC-ESCC1 (CXD8421Q) serial controller
	map(0x1e900000, 0x1e93ffff).m(m_fifo1, FUNC(cxd8442q_device::map));
	map(0x1e940000, 0x1e94000f).rw(FUNC(news_r4k_state::escc_ch_read<CHB>), FUNC(news_r4k_state::escc_ch_write<CHB>));
	map(0x1e950000, 0x1e95000f).rw(FUNC(news_r4k_state::escc_ch_read<CHA>), FUNC(news_r4k_state::escc_ch_write<CHA>));
	map(0x1e960000, 0x1e960003).rw(FUNC(news_r4k_state::escc1_int_status_r), FUNC(news_r4k_state::escc1_int_status_w));

	// Interestingly, this is also the RAM that is used before main memory is initialized
	// It is also the only RAM avaliable if "no memory mode" is set (DIP switch #8)
	map(0x1e980000, 0x1e99ffff).m(m_fifo1, FUNC(cxd8442q_device::map_fifo_ram));

	// SONIC3 APbus interface and SONIC ethernet controller
	map(0x1e500000, 0x1e50003f).m(m_sonic3, FUNC(cxd8452aq_device::map));
	map(0x1e610000, 0x1e6101ff).m(m_sonic, FUNC(dp83932c_device::map)).umask64(0x000000000000ffff);
	map(0x1e620000, 0x1e627fff).lrw8(NAME([this](offs_t offset)
										  { return m_net_ram->read(offset); }),
									 NAME([this](offs_t offset, uint8_t data)
										  {
											  LOGMASKED(LOG_MEMORY, "Host write to net RAM @ offset 0x%x: 0x%x (%s)\n", offset, data, machine().describe_context());
											  m_net_ram->write(offset, data);
										  }));

	// DMAC3 DMA Controller
	map(0x1e200000, 0x1e200017).m(m_dmac, FUNC(dmac3_device::map<dmac3_device::CTRL0>));
	map(0x1e300000, 0x1e300017).m(m_dmac, FUNC(dmac3_device::map<dmac3_device::CTRL1>));

	// SPIFI SCSI controllers
	map(0x1e280000, 0x1e2803ff).m(m_scsi0, FUNC(spifi3_device::map));
	map(0x1e380000, 0x1e3803ff).m(m_scsi1, FUNC(spifi3_device::map));

	// HID (kb + ms)
	map(0x1f900000, 0x1f900047).m(m_hid, FUNC(news_hid_hle_device::map_apbus));

	// FDC controller register mapping
	// TODO: to be hardware accurate, these shouldn't be umasked.
	// instead, they should be duplicated across each 32-bit segment to emulate the open address lines
	// (i.e. status register A and B values of 56 c0 look like 56565656 c0c0c0c0)
	map(0x1ed60000, 0x1ed6001f).m(m_fdc, FUNC(pc8477b_device::map)).umask32(0x000000ff);

	// The NEWS monitor ROM FDC routines won't run if DRV2 is inactive.
	// TODO: The 5000X only has a single floppy drive. Why is this needed?
	map(0x1ed60000, 0x1ed60003).lr8(NAME([this](offs_t offset)
										 { return m_fdc->sra_r() & ~0x40; }))
		.umask32(0x000000ff);

	// TODO: Floppy aux registers
	map(0x1ed60200, 0x1ed60207).noprw();
	map(0x1ed60200, 0x1ed60207).lr8(NAME([](offs_t offset)
										 { return 0x1; }));

	// Map FDCAUX interrupt read (TODO: still need to implement button interrupt mask from 208-20b)
	map(0x1ed60208, 0x1ed6020f).lr32(NAME([this](offs_t offset)
										  { return (offset == 1) && ((m_intst[0] & irq0_number::FDC) > 0) ? 0x2 : 0x0; }));

	// Map FDC and sound FIFO register file
	map(0x1ed00000, 0x1ed3ffff).m(m_fifo0, FUNC(cxd8442q_device::map));
	map(0x1ed80000, 0x1ed9ffff).m(m_fifo0, FUNC(cxd8442q_device::map_fifo_ram));

	// TODO: DSC-39 xb framebuffer/video card (0x14900000)
	map(0x14900000, 0x149fffff).lr8(NAME([](offs_t offset)
										 { return 0xff; }));

	// TODO: sb sound subsystem (0x1ed00000)
	// TODO: lp line printer subsystem (0x1ed30000)

	// Assign debug mappings
	cpu_map_debug(map);
}

/*
 * cpu_map_main_memory
 *
 * Maps the main memory and supporting items into the provided address map.
 * TODO: Memory controller logic for non-64MB configurations
 */
void news_r4k_state::cpu_map_main_memory(address_map &map)
{
	// The mapping mechanism was determined experimentally and only supports the 64MB configuration for now.
	// This works for the monitor ROM, NEWS-OS kernel, and NetBSD kernel but will not work with other amounts of RAM.
	map(0x0, 0x7ffffff).rw(FUNC(news_r4k_state::ram_r), FUNC(news_r4k_state::ram_w));

	// Not 100% sure where the below lives on the system. This seems to be related to memory and perhaps APbus init.
	map(0x14400000, 0x14400003).lr32(NAME([](offs_t offset)
										  { return 0x0; }));
	map(0x14400004, 0x14400007).lr32(NAME([](offs_t offset)
										  { return 0x3ff17; }));
	map(0x14400024, 0x14400027).lr32(NAME([](offs_t offset)
										  { return 0x600a4; }));
	map(0x1440003c, 0x1440003f)
		.lw32(NAME([this](offs_t offset, uint32_t data)
				   {
					   if (data == 0x10001)
					   {
						   LOG("Enabling RAM map shift!\n");
						   m_ram_map_shift = true;
					   }
					   else
					   {
						   LOG("Disabling RAM map shift!\n");
						   m_ram_map_shift = false;
					   }
				   }));
}

/*
 * cpu_map_debug
 *
 * Method with temporary address map assignments. Everything in this function can be moved to the main memory
 * map function once it is understood. This separates the "real" mapping from the workarounds required to get
 * the monitor ROM to boot due to missing information or incomplete emulation.
 */
void news_r4k_state::cpu_map_debug(address_map &map)
{
	// APbus WBFLUSH + unknown
	map(0x1f520000, 0x1f520017).nopw();
	map(0x1f520000, 0x1f520017).lr8(NAME([](offs_t offset)
										 {
											 uint8_t value = 0x0;
											 if (offset == 7)
											 {
												 value = 0x1;
											 }
											 else if (offset == 11)
											 {
												 value = 0x1;
											 }
											 else if (offset == 15)
											 {
												 value = 0xc8;
											 }
											 else if (offset == 19)
											 {
												 value = 0x32;
											 }

											 return value;
										 }));

	// More onboard devices that needs to be mapped for the platform to boot
	map(0x1fe00000, 0x1fe03fff).ram().mirror(0x1fc000);
	map(0x1f3e0000, 0x1f3effff).lr8(NAME([](offs_t offset)
										 {
											 uint8_t value = 0x0;
											 if (offset % 4 == 2)
											 {
												 value = 0x6f;
											 }
											 else if (offset % 4 == 3)
											 {
												 value = 0xe0;
											 }

											 return value;
										 }));
}

/*
 * sonic3_map
 *
 * Memory accessor for the SONIC. The SONIC3 ASIC arbitrates memory access for the SONIC. The host platform can
 * access SONIC's memory directly, but SONIC may not access the bus directly.
 */
void news_r4k_state::sonic3_map(address_map &map)
{
	map(0x0, 0x7fff).lrw8(NAME([this](offs_t offset)
							   { return m_net_ram->read(offset); }),
						  NAME([this](offs_t offset, uint8_t data)
							   { m_net_ram->write(offset, data); }));
}

/*
 * ram_r
 *
 * Method that returns the byte at `offset` in main memory.
 * TODO: Memory controller logic for non-64MB configurations
 */
uint8_t news_r4k_state::ram_r(offs_t offset)
{
	uint8_t result = 0xff;
	if ((offset <= 0x1ffffff) || (m_ram_map_shift && offset <= 0x3ffffff))
	{
		result = m_ram->read(offset);
	}
	else if (!m_ram_map_shift && offset >= 0x7f00000) // upper 32MB before it is mapped into contiguous space
	{
		result = m_ram->read(offset - 0x5f00000);
	}
	else
	{
		LOG("Unmapped RAM read attempted at offset 0x%x\n", offset);
	}
	return result;
}

/*
 * ram_w
 *
 * Method that writes the byte `data` at `offset` in main memory.
 * TODO: Memory controller logic for non-64MB configurations
 */
void news_r4k_state::ram_w(offs_t offset, uint8_t data)
{
	if ((offset <= 0x1ffffff) || (m_ram_map_shift && offset <= 0x3ffffff))
	{
		m_ram->write(offset, data);
	}
	else if (!m_ram_map_shift && offset >= 0x7f00000) // upper 32MB before it is mapped into contiguous space
	{
		m_ram->write(offset - 0x5f00000, data);
	}
	else
	{
		LOG("Unmapped RAM write attempted at offset 0x%x (data: 0x%x)\n", offset, data);
	}
}

/*
 * machine_start
 *
 * Initialization method called when the machine first starts.
 */
void news_r4k_state::machine_start()
{
	// Init front panel LEDs
	m_led.resolve();

	// Save state support
	save_item(NAME(m_inten));
	save_item(NAME(m_intst));
	save_item(NAME(m_int_state));
	save_item(NAME(m_freerun_timer_val));

	// Allocate hardware timers
	m_freerun_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r4k_state::freerun_clock), this));
	m_timer0_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r4k_state::timer0_clock), this));

	// Cache for main bus access (APbus address translation)
	m_cpu->space(0).cache(m_main_cache);
}

/*
 * freerun_clock
 *
 * Method to handle a tick of the freerunning clock.
 */
TIMER_CALLBACK_MEMBER(news_r4k_state::freerun_clock)
{
	m_freerun_timer_val++;
}

/*
 * timer0_clock
 *
 * Method to handle a tick of the TIMER0 hardware clock.
 */
TIMER_CALLBACK_MEMBER(news_r4k_state::timer0_clock)
{
	// Whenever this timer elapses, we need to trigger the timer0 interrupt
	m_intst[2] |= irq2_number::TIMER0;
	int_check();
}

/*
 * machine_reset
 *
 * Method called whenever the machine is reset.
 */
void news_r4k_state::machine_reset()
{
	m_freerun_timer_val = 0;
	m_freerun_timer->adjust(attotime::zero, 0, attotime::from_hz(FREERUN_FREQUENCY));
	m_timer0_timer->adjust(attotime::zero, 0, attotime::from_hz(TIMER0_FREQUENCY));
}

/*
 * front_panel_r
 *
 * Method to read the state of the front panel DIP switches.
 */
uint64_t news_r4k_state::front_panel_r(offs_t offset)
{
	ioport_value dipsw = m_dip_switch->read();
	dipsw |= 0xff00; // Matches physical platform
	return ((uint64_t)dipsw << 32) | dipsw;
}

/*
 * led_state_w
 *
 * Sets and logs the status of the front panel LEDs.
 */
void news_r4k_state::led_state_w(offs_t offset, uint32_t data)
{
	if (m_led[offset] != data)
	{
		LOGMASKED(LOG_LED, "%s: %s\n", LED_MAP[offset], data ? "ON" : "OFF");
		m_led[offset] = data;
	}
}

/*
 * apbus_virt_to_phys
 *
 * The APbus has its own virtual->physical addressing scheme. Like the CPU's TLB, the host OS
 * is responsible for populating the APbus TLB. The `apbus_pte` struct defines what each entry looks like. Note that to avoid
 * bit-order dependencies and other platform-specific stuff, in this implementation, valid, coherent, pad2, and pfnum are
 * separate variables, but the actual register is packed like this:
 *
 * 0x xxxx xxxx xxxx xxxx
 *   |   pad   |  entry  |
 *
 * Entry is packed as follows:
 * 0b    x      x     xxxxxxxxxx xxxxxxxxxxxxxxxxxxxx
 *    |valid|coherent|    pad2  |       pfnum        |
 *
 * The APbus requires RAM to hold the TLB. On the NWS-5000X, this is 128KiB starting at physical address 0x14c20000 (and goes to 0x14c3ffff)
 *
 * For example, the monitor ROM populates the PTEs as follows in response to a `dl` command.
 * Addr       PTE1             PTE2
 * 0x14c20000 0000000080103ff5 0000000080103ff6
 *
 * It also loads the `address` register of the DMAC3 with 0xd60.
 *
 * This will cause the consuming ASIC (in this case, the DMAC3) to start mapping from virtual address 0xd60 to physical address 0x3ff5d60.
 * If the address register goes beyond 0xFFF, bit 12 will increment. This will increase the page number so the virtual address will be
 * 0x1000, and will cause the DMAC to use the next PTE (in this case, the next sequential page, 0x3ff6000).
 *
 * NetBSD splits the mapping RAM into two sections, one for each DMAC3 controller. If the OS does not keep track, the ASICs
 * could end up in a configuration that would cause them to overwrite each other's data. NEWS-OS makes even more extensive
 * use of APbus DMA, including the WSC-SONIC3 for network DMA.
 *
 * Another note: NetBSD mentions that the `pad2` section of the register is 10 bits. However, this might not be fully accurate.
 * On the NWS-5000X, the physical address bus is 36 bits because it has an R4400SC. The 32nd bit is sometimes set, depending
 * on the virtual address being used (maybe it goes to the memory controller). It doesn't impact the normal operation of the
 * computer, but does mean that the `pad2` section might only be 6 bits, not 10 bits.
 */
uint32_t news_r4k_state::apbus_virt_to_phys(uint32_t v_address)
{
	// Convert page number to PTE address and read raw PTE data from the APbus DMA mapping RAM
	uint32_t apbus_page_address = APBUS_DMA_MAP_ADDRESS + 8 * (v_address >> 12);
	if (apbus_page_address >= (APBUS_DMA_MAP_ADDRESS + APBUS_DMA_MAP_RAM_SIZE))
	{
		fatalerror("%s: APbus address decoder ran past the bounds of the DMA map RAM!", tag());
	}

	uint64_t raw_pte = m_main_cache.read_qword(apbus_page_address);

	// Marshal raw data to struct
	apbus_pte pte;
	pte.valid = raw_pte & ENTRY_VALID;
	pte.coherent = raw_pte & ENTRY_COHERENT;
	pte.pad2 = (raw_pte & ENTRY_PAD) >> ENTRY_PAD_SHIFT;
	pte.pfnum = raw_pte & ENTRY_PFNUM;

	// This might be able to trigger some kind of interrupt - for now, treat as a fatal error.
	// However, based on the NetBSD source, this would likely trigger INT4.
	if (!pte.valid)
	{
		fatalerror("%s: APbus DMA referenced invalid entry! Raw PTE (v_address = 0x%x, page_address = 0x%x): 0x%x\n", tag(), v_address, apbus_page_address, raw_pte);
	}

	// Since the entry is valid, use it to calculate the physical address
	return (pte.pfnum << 12) + (v_address & 0xFFF);
}

/*
 * apbus_cmd_r
 *
 * Emulates a call to the APbus controller (placeholder)
 */
uint8_t news_r4k_state::apbus_cmd_r(offs_t offset)
{
	// This function spoofs the APbus being fully initialized
	// More may have to be added to this method in the future
	uint8_t value = 0x0;
	if (offset == 7)
	{
		value = 0x1;
	}
	else if (offset == 11)
	{
		value = 0x1;
	}
	else if (offset == 15)
	{
		value = 0xc8;
	}
	else if (offset == 19)
	{
		value = 0x32;
	}

	LOGMASKED(LOG_APBUS, "APbus read triggered at offset 0x%x, returning 0x%x\n", offset, value);
	return value;
}

/*
 * apbus_cmd_w
 *
 * Emulates a call to the APbus controller (placeholder)
 */
void news_r4k_state::apbus_cmd_w(offs_t offset, uint32_t data)
{
	// Don't take the performance hit unless user cares about logs
#if (VERBOSE & LOG_APBUS) > 0
	auto cmd_iter = m_apbus_cmd_decoder.find(offset);
	if (cmd_iter != m_apbus_cmd_decoder.end())
	{
		LOGMASKED(LOG_APBUS, "APbus %s command invoked, data 0x%x\n", cmd_iter->second, data);
	}
	else
	{
		LOGMASKED(LOG_APBUS, "Unknown APbus command called, offset 0x%x, set to 0x%x\n", offset, data);
	}
#endif
}

/*
 * freerun_r
 *
 * Get the current value of the freerunning clock
 */
uint32_t news_r4k_state::freerun_r(offs_t offset)
{
	return m_freerun_timer_val;
}

/*
 * freerun_w
 *
 * Set the current value of the freerunning clock
 */
void news_r4k_state::freerun_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_INTERRUPT, "freerun_w: Set freerun timer to 0x%x\n", data);
	m_freerun_timer_val = data;
}

/*
 * timer0_w
 *
 * Clears the TIMER0 interrupt and starts the timer again.
 * See https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/newsmips/news5000.c#L114
 */
void news_r4k_state::timer0_w(offs_t offset, uint32_t data)
{
	m_intst[2] &= ~0x1;
	int_check();
}

/*
 * inten_w
 *
 * Update the enable status of the interrupt indicated by `offset`.
 */
void news_r4k_state::inten_w(offs_t offset, uint32_t data)
{
	LOGMASKED(LOG_INTERRUPT, "inten_w: INTEN%d = 0x%x\n", offset, data);
	m_inten[offset] = data;
	int_check();
}

/*
 * inten_r
 *
 * Get the enable status of the interrupt indicated by `offset`.
 */
uint32_t news_r4k_state::inten_r(offs_t offset)
{
	LOGMASKED(LOG_ALL_INTERRUPT, "inten_r: INTEN%d = 0x%x\n", offset, m_inten[offset]);
	return m_inten[offset];
}

/*
 * intst_r
 *
 * Get the status of the interrupt indicated by `offset`.
 */
uint32_t news_r4k_state::intst_r(offs_t offset)
{
	LOGMASKED(LOG_ALL_INTERRUPT, "intst_r: INTST%d = 0x%x\n", offset, m_intst[offset]);
	return m_intst[offset];
}

/*
 * generic_irq_w
 *
 * Set or clear the provided interrupt using `mask`.
 */
void news_r4k_state::generic_irq_w(uint32_t irq, uint32_t mask, int state)
{
	LOGMASKED(LOG_INTERRUPT, "generic_irq_w: INTST%d IRQ %d set to %d\n", irq, mask, state);
	if (state)
	{
		m_intst[irq] |= mask;
	}
	else
	{
		m_intst[irq] &= ~mask;
	}
	int_check();
}

/*
 * intclr_w
 *
 * Clear the provided interrupt.
 */
void news_r4k_state::intclr_w(offs_t offset, uint32_t data)
{
	// TODO: Haven't found much that uses INTCLR. Might be some nuances to this that will be discovered later.
	LOGMASKED(LOG_INTERRUPT, "intclr_w: INTCLR%d = 0x%x\n", offset, data);
	m_intst[offset] &= ~data;
	int_check();
}

/*
 * int_check
 *
 * Observes the platform state and updates the R4400's interrupt lines if needed.
 */
void news_r4k_state::int_check()
{
	// The R4000 series has 6 bits in the interrupt register
	// These map to the 6 INTST/EN/CLR groups on the NEWS platform
	// See https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/newsmips/news5000.c
	// and https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/apbus.c
	// This has been tested with a few different devices and seems OK so far, but there might be some things missing.
	for (int i = 0; i < 6; i++)
	{
		bool state = m_intst[i] & m_inten[i];

		if (m_int_state[i] != state)
		{
			LOGMASKED(LOG_ALL_INTERRUPT, "Setting CPU input line %d to %d\n", interrupt_map[i], state ? 1 : 0);
			m_int_state[i] = state;
			m_cpu->set_input_line(interrupt_map[i], state ? 1 : 0);
		}
	}
}

/*
 * escc_ch_read
 *
 * Accesses the read port of the specified ESCC channel.
 */
uint32_t news_r4k_state::escc_ch_read(escc_channel channel, offs_t offset)
{
	if (offset < 2)
	{
		offset |= (channel == CHA ? 0x2 : 0x0);
		return m_escc->ab_dc_r(offset);
	}
	else
	{
		// Placeholder for ESCC1 extport and ctl registers
		LOG("escc1 ch%d r non-passthrough: 0x%x\n", channel, offset);
		return 0x0;
	}
}

/*
 * escc_ch_read
 *
 * Writes `data` to the ESCC specified by channel.
 */
void news_r4k_state::escc_ch_write(escc_channel channel, offs_t offset, uint32_t data)
{
	if (offset < 2)
	{
		offset |= (channel == CHA ? 0x2 : 0x0);
		m_escc->ab_dc_w(offset, data);
	}
	else
	{
		// Placeholder for ESCC1 extport and ctl registers
		LOG("escc1 ch%d w non-passthrough: 0x%x = 0x%x\n", channel, offset, data);
		return;
	}
}

/*
 * escc1_int_status_r
 *
 * Reads the ESCC1 interrupt status register. This is used in conjunction with the ESCC and FIFO chips
 * to provide asynchronous duplex serial communication.
 */
uint32_t news_r4k_state::escc1_int_status_r(offs_t offset)
{
	LOGMASKED(LOG_ESCC, "Read ESCC1 int status 0x%x (%s)\n", escc1_int_status, machine().describe_context());
	return escc1_int_status;
}

/*
 * escc1_int_status_w
 *
 * Clears the ESCC1 interrupt status register. This is used in conjunction with the ESCC and FIFO chips
 * to provide asynchronous duplex serial communication.
 */
void news_r4k_state::escc1_int_status_w(offs_t offset, uint32_t data)
{
	// assume clear for now
	LOGMASKED(LOG_ESCC, "Clear ESCC1 int status (%s)\n", machine().describe_context());
	escc1_int_status = 0;
}

/*
 * NWS-5000X DIP switches
 *
 * 1. Console - switch between serial and bitmap console. Bitmap is not implemented yet.
 * 2. Bitmap Disable - Enable or disable the internal video card
 * 3. Abort/Resume Enable - Unknown, NEWS-OS refers to this but doesn't elaborate as to what it does
 * 4. Clear NVRAM - Upon boot, clear NVRAM contents and restore default values if set
 * 5. Auto Boot - Upon boot, automatically attempt to boot from the disk specified by the bootdev NVRAM variable
 * 6. Run Diagnostic Test - Attempt to run diagnostic test after ROM monitor has booted
 * 7. External APbus Slot Probe Disable - If set, do not attempt to probe the expansion APbus slots
 * 8. No Memory Mode - If set, do not use the main memory (limits system to 128KiB)
 *
 * On real hardware, the default for sw7 is off (probe external APslots).
 * Since the APbus expansion slots are not emulated yet, setting this avoids a harmless error message during the monitor ROM boot sequence
 */
static INPUT_PORTS_START(nws5000)
	PORT_START("FRONT_PANEL")
		PORT_DIPNAME(0x01, 0x00, "Console")
			PORT_DIPLOCATION("FRONT_PANEL:1")
			PORT_DIPSETTING(0x00, "Serial Terminal")
			PORT_DIPSETTING(0x01, "Bitmap")
		PORT_DIPNAME(0x02, 0x00, "Bitmap Disable")
			PORT_DIPLOCATION("FRONT_PANEL:2")
			PORT_DIPSETTING(0x00, "Enable Built-in Bitmap")
			PORT_DIPSETTING(0x02, "Disable Built-in Bitmap")
		PORT_DIPNAME(0x04, 0x00, "Abort/Resume Enable")
			PORT_DIPLOCATION("FRONT_PANEL:3")
			PORT_DIPSETTING(0x00, "Disable Abort/Resume")
			PORT_DIPSETTING(0x04, "Enable Abort/Resume")
		PORT_DIPNAME(0x08, 0x00, "Clear NVRAM")
			PORT_DIPLOCATION("FRONT_PANEL:4")
			PORT_DIPSETTING(0x00, "Normal Operation (No Clear)")
			PORT_DIPSETTING(0x08, "Clear NVRAM")
		PORT_DIPNAME(0x10, 0x00, "Auto Boot")
			PORT_DIPLOCATION("FRONT_PANEL:5")
			PORT_DIPSETTING(0x00, "Auto Boot Disable")
			PORT_DIPSETTING(0x10, "Auto Boot Enable")
		PORT_DIPNAME(0x20, 0x00, "Run Diagnostic Test")
			PORT_DIPLOCATION("FRONT_PANEL:6")
			PORT_DIPSETTING(0x00, "No Diagnostic Test")
			PORT_DIPSETTING(0x20, "Run Diagnostic Test")
		PORT_DIPNAME(0x40, 0x40, "External APbus Slot Probe Disable")
			PORT_DIPLOCATION("FRONT_PANEL:7")
			PORT_DIPSETTING(0x00, "Enable External Slot Probe")
			PORT_DIPSETTING(0x40, "Disable External Slot Probe")
		PORT_DIPNAME(0x80, 0x00, "No Memory Mode")
			PORT_DIPLOCATION("FRONT_PANEL:8")
			PORT_DIPSETTING(0x00, "Main Memory Enabled")
			PORT_DIPSETTING(0x80, "Main Memory Disabled");
INPUT_PORTS_END

ROM_START(nws5000x)
	ROM_REGION64_BE(0x40000, "mrom", 0)
	ROM_SYSTEM_BIOS(0, "nws5000x", "APbus System Monitor Release 3.201")
	ROMX_LOAD("mpu-33__ver3.201__1994_sony.rom", 0x00000, 0x40000, CRC(8a6ca2b7) SHA1(72d52e24a554c56938d69f7d279b2e65e284fd59), ROM_BIOS(0))

	// idrom and macrom dumps include unmapped space that would ideally be umasked, but this is functionally the same.
	// Additionally, there is machine-specific information contained within each of these, so there are no "golden" full-chip hashes.
	ROM_REGION64_BE(0x400, "idrom", 0)
	ROM_LOAD("idrom.rom", 0x000, 0x400, CRC(28ff30c9) SHA1(288fd7d9133ac5e40d90a9b6e24db057cd6b05ad) BAD_DUMP)

	ROM_REGION64_BE(0x400, "macrom", 0)
	ROM_LOAD("macrom.rom", 0x000, 0x400, CRC(22d384d2) SHA1(b78a2861310929e92f16deab989ac51f9da3131a) BAD_DUMP)
ROM_END

} // anonymous namespace

// Machine definitions
//   YEAR  NAME      P  CM MACHINE   INPUT    CLASS           INIT        COMPANY FULLNAME                      FLAGS
COMP(1994, nws5000x, 0, 0, nws5000x, nws5000, news_r4k_state, empty_init, "Sony", "NET WORK STATION NWS-5000X", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND)
