function f ()

-- id = player_get_id()
id = context_get_id()

item_set_anim("cursor3.gif");

-- center
item_w = item_get_w()
item_h = item_get_h()

-- text = string.format("id = %s",id)
-- print_text_debug(text)
-- text = string.format("item_w = %d",item_w)
-- print_text_debug(text)
-- text = string.format("item_h = %d",item_h)
-- print_text_debug(text)
-- text = string.format("_G.current_X[%s] = %d",id, _G.current_X[id])
-- print_text_debug(text)
-- text = string.format("_G.current_W[%s] = %d",id, _G.current_W[id])
-- print_text_debug(text)
-- text = string.format("_G.current_Y[%s]= %d",id, _G.current_Y[id])
-- print_text_debug(text)
-- text = string.format("_G.current_H[%s] = %d",id, _G.current_H[id])
-- print_text_debug(text)

cursor_pos_x = _G.current_X[id] + ((_G.current_W[id] - item_w) / 2)
cursor_pos_y = _G.current_Y[id] + ((_G.current_H[id] - item_h) / 2)

item_set_x(cursor_pos_x)
item_set_y(cursor_pos_y)


return 0

end

