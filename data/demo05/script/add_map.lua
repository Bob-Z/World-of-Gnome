function f (map_name,map_x,map_y)

player_id = player_get_id()
map = character_get_map(player_id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_map = map_new(map_name,0,map_w,map_h,32,32,"b.gif","b")

if map_y == "0" then
	for i=0,map_w-1 do
		map_set_tile(map_name,0,"s.gif",i,0)
		map_set_tile_type(map_name,0,"s",i,0)

		map_set_tile(map_name,0,"g.gif",i,1)
		map_set_tile_type(map_name,0,"g",i,1)
	end
end

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
