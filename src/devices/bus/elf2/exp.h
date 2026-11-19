// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Netronics Elf II 86-pin expansion bus emulation

**********************************************************************

                   +5 V       1      2       +5 V
                   +5 V       3      4       +5 V
                              5      6       CLOCK
                              7      8       XTAL
                              9     10       _LOAD
                             11     12       _DMA IN
                             13     14       _RUN
                             15     16       _DMA OUT
                             17     18       Q
                             19     20       _INT
                             21     22       SC1
                             23     24       _MWR
                             25     26       SC0
                             27     28       TPA
                             29     30       MRD
                             31     32       TPB
                             33     34       D7
                             35     36       MA7
                             37     38       D6
                             39     40       MA6
                             41     42       D5
                             43     44       MA5
                             45     46       D4
                             47     48       MA4
                             49     50       D3
                             51     52       MA3
                             53     54       D2
                             55     56       MA2
                             57     58       D1
                             59     60       MA1
                             61     62       D0
                             63     64       MA0
                             65     66       _EF1
                             67     68       N2
                             69     70       _EF2
                             71     72       N1
                             73     74       _EF3
                             75     76       N0
                             77     78       _EF4
                             79     80       GND
                             81     82       GND
                             83     84
                             85     86       3.58 MHz

**********************************************************************/

#ifndef MAME_BUS_ELF2_EXP_H
#define MAME_BUS_ELF2_EXP_H

#pragma once

#include <forward_list>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class elf2_expansion_bus_device;
class device_elf2_expansion_card_interface;


// ======================> elf2_expansion_slot_device

class elf2_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_elf2_expansion_card_interface>
{
public:
	template <typename T, typename U>
	elf2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&bus_tag, U &&opts, const char *dflt)
		: elf2_expansion_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<U>(opts), dflt, false);
		m_bus.set_tag(std::forward<T>(bus_tag));
	}
	elf2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_device<elf2_expansion_bus_device> m_bus;
};


// ======================> elf2_expansion_bus_device

class elf2_expansion_bus_device : public device_t
{
public:
	elf2_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	address_space &program() { return *m_program; }
	address_space &io() { return *m_io; }

	void add_card(device_elf2_expansion_card_interface &card);

	// computer interface
	int ef1_r();
	int ef2_r();
	int ef3_r();
	int ef4_r();
	void q_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_program;
	required_address_space m_io;

	std::forward_list<device_elf2_expansion_card_interface *> m_cards;
};


// ======================> device_elf2_expansion_card_interface

class device_elf2_expansion_card_interface : public device_interface
{
	friend class elf2_expansion_bus_device;

protected:
	device_elf2_expansion_card_interface(const machine_config &mconfig, device_t &device);

	virtual int elf2_ef1_r() { return 0; }
	virtual int elf2_ef2_r() { return 0; }
	virtual int elf2_ef3_r() { return 0; }
	virtual int elf2_ef4_r() { return 0; }
	virtual void elf2_q_w(int state) { }

	virtual void interface_pre_start() override;

	elf2_expansion_bus_device *m_bus;
};


DECLARE_DEVICE_TYPE(ELF2_EXPANSION_SLOT, elf2_expansion_slot_device)
DECLARE_DEVICE_TYPE(ELF2_EXPANSION_BUS, elf2_expansion_bus_device)

void elf2_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_ELF2_EXP_H
