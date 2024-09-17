#include "ui.h"

#define SET_HOOK_FUNCTION(RADDRESS, DADDRESS) *(size_t*)(RADDRESS) = (size_t)&DADDRESS;

void InitHook::SetUIInitHook() {
	SET_HOOK_FUNCTION(GlobalData::HookInit[0], InitUI::CreateFatherWindow);
	SET_HOOK_FUNCTION(GlobalData::HookInit[1], InitUI::CreateDevice12);
	SET_HOOK_FUNCTION(GlobalData::HookInit[2], InitUI::ShowWindow);
	SET_HOOK_FUNCTION(GlobalData::HookInit[3], InitUI::ImGui);
}

void InitHook::SetRanderHook() {
	GlobalData::HookRander.Function1 = ::Render::Render;
	GlobalData::HookRander.Function2 = ::Render::UserInterface;
}