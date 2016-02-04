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
#include "emuopts.h"
#include "xmlfile.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "cheat.h"
#include "debug/debugcpu.h"

#include <ctype.h>



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
		case XML_INT_FORMAT_DECIMAL:
			return strformat("%d", (UINT32)m_value);

		case XML_INT_FORMAT_DECIMAL_POUND:
			return strformat("#%d", (UINT32)m_value);

		case XML_INT_FORMAT_HEX_DOLLAR:
			return strformat("$%X", (UINT32)m_value);

		case XML_INT_FORMAT_HEX_C:
			return strformat("0x%X", (UINT32)m_value);
	}
}



//**************************************************************************
//  CHEAT PARAMETER
//**************************************************************************

//-------------------------------------------------
//  cheat_parameter - constructor
//-------------------------------------------------

cheat_parameter::cheat_parameter(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &paramnode)
	: m_value(0)
{
	// read the core attributes
	m_minval = number_and_format(xml_get_attribute_int(&paramnode, "min", 0), xml_get_attribute_int_format(&paramnode, "min"));
	m_maxval = number_and_format(xml_get_attribute_int(&paramnode, "max", 0), xml_get_attribute_int_format(&paramnode, "max"));
	m_stepval = number_and_format(xml_get_attribute_int(&paramnode, "step", 1), xml_get_attribute_int_format(&paramnode, "step"));

	// iterate over items
	for (xml_data_node *itemnode = xml_get_sibling(paramnode.child, "item"); itemnode != nullptr; itemnode = xml_get_sibling(itemnode->next, "item"))
	{
		// check for NULL text
		if (itemnode->value == nullptr || itemnode->value[0] == 0)
			throw emu_fatalerror("%s.xml(%d): item is missing text\n", filename, itemnode->line);

		// check for non-existant value
		if (xml_get_attribute(itemnode, "value") == nullptr)
			throw emu_fatalerror("%s.xml(%d): item is value\n", filename, itemnode->line);

		// extract the parameters
		UINT64 value = xml_get_attribute_int(itemnode, "value", 0);
		int format = xml_get_attribute_int_format(itemnode, "value");

		// allocate and append a new item
		item &curitem = m_itemlist.append(*global_alloc(item(itemnode->value, value, format)));

		// ensure the maximum expands to suit
		m_maxval = MAX(m_maxval, curitem.value());
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
		strprintf(m_curtext,"%d (0x%X)", UINT32(m_value), UINT32(m_value));

	// if not, we're an item cheat
	else
	{
		strprintf(m_curtext, "??? (%d)", UINT32(m_value));
		for (item *curitem = m_itemlist.first(); curitem != nullptr; curitem = curitem->next())
			if (curitem->value() == m_value)
			{
				m_curtext.assign(curitem->text());
				break;
			}
	}
	return m_curtext.c_str();
}


//-------------------------------------------------
//  save - save a single cheat parameter
//-------------------------------------------------

void cheat_parameter::save(emu_file &cheatfile) const
{
	// output the parameter tag
	cheatfile.printf("\t\t<parameter");

	// if no items, just output min/max/step
	if (!has_itemlist())
	{
		if (m_minval != 0)
			cheatfile.printf(" min=\"%s\"", m_minval.format().c_str());
		if (m_maxval != 0)
			cheatfile.printf(" max=\"%s\"", m_maxval.format().c_str());
		if (m_stepval != 1)
			cheatfile.printf(" step=\"%s\"", m_stepval.format().c_str());
		cheatfile.printf("/>\n");
	}

	// iterate over items
	else
	{
		for (const item *curitem = m_itemlist.first(); curitem != nullptr; curitem = curitem->next())
			cheatfile.printf("\t\t\t<item value=\"%s\">%s</item>\n", curitem->value().format().c_str(), curitem->text());
		cheatfile.printf("\t\t</parameter>\n");
	}
}


//-------------------------------------------------
//  set_minimum_state - set the minimum state
//-------------------------------------------------

bool cheat_parameter::set_minimum_state()
{
	UINT64 origvalue = m_value;

	// set based on whether we have an item list
	m_value = (!has_itemlist()) ? m_minval : m_itemlist.first()->value();

	return (m_value != origvalue);
}


//-------------------------------------------------
//  set_minimum_state - set the previous state
//-------------------------------------------------

bool cheat_parameter::set_prev_state()
{
	UINT64 origvalue = m_value;

	// are we a value cheat?
	if (!has_itemlist())
	{
		if (m_value < m_minval + m_stepval)
			m_value = m_minval;
		else
			m_value -= m_stepval;
	}

	// if not, we're an item cheat
	else
	{
		item *curitem, *previtem = nullptr;
		for (curitem = m_itemlist.first(); curitem != nullptr; previtem = curitem, curitem = curitem->next())
			if (curitem->value() == m_value)
				break;
		if (previtem != nullptr)
			m_value = previtem->value();
	}

	return (m_value != origvalue);
}


//-------------------------------------------------
//  set_next_state - advance to the next state
//-------------------------------------------------

bool cheat_parameter::set_next_state()
{
	UINT64 origvalue = m_value;

	// are we a value cheat?
	if (!has_itemlist())
	{
		if (m_value > m_maxval - m_stepval)
			m_value = m_maxval;
		else
			m_value += m_stepval;
	}

	// if not, we're an item cheat
	else
	{
		item *curitem;
		for (curitem = m_itemlist.first(); curitem != nullptr; curitem = curitem->next())
			if (curitem->value() == m_value)
				break;
		if (curitem != nullptr && curitem->next() != nullptr)
			m_value = curitem->next()->value();
	}

	return (m_value != origvalue);
}



//**************************************************************************
//  CHEAT SCRIPT
//**************************************************************************

//-------------------------------------------------
//  cheat_script - constructor
//-------------------------------------------------

cheat_script::cheat_script(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &scriptnode)
	: m_state(SCRIPT_STATE_RUN)
{
	// read the core attributes
	const char *state = xml_get_attribute_string(&scriptnode, "state", "run");
	if (strcmp(state, "on") == 0)
		m_state = SCRIPT_STATE_ON;
	else if (strcmp(state, "off") == 0)
		m_state = SCRIPT_STATE_OFF;
	else if (strcmp(state, "change") == 0)
		m_state = SCRIPT_STATE_CHANGE;
	else if (strcmp(state, "run") != 0)
		throw emu_fatalerror("%s.xml(%d): invalid script state '%s'\n", filename, scriptnode.line, state);

	// iterate over nodes within the script
	for (xml_data_node *entrynode = scriptnode.child; entrynode != nullptr; entrynode = entrynode->next)
	{
		// handle action nodes
		if (strcmp(entrynode->name, "action") == 0)
			m_entrylist.append(*global_alloc(script_entry(manager, symbols, filename, *entrynode, true)));

		// handle output nodes
		else if (strcmp(entrynode->name, "output") == 0)
			m_entrylist.append(*global_alloc(script_entry(manager, symbols, filename, *entrynode, false)));

		// anything else is ignored
		else
		{
			osd_printf_warning("%s.xml(%d): unknown script item '%s' will be lost if saved\n", filename, entrynode->line, entrynode->name);
			continue;
		}
	}
}


//-------------------------------------------------
//  execute - execute ourself
//-------------------------------------------------

void cheat_script::execute(cheat_manager &manager, UINT64 &argindex)
{
	// do nothing if disabled
	if (!manager.enabled())
		return;

	// iterate over entries
	for (script_entry *entry = m_entrylist.first(); entry != nullptr; entry = entry->next())
		entry->execute(manager, argindex);
}


//-------------------------------------------------
//  save - save a single cheat script
//-------------------------------------------------

void cheat_script::save(emu_file &cheatfile) const
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
	for (const script_entry *entry = m_entrylist.first(); entry != nullptr; entry = entry->next())
		entry->save(cheatfile);

	// close the tag
	cheatfile.printf("\t\t</script>\n");
}


//-------------------------------------------------
//  script_entry - constructor
//-------------------------------------------------

cheat_script::script_entry::script_entry(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &entrynode, bool isaction)
	: m_next(nullptr),
		m_condition(&symbols),
		m_expression(&symbols)
{
	const char *expression = nullptr;
	try
	{
		// read the condition if present
		expression = xml_get_attribute_string(&entrynode, "condition", nullptr);
		if (expression != nullptr)
			m_condition.parse(expression);

		// if this is an action, parse the expression
		if (isaction)
		{
			expression = entrynode.value;
			if (expression == nullptr || expression[0] == 0)
				throw emu_fatalerror("%s.xml(%d): missing expression in action tag\n", filename, entrynode.line);
			m_expression.parse(expression);
		}

		// otherwise, parse the attributes and arguments
		else
		{
			// extract format
			const char *format = xml_get_attribute_string(&entrynode, "format", nullptr);
			if (format == nullptr || format[0] == 0)
				throw emu_fatalerror("%s.xml(%d): missing format in output tag\n", filename, entrynode.line);
			m_format.assign(format);

			// extract other attributes
			m_line = xml_get_attribute_int(&entrynode, "line", 0);
			m_justify = JUSTIFY_LEFT;
			const char *align = xml_get_attribute_string(&entrynode, "align", "left");
			if (strcmp(align, "center") == 0)
				m_justify = JUSTIFY_CENTER;
			else if (strcmp(align, "right") == 0)
				m_justify = JUSTIFY_RIGHT;
			else if (strcmp(align, "left") != 0)
				throw emu_fatalerror("%s.xml(%d): invalid alignment '%s' specified\n", filename, entrynode.line, align);

			// then parse arguments
			int totalargs = 0;
			for (xml_data_node *argnode = xml_get_sibling(entrynode.child, "argument"); argnode != nullptr; argnode = xml_get_sibling(argnode->next, "argument"))
			{
				output_argument &curarg = m_arglist.append(*global_alloc(output_argument(manager, symbols, filename, *argnode)));

				// verify we didn't overrun the argument count
				totalargs += curarg.count();
				if (totalargs > MAX_ARGUMENTS)
					throw emu_fatalerror("%s.xml(%d): too many arguments (found %d, max is %d)\n", filename, argnode->line, totalargs, MAX_ARGUMENTS);
			}

			// validate the format against the arguments
			validate_format(filename, entrynode.line);
		}
	}
	catch (expression_error &err)
	{
		throw emu_fatalerror("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, entrynode.line, expression, err.code_string());
	}
}


//-------------------------------------------------
//  execute - execute a single script entry
//-------------------------------------------------

void cheat_script::script_entry::execute(cheat_manager &manager, UINT64 &argindex)
{
	// evaluate the condition
	if (!m_condition.is_empty())
	{
		try
		{
			UINT64 result = m_condition.execute();
			if (result == 0)
				return;
		}
		catch (expression_error &err)
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
		UINT64 params[MAX_ARGUMENTS];
		int curarg = 0;
		for (output_argument *arg = m_arglist.first(); arg != nullptr; arg = arg->next())
			curarg += arg->values(argindex, &params[curarg]);

		// generate the astring
		strprintf(manager.get_output_astring(m_line, m_justify), m_format.c_str(),
			(UINT32)params[0],  (UINT32)params[1],  (UINT32)params[2],  (UINT32)params[3],
			(UINT32)params[4],  (UINT32)params[5],  (UINT32)params[6],  (UINT32)params[7],
			(UINT32)params[8],  (UINT32)params[9],  (UINT32)params[10], (UINT32)params[11],
			(UINT32)params[12], (UINT32)params[13], (UINT32)params[14], (UINT32)params[15],
			(UINT32)params[16], (UINT32)params[17], (UINT32)params[18], (UINT32)params[19],
			(UINT32)params[20], (UINT32)params[21], (UINT32)params[22], (UINT32)params[23],
			(UINT32)params[24], (UINT32)params[25], (UINT32)params[26], (UINT32)params[27],
			(UINT32)params[28], (UINT32)params[29], (UINT32)params[30], (UINT32)params[31]);
	}
}


//-------------------------------------------------
//  save - save a single action or output
//-------------------------------------------------

void cheat_script::script_entry::save(emu_file &cheatfile) const
{
	// output an action
	if (m_format.empty())
	{
		cheatfile.printf("\t\t\t<action");
		if (!m_condition.is_empty())
			cheatfile.printf(" condition=\"%s\"", cheat_manager::quote_expression(m_condition).c_str());
		cheatfile.printf(">%s</action>\n", cheat_manager::quote_expression(m_expression).c_str());
	}

	// output an output
	else
	{
		cheatfile.printf("\t\t\t<output format=\"%s\"", m_format.c_str());
		if (!m_condition.is_empty())
			cheatfile.printf(" condition=\"%s\"", cheat_manager::quote_expression(m_condition).c_str());
		if (m_line != 0)
			cheatfile.printf(" line=\"%d\"", m_line);
		if (m_justify == JUSTIFY_CENTER)
			cheatfile.printf(" align=\"center\"");
		else if (m_justify == JUSTIFY_RIGHT)
			cheatfile.printf(" align=\"right\"");
		if (m_arglist.count() == 0)
			cheatfile.printf(" />\n");

		// output arguments
		else
		{
			cheatfile.printf(">\n");
			for (const output_argument *curarg = m_arglist.first(); curarg != nullptr; curarg = curarg->next())
				curarg->save(cheatfile);
			cheatfile.printf("\t\t\t</output>\n");
		}
	}
}


//-------------------------------------------------
//  validate_format - check that a format string
//  has the correct number and type of arguments
//-------------------------------------------------

void cheat_script::script_entry::validate_format(const char *filename, int line)
{
	// first count arguments
	int argsprovided = 0;
	for (const output_argument *curarg = m_arglist.first(); curarg != nullptr; curarg = curarg->next())
		argsprovided += curarg->count();

	// now scan the string for valid argument usage
	const char *p = strchr(m_format.c_str(), '%');
	int argscounted = 0;
	while (p != nullptr)
	{
		// skip past any valid attributes
		p++;
		while (strchr("lh0123456789.-+ #", *p) != nullptr)
			p++;

		// look for a valid type
		if (strchr("cdiouxX", *p) == nullptr)
			throw emu_fatalerror("%s.xml(%d): invalid format specification \"%s\"\n", filename, line, m_format.c_str());
		argscounted++;

		// look for the next one
		p = strchr(p, '%');
	}

	// did we match?
	if (argscounted < argsprovided)
		throw emu_fatalerror("%s.xml(%d): too many arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, m_format.c_str());
	if (argscounted > argsprovided)
		throw emu_fatalerror("%s.xml(%d): not enough arguments provided (%d) for format \"%s\"\n", filename, line, argsprovided, m_format.c_str());
}


//-------------------------------------------------
//  output_argument - constructor
//-------------------------------------------------

cheat_script::script_entry::output_argument::output_argument(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &argnode)
	: m_next(nullptr),
		m_expression(&symbols),
		m_count(0)
{
	// first extract attributes
	m_count = xml_get_attribute_int(&argnode, "count", 1);

	// read the expression
	const char *expression = argnode.value;
	if (expression == nullptr || expression[0] == 0)
		throw emu_fatalerror("%s.xml(%d): missing expression in argument tag\n", filename, argnode.line);

	// parse it
	try
	{
		m_expression.parse(expression);
	}
	catch (expression_error &err)
	{
		throw emu_fatalerror("%s.xml(%d): error parsing cheat expression \"%s\" (%s)\n", filename, argnode.line, expression, err.code_string());
	}
}


//-------------------------------------------------
//  value - return the evaluated value of the
//  given output argument
//-------------------------------------------------

int cheat_script::script_entry::output_argument::values(UINT64 &argindex, UINT64 *result)
{
	for (argindex = 0; argindex < m_count; argindex++)
	{
		try
		{
			result[argindex] = m_expression.execute();
		}
		catch (expression_error &err)
		{
			osd_printf_warning("Error executing argument expression \"%s\": %s\n", m_expression.original_string(), err.code_string());
		}
	}
	return m_count;
}


//-------------------------------------------------
//  save - save a single output argument
//-------------------------------------------------

void cheat_script::script_entry::output_argument::save(emu_file &cheatfile) const
{
	cheatfile.printf("\t\t\t\t<argument");
	if (m_count != 1)
		cheatfile.printf(" count=\"%d\"", (int)m_count);
	cheatfile.printf(">%s</argument>\n", cheat_manager::quote_expression(m_expression).c_str());
}



//**************************************************************************
//  CHEAT ENTRY
//**************************************************************************

//-------------------------------------------------
//  cheat_entry - constructor
//-------------------------------------------------

cheat_entry::cheat_entry(cheat_manager &manager, symbol_table &globaltable, const char *filename, xml_data_node &cheatnode)
	: m_manager(manager),
		m_next(nullptr),
		m_symbols(&manager.machine(), &globaltable),
		m_state(SCRIPT_STATE_OFF),
		m_numtemp(DEFAULT_TEMP_VARIABLES),
		m_argindex(0)
{
	// reset scripts
	try
	{
		// pull the variable count out ahead of things
		int tempcount = xml_get_attribute_int(&cheatnode, "tempvariables", DEFAULT_TEMP_VARIABLES);
		if (tempcount < 1)
			throw emu_fatalerror("%s.xml(%d): invalid tempvariables attribute (%d)\n", filename, cheatnode.line, tempcount);

		// allocate memory for the cheat
		m_numtemp = tempcount;

		// get the description
		const char *description = xml_get_attribute_string(&cheatnode, "desc", nullptr);
		if (description == nullptr || description[0] == 0)
			throw emu_fatalerror("%s.xml(%d): empty or missing desc attribute on cheat\n", filename, cheatnode.line);
		m_description = description;

		// create the symbol table
		m_symbols.add("argindex", symbol_table::READ_ONLY, &m_argindex);
		for (int curtemp = 0; curtemp < tempcount; curtemp++) {
			m_symbols.add(strformat("temp%d", curtemp).c_str(), symbol_table::READ_WRITE);
		}

		// read the first comment node
		xml_data_node *commentnode = xml_get_sibling(cheatnode.child, "comment");
		if (commentnode != nullptr)
		{
			// set the value if not NULL
			if (commentnode->value != nullptr && commentnode->value[0] != 0)
				m_comment.assign(commentnode->value);

			// only one comment is kept
			commentnode = xml_get_sibling(commentnode->next, "comment");
			if (commentnode != nullptr)
				osd_printf_warning("%s.xml(%d): only one comment node is retained; ignoring additional nodes\n", filename, commentnode->line);
		}

		// read the first parameter node
		xml_data_node *paramnode = xml_get_sibling(cheatnode.child, "parameter");
		if (paramnode != nullptr)
		{
			// load this parameter
			m_parameter.reset(global_alloc(cheat_parameter(manager, m_symbols, filename, *paramnode)));

			// only one parameter allowed
			paramnode = xml_get_sibling(paramnode->next, "parameter");
			if (paramnode != nullptr)
				osd_printf_warning("%s.xml(%d): only one parameter node allowed; ignoring additional nodes\n", filename, paramnode->line);
		}

		// read the script nodes
		for (xml_data_node *scriptnode = xml_get_sibling(cheatnode.child, "script"); scriptnode != nullptr; scriptnode = xml_get_sibling(scriptnode->next, "script"))
		{
			// load this entry
			auto curscript = global_alloc(cheat_script(manager, m_symbols, filename, *scriptnode));

			// if we have a script already for this slot, it is an error
			std::unique_ptr<cheat_script> &slot = script_for_state(curscript->state());
			if (slot != nullptr)
				osd_printf_warning("%s.xml(%d): only one on script allowed; ignoring additional scripts\n", filename, scriptnode->line);
			else
				slot.reset(curscript);
		}
	}
	catch (emu_fatalerror &)
	{
		// call our destructor to clean up and re-throw
		this->~cheat_entry();
		throw;
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

void cheat_entry::save(emu_file &cheatfile) const
{
	// determine if we have scripts
	bool has_scripts = (m_off_script != nullptr || m_on_script != nullptr || m_run_script != nullptr || m_change_script != nullptr);

	// output the cheat tag
	cheatfile.printf("\t<cheat desc=\"%s\"", m_description.c_str());
	if (m_numtemp != DEFAULT_TEMP_VARIABLES)
		cheatfile.printf(" tempvariables=\"%d\"", m_numtemp);
	if (m_comment.empty() && m_parameter == nullptr && !has_scripts)
		cheatfile.printf(" />\n");
	else
	{
		cheatfile.printf(">\n");

		// save the comment
		if (!m_comment.empty())
			cheatfile.printf("\t\t<comment><![CDATA[\n%s\n\t\t]]></comment>\n", m_comment.c_str());

		// output the parameter, if present
		if (m_parameter != nullptr)
			m_parameter->save(cheatfile);

		// output the script nodes
		if (m_on_script != nullptr)
			m_on_script->save(cheatfile);
		if (m_off_script != nullptr)
			m_off_script->save(cheatfile);
		if (m_change_script != nullptr)
			m_change_script->save(cheatfile);
		if (m_run_script != nullptr)
			m_run_script->save(cheatfile);

		// close the cheat tag
		cheatfile.printf("\t</cheat>\n");
	}
}


//-------------------------------------------------
//  activate - activate a oneshot cheat
//-------------------------------------------------

bool cheat_entry::activate()
{
	bool changed = false;

	// if cheats have been toggled off no point in even trying to do anything
	if (!m_manager.enabled())
		return changed;

	// if we're a oneshot cheat, execute the "on" script and indicate change
	if (is_oneshot())
	{
		execute_on_script();
		changed = true;
		manager().machine().popmessage("Activated %s", m_description.c_str());
	}

	// if we're a oneshot parameter cheat and we're active, execute the "state change" script and indicate change
	else if (is_oneshot_parameter() && m_state != SCRIPT_STATE_OFF)
	{
		execute_change_script();
		changed = true;
		manager().machine().popmessage("Activated\n %s = %s", m_description.c_str(), m_parameter->text());
	}

	return changed;
}


//-------------------------------------------------
//  select_default_state - select the default
//  state for a cheat, or activate a oneshot cheat
//-------------------------------------------------

bool cheat_entry::select_default_state()
{
	bool changed = false;

	// if we're a oneshot cheat, there is no default state
	if (is_oneshot())
		;

	// all other types switch to the "off" state
	else
		changed = set_state(SCRIPT_STATE_OFF);

	return changed;
}


//-------------------------------------------------
//  select_previous_state - select the previous
//  state for a cheat
//-------------------------------------------------

bool cheat_entry::select_previous_state()
{
	bool changed = false;

	// if we're a oneshot, there is no previous state
	if (is_oneshot())
		;

	// if we're on/off, toggle to off
	else if (is_onoff())
		changed = set_state(SCRIPT_STATE_OFF);

	// if we have a parameter, set the previous state
	else if (m_parameter != nullptr)
	{
		// if we're at our minimum, turn off
		if (m_parameter->is_minimum())
			changed = set_state(SCRIPT_STATE_OFF);
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
	bool changed = false;

	// if we're a oneshot, there is no next state
	if (is_oneshot())
		;

	// if we're on/off, toggle to running state
	else if (is_onoff())
		changed = set_state(SCRIPT_STATE_RUN);

	// if we have a parameter, set the next state
	else if (m_parameter != nullptr)
	{
		// if we're off, switch on to the minimum state
		if (m_state == SCRIPT_STATE_OFF)
		{
			changed = set_state(SCRIPT_STATE_RUN);
			m_parameter->set_minimum_state();
		}

		// otherwise, switch to the next state
		else
			changed = m_parameter->set_next_state();

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

void cheat_entry::menu_text(std::string &description, std::string &state, UINT32 &flags)
{
	// description is standard
	description.assign(m_description);
	state.clear();
	flags = 0;

	// some cheat entries are just text for display
	if (is_text_only())
	{
		if (!description.empty())
		{
			strtrimspace(description);
			if (description.empty())
				description.assign(MENU_SEPARATOR_ITEM);
		}
		flags = MENU_FLAG_DISABLE;
	}

	// if we have no parameter and no run or off script, it's a oneshot cheat
	else if (is_oneshot())
		state.assign("Set");

	// if we have no parameter, it's just on/off
	else if (is_onoff())
	{
		state.assign((m_state == SCRIPT_STATE_RUN) ? "On" : "Off");
		flags = (m_state != 0) ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW;
	}

	// if we have a value parameter, compute it
	else if (m_parameter != nullptr)
	{
		if (m_state == SCRIPT_STATE_OFF)
		{
			state.assign(is_oneshot_parameter() ? "Set" : "Off");
			flags = MENU_FLAG_RIGHT_ARROW;
		}
		else
		{
			state.assign(m_parameter->text());
			flags = MENU_FLAG_LEFT_ARROW;
			if (!m_parameter->is_maximum())
				flags |= MENU_FLAG_RIGHT_ARROW;
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
	else if (newstate == SCRIPT_STATE_ON || newstate == SCRIPT_STATE_RUN)
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



//**************************************************************************
//  CHEAT MANAGER
//**************************************************************************

//-------------------------------------------------
//  cheat_manager - constructor
//-------------------------------------------------

cheat_manager::cheat_manager(running_machine &machine)
	: m_machine(machine),
		m_disabled(true),
		m_symtable(&machine)
{
	// if the cheat engine is disabled, we're done
	if (!machine.options().cheat())
		return;

	m_output.resize(UI_TARGET_FONT_ROWS*2);
	m_justify.resize(UI_TARGET_FONT_ROWS*2);

	// request a callback
	machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(FUNC(cheat_manager::frame_update), this));

	// create a global symbol table
	m_symtable.add("frame", symbol_table::READ_ONLY, &m_framecount);
	m_symtable.add("frombcd", nullptr, 1, 1, execute_frombcd);
	m_symtable.add("tobcd", nullptr, 1, 1, execute_tobcd);

	// we rely on the debugger expression callbacks; if the debugger isn't
	// enabled, we must jumpstart them manually
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) == 0)
		debug_cpu_init(machine);

	// configure for memory access (shared with debugger)
	debug_cpu_configure_memory(machine, m_symtable);

	// load the cheats
	reload();
}


//-------------------------------------------------
//  set_enable - globally enable or disable the
//  cheat engine
//-------------------------------------------------

void cheat_manager::set_enable(bool enable)
{
	// if the cheat engine is disabled, we're done
	if (!machine().options().cheat())
		return;

	// if we're enabled currently and we don't want to be, turn things off
	if (!m_disabled && !enable)
	{
		// iterate over running cheats and execute any OFF Scripts
		for (cheat_entry *cheat = m_cheatlist.first(); cheat != nullptr; cheat = cheat->next())
			if (cheat->state() == SCRIPT_STATE_RUN)
				cheat->execute_off_script();
		machine().popmessage("Cheats Disabled");
		m_disabled = true;
	}

	// if we're disabled currently and we want to be enabled, turn things on
	else if (m_disabled && enable)
	{
		// iterate over running cheats and execute any ON Scripts
		m_disabled = false;
		for (cheat_entry *cheat = m_cheatlist.first(); cheat != nullptr; cheat = cheat->next())
			if (cheat->state() == SCRIPT_STATE_RUN)
				cheat->execute_on_script();
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
	m_cheatlist.reset();

	// reset state
	m_framecount = 0;
	m_numlines = 0;
	m_lastline = 0;
	m_disabled = false;

	// load the cheat file, if it's a system that has a software list then try softlist_name/shortname.xml first,
	// if it fails to load then try to load via crc32 - basename/crc32.xml ( eg. 01234567.xml )
	image_interface_iterator iter(machine().root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next())
		if (image->exists())
		{
			// if we are loading through a software list, try to load softlist_name/shortname.xml
			// this allows the coexistence of arcade cheats with cheats for home conversions which
			// have the same shortname
			if (image->software_entry() != nullptr)
			{
				std::string filename;
				strprintf(filename, "%s%s%s", image->software_list_name(), PATH_SEPARATOR, image->basename());
				load_cheats(filename.c_str());
				break;
			}
			// else we are loading outside the software list, try to load machine_basename/crc32.xml
			else
			{
				UINT32 crc = image->crc();
				if (crc != 0)
				{
					std::string filename;
					strprintf(filename, "%s%s%08X", machine().basename(), PATH_SEPARATOR, crc);
					load_cheats(filename.c_str());
					break;
				}
			}
		}

	// if we haven't found the cheats yet, load by basename
	if (m_cheatlist.count() == 0)
		load_cheats(machine().basename());

	// temporary: save the file back out as output.xml for comparison
	if (m_cheatlist.count() != 0)
		save_all("output");
}


//-------------------------------------------------
//  cheat_list_save - save a cheat file from
//  memory to the given filename
//-------------------------------------------------

bool cheat_manager::save_all(const char *filename)
{
	// open the file with the proper name
	emu_file cheatfile(machine().options().cheat_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = cheatfile.open(filename, ".xml");

	// if that failed, return nothing
	if (filerr != FILERR_NONE)
		return false;

	// wrap the rest of catch errors
	try
	{
		// output the outer layers
		cheatfile.printf("<?xml version=\"1.0\"?>\n");
		cheatfile.printf("<!-- This file is autogenerated; comments and unknown tags will be stripped -->\n");
		cheatfile.printf("<mamecheat version=\"%d\">\n", CHEAT_VERSION);

		// iterate over cheats in the list and save them
		for (cheat_entry *cheat = m_cheatlist.first(); cheat != nullptr; cheat = cheat->next())
			cheat->save(cheatfile);

		// close out the file
		cheatfile.printf("</mamecheat>\n");
		return true;
	}

	// catch errors and cleanup
	catch (emu_fatalerror &err)
	{
		osd_printf_error("%s\n", err.string());
		cheatfile.remove_on_close();
	}
	return false;
}


//-------------------------------------------------
//  render_text - called by the UI system to
//  render text
//-------------------------------------------------

void cheat_manager::render_text(render_container &container)
{
	// render any text and free it along the way
	for (int linenum = 0; linenum < m_output.size(); linenum++)
		if (!m_output[linenum].empty())
		{
			// output the text
			machine().ui().draw_text_full(&container, m_output[linenum].c_str(),
					0.0f, (float)linenum * machine().ui().get_line_height(), 1.0f,
					m_justify[linenum], WRAP_NEVER, DRAW_OPAQUE,
					ARGB_WHITE, ARGB_BLACK, nullptr, nullptr);
		}
}


//-------------------------------------------------
//  get_output_astring - return a reference to
//  the given row's string, and set the
//  justification
//-------------------------------------------------

std::string &cheat_manager::get_output_astring(int row, int justify)
{
	// if the row is not specified, grab the next one
	if (row == 0)
		row = (m_lastline >= 0) ? m_lastline + 1 : m_lastline - 1;

	// remember the last request
	m_lastline = row;

	// invert if negative
	row = (row < 0) ? m_numlines + row : row - 1;

	// clamp within range
	row = MAX(row, 0);
	row = MIN(row, m_numlines - 1);

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

	strreplace(str, " <= ", " le ");
	strreplace(str, " <=", " le ");
	strreplace(str, "<= ", " le ");
	strreplace(str, "<=", " le ");

	strreplace(str, " < ", " lt ");
	strreplace(str, " <", " lt ");
	strreplace(str, "< ", " lt ");
	strreplace(str, "<", " lt ");

	strreplace(str, " << ", " lshift ");
	strreplace(str, " <<", " lshift ");
	strreplace(str, "<< ", " lshift ");
	strreplace(str, "<<", " lshift ");

	return str;
}


//-------------------------------------------------
//  execute_frombcd - convert a value from BCD
//-------------------------------------------------

UINT64 cheat_manager::execute_frombcd(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	UINT64 value = param[0];
	UINT64 multiplier = 1;
	UINT64 result = 0;

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

UINT64 cheat_manager::execute_tobcd(symbol_table &table, void *ref, int params, const UINT64 *param)
{
	UINT64 value = param[0];
	UINT64 result = 0;
	UINT8 shift = 0;

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
	m_numlines = floor(1.0f / machine().ui().get_line_height());
	m_numlines = MIN(m_numlines, m_output.size());
	for (auto & elem : m_output)
		elem.clear();

	// iterate over running cheats and execute them
	for (cheat_entry *cheat = m_cheatlist.first(); cheat != nullptr; cheat = cheat->next())
		cheat->frame_update();

	// increment the frame counter
	m_framecount++;
}


//-------------------------------------------------
//  load_cheats - load a cheat file into memory
//  and create the cheat entry list
//-------------------------------------------------

void cheat_manager::load_cheats(const char *filename)
{
	xml_data_node *rootnode = nullptr;
	std::string searchstr(machine().options().cheat_path());
	path_iterator path(searchstr.c_str());
	std::string curpath;
	while (path.next(curpath))
	{
		searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append("cheat");
	}
	emu_file cheatfile(searchstr.c_str(), OPEN_FLAG_READ);
	try
	{
		// open the file with the proper name
		file_error filerr = cheatfile.open(filename, ".xml");

		// loop over all instrances of the files found in our search paths
		while (filerr == FILERR_NONE)
		{
			osd_printf_verbose("Loading cheats file from %s\n", cheatfile.fullpath());

			// read the XML file into internal data structures
			xml_parse_options options = { nullptr };
			xml_parse_error error;
			options.error = &error;
			rootnode = xml_file_read(cheatfile, &options);

			// if unable to parse the file, just bail
			if (rootnode == nullptr)
				throw emu_fatalerror("%s.xml(%d): error parsing XML (%s)\n", filename, error.error_line, error.error_message);

			// find the layout node
			xml_data_node *mamecheatnode = xml_get_sibling(rootnode->child, "mamecheat");
			if (mamecheatnode == nullptr)
				throw emu_fatalerror("%s.xml: missing mamecheatnode node", filename);

			// validate the config data version
			int version = xml_get_attribute_int(mamecheatnode, "version", 0);
			if (version != CHEAT_VERSION)
				throw emu_fatalerror("%s.xml(%d): Invalid cheat XML file: unsupported version", filename, mamecheatnode->line);

			// parse all the elements
			for (xml_data_node *cheatnode = xml_get_sibling(mamecheatnode->child, "cheat"); cheatnode != nullptr; cheatnode = xml_get_sibling(cheatnode->next, "cheat"))
			{
				// load this entry
				auto curcheat = global_alloc(cheat_entry(*this, m_symtable, filename, *cheatnode));

				// make sure we're not a duplicate
				cheat_entry *scannode = nullptr;
				if (REMOVE_DUPLICATE_CHEATS)
					for (scannode = m_cheatlist.first(); scannode != nullptr; scannode = scannode->next())
						if (strcmp(scannode->description(), curcheat->description()) == 0)
						{
							osd_printf_verbose("Ignoring duplicate cheat '%s' from file %s\n", curcheat->description(), cheatfile.fullpath());
							break;
						}

				// add to the end of the list
				if (scannode == nullptr)
					m_cheatlist.append(*curcheat);
				else
					global_free(curcheat);
			}

			// free the file and loop for the next one
			xml_file_free(rootnode);

			// open the next file in sequence
			filerr = cheatfile.open_next();
		}
	}

	// handle errors cleanly
	catch (emu_fatalerror &err)
	{
		osd_printf_error("%s\n", err.string());
		m_cheatlist.reset();
		if (rootnode != nullptr)
			xml_file_free(rootnode);
	}
}
