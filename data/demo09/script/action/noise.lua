function f (sound_file)

player = player_get_id()
map = character_get_map(player)
map_effect(map,"effect/noise.lua",sound_file)

end
