/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include <Handler.h>
#include <Messenger.h>
#include <SupportDefs.h>

#include "interface/GMessage.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <vector>

using std::map;
using std::vector;

namespace Observable {

	namespace Private {
		class ObserverList {
			public:
				ObserverList();
				~ObserverList();

				status_t SendNotices(uint32 what, const BMessage* notice);
				status_t Add(const BHandler* handler, uint32 what);
				status_t Add(const BMessenger& messenger, uint32 what);
				status_t Remove(const BHandler* handler, uint32 what);
				status_t Remove(const BMessenger& messenger, uint32 what);
				bool IsEmpty();

			private:
				typedef map<uint32, vector<const BHandler*> > HandlerObserverMap;
				typedef map<uint32, vector<BMessenger> > MessengerObserverMap;

				void _ValidateHandlers(uint32 what);
				void _SendNotices(uint32 what, BMessage* notice);

				HandlerObserverMap		fHandlerMap;
				MessengerObserverMap	fMessengerMap;
		};


		inline ObserverList::ObserverList()
		{
		}


		inline ObserverList::~ObserverList()
		{
		}


		inline void
		ObserverList::_ValidateHandlers(uint32 what)
		{
			vector<const BHandler*>& handlers = fHandlerMap[what];
			vector<const BHandler*>::iterator iterator = handlers.begin();

			while (iterator != handlers.end()) {
				BMessenger target(*iterator);
				if (!target.IsValid()) {
					iterator++;
					continue;
				}

				Add(target, what);
				iterator = handlers.erase(iterator);
			}
			if (handlers.empty())
				fHandlerMap.erase(what);
		}


		inline void
		ObserverList::_SendNotices(uint32 what, BMessage* notice)
		{
			// first iterate over the list of handlers and try to make valid
			// messengers out of them
			_ValidateHandlers(what);

			// now send it to all messengers we know
			vector<BMessenger>& messengers = fMessengerMap[what];
			vector<BMessenger>::iterator iterator = messengers.begin();

			while (iterator != messengers.end()) {
				if (!(*iterator).IsValid()) {
					iterator = messengers.erase(iterator);
					continue;
				}

				(*iterator).SendMessage(notice);
				iterator++;
			}
			if (messengers.empty())
				fMessengerMap.erase(what);
		}


		inline status_t
		ObserverList::SendNotices(uint32 what, const BMessage* notice)
		{
			// printf("ObserverList::SendNotices(uint32 what, const BMessage* notice)\n");
			BMessage* copy = NULL;
			if (notice != NULL) {
				copy = new BMessage(*notice);
				copy->what = B_OBSERVER_NOTICE_CHANGE;
				copy->AddInt32(B_OBSERVE_ORIGINAL_WHAT, notice->what);
			} else
				copy = new BMessage(B_OBSERVER_NOTICE_CHANGE);

			copy->AddInt32(B_OBSERVE_WHAT_CHANGE, what);

			_SendNotices(what, copy);
			_SendNotices(B_OBSERVER_OBSERVE_ALL, copy);

			delete copy;

			return B_OK;
		}


		inline status_t
		ObserverList::Add(const BHandler* handler, uint32 what)
		{
			if (handler == NULL)
				return B_BAD_HANDLER;

			// if this handler already represents a valid target, add its messenger
			BMessenger target(handler);
			if (target.IsValid())
				return Add(target, what);

			vector<const BHandler*> &handlers = fHandlerMap[what];

			vector<const BHandler*>::iterator iter;
			iter = find(handlers.begin(), handlers.end(), handler);
			if (iter != handlers.end()) {
				// TODO: do we want to have a reference count for this?
				return B_OK;
			}

			handlers.push_back(handler);
			return B_OK;
		}


		inline status_t
		ObserverList::Add(const BMessenger &messenger, uint32 what)
		{
			vector<BMessenger> &messengers = fMessengerMap[what];

			vector<BMessenger>::iterator iter;
			iter = find(messengers.begin(), messengers.end(), messenger);
			if (iter != messengers.end()) {
				// TODO: do we want to have a reference count for this?
				return B_OK;
			}

			messengers.push_back(messenger);
			return B_OK;
		}


		inline status_t
		ObserverList::Remove(const BHandler* handler, uint32 what)
		{
			if (handler == NULL)
				return B_BAD_HANDLER;

			// look into the list of messengers
			BMessenger target(handler);
			if (target.IsValid() && Remove(target, what) == B_OK)
				return B_OK;

			status_t status = B_BAD_HANDLER;

			vector<const BHandler*> &handlers = fHandlerMap[what];

			vector<const BHandler*>::iterator iterator = find(handlers.begin(),
				handlers.end(), handler);
			if (iterator != handlers.end()) {
				handlers.erase(iterator);
				status = B_OK;
			}
			if (handlers.empty())
				fHandlerMap.erase(what);

			return status;
		}


