// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "inputdev.h"
#include "natkeyboard.h"
#include "render.h"
#include "uiinput.h"


//-------------------------------------------------
//  initialize_input - register input user types
//-------------------------------------------------

void lua_engine::initialize_input()
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
 */

	auto ioport_manager_type = sol().registry().new_usertype<ioport_manager>("ioport", "new", sol::no_constructor);
	ioport_manager_type.set("count_players", &ioport_manager::count_players);
	ioport_manager_type.set("natkeyboard", &ioport_manager::natkeyboard);
	ioport_manager_type.set("type_group", [](ioport_manager &im, ioport_type type, int player) {
			return im.type_group(type, player);
		});
	ioport_manager_type.set("ports", sol::property([this](ioport_manager &im) {
			sol::table port_table = sol().create_table();
			for (auto &port : im.ports())
				port_table[port.second->tag()] = port.second.get();
			return port_table;
		}));
	ioport_manager_type.set("type_seq", [](ioport_manager &m, ioport_type type, int player, input_seq_type seqtype) {
			return m.type_seq(type, player, seqtype);
		});


/* natural_keyboard library
 *
 * manager:machine():ioport():natkeyboard()
 *
 * natkeyboard:paste() - paste clipboard data
 * natkeyboard:post() - post data to natural keyboard
 * natkeyboard:post_coded() - post data to natural keyboard
 *
 * natkeyboard.empty - is the natural keyboard buffer empty?
 * natkeyboard.in_use - is the natural keyboard in use?
 */

	auto natkeyboard_type = sol().registry().new_usertype<natural_keyboard>("natkeyboard", "new", sol::no_constructor);
	natkeyboard_type.set("empty", sol::property(&natural_keyboard::empty));
	natkeyboard_type.set("in_use", sol::property(&natural_keyboard::in_use, &natural_keyboard::set_in_use));
	natkeyboard_type.set("paste", &natural_keyboard::paste);
	natkeyboard_type.set("post", [](natural_keyboard &nat, const std::string &text)          { nat.post_utf8(text); });
	natkeyboard_type.set("post_coded", [](natural_keyboard &nat, const std::string &text)    { nat.post_coded(text); });


