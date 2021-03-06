function f (lower_layer, lower_image_name, higher_layer, higher_image_name)

player = player_get_id()

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(player, "You must choose a tile")
        return
end

map = character_get_map(player)

-- Get coordinate in pixel
px = tile_get_x(map,1,x,y)
py = tile_get_y(map,1,x,y)

map_add_scenery(map,lower_layer,px,py,lower_image_name)
map_add_scenery(map,higher_layer,px,py,higher_image_name)

end

