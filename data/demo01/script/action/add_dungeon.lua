function f ()

id = player_get_id()
x = character_get_selected_map_tile_x(id)
y = character_get_selected_map_tile_y(id)
if x == -1 or y == -1 then
	print_text_id(id, "You must choose a tile")
	return
end

map = character_get_map(id)
-- test to know where to move back the player
cx=x-1
cy=y
tile_type = map_get_tile_type(map,0,cx,cy)
if tile_type ~= "ground" then
	cx=x+1
	cy=y
	tile_type = map_get_tile_type(map,0,cx,cy)
	if tile_type ~= "ground" then
		cx=x
		cy=y-1
		tile_type = map_get_tile_type(map,0,cx,cy)
		if tile_type ~= "ground" then
			cx=x
			cy=y+1
			tile_type = map_get_tile_type(map,0,cx,cy)
			if tile_type ~= "ground" then
				print_text_id(id, "No space left for going back from this dungeon")
				return
			end
		end
	end
end

new_map = map_new("",0,32,32,64,64,"tile/dungeon_wall.gif","wall")
map_set_tile(new_map,0,"tile/dungeon_stairs_up.gif",16,15)
map_set_tile_type(new_map,0,"stair",16,15)
map_set_tile(new_map,0,"tile/dungeon.gif",16,16)
map_set_tile_type(new_map,0,"ground",16,16)
	
event = map_add_event(new_map,0,"goto.lua",16,15)
map_add_event_param(new_map,0,event,map)
map_add_event_param(new_map,0,event,cx)
map_add_event_param(new_map,0,event,cy)

map_set_tile(map,0,"tile/dungeon_stairs_down.gif",x,y)
map_set_tile_type(map,0,"stair",x,y)
event = map_add_event(map,0,"goto.lua",x,y)
map_add_event_param(map,0,event,new_map)
map_add_event_param(map,0,event,16)
map_add_event_param(map,0,event,16)

end
