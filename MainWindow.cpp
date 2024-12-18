/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DropView.h"
#include "DroppedItem.h"
#include "DockListView.hpp"
#include "MainWindow.h"

#include <AppDefs.h>
#include <Application.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <MessageRunner.h>
#include <ScrollView.h>
#include <StringView.h>

#include <cstdio>


MainWindow::MainWindow(void)
	:	BWindow(BRect(0, 290, 128, 790), "DropIt!", B_NO_BORDER_WINDOW_LOOK,
			B_FLOATING_ALL_WINDOW_FEEL,
			B_NOT_MOVABLE | B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE |
			B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FOCUS |	B_ASYNCHRONOUS_CONTROLS,
			B_ALL_WORKSPACES),
	fButton(nullptr),
	fHasItems(false)
{
	fButton = new BButton("Dropped!", new BMessage(kMsgDismiss));
	fDropView = new DropView();

	auto gridList = new DockListView<DroppedItem, BString, DragAndDrop::DragAndDrop*>("dock",
		&fNegotiations,	B_VERTICAL);

	auto scrollView = new BScrollView("scroll_trans", gridList, B_WILL_DRAW,
		false, true, B_PLAIN_BORDER);
	scrollView->SetBorderHighlighted(true);

	fPanels = BLayoutBuilder::Cards<>(this)
		.Add(fDropView)
		.Add(scrollView)
		.SetVisibleItem(0);

	ShowWindow(false);
}


void
MainWindow::MessageReceived(BMessage *message)
{
	if (message->IsReply()) {
		printf("MainWindow::MessageReceived Is Reply\n");
		message->PrintToStream();
		const BMessage *previous = message->Previous();
		if (previous != nullptr) {
			printf("MainWindow::MessageReceived previous message is present.\n");
			BString negotiationID = previous->GetString("dropit:negotiation_id", "");
			if (!negotiationID.IsEmpty()) {
				printf("MainWindow::MessageReceived negotiationID = %s.\n", negotiationID.String());
				fNegotiations.PrintKeysToStream();
				if (fNegotiations.Find(negotiationID) != fNegotiations.end()) {
					DragAndDrop::DragAndDrop *dnd = fNegotiations.Get(negotiationID);
					printf("MainWindow::MessageReceived negotiation found.\n");
					dnd->ProcessReply(message, this);
				}
			} else {
				return; // TODO: log the error
			}
		}
	}

	switch(message->what) {
		case kMsgDismiss: {
			fPanels->SetVisibleItem(0);
			fHasItems = fNegotiations.Size();
			// ShowWindow(false);
			break;
		}
		case kMsgDropped: {
			// message->PrintToStream();
			BMessage *droppedMsg = new BMessage();
			if (message->FindMessage("dropped_message", droppedMsg) == B_OK) {
				auto dragAndDrop = new DragAndDrop::DragAndDrop(droppedMsg);
				fNegotiations.Insert(dragAndDrop->NegotiationID(), dragAndDrop);
				fNegotiations.PrintKeysToStream();
				droppedMsg->PrintToStream();
			}
			fPanels->SetVisibleItem(1);
			fHasItems = fNegotiations.Size();
			ShowWindow(fHasItems);
			break;
		}
		case kMsgDragging: {
			fPanels->SetVisibleItem(0);
			break;
		}
		case kMsgDragCanceled: {
			fPanels->SetVisibleItem(1);
			break;
		}
		case DragAndDrop::kMsgNegotiationFinished: {
			BString negotiationID = message->GetString("dropit:negotiation_id", "");
			printf("MainWindow::MessageReceived kMsgNegotiationFinished: %s\n", negotiationID.String());
			fNegotiations.PrintKeysToStream();
			fNegotiations.Erase(negotiationID);
			printf("MainWindow::MessageReceived %s erased\n", negotiationID.String());
			fNegotiations.PrintKeysToStream();
			fHasItems = fNegotiations.Size();
			ShowWindow(fHasItems);
			break;
		}
		default: {
			BWindow::MessageReceived(message);
			break;
		}
	}
}


bool
MainWindow::QuitRequested(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::ShowWindow(bool show) {
	if (show) {
		BPoint position(0, Frame().top);
		MoveTo(position);
	} else {
		BPoint position(-Bounds().Width()-1, Frame().top);
		MoveTo(position);
	}
}


bool
MainWindow::IsVisible() {
	return !(Frame().right < 0);
}


BRect
MainWindow::SensitiveArea() {
	BRect sensitiveArea;
	sensitiveArea.left = 0;
	sensitiveArea.right = Frame().Width();
	sensitiveArea.top = Frame().top;
	sensitiveArea.bottom = Frame().bottom;
	sensitiveArea.left -= sensitiveArea.Width() * 0.4;
	sensitiveArea.right += sensitiveArea.Width() * 0.4;
	sensitiveArea.top -= sensitiveArea.Height() * 0.2;
	sensitiveArea.bottom += sensitiveArea.Height() * 0.2;
	// sensitiveArea.PrintToStream();
	return sensitiveArea;
}


bool
MainWindow::HasItems()
{
	return fHasItems;
}