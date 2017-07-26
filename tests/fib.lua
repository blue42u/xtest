require 'xtask'

local function add(a, b)
	return a + b
end

local function f(n)
	return n <= 1 and n or defer(add, defer(f,n-1), defer(f,n-2))
end

local fn = tonumber(arg[1]:sub(3))
local wn = tonumber(arg[2]:sub(3))
print(defer(f, fn)({workers=wn}))
