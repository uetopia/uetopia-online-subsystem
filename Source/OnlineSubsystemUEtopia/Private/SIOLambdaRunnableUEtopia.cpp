// Copyright 2016-2018 UEtopia, LLC. All Rights Reserved.
#pragma once

#include "OnlineSubsystemUEtopiaPrivatePCH.h"
#include "SIOLambdaRunnableUEtopia.h"


uint64 FSIOLambdaRunnableUEtopia::ThreadNumber = 0;

FSIOLambdaRunnableUEtopia::FSIOLambdaRunnableUEtopia(TFunction< void()> InFunction)
{
	FunctionPointer = InFunction;
	Finished = false;
	Number = ThreadNumber;

	FString threadStatGroup = FString::Printf(TEXT("FSIOLambdaRunnable%d"), ThreadNumber);
	Thread = NULL;
	Thread = FRunnableThread::Create(this, *threadStatGroup, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	ThreadNumber++;

	//Runnables.Add(this);
}

FSIOLambdaRunnableUEtopia::~FSIOLambdaRunnableUEtopia()
{
	if (Thread == NULL)
	{
		delete Thread;
		Thread = NULL;
	}

	//Runnables.Remove(this);
}

//Init
bool FSIOLambdaRunnableUEtopia::Init()
{
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Init"), Number);
	return true;
}

//Run
uint32 FSIOLambdaRunnableUEtopia::Run()
{
	if (FunctionPointer != nullptr)
	{
		FunctionPointer();
	}
	UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Run complete"), Number);
	return 0;
}

//stop
void FSIOLambdaRunnableUEtopia::Stop()
{
	Finished = true;
}

void FSIOLambdaRunnableUEtopia::Kill()
{
	UE_LOG(LogClass, Log, TEXT("Yolo!"));
	Thread->Kill(false);
	Finished = true;
}

void FSIOLambdaRunnableUEtopia::Exit()
{
	Finished = true;
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Exit"), Number);

	//delete ourselves when we're done
	delete this;
}

void FSIOLambdaRunnableUEtopia::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

FSIOLambdaRunnableUEtopia* FSIOLambdaRunnableUEtopia::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	FSIOLambdaRunnableUEtopia* Runnable;
	if (FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FSIOLambdaRunnableUEtopia(InFunction);
		//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable RunLambdaBackGroundThread"));
		return Runnable;
	}
	else
	{
		return nullptr;
	}
}

FGraphEventRef FSIOLambdaRunnableUEtopia::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

void FSIOLambdaRunnableUEtopia::ShutdownThreads()
{
	/*for (auto Runnable : Runnables)
	{
	if (Runnable != nullptr)
	{
	delete Runnable;
	}
	Runnable = nullptr;
	}*/
}
