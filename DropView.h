/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "DragAndDrop.h"
#include "DroppedItem.h"
#include <View.h>

class BStringView;

static const int32 kMsgDismiss = 'dism';
static const int32 kMsgDropped = 'mdro';
static const int32 kMsgDragging = 'mdre';
static const int32 kMsgDragCanceled = 'mdrc';
static const int32 kTimeout = 1000000;

class DropView: public BView {
public:
						DropView();
						~DropView();

	virtual void 		AttachedToWindow() override;
	virtual void 		Draw(BRect updateRect) override;
	virtual void 		MessageReceived(BMessage *message) override;
	virtual void		MouseMoved(BPoint point, uint32 transit, const BMessage *message) override;
	virtual void		MouseUp(BPoint point) override;
private:
	BBitmap*			fDropUpIcon;
	BBitmap*			fDropDownIcon;
	bool				fDropUp;
};