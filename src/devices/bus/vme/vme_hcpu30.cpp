// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/*
 * Besta HCPU30 board.
 *
 * Supported by SysV R3 "Bestix" port and also by Linux port,
 * see https://github.com/shattered/linux-m68k
 *
 * 68030 @ 33 MHz - primary CPU
 * 68882 @ 33 Mhz - FPU
 * 68020 @ 16 MHz - I/O CPU (using shared memory region)
 *
 * 4 or 16 MB of DRAM
 * 8 or 32 KB of NVRAM
 *
 * 33C93A - SCSI
 * DP8473 - Floppy
 * i82590 - Ethernet
 * 62421A - Real-time clock
 *
 * To do:
 *
 * - pass functional test
 * - boot to multiuser (SysV and Linux)
 * - add LAN and Centronics
 * - floppy: how is TC signal generated? (m_fdcdrq_hack); ready signal routing; dp8493 hacks
 * - dump PALs (should help with irq routing)
 *
 */

#include "emu.h"
#include "vme_hcpu30.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


#define DUSCC_CLOCK XTAL(14'745'600) /* XXX Unverified */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_HCPU30, vme_hcpu30_card_device, "hcpu30", "Besta HCPU30 CPU board")

void vme_hcpu30_card_device::hcpu30_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x003fffff).ram().share("dram"); // local bus DRAM, 4 MB
	map(0x00400000, 0x00ffffff).ram().share("hldram"); // optional HLDRAM
	map(0xff000000, 0xff007fff).rom().region("user1", 0).mirror(0x8000);
	map(0xfff1f400, 0xfff1f4ff).unmaprw(); // LAN DMA
	map(0xff020000, 0xff021fff).ram().share("mailbox").mirror(0x8000); // SRAM 32KB -- shared with iocpu
	map(0xff022000, 0xff027fff).ram().mirror(0x8000);
	map(0xffff8000, 0xffff9fff).ram().share("mailbox");
	map(0xfffff000, 0xfffff0ff).rw(FUNC(vme_hcpu30_card_device::dma_r), FUNC(vme_hcpu30_card_device::dma_w));
	map(0xfffff100, 0xfffff11f).rw(FUNC(vme_hcpu30_card_device::irq_state_r), FUNC(vme_hcpu30_card_device::irq_mask_w));
	map(0xfffff120, 0xfffff13f).rw(FUNC(vme_hcpu30_card_device::rtc_r), FUNC(vme_hcpu30_card_device::rtc_w));
	map(0xfffff200, 0xfffff2ff).rw("scsi:7:wd33c93", FUNC(wd33c93_device::indir_r), FUNC(wd33c93_device::indir_w)).umask32(0xffff0000);
	map(0xfffff300, 0xfffff3ff).rw("duscc", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write));
	map(0xfffff580, 0xfffff583).rw("scsi:7:wd33c93", FUNC(wd33c93_device::dma_r), FUNC(wd33c93_device::dma_w)).umask32(0xff000000);
	map(0xfffff600, 0xfffff6ff).unmaprw(); // LAN
	map(0xfffff700, 0xfffff7ff).m("floppy", FUNC(dp8473_device::map));
}

void vme_hcpu30_card_device::hcpu30_os_mem(address_map &map)
{
// bus error handler
	map(0x00000000, 0xffffffff).rw(FUNC(vme_hcpu30_card_device::trap_r), FUNC(vme_hcpu30_card_device::trap_w));
// shared memory with iocpu
	map(0x00000000, 0x003fffff).ram().share("dram");
	map(0x00400000, 0x00ffffff).ram().share("hldram");
	map(0xffff8000, 0xffff9fff).ram().share("mailbox");
}

void vme_hcpu30_card_device::cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).lr16(NAME([](offs_t offset) -> u16 { return 0x18 + offset; }));
}

void vme_hcpu30_card_device::oscpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).lr16(NAME([this](offs_t offset) -> u16 {
		u16 vec = (offset & 1) ? (m_mailbox[offset >> 1] >> 8) : (m_mailbox[offset >> 1] >> 24);
		logerror("68030 iack %d = %02x\n", offset, vec);
		if (1 || BIT(m_irq_state, 6)) // FIXME: irq routing is not fully understood
		{
			m_irq_state &= ~(1 << 6); // raise IRQ30*
			update_030_irq(offset, CLEAR_LINE);
			return vec;
		}
		else
		{
			return 0;
		}
	}));
}

