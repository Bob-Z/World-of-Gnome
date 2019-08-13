function f ()

-- id = player_get_id()
id = context_get_id()

item_set_anim("red_square.png");

text = string.format("id = %s",id)
print_text_debug(text)

--text = string.format("id = %s",id)
--print_text_debug(text)
--text = string.format("item_w = %d",item_w)
--print_text_debug(text)
--text = string.format("item_h = %d",item_h)
--print_text_debug(text)
--text = string.format("_G.current_X[%s] = %d",id, _G.current_X[id])
--print_text_debug(text)
--text = string.format("_G.current_W[%s] = %d",id, _G.current_W[id])
--print_text_debug(text)
--text = string.format("_G.current_Y[%s]= %d",id, _G.current_Y[id])
--print_text_debug(text)
--text = string.format("_G.current_H[%s] = %d",id, _G.current_H[id])
--print_text_debug(text)

if(_G.current_X[id] == nil) then
	current_px = item_get_px()
else
	current_px = _G.current_X[id]
end

if(_G.current_Y[id] == nil) then
	current_py = item_get_py()
else
	current_py = _G.current_Y[id]
end

if(_G.current_W[id] == nil) then
	current_pw = item_get_w()
else
	current_pw = _G.current_W[id]
end

if(_G.current_H[id] == nil) then
	current_ph = item_get_h()
else
	current_ph = _G.current_H[id]
end

item_pw = item_get_w()
item_ph = item_get_h()

-- center
cursor_px = current_px + ((current_pw - item_pw) / 2)
text = string.format("cursor_px = %d", cursor_px )
print_text_debug(text)
cursor_py = current_py + ((current_ph - item_ph) / 2)
text = string.format("cursor_py = %d", cursor_py )
print_text_debug(text)

item_set_px(cursor_px)
item_set_py(cursor_py)

return 0

end

