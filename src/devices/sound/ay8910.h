// license:BSD-3-Clause
// copyright-holders:Couriersud
#pragma once

#ifndef __AY8910_H__
#define __AY8910_H__

#include "emu.h"

/*
AY-3-8910A: 2 I/O ports
AY-3-8912A: 1 I/O port
AY-3-8913A: 0 I/O port
AY-3-8914:  same as 8910 except for different register mapping and two bit envelope enable / volume field
AY8930: upper compatible with 8910.
In extended mode, it has higher resolution and duty ratio setting
YM2149: higher resolution, selectable clock divider
YM3439: same as 2149
YMZ284: 0 I/O port, different clock divider
YMZ294: 0 I/O port
*/

#define ALL_8910_CHANNELS -1

/* Internal resistance at Volume level 7. */

#define AY8910_INTERNAL_RESISTANCE  (356)
#define YM2149_INTERNAL_RESISTANCE  (353)

/*
 * The following is used by all drivers not reviewed yet.
 * This will like the old behaviour, output between
 * 0 and 7FFF
 */
#define AY8910_LEGACY_OUTPUT        (0x01)

/*
 * Specifing the next define will simulate the special
 * cross channel mixing if outputs are tied together.
 * The driver will only provide one stream in this case.
 */
#define AY8910_SINGLE_OUTPUT        (0x02)

/*
 * The following define is the default behaviour.
 * Output level 0 is 0V and 7ffff corresponds to 5V.
 * Use this to specify that a discrete mixing stage
 * follows.
 */
#define AY8910_DISCRETE_OUTPUT      (0x04)

/*
 * The following define causes the driver to output
 * resistor values. Intended to be used for
 * netlist interfacing.
 */

#define AY8910_RESISTOR_OUTPUT      (0x08)

/*
 * This define specifies the initial state of YM2149
 * pin 26 (SEL pin). By default it is set to high,
 * compatible with AY8910.
 */
/* TODO: make it controllable while it's running (used by any hw???) */
#define YM2149_PIN26_HIGH           (0x00) /* or N/C */
#define YM2149_PIN26_LOW            (0x10)


#define AY8910_NUM_CHANNELS 3


#define MCFG_AY8910_OUTPUT_TYPE(_flag) \
	ay8910_device::set_flags(*device, _flag);

#define MCFG_AY8910_RES_LOADS(_res0, _res1, _res2) \
	ay8910_device::set_resistors_load(*device, _res0, _res1, _res2);

