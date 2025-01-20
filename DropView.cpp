/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "DropView.h"
#include "MainWindow.h"
#include "Utils.h"

#include <Bitmap.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <Mime.h>
#include <StringView.h>
#include <cstdio>


DropView::DropView()
	: BView("DropView", B_WILL_DRAW | B_FRAME_EVENTS),
	fDropUpIcon(nullptr),
	fDropDownIcon(nullptr),
	fDropUp(false)
{
	AdoptSystemColors();
}


DropView::~DropView()
{
	delete fDropUpIcon;
	delete fDropDownIcon;
}


void
DropView::AttachedToWindow()
{
	SetEventMask(B_POINTER_EVENTS, B_FULL_POINTER_HISTORY);

	fDropUpIcon = new BBitmap(BRect(0, 0, Window()->Bounds().Width() * 0.7 - 1,
		Window()->Bounds().Width() * 0.7 - 1), B_RGBA32);
	GetVectorIcon("DropUpIcon", fDropUpIcon);
	fDropDownIcon = new BBitmap(BRect(0, 0, Window()->Bounds().Width() * 0.7 - 1,
		Window()->Bounds().Width() * 0.7 - 1), B_RGBA32);
	GetVectorIcon("DropDownIcon", fDropDownIcon);
}


void
DropView::Draw(BRect updateRect)
{
	SetDrawingMode( B_OP_COPY );
	SetHighColor(ui_color(B_CONTROL_BORDER_COLOR));
	StrokeRect(Bounds());

	SetDrawingMode(B_OP_ALPHA);
	BPoint iconStartingPoint((Bounds().Width() - fDropUpIcon->Bounds().Width()) / 2,
		(Bounds().Height() - fDropUpIcon->Bounds().Height()) / 2);
	if (fDropUp)
		DrawBitmap(fDropUpIcon, iconStartingPoint);
	else
		DrawBitmap(fDropDownIcon, iconStartingPoint);
}


void
DropView::MessageReceived(BMessage *message)
{
	if (message->WasDropped()) {
		// message->PrintToStream();
		message->RemoveName("_drop_point_");
		message->RemoveName("_drop_offset_");
		BMessage *dropMsg = new BMessage(kMsgDropped);
		dropMsg->AddMessage("dropped_message", message);
		Window()->PostMessage(dropMsg);
	} else {
		BView::MessageReceived(message);
	}
}


void
DropView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	MainWindow *window = reinterpret_cast<MainWindow*>(Window());
	if (message != nullptr) {
		if (!message->HasString("dropit:negotiation_id")) {
			switch (transit) {
				case B_OUTSIDE_VIEW:
				{
					// determine if the pointer gets close to the window
					// point.PrintToStream();
					if (window->SensitiveArea().Contains(ConvertToScreen(point))) {
						window->ShowWindow(true);
						window->PostMessage(kMsgDragging);
					} else {
						if (!window->HasItems()) {
							window->ShowWindow(false);
						}
					}
					fDropUp = true;
					Invalidate();
					break;
				}
				case B_ENTERED_VIEW:
				case B_INSIDE_VIEW: {
					fDropUp = false;
					Invalidate();
					break;
				}
			}
		} else {
			if (!window->HasItems() && window->IsVisible())
				window->ShowWindow(false);
		}
	}
	BView::MouseMoved(point, transit, message);
}


void
DropView::MouseUp(BPoint point)
{
	MainWindow *window = reinterpret_cast<MainWindow*>(Window());
	// point.PrintToStream();
	// rect.PrintToStream();
	if (!Frame().Contains(point) && !window->HasItems()) {
		window->ShowWindow(false);
		fDropUp = false;
	} else {
		window->PostMessage(kMsgDragCanceled);
	}
}
