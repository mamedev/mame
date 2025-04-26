// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Patrick Mackinlay

/*
 * This device emulates the Multibus Sun-1 CPU board, which consists of the
 * following key components:
 *
 *  - 68000 CPU @ 8/10MHz
 *  - Sun-1 memory management unit
 *  - 256KiB RAM + up to 32KiB EPROM
 *  - Am9513 system timing controller
 *  - i8247/μPD7201 dual UART
 *
 * Sources:
 *  - Sun-1 System Reference Manual, Draft Version 1.0, July 27, 1982, Sun Microsystems, Inc.
 *  - Sun 68000 Board User's Manual, Revision B, February 1983, Sun Microsystems Inc.
 *
 * TODO:
 *  - second generation Sun-1.5 variant
 *  - variants from other companies
 *  - keyboard, mouse
 *  - jumpers
 */

/*
 * WIP:
 *  - bios 1: "t" enters CYB monitor
 *  - bios 6: top 3 bits of parallel register control tests
 */

#include "emu.h"

#include "sun1_cpu.h"

#include "bus/rs232/rs232.h"
#include "machine/clock.h"

#define LOG_DOG (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_DOG)
#include "logmacro.h"

enum irq_mask : u8
{
	WATCHDOG = 0x80,
};

DEFINE_DEVICE_TYPE(MULTIBUS_SUN1CPU, sun1cpu_device, "sun1cpu", "Sun-1 CPU")

sun1cpu_device::sun1cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MULTIBUS_SUN1CPU, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mmu(*this, "mmu")
	, m_duart(*this, "duart")
	, m_stc(*this, "stc")
	, m_boot(*this, "boot")
{
}