#define MCFG_AY8910_PORT_A_READ_CB(_devcb) \
	devcb = &ay8910_device::set_port_a_read_callback(*device, DEVCB_##_devcb);

#define MCFG_AY8910_PORT_B_READ_CB(_devcb) \
	devcb = &ay8910_device::set_port_b_read_callback(*device, DEVCB_##_devcb);

#define MCFG_AY8910_PORT_A_WRITE_CB(_devcb) \
	devcb = &ay8910_device::set_port_a_write_callback(*device, DEVCB_##_devcb);

#define MCFG_AY8910_PORT_B_WRITE_CB(_devcb) \
	devcb = &ay8910_device::set_port_b_write_callback(*device, DEVCB_##_devcb);


class ay8910_device : public device_t,
									public device_sound_interface
{
public:
	enum psg_type_t
	{
		PSG_TYPE_AY,
		PSG_TYPE_YM
	};

	// construction/destruction
	ay8910_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	ay8910_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner,
					UINT32 clock, psg_type_t psg_type, int streams, int ioports, std::string shortname, std::string source);

	// static configuration helpers
	static void set_flags(device_t &device, int flags) { downcast<ay8910_device &>(device).m_flags = flags; }
	static void set_resistors_load(device_t &device, int res_load0, int res_load1, int res_load2) { downcast<ay8910_device &>(device).m_res_load[0] = res_load0; downcast<ay8910_device &>(device).m_res_load[1] = res_load1; downcast<ay8910_device &>(device).m_res_load[2] = res_load2; }
	template<class _Object> static devcb_base &set_port_a_read_callback(device_t &device, _Object object) { return downcast<ay8910_device &>(device).m_port_a_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_b_read_callback(device_t &device, _Object object) { return downcast<ay8910_device &>(device).m_port_b_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_a_write_callback(device_t &device, _Object object) { return downcast<ay8910_device &>(device).m_port_a_write_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_b_write_callback(device_t &device, _Object object) { return downcast<ay8910_device &>(device).m_port_b_write_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_WRITE8_MEMBER( data_w );

	/* /RES */
	DECLARE_WRITE8_MEMBER( reset_w );

	/* use this when BC1 == A0; here, BC1=0 selects 'data' and BC1=1 selects 'latch address' */
	DECLARE_WRITE8_MEMBER( data_address_w );

	/* use this when BC1 == !A0; here, BC1=0 selects 'latch address' and BC1=1 selects 'data' */
	DECLARE_WRITE8_MEMBER( address_data_w );

	void set_volume(int channel,int volume);
	void ay_set_clock(int clock);

	struct ay_ym_param
	{
		double r_up;
		double r_down;
		int    res_count;
		double res[32];
	};

	struct mosfet_param
	{
		double m_Vth;
		double m_Vg;
		int    m_count;
		double m_Kn[32];
	};

	void ay8910_write_ym(int addr, int data);
	int ay8910_read_ym();
	void ay8910_reset_ym();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal helpers
	inline UINT16 mix_3D();
	void ay8910_write_reg(int r, int v);
	void build_mixer_table();
	void ay8910_statesave();

	// internal state
	psg_type_t m_type;
	int m_streams;
	int m_ioports;
	int m_ready;
	sound_stream *m_channel;
	INT32 m_register_latch;
	UINT8 m_regs[16];
	INT32 m_last_enable;
	INT32 m_count[AY8910_NUM_CHANNELS];
	UINT8 m_output[AY8910_NUM_CHANNELS];
	UINT8 m_prescale_noise;
	INT32 m_count_noise;
	INT32 m_count_env;
	INT8 m_env_step;
	UINT32 m_env_volume;
	UINT8 m_hold,m_alternate,m_attack,m_holding;
	INT32 m_rng;
	UINT8 m_env_step_mask;
	/* init parameters ... */
	int m_step;
	int m_zero_is_off;
	UINT8 m_vol_enabled[AY8910_NUM_CHANNELS];
	const ay_ym_param *m_par;
	const ay_ym_param *m_par_env;
	INT32 m_vol_table[AY8910_NUM_CHANNELS][16];
	INT32 m_env_table[AY8910_NUM_CHANNELS][32];
	INT32 m_vol3d_table[8*32*32*32];
	int m_flags;          /* Flags */
	int m_res_load[3];    /* Load on channel in ohms */
	devcb_read8 m_port_a_read_cb;
	devcb_read8 m_port_b_read_cb;
	devcb_write8 m_port_a_write_cb;
	devcb_write8 m_port_b_write_cb;
};

extern const device_type AY8910;

class ay8912_device : public ay8910_device
{
public:
	ay8912_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type AY8912;

class ay8913_device : public ay8910_device
{
public:
	ay8913_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type AY8913;

class ay8914_device : public ay8910_device
{
public:
	ay8914_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	/* AY8914 handlers needed due to different register map */
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
};

extern const device_type AY8914;

class ay8930_device : public ay8910_device
{
public:
	ay8930_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type AY8930;

class ym2149_device : public ay8910_device
{
public:
	ym2149_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type YM2149;

class ym3439_device : public ay8910_device
{
public:
	ym3439_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type YM3439;

class ymz284_device : public ay8910_device
{
public:
	ymz284_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type YMZ284;

class ymz294_device : public ay8910_device
{
public:
	ymz294_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

extern const device_type YMZ294;


#endif /* __AY8910_H__ */
