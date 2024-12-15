/*
 * Parts Copyright 2017-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Utils.h"

#include <Application.h>
#include <IconUtils.h>
#include <Resources.h>
#include <TypeConstants.h>


status_t
GetVectorIcon(const BString icon, BBitmap* bitmap)
{
	if (bitmap == nullptr)
		return B_ERROR;

	BResources* resources = BApplication::AppResources();
	size_t size;
	const uint8* rawIcon;
	rawIcon = (const uint8*)resources->LoadResource(B_VECTOR_ICON_TYPE, icon, &size);
	if (rawIcon == nullptr)
		return B_ERROR;

	return BIconUtils::GetVectorIcon(rawIcon, size, bitmap);
}