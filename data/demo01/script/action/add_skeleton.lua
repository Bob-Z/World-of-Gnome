function f ()

player = player_get_id()

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(id, "You must choose a tile")
        return
end
map = character_get_map(player)

id = character_create_from_template("skeleton",map,0,x,y)
if id == nil then
        print_text_id(player, "Cannot create skeleton here")
        return
end

character_set_npc(id,1)

end
