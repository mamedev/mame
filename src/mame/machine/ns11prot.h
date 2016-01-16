// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Namco System 11 Protection
 *
 */

#include "emu.h"

class ns11_keycus_device : public device_t
{
protected:
	ns11_keycus_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock,std::string shortname, std::string source);

	virtual void device_start() override;
	virtual void device_reset() override;

protected:
	UINT16 m_p1;
	UINT16 m_p2;
	UINT16 m_p3;

public:
	virtual DECLARE_READ16_MEMBER( read ) = 0;
	virtual DECLARE_WRITE16_MEMBER( write ) = 0;
};

/* tekken 2 */

class keycus_c406_device : public ns11_keycus_device
{
public:
	keycus_c406_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C406;

/* soul edge */

class keycus_c409_device : public ns11_keycus_device
{
public:
	keycus_c409_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C409;

/* dunk mania */

class keycus_c410_device : public ns11_keycus_device
{
public:
	keycus_c410_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C410;

/* prime goal ex */

class keycus_c411_device : public ns11_keycus_device
{
public:
	keycus_c411_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C411;

/* xevious 3d/g */

class keycus_c430_device : public ns11_keycus_device
{
public:
	keycus_c430_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C430;

/* dancing eyes */

class keycus_c431_device : public ns11_keycus_device
{
public:
	keycus_c431_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C431;

/* pocket racer */

class keycus_c432_device : public ns11_keycus_device
{
public:
	keycus_c432_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C432;

/* star sweep */

class keycus_c442_device : public ns11_keycus_device
{
public:
	keycus_c442_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C442;

/* kosodate quiz my angel 3 / point blank 2 */

class keycus_c443_device : public ns11_keycus_device
{
public:
	keycus_c443_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER( read ) override;
	virtual DECLARE_WRITE16_MEMBER( write ) override;
};

extern const device_type KEYCUS_C443;
