id = player_get_id();
x = character_get_selected_map_tile_x(id);
y = character_get_selected_map_tile_y(id);
if x == -1 or y == -1 then
	print_text_id(id, "You must choose a tile")
	return
end

new_map = map_new(32,32,64,64,"dungeon_wall");
map_set_tile(new_map,"dungeon_up",16,16);

map = character_get_map(id);
tile = "dungeon_down";
map_set_tile(map,tile,x,y);
