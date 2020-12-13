// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "iptseqpoll.h"

#include "inputdev.h"
#include "natkeyboard.h"
#include "render.h"
#include "uiinput.h"

#include <cstring>


namespace {

struct natkbd_kbd_dev
{
	natkbd_kbd_dev(natural_keyboard &m, std::size_t i) : manager(m), index(i) { }

	natural_keyboard &manager;
	std::size_t index;
};


struct natkbd_kbd_list
{
	natkbd_kbd_list(natural_keyboard &m) : manager(m) { }

	natural_keyboard &manager;
};

} // anonymous namespace


namespace sol {

template <> struct is_container<natkbd_kbd_list> : std::true_type { };


template <>
struct usertype_container<natkbd_kbd_list> : lua_engine::immutable_container_helper<natkbd_kbd_list>
{
private:
	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		natkbd_kbd_dev &i(stack::unqualified_get<user<natkbd_kbd_dev> >(L, 1));
		if (i.manager.keyboard_count() <= i.index)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.index + 1);
		else
			result = stack::push(L, i.manager.keyboard_device(i.index).tag());
		result += stack::push(L, i);
		++i.index;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<natkbd_kbd_dev> >(L, self.manager, 0);
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 < index) && (self.manager.keyboard_count() >= index))
			return stack::push(L, natkbd_kbd_dev(self.manager, index - 1));
		else
			return stack::push(L, lua_nil);
	}

	static int get(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		char const *const tag(stack::unqualified_get<char const *>(L));
		for (std::size_t i = 0; self.manager.keyboard_count() > i; ++i)
		{
			if (!std::strcmp(self.manager.keyboard_device(i).tag(), tag))
				return stack::push(L, natkbd_kbd_dev(self.manager, i));
		}
		return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int size(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		return stack::push(L, self.manager.keyboard_count());
	}

	static int empty(lua_State *L)
	{
		natkbd_kbd_list &self(get_self(L));
		return stack::push(L, !self.manager.keyboard_count());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};

} // namespace sol


//-------------------------------------------------
//  initialize_input - register input user types
//-------------------------------------------------

