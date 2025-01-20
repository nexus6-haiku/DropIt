/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "DragAndDrop.h"
#include "interface/Task.hpp"

#include <View.h>

class BBitmap;
class BMessageRunner;

using namespace Genio::Task;

using dnd = DragAndDrop::DragAndDrop;

class DroppedItem: public BView {
public:
					DroppedItem(dnd* item);
					~DroppedItem();

	virtual void 	AttachedToWindow() override;
	virtual void 	Draw(BRect updateRect) override;
	virtual	void	MouseDown(BPoint oldWhere) override;
	virtual	void	MouseUp(BPoint oldWhere) override;
	virtual void 	MessageReceived(BMessage *message) override;

	virtual	BSize	MinSize() override;
	virtual	BSize	MaxSize() override;
	virtual	BSize	PreferredSize() override;

private:
	dnd*			fItem;
	BBitmap*		fIcon;
	BMessageRunner*	fRunner;
	Task<void>*		fTimeoutTask;
	bool			fRedragging;
	BString			fLabel;
	float			fLabelHeight;

	void			_CalculateSize();
};