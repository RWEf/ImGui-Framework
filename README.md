>> environment: C++17; Dx12; ImGui Master; Linker(d3d12.lib; d3dcompiler.lib; dxgi.lib).

<br>

# If you need an [__ImGui__][GotoImGui] template, Can use it for your project.

[GotoImGui]: https://github.com/ocornut/imgui

- C++ Main.h: Write this code to use the template.
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
    InitHook::SetRanderHook();

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
- C++ ui.h:
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
##### The main rendering task is in the __Render::UserInterface()__ function in the __user_interface.cpp__ file.

<br>

## Where is ImGui variables config?
##### This ImGui variables config of ImGui template is __ImGui_Cfg__ in the ui.h file.

<br>

- ## Here's the full example user_interface.cpp:
```C++
#include "ui.h"
#include <iostream>

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

int main() {
    //Init the Hook.
    InitHook::SetUIInitHook();
    InitHook::SetRanderHook();

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