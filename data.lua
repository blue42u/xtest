#!/usr/bin/env lua

local d = {}
local tcnt = tonumber(arg[1])

local totaltime, points, cnt = 0, 0, 0
local sts = {total=0, rcnt=0, min=math.huge, max=0}
print('Threads TPS CPU')
local function outdata()
	if d.cores then
		sts.rcnt = sts.rcnt + d.count
		sts.total = sts.total + d.real
		sts.min = math.min(sts.min, d.real / d.count)
		sts.max = math.max(sts.max, d.real / d.count)

		d.real = d.real / d.count
		d.user = d.user / d.count
		d.sys = d.sys / d.count

		print(('%d %f %f'):format(d.cores,
			tcnt / d.real,
			(d.user+d.sys)/d.real
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
io.stderr:write(string.format('%d in %.2fs, %.2f/%.2f/%.2f',
	sts.rcnt, sts.total, sts.min, sts.total/sts.rcnt, sts.max))
