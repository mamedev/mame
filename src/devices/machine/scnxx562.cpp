// license:BSD-3-Clause copyright-holders: Joakim Larsson Edstrom
/***************************************************************************

    DUSCC Dual Serial Communications Controller emulation

    The DUSCC was introduced in the mid 80:ies by Signetics, a part of Philips
    Semiconductor that later became NXP, and apparantly trying to dig into
    the huge success of the Zilog SCC with a very similar feature set but not
    software compatible at all.

    The variants in the DUSCC family are as follows:

                    Bus type
                Intel   Motorola
----------------------------------
    NMOS         26562   68562
    CMOS        26C562  68C562
----------------------------------
    For more info see:
      page 511: http://bitsavers.informatik.uni-stuttgart.de/pdf/signetics/_dataBooks/1986_Signetics_Microprocessor.pdf
      page 514: http://bitsavers.informatik.uni-stuttgart.de/pdf/signetics/_dataBooks/1994_Signetics_Data_Communications.pdf

Designs known of including one or more DUSCCs
------------------------------------------------
 Force Computers
   CPU VME boards: CPU-22, CPU-26, CPU-30, CPU-33, CPU-386, CPU-40, CPU-41
   Graphics VME boards: AGC-1
   Serial VME boards: ISIO-1, ISIO-2
 Digital Equipment
  DEC MicroServer DEMSA, DECrouter-150, DECrouter-250
------------------------------------------------

TODO/                     "NDUSCC"      "CDUSCC"
DONE (x) (p=partly)         NMOS         CMOS
------------------------------------------------
    Channels                2 FD         2 FD
    Synch data rates        4Mbps        10Mbps
 ----- asynchrounous features ------------------
 p  5-8 bit per char         Y             Y
 y  1,1.5,2 stop bits        Y             Y     in 1/16 bit increments
 p  odd/even parity          Y             Y
    x1,x16                   Y             Y
    break det/gen            Y             Y
    parity, framing &        Y             Y
    overrun error det
    -- byte oriented synchrounous features --
    Int/ext char sync        Y             Y
    1/2 synch chars          ?             ?
    Aut CRC gen/det          Y             Y
    -- SDLC/HDLC capabilities ---------------
    Abort seq gen/chk        Y             Y
    Aut zero ins/det         Y             Y
    Aut flag insert          Y             Y
    Addr field rec           Y             Y
    I-fld resid hand         Y             Y
    CRC gen/det              Y             Y
    SDLC loop w EOP          Y             Y
    --
    Receiver FIFO            4             16
    Transmitter FIFO         4             16
    NRZ, NRZI, FM1 or        Y             Y
     FM2 enc/dec
    Manchester dec           Y             Y
    Baud gen per chan        Y             Y
    DPLL clock recov         Y             Y
    -- Additional features CMOS versions -----
    Status FIFO              N             Y
    Watchdog timer           N             Y
    Fifo Fill status         N             Y
    DMA frame status         N             Y
    Rx/TxRDY on FIFO lvl     N             Y
    TxFifo Empty status      N             Y
    Interrupt enable bits    N             Y
    X.21 pattern recogn      N             Y
    Improved BiSync support  N             Y
    -------------------------------------------------------------------------
   x/p = Features that has been implemented  n/a = features that will not
***************************************************************************/

#include "scnxx562.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************
/* Useful temporary debug printout format */
// printf("TAG %lld %s%s Data:%d\n", machine().firstcpu->total_cycles(), __PRETTY_FUNCTION__, m_owner->tag(), data);

#define VERBOSE 2

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOGR(x)
#if VERBOSE == 0
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64%"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
const device_type DUSCC         = &device_creator<duscc_device>;
const device_type DUSCC_CHANNEL = &device_creator<duscc_channel>;
const device_type DUSCC26562    = &device_creator<duscc26562_device>;
const device_type DUSCC26C562   = &device_creator<duscc26C562_device>;
const device_type DUSCC68562    = &device_creator<duscc68562_device>;
const device_type DUSCC68C562   = &device_creator<duscc68C562_device>;

//-------------------------------------------------
//  device_mconfig_additions -
//-------------------------------------------------
MACHINE_CONFIG_FRAGMENT( duscc )
	MCFG_DEVICE_ADD(CHANA_TAG, DUSCC_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, DUSCC_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor duscc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( duscc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  duscc_device - constructor
//-------------------------------------------------
duscc_device::duscc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		//  device_z80daisy_interface(mconfig, *this),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
#if 0
	m_rxca(0),
	m_txca(0),
	m_rxcb(0),
	m_txcb(0),
#endif
	m_out_txda_cb(*this),
	m_out_dtra_cb(*this),
	m_out_rtsa_cb(*this),
	m_out_synca_cb(*this),
	m_out_txdb_cb(*this),
	m_out_dtrb_cb(*this),
	m_out_rtsb_cb(*this),
	m_out_syncb_cb(*this),
	m_variant(variant)
{
	for (auto & elem : m_int_state)
		elem = 0;
}

duscc_device::duscc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DUSCC, "DUSCC", tag, owner, clock, "duscc", __FILE__),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
	m_out_txda_cb(*this),
	m_out_dtra_cb(*this),
	m_out_rtsa_cb(*this),
	m_out_synca_cb(*this),
	m_out_txdb_cb(*this),
	m_out_dtrb_cb(*this),
	m_out_rtsb_cb(*this),
	m_out_syncb_cb(*this),
	m_variant(TYPE_DUSCC)
{
	for (auto & elem : m_int_state)
		elem = 0;
}

duscc26562_device::duscc26562_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: duscc_device(mconfig, DUSCC26562, "DUSCC 26562", tag, owner, clock, TYPE_DUSCC26562, "duscc26562", __FILE__){ }

duscc26C562_device::duscc26C562_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: duscc_device(mconfig, DUSCC26C562, "DUSCC 26C562", tag, owner, clock, TYPE_DUSCC26C562, "duscc26C562", __FILE__){ }

duscc68562_device::duscc68562_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: duscc_device(mconfig, DUSCC68562, "DUSCC 68562", tag, owner, clock, TYPE_DUSCC68562, "duscc68562", __FILE__){ }

duscc68C562_device::duscc68C562_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: duscc_device(mconfig, DUSCC68C562, "DUSCC 68C562", tag, owner, clock, TYPE_DUSCC68C562, "duscc68C562", __FILE__){ }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void duscc_device::device_start()
{
	LOG(("%s\n", FUNCNAME));
	// resolve callbacks
	m_out_txda_cb.resolve_safe();
	m_out_dtra_cb.resolve_safe();
	m_out_rtsa_cb.resolve_safe();
	m_out_synca_cb.resolve_safe();
	m_out_txdb_cb.resolve_safe();
	m_out_dtrb_cb.resolve_safe();
	m_out_rtsb_cb.resolve_safe();
	m_out_syncb_cb.resolve_safe();

	// state saving
	//save_item(NAME(m_int_state));

	LOG((" - DUSCC variant %02x\n", m_variant));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void duscc_device::device_reset()
{
	LOG(("%s %s \n",tag(), FUNCNAME));

	m_chanA->reset();
	m_chanB->reset();
}

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void duscc_device::check_interrupts()
{
	LOG(("%s %s - not implemented\n",tag(), FUNCNAME));
	//  m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void duscc_device::reset_interrupts()
{
	LOG(("%s %s - not implemented \n",tag(), FUNCNAME));
#if 0
	// reset internal interrupt sources
	for (auto & elem : m_int_state)
	{
		elem = 0;
	}

	// check external interrupt sources
	check_interrupts();
#endif
}

UINT8 duscc_device::modify_vector(UINT8 vec, int i, UINT8 src)
{
	LOG(("%s %s - not implemented\n",tag(), FUNCNAME));
	return vec;
}


//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------
void duscc_device::trigger_interrupt(int index, int state)
{
	LOG(("%s %s - not implemented\n",tag(), FUNCNAME));
}

READ8_MEMBER( duscc_device::read )
{
	if ( offset & 0x20 )
		return m_chanB->read(offset);
	else
		return m_chanA->read(offset);
}

WRITE8_MEMBER( duscc_device::write )
{
	if ( offset & 0x20 )
		m_chanB->write(data, offset);
	else
		m_chanA->write(data, offset);
	return;
}

//**************************************************************************
//  DUSCC CHANNEL
//**************************************************************************
duscc_channel::duscc_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DUSCC_CHANNEL, "DUSCC channel", tag, owner, clock, "duscc_channel", __FILE__),
		device_serial_interface(mconfig, *this),
		m_brg_rx_rate(0),
		m_brg_tx_rate(0),
		m_brg_const(1),
		m_rx_error(0),
		m_rx_clock(0),
		m_rx_first(0),
		m_rx_break(0),
		m_rxd(0),
		m_cts(0),
		m_dcd(0),
		m_tx_data(0),
		m_tx_clock(0),
		m_dtr(0),
		m_rts(0),
		m_sync(0)
{
	LOG(("%s\n",FUNCNAME));

	// Reset all registers
	m_cmr1 =  m_cmr2 =  m_s1r =  m_s2r =  m_tpr =  m_ttr =  m_rpr =  m_rtr
		=  m_ctprh =  m_ctprl =  m_ctcr =  m_omr =  m_cth =  m_ctl =  m_pcr
		=  m_ccr =  m_rsr =  m_trsr =  m_ictsr =  m_gsr =  m_ier /* =  m_rea  */
		=  m_cid =  m_ivr =  m_icr =  /*m_sea =*/  m_ivrm =  m_mrr =  m_ier1
		=  m_ier2 =  m_ier3 =  m_trcr =  m_rflr =  m_ftlr =  m_trmsr =  m_telr = 0;

	for (int i = 0; i < sizeof(m_rx_data_fifo); i++)
	{
		m_rx_data_fifo[i] = 0;
		m_rx_error_fifo[i] = 0;
	}
	for (int i = 0; i < sizeof(m_tx_data_fifo); i++)
	{
		m_tx_data_fifo[i] = 0;
		m_tx_error_fifo[i] = 0;
	}
}

//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void duscc_channel::device_start()
{
	LOG(("%s\n", FUNCNAME));
	m_uart = downcast<duscc_device *>(owner());
	m_index = m_uart->get_channel_index(this);

	m_rx_fifo_sz = (m_uart->m_variant & SET_CMOS) ? 16 : 4;
	m_rx_fifo_wp = m_rx_fifo_rp = 0;

	m_tx_fifo_sz = (m_uart->m_variant & SET_CMOS) ? 16 : 4;
	m_tx_fifo_wp = m_tx_fifo_rp = 0;

	m_cid = (m_uart->m_variant & SET_CMOS) ? 0x7f : 0xff; // TODO: support CMOS rev A = 0xbf

	// state saving
	save_item(NAME(m_cmr1));
	save_item(NAME(m_cmr2));
	save_item(NAME(m_s1r));
	save_item(NAME(m_s2r));
	save_item(NAME(m_tpr));
	save_item(NAME(m_ttr));
	save_item(NAME(m_rpr));
	save_item(NAME(m_rtr));
	save_item(NAME(m_ctprh));
	save_item(NAME(m_ctprl));
	save_item(NAME(m_ctcr));
	save_item(NAME(m_omr));
	save_item(NAME(m_cth));
	save_item(NAME(m_ctl));
	save_item(NAME(m_pcr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_txfifo));
	save_item(NAME(m_rxfifo));
	save_item(NAME(m_rsr));
	save_item(NAME(m_trsr));
	save_item(NAME(m_ictsr));
	save_item(NAME(m_gsr)); // TODO: Move this to the device instead, it is a global register
	save_item(NAME(m_ier));
	//  save_item(NAME(m_rea));
	save_item(NAME(m_cid));
	save_item(NAME(m_ivr));
	save_item(NAME(m_icr));
	//  save_item(NAME(m_sea));
	save_item(NAME(m_ivrm));
	save_item(NAME(m_mrr));
	save_item(NAME(m_ier1));
	save_item(NAME(m_ier2));
	save_item(NAME(m_ier3));
	save_item(NAME(m_trcr));
	save_item(NAME(m_rflr));
	save_item(NAME(m_ftlr));
	save_item(NAME(m_trmsr));
	save_item(NAME(m_telr));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_fifo_rp));
	save_item(NAME(m_rx_fifo_wp));
	save_item(NAME(m_rx_fifo_sz));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_ri));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_sync));

	device_serial_interface::register_save_state(machine().save(), this);
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void duscc_channel::device_reset()
{
	LOG(("%s\n", FUNCNAME));

	// Reset RS232 emulation
	receive_register_reset();
	transmit_register_reset();

	// Soft/Channel Reset values according to DUSCC users guide
	m_cmr1      =0x00;
	m_cmr2      =0x00;
	m_s1r       =0x00;
	m_s2r       =0x00;
	m_tpr       =0x00;
	m_ttr       =0x00;
	m_rpr       =0x00;
	m_rtr       =0x00;
	m_ctcr      =0x00;
	m_omr       =0x00;
	m_pcr       =0x00;
	m_ccr       =0x00;
	m_rsr       =0x00;
	m_trsr      =0x00;
	m_ictsr     =0x00;
	m_gsr       =0x00;
	m_ier       =0x00;
	//  m_rea       =0x00;
	m_ivr       =0x0f;
	m_icr       =0x00;
	//  m_sea       =0x00;
	m_ivrm      =0x00;
	m_mrr       =0x00; // TODO: Need a read after reset to enable CMOS features
	m_ier1      =0x00;
	m_ier2      =0x00;
	m_ier3      =0x00;
	m_trcr      =0x00;
	m_rflr      =0x00;
	m_ftlr      =0x33;
	m_trmsr     =0x00;
	m_telr      =0x10;

	// reset external lines TODO: check relation to control bits and reset
	set_rts(1);
	set_dtr(1);

	// reset interrupts
	if (m_index == duscc_device::CHANNEL_A)
	{
		m_uart->reset_interrupts();
	}

	m_a7 = 0;
}

