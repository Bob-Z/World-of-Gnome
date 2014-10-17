function f ()

id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)

new_pos_x = pos_x + math.random(-1,1)
new_pos_y = pos_y + math.random(-1,1)

character_set_pos(id,map,new_pos_x,new_pos_y)

-- return the time in ms before the next NPC AI action
delay = math.random(100,1000)
return delay

end
