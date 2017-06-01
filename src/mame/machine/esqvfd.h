// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_ESQVFD_H
#define MAME_MACHINE_ESQVFD_H


#define MCFG_ESQ1X22_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, ESQ1X22, 60)

#define MCFG_ESQ1x22_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ESQ2X40_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, ESQ2X40, 60)

#define MCFG_ESQ2X40_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ESQ2X40_SQ1_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, ESQ2X40_SQ1, 60)

#define MCFG_ESQ2X40_SQ1_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

class esqvfd_device : public device_t {
public:
	DECLARE_WRITE8_MEMBER( write ) { write_char(data); }

	virtual void write_char(int data) = 0;
	virtual void update_display();

	uint32_t conv_segments(uint16_t segin);

protected:
	esqvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	static constexpr uint8_t AT_NORMAL      = 0x00;
	static constexpr uint8_t AT_BOLD        = 0x01;
	static constexpr uint8_t AT_UNDERLINE   = 0x02;
	static constexpr uint8_t AT_BLINK       = 0x04;
	static constexpr uint8_t AT_BLINKED     = 0x80;   // set when character should be blinked off

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	int m_cursx, m_cursy;
	int m_savedx, m_savedy;
	int m_rows, m_cols;
	uint8_t m_curattr;
	uint8_t m_lastchar;
	uint8_t m_chars[2][40];
	uint8_t m_attrs[2][40];
	uint8_t m_dirty[2][40];
};

class esq1x22_device : public esqvfd_device {
public:
	esq1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(int data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
};

class esq2x40_device : public esqvfd_device {
public:
	esq2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(int data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class esq2x40_sq1_device : public esqvfd_device {
public:
	esq2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(int data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	bool m_wait87shift, m_wait88shift;
};

DECLARE_DEVICE_TYPE(ESQ1X22,     esq1x22_device)
DECLARE_DEVICE_TYPE(ESQ2X40,     esq2x40_device)
DECLARE_DEVICE_TYPE(ESQ2X40_SQ1, esq2x40_sq1_device)

#endif // MAME_MACHINE_ESQVFD_H
