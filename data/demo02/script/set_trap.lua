function f ()

id = player_get_id()

map = character_get_map(id)
x = character_get_x(id)
y = character_get_y(id)
map_add_event(map,"trap.lua",x,y)

end
