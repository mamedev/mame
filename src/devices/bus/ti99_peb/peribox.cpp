// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Peripheral expansion system

    The ti-99/4, ti-99/4a, ti computer 99/8, myarc geneve, and snug sgcpu
    99/4p systems all feature a bus connector that enables the connection of
    extension cards.  (Although the hexbus is the preferred bus to add
    additional peripherals to a ti-99/8, ti-99/8 is believed to be compatible
    with the older PEB system.)  In the case of the TI consoles, this bus
    connector is located on the right side of the console.

    While a few extension cards connect to the side bus connector of the
    ti-99/4(a) console directly, most extension cards were designed to be
    inserted in a PEB instead.  The PEB (Peripheral Expansion Box) is a big box
    with power supply, a few bus drivers, and several card slots, that
    connects to the ti-99/4(a) side port.  The reason for using a PEB is that
    daisy-chaining many modules caused the system to be unreliable due to the
    noise produced by the successive contacts.  (As a matter of fact, TI
    initially released most of its extension cards as side bus units, but when
    the design proved to be unreliable, the PEB was introduced.  The TI speech
    synthesizer was the only TI extension that remained on the side bus after
    the introduction of the PEB, probably because TI wanted the speech
    synthesizer to be a cheap extension, and the PEB was not cheap.)


    =================##  connection cable to console
                     ||
                     ||
    +--------------+----+----+----+----+----+----+----+----+------------+
    |              | S  | S  | S  | S  | S  | S  | S  | S  x passthru   |
    |              | L  | L  | L  | L  | L  | L  | L  | L  x for cable  |
    |  Power       | O  | O  | O  | O  | O  | O  | O  | O  x            |
    |  unit        | T  | T  | T  | T  | T  | T  | T  | T  |            |
    |  and         |    |    |    |    |    |    |    |    | Floppy     |
    |  ventilation | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | drive      |
    |              |    |    |    |    |    |    |    |    | compartment|
    |              |    |    |    |    |    |    |    |    |            |
    |              |    |    |    |    |    |    |    |    | 1 full hgt |
    |              |    |    |    |    |    |    |    |LED | or 2 "slim"|
    |              +----+----+----+----+----+----+----+-|--+ (half-hgt) |
    |              : |   seen from above |    |    |    |  |            |
    +--------------+-O----O----O----O----O----O----O----O---------------+

    All slots are connected in parallel with all signal lines. The cards
    must be equipped with bus drivers (244/245) and are usually activated
    by turning a CRU bit on (except for the memory expansions which are
    always active).

    This is emulation in exactly the same way. The data coming from the
    console is propagated to all slots, and each card decides whether to
    react or not. Similarly, for read operations, all cards are checked,
    and only the active cards actually put a value on the data bus
    (we do this using the bus8z_device; see ti99defs.h).

    Slot 1 is usually reserved for the "Flex cable interface" when connecting
    a TI-99/4(A)/8 console. Also, the Geneve is put into slot 1. We therefore
    do not offer peb:slot1. Slot 8 is usually used for floppy controllers
    as there is a passthrough for cables.


    Slots:

          REAR
     +8V  1||2   +8V
     GND  3||4   READY
     GND  5||6   RESET*
     GND  7||8   SCLK
 BOOTPG*  9||10  AUDIO
 RDBENA* 11||12  PCBEN
   HOLD* 13||14  IAQHDA
 SENILA* 15||16  SENILB*
  INTA*  17||18  INTB*
     D7  19||20  GND
     D5  21||22  D6
     D3  23||24  D4
     D1  25||26  D2
    GND  27||28  D0
    A14  29||30  A15/CRUOUT
    A12  31||32  A13
    A10  33||34  A11
     A8  35||36  A9
     A6  37||38  A7
     A4  39||40  A5
     A2  41||42  A3
     A0  43||44  A1
    AMB  45||46  AMA
    GND  47||48  AMC
    GND  49||50  CLKOUT*
