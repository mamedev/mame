// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cheat.c

    MAME cheat system.

****************************************************************************

    Cheat XML format:

    <mamecheat version="1">
        <cheat desc="blah">
           <parameter min="minval(0)" max="maxval(numitems)" step="stepval(1)">
              <item value="itemval(previtemval|minval+stepval)">text</item>
              ...
           </parameter>
           <script state="on|off|run|change(run)">
              <action condition="condexpr(1)">expression</action>
              ...
              <output condition="condexpr(1)" format="format(required)" line="line(0)" align="left|center|right(left)">
                 <argument count="count(1)">expression</argument>
              </output>
              ...
           </script>
           ...
           <comment>
              ... text ...
           </comment>
        </cheat>
        ...
    </mamecheat>

****************************************************************************

    Expressions are standard debugger expressions. Note that & and
    < must be escaped per XML rules. Within attributes you must use
    &amp; and &lt;. For tags, you can also use <![CDATA[ ... ]]>.

    Each cheat has its own context-specific variables:

        temp0-temp9 -- 10 temporary variables for any use
        param       -- the current value of the cheat parameter
        frame       -- the current frame index
        argindex    -- for arguments with multiple iterations, this is the index

    By default, each cheat has 10 temporary variables that are
    persistent while executing its scripts. Additional temporary
    variables may be requested via the 'tempvariables' attribute
    on the cheat.

****************************************************************************

    Cheats are generally broken down into categories based on
    which actions are defined and whether or not there is a
    parameter present:

    ---- Actions -----
    On   Off  Run  Chg  Param?  Type
    ===  ===  ===  ===  ======  =================================
     N    N    N    ?    None   Text-only (displays text in menu)
     Y    N    N    ?    None   Oneshot (select to activate)
     Y    Y    N    ?    None   On/Off (select to toggle)
     ?    ?    Y    ?    None   On/Off (select to toggle)

     ?    N    N    Y    Any    Oneshot parameter (select to alter)
     ?    Y    ?    ?    Value  Value parameter (off or a live value)
     ?    ?    Y    ?    Value  Value parameter (off or a live value)
     ?    Y    ?    ?    List   Item list parameter (off or a live value)
     ?    ?    Y    ?    List   Item list parameter (off or a live value)

***************************************************************************/

#include "emu.h"
#include "cheat.h"

#include "mame.h"
#include "ui/ui.h"
#include "ui/menu.h"

#include "corestr.h"
#include "emuopts.h"
#include "fileio.h"

#include <cstring>
#include <iterator>
#include <utility>

#include <cctype>



//**************************************************************************
//  PARAMETERS
//**************************************************************************

// turn this on to enable removing duplicate cheats; not sure if we should
#define REMOVE_DUPLICATE_CHEATS 0



//**************************************************************************
//  NUMBER AND FORMAT
//**************************************************************************

//-------------------------------------------------
//  format - format an integer according to
//  the format
//-------------------------------------------------

inline std::string number_and_format::format() const
{
	switch (m_format)
	{
	default:
	case util::xml::data_node::int_format::DECIMAL:
		return string_format("%d", uint32_t(m_value));

	case util::xml::data_node::int_format::DECIMAL_HASH:
		return string_format("#%d", uint32_t(m_value));

	case util::xml::data_node::int_format::HEX_DOLLAR:
		return string_format("$%X", uint32_t(m_value));

	case util::xml::data_node::int_format::HEX_C:
		return string_format("0x%X", uint32_t(m_value));
	}
}



//**************************************************************************
//  CHEAT PARAMETER
//**************************************************************************

//-------------------------------------------------
//  cheat_parameter - constructor
//-------------------------------------------------

cheat_parameter::cheat_parameter(cheat_manager &manager, symbol_table &symbols, std::string const &filename, util::xml::data_node const &paramnode)
	: m_minval(number_and_format(paramnode.get_attribute_int("min", 0), paramnode.get_attribute_int_format("min")))
	, m_maxval(number_and_format(paramnode.get_attribute_int("max", 0), paramnode.get_attribute_int_format("max")))
	, m_stepval(number_and_format(paramnode.get_attribute_int("step", 1), paramnode.get_attribute_int_format("step")))
	, m_value(0)
	, m_curtext()
	, m_itemlist()
{
	// iterate over items
	for (util::xml::data_node const *itemnode = paramnode.get_child("item"); itemnode != nullptr; itemnode = itemnode->get_next_sibling("item"))
	{
		// check for nullptr text
		if (!itemnode->get_value() || !itemnode->get_value()[0])
			throw emu_fatalerror("%s.xml(%d): item is missing text\n", filename, itemnode->line);

		// check for non-existent value
		if (!itemnode->has_attribute("value"))
			throw emu_fatalerror("%s.xml(%d): item is value\n", filename, itemnode->line);

		// extract the parameters
		uint64_t const value(itemnode->get_attribute_int("value", 0));
		util::xml::data_node::int_format const format(itemnode->get_attribute_int_format("value"));

		// allocate and append a new item
		item &curitem(m_itemlist.emplace_back(itemnode->get_value(), value, format));

		// ensure the maximum expands to suit
		m_maxval = std::max(m_maxval, curitem.value());
	}

	// add a variable to the symbol table for our value
	symbols.add("param", symbol_table::READ_ONLY, &m_value);
}


//-------------------------------------------------
//  text - return the current text for a parameter
//-------------------------------------------------

