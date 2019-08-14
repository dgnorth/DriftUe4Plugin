// Copyright 2016-2017 Directive Games Limited - All Rights Reserved.

#include "OnlineAsyncTaskManagerDrift.h"
#include "OnlineSubsystemDriftPrivatePCH.h"
#include "OnlineSubsystemDrift.h"

void FOnlineAsyncTaskManagerDrift::OnlineTick()
{
	check(DriftSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId || !FPlatformProcess::SupportsMultithreading());
}

