// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
/***************************************************************************

    68153 BIM Bus Interrupter Module

    The Bus Interrupter Module (BIM) provides an interface between interrupting devices and a system bus such as
    the VMEbus or VERSAbus™. It generates a maximum of 7 bus interrupts on the IRQ1-IRQ7 outputs and responds to
    interrupt acknowledge cycles for up to 4 independent slaves. The BIM can also supply an interrupt vector
    during an interrupt acknowledge cycle. Moreover, it sits in the interrupt acknowledge daisychain which allows
    for multiple interrupts on the level acknowledged.

 ----- Features ----------------------------------------------------------
 x 4 channels
 x Programmable Interrupt Request levels
 x Programmable Interrupt Vectors
 x Daisy Chain support

   Full functionality is implemented

 -------------------------------------------------------------------------
  Level of implementation:  x = done p = partial
 -------------------------------------------------------------------------

*/
#include "emu.h"
#include "68153bim.h"
#include "cpu/m68000/m68000.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)
#define LOG_READ    (1U <<  3)
#define LOG_IACK    (1U <<  4)

//#define VERBOSE ( LOG_SETUP | LOG_INT | LOG_IACK | LOG_GENERAL | LOG_READ)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

//#define LOG(...)      LOGMASKED(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGIACK(...)  LOGMASKED(LOG_IACK,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define CHN0_TAG   "ch0"
#define CHN1_TAG   "ch1"
#define CHN2_TAG   "ch2"
#define CHN3_TAG   "ch3"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
DEFINE_DEVICE_TYPE(MC68153_CHANNEL, bim68153_channel, "bim68153_channel", "68153 BIM channel")
DEFINE_DEVICE_TYPE(MC68153,         bim68153_device,  "m68153bim",        "Motorola MC68153 BIM")
DEFINE_DEVICE_TYPE(EI68C153,        ei68c153_device,  "ei68c153",         "EPIC EI68C153 BIM")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bim68153_device::device_add_mconfig(machine_config &config)
{
	MC68153_CHANNEL(config, m_chn[0], 0);
	MC68153_CHANNEL(config, m_chn[1], 0);
	MC68153_CHANNEL(config, m_chn[2], 0);
	MC68153_CHANNEL(config, m_chn[3], 0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bim68153_device - constructor
//-------------------------------------------------
bim68153_device::bim68153_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock)
	, m_chn{{*this, CHN0_TAG}, {*this, CHN1_TAG}, {*this, CHN2_TAG}, {*this, CHN3_TAG}}
	, m_out_int_cb(*this)
	, m_out_intal0_cb(*this)
	, m_out_intal1_cb(*this)
	, m_out_iackout_cb(*this)
	, m_iackin(ASSERT_LINE)
	, m_irq_level(0)
{
	// FIXME: is the unused 'variant' parameter supposed to be useful for something?
	LOG("%s\n", FUNCNAME);
}

bim68153_device::bim68153_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bim68153_device(mconfig, MC68153, tag, owner, clock, TYPE_MC68153)
{
	LOG("%s\n", FUNCNAME);
}

/* The EPIC EI68C153 is a CMOS implementation that is fully compatible with the bipolar MC68153 from Motorola */
ei68c153_device::ei68c153_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bim68153_device(mconfig, EI68C153, tag, owner, clock, TYPE_EI68C153)
{
	LOG("%s\n", FUNCNAME);
}

//-------------------------------------------------
//  get_channel_index
//-------------------------------------------------
int bim68153_device::get_channel_index(bim68153_channel *ch)
{
	assert(ch == m_chn[CHN_0] || ch == m_chn[CHN_1] || ch == m_chn[CHN_2] || ch == m_chn[CHN_3]);

	if      (ch == m_chn[CHN_0]) return 0;
	else if (ch == m_chn[CHN_1]) return 1;
	else if (ch == m_chn[CHN_2]) return 2;
	else                         return 3;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bim68153_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	// resolve callbacks
	m_out_int_cb.resolve_safe();
	m_out_intal0_cb.resolve_safe();
	m_out_intal1_cb.resolve_safe();
	m_out_iackout_cb.resolve_safe(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void bim68153_device::device_reset()
{
	LOG("%s %s \n",tag(), FUNCNAME);

	// Reset on all channels
	for (auto & elem : m_chn)
		elem->reset();
}

/*
 * Interrupts
 * The BIM accepts device interrupt requests on inputs INT0, INT1, INT2 and INT3. Each input is regulated by
 * Bit 4 (IRE) of the associated control register (CRO controls INT0, CR! controls INT1,etc.). If IRE (Interrupt
 * Enable) is set and a device input is asserted, an Interrupt Request open-collector output (IRQ1 - IRQ7)
 * is asserted. The asserted IRQX output is selected by the value programmed in Bits 0, 1, and 2 of the control
 * register (L0, L1, and L3). This 3-bit field determines the interrupt request level as set by software.
 *
 * Two or more interrupt sources can be programmed to the same request level. The corresponding IRQX output
 * will remain asserted until multiple interrupt acknowledge cycles respond to all requests.
 *
 * If the interrupt request level is set to zero, the interrupt is disabled because there is no corresponding IRQ output.
*/

/*
  The response of an interrupt Handler to a bus interrupt request is an interrupt acknowledge cycle. The IACK
  cycle is initiated in BIM by receiving IACK low R/W, A1, A2, A3 are latched, and the interrupt level on line A1-A3
  is compared with any interrupt requests pending in the chip. Further activity can be one of four cases.*/
#define MAX_VECTOR 255
IRQ_CALLBACK_MEMBER(bim68153_device::iack)
{
	int vec = M68K_INT_ACK_AUTOVECTOR;
	int found = 0;
	//  int level = 0;
	int ch = -1;

	LOGIACK("%s %s()\n", tag(), FUNCNAME);

	/* 1. No further action required — This occurs if IACKIN is not asserted. Asserting IACKN only starts the BIM activity.
	 *    If the daisy chain signal never reaches the BIM (IACKIN is not asserted), another interrupter has responded to the
	 *    IACK cycle. The cycle will end, the IACK is negated, and no additional action is required. */
	if (m_iackin == CLEAR_LINE)
	{
		LOGIACK(" - IRQ cleared due to IACKIN\n");
		m_out_iackout_cb(CLEAR_LINE);
		m_out_int_cb(CLEAR_LINE);  // should really be tristated
		return MAX_VECTOR + 1; // This is a 68K emulation specific response and will terminate the iack cycle
	}

	for (auto & elem : m_chn)
	{   // If this channel has interrupts enabled and pending
		if (elem->m_int_state == bim68153_channel::PENDING && elem->m_control & bim68153_channel::REG_CNTRL_INT_ENABLE)
		{   // and the level matches
			if ((elem->m_control & bim68153_channel::REG_CNTRL_INT_LVL_MSK) == irqline)
			{   // then remember it
				ch = get_channel_index(elem);
				found = 1;
			}
		}
	}
	/* 2. Pass on the interrupt daisy chain — For this case, IACKIN input is asserted by the preceding daisy chain interrupter,
	 * and IACKOUT output is in turn asserted. The daisy chain signal is passed on when no interrupts are pending on a matching
	 * level or when any possible interrupts are disabled. The Interrupt Enable (IRE) bit of a control register can disable any
	 * interrupt requests, and in turn, any possible matches */
	if (found == 0)
	{
		m_out_iackout_cb(CLEAR_LINE); // No more interrupts to serve, pass the message to next device in daisy chain
		m_out_int_cb(CLEAR_LINE); // should really be tristated but board driver must make sure to mitigate if this is a problem
		return MAX_VECTOR + 1; // This is a 68K emulation specific response and will terminate the iack cycle
	}

	m_irq_level = m_chn[ch]->m_control & bim68153_channel::REG_CNTRL_INT_LVL_MSK;

	if ((m_chn[ch]->m_control & bim68153_channel::REG_CNTRL_INT_EXT) == 0)
	{
		/* 3. Respond internally - For this case, IACKIN is asserted and a match is found. The BIM completes the IACK cycle by
		 * supplying an interrupt vector from the proper vector register followed by a DTACK signal asserted because the interrupt
		 * acknowledge cycle is completed by this device. For the BIM to respond in this mode of operation, the EXTERNAL/INTERNAl
		 * control register bit (X/IN) must be zero. For each source of interrupt request, the associated control register determines
		 * the BIM response to an IACK cycle, and the X/IN bit sets this response either internally (X/IN = 0 ) or externally (X/IN = 1). */

		vec = m_chn[ch]->m_vector; // Internal vector
	}
	else
	{
		/* 4. Respond externally — For the final case, IACKIN is also asserted, a match is found and the associated control register has
		 * X/IN bit set to one. The BIM does not assert IACKOUT and does assert INTAE low.INTAE signals that the requesting device must
		 * complete the IACK cycle (supplying a vector and DTACK) and that the 2-bit code contained on outputs INTALO and INTAL1 shows
		 * which interrupt source is being acknowledged*/

		vec = m_chn[ch]->m_out_iack_cb();  // External vector

		/* Also support INTAL0 and INTAL1 in case additional logic relies on it */
		m_out_intal0_cb( ch       & 1);
		m_out_intal1_cb((ch >> 1) & 1);
		/* TODO: Figure out a way to update the vector in case INTAL0/INTAL1 is involved creating it */
	}
	LOGIACK(" - Interrupt Acknowledge Vector %02x, next interrupt is off %02x\n", vec, m_irq_level);
	if (m_chn[ch]->m_control & bim68153_channel::REG_CNTRL_INT_AUT_DIS)
	{
		LOGIACK(" - Interrupts on channel %d disabled due to the IRAC (Auto Clear) bit is set\n", ch);
		m_chn[ch]->m_control &= ~bim68153_channel::REG_CNTRL_INT_ENABLE;
	}

	// This is not explicitly said in the 68153 datasheet but what would a flag be used for otherwise?
	m_chn[ch]->m_control &= ~bim68153_channel::REG_CNTRL_INT_FLAG;

	m_out_iackout_cb(CLEAR_LINE);
	m_out_int_cb(CLEAR_LINE);
	return vec;
}

int bim68153_device::get_irq_level()
{
	LOGINT("%s %s() - %02x\n", tag(), FUNCNAME, m_irq_level);
	return m_irq_level;
}

//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------
void bim68153_device::trigger_interrupt(int ch)
{
	LOGINT("%s %s CHN:%d\n",FUNCNAME, tag(), ch);

	if (!(m_chn[ch]->m_control & bim68153_channel::REG_CNTRL_INT_ENABLE))
	{
		LOGINT("Interrupt Enable for channel %d is not set, blocking attempt to interrupt\n", ch);
		return;
	}

	m_irq_level = (m_chn[ch]->m_control & bim68153_channel::REG_CNTRL_INT_LVL_MSK);

	// trigger interrupt
	m_chn[ch]->m_int_state = bim68153_channel::PENDING;

	// assert daisy chain
	m_out_iackout_cb(ASSERT_LINE);

	// Set flag
	// This is not explicitly said in the 68153 datasheet but what would a flag be used for otherwise?
	m_chn[ch]->m_control |= bim68153_channel::REG_CNTRL_INT_FLAG;

	// assert interrupt
	m_out_int_cb(ASSERT_LINE);
}

//-------------------------------------------------
//  read
//-------------------------------------------------
READ8_MEMBER( bim68153_device::read )
{
	int vc = offset & REG_VECTOR;
	int ch = offset & CHN_MSK;

	LOGR(" * %s %d Reg %s -> %02x  \n", tag(), ch, vc ? "vector" : "control",
			 vc ? m_chn[ch]->do_bimreg_vector_r() : m_chn[ch]->do_bimreg_control_r());

	return vc ? m_chn[ch]->do_bimreg_vector_r() : m_chn[ch]->do_bimreg_control_r();
}

//-------------------------------------------------
//  write
//-------------------------------------------------
WRITE8_MEMBER( bim68153_device::write )
{
	int vc = offset & REG_VECTOR;
	int ch = offset & CHN_MSK;

	LOGSETUP(" * %s %d Reg %s <- %02x  \n", tag(), ch, vc ? "vector" : "control", data);

	if (vc)
		m_chn[ch]->do_bimreg_vector_w(data);
	else
		m_chn[ch]->do_bimreg_control_w(data);
}

//**************************************************************************
//  BIM CHANNEL
//**************************************************************************

bim68153_channel::bim68153_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC68153_CHANNEL, tag, owner, clock)
	, m_out_iack_cb(*this)
	, m_int_state(NONE)
	, m_control(0)
	, m_vector(0)
{
	LOG("%s\n",FUNCNAME);
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void bim68153_channel::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);
	m_bim = downcast<bim68153_device *>(owner());
	m_index = m_bim->get_channel_index(this);

	// state saving
	save_item(NAME(m_control));
	save_item(NAME(m_vector));
	save_item(NAME(m_int_state));

	// Resolve callbacks
	m_out_iack_cb.resolve_safe(0);
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void bim68153_channel::device_reset()
{
	LOGSETUP("%s\n", FUNCNAME);

	// Reset all registers
	m_control = 0;
	m_vector = 0x0f;
	m_int_state = NONE;
}

/* Trigger an interrupt */
WRITE_LINE_MEMBER( bim68153_channel::int_w )
{
	LOGINT("%s Ch %d: %s\n",FUNCNAME, m_index, state == CLEAR_LINE ? "Cleared" : "Asserted");
	if (state == ASSERT_LINE)
	{
		m_bim->trigger_interrupt(m_index);
	}
}

uint8_t bim68153_channel::do_bimreg_control_r()
{
	LOG("%s ch %d returns %02x\n", FUNCNAME, m_index, m_control);
	return m_control;
}

uint8_t bim68153_channel::do_bimreg_vector_r()
{
	LOG("%s ch %d returns %02x\n", FUNCNAME, m_index, m_vector);
	return m_vector;
}

void bim68153_channel::do_bimreg_control_w(uint8_t data)
{
	LOG("%s ch %d set control to %02x\n", FUNCNAME, m_index, data);
	LOGSETUP(" - Lev:%d Auto Disable:%d Int Enable:%d Vector:%d Auto Clear:%d Flag:%d\n",
			 data & REG_CNTRL_INT_LVL_MSK,
			 data & REG_CNTRL_INT_AUT_DIS ? 1 : 0,
			 data & REG_CNTRL_INT_ENABLE  ? 1 : 0,
			 data & REG_CNTRL_INT_EXT     ? 1 : 0,
			 data & REG_CNTRL_INT_AUT_CLR ? 1 : 0,
			 data & REG_CNTRL_INT_FLAG    ? 1 : 0);

	m_control = data;
}

void bim68153_channel::do_bimreg_vector_w(uint8_t data)
{
	LOG("%s ch %d set vector to %02x\n", FUNCNAME, m_index, data);
	m_vector = data;
}
