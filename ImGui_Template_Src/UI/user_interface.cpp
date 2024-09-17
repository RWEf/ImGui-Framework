#include "ui.h"
#include <iostream>

void Render::UserInterface() {

	// Main rendering task...

	

    // Don't delete.
	for (const auto& ui : GlobalData::UIFInterface) {
		ui();
	}
}