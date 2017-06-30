// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
INTELLEC® 4/MOD 40 Universal Slot

                         1    2  /TEST
                  GND    3    4  GND
                   NC    5    6  NC
                   NC    7    8  NC
                   NC    9   10  NC
                  MA0   11   12  MA1
                  MA2   13   14  MA3
                  MA4   15   16  MA5
                  MA6   17   18  MA7
                   C0   19   20  C1
                        21   22  NC
                /MDI0   23   24  NC
                /MDI1   25   26
                /MDI3   27   28
                /MDI2   29   30
                /MDI5   31   32
                /MDI4   33   34
                /MDI7   35   36
                /MDI6   37   38
                        39   40
                 /OUT   41   42  /ENABLE MON PROM
                 -10V   43   44  -10V
           /CPU RESET   45   46  /USER RESET
             /CM-RAM2   47   48  /CM-RAM3
             /CM-RAM0   49   50  /CM-RAM1
                I/O 1   51   52  I/O 0
                I/O 2   53   54  /IN
                  F/L   55   56  I/O 3
                        57   58
                        59   60
                        61   62
                        63   64
                        65   66
                        67   68
                        69   70
                        71   72  /D3
    /STOP ACKNOWLEDGE   73   74  /STOP
                        75   76  /D2
                        77   78
                /SYNC   79   80  /D1
                        81   82  /PROM SEL
                  /D0   83   84
                        85   86
                        87   88  /RESET-4002
                        89   90
                        91   92
              /CM-ROM   93   94  C3
                    W   95   96  C2
              PHASE 2   97   98  PHASE 1
                  +5V   99  100  +5V

NC                      Not connected on backplane
GND                     Common ground
+5V                     Power supply
-10V                    Power supply
PHASE1                  5.185MHz/7 clock phase 1
PHASE2                  5.185MHz/7 clock phase 2
/SYNC                   CPU /SYNC output (instruction cycle synchronisation)
/TEST                   CPU /TEST input, wired-or (cards should pull low to assert)
/STOP                   CPU /STP input, wired-or (cards should pull low to assert)
/STOP ACKNOWLEDGE       CPU /STP ACK output (stop/halt acknowledge)
/CM-ROM                 CPU /CM-ROM0 output (ROM chip select)
/CM-RAM0.../CM-RAM3     CPU /CM-RAM outputs (RAM chip select)
/D0.../D3               CPU data bus
MA0...MA7               A outputs from 4289 on CPU board (low ROM address/SRC address)
C0...C3                 C outputs from 4289 on CPU board (high ROM address/I/O chip select)
W                       PM output from 4289 on CPU board (program memory enable)
F/L                     FL output from 4289 on CPU board (program memory nybble select)
I/O 0...I/O 3           I/O lines to 4289 on CPU board (bidirectional I/O data)
/MDI0.../MDI7           OPR/OPA multiplexer inputs on CPU board (ROM data when onboard monitor PROM is not selected)
/IN                     IN output from 4289 on CPU board (I/O read strobe)
/OUT                    OUT output from 4289 on CPU board (I/O write strobe)
/CPU RESET              CPU/4289 reset output from control board
/RESET-4002             Open collector output from control board
/ENABLE MON PROM        Output from control board
/PROM SEL               Output from control board
/USER RESET             Input to control board, edge sensitive, wired or (cards should pull low to assert)

other pins connected between universal slots but not connected to CPU or control cards
*/
#ifndef MAME_BUS_INTELLEC4_INTELLEC4_H
#define MAME_BUS_INTELLEC4_INTELLEC4_H

#pragma once


#define MCFG_INTELLEC4_UNIV_SLOT_ADD(bus_tag, slot_tag, clock, slot_intf, def_slot) \
		MCFG_DEVICE_ADD(slot_tag, INTELLEC4_UNIV_SLOT, clock) \
		MCFG_DEVICE_SLOT_INTERFACE(slot_intf, def_slot, false) \
		bus::intellec4::univ_slot_device::set_bus_tag(*device, bus_tag);

#define MCFG_INTELLEC4_UNIV_SLOT_REMOVE(slot_tag) \
		MCFG_DEVICE_REMOVE(slot_tag)


