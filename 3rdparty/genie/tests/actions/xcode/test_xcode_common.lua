--
-- tests/actions/xcode/test_xcode_common.lua
-- Automated test suite for functions shared between Xcode projects and solutions
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.xcode3common = { }

	local suite = T.xcode3common
	local xcode = premake.xcode


--
-- Replacement for xcode.newid(). Creates a synthetic ID based on the node name,
-- its intended usage (file ID, build ID, etc.) and its place in the tree. This 
-- makes it easier to tell if the right ID is being used in the right places.
--

	xcode.used_ids = {}
	
	xcode.newid = function(node, usage)
		local name = node.name
		if usage then
			name = name .. ":" .. usage
		end
		
		if xcode.used_ids[name] then
			local count = xcode.used_ids[name] + 1
			xcode.used_ids[name] = count
			name = name .. "(" .. count .. ")"
		else
			xcode.used_ids[name] = 1
		end
		
		return "[" .. name .. "]"
	end


---------------------------------------------------------------------------
-- Header/footer tests
---------------------------------------------------------------------------

	function suite.Header_IsCorrect()
		xcode.Header()
		test.capture [[
// !$*UTF8*$!
{
	archiveVersion = 1;
	classes = {
	};
	objectVersion = 45;
	objects = {

		]]
	end


	function suite.Footer()
		xcode.Footer()
		test.capture [[
	};
	rootObject = 08FB7793FE84155DC02AAC07 /* Project object */;
}
		]]
	end


