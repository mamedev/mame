// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inputdev.h

    Input devices, items and device classes.

***************************************************************************/

#ifndef MAME_EMU_INPUTDEV_H
#define MAME_EMU_INPUTDEV_H

#pragma once

#include "interface/inputdev.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> joystick_map

// a 9x9 joystick map
class joystick_map
{
public:
	// construction/destruction
	joystick_map();
	joystick_map(const joystick_map &src) { copy(src); }

	// operators
	joystick_map &operator=(const joystick_map &src) { if (this != &src) copy(src); return *this; }

	// parse from a string
	bool parse(const char *mapstring);

	// create a friendly string
	std::string to_string() const;

	// update the state of a live map
	u8 update(s32 xaxisval, s32 yaxisval);

	// joystick mapping codes
	static constexpr u8 JOYSTICK_MAP_NEUTRAL = 0x00;
	static constexpr u8 JOYSTICK_MAP_LEFT    = 0x01;
	static constexpr u8 JOYSTICK_MAP_RIGHT   = 0x02;
	static constexpr u8 JOYSTICK_MAP_UP      = 0x04;
	static constexpr u8 JOYSTICK_MAP_DOWN    = 0x08;
	static constexpr u8 JOYSTICK_MAP_STICKY  = 0x0f;

private:
	// internal helpers
	void copy(const joystick_map &src)
	{
		memcpy(m_map, src.m_map, sizeof(m_map));
		m_lastmap = JOYSTICK_MAP_NEUTRAL;
		m_origstring = src.m_origstring;
	}

	// internal state
	u8                      m_map[9][9];            // 9x9 grid
	u8                      m_lastmap;              // last value returned (for sticky tracking)
	std::string             m_origstring;           // originally parsed string
};


// ======================> input_device_item

// a single item on an input device
class input_device_item
{
public:
	using item_get_state_func = osd::input_device::item_get_state_func;

	virtual ~input_device_item();

	// getters
	input_device &device() const { return m_device; }
	input_manager &manager() const;
	running_machine &machine() const;
	const std::string &name() const { return m_name; }
	void *internal() const { return m_internal; }
	input_item_id itemid() const { return m_itemid; }
	input_item_class itemclass() const { return m_itemclass; }
	input_code code() const;
	const std::string &token() const { return m_token; }
	s32 current() const { return m_current; }

	// helpers
	s32 update_value();
	bool check_axis(input_item_modifier modifier, s32 memory);

	// readers
	virtual s32 read_as_switch(input_item_modifier modifier) = 0;
	virtual s32 read_as_relative(input_item_modifier modifier) = 0;
	virtual s32 read_as_absolute(input_item_modifier modifier) = 0;
	virtual bool item_check_axis(input_item_modifier modifier, s32 memory) = 0;

protected:
	// construction/destruction
	input_device_item(
			input_device &device,
			std::string_view name,
			std::string_view tokenhint,
			void *internal,
			input_item_id itemid,
			item_get_state_func getstate,
			input_item_class itemclass);

	// internal state
	input_device &          m_device;               // reference to our owning device
	std::string             m_name;                 // string name of item
	void *                  m_internal;             // internal callback pointer
	input_item_id           m_itemid;               // originally specified item id
	input_item_class        m_itemclass;            // class of the item
	item_get_state_func     m_getstate;             // get state callback
	std::string             m_token;                // tokenized name for non-standard items

	// live state
	s32                     m_current;              // current raw value
};


// ======================> input_device

// a logical device of a given class that can provide input
class input_device : public osd::input_device
{
	friend class input_class;

public:
	// construction/destruction
	input_device(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal);
	virtual ~input_device();

	// getters
	input_manager &manager() const { return m_manager; }
	running_machine &machine() const { return m_manager.machine(); }
	input_device_class devclass() const { return device_class(); }
	const std::string &name() const { return m_name; }
	const std::string &id() const { return m_id; }
	int devindex() const { return m_devindex; }
	input_device_item *item(input_item_id index) const { return m_item[index].get(); }
	const assignment_vector &default_assignments() const { return m_default_assignments; }
	input_item_id maxitem() const { return m_maxitem; }
	void *internal() const { return m_internal; }
	s32 threshold() const { return m_threshold; }
	bool steadykey_enabled() const { return m_steadykey_enabled; }

	// setters
	void set_devindex(int devindex) { m_devindex = devindex; }

	// interface for host input device
	virtual input_item_id add_item(
			std::string_view name,
			std::string_view tokenhint,
			input_item_id itemid,
			item_get_state_func getstate,
			void *internal) override;
	virtual void set_default_assignments(assignment_vector &&assignments) override;

	// helpers
	s32 adjust_absolute(s32 value) const { return adjust_absolute_value(value); }
	bool match_device_id(std::string_view deviceid) const;

protected:
	// specific overrides
	virtual input_device_class device_class() const = 0;
	virtual s32 adjust_absolute_value(s32 value) const { return value; }

private:
	// internal state
	input_manager &         m_manager;              // reference to input manager
	std::string             m_name;                 // string name of device
	std::string             m_id;                   // id of device
	int                     m_devindex;             // device index of this device
	std::unique_ptr<input_device_item> m_item[ITEM_ID_ABSOLUTE_MAXIMUM+1]; // array of pointers to items
	assignment_vector       m_default_assignments;  // additional assignments
	input_item_id           m_maxitem;              // maximum item index
	void *const             m_internal;             // internal callback pointer

	s32 const               m_threshold;            // threshold for treating absolute axis as active
	bool const              m_steadykey_enabled;    // steadykey enabled for keyboards
};


// ======================> input_device_keyboard

