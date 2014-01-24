player = player_get_id()
id = character_create_from_template("dolphin")
--need instantiation early to allow others modifications
character_set_npc(id,1)

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(id, "You must choose a tile")
        return
end
map = character_get_map(player)

character_set_pos(id,map,x,y)
