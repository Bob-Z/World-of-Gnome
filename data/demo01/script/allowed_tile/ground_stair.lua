function f (id,map,x, y)

text = string.format("****** allowing ground and stairs for %s on %s at %s %s",id,map,x,y)
print_text_debug(text)

tile_type = map_get_tile_type(map,0,x,y)
text = string.format("****** tile type : %s",tile_type)
print_text_debug(text)
if tile_type == "ground" then
print_text_debug("return 1")
	return 1
end
if tile_type == "stair" then
print_text_debug("return 1")
	return 1
end

print_text_debug("return 0")
return 0

end
