// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
/****************************************************************************

    Apple "Capella" (343S1181) PowerPC-to-68k bus bridge
    This handles bus translation on the Cordyceps (Power Macintosh x200/x300) board.

    Memory map based on how the 6200 bootrom behaves:

    - $51xxxxxx: Cache RAM, 64 bits wide (on ROM/L2 card)

    - $52xxxxxx: Tag RAM (2x8k SRAMs on motherboard), 8 bit accesses only (ROM accesses $52000007, $5200000F, etc.)

    - $53xxxxxx: Capella registers
        Per the 6200 Developer Notes, the Capella only has D0-D4 connected, so only 5 bits
        can be read/written at a time.

        - $53000007: ???
        ROM clears it to 0

        - $5300000F: RAM / control switch
            Bit 2 = Tag RAM map enable?
            Bit 1 = L2 cache map enable?
            Bit 0 = ?

        - $53000017: ???
        ROM sets then clears bit 0; also clears bit 5
        68k routine at 0x125c checks bit 3; if 0, it sets it and exits,
        otherwise it falls through to an A-trap (0xA092 / _EgretDispatch),
        and the machine seems to reboot then.
        Note that for non-Cordyceps machines, that routine instead checks bit 2

        - $5300001f: IRQ acknowledge
        ROM does: read, eieio, write 0, eieio, read twice, isync.

        - $53000027: 68040 IPL line state
        This is right off the bus, so they have to be inverted for processing.
        Cordyceps-specific IRQ handler at 0x0315840 inverts these bits with XOR 7
        before sending them off to the 68k emulator.
        See https://github.com/elliotnunn/NanoKernel/blob/master/ExternalInts.s, ExtIntHandlerCordyceps

    Interrupt model:
        Same as AMIC / PDM: the PowerPC must be interrupted whenever the 68k lines *change*, not
        just when one is raised.

    The 6260 developer notes mention a "M7 bit" in the Capella that, when toggled, acts as a ROM switch.
    This isn't really needed to start the system; it's enough to map the ROM where it should go.

    Die shot (very sparsely populated):
    https://siliconpr0n.org/archive/doku.php?id=bercovici:vlsi:vy16669-apple-343s1181-a-capella&s[]=capella

****************************************************************************/

#include "emu.h"
#include "capella.h"

DEFINE_DEVICE_TYPE(CAPELLA, capella_device, "maccapella", "Apple Capella PowerPC-to-68040 bridge")

//-------------------------------------------------

void capella_device::map(address_map &map)
{
	map(0x53000008, 0x5300000f).rw(FUNC(capella_device::ctrl_r), FUNC(capella_device::ctrl_w));
	map(0x53000010, 0x53000017).rw(FUNC(capella_device::ctrl_b_r), FUNC(capella_device::ctrl_b_w));
	map(0x53000018, 0x5300001f).rw(FUNC(capella_device::irq_ack_r), FUNC(capella_device::irq_ack_w));
	map(0x53000020, 0x53000027).r(FUNC(capella_device::ipl_lines_r));
}

//-------------------------------------------------
//  capella_device - constructor
//-------------------------------------------------

capella_device::capella_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CAPELLA, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_ctrl_reg(0),
	m_ctrl_reg_b(0),
	m_iosb_ipl(0),
	m_nmi(0),
	m_ipl(0),
	m_irq_pending(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void capella_device::device_start()
{
	save_item(NAME(m_ctrl_reg));
	save_item(NAME(m_ctrl_reg_b));
	save_item(NAME(m_iosb_ipl));
	save_item(NAME(m_nmi));
	save_item(NAME(m_ipl));
	save_item(NAME(m_irq_pending));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void capella_device::device_reset()
{
	m_ctrl_reg = 0;
	m_ctrl_reg_b = 0; // CPU bootloops otherwise
	m_ipl = m_nmi ? 7 : m_iosb_ipl;
	m_irq_pending = false;
	m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
}

//-------------------------------------------------

u64 capella_device::ctrl_r(offs_t offset)
{
	return m_ctrl_reg & 0x1f;
}

void capella_device::ctrl_w(offs_t offset, u64 data)
{
	m_ctrl_reg = data & 0x1f;
}

u64 capella_device::ctrl_b_r(offs_t offset)
{
	return m_ctrl_reg_b & 0x1f;
}

void capella_device::ctrl_b_w(offs_t offset, u64 data)
{
	m_ctrl_reg_b = data & 0x1f;
}

u64 capella_device::ipl_lines_r(offs_t offset)
{
	return (~m_ipl) & 7;
}

u64 capella_device::irq_ack_r(offs_t offset)
{
	return m_irq_pending ? 1 : 0;
}

void capella_device::irq_ack_w(offs_t offset, u64 data)
{
	m_irq_pending = false;
	m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
}

//-------------------------------------------------

void capella_device::translate_ipl_state_change(int ipl)
{
	// should only see IRQs 1, 2, 4 from PrimeTime
	m_iosb_ipl = ((ipl >= 1) && (ipl <= 7)) ? ipl : 0;
	update_ipl();
}

void capella_device::nmi_w(int state)
{
	m_nmi = state ? 1 : 0;
	update_ipl();
}

void capella_device::update_ipl()
{
	const u8 ipl = m_nmi ? 7 : m_iosb_ipl;
	if (ipl == m_ipl)
	{
		return;
	}

	// *any* change on the IPL lines interrupts the CPU
	m_ipl = ipl;
	m_irq_pending = true;
	m_maincpu->set_input_line(PPC_IRQ, ASSERT_LINE);
}
