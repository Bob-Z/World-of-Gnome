function f (tile_type)

id = player_get_id()
x = character_get_selected_map_tile_x(id)
y = character_get_selected_map_tile_y(id)
if x == -1 or y == -1 then
	print_text_id(id, "You must choose a tile")
	return
end
map = character_get_map(id)
map_set_tile_type(map,0,tile_type,x,y)

end
