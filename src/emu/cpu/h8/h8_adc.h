/***************************************************************************

    h8_adc.h

    H8 Analog to Digital Converter subsystem

****************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __H8_ADC_H__
#define __H8_ADC_H__

#include "h8.h"
#include "h8_intc.h"

#define MCFG_H8_ADC_GENERIC_ADD( _tag, _type, intc, vect )  \
	MCFG_DEVICE_ADD( _tag, _type, 0 ) \
	downcast<h8_adc_device *>(device)->set_info(intc, vect);

#define MCFG_H8_ADC_3337_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_3337, intc, vect )

#define MCFG_H8_ADC_3006_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_3006, intc, vect )

#define MCFG_H8_ADC_2245_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_2245, intc, vect )

#define MCFG_H8_ADC_2320_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_2320, intc, vect )

#define MCFG_H8_ADC_2357_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_2357, intc, vect )

#define MCFG_H8_ADC_2655_ADD( _tag, intc, vect ) \
	MCFG_H8_ADC_GENERIC_ADD( _tag, H8_ADC_2655, intc, vect )

class h8_adc_device : public device_t {
public:
	h8_adc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void set_info(const char *intc_tag, int vect);

	DECLARE_READ8_MEMBER(addr8_r);
	DECLARE_READ16_MEMBER(addr16_r);
	DECLARE_READ8_MEMBER(adcsr_r);
	DECLARE_READ8_MEMBER(adcr_r);
	DECLARE_WRITE8_MEMBER(adcsr_w);
	DECLARE_WRITE8_MEMBER(adcr_w);
	DECLARE_WRITE_LINE_MEMBER(adtrg_w);

	void set_suspend(bool suspend);
	UINT64 internal_update(UINT64 current_time);

protected:
	required_device<h8_device> cpu;
	h8_intc_device *intc;
	address_space *io;
	const char *intc_tag;
	int intc_vector;

	enum {
		T_SOFT  = 1<<0,
		T_TPU   = 1<<1,
		T_TIMER = 1<<2,
		T_EXT   = 1<<3
	};

	enum {
		F_ADF  = 0x80,
		F_ADIE = 0x40,
		F_ADST = 0x20
	};

	enum {
		IDLE = 0,
		ACTIVE = 1,
		HALTED = 2,
		REPEAT = 4,
		ROTATE = 8,
		DUAL = 16,
		BUFFER = 32,
		COUNTED = 64
	};

	UINT16 addr[8], buf[2];
	UINT8 adcsr, adcr;
	int register_mask;
	int trigger, start_mode, start_channel, end_channel, start_count;
	bool suspend_on_interrupt, analog_power_control;
	int mode, channel, count;
	bool analog_powered, adtrg;
	UINT64 next_event;

	virtual void device_start();
	virtual void device_reset();

	void sampling();
	void start_conversion();
	void conversion_wait(bool first, bool poweron, UINT64 current_time = 0);
	void buffer_value(int port, int buffer = 0);
	void commit_value(int reg, int buffer = 0);
	void timeout(UINT64 current_time);
	void done();

	virtual int conversion_time(bool first, bool poweron) = 0;
	virtual void mode_update() = 0;
	virtual void do_buffering(int buffer);
	virtual int get_channel_index(int count);
};

class h8_adc_3337_device : public h8_adc_device {
public:
	h8_adc_3337_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
};

class h8_adc_3006_device : public h8_adc_device {
public:
	h8_adc_3006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
};

class h8_adc_2245_device : public h8_adc_device {
public:
	h8_adc_2245_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
};

class h8_adc_2320_device : public h8_adc_device {
public:
	h8_adc_2320_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
};

class h8_adc_2357_device : public h8_adc_device {
public:
	h8_adc_2357_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
};

class h8_adc_2655_device : public h8_adc_device {
public:
	h8_adc_2655_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int conversion_time(bool first, bool poweron);
	virtual void mode_update();
	virtual void do_buffering(int buffer);
	virtual int get_channel_index(int count);
};

extern const device_type H8_ADC_3337;
extern const device_type H8_ADC_3006;
extern const device_type H8_ADC_2245;
extern const device_type H8_ADC_2320;
extern const device_type H8_ADC_2357;
extern const device_type H8_ADC_2655;

#endif
