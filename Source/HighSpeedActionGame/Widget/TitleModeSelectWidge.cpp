// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleModeSelectWidge.h"

void UTitleModeSelectWidge::NotifyInput(ETitleMenuInputType _inputType, int32 index) {
	CurrentIndex = index;

	OnTitleMenuInput.Broadcast(_inputType);

}

