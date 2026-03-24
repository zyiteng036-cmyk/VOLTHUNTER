// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultWidget.h"

void UResultWidget::NotifyInput(const EResultMenuInputType _inputType, const int32 _index) {
	CurrentIndex = _index;
	
	OnResultMenuInput.Broadcast(_inputType);
}

