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

#include "ChatHistoryBox.h"
#include <memory>

#include "SdlItemCore.h"
#include "textview.h"

static constexpr Uint32 BACKGROUND_COLOR = 0xFFFFFF40;

static constexpr Uint32 TEXT_TIMEOUT_MS = 5000;

/*****************************************************************************/
ChatHistoryBox::ChatHistoryBox(TTF_Font * const font) :
		m_font(font)
{
}

/*****************************************************************************/
ChatHistoryBox::~ChatHistoryBox()
{
}

/*****************************************************************************/
void ChatHistoryBox::setFont(TTF_Font * font)
{
	m_font = font;
}

/*****************************************************************************/
void ChatHistoryBox::compose(ItemArray &itemArray, const int upperBorderHeight, const int lowerBorderHeight)
{
	if (m_font == nullptr)
	{
		return;
	}

	const std::vector<history_entry_t> & history = textview_get_history();
	Uint32 timeStamp = SDL_GetTicks();
	int historyBoxY = lowerBorderHeight;

	// Newest entries are at the back of the vector
	for (auto iter = history.rbegin(); iter != history.rend(); ++iter)
	{
		if (timeStamp > iter->time + TEXT_TIMEOUT_MS)
		{
			return;
		}

		auto itemOwner = std::make_unique<SdlItem>();
		SdlItem *item = itemOwner.get();
		itemArray.push_back(std::move(itemOwner));

		item->setOverlay(true);
		item->setText(iter->text);
		item->setBackGroudColor(BACKGROUND_COLOR);
		item->setFont(m_font);

		int textWidth = 0;
		int textHeight = 0;
		sdl_get_string_size(m_font, iter->text, &textWidth, &textHeight);

		item->setPos(0, historyBoxY - textHeight);
		item->setShape(textWidth, textHeight);
		historyBoxY -= textHeight;
		if (upperBorderHeight > historyBoxY)
		{
			return;
		}
	}
}
