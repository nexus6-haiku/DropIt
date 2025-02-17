/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "interface/ObservableMap.hpp"
#include <Button.h>
#include <LayoutBuilder.h>
#include <SupportDefs.h>
#include <ListItem.h>
#include <ScrollView.h>
#include <View.h>


using Observable::ObservableMap;


template <typename Item>
class DockListItem: public BView {
public:
							DockListItem(Item item);
	virtual					~DockListItem();

protected:
	Item					fItem;
};


template <typename Item>
DockListItem<Item>::DockListItem(Item item)
	: BView("item", B_WILL_DRAW | B_FRAME_EVENTS),
	fItem(item)
{
}

template <typename Item>
DockListItem<Item>::~DockListItem()
{
}


// template<Derived<DockListItem> T, typename Key, typename Value>
template<typename T, typename Key, typename Value>
class DockListView: public BView {
public:
									DockListView(const char* name,
										ObservableMap<Key, Value>* dataSource,
										orientation orientation = B_VERTICAL);
	virtual							~DockListView();

	void 							SetDataSource(ObservableMap<Key, Value>* dataSource);

	virtual void 					Draw(BRect updateRect) override;
	virtual void 					AttachedToWindow() override;
	virtual void 					TargetedByScrollView(BScrollView* scrollView) override;
	virtual void					MessageReceived(BMessage* message) override;
	virtual void					ScrollTo(BPoint point) override;

private:
		ObservableMap<Key, Value>*	fDataSource;
		BLayoutBuilder::Group<>		fLayout;
		BScrollView*				fScrollView;

		void						_InitData();
		void						_FixupScrollBar();
		void						_RedoLayout();
};


template<typename T, typename Key, typename Value>
DockListView<T, Key, Value>::DockListView(const char* name,
								ObservableMap<Key, Value>* dataSource,
								orientation orientation)
	: BView(name, B_WILL_DRAW | B_FRAME_EVENTS | B_SCROLL_VIEW_AWARE),
	fDataSource(dataSource)
{
	(fLayout = BLayoutBuilder::Group<>(this, orientation, B_USE_SMALL_SPACING))
		.SetInsets(10, 10, 10, 10)
		.SetExplicitAlignment(BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_MIDDLE));
		// .AddGlue();
}


template<typename T, typename Key, typename Value>
DockListView<T, Key, Value>::~DockListView()
{
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::AttachedToWindow()
{
	BView::AttachedToWindow();
	_InitData();
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::Draw(BRect updateRect)
{
	auto bounds = fLayout.View()->Bounds();
	SetExplicitSize(bounds.Size());
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::SetDataSource(ObservableMap<Key, Value>* dataSource)
{
	for (int i = 0; i < CountChildren(); i++)
		RemoveChild(ChildAt(i));
	fDataSource = dataSource;
	_InitData();
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::TargetedByScrollView(BScrollView* scrollView)
{
	// BView::TargetedByScrollView(scrollView);
	// _FixupScrollBar();
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case 'rdlo': {
			printf("Redo layout\n");
			_RedoLayout();
			break;
		}
		case B_OBSERVER_NOTICE_CHANGE: {
			int32 code;
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &code);
			switch (code) {
				case Observable::ItemInserted: {
					printf("Observable::ItemInserted\n");
					// message->PrintToStream();
					GMessage *msg = (GMessage *)message;
					Key key = (*msg)["key"];
					// printf("key = %s\n", key.String());
					auto item = fDataSource->Get(key);
					auto droppeditem = new T(item);
					fLayout.Add(droppeditem);
					// fLayout.AddGlue();
					_RedoLayout();
					break;
				}
				case Observable::ItemErased: {
					printf("Observable::ItemErased\n");
					message->AddMessenger("sender", BMessenger(this));
					message->RemoveData("be:observe_orig_what");
					message->RemoveData("be:observe_change_what");
					SendNotices(Observable::ItemErased, message);
					break;
				}
				default:
					break;
			}
		}
		default:
			BView::MessageReceived(message);
	}
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::ScrollTo(BPoint point)
{
	BView::ScrollTo(point);
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::_InitData()
{
	// TODO: if StartWatching fails we need to notify the caller. Maybe raising an exception?
	fDataSource->StartWatching(this, Observable::ItemInserted);
	fDataSource->StartWatching(this, Observable::ItemErased);
	fDataSource->StartWatching(this, Observable::ItemsCleared);

	for (auto it = fDataSource->begin(); it != fDataSource->end(); ++it) {
		auto item = new T(it->second);
		fLayout.Add(item);
	}
	_RedoLayout();
}


template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::_FixupScrollBar()
{
	printf("_FixupScrollBar()\n");

	BRect bounds = Bounds();
	if (bounds.Size().Height() <= 0 || bounds.Size().Width() <= 0)
		return;

	BScrollBar* vertScroller = ScrollBar(B_VERTICAL);
	if (vertScroller != NULL) {

		int32 size = fDataSource->Size();
		int32 count = fLayout.View()->CountChildren();
		printf("Size %d - count %d\n", size, count);
		// printf("Bounds: ");
		// bounds.PrintToStream();

		float itemHeight = 0.0;

		if (size > 0) {
			// for (int i = 0; i < count; i++) {
				auto itemFrame = fLayout.View()->ChildAt(count - 1)->Frame();
				// printf("Item frame: ");
				// itemFrame.PrintToStream();
				auto currentHeight = itemFrame.bottom;
				itemHeight = itemFrame.bottom;
				// if (currentHeight > itemHeight)
					// itemHeight = currentHeight;
				itemHeight += 17;
			// }
		}
		// printf("itemHeight %f\n", itemHeight);
		if (bounds.Height() >= itemHeight) {
			// no scrolling
			vertScroller->SetRange(0.0, 0.0);
			vertScroller->SetValue(0.0);
				// also scrolls to the top
			// printf("bounds.Height() >= itemHeight\n");
		} else {
			// printf("bounds.Height() < itemHeight\n");
			vertScroller->SetRange(0.0, itemHeight - bounds.Height());
			vertScroller->SetProportion(bounds.Height () / itemHeight);
			// scroll up if there is empty room on bottom
			if (itemHeight < bounds.bottom)
				ScrollBy(0.0, bounds.bottom - itemHeight);
		}

		if (size != 0) {
			// auto steps = ceilf((fLayout.View()->ChildAt(count - 1))->Frame().Height());
			auto steps = ceilf(itemHeight);
			vertScroller->SetSteps(steps, bounds.Height());
		}
	}

	BScrollBar* horizontalScroller = ScrollBar(B_HORIZONTAL);
	if (horizontalScroller != NULL) {
		float w;
		GetPreferredSize(&w, NULL);
		BRect scrollBarSize = horizontalScroller->Bounds();

		if (w <= scrollBarSize.Width()) {
			// no scrolling
			horizontalScroller->SetRange(0.0, 0.0);
			horizontalScroller->SetValue(0.0);
		} else {
			horizontalScroller->SetRange(0, w - scrollBarSize.Width());
			horizontalScroller->SetProportion(scrollBarSize.Width() / w);
		}
	}
}

template<typename T, typename Key, typename Value>
void
DockListView<T, Key, Value>::_RedoLayout()
{
	Layout(true);
	_FixupScrollBar();
}