void lua_engine::initialize_input(sol::table &emu)
{

	static const enum_parser<input_seq_type, 3> s_seq_type_parser =
	{
		{ "standard", SEQ_TYPE_STANDARD },
		{ "increment", SEQ_TYPE_INCREMENT },
		{ "decrement", SEQ_TYPE_DECREMENT },
	};


/* ioport_manager library
 *
 * manager:machine():ioport()
 *
 * ioport:count_players() - get count of player controllers
 * ioport:type_group(type, player)
 * ioport:type_seq(type, player, seqtype) - get input sequence for ioport type/player
 *
 * ioport.ports[] - ioports table (k=tag, v=ioport_port)
 * ioport.natkeyboard - get natural keyboard manager
 */

	auto ioport_manager_type = sol().registry().new_usertype<ioport_manager>("ioport", sol::no_constructor);
	ioport_manager_type["count_players"] = &ioport_manager::count_players;
	ioport_manager_type["type_group"] = &ioport_manager::type_group;
	ioport_manager_type["type_seq"] = &ioport_manager::type_seq;
	ioport_manager_type["ports"] = sol::property([] (ioport_manager &im) { return tag_object_ptr_map<ioport_list>(im.ports()); });
	ioport_manager_type["natkeyboard"] = sol::property(&ioport_manager::natkeyboard);


/* natural_keyboard library
 *
 * manager:machine():ioport().natkeyboard
 *
 * natkeyboard:post(text) - post data to natural keyboard
 * natkeyboard:post_coded(text) - post data to natural keyboard
 * natkeyboard:paste() - paste host clipboard text
 * natkeyboard:dump() - returns human-readable description of character mappings
 *
 * natkeyboard.empty - is the natural keyboard buffer empty?
 * natkeyboard.full - is the natural keyboard buffer full?
 * natkeyboard.can_post - does the system support posting characters via natural keyboard?
 * natkeyboard.is_posting - is a post operation currently in progress?
 * natkeyboard.in_use - is natural keyboard mode enabled (read/write)?
 * natkeyboard.keyboards[] - get keyboard devices in system (k=tag, v=natkbd_kbd_dev)
 */

	auto natkeyboard_type = sol().registry().new_usertype<natural_keyboard>("natkeyboard", sol::no_constructor);
	natkeyboard_type["post"] = [] (natural_keyboard &nat, std::string const &text) { nat.post_utf8(text); };
	natkeyboard_type["post_coded"] = [] (natural_keyboard &nat, std::string const &text) { nat.post_coded(text); };
	natkeyboard_type["paste"] = &natural_keyboard::paste;
	natkeyboard_type["dump"] = static_cast<std::string (natural_keyboard::*)() const>(&natural_keyboard::dump);
	natkeyboard_type["empty"] = sol::property(&natural_keyboard::empty);
	natkeyboard_type["full"] = sol::property(&natural_keyboard::full);
	natkeyboard_type["can_post"] = sol::property(&natural_keyboard::can_post);
	natkeyboard_type["is_posting"] = sol::property(&natural_keyboard::is_posting);
	natkeyboard_type["in_use"] = sol::property(&natural_keyboard::in_use, &natural_keyboard::set_in_use);
	natkeyboard_type["keyboards"] = sol::property([] (natural_keyboard &nat) { return natkbd_kbd_list(nat); });


/* natkbd_kbd_dev library
 *
 * manager:machine():ioport().natkeyboard.keyboards[tag]
 *
 * keyboard.device - underlying device that the inputs belong to
 * keyboard.tag - absolute tag of the device
 * keyboard.basetag - last component of the device tag ("root" for root device)
 * keyboard.name - device type full name
 * keyboard.shortname - device type short name
 * keyboard.is_keypad - does the device have keypad inputs but no keyboard inputs?
 * keyboard.enabled - are the device's keyboard/keypad inputs enabled (read/write)?
 */

	auto natkbddev_type = sol().registry().new_usertype<natkbd_kbd_dev>("natkeyboard_device", sol::no_constructor);
	natkbddev_type["device"] = sol::property([] (natkbd_kbd_dev const &kbd) -> device_t & { return kbd.manager.keyboard_device(kbd.index); });
	natkbddev_type["tag"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).tag(); });
	natkbddev_type["basetag"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).basetag(); });
	natkbddev_type["name"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).name(); });
	natkbddev_type["shortname"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_device(kbd.index).shortname(); });
	natkbddev_type["is_keypad"] = sol::property([] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_is_keypad(kbd.index); });
	natkbddev_type["enabled"] = sol::property(
			[] (natkbd_kbd_dev const &kbd) { return kbd.manager.keyboard_enabled(kbd.index); },
			[] (natkbd_kbd_dev &kbd, bool enable)
			{
				if (enable)
					kbd.manager.enable_keyboard(kbd.index);
				else
					kbd.manager.disable_keyboard(kbd.index);
			});


/* ioport_port library
 *
 * manager:machine():ioport().ports[port_tag]
 *
 * port:read() - get port value
 * port:write(val, mask) - set port to value & mask (output fields only, for other fields use field:set_value(val))
 * port:field(mask) - get ioport_field for port and mask
 *
 * port.device - get device that the port belongs to
 * port.tag - get port tag
 * port.active - get port status
 * port.live - get port ioport_port_live (TODO: not usable from lua as of now)
 * port.fields[] - get ioport_field table (k=name, v=ioport_field)
 */

	auto ioport_port_type = sol().registry().new_usertype<ioport_port>("ioport_port", "new", sol::no_constructor);
	ioport_port_type["read"] = &ioport_port::read;
	ioport_port_type["write"] = &ioport_port::write;
	ioport_port_type["field"] = &ioport_port::field;
	ioport_port_type["device"] = sol::property(&ioport_port::device);
	ioport_port_type["tag"] = sol::property(&ioport_port::tag);
	ioport_port_type["active"] = sol::property(&ioport_port::active);
	ioport_port_type["live"] = sol::property(&ioport_port::live);
	ioport_port_type["fields"] = sol::property(
			[this] (ioport_port &p)
			{
				sol::table f_table = sol().create_table();
				// parse twice for custom and default names, default has priority
				for (ioport_field &field : p.fields())
				{
					if (field.type_class() != INPUT_CLASS_INTERNAL)
						f_table[field.name()] = &field;
				}
				for (ioport_field &field : p.fields())
				{
					if (field.type_class() != INPUT_CLASS_INTERNAL)
					{
						if (field.specific_name())
							f_table[field.specific_name()] = &field;
						else
							f_table[field.manager().type_name(field.type(), field.player())] = &field;
					}
				}
				return f_table;
			});