static INPUT_PORTS_START(hcpu30)
	PORT_START("SA1")
	PORT_DIPNAME(0x03, 0x00, "Console port speed")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x01, "19200")
	PORT_DIPSETTING(0x02, "38400")
	PORT_DIPSETTING(0x03, "4800")
	PORT_DIPNAME(0x04, 0x04, "Boot into...")
	PORT_DIPSETTING(0x00, "UNIX")
	PORT_DIPSETTING(0x04, "Monitor")
	PORT_DIPNAME(0x08, 0x00, "Undefined 1")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x08, "On")
	PORT_DIPNAME(0x10, 0x00, "VME bus width")
	PORT_DIPSETTING(0x00, "32 bits")
	PORT_DIPSETTING(0x10, "16 bits")
	PORT_DIPNAME(0x20, 0x00, "VME bus free")
	PORT_DIPSETTING(0x00, "ROR")
	PORT_DIPSETTING(0x20, "REC")
	PORT_DIPNAME(0x40, 0x00, "Cache burst mode")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x40, "On")
	PORT_DIPNAME(0x80, 0x00, "Undefined 2")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x80, "On")
INPUT_PORTS_END

ROM_START(hcpu30)
	ROM_REGION32_BE(0x8000, "user1", ROMREGION_ERASEFF)
	// Rev 1.E of 09-NOV-1993
	ROM_LOAD("hcpu30.27c256.dat", 0x0000, 0x8000, CRC(d24da66e) SHA1(5431b0559b168a995e725b35e1465a0b8ee8aa72))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vme_hcpu30_card_device::device_rom_region() const
{
	return ROM_NAME(hcpu30);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vme_hcpu30_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hcpu30);
}

static void hcpu_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_hcpu30_card_device::device_add_mconfig(machine_config &config)
{
	// I/O CPU
	M68020(config, m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_hcpu30_card_device::hcpu30_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_hcpu30_card_device::cpu_space_map);
	m_maincpu->disable_interrupt_mixer();

	// FIXME: functional test expects dtr->dcd, rts->cts connections on both ports and tx->rx connection on port B
	DUSCC68562(config, m_dusccterm, DUSCC_CLOCK);
	m_dusccterm->configure_channels(0, 0, 0, 0);
	m_dusccterm->out_txda_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_txd));
	m_dusccterm->out_dtra_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_dtr));
	m_dusccterm->out_rtsa_callback().set(RS232P1_TAG, FUNC(rs232_port_device::write_rts));
//  m_dusccterm->out_dtra_callback().set(m_dusccterm, FUNC(duscc68562_device::dcda_w));
//  m_dusccterm->out_rtsa_callback().set(m_dusccterm, FUNC(duscc68562_device::ctsa_w));
	m_dusccterm->out_txdb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_txd));
	m_dusccterm->out_dtrb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_dtr));
	m_dusccterm->out_rtsb_callback().set(RS232P2_TAG, FUNC(rs232_port_device::write_rts));