CRUCLK*  51||52  DBIN
    GND  53||54  WE*
  CRUIN  55||56  MEMEN*
   -16V  57||58  -16V
   +16V  59||60  +16V
         FRONT

        < from box to console
        > from console into box

    READYA  <    System ready (goes to READY, 10K pull-up to +5V) A low level puts the cpu on hold.
    RESET*  >    System reset (active low)
    SCLK    nc   System clock (not connected in interface card)
    LCP*    nc   CPU indicator 1=TI99 0=2nd generation (not connected in interface card)
    BOOTPG* nc   ?
    AUDIO   <    Input audio (to AUDIOIN in console)
    RDBENA* <    Active low: enable flex cable data bus drivers (1K pull-up)
    PCBEN   H    PCB enable for burn-in (always High)
    HOLD*   H    Active low CPU hold request (always High)
    IAQHDA  nc   IAQ [or] HOLDA (logical or)
    SENILA* H(>) Interrupt level A sense enable (always High)
    SENILB* H(>) Interrupt level B sense enable (always High)
    INTA*   <    Interrupt level A (active low, goes to EXTINT*)
    INTB*   nc   Interrupt level B (not used)
    LOAD*   nc   Unmaskable interrupt (not carried by interface cable/card)
    D0-D7   <>   Data bus (D0 most significant)
    A0-A15  >    Address bus (A0 most sig; A15 also used as CRUOUT)
    AMA/B/C H    Extra address bits (always high for TI-99/4x, but used with SGCPU and Geneve)
    CLKOUT* >    Inverted PHI3 clock, from TIM9904 clock generator
    CRUCLK* >    Inverted CRU clock, from TMS9900 CRUCLK pin
    DBIN    >    Active high = read memory. Drives the data bus buffers.
    WE*     >    Write Enable pulse (derived from TMS9900 WE* pin)
    CRUIN   <    CRU input bit to TMS9900
    MEMEN*  >    Memory access enable (active low)

    The SENILx lines are somewhat obscure, since there have never been hardware
    or software that made practical use of them. The intended use can be guessed
    from some few traditional cards (like the TI RS232) which indeed have
    connections.

    With SENILA* asserted (low), a value shall be put on the data bus,
    representing the interrupt status bits. It can also be used to determine
    the source of the interrupt: The RS232 card (in its standard configuration)
    uses the data bus bits 0 and 1 for its two UARTs, while in the second
    configuration, it uses bits 4 and 5.

    SENILB* / INTB* was planned to be used with disk controllers. The PHP1240 disk
    controller puts the value of INTB* on D0 when SENILB* gets active (low) which
    reflects the INTRQ output pin of the WD1771. This signal is not used, however.
    Instead, the disk controller combines DRQ and IRQ and makes use of a READY/HOLD
    control of the CPU.

    Obviously, SENILA* and SENILB* should never be active at the same time, and
    neither should any memory access to a card be active at the same time, for in
    both cases, data bus lines may be set to different levels simultaneously. One
    possible application case is to turn off all cards in the box, assert SENILA*,
    and then do a read access in the memory area of any card in the P-Box (e.g.
    0x4000-0x5fff). Another possiblity is that the currently active card simply
    does not respond to a certain memory access, and in this case the status bits
    can be read.

    Also note that the SENILx lines access all cards in parallel, meaning that there
    must be an agreement which cards may use which bits on the data bus. The lines
    do not depend on the card being active at that time.

    Emulation architecture

    console ---- peribox --+ [-- slot1 (always occupied by console connector) ]
                           |
                           +--- slot2 --- card
                           |
                           +--- slot3 --- card
                           :
                           +--- slot8 --- card (usually some floppy controller)

    We have an instance of peribox_device which contains 7 slots
    (peribox_slot_device) which are subclasses of device_slot_interface.
    Each slot may host one of several cards (ti_expansion_card_device),
    which are subclassed from device_slot_card_interface.

    For bus8z_device consult ti99defs.h
    ---------------------

    June 2010: Reimplemented using device structure (MZ)
    January 2012: Reimplemented as class (MZ)

*****************************************************************************/

#include "emu.h"
#include "peribox.h"

// The cards
#include "ti_32kmem.h"
#include "ti_fdc.h"
#include "bwg.h"
#include "hfdc.h"
#include "pcode.h"
#include "myarcmem.h"
#include "samsmem.h"
#include "tn_ide.h"
#include "tn_usbsm.h"
#include "evpc.h"
#include "hsgpl.h"
#include "ti_rs232.h"
#include "spchsyn.h"
#include "memex.h"
#include "horizon.h"

