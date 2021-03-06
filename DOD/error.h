#pragma once

typedef enum _AGERESULT
{
	AGE_SUCCESS,

	// Vulkan
	AGE_ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS,
	AGE_ERROR_GRAPHICS_CREATE_INSTANCE,
	AGE_ERROR_GRAPHICS_SETUP_DEBUG_UTILS_MESSENGER,
	AGE_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER,
	AGE_ERROR_GRAPHICS_DESTROY_DEBUG_UTILS_MESSENGER,
	AGE_ERROR_GRAPHICS_CREATE_SURFACE,
	AGE_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE,
	AGE_ERROR_GRAPHICS_POPULATE_DEVICE_LAYERS_AND_EXTENSIONS,
	AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE,
	AGE_ERROR_GRAPHICS_SURFACE_SUPPORT,
	AGE_ERROR_GRAPHICS_CREATE_SWAPCHAIN,
	AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW,
	AGE_ERROR_GRAPHICS_CREATE_BUFFER,
	AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY,
	AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY,
	AGE_ERROR_GRAPHICS_CREATE_IMAGE,
	AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY,
	AGE_ERROR_GRAPHICS_MAP_MEMORY,
	AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT,
	AGE_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT,
	AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL,
	AGE_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS,
	AGE_ERROR_GRAPHICS_CREATE_RENDER_PASS,
	AGE_ERROR_GRAPHICS_CREATE_SHADER_MODULE,
	AGE_ERROR_GRAPHICS_CREATE_FRAMEBUFFER,
	AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER,
	AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER,
	AGE_ERROR_GRAPHICS_RESET_COMMAND_BUFFER,
	AGE_ERROR_GRAPHICS_CREATE_COMMAND_POOL,
	AGE_ERROR_GRAPHICS_RESET_COMMAND_POOL,
	AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER,
	AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE,
	AGE_ERROR_GRAPHICS_CREATE_SEMAPHORE,
	AGE_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE,
	AGE_ERROR_GRAPHICS_WAIT_FOR_FENCES,
	AGE_ERROR_GRAPHICS_CREATE_FENCE,
	AGE_ERROR_GRAPHICS_RESET_FENCES,
	AGE_ERROR_GRAPHICS_QUEUE_SUBMIT,
	AGE_ERROR_GRAPHICS_QUEUE_PRESENT,
	AGE_ERROR_GRAPHICS_UPDATE_UNIFORM_BUFFER,
	AGE_ERROR_GRAPHICS_CREATE_SAMPLER,

	// GLTF
	AGE_ERROR_GLTF_IMPORT,

	// Network

	// Physics

	// System
	AGE_ERROR_SYSTEM_ALLOCATE_MEMORY,
	AGE_ERROR_SYSTEM_TMP_FILE

} AGE_RESULT;