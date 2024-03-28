// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SCSI adapter card
    designed and realized by Western Horizon Technologies

    Latest two revisions:
    Rev F: using logic circuits and a PAL for decoding
    Rev G: using a PLD custom chip

    This is an emulation of the WHTech SCSI adapter. It has been modified over
    the years in various ways, in particular to get it working with the Geneve
    9640, and to enable word transfers. This modification was only effective
    on the Geneve because the TMS9995 uses an even/odd word transfer, while the
    TI-99/4A console uses an odd/even word transfer (see datamux.cpp), hence
    swapping the bytes from the controller. An additional SWPB operation would
    have been required which would have slowed down the transfer, so there would
    not have be a measurable performance gain. Accordingly, the DSR (firmware)
    of the card does not activate word transfer.

    Further modifications were done to enable block DMA transfer. This could
    only be used with a new DSR (1.6) and a daughterboard to replace the
    PLD circuit.

    Note that the DSR is not used when the card is used with the Geneve, since
    the Geneve operating system brings its own SCSI driver. Block mode can
    be explicitly activated with PDMA ON, but for this version of the emulation,
    this will lead to a lockup, so PDMA OFF must be ensured.

    A clone card was designed by the System 99 user group (SNUG), called ASCSI,
    which contained all modifications.


    Components
    ----------
    ispLSI2064   PLD (Custom decoder chip) (Rev G)
    62256        RAM 32Kx8
    27512        EPROM 64Kx8
    NEC 53C80    SCSI Controller
    Dip switches


    Memory map
    ----------

    CB5=0 (CRU bit 5)
    4000-4FDF: EPROM (1 of 8 banks @ 8K)
    4FE0-4FFF: Controller
    5000-5FFF: SRAM (if CB1=1) or EPROM (if CB1=0)

    CB5=1:
    4000-4FFF: SRAM (if CB1=1) or EPROM (if CB1=0)
    5000-5FDF: EPROM (1 of 8 banks)
    5FE0-5FFF: Controller


    DIP switches [1]
    SW1: CRU base 0600 - 1f00
    SW2: 0: Driver LOAD/Parity enable
         1: SCSI ID Bit 0 (6 or 7)
         2: Normally on
         3: Normally on
         4: Geneve mode

    CRU mapping
    -----------
    Read                               Write
    0: SCSI IRQ                        0: Card select
    1: SCSI DRQ                        1: SRAM enable (hides EPROM)
    2: SCSI READY                      2: SCSI DMA mode enable
    3-7: DIP SW2                       3: SCSI EOP (End of process)
                                       4: Word transfer mode select
                                       5: Bank swap
                                       6: -
                                       7: -
                                       8-10: EPROM bank (10=MSB)
                                       11: -
                                       12-14: SRAM bank (14=MSB)
                                       15: -


    I/O ports (4Fxx for CB5=0, 5Fxx for CB5=1)
    ---------
    Read                                 Write
    4FE0  Current SCSI data              4FF0   Output data (PIO and DMA)
    4FE2  Initiator command              4FF2   Initiator command
    4FE4  Mode                           4FF4   Mode
    4FE6  Target command                 4FF6   Target command
    4FE8  Current SCSI bus status        4FF8   Select enable
    4FEA  Bus and status                 4FFA   Start DMA send
    4FEC  Input data (DMA)               4FFC   Start DMA target receive
    4FEE  Reset parity/interrupt         4FFE   Start DMA initiator receive


    Drive mappings:
    SCS1 = SCSI ID 0
    SCS2 = SCSI ID 1
    ...
    SCS8 = SCSI ID 7


    Michael Zapf

    References

    [1] Western Horizon Technologies: SCSI Host Adapter Card / Hardware
        Installation Manual

    Acknowledgements

    Thanks to Don O'Neil of WHT for providing the required schematics, ROM dumps,
    and documentation

    TODO: Enable block DMA. The only other driver using eop is lbpc.

*****************************************************************************/