//  m_dusccterm->out_txdb_callback().set(m_dusccterm, FUNC(duscc68562_device::rxb_w));
//  m_dusccterm->out_dtrb_callback().set(m_dusccterm, FUNC(duscc68562_device::dcdb_w));
//  m_dusccterm->out_rtsb_callback().set(m_dusccterm, FUNC(duscc68562_device::ctsb_w));
	m_dusccterm->out_int_callback().set(FUNC(vme_hcpu30_card_device::dusirq_callback));

	rs232_port_device &rs232p1(RS232_PORT(config, RS232P1_TAG, default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set(m_dusccterm, FUNC(duscc68562_device::rxa_w));
	rs232p1.cts_handler().set(m_dusccterm, FUNC(duscc68562_device::ctsa_w));

	rs232_port_device &rs232p2(RS232_PORT(config, RS232P2_TAG, default_rs232_devices, nullptr));
	rs232p2.rxd_handler().set(m_dusccterm, FUNC(duscc68562_device::rxb_w));
	rs232p2.cts_handler().set(m_dusccterm, FUNC(duscc68562_device::ctsb_w));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("hd", NSCSI_HARDDISK);
	NSCSI_CONNECTOR(config, "scsi:1").option_set("hd", NSCSI_HARDDISK);
	NSCSI_CONNECTOR(config, "scsi:2").option_set("hd", NSCSI_HARDDISK);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93", WD33C93A).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &wd33c93(downcast<wd33c9x_base_device &>(*device));

			wd33c93.set_clock(16670000/4); // default internal divisor is 2
			wd33c93.irq_cb().set(*this, FUNC(vme_hcpu30_card_device::scsiirq_callback)).invert();
			wd33c93.drq_cb().set(*this, FUNC(vme_hcpu30_card_device::scsidrq_callback));
		});

	// schematics connect INT to IPL1, not DRQ; could be outdated
	DP8473(config, m_fdc, 24_MHz_XTAL);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, M68K_IRQ_IPL1);
	m_fdc->intrq_wr_callback().set(*this, FUNC(vme_hcpu30_card_device::fdcirq_callback)).invert();

	// FIXME: drive select signals are swapped, handle this
	FLOPPY_CONNECTOR(config, "floppy:0", hcpu_floppies, "525qd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, "floppy:1", hcpu_floppies, "525qd", floppy_image_device::default_pc_floppy_formats);

	RTC62421(config, m_rtc, 32.768_kHz_XTAL);

	// FIXME: functional test expects A26-C24 A27-C28 A28-C26 A29-C30 A30-C32
	// i.e. ACKNOWL-CENTDS BUSY-CENTD3 PE-CENTD1 SLCT-CENTD5 ERROR-CENTD7
	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);

	// OS CPU
	M68030(config, m_oscpu, 2*16670000);
	m_oscpu->set_addrmap(AS_PROGRAM, &vme_hcpu30_card_device::hcpu30_os_mem);
	m_oscpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_hcpu30_card_device::oscpu_space_map);
	m_oscpu->set_disable();
}

uint32_t vme_hcpu30_card_device::rtc_r(offs_t offset)
{
	uint32_t data;

	if (m_rtc_hack)
	{
		data = (m_rtc_reg[offset << 1] << 28) | (m_rtc_reg[(offset << 1) + 1] << 12);
		data |= (m_rtc_reg[offset << 1] << 20) | (m_rtc_reg[(offset << 1) + 1] << 4);
	}
	else
	{
		data = (m_rtc->read(offset << 1) << 28) | (m_rtc->read((offset << 1) + 1) << 12);
		data |= (m_rtc->read(offset << 1) << 20) | (m_rtc->read((offset << 1) + 1) << 4);
	}

	if (offset == 0)
	{
		data &= 0xffffff;
		data |= ioport("SA1")->read() << 24;
	}
	if (offset == 7)
	{
		data &= 0xff00ffff;
		data |= (m_cent_status_in->read() ^ 0xff) << 16;
	}
	LOG("%s(%02x)==%08x%s\n", FUNCNAME, offset, data, m_rtc_hack ? " hacked" : "");
	return data;
}

void vme_hcpu30_card_device::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s(%02x,%08x)<-%08x\n", FUNCNAME, offset, mem_mask, data);
	if (mem_mask == 0xffff0000)
	{
		m_rtc->write(offset << 1, (data >> 4) & 15);
		m_rtc_reg[offset << 1] = (data >> 4) & 15;
		if (offset == 7)
		{
			m_cent_data_out->write(data >> 8);
		}
	}
	else
	{
		m_rtc->write((offset << 1) + 1, (data >> 20) & 15);
		m_rtc_reg[(offset << 1) + 1] = (data >> 20) & 15;
	}
	if (offset < 6 || (offset == 6 && mem_mask == 0xffff0000)) m_rtc_hack = true; else m_rtc_hack = false;
}

