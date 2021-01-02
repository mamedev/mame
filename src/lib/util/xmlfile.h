// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    xmlfile.h

    XML file parsing code.

***************************************************************************/
#ifndef MAME_LIB_UTIL_XMLFILE_H
#define MAME_LIB_UTIL_XMLFILE_H

#pragma once

#include "corefile.h"

#include <list>
#include <string>
#include <utility>


// forward type declarations
struct XML_ParserStruct;


namespace util::xml {

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	PARSE_FLAG_WHITESPACE_SIGNIFICANT = 1
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* extended error information from parsing */
struct parse_error
{
	parse_error() = default;

	const char *            error_message = nullptr;
	int                     error_line = 0;
	int                     error_column = 0;
};


// parsing options
struct parse_options
{
	parse_options() = default;

	parse_error *       error = nullptr;
	void                (*init_parser)(XML_ParserStruct *parser) = nullptr;
	uint32_t            flags = 0;
};


// a node representing a data item and its relationships
class data_node
{
public:
	enum class int_format
	{
		DECIMAL,
		DECIMAL_HASH,
		HEX_DOLLAR,
		HEX_C
	};


	/* ----- XML node management ----- */

	char const *get_name() const { return m_name.empty() ? nullptr : m_name.c_str(); }

	char const *get_value() const { return m_value.empty() ? nullptr : m_value.c_str(); }
	void set_value(char const *value);
	void append_value(char const *value, int length);
	void trim_whitespace();

	data_node *get_parent() { return m_parent; }
	data_node const *get_parent() const { return m_parent; }

	// count the number of children
	std::size_t count_children() const;
	std::size_t count_attributes() const;

	// get the first child
	data_node *get_first_child() { return m_first_child; }
	data_node const *get_first_child() const { return m_first_child; }

	// find the first child with the given tag
	data_node *get_child(const char *name);
	data_node const *get_child(const char *name) const;

	// find the first child with the given tag and/or attribute/value pair
	data_node *find_first_matching_child(const char *name, const char *attribute, const char *matchval);
	data_node const *find_first_matching_child(const char *name, const char *attribute, const char *matchval) const;

	// get the next sibling
	data_node *get_next_sibling() { return m_next; }
	data_node const *get_next_sibling() const { return m_next; }

	// find the next sibling with the given tag
	data_node *get_next_sibling(const char *name);
	data_node const *get_next_sibling(const char *name) const;

	// find the next sibling with the given tag and/or attribute/value pair
	data_node *find_next_matching_sibling(const char *name, const char *attribute, const char *matchval);
	data_node const *find_next_matching_sibling(const char *name, const char *attribute, const char *matchval) const;

	// add a new child node
	data_node *add_child(const char *name, const char *value);

	// either return an existing child node or create one if it doesn't exist
	data_node *get_or_add_child(const char *name, const char *value);

	// recursively copy as child of another node
	data_node *copy_into(data_node &parent) const;

	// delete a node and its children
	void delete_node();


	/* ----- XML attribute management ----- */

	// return whether a node has the specified attribute
	bool has_attribute(const char *attribute) const;

	// return a pointer to the string value of an attribute, or nullptr if not present
	std::string const *get_attribute_string_ptr(const char *attribute) const;

	// return the string value of an attribute, or the specified default if not present
	const char *get_attribute_string(const char *attribute, const char *defvalue) const;

	// return the integer value of an attribute, or the specified default if not present
	long long get_attribute_int(const char *attribute, long long defvalue) const;

	// return the format of the given integer attribute
	int_format get_attribute_int_format(const char *attribute) const;

	// return the float value of an attribute, or the specified default if not present
	float get_attribute_float(const char *attribute, float defvalue) const;

	// set the string value of an attribute
	void set_attribute(const char *name, const char *value);

	// set the integer value of an attribute
	void set_attribute_int(const char *name, long long value);

	// set the float value of an attribute
	void set_attribute_float(const char *name, float value);

	// add an attribute even if an attribute with the same name already exists
	void add_attribute(const char *name, const char *value);



	int                     line;           /* line number for this node's start */


protected:
	data_node();
	~data_node();

	void write_recursive(int indent, util::core_file &file) const;


private:
	// a node representing an attribute
	struct attribute_node
	{
		template <typename T, typename U> attribute_node(T &&name, U &&value) : name(std::forward<T>(name)), value(std::forward<U>(value)) { }

		std::string name;
		std::string value;
	};


	data_node(data_node *parent, const char *name, const char *value);

	data_node(data_node const &) = delete;
	data_node(data_node &&) = delete;
	data_node &operator=(data_node &&) = delete;
	data_node &operator=(data_node const &) = delete;

	data_node *get_sibling(const char *name);
	data_node const *get_sibling(const char *name) const;
	data_node *find_matching_sibling(const char *name, const char *attribute, const char *matchval);
	data_node const *find_matching_sibling(const char *name, const char *attribute, const char *matchval) const;

	attribute_node *get_attribute(const char *attribute);
	attribute_node const *get_attribute(const char *attribute) const;

	void free_children();


	data_node *                 m_next;
	data_node *                 m_first_child;
	std::string                 m_name;
	std::string                 m_value;
	data_node *                 m_parent;
	std::list<attribute_node>   m_attributes;
};


// a node representing the root of a document
class file : public data_node
{
public:
	using ptr = std::unique_ptr<file>;


	~file();

	// create a new, empty XML file
	static ptr create();

	// parse an XML file into its nodes
	static ptr read(util::core_file &file, parse_options const *opts);

	// parse an XML string into its nodes
	static ptr string_read(const char *string, parse_options const *opts);

	// write an XML tree to a file
	void write(util::core_file &file) const;


private:
	file();
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- miscellaneous interfaces ----- */

/* normalize a string into something that can be written to an XML file */
const char *normalize_string(const char *string);

} // namespace util::xml

#endif // MAME_LIB_UTIL_XMLFILE_H
