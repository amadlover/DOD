#include "log.h"
#include <stdio.h>

AGE_RESULT log_error (const AGE_RESULT result)
{
	switch (result)
	{
	case AGE_ERROR_GRAPHICS_POPULATE_INSTANCE_LAYERS_AND_EXTENSIONS:
		printf ("Graphics Error: Populating instance Layers and Extensions\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_INSTANCE:
		printf ("Graphics Error: Create instance\n");
		break;

	case AGE_ERROR_GRAPHICS_SETUP_DEBUG_UTILS_MESSENGER:
		printf ("Graphics Error: Setup Debug Utils Messenger\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER:
		printf ("Graphics Error: Create Debug Utils Messenger\n");
		break;

	case AGE_ERROR_GRAPHICS_DESTROY_DEBUG_UTILS_MESSENGER:
		printf ("Graphics Error: Destroy Debud Utils Messenger\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_SURFACE:
		printf ("Graphics Error: Create surface\n");
		break;

	case AGE_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE:
		printf ("Graphics Error: Get Physical Device\n");
		break;

	case AGE_ERROR_GRAPHICS_POPULATE_DEVICE_LAYERS_AND_EXTENSIONS:
		printf ("Graphics Error: Populate Device Layers and Extensions\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE:
		printf ("Graphics Error: Create Graphics Device\n");
		break;

	case AGE_ERROR_GRAPHICS_SURFACE_SUPPORT:
		printf ("Graphics Error: surface Support\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_SWAPCHAIN:
		printf ("Graphics Error: Create swapchain\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW:
		printf ("Graphics Error: Create Image View\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_BUFFER:
		printf ("Graphics Error: Create buffer\n");
		break;

	case AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY:
		printf ("Graphics Error: Allocate Memory\n");
		break;

	case AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY:
		printf ("Graphics Error: Bind buffer Memory\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_IMAGE:
		printf ("Graphics Error: Create Image\n");
		break;

	case AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY:
		printf ("Graphics Error: Bind Image Memory\n");
		break;

	case AGE_ERROR_GRAPHICS_MAP_MEMORY:
		printf ("Graphics Error: Map Image Memory\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT:
		printf ("Graphics Error: Create Descriptor Set Layout\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT:
		printf ("Graphics Error: Create Pipeline Layout\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL:
		printf ("Graphics Error: Create Descriptor Pool\n");
		break;

	case AGE_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS:
		printf ("Graphics Error: Allocate Descriptor Set\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_RENDER_PASS:
		printf ("Graphics Error: Create Render Pass\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_SHADER_MODULE:
		printf ("Graphics Error: Create Shader Module\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_FRAMEBUFFER:
		printf ("Graphics Error: Create Framebuffer\n");
		break;

	case AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER:
		printf ("Graphics Error: Begin Command buffer\n");
		break;

	case AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER:
		printf ("Graphics Error: End Command buffer\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_COMMAND_POOL:
		printf ("Graphics Error: Create Command Pool\n");
		break;

	case AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER:
		printf ("Graphics Error: Allocate Command buffer\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE:
		printf ("Graphics Error: Create Graphics Pipeline\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_SEMAPHORE:
		printf ("Graphics Error: Create Semaphore\n");
		break;

	case AGE_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE:
		printf ("Graphics Error: Acquire Next Image\n");
		break;

	case AGE_ERROR_GRAPHICS_WAIT_FOR_FENCES:
		printf ("Graphics Error: Wait for Fence\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_FENCE:
		printf ("Graphics Error: Create Fence\n");
		break;

	case AGE_ERROR_GRAPHICS_RESET_FENCE:
		printf ("Graphics Error: Reset Fence\n");
		break;

	case AGE_ERROR_GRAPHICS_QUEUE_SUBMIT:
		printf ("Graphics Error: Queue Submit\n");
		break;

	case AGE_ERROR_GRAPHICS_QUEUE_PRESENT:
		printf ("Graphics Error: Queue Present\n");
		break;

	case AGE_ERROR_GRAPHICS_UPDATE_UNIFORM_BUFFER:
		printf ("Graphics Error: Update Uniform buffer\n");
		break;

	case AGE_ERROR_GRAPHICS_CREATE_SAMPLER:
		printf ("Graphics Error: Create Sampler\n");
		break;

	case AGE_ERROR_GLTF_IMPORT:
		printf ("GLTF Error: Import GLTF File\n");
		break;

	case AGE_ERROR_SYSTEM_ALLOCATE_MEMORY:
		printf ("System Error: Allocate Memory\n");
		break;

	case AGE_ERROR_SYSTEM_TMP_FILE:
		printf ("System Error: Create TMP File\n");
		break;

	default:
		break;
	}

	return AGE_SUCCESS;
}