id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)

character_set_pos(id,map,pos_x,pos_y)

-- return the time in ms before the next NPC AI action
return 1000000