/*
    Debugging flags. Set to 0 or 1.
*/
// Show interrupt line activity
#define TRACE_INT 0

// Show ready line activity
#define TRACE_READY 0

// Show emulation details
#define TRACE_EMU 1

#define PEBSLOT2 "slot2"
#define PEBSLOT3 "slot3"
#define PEBSLOT4 "slot4"
#define PEBSLOT5 "slot5"
#define PEBSLOT6 "slot6"
#define PEBSLOT7 "slot7"
#define PEBSLOT8 "slot8"

peribox_device::peribox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: bus8z_device(mconfig, PERIBOX, "Peripheral expansion box", tag, owner, clock, "peribox", __FILE__),
	m_console_inta(*this),
	m_console_intb(*this),
	m_datamux_ready(*this), m_inta_flag(0), m_intb_flag(0), m_ready_flag(0)
{
	for (int i=2; i <= 8; i++) m_slot[i] = nullptr;
	// The address prefix is actually created by the "Flex cable interface"
	// which sits in slot 1.
	m_address_prefix = 0x70000;
}

/*
    Constructor called from subclasses.
*/
peribox_device::peribox_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
: bus8z_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_console_inta(*this),
	m_console_intb(*this),
	m_datamux_ready(*this), m_inta_flag(0), m_intb_flag(0), m_ready_flag(0), m_address_prefix(0), m_msast(false), m_memen(false)
{
	for (int i=2; i <= 8; i++) m_slot[i] = nullptr;
}

READ8Z_MEMBER(peribox_device::readz)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->readz(space, offset | m_address_prefix, value, mem_mask);
	}
}

WRITE8_MEMBER(peribox_device::write)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->write(space, offset | m_address_prefix, data, mem_mask);
	}
}

SETADDRESS_DBIN_MEMBER(peribox_device::setaddress_dbin)
{
	// Ignore the address when the TI-99/8 transmits the high-order 8 bits
	if (!m_memen) return;

	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->setaddress_dbin(space, offset | m_address_prefix, state);
	}
}

READ8Z_MEMBER(peribox_device::crureadz)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->crureadz(space, offset, value);
	}
}

WRITE8_MEMBER(peribox_device::cruwrite)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->cruwrite(space, offset, data);
	}
}

/*
    And here are finally the two mythical lines SENILA* and SENILB*; mythical
    since there is no report of any software that ever used them.
*/
WRITE_LINE_MEMBER(peribox_device::senila)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->senila(state);
	}
}

WRITE_LINE_MEMBER(peribox_device::senilb)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->senilb(state);
	}
}

/*
    MEMEN input. Used to start the external memory access cycle.
*/
WRITE_LINE_MEMBER(peribox_device::memen_in)
{
	m_memen = (state==ASSERT_LINE);
}

/*
    MSAST input. Defined by TI-99/8; we ignore this part in the PEB.
*/
WRITE_LINE_MEMBER(peribox_device::msast_in)
{
	m_msast = (state==ASSERT_LINE);
}

/*
    CLKOUT line
*/

WRITE_LINE_MEMBER(peribox_device::clock_in)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->clock_in(state);
	}
}

/*
    The Genmod modification is only of interest for the Geneve. It requires
    to modify the decoding of each single card.
*/
INPUT_CHANGED_MEMBER( peribox_device::genmod_changed )
{
	set_genmod(newval==1);
}

void peribox_device::set_genmod(bool set)
{
	for (int i=2; i <= 8; i++)
	{
		if (m_slot[i]!=nullptr) m_slot[i]->set_genmod(set);
	}
}

/*
    The INTA*, INTB*, and READY* lines are connected to each PEB card and are
    pulled up when inactive. If any card asserts the line (pulling down), the
    line state goes down. So we must keep a record which cards pull down the
    line.

    (We're doing a kind of wired-AND here.)
*/
void peribox_device::inta_join(int slot, int state)
{
	if (TRACE_INT) logerror("%s: propagating INTA from slot %d to console: %d\n", tag(), slot, state);
	if (state==ASSERT_LINE)
		m_inta_flag |= (1 << slot);
	else
		m_inta_flag &= ~(1 << slot);

	m_console_inta((m_inta_flag != 0)? ASSERT_LINE : CLEAR_LINE);
}

