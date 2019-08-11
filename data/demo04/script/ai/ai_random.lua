function f ()

id = player_get_id()

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)

new_tile_x = tile_x + math.random(-1,1)
new_tile_y = tile_y + math.random(-1,1)

character_set_pos(id,map,new_tile_x,new_tile_y)

-- return the time in ms before the next NPC AI action
delay = math.random(100,1000)
return delay

end
