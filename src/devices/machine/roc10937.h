// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

    Rockwell 10937/10957 interface and simlar chips
    Emulation by J.Wallace
    OKI MSC1937 is a clone of this chip, with many others.

**********************************************************************/
#pragma once

#ifndef ROC10937_H
#define ROC10937_H

#define MCFG_ROC10937_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_ROC10937_PORT(_val)

#define MCFG_ROC10937_PORT(_val) \
	roc10937_t::static_set_value(*device, _val);
#define MCFG_ROC10937_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ROC10957_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10957,60)\
		MCFG_ROC10957_PORT(_val)

#define MCFG_ROC10957_PORT(_val) \
	roc10957_t::static_set_value(*device, _val);
#define MCFG_ROC10957_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_MSC1937_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_MSC1937_PORT(_val)

#define MCFG_MSC1937_PORT(_val) \
	MCFG_ROC10937_PORT(_val)

#define MCFG_MSC1937_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_S16LF01_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, S16LF01,60)\
		MCFG_S16LF01_PORT(_val)

#define MCFG_S16LF01_PORT(_val) \
	MCFG_ROC10937_PORT(_val)

class rocvfd_t : public device_t {
public:
	rocvfd_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// inline configuration helpers
	static void static_set_value(device_t &device, int val);
	virtual void update_display();
	UINT8   m_port_val;
	void shift_clock(int data);
	void write_char(int data);
	UINT32 set_display(UINT32 segin);
	DECLARE_WRITE_LINE_MEMBER( sclk );
	DECLARE_WRITE_LINE_MEMBER( data );
	DECLARE_WRITE_LINE_MEMBER( por );


protected:
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
	UINT8 m_cursor;
	UINT32 m_chars[16];
	UINT32 m_outputs[16];

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
};


class roc10937_t : public rocvfd_t {
public:
	roc10937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class msc1937_t : public rocvfd_t {
public:
	msc1937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class roc10957_t : public rocvfd_t {
public:
	roc10957_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void write_char(int data);
};

class s16lf01_t : public rocvfd_t {
public:
	s16lf01_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ROC10937;
extern const device_type MSC1937;
extern const device_type ROC10957;
extern const device_type S16LF01;

#endif
