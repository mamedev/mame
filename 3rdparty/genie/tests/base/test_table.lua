--
-- tests/base/test_table.lua
-- Automated test suite for the new table functions.
-- Copyright (c) 2008-2010 Jason Perkins and the Premake project
--


	T.table = { }
	local suite = T.table


--
-- table.contains() tests
--

	function suite.contains_OnContained()
		t = { "one", "two", "three" }
		test.istrue( table.contains(t, "two") )
	end

	function suite.contains_OnNotContained()
		t = { "one", "two", "three" }
		test.isfalse( table.contains(t, "four") )
	end


--
-- table.flatten() tests
--

	function suite.flatten_OnMixedValues()
		t = { "a", { "b", "c" }, "d" }
		test.isequal({ "a", "b", "c", "d" }, table.flatten(t))
	end


--
-- table.implode() tests
--

	function suite.implode()
		t = { "one", "two", "three", "four" }
		test.isequal("[one], [two], [three], [four]", table.implode(t, "[", "]", ", "))
	end


--
-- table.isempty() tests
--

	function suite.isempty_ReturnsTrueOnEmpty()
		test.istrue(table.isempty({}))
	end

	function suite.isempty_ReturnsFalseOnNotEmpty()
		test.isfalse(table.isempty({ 1 }))
	end
	
	function suite.isempty_ReturnsFalseOnNotEmptyMap()
		test.isfalse(table.isempty({ name = 'premake' }))
	end
	
	function suite.isempty_ReturnsFalseOnNotEmptyMapWithFalseKey()
		test.isfalse(table.isempty({ [false] = 0 }))
	end
