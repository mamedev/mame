// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mconfig.cpp

    Machine configuration macros and functions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "screen.h"

#include <cctype>

#include <cstring>
#include <numeric>


//**************************************************************************
//  MACHINE CONFIGURATIONS
//**************************************************************************

class machine_config::current_device_stack
{
public:
	current_device_stack(current_device_stack const &) = delete;
	current_device_stack(machine_config &host) : m_host(host), m_device(host.m_current_device) { m_host.m_current_device = nullptr; }
	~current_device_stack() { assert(!m_host.m_current_device); m_host.m_current_device = m_device; }
private:
	machine_config &m_host;
	device_t *const m_device;
};


//-------------------------------------------------
//  machine_config - constructor
//-------------------------------------------------

machine_config::machine_config(const game_driver &gamedrv, emu_options &options)
	: m_gamedrv(gamedrv)
	, m_options(options)
	, m_root_device()
	, m_default_layouts([] (char const *a, char const *b) { return 0 > std::strcmp(a, b); })
	, m_current_device(nullptr)
	, m_maximum_quantums([] (char const *a, char const *b) { return 0 > std::strcmp(a, b); })
	, m_perfect_quantum_device(nullptr, "")
{
	// add the root device
	device_add("root", gamedrv.type, 0);

	// intialize slot devices - make sure that any required devices have been allocated
	for (device_slot_interface &slot : slot_interface_enumerator(root_device()))
	{
		device_t &owner = slot.device();
		const char *slot_option_name = owner.tag() + 1;

		// figure out which device goes into this slot
		bool const has_option = options.has_slot_option(slot_option_name);
		const char *selval;
		bool is_default;
		if (!has_option)
		{
			// The only time we should be getting here is when emuopts.cpp is invoking
			// us to evaluate slot/image options, and the internal state of emuopts.cpp has
			// not caught up yet
			selval = slot.default_option();
			is_default = true;
		}
		else
		{
			const slot_option &opt = options.slot_option(slot_option_name);
			selval = opt.value().c_str();
			is_default = !opt.specified();
		}

		if (selval && *selval)
		{
			// TODO: make this thing more self-contained so it can apply itself - shouldn't need to know all this here
			device_slot_interface::slot_option const *option = slot.option(selval);
			if (option && (is_default || option->selectable()))
			{
				// create the device
				token const tok(begin_configuration(owner));
				device_t *const new_dev = device_add(option->name(), option->devtype(), option->clock());
				slot.set_card_device(new_dev);

				char const *const default_bios = option->default_bios();
				if (default_bios)
					new_dev->set_default_bios_tag(default_bios);

				auto additions = option->machine_config();
				if (additions)
					additions(new_dev);

				input_device_default const *const input_device_defaults = option->input_device_defaults();
				if (input_device_defaults)
					new_dev->set_input_default(input_device_defaults);
			}
			else
				throw emu_fatalerror("Unknown slot option '%s' in slot '%s'", selval, owner.tag()+1);
		}
	}

	// then notify all devices that their configuration is complete
	for (device_t &device : device_enumerator(root_device()))
		if (!device.configured())
			device.config_complete();
}


//-------------------------------------------------
//  ~machine_config - destructor
//-------------------------------------------------

machine_config::~machine_config()
{
}


//-------------------------------------------------
//  maximum_quantum - get smallest configured
//  maximum quantum
//-------------------------------------------------

attotime machine_config::maximum_quantum(attotime const &default_quantum) const
{
	return std::accumulate(
			m_maximum_quantums.begin(),
			m_maximum_quantums.end(),
			default_quantum,
			[] (attotime const &lhs, maximum_quantum_map::value_type const &rhs) { return (std::min)(lhs, rhs.second); });
}

//-------------------------------------------------
//  perfect_quantum_device - get device configured
//  for perfect quantum if any
//-------------------------------------------------