/* ioport_field library
 *
 * manager:machine():ioport().ports[port_tag].fields[field_name]
 *
 * field:set_value(value)
 * field:set_input_seq(seq_type, seq)
 * field:input_seq(seq_type)
 * field:set_default_input_seq(seq_type, seq)
 * field:default_input_seq(seq_type)
 * field:keyboard_codes(which)
 *
 * field.device - get associated device_t
 * field.port - get associated ioport_port
 * field.live - get ioport_field_live
 * field.name
 * field.default_name
 * field.player
 * field.mask
 * field.defvalue
 * field.sensitivity
 * field.way - amount of available directions
 * field.type_class
 * field.is_analog
 * field.is_digital_joystick
 * field.enabled
 * field.optional
 * field.cocktail
 * field.toggle - whether field is a toggle
 * field.rotated
 * field.analog_reverse
 * field.analog_reset
 * field.analog_wraps
 * field.analog_invert
 * field.impulse
 * field.type
 * field.crosshair_scale
 * field.crosshair_offset
 * field.user_value
 *
 * field.settings[] - ioport_setting table (k=value, v=name)
 */

	auto ioport_field_type = sol().registry().new_usertype<ioport_field>("ioport_field", sol::no_constructor);
	ioport_field_type["set_value"] = &ioport_field::set_value;
	ioport_field_type["set_input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string, const input_seq &seq)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			ioport_field::user_settings settings;
			f.get_user_settings(settings);
			settings.seq[seq_type] = seq;
			f.set_user_settings(settings);
		};
	ioport_field_type["input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.seq(seq_type);
		};
	ioport_field_type["set_default_input_seq"] =
		[] (ioport_field &f, std::string const &seq_type_string, input_seq const &seq)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			f.set_defseq(seq_type, seq);
		};
	ioport_field_type["default_input_seq"] =
		[] (ioport_field &f, const std::string &seq_type_string)
		{
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.defseq(seq_type);
		};
	ioport_field_type["keyboard_codes"] =
		[this] (ioport_field &f, int which)
		{
			sol::table result = sol().create_table();
			int index = 1;
			for (char32_t code : f.keyboard_codes(which))
				result[index++] = code;
			return result;
		};
	ioport_field_type["device"] = sol::property(&ioport_field::device);
	ioport_field_type["port"] = sol::property(&ioport_field::port);
	ioport_field_type["name"] = sol::property(&ioport_field::name);
	ioport_field_type["default_name"] = sol::property(
			[] (ioport_field &f)
			{
				return f.specific_name() ? f.specific_name() : f.manager().type_name(f.type(), f.player());
			});
	ioport_field_type["player"] = sol::property(&ioport_field::player, &ioport_field::set_player);
	ioport_field_type["mask"] = sol::property(&ioport_field::mask);
	ioport_field_type["defvalue"] = sol::property(&ioport_field::defvalue);
	ioport_field_type["sensitivity"] = sol::property(&ioport_field::sensitivity);
	ioport_field_type["way"] = sol::property(&ioport_field::way);
	ioport_field_type["type_class"] = sol::property(
			[] (ioport_field &f)
			{
				switch (f.type_class())
				{
				case INPUT_CLASS_KEYBOARD:      return "keyboard";
				case INPUT_CLASS_CONTROLLER:    return "controller";
				case INPUT_CLASS_CONFIG:        return "config";
				case INPUT_CLASS_DIPSWITCH:     return "dipswitch";
				case INPUT_CLASS_MISC:          return "misc";
				default:                        break;
				}
				throw false;
			});
	ioport_field_type["is_analog"] = sol::property(&ioport_field::is_analog);
	ioport_field_type["is_digital_joystick"] = sol::property(&ioport_field::is_digital_joystick);
	ioport_field_type["enabled"] = sol::property(&ioport_field::enabled);
	ioport_field_type["optional"] = sol::property(&ioport_field::optional);
	ioport_field_type["cocktail"] = sol::property(&ioport_field::cocktail);
	ioport_field_type["toggle"] = sol::property(&ioport_field::toggle);
	ioport_field_type["rotated"] = sol::property(&ioport_field::rotated);
	ioport_field_type["analog_reverse"] = sol::property(&ioport_field::analog_reverse);
	ioport_field_type["analog_reset"] = sol::property(&ioport_field::analog_reset);
	ioport_field_type["analog_wraps"] = sol::property(&ioport_field::analog_wraps);
	ioport_field_type["analog_invert"] = sol::property(&ioport_field::analog_invert);
	ioport_field_type["impulse"] = sol::property(&ioport_field::impulse);
	ioport_field_type["type"] = sol::property(&ioport_field::type);
	ioport_field_type["live"] = sol::property(&ioport_field::live);
	ioport_field_type["crosshair_scale"] = sol::property(&ioport_field::crosshair_scale, &ioport_field::set_crosshair_scale);
	ioport_field_type["crosshair_offset"] = sol::property(&ioport_field::crosshair_offset, &ioport_field::set_crosshair_offset);
	ioport_field_type["user_value"] = sol::property(
			[] (ioport_field &f)
			{
				ioport_field::user_settings settings;
				f.get_user_settings(settings);
				return settings.value;
			},
			[] (ioport_field &f, ioport_value val)
			{
				ioport_field::user_settings settings;
				f.get_user_settings(settings);
				settings.value = val;
				f.set_user_settings(settings);
			});
	ioport_field_type["settings"] = sol::property(
			[this] (ioport_field &f)
			{
				sol::table result = sol().create_table();
				for (ioport_setting &setting : f.settings())
					if (setting.enabled())
						result[setting.value()] = setting.name();
				return result;
			});