#include "emu.h"
#include "scsicard.h"
#include "bus/nscsi/devices.h"

#define LOG_WARN       (1U << 1)
#define LOG_CRU        (1U << 2)
#define LOG_EPROM      (1U << 3)
#define LOG_RAM        (1U << 4)
#define LOG_CONTR      (1U << 5)
#define LOG_CB         (1U << 6)
#define LOG_READY      (1U << 7)
#define LOG_DMA        (1U << 8)

#define VERBOSE (LOG_GENERAL | LOG_WARN)

#include "logmacro.h"

#define BUFFER "ram"
#define PLD_TAG "pld"
#define CONTR_TAG "scsibus:7:controller"
#define SCSIBUS_TAG "scsibus"

DEFINE_DEVICE_TYPE(TI99_WHTSCSI, bus::ti99::peb::whtech_scsi_card_device, "ti99_whtscsi", "Western Horizon Technologies SCSI host adapter")
DEFINE_DEVICE_TYPE(WHTSCSI_PLD, bus::ti99::peb::whtscsi_pld_device, PLD_TAG, "WHTech SCSI PLD")

namespace bus::ti99::peb {

whtech_scsi_card_device::whtech_scsi_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_WHTSCSI, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_buffer_ram(*this, BUFFER),
	m_pld(*this, PLD_TAG),
	m_controller(*this, CONTR_TAG),
	m_scsibus(*this, SCSIBUS_TAG),
	m_irq(false),
	m_drq(false),
	m_readyset(false)
{
}

void whtech_scsi_card_device::setaddress_dbin(offs_t offset, int state)
{
	// Do not allow setaddress for debugger
	if (machine().side_effects_disabled()) return;
	m_address = offset;
	if (in_dsr_space(offset, true) && m_pld->card_selected())
	{
		LOGMASKED(LOG_READY, "setadd %04x (%s)\n", offset & 0xffff, machine().describe_context());
		operate_ready_line();
	}
}

void whtech_scsi_card_device::operate_ready_line()
{
	m_pld->update_line_states(m_address, m_drq, m_irq);

	// Clear or assert the outgoing READY line
	bool newready = m_pld->ready();
	if (newready != m_readyset)
	{
		LOGMASKED(LOG_READY, "READY line changed to %d\n", newready);
		m_slot->set_ready(newready);
		m_readyset = newready;
	}
}

/*
    Debugger access.
*/
void whtech_scsi_card_device::debug_read(offs_t offset, uint8_t* value)
{
	offs_t addrcopy = m_address;
	m_address = offset;

	if (m_pld->sram_cs())
	{
		*value = m_buffer_ram->pointer()[(m_address & 0x0fff) | (m_pld->sram_bank()<<12)];
	}

	if (m_pld->eprom_cs())
	{
		// EPROM selected
		int base = (m_pld->eprom_bank()<<13);
		*value = m_eprom[base | (m_address & 0x1fff)];
	}
	m_address = addrcopy;
}

/*
    Debugger access.
*/
void whtech_scsi_card_device::debug_write(offs_t offset, uint8_t data)
{
	offs_t addrcopy = m_address;
	m_address = offset;

	if (m_pld->sram_cs())
	{
		m_buffer_ram->pointer()[(m_address & 0x0fff) | (m_pld->sram_bank()<<12)] = data;
	}
	m_address = addrcopy;
}

void whtech_scsi_card_device::readz(offs_t offset, uint8_t *value)
{
	if (machine().side_effects_disabled())
	{
		debug_read(offset, value);
		return;
	}

	if (!m_pld->card_selected()) return;

	if (in_dsr_space(offset, true)) LOGMASKED(LOG_READY, "rd %04x\n", offset & 0xffff);

	if (m_pld->eprom_cs())
	{
		// Lines A0-A12 directly connected to the EPROM (chip pin order)
		// Lines A13-A15 connected to PLD
		int base = (m_pld->eprom_bank()<<13);
		uint8_t* rom = &m_eprom[base | (m_address & 0x1fff)];
		*value = *rom;

		if (WORD_ALIGNED(m_address))
		{
			// Do logging by 16 bit words, as we typically have instructions in
			// the eprom
			uint16_t val = (*rom << 8) | (*(rom+1));
			LOGMASKED(LOG_EPROM, "DSR: %04x (bank %d) -> %04x\n", m_address & 0xffff, m_pld->eprom_bank(), val);
		}
	}

	if (m_pld->sram_cs())
	{
		// Lines A0-A11 directly connected to the SRAM (chip pin order)
		// Lines A12-A14 connected to PLD
		*value = m_buffer_ram->pointer()[(m_address & 0x0fff) | (m_pld->sram_bank()<<12)];
		LOGMASKED(LOG_RAM, "RAM: %04x (bank %d) -> %02x\n", m_address & 0xffff, m_pld->sram_bank(), *value);
	}

	if (m_pld->scsi_cs())
	{
		// Only for addresses 4fe0-4fef
		if ((m_address & 0x0010)==0x0000)
		{
			int reg = (m_address >> 1)&0x07;

			// If we are in DMA mode, reading from register 6 means DMA read
			if (m_drq && (reg == 6))
			{
				*value = m_controller->dma_r();
				LOGMASKED(LOG_DMA, "CTR: DMA in [%d] -> %02x (%s)\n",  m_dmacount++, *value, machine().describe_context());
			}
			else
			{
				*value = m_controller->read(reg);
				LOGMASKED(LOG_CONTR, "CTR: Reg %d (%04x) -> %02x (%s)\n", reg, m_address & 0xffff, *value, machine().describe_context());
			}
		}
	}
}

void whtech_scsi_card_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
	{
		debug_write(offset, data);
		return;
	}

