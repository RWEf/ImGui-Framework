>> Environment: C++17; Dx12; ImGui Master; Linker(d3d12.lib; d3dcompiler.lib; dxgi.lib).

<br>

# If you need an [__ImGui__][GotoImGui] framework, Can use it for your project.

- C++ Main.h: Write this code to use the framework.
```C++
#include "ui.h"
```

<br>

### Before using, you need to initialize it.
> #### Initialize in main function. (The demo)
- C++ Main.h:
```C++
int main() {
    //Init the Hook.
    InitHook::SetUIInitHook();
    InitHook::SetRenderHook();

    //Calls hook function.
    for (size_t i = 0; i < 4; i++) {
	    GlobalData::HookInit(i);
    }

    //UI render work...

    //Start render work now.
    Render::Start(
        []() {
            //This is rendering work.
            //If the button is on, prints str in cmd.
            if (ImGui_Cfg::Switch::Enable_UTPT) {
            	printf("Enabled the Enable_UTPT\n");
            }
        }
    );

    return EXIT_SUCCESS;
}
```

<br>

## How to add render work?
- C++ [ui.h][GotoUI.h]:
```C++
    //namespace Render {...};
    template <class T>
    void Render::AddUI(const T&& Function);
```
#### Function
```
-- Render::AddUI
    Description: It's used for rendering tasks.
    Par description: It's a closure function, use ImGui function in the closure function 
    to perform a specific task.
```

<br>

## Where is the main rendering task?
#### The main rendering task is in the __Render::UserInterface()__ function in the [__user_interface.cpp__][GotoUI.cpp] file.

<br>

## Where is ImGui variables config?
#### This ImGui variables config of ImGui framework is [__ImGui_Cfg__][GotoUI.h] in the [ui.h][GotoUI.h] file.

<br>

- ## Here's the full example [user_interface.cpp][GotoUI.cpp]:
```C++
#include "ui.h"

void Render::UserInterface() {

	// Main rendering task...
	if (!GlobalData::Init) {
		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::SetNextWindowSize({ 600, 400 });

		GlobalData::Init = true;
	}

	if (ImGui::Begin("Main")) {

		ImGui::End();
	}

    // Don't delete.
	for (const auto& ui : GlobalData::UIFInterface) {
		ui();
	}
}
```

<br>

- ## Here's the full example Main.h:
 ```C++
#include "ui.h"
#include <iostream>

int main() {
    //Init the Hook.
    InitHook::SetUIInitHook();
    InitHook::SetRenderHook();

    //Calls hook function.
    for (size_t i = 0; i < 4; i++) {
        GlobalData::HookInit(i);
    }

    //UI render work...
    bool ini = false;
    Render::AddUI(
        [&ini]() {
            if (!ini) {
                ImGui::SetNextWindowPos({ 100, 100 });
                ImGui::SetNextWindowSize({ 600, 400 });
                
                ini = true;
            }

            if (ImGui::Begin("DEBUG")) {

                {
                    ImGui::Checkbox("UTPT", &ImGui_Cfg::Switch::Enable_UTPT);
                }

                ImGui::End();
            }
        }
    );

    //Start the render work now.
    Render::Start(
        []() {
            //This is rendering work.
            //If the button is on, prints str in cmd.
            if (ImGui_Cfg::Switch::Enable_UTPT) {
                printf("Enabled the Enable_UTPT\n");
            }
        }
    );

    return EXIT_SUCCESS;
}
 ```

[GotoImGui]: https://github.com/ocornut/imgui
[GotoUI.h]: https://github.com/RWEf/ImGuiTemplate/blob/main/ImGui_Template_Src/UI/ui.h
[GotoUI.cpp]: https://github.com/RWEf/ImGuiTemplate/blob/main/ImGui_Template_Src/UI/user_interface.cpp