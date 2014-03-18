function f (id,map,x, y)

text = string.format("****** allowing sea for %s on %s at %s %s",id,map,x,y)
print_text_debug(text)

tile_type = map_get_tile_type(map,x,y)

if tile_type == "sea" then
	return 1
end

return 0

end
