function f (npc,player,keyword)

print_text_debug("npc_speak called")

if keyword == nil then
	network_send_speak(npc,player,"Coucou")
end

end
