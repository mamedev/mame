// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    68561mpcc.c

    Rockwell 68561 MPCC (Multi Protocol Communications Controller)

    skeleton driver, just enough for besta.c console to work

*********************************************************************/


#include "emu.h"
#include "68561mpcc.h"

const device_type MPCC68561 = &device_creator<mpcc68561_t>;


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_MPCC    (1)

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

mpcc68561_t::mpcc68561_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MPCC68561, "Rockwell 68561 MPCC", tag, owner, clock, "mpcc68561", __FILE__), mode(0), reg(0), status(0), IRQV(0), MasterIRQEnable(0), lastIRQStat(0), IRQType(),
	intrq_cb(*this)
{
}

/*-------------------------------------------------
    mpcc_updateirqs
-------------------------------------------------*/

void mpcc68561_t::updateirqs()
{
	int irqstat;

	irqstat = 0;
	if (MasterIRQEnable)
	{
		if ((channel[0].txIRQEnable) && (channel[0].txIRQPending))
		{
			IRQType = IRQ_B_TX;
			irqstat = 1;
		}
		else if ((channel[1].txIRQEnable) && (channel[1].txIRQPending))
		{
			IRQType = IRQ_A_TX;
			irqstat = 1;
		}
		else if ((channel[0].extIRQEnable) && (channel[0].extIRQPending))
		{
			IRQType = IRQ_B_EXT;
			irqstat = 1;
		}
		else if ((channel[1].extIRQEnable) && (channel[1].extIRQPending))
		{
			IRQType = IRQ_A_EXT;
			irqstat = 1;
		}
	}
	else
	{
		IRQType = IRQ_NONE;
	}

//  printf("mpcc: irqstat %d, last %d\n", irqstat, lastIRQStat);
//  printf("ch0: en %d pd %d  ch1: en %d pd %d\n", channel[0].txIRQEnable, channel[0].txIRQPending, channel[1].txIRQEnable, channel[1].txIRQPending);

	// don't spam the driver with unnecessary transitions
	if (irqstat != lastIRQStat)
	{
		lastIRQStat = irqstat;

		// tell the driver the new IRQ line status if possible
#if LOG_MPCC
		printf("mpcc68561 IRQ status => %d\n", irqstat);
#endif
		if(!intrq_cb.isnull())
			intrq_cb(irqstat);
	}
}

/*-------------------------------------------------
    mpcc_initchannel
-------------------------------------------------*/
void mpcc68561_t::initchannel(int ch)
{
	channel[ch].syncHunt = 1;
}

/*-------------------------------------------------
    mpcc_resetchannel
-------------------------------------------------*/
void mpcc68561_t::resetchannel(int ch)
{
	emu_timer *timersave = channel[ch].baudtimer;

	memset(&channel[ch], 0, sizeof(Chan));

	channel[ch].txUnderrun = 1;
	channel[ch].baudtimer = timersave;

	channel[ch].baudtimer->adjust(attotime::never, ch);
}

/*-------------------------------------------------
    mpcc68561_baud_expire - baud rate timer expiry
-------------------------------------------------*/

void mpcc68561_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	Chan *pChan = &channel[id];
	int brconst = pChan->reg_val[13]<<8 | pChan->reg_val[14];
	int rate;

	if (brconst)
	{
		rate = clock() / brconst;
	}
	else
	{
		rate = 0;
	}

	// is baud counter IRQ enabled on this channel?
	// always flag pending in case it's enabled after this
	pChan->baudIRQPending = 1;
	if (pChan->baudIRQEnable)
	{
		if (pChan->extIRQEnable)
		{
			pChan->extIRQPending = 1;
			pChan->baudIRQPending = 0;
			updateirqs();
		}
	}

	// reset timer according to current register values
	if (rate)
	{
		timer.adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
	}
	else
	{
		timer.adjust(attotime::never, 0, attotime::never);
	}
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void mpcc68561_t::device_start()
{
	intrq_cb.resolve_safe();

	memset(channel, 0, sizeof(channel));

	mode = 0;
	reg = 0;
	status = 0;
	IRQV = 0;
	MasterIRQEnable = 0;
	lastIRQStat = 0;
	IRQType = IRQ_NONE;

	channel[0].baudtimer = timer_alloc(0);
}


/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/
void mpcc68561_t::device_reset()
{
	IRQType = IRQ_NONE;
	MasterIRQEnable = 0;
	IRQV = 0;

	initchannel(0);
	resetchannel(0);
}

/*-------------------------------------------------
    mpcc_set_status
-------------------------------------------------*/

void mpcc68561_t::set_status(int _status)
{
	status = _status;
}

/*-------------------------------------------------
    mpcc_acknowledge
-------------------------------------------------*/

void mpcc68561_t::acknowledge()
{
	if(!intrq_cb.isnull())
		intrq_cb(0);
}

/*-------------------------------------------------
    mpcc_getreg
-------------------------------------------------*/

UINT8 mpcc68561_t::getreg()
{
	/* Not yet implemented */
	#if LOG_MPCC
	printf("mpcc: port A reg %d read 0x%02x\n", reg, channel[0].reg_val[reg]);
	#endif

	if (reg == 0)
	{
		UINT8 rv = 0;

		Chan *ourCh = &channel[0];

		rv |= (ourCh->txUnderrun) ? 0x40 : 0;
		rv |= (ourCh->syncHunt) ? 0x10 : 0;
		rv |= channel[0].reg_val[0] & 0x05; // pick up TXBE and RXBF bits

		return rv;
	}
	else if (reg == 10)
	{
		return 0;
	}
	return channel[0].reg_val[reg];
}

/*-------------------------------------------------
    mpcc_putreg
-------------------------------------------------*/

void mpcc68561_t::putreg(int ch, UINT8 data)
{
	Chan *pChan = &channel[ch];

	channel[ch].reg_val[reg] = data;
	#if LOG_MPCC
	printf("mpcc: port %c reg %d write 0x%02x\n", 'A'+ch, reg, data);
	#endif

	switch (reg)
	{
		case 0: // command register
			switch ((data >> 3) & 7)
			{
				case 1: // select high registers (handled elsewhere)
					break;

				case 2: // reset external and status IRQs
					pChan->syncHunt = 0;
					break;

				case 5: // ack Tx IRQ
					pChan->txIRQPending = 0;
					updateirqs();
					break;

				case 0: // nothing
				case 3: // send SDLC abort
				case 4: // enable IRQ on next Rx byte
				case 6: // reset errors
				case 7: // reset highest IUS
					// we don't handle these yet
					break;

			}
			break;

		case 1: // Tx/Rx IRQ and data transfer mode defintion
			pChan->extIRQEnable = (data & 1);
			pChan->txIRQEnable = (data & 2) ? 1 : 0;
			pChan->rxIRQEnable = (data >> 3) & 3;
			updateirqs();
			break;

		case 2: // IRQ vector
			IRQV = data;
			break;

		case 3: // Rx parameters and controls
			pChan->rxEnable = (data & 1);
			pChan->syncHunt = (data & 0x10) ? 1 : 0;
			break;

		case 5: // Tx parameters and controls
//          printf("ch %d TxEnable = %d [%02x]\n", ch, data & 8, data);
			pChan->txEnable = data & 8;

			if (pChan->txEnable)
			{
				pChan->reg_val[0] |= 0x04;  // Tx empty
			}
			break;

		case 4: // Tx/Rx misc parameters and modes
		case 6: // sync chars/SDLC address field
		case 7: // sync char/SDLC flag
			break;

		case 9: // master IRQ control
			MasterIRQEnable = (data & 8) ? 1 : 0;
			updateirqs();

			// channel reset command
			switch ((data>>6) & 3)
			{
				case 0: // do nothing
					break;

				case 1: // reset channel B
					resetchannel(0);
					break;

				case 3: // force h/w reset (entire chip)
					IRQType = IRQ_NONE;
					MasterIRQEnable = 0;
					IRQV = 0;

					initchannel(0);
					resetchannel(0);

					// make sure we stop yanking the IRQ line if we were
					updateirqs();
					break;

			}
			break;

		case 10:    // misc transmitter/receiver control bits
		case 11:    // clock mode control
		case 12:    // lower byte of baud rate gen
		case 13:    // upper byte of baud rate gen
			break;

		case 14:    // misc control bits
			if (data & 0x01)    // baud rate generator enable?
			{
				int brconst = pChan->reg_val[13]<<8 | pChan->reg_val[14];
				int rate = clock() / brconst;

				pChan->baudtimer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
			}
			break;

		case 15:    // external/status interrupt control
			pChan->baudIRQEnable = (data & 2) ? 1 : 0;
			pChan->DCDEnable = (data & 8) ? 1 : 0;
			pChan->CTSEnable = (data & 0x20) ? 1 : 0;
			pChan->txUnderrunEnable = (data & 0x40) ? 1 : 0;
			break;
	}
}

/*-------------------------------------------------
    mpcc68561_get_reg_a
-------------------------------------------------*/

UINT8 mpcc68561_t::get_reg_a(int reg)
{
	return channel[0].reg_val[reg];
}



/*-------------------------------------------------
    mpcc68561_set_reg_a
-------------------------------------------------*/

void mpcc68561_t::set_reg_a(int reg, UINT8 data)
{
	channel[0].reg_val[reg] = data;
}



/*-------------------------------------------------
    mpcc68561_r
-------------------------------------------------*/

READ8_MEMBER( mpcc68561_t::reg_r)
{
	UINT8 result = 0;

	offset %= 4;

	switch(offset)
	{
		case 1:
			/* Channel A (Modem Port) Control */
			if (mode == 1)
				mode = 0;
			else
				reg = 0;

			result = getreg();
			break;

		case 3:
			/* Channel A (Modem Port) Data */
			return channel[0].rxData;
			break;
	}
	return result;
}



/*-------------------------------------------------
    mpcc68561_w
-------------------------------------------------*/

WRITE8_MEMBER( mpcc68561_t::reg_w )
{
	Chan *pChan;

	offset &= 3;

//  printf(" mode %d data %x offset %d  \n", mode, data, offset);

	switch(offset)
	{
		case 1:
			/* Channel A (Modem Port) Control */
			if (mode == 0)
			{
				if((data & 0xf0) == 0)  // not a reset command
				{
					mode = 1;
					reg = data & 0x0f;
//                  putareg(data & 0xf0);
				}
				else if (data == 0x10)
				{
					pChan = &channel[0];
					// clear ext. interrupts
					pChan->extIRQPending = 0;
					pChan->baudIRQPending = 0;
					updateirqs();
				}
			}
			else
			{
				mode = 0;
				putreg(0, data);
			}
			break;

		case 3:
			/* Channel A (Modem Port) Data */
			pChan = &channel[0];

			if (pChan->txEnable)
			{
				pChan->txData = data;
				// local loopback?
				if (pChan->reg_val[14] & 0x10)
				{
					pChan->rxData = data;
					pChan->reg_val[0] |= 0x01;  // Rx character available
				}
				pChan->reg_val[1] |= 0x01;  // All sent
				pChan->reg_val[0] |= 0x04;  // Tx empty
				pChan->txUnderrun = 1;
				pChan->txIRQPending = 1;
				updateirqs();
			}
			break;
	}
}
