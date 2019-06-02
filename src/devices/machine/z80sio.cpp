// license:BSD-3-Clause
// copyright-holders:Curt Coder, Joakim Larsson Edstrom
/***************************************************************************

    Z80-SIO Serial Input/Output emulation

    The variants in the SIO family are only different in the packaging
    but has the same register features. However, since some signals are
    not connected to the pins on the package or share a pin with another
    signal the functionality is limited. However, this driver does not
    check that an operation is invalid because of package type but relies
    on the software to be adapated for the particular version.

    Package:                DIP40  SIO/0, SIO/1, SIO/2, SIO/9
                            QFP44  SIO/3
                            PLCC44 SIO/4
    -------------------------------------------------------------------
    Channels / Full Duplex  2 / Y
    Synch data rates  2Mhz  500Kbps
                      4MHz  800Kbps
                      6MHz 1200Kbps
                     10MHz 2500Kbps
   -- Asynchrounous features -------------------------------------------
  * 5-8 bit per char         Y
  * 1,1.5,2 stop bits        Y
  * odd/even parity          Y
  * x1,x16,x32,x64           Y
    break det/gen            Y
  * parity, framing &        Y
      overrun error det      Y
    -- Byte oriented synchrounous features -------------------------------
    Int/ext char sync        Y
    1/2 synch chars          Y
    Aut synch char insertion Y
    Aut CRC gen/det          Y
    -- SDLC/HDLC capabilities --------------------------------------------
    Abort seq gen/chk        Y
    Aut zero ins/det         Y
    Aut flag insert          Y
    Addr field rec           Y
    1-fld resid hand         Y
    Valid rec msg protection Y
    --
  * Receiver FIFO            3
  * Transmitter FIFO         1
    -------------------------------------------------------------------------
    * = Features that has been implemented  n/a = features that will not
***************************************************************************/

#include "emu.h"
#include "z80sio.h"

#include "machine/sdlc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_INT     (1U <<  3)
#define LOG_CMD     (1U <<  4)
#define LOG_TX      (1U <<  5)
#define LOG_RCV     (1U <<  6)
#define LOG_CTS     (1U <<  7)
#define LOG_DCD     (1U <<  8)
#define LOG_SYNC    (1U <<  9)
#define LOG_BIT     (1U <<  10)

//#define VERBOSE  (LOG_INT | LOG_READ | LOG_SETUP | LOG_TX | LOG_CMD | LOG_CTS)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)
#define LOGCMD(...)   LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGTX(...)    LOGMASKED(LOG_TX,      __VA_ARGS__)
#define LOGRCV(...)   LOGMASKED(LOG_RCV,     __VA_ARGS__)
#define LOGCTS(...)   LOGMASKED(LOG_CTS,     __VA_ARGS__)
#define LOGDCD(...)   LOGMASKED(LOG_DCD,     __VA_ARGS__)
#define LOGSYNC(...)  LOGMASKED(LOG_SYNC,    __VA_ARGS__)
#define LOGBIT(...)   LOGMASKED(LOG_BIT,     __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"


enum : uint8_t
{
	RR0_RX_CHAR_AVAILABLE     = 0x01,
	RR0_INTERRUPT_PENDING     = 0x02,
	RR0_TX_BUFFER_EMPTY       = 0x04,
	RR0_DCD                   = 0x08,
	RR0_SYNC_HUNT             = 0x10,
	RR0_CTS                   = 0x20,
	RR0_TX_UNDERRUN           = 0x40,
	RR0_BREAK_ABORT           = 0x80
};

enum : uint8_t
{
	RR1_ALL_SENT              = 0x01,
	RR1_RESIDUE_CODE_MASK     = 0x0e,
	RR1_PARITY_ERROR          = 0x10,
	RR1_RX_OVERRUN_ERROR      = 0x20,
	RR1_CRC_FRAMING_ERROR     = 0x40,
	RR1_END_OF_FRAME          = 0x80
};

enum : uint8_t
{
	RR2_INT_VECTOR_MASK       = 0xff,
	RR2_INT_VECTOR_V1         = 0x02,
	RR2_INT_VECTOR_V2         = 0x04,
	RR2_INT_VECTOR_V3         = 0x08
};

enum : uint8_t
{
	WR0_REGISTER_MASK         = 0x07,
	WR0_COMMAND_MASK          = 0x38,
	WR0_NULL                  = 0x00,
	WR0_SEND_ABORT            = 0x08,
	WR0_RESET_EXT_STATUS      = 0x10,
	WR0_CHANNEL_RESET         = 0x18,
	WR0_ENABLE_INT_NEXT_RX    = 0x20,
	WR0_RESET_TX_INT          = 0x28,
	WR0_ERROR_RESET           = 0x30,
	WR0_RETURN_FROM_INT       = 0x38,
	WR0_CRC_RESET_CODE_MASK   = 0xc0,
	WR0_CRC_RESET_NULL        = 0x00,
	WR0_CRC_RESET_RX          = 0x40,
	WR0_CRC_RESET_TX          = 0x80,
	WR0_CRC_RESET_TX_UNDERRUN = 0xc0
};

enum : uint8_t
{
	WR1_EXT_INT_ENABLE        = 0x01,
	WR1_TX_INT_ENABLE         = 0x02,
	WR1_STATUS_VECTOR         = 0x04,
	WR1_RX_INT_MODE_MASK      = 0x18,
	WR1_RX_INT_DISABLE        = 0x00,
	WR1_RX_INT_FIRST          = 0x08,
	WR1_RX_INT_ALL_PARITY     = 0x10,
	WR1_RX_INT_ALL            = 0x18,
	WR1_WRDY_ON_RX_TX         = 0x20,
	WR1_WRDY_FUNCTION         = 0x40, // WAIT not supported
	WR1_WRDY_ENABLE           = 0x80
};

enum : uint8_t
{
	WR2_DATA_XFER_INT         = 0x00, // not supported
	WR2_DATA_XFER_DMA_INT     = 0x01, // not supported
	WR2_DATA_XFER_DMA         = 0x02, // not supported
	WR2_DATA_XFER_ILLEGAL     = 0x03, // not supported
	WR2_DATA_XFER_MASK        = 0x03, // not supported
	WR2_PRIORITY              = 0x04,
	WR2_MODE_8085_1           = 0x00, // not supported
	WR2_MODE_8085_2           = 0x08, // not supported
	WR2_MODE_8086_8088        = 0x10, // not supported
	WR2_MODE_ILLEGAL          = 0x18, // not supported
	WR2_MODE_MASK             = 0x18, // not supported
	WR2_VECTORED_INT          = 0x20, // partially supported
	WR2_PIN10_SYNDETB_RTSB    = 0x80  // not supported
};

enum : uint8_t
{
	WR3_RX_ENABLE             = 0x01,
	WR3_SYNC_CHAR_LOAD_INHIBIT= 0x02, // not supported
	WR3_ADDRESS_SEARCH_MODE   = 0x04, // not supported
	WR3_RX_CRC_ENABLE         = 0x08, // not supported
	WR3_ENTER_HUNT_PHASE      = 0x10,
	WR3_AUTO_ENABLES          = 0x20,
	WR3_RX_WORD_LENGTH_MASK   = 0xc0,
	WR3_RX_WORD_LENGTH_5      = 0x00,
	WR3_RX_WORD_LENGTH_7      = 0x40,
	WR3_RX_WORD_LENGTH_6      = 0x80,
	WR3_RX_WORD_LENGTH_8      = 0xc0
};

enum : uint8_t
{
	WR4_PARITY_ENABLE         = 0x01,
	WR4_PARITY_EVEN           = 0x02,
	WR4_STOP_BITS_MASK        = 0x0c,
	WR4_STOP_BITS_SYNC        = 0x00, // partially supported
	WR4_STOP_BITS_1           = 0x04,
	WR4_STOP_BITS_1_5         = 0x08,
	WR4_STOP_BITS_2           = 0x0c,
	WR4_SYNC_MODE_MASK        = 0x30, // partially supported
	WR4_SYNC_MODE_8_BIT       = 0x00, // partially supported
	WR4_SYNC_MODE_16_BIT      = 0x10, // partially supported
	WR4_SYNC_MODE_SDLC        = 0x20, // partially supported
	WR4_SYNC_MODE_EXT         = 0x30, // partially supported
	WR4_CLOCK_RATE_MASK       = 0xc0,
	WR4_CLOCK_RATE_X1         = 0x00,
	WR4_CLOCK_RATE_X16        = 0x40,
	WR4_CLOCK_RATE_X32        = 0x80,
	WR4_CLOCK_RATE_X64        = 0xc0
};

