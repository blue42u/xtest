#!/usr/bin/env lua

local d = {}

print('Cores Norm     Time     HotTime    HotProp')

local firsttime

local totaltime = 0
local function outdata()
	if d.cores then
		totaltime = totaltime + d.real
		d.real = d.real / d.count
		d.user = d.user / d.count
		d.sys = d.sys / d.count
		if not firsttime then firsttime = d.real end
		print(('%-5d %-8.2f %-8.2f %-10.2f %07.4f'):format(
			d.cores, d.real / firsttime, d.real, d.user, 100*d.user/(d.user+d.sys)
		))
	end
end

for l in io.lines() do
	if l:match('^Cores') then
		local cores = tonumber(l:match('^Cores (%d+)'))
		if d.cores == cores then
			d.count = d.count + 1
		else
			outdata()
			d = {cores=cores, count=1}
		end
	elseif l:match('^Failed') then
		d = {}
	elseif #l > 3 then
		local k,v = l:match('^(%a+) ([%d\\.]+)$')
		d[k] = (d[k] or 0) + tonumber(v)
	end
end

outdata()

io.stderr:write('Total time used: '..totaltime..'\n')
