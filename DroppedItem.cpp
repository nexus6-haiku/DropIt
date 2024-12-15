/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "DroppedItem.h"
#include "DragAndDrop.h"
#include "Utils.h"

#include <Bitmap.h>
#include <Button.h>
#include <View.h>
#include <Window.h>

#include <cstdio>


DroppedItem::DroppedItem(BMessage *message)
	// : GridListItem(message),
	: BView("item", B_WILL_DRAW | B_FRAME_EVENTS | B_DRAW_ON_CHILDREN),
	fIcon(nullptr),
	fItem(message)
{
	// printf("DroppedItem::DroppedItem()\n");
}


DroppedItem::~DroppedItem()
{
	// delete fItem;
	// delete fIcon;
	// printf("DroppedItem::~DroppedItem()\n");
}


void
DroppedItem::AttachedToWindow()
{
	fIcon = new BBitmap(BRect(0, 0, 128 * 0.7 - 1,
		128 * 0.7 - 1), B_RGBA32);
	GetVectorIcon("DropUpIcon", fIcon);

	SetExplicitSize(fIcon->Bounds().Size());

	if (Window()->LockLooper()) {
		Window()->StartWatching(this, DragAndDrop::kMsgNegotiationFinished);
		Window()->UnlockLooper();
	}
	// printf("DroppedItem::AttachedToWindow()\n");
}


void
DroppedItem::Draw(BRect updateRect)
{
	// printf("DroppedItem::Draw(BRect updateRect)\n");
	SetDrawingMode(B_OP_ALPHA);
	// BPoint iconStartingPoint(B_ORIGIN);
	BPoint iconStartingPoint((Bounds().Width() - fIcon->Bounds().Width()) / 2,
		(Bounds().Height() - fIcon->Bounds().Height()) / 2);
	DrawBitmap(fIcon, B_ORIGIN);
}


void
DroppedItem::MouseDown(BPoint where)
{
	if (Bounds().Contains(where)) {
		// printf("drag\n");
		SetMouseEventMask(B_POINTER_EVENTS, 0);
		// fItem->PrintToStream();
		BBitmap *dragicon = new BBitmap(fIcon);
		DragMessage(fItem, dragicon, B_OP_ALPHA, BPoint(32,32), (BHandler*)Window());
	}
}


void
DroppedItem::MessageReceived(BMessage *message)
{
	// message->PrintToStream();
	switch(message->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			int32 code;
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &code);
			if (code == DragAndDrop::kMsgNegotiationFinished) {
				BString negotiationID = message->GetString("dropit:negotiation_id", "");
				BString thisID = fItem->GetString("dropit:negotiation_id", "");
				if (negotiationID == thisID) {
					RemoveSelf();
					delete this;
				}
			}
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}