enum : uint8_t
{
	WR5_TX_CRC_ENABLE         = 0x01,
	WR5_RTS                   = 0x02,
	WR5_CRC16                 = 0x04,
	WR5_TX_ENABLE             = 0x08,
	WR5_SEND_BREAK            = 0x10,
	WR5_TX_WORD_LENGTH_MASK   = 0x60,
	WR5_TX_WORD_LENGTH_5      = 0x00,
	WR5_TX_WORD_LENGTH_6      = 0x40,
	WR5_TX_WORD_LENGTH_7      = 0x20,
	WR5_TX_WORD_LENGTH_8      = 0x60,
	WR5_DTR                   = 0x80
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(Z80SIO_CHANNEL, z80sio_channel,     "z80sio_channel", "Z80 SIO channel")
DEFINE_DEVICE_TYPE(I8274_CHANNEL,  i8274_channel,      "i8274_channel",  "Intel 8274 MPSC channel")
DEFINE_DEVICE_TYPE(Z80SIO,         z80sio_device,      "z80sio",         "Z80 SIO")
DEFINE_DEVICE_TYPE(I8274_NEW,      i8274_new_device,   "i8274_new",      "Intel 8274 MPSC (new)") // Remove trailing N when z80dart.cpp's 8274 implementation is fully replaced
DEFINE_DEVICE_TYPE(UPD7201_NEW,    upd7201_new_device, "upd7201_new",    "NEC uPD7201 MPSC (new)") // Remove trailing N when z80dart.cpp's 7201 implementation is fully replaced

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void z80sio_device::device_add_mconfig(machine_config &config)
{
	Z80SIO_CHANNEL(config, CHANA_TAG, 0);
	Z80SIO_CHANNEL(config, CHANB_TAG, 0);
}

void i8274_new_device::device_add_mconfig(machine_config &config)
{
	I8274_CHANNEL(config, CHANA_TAG, 0);
	I8274_CHANNEL(config, CHANB_TAG, 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

inline void z80sio_channel::out_txd_cb(int state)
{
	m_uart->m_out_txd_cb[m_index](state);
}

inline void z80sio_channel::out_rts_cb(int state)
{
	m_uart->m_out_rts_cb[m_index](state);
}

inline void z80sio_channel::out_dtr_cb(int state)
{
	m_uart->m_out_dtr_cb[m_index](state);
}

inline void z80sio_channel::set_ready(bool ready)
{
	// WAIT mode not supported yet
	if (m_wr1 & WR1_WRDY_FUNCTION)
		m_uart->m_out_wrdy_cb[m_index](ready ? 0 : 1);
}

inline bool z80sio_channel::receive_allowed() const
{
	return (m_wr3 & WR3_RX_ENABLE) && (!(m_wr3 & WR3_AUTO_ENABLES) || !m_dcd);
}

inline bool z80sio_channel::transmit_allowed() const
{
	return (m_wr5 & WR5_TX_ENABLE) && (!(m_wr3 & WR3_AUTO_ENABLES) || !m_cts);
}

inline void z80sio_channel::set_rts(int state)
{
	if (bool(m_rts) != bool(state))
	{
		LOG("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
		out_rts_cb(m_rts = state);
	}
}

inline void z80sio_channel::set_dtr(int state)
{
	if (bool(m_dtr) != bool(state))
	{
		LOG("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
		out_dtr_cb(m_dtr = state);
	}
}

inline void z80sio_channel::tx_setup(uint16_t data, int bits, int parity, bool framing, bool special)
{
	m_tx_bits = bits;
	m_tx_parity = parity;
	m_tx_sr = data | (~uint16_t(0) << bits);
	if (parity)
	{
		if (m_wr4 & WR4_PARITY_EVEN)
			m_tx_sr &= ~(uint16_t(1) << m_tx_bits);
		++m_tx_bits;
	}
	m_tx_flags =
			((!framing && (m_wr5 & WR5_TX_CRC_ENABLE)) ? TX_FLAG_CRC : 0U) |
			(framing ? TX_FLAG_FRAMING : 0U) |
			(special ? TX_FLAG_SPECIAL : 0U);
}

inline void z80sio_channel::tx_setup_idle()
{
	switch (m_wr4 & WR4_SYNC_MODE_MASK)
	{
	case WR4_SYNC_MODE_8_BIT:
		tx_setup(m_wr6, 8, 0, true, false);
		break;
	case WR4_SYNC_MODE_16_BIT:
		tx_setup(uint16_t(m_wr6) | (uint16_t(m_wr7) << 8), 16, 0, true, false);
		break;
	case WR4_SYNC_MODE_SDLC:
		// SDLC transmit examples don't show flag being loaded, implying it's hard-coded on the transmit side
		tx_setup(0x7e, 8, 0, true, false);
		break;
	case WR4_SYNC_MODE_EXT:
		// TODO: what does a real chip do for sync idle in external sync mode?
		// This is based on the assumption that bit 4 controls 8-/16-bit idle pattern (fits for monosync/bisync/SDLC).
		tx_setup(uint16_t(m_wr6) | (uint16_t(m_wr7) << 8), 16, 0, true, false);
		break;
	}
}


//-------------------------------------------------
//  z80sio_device - constructor
//-------------------------------------------------
z80sio_device::z80sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_z80daisy_interface(mconfig, *this),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
	m_hostcpu(*this, finder_base::DUMMY_TAG),
	m_out_txd_cb{ { *this }, { *this } },
	m_out_dtr_cb{ { *this }, { *this } },
	m_out_rts_cb{ { *this }, { *this } },
	m_out_wrdy_cb{ { *this }, { *this } },
	m_out_sync_cb{ { *this }, { *this } },
	m_out_int_cb(*this),
	m_out_rxdrq_cb{ { *this }, { *this } },
	m_out_txdrq_cb{ { *this }, { *this } }
{
	for (auto & elem : m_int_state)
		elem = 0;
}

z80sio_device::z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	z80sio_device(mconfig, Z80SIO, tag, owner, clock)
{
}

i8274_new_device::i8274_new_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	z80sio_device(mconfig, type, tag, owner, clock)
{
}

i8274_new_device::i8274_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8274_new_device(mconfig, I8274_NEW, tag, owner, clock)
{
}

upd7201_new_device::upd7201_new_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8274_new_device(mconfig, UPD7201_NEW, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_validity_check - device-specific validation
//-------------------------------------------------
void z80sio_device::device_validity_check(validity_checker &valid) const
{
	if ((m_hostcpu.finder_tag() != finder_base::DUMMY_TAG) && !m_hostcpu)
		osd_printf_error("Host CPU configured but not found.\n");
}

//-------------------------------------------------
//  device_resolve_objects - device-specific setup
//-------------------------------------------------
void z80sio_device::device_resolve_objects()
{
	LOG("%s\n", FUNCNAME);

	// resolve callbacks
	m_out_txd_cb[CHANNEL_A].resolve_safe();
	m_out_dtr_cb[CHANNEL_A].resolve_safe();
	m_out_rts_cb[CHANNEL_A].resolve_safe();
	m_out_wrdy_cb[CHANNEL_A].resolve_safe();
	m_out_sync_cb[CHANNEL_A].resolve_safe();
	m_out_txd_cb[CHANNEL_B].resolve_safe();
	m_out_dtr_cb[CHANNEL_B].resolve_safe();
	m_out_rts_cb[CHANNEL_B].resolve_safe();
	m_out_wrdy_cb[CHANNEL_B].resolve_safe();
	m_out_sync_cb[CHANNEL_B].resolve_safe();
	m_out_int_cb.resolve_safe();
	m_out_rxdrq_cb[CHANNEL_A].resolve_safe();
	m_out_txdrq_cb[CHANNEL_A].resolve_safe();
	m_out_rxdrq_cb[CHANNEL_B].resolve_safe();
	m_out_txdrq_cb[CHANNEL_B].resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void z80sio_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	// state saving
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void z80sio_device::device_reset()
{
	LOG("%s \"%s\" \n", FUNCNAME, tag());
}

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------
int z80sio_device::z80daisy_irq_state()
{

	int const *const prio = interrupt_priorities();
	LOGINT("%s %s Hi->Lo:%d%d%d%d%d%d ", tag(), FUNCNAME,
			m_int_state[prio[0]], m_int_state[prio[1]], m_int_state[prio[2]],
			m_int_state[prio[3]], m_int_state[prio[4]], m_int_state[prio[5]]);

	// loop over all interrupt sources
	int state = 0;
	for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[prio[i]] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[prio[i]];
	}

	LOGINT("Interrupt State %u\n", state);
	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------
int z80sio_device::z80daisy_irq_ack()
{
	LOGINT("%s \n", FUNCNAME);

	// loop over all interrupt sources
	int const *const prio = interrupt_priorities();
	for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			m_int_state[prio[i]] |= Z80_DAISY_IEO; // Set IUS bit (called IEO in z80 daisy lingo)
			unsigned const vector = read_vector();
			LOGINT(" - Found an INT request, returning RR2: %02x\n", vector);
			check_interrupts();
			return vector;
		}
	}

	// Did we not find a vector? Get the notion of a default vector from the CPU implementation
	logerror(" - failed to find an interrupt to ack!\n");
	if (m_hostcpu)
	{
		// default irq vector is -1 for 68000 but 0 for z80 for example...
		int const ret = m_hostcpu->default_irq_vector(INPUT_LINE_IRQ0);
		LOGINT(" - failed to find an interrupt to ack [%s], returning default IRQ vector: %02x\n", m_hostcpu->tag(), ret);
		return ret;
	}

	// indicate default vector
	return -1;
}

int i8274_new_device::z80daisy_irq_ack()
{
	// FIXME: we're not modelling the full behaviour of this chip
	// The 8274 is designed to work with Intel processors with multiple interrupt acknowledge cycles
	// Values placed on the bus depend on WR2 A mode bits and /IPI input
	// +----+----+----+------+--------------+-------+--------+
	// | D5 | D4 | D3 | /IPI | Mode         | Cycle | Data   |
	// +----+----+----+------+--------------+-------+--------+
	// | 0  | -  | -  | -    | non-vectored | -     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 0  | 0    | 8085 (1)     | 1     | 0xcd   |
	// |    |    |    |      |              | 2     | vector |
	// |    |    |    |      |              | 3     | 0x00   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 0  | 1    | 8085 (1)     | 1     | 0xcd   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// |    |    |    |      |              | 3     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 1  | 0    | 8085 (2)     | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | vector |
	// |    |    |    |      |              | 3     | 0x00   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 1  | 1    | 8085 (2)     | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// |    |    |    |      |              | 3     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 1  | 0  | 0    | 8086         | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | vector |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 1  | 0  | 1    | 8086         | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	LOGINT("%s \n", FUNCNAME);

	// don't do this in non-vectored mode
	if (m_chanB->m_wr2 & WR2_VECTORED_INT)
	{
		// loop over all interrupt sources
		int const *const prio = interrupt_priorities();
		for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
		{
			// find the first channel with an interrupt requested
			if (m_int_state[prio[i]] & Z80_DAISY_INT)
			{
				m_int_state[prio[i]] |= Z80_DAISY_IEO; // Set IUS bit (called IEO in z80 daisy lingo)
				unsigned const vector = read_vector();
				LOGINT(" - Found an INT request, returning RR2: %02x\n", vector);
				check_interrupts();
				return vector;
			}
		}

		// Did we not find a vector? Get the notion of a default vector from the CPU implementation
		logerror(" - failed to find an interrupt to ack!\n");
	}

	if (m_hostcpu)
	{
		// default irq vector is -1 for 68000 but 0 for z80 for example...
		int const ret = m_hostcpu->default_irq_vector(INPUT_LINE_IRQ0);
		LOGINT(" - failed to find an interrupt to ack [%s], returning default IRQ vector: %02x\n", m_hostcpu->tag(), ret);
		return ret;
	}

	// indicate default vector
	return -1;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------
void z80sio_device::z80daisy_irq_reti()
{
	LOGINT("%s\n", FUNCNAME);
	return_from_interrupt();
}

void i8274_new_device::z80daisy_irq_reti()
{
	LOGINT("%s - i8274/uPD7201 lacks RETI detection, no action taken\n", FUNCNAME);
}


//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------
void z80sio_device::check_interrupts()
{
	LOGINT("%s %s \n", FUNCNAME, tag());
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------
void z80sio_device::reset_interrupts()
{
	LOGINT("%s %s \n",FUNCNAME, tag());
	// reset internal interrupt sources
	for (auto & elem : m_int_state)
	{
		elem = 0;
	}

	check_interrupts();
}

//-------------------------------------------------
//  trigger_interrupt - interrupt has fired
//-------------------------------------------------
void z80sio_device::trigger_interrupt(int index, int type)
{
	LOGINT("%s Chan:%c Type:%s\n", FUNCNAME, 'A' + index, std::array<char const *, 3>
		   {{"INT_TRANSMIT", "INT_EXTERNAL", "INT_RECEIVE"}}[type]);

	// trigger interrupt
	m_int_state[(index * 3) + type] |= Z80_DAISY_INT;
	m_chanA->m_rr0 |= RR0_INTERRUPT_PENDING;

	// check for interrupt
	check_interrupts();
}


//-------------------------------------------------
//  clear_interrupt - interrupt has been cleared
//-------------------------------------------------
void z80sio_device::clear_interrupt(int index, int type)
{
	LOGINT("%s Chan:%c Type:%s\n", FUNCNAME, 'A' + index, std::array<char const *, 3>
		   {{"INT_TRANSMIT", "INT_EXTERNAL", "INT_RECEIVE"}}[type]);

	// clear interrupt
	m_int_state[(index * 3) + type] &= ~Z80_DAISY_INT;
	if (std::find_if(std::begin(m_int_state), std::end(m_int_state), [] (int state) { return bool(state & Z80_DAISY_INT); }) == std::end(m_int_state))
		m_chanA->m_rr0 &= ~RR0_INTERRUPT_PENDING;

	// update interrupt output
	check_interrupts();
}


//-------------------------------------------------
//  return_from_interrupt - reset interrupt under
//  service latch
//-------------------------------------------------
void z80sio_device::return_from_interrupt()
{
	// loop over all interrupt sources
	int const *const prio = interrupt_priorities();
	for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[prio[i]] & (Z80_DAISY_IEO))
		{
			// clear the IEO state and update the IRQs
			m_int_state[prio[i]] &= ~Z80_DAISY_IEO;
			check_interrupts();
			LOGINT("%s - cleared IEO\n", FUNCNAME);
			return;
		}
	}
	LOGINT("%s - failed to find an interrupt to clear IEO on!", FUNCNAME);
}


//-------------------------------------------------
//  read_vector - read modified interrupt vector
//-------------------------------------------------
uint8_t z80sio_device::read_vector()
{
	uint8_t vec = m_chanB->m_wr2;

	// if status doesn't affect vector, return unmodified value
	if (!(m_chanB->m_wr1 & WR1_STATUS_VECTOR))
		return vec;

	// modify vector for highest-priority pending interrupt
	int const *const prio = interrupt_priorities();
	vec &= 0xf1U;
	for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
	{
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			constexpr uint8_t RR1_SPECIAL(RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR | RR1_END_OF_FRAME);
			switch (prio[i])
			{
			case 0 + z80sio_channel::INT_TRANSMIT:
				return vec | 0x08U;
			case 0 + z80sio_channel::INT_EXTERNAL:
				return vec | 0x0aU;
			case 0 + z80sio_channel::INT_RECEIVE:
				if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanA->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | 0x0eU;
				else if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanA->m_rr1 & RR1_SPECIAL))
					return vec | 0x0eU;
				else
					return vec | 0x0cU;
			case 3 + z80sio_channel::INT_TRANSMIT:
				return vec | 0x00U;
			case 3 + z80sio_channel::INT_EXTERNAL:
				return vec | 0x02U;
			case 3 + z80sio_channel::INT_RECEIVE:
				if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanB->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | 0x06U;
				else if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanB->m_rr1 & RR1_SPECIAL))
					return vec | 0x06U;
				else
					return vec | 0x04U;
			}
		}
	}

