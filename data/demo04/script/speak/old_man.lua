function f (npc,player,keyword)

if keyword == nil then
	keyword = "start"
end

player_name = character_get_name(player)

if keyword == "start" then
	text = string.format("Hello %s. What do you need ?",player_name)
	speak_send(npc,"portrait/old_man.gif",player,text,
		"item/sword.gif","Buy a Sword (200 gold)","sword",
		"item/shield.gif","Buy a Shield","shield (150 gold)",
		"item/bag.gif","Buy a Bag (25 gold)","bag"
	)
end

if keyword == "sword" then
	price = 200
	gold = inventory_get_by_name(player,"gold")
	gold_qty = resource_get_quantity(gold)
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		item = item_create_from_template("sword")
		inventory_add(player,item)
		text = string.format("You got a new sword. What do you need ?",player_name)
		portrait = "portrait/old_man_happy.gif"
	else
		text = string.format("You don't have enought gold, get out !",player_name)
		portrait = "portrait/old_man_angry.gif"
	end
	speak_send(npc,portrait,player,text,
		"item/sword.gif","Buy a Sword (200 gold)","sword",
		"item/shield.gif","Buy a Shield (150 gold)","shield",
		"item/bag.gif","Buy a Bag (25 gold)","bag"
	)
end

if keyword == "shield" then
	price = 150
	gold = inventory_get_by_name(player,"gold")
	gold_qty = resource_get_quantity(gold)
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		item = item_create_from_template("shield")
		inventory_add(player,item)
		text = string.format("You got a new shield. What do you need ?",player_name)
		portrait = "portrait/old_man_happy.gif"
	else
		text = string.format("You don't have enought gold, get out !",player_name)
		portrait = "portrait/old_man_angry.gif"
	end
	speak_send(npc,portrait,player,text,
		"item/sword.gif","Buy a Sword (200 gold)","sword",
		"item/shield.gif","Buy a Shield (150 gold)","shield",
		"item/bag.gif","Buy a Bag (25 gold)","bag"
	)
end

if keyword == "bag" then
	prise = 25
	gold = inventory_get_by_name(player,"gold")
	gold_qty = resource_get_quantity(gold)
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		item = item_create_from_template("bag")
		inventory_add(player,item)
		text = string.format("You got a new bag. What do you need ?",player_name)
		portrait = "portrait/old_man_happy.gif"
	else
		text = string.format("You don't have enought gold, get out !",player_name)
		portrait = "portrait/old_man_angry.gif"
	end
	speak_send(npc,portrait,player,text,
		"item/sword.gif","Buy a Sword (200 gold)","sword",
		"item/shield.gif","Buy a Shield (150 gold)","shield",
		"item/bag.gif","Buy a Bag (25 gold)","bag"
	)
end

end
