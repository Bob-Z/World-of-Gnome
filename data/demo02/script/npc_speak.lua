function f (npc,player,keyword)

print_text_debug("npc_speak called")

if keyword == nil then
	speak_send(npc,player,"Coucou")
end

end
