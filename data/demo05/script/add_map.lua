function f (map_name,map_x,map_y)

player_id = player_get_id()
map_w = character_get_map_w(player_id)
map_h = character_get_map_h(player_id)

new_map = map_new(map_name,map_w,map_h,32,32,"b.gif","b")

if map_y == "0" then
	for i=0,map_w-1 do
		map_set_tile(map_name,"s.gif",i,0,0)
		map_set_tile_type(map_name,"s",i,0)

		map_set_tile(map_name,"g.gif",i,1,0)
		map_set_tile_type(map_name,"g",i,1)
	end
end

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
