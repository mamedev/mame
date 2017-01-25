// license: GPL-2.0+
// copyright-holders: Dirk Best
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

#pragma once

#ifndef MAME_DEVICES_MACHINE_8364_PAULA_H
#define MAME_DEVICES_MACHINE_8364_PAULA_H

#include "emu.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> paula_8364_device

class paula_8364_device : public device_t, public device_sound_interface
{
public:
	paula_8364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~paula_8364_device() {}

	void update();
	void data_w(int which, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	static const int CLOCK_DIVIDER = 16;

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
	};

	void dma_reload(audio_channel *chan);

	// internal state
	audio_channel m_channel[4];
	sound_stream *m_stream;

	TIMER_CALLBACK_MEMBER( signal_irq );
};

// device type definition
extern const device_type PAULA_8364;

#endif // MAME_DEVICES_MACHINE_8364_PAULA_H
