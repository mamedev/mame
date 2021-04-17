-- license:MIT
-- copyright-holders:Gavin Kistner

local exports = {}
exports.name = "SLAXML"
exports.version = "0.8"
exports.homepage = "http://github.com/Phrogz/SLAXML"
exports.description = "Lua SLAX XML parser"
exports.tags = {"xml"}
exports.license = "MIT"
exports.author = {
  name = "Gavin Kistner",
}

local SLAXML = exports

--[=====================================================================[
v0.8 Copyright © 2013-2018 Gavin Kistner <!@phrogz.net>; MIT Licensed
See http://github.com/Phrogz/SLAXML for details.
--]=====================================================================]
SLAXML.VERSION = "0.8"
SLAXML._call = {
	pi = function(target,content)
		print(string.format("<?%s %s?>",target,content))
	end,
	comment = function(content)
		print(string.format("<!-- %s -->",content))
	end,
	startElement = function(name,nsURI,nsPrefix)
		io.write("<")
		if nsPrefix then io.write(nsPrefix,":") end
		io.write(name)
		if nsURI    then io.write(" (ns='",nsURI,"')") end
		print(">")
	end,
	attribute = function(name,value,nsURI,nsPrefix)
		io.write('  ')
		if nsPrefix then io.write(nsPrefix,":") end
		io.write(name,'=',string.format('%q',value))
		if nsURI    then io.write(" (ns='",nsURI,"')") end
		io.write("\n")
	end,
	text = function(text,cdata)
		print(string.format("  %s: %q",cdata and 'cdata' or 'text',text))
	end,
	closeElement = function(name,nsURI,nsPrefix)
		io.write("</")
		if nsPrefix then io.write(nsPrefix,":") end
		print(name..">")
	end,
}

function SLAXML:parser(callbacks)
	return { _call=callbacks or self._call, parse=SLAXML.parse }
end

