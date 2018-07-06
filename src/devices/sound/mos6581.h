// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Curt Coder
/**********************************************************************

    MOS 6581/8580 Sound Interface Device emulation

**********************************************************************
                            _____   _____
                 CAP1A   1 |*    \_/     | 28  Vdd
                 CAP1B   2 |             | 27  AUDIO OUT
                 CAP2A   3 |             | 26  EXT IN
                 CAP2B   4 |             | 25  Vcc
                  _RES   5 |             | 24  POTX
                  phi2   6 |             | 23  POTY
                  R/_W   7 |   MOS6581   | 22  D7
                   _CS   8 |   MOS8580   | 21  D6
                    A0   9 |             | 20  D5
                    A1  10 |             | 19  D4
                    A2  11 |             | 18  D3
                    A3  12 |             | 17  D2
                    A4  13 |             | 16  D1
                   GND  14 |_____________| 15  D0

**********************************************************************/

#ifndef MAME_SOUND_MOS6581_H
#define MAME_SOUND_MOS6581_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6581_POTX_CALLBACK(_read) \
	downcast<mos6581_device &>(*device).set_potx_rd_callback(DEVCB_##_read);

#define MCFG_MOS6581_POTY_CALLBACK(_read) \
	downcast<mos6581_device &>(*device).set_poty_rd_callback(DEVCB_##_read);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6581_device

struct SID6581_t;

class mos6581_device : public device_t, public device_sound_interface
{
public:
	// used by the actual SID emulator
	enum
	{
		TYPE_6581,
		TYPE_8580
	};

	mos6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~mos6581_device();

	template <class Object> devcb_base &set_potx_rd_callback(Object &&cb) { return m_read_potx.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_poty_rd_callback(Object &&cb) { return m_read_poty.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	mos6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	devcb_read8  m_read_potx;
	devcb_read8  m_read_poty;

	sound_stream *m_stream;

	int const m_variant;

	std::unique_ptr<SID6581_t> m_token;
};


// ======================> mos8580_device

class mos8580_device : public mos6581_device
{
public:
	mos8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(MOS6581, mos6581_device)
DECLARE_DEVICE_TYPE(MOS8580, mos8580_device)

#endif // MAME_SOUND_MOS6581_H
