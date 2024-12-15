/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "DragAndDrop.h"
// #include "interface/GridListView/GridListView.h"
#include <View.h>

class BBitmap;

// class DroppedItem: public GridListItem<BMessage*> {
class DroppedItem: public BView {
public:
					DroppedItem(BMessage* item);
					~DroppedItem();

	virtual void 	AttachedToWindow() override;
	virtual void 	Draw(BRect updateRect) override;
	virtual	void	MouseDown(BPoint oldWhere) override;
	virtual void 	MessageReceived(BMessage *message) override;

private:
	BMessage*		fItem;
	BBitmap*		fIcon;
	// BView*			fView;
};