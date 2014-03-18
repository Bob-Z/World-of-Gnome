function f ()

player = player_get_id()

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(id, "You must choose a tile")
        return
end
map = character_get_map(player)

r = math.random(1,3)
if r == 1 then
        id = character_create_from_template("flower1",map,x,y)
elseif r == 2 then
        id = character_create_from_template("flower2",map,x,y)
else
        id = character_create_from_template("flower3",map,x,y)
end

if id == nil then
	return
end

character_set_npc(id,1)

end
