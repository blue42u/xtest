#!/usr/bin/env lua

local d

print('Cores Time     HotTime    HotProp')

local function outdata()
	print(('%-5d %-8.2f %-10.2f %07.4f'):format(
		d.cores, d.real, d.user, 100*d.sys/(d.user+d.sys)
	))
end

for l in io.lines() do
	if l:match('^Cores') then
		if d then outdata() end
		d = {}
		d.cores = tonumber(l:match('^Cores (%d+)'))
	elseif l:match('^Failed') then
		d = nil
	elseif #l > 3 then
		local k,v = l:match('^(%a+) ([%d\\.]+)$')
		d[k] = tonumber(v)
	end
end

if d then outdata() end

