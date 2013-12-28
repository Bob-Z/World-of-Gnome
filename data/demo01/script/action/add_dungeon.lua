id = player_get_id();
x = character_get_selected_map_tile_x(id);
y = character_get_selected_map_tile_y(id);
if x == -1 or y == -1 then
	print_text_id(id, "You must choose a tile")
	return
end

map = character_get_map(id);
-- test to know where to move back the player
cx=x-1;
cy=y;
tile_type = map_get_tile_type(map,cx,cy);
if tile_type ~= "ground" then
	cx=x+1;
	cy=y;
	tile_type = map_get_tile_type(map,cx,cy);
	if tile_type ~= "ground" then
		cx=x;
		cy=y-1;
		tile_type = map_get_tile_type(map,cx,cy);
		if tile_type ~= "ground" then
			cx=x;
			cy=y+1;
			tile_type = map_get_tile_type(map,cx,cy);
			if tile_type ~= "ground" then
				print_text_id(id, "No space left for going back from this dungeon")
				return
			end
		end
	end
end

new_map = map_new(32,32,64,64,"dungeon_wall");
tile = "dungeon_up";
map_set_tile(new_map,tile,16,15);
tile = "dungeon_ground";
map_set_tile(new_map,tile,16,16);
	
event = map_add_event(new_map,"goto.lua",16,15)
map_add_event_param(new_map,event,map);
map_add_event_param(new_map,event,cx);
map_add_event_param(new_map,event,cy);

tile = "dungeon_down";
map_set_tile(map,tile,x,y);
event = map_add_event(map,"goto.lua",x,y)
map_add_event_param(map,event,new_map);
map_add_event_param(map,event,16);
map_add_event_param(map,event,16);

