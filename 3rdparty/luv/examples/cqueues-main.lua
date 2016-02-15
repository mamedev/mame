--[[
Demonstrates using luv with a cqueues mainloop
]]

local cqueues = require "cqueues"
local uv = require "luv"

local cq = cqueues.new()

cq:wrap(function()
	while cqueues.poll({
		pollfd = uv.backend_fd();
		timeout = uv.backend_timeout() / 1000;
		events = "r";
	}) do
		uv.run("nowait")
	end
end)

cq:wrap(function()
	while true do
		cqueues.sleep(1)
		print("HELLO FROM CQUEUES")
	end
end)

uv.new_timer():start(1000, 1000, function()
	print("HELLO FROM LUV")
end)

assert(cq:loop())