function SLAXML:parse(xml,options)
	if not options then options = { stripWhitespace=false } end

	-- Cache references for maximum speed
	local find, sub, gsub, char, push, pop, concat = string.find, string.sub, string.gsub, string.char, table.insert, table.remove, table.concat
	local first, last, match1, match2, match3, pos2, nsURI
	local unpack = unpack or table.unpack
	local pos = 1
	local state = "text"
	local textStart = 1
	local currentElement={}
	local currentAttributes={}
	local currentAttributeCt -- manually track length since the table is re-used
	local nsStack = {}
	local anyElement = false

	local utf8markers = { {0x7FF,192}, {0xFFFF,224}, {0x1FFFFF,240} }
	local function utf8(decimal) -- convert unicode code point to utf-8 encoded character string
		if decimal<128 then return char(decimal) end
		local charbytes = {}
		for bytes,vals in ipairs(utf8markers) do
			if decimal<=vals[1] then
				for b=bytes+1,2,-1 do
					local mod = decimal%64
					decimal = (decimal-mod)/64
					charbytes[b] = char(128+mod)
				end
				charbytes[1] = char(vals[2]+decimal)
				return concat(charbytes)
			end
		end
	end
	local entityMap  = { ["lt"]="<", ["gt"]=">", ["amp"]="&", ["quot"]='"', ["apos"]="'" }
	local entitySwap = function(orig,n,s) return entityMap[s] or n=="#" and utf8(tonumber('0'..s)) or orig end
	local function unescape(str) return gsub( str, '(&(#?)([%d%a]+);)', entitySwap ) end

	local function finishText()
		if first>textStart and self._call.text then
			local text = sub(xml,textStart,first-1)
			if options.stripWhitespace then
				text = gsub(text,'^%s+','')
				text = gsub(text,'%s+$','')
				if #text==0 then text=nil end
			end
			if text then self._call.text(unescape(text),false) end
		end
	end

	local function findPI()
		first, last, match1, match2 = find( xml, '^<%?([:%a_][:%w_.-]*) ?(.-)%?>', pos )
		if first then
			finishText()
			if self._call.pi then self._call.pi(match1,match2) end
			pos = last+1
			textStart = pos
			return true
		end
	end

	local function findComment()
		first, last, match1 = find( xml, '^<!%-%-(.-)%-%->', pos )
		if first then
			finishText()
			if self._call.comment then self._call.comment(match1) end
			pos = last+1
			textStart = pos
			return true
		end
	end

	local function nsForPrefix(prefix)
		if prefix=='xml' then return 'http://www.w3.org/XML/1998/namespace' end -- http://www.w3.org/TR/xml-names/#ns-decl
		for i=#nsStack,1,-1 do if nsStack[i][prefix] then return nsStack[i][prefix] end end
		error(("Cannot find namespace for prefix %s"):format(prefix))
	end

	local function startElement()
		anyElement = true
		first, last, match1 = find( xml, '^<([%a_][%w_.-]*)', pos )
		if first then
			currentElement[2] = nil -- reset the nsURI, since this table is re-used
			currentElement[3] = nil -- reset the nsPrefix, since this table is re-used
			finishText()
			pos = last+1
			first,last,match2 = find(xml, '^:([%a_][%w_.-]*)', pos )
			if first then
				currentElement[1] = match2
				currentElement[3] = match1 -- Save the prefix for later resolution
				match1 = match2
				pos = last+1
			else
				currentElement[1] = match1
				for i=#nsStack,1,-1 do if nsStack[i]['!'] then currentElement[2] = nsStack[i]['!']; break end end
			end
			currentAttributeCt = 0
			push(nsStack,{})
			return true
		end
	end

	local function findAttribute()
		first, last, match1 = find( xml, '^%s+([:%a_][:%w_.-]*)%s*=%s*', pos )
		if first then
			pos2 = last+1
			first, last, match2 = find( xml, '^"([^<"]*)"', pos2 ) -- FIXME: disallow non-entity ampersands
			if first then
				pos = last+1
				match2 = unescape(match2)
			else
				first, last, match2 = find( xml, "^'([^<']*)'", pos2 ) -- FIXME: disallow non-entity ampersands
				if first then
					pos = last+1
					match2 = unescape(match2)
				end
			end
		end
		if match1 and match2 then
			local currentAttribute = {match1,match2}
			local prefix,name = string.match(match1,'^([^:]+):([^:]+)$')
			if prefix then
				if prefix=='xmlns' then
					nsStack[#nsStack][name] = match2
				else
					currentAttribute[1] = name
					currentAttribute[4] = prefix
				end
			else
				if match1=='xmlns' then
					nsStack[#nsStack]['!'] = match2
					currentElement[2]      = match2
				end
			end
			currentAttributeCt = currentAttributeCt + 1
			currentAttributes[currentAttributeCt] = currentAttribute
			return true
		end
	end

	local function findCDATA()
		first, last, match1 = find( xml, '^<!%[CDATA%[(.-)%]%]>', pos )
		if first then
			finishText()
			if self._call.text then self._call.text(match1,true) end
			pos = last+1
			textStart = pos
			return true
		end
	end

	local function closeElement()
		first, last, match1 = find( xml, '^%s*(/?)>', pos )
		if first then
			state = "text"
			pos = last+1
			textStart = pos

			-- Resolve namespace prefixes AFTER all new/redefined prefixes have been parsed
			if currentElement[3] then currentElement[2] = nsForPrefix(currentElement[3])    end
			if self._call.startElement then self._call.startElement(unpack(currentElement)) end
			if self._call.attribute then
				for i=1,currentAttributeCt do
					if currentAttributes[i][4] then currentAttributes[i][3] = nsForPrefix(currentAttributes[i][4]) end
					self._call.attribute(unpack(currentAttributes[i]))
				end
			end

			if match1=="/" then
				pop(nsStack)
				if self._call.closeElement then self._call.closeElement(unpack(currentElement)) end
			end
			return true
		end
	end

	local function findElementClose()
		first, last, match1, match2 = find( xml, '^</([%a_][%w_.-]*)%s*>', pos )
		if first then
			nsURI = nil
			for i=#nsStack,1,-1 do if nsStack[i]['!'] then nsURI = nsStack[i]['!']; break end end
		else
			first, last, match2, match1 = find( xml, '^</([%a_][%w_.-]*):([%a_][%w_.-]*)%s*>', pos )
			if first then nsURI = nsForPrefix(match2) end
		end
		if first then
			finishText()
			if self._call.closeElement then self._call.closeElement(match1,nsURI) end
			pos = last+1
			textStart = pos
			pop(nsStack)
			return true
		end
	end

	while pos<#xml do
		if state=="text" then
			if not (findPI() or findComment() or findCDATA() or findElementClose()) then
				if startElement() then
					state = "attributes"
				else
					first, last = find( xml, '^[^<]+', pos )
					pos = (first and last or pos) + 1
				end
			end
		elseif state=="attributes" then
			if not findAttribute() then
				if not closeElement() then
					error("Was in an element and couldn't find attributes or the close.")
				end
			end
		end
	end

	if not anyElement then error("Parsing did not discover any elements") end
	if #nsStack > 0 then error("Parsing ended with unclosed elements") end
end

-- Optional parser that creates a flat DOM from parsing
function SLAXML:dom(xml,opts)
	if not opts then opts={} end
	local rich = not opts.simple
	local push, pop = table.insert, table.remove
	local doc = {type="document", name="#doc", kids={}}
	local current,stack = doc, {doc}
	local builder = SLAXML:parser{
		startElement = function(name,nsURI,nsPrefix)
			local el = { type="element", name=name, kids={}, el=rich and {} or nil, attr={}, nsURI=nsURI, nsPrefix=nsPrefix, parent=rich and current or nil }
			if current==doc then
				if doc.root then error(("Encountered element '%s' when the document already has a root '%s' element"):format(name,doc.root.name)) end
				doc.root = rich and el or nil
			end
			push(current.kids,el)
			if current.el then push(current.el,el) end
			current = el
			push(stack,el)
		end,
		attribute = function(name,value,nsURI,nsPrefix)
			if not current or current.type~="element" then error(("Encountered an attribute %s=%s but I wasn't inside an element"):format(name,value)) end
			local attr = {type='attribute',name=name,nsURI=nsURI,nsPrefix=nsPrefix,value=value,parent=rich and current or nil}
			if rich then current.attr[name] = value end
			push(current.attr,attr)
		end,
		closeElement = function(name)
			if current.name~=name or current.type~="element" then error(("Received a close element notification for '%s' but was inside a '%s' %s"):format(name,current.name,current.type)) end
			pop(stack)
			current = stack[#stack]
		end,
		text = function(value,cdata)
			-- documents may only have text node children that are whitespace: https://www.w3.org/TR/xml/#NT-Misc
			if current.type=='document' and not value:find('^%s+$') then error(("Document has non-whitespace text at root: '%s'"):format(value:gsub('[\r\n\t]',{['\r']='\\r', ['\n']='\\n', ['\t']='\\t'}))) end
			push(current.kids,{type='text',name='#text',cdata=cdata and true or nil,value=value,parent=rich and current or nil})
		end,
		comment = function(value)
			push(current.kids,{type='comment',name='#comment',value=value,parent=rich and current or nil})
		end,
		pi = function(name,value)
			push(current.kids,{type='pi',name=name,value=value,parent=rich and current or nil})
		end
	}
	builder:parse(xml,opts)
	return doc
end

local escmap = {["<"]="&lt;", [">"]="&gt;", ["&"]="&amp;", ['"']="&quot;", ["'"]="&apos;"}
local function esc(s) return s:gsub('[<>&"]', escmap) end

-- opts.indent: number of spaces, or string
function SLAXML:xml(n,opts)
	opts = opts or {}
	local out = {}
	local tab = opts.indent and (type(opts.indent)=="number" and string.rep(" ",opts.indent) or opts.indent) or ""
	local ser = {}
	local omit = {}
	if opts.omit then for _,s in ipairs(opts.omit) do omit[s]=true end end

	function ser.document(n)
		for _,kid in ipairs(n.kids) do
			if ser[kid.type] then ser[kid.type](kid,0) end
		end
	end

	function ser.pi(n,depth)
		depth = depth or 0
		table.insert(out, tab:rep(depth)..'<?'..n.name..' '..n.value..'?>')
	end

	function ser.element(n,depth)
		if n.nsURI and omit[n.nsURI] then return end
		depth = depth or 0
		local indent = tab:rep(depth)
		local name = n.nsPrefix and n.nsPrefix..':'..n.name or n.name
		local result = indent..'<'..name
		if n.attr and n.attr[1] then
			local sorted = n.attr
			if opts.sort then
				sorted = {}
				for i,a in ipairs(n.attr) do sorted[i]=a end
				table.sort(sorted,function(a,b)
					if a.nsPrefix and b.nsPrefix then
						return a.nsPrefix==b.nsPrefix and a.name<b.name or a.nsPrefix<b.nsPrefix
					elseif not (a.nsPrefix or b.nsPrefix) then
						return a.name<b.name
					elseif b.nsPrefix then
						return true
					else
						return false
					end
				end)
			end

			local attrs = {}
			for _,a in ipairs(sorted) do
				if (not a.nsURI or not omit[a.nsURI]) and not (omit[a.value] and a.name:find('^xmlns:')) then
					attrs[#attrs+1] = ' '..(a.nsPrefix and (a.nsPrefix..':') or '')..a.name..'="'..esc(a.value)..'"'
				end
			end
			result = result..table.concat(attrs,'')
		end
		result = result .. (n.kids and n.kids[1] and '>' or '/>')
		table.insert(out, result)
		if n.kids and n.kids[1] then
			for _,kid in ipairs(n.kids) do
				if ser[kid.type] then ser[kid.type](kid,depth+1) end
			end
			table.insert(out, indent..'</'..name..'>')
		end
	end

	function ser.text(n,depth)
		if n.cdata then
			table.insert(out, tab:rep(depth)..'<![CDATA['..n.value..']]>')
		else
			table.insert(out, tab:rep(depth)..esc(n.value))
		end
	end

	function ser.comment(n,depth)
		table.insert(out, tab:rep(depth)..'<!--'..n.value..'-->')
	end

	ser[n.type](n,0)

	return table.concat(out, opts.indent and '\n' or '')
end

return SLAXML
