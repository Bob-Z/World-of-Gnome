function f (id,map,x, y)

print_text_debug("***** testing ground")
tile_type = map_get_tile_type(map,x,y)

if tile_type == "ground" then
	return 1
end

return 0

end
