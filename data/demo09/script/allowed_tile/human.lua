function f (id,map,x, y)

player_layer = 15
tile_type = map_get_tile_type(map,player_layer,x,y)

-- wall
if tile_type == "w" then
	return 0
end

return 1

end
