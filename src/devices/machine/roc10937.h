// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

    Rockwell 10937/10957 interface and similar chips
    Emulation by J.Wallace
    OKI MSC1937 is a clone of this chip, with many others.

**********************************************************************/
#ifndef MAME_MACHINE_ROC10937_H
#define MAME_MACHINE_ROC10937_H

#pragma once

#define MCFG_ROC10937_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_ROC10937_PORT(_val)

#define MCFG_ROC10937_PORT(_val) \
		roc10937_device::static_set_value(*device, _val);
#define MCFG_ROC10937_REMOVE(_tag) \
		MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ROC10957_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10957,60)\
		MCFG_ROC10957_PORT(_val)

#define MCFG_ROC10957_PORT(_val) \
		roc10957_device::static_set_value(*device, _val);
#define MCFG_ROC10957_REMOVE(_tag) \
		MCFG_DEVICE_REMOVE(_tag)

#define MCFG_MSC1937_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_MSC1937_PORT(_val)

#define MCFG_MSC1937_PORT(_val) \
		MCFG_ROC10937_PORT(_val)

#define MCFG_MSC1937_REMOVE(_tag) \
		MCFG_DEVICE_REMOVE(_tag)

#define MCFG_MIC10937_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, MIC10937,60)\
		MCFG_MIC10937_PORT(_val)

#define MCFG_MIC10937_PORT(_val) \
		MCFG_ROC10937_PORT(_val)

#define MCFG_MIC10937_REMOVE(_tag) \
		MCFG_DEVICE_REMOVE(_tag)

#define MCFG_S16LF01_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, S16LF01,60)\
		MCFG_S16LF01_PORT(_val)

#define MCFG_S16LF01_PORT(_val) \
		MCFG_ROC10937_PORT(_val)

class rocvfd_device : public device_t {
public:
	// inline configuration helpers
	static void static_set_value(device_t &device, int val);
	virtual void update_display();
	void shift_clock(int data);
	void write_char(int data);
	uint32_t set_display(uint32_t segin);
	DECLARE_WRITE_LINE_MEMBER( sclk );
	DECLARE_WRITE_LINE_MEMBER( data );
	DECLARE_WRITE_LINE_MEMBER( por );


protected:
	rocvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint8_t m_port_val;
	int m_cursor_pos;
	int m_window_size;      // window  size
	int m_shift_count;
	int m_shift_data;
	int m_pcursor_pos;
	int m_brightness;
	int m_count;
	int m_data;
	int m_duty;
	int m_disp;
	int m_sclk;
	uint8_t m_cursor;
	uint32_t m_chars[16];
	uint32_t m_outputs[16];

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
};


class roc10937_device : public rocvfd_device {
public:
	roc10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class msc1937_device : public rocvfd_device {
public:
	msc1937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mic10937_device : public rocvfd_device {
public:
	mic10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class roc10957_device : public rocvfd_device {
public:
	roc10957_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_char(int data);
};

class s16lf01_device : public rocvfd_device {
public:
	s16lf01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(ROC10937, roc10937_device)
DECLARE_DEVICE_TYPE(MSC1937,  msc1937_device)
DECLARE_DEVICE_TYPE(MIC10937, mic10937_device)
DECLARE_DEVICE_TYPE(ROC10957, roc10957_device)
DECLARE_DEVICE_TYPE(S16LF01,  s16lf01_device)

#endif // MAME_MACHINE_ROC10937_H
