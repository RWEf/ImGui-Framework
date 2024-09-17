#include "ui.h"

LPWNDCLASSEXW GlobalData::WindowClass = nullptr;
HWND GlobalData::WindowHwnd		      = nullptr;

UINT GlobalData::ScreenWidth  = 0;
UINT GlobalData::ScreenHeight = 0;

Hook::HookInit   GlobalData::HookInit;
Hook::HookRander GlobalData::HookRander;

ImVec2 GlobalData::WindowPos = { 0, 0 };

std::vector<std::function<void()>> GlobalData::UIFInterface = {};

bool GlobalData::Init = false;

bool ImGui_Cfg::Switch::Enable_UTPT = false;