uint32_t vme_hcpu30_card_device::dma_r(offs_t offset)
{
	if (m_fdcdrq_hack++ == 1023)
	{
		m_fdc->tc_w(1);
		m_fdcdrq_hack = 0;
	}
	else
	{
		m_fdc->tc_w(0);
	}
	return m_fdc->dma_r() << 24;
}

void vme_hcpu30_card_device::dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_fdcdrq_hack++ == 1023)
	{
		m_fdc->tc_w(1);
		m_fdcdrq_hack = 0;
	}
	else
	{
		m_fdc->tc_w(0);
	}
	m_fdc->dma_w(data);
}

uint32_t vme_hcpu30_card_device::trap_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled()) set_bus_error((offset << 2), true, mem_mask);

	return 0xffffffff;
}

void vme_hcpu30_card_device::trap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s(%08x,%08X)\n", FUNCNAME, offset << 2, data);
	if (!machine().side_effects_disabled()) set_bus_error((offset << 2), false, mem_mask);
}

// AH?
WRITE_LINE_MEMBER(vme_hcpu30_card_device::dusirq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);
	m_irq_state &= ~(1 << (8+4));
	m_irq_state |= (state << (8+4));
}

// AL?
WRITE_LINE_MEMBER(vme_hcpu30_card_device::scsiirq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);
	m_irq_state &= ~(1 << 8);
	m_irq_state |= (state << 8);
}

// AL?
WRITE_LINE_MEMBER(vme_hcpu30_card_device::scsidrq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);
	m_irq_state &= ~(1 << 7);
	m_irq_state |= (state << 7);
}

// AL?
WRITE_LINE_MEMBER(vme_hcpu30_card_device::fdcirq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);
	m_irq_state &= ~(1 << (8+2));
	m_irq_state |= (state << (8+2));
}

// AL?
WRITE_LINE_MEMBER(vme_hcpu30_card_device::fdcdrq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);
#if 0
	if (state)
	{
		if (m_fdcdrq_hack++ == 1022)
		{
			m_fdc->tc_w(1);
			m_fdcdrq_hack = 0;
		}
		else
		{
			m_fdc->tc_w(0);
		}
	}
#endif
}

// FF0003F2: move.w  #$1d40, D7
// i.e. DUSIRQ, ABORT, FDCIRQ, SCSIIRQ, IRQ30
// FF00074C: eori.w  #$2540, D0
// i.e. bits 6, 8, 10, 13 are active low
// D70 translates to DL16-23
// D69 translates to DI0-7, D67 to DL24-31
//
// == f101.w ==
// 0    - SYSFAIL*
// 1    - ACFAIL*
// 2    - TERMRES*
// 3    - nc
// 4    - FPSENSE*
// 5    - RST*
// 6    - IRQ30*    D119 PLM output (CLRINT* and VECT20 inputs)
// 7    - SDMRQ*
// == f100.w ==
// 8    - SCSIIRQ
// 9    - CENTIRQ*
// 10   - FDCIRQ*
// 11   - ABORT*
// 12   - DUSIRQ*
// 13   - LANIRQ
// 14   - IPEND*    from 030
// 15   - LDMARQ*
uint32_t vme_hcpu30_card_device::irq_state_r(offs_t offset)
{
	return m_irq_state << 16;
}

void vme_hcpu30_card_device::update_030_irq(int irq, line_state state)
{
	if (irq != 0)
	{
		if (state == ASSERT_LINE)
		{
			LOG("triggering 68030 irq %d\n", irq);
			m_oscpu->set_input_line(M68K_IRQ_NONE + irq, ASSERT_LINE);
		}
		else
		{
			LOG("clearing 68030 irq %d\n", irq);
			m_oscpu->set_input_line(M68K_IRQ_NONE + irq, CLEAR_LINE);
		}
	}
}