/* ioport_field_live library
 *
 * manager:machine():ioport().ports[port_tag].fields[field_name].live
 *
 * live.name
 */

	auto ioport_field_live_type = sol().registry().new_usertype<ioport_field_live>("ioport_field_live", sol::no_constructor);
	ioport_field_live_type["name"] = &ioport_field_live::name;


/* input_manager library
 *
 * manager:machine():input()
 *
 * input:code_value(code) -
 * input:code_pressed(code) - get pressed state for input_code
 * input:code_pressed_once(code) -
 * input:code_name(code) - get code friendly name
 * input:code_to_token(code) - get KEYCODE_* string token for code
 * input:code_from_token(token) - get input_code for KEYCODE_* string token
 * input:seq_pressed(seq) - get pressed state for input_seq
 * input:seq_clean(seq) - clean the seq and remove invalid elements
 * input:seq_name(seq) - get seq friendly name
 * input:seq_to_tokens(seq) - get KEYCODE_* string tokens for seq
 * input:seq_from_tokens(tokens) - get input_seq for multiple space separated KEYCODE_* string tokens
 * input:sequence_poller() - get an input sequence poller
 *
 * input.device_classes[] - returns device classes (k=name, v=input_device_class)
 */

	auto input_type = sol().registry().new_usertype<input_manager>("input", sol::no_constructor);
	input_type["code_value"] = &input_manager::code_value;
	input_type["code_pressed"] = &input_manager::code_pressed;
	input_type["code_pressed_once"] = &input_manager::code_pressed_once;
	input_type["code_name"] = &input_manager::code_name;
	input_type["code_to_token"] = &input_manager::code_to_token;
	input_type["code_from_token"] = &input_manager::code_from_token;
	input_type["seq_pressed"] = &input_manager::seq_pressed;
	input_type["seq_clean"] = &input_manager::seq_clean;
	input_type["seq_name"] = &input_manager::seq_name;
	input_type["seq_to_tokens"] = &input_manager::seq_to_tokens;
	input_type["seq_from_tokens"] =
		[] (input_manager &input, const char *tokens)
		{
			input_seq seq;
			input.seq_from_tokens(seq, tokens);
			return seq;
		};
	input_type["sequence_poller"] = [] (input_manager &input) { return input_sequence_poller(input); };
	input_type["device_classes"] = sol::property(
			[this] (input_manager &input)
			{
				sol::table result = sol().create_table();
				for (input_device_class devclass_id = DEVICE_CLASS_FIRST_VALID; devclass_id <= DEVICE_CLASS_LAST_VALID; devclass_id++)
				{
					input_class &devclass = input.device_class(devclass_id);
					result[devclass.name()] = &devclass;
				}
				return result;
			});


