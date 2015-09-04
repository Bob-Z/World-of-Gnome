function f (creature_type)

player = player_get_id()

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(player, "You must choose a tile")
        return
end
map = character_get_map(player)

id = character_create_from_template(creature_type,map,0,x,y)
if id == nil then
	text = string.format("Cannot create %s here",creature_type)
        print_text_id(player, text)
        return
end

character_set_npc(id,1)

end

