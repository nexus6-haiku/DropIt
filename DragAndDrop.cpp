/*
 * Copyright 2024, Nexus6 <nexus6@disroot.org>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "DragAndDrop.h"

#include <Message.h>
#include <Messenger.h>
#include <Uuid.h>

namespace DragAndDrop {

	DragAndDrop::DragAndDrop(BMessage *dragMessage)
		:
		fNegotiationID(nullptr),
		fIsNegotiated(true),
		fDragMessage(dragMessage),
		fSender(dragMessage->ReturnAddress()),
		fCompleted(false)
	{
		// set message and negotiation ID
		_DetectNegotiation();
		fNegotiationID = BUuid().SetToRandom().ToString();
		BString messageID = BUuid().SetToRandom().ToString();
		fDragMessage->AddString("dropit:negotiation_id", fNegotiationID);

		// printf("DragAndDrop::DragAndDrop return address Team = %d.\n", fSender.Team());
	}


	DragAndDrop::~DragAndDrop()
	{
	}


	void
	DragAndDrop::NotifyCompleted(BHandler *replyTo)
	{
		BMessenger replyToMessenger(replyTo);
		BMessage replyMessage(kMsgNegotiationFinished);
		replyMessage.AddString("dropit:negotiation_id", fNegotiationID);
		replyToMessenger.SendMessage(&replyMessage);
		printf("kMsgNegotiationFinished sent\n");
	}

	void
	DragAndDrop::ProcessReply(BMessage *message, BHandler *replyTo)
	{
		// printf("DragAndDrop::ProcessReply.\n");
		// message->PrintToStream();

		// check who sent this
		team_id replyTeam = message->ReturnAddress().Team();
		if (replyTeam != fSender.Team()) {
			fSender.SendMessage(message);
			while (!message->WasDelivered())
				sleep(500);
			// NotifyCompleted(replyTo);
			Completed();
			printf("reply was delivered, negotiation completed\n");
		}
	}

	bool
	DragAndDrop::IsNegotiated() const
	{
		return fIsNegotiated;
	}


	void
	DragAndDrop::_DetectNegotiation()
	{
		// TODO
		fIsNegotiated = false;
	}


	BString
	DragAndDrop::NegotiationID() const
	{
		return fNegotiationID;
	}

	bool
	DragAndDrop::IsCompleted() const
	{
		return fCompleted;
	}

	void
	DragAndDrop::Completed()
	{
		fCompleted = true;
	}

}