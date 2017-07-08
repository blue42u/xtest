#!/usr/bin/env lua

-- Usage: data.lua <taskcnt>

local tasks = tonumber(arg[1])
print('Threads TPS CPU')

local d = {}
local err
for l in io.lines() do
	local th,real,usr,sys = l:match('^(%g+) (%g+) (%g+) (%g+) TIME$')
	if th then
		th,real,usr,sys = tonumber(th),tonumber(real),tonumber(usr),tonumber(sys)
		if real > 0 then
			if not d[th] then d[th] = {{r=real, u=usr, s=sys}}
			else table.insert(d[th], {r=real, u=usr, s=sys}) end
		end
	elseif l == 'ERR' then
		err = true
	else
		io.stderr:write(l..'\n')
	end
end

local cnt, sum, min, max = 0,0,math.huge,-math.huge
for t,rs in pairs(d) do for _,r in ipairs(rs) do
	cnt,sum,min,max = cnt+1, sum+r.r, math.min(min,r.r), math.max(max,r.r)
	print(('%d %f %f'):format(t, tasks/r.r, (r.u+r.s)/r.r ))
end end

io.stderr:write(('%d in %.2fs, %.2f/%.2f/%.2f\n'):format(
	cnt, sum, min, sum/cnt, max))

if err then os.exit(1, true) end
