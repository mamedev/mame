// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_NAMCOIO_H
#define MAME_NAMCO_NAMCOIO_H


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class namcoio_device : public device_t
{
public:
	template <unsigned N> auto in_callback() { return m_in_cb[N].bind(); }
	template <unsigned N> auto out_callback() { return m_out_cb[N].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void set_reset_line(int state);
	int read_reset_line();

	virtual void customio_run() = 0;

protected:
	namcoio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int device_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	enum {
			TYPE_NAMCO56XX,
			TYPE_NAMCO58XX,
			TYPE_NAMCO59XX
	};

	// internal state
	uint8_t          m_ram[16]{};

	devcb_read8::array<4> m_in_cb;
	devcb_write8::array<2> m_out_cb;

	int            m_reset = 0;
	int32_t        m_lastcoins = 0, m_lastbuttons = 0;
	int32_t        m_credits = 0;
	int32_t        m_coins[2]{};
	int32_t        m_coins_per_cred[2]{};
	int32_t        m_creds_per_coin[2]{};
	int32_t        m_in_count = 0;

	void handle_coins( int swap );

private:
	//int m_device_type;
};

class namco56xx_device : public namcoio_device
{
public:
	namco56xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void customio_run() override;
};

class namco58xx_device : public namcoio_device
{
public:
	namco58xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void customio_run() override;
};

class namco59xx_device : public namcoio_device
{
public:
	namco59xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void customio_run() override;
};

DECLARE_DEVICE_TYPE(NAMCO_56XX, namco56xx_device)
DECLARE_DEVICE_TYPE(NAMCO_58XX, namco58xx_device)
DECLARE_DEVICE_TYPE(NAMCO_59XX, namco59xx_device)

#endif // MAME_NAMCO_NAMCOIO_H
