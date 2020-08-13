/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2020 carabobz@gmail.com

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

#include "LockGuard.h"
#include "RunningAction.h"

/*****************************************************************************/
RunningAction::RunningAction(Context * context, const std::string & action, const std::vector<std::string> params, int coolDownMs) :
		m_lock(), m_running(true), m_context(context), m_action(action), m_params(params), m_coolDownMs(coolDownMs)
{
}

/*****************************************************************************/
RunningAction::~RunningAction()
{
}

/*****************************************************************************/
void RunningAction::beginAction()
{
	m_lock.lock();
}

/*****************************************************************************/
void RunningAction::endAction()
{
	m_lock.unlock();
}

/*****************************************************************************/
bool RunningAction::isRunning() const
{
	return m_running;
}

/*****************************************************************************/
void RunningAction::stop()
{
	LockGuard guard(m_lock);

	m_running = false;
}

/*****************************************************************************/
Context* RunningAction::getContext() const
{
	return m_context;
}

/*****************************************************************************/
const std::vector<std::string>& RunningAction::getParams() const
{
	return m_params;
}

/*****************************************************************************/
const std::string& RunningAction::getAction() const
{
	return m_action;
}

/*****************************************************************************/
const int RunningAction::getCoolDownMs() const
{
	return m_coolDownMs;
}
