// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Seibu Sound System v1.02, games using this include:

    Cross Shooter    1987   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Cabal            1988   * "Michel/Seibu    sound 11/04/88" (YM2151 substituted for YM3812, MSM5205 ADPCM)
    Dead Angle       1988   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (2xYM2203 substituted for YM3812, MSM5205 ADPCM)
    Dynamite Duke    1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden           1990   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Blood Brothers   1990     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    D-Con            1992     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    Related sound programs (not implemented yet):

    Zero Team                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Legionnaire               "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden 2                  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Raiden DX                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Cup Soccer                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    SD Gundam Psycho Salamander "Copyright by King Bee Sol 1991"
    * = encrypted

***************************************************************************/
#ifndef MAME_SHARED_SEIBUSOUND_H
#define MAME_SHARED_SEIBUSOUND_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/msm5205.h"

#include "dirom.h"


class seibu_sound_common {
public:
	virtual ~seibu_sound_common() = default;

protected:
	void seibu_sound_map(address_map &map) ATTR_COLD;
};

class seibu_sound_device : public device_t
{
public:
	seibu_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~seibu_sound_device() { }

	//  configuration
	template <typename T> void set_rom_tag(T &&tag) { m_sound_rom.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_rombank_tag(T &&tag) { m_rom_bank.set_tag(std::forward<T>(tag)); }
	auto int_callback()  { return m_int_cb.bind(); }
	auto ym_read_callback()  { return m_ym_read_cb.bind(); }
	auto ym_write_callback() { return m_ym_write_cb.bind(); }

	u8 main_r(offs_t offset);
	void main_w(offs_t offset, u8 data);
	void main_mustb_w(offs_t, u16 data, u16 mem_mask);
	void irq_clear_w(u8);
	void rst10_ack_w(u8);
	void rst18_ack_w(u8);
	u8 ym_r(offs_t offset);
	void ym_w(offs_t offset, u8 data);
	void bank_w(u8 data);
	void coin_w(u8 data);
	void fm_irqhandler(int state);
	u8 soundlatch_r(offs_t offset);
	u8 main_data_pending_r();
	void main_data_w(offs_t offset, u8 data);
	void pending_w(u8);

	IRQ_CALLBACK_MEMBER(im0_vector_cb);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_irq_lines(s32 param);
	TIMER_CALLBACK_MEMBER(update_irq_synced);

	// device callbacks
	devcb_write_line m_int_cb;
	devcb_read8 m_ym_read_cb;
	devcb_write8 m_ym_write_cb;

	// internal state
	optional_region_ptr<uint8_t> m_sound_rom;
	optional_memory_bank m_rom_bank;
	uint8_t m_main2sub[2];
	uint8_t m_sub2main[2];
	int m_main2sub_pending;
	int m_sub2main_pending;
	bool m_rst10_irq;
	bool m_rst18_irq;
	bool m_rst10_service;
	bool m_rst18_service;

	enum
	{
		VECTOR_INIT,
		RST10_ASSERT,
		RST10_CLEAR,
		RST10_ACKNOWLEDGE,
		RST10_EOI,
		RST18_ASSERT,
		RST18_ACKNOWLEDGE,
		RST18_EOI
	};
};

DECLARE_DEVICE_TYPE(SEIBU_SOUND, seibu_sound_device)


// SEI80BU (Z80 program decryption)

class sei80bu_device : public device_t, public device_rom_interface<16>
{
public:
	sei80bu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 data_r(offs_t offset);
	u8 opcode_r(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override { }
};

DECLARE_DEVICE_TYPE(SEI80BU, sei80bu_device)

// Seibu ADPCM device

class seibu_adpcm_device : public device_t
{
public:
	template <typename T> seibu_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&msm5205_tag)
		: seibu_adpcm_device(mconfig, tag, owner, clock)
	{
		m_msm.set_tag(std::forward<T>(msm5205_tag));
	}
	seibu_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~seibu_adpcm_device() { }

	void decrypt();
	void adr_w(offs_t offset, u8 data);
	void ctl_w(u8 data);
	void msm_int(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	required_device<msm5205_device> m_msm;
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

/**************************************************************************/

#endif // MAME_SHARED_SEIBUSOUND_H
