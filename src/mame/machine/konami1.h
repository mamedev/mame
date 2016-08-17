// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "cpu/m6809/m6809.h"

class konami1_device : public m6809_base_device {
public:
	konami1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_encryption_boundary(UINT16 adr);

protected:
	class mi_konami1 : public mi_default {
	public:
		UINT16 m_boundary;
		mi_konami1(UINT16 boundary);
		virtual ~mi_konami1() {}
		virtual UINT8 read_opcode(UINT16 adr) override;
	};

	UINT16 m_boundary;

	virtual void device_start() override;
};

extern const device_type KONAMI1;