	if (!m_pld->card_selected()) return;

	if (in_dsr_space(offset, true)) LOGMASKED(LOG_READY, "wr %04x\n", offset & 0xffff);

	if (m_pld->sram_cs())
	{
		// Lines A0-A11 directly connected to the SRAM (chip pin order)
		// Lines A12-A14 connected to PLD
		LOGMASKED(LOG_RAM, "RAM: %04x (bank %d) <- %02x\n", m_address & 0xffff, m_pld->sram_bank(), data);
		m_buffer_ram->pointer()[(m_address & 0x0fff) | (m_pld->sram_bank()<<12)] = data;
	}

	if (m_pld->scsi_cs())
	{
		// Only for addresses 4ff0-4fff
		if ((m_address & 0x0010)==0x0010)
		{
			int reg = (m_address >> 1)&0x07;

			// If we are in DMA mode, writing to register 0 means DMA write
			if (m_drq && (reg == 0))
			{
				LOGMASKED(LOG_DMA, "CTR: DMA out <- %02x [%d] (%s)\n",  data, m_dmacount++, machine().describe_context());
				m_controller->dma_w(data);
			}
			else
			{
				LOGMASKED(LOG_CONTR, "CTR: Reg %d (%04x) <- %02x (%s)\n", reg, m_address & 0xffff, data, machine().describe_context());
				m_controller->write(reg, data);
			}
		}
	}
}

/*
    CRU read access
*/
void whtech_scsi_card_device::crureadz(offs_t offset, uint8_t *value)
{
	m_pld->crureadz(offset, value);
}

/*
    CRU write access. The flags are kept in the PLD.
*/
void whtech_scsi_card_device::cruwrite(offs_t offset, uint8_t data)
{
	m_pld->cruwrite(offset, data);
	operate_ready_line();
}

/*
    Callbacks for the controller chip.
*/
void whtech_scsi_card_device::drq_w(int state)
{
	LOGMASKED(LOG_CB, "DRQ pin from controller = %d\n", state);
	bool drq = (state==ASSERT_LINE);
	if (drq != m_drq)
	{
		m_drq = drq;
		m_slot->set_inta((m_drq || m_irq)? ASSERT_LINE : CLEAR_LINE);
	}
	operate_ready_line();
}