	// no interrupt pending - stuff 011 in the variable bits
	return vec | 0x06U;
}

/*
   8274: "RR2 contains the vector which gets modified to indicate the source of interrupt. However, the state of
   the vector does not change if no new interrupts are generated. The contents of RR2 are only changed when
   a new interrupt is generated. In order to get the correct information, RR2 must be read only after an
   interrrupt is generated, otherwise it will indicate the previous state."
   8274: "If RR2 is specified but not read, no internal interrupts, regardless of priority, are accepted."
*/
uint8_t i8274_new_device::read_vector()
{
	// 8086 and 8085 modes have different variable bits
	bool const aff(m_chanB->m_wr1 & WR1_STATUS_VECTOR);
	int const shift(((m_chanA->m_wr2 & WR2_MODE_MASK) == WR2_MODE_8086_8088) ? 0 : 2);
	uint8_t vec(m_chanB->m_wr2);

	// if status doesn't affect vector, return unmodified value
	if (aff)
		vec &= ~(0x07U << shift);

	// modify vector for highest-priority pending interrupt
	int const *const prio = interrupt_priorities();
	for (int i = 0; ARRAY_LENGTH(m_int_state) > i; ++i)
	{
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			constexpr uint8_t RR1_SPECIAL(RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR | RR1_END_OF_FRAME);

			// in non-vectored mode this serves the same function as the end of the second acknowldege cycle
			if (!(m_chanB->m_wr2 & WR2_VECTORED_INT) && !machine().side_effects_disabled())
			{
				m_int_state[prio[i]] |= Z80_DAISY_IEO;
				check_interrupts();
			}

			// if status doesn't affect vector return unmodified value
			if (!aff)
				return vec;

			switch (prio[i])
			{
			case 0 + z80sio_channel::INT_TRANSMIT:
				return vec | (0x04U << shift);
			case 0 + z80sio_channel::INT_EXTERNAL:
				return vec | (0x05U << shift);
			case 0 + z80sio_channel::INT_RECEIVE:
				if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanA->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | (0x07U << shift);
				else if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanA->m_rr1 & RR1_SPECIAL))
					return vec | (0x07U << shift);
				else
					return vec | (0x06U << shift);
			case 3 + z80sio_channel::INT_TRANSMIT:
				return vec | (0x00U << shift);
			case 3 + z80sio_channel::INT_EXTERNAL:
				return vec | (0x01U << shift);
			case 3 + z80sio_channel::INT_RECEIVE:
				if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanB->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | (0x03U << shift);
				else if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanB->m_rr1 & RR1_SPECIAL))
					return vec | (0x03U << shift);
				else
					return vec | (0x02U << shift);
			}
		}
	}

	// no interrupt pending - stuff 111 in the variable bits
	return aff ? (vec | (0x07 << shift)) : vec;
}


//-------------------------------------------------
//  interrupt_priorities - get interrupt indexes
//  in priority order
//-------------------------------------------------
int const *z80sio_device::interrupt_priorities() const
{
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio{
			0 + z80sio_channel::INT_RECEIVE, 0 + z80sio_channel::INT_TRANSMIT, 0 + z80sio_channel::INT_EXTERNAL,
			3 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_TRANSMIT, 3 + z80sio_channel::INT_EXTERNAL };
	return prio;
}

int const *i8274_new_device::interrupt_priorities() const
{
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio_a{
			0 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_RECEIVE,
			0 + z80sio_channel::INT_TRANSMIT, 3 + z80sio_channel::INT_TRANSMIT,
			0 + z80sio_channel::INT_EXTERNAL, 3 + z80sio_channel::INT_EXTERNAL };
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio_b{
			0 + z80sio_channel::INT_RECEIVE, 0 + z80sio_channel::INT_TRANSMIT,
			3 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_TRANSMIT,
			0 + z80sio_channel::INT_EXTERNAL, 3 + z80sio_channel::INT_EXTERNAL };
	return (m_chanA->m_wr2 & WR2_PRIORITY) ? prio_a : prio_b;
}


//-------------------------------------------------
//  m1_r - interrupt acknowledge
//-------------------------------------------------
int z80sio_device::m1_r()
{
	LOGINT("%s %s \n",FUNCNAME, tag());
	return z80daisy_irq_ack();
}

