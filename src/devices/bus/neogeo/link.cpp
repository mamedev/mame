// license:BSD-3-Clause
// copyright-holders:bugjosh
/***********************************************************************************************************

 Neo Geo cart with the multi-play link MCU (PROG-HERO board: Riding Hero)

 An HD6301V1 runs the link to the other cabinets or consoles over its serial port.  The 68000 sees it throughout
 $200000-$2FFFFF (the board's buffers are enabled by the port zone strobes alone, with no address decoding):
  - writing a byte to $200001 latches it into the MCU's port 3 and strobes IS3, which interrupts it (the
    program sets P3CSR to $48: latch enable, IS3 interrupt enable; its IRQ1 handler at $F4CB reads the byte);
  - reading $200001 gives the MCU's port 4, its reply, through a 74LS245;
  - reading $200000 gives its status: bits 0-3 are port 1 bits 4-7 through a 74LS368, which inverts them, and
    bits 4-7 aren't driven (the driver gives them the open bus).  The program flips P1.7 after each command (the
    handshake: the game waits for bit 3 to change, at $00ABB6), sets P1.5 when a block transfer's closing byte
    doesn't match or it has no node number, and P1.6 on a receive overrun or framing error (ORFE).  After a block
    transfer Riding Hero goes on only with status bits 1 and 2 both set (at $0096B2): through the inversion,
    neither error.

 Port 1 bit 1 is the slot select: while it reads high the program holds its state cleared ($F056).  It is the
 MVS's SLOTCS, high while another slot is selected, and grounded on the AES.  Port 1 bits 2 and 3 aren't connected.

 The link is an RS-485 bus at 62,500 baud, daisy-chained through the carts' two jacks, driven by an SN75176: its
 driver takes the serial output (P2.4) while port 1 bit 0 enables it (DE), and its receiver, enabled all the time
 (/RE grounded), gives the serial input (P2.3).  So a cart hears its own bytes, and the program relies on it: it
 sends each byte of a message after receiving the one before ($F309).  With nothing driving it, the bus is taken to
 read as idle (1); the board has 680 ohms across A and B and 2.2k from B to ground but no pull-up on A, so on
 hardware that level is up to the receiver.  No other cart is connected here, as on a single machine.

 The MCU's EXTAL takes the 4 MHz clock from the cartridge edge (4M, the 24 MHz master clock divided by 6), so it
 runs at 1 MHz.

 Sources: the NeoGeoDev wiki (HD6301V1 and Multiplayer pages, and the "MVS comm circuit" schematic), and the
 commented disassembly at github.com/neogeodev/COM-HD6301 (HPMAN, furrtek), whose RHCOM.BIN is the same 4 KB as
 MAME's dump.

 The internal ROM wasn't dumped; the program here was dumped from a prototype with an external ROM (8 KB, the
 same 4 KB twice), with its vectors at $FFF0-$FFFF (reset $F000).

 ***********************************************************************************************************/

#include "emu.h"
#include "link.h"


DEFINE_DEVICE_TYPE(NEOGEO_LINK_CART, neogeo_link_cart_device, "neocart_link", "Neo Geo Link MCU Cart")


ROM_START( neogeo_link )
	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASEFF ) // filled from the cartridge's MCU program at reset
ROM_END


neogeo_link_cart_device::neogeo_link_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_rom_device(mconfig, NEOGEO_LINK_CART, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_mcu_rom(*this, "mcu"),
	m_to_mcu(0),
	m_p1(0xff),
	m_p4(0xff),
	m_slot_selected(true),
	m_tx(1)
{
}

const tiny_rom_entry *neogeo_link_cart_device::device_rom_region() const
{
	return ROM_NAME( neogeo_link );
}

void neogeo_link_cart_device::device_add_mconfig(machine_config &config)
{
	HD6301V1(config, m_mcu, 24_MHz_XTAL / 6); // 4M from the cartridge edge
	m_mcu->in_p1_cb().set([this] () { return m_slot_selected ? 0x00 : 0x02; });
	m_mcu->out_p1_cb().set([this] (uint8_t data) { m_p1 = data; });
	m_mcu->in_p2_cb().set([this] () { return link_level() ? 0xff : 0xf7; }); // P2.0-P2.2 pulled high (mode 7)
	m_mcu->out_ser_tx_cb().set([this] (int state) { m_tx = state ? 1 : 0; });
	m_mcu->in_p3_cb().set([this] () { return m_to_mcu; });
	m_mcu->out_p4_cb().set([this] (uint8_t data) { m_p4 = data; });
}

void neogeo_link_cart_device::device_start()
{
	save_item(NAME(m_to_mcu));
	save_item(NAME(m_p1));
	save_item(NAME(m_p4));
	save_item(NAME(m_slot_selected));
	save_item(NAME(m_tx));
}

void neogeo_link_cart_device::device_reset()
{
	// the MCU's program is the slot's mcu region, from the set's ROMs or the software list's mcu data area (which
	// isn't loaded yet at device start); its last 4 KB.  This runs before the MCU's own reset fetches its vector.
	memory_region *const program = owner()->memregion("mcu");
	if (program && (program->bytes() >= 0x1000))
		memcpy(m_mcu_rom->base(), program->base() + program->bytes() - 0x1000, 0x1000);
	else
		logerror("No link MCU program in the cartridge's mcu region\n");
}

TIMER_CALLBACK_MEMBER(neogeo_link_cart_device::to_mcu)
{
	m_to_mcu = param;
	m_mcu->set_input_line(M6801_IS3_LINE, ASSERT_LINE); // port 3 latches on the edge
	m_mcu->set_input_line(M6801_IS3_LINE, CLEAR_LINE);
}

int neogeo_link_cart_device::link_level() const
{
	// the bus carries this cart's serial output while its driver is enabled, else it's idle
	return BIT(m_p1, 0) ? m_tx : 1;
}

uint16_t neogeo_link_cart_device::protection_r(address_space &space, offs_t offset)
{
	// D8-D11: port 1 bits 4-7 inverted by the 74LS368; D0-D7: port 4.  D12-D15 aren't driven.
	return 0xf000 | (BIT(~m_p1, 4, 4) << 8) | m_p4;
}

void neogeo_link_cart_device::protection_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(neogeo_link_cart_device::to_mcu), this), data & 0xff);
}
