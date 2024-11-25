// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Namco System 11 Protection
 *
 */
#ifndef MAME_NAMCO_NS11PROT_H
#define MAME_NAMCO_NS11PROT_H

#pragma once


class ns11_keycus_device : public device_t
{
protected:
	ns11_keycus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint16_t m_p1 = 0;
	uint16_t m_p2 = 0;
	uint16_t m_p3 = 0;

public:
	virtual uint16_t read(offs_t offset) = 0;
	virtual void write(offs_t offset, uint16_t data) = 0;
};

/* tekken 2 */

class keycus_c406_device : public ns11_keycus_device
{
public:
	keycus_c406_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C406, keycus_c406_device)

/* soul edge */

class keycus_c409_device : public ns11_keycus_device
{
public:
	keycus_c409_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C409, keycus_c409_device)

/* dunk mania */

class keycus_c410_device : public ns11_keycus_device
{
public:
	keycus_c410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C410, keycus_c410_device)

/* prime goal ex */

class keycus_c411_device : public ns11_keycus_device
{
public:
	keycus_c411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C411, keycus_c411_device)

/* xevious 3d/g */

class keycus_c430_device : public ns11_keycus_device
{
public:
	keycus_c430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C430, keycus_c430_device)

/* dancing eyes */

class keycus_c431_device : public ns11_keycus_device
{
public:
	keycus_c431_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C431, keycus_c431_device)

/* pocket racer */

class keycus_c432_device : public ns11_keycus_device
{
public:
	keycus_c432_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C432, keycus_c432_device)

/* star sweep */

class keycus_c442_device : public ns11_keycus_device
{
public:
	keycus_c442_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C442, keycus_c442_device)

/* kosodate quiz my angel 3 / point blank 2 */

class keycus_c443_device : public ns11_keycus_device
{
public:
	keycus_c443_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(KEYCUS_C443, keycus_c443_device)

#endif // MAME_NAMCO_NS11PROT_H