void whtech_scsi_card_device::irq_w(int state)
{
	LOGMASKED(LOG_CB, "IRQ pin from controller = %d\n", state);
	bool irq = (state==ASSERT_LINE);
	if (irq != m_irq)
	{
		m_irq = irq;
		m_slot->set_inta((m_drq || m_irq)? ASSERT_LINE : CLEAR_LINE);
	}
	operate_ready_line();
}

void whtech_scsi_card_device::signal_scsi_eop(int state)
{
	m_controller->eop_w(state);
}

void whtech_scsi_card_device::device_add_mconfig(machine_config &config)
{
	// RAM circuit
	RAM(config, BUFFER).set_default_size("32K").set_default_value(0);

	// PLD circuit
	WHTSCSI_PLD(config, PLD_TAG, 0);

	// SCSI bus
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("controller", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.drq_handler().set(*this, FUNC(whtech_scsi_card_device::drq_w));
		adapter.irq_handler().set(*this, FUNC(whtech_scsi_card_device::irq_w));
	});
}

void whtech_scsi_card_device::device_start()
{
	m_eprom = memregion(TI99_DSRROM)->base();
	save_item(NAME(m_address));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_readyset));
}

void whtech_scsi_card_device::device_reset()
{
	m_cru_base = (ioport("SW1")->read()) << 8;
	m_sw2 = ioport("SW2")->read();
	m_drq = false;
	m_irq = false;
}

int whtech_scsi_card_device::get_sw1()
{
	return m_cru_base;
}

int whtech_scsi_card_device::get_sw2()
{
	return m_sw2;
}

/*
    The CRU address base for the card. According to [1], settings below
    1000 only make sense with an installed AT keyboard and ROM upgrade.
*/
INPUT_PORTS_START( whtscsi )
	PORT_START("SW1")
	PORT_DIPNAME(0x1f, 0x12, "SCSI card CRU base")
		PORT_DIPSETTING(0x06, "0600")
		PORT_DIPSETTING(0x07, "0700")
		PORT_DIPSETTING(0x08, "0800")
		PORT_DIPSETTING(0x09, "0900")
		PORT_DIPSETTING(0x0a, "0a00")
		PORT_DIPSETTING(0x0b, "0b00")
		PORT_DIPSETTING(0x0c, "0c00")
		PORT_DIPSETTING(0x0d, "0d00")
		PORT_DIPSETTING(0x0e, "0e00")
		PORT_DIPSETTING(0x0f, "0f00")
		PORT_DIPSETTING(0x10, "1000")
		PORT_DIPSETTING(0x11, "1100")
		PORT_DIPSETTING(0x12, "1200")    // Default setting
		PORT_DIPSETTING(0x13, "1300")
		PORT_DIPSETTING(0x14, "1400")
		PORT_DIPSETTING(0x15, "1500")
		PORT_DIPSETTING(0x16, "1600")
		PORT_DIPSETTING(0x17, "1700")
		PORT_DIPSETTING(0x18, "1800")
		PORT_DIPSETTING(0x19, "1900")
		PORT_DIPSETTING(0x1a, "1a00")
		PORT_DIPSETTING(0x1b, "1b00")
		PORT_DIPSETTING(0x1c, "1c00")
		PORT_DIPSETTING(0x1d, "1d00")
		PORT_DIPSETTING(0x1e, "1e00")
		PORT_DIPSETTING(0x1f, "1f00")

	PORT_START("SW2")
	PORT_DIPNAME(0x01, 0x00, "Driver LOAD/Parity enable")
		PORT_DIPSETTING(0x00, DEF_STR(Off))
		PORT_DIPSETTING(0x01, DEF_STR(On))

		// Currently, the controller is fixed to ID 7, according to device_add_mconfig,
		// and we cannot relocate the card on the bus
		PORT_DIPNAME(0x0e, 0x0e, "SCSI card ID")
		// PORT_DIPSETTING(0x0c, "6")
		PORT_DIPSETTING(0x0e, "7")
	PORT_DIPNAME(0x10, 0x00, "Geneve mode")
		PORT_DIPSETTING(0x00, DEF_STR(Off))
		PORT_DIPSETTING(0x10, DEF_STR(On))
