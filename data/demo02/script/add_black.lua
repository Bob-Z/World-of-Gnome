function f ()

id = player_get_id()
x = character_get_selected_map_tile_x(id)
y = character_get_selected_map_tile_y(id)
if x == -1 or y == -1 then
	print_text_id(id, "You must choose a tile")
	return
end
map = character_get_map(id)
tile = "0"
map_set_tile(map,tile,x,y,0)

end