ROM_START(sun1)
	/*
	 * These different firmware versions expect slightly different hardware/timer
	 * configurations and probably come from CPU boards that were made or modified
	 * by other companies. The standard configuration according to the available
	 * documentation should be:
	 *
	 *  - stc @ 5MHz
	 *  - counter 1: watchdog (@2.98ms)
	 *  - counter 2: user irq 6
	 *  - cuonter 3: refresh irq 7 (@2ms)
	 *  - counter 4: uarta (16x baud)
	 *  - counter 5: uartb (16x baud)
	 *
	 * Most of these firmware versions do not configure timer 1 as a watchdog, and
	 * probably rely on a dedicated watchdog timer circuit like that described in
	 * the VSI schematics instead.
	 *
	 * The "aj" firmware is quite different: it does not use the STC for the DUART
	 * clocks, and uses counter 5 for refresh.
	 *
	 */
	ROM_SYSTEM_BIOS(0, "sun1", "Sun Monitor, Version 1.0")                                      // osc=16, div=4
	ROM_SYSTEM_BIOS(1, "cyb", "Sun Network Monitor, Version 0.10 (CYB)")                        // osc=16, div=4
	ROM_SYSTEM_BIOS(2, "test", "Interactive Tests")                                             // osc=16, div=2
	ROM_SYSTEM_BIOS(3, "fti10_11", "Sun Monitor, Version 1.1 (FTI10)")                          // osc=19.6, div=8
	ROM_SYSTEM_BIOS(4, "fti10_10", "Sun Monitor, Version 1.0 (FTI10)")                          // osc=19.6, div=8
	ROM_SYSTEM_BIOS(5, "smi10", "Sun Monitor, Version 1.1 (SMI10)")                             // osc=19.6, div=4, watchdog=1
	ROM_SYSTEM_BIOS(6, "aj", "Sun-1 Workstation Monitor; PROM 1&3: Rev AJ; PROM 2&4: Rev AJ")   // osc=20, div=4, watchdog=1

	/*
	 * The prom0 region at 0x02'0000 holds the primary firmware, and is also
	 * readable from address 0x00'0000 until boot mode is disabled.
	 */
	ROM_REGION16_BE(0x4000, "prom0", ROMREGION_ERASEFF)

	// stc @ 4MHz (16MHz / 4)
	// counter 3 mode 0b22 load=8000 == 2ms
	// counter 4 mode 0b22 load=13
	// counter 5 mode 0b22 load=13
	ROMX_LOAD("v10.8.u103", 0x0000, 0x2000, CRC(3528a0f8) SHA1(be437dd93d1a44eccffa6f5e05935119482beab0), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("v10.0.u101", 0x0001, 0x2000, CRC(1ad4c52a) SHA1(4bc1a19e8f202378d5d7baa8b95319275c040a6d), ROM_SKIP(1) | ROM_BIOS(0))

	// Sun Network Monitor, Version 0.10
	// stc @ 4MHz (16MHz / 4)
	// counter 3 mode 0b22 load=8000 == 2ms
	// counter 4 mode 0b22 load=13
	// counter 5 mode 0b22 load=13
	ROMX_LOAD("cybu103-sun10v.u103", 0x0000, 0x2000, CRC(32e53691) SHA1(5b0a3c4b5352d4c19067d9eb4db6cd1f76427892), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cybu101-sun10v.u101", 0x0001, 0x2000, CRC(900b725c) SHA1(90343de9e64ff8227c7e58c9d4df02783ecf5a76), ROM_SKIP(1) | ROM_BIOS(1))

	// Interactive Tests
	// stc @ 8MHz (16MHz / 2)
	// counter 3 mode 0b22 load=16000 == 2ms
	// counter 4 mode 0b22 load=26
	// counter 5 mode 0b22 load=26
	ROMX_LOAD("8mhzdiag.8.u103", 0x0000, 0x2000, CRC(808a549e) SHA1(d2aba014a5507c1538f2c1a73e1d2524f28034f4), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("8mhzdiag.0.u101", 0x0001, 0x2000, CRC(7a92d506) SHA1(5df3800f7083293fc01bb6a7e7538ad425bbebfb), ROM_SKIP(1) | ROM_BIOS(2))

	// Sun Monitor, Version 1.1
	// stc @ 2.4576MHz (19.6MHz / 8)
	// counter 3 mode 0b22 load=4915 == ~2ms
	// counter 4 mode 0b22 load=8
	// counter 5 mode 0b22 load=8
	ROMX_LOAD("fti10_11.u103", 0x0000, 0x1000, CRC(1a5befa5) SHA1(cd7fc4c88a76ae393ada6a3f427993fbacb8a9c6), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("fti10_11.u101", 0x0001, 0x1000, CRC(ac733618) SHA1(baff471248b355d761a5dab375fc88d861cb6853), ROM_SKIP(1) | ROM_BIOS(3))

	// Sun Monitor, Version 1.0
	// stc @ 2.4576MHz (19.6MHz / 8)
	// counter 3 mode 0b22 load=4915 == ~2ms
	// counter 4 mode 0b22 load=8
	// counter 5 mode 0b22 load=8
	ROMX_LOAD("fti10_10.u103", 0x0000, 0x1000, CRC(fb5e599b) SHA1(5ab1ac61034aac956fd6e39b964996e0f9d0da97), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("fti10_10.u101", 0x0001, 0x1000, CRC(e7371fd6) SHA1(5a377a4363b59c5508f941c356d8827594a72dc8), ROM_SKIP(1) | ROM_BIOS(4))

	// Sun Monitor, Version 1.1
	// stc @ 4.9152MHz (19.6MHz / 4)
	// counter 1 mode 0621 load=7568 == ~3ms
	// counter 3 mode 0622 load=4915 == ~2ms
	// counter 4 mode 0b22 load=16
	// counter 5 mode 0b22 load=16
	ROMX_LOAD("smi10_11.u103", 0x0000, 0x1000, CRC(89e1a6ed) SHA1(2dfcba08fb6e4582e05b803c4507419c25dbdaa8), ROM_SKIP(1) | ROM_BIOS(5))
	ROMX_LOAD("smi10_11.u101", 0x0001, 0x1000, CRC(432db053) SHA1(99a54a653f18b914ae15f230a35ac55c5d076a4c), ROM_SKIP(1) | ROM_BIOS(5))

	// PROM 1&3: Rev AJ
	// stc @ 5MHz (20MHz / 4)
	// counter 1 mode 0621 load=7450 == 2.98ms
	// counter 5 mode 0622 load=5000 == 2ms
	// uarts fixed @ 9600
	ROMX_LOAD("sun1c_1h.u103", 0x0000, 0x2000, CRC(4c58de6c) SHA1(f417a0a013d4ca3aaf565b0e7a88dcf3da1347c1), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD("sun1c_1l.u101", 0x0001, 0x2000, CRC(41c47355) SHA1(a38df286ab8ba9a462862ab1cc9fa28b79fb9949), ROM_SKIP(1) | ROM_BIOS(6))

	/*
	 * The prom1 region at 0x04'0000 holds secondary/optional diagnostic or
	 * boot firmware.
	 */
	ROM_REGION16_BE(0x4000, "prom1", ROMREGION_ERASEFF)

	// CYB Monitor - Version 2.1  6/23/84
	ROMX_LOAD("cybu104-autov.u104", 0x0000, 0x2000, CRC(46570c0e) SHA1(42ad96ffb7cb0e2d9c0aa0ace283ff646eaa4584), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cybu102-autov.u102", 0x0001, 0x2000, CRC(6198c5f6) SHA1(7d880395c4da03ac5130905c850ad638cd4b86e9), ROM_SKIP(1) | ROM_BIOS(1))

	// PROM 2&4: Rev AJ
	ROMX_LOAD("sun1c_2h.u104", 0x0000, 0x2000, CRC(b5cbcb2b) SHA1(8f906dbdc9b2af90bc8f942eebe7006be0ff4e86), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD("sun1c_2l.u102", 0x0001, 0x2000, CRC(dc82578b) SHA1(65bf18a8a40171ec08137a0c446da3b272b9ccbc), ROM_SKIP(1) | ROM_BIOS(6))

	// TODO: move the following firmware to a graphics card
	ROM_REGION(0x10000, "gfx", ROMREGION_ERASEFF)
	ROM_LOAD("gfxu605.g4.bin",  0x0000, 0x0200, CRC(274b7b3d) SHA1(40d8be2cfcbd03512a05925991bb5030d5d4b5e9))
	ROM_LOAD("gfxu308.g21.bin", 0x0200, 0x0200, CRC(35a6eed8) SHA1(25cb2dd8e5343cd7927c3045eb4cb96dc9935a37))
	ROM_LOAD("gfxu108.g20.bin", 0x0400, 0x0200, CRC(ecee335e) SHA1(5f4d32dc918af15872cd6e700a04720caeb6c657))
	ROM_LOAD("gfxu105.g0.bin",  0x0600, 0x0200, CRC(8e1a24b3) SHA1(dad2821c3a3137ad69e78b6fc29ab582e5d78646))
	ROM_LOAD("gfxu104.g1.bin",  0x0800, 0x0200, CRC(86f7a483) SHA1(8eb3778f5497741cd4345e81ff1a903c9a63c8bb))
	ROM_LOAD("gfxu307.g61.bin", 0x0a00, 0x0020, CRC(b190f25d) SHA1(80fbdc843f1eb68a2d3713499f04d99dab88ce83))
	ROM_LOAD("gfxu107.g60.bin", 0x0a20, 0x0020, CRC(425d3a98) SHA1(9ae4ce3761c2f995d00bed8d752c55224d274062))

	ROM_REGION(0x10000, "cpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpuu503.p2.bin", 0x0000, 0x0200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848))
	ROM_LOAD("cpuu602.p1.bin", 0x0200, 0x0020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805))
	ROM_LOAD("cpuu502.p0.bin", 0x0220, 0x0020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735))
ROM_END

const tiny_rom_entry *sun1cpu_device::device_rom_region() const
{
	return ROM_NAME(sun1);
}

static INPUT_PORTS_START(sun1)
INPUT_PORTS_END

ioport_constructor sun1cpu_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sun1);
}

void sun1cpu_device::device_start()
{
	save_item(NAME(m_irq));
	save_item(NAME(m_watchdog));
	save_item(NAME(m_parity));

	m_cpu->space(AS_PROGRAM).specific(m_cpu_mem);
	m_cpu->space(m68000_device::AS_CPU_SPACE).specific(m_cpu_spc);

	m_irq = 0;
}

void sun1cpu_device::device_reset()
{
	// HACK: configure oscillator, divisor and watchdog type to match selected
	// firmware until more accurate board type/configuration can be determined
	XTAL oscillator = 16_MHz_XTAL;
	unsigned divisor = 4;
	m_watchdog = false;

	switch (system_bios())
	{
	case 1:
	case 2:
		break;
	case 3:
		divisor = 2;
		break;
	case 4:
	case 5:
		oscillator = 19.6608_MHz_XTAL;
		divisor = 8;
		break;
	case 6:
		oscillator = 19.6608_MHz_XTAL;
		m_watchdog = true;
		break;
	case 7:
		oscillator = 20_MHz_XTAL;
		m_watchdog = true;
		break;
	}

	// configure cpu, stc and duart clocks
	m_cpu->set_clock(oscillator / 2);
	m_stc->set_clock(oscillator / divisor);
	m_duart->set_clock(oscillator / divisor);

	m_parity = false;

	if (!boot())
	{
		m_cpu->set_current_mmu(this);

		// disable interrupts
		for (unsigned i = 0; i < 7; i++)
			if (BIT(m_irq, i))
				m_cpu->set_input_line(M68K_IRQ_1 + i, CLEAR_LINE);

		m_boot.select(0);
	}
}

void sun1cpu_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 0);
	m_cpu->set_current_mmu(this);
	m_cpu->set_addrmap(AS_PROGRAM, &sun1cpu_device::cpu_mem);

	// route multibus interrupts 1..4 to cpu
	int_callback<1>().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_1>)).invert();
	int_callback<2>().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_2>)).invert();
	int_callback<3>().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_3>)).invert();
	int_callback<4>().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_4>)).invert();

	SUN1MMU(config, m_mmu);
	m_mmu->error().set(
		[this](int state)
		{
			// check multibus access pending
			if (state == sun1mmu_device::MMU_DEFER)
			{
				if (!m_watchdog || (m_irq & WATCHDOG))
				{
					LOGMASKED(LOG_DOG, "watchdog bus error\n");
					m_cpu->trigger_bus_error();
					m_mmu->reset();
				}
				else
					m_cpu->defer_access();
			}
			else
				m_cpu->trigger_bus_error();
		});

	// default configuration (1=watchdog, 2=user, 3=refresh, 4=uarta, 5=uartb)
	AM9513(config, m_stc, 0);
	m_stc->fout_cb().set(m_stc, FUNC(am9513_device::gate1_w));
	m_stc->out1_cb().set(FUNC(sun1cpu_device::watchdog_w));
	m_stc->out2_cb().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_6>));
	m_stc->out3_cb().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_7>));
	m_stc->out4_cb().set(m_duart, FUNC(upd7201_device::rxca_w));
	m_stc->out4_cb().append(m_duart, FUNC(upd7201_device::txca_w));
	m_stc->out5_cb().set(m_duart, FUNC(upd7201_device::rxcb_w));
	m_stc->out5_cb().append(m_duart, FUNC(upd7201_device::txcb_w));

	if (false)
	{
		// TODO: "aj" firmware has refresh on counter 5, and fixed-speed uarts
		m_stc->out3_cb().set_nop();
		m_stc->out4_cb().set_nop();
		m_stc->out5_cb().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_7>));

		// assume duart rxc/txc driven at 9600baud (x16 clock)
		CLOCK(config, "duart_clock", 9600 * 16).signal_handler().set(
			[this](int state)
			{
				m_duart->txca_w(state);
				m_duart->rxca_w(state);
				m_duart->txcb_w(state);
				m_duart->rxcb_w(state);
			});
	}

	// port A: txd, rxd, rts, cts, dsr, dtr
	// port B: txd, rxd
	UPD7201(config, m_duart, 0);
	m_duart->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_duart->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_duart->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_duart->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_duart->out_int_callback().set(FUNC(sun1cpu_device::irq_w<M68K_IRQ_5>));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_duart, FUNC(upd7201_device::rxa_w));
	rs232a.cts_handler().set(m_duart, FUNC(upd7201_device::ctsa_w));
	rs232a.dcd_handler().set(m_duart, FUNC(upd7201_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_duart, FUNC(upd7201_device::rxb_w));
}

