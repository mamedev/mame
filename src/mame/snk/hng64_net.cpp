// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
/* HNG64 Communication / Network CPU */

// this is driven by a KL5C80A12CFP which is basically a super-charged Z80
// MMU handling has been moved to the core; the rest is not implemented yet.

// I believe this CPU is used for network only (the racing games) and not related to the I/O MCU

/*
0x6010: tests RAM at [3]8000

*/

#include "emu.h"
#include "includes/hng64.h"
#include "cpu/z80/kl5c80a12.h"

void hng64_state::hng_comm_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("user2", 0);
	map(0xf0000, 0xfffff).ram();
}

void hng64_state::hng_comm_io_map(address_map &map)
{
	map.global_mask(0xff);
	/* Reserved for the KL5C80 internal hardware */
//  map(0x00, 0x07).rw(FUNC(hng64_state::hng64_comm_mmu_r), FUNC(hng64_state::hng64_comm_mmu_w));
//  map(0x08, 0x1f).noprw();              /* Reserved */
//  map(0x20, 0x25).rw(hng64_state::));   /* Timer/Counter B */           /* hng64 writes here */
//  map(0x27, 0x27).noprw();              /* Reserved */
//  map(0x28, 0x2b).rw(hng64_state::)); /* Timer/Counter A */           /* hng64 writes here */
//  map(0x2c, 0x2f).rw(hng64_state::)); /* Parallel port A */
//  map(0x30, 0x33).rw(hng64_state::)); /* Parallel port B */
//  map(0x34, 0x37).rw(hng64_state::)); /* Interrupt controller */      /* hng64 writes here */
//  map(0x38, 0x39).rw(hng64_state::)); /* Serial port */               /* hng64 writes here */
//  map(0x3a, 0x3b).rw(hng64_state::)); /* System control register */   /* hng64 writes here */
//  map(0x3c, 0x3f).noprw();              /* Reserved */

	/* General IO */
	map(0x50, 0x57).rw(FUNC(hng64_state::hng64_com_share_r), FUNC(hng64_state::hng64_com_share_w));
//  map(0x72, 0x72).w(hng64_state::));            /* dunno yet */
}


void hng64_state::reset_net()
{
//  m_comm->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // reset the CPU and let 'er rip
//  m_comm->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);     // hold on there pardner...
}

void hng64_state::hng64_network(machine_config &config)
{
	KL5C80A12(config, m_comm, HNG64_MASTER_CLOCK / 4);        /* KL5C80A12CFP - binary compatible with Z80. */
	m_comm->set_addrmap(AS_PROGRAM, &hng64_state::hng_comm_map);
	m_comm->set_addrmap(AS_IO, &hng64_state::hng_comm_io_map);
}