void duscc_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	//  LOG(("%s %d\n", FUNCNAME, id));
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void duscc_channel::tra_callback()
{
	if (!is_transmit_register_empty())
	{
		int db = transmit_register_get_data_bit();

		LOGR((LLFORMAT " %s() \"%s \"Channel %c transmit data bit %d\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, db));

		// transmit data
		if (m_index == duscc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(db);
		else
			m_uart->m_out_txdb_cb(db);
	}
	else
	{
		LOG((LLFORMAT " %s() \"%s \"Channel %c Failed to transmit \n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index));
		logerror("%s \"%s \"Channel %c Failed to transmit\n", FUNCNAME, m_owner->tag(), 'A' + m_index);
	}
}


//------------------------------------------
//  tra_complete -
// TODO:
// - Fix mark and space tx support
//------------------------------------------

void duscc_channel::tra_complete()
{
	if (m_tra == 1) // transmitter enabled?
	{
		if (m_tx_fifo_rp != m_tx_fifo_wp) // there are more characters to send?
		{
			transmit_register_setup(m_tx_data_fifo[m_tx_fifo_rp]); // Reload the shift register
			m_tx_fifo_rp_step();
		}
		if (m_omr & REG_OMR_TXRDY_ACTIVATED)// Wait until FIFO empty before ready for more data?
		{
			if (m_tx_fifo_wp == m_tx_fifo_rp) // So is Tx FIFO empty?
				m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
		}
		else // Always ready for more!
			m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
	}
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void duscc_channel::rcv_callback()
{
	if (m_rcv == 1)
	{
		LOG((LLFORMAT " %s() \"%s \"Channel %c received data bit %d\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_rxd));
		receive_register_update_bit(m_rxd);
	}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void duscc_channel::rcv_complete()
{
	UINT8 data;

	receive_register_extract();
	data = get_received_char();
	LOG((LLFORMAT " %s() \"%s \"Channel %c Received Data %c\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, data));
	receive_data(data);
}


//-------------------------------------------------
//  get_xx_clock_mode - get clock divisor
// TODO
// - support all the other clock divisors
// - actually use the divisors when calculating baud
//-------------------------------------------------

int duscc_channel::get_rx_clock_mode()
{
	int clocks = 1;

	if ( (m_rtr & REG_RTR_RXCLK_MASK) == REG_RTR_RXCLK_BRG )
		clocks = 32;

	return clocks;
}

int duscc_channel::get_tx_clock_mode()
{
	int clocks = 1;

	if ( (m_ttr & REG_TTR_TXCLK_MASK) == REG_TTR_TXCLK_BRG )
		clocks = 32;

	return clocks;
}

void duscc_channel::set_rts(int state)
{
	LOG(("%s(%d) \"%s\": %c \n", FUNCNAME, state, m_owner->tag(), 'A' + m_index));
	if (m_index == duscc_device::CHANNEL_A)
		m_uart->m_out_rtsa_cb(state);
	else
		m_uart->m_out_rtsb_cb(state);
}

/* --------------------------------------------------------------------------
 *  get_stop_bits - get number of stop bits
 *  The DUSCC supports from 1/2 stop bit to 2 stop bits in 1/16th bit increments
 *  This is not yet supported by diserial so we need to translate into 1, 1.5
 *  or 2 stop bits. It is also dependent on the data bit length
 *  TODO: Support finer granularity of stop bits in diserial if/when nessesarry
 * ---------------------------------------------------------------------------
 *  TPR[4:7]   TPR[0:1]
 *           5 bits  6-8 bits
 *             00   01,10,11
 * ---------------------------
 *  0 0 0 0  1.063   0.563
 *  0 0 0 1  1.125   0.625
 *  0 0 1 0  1.188   0.688
 *  0 0 1 1  1.250   0.750
 *  0 1 0 0  1.313   0.813
 *  0 1 0 1  1.375   0.875
 *  0 1 1 0  1.438   0.938
 *  0 1 1 1  1.500   1.000
 *  1 0 0 0  1.563   1.563
 *  1 0 0 1  1.625   1.625
 *  1 0 1 0  1.688   1.688
 *  1 0 1 1  1.750   1.750
 *  1 1 0 0  1.813   1.813
 *  1 1 0 1  1.875   1.875
 *  1 1 1 0  1.938   1.938
 *  1 1 1 1  2.000   2.000
 * --------------------------------------------------------------------------
 */
device_serial_interface::stop_bits_t duscc_channel::get_stop_bits()
{
	const stop_bits_t bits5[] =
		{ STOP_BITS_1,   STOP_BITS_1,   STOP_BITS_1,   STOP_BITS_1, STOP_BITS_1_5, STOP_BITS_1_5, STOP_BITS_1_5, STOP_BITS_1_5,
			STOP_BITS_1_5, STOP_BITS_1_5, STOP_BITS_1_5, STOP_BITS_2, STOP_BITS_2,   STOP_BITS_2,   STOP_BITS_2,   STOP_BITS_2 };
	const stop_bits_t bits6to8[] =
		{ STOP_BITS_1,   STOP_BITS_1,   STOP_BITS_1,   STOP_BITS_1, STOP_BITS_1, STOP_BITS_1, STOP_BITS_1, STOP_BITS_1,
			STOP_BITS_1,   STOP_BITS_1_5, STOP_BITS_1_5, STOP_BITS_2, STOP_BITS_2, STOP_BITS_2, STOP_BITS_2, STOP_BITS_2 };

	/* 5 data bits */
	if (get_tx_word_length() == 5)
	{
		return bits5[((m_tpr & REG_TPR_STOP_BITS_MASK) >> 4) & 0x0f];
	}
	else /* 6-8 data bits */
	{
		return bits6to8[((m_tpr & REG_TPR_STOP_BITS_MASK) >> 4) & 0x0f];
	}

	return STOP_BITS_0;
}

//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------

int duscc_channel::get_rx_word_length()
{
	int bits = 5;

	switch (m_rpr & REG_RPR_DATA_BITS_MASK)
	{
	case REG_RPR_DATA_BITS_5BIT: bits = 5; break;
	case REG_RPR_DATA_BITS_6BIT: bits = 6; break;
	case REG_RPR_DATA_BITS_7BIT: bits = 7; break;
	case REG_RPR_DATA_BITS_8BIT: bits = 8; break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------

int duscc_channel::get_tx_word_length()
{
	int bits = 5;

	switch (m_tpr & REG_TPR_DATA_BITS_MASK)
	{
	case REG_TPR_DATA_BITS_5BIT: bits = 5; break;
	case REG_TPR_DATA_BITS_6BIT: bits = 6; break;
	case REG_TPR_DATA_BITS_7BIT: bits = 7; break;
	case REG_TPR_DATA_BITS_8BIT: bits = 8; break;
	}

	return bits;
}

UINT8 duscc_channel::do_dusccreg_cmr1_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_cmr2_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_s1r_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_s2r_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_tpr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ttr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_rpr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_rtr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ctprh_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ctprl_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ctcr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_omr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_cth_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ctl_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_pcr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }

/* Commands to the DUSCC are entered through the channel command register.A read of this
   register returns the last invoked command (with bits 4 and 5 set to 1). */
UINT8 duscc_channel::do_dusccreg_ccr_r()
{
	LOG(("%s\n", FUNCNAME));
	return (UINT8) m_ccr | 1 << 4 | 1 << 5;
}

UINT8 duscc_channel::do_dusccreg_rxfifo_r()
{
	UINT8 data = 0;

	LOG(("%s\n", FUNCNAME));
	LOG((" - RX rp:%d wp:%d sz:%d\n", m_rx_fifo_rp, m_rx_fifo_wp, m_rx_fifo_sz));

	/* So is there a character in the FIFO? */
	if (m_rx_fifo_rp != m_rx_fifo_wp)
	{
		data = m_rx_data_fifo[m_rx_fifo_rp];
		m_rx_fifo_rp_step();
		LOG((" - RX reading out data:%02x '%c'\n", data, isalnum(data) ? data : ' '));
	}
	else
	{
		logerror("- RX FIFO empty despite RxREADY\n");
		LOG(("- RX FIFO empty despite RxREADY\n"));
	}

	return (UINT8) data;
}

UINT8 duscc_channel::do_dusccreg_rsr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_trsr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ictsr_r()
{
	logerror("register access method %s is not implemented yet\n", FUNCNAME);
	return (UINT8) m_ictsr;
}

/* General Status Register (GSR)
   This register provides a 'quick look' at the overall status of both channels of the DUSCC. A write to this register with ls at the
   corresponding bit pOSitions causes TxRDY (bits 5 and 1) and/or RxRDY (bits 4 and 0) to be reset. The other status bits can be reset
   only by resetting the individual status bits that they point to.
    [7] Channel B External or Counter/timer Status - This bit indicates that one of the following status bits is asserted: ICTSRB[6:4]
    [6] Channel B Receiver or Transmitter Status - This bit indicates that one of the following status bits is asserted: TRSRB[7:1], TRSRB[7:3].
    [5] Channel B Transmitter Ready - The assertion of this bit indicates that one or more characters may be loaded into the Channel B transmitter
       FIFO to be serialized by the transmit shift register. See description of OMR[4j. This bit can be asserted only when the transmitter is enabled.
       Resetting the transmitter negates TxRDY.
    [4] Channel B Receiver Ready - The assertion of this bit indicates that one or more characters are available in the Channel B receiver
       FIFO to be read by the CPU. See deSCription of OMR[3]. RxRDY is initially reset (negated) by a chip reset or when a 'reset Channel B
       receiver' command is invoked.
    [3] Channel A External or Countermmer Status - This bit indicates that one of the following status bits is asserted: ICTSRA[6:4].
    [2] Channel A Receiver or Transmitter Status - This bit indicates that one of the following status bits is asserted: TRSRA[7:0], TRSRA[7:3].
    [1] Channel A Transmitter Ready - The assertion of this bit indicates that one or more characters may be loaded into the Channel A
        transmitter FIFO to be serialized by the transmit shift register. See description of OMR[4]. This bit can be asserted only
        when the transmitter is enabled. Resetting the transmitter negates TxRDY.
    [0] Channel A Receiver Ready - The assertion of this bit indicates that one or more characters are available in the Channel A receiver
    FIFO to be read by the CPU. See description of OMR[3]. RxRDY is initially reset (negated) by a chip reset or when a 'reset Channel A
    receiver' command is invoked.
*/
UINT8 duscc_channel::do_dusccreg_gsr_r()
{
	LOGR(("%s <- %02x\n", FUNCNAME, m_gsr));
	return (UINT8) m_gsr;
}

UINT8 duscc_channel::do_dusccreg_ier_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }

UINT8 duscc_channel::do_dusccreg_cid_r()
{
	LOG(("%s\n", FUNCNAME));
	if ( m_uart->m_variant != SET_CMOS )
	{
		logerror("Attempt read out CDUSCC register CID on an NDUSCC\n");
		return 0;
	}
	if ( m_index != duscc_device::CHANNEL_B )
	{
		logerror("Attempt read out CID on channel B not allowed\n");
		return 0;
	}
	else
		return m_cid;
}

UINT8 duscc_channel::do_dusccreg_ivr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_icr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ivrm_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_mrr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ier1_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ier2_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ier3_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_trcr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_rflr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_ftlr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_trmsr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }
UINT8 duscc_channel::do_dusccreg_telr_r(){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return (UINT8) 0; }

	// write register handlers
/* CMR1 register
    [7:6] Data Encoding - These bits select the data encoding for the received and transmitted data:
     00 If the DPLL is set to NRZI mode (see DPLL commands), it selects positive logic (1 = high, 0 = low).
        If the DPLL is set to FM mode (see DPLL commands), Manchester (bi-phase level) encoding is selected.
     01 NRZI. Non-return-to-zero inverted.
     10 FMO. Bi-phase space.
     11 FM 1. Bi-phase mark.
    [5] Extended Control (BOP) -
      0 No. A one-octet control field follows the address field.
      1 Yes. A two-octet control field follows the address field.
    [5] Parity (COP/ ASYNC), Code Select (BISYNC)
      0 Even parity if with parity is selected by [4:3] or a 0 in the parity bit position if force parity is
        selected by [4:3]. In BISYNC protocol mode, internal character comparisons are made using EBCDIC coding.
      1 Odd parity if with parity is selected by [4:3] or a 1 in the parity bit position if force parity is selected by [4:3].
        In BISYNC protocol mode, internal character comparisons are made using Bbit ASCII coding.
    [4:3] Address Mode (BOP) -
        This field controls whether a single octet or multiple octets follow the opening FLAG(s) for both the receiver and the transmitter.
        This field is activated by selection of BOP secondary mode through the channel protocol mode bits CMR1_[2:0] (see Detailed Operation).
     00 Single octet address.
     01 Extended address.
     10 Dual octet address.
     11 Dual octet address with group.
    [4:3] Parity Mode (COP/ASYNC) -
        This field selects the parity mode for both the receiver and the transmitter. A parity bit is added to the programmed character length if
        with parity or force parity is selected:
     00 No parity. Required when BISYNC protocol mode is programmed.
     01 Reserved.
     10 With parity. Odd or even parity is selected by [5].
     11 Force parity. The parity bit is forced to the state selected by [5].
    [2:0] Channel Protocol Mode -
        This field selects the operational protocol and submode for both the receiver and transmitter:
     000 - BOP Primary. No address comparison is performed. For receive, all characters received after the opening FLAG(s) are transferred to the FIFO.
     001 - BOP Secondary. This mode activates the address modes selected by [4:3]. Except in the case of extended address ([4:3]=01), an address comparison
        is performed to determine if a frame should be received. Refer to Detailed Operation for details of the various addressing modes. If a valid comparison
        occurs, the receiver is activated and the address octets and all subsequent received characters of the frame are transferred to the receive FIFO.
     010 - BOP Loop. The DUSCC acts as a secondary station in a loop. The GO-ON-LOOP and GO-OFF-LOOP commands are used to cause the DUSCC to go on and off the
        loop. Normally, the TXD output echoes the RXD input with a three bit time delay. If the transmitter is enabled and the 'go active on poll' command has been
        asserted, the transmitter will begin sending when an EOP sequence consisting of a zero followed by seven ones is detected. The DUSCC changes the last one of
        the EOP to zero, making it another FLAG, and then operates as described in the detailed operation section. The loop sending status bit (TRSR[6]) is asserted
        concurrent with the beginning of transmission. The frame should normally be terminated with an EOM followed by an echo of the marking RXD line so that secondary
        stations further down the loop can append their messages to the messages from up-loop stations by the same process. If the 'go active on poll'command is not
        asserted, the transmitter remains inactive (other than echOing the received data) even when the EOP sequence is received.
    011 - BOP Loop without address comparison. Same as normal loop mode except that address field comparisons are disabled. All received frames aretransmitted to the CPU.
    100 - COP Dual SYN. Character sync is achieved upon receipt of a bit sequence matching the contents of the appropriate bits of SIR and S2R (SYNI-SYN2), including
        parity bits if any.
    101 - COP Dual SYN (BISYNC). Character sync is achieved upon receipt of a bit sequence matching the contents of the appropriate bits of SI Rand S2R
        (SYN1?SYN2). In this mode, special transmitter and receive logic is activated. Transmitter and receiver character length must be programmed to 8 bits and no parity
    110 - COP Single SYN. Character sync is achieved upon receipt of a bit sequence matching the contents of the appropriate bits of Sl R (SYN1), including parity bit if any.
        This mode is required when the external sync mode is selected.
    111 Asynchronous. Start/stop format.
*/
void duscc_channel::do_dusccreg_cmr1_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_cmr1 = data;
	LOG(("- Setting up %s mode\n", (m_cmr1 & REG_CMR1_CPMODE_MASK) == REG_CMR1_CPMODE_ASYNC ? "ASYNC" : "SYNC"));
	LOG(("- Parity: %s\n", ((m_cmr1 & REG_CMR1_PMMODE_MASK) == REG_CMR1_PMMODE_PARITY ?  (m_cmr1 & REG_CMR1_PARITY ? "odd" : "even") : "none")));
	return;
}

/* CMR2 register
    [7:6] Channel Connection - This field selects the mode of operation of the channel. The user must exercise care when switching into and out of the various modes. The
          selected mode will be activated immediately upon mode selection, even if this occurs in the middle of a received or transmitted character.

     00 - Normal mode. The 1ransmitter and receiver operate independently in either half or full-duplex, controlled by the respective enable commands.

     01 - Automatic echo mode. Automatically retransmits the received data with a half-bit time delay (ASYNC, 16X clock mode) or a one-bit time delay (allother modes).
     The following conditions are true while in automatic echo mode:
      1. Received data is reclocked and retransmitted on the TXD output.
      2. The receiver clock is used for the transmitter.
      3. The receiver must be enabled, but the transmitter need not be enabled.
      4. The TXRDY and underrun status bits are inactive.
      5. The received parity and/or FCS are checked if required, but are not regenerated for transmission,
         i.e., transmitted parity and/ or FCS are as received.
      6. In ASYNC mode, character framing is checked, but the stop bits are retransmitted as received.
         A received break is echoed as received.
      7. CPU to receiver communication continues normally, but the CPU to transmitter link is disabled.

     10 - Local loopback mode. In this mode:
      1. The transmitter output is internally connected to the receiver input.
      2. The transmit clock is used for the receiver if NRZI or NRZ encoding is used. For FM or Manchester encoding because the receiver clock is derived from the DPLL,
         the DPLL source clock must be maintained.
      3. The TXD output is held high.
      4. The RXD input is ignored.
      5. The receiver and transmitter must be enabled.
      6. CPU to transmitter and receiver communications continue normally.

     11 - Reserved.

    [5:3] Data Transfer Interface - This field specifies the type of data transfer between the DUSCC's RX and TX FIFOs and the CPU.
          All interrupt and status functions operate normally regardless of the data transfer interface programmed.
     000 - Half duplex single address DMA.
     001 - Half duplex dual address DMA.
     010 - Full duplex single address DMA.
     011 - Full duplex dual address DMA.
     100 - Wait on receive only. In this mode a read of a non-empty receive FIFO results in a normal bus cycle. However, if the receive FIFO of the channel
           is empty when a read RX FIFO cycle is initiated, the DTACKN output remains negated until a character is received and loaded into the FIFO.
           DT ACKN is then asserted and the cycle is completed normally.
     101 - Wait on transmit only. In this mode a write to a non-full transmit FI Fa results in a normal bus cycle. However, if the transmit FIFO of the channel is
           full when a write TX FIFO cycle is initiated, the DTACKN output remains negated until a FI Fa position becomes available for the new character. DT ACKN
           is then asserted and the cycle is completed normally.
     110 - Wait on transmit and receive. As above for both wait on receive and transmit operations.
     111 - Polled or interrupt. DMA and wait functions of the channel are not activated. Data transfers to the RX and TX FIFOs are via normal bus read and
           write cycles in response to polling of the status registers and/or interrupts.

    [2:0] Frame Check Sequence Select - This field selects the optional frame check sequence (FCS) to be appended at the end of a transmitted frame.
          When CRC is selected in COP, then no parity and 8-bit character length must be used. The selected FCS is transmitted as follows:
     1. Following the transmission of a FIFO'ed character tagged with the 'send EOM' command.
     2. If underrun control (TPR[7:6]) is programmed for TEOM, upon occurrence of an underrun.
     3. If TEOM on zero count or done (TPR[4]) is asserted and the counter/timer is counting transmitted characters, after transmission of the character which
        causes the counter to reach zero count.
     4. In DMA mode with TEOM on zero count or done (TPR[4]) set, after transmission of a character if DONEN is asserted when that character was loaded into the
        TX FIFO by the DMA controller.

     000 - No frame check sequence.
     001 - Reserved
     010 - LRC8: Divisor ~ x8+ 1, dividend preset to zeros. The TX sends the calculated LRC non-inverted. The RX indicates an error if the computed LRC is
           not equal to O. Valid for COP modes only.
     011 - LRC8: Divisor ~ x8+ 1, dividend preset to ones. The TX sends the calculated LRC non-inverted. The RX indicates
           an error if the computed LRC is not equal to O. Valid for COP modes only.
     100 - CRCI6: Divisor ~ x16+x15+x2+1, dividend preset to zeros. The TX sends the calculated CRC non-inverted. The RX indicates an error if the
           computed CRC is not equal to O. Not valid for ASYNC mode.
     101 - CRCI6: Divisor ~ x16+x15+x2+1, dividend preset to ones. The TX sends the calculated CRC non-inverted. The RX indicates an error if the
           computed CRC is not equal to O. Not valid for ASYNC mode.
     110 - CRC-CCITT: Divisor ~ x16+x12+x5+1, dividend preset to zeros. The TX sends the calculated CRC non-inverted. The RX indicates an error if the
           computed CRC is not equal to O. Not valid for ASYNC mode.
     111 CRC-CCITT: Divisor ~ x16+x12+x5+1, dividend preset to ones. The TX sends the calculated CRC inverted. The RX indicates an error if the computed
           CRC is not equal to H' FOB8'. Not valid for ASYNC mode.
*/
void duscc_channel::do_dusccreg_cmr2_w(UINT8 data)
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	m_cmr2 = data;
	LOG(("- Preparing for %s driven transfers\n", (m_cmr2 & REG_CMR2_DTI_MASK) == REG_CMR2_DTI_NODMA ? "polled or interrupt" : "dma"));
	return;
}

void duscc_channel::do_dusccreg_s1r_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_s2r_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }

/* Transmitter Parameter Register (TPRA, TPRB)
    SYNC mode
    [7:6] Underrun Control - In BOP and COP modes, this field selects the transmitter response in the event of an underrun (i.e., the TX FIFO is empty).
     00 - Normal end of message termination. In BOP, the transmitter sends the FCS (if selected by CMR2[2:011 followed by a FLAG and then either MARKs or
          FLAGs, as specified by [5]. In COP, the transmitter sends the FCS (if selected by CMR2[2:0]) and then either MARKs or SYNs, as specified by [5].
     01 - Reserved.
     l0 - in BOP, the transmitter sends an ABORT (11111111) and then places the TXD output in a marking condition until receipt of further instructions.
          In COP, the transmitter places the TXD output in a marking condition until receipt of further instructions.
     11 - In BOP, the transmitter sends an ABORT (11111111) and then sends FLAGs until receipt of further instructions. In COP, the transmitter sends
          SYNs until receipt of further instructions.
    [5] Idle - In BOP and COP modes, this bit selects the transmitter output during idle. Idle is defined as the state following a normal end of message until
        receipt of the next transmitter command.
     0 - Idle in marking condition.
     1 - Idle sending SYNs (COP) or FLAGs (BOP).
    [4] Transmit EOM on Zero Count or Done - In BOP and COP modes, the assertion of this bit causes the end of message (FCS in COP, FCS-FLAG in BOP) to be transmitted
        upon the following events:
        1. If the counterltimer is counting transmitted characters, after transmission of the character which causes the counter to reach zero count. (DONEN is also asserted
           as an output if the channel is in a DMA operation.)
        2. If the channel is operating in DMA mode, after transmission of a character if DONEN was asserted when that character was loaded into the TX FIFO by the DMA controller.

    ASYNC mode
    [7:4] Stop Bits per Character - In ASYNC mode, this field programs the length of the stop bit appended to the transmitted character
        Stop bit lengths of 9/16 to 1 and 1-9/16 to 2 bits, in increments of 1/16 bit, can be programmed for character lengths of 6, 7, and 8 bits.
        For a character length of 5 bits, 1-1/16 to 2 stop bits can be programmed in increments of 1/16 bit. The receiver only checks for a 'mark'
        condition at the center of the first stop bit position (one bit time after the last data bit, or after the parity bit if parity is enabled) in all cases.
        If an external 1 X clock is used for the transmitter, [7) = 0 selects one stop bit and [7) = 1 selects two stop bits to be transmitted.
        If Manchester, NRZI, or FM data encoding is selected, only integral stop bit lengths should be used.
    [3] Transmitter Request-to-Send Control - This bit controls the deactivation of the RTS_N output by the transmitter
     0 - RTS_N is not affected by status of transmitter.
     1 - RTS_N changes state as a function of transmitter status.
    [2] Clear-ta-Send Enable Transmitter - The state of this bit determines if the CTS N input controls the operation of the channels transmitter
        The duration of CTS level change is described in the discussion of ICTSR[4).
     0 - CTS_N has no affect on the transmitter.
     1 - CTS_N affects the state of the transmitter.
    [1:0] Transmitted Bits per Character - This field selects the number of data bits per character to be transmitted. The character length does not
          include the start, parity, and stop bits in ASYNC or the parity bit in COP. In BOP modes the character length for the address and control
          fields is always 8 bits, and the value of this field only applies to the information (I) field, except for the last character of the I field,
          whose length is specified by OMR[7:5).
*/
void duscc_channel::do_dusccreg_tpr_w(UINT8 data)
{
	LOG(("%s(%02x) Setting up Transmit Parameters\n", FUNCNAME, data));
	m_tpr = data;
	LOG(("- RTS %u\n", (m_tpr & REG_TPR_RTS) ? 1 : 0));
	LOG(("- CTS %u\n", (m_tpr & REG_TPR_CTS) ? 1 : 0));
	LOG(("- Stop Bits %s\n", stop_bits_tostring(get_stop_bits())));
	LOG(("- Data Tx bits %u\n", get_tx_word_length()));

	update_serial();
	return;
}

/* Transmitter Timing Register (TTRA, TTRB)
    [7] External Source - This bit selects the RTxC pin or the TRxC pin of the channel as the transmitter clock input when [6:4] specifies
        external. When used for input, the selected pin must be programmed as an input in the PCR [4:3] or [2:0].
     0 External input form RTxC pin.
     1 External input from TRxC pin.
    [6:4] Transmitter Clock Select - This field selects the clock for the transmitter.
     000 External clock from TRxC or RTXC at 1 X the shift (baud) rate.
     001 External clock from TRXC or RTxC at 16X the shift rate.
     010 Internal clock from the phase-locked loop at IX the bit rate. It should be used only in half-duplex operation since the
         DPLL will periodically resync itself to the received data if in full-duplex operation.
     0ll Internal clock from the bit rate generator at 32X the shift rate. The clock signal is divided by two before use in the
         transmitter which operates at 16X the baud rate. Rate selected by [3:0].
     100 Internal clock from counter/timer of other channel. The C/T should be programmed to produce a clock at 2X the shift rate.
     101 Internal clock from counter/timer of other channel. The C/T should be programmed to produce a clock at 32X the shift rate.
     110 Internal clock from the counter/timer of own channel. The C/T should be programmed to produce a clock at 2X the shift rate.
     111 Internal clock from the counter/timer of own channel. The C/T should be programmed to produce a clock at 32X the shift rate.
    [3:0] Bit Rate Select - This field selects an output from the bit rate generator to be used by the transmitter circuits. The actual
          frequency output from the BRG is 32X the bit rate shown in Table 5. With a crystal or external clock of 14.7456MHz the bit rates are as
          given in Table 5 (this input is divided by two before being applied to the oscillator circuit).

          Table 5. Receiver/Transmitter Baud Rates
          [3:0] BIT RATE    [3:0]   BIT RATE
          0000  50          1000    1050
          0001  75          1001    1200
          0010  110         1010    2000
          0011  134.5       1011    2400
          0100  150         1100    4800
          0101  200         1101    9600
          0110  300         1110    19.2K
          0111  600         1111    38.4K
*/
void duscc_channel::do_dusccreg_ttr_w(UINT8 data)
{
	LOG(("%s(%02x) Setting up Transmit Timing\n", FUNCNAME, data));
	m_ttr = data;
	LOG(("- External source: %s\n", (m_ttr & REG_TTR_EXT) ? "TRxC" : "RTxC"));
	LOG(("- Transmit Clock: "));
#if VERBOSE > 0
	switch(m_ttr & REG_TTR_TXCLK_MASK)
	{
	case REG_TTR_TXCLK_1XEXT:       LOG(("1x External - not implemented\n")); break;
	case REG_TTR_TXCLK_16XEXT:      LOG(("16x External - not implemented\n")); break;
	case REG_TTR_TXCLK_DPLL:        LOG(("DPLL - not implemented\n")); break;
	case REG_TTR_TXCLK_BRG:
		LOG(("BRG\n"));
		m_brg_tx_rate = get_baudrate(m_ttr & REG_TTR_BRG_RATE_MASK);
		break;
	case REG_TTR_TXCLK_2X_OTHER:    LOG(("2x other channel C/T - not implemented\n")); break;
	case REG_TTR_TXCLK_32X_OTHER:   LOG(("32x other channel C/T - not implemented\n")); break;
	case REG_TTR_TXCLK_2X_OWN:      LOG(("2x own channel C/T - not implemented\n")); break;
	case REG_TTR_TXCLK_32X_OWN:     LOG(("32x own channel C/T - not implemented\n")); break;
	default: LOG(("Wrong programming\n")); break; // Should never happen
	}
#endif

	LOG(("- BRG Tx rate %u assuming a 14.7456MHz CLK crystal\n", get_baudrate(m_ttr & REG_TTR_BRG_RATE_MASK)));
	update_serial();

	return;
}

/* Receiver Parameter Resgister (RPRA, RPRB)
    [7] SYN Stripping - This bit controls the DUSCC processing in COP modes of SYN 'character patterns' that occur after the initial
       character synchronization. Refer to Detailed Operation of the receiver for details and definition of SYN 'patterns', and their
       accumulation of FCS.
     0 Strip only leading SYN 'patterns' (i.e. before a message).
     1 Strip all SYN 'patterns' (including all odd DLE's in BISYNC transparent mode).

    [6] Transfer Received FCS to FIFO - In BISYNC and BOP modes, the assertion of this bit causes the received FCS to be loaded into the
       RxFIFO. When this bit is set, BOP mode operates correctly only if a minimum of two extra FLAGs (without shared zeros) are appended
       to the frame. If the FCS is specified to be transferred to the FI FO, the EOM status bit will be tagged onto the last byte of the
       FCS instead of to the last character of the message.
     0 Do not transfer FCS to RxFIFO.
     1 Transfer FCS to RxFIFO.

    [5] Auto-Hunt and Pad Check (BISYNC) -In BISYNC rnode, the assertion of this bit causes the receiver to go into hunt for character
        sync mode after detecting certain End-Ol-Message (EOM) characters. These are defined in the Detailed Operations section for
        COP receiver operation. After the EOT and NAK sequences, the receiver also does a check for a closing PAD of four 1 s.
     0 Disable auto-hunt and PAD check.
     1 Enable auto-hunt and PAD check.
     [5] Overrun Mode (BOP) - The state of this control bit deterrnines the operation of the receiver in the event of a data overrun, i.e.,
         when a character is received while the RxFIFO and the Rx shift register are both full.
     0 The receiver terrninates receiving the current frame and goes into hunt phase, looking for a FLAG to be received.
     1 The receiver continues receiving the current frame. The overrunning character is lost. (The five characters already
       assembled in the RxFIFO and Rx shift register are protected).

    [4] Receiver Request-to-Send Control (ASYNC)
     0 Receiver does not control RTSN output.
     1 Receiver can negate RTSN output.
    [4] External Sync (COP) - In COP single SYN mode, the assertion of this bit enables external character synchronization and
        receipt of SYN patterns is not required. In order to use this feature, the DUSCC must be programmed to COP single SYN mode,
        CMR1[2:0] = 110, which is used to set up the internal data paths. In all other respects, however, the external sync mode operation is
        protocol transparent. A negative signal on the DCDN/SYNIN pin will cause the receiver to establish synchronization on the next rising
        edge of the receiver clock. Character assembly will start at this edge with the RxD input pin considered to have the second bit of
        data. The sync signal can then be negated. Receipt of the Active-High external sync input causes the SYN detect status bit
        (RSR[2]) to be set and the SYNBOUTN pin to be asserted for one bit time. When this mode is enable, the internal SYN (COP mode)
        detection and special character recognition (e.g., IDLE, STX, ETX, etc.) circuits are disabled. Character assembly begins as ~ in the
        I-field with character length as programmed in RPR[I :)]. Incoming COP frames with parity specified optionally can have it stripped by
        programming RPR[3J. The user must wait at least eight bit times after Rx is enabled before applying the SYNIN signal. This time is
        required to flush the internal data paths. The receiver remains in this mode and further external sync pulses are ignored until the
        receiver is disabled and then reenabled to resynchronize or to return to normal mode.
     0 External sync not enabled.
     1 External sync enabled.
     Note that EXT SYNC and DCD ENABLE Rx cannot be asserted simultaneously since they use the same pin.

    [3] Strip Parity - In COP and ASYNC modes with parity enabled, this bit controls whether the received parity bit is stripped from the
        data placed in the receiver FIFO. It is valid ony for programmed character lengths of 5, 6, and 7 bits. If the bit is stripped, the
        corresponding bit in the received data is set to zero.
     0 Transfer parity bit as received.
     1 Stop parity bit from data.
    [3] All Parties Address - In BOP secondary modes, the assertion of this bit causes the receiver to 'wake-up' upon receipt of the
        address H'FF' or H'FF, FF', for single- and dual-octet address modes, respectively, in addition to its normal station address. This
        feature allows all stations to receive a message.
     0 Don't recognize all parties address.
     1 Recognize all parties address.

    [2] DCD Enable Receiver - If this bit is asserted, the DCDN/SYNIN input must be Low in order for the receiver to operate.
        If the input is negated (goes High) while a character is being received, the receiver terminates receipt of the current message
        (this action in effect disables the receiver). If DCD is subsequently asserted, the receiver will search for the start bit, SYN pattern, or
        FLAG, depending on the channel protocol. (Note that the change of input can be programmed to generate an interrupt; the duration of
        the DCD level change is described in the discussion of the input and counter/timer status register (CTSR[5]).
     0 DCD not used to enabled receiver.
     1 DCD used to enabled receiver.
     NOTE that EXT SYNC and DCD ENABLE Rx cannot be asserted simultaneously since they use the same pin.

    [1:0] Received Bits per Character - This field selects the number of data bits per character to be assembled by the receiver. The
    character length does not include the start, parity, and stop bits in the ASYNC or the parity bit in COP. In BOP modes, the character
    length for the address and control field is always 8 bits, and the value of this field only applies to the information field. lithe number
    of bits assembled for the last character of the l-field is less than the value programmed in this field, RCL not zero (RSR[O]) is asserted
    and the actual number of bits received is given in TRSR[2:0].
*/
void duscc_channel::do_dusccreg_rpr_w(UINT8 data)
{
	LOG(("%s(%02x) Setting up Receiver Parameters\n", FUNCNAME, data));
	m_rpr = data;
	LOG(("- RTS output %u\n", (m_rpr & REG_RPR_RTS) ? 1 : 0));
	LOG(("- Strip Parity %u\n", (m_rpr & REG_RPR_STRIP_PARITY && get_rx_word_length() < 8) ? 1 : 0));
	LOG(("- DCD/SYNIN input %u\n", (m_rpr & REG_RPR_DCD) ? 1 : 0));
	LOG(("- Data Rx bits %u\n", get_rx_word_length()));

	update_serial();
	return;
}

/* Receiver Timing Register (RTRA, RTRB)
    [7] External Source - This M selects the RTxC pin or the TRxC pin of the channel as the receiver or DPLL clock input, when [6:4J
        specifies external. When used for input, the selected pin must be programmed as an input in the PCR [4:3] or [2:0].
     0 External input form RTxC pin.
     1 External input form TRxC pin.
    [6:4] Receiver Clock Select- This field selects the clock for the receiver.
     000 External clock from TRxC or RTxC at 1 X the shift (baud) rate.
     001 External clock fromTRxC or RTxC at 16X the shift rate. Used for ASYNC mode only.
     010 Internal clock from the bit rate generator at 32X the shift rate. Clock is divided by two before used by the receiver
         logic, which operates at 16X the baud rate. Rate selected
         by [3:0J. Used for ASYNC mode only.
     011 Internal clock from counter/timer of own channel. The CIT should be programmed to produce a clock at 32X the shift
         rate. Clock is divided by two before use in the receiver logic. Used for ASYNC mode only.
     100 Internal clock from the digital phase- locked loop. The clock for the DPLL is a 64X clock from the crystal oscillator or
         system clock input. (The input to the oscillator is divided by two).
     101 Internal clock from the digital phase- locked loop. The clock for the DPLL is an external 32X clock from the RTxC or
         TRxC pin, as selected by [7J.
     110 Internal clock from the digital phase- locked loop. The clock for the DPLL is a 32X clock from the BRG. The frequency
         is programmed by [3:0].
     111 Internal clock from the digital phase- locked loop. The clock for the DPLL is a 32X clock from the counter/timer of the
         channel.
    [3:0] Bit Rate Select- This field selects an output from the bit rate generator to be used by the receiver circuits. The actual frequency
          output from the BRG is 32X the bit rate shown in Table 5.*/

void duscc_channel::do_dusccreg_rtr_w(UINT8 data)
{
	LOG(("%s(%02x) Setting up Receiver Timing\n", FUNCNAME, data));
	m_rtr = data;
	LOG(("- External source: %s\n", (m_rtr & REG_RTR_EXT) ? "TRxC" : "RTxC"));
	LOG(("- Receiver Clock: "));

#if VERBOSE > 0
	switch(m_rtr & REG_RTR_RXCLK_MASK)
	{
	case REG_RTR_RXCLK_1XEXT:       LOG(("1x External - not implemented\n")); break;
	case REG_RTR_RXCLK_16XEXT:      LOG(("16x External - not implemented\n")); break;
	case REG_RTR_RXCLK_BRG:
		LOG(("BRG\n"));
		m_brg_rx_rate = get_baudrate(m_rtr & REG_RTR_BRG_RATE_MASK);
		break;
	case REG_RTR_RXCLK_CT:          LOG(("C/T of channel - not implemented\n")); break;
	case REG_RTR_RXCLK_DPLL_64X_X1: LOG(("DPLL, source = 64X X1/CLK - not implemented\n")); break;
	case REG_RTR_RXCLK_DPLL_32X_EXT:LOG(("DPLL, source = 32X External - not implemented\n")); break;
	case REG_RTR_RXCLK_DPLL_32X_BRG:LOG(("DPLL, source = 32X BRG - not implemented\n")); break;
	case REG_RTR_RXCLK_DPLL_32X_CT: LOG(("DPLL, source = 32X C/T - not implemented\n")); break;
	default: LOG(("Wrong programming\n")); break; // Should never happen
	}
#endif

	LOG(("- BRG Rx rate %u assuming a 14.7456MHz CLK crystal\n", get_baudrate(m_rtr & REG_RTR_BRG_RATE_MASK)));
	update_serial();

	return;
}

void duscc_channel::do_dusccreg_ctprh_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ctprl_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ctcr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }

/* Output and Miscellaneous Register (OMRA, OMRB)
    [7:5] Transmitted Residual Character Length - In BOP modes, this field determines the number of bits transmitted for the last
          character in the information field. This length applies to:
          - The character in the transmit FIFO accompanied by the FIFOed TEOM command.
          - The character loaded into the FIFO by the DMA controller if DONEN is simultaneously asserted and TPR(4) is asserted.
          - The character loaded into the FIFO which causes the counter to reach zero count when TPR[4J is asserted.
          The length of all other characters in the frame's information field is selected by TPR[I :OJ. If this field is 111,
          the number of bits in the last character is the same as programmed in TPR[1:0].
    [4] TxRDY Activate Mode -
     0 FIFO not full. The channel's TxRDY status bit is asserted each time a character is transferred from the transmit FIFO
       to the transmit shift register. If not reset by the CPU, TxRDY remains asserted until the FIFO is full, at which time
       it is automatically negated.
     1 FIFO empty. The channel's TxRDY status bit is asserted when a character transfer from the transmit FIFO to the
       transmit shift register causes the FI FO to become empty. If not reset by the CPU, TxRDY remains asserted until the
       FIFO is full, at which time it is negated.
     If the TxRDY status bit is reset by the CPU, it will remain negated regardless of the current state of the transmit
     FIFO, until it is asserted again due to the occurrence of one of the above conditions.
    [3] RxRDY Activate Mode -
     0 FIFO not empty. The channel's RxRDY status bit is asserted each time a character is transferred from the
       receive shift register to the receive FIFO. If not reset by the CPU, RxRDY remains asserted until the receive FIFO is
       empty, at which time it is automatically negated.
     1 FIFO full. The channel's RxRDY status bit is asserted when a character transfer from the receive shift register to the
       receive FIFO causes the FIFO to become full. If not reset by the CPU, RxRDY reamins asserted until the FIFO is empty,
       at which time it is negated.
     The RxRDY status bit will also be asserted, regardless of the receiver FIFO full condition, when an end-of-message
     character is loaded in the RxFIFO (BOP/BISYNC), when a BREAK condition (ASYNC mode) is detected in RSR[2), or
     when the counterltimer is programmed to count received characters and the character which causes it to reach zero
     is loaded in the FIFO (all modes). If reset by the CPU, the RxRDY status bit will remain negated, regardless of the
     current state of the receiver FIFO, until it is asserted again due to one of the above conditions.
    [2] General Purpose Output 2 -
     This general purpose bit is used to control the TxDRQN/GP02lRTSN pin, when it is used as an output. The output is
     High when the bit is a 0 and is Low when the bit is a 1.
    [1] General Purpose Output 1 - This bit is used to control the RTxDRQN/GPOl N output, which is a general purpose output
     when the channel is not in DMA mode. The output is High when the bit is a 0 and is Low when the bit is a 1.
    [0] Request-to-Send Output - This bit controls the TxDRQN/GP02N/RTSN and SYNOUTN/RTSN pin, when either is
     used as a RTS output. The output is High when the bit is a 0 and is Low when the bit is a 1.
*/
void duscc_channel::do_dusccreg_omr_w(UINT8 data)
{
	LOG(("%s(%02x) Output and Miscellaneous Register\n", FUNCNAME, data));
	m_omr = data;
	LOG(("- Tx Residual Character Length is "));
	if ((m_omr & REG_OMR_TXRCL_MASK) == REG_OMR_TXRCL_8BIT)
		LOG(("determined by TPR[1:0], the Transmitter Parameter Register\n"));
	else
		LOG(("%u bits\n", (((m_omr & REG_OMR_TXRCL_MASK) >> 5) & 0x07) + 1));
	LOG(("- TxRDY activated by %s\n", m_omr & REG_OMR_TXRDY_ACTIVATED ? "FIFO empty" : "FIFO not full"));
	LOG(("- RxRDY activated by %s\n", m_omr & REG_OMR_RXRDY_ACTIVATED ? "FIFO full"  : "FIFO not empty"));
	LOG(("- GP02, if configured as output, is: %u\n", m_omr & REG_OMR_GP02 ? 0 : 1));
	LOG(("- GP01, if configured as output, is: %u\n", m_omr & REG_OMR_GP01 ? 0 : 1));
	LOG(("- RTS, either pin if configured as output, is: %u\n", m_omr & REG_OMR_RTS  ? 0 : 1));
	return;
}

/* Pin Configuration Register (PCRA, PCRB)
    This register selects the functions for multipurpose 1/0 pins.
    [7] X2IIDC - This bit is defined only for PCRA. It is not used in PCRB.
     0 The X2/IDCN pin is used as a crystal connection.
     1 The X2/IDCN pin is the interrupt daisy chain output.
    [6] GP02/RTS - The function of this pin is programmable only when not operating in full-duplex DMA mode.
     0 The TxDRQN/GP02N/RTSN pin is a general purpose output. It is Low when OMR[2] is a 1 and High when OMR[2] is a O.
     1 The pin is a request-to-send output The logical stale of the pin is controlled by OMR[O]. When OMR[O] is set, the output is Low.
    [5] SYNOUT/RTS -
     0 The SYNOUTN/RTSN pin is an active-Low output which is asserted one bit time after a SYN pattern (COP modes) in HSRH/HSRL or FLAG
       (BOP modes) is detected in CCSR.The output remains asserted for one receiver clock period.
     1 The pin is a request-to-send output The,logical state of the pin Is controlled by OMR[O] when OMR[O] is set, the output is Low.
    [4:3] RTxC-
     00 The pin is an input. It must be programmed for input when used as the input for the receiver or transmitter clock, the DPLL, or the CIT.
     01 The pin is an output for the counterltimer.
     10 The pin is an output for the transmitter shift register clock.
     11 The pin is an output for the receiver shift register clock.
    [2:0]TRxC-
     000 The pin is an input. It must be programmed for input when used as the input for the receiver or transmitter clock, the DPLL, or the CIT.
     001 The pin is an output from the crystal oscillator divided by two.
     010 The pin is an outputfor the DPLL output clock.
     011 The pin is an output for the counterltimer. Refer to CTCRAIB description.
     100 The pin is an output for the transmitter BRG at 16X the rate selected by TTR [3:0].
     101 The pin is an output for the receiver BRG at 16X the rate selected by RTR [3:0].
     110 The pin is an output for the transmitter shift register clock.
     111 The pin is an output for the receiver shift register clock.
*/
void duscc_channel::do_dusccreg_pcr_w(UINT8 data)
{
	LOG(("%s(%02x) Pin Configuration Register\n", FUNCNAME, data));
	m_pcr = data;
	LOG(("- The X2/IDCN pin is %s\n", m_index == duscc_device::CHANNEL_B ? "ignored for channel B" :
			((m_pcr & REG_PCR_X2_IDC) ? "crystal input" : "daisy chain interrupt output")));
	LOG(("- The GP02/RTS pin is %s\n", m_pcr & REG_PCR_GP02_RTS ?  "RTS" : "GP02"));
	LOG(("- The SYNOUT/RTS pin is %s\n", m_pcr & REG_PCR_SYNOUT_RTS ? "RTS" : "SYNOUT"));

#if VERBOSE > 0
	LOG(("- The RTxC pin is "));
	switch ( m_pcr & REG_PCR_RTXC_MASK )
	{
	case REG_PCR_RTXC_INPUT:    LOG(("- an input\n")); break;
	case REG_PCR_RTXC_CNTR_OUT: LOG(("- a counter/timer output\n")); break;
	case REG_PCR_RTXC_TXCLK_OUT:LOG(("- a Tx clock output\n")); break;
	case REG_PCR_RTXC_RXCLK_OUT:LOG(("- a Rx clock output\n")); break;
	default: LOG(("Wrong programming\n")); break; // Should never happen
	}
	LOG(("- The TRxC pin is "));
	switch( m_pcr & REG_PCR_TRXC_MASK )
	{
	case REG_PCR_TRXC_INPUT:    LOG(("- an input\n")); break;
	case REG_PCR_TRXC_CRYST_OUT:LOG(("- a crystal/2 output\n")); break;
	case REG_PCR_TRXC_DPLL_OUT: LOG(("- a DPLL output\n")); break;
	case REG_PCR_TRXC_CNTR_OUT: LOG(("- a counter/timer output\n")); break;
	case REG_PCR_TRXC_TXBRG_OUT:LOG(("- a Tx BRG output\n")); break;
	case REG_PCR_TRXC_RXBRG_OUT:LOG(("- a Rx BRG output\n")); break;
	case REG_PCR_TRXC_TXCLK_OUT:LOG(("- a Tx CLK output\n")); break;
	case REG_PCR_TRXC_RXCLK_OUT:LOG(("- a Rx CLK output\n")); break;
	default: LOG(("Wrong programming\n")); break; // Should never happen
	}

#endif

	return;
}

/*
 * Commands to the DUSCC are entered through the CCR channel command register.
 *
 * TODO:
 * - support enable/disable of Tx/Rx using m_tra/m_rcv respectivelly
 */
void duscc_channel::do_dusccreg_ccr_w(UINT8 data)
{
	m_ccr = data;
	LOG(("%s\n", FUNCNAME));
	switch(m_ccr)
	{
	// TRANSMITTER COMMANDS

	/* Reset transmitter. Causes the transmitter to cease operation immediately.
	   The transmit FIFO is cleared and the TxD output goes into the marking state.
	   Also clears the transmitter status bits (TRSR[7:4]) and resets the TxRDY
	   status bit (GSR[I] or GSR[5] for Channels A and B, respectively).
	   The counter/timer and other registers are not affected*/
	case REG_CCR_RESET_TX: LOG(("- Reset Tx\n"));
		set_tra_rate(0);
		m_tx_fifo_wp = m_tx_fifo_rp = 0;
		m_trsr &= 0x0f;
		m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
		break;

	/* Enable transmitter. Enables transmitter operation, conditioned by the state of
	   the CTS ENABLE Tx bit, TPR[2]. Has no effect if invoked when the transmitter has
	   previously been enabled.*/
	case REG_CCR_ENABLE_TX: LOG(("- Enable Tx\n"));
		m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
		m_tra = 1;
		set_tra_rate(m_brg_tx_rate);
		break;

	/* Disable transmitter. Terminates transmitter operation and places the TXD output in the
	   marking state at the next occurrence of a transmit FIFO empty condition. All characters
	   currently in the FIFO, or any loaded subsequently prior to attaining an empty condition,
	   will be transmitted.
	   TODO: let all the chararcters be transmitted before shutting down shifter */
	case REG_CCR_DISABLE_TX: LOG(("- Disable Tx\n"));
		set_tra_rate(0);
		m_tra = 0;
		m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
		break;

	// RECEIVER COMMANDS

	/* Reset Receiver. Causes the receiver to cease operation, clears the receiver FIFO,
	   clears the data path, and clears the receiver status (RSR[7:0], TRSR[3:0], and either
	   GSR[O] or GSR[4] for Channels A and B, respectively). The counter/timer and other
	   registers are not affected.*/
	case REG_CCR_RESET_RX: LOG(("- Reset Rx\n"));
		set_rcv_rate(0);
		m_rx_fifo_wp = m_rx_fifo_rp = 0;
		m_trsr &= 0xf0;
		m_rsr = 0;
		m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);
		break;

	/* Enable receiver. Causes receiver operation to begin, conditioned by the state of the DCD
	  ENABLED Rx bit, RPR[2]. Receiver goes into START, SYN, or FLAG search mode depending on
	  channel protocol mode. Has no effect if invoked when the receiver has previously been enabled.*/
	case REG_CCR_ENABLE_RX: LOG(("- Enable Rx\n"));
		m_rcv = 1;
		set_rcv_rate(m_brg_rx_rate);
		//m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);
		break;

	/* Disable receiver. Terminates operation of the receiver. Any character currently being assembled
	   will be lost. Does not affect FIFO or any status.*/
	case REG_CCR_DISABLE_RX: LOG(("- Disable Rx\n"));
		m_rcv = 0;
		m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);
		break;
	default: LOG((" - command %02x not implemented yet\n", data));
	}
	return;
}

void duscc_channel::do_dusccreg_txfifo_w(UINT8 data)
{
	LOG(("%s(%02x)'%c'\n", FUNCNAME,data, isalnum(data) ? data : ' '));
	LOG((" - TX wp:%d rp:%d sz:%d\n", m_tx_fifo_wp, m_tx_fifo_rp, m_tx_fifo_sz));

	/* Tx FIFO is full or...? */
	if (m_tx_fifo_wp + 1 == m_tx_fifo_rp || ( (m_tx_fifo_wp + 1 == m_tx_fifo_sz) && (m_tx_fifo_rp == 0) ))
	{
		logerror("- TX FIFO is full, discarding data\n");
		LOG(("- TX FIFO is full, discarding data\n"));
	}
	else // ..there is still room
	{
		m_tx_data_fifo[m_tx_fifo_wp++] = data;
		//m_rsr &= ~REG_RSR_OVERRUN_ERROR;
		//m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);
		if (m_tx_fifo_wp >= m_tx_fifo_sz)
		{
			m_tx_fifo_wp = 0;
		}
	}

	/* Transmitter enabled?  */
	if ( m_tra == 1 )
	{
		if ( is_transmit_register_empty()) // Is the shift register loaded?
		{
			LOG(("- Setting up transmitter\n"));
			transmit_register_setup(m_tx_data_fifo[m_tx_fifo_rp]); // Load the shift register, reload is done in tra_complete()
			m_tx_fifo_rp_step();
		}
	}
	// check if Tx FIFO is FULL and set TxREADY accordingly
	if (m_tx_fifo_wp + 1 == m_tx_fifo_rp || ( (m_tx_fifo_wp + 1 == m_tx_fifo_sz) && (m_tx_fifo_rp == 0) ))
	{
		m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
	}
	else
	{
		m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
	}

	return;
}

void duscc_channel::do_dusccreg_rsr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_trsr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ictsr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_gsr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ier_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ivr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_icr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }

/* Short cutted non complex features */
//void duscc_channel::do_dusccreg_rea_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
//void duscc_channel::do_dusccreg_sea_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }

void duscc_channel::do_dusccreg_mrr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ier1_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ier2_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ier3_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_trcr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_ftlr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }
void duscc_channel::do_dusccreg_trmsr_w(UINT8 data){ logerror("register access method %s is not implemented yet\n", FUNCNAME); return; }

//-------------------------------------------------
//  control_read - read register
//-------------------------------------------------
UINT8 duscc_channel::read(offs_t &offset)
{
	UINT8 data = 0;
	int reg = offset | m_a7;

	switch (reg)
	{
	case REG_CID:       data = do_dusccreg_cid_r(); break;
	case REG_CCR:       data = do_dusccreg_ccr_r(); break;
	case REG_RXFIFO_0:  data = do_dusccreg_rxfifo_r(); break;
	case REG_RXFIFO_1:  data = do_dusccreg_rxfifo_r(); break;
	case REG_RXFIFO_2:  data = do_dusccreg_rxfifo_r(); break;
	case REG_RXFIFO_3:  data = do_dusccreg_rxfifo_r(); break;
	case REG_GSR:       data = do_dusccreg_gsr_r(); break;
	default:
		logerror(" \"%s\" %s: %c : Unsupported RRx register:%02x\n", m_owner->tag(), FUNCNAME, 'A' + m_index, reg);
	}

	//LOG(("%s \"%s\": %c : Register R%d read '%02x'\n", FUNCNAME, m_owner->tag(), 'A' + m_index, reg, data));
	return data;
}

//-------------------------------------------------
//  write - write register
//-------------------------------------------------

void duscc_channel::write(UINT8 data, offs_t &offset)
//WRITE8_MEMBER( duscc_channel::write)
{
	int reg = offset | m_a7;

	LOG(("\"%s\" %s: %c : Register write '%02x' -> [%02x]", m_owner->tag(), FUNCNAME, 'A' + m_index, data, reg ));
	switch (reg)
	{
	case REG_SEA: /*Also REG_REA depending on which channel is written to */
		if ( m_uart->m_variant != SET_CMOS )
		{
			logerror("Attempt set/clear the CDUSCC A7 bit on an NDUSCC\n");
			m_a7 = 0;
		}
		else
			m_a7 = (m_index == duscc_device::CHANNEL_A ? 0x40 : 0);
		break;
	case REG_CMR1:      do_dusccreg_cmr1_w(data); break;
	case REG_CMR2:      do_dusccreg_cmr2_w(data); break;
	case REG_S1R:       LOG(("REG_S1R \n")); break;
	case REG_S2R:       LOG(("REG_S2R \n")); break;
	case REG_TPR:       do_dusccreg_tpr_w(data); break;
	case REG_TTR:       do_dusccreg_ttr_w(data); break;
	case REG_RPR:       do_dusccreg_rpr_w(data); break;
	case REG_RTR:       do_dusccreg_rtr_w(data); break;
	case REG_CTPRH:     LOG(("REG_CTPRH\n")); break;
	case REG_CTPRL:     LOG(("REG_CTPRL\n")); break;
	case REG_CTCR:      LOG(("REG_CTCR\n")); break;
	case REG_OMR:       do_dusccreg_omr_w(data); break;
	case REG_CTH:       LOG(("REG_CTH   \n")); break;
	case REG_CTL:       LOG(("REG_CTL   \n")); break;
	case REG_PCR:       do_dusccreg_pcr_w(data); break;
	case REG_CCR:       do_dusccreg_ccr_w(data); break;
	case REG_TXFIFO_0:  do_dusccreg_txfifo_w(data); break;
	case REG_TXFIFO_1:  do_dusccreg_txfifo_w(data); break;
	case REG_TXFIFO_2:  do_dusccreg_txfifo_w(data); break;
	case REG_TXFIFO_3:  do_dusccreg_txfifo_w(data); break;
	case REG_RSR:       LOG(("REG_RSR   \n")); break;
	case REG_TRSR:      LOG(("REG_TRSR\n")); break;
	case REG_ICTSR:     LOG(("REG_ICTSR\n")); break;
	case REG_GSR:       LOG(("REG_GSR   \n")); break;
	case REG_IER:       LOG(("REG_IER   \n")); break;
//  case REG_CID:       LOG(("REG_CID   \n")); break;
	case REG_IVR:       LOG(("REG_IVR   \n")); break;
	case REG_ICR:       LOG(("REG_ICR   \n")); break;
//  case REG_SEA:       LOG(("REG_SEA   \n")); break;
//  case REG_IVRM:      LOG(("REG_IVRM\n")); break;
//  case REG_MRR:       LOG(("REG_MRR   \n")); break;
	case REG_IER1:      LOG(("REG_IER1\n")); break;
	case REG_IER2:      LOG(("REG_IER2\n")); break;
	case REG_IER3:      LOG(("REG_IER3\n")); break;
	case REG_TRCR:      LOG(("REG_TRCR\n")); break;
	case REG_RFLR:      LOG(("REG_RFLR\n")); break;
	case REG_FTLR:      LOG(("REG_FTLR\n")); break;
	case REG_TRMSR:     LOG(("REG_TRMSR\n")); break;
	case REG_TELR:      LOG(("REG_TELR\n")); break;

	default:
		logerror(" \"%s\" %s: %c : Unsupported WRx register:%02x(%02x)\n", m_owner->tag(), FUNCNAME, 'A' + m_index, reg, data);
	}
}

/* Get data from top of fifo data but restore read pointer in case of exit latch lock */
UINT8 duscc_channel::m_rx_fifo_rp_data()
{
		UINT8 data;
		UINT8 old_rp = m_rx_fifo_rp;
		m_rx_fifo_rp_step();
		data = m_rx_data_fifo[m_rx_fifo_rp];
		m_rx_fifo_rp = old_rp;

		return data;
}

/* Step read pointer */
void duscc_channel::m_rx_fifo_rp_step()
{
		m_rx_fifo_rp++;
		if (m_rx_fifo_rp >= m_rx_fifo_sz)
		{
				m_rx_fifo_rp = 0;
		}

		// check if FIFO is empty
		if (m_rx_fifo_rp == m_rx_fifo_wp)
		{
				// no more characters available in the FIFO
			//              m_rr0 &= ~ RR0_RX_CHAR_AVAILABLE;
			m_gsr &= ~(m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);
		}
}

/* Step TX read pointer */
void duscc_channel::m_tx_fifo_rp_step()
{
		m_tx_fifo_rp++;
		if (m_tx_fifo_rp >= m_tx_fifo_sz)
		{
				m_tx_fifo_rp = 0;
		}
}

//-------------------------------------------------
//  receive_data - receive data word into fifo
//-------------------------------------------------

void duscc_channel::receive_data(UINT8 data)
{
	LOG(("\"%s\": %c : Receive Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, data));
#
	if (m_rx_fifo_wp + 1 == m_rx_fifo_rp || ( (m_rx_fifo_wp + 1 == m_rx_fifo_sz) && (m_rx_fifo_rp == 0) ))
	{
		// receive overrun error detected
		m_rsr |= REG_RSR_OVERRUN_ERROR;
		logerror("Receive_data() Error %02x\n", m_rsr);
	}
	else
	{
		m_rx_data_fifo[m_rx_fifo_wp] = data;
		m_rx_error_fifo[m_rx_fifo_wp] &= ~REG_RSR_OVERRUN_ERROR;
		m_rsr &= ~REG_RSR_OVERRUN_ERROR;
		m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_RXREADY : REG_GSR_CHAN_B_RXREADY);

		m_rx_fifo_wp++;
		if (m_rx_fifo_wp >= m_rx_fifo_sz)
		{
			m_rx_fifo_wp = 0;
		}
	}
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------

WRITE_LINE_MEMBER( duscc_channel::cts_w )
{
	LOG(("\"%s\" %s: %c : CTS %u\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));

	if (m_cts != state)
	{
		// enable transmitter if in auto enables mode
		if (!state)
		{
			m_ictsr |= REG_ICTSR_DELTA_CTS;
		}
		else
		{
			m_ictsr &= ~REG_ICTSR_DELTA_CTS;
		}

		if (m_tpr & REG_TPR_CTS && m_tra)
		{
			m_gsr |= (m_index == duscc_device::CHANNEL_A ? REG_GSR_CHAN_A_TXREADY : REG_GSR_CHAN_B_TXREADY);
		}

		// set clear to send
		m_cts = state;
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------
WRITE_LINE_MEMBER( duscc_channel::dcd_w )
{
	LOG(("\"%s\" %s: %c : DCD %u - not implemented\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));
#if 0

	if (m_dcd != state)
	{
		// enable receiver if in auto enables mode
		if (!state)
			if (reg & REG_AUTO_ENABLES)
			{
				reg |= REG_RX_ENABLE;
			}

		// set data carrier detect
		m_dcd = state;
	}
#endif
}

//-------------------------------------------------
//  ri_w - ring indicator handler
//-------------------------------------------------

WRITE_LINE_MEMBER( duscc_channel::ri_w )
{
	LOG(("\"%s\" %s: %c : RI %u - not implemented\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));
#if 0
	if (m_ri != state)
	{
		// set ring indicator state
		m_ri = state;
	}
#endif
}

//-------------------------------------------------
//  sync_w - sync handler
//-------------------------------------------------
WRITE_LINE_MEMBER( duscc_channel::sync_w )
{
	LOG(("\"%s\" %s: %c : SYNC %u - not implemented\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));
}

//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------
WRITE_LINE_MEMBER( duscc_channel::rxc_w )
{
	LOG(("\"%s\" %s: %c : RXC %u - not implemented\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));
}

//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------
WRITE_LINE_MEMBER( duscc_channel::txc_w )
{
	LOG(("\"%s\" %s: %c : TXC %u - not implemented\n", m_owner->tag(), FUNCNAME, 'A' + m_index, state));
}

//-------------------------------------------------
//  update_serial -
//-------------------------------------------------
void duscc_channel::update_serial()
{
	int data_bit_count = get_rx_word_length();
	stop_bits_t stop_bits = get_stop_bits();
	parity_t parity;

	if ((m_cmr1 & REG_CMR1_PMMODE_MASK) == REG_CMR1_PMMODE_PARITY)
	{
		if ( (m_cmr1 & REG_CMR1_PARITY) == 0)
			parity = PARITY_EVEN;
		else
			parity = PARITY_ODD;
	}
	else
		parity = PARITY_NONE;

	LOG((LLFORMAT " %s() \"%s \"Channel %c setting data frame %d+%d%c%d\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, 1,
			data_bit_count, parity == PARITY_NONE ? 'N' : parity == PARITY_EVEN ? 'E' : 'O', (stop_bits + 1) / 2));

	set_data_frame(1, data_bit_count, parity, stop_bits);

	int clocks = get_rx_clock_mode();

	if (m_rxc > 0)
	{
		set_rcv_rate(m_rxc / clocks);
				LOG(("   - Receiver clock: %d mode: %d rate: %d/%xh\n", m_rxc, clocks, m_rxc / clocks, m_rxc / clocks));
	}

	clocks = get_tx_clock_mode();
	if (m_txc > 0)
	{
		set_tra_rate(m_txc / clocks);
		LOG(("   - Transmit clock: %d mode: %d rate: %d/%xh\n", m_rxc, clocks, m_rxc / clocks, m_rxc / clocks));
	}

	if (m_brg_rx_rate != 0)
	{
		if (m_brg_rx_rate == 1) m_brg_rx_rate = 0; // BRG beeing disabled
		set_rcv_rate(m_brg_rx_rate);
		LOG(("   - Baud Rate Generator: %d mode: RX:%dx\n", m_brg_rx_rate, get_rx_clock_mode()));
	}
	if (m_brg_tx_rate != 0)
	{
		if (m_brg_tx_rate == 1) m_brg_tx_rate = 0; // BRG beeing disabled
		set_tra_rate(m_brg_tx_rate);
		LOG(("   - Baud Rate Generator: %d mode: TX:%dx\n", m_brg_tx_rate, get_tx_clock_mode()));
	}
}

//-------------------------------------------------
//  set_dtr -
//-------------------------------------------------
void duscc_channel::set_dtr(int state)
{
	LOG(("%s(%d)\n", FUNCNAME, state));
	m_dtr = state;

	if (m_index == duscc_device::CHANNEL_A)
		m_uart->m_out_dtra_cb(m_dtr);
	else
		m_uart->m_out_dtrb_cb(m_dtr);
}



//-------------------------------------------------
//  write_rx - called by terminal through rs232/diserial
//         when character is sent to board
//-------------------------------------------------
WRITE_LINE_MEMBER(duscc_channel::write_rx)
{
	m_rxd = state;
	//only use rx_w when self-clocked
	if(m_rxc != 0 || m_brg_rx_rate != 0)
		device_serial_interface::rx_w(state);
}