void sun1cpu_device::device_config_complete()
{
	m_mmu.lookup()->set_space<0>(m_cpu, AS_PROGRAM);
	m_mmu.lookup()->set_space<1>(m_cpu, m68000_device::AS_CPU_SPACE);
	m_mmu.lookup()->set_space<2>(m_bus, AS_PROGRAM);
	m_mmu.lookup()->set_space<3>(m_bus, AS_IO);
}

void sun1cpu_device::cpu_mem(address_map &map)
{
	map(0x00'0000, 0x03'ffff).ram();

	map(0x00'0000, 0x1f'ffff).view(m_boot);
	m_boot[0](0x00'0000, 0x00'3fff).mirror(0x03'c000).rom().region("prom0", 0);

	map(0x20'0000, 0x20'3fff).mirror(0x03'c000).rom().region("prom0", 0).w(FUNC(sun1cpu_device::prom0_w));
	map(0x40'0000, 0x40'3fff).mirror(0x03'c000).rom().region("prom1", 0);
	map(0x60'0000, 0x60'0007).mirror(0x1ffff8).rw(m_duart, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0xff00);
	map(0x80'0000, 0x80'0003).mirror(0x1ffffc).rw(m_stc, FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0xa0'0000, 0xbf'ffff).rw(m_mmu, FUNC(sun1mmu_device::page_r), FUNC(sun1mmu_device::page_w));
	map(0xc0'0000, 0xdf'ffff).rw(m_mmu, FUNC(sun1mmu_device::segment_r), FUNC(sun1mmu_device::segment_w));
	map(0xe0'0000, 0xe0'0001).mirror(0x1f'fffe).r(FUNC(sun1cpu_device::parallel_r));
	map(0xe0'0000, 0xe0'0001).mirror(0x1f'fffe).w(m_mmu, FUNC(sun1mmu_device::context_w));
}

void sun1cpu_device::watchdog_w(int state)
{
	if (state)
		m_irq |= WATCHDOG;
	else
		m_irq &= ~WATCHDOG;

	if (!BIT(m_irq, 6))
	{
		if (state)
		{
			LOGMASKED(LOG_DOG, "watchdog reset\n");
			reset();
		}
	}
}

template <unsigned N> void sun1cpu_device::irq_w(int state)
{
	if (state)
		m_irq |= 1U << (N - M68K_IRQ_1);
	else
		m_irq &= ~(1U << (N - M68K_IRQ_1));

	if (!boot())
		m_cpu->set_input_line(N, state);
}

void sun1cpu_device::prom0_w(u16 data)
{
	m_parity = BIT(data, 0);

	if (boot())
	{
		LOG("boot mode disabled (%s)\n", machine().describe_context());

		m_cpu->set_current_mmu(m_mmu);

		// enable interrupts
		for (unsigned i = 0; i < 7; i++)
			if (BIT(m_irq, i))
				m_cpu->set_input_line(M68K_IRQ_1 + i, ASSERT_LINE);

		m_boot.disable();
	}
}
