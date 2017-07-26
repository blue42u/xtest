require 'xtask'

local function nap() end

local sn,wn = tonumber(arg[1]:sub(3)), tonumber(arg[2]:sub(3))
local t = {}
for i=1,sn do t[i] = defer(nap) end
defer(nap, table.unpack(t))
