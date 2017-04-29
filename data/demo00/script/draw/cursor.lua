function f ()

-- id = player_get_id()
id = context_get_id()

item_set_x(_G.current_X[id])
item_set_y(_G.current_Y[id])

item_set_anim("cursor.png");

return 0

end