const char *cheat_parameter::text()
{
	// are we a value cheat?
	if (!has_itemlist())
	{
		m_curtext = string_format("%u (0x%X)", uint64_t(m_value), uint64_t(m_value));
	}
	else
	{
		// if not, we're an item cheat
		m_curtext = string_format("??? (%u)", uint64_t(m_value));
		for (item const &curitem : m_itemlist)
		{
			if (curitem.value() == m_value)
			{
				m_curtext = curitem.text();
				break;
			}
		}
	}
	return m_curtext.c_str();
}


//-------------------------------------------------
//  save - save a single cheat parameter
//-------------------------------------------------

void cheat_parameter::save(util::core_file &cheatfile) const
{
	// output the parameter tag
	cheatfile.printf("\t\t<parameter");

	if (!has_itemlist())
	{
		// if no items, just output min/max/step
		if (m_minval != 0)
			cheatfile.printf(" min=\"%s\"", m_minval.format());
		if (m_maxval != 0)
			cheatfile.printf(" max=\"%s\"", m_maxval.format());
		if (m_stepval != 1)
			cheatfile.printf(" step=\"%s\"", m_stepval.format());
		cheatfile.printf("/>\n");
	}
	else
	{
		// iterate over items
		cheatfile.printf(">\n");
		for (item const &curitem : m_itemlist)
			cheatfile.printf("\t\t\t<item value=\"%s\">%s</item>\n", curitem.value().format(), curitem.text());
		cheatfile.printf("\t\t</parameter>\n");
	}
}


//-------------------------------------------------
//  set_minimum_state - set the minimum state
//-------------------------------------------------

bool cheat_parameter::set_minimum_state()
{
	uint64_t const origvalue(m_value);

	// set based on whether we have an item list
	m_value = !has_itemlist() ? m_minval : m_itemlist.front().value();

	return m_value != origvalue;
}


//-------------------------------------------------
//  set_minimum_state - set the previous state
//-------------------------------------------------

bool cheat_parameter::set_prev_state()
{
	uint64_t const origvalue(m_value);

	if (!has_itemlist())
	{
		// are we a value cheat?
		if (m_value < (m_minval + m_stepval))
			m_value = m_minval;
		else
			m_value -= m_stepval;
	}
	else
	{
		// if not, we're an item cheat
		std::vector<item>::const_iterator it;
		for (it = m_itemlist.begin(); (m_itemlist.end() != it) && (it->value() != m_value); ++it) { }
		if (m_itemlist.begin() != it)
			m_value = std::prev(it)->value();
	}

	return m_value != origvalue;
}


//-------------------------------------------------
//  set_next_state - advance to the next state
//-------------------------------------------------

bool cheat_parameter::set_next_state()
{
	uint64_t const origvalue(m_value);

	if (!has_itemlist())
	{
		// are we a value cheat?
		if (m_value > m_maxval - m_stepval)
			m_value = m_maxval;
		else
			m_value += m_stepval;
	}
	else
	{
		// if not, we're an item cheat
		std::vector<item>::const_iterator it;
		for (it = m_itemlist.begin(); (m_itemlist.end() != it) && (it->value() != m_value); ++it) { }
		if ((m_itemlist.end() != it) && (m_itemlist.end() != ++it))
			m_value = it->value();
	}

	return m_value != origvalue;
}



//**************************************************************************
//  CHEAT SCRIPT
//**************************************************************************

constexpr int cheat_script::script_entry::MAX_ARGUMENTS;


//-------------------------------------------------
//  cheat_script - constructor
//-------------------------------------------------

cheat_script::cheat_script(
		cheat_manager &manager,
		symbol_table &symbols,
		std::string const &filename,
		util::xml::data_node const &scriptnode)
	: m_state(SCRIPT_STATE_RUN)
{
	// read the core attributes
	char const *const state(scriptnode.get_attribute_string("state", "run"));
	if (!std::strcmp(state, "on"))
		m_state = SCRIPT_STATE_ON;
	else if (!std::strcmp(state, "off"))
		m_state = SCRIPT_STATE_OFF;
	else if (!std::strcmp(state, "change"))
		m_state = SCRIPT_STATE_CHANGE;
	else if (std::strcmp(state, "run"))
		throw emu_fatalerror("%s.xml(%d): invalid script state '%s'\n", filename, scriptnode.line, state);

	// iterate over nodes within the script
	for (util::xml::data_node const *entrynode = scriptnode.get_first_child(); entrynode != nullptr; entrynode = entrynode->get_next_sibling())
	{
		if (!std::strcmp(entrynode->get_name(), "action")) // handle action nodes
			m_entrylist.push_back(std::make_unique<script_entry>(manager, symbols, filename, *entrynode, true));
		else if (!std::strcmp(entrynode->get_name(), "output")) // handle output nodes
			m_entrylist.push_back(std::make_unique<script_entry>(manager, symbols, filename, *entrynode, false));
		else // anything else is ignored
			osd_printf_warning("%s.xml(%d): unknown script item '%s' will be lost if saved\n", filename, entrynode->line, entrynode->get_name());
	}
}


//-------------------------------------------------
//  execute - execute ourself
//-------------------------------------------------

void cheat_script::execute(cheat_manager &manager, uint64_t &argindex)
{
	// do nothing if disabled
	if (!manager.enabled())
		return;

	// iterate over entries
	for (auto &entry : m_entrylist)
		entry->execute(manager, argindex);
}


