function f (index,name)

id = player_get_id()

filename = string.format("character/%d/WC_S_%s.png",index,name)
character_set_sprite(id,index,filename)

filename = string.format("character/%d/WC_N_%s.png",index,name)
character_set_sprite_dir(id,"N",index,filename)
filename = string.format("character/%d/WC_S_%s.png",index,name)
character_set_sprite_dir(id,"S",index,filename)
filename = string.format("character/%d/WC_W_%s.png",index,name)
character_set_sprite_dir(id,"W",index,filename)
filename = string.format("character/%d/WC_E_%s.png",index,name)
character_set_sprite_dir(id,"E",index,filename)

filename = string.format("character/%d/WC_N_%s.zip",index,name)
character_set_sprite_move(id,"N",index,filename)
filename = string.format("character/%d/WC_S_%s.zip",index,name)
character_set_sprite_move(id,"S",index,filename)
filename = string.format("character/%d/WC_W_%s.zip",index,name)
character_set_sprite_move(id,"W",index,filename)
filename = string.format("character/%d/WC_E_%s.zip",index,name)
character_set_sprite_move(id,"E",index,filename)

character_broadcast(id)

end