/* input_sequence_poller library
 *
 * manager:machine():input():seq_poll()
 *
 * poller:start(class, [opt] start_seq) - start polling for input_item_class passed as string
 *                                                (switch/abs[olute]/rel[ative]/max[imum])
 * poller:poll() - poll once, returns true if input was fetched
 *
 * poller.sequence - get current input_seq
 * poller.valid - true if input sequence is valid
 * poller.modified - true if input sequence was modified
 */

	auto seqpoll_type = sol().registry().new_usertype<input_sequence_poller>("input_seq_poller", sol::no_constructor);
	seqpoll_type["start"] =
		[] (input_sequence_poller &poller, char const *cls_string, sol::object seq)
		{
			input_item_class cls;
			if (!strcmp(cls_string, "switch"))
				cls = ITEM_CLASS_SWITCH;
			else if (!strcmp(cls_string, "absolute") || !strcmp(cls_string, "abs"))
				cls = ITEM_CLASS_ABSOLUTE;
			else if (!strcmp(cls_string, "relative") || !strcmp(cls_string, "rel"))
				cls = ITEM_CLASS_RELATIVE;
			else if (!strcmp(cls_string, "maximum") || !strcmp(cls_string, "max"))
				cls = ITEM_CLASS_MAXIMUM;
			else
				cls = ITEM_CLASS_INVALID;

			if (seq.is<sol::user<input_seq>>())
				poller.start(cls, seq.as<input_seq>());
			else
				poller.start(cls);
		};
	seqpoll_type["poll"] = &input_sequence_poller::poll;
	seqpoll_type["sequence"] = sol::property(&input_sequence_poller::sequence);
	seqpoll_type["valid"] = sol::property(&input_sequence_poller::valid);
	seqpoll_type["modified"] = sol::property(&input_sequence_poller::modified);


/* input_class library
 *
 * manager:machine():input().device_classes[devclass]
 *
 * devclass.name
 * devclass.enabled
 * devclass.multi
 * devclass.devices[]
 */

	auto input_class_type = sol().registry().new_usertype<input_class>("input_class", "new", sol::no_constructor);
	input_class_type["name"] = sol::property(&input_class::name);
	input_class_type["enabled"] = sol::property(&input_class::enabled, &input_class::enable);
	input_class_type["multi"] = sol::property(&input_class::multi, &input_class::set_multi);
	input_class_type["devices"] = sol::property(
			[this] (input_class &devclass)
			{
				sol::table result = sol().create_table();
				int index = 1;
				for (int devindex = 0; devindex <= devclass.maxindex(); devindex++)
				{
					input_device *dev = devclass.device(devindex);
					if (dev)
						result[index++] = dev;
				}
				return result;
			});


/* input_device library
 *
 * manager:machine():input().device_classes[devclass].devices[index]
 *
 * device.name -
 * device.id -
 * device.devindex -
 * device.items[] -
 */

	auto input_device_type = sol().registry().new_usertype<input_device>("input_device", "new", sol::no_constructor);
	input_device_type["name"] = sol::property(&input_device::name);
	input_device_type["id"] = sol::property(&input_device::id);
	input_device_type["devindex"] = sol::property(&input_device::devindex);
	input_device_type["items"] = sol::property(
			[this] (input_device &dev)
			{
				sol::table result = sol().create_table();
				for (input_item_id id = ITEM_ID_FIRST_VALID; id < dev.maxitem(); id++)
				{
					input_device_item *item = dev.item(id);
					if (item)
						result[id] = dev.item(id);
				}
				return result;
			});


/* input_device_item library
 *
 * manager:machine():input().device_classes[devclass].devices[index].items[item_id]
 *
 * item.name -
 * item.code -
 * item.token -
 * item.current -
 */

	auto input_device_item_type = sol().registry().new_usertype<input_device_item>("input_device_item", "new", sol::no_constructor);
	input_device_item_type["name"] = sol::property(&input_device_item::name);
	input_device_item_type["code"] = sol::property(&input_device_item::code);
	input_device_item_type["token"] = sol::property(&input_device_item::token);
	input_device_item_type["current"] = sol::property(&input_device_item::current);


/* ui_input_manager library
 *
 * manager:machine():uiinput()
 *
 * uiinput:find_mouse() - return x, y, button state, ui render target
 * uiinput:pressed(key) - get pressed state for ui key
 * uiinput.presses_enabled - enable/disable ui key presses
 */

	auto uiinput_type = sol().registry().new_usertype<ui_input_manager>("uiinput", "new", sol::no_constructor);
	uiinput_type.set("find_mouse", [](ui_input_manager &ui) {
			int32_t x, y;
			bool button;
			render_target *rt = ui.find_mouse(&x, &y, &button);
			return std::tuple<int32_t, int32_t, bool, render_target *>(x, y, button, rt);
		});
	uiinput_type.set("pressed", &ui_input_manager::pressed);
	uiinput_type.set("presses_enabled", sol::property(&ui_input_manager::presses_enabled, &ui_input_manager::set_presses_enabled));

}