// D60 translates from DI0-7 (DL24-31)
// D61 translates from DL16-21
// 0    - INTL0
// 1    - INTL1
// 2    - INTL2
// 3    - VME16
// 4    - CLRINT*   D119 PLM input
// 5    - LPBK* (loopback?)
// 6-7  - nc
// 8    - SCSIRES*
// 9    - ROR
// 10   - CENTDS (strobe)
// 11   - RESET*
// 12   - HALT*
// 13   - SRLOCL*
// 14   - PWRDOWN
// 15   - INTENA*   "enable input" pin of LS148 priority encoder
void vme_hcpu30_card_device::irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint16_t diff;

	data >>= 16;
	diff = data ^ m_irq_mask;

	LOG("%s(%04x,%04x)\n", FUNCNAME, data, diff);

	if (BIT(diff, 15))
	{
		update_030_irq((BIT(data, 15) ? m_irq_mask : data) & 7, BIT(data, 15) ? CLEAR_LINE : ASSERT_LINE);
	}
	// CLRINT* affects IRQ30*
	if (BIT(diff, 4))
	{
		m_irq_state &= ~(1 << 6);
		m_irq_state |= (BIT(data, 4) << 6);
	}
	if (BIT(diff, 8))
	{
		m_scsi->reset_w(BIT(data, 8));
		m_irq_state &= ~(1 << 5);
		m_irq_state |= (!BIT(data, 8) << 5);
	}
	if (BIT(diff, 10))
	{
		m_centronics->write_strobe(BIT(data, 10));
	}
	if (BIT(diff, 11))
	{
		if (BIT(m_irq_mask, 11))
		{
			m_oscpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			LOG("68030 halted by reset\n");
		}
		else
		{
			if (m_oscpu->suspended(SUSPEND_REASON_RESET))
			{
				m_oscpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				LOG("68030 started after reset\n");
			}
		}
	}
	if (BIT(diff, 12))
	{
		if (BIT(m_irq_mask, 12))
		{
			m_oscpu->suspend(SUSPEND_REASON_DISABLE, 1);
			LOG("68030 halted\n");
		}
		else
		{
			if (m_oscpu->suspended(SUSPEND_REASON_DISABLE))
			{
				m_oscpu->resume(SUSPEND_REASON_DISABLE);
				LOG("68030 started\n");
			}
		}
	}
	m_irq_mask = data;
}

void vme_hcpu30_card_device::set_bus_error(uint32_t address, bool rw, uint32_t mem_mask)
{
	if (m_bus_error)
	{
		return;
	}
	LOG("bus error at %08x & %08x (%s)\n", address, mem_mask, rw ? "read" : "write");
	if (!ACCESSING_BITS_16_31)
	{
		address++;
	}
	m_bus_error = true;
	m_oscpu->set_buserror_details(address, rw, m_oscpu->get_fc());
	m_oscpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_bus_error_timer->adjust(m_oscpu->cycles_to_attotime(16)); // let rmw cycles complete
}

vme_hcpu30_card_device::vme_hcpu30_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_dusccterm(*this, "duscc")
	, m_scsi(*this, "scsi:7:wd33c93")
	, m_fdc(*this, "floppy")
	, m_floppy0(*this, "floppy:0")
	, m_floppy1(*this, "floppy:1")
	, m_rtc(*this, "rtc")
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
	, m_cent_status_in(*this, "cent_status_in")
	, m_oscpu(*this, "oscpu")
	, m_mailbox(*this, "mailbox")
	, m_p_ram(*this, "dram")
	, m_sysrom(*this, "user1")
{
	LOG("%s %s\n", tag, FUNCNAME);
	m_slot = 1;
}

//

vme_hcpu30_card_device::vme_hcpu30_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_hcpu30_card_device(mconfig, VME_HCPU30, tag, owner, clock)
{
}

void vme_hcpu30_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	m_bus_error_timer = timer_alloc(0);
}

void vme_hcpu30_card_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	LOG("%s %s\n", tag(), FUNCNAME);
	m_irq_state = (1 << 10) | (1 << 6); // fdcirq | irq30*
	m_irq_mask = 0;
	m_rtc_hack = false;
	m_fdcdrq_hack = 0;
	m_fdc->ready_w(false);

	program.install_rom(0x00000000, 0x00000007, m_sysrom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xff000000, 0xff007fff,
			"rom_shadow_r",
			[this] (offs_t offset, u32 &data, u32 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x00000000, 0x00000007, m_p_ram);
				}
			},
			&m_rom_shadow_tap);
}

void vme_hcpu30_card_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	m_bus_error = false;
}
