// license:GPL2+
// copyright-holders:Felipe Sanches
#ifndef MAME_INCLUDES_ZEZINHO_H
#define MAME_INCLUDES_ZEZINHO_H

#pragma once

class zezinho_state : public driver_device {
public:
	zezinho_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(ita2);

protected:
	virtual void machine_start() override;
	required_device<zezinho_cpu_device> m_maincpu;
private:
};

#endif // MAME_INCLUDES_ZEZINHO_H
