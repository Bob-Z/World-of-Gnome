function f (move_x,move_y)

id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_pos_x = pos_x + move_x
new_pos_y = pos_y + move_y

character_set_pos(id,map,new_pos_x,new_pos_y)

end
