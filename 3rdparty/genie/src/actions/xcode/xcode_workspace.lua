local premake = premake
local xcode = premake.xcode

xcode.allscheme = false

function xcode.workspace_head()
	_p('<?xml version="1.0" encoding="UTF-8"?>')
	_p('<Workspace')
	_p(1,'version = "1.0">')
end

function xcode.workspace_tail()
	_p('</Workspace>')
end

function xcode.workspace_file_ref(prj, indent)
	local projpath = path.getrelative(prj.solution.location, prj.location)
	if projpath == '.' then projpath = ''
	else projpath = projpath ..'/'
	end
	_p(indent, '<FileRef')
	_p(indent + 1, 'location = "group:%s">', projpath .. prj.name .. '.xcodeproj')
	_p(indent, '</FileRef>')
end

function xcode.workspace_group(grp, indent)
	_p(indent, '<Group')
	_p(indent + 1, 'location = "container:"')
	_p(indent + 1, 'name = "%s">', grp.name)

	local function comparenames(a, b)
		return a.name < b.name
	end

	local groups = table.join(grp.groups)
	local projects = table.join(grp.projects)
	table.sort(groups, comparenames)
	table.sort(projects, comparenames)

	for _, child in ipairs(groups) do
		xcode.workspace_group(child, indent + 1)
	end

	for _, prj in ipairs(projects) do
		xcode.workspace_file_ref(prj, indent + 1)
	end

	_p(indent, '</Group>')
end

function xcode.workspace_generate(sln)
	xcode.preparesolution(sln)
	xcode.workspace_head()
	xcode.reorderProjects(sln)

	for grp in premake.solution.eachgroup(sln) do
		if grp.parent == nil then
			xcode.workspace_group(grp, 1)
		end
	end

	for prj in premake.solution.eachproject(sln) do
		if prj.group == nil then
			xcode.workspace_file_ref(prj, 1)
		end
	end

	xcode.workspace_tail()
end


--
-- Generate the `-ALL` scheme, building all targets.
--

function xcode.workspace_scheme(sln)
	if not xcode.allscheme then
		return false
	end

	local projects = {}
	local primary = nil

	for prj in premake.solution.eachproject(sln) do
		if not primary or (sln.startproject == prj.name) then
			primary = prj
		end
		table.insert(projects, prj)
	end

	xcode.scheme(projects, primary)
end

--
-- Generate the workspace settings file, preventing xcode from auto-generating
-- schemes we have manually provided.
--

function xcode.workspace_settings(sln)
	_p('<?xml version="1.0" encoding="UTF-8"?>')
	_p('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">')
	_p('<plist version="1.0">')
	_p('<dict>')
	_p(1, '<key>IDEWorkspaceSharedSettings_AutocreateContextsIfNeeded</key>')
	_p(1, '<false/>')
	_p('</dict>')
	_p('</plist>')
end

--
-- If a startup project is specified, move it to the front of the project list.
-- This will make Visual Studio treat it like a startup project.
--

function xcode.reorderProjects(sln)
	if sln.startproject then
			for i, prj in ipairs(sln.projects) do
				if sln.startproject == prj.name then
					-- Move group tree containing the project to start of group list
					local cur = prj.group
					while cur ~= nil do
						-- Remove group from array
						for j, group in ipairs(sln.groups) do
							if group == cur then
								table.remove(sln.groups, j)
								break
							end
						end

						-- Add back at start
						table.insert(sln.groups, 1, cur)
						cur = cur.parent
					end
						-- Move the project itself to start
					table.remove(sln.projects, i)
					table.insert(sln.projects, 1, prj)
					break
				end
			end
	end
end




