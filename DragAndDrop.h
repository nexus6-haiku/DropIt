/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include <Archivable.h>
#include <SupportDefs.h>
#include <String.h>
#include <Messenger.h>

#include <string>
#include <map>

class BHandler;
class BMessage;

namespace DragAndDrop {

	static const int32 kMsgNegotiationFinished = 'mngf';

	class DragAndDrop: public BArchivable {
	public:
										DragAndDrop(BMessage *dragMessage);
										~DragAndDrop();

		void							ProcessReply(BMessage *message, BHandler *replyTo);
		void							End() const;

		bool							IsNegotiated() const;
		BString							NegotiationID() const;

		BMessage*						DragMessage() { return fDragMessage; }

	private:
		bool							fIsNegotiated;
		// std::map<BString, BMessage *>	fMessages;
		BMessage*						fDragMessage;
		BString							fNegotiationID;

		void 							_DetectNegotiation();

		BMessenger						fSender;
		BMessenger						freceiver;
	};

}