class input_device_keyboard : public input_device
{
public:
	// construction/destruction
	input_device_keyboard(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal);

	// helpers
	void apply_steadykey() const;

protected:
	// specific overrides
	virtual input_device_class device_class() const override { return DEVICE_CLASS_KEYBOARD; }
};


// ======================> input_device_mouse

class input_device_mouse : public input_device
{
public:
	// construction/destruction
	input_device_mouse(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal);

protected:
	// specific overrides
	virtual input_device_class device_class() const override { return DEVICE_CLASS_MOUSE; }
};


// ======================> input_device_lightgun

class input_device_lightgun : public input_device
{
public:
	// construction/destruction
	input_device_lightgun(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal);

protected:
	// specific overrides
	virtual input_device_class device_class() const override { return DEVICE_CLASS_LIGHTGUN; }
};


// ======================> input_device_joystick

// a logical device of a given class that can provide input
class input_device_joystick : public input_device
{
public:
	// construction/destruction
	input_device_joystick(input_manager &manager, std::string_view _name, std::string_view _id, void *_internal);

	// getters
	joystick_map &joymap() { return m_joymap; }

	// item management
	void set_joystick_map(const joystick_map &map) { m_joymap = map; }

protected:
	// specific overrides
	virtual input_device_class device_class() const override { return DEVICE_CLASS_JOYSTICK; }
	virtual s32 adjust_absolute_value(s32 value) const override;

private:
	// joystick information
	joystick_map    m_joymap;      // joystick map for this device
	s32 const       m_deadzone;    // deadzone for joystick
	s32 const       m_saturation;  // saturation position for joystick
	s64 const       m_range;       // difference between saturation and deadzone
};


// ======================> input_class

// a class of device that provides input
class input_class
{
public:
	// construction/destruction
	input_class(input_manager &manager, input_device_class devclass, const char *name, bool enabled = false, bool multi = false);
	virtual ~input_class();

	// getters
	input_manager &manager() const { return m_manager; }
	running_machine &machine() const  { return m_manager.machine(); }
	input_device *device(int index) const { return (index <= m_maxindex) ? m_device[index].get() : nullptr; }
	input_device_class devclass() const { return m_devclass; }
	const char *name() const { return m_name; }
	int maxindex() const { return m_maxindex; }
	bool enabled() const { return m_enabled; }
	bool multi() const { return m_multi; }

	// setters
	void enable(bool state = true) { m_enabled = state; }
	void set_multi(bool multi = true) { m_multi = multi; }

	// device management
	input_device &add_device(std::string_view name, std::string_view id, void *internal = nullptr);

	// misc helpers
	input_item_class standard_item_class(input_item_id itemid) const;
	void remap_device_index(int oldindex, int newindex);

protected:
	// specific overrides
	virtual std::unique_ptr<input_device> make_device(std::string_view name, std::string_view id, void *internal) = 0;

private:
	// internal helpers
	input_device &add_device(std::unique_ptr<input_device> &&new_device);
	int newindex(input_device &device);

	// internal state
	input_manager &         m_manager;              // reference to our manager
	std::unique_ptr<input_device> m_device[DEVICE_INDEX_MAXIMUM]; // array of devices in this class
	input_device_class      m_devclass;             // our device class
	const char *            m_name;                 // name of class (used for option settings)
	int                     m_maxindex;             // maximum populated index
	bool                    m_enabled;              // is this class enabled?
	bool                    m_multi;                // are multiple instances of this class allowed?
};


// ======================> input_class_keyboard

// class of device that provides keyboard input
class input_class_keyboard : public input_class
{
public:
	// construction/destruction
	input_class_keyboard(input_manager &manager);

protected:
	// specific overrides
	virtual std::unique_ptr<input_device> make_device(std::string_view name, std::string_view id, void *internal) override
	{
		return std::make_unique<input_device_keyboard>(manager(), name, id, internal);
	}

private:
	// internal helpers
	void frame_callback();
};


// ======================> input_class_mouse

// class of device that provides mouse input
class input_class_mouse : public input_class
{
public:
	// construction/destruction
	input_class_mouse(input_manager &manager);

protected:
	// specific overrides
	virtual std::unique_ptr<input_device> make_device(std::string_view name, std::string_view id, void *internal) override
	{
		return std::make_unique<input_device_mouse>(manager(), name, id, internal);
	}
};


// ======================> input_class_lightgun

// class of device that provides lightgun input
class input_class_lightgun : public input_class
{
public:
	// construction/destruction
	input_class_lightgun(input_manager &manager);

protected:
	// specific overrides
	virtual std::unique_ptr<input_device> make_device(std::string_view name, std::string_view id, void *internal) override
	{
		return std::make_unique<input_device_lightgun>(manager(), name, id, internal);
	}
};


// ======================> input_class_joystick

// class of device that provides joystick input
class input_class_joystick : public input_class
{
public:
	// construction/destruction
	input_class_joystick(input_manager &manager);

	// standard joystick maps
	static const char map_8way[];

protected:
	// specific overrides
	virtual std::unique_ptr<input_device> make_device(std::string_view name, std::string_view id, void *internal) override
	{
		return std::make_unique<input_device_joystick>(manager(), name, id, internal);
	}
};


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// input_device_item helpers
inline input_manager &input_device_item::manager() const { return m_device.manager(); }
inline running_machine &input_device_item::machine() const { return m_device.machine(); }
inline input_code input_device_item::code() const { return input_code(m_device.devclass(), m_device.devindex(), m_itemclass, ITEM_MODIFIER_NONE, m_itemid); }
inline s32 input_device_item::update_value() { return m_current = (*m_getstate)(m_device.internal(), m_internal); }

#endif  // MAME_EMU_INPUTDEV_H
