--
-- tests/base/test_tree.lua
-- Automated test suite source code tree handling.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.tree = { }
	local suite = T.tree
	local tree = premake.tree


--
-- Setup/teardown
--

	local tr, nodes
			
	function suite.setup()
		tr = tree.new()
		nodes = { }
	end

	local function getresult()
		tree.traverse(tr, {
			onnode = function(node, depth)
				table.insert(nodes, string.rep(">", depth) .. node.name)
			end
		})
		return table.concat(nodes)
	end

	

--
-- Tests for tree.new()
--

	function suite.NewReturnsObject()
		test.isnotnil(tr)
	end


--
-- Tests for tree.add()
--

	function suite.CanAddAtRoot()
		tree.add(tr, "Root")
		test.isequal("Root", getresult())
	end

	function suite.CanAddAtChild()
		tree.add(tr, "Root/Child")
		test.isequal("Root>Child", getresult())
	end

	function suite.CanAddAtGrandchild()
		tree.add(tr, "Root/Child/Grandchild")
		test.isequal("Root>Child>>Grandchild", getresult())
	end
	
	function suite.SkipsLeadingDotDots()
		tree.add(tr, "../MyProject/hello")
		test.isequal("MyProject>hello", getresult())
	end

	function suite.SkipsInlineDotDots()
		tree.add(tr, "MyProject/../hello")
		test.isequal("MyProject>hello", getresult())
	end
	
	function suite.AddsNodes_OnDifferentParentLevel()
		tree.add(tr, "../Common")
		tree.add(tr, "../../Common")
		test.isequal(2, #tr.children)
		test.isequal("Common", tr.children[1].name)
		test.isequal("Common", tr.children[2].name)
		test.isequal("../Common", tr.children[1].path)
		test.isequal("../../Common", tr.children[2].path)
	end


--
-- Tests for tree.getlocalpath()
--

	function suite.GetLocalPath_ReturnsPath_OnNoParentPath()
		local c = tree.add(tr, "Root/Child")
		c.parent.path = nil
		test.isequal("Root/Child", tree.getlocalpath(c))
	end

	function suite.GetLocalPath_ReturnsName_OnParentPathSet()
		local c = tree.add(tr, "Root/Child")
		test.isequal("Child", tree.getlocalpath(c))
	end


--
-- Tests for tree.remove()
--

	function suite.Remove_RemovesNodes()
		local n1 = tree.add(tr, "1")
		local n2 = tree.add(tr, "2")
		local n3 = tree.add(tr, "3")
		tree.remove(n2)
		local r = ""
		for _, n in ipairs(tr.children) do r = r .. n.name end
		test.isequal("13", r)
	end

	
	function suite.Remove_WorksInTraversal()
		tree.add(tr, "Root/1")
		tree.add(tr, "Root/2")
		tree.add(tr, "Root/3")
		local r = ""
		tree.traverse(tr, {
			onleaf = function(node)
				r = r .. node.name
				tree.remove(node)
			end
		})
		test.isequal("123", r)
		test.isequal(0, #tr.children[1])
	end


--
-- Tests for tree.sort()
--

	function suite.Sort_SortsAllLevels()
		tree.add(tr, "B/3")
		tree.add(tr, "B/1")
		tree.add(tr, "A/2")
		tree.add(tr, "A/1")
		tree.add(tr, "B/2")
		tree.sort(tr)
		test.isequal("A>1>2B>1>2>3", getresult(tr))
	end