/* ioport_port library
 *
 * manager:machine():ioport().ports[port_tag]
 *
 * port:tag() - get port tag
 * port:active() - get port status
 * port:live() - get port ioport_port_live (TODO: not usable from lua as of now)
 * port:read() - get port value
 * port:write(val, mask) - set port to value & mask (output fields only, for other fields use field:set_value(val))
 * port:field(mask) - get ioport_field for port and mask
 *
 * port.fields[] - get ioport_field table (k=name, v=ioport_field)
 */

	auto ioport_port_type = sol().registry().new_usertype<ioport_port>("ioport_port", "new", sol::no_constructor);
	ioport_port_type.set("tag", &ioport_port::tag);
	ioport_port_type.set("active", &ioport_port::active);
	ioport_port_type.set("live", &ioport_port::live);
	ioport_port_type.set("read", &ioport_port::read);
	ioport_port_type.set("write", &ioport_port::write);
	ioport_port_type.set("field", &ioport_port::field);
	ioport_port_type.set("fields", sol::property([this](ioport_port &p){
			sol::table f_table = sol().create_table();
			// parse twice for custom and default names, default has priority
			for(ioport_field &field : p.fields())
			{
				if (field.type_class() != INPUT_CLASS_INTERNAL)
					f_table[field.name()] = &field;
			}
			for(ioport_field &field : p.fields())
			{
				if (field.type_class() != INPUT_CLASS_INTERNAL)
				{
					if(field.specific_name())
						f_table[field.specific_name()] = &field;
					else
						f_table[field.manager().type_name(field.type(), field.player())] = &field;
				}
			}
			return f_table;
		}));


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

	auto ioport_field_type = sol().registry().new_usertype<ioport_field>("ioport_field", "new", sol::no_constructor);
	ioport_field_type.set("set_value", &ioport_field::set_value);
	ioport_field_type.set("set_input_seq", [](ioport_field &f, const std::string &seq_type_string, const input_seq &seq) {
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			ioport_field::user_settings settings;
			f.get_user_settings(settings);
			settings.seq[seq_type] = seq;
			f.set_user_settings(settings);
		});
	ioport_field_type.set("input_seq", [](ioport_field &f, const std::string &seq_type_string) {
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.seq(seq_type);
		});
	ioport_field_type.set("set_default_input_seq", [](ioport_field &f, const std::string &seq_type_string, const input_seq &seq) {
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			f.set_defseq(seq_type, seq);
		});
	ioport_field_type.set("default_input_seq", [](ioport_field &f, const std::string &seq_type_string) {
			input_seq_type seq_type = s_seq_type_parser(seq_type_string);
			return f.defseq(seq_type);
		});
	ioport_field_type.set("keyboard_codes", [this](ioport_field &f, int which) {
			sol::table result = sol().create_table();
			int index = 1;
			for (char32_t code : f.keyboard_codes(which))
				result[index++] = code;
			return result;
		});
	ioport_field_type.set("device", sol::property(&ioport_field::device));
	ioport_field_type.set("port", sol::property(&ioport_field::port));
	ioport_field_type.set("name", sol::property(&ioport_field::name));
	ioport_field_type.set("default_name", sol::property([](ioport_field &f) {
			return f.specific_name() ? f.specific_name() : f.manager().type_name(f.type(), f.player());
		}));
	ioport_field_type.set("player", sol::property(&ioport_field::player, &ioport_field::set_player));
	ioport_field_type.set("mask", sol::property(&ioport_field::mask));
	ioport_field_type.set("defvalue", sol::property(&ioport_field::defvalue));
	ioport_field_type.set("sensitivity", sol::property(&ioport_field::sensitivity));
	ioport_field_type.set("way", sol::property(&ioport_field::way));
	ioport_field_type.set("type_class", sol::property([](ioport_field &f) {
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
		}));
	ioport_field_type.set("is_analog", sol::property(&ioport_field::is_analog));
	ioport_field_type.set("is_digital_joystick", sol::property(&ioport_field::is_digital_joystick));
	ioport_field_type.set("enabled", sol::property(&ioport_field::enabled));
	ioport_field_type.set("optional", sol::property(&ioport_field::optional));
	ioport_field_type.set("cocktail", sol::property(&ioport_field::cocktail));
	ioport_field_type.set("toggle", sol::property(&ioport_field::toggle));
	ioport_field_type.set("rotated", sol::property(&ioport_field::rotated));
	ioport_field_type.set("analog_reverse", sol::property(&ioport_field::analog_reverse));
	ioport_field_type.set("analog_reset", sol::property(&ioport_field::analog_reset));
	ioport_field_type.set("analog_wraps", sol::property(&ioport_field::analog_wraps));
	ioport_field_type.set("analog_invert", sol::property(&ioport_field::analog_invert));
	ioport_field_type.set("impulse", sol::property(&ioport_field::impulse));
	ioport_field_type.set("type", sol::property(&ioport_field::type));
	ioport_field_type.set("live", sol::property(&ioport_field::live));
	ioport_field_type.set("crosshair_scale", sol::property(&ioport_field::crosshair_scale, &ioport_field::set_crosshair_scale));
	ioport_field_type.set("crosshair_offset", sol::property(&ioport_field::crosshair_offset, &ioport_field::set_crosshair_offset));
	ioport_field_type.set("user_value", sol::property(
		[](ioport_field &f) {
			ioport_field::user_settings settings;
			f.get_user_settings(settings);
			return settings.value;
		},
		[](ioport_field &f, ioport_value val) {
			ioport_field::user_settings settings;
			f.get_user_settings(settings);
			settings.value = val;
			f.set_user_settings(settings);
		}));
	ioport_field_type.set("settings", sol::property([this](ioport_field &f) {
			sol::table result = sol().create_table();
			for (ioport_setting &setting : f.settings())
				if (setting.enabled())
					result[setting.value()] = setting.name();
			return result;
		}));


/* ioport_field_live library
 *
 * manager:machine():ioport().ports[port_tag].fields[field_name].live
 *
 * live.name
 */

	sol().registry().new_usertype<ioport_field_live>("ioport_field_live", "new", sol::no_constructor,
			"name", &ioport_field_live::name);


