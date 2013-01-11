/**********************************************************************

    Rockwell 10937/10957 interface and emulation by J.Wallace
    OKI MSC1937 is a clone of this chip

**********************************************************************/
#pragma once

#ifndef ROC10937_H
#define ROC10937_H

#define LEFT_TO_RIGHT 1
#define RIGHT_TO_LEFT 0

#define MCFG_ROC10937_ADD(_tag,_val,_reversed) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_ROC10937_PORT(_val) \
		MCFG_ROC10937_REVERSE(_reversed) \

#define MCFG_ROC10937_PORT(_val) \
	roc10937_t::static_set_value(*device, _val); \

#define MCFG_ROC10937_REVERSE(_reversed) \
	roc10937_t::static_set_zero(*device, _reversed); \

#define MCFG_ROC10937_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ROC10957_ADD(_tag,_val,_reversed) \
		MCFG_DEVICE_ADD(_tag, ROC10957,60)\
		MCFG_ROC10957_PORT(_val) \
		MCFG_ROC10957_REVERSE(_reversed) \

#define MCFG_ROC10957_PORT(_val) \
	roc10957_t::static_set_value(*device, _val); \

#define MCFG_ROC10957_REVERSE(_reversed) \
	roc10957_t::static_set_zero(*device, _reversed); \

#define MCFG_ROC10957_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_MSC1937_ADD(_tag,_val,_reversed) \
		MCFG_DEVICE_ADD(_tag, ROC10937,60)\
		MCFG_MSC1937_PORT(_val) \
		MCFG_MSC1937_REVERSE(_reversed) \

#define MCFG_MSC1937_PORT(_val) \
	MCFG_ROC10937_PORT(_val)

#define MCFG_MSC1937_REVERSE(_reversed) \
	roc10937_t::static_set_zero(*device, _reversed); \

#define MCFG_MSC1937_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)


class rocvfd_t : public device_t {
public:
	typedef delegate<void (bool state)> line_cb;

	rocvfd_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_value(device_t &device, int val);
	static void static_set_zero(device_t &device, bool reversed);
	virtual void update_display();
	UINT8   m_port_val;
	bool m_reversed;
	void blank(int data);
	void shift_data(int data);
	void write_char(int data);
	void setdata(int segdata, int data);
	UINT32 set_display(UINT32 segin);


protected:
	int m_cursor_pos;
	int m_window_size;      // window  size
	int m_shift_count;
	int m_shift_data;
	int m_pcursor_pos;
	int m_brightness;
	int m_count;
	int m_duty;
	int m_disp;
	UINT8 m_cursor;
	UINT32 m_chars[16];
	UINT32 m_outputs[16];

	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
};


class roc10937_t : public rocvfd_t {
public:
	roc10937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:

};

class msc1937_t : public rocvfd_t {
public:
	msc1937_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:

};

class roc10957_t : public rocvfd_t {
public:
	roc10957_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void write_char(int data);

protected:

};

extern const device_type ROC10937;
extern const device_type MSC1937;
extern const device_type ROC10957;

#endif