//-------------------------------------------------
//  save - save a single cheat script
//-------------------------------------------------

void cheat_script::save(util::core_file &cheatfile) const
{
	// output the script tag
	cheatfile.printf("\t\t<script");
	switch (m_state)
	{
	case SCRIPT_STATE_OFF:      cheatfile.printf(" state=\"off\"");     break;
	case SCRIPT_STATE_ON:       cheatfile.printf(" state=\"on\"");      break;
	default:
	case SCRIPT_STATE_RUN:      cheatfile.printf(" state=\"run\"");     break;
	case SCRIPT_STATE_CHANGE:   cheatfile.printf(" state=\"change\"");  break;
	}
	cheatfile.printf(">\n");

	// output entries
	for (auto &entry : m_entrylist)
		entry->save(cheatfile);

	// close the tag
	cheatfile.printf("\t\t</script>\n");
}


//-------------------------------------------------
//  script_entry - constructor
//-------------------------------------------------

cheat_script::script_entry::script_entry(
		cheat_manager &manager,
		symbol_table &symbols,
		std::string const &filename,
		util::xml::data_node const &entrynode,
		bool isaction)
	: m_condition(symbols)
	, m_expression(symbols)
{
	char const *expression(nullptr);
	try
	{
		// read the condition if present
		expression = entrynode.get_attribute_string("condition", nullptr);
		if (expression)
			m_condition.parse(expression);

		if (isaction)
		{
			// if this is an action, parse the expression
			expression = entrynode.get_value();
			if (!expression || !expression[0])
				throw emu_fatalerror("%s.xml(%d): missing expression in action tag\n", filename, entrynode.line);
			m_expression.parse(expression);

			// initialise these to defautlt values
			m_line = 0;
			m_justify = ui::text_layout::text_justify::LEFT;
		}
		else
		{
			// otherwise, parse the attributes and arguments

			// extract format
			char const *const format(entrynode.get_attribute_string("format", nullptr));
			if (!format || !format[0])
				throw emu_fatalerror("%s.xml(%d): missing format in output tag\n", filename, entrynode.line);
			m_format = format;

			// extract other attributes
			m_line = entrynode.get_attribute_int("line", 0);
			m_justify = ui::text_layout::text_justify::LEFT;
			char const *const align(entrynode.get_attribute_string("align", "left"));
			if (!std::strcmp(align, "center"))
				m_justify = ui::text_layout::text_justify::CENTER;
			else if (!std::strcmp(align, "right"))
				m_justify = ui::text_layout::text_justify::RIGHT;
			else if (std::strcmp(align, "left"))
				throw emu_fatalerror("%s.xml(%d): invalid alignment '%s' specified\n", filename, entrynode.line, align);

			// then parse arguments
			int totalargs(0);
			for (util::xml::data_node const *argnode = entrynode.get_child("argument"); argnode != nullptr; argnode = argnode->get_next_sibling("argument"))
			{
				auto curarg = std::make_unique<output_argument>(manager, symbols, filename, *argnode);
				// verify we didn't overrun the argument count
				totalargs += curarg->count();

				m_arglist.push_back(std::move(curarg));

				if (totalargs > MAX_ARGUMENTS)
					throw emu_fatalerror("%s.xml(%d): too many arguments (found %d, max is %d)\n", filename, argnode->line, totalargs, MAX_ARGUMENTS);
			}

			// validate the format against the arguments
			validate_format(filename, entrynode.line);
		}
	}
	catch (expression_error const &err)
	{
		throw emu_fatalerror("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, entrynode.line, expression, err.code_string());
	}
}


//-------------------------------------------------
//  execute - execute a single script entry
//-------------------------------------------------

void cheat_script::script_entry::execute(cheat_manager &manager, uint64_t &argindex)
{
	// evaluate the condition
	if (!m_condition.is_empty())
	{
		try
		{
			uint64_t const result(m_condition.execute());
			if (!result)
				return;
		}
		catch (expression_error const &err)
		{
			osd_printf_warning("Error executing conditional expression \"%s\": %s\n", m_condition.original_string(), err.code_string());
			return;
		}
	}

	// if there is an action, execute it
	if (!m_expression.is_empty())
	{
		try
		{
			m_expression.execute();
		}
		catch (expression_error &err)
		{
			osd_printf_warning("Error executing expression \"%s\": %s\n", m_expression.original_string(), err.code_string());
		}
	}

	// if there is a string to display, compute it
	if (!m_format.empty())
	{
		// iterate over arguments and evaluate them
		uint64_t params[MAX_ARGUMENTS];
		int curarg = 0;
		for (auto &arg : m_arglist)
			curarg += arg->values(argindex, &params[curarg]);

		// generate the astring
		manager.get_output_string(m_line, m_justify) = string_format(m_format,
				params[0],  params[1],  params[2],  params[3],
				params[4],  params[5],  params[6],  params[7],
				params[8],  params[9],  params[10], params[11],
				params[12], params[13], params[14], params[15],
				params[16], params[17], params[18], params[19],
				params[20], params[21], params[22], params[23],
				params[24], params[25], params[26], params[27],
				params[28], params[29], params[30], params[31]);
	}
}


//-------------------------------------------------
//  save - save a single action or output
//-------------------------------------------------

void cheat_script::script_entry::save(util::core_file &cheatfile) const
{
	if (m_format.empty())
	{
		// output an action
		cheatfile.printf("\t\t\t<action");
		if (!m_condition.is_empty())
			cheatfile.printf(" condition=\"%s\"", cheat_manager::quote_expression(m_condition));
		cheatfile.printf(">%s</action>\n", cheat_manager::quote_expression(m_expression));
	}
	else
	{
		// output an output
		cheatfile.printf("\t\t\t<output format=\"%s\"", m_format);
		if (!m_condition.is_empty())
			cheatfile.printf(" condition=\"%s\"", cheat_manager::quote_expression(m_condition));

		if (m_line != 0)
			cheatfile.printf(" line=\"%d\"", m_line);

		if (m_justify == ui::text_layout::text_justify::CENTER)
			cheatfile.printf(" align=\"center\"");
		else if (m_justify == ui::text_layout::text_justify::RIGHT)
			cheatfile.printf(" align=\"right\"");

		if (m_arglist.size() == 0)
		{
			cheatfile.printf(" />\n");
		}
		else
		{
			// output arguments
			cheatfile.printf(">\n");
			for (auto &curarg : m_arglist)
				curarg->save(cheatfile);
			cheatfile.printf("\t\t\t</output>\n");
		}
	}
}


//-------------------------------------------------
//  validate_format - check that a format string
//  has the correct number and type of arguments
//-------------------------------------------------

void cheat_script::script_entry::validate_format(std::string const &filename, int line)
{
	// first count arguments
	int argsprovided(0);
	for (auto &curarg : m_arglist)
		argsprovided += curarg->count();

	// now scan the string for valid argument usage
	int argscounted = 0;
	for (char const *p = strchr(m_format.c_str(), '%'); p; ++argscounted, p = strchr(p, '%'))
	{
		// skip past any valid attributes
		for (++p; strchr("lh0123456789.-+ #", *p); ++p) { }

		// look for a valid type
		if (!strchr("cdiouxX", *p))
			throw emu_fatalerror("%s.xml(%d): invalid format specification \"%s\"\n", filename, line, m_format);
	}

	// did we match?
	if (argscounted < argsprovided)
		throw emu_fatalerror("%s.xml(%d): too many arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, m_format);
	if (argscounted > argsprovided)
		throw emu_fatalerror("%s.xml(%d): not enough arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, m_format);
}


//-------------------------------------------------
//  output_argument - constructor
//-------------------------------------------------

cheat_script::script_entry::output_argument::output_argument(
		cheat_manager &manager,
		symbol_table &symbols,
		std::string const &filename,
		util::xml::data_node const &argnode)
	: m_expression(symbols)
	, m_count(0)
{
	// first extract attributes
	m_count = argnode.get_attribute_int("count", 1);

	// read the expression
	char const *const expression(argnode.get_value());
	if (!expression || !expression[0])
		throw emu_fatalerror("%s.xml(%d): missing expression in argument tag\n", filename, argnode.line);

	// parse it
	try
	{
		m_expression.parse(expression);
	}
	catch (expression_error const &err)
	{
		throw emu_fatalerror("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, argnode.line, expression, err.code_string());
	}
}


//-------------------------------------------------
//  value - return the evaluated value of the
//  given output argument
//-------------------------------------------------

int cheat_script::script_entry::output_argument::values(uint64_t &argindex, uint64_t *result)
{
	for (argindex = 0; argindex < m_count; argindex++)
	{
		try
		{
			result[argindex] = m_expression.execute();
		}
		catch (expression_error const &err)
		{
			osd_printf_warning("Error executing argument expression \"%s\": %s\n", m_expression.original_string(), err.code_string());
		}
	}
	return m_count;
}


//-------------------------------------------------
//  save - save a single output argument
//-------------------------------------------------

void cheat_script::script_entry::output_argument::save(util::core_file &cheatfile) const
{
	cheatfile.printf("\t\t\t\t<argument");
	if (m_count != 1)
		cheatfile.printf(" count=\"%d\"", int(m_count));
	cheatfile.printf(">%s</argument>\n", cheat_manager::quote_expression(m_expression));
}



//**************************************************************************
//  CHEAT ENTRY
//**************************************************************************

//-------------------------------------------------
//  cheat_entry - constructor
//-------------------------------------------------

cheat_entry::cheat_entry(cheat_manager &manager, symbol_table &globaltable, std::string const &filename, util::xml::data_node const &cheatnode)
	: m_manager(manager)
	, m_symbols(manager.machine(), symbol_table::CHEAT_ENTRY, &globaltable)
	, m_state(SCRIPT_STATE_OFF)
	, m_numtemp(DEFAULT_TEMP_VARIABLES)
	, m_argindex(0)
{
	// pull the variable count out ahead of things
	int const tempcount(cheatnode.get_attribute_int("tempvariables", DEFAULT_TEMP_VARIABLES));
	if (tempcount < 1)
		throw emu_fatalerror("%s.xml(%d): invalid tempvariables attribute (%d)\n", filename, cheatnode.line, tempcount);

	// allocate memory for the cheat
	m_numtemp = tempcount;

	// get the description
	char const *const description(cheatnode.get_attribute_string("desc", nullptr));
	if (!description || !description[0])
		throw emu_fatalerror("%s.xml(%d): empty or missing desc attribute on cheat\n", filename, cheatnode.line);
	m_description = description;

	// create the symbol table
	m_symbols.add("argindex", symbol_table::READ_ONLY, &m_argindex);
	for (int curtemp = 0; curtemp < tempcount; curtemp++)
		m_symbols.add(string_format("temp%d", curtemp).c_str(), symbol_table::READ_WRITE);

	// read the first comment node
	util::xml::data_node const *const commentnode(cheatnode.get_child("comment"));
	if (commentnode != nullptr)
	{
		// set the value if not nullptr
		if (commentnode->get_value() && commentnode->get_value()[0])
			m_comment.assign(commentnode->get_value());

		// only one comment is kept
		util::xml::data_node const *const nextcomment(commentnode->get_next_sibling("comment"));
		if (nextcomment)
			osd_printf_warning("%s.xml(%d): only one comment node is retained; ignoring additional nodes\n", filename, nextcomment->line);
	}

	// read the first parameter node
	util::xml::data_node const *const paramnode(cheatnode.get_child("parameter"));
	if (paramnode != nullptr)
	{
		// load this parameter
		m_parameter.reset(new cheat_parameter(manager, m_symbols, filename, *paramnode));

		// only one parameter allowed
		util::xml::data_node const *const nextparam(paramnode->get_next_sibling("parameter"));
		if (nextparam)
			osd_printf_warning("%s.xml(%d): only one parameter node allowed; ignoring additional nodes\n", filename, nextparam->line);
	}

	// read the script nodes
	for (util::xml::data_node const *scriptnode = cheatnode.get_child("script"); scriptnode != nullptr; scriptnode = scriptnode->get_next_sibling("script"))
	{
		// load this entry
		std::unique_ptr<cheat_script> curscript(new cheat_script(manager, m_symbols, filename, *scriptnode));

		// if we have a script already for this slot, it is an error
		std::unique_ptr<cheat_script> &slot = script_for_state(curscript->state());
		if (slot)
			osd_printf_warning("%s.xml(%d): only one on script allowed; ignoring additional scripts\n", filename, scriptnode->line);
		else
			slot = std::move(curscript);
	}
}


//-------------------------------------------------
//  ~cheat_entry - destructor
//-------------------------------------------------

cheat_entry::~cheat_entry()
{
}


//-------------------------------------------------
//  save - save a single cheat entry
//-------------------------------------------------

void cheat_entry::save(util::core_file &cheatfile) const
{
	// determine if we have scripts
	bool const has_scripts(m_off_script || m_on_script || m_run_script || m_change_script);

	// output the cheat tag
	cheatfile.printf("\t<cheat desc=\"%s\"", m_description);
	if (m_numtemp != DEFAULT_TEMP_VARIABLES)
		cheatfile.printf(" tempvariables=\"%d\"", m_numtemp);

	if (m_comment.empty() && !m_parameter && !has_scripts)
	{
		cheatfile.printf(" />\n");
	}
	else
	{
		cheatfile.printf(">\n");

		// save the comment
		if (!m_comment.empty())
			cheatfile.printf("\t\t<comment><![CDATA[\n%s\n\t\t]]></comment>\n", m_comment);

		// output the parameter, if present
		if (m_parameter) m_parameter->save(cheatfile);

		// output the script nodes
		if (m_on_script) m_on_script->save(cheatfile);
		if (m_off_script) m_off_script->save(cheatfile);
		if (m_change_script) m_change_script->save(cheatfile);
		if (m_run_script) m_run_script->save(cheatfile);

		// close the cheat tag
		cheatfile.printf("\t</cheat>\n");
	}
}


//-------------------------------------------------
//  activate - activate a oneshot cheat
//-------------------------------------------------

bool cheat_entry::activate()
{
	bool changed(false);

	// if cheats have been toggled off no point in even trying to do anything
	if (!m_manager.enabled())
		return changed;

	if (is_oneshot())
	{
		// if we're a oneshot cheat, execute the "on" script and indicate change
		execute_on_script();
		changed = true;
		m_manager.machine().popmessage("Activated %s", m_description);
	}
	else if (is_oneshot_parameter() && (m_state != SCRIPT_STATE_OFF))
	{
		// if we're a oneshot parameter cheat and we're active, execute the "state change" script and indicate change
		execute_change_script();
		changed = true;
		m_manager.machine().popmessage("Activated\n %s = %s", m_description, m_parameter->text());
	}

	return changed;
}


//-------------------------------------------------
//  select_default_state - select the default
//  state for a cheat, or activate a oneshot cheat
//-------------------------------------------------

bool cheat_entry::select_default_state()
{
	bool changed(false);

	if (is_oneshot())
	{
		// if we're a oneshot cheat, there is no default state
	}
	else
	{
		// all other types switch to the "off" state
		changed = set_state(SCRIPT_STATE_OFF);
	}

	return changed;
}


//-------------------------------------------------
//  select_previous_state - select the previous
//  state for a cheat
//-------------------------------------------------

bool cheat_entry::select_previous_state()
{
	bool changed(false);

	if (is_oneshot())
	{
		// if we're a oneshot, there is no previous state
	}
	else if (is_onoff())
	{
		// if we're on/off, toggle to off
		changed = set_state(SCRIPT_STATE_OFF);
	}
	else if (m_parameter != nullptr)
	{
		// if we have a parameter, set the previous state

		// if we're at our minimum, turn off
		if (m_parameter->is_minimum())
		{
			changed = set_state(SCRIPT_STATE_OFF);
		}
		else
		{
			// if we changed, ensure we are in the running state and signal state change
			changed = m_parameter->set_prev_state();
			if (changed)
			{
				set_state(SCRIPT_STATE_RUN);
				if (!is_oneshot_parameter())
					execute_change_script();
			}
		}
	}

	return changed;
}


//-------------------------------------------------
//  select_next_state - select the next state for
//  a cheat
//-------------------------------------------------

bool cheat_entry::select_next_state()
{
	bool changed(false);

	if (is_oneshot())
	{
		// if we're a oneshot, there is no next state
	}
	else if (is_onoff())
	{
		// if we're on/off, toggle to running state
		changed = set_state(SCRIPT_STATE_RUN);
	}
	else if (m_parameter != nullptr)
	{
		// if we have a parameter, set the next state
		if (m_state == SCRIPT_STATE_OFF)
		{
			// if we're off, switch on to the minimum state
			changed = set_state(SCRIPT_STATE_RUN);
			m_parameter->set_minimum_state();
		}
		else
		{
			// otherwise, switch to the next state
			changed = m_parameter->set_next_state();
		}

		// if we changed, signal a state change
		if (changed && !is_oneshot_parameter())
			execute_change_script();
	}

	return changed;
}


//-------------------------------------------------
//  menu_text - return the text needed to display
//  this cheat in a menu item
//-------------------------------------------------

void cheat_entry::menu_text(std::string &description, std::string &state, uint32_t &flags)
{
	// description is standard
	description = m_description;
	state.clear();
	flags = 0;

	if (is_text_only())
	{
		// some cheat entries are just text for display
		if (!description.empty())
		{
			description = strtrimspace(description);
			if (description.empty())
				description = MENU_SEPARATOR_ITEM;
		}
		flags = ui::menu::FLAG_DISABLE;
	}
	else if (is_oneshot())
	{
		// if we have no parameter and no run or off script, it's a oneshot cheat
		state = "Set";
	}
	else if (is_onoff())
	{
		// if we have no parameter, it's just on/off
		state = (m_state == SCRIPT_STATE_RUN) ? "On" : "Off";
		flags = (m_state != 0) ? ui::menu::FLAG_LEFT_ARROW : ui::menu::FLAG_RIGHT_ARROW;
	}
	else if (m_parameter != nullptr)
	{
		// if we have a value parameter, compute it
		if (m_state == SCRIPT_STATE_OFF)
		{
			state = is_oneshot_parameter() ? "Set" : "Off";
			flags = ui::menu::FLAG_RIGHT_ARROW;
		}
		else
		{
			state = m_parameter->text();
			flags = ui::menu::FLAG_LEFT_ARROW;
			if (!m_parameter->is_maximum())
				flags |= ui::menu::FLAG_RIGHT_ARROW;
		}
	}
}


//-------------------------------------------------
//  set_state - switch to the given state
//-------------------------------------------------

bool cheat_entry::set_state(script_state newstate)
{
	// if we're already in the state, indicate no change
	if (m_state == newstate)
		return false;

	// change to the state and run the appropriate script
	m_state = newstate;
	if (newstate == SCRIPT_STATE_OFF)
		execute_off_script();
	else if ((newstate == SCRIPT_STATE_ON) || (newstate == SCRIPT_STATE_RUN))
		execute_on_script();

	return true;
}


//-------------------------------------------------
//  script_for_state - get a reference to the
//  given script pointer
//-------------------------------------------------

std::unique_ptr<cheat_script> &cheat_entry::script_for_state(script_state state)
{
	switch (state)
	{
	case SCRIPT_STATE_ON:       return m_on_script;
	case SCRIPT_STATE_OFF:      return m_off_script;
	case SCRIPT_STATE_CHANGE:   return m_change_script;
	default:
	case SCRIPT_STATE_RUN:      return m_run_script;
	}
}


//-------------------------------------------------
//  is_duplicate - determine if a new cheat entry
//  is already included in the list
//-------------------------------------------------

bool cheat_entry::is_duplicate() const
{
	for (auto &scannode : manager().entries())
	{
		if (!std::strcmp(scannode->description(), description()))
			return true;
	}
	return false;
}


//**************************************************************************
//  CHEAT MANAGER
//**************************************************************************

constexpr int cheat_manager::CHEAT_VERSION;


//-------------------------------------------------
//  cheat_manager - constructor
//-------------------------------------------------

cheat_manager::cheat_manager(running_machine &machine)
	: m_machine(machine)
	, m_framecount(0)
	, m_numlines(0)
	, m_lastline(0)
	, m_disabled(true)
	, m_symtable(machine, symbol_table::CHEAT_MANAGER)
{
	// if the cheat engine is disabled, we're done
	if (!machine.options().cheat())
		return;

	// in its current form, cheat_manager is tightly coupled to mame_ui_manager; therefore we
	// expect this call to succeed
	mame_ui_manager *ui = dynamic_cast<mame_ui_manager *>(&machine.ui());
	assert(ui);

	int target_font_rows = ui->options().font_rows();
	m_output.resize(target_font_rows * 2);
	m_justify.resize(target_font_rows * 2);

	// request a callback
	machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&cheat_manager::frame_update, this));

	// create a global symbol table
	m_symtable.add("frame", symbol_table::READ_ONLY, &m_framecount);
	m_symtable.add("frombcd", 1, 1, execute_frombcd);
	m_symtable.add("tobcd", 1, 1, execute_tobcd);

	// load the cheats
	reload();
}


//-------------------------------------------------
//  set_enable - globally enable or disable the
//  cheat engine
//-------------------------------------------------

void cheat_manager::set_enable(bool enable, bool show)
{
	// if the cheat engine is disabled, we're done
	if (!machine().options().cheat())
		return;

	if (!m_disabled && !enable)
	{
		// if we're enabled currently and we don't want to be, turn things off

		// iterate over running cheats and execute any OFF Scripts
		for (auto &cheat : m_cheatlist)
		{
			if (cheat->state() == SCRIPT_STATE_RUN)
				cheat->execute_off_script();
		}
		if (show)
			machine().popmessage("Cheats Disabled");
		m_disabled = true;
	}
	else if (m_disabled && enable)
	{
		// if we're disabled currently and we want to be enabled, turn things on

		// iterate over running cheats and execute any ON Scripts
		m_disabled = false;
		for (auto &cheat : m_cheatlist)
		{
			if (cheat->state() == SCRIPT_STATE_RUN)
				cheat->execute_on_script();
		}
		if (show)
			machine().popmessage("Cheats Enabled");
	}
}


//-------------------------------------------------
//  reload - re-initialize the cheat engine, and
//  and reload the cheat file(s)
//-------------------------------------------------

void cheat_manager::reload()
{
	// if the cheat engine is disabled, we're done
	if (!machine().options().cheat())
		return;

	// free everything
	m_cheatlist.clear();

	// reset state
	m_framecount = 0;
	m_numlines = 0;
	m_lastline = 0;
	m_disabled = false;

	// load the cheat file, if it's a system that has a software list then try softlist_name/shortname.xml first,
	// if it fails to load then try to load via crc32 - basename/crc32.xml ( eg. 01234567.xml )
	for (device_image_interface &image : image_interface_enumerator(machine().root_device()))
	{
		if (image.exists())
		{
			// if we are loading through a software list, try to load softlist_name/shortname.xml
			// this allows the coexistence of arcade cheats with cheats for home conversions which
			// have the same shortname
			if (image.loaded_through_softlist())
			{
				load_cheats(string_format("%s" PATH_SEPARATOR "%s", image.software_list_name(), image.basename()));
				break;
			}
			// else we are loading outside the software list, try to load machine_basename/crc32.xml
			else
			{
				uint32_t crc = image.crc();
				if (crc != 0)
				{
					load_cheats(string_format("%s" PATH_SEPARATOR "%08X", machine().basename(), crc));
					break;
				}
			}
		}
	}

	// if we haven't found the cheats yet, load by basename
	if (m_cheatlist.empty())
		load_cheats(machine().basename());

	// temporary: save the file back out as output.xml for comparison
	if (m_cheatlist.size() != 0)
		save_all("output");
}


//-------------------------------------------------
//  cheat_list_save - save a cheat file from
//  memory to the given filename
//-------------------------------------------------

bool cheat_manager::save_all(std::string const &filename)
{
	// open the file with the proper name
	emu_file cheatfile(machine().options().cheat_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr(cheatfile.open(filename + ".xml"));

	// if that failed, return nothing
	if (filerr)
		return false;

	// wrap the rest of catch errors
	try
	{
		// output the outer layers
		cheatfile.printf("<?xml version=\"1.0\"?>\n");
		cheatfile.printf("<!-- This file is autogenerated; comments and unknown tags will be stripped -->\n");
		cheatfile.printf("<mamecheat version=\"%d\">\n", CHEAT_VERSION);

		// iterate over cheats in the list and save them
		for (auto &cheat : m_cheatlist)
			cheat->save(cheatfile);

		// close out the file
		cheatfile.printf("</mamecheat>\n");
		return true;
	}
	catch (emu_fatalerror const &err)
	{
		// catch errors and cleanup
		osd_printf_error("%s\n", err.what());
		cheatfile.remove_on_close();
	}
	return false;
}


//-------------------------------------------------
//  render_text - called by the UI system to
//  render text
//-------------------------------------------------

void cheat_manager::render_text(mame_ui_manager &mui, render_container &container)
{
	// render any text and free it along the way
	for (int linenum = 0; linenum < m_output.size(); linenum++)
	{
		if (!m_output[linenum].empty())
		{
			// output the text
			mui.draw_text_full(
					container,
					m_output[linenum],
					0.0f, float(linenum) * mui.get_line_height(), 1.0f,
					m_justify[linenum], ui::text_layout::word_wrapping::NEVER,
					mame_ui_manager::OPAQUE_, rgb_t::white(), rgb_t::black(),
					nullptr, nullptr);
		}
	}
}


//-------------------------------------------------
//  get_output_string - return a reference to
//  the given row's string, and set the
//  justification
//-------------------------------------------------

std::string &cheat_manager::get_output_string(int row, ui::text_layout::text_justify justify)
{
	// if the row is not specified, grab the next one
	if (row == 0)
		row = (m_lastline >= 0) ? (m_lastline + 1) : (m_lastline - 1);

	// remember the last request
	m_lastline = row;

	// invert if negative
	row = (row < 0) ? (m_numlines + row) : (row - 1);

	// clamp within range
	assert(m_numlines > 0);
	row = std::clamp(row, 0, m_numlines - 1);

	// return the appropriate string
	m_justify[row] = justify;
	return m_output[row];
}


//-------------------------------------------------
//  quote_expression - quote an expression
//  string so that it is valid to embed in an XML
//  document
//-------------------------------------------------

std::string cheat_manager::quote_expression(const parsed_expression &expression)
{
	std::string str = expression.original_string();

	strreplace(str, " && ", " and ");
	strreplace(str, " &&", " and ");
	strreplace(str, "&& ", " and ");
	strreplace(str, "&&", " and ");

	strreplace(str, " & ", " band ");
	strreplace(str, " &", " band ");
	strreplace(str, "& ", " band ");
	strreplace(str, "&", " band ");

	strreplace(str, " << ", " lshift ");
	strreplace(str, " <<", " lshift ");
	strreplace(str, "<< ", " lshift ");
	strreplace(str, "<<", " lshift ");

	strreplace(str, " <= ", " le ");
	strreplace(str, " <=", " le ");
	strreplace(str, "<= ", " le ");
	strreplace(str, "<=", " le ");

	strreplace(str, " < ", " lt ");
	strreplace(str, " <", " lt ");
	strreplace(str, "< ", " lt ");
	strreplace(str, "<", " lt ");

	return str;
}


//-------------------------------------------------
//  execute_frombcd - convert a value from BCD
//-------------------------------------------------

uint64_t cheat_manager::execute_frombcd(int params, const uint64_t *param)
{
	uint64_t value(param[0]);
	uint64_t multiplier(1);
	uint64_t result(0);

	while (value != 0)
	{
		result += (value & 0x0f) * multiplier;
		value >>= 4;
		multiplier *= 10;
	}
	return result;
}


//-------------------------------------------------
//  execute_tobcd - convert a value to BCD
//-------------------------------------------------

uint64_t cheat_manager::execute_tobcd(int params, const uint64_t *param)
{
	uint64_t value(param[0]);
	uint64_t result(0);
	uint8_t shift(0);

	while (value != 0)
	{
		result += (value % 10) << shift;
		value /= 10;
		shift += 4;
	}
	return result;
}


//-------------------------------------------------
//  frame_update - per-frame callback
//-------------------------------------------------

void cheat_manager::frame_update()
{
	// set up for accumulating output
	m_lastline = 0;
	m_numlines = floor(1.0f / mame_machine_manager::instance()->ui().get_line_height());
	m_numlines = std::min<uint8_t>(m_numlines, m_output.size());
	for (auto & elem : m_output)
		elem.clear();

	// iterate over running cheats and execute them
	for (auto &cheat : m_cheatlist)
		cheat->frame_update();

	// increment the frame counter
	m_framecount++;
}


//-------------------------------------------------
//  load_cheats - load a cheat file into memory
//  and create the cheat entry list
//-------------------------------------------------

void cheat_manager::load_cheats(std::string const &filename)
{
	std::string searchstr(machine().options().cheat_path());
	std::string curpath;
	for (path_iterator path(searchstr); path.next(curpath); )
	{
		searchstr.append(";").append(curpath).append(PATH_SEPARATOR "cheat");
	}
	emu_file cheatfile(std::move(searchstr), OPEN_FLAG_READ);
	try
	{
		// loop over all instances of the files found in our search paths
		for (std::error_condition filerr = cheatfile.open(filename + ".xml"); !filerr; filerr = cheatfile.open_next())
		{
			osd_printf_verbose("Loading cheats file from %s\n", cheatfile.fullpath());

			// read the XML file into internal data structures
			util::xml::parse_options options = { nullptr };
			util::xml::parse_error error;
			options.error = &error;
			util::xml::file::ptr const rootnode(util::xml::file::read(cheatfile, &options));

			// if unable to parse the file, just bail
			if (!rootnode)
				throw emu_fatalerror("%s.xml(%d): error parsing XML (%s)\n", filename, error.error_line, error.error_message);

			// find the layout node
			util::xml::data_node const *const mamecheatnode(rootnode->get_child("mamecheat"));
			if (mamecheatnode == nullptr)
				throw emu_fatalerror("%s.xml: missing mamecheatnode node", filename);

			// validate the config data version
			int const version(mamecheatnode->get_attribute_int("version", 0));
			if (version != CHEAT_VERSION)
				throw emu_fatalerror("%s.xml(%d): Invalid cheat XML file: unsupported version", filename, mamecheatnode->line);

			// parse all the elements
			for (util::xml::data_node const *cheatnode = mamecheatnode->get_child("cheat"); cheatnode != nullptr; cheatnode = cheatnode->get_next_sibling("cheat"))
			{
				try
				{
					// load this entry
					auto curcheat = std::make_unique<cheat_entry>(*this, m_symtable, filename, *cheatnode);

					// make sure we're not a duplicate
					if (REMOVE_DUPLICATE_CHEATS && curcheat->is_duplicate())
					{
						osd_printf_verbose("Ignoring duplicate cheat '%s' from file %s\n", curcheat->description(), cheatfile.fullpath());
					}
					else
					{
						// add to the end of the list
						m_cheatlist.push_back(std::move(curcheat));
					}
				}
				catch (emu_fatalerror const &err)
				{
					// just move on to the next cheat
					osd_printf_error("%s\n", err.what());
				}
			}
		}
	}
	catch (emu_fatalerror const &err)
	{
		// handle errors cleanly
		osd_printf_error("%s\n", err.what());
		m_cheatlist.clear();
	}
}