int i8274_new_device::m1_r()
{
	LOGINT("%s %s \n",FUNCNAME, tag());
	return 0;
}


//-------------------------------------------------
//  cd_ba_r -
//-------------------------------------------------
uint8_t z80sio_device::cd_ba_r(offs_t offset)
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  cd_ba_w -
//-------------------------------------------------
void z80sio_device::cd_ba_w(offs_t offset, uint8_t data)
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}


//-------------------------------------------------
//  ba_cd_r -
//-------------------------------------------------
uint8_t z80sio_device::ba_cd_r(offs_t offset)
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------
void z80sio_device::ba_cd_w(offs_t offset, uint8_t data)
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}

//**************************************************************************
//  SIO CHANNEL
//**************************************************************************

//-------------------------------------------------
//  z80sio_channel - constructor
//-------------------------------------------------
z80sio_channel::z80sio_channel(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t rr1_auto_reset)
	: device_t(mconfig, type, tag, owner, clock)
	, m_rx_fifo_depth(0)
	, m_rx_data_fifo(0)
	, m_rx_error_fifo(0)
	, m_rx_clock(0)
	, m_rx_count(0)
	, m_rx_bit(0)
	, m_rx_sr(0)
	, m_rx_first(0)
	, m_rx_break(0)
	, m_rxd(1)
	, m_tx_data(0)
	, m_tx_clock(0), m_tx_count(0), m_tx_bits(0), m_tx_parity(0), m_tx_sr(0), m_tx_crc(0), m_tx_hist(0), m_tx_flags(0)
	, m_txd(1), m_dtr(0), m_rts(0)
	, m_ext_latched(0), m_brk_latched(0), m_cts(0), m_dcd(0), m_sync(0)
	, m_rr1_auto_reset(rr1_auto_reset)
{
	LOG("%s\n",FUNCNAME);

	// Reset all registers
	m_rr0 = m_rr1 =  0;
	m_wr0 = m_wr1 = m_wr2 = m_wr3 = m_wr4 = m_wr5 = m_wr6 = m_wr7 = 0;
}

z80sio_channel::z80sio_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, Z80SIO_CHANNEL, tag, owner, clock, RR1_CRC_FRAMING_ERROR)
{
}

i8274_channel::i8274_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, I8274_CHANNEL, tag, owner, clock, RR1_RX_OVERRUN_ERROR)
{
}


//-------------------------------------------------
//  resove_objects - channel setup
//-------------------------------------------------
void z80sio_channel::device_resolve_objects()
{
	LOG("%s\n",FUNCNAME);
	m_uart = downcast<z80sio_device *>(owner());
	m_index = m_uart->get_channel_index(this);
}

//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------
void z80sio_channel::device_start()
{
	LOG("%s\n",FUNCNAME);

	// state saving
	save_item(NAME(m_rr0));
	save_item(NAME(m_rr1));
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_wr2));
	save_item(NAME(m_wr3));
	save_item(NAME(m_wr4));
	save_item(NAME(m_wr5));
	save_item(NAME(m_wr6));
	save_item(NAME(m_wr7));
	save_item(NAME(m_rx_fifo_depth));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_sr));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_sr));
	save_item(NAME(m_tx_crc));
	save_item(NAME(m_tx_hist));
	save_item(NAME(m_tx_flags));
	save_item(NAME(m_txd));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_ext_latched));
	save_item(NAME(m_brk_latched));
	save_item(NAME(m_dcd));
	save_item(NAME(m_sync));
	save_item(NAME(m_cts));
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------
void z80sio_channel::device_reset()
{
	LOG("%s\n", FUNCNAME);

	// Reset RS232 emulation
	m_rx_fifo_depth = 0;
	m_rx_data_fifo = m_rx_error_fifo = 0U;
	m_rx_bit = 0;
	m_tx_count = 0;
	m_tx_bits = 0;
	m_rr0 &= ~RR0_RX_CHAR_AVAILABLE;
	m_rr1 &= ~(RR1_PARITY_ERROR | RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR);

	// disable receiver
	m_wr3 &= ~WR3_RX_ENABLE;

	// disable transmitter
	m_wr5 &= ~WR5_TX_ENABLE;
	m_rr0 |= RR0_TX_BUFFER_EMPTY | RR0_TX_UNDERRUN;
	m_rr1 |= RR1_ALL_SENT;
	m_tx_flags = 0U;

	// TODO: what happens to WAIT/READY?

	// reset external lines
	out_rts_cb(m_rts = 1);
	out_dtr_cb(m_dtr = 1);

	// reset interrupts
	m_uart->clear_interrupt(m_index, INT_TRANSMIT);
	m_uart->clear_interrupt(m_index, INT_RECEIVE);
	reset_ext_status();
	// FIXME: should this actually reset all the interrtupts, or just the prioritisation (daisy chain) logic?
	if (m_index == z80sio_device::CHANNEL_A)
		m_uart->reset_interrupts();
}


//-------------------------------------------------
//  transmit_enable - start transmission if
//  conditions met
//-------------------------------------------------
void z80sio_channel::transmit_enable()
{
	if (!m_tx_bits && transmit_allowed())
	{
		if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		{
			LOGTX("Channel %c synchronous transmit enabled - load sync pattern\n", 'A' + m_index);
			tx_setup_idle();
			if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
				set_ready(true);
		}
		else if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
		{
			async_tx_setup();
		}
	}
}

//-------------------------------------------------
//  transmit_complete - transmit shift register
//  empty
//-------------------------------------------------
void z80sio_channel::transmit_complete()
{
	LOG("%s %s\n",FUNCNAME, tag());

	if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		sync_tx_sr_empty();
	else if (transmit_allowed() && !(m_rr0 & RR0_TX_BUFFER_EMPTY))
		async_tx_setup(); // async mode, with data available
	else
		LOGTX("%s() \"%s \"Channel %c Transmit buffer empty m_wr5:%02x\n", FUNCNAME, owner()->tag(), 'A' + m_index, m_wr5);
}

