function f (map_name,map_x,map_y)

new_map = map_new(map_name,32,32,32,32,"b.gif","b")

for i=0,31 do
	event = map_add_event(map_name,"goto.lua",0,i)
	map_add_event_param(new_map,event,"-1")
	map_add_event_param(new_map,event,"0")

	event = map_add_event(map_name,"goto.lua",31,i)
	map_add_event_param(new_map,event,"1")
	map_add_event_param(new_map,event,"0")

	event = map_add_event(map_name,"goto.lua",i,0)
	map_add_event_param(new_map,event,"0")
	map_add_event_param(new_map,event,"-1")

	event = map_add_event(map_name,"goto.lua",i,31)
	map_add_event_param(new_map,event,"0")
	map_add_event_param(new_map,event,"1")
end

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
