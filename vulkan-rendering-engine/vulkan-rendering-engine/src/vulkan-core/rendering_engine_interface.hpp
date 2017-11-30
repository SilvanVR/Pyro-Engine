#ifndef RENDERING_ENGINE_INTERFACE_H_
#define RENDERING_ENGINE_INTERFACE_H_

// This header-file serves as the main-interface for the rendering-engine.
// Basic Idea: If you include this you have access to everything in the engine.

// Includes all core Engine-Files
#include "script_interface.hpp"

// Include all Script-files here
#include "scripts/camera_mov.h"
#include "scripts/rotation_script.h"
#include "scripts/move_script.h"
#include "scripts/look_at.h"
#include "scripts/debug_menu.h"
#include "scripts/interaction_script.h"
#include "scripts/light_spawn.h"
#include "scripts/object_spawn.h"
#include "scripts/billboard.h"
#include "scripts/lens_flares.h"

#endif // !ENGINE_INTERFACE_H_
