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

class rocvfd_device : public device_t
{
public:
	// inline configuration helpers
	void set_port_value(uint8_t val) { m_port_val = val; }

	virtual void update_display();
	void shift_clock(int data);
	void write_char(int data);
	void sclk(int state);
	void data(int state);
	void por(int state);


protected:
	rocvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	std::unique_ptr<output_finder<16> > m_outputs;
	std::unique_ptr<output_finder<1> > m_brightness;

	int m_cursor_pos;
	int m_window_size;
	int m_shift_count;
	int m_shift_data;
	int m_pcursor_pos;
	int m_count;
	int m_data;
	int m_duty;
	int m_sclk;
	int m_por;
	uint8_t m_cursor;
	uint32_t m_chars[16];

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	static uint32_t set_display(uint32_t segin);

	uint8_t m_port_val;
};


class roc10937_device : public rocvfd_device {
public:
	roc10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 60);
};

class msc1937_device : public rocvfd_device {
public:
	msc1937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 60);
};

class mic10937_device : public rocvfd_device {
public:
	mic10937_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 60);
};

class roc10957_device : public rocvfd_device {
public:
	roc10957_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 60);

	void write_char(int data);
};

class s16lf01_device : public rocvfd_device {
public:
	s16lf01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 60);
};


DECLARE_DEVICE_TYPE(ROC10937, roc10937_device)
DECLARE_DEVICE_TYPE(MSC1937,  msc1937_device)
DECLARE_DEVICE_TYPE(MIC10937, mic10937_device)
DECLARE_DEVICE_TYPE(ROC10957, roc10957_device)
DECLARE_DEVICE_TYPE(S16LF01,  s16lf01_device)

#endif // MAME_MACHINE_ROC10937_H