//-------------------------------------------------
//  sync_tx_sr_empty - set up next chunk of bits
//  to send
//-------------------------------------------------
void z80sio_channel::sync_tx_sr_empty()
{
	if (!transmit_allowed())
	{
		LOGTX("%s() Channel %c Transmitter Disabled m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_wr5);

		// transmit disabled, set flag if nothing pending
		m_tx_flags &= ~TX_FLAG_SPECIAL;
		if (m_rr0 & RR0_TX_BUFFER_EMPTY)
			m_rr1 |= RR1_ALL_SENT;
	}
	else if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
	{
		LOGTX("%s() Channel %c Transmit Data Byte '%02x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_data, m_wr5);
		tx_setup(m_tx_data, get_tx_word_length(m_tx_data), (m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0, false, false);

		// empty transmit buffer
		m_rr0 |= RR0_TX_BUFFER_EMPTY;
		if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
			set_ready(true);
		if (m_wr1 & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else if ((m_rr0 & RR0_TX_UNDERRUN) || ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_8_BIT))
	{
		// uts20 always resets the underrun/end-of-message flag if it sees it set, but wants to see sync (not CRC) on the loopback.
		// It seems odd that automatic CRC transmission would be disabled by certain modes, but this at least allows the test to pass.

		LOGTX("%s() Channel %c Underrun - load sync pattern m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_wr5);
		bool const first_idle((m_tx_flags & TX_FLAG_SPECIAL) || !(m_tx_flags & TX_FLAG_FRAMING));
		tx_setup_idle();

		if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
			set_ready(true);
		m_rr1 |= RR1_ALL_SENT;

		// if this is the first sync pattern, generate an interrupt indicating that the next frame can be sent
		// FIXME: uts20 definitely doesn't want a Tx interrupt here, but what does SDLC mode want?
		// In that case, it would seem that the underrun flag would already be set when the CRC was loaded.
		if (!(m_rr0 & RR0_TX_UNDERRUN))
			trigger_ext_int();
		else if (first_idle && (m_wr1 & WR1_TX_INT_ENABLE))
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else
	{
		LOGTX("%s() Channel %c Transmit FCS '%04x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_crc, m_wr5);

		// just for fun, SDLC sends the FCS inverted in reverse bit order
		uint16_t const fcs(bitswap<16>(m_tx_crc, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
		tx_setup(((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC) ? ~fcs : fcs, 16, 0, false, true);

		// set the underrun flag so it will send sync next time
		m_rr0 |= RR0_TX_UNDERRUN;
		trigger_ext_int();
	}
}


//-------------------------------------------------
//  async_tx_setup - set up for asynchronous
//  transmission
//-------------------------------------------------
void z80sio_channel::async_tx_setup()
{
	LOGTX("%s() Channel %c Transmit Data Byte '%02x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_data, m_wr5);

	tx_setup(uint16_t(m_tx_data) << 1, get_tx_word_length(m_tx_data) + 1, (m_wr4 & WR4_PARITY_ENABLE) ? 2 : 0, false, false);
	++m_tx_bits; // stop bit

	// empty transmit buffer
	m_rr0 |= RR0_TX_BUFFER_EMPTY;
	if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
		set_ready(true);
	if (m_wr1 & WR1_TX_INT_ENABLE)
		m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
}


//-------------------------------------------------
//  reset_ext_status - reset external/status
//  condiotions
//-------------------------------------------------
void z80sio_channel::reset_ext_status()
{
	// this will clear latched external pin state
	m_ext_latched = 0;
	m_brk_latched = 0;
	read_ext();

	// Clear any pending External interrupt
	m_uart->clear_interrupt(m_index, INT_EXTERNAL);
}


//-------------------------------------------------
//  read_ext - copy external status to register
//-------------------------------------------------
void z80sio_channel::read_ext()
{
	// clear to send
	if (m_cts)
		m_rr0 &= ~RR0_CTS;
	else
		m_rr0 |= RR0_CTS;

	// data carrier detect
	if (m_dcd)
		m_rr0 &= ~RR0_DCD;
	else
		m_rr0 |= RR0_DCD;

	// sync is a general-purpose input in asynchronous mode
	if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC)
	{
		if (m_sync)
			m_rr0 &= ~RR0_SYNC_HUNT;
		else
			m_rr0 |= RR0_SYNC_HUNT;
	}
}

//-------------------------------------------------
//  trigger_ext_int - trigger external signal
//  interrupt
//-------------------------------------------------
void z80sio_channel::trigger_ext_int()
{
	// update line
	if (!m_ext_latched)
		read_ext();
	m_ext_latched = 1;

	// trigger interrupt if enabled
	if (m_wr1 & WR1_EXT_INT_ENABLE)
		m_uart->trigger_interrupt(m_index, INT_EXTERNAL);
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------
int z80sio_channel::get_clock_mode()
{
	//LOG("%s %s\n",FUNCNAME, tag());
	int clocks = 1;

	switch (m_wr4 & WR4_CLOCK_RATE_MASK)
	{
	case WR4_CLOCK_RATE_X1: clocks = 1; break;
	case WR4_CLOCK_RATE_X16:    clocks = 16;    break;
	case WR4_CLOCK_RATE_X32:    clocks = 32;    break;
	case WR4_CLOCK_RATE_X64:    clocks = 64;    break;
	}

	return clocks;
}

/*
   From "uPD7201/7201A MULTI PROTOCOL SERIAL COMMUNICATION CONTROLLER" by NEC:
   "RTSA (Request to Send A): The state of the RTS bit (01 of the CR5 register) controls this pin. If
   the RTS bit is reset in the asynchronous mode, a high level will not be output on the RTS pin until
   all transmit characters are written and the all sent bit (D0 of the SR1 register) is set. In the
   synchronous mode, the state of the RTS bit is used as is. That is, when the RTS bit is 0, the RTS
   pin is 1. When the RTS bit is 1, the RTS pin is O."

   CR5 = m_wr5 and SR1 = m_rr1
*/
void z80sio_channel::update_dtr_rts_break()
{
	//    LOG("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
	LOG("%s() \"%s\" Channel %c \n", FUNCNAME, owner()->tag(), 'A' + m_index);

	// RTS is affected by transmit queue state in asynchronous mode
	if (m_wr5 & WR5_RTS)
		set_rts(0); // when the RTS bit is set, the _RTS output goes low
	else if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		set_rts(1); // in synchronous mode, there's no automatic RTS
	else
		set_rts(((m_rr0 & RR0_TX_BUFFER_EMPTY) && !m_tx_count) ? 1 : 0); // TODO: is this affected by transmit enable?

	// break immediately forces spacing condition on TxD output
	out_txd_cb((m_wr5 & WR5_SEND_BREAK) ? 0 : m_txd);

	// data terminal ready output follows the state programmed into the DTR bit
	set_dtr((m_wr5 & WR5_DTR) ? 0 : 1);
}


//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------
int z80sio_channel::get_rx_word_length()
{
	LOG("%s %s\n",FUNCNAME, tag());
	int bits = 5;

	switch (m_wr3 & WR3_RX_WORD_LENGTH_MASK)
	{
	case WR3_RX_WORD_LENGTH_5:  bits = 5;   break;
	case WR3_RX_WORD_LENGTH_6:  bits = 6;   break;
	case WR3_RX_WORD_LENGTH_7:  bits = 7;   break;
	case WR3_RX_WORD_LENGTH_8:  bits = 8;   break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------
int z80sio_channel::get_tx_word_length() const
{
	LOG("%s\n", FUNCNAME);

	switch (m_wr5 & WR5_TX_WORD_LENGTH_MASK)
	{
	case WR5_TX_WORD_LENGTH_5: return 5;
	case WR5_TX_WORD_LENGTH_6: return 6;
	case WR5_TX_WORD_LENGTH_7: return 7;
	case WR5_TX_WORD_LENGTH_8: return 8;
	}

	return 5;
}

int z80sio_channel::get_tx_word_length(uint8_t data) const
{
	LOG("%s(%02x)\n", FUNCNAME, data);

	// deal with "five bits or less" mode (the actual chips probably detect a sentinel pattern in the transmit shift register)
	int bits = get_tx_word_length();
	if (5 == bits)
	{
		for (int b = 7; (b >= 4) && BIT(data, b); --b)
			--bits;
	}
	return bits;
}

/*
 * This register contains the status of the receive and transmit buffers; the
 * DCD, CTS, and SYNC inputs; the Transmit Underrun/EOM latch; and the
 * Break/Abort latch. */
uint8_t z80sio_channel::do_sioreg_rr0()
{
	LOGR("%s\n", FUNCNAME);
	if (m_tx_flags & TX_FLAG_SPECIAL)
		return m_rr0 & ~RR0_TX_BUFFER_EMPTY;
	else
		return m_rr0;
}

/*
 * This register contains the Special Receive condition status bits and Residue
 * codes for the I-Field in the SDLC Receive Mode. */
uint8_t z80sio_channel::do_sioreg_rr1()
{
	LOGR("%s\n", FUNCNAME);
	return m_rr1;
}

/* Z80-SIO Technical Manual: "This register contains the interrupt vector
   written into WR2 if the Status Affects Vector control bit is not set.
   If the control bit is set, it contains the modified vector listed in
   the Status Affects Vector paragraph of the Write Register 1 section.
   When this register is read, the vector returned is modified by the
   highest priority interrupting condition at the time of the read. If
   no interrupts are pending, the vector is modified with V3 = 0, V2 = 1, and
   V1 = 1. This register is read only through Channel B."

   Intel 8274 datasheet: "RR2 - Channel B: Interrupt Vector - Contains the interrupt
   vector programmed in into WR2. If the status affects vector mode is selected (WR1:D2),
   it containes the modified vector for the highest priority interrupt pending.
   If no interrupts are pending the variable bits in the vector are set to one."

   NEC upd7201 MPSC2 Technical Manual: "When the MPSC2 is used in vectored mode, the
   contents of this register are placed on the bus during the appropriate portion of
   interrupt acknowledge sequence. You can read the value of CR2B at any time.
   This is particularly useful in determining the cause of an interrupt when using the
   MPSC2 in Non-vectored mode."
*/
uint8_t z80sio_channel::do_sioreg_rr2()
{
	LOGINT("%s %s Chan:%c\n", tag(), FUNCNAME, 'A' + m_index);

	// channel B only, channel A returns 0
	if (m_index == z80sio_device::CHANNEL_A)
		return 0U;
	else
		return m_uart->read_vector();
}


//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------
uint8_t z80sio_channel::control_read()
{
	uint8_t data = 0;
	uint8_t const reg  = m_wr0 & WR0_REGISTER_MASK;

	//LOG("%s %s\n",FUNCNAME, tag());
	// mask out register index
	if (!machine().side_effects_disabled())
		m_wr0 &= ~WR0_REGISTER_MASK;

	switch (reg)
	{
	case REG_RR0_STATUS:         data = do_sioreg_rr0(); break;
	case REG_RR1_SPEC_RCV_COND:  data = do_sioreg_rr1(); break;
	case REG_RR2_INTERRUPT_VECT: data = do_sioreg_rr2(); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported RRx register:%02x\n", owner()->tag(), 'A' + m_index, reg);
		LOG("%s %s unsupported register:%02x\n",FUNCNAME, tag(), reg);
	}

	LOGR(" * %s %c Reg %02x -> %02x - %s\n", tag(), 'A' + m_index, reg, data, std::array<char const *, 3>
		 {{"RR0 status register", "RR1 - Special Receive Conditions", "RR2 - Interrupt Vector"}}[reg]);
	return data;
}

/* SIO CRC Initialization Code handling - candidate for breaking out in a z80sio_base class
 Handle the WR0 CRC Reset/Init bits separatelly, needed by derived devices separatelly from the commands */
void z80sio_channel::do_sioreg_wr0_resets(uint8_t data)
{
	LOG("%s\n", FUNCNAME);
	switch (data & WR0_CRC_RESET_CODE_MASK)
	{
	case WR0_CRC_RESET_NULL:
		LOGCMD("Z80SIO Channel %c : CRC_RESET_NULL\n", 'A' + m_index);
		break;
	case WR0_CRC_RESET_RX: /* In Synchronous mode: all Os (zeros) (CCITT-O CRC-16) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_RX - not implemented\n", 'A' + m_index);
		break;
	case WR0_CRC_RESET_TX: /* In HDLC mode: all 1s (ones) (CCITT-1) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_TX\n", 'A' + m_index);
		m_tx_crc = ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC) ? ~uint16_t(0U) : uint16_t(0U);
		break;
	case WR0_CRC_RESET_TX_UNDERRUN: /* Resets Tx underrun/EOM bit (D6 of the SRO register) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_TX_UNDERRUN\n", 'A' + m_index);
		m_rr0 &= ~RR0_TX_UNDERRUN;
		break;
	default: /* Will not happen unless someone messes with the mask */
		logerror("Z80SIO Channel %c : %s Wrong CRC reset/init command:%02x\n", 'A' + m_index, FUNCNAME, data & WR0_CRC_RESET_CODE_MASK);
	}
}

void z80sio_channel::do_sioreg_wr0(uint8_t data)
{
	m_wr0 = data;

	if ((data & WR0_COMMAND_MASK) != WR0_NULL)
		LOGSETUP(" * %s %c Reg %02x <- %02x \n", owner()->tag(), 'A' + m_index, 0, data);
	switch (data & WR0_COMMAND_MASK)
	{
	case WR0_NULL:
		LOGCMD("%s Ch:%c : Null command\n", FUNCNAME, 'A' + m_index);
		break;
	case WR0_SEND_ABORT:
		// TODO: what actually happens if you try this in a mode other than SDLC?
		if ((m_wr4 & (WR4_STOP_BITS_MASK | WR4_SYNC_MODE_MASK)) != (WR4_STOP_BITS_SYNC | WR4_SYNC_MODE_SDLC))
		{
			LOGCMD("%s Ch:%c : Send abort command (not in SDLC mode, ignored)\n", FUNCNAME, 'A' + m_index);
		}
		else
		{
			LOGCMD("%s Ch:%c : Send abort command\n", FUNCNAME, 'A' + m_index);
			// FIXME: how does this interact with interrupts?
			// For now assume it behaves like automatically sending CRC and generates a transmit interrupt when a new frame can be sent.
			tx_setup(0xff, 8, 0, true, true);
			m_rr0 |= RR0_TX_BUFFER_EMPTY;
			m_rr1 &= ~RR1_ALL_SENT;
			if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
				set_ready(false);
		}
		break;
	case WR0_RESET_EXT_STATUS:
		reset_ext_status();
		LOGINT("%s Ch:%c : Reset External/Status Interrupt\n", FUNCNAME, 'A' + m_index);
		break;
	case WR0_CHANNEL_RESET:
		// channel reset
		LOGCMD("%s Ch:%c : Channel Reset\n", FUNCNAME, 'A' + m_index);
		device_reset();
		break;
	case WR0_ENABLE_INT_NEXT_RX:
		// enable interrupt on next receive character
		LOGINT("%s Ch:%c : Enable Interrupt on Next Received Character\n", FUNCNAME, 'A' + m_index);
		m_rx_first = 1;
		break;
	case WR0_RESET_TX_INT:
		LOGCMD("%s Ch:%c : Reset Transmitter Interrupt Pending\n", FUNCNAME, 'A' + m_index);
		// reset transmitter interrupt pending
		m_uart->clear_interrupt(m_index, INT_TRANSMIT);
		break;
	case WR0_ERROR_RESET:
		// error reset
		LOGCMD("%s Ch:%c : Error Reset\n", FUNCNAME, 'A' + m_index);
		if ((WR1_RX_INT_FIRST == (m_wr1 & WR1_RX_INT_MODE_MASK)) && (m_rr1 & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR)))
		{
			// clearing framing and overrun errors advances the FIFO
			// TODO: Intel 8274 manual doesn't mention this behaviour - is it specific to Z80 SIO?
			m_rr1 &= ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
			advance_rx_fifo();
		}
		else
		{
			m_rr1 &= ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
		}
		break;
	case WR0_RETURN_FROM_INT:
		LOGINT("%s Ch:%c : Return from interrupt\n", FUNCNAME, 'A' + m_index);
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->return_from_interrupt();
		break;
	default:
		LOG("Z80SIO Channel %c : Unsupported WR0 command %02x mask %02x\n", 'A' + m_index, data, WR0_REGISTER_MASK);

	}
	do_sioreg_wr0_resets(data);
}

void z80sio_channel::do_sioreg_wr1(uint8_t data)
{
/* TODO: implement vector modifications when WR1 bit D2 is changed */
	m_wr1 = data;
	LOG("Z80SIO \"%s\" Channel %c : External Interrupt Enable %u\n", owner()->tag(), 'A' + m_index, (data & WR1_EXT_INT_ENABLE) ? 1 : 0);
	LOG("Z80SIO \"%s\" Channel %c : Transmit Interrupt Enable %u\n", owner()->tag(), 'A' + m_index, (data & WR1_TX_INT_ENABLE) ? 1 : 0);
	LOG("Z80SIO \"%s\" Channel %c : Status Affects Vector %u\n", owner()->tag(), 'A' + m_index, (data & WR1_STATUS_VECTOR) ? 1 : 0);
	LOG("Z80SIO \"%s\" Channel %c : Wait/Ready Enable %u\n", owner()->tag(), 'A' + m_index, (data & WR1_WRDY_ENABLE) ? 1 : 0);
	LOG("Z80SIO \"%s\" Channel %c : Wait/Ready Function %s\n", owner()->tag(), 'A' + m_index, (data & WR1_WRDY_FUNCTION) ? "Ready" : "Wait");
	LOG("Z80SIO \"%s\" Channel %c : Wait/Ready on %s\n", owner()->tag(), 'A' + m_index, (data & WR1_WRDY_ON_RX_TX) ? "Receive" : "Transmit");

	switch (data & WR1_RX_INT_MODE_MASK)
	{
	case WR1_RX_INT_DISABLE:
		LOG("Z80SIO \"%s\" Channel %c : Receiver Interrupt Disabled\n", owner()->tag(), 'A' + m_index);
		break;

	case WR1_RX_INT_FIRST:
		LOG("Z80SIO \"%s\" Channel %c : Receiver Interrupt on First Character\n", owner()->tag(), 'A' + m_index);
		break;

	case WR1_RX_INT_ALL_PARITY:
		LOG("Z80SIO \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", owner()->tag(), 'A' + m_index);
		break;

	case WR1_RX_INT_ALL:
		LOG("Z80SIO \"%s\" Channel %c : Receiver Interrupt on All Characters\n", owner()->tag(), 'A' + m_index);
		break;
	}

	if (!(data & WR1_WRDY_ENABLE))
		set_ready(false);
	else if (data & WR1_WRDY_ON_RX_TX)
		set_ready(bool(m_rr0 & RR0_RX_CHAR_AVAILABLE));
	else
		set_ready((m_rr0 & RR0_TX_BUFFER_EMPTY) && !(m_tx_flags & TX_FLAG_SPECIAL));
}

void z80sio_channel::do_sioreg_wr2(uint8_t data)
{
	m_wr2 = data;
	LOG("Z80SIO \"%s\" Channel %c : Interrupt Vector %02x\n", owner()->tag(), 'A' + m_index, data);
}

void z80sio_channel::do_sioreg_wr3(uint8_t data)
{
	LOGSETUP("Z80SIO Channel %c : Receiver Enable %u\n", 'A' + m_index, (data & WR3_RX_ENABLE) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Sync Character Load Inhibit %u\n", 'A' + m_index, (data & WR3_SYNC_CHAR_LOAD_INHIBIT) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Receive CRC Enable %u\n", 'A' + m_index, (data & WR3_RX_CRC_ENABLE) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Auto Enables %u\n", 'A' + m_index, (data & WR3_AUTO_ENABLES) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Receiver Bits/Character %u\n", 'A' + m_index, get_rx_word_length());
	if (data & WR3_ENTER_HUNT_PHASE)
		LOGCMD("Z80SIO Channel %c : Enter Hunt Phase\n", 'A' + m_index);

	bool const was_allowed(receive_allowed());
	m_wr3 = data;

	if (!was_allowed && receive_allowed())
	{
		receive_enabled();
	}
	else if ((data & WR3_ENTER_HUNT_PHASE) && ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC))
	{
		// TODO: should this re-initialise hunt logic if already in hunt phase for 8-bit/16-bit/SDLC sync?
		if ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_EXT)
		{
			m_rx_bit = 0;
		}
		else if (!(m_rr0 & RR0_SYNC_HUNT))
		{
			m_rx_bit = 0;
			m_rr0 |= RR0_SYNC_HUNT;
			trigger_ext_int();
		}
	}
}

void z80sio_channel::do_sioreg_wr4(uint8_t data)
{
	m_wr4 = data;
	LOG("Z80SIO \"%s\" Channel %c : Parity Enable %u\n", owner()->tag(), 'A' + m_index, (data & WR4_PARITY_ENABLE) ? 1 : 0);
	LOG("Z80SIO \"%s\" Channel %c : Parity %s\n", owner()->tag(), 'A' + m_index, (data & WR4_PARITY_EVEN) ? "Even" : "Odd");
	if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		LOG("Z80SIO \"%s\" Channel %c : Synchronous Mode\n", owner()->tag(), 'A' + m_index);
	else
		LOG("Z80SIO \"%s\" Channel %c : Stop Bits %g\n", owner()->tag(), 'A' + m_index, (((m_wr4 & WR4_STOP_BITS_MASK) >> 2) + 1) / 2.);
	LOG("Z80SIO \"%s\" Channel %c : Clock Mode %uX\n", owner()->tag(), 'A' + m_index, get_clock_mode());
}

void z80sio_channel::do_sioreg_wr5(uint8_t data)
{
	m_wr5 = data;
	LOG("Z80SIO Channel %c : Transmitter Enable %u\n", 'A' + m_index, (data & WR5_TX_ENABLE) ? 1 : 0);
	LOG("Z80SIO Channel %c : Transmitter Bits/Character %u\n", 'A' + m_index, get_tx_word_length());
	LOG("Z80SIO Channel %c : Transmit CRC Enable %u\n", 'A' + m_index, (data & WR5_TX_CRC_ENABLE) ? 1 : 0);
	LOG("Z80SIO Channel %c : %s Frame Check Polynomial\n", 'A' + m_index, (data & WR5_CRC16) ? "CRC-16" : "SDLC");
	LOG("Z80SIO Channel %c : Send Break %u\n", 'A' + m_index, (data & WR5_SEND_BREAK) ? 1 : 0);
	LOG("Z80SIO Channel %c : Request to Send %u\n", 'A' + m_index, (data & WR5_RTS) ? 1 : 0);
	LOG("Z80SIO Channel %c : Data Terminal Ready %u\n", 'A' + m_index, (data & WR5_DTR) ? 1 : 0);

	if (~data & WR5_TX_ENABLE)
		m_uart->clear_interrupt(m_index, INT_TRANSMIT);
}

void z80sio_channel::do_sioreg_wr6(uint8_t data)
{
	LOG("Z80SIO \"%s\" Channel %c : Transmit Sync/Sync 1/SDLC Address %02x\n", owner()->tag(), 'A' + m_index, data);
	m_wr6 = data;
}

void z80sio_channel::do_sioreg_wr7(uint8_t data)
{
	LOG("Z80SIO \"%s\" Channel %c : Receive Sync/Sync 2/SDLC Flag %02x\n", owner()->tag(), 'A' + m_index, data);
	m_wr7 = data;
}

//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------
void z80sio_channel::control_write(uint8_t data)
{
	uint8_t   reg = m_wr0 & WR0_REGISTER_MASK;

	if (reg != 0)
	{
		LOGSETUP(" * %s %c Reg %02x <- %02x - %s\n", tag(), 'A' + m_index, reg, data, std::array<char const *, 8>
			 {{"WR0", "WR1", "WR2", "WR3 - Async Rx setup", "WR4 - Async Clock, Parity and stop bits", "WR5 - Async Tx setup", "WR6", "WR7"}}[reg]);
		// mask out register index
		m_wr0 &= ~WR0_REGISTER_MASK;
	}

	LOG("%s(%02x) reg %02x\n", FUNCNAME, data, reg);

	switch (reg)
	{
	case REG_WR0_COMMAND_REGPT:     do_sioreg_wr0(data); break;
	case REG_WR1_INT_DMA_ENABLE:    do_sioreg_wr1(data); m_uart->check_interrupts(); break;
	case REG_WR2_INT_VECTOR:        do_sioreg_wr2(data); break;
	case REG_WR3_RX_CONTROL:        do_sioreg_wr3(data); break;
	case REG_WR4_RX_TX_MODES:       do_sioreg_wr4(data); update_dtr_rts_break(); break;
	case REG_WR5_TX_CONTROL:        do_sioreg_wr5(data); update_dtr_rts_break(); transmit_enable(); break;
	case REG_WR6_SYNC_OR_SDLC_A:    do_sioreg_wr6(data); break;
	case REG_WR7_SYNC_OR_SDLC_F:    do_sioreg_wr7(data); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported WRx register:%02x\n", owner()->tag(), 'A' + m_index, reg);
	}
}


//-------------------------------------------------
//  data_read - read data register
//-------------------------------------------------
uint8_t z80sio_channel::data_read()
{
	uint8_t const data = uint8_t(m_rx_data_fifo & 0x000000ffU);

	if (!machine().side_effects_disabled())
	{
		// framing and overrun errors need to be cleared to advance the FIFO in interrupt-on-first mode
		// TODO: Intel 8274 manual doesn't mention this behaviour - is it specific to Z80 SIO?
		if ((WR1_RX_INT_FIRST != (m_wr1 & WR1_RX_INT_MODE_MASK)) || !(m_rr1 & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR)))
			advance_rx_fifo();

		LOG("Z80SIO \"%s\" Channel %c : Data Register Read '%02x'\n", owner()->tag(), 'A' + m_index, data);
	}

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------
void z80sio_channel::data_write(uint8_t data)
{
	if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
		LOGTX("Z80SIO \"%s\" Channel %c : Dropped Data Byte '%02x'\n", owner()->tag(), 'A' + m_index, m_tx_data);
	LOGTX("Z80SIO Channel %c : Queue Data Byte '%02x'\n", 'A' + m_index, data);

	// fill transmit buffer
	m_tx_data = data;
	m_rr0 &= ~RR0_TX_BUFFER_EMPTY;
	m_rr1 &= ~RR1_ALL_SENT;
	if ((m_wr1 & WR1_WRDY_ENABLE) && !(m_wr1 & WR1_WRDY_ON_RX_TX))
		set_ready(false);

	// handle automatic RTS
	bool const async((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC);
	if (async && !(m_wr5 & WR5_RTS))
		set_rts(0); // TODO: if transmission is disabled when the data buffer is full, is this still asserted?

	// clear transmit interrupt
	m_uart->clear_interrupt(m_index, INT_TRANSMIT);

	// may be possible to transmit immediately (synchronous mode will load when sync pattern completes)
	if (async && !m_tx_bits && transmit_allowed())
		async_tx_setup();
}


//-------------------------------------------------
//  advance_rx_fifo - move to next received byte
//-------------------------------------------------
void z80sio_channel::advance_rx_fifo()
{
	if (m_rx_fifo_depth)
	{
		if (--m_rx_fifo_depth)
		{
			// shift the FIFO
			m_rx_data_fifo >>= 8;
			m_rx_error_fifo >>= 8;

			// load error status from the FIFO
			m_rr1 = (m_rr1 & ~m_rr1_auto_reset) | uint8_t(m_rx_error_fifo & 0x000000ffU);

			// if we're in interrupt-on-first mode, clear interrupt if there's no pending error condition
			if ((m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_FIRST)
			{
				for (int i = 0; m_rx_fifo_depth > i; ++i)
				{
					if (uint8_t(m_rx_error_fifo >> (i * 8)) & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR))
						return;
				}
				m_uart->clear_interrupt(m_index, INT_RECEIVE);
			}
		}
		else
		{
			// no more characters available in the FIFO
			m_rr0 &= ~RR0_RX_CHAR_AVAILABLE;
			if ((m_wr1 & WR1_WRDY_ENABLE) && (m_wr1 & WR1_WRDY_ON_RX_TX))
				set_ready(false);
			m_uart->clear_interrupt(m_index, INT_RECEIVE);
		}
	}
}


//-------------------------------------------------
//  receive_enabled - conditions have changed
//  allowing reception to begin
//-------------------------------------------------

void z80sio_channel::receive_enabled()
{
	bool const sync_mode((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC);
	m_rx_count = sync_mode ? 0 : ((get_clock_mode() - 1) / 2);
	m_rx_bit = 0;
	if (sync_mode && ((m_wr4 & WR4_SYNC_MODE_MASK) != WR4_SYNC_MODE_EXT))
		m_rr0 |= RR0_SYNC_HUNT;
}


//-------------------------------------------------
//  sync_receive - synchronous reception handler
//-------------------------------------------------

void z80sio_channel::sync_receive()
{
	// TODO: this is a fundamentally flawed approach - it's just the quickest way to get uts20 to pass some tests
	// Sync acquisition works, but sync load suppression doesn't work right.
	// Assembled data needs to be separated from the receive shift register for SDLC.
	// Supporting receive checksum for modes other than SDLC is going to be very complicated due to all the bit delays involved.

	bool const ext_sync((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_EXT);
	bool const hunt_phase(ext_sync ? m_sync : (m_rr0 & RR0_SYNC_HUNT));
	if (hunt_phase)
	{
		// check for sync detection
		bool acquired(false);
		int limit(16);
		switch (m_wr4 & WR4_SYNC_MODE_MASK)
		{
		case WR4_SYNC_MODE_8_BIT:
		case WR4_SYNC_MODE_SDLC:
			acquired = (m_rx_bit >= 8) && ((m_rx_sr & 0xff00U) == (uint16_t(m_wr7) << 8));
			limit = 8;
			break;
		case WR4_SYNC_MODE_16_BIT:
			acquired = (m_rx_bit >= 16) && (m_rx_sr == ((uint16_t(m_wr7) << 8) | uint16_t(m_wr6)));
			break;
		}
		if (acquired)
		{
			// TODO: make this do something sensible in SDLC mode
			// FIXME: set sync output for one receive bit cycle
			// FIXME: what if sync load isn't suppressed?
			LOGRCV("%s() Channel %c Character Sync Acquired\n", FUNCNAME, 'A' + m_index);
			m_rr0 &= ~RR0_SYNC_HUNT;
			m_rx_bit = 0;
			trigger_ext_int();
		}
		else
		{
			// track number of bits we have
			m_rx_bit = (std::min)(m_rx_bit + 1, limit);
		}
	}
	else
	{
		// FIXME: SDLC needs to monitor for flag/abort
		// FIXME: what if sync load is suppressed?
		// FIXME: what about receive checksum and the nasty internal shift register delays?
		int const word_length(get_rx_word_length() + ((m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0));
		if (++m_rx_bit == word_length)
		{
			uint16_t const data((m_rx_sr >> (16 - word_length)) | (~uint16_t(0) << word_length));
			m_rx_bit = 0;
			LOGRCV("%s() Channel %c Received Data %02x\n", FUNCNAME, 'A' + m_index, data & 0xff);
			queue_received(data, 0U);
		}
	}

	LOGBIT("%s() Channel %c Read Bit %d\n", FUNCNAME, 'A' + m_index, m_rxd);
	m_rx_sr = (m_rx_sr >> 1) | (m_rxd ? 0x8000U : 0x0000U);
}

//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80sio_channel::receive_data()
{
}

//-------------------------------------------------
//  queue_recevied - queue recevied character
//-------------------------------------------------

void z80sio_channel::queue_received(uint16_t data, uint32_t error)
{
	if (m_wr4 & WR4_PARITY_ENABLE)
	{
		int const word_length = get_rx_word_length();
		uint16_t par(data);
		for (int i = 1; word_length >= i; ++i)
			par ^= BIT(par, i);

		if (bool(BIT(par, 0)) == bool(m_wr4 & WR4_PARITY_EVEN))
		{
			LOGRCV("  Parity error detected\n");
			error |= RR1_PARITY_ERROR;
		}
	}

	if (3 == m_rx_fifo_depth)
	{
		LOG("  Receive FIFO overrun detected\n");
		// receive overrun error detected
		error |= RR1_RX_OVERRUN_ERROR;

		m_rx_data_fifo = (m_rx_data_fifo & 0x0000ffffU) | (uint32_t(data & 0x00ffU) << 16);
		m_rx_error_fifo = (m_rx_error_fifo & 0x0000ffffU) | (error << 16);
	}
	else
	{
		// store received character and error status into FIFO
		if (!m_rx_fifo_depth)
			m_rx_data_fifo = m_rx_error_fifo = 0U;
		m_rx_data_fifo |= uint32_t(data & 0x00ffU) << (8 * m_rx_fifo_depth);
		m_rx_error_fifo |= error << (8 * m_rx_fifo_depth);
		if (!m_rx_fifo_depth)
			m_rr1 |= uint8_t(error);
		++m_rx_fifo_depth;
	}

	m_rr0 |= RR0_RX_CHAR_AVAILABLE;
	if ((m_wr1 & WR1_WRDY_ENABLE) && (m_wr1 & WR1_WRDY_ON_RX_TX))
		set_ready(true);

	// receive interrupt
	switch (m_wr1 & WR1_RX_INT_MODE_MASK)
	{
	case WR1_RX_INT_FIRST:
		if (m_rx_first || (error & (RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR)))
			m_uart->trigger_interrupt(m_index, INT_RECEIVE);
		m_rx_first = 0;
		break;

	case WR1_RX_INT_ALL_PARITY:
	case WR1_RX_INT_ALL:
		m_uart->trigger_interrupt(m_index, INT_RECEIVE);
		break;

	default:
		LOG("No receive interrupt triggered\n");
	}
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------
WRITE_LINE_MEMBER( z80sio_channel::cts_w )
{
	if (bool(m_cts) != bool(state))
	{
		LOGCTS("Z80SIO Channel %c : CTS %u\n", 'A' + m_index, state);

		m_cts = state;
		trigger_ext_int();

		// this may enable transmission
		if (!state)
			transmit_enable();
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------
WRITE_LINE_MEMBER( z80sio_channel::dcd_w )
{
	if (bool(m_dcd) != bool(state))
	{
		LOG("Z80SIO Channel %c : DCD %u\n", 'A' + m_index, state);

		bool const was_allowed(receive_allowed());
		m_dcd = state;
		trigger_ext_int();

		// in auto-enable mode, this can start the receiver
		if (!was_allowed && receive_allowed())
			receive_enabled();
	}
}


//-------------------------------------------------
//  sh_w - Sync Hunt handler
//-------------------------------------------------
WRITE_LINE_MEMBER( z80sio_channel::sync_w )
{
	if (bool(m_sync) != bool(state))
	{
		LOG("Z80SIO Channel %c : Sync %u\n", 'A' + m_index, state);

		m_sync = state;

		// sync is a general-purpose input in asynchronous mode
		if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC)
			trigger_ext_int();
	}
}


//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------
WRITE_LINE_MEMBER( z80sio_channel::rxc_w )
{
	//LOG("Z80SIO \"%s\" Channel %c : Receiver Clock Pulse\n", owner()->tag(), m_index + 'A');
	if (receive_allowed() && state && !m_rx_clock)
	{
		// RxD sampled on rising edge
		int const clocks = get_clock_mode() - 1;

		// break termination detection
		// TODO: how does this interact with receiver being disable or synchronous modes?
		if (m_rxd && !m_brk_latched && (m_rr0 & RR0_BREAK_ABORT))
		{
			LOGRCV("Break termination detected\n");
			m_rr0 &= ~RR0_BREAK_ABORT;
			m_brk_latched = 1;
			trigger_ext_int();
		}

		if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		{
			// synchronous receive is a different beast
			if (!m_rx_count)
			{
				sync_receive();
				m_rx_count = clocks;
			}
			else
			{
				--m_rx_count;
			}
		}
		else if (!m_rx_bit)
		{
			// look for start bit
			if (m_rxd)
			{
				// line idle
				m_rx_count = (std::max)(m_rx_count, (clocks / 2) + 1) - 1;
			}
			else if (!m_rx_count)
			{
				// half a bit period expired, start shifting bits
				m_rx_count = clocks;
				++m_rx_bit;
				m_rx_sr = ~uint16_t(0U);
			}
			else
			{
				// ensure start bit lasts long enough
				--m_rx_count;
			}
		}
		else if (!m_rx_count)
		{
			// sample a data/parity/stop bit
			if (!m_rxd)
				m_rx_sr &= ~uint16_t(1U << (m_rx_bit - 1));
			int const word_length(get_rx_word_length() + ((m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0));
			bool const stop_reached((word_length + 1) == m_rx_bit);
			LOGBIT("%s() Channel %c Received %s Bit %d\n", FUNCNAME, 'A' + m_index, stop_reached ? "Stop" : "Data", m_rxd);

			if (stop_reached)
			{
				// this is the stop bit - framing error adds a half bit period
				m_rx_count = m_rxd ? (clocks / 2) : clocks;
				m_rx_bit = 0;

				LOGRCV("%s() Channel %c Received Data %02x\n", FUNCNAME, 'A' + m_index, m_rx_sr & 0xff);

				// check framing errors and break condition
				uint16_t const stop_bit = uint16_t(1U) << word_length;
				bool const brk(!(m_rx_sr & ((stop_bit << 1) - 1)));
				queue_received(m_rx_sr | stop_bit, (m_rx_sr & stop_bit) ? 0U : RR1_CRC_FRAMING_ERROR);

				// break interrupt
				if (brk && !m_brk_latched && !(m_rr0 & RR0_BREAK_ABORT))
				{
					LOGRCV("Break detected\n");
					m_rr0 |= RR0_BREAK_ABORT;
					m_brk_latched = 1;
					trigger_ext_int();
				}
			}
			else
			{
				// wait a whole bit period for the next bit
				m_rx_count = clocks;
				++m_rx_bit;
			}
		}
		else
		{
			// bit period hasn't expired
			--m_rx_count;
		}
	}
	m_rx_clock = state;
}


//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------
WRITE_LINE_MEMBER( z80sio_channel::txc_w )
{
	//LOG("Z80SIO \"%s\" Channel %c : Transmitter Clock Pulse\n", owner()->tag(), m_index + 'A');
	if (!state && m_tx_clock)
	{
		// falling edge active
		if (m_tx_count)
		{
			// divide transmit clock
			--m_tx_count;
		}
		else if (!m_tx_bits)
		{
			// idle marking line
			if (!m_txd)
			{
				m_txd = 1;
				if (!(m_wr5 & WR5_SEND_BREAK))
					out_txd_cb(1);
			}

			if (((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC) && (m_rr0 & RR0_TX_BUFFER_EMPTY))
			{
				// when the RTS bit is reset in asynchronous mode, the _RTS output goes high after the transmitter empties
				if (!(m_wr5 & WR5_RTS) && !m_rts)
					set_rts(1); // TODO: if transmission is disabled when the data buffer is full, is this still asserted?

				// if transmit buffer is empty in asynchronous mode then all characters have been sent
				m_rr1 |= RR1_ALL_SENT;
			}
		}
		else
		{
			bool const sdlc_mode((m_wr4 & (WR4_STOP_BITS_MASK | WR4_SYNC_MODE_MASK)) == (WR4_STOP_BITS_SYNC | WR4_SYNC_MODE_SDLC));
			bool const framing(m_tx_flags & TX_FLAG_FRAMING);
			bool const stuff_zero(sdlc_mode && !framing && ((m_tx_hist & 0x1fU) == 0x1fU));

			// have bits, shift out
			int const db(stuff_zero ? 0 : BIT(m_tx_sr, 0));
			if (!stuff_zero)
			{
				LOGBIT("%s() Channel %c transmit %s bit %d m_wr5:%02x\n", FUNCNAME, 'A' + m_index, framing ? "framing" : "data", db, m_wr5);
				if (m_tx_parity >= m_tx_bits)
					m_tx_parity = 0;
				else if (m_tx_parity)
					m_tx_sr ^= uint16_t(db) << (m_tx_bits - m_tx_parity);
				m_tx_sr >>= 1;

				if (m_tx_flags & TX_FLAG_CRC)
				{
					uint16_t const poly((m_wr5 & WR5_CRC16) ? 0x8005U : device_sdlc_consumer_interface::POLY_SDLC);
					m_tx_crc = device_sdlc_consumer_interface::update_frame_check(poly, m_tx_crc, db);
				}
			}
			else
			{
				LOGBIT("%s() Channel %c stuff bit %d m_wr5:%02x\n", FUNCNAME, 'A' + m_index, db, m_wr5);
			}
			m_tx_hist = (m_tx_hist << 1) | db;

			// update output line state
			if (bool(m_txd) != bool(db))
			{
				m_txd = db;
				if (!(m_wr5 & WR5_SEND_BREAK))
					out_txd_cb(m_txd);
			}

			// calculate next bit time
			m_tx_count = get_clock_mode();
			if (!stuff_zero && !--m_tx_bits)
			{
				switch (m_wr4 & WR4_STOP_BITS_MASK)
				{
				case WR4_STOP_BITS_SYNC:
				case WR4_STOP_BITS_1:
					break;
				case WR4_STOP_BITS_1_5:
					m_tx_count = ((m_tx_count * 3) + 1) / 2; // TODO: what does 1.5 stop bits do in TxC/1 mode?  the +1 here rounds it up
					break;
				case WR4_STOP_BITS_2:
					m_tx_count *= 2;
					break;
				}
				transmit_complete();
			}
			--m_tx_count;
		}
	}
	m_tx_clock = state;
}