		inline status_t
		ObserverList::Remove(const BMessenger &messenger, uint32 what)
		{
			status_t status = B_BAD_HANDLER;

			vector<BMessenger> &messengers = fMessengerMap[what];

			vector<BMessenger>::iterator iterator = find(messengers.begin(),
				messengers.end(), messenger);
			if (iterator != messengers.end()) {
				messengers.erase(iterator);
				status = B_OK;
			}
			if (messengers.empty())
				fMessengerMap.erase(what);

			return status;
		}


		inline bool
		ObserverList::IsEmpty()
		{
			return fHandlerMap.empty() && fMessengerMap.empty();
		}
	}


	enum {
		ItemErased,
		ItemInserted,
		ItemsCleared
	};

	class IObservableContainer {
	public:
		// Observer calls for observing targets in the local team
		virtual status_t				StartWatching(BHandler* observer, uint32 what) = 0;
		virtual status_t				StartWatchingAll(BHandler* observer) = 0;
		virtual status_t				StopWatching(BHandler* observer, uint32 what) = 0;
		virtual status_t				StopWatchingAll(BHandler* observer) = 0;
	protected:
		virtual void					_SendNotices(uint32 what, const BMessage* notice) = 0;
	};


	template<typename Key, typename Value>
	class ObservableMap: public IObservableContainer {
	public:
										ObservableMap();
		virtual							~ObservableMap();

		Value&							Get(const Key& key) { return fMap.at(key); }
		void 							Insert(const Key& key, const Value& value);
		bool 							Erase(const Key& key);
		void 							Clear();
		auto							Size() const { return fMap.size(); };
		auto							Find(Key key) { return fMap.find(key); }

		auto	 						begin() { return fMap.begin(); };
		auto		 					end() { return fMap.end(); };

		void							PrintKeysToStream() const;

		virtual status_t				StartWatching(BHandler* observer, uint32 what) override;
		virtual status_t				StartWatchingAll(BHandler* observer) override;
		virtual status_t				StopWatching(BHandler* observer, uint32 what) override;
		virtual status_t				StopWatchingAll(BHandler* observer) override;

		// Value& operator[](const Key& key) { return fMap[key]; }
	private:
		std::map<Key, Value>			fMap;
		Private::ObserverList*			fObserverList;

		Private::ObserverList*			_ObserverList();
		virtual void					_SendNotices(uint32 what, const BMessage* notice) override;
	};


	template<typename Key, typename Value>
	ObservableMap<Key, Value>::ObservableMap()
	{
	}


	template<typename Key, typename Value>
	ObservableMap<Key, Value>::~ObservableMap()
	{
	}


	template<typename Key, typename Value>
	void
	ObservableMap<Key, Value>::Insert(const Key& key, const Value& value)
	{
		fMap.insert_or_assign(key, value);
		_SendNotices(ItemInserted, new GMessage({{"what", ItemInserted}, {"key", key}}));
	}


	// Remove
	template<typename Key, typename Value>
	bool
	ObservableMap<Key, Value>::Erase(const Key& key)
	{
		auto value = fMap.at(key);
		_SendNotices(ItemErased, new GMessage({{"what", ItemErased}, {"key", key}}));
		return fMap.erase(key);
	}


	template<typename Key, typename Value>
	void
	ObservableMap<Key, Value>::Clear()
	{
		_SendNotices(ItemsCleared, new GMessage({{"what", ItemsCleared}}));
		fMap.clear();
	}


	template<typename Key, typename Value>
	status_t
	ObservableMap<Key, Value>::StartWatching(BHandler* observer, uint32 what)
	{
		Private::ObserverList* list = _ObserverList();
		if (list == NULL)
			return B_NO_MEMORY;

		return list->Add(observer, what);
	}


	template<typename Key, typename Value>
	status_t
	ObservableMap<Key, Value>::StartWatchingAll(BHandler* observer)
	{
		return B_OK;
	}


	template<typename Key, typename Value>
	status_t
	ObservableMap<Key, Value>::StopWatching(BHandler* observer, uint32 what)
	{
		Private::ObserverList* list = _ObserverList();
		if (list == NULL)
			return B_NO_MEMORY;

		return fObserverList->Remove(observer, what);
	}


	template<typename Key, typename Value>
	status_t
	ObservableMap<Key, Value>::StopWatchingAll(BHandler* observer)
	{
		return B_OK;
	}


	template<typename Key, typename Value>
	void
	ObservableMap<Key, Value>::_SendNotices(uint32 what, const BMessage* notice)
	{
		if (fObserverList != NULL)
			fObserverList->SendNotices(what, notice);
	}


	template<typename Key, typename Value>
	Private::ObserverList*
	ObservableMap<Key, Value>::_ObserverList()
	{
		if (fObserverList == NULL)
			fObserverList = new (std::nothrow) Private::ObserverList();

		return fObserverList;
	}


	template<typename Key, typename Value>
	void
	ObservableMap<Key, Value>::PrintKeysToStream() const
	{
		for (auto it = fMap.begin(); it != fMap.end(); ++it) {
			std::cout << "Key = " << it->first << std::endl;
		}
	}

}