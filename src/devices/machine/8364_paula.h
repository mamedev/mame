// license: BSD-3-Clause
// copyright-holders: Aaron Giles, Dirk Best
/***************************************************************************

    Commodore 8364 "Paula"

    Multi-purpose chip that is part of the Amiga chipset. The name "Paula"
    is derived from "Ports, Audio, UART and Logic". It features 4-channel
    DMA driven audio, the floppy controller, a serial receiver/transmitter,
    analog inputs and contains the interrupt controller.

                ____ ____
        D8   1 |*   u    | 48  D9
        D7   2 |         | 47  D10
        D6   3 |         | 46  D11
        D5   4 |         | 45  D12
        D4   5 |         | 44  D13
        D3   6 |         | 43  D14
        D2   7 |         | 42  D15
       GND   8 |         | 41  RXD
        D1   9 |         | 40  TXD
        D0  10 |         | 39  DKWE
      /RES  11 |         | 38  /DKWD
      DMAL  12 |         | 37  /DKRD
     /IPL0  13 |         | 36  POT1Y
     /IPL1  14 |         | 35  POT1X
     /IPL2  15 |         | 34  ANAGND
     /INT2  16 |         | 33  POT0Y
     /INT3  17 |         | 32  POT0X
     /INT6  18 |         | 31  AUDL
      RGA8  19 |         | 30  AUDR
      RGA7  20 |         | 29  CCKQ
      RGA6  21 |         | 28  CCK
      RGA5  22 |         | 27  VCC
      RGA4  23 |         | 26  RGA1
      RGA3  24 |_________| 25  RGA2

***************************************************************************/

#ifndef MAME_MACHINE_8364_PAULA_H
#define MAME_MACHINE_8364_PAULA_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> paula_8364_device

class paula_8364_device : public device_t, public device_sound_interface
{
public:
	paula_8364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto mem_read_cb() { return m_mem_r.bind(); }
	auto int_cb() { return m_int_w.bind(); }

	DECLARE_READ16_MEMBER(reg_r);
	DECLARE_WRITE16_MEMBER(reg_w);

	void update();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum
	{
		CHAN_0 = 0,
		CHAN_1 = 1,
		CHAN_2 = 2,
		CHAN_3 = 3
	};

	enum
	{
		REG_DMACONR = 0x02/2,
		REG_ADKCONR = 0x10/2,
		REG_DMACON  = 0x96/2,
		REG_INTREQ  = 0x9c/2,
		REG_ADKCON  = 0x9e/2,
		REG_AUD0LCH = 0xa0/2,  // to be moved, not part of paula
		REG_AUD0LCL = 0xa2/2,  // to be moved, not part of paula
		REG_AUD0LEN = 0xa4/2,
		REG_AUD0PER = 0xa6/2,
		REG_AUD0VOL = 0xa8/2,
		REG_AUD0DAT = 0xaa/2,
		REG_AUD1LCH = 0xb0/2,  // to be moved, not part of paula
		REG_AUD1LCL = 0xb2/2,  // to be moved, not part of paula
		REG_AUD1LEN = 0xb4/2,
		REG_AUD1PER = 0xb6/2,
		REG_AUD1VOL = 0xb8/2,
		REG_AUD1DAT = 0xba/2,
		REG_AUD2LCH = 0xc0/2,  // to be moved, not part of paula
		REG_AUD2LCL = 0xc2/2,  // to be moved, not part of paula
		REG_AUD2LEN = 0xc4/2,
		REG_AUD2PER = 0xc6/2,
		REG_AUD2VOL = 0xc8/2,
		REG_AUD2DAT = 0xca/2,
		REG_AUD3LCH = 0xd0/2,  // to be moved, not part of paula
		REG_AUD3LCL = 0xd2/2,  // to be moved, not part of paula
		REG_AUD3LEN = 0xd4/2,
		REG_AUD3PER = 0xd6/2,
		REG_AUD3VOL = 0xd8/2,
		REG_AUD3DAT = 0xda/2
	};

	static constexpr int CLOCK_DIVIDER = 16;

	struct audio_channel
	{
		emu_timer *irq_timer;
		uint32_t curlocation;
		uint16_t curlength;
		uint16_t curticks;
		uint8_t index;
		bool dma_enabled;
		bool manualmode;
		int8_t latched;

		// custom chip registers
		uint32_t loc;  // to be moved, not part of paula
		uint16_t len;
		uint16_t per;
		uint16_t vol;
		uint16_t dat;
	};

	void dma_reload(audio_channel *chan);

	// callbacks
	devcb_read16 m_mem_r;
	devcb_write_line m_int_w;

	// internal state
	uint16_t m_dmacon;
	uint16_t m_adkcon;

	audio_channel m_channel[4];
	sound_stream *m_stream;

	TIMER_CALLBACK_MEMBER( signal_irq );
};

// device type definition
DECLARE_DEVICE_TYPE(PAULA_8364, paula_8364_device)

#endif // MAME_DEVICES_MACHINE_8364_PAULA_H