void peribox_device::intb_join(int slot, int state)
{
	if (TRACE_INT) logerror("%s: propagating INTB from slot %d to console: %d\n", tag(), slot, state);
	if (state==ASSERT_LINE)
		m_intb_flag |= (1 << slot);
	else
		m_intb_flag &= ~(1 << slot);

	m_console_intb((m_intb_flag != 0)? ASSERT_LINE : CLEAR_LINE);
}

/*
    When any device pulls down READY, READY goes down.
*/
void peribox_device::ready_join(int slot, int state)
{
	if (TRACE_READY) logerror("%s: Incoming READY=%d from slot %d\n", tag(), state, slot);
	// We store the inverse state
	if (state==CLEAR_LINE)
		m_ready_flag |= (1 << slot);
	else
		m_ready_flag &= ~(1 << slot);

	m_datamux_ready((m_ready_flag != 0)? CLEAR_LINE : ASSERT_LINE);
}

void peribox_device::set_slot_loaded(int slot, peribox_slot_device* slotdev)
{
	m_slot[slot] = slotdev;
}

void peribox_device::device_start(void)
{
	if (TRACE_EMU) logerror("%s: started\n", tag());

	// Resolve the callback lines to the console
	m_console_inta.resolve();
	m_console_intb.resolve();
	m_datamux_ready.resolve();

	if (TRACE_EMU)
	{
		logerror("%s: AMA/B/C address prefix set to %05x\n", tag(), m_address_prefix);
		for (int i=2; i < 9; i++)
		{
			logerror("%s: Slot %d = %s\n", tag(), i, (m_slot[i] != nullptr)? m_slot[i]->m_card->tag() : "EMPTY");
		}
	}
}

void peribox_device::device_config_complete()
{
	m_inta_flag = 0;
	m_intb_flag = 0;
	m_ready_flag = 0;
}

SLOT_INTERFACE_START( peribox_slot )
	SLOT_INTERFACE("32kmem", TI_32KMEM)
	SLOT_INTERFACE("myarcmem", TI99_MYARCMEM)
	SLOT_INTERFACE("samsmem", TI99_SAMSMEM)
	SLOT_INTERFACE("pcode", TI99_P_CODE)
	SLOT_INTERFACE("hsgpl", TI99_HSGPL)
	SLOT_INTERFACE("tirs232", TI99_RS232)
	SLOT_INTERFACE("speech", TI99_SPEECH)
	SLOT_INTERFACE("horizon", TI99_HORIZON)
	SLOT_INTERFACE("ide", TI99_IDE)
	SLOT_INTERFACE("usbsm", TI99_USBSM)
	SLOT_INTERFACE("bwg", TI99_BWG)
	SLOT_INTERFACE("hfdc", TI99_HFDC)
	SLOT_INTERFACE("tifdc", TI99_FDC)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( peribox_device )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT2, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT3, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT4, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT5, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT6, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT7, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT8, peribox_slot )
MACHINE_CONFIG_END

machine_config_constructor peribox_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( peribox_device );
}

/****************************************************************************
    A variant of the box used for the Geneve.
*****************************************************************************/

peribox_gen_device::peribox_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: peribox_device(mconfig, PERIBOX_GEN, "Peripheral expansion box Geneve", tag, owner, clock, "peribox_gen", __FILE__)
{
	// The Geneve sits in slot 1; there is no prefix here - it can control
	// a maximum address space of 512 KiB in the box. With the Genmod
	// modification, the full 2 MiB space is available.
	m_address_prefix = 0x00000;
}

// The BwG controller will not run with the Geneve due to its wait state
// logic (see bwg.c)

