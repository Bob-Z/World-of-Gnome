/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2019 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "ChatEditBox.h"

#include "sdl.h"
#include "SdlItem.h"

static constexpr Uint32 BACKGROUND_COLOR = 0xFFFFFF40;

static constexpr int CHAT_BOX_EDIT_WIDTH_MIN = 100;

/*****************************************************************************/
ChatEditBox::ChatEditBox(TTF_Font * const font, const std::function<void(const std::string)> editCallBack) :
		m_font(font), m_editCallBack(editCallBack)
{
}

/*****************************************************************************/
ChatEditBox::~ChatEditBox()
{
}

/*****************************************************************************/
int ChatEditBox::compose(std::vector<SdlItem *> & itemArray, const int upperBorderHeight, const int lowerBorderHeight)
{
	SdlItem * item = new SdlItem;
	itemArray.push_back(item);

	item->setOverlay(true);
	item->setBackGroudColor(BACKGROUND_COLOR);
	item->setFont(m_font);
	item->setEditable(true);
	item->setEditCb(m_editCallBack);

	int textWidth = 0;
	int textHeight = 0;
	sdl_get_string_size(item->getFont(), item->getText(), &textWidth, &textHeight);

	int chatBoxWidth = 0;
	chatBoxWidth = textWidth;
	if (textWidth < CHAT_BOX_EDIT_WIDTH_MIN)
	{
		chatBoxWidth = CHAT_BOX_EDIT_WIDTH_MIN;
	}

	int chatBoxHeight = 0;
	chatBoxHeight = textHeight;
	if (chatBoxHeight < TTF_FontHeight(m_font))
	{
		chatBoxHeight = TTF_FontHeight(m_font);
	}

	int screenWidth = 0;
	int screenHeight = 0;
	sdl_get_output_size(&screenWidth, &screenHeight);
	int chatBoxY = screenHeight - lowerBorderHeight - chatBoxHeight;

	item->setPos(0, chatBoxY);
	item->setShape(chatBoxWidth, chatBoxHeight);

	//if (upperBorderHeight > chatBoxY)
	//{
//		return chatBoxHeight;
	//}

	return chatBoxHeight;
}
