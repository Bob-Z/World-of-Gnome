function f (id,map,x, y)

tile_type = map_get_tile_type(map,x,y)

if tile_type == "ground" then
	return 1
end

return 0

end