#define MCFG_INTELLEC4_UNIV_BUS_STOP_CB(obj) \
		bus::intellec4::univ_bus_device::set_stop_out_cb(*device, DEVCB_##obj);

#define MCFG_INTELLEC4_UNIV_BUS_TEST_CB(obj) \
		bus::intellec4::univ_bus_device::set_test_out_cb(*device, DEVCB_##obj);

#define MCFG_INTELLEC4_UNIV_BUS_RESET_4002_CB(obj) \
		bus::intellec4::univ_bus_device::set_reset_4002_out_cb(*device, DEVCB_##obj);

#define MCFG_INTELLEC4_UNIV_BUS_USER_RESET_CB(obj) \
		bus::intellec4::univ_bus_device::set_user_reset_out_cb(*device, DEVCB_##obj);


namespace bus { namespace intellec4 {

class univ_slot_device;
class univ_bus_device;
class device_univ_card_interface;


class univ_slot_device : public device_t, public device_slot_interface
{
public:
	// configuration helpers
	static void set_bus_tag(device_t &device, char const *bus_tag);

	univ_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override;
};


class univ_bus_device : public device_t
{
public:
	// configuration helpers
	template <typename Obj> static devcb_base &set_stop_out_cb(device_t &device, Obj &&cb)
	{ return downcast<univ_bus_device &>(device).m_stop_out_cb.set_callback(std::forward<Obj>(cb)); }
	template <typename Obj> static devcb_base &set_test_out_cb(device_t &device, Obj &&cb)
	{ return downcast<univ_bus_device &>(device).m_test_out_cb.set_callback(std::forward<Obj>(cb)); }
	template <typename Obj> static devcb_base &set_reset_4002_out_cb(device_t &device, Obj &&cb)
	{ return downcast<univ_bus_device &>(device).m_reset_4002_out_cb.set_callback(std::forward<Obj>(cb)); }
	template <typename Obj> static devcb_base &set_user_reset_out_cb(device_t &device, Obj &&cb)
	{ return downcast<univ_bus_device &>(device).m_user_reset_out_cb.set_callback(std::forward<Obj>(cb)); }

	univ_bus_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// input lines
	DECLARE_WRITE_LINE_MEMBER(sync_in);
	DECLARE_WRITE_LINE_MEMBER(test_in);
	DECLARE_WRITE_LINE_MEMBER(stop_in);
	DECLARE_WRITE_LINE_MEMBER(stop_acknowledge_in);
	DECLARE_WRITE_LINE_MEMBER(cpu_reset_in);
	DECLARE_WRITE_LINE_MEMBER(reset_4002_in);

	// output lines
	DECLARE_READ_LINE_MEMBER(stop_out) const        { return (m_stop        & ~u16(1U)) ? 0 : 1; }
	DECLARE_READ_LINE_MEMBER(test_out) const        { return (m_test        & ~u16(1U)) ? 0 : 1; }
	DECLARE_READ_LINE_MEMBER(reset_4002_out) const  { return (m_reset_4002  & ~u16(1U)) ? 0 : 1; }
	DECLARE_READ_LINE_MEMBER(user_reset_out) const  { return (m_user_reset  & ~u16(1U)) ? 0 : 1; }

protected:
	// device_t implementation
	virtual void device_start() override;

private:
	// output line callbacks
	devcb_write_line    m_stop_out_cb;
	devcb_write_line    m_test_out_cb;
	devcb_write_line    m_reset_4002_out_cb;
	devcb_write_line    m_user_reset_out_cb;

	// cards
	device_univ_card_interface  *m_cards[15];

	// packed line states
	u16 m_stop, m_test, m_reset_4002, m_user_reset;
};


class device_univ_card_interface : public device_slot_card_interface
{
};

} } // namespace bus::intellec4


DECLARE_DEVICE_TYPE_NS(INTELLEC4_UNIV_SLOT, bus::intellec4, univ_slot_device)
DECLARE_DEVICE_TYPE_NS(INTELLEC4_UNIV_BUS,  bus::intellec4, univ_bus_device)

SLOT_INTERFACE_EXTERN( intellec4_univ_cards );

#endif // MAME_BUS_INTELLEC4_INTELLEC4_H