device_execute_interface *machine_config::perfect_quantum_device() const
{
	if (!m_perfect_quantum_device.first)
		return nullptr;

	device_t *const found(m_perfect_quantum_device.first->subdevice(m_perfect_quantum_device.second));
	if (!found)
	{
		throw emu_fatalerror(
				"Device %s relative to %s specified for perfect interleave is not present!\n",
				m_perfect_quantum_device.second,
				m_perfect_quantum_device.first->tag());
	}

	device_execute_interface *result;
	if (!found->interface(result))
	{
		throw emu_fatalerror("Device %s (%s) specified for perfect interleave does not implement device_execute_interface!\n",
				found->tag(),
				found->shortname());
	}

	return result;
}


//-------------------------------------------------
//  set_default_layout - set layout for current
//  device
//-------------------------------------------------

void machine_config::set_default_layout(internal_layout const &layout)
{
	std::pair<default_layout_map::iterator, bool> const ins(m_default_layouts.emplace(current_device().tag(), &layout));
	if (!ins.second)
		ins.first->second = &layout;
}


//-------------------------------------------------
//  set_maximum_quantum - set maximum scheduling
//  quantum for current device device
//-------------------------------------------------

void machine_config::set_maximum_quantum(attotime const &quantum)
{
	std::pair<maximum_quantum_map::iterator, bool> const ins(m_maximum_quantums.emplace(current_device().tag(), quantum));
	if (!ins.second)
		ins.first->second = quantum;
}


//-------------------------------------------------
//  device_add - configuration helper to add a
//  new device
//-------------------------------------------------

device_t *machine_config::device_add(const char *tag, device_type type, u32 clock)
{
	std::pair<const char *, device_t *> const owner(resolve_owner(tag));
	return &add_device(type.create(*this, owner.first, owner.second, clock), owner.second);
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_t *machine_config::device_replace(const char *tag, device_type type, u32 clock)
{
	std::tuple<const char *, device_t *, device_t *> const existing(prepare_replace(tag));
	std::unique_ptr<device_t> device(type.create(*this, std::get<0>(existing), std::get<1>(existing), clock));
	return &replace_device(std::move(device), *std::get<1>(existing), std::get<2>(existing));
}


//-------------------------------------------------
//  device_remove - configuration helper to
//  remove a device
//-------------------------------------------------

device_t *machine_config::device_remove(const char *tag)
{
	// find the original device by relative tag (must exist)
	assert(m_current_device);
	device_t *const device = m_current_device->subdevice(tag);
	if (device == nullptr)
	{
		osd_printf_warning("Warning: attempting to remove non-existent device '%s'\n", tag);
	}
	else
	{
		// make sure we have the old device's actual owner
		device_t *const owner = device->owner();
		assert(owner);

		// remove references to the old device
		remove_references(*device);

		// let the device's owner do the work
		owner->subdevices().remove(*device);
	}
	return nullptr;
}


//-------------------------------------------------
//  resolve_owner - get the actual owner and base
//  tag given tag relative to current context
//-------------------------------------------------

std::pair<const char *, device_t *> machine_config::resolve_owner(const char *tag) const
{
	assert(bool(m_current_device) == bool(m_root_device));
	char const *const orig_tag = tag;
	device_t *owner(m_current_device);

	// if the device path is absolute, start from the root
	if (!*tag || (':' == *tag) || ('^' == *tag))
		throw emu_fatalerror("Attempting to add device with tag containing parent references '%s'\n", orig_tag);

	// go down the path until we're done with it
	char const *next;
	while ((next = strchr(tag, ':')) != nullptr)
	{
		assert(next != tag);
		std::string_view part(tag, next - tag);
		owner = owner->subdevices().find(part);
		if (!owner)
			throw emu_fatalerror("Could not find '%s' when looking up path for device '%s'\n", part, orig_tag);
		tag = next + 1;
		if ('^' == *tag)
			throw emu_fatalerror("Attempting to add device with tag containing parent references '%s'\n", orig_tag);
	}
	assert(*tag != '\0');

	return std::make_pair(tag, owner);
}


//-------------------------------------------------
//  prepare_replace - ensure owner is present and
//  existing device is removed if necessary
//-------------------------------------------------

std::tuple<const char *, device_t *, device_t *> machine_config::prepare_replace(const char *tag)
{
	// make sure we have the old device's actual owner
	std::pair<const char *, device_t *> const owner(resolve_owner(tag));
	assert(owner.second);

	// remove references to the old device
	device_t *const old_device(owner.second->subdevice(owner.first));
	if (old_device)
		remove_references(*old_device);
	else
		throw emu_fatalerror("Attempting to replace non-existent device '%s'\n", tag);

	return std::make_tuple(owner.first, owner.second, old_device);
}


//-------------------------------------------------
//  add_device - add a new device at the correct
//  point in the hierarchy
//-------------------------------------------------

device_t &machine_config::add_device(std::unique_ptr<device_t> &&device, device_t *owner)
{
	current_device_stack const context(*this);
	if (owner)
	{
		// allocate the new device and append it to the owner's list
		device_t &result(owner->subdevices().append(std::move(device)));
		result.add_machine_configuration(*this);
		return result;
	}
	else
	{
		// allocate the root device directly
		assert(!m_root_device);
		m_root_device = std::move(device);
		m_root_device->add_machine_configuration(*this);
		return *m_root_device;
	}
}


//-------------------------------------------------
//  replace_device - substitute the new device for
//  the old one in the owner's list
//-------------------------------------------------

device_t &machine_config::replace_device(std::unique_ptr<device_t> &&device, device_t &owner, device_t *existing)
{
	current_device_stack const context(*this);
	device_t &result(existing
			? owner.subdevices().replace_and_remove(std::move(device), *existing)
			: owner.subdevices().append(std::move(device)));
	result.add_machine_configuration(*this);
	return result;
}


//-------------------------------------------------
//  remove_references - globally remove references
//  to a device about to be removed from the tree
//-------------------------------------------------

void machine_config::remove_references(device_t &device)
{
	// sanity check
	if (m_perfect_quantum_device.first == &device)
	{
		throw emu_fatalerror(
				"Removing %s device %s would make the perfect quantum device target invalid\n",
				device.shortname(),
				device.tag());
	}

	// remove default layouts and maximum quantum settings for subdevices
	char const *const tag(device.tag());
	std::size_t const taglen(std::strlen(tag));
	for (auto it = m_default_layouts.lower_bound(tag); (m_default_layouts.end() != it) && !std::strncmp(tag, it->first, taglen); )
	{
		if (!it->first[taglen] || (':' == it->first[taglen]))
			it = m_default_layouts.erase(it);
		else
			++it;
	}
	for (auto it = m_maximum_quantums.lower_bound(tag); (m_maximum_quantums.end() != it) && !std::strncmp(tag, it->first, taglen); )
	{
		if (!it->first[taglen] || (':' == it->first[taglen]))
			it = m_maximum_quantums.erase(it);
		else
			++it;
	}
}


//-------------------------------------------------
//  set_perfect_quantum - set device to base
//  scheduling interval on
//-------------------------------------------------

void machine_config::set_perfect_quantum(device_t &device, std::string tag)
{
	if (!m_current_device)
	{
		throw emu_fatalerror(
				"Perfect quantum device can only be set during configuration (set to %s relative to %s)\n",
				tag,
				device.tag());
	}

	if (m_current_device != m_root_device.get())
	{
		throw emu_fatalerror(
				"Perfect quantum device can only be set by the root device (set to %s relative to %s while configuring %s device %s)\n",
				tag,
				device.tag(),
				m_current_device->shortname(),
				m_current_device->tag());
	}

	m_perfect_quantum_device.first = tag.empty() ? nullptr : &device;
	m_perfect_quantum_device.second = std::move(tag);
}
