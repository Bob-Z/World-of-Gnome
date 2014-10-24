function f (map_name,map_x,map_y)

new_map = map_new(map_name,32,32,32,32,"b.gif","b")

if map_y == "0" then
	for i=0,31 do
		map_set_tile(map_name,"s.gif",i,0,0)
		map_set_tile_type(map_name,"s",i,0)

		map_set_tile(map_name,"g.gif",i,1,0)
		map_set_tile_type(map_name,"g",i,1)
	end
end

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