SLOT_INTERFACE_START( peribox_slotg )
	SLOT_INTERFACE("memex", TI99_MEMEX)
	SLOT_INTERFACE("tirs232", TI99_RS232)
	SLOT_INTERFACE("speech", TI99_SPEECH)
	SLOT_INTERFACE("horizon", TI99_HORIZON)
	SLOT_INTERFACE("ide", TI99_IDE)
	SLOT_INTERFACE("usbsm", TI99_USBSM)
	SLOT_INTERFACE("tifdc", TI99_FDC)
	SLOT_INTERFACE("hfdc", TI99_HFDC)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( peribox_gen_device )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT2, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT3, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT4, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT5, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT6, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT7, peribox_slotg )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT8, peribox_slotg )
MACHINE_CONFIG_END

machine_config_constructor peribox_gen_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( peribox_gen_device );
}

/****************************************************************************
    A variant of the box used for the TI-99/8
*****************************************************************************/

peribox_998_device::peribox_998_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: peribox_device(mconfig, PERIBOX_998, "Peripheral expansion box 99/8", tag, owner, clock, "peribox_998", __FILE__)
{
	m_address_prefix = 0x70000;
}

// The BwG controller will not run with the TI-99/8 for the same reason why
// it won't work with the Geneve.
// We don't have many options here. The P-Box is not the prefered device for
// the 99/8; it was intended to use the Hexbus interface. None of the memory
// expansions are really supposed to work here.
SLOT_INTERFACE_START( peribox_slot998 )
	SLOT_INTERFACE("ide", TI99_IDE)
	SLOT_INTERFACE("usbsm", TI99_USBSM)
	SLOT_INTERFACE("tirs232", TI99_RS232)
	SLOT_INTERFACE("tifdc", TI99_FDC)
	SLOT_INTERFACE("hfdc", TI99_HFDC)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( peribox_998_device )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT2, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT3, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT4, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT5, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT6, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT7, peribox_slot998 )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT8, peribox_slot998 )
MACHINE_CONFIG_END

machine_config_constructor peribox_998_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( peribox_998_device );
}

/****************************************************************************
    A variant of the box used for the SGCPU (aka TI-99/4P).
*****************************************************************************/

peribox_sg_device::peribox_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: peribox_device(mconfig, PERIBOX_SG, "Peripheral expansion box SGCPU", tag, owner, clock, "peribox_sg", __FILE__)
{
	m_address_prefix = 0x70000;
}

SLOT_INTERFACE_START( peribox_slotp )
	SLOT_INTERFACE("pcode", TI99_P_CODE)
	SLOT_INTERFACE("tirs232", TI99_RS232)
	SLOT_INTERFACE("speech", TI99_SPEECH)
	SLOT_INTERFACE("myarcmem", TI99_MYARCMEM)
	SLOT_INTERFACE("samsmem", TI99_SAMSMEM)
	SLOT_INTERFACE("horizon", TI99_HORIZON)
	SLOT_INTERFACE("ide", TI99_IDE)
	SLOT_INTERFACE("usbsm", TI99_USBSM)
	SLOT_INTERFACE("bwg", TI99_BWG)
	SLOT_INTERFACE("hfdc", TI99_HFDC)
	SLOT_INTERFACE("tifdc", TI99_FDC)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( peribox_ev_slot )
	SLOT_INTERFACE("evpc", TI99_EVPC)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( peribox_hs_slot )
	SLOT_INTERFACE("hsgpl", TI99_HSGPL)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( peribox_sg_device )
	MCFG_PERIBOX_SLOT_ADD_DEF( PEBSLOT2, peribox_ev_slot, "evpc" )
	MCFG_PERIBOX_SLOT_ADD_DEF( PEBSLOT3, peribox_hs_slot, "hsgpl" )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT4, peribox_slotp )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT5, peribox_slotp )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT6, peribox_slotp )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT7, peribox_slotp )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT8, peribox_slotp )
MACHINE_CONFIG_END

machine_config_constructor peribox_sg_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( peribox_sg_device );
}

/****************************************************************************
    Another variant of the box; used for the TI with EVPC. The EVPC is
    obviously required.
*****************************************************************************/

peribox_ev_device::peribox_ev_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: peribox_device(mconfig, PERIBOX_EV, "Peripheral expansion box EVPC", tag, owner, clock, "peribox_ev", __FILE__)
{
	m_address_prefix = 0x70000;
}

