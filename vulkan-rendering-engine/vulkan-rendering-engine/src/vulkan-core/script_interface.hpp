#ifndef SCRIPT_INTERFACE_H_
#define SCRIPT_INTERFACE_H_

// Contains includes for all important header files in the engine
// It's called script-interface, because scripts should include 
// this header-file to implement them

#include "structs.hpp"
#include "build_options.h"
#include "vulkan_base.h"
#include "rendering_engine.h"
#include "pipelines/shaders/forward_shader.h"
#include "data/mesh/mesh.h"
#include "data/material/material.h"
#include "data/material/texture/texture.h"
#include "data/material/texture/cubemap.h"
#include "data/color/color.h"
#include "data/lighting/directional_light.h"
#include "data/lighting/point_light.h"
#include "data/lighting/spot_light.h"
#include "scene_graph/scene_manager.h"
#include "scene_graph/example_meshes/cube.h"
#include "scene_graph/example_meshes/quad.h"
#include "scene_graph/example_meshes/sphere.h"
#include "scene_graph/nodes/renderables/renderable.h"
#include "scene_graph/nodes/camera/camera.h"
#include "scene_graph/nodes/renderables/skybox.h"
#include "gui/gui.h"
#include "scene_graph/scene_manager.h"
#include "vulkan-core/pipelines/shaders/shader.h"
#include "vulkan-core/post_processing/post_process.h"
#include "scene_graph/nodes/components/colliders/sphere_collider.h"
#include "advanced_classes/time_of_day.h"
#include "mouse_picker/mouse_picker.h"
#include "resource_manager/resource_manager.h"
#include "scene_graph/layers/layer_manager.h"
#include "data/material/basic_material.h"
#include "data/material/pbr_material.h"
#include "vulkan-core/memory_management/vulkan_memory_manager.h"


#include "memory_manager/memory_manager.h"
#include "file_system/vfs.h"

#endif // !SCRIPT_INTERFACE_H_
