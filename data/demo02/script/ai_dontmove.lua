function f ()

id = player_get_id()

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)

character_set_pos(id,map,tile_x,tile_y)

-- return the time in ms before the next NPC AI action
return 1000000

end
