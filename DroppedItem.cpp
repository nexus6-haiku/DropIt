/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "DroppedItem.h"
#include "DragAndDrop.h"
#include "Utils.h"
#include "interface/ObservableMap.hpp"

#include <Bitmap.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutUtils.h>
#include <MessageRunner.h>
#include <View.h>
#include <Window.h>

#include <cstdio>

const int kTimeout = 500000; // 0.5sec
const int kMsgTimeout = 'timo';

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Item"

DroppedItem::DroppedItem(dnd *item)
	: BView("item", B_WILL_DRAW | B_FRAME_EVENTS | B_DRAW_ON_CHILDREN),
	fIcon(nullptr),
	fItem(item),
	fRunner(nullptr),
	fTimeoutTask(nullptr),
	fRedragging(false)
{
	fLabel = fItem->DragMessage()->GetString("be:clip_name", B_TRANSLATE("Unknown clip"));
	printf("DroppedItem::DroppedItem()\n");
}


DroppedItem::~DroppedItem()
{
	delete fIcon;
	// printf("DroppedItem::~DroppedItem()\n");
}


void
DroppedItem::AttachedToWindow()
{
	// printf("DroppedItem::AttachedToWindow()\n");

	fIcon = new BBitmap(BRect(0, 0, 128 * 0.5 - 1,
		128 * 0.5 - 1), B_RGBA32);
	GetVectorIcon("default", fIcon);

	if (Parent()->LockLooper()) {
		Parent()->StartWatching(this, Observable::ItemErased);
		Parent()->UnlockLooper();
	}

	_CalculateSize();
}


void
DroppedItem::Draw(BRect updateRect)
{
	// printf("DroppedItem::Draw(BRect updateRect) START\n");
	SetDrawingMode(B_OP_ALPHA);
	BPoint iconStartingPoint((Bounds().Width() - fIcon->Bounds().Width()) / 2,
		(Bounds().Height() - fIcon->Bounds().Height()) / 2);
	DrawBitmap(fIcon, B_ORIGIN);

	auto size = fIcon->Bounds().Size();
	BFont font;
	GetFont(&font);
	BString truncatedString(fLabel);
	font.TruncateString(&truncatedString, B_TRUNCATE_MIDDLE, size.Width());

	MovePenBy(iconStartingPoint.x, size.Height() + fLabelHeight);

	DrawString(truncatedString);

	// BString tooltip = BString("Frame: ");
	// tooltip << fLabel << " " << Frame().LeftTop().y << " " << Frame().RightBottom().y;
	// SetToolTip(tooltip);
	// Frame().PrintToStream();
	// printf("DroppedItem::Draw(BRect updateRect) END\n");
}


void
DroppedItem::MouseDown(BPoint where)
{
	if (Bounds().Contains(where)) {
		// printf("drag\n");
		SetMouseEventMask(B_POINTER_EVENTS, 0);
		fItem->DragMessage()->PrintToStream();
		BBitmap *dragicon = new BBitmap(fIcon);
		DragMessage(fItem->DragMessage(), dragicon, B_OP_ALPHA, BPoint(32,32), (BHandler*)Window());
		fRedragging = true;
	}
}


void
DroppedItem::MouseUp(BPoint where)
{
	fTimeoutTask = new Task<void>("timeoutTask", BMessenger(this),
		[this]() {
			BMessage timeoutMessage(kMsgTimeout);
			fRunner = new BMessageRunner(BMessenger(this), &timeoutMessage, kTimeout, 1);
			while (!fItem->DragMessage()->WasDelivered())
				sleep(500);
			fRunner->SetInterval(kTimeout);
			while (!fItem->IsCompleted())
				sleep(500);
			fRunner->SetInterval(kTimeout);
		}
	);
	fTimeoutTask->Run();
	fRedragging = false;
}


void
DroppedItem::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			message->PrintToStream();
			int32 code;
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &code);
			if (code == Observable::ItemErased) {
				printf("DroppedItem::MessageReceived() Observable::ItemErased\n");
				GMessage *msg = (GMessage *)message;
				BString negotiationID = (*msg)["key"];
				// BString negotiationID = message->GetString("dropit:negotiation_id", "");
				BString thisID = fItem->DragMessage()->GetString("dropit:negotiation_id", "");
				if (negotiationID == thisID) {
					printf("negotiationID == thisID\n");
					RemoveSelf();
					BMessenger sender;
					message->FindMessenger("sender", &sender);
					sender.SendMessage('rdlo');
					delete this;
				}
			}
			break;
		}
		case kMsgTimeout: {
			printf("kMsgTimeout\n");
			delete fRunner;
			fRunner = nullptr;
			fTimeoutTask->Stop();
			fItem->NotifyCompleted((BHandler*)Window());
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}


BSize
DroppedItem::MinSize()
{
	auto size = BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(128, 128));
	// printf("DroppedItem::MinSize() %f %f\n", size.Width(), size.Height());
	return size;
}


BSize
DroppedItem::MaxSize()
{
	auto size = BView::MaxSize();
	// printf("DroppedItem::MaxSize() %f %f\n", size.Width(), size.Height());
	return size;
}


BSize
DroppedItem::PreferredSize()
{
	auto size = BLayoutUtils::ComposeSize(ExplicitMaxSize(), BSize(128, 128));
	// printf("DroppedItem::PreferredSize() %f %f\n", size.Width(), size.Height());
	return size;
}


void
DroppedItem::_CalculateSize()
{
	BFont font;
	GetFont(&font);
	font_height fheight;
	font.GetHeight(&fheight);
	fLabelHeight = ceilf(fheight.ascent) + ceilf(fheight.descent) + ceilf(fheight.leading) + 4;
	auto size = fIcon->Bounds().Size();
	SetExplicitSize(BSize(size.Width(), size.Height() + fLabelHeight + fheight.descent));
	// printf("DroppedItem::_CalculateSize() frame: ");
	// Frame().PrintToStream();
}