INPUT_PORTS_END

ROM_START( whtscsi )
	ROM_REGION(0x10000, TI99_DSRROM, 0)
	ROM_LOAD("scsidsr150.u6", 0x0000, 0x10000, CRC(6e067c22) SHA1(f8d8861863e6a17042428e94967fd4ffc4b9d1de)) /* HFDC disk DSR ROM */
ROM_END


const tiny_rom_entry *whtech_scsi_card_device::device_rom_region() const
{
	return ROM_NAME( whtscsi );
}

ioport_constructor whtech_scsi_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(whtscsi);
}

// ========================================================================
//    PLD circuit on the board
// ========================================================================

whtscsi_pld_device::whtscsi_pld_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:  device_t(mconfig, WHTSCSI_PLD, tag, owner, clock),
	   m_board(nullptr),
	   m_selected(false),
	   m_sram_shadow(false),
	   m_dma_lock(false),
	   m_word_transfer(false),
	   m_bank_swapped(false),
	   m_readyout(true),
	   m_eprom_bank(0),
	   m_sram_bank(0)
{
}

void whtscsi_pld_device::device_start()
{
	save_item(NAME(m_selected));
	save_item(NAME(m_sram_shadow));
	save_item(NAME(m_dma_lock));
	save_item(NAME(m_word_transfer));
	save_item(NAME(m_bank_swapped));
	save_item(NAME(m_readyout));
	save_item(NAME(m_eprom_bank));
	save_item(NAME(m_sram_bank));
}

void whtscsi_pld_device::device_reset()
{
	m_selected = false;
	m_sram_shadow = false;
	m_readyout = true;
	m_dma_lock = false;
	m_word_transfer = false;
	m_bank_swapped = false;
	m_eprom_bank = 0;
	m_sram_bank = 0;
}

/*
    SRAM is accessible at 5000-5fff for bank_swapped=false, else at 4000-4fff.
    Needs to be turned on with the flag sram_shadow.
*/
line_state whtscsi_pld_device::sram_cs()
{
	if (!m_sram_shadow) return CLEAR_LINE;
	return (busen()
		&& (((m_board->m_address & 0x1000)==0) == m_bank_swapped))? ASSERT_LINE : CLEAR_LINE;
}

/*
    EPROM is selected when we are in the correct address space and neither
    SRAM nor the controller are selected.
*/
line_state whtscsi_pld_device::eprom_cs()
{
	return (busen()
		&& (sram_cs()==CLEAR_LINE)
		&& (scsi_cs()==CLEAR_LINE))? ASSERT_LINE : CLEAR_LINE;
}

/*
    The controller is selected in the 4fe0-4ffe space, or in 5fe0-5ffe, depending
    on the bank_swapped flag (4xxx for bank_swapped=false).
    Also, for byte mode, only the even addresses are mapped. In word mode, the
    controller shall be active for both even and odd address accesses. This is
    only needed for DMA.
*/
line_state whtscsi_pld_device::scsi_cs()
{
	return (busen()
		&& ((m_board->m_address & 0x0fe0)==0x0fe0)
		&& (((m_board->m_address & 0x1000)!=0) == m_bank_swapped)
		&& (((m_board->m_address & 1)==0) || m_word_transfer))? ASSERT_LINE : CLEAR_LINE;
}

/*
    Are we accessing the memory area 4000-5FFF, and this card has been turned on?
*/
bool whtscsi_pld_device::busen()
{
	return (((m_board->m_address & 0x7e000)==0x74000) && m_selected);
}

/*
    CRU read access
*/
void whtscsi_pld_device::crureadz(offs_t offset, uint8_t *value)
{
	int crubase = m_board->get_sw1();

	if ((offset & 0xff00)==crubase)
	{
		int bit = (offset >> 1) & 0x1f;
		switch (bit)
		{
		case 0:
			*value = m_board->m_irq? 1 : 0;
			break;
		case 1:
			*value = m_board->m_drq? 1 : 0;
			break;
		case 2:
			// READY; not available in the current 53C80 emulation
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			*value = ((m_board->get_sw2() & (1<<(bit-3)))!=0)? 1 : 0;
			break;
		default:
			break;
		}
		// LOGMASKED(LOG_CRU, "CRU %04x -> %d\n", offset & 0xffff, *value);
	}
}

/*
    CRU write access
*/
void whtscsi_pld_device::cruwrite(offs_t offset, uint8_t data)
{
	int crubase = m_board->get_sw1();
	if ((offset & 0xff00)==crubase)
	{
		// LOGMASKED(LOG_CRU, "CRU %04x <- %d (%s)\n", offset & 0xffff, data, machine().describe_context());
		int bit = (offset >> 1) & 0x1f;
		switch (bit)
		{
		case 0:           // card activation
			LOGMASKED(LOG_CRU, "DSR %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_selected = (data != 0);
			break;
		case 1:           // SRAM shadow
			LOGMASKED(LOG_CRU, "SRAM shadow %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_sram_shadow = (data != 0);
			break;
		case 2:           // DMA lock enable
			LOGMASKED(LOG_CRU, "DMA lock %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_dma_lock = (data != 0);
			m_board->m_dmacount = 0;
			break;
		case 3:           // SCSI EOP
			LOGMASKED(LOG_CRU, "SCSI EOP %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_board->signal_scsi_eop((data != 0)? ASSERT_LINE : CLEAR_LINE);
			break;
		case 4:
			LOGMASKED(LOG_CRU, "Word transfer %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_word_transfer = (data != 0);
			break;
		case 5:
			LOGMASKED(LOG_CRU, "Bank swap %s (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			m_bank_swapped = (data != 0);
			break;
		case 6:
			LOGMASKED(LOG_CRU, "Block mode %s (not implemented) (%s)\n", (data!=0)? "on" : "off", machine().describe_context());
			break;
		case 8:
		case 9:
		case 10:
			if (data != 0)
				m_eprom_bank = m_eprom_bank | (1<<(bit-8));
			else
				m_eprom_bank = m_eprom_bank & ~(1<<(bit-8));
			LOGMASKED(LOG_CRU, "Set EPROM bank %d (%s)\n", m_eprom_bank, machine().describe_context());
			break;
		case 12:
		case 13:
		case 14:
			if (data != 0)
				m_sram_bank = m_sram_bank | (1<<(bit-12));
			else
				m_sram_bank = m_sram_bank & ~(1<<(bit-12));
			LOGMASKED(LOG_CRU, "Set SRAM bank %d (%s)\n", m_sram_bank, machine().describe_context());
			break;
		default:
			break;
		}
	}
}

bool whtscsi_pld_device::ready()
{
	return m_readyout;
}

bool whtscsi_pld_device::card_selected()
{
	return m_selected;
}

/*
    Updates the state of the READY output of the PLD. In the real device, the
    SCSIDACK line is also updated, but this line is not used with the current
    implementation of the 53C80.

    READY is always asserted when an IRQ is pending or when we do not use DMA
    Otherwise, READY is asserted when a DRQ is pending
    When the DMA lock is active, and there is no DRQ yet, let the CPU wait
*/
void whtscsi_pld_device::update_line_states(int address, bool drq, bool irq)
{
	m_readyout = (irq || !m_dma_lock)? true : drq;
}

void whtscsi_pld_device::device_config_complete()
{
	m_board = dynamic_cast<whtech_scsi_card_device*>(owner());
	// owner is the empty_state during -listxml, so this will be nullptr
}

} // end namespace bus::ti99::peb
