// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Seibu Sound System v1.02, games using this include:

    Cross Shooter    1987   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Cabal            1988   * "Michel/Seibu    sound 11/04/88" (YM2151 substituted for YM3812, unknown ADPCM)
    Dead Angle       1988   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (2xYM2203 substituted for YM3812, unknown ADPCM)
    Dynamite Duke    1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden           1990   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Blood Brothers   1990     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    D-Con            1992     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    Related sound programs (not implemented yet):

    Zero Team                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Legionnaire               "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Raiden 2                  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Raiden DX                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Cup Soccer                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    SD Gundam Psycho Salamander "Copyright by King Bee Sol 1991"
    * = encrypted

***************************************************************************/
#ifndef MAME_AUDIO_SEIBU_H
#define MAME_AUDIO_SEIBU_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/okiadpcm.h"

class seibu_sound_common {
public:
	virtual ~seibu_sound_common() = default;
	void seibu_sound_map(address_map &map);
};

class seibu_sound_device : public device_t
{
public:
	seibu_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~seibu_sound_device() {}

	// static configuration
	static void static_set_cpu_tag(device_t &device, const char *tag);
	static void static_set_rombank_tag(device_t &device, const char *tag);
	template<class _Object> static devcb_base &set_ym_read_callback(device_t &device, _Object object)  { return downcast<seibu_sound_device &>(device).m_ym_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_ym_write_callback(device_t &device, _Object object) { return downcast<seibu_sound_device &>(device).m_ym_write_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( main_r );
	DECLARE_WRITE8_MEMBER( main_w );
	DECLARE_WRITE16_MEMBER( main_mustb_w );
	DECLARE_WRITE8_MEMBER( irq_clear_w );
	DECLARE_WRITE8_MEMBER( rst10_ack_w );
	DECLARE_WRITE8_MEMBER( rst18_ack_w );
	DECLARE_READ8_MEMBER( ym_r );
	DECLARE_WRITE8_MEMBER( ym_w );
	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_WRITE8_MEMBER( coin_w );
	WRITE_LINE_MEMBER( fm_irqhandler );
	DECLARE_READ8_MEMBER( soundlatch_r );
	DECLARE_READ8_MEMBER( main_data_pending_r );
	DECLARE_WRITE8_MEMBER( main_data_w );
	DECLARE_WRITE8_MEMBER( pending_w );

	void update_irq_lines(int param);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// device callbacks
	devcb_read8 m_ym_read_cb;
	devcb_write8 m_ym_write_cb;

	// internal state
	required_device<cpu_device> m_sound_cpu;
	optional_region_ptr<uint8_t> m_sound_rom;
	optional_memory_bank m_rom_bank;
	uint8_t m_main2sub[2];
	uint8_t m_sub2main[2];
	int m_main2sub_pending;
	int m_sub2main_pending;
	uint8_t m_rst10_irq;
	uint8_t m_rst18_irq;

	enum
	{
		VECTOR_INIT,
		RST10_ASSERT,
		RST10_CLEAR,
		RST18_ASSERT,
		RST18_CLEAR
	};
};

DECLARE_DEVICE_TYPE(SEIBU_SOUND, seibu_sound_device)


// SEI80BU (Z80 program decryption)

class sei80bu_device : public device_t, public device_rom_interface
{
public:
	sei80bu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_READ8_MEMBER(opcode_r);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void rom_bank_updated() override { }
};

DECLARE_DEVICE_TYPE(SEI80BU, sei80bu_device)

// Seibu ADPCM device

class seibu_adpcm_device : public device_t,
									public device_sound_interface
{
public:
	seibu_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~seibu_adpcm_device() {}

	void decrypt();
	DECLARE_WRITE8_MEMBER( adr_w );
	DECLARE_WRITE8_MEMBER( ctl_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	oki_adpcm_state m_adpcm;
	sound_stream *m_stream;
	uint32_t m_current;
	uint32_t m_end;
	uint8_t m_nibble;
	uint8_t m_playing;
	required_region_ptr<uint8_t> m_base;
};

DECLARE_DEVICE_TYPE(SEIBU_ADPCM, seibu_adpcm_device)

/**************************************************************************/

#define SEIBU_COIN_INPUTS                                           \
	PORT_START("COIN")                                              \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(4)     \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(4)     \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                    \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                    \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )                    \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )                    \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )                    \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

#define SEIBU_COIN_INPUTS_INVERT                                    \
	PORT_START("COIN")                                              \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(4)      \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(4)      \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                     \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                     \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                     \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                     \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                     \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

#define MCFG_SEIBU_SOUND_CPU(_audiocputag) \
	seibu_sound_device::static_set_cpu_tag(*device, "^" _audiocputag);

#define MCFG_SEIBU_SOUND_ROMBANK(_banktag) \
	seibu_sound_device::static_set_rombank_tag(*device, "^" _banktag);

#define MCFG_SEIBU_SOUND_YM_READ_CB(_devcb) \
	devcb = &seibu_sound_device::set_ym_read_callback(*device, DEVCB_##_devcb);

#define MCFG_SEIBU_SOUND_YM_WRITE_CB(_devcb) \
	devcb = &seibu_sound_device::set_ym_write_callback(*device, DEVCB_##_devcb);

/**************************************************************************/

#endif // MAME_AUDIO_SEIBU_H
