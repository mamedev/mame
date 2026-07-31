#ifdef PUGIXML_COMPACT
#include "test.hpp"

using namespace pugi;

static void overflow_hash_table(xml_document& doc)
{
	xml_node n = doc.child(STR("n"));

	// compact encoding assumes next_sibling is a forward-only pointer so we can allocate hash entries by reordering nodes
	// we allocate enough hash entries to be exactly on the edge of rehash threshold
	for (int i = 0; i < 8; ++i)
		CHECK(n.prepend_child(node_element));
}

TEST_XML_FLAGS(compact_out_of_memory_string, "<n a='v'/><?n v?>", parse_pi)
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_attribute a = doc.child(STR("n")).attribute(STR("a"));
	xml_node pi = doc.last_child();

	CHECK_ALLOC_FAIL(CHECK(!pi.set_name(STR("name"))));
	CHECK_ALLOC_FAIL(CHECK(!pi.set_value(STR("value"))));
	CHECK_ALLOC_FAIL(CHECK(!a.set_name(STR("name"))));
	CHECK_ALLOC_FAIL(CHECK(!a.set_value(STR("value"))));
}

TEST_XML(compact_out_of_memory_attribute, "<n a='v'/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));
	xml_attribute a = n.attribute(STR("a"));

	CHECK_ALLOC_FAIL(CHECK(!n.append_attribute(STR(""))));
	CHECK_ALLOC_FAIL(CHECK(!n.prepend_attribute(STR(""))));
	CHECK_ALLOC_FAIL(CHECK(!n.insert_attribute_after(STR(""), a)));
	CHECK_ALLOC_FAIL(CHECK(!n.insert_attribute_before(STR(""), a)));
}

TEST_XML(compact_out_of_memory_attribute_copy, "<n a='v'/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));
	xml_attribute a = n.attribute(STR("a"));

	CHECK_ALLOC_FAIL(CHECK(!n.append_copy(a)));
	CHECK_ALLOC_FAIL(CHECK(!n.prepend_copy(a)));
	CHECK_ALLOC_FAIL(CHECK(!n.insert_copy_after(a, a)));
	CHECK_ALLOC_FAIL(CHECK(!n.insert_copy_before(a, a)));
}

TEST_XML(compact_out_of_memory_node, "<n/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));

	CHECK_ALLOC_FAIL(CHECK(!doc.append_child(node_element)));
	CHECK_ALLOC_FAIL(CHECK(!doc.prepend_child(node_element)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_child_after(node_element, n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_child_before(node_element, n)));
}

TEST_XML(compact_out_of_memory_node_copy, "<n/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));

	CHECK_ALLOC_FAIL(CHECK(!doc.append_copy(n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.prepend_copy(n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_copy_after(n, n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_copy_before(n, n)));
}

TEST_XML(compact_out_of_memory_node_move, "<n/><ne/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));
	xml_node ne = doc.child(STR("ne"));

	CHECK_ALLOC_FAIL(CHECK(!doc.append_move(n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.prepend_move(n)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_move_after(n, ne)));
	CHECK_ALLOC_FAIL(CHECK(!doc.insert_move_before(n, ne)));
}

TEST_XML(compact_out_of_memory_remove, "<n a='v'/>")
{
	test_runner::_memory_fail_threshold = 1;

	overflow_hash_table(doc);

	xml_node n = doc.child(STR("n"));
	xml_attribute a = n.attribute(STR("a"));

	CHECK_ALLOC_FAIL(CHECK(!n.remove_attribute(a)));
	CHECK_ALLOC_FAIL(CHECK(!doc.remove_child(n)));
}

TEST_XML(compact_pointer_attribute_list, "<n a='v'/>")
{
	xml_node n = doc.child(STR("n"));
	xml_attribute a = n.attribute(STR("a"));

	// make sure we fill the page with node x
	for (int i = 0; i < 1000; ++i)
		doc.append_child(STR("x"));

	// this requires extended encoding for prev_attribute_c/next_attribute
	n.append_attribute(STR("b"));

	// this requires extended encoding for first_attribute
	n.remove_attribute(a);

	CHECK(!n.attribute(STR("a")));
	CHECK(n.attribute(STR("b")));
}

TEST_XML(compact_pointer_node_list, "<n/>")
{
	xml_node n = doc.child(STR("n"));

	// make sure we fill the page with node x
	// this requires extended encoding for prev_sibling_c/next_sibling
	for (int i = 0; i < 1000; ++i)
		doc.append_child(STR("x"));

	// this requires extended encoding for first_child
	n.append_child(STR("child"));

	CHECK(n.child(STR("child")));
}
#endif