MACHINE_CONFIG_FRAGMENT( peribox_ev_device )
	MCFG_PERIBOX_SLOT_ADD_DEF( PEBSLOT2, peribox_ev_slot, "evpc" )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT3, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT4, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT5, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT6, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT7, peribox_slot )
	MCFG_PERIBOX_SLOT_ADD( PEBSLOT8, peribox_slot )
MACHINE_CONFIG_END

machine_config_constructor peribox_ev_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( peribox_ev_device );
}

/***************************************************************************
    Implementation of a slot within the box.
****************************************************************************/

int peribox_slot_device::get_index_from_tagname()
{
	const char *mytag = tag();
	int maxlen = strlen(mytag);
	int i;
	for (i=maxlen-1; i >=0; i--)
		if (mytag[i] < 48 || mytag[i] > 57) break;

	return atoi(mytag+i+1);
}

peribox_slot_device::peribox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: bus8z_device(mconfig, PERIBOX_SLOT, "TI P-Box slot", tag, owner, clock, "peribox_slot", __FILE__), device_slot_interface(mconfig, *this), m_card(nullptr), m_slotnumber(0)
{
}

READ8Z_MEMBER(peribox_slot_device::readz)
{
	m_card->readz(space, offset, value, mem_mask);
}

WRITE8_MEMBER(peribox_slot_device::write)
{
	m_card->write(space, offset, data, mem_mask);
}

SETADDRESS_DBIN_MEMBER(peribox_slot_device::setaddress_dbin)
{
	m_card->setaddress_dbin(space, offset, state);
}

READ8Z_MEMBER(peribox_slot_device::crureadz)
{
	m_card->crureadz(space, offset, value);
}

WRITE8_MEMBER(peribox_slot_device::cruwrite)
{
	m_card->cruwrite(space, offset, data);
}

WRITE_LINE_MEMBER( peribox_slot_device::senila )
{
	m_card->set_senila(state);
}

WRITE_LINE_MEMBER( peribox_slot_device::senilb )
{
	m_card->set_senilb(state);
}

WRITE_LINE_MEMBER( peribox_slot_device::clock_in )
{
	m_card->clock_in(state);
}

/*
    Genmod support
*/
void peribox_slot_device::set_genmod(bool set)
{
	m_card->m_genmod = set;
}

void peribox_slot_device::device_start(void)
{
}

void peribox_slot_device::device_config_complete()
{
	m_slotnumber = get_index_from_tagname();
	device_t *carddev = subdevices().first();
	peribox_device *peb = static_cast<peribox_device*>(owner());
	if (carddev != nullptr)
	{
		peb->set_slot_loaded(m_slotnumber, this);
		m_card = static_cast<ti_expansion_card_device*>(carddev);
	}
	else
	{
		peb->set_slot_loaded(m_slotnumber, nullptr);
		m_card = nullptr;
	}
}

/*
    Delivers the drives that are installed in the box. Called from a card.
*/
device_t* peribox_slot_device::get_drive(const char* name)
{
	return owner()->subdevice(name);
}

/*
    These methods are called from the expansion cards. They add the
    slot number to identify the slot to the box.
*/
WRITE_LINE_MEMBER( peribox_slot_device::set_inta )
{
	peribox_device *peb = static_cast<peribox_device*>(owner());
	peb->inta_join(m_slotnumber, state);
}

WRITE_LINE_MEMBER( peribox_slot_device::set_intb )
{
	peribox_device *peb = static_cast<peribox_device*>(owner());
	peb->intb_join(m_slotnumber, state);
}

WRITE_LINE_MEMBER( peribox_slot_device::set_ready )
{
	peribox_device *peb = static_cast<peribox_device*>(owner());
	peb->ready_join(m_slotnumber, state);
}

/***************************************************************************/
const device_type PERIBOX = &device_creator<peribox_device>;
const device_type PERIBOX_SLOT = &device_creator<peribox_slot_device>;

const device_type PERIBOX_EV = &device_creator<peribox_ev_device>;
const device_type PERIBOX_SG = &device_creator<peribox_sg_device>;
const device_type PERIBOX_GEN = &device_creator<peribox_gen_device>;
const device_type PERIBOX_998 = &device_creator<peribox_998_device>;
