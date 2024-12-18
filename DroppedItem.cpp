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
	printf("DroppedItem::~DroppedItem()\n");
}


void
DroppedItem::AttachedToWindow()
{
	fIcon = new BBitmap(BRect(0, 0, 128 * 0.5 - 1,
		128 * 0.5 - 1), B_RGBA32);
	GetVectorIcon("default", fIcon);

	SetExplicitSize(fIcon->Bounds().Size());

	if (Parent()->LockLooper()) {
		Parent()->StartWatching(this, Observable::ItemErased);
		Parent()->UnlockLooper();
	}

	SetToolTip(fLabel);

	printf("DroppedItem::AttachedToWindow()\n");
}


void
DroppedItem::Draw(BRect updateRect)
{
	printf("DroppedItem::Draw(BRect updateRect)\n");
	SetDrawingMode(B_OP_ALPHA);
	BPoint iconStartingPoint((Bounds().Width() - fIcon->Bounds().Width()) / 2,
		(Bounds().Height() - fIcon->Bounds().Height()) / 2);
	DrawBitmap(fIcon, B_ORIGIN);

	auto size = fIcon->Bounds().Size();
	BFont font;
	GetFont(&font);
	BString truncatedString(fLabel);
	font.TruncateString(&truncatedString, B_TRUNCATE_MIDDLE, size.Width());

	font_height fheight;
	font.GetHeight(&fheight);
	auto height = ceilf(fheight.ascent) + ceilf(fheight.descent) + ceilf(fheight.leading) + 4;

	MovePenBy(iconStartingPoint.x, size.Height() + height);

	SetExplicitSize(BSize(size.Width(), size.Height() + height + fheight.descent));

	DrawString(truncatedString);
}


void
DroppedItem::MouseDown(BPoint where)
{
	if (Bounds().Contains(where)) {
		// printf("drag\n");
		SetMouseEventMask(B_POINTER_EVENTS, 0);
		// fItem->PrintToStream();
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