////////////////////////////////////////////////
// File:	EventManager.cpp
//
// Author:	Liam Russell (A.K.A. BitSlapper)
//
// Copyright:	2014-2015 Liam Russell
//
// License:	GNU GENERAL PUBLIC LICENSE V3
//////////////////////////////////////////////

#include "EventManager.h"
#include "Event.h"
#include "IListener.h"

namespace Events
{


	EventManager* EventManager::sm_pInstance;

	EventManager::EventManager()
	{
		m_bIsInitialized = false;
		sm_pInstance = NULL;
	}

	EventManager* EventManager::GetInstance()
	{
		if (!sm_pInstance)
		{
			sm_pInstance = new EventManager();
		}

		return sm_pInstance;
	}

	void EventManager::Initialize()
	{
		if (m_bIsInitialized)
		{
			return;
		}

		m_bIsInitialized = true;
	}

	void EventManager::Start()
	{
		if (!m_bIsInitialized)
		{
			return;
		}

		m_thTHREAD = boost::thread(&EventManager::HandleEvents, this);
	}

	void EventManager::Shutdown()
	{
		m_thTHREAD.detach();
		Clear();
		m_bIsInitialized = false;
	}

	void EventManager::AddEvent(Event* pEvent)
	{
		if (!pEvent)
		{
			return;
		}

		m_vecEvents.push_back(pEvent);
	}

	void EventManager::Register(std::string szEventName, IListener* pListener)
	{
		if (!pListener)
		{
			return;
		}

		for (std::multimap<std::string, IListener*>::iterator iter = m_mapListeners.begin(); iter != m_mapListeners.end(); ++iter)
		{
			if ((*iter).first == szEventName && (*iter).second == pListener)
			{
				return;
			}
		}

		m_mapListeners.insert(std::make_pair(szEventName, pListener));
	}

	void EventManager::Deregister(IListener* pListener)
	{
		std::multimap<std::string, IListener*>::iterator iter = m_mapListeners.begin();
		while (iter != m_mapListeners.end())
		{
			if ((*iter).second == pListener)
			{
				delete(iter->second);
				iter->second = NULL;

				iter = m_mapListeners.erase(iter);
				continue;
			}
			++iter;
		}
	}

	void EventManager::Clear()
	{
		ClearEvents();
		ClearListeners();
	}

	void EventManager::ClearEvents()
	{
		for (int index = 0; index < m_vecEvents.size(); ++index)
		{
			Event* pEvent = m_vecEvents[index];
			delete(pEvent);
			pEvent = NULL;
		}

		m_vecEvents.clear();
	}

	void EventManager::ClearListeners()
	{
		for (std::multimap<std::string, IListener*>::iterator iter = m_mapListeners.begin(); iter != m_mapListeners.end(); ++iter)
		{
			IListener* pListener = (*iter).second;

			if (pListener)
			{
				delete(pListener);
				pListener = NULL;
			}
		}

		m_mapListeners.clear();
	}

	void EventManager::HandleEvents()
	{
		//Run this thread at 1 Cycle per 10 Milliseconds.
		Sleep(10);

		while (!m_vecEvents.empty())
		{
			Event* pEvent = m_vecEvents.front();
			std::pair<std::multimap<std::string, IListener*>::iterator, std::multimap<std::string, IListener*>::iterator> listenerRange = m_mapListeners.equal_range(pEvent->GetName());

			for (std::multimap<std::string, IListener*>::iterator iter = listenerRange.first; iter != listenerRange.second; ++iter)
			{
				IListener* pListener = iter->second;
				pListener->HandleEvent(pEvent);
			}

			delete(*m_vecEvents.begin());
			m_vecEvents.pop_back();
		}
	}
}