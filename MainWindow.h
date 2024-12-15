/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "interface/ObservableMap.hpp"
#include "DragAndDrop.h"

#include <GroupView.h>
#include <LayoutBuilder.h>
#include <Window.h>

#include <map>

using Observable::ObservableMap;

class BButton;
class BCardLayout;
class BListView;
class BMessageRunner;
class DropView;

class MainWindow : public BWindow
{
public:
								MainWindow();

	virtual void				MessageReceived(BMessage *msg) override;
	virtual bool				QuitRequested(void) override;

	void						ShowWindow(bool show);
	bool						IsVisible();
	BRect						SensitiveArea();

	bool						HasItems();
private:
	BButton*					fButton;
	bool						fHasItems;
	BCardLayout*				fPanels;
	DropView*					fDropView;
	BLayoutBuilder::Group<>		fDock;

	ObservableMap<BString, DragAndDrop::DragAndDrop*> fNegotiations;
};