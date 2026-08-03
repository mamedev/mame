#define PUGIXML_DEPRECATED // Suppress deprecated declarations to avoid warnings

#include "test.hpp"

using namespace pugi;

TEST(document_deprecated_load)
{
	xml_document doc;
	CHECK(doc.load(STR("<node/>")));
	CHECK_NODE(doc, STR("<node/>"));
}

#ifndef PUGIXML_NO_XPATH
TEST_XML(xpath_api_deprecated_select_single_node, "<node><head/><foo id='1'/><foo/><tail/></node>")
{
	xpath_node n1 = doc.select_single_node(STR("node/foo"));

	xpath_query q(STR("node/foo"));
	xpath_node n2 = doc.select_single_node(q);

	CHECK(n1.node().attribute(STR("id")).as_int() == 1);
	CHECK(n2.node().attribute(STR("id")).as_int() == 1);
}
#endif
