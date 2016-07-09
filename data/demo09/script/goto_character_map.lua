function f (map_index,x, y)

id = player_get_id()

sex = character_attribute_tag_get(id,"sex")

map = string.format("C%d%s",map_index,sex)

character_set_pos(id,map,x,y)

end
