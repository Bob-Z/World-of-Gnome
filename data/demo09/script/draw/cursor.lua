function f ()

-- id = player_get_id()
id = context_get_id()

item_set_anim("red_square.png");

-- center
item_w = item_get_w()
item_h = item_get_h()


cursor_pos_x = _G.current_X[id] + ((_G.current_W[id] - item_w) / 2)
cursor_pos_y = _G.current_Y[id] + ((_G.current_H[id] - item_h) / 2)

 text = string.format("id = %s, item_w = %d, item_h = %d, _G.current_X[id] = %d, _G.current_W[id] = %d, cursor_pos_x = %d, _G.current_Y[id]= %d, _G.current_H[id] = %d, cursor_pos_y = %d",id, item_w,item_h,_G.current_X[id],_G.current_W[id],cursor_pos_x,_G.current_Y[id],_G.current_H[id],cursor_pos_y)
  print_text_debug(text)

item_set_x(cursor_pos_x)
item_set_y(cursor_pos_y)


return 0

end

