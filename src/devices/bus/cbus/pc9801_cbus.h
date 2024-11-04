// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************

                            <B>          <A>
                                +------+
                            GND |[ 01 ]| GND
                             V1 |[ 02 ]| V1
                             V2 |[ 03 ]| V2
                          DB001 |[ 04 ]| AB001
                          DB011 |[ 05 ]| AB011
                          DB021 |[ 06 ]| AB021
                          DB031 |[ 07 ]| AB031
                          DB041 |[ 08 ]| AB041
                          DB051 |[ 09 ]| AB051
                          DB061 |[ 10 ]| AB061
                            GND |[ 11 ]| GND
                          DB071 |[ 12 ]| AB071
                          DB081 |[ 13 ]| AB081
                          DB091 |[ 14 ]| AB091
                          DB101 |[ 15 ]| AB101
                          DB111 |[ 16 ]| AB111
                          DB121 |[ 17 ]| AB121
                          DB131 |[ 18 ]| AB131
                          DB141 |[ 19 ]| AB141
                          DB151 |[ 20 ]| AB151
                            GND |[ 21 ]| GND
                          +12 V |[ 22 ]| AB161
                          +12 V |[ 23 ]| AB171
            "INT0"         IR31 |[ 24 ]| AB181
            "INT1"         IR51 |[ 25 ]| AB191
            "INT2"         IR61 |[ 26 ]| AB201
            "INT3"         IR91 |[ 27 ]| AB211
            "INT4"  IR101/IR111 |[ 28 ]| AB221
            "INT5"        IR121 |[ 29 ]| AB231
            "INT6"        IR131 |[ 30 ]| INT0
                            GND |[ 31 ]| GND
                          -12 V |[ 32 ]| IOCHK0
                          -12 V |[ 33 ]| IOR0
                         RESET0 |[ 34 ]| IOW0
                         DACK00 |[ 35 ]| MRC0
                  DACK30/DACK20 |[ 36 ]| MWC0
                          DRQ00 |[ 37 ]| S00        (INTA0)
                    DRQ30/DRQ20 |[ 38 ]| S10        (NOWAIT0)
                          WORD0 |[ 39 ]| S20        (SALE1)
            (EXHRQ10)   CPKILL0 |[ 40 ]| LOCK       (MACS0)
                            GND |[ 41 ]| GND
            (EXHLA10)     RQGT0 |[ 42 ]| CPUENB10
                         DMATC0 |[ 43 ]| RFSH0
                           NMI0 |[ 44 ]| BHE0
                           MWE0 |[ 45 ]| IORDY1
            (EXHLA20)    HLDA00 |[ 46 ]| SCLK1
            (EXHRQ20)     HRQ00 |[ 47 ]| S18CLK1    = 307.2 kHz
            (SUBSRQ1)   DMAHLD0 |[ 48 ]| POWER0
                           +5 V |[ 49 ]| +5 V
                           +5 V |[ 50 ]| +5 V
                                +------+
                            <B>          <A>

**********************************************************************/
#ifndef MAME_MACHINE_PC9801_CBUS_H
#define MAME_MACHINE_PC9801_CBUS_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************




//class pc9801_slot_device;

#if 0
class device_pc9801_slot_card_interface : public device_slot_card_interface
{
	friend class pc9801_slot_device;

public:
	// construction/destruction
	device_pc9801_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pc9801_card_interface();
};
#endif

// ======================> pc9801_slot_device

class pc9801_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	pc9801_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pc9801_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pc9801_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	// configuration access
	template<std::size_t Line> auto int_cb() { return m_int_callback[Line].bind(); }

	address_space &program_space() const { return *m_memspace; }
	address_space &io_space() const { return *m_iospace; }
	template<int I> void int_w(bool state) { m_int_callback[I](state); }
	template<typename R, typename W> void install_io(offs_t start, offs_t end, R rhandler, W whandler);

	void flush_install_io(const char *client_tag, u16 old_io, u16 new_io, u16 size, read8sm_delegate rhandler, write8sm_delegate whandler);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
//  device_pc9801_slot_card_interface *m_card;
	required_address_space m_memspace;
	required_address_space m_iospace;
	devcb_write_line::array<7> m_int_callback;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801CBUS_SLOT, pc9801_slot_device)

#endif // MAME_MACHINE_PC9801_CBUS_H
