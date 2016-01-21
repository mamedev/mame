// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiout.h

    MIDI Out image device

*********************************************************************/

#ifndef __MIDIOUT_H__
#define __MIDIOUT_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/


#define MCFG_MIDIOUT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MIDIOUT, 0)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class midiout_device :    public device_t,
						public device_image_interface,
						public device_serial_interface
{
public:
	// construction/destruction
	midiout_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const override { return IO_MIDIOUT; }
	virtual bool is_readable()  const override { return 0; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *file_extensions() const override { return "mid"; }
	virtual bool core_opens_image_file() const override { return FALSE; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	virtual void tx(UINT8 state) { rx_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_config_complete() override;

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte

private:
	osd_midi_device *m_midi;
};

// device type definition
extern const device_type MIDIOUT;

// device iterator
typedef device_type_iterator<&device_creator<midiout_device>, midiout_device> midiout_device_iterator;

#endif /* __MIDIOUT_H__ */