/* input_manager library
 *
 * manager:machine():input()
 *
 * input:code_from_token(token) - get input_code for KEYCODE_* string token
 * input:code_pressed(code) - get pressed state for input_code
 * input:code_to_token(code) - get KEYCODE_* string token for code
 * input:code_name(code) - get code friendly name
 * input:seq_from_tokens(tokens) - get input_seq for multiple space separated KEYCODE_* string tokens
 * input:seq_pressed(seq) - get pressed state for input_seq
 * input:seq_to_tokens(seq) - get KEYCODE_* string tokens for seq
 * input:seq_name(seq) - get seq friendly name
 * input:seq_clean(seq) - clean the seq and remove invalid elements
 * input:seq_poll_start(class, [opt] start_seq) - start polling for input_item_class passed as string
 *                                                (switch/abs[olute]/rel[ative]/max[imum])
 * input:seq_poll() - poll once, returns true if input was fetched
 * input:seq_poll_final() - get final input_seq
 * input.device_classes - returns device classes
 */

	auto input_type = sol().registry().new_usertype<input_manager>("input", "new", sol::no_constructor);
	input_type.set("code_from_token", [](input_manager &input, const char *token) { return input.code_from_token(token); });
	input_type.set("code_pressed", [](input_manager &input, const input_code &code) { return input.code_pressed(code); });
	input_type.set("code_to_token", [](input_manager &input, const input_code &code) { return input.code_to_token(code); });
	input_type.set("code_name", [](input_manager &input, const input_code &code) { return input.code_name(code); });
	input_type.set("seq_from_tokens", [](input_manager &input, const char *tokens) { input_seq seq; input.seq_from_tokens(seq, tokens); return seq; });
	input_type.set("seq_pressed", [](input_manager &input, const input_seq &seq) { return input.seq_pressed(seq); });
	input_type.set("seq_to_tokens", [](input_manager &input, const input_seq &seq) { return input.seq_to_tokens(seq); });
	input_type.set("seq_name", [](input_manager &input, const input_seq &seq) { return input.seq_name(seq); });
	input_type.set("seq_clean", [](input_manager &input, const input_seq &seq) { return input.seq_clean(seq); });
	input_type.set("seq_poll_start", [this](input_manager &input, const char *cls_string, sol::object seq) {
			if (!m_seq_poll)
				m_seq_poll.reset(new input_sequence_poller(input));

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
				m_seq_poll->start(cls, seq.as<sol::user<input_seq>>());
			else
				m_seq_poll->start(cls);
		});
	input_type.set("seq_poll", [this](input_manager &input) -> sol::object {
			if (!m_seq_poll)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), m_seq_poll->poll());
		});
	input_type.set("seq_poll_final", [this](input_manager &input) -> sol::object {
			if (!m_seq_poll)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), m_seq_poll->valid() ? m_seq_poll->sequence() : input_seq());
		});
	input_type.set("seq_poll_modified", [this](input_manager &input) -> sol::object {
			if (!m_seq_poll)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), m_seq_poll->modified());
		});
	input_type.set("seq_poll_valid", [this](input_manager &input) -> sol::object {
			if (!m_seq_poll)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), m_seq_poll->valid());
		});
	input_type.set("seq_poll_sequence", [this](input_manager &input) -> sol::object {
			if (!m_seq_poll)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), m_seq_poll->sequence());
	});
	input_type.set("device_classes", sol::property([this](input_manager &input) {
			sol::table result = sol().create_table();
			for (input_device_class devclass_id = DEVICE_CLASS_FIRST_VALID; devclass_id <= DEVICE_CLASS_LAST_VALID; devclass_id++)
			{
				input_class &devclass = input.device_class(devclass_id);
				result[devclass.name()] = &devclass;
			}
			return result;
		}));


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
	input_class_type.set("name", sol::property(&input_class::name));
	input_class_type.set("enabled", sol::property(&input_class::enabled, &input_class::enable));
	input_class_type.set("multi", sol::property(&input_class::multi, &input_class::set_multi));
	input_class_type.set("devices", sol::property([this](input_class &devclass) {
			sol::table result = sol().create_table();
			int index = 1;
			for (int devindex = 0; devindex <= devclass.maxindex(); devindex++)
			{
				input_device *dev = devclass.device(devindex);
				if (dev)
					result[index++] = dev;
			}
			return result;
		}));


/* input_device library
 *
 * manager:machine():input().device_classes[devclass].devices[index]
 * device.name
 * device.id
 * device.devindex
 * device.items[]
 */

	auto input_device_type = sol().registry().new_usertype<input_device>("input_device", "new", sol::no_constructor);
	input_device_type.set("name", sol::property(&input_device::name));
	input_device_type.set("id", sol::property(&input_device::id));
	input_device_type.set("devindex", sol::property(&input_device::devindex));
	input_device_type.set("items", sol::property([this](input_device &dev) {
			sol::table result = sol().create_table();
			for (input_item_id id = ITEM_ID_FIRST_VALID; id < dev.maxitem(); id++)
			{
				input_device_item *item = dev.item(id);
				if (item)
					result[id] = dev.item(id);
			}
			return result;
		}));


/* input_device_item library
 *
 * manager:machine():input().device_classes[devclass].devices[index].items[item_id]
 * item.name
 * item.token
 * item:code()
 */

	auto input_device_item_type = sol().registry().new_usertype<input_device_item>("input_device_item", "new", sol::no_constructor);
	input_device_item_type.set("name", sol::property(&input_device_item::name));
	input_device_item_type.set("token", sol::property(&input_device_item::token));
	input_device_item_type.set("code", [](input_device_item &item) {
			return input_code(item.device().devclass(), item.device().devindex(), item.itemclass(), ITEM_MODIFIER_NONE, item.itemid());
		});


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
