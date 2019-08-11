function f (move_x,move_y)

id = player_get_id()

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_tile_x = tile_x + move_x
new_tile_y = tile_y + move_y

character_set_pos(id,map,new_tile_x,new_tile_y)

end
