function f (map_name,map_x,map_y)

player_id = player_get_id()
map_w = character_get_map_w(player_id)
map_h = character_get_map_h(player_id)

new_map = map_new(map_name,0,map_w,map_h,72,72,"tile/gr1.png","gr")

for x=0,map_w-1 do
	for y=0,map_h-1 do
		tile_name = string.format("tile/gr%d.png",math.random(1,8))
		map_set_tile(map_name,0,tile_name,x,y)
	end
end

map_set_offscreen(map_name,0,"offscreen.lua")

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
