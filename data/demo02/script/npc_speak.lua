function f (npc,player,keyword)

if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	speak_send(npc,player,"Coucou")
end

end
