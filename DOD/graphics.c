#include "graphics.h"
#include "utils.h"
#include "error.h"
#include "actor_vert.h"
#include "actor_frag.h"
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>


bool is_validation_needed = false;

VkInstance instance = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;
VkPhysicalDevice physical_device = VK_NULL_HANDLE;

uint32_t graphics_queue_family_index = -1;
uint32_t compute_queue_family_index = -1;
uint32_t transfer_queue_family_index = -1;

VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {0};
VkPhysicalDeviceLimits physical_device_limits = {0};
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR surface_capabilities = {0};
VkSurfaceFormatKHR chosen_surface_format = {0};
VkPresentModeKHR chosen_present_mode = -1;
VkDevice graphics_device = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

VkImage* swapchain_images = NULL;
VkImageView* swapchain_image_views = NULL;
size_t swapchain_image_count = 0;

VkQueue graphics_queue = VK_NULL_HANDLE;
VkQueue compute_queue = VK_NULL_HANDLE;
VkQueue transfer_queue = VK_NULL_HANDLE;

VkRenderPass render_pass = VK_NULL_HANDLE;
VkFramebuffer* swapchain_framebuffers = NULL;

VkCommandPool swapchain_command_pool = VK_NULL_HANDLE;
VkCommandBuffer* swapchain_command_buffers = NULL;

VkSemaphore wait_semaphore = VK_NULL_HANDLE;
VkSemaphore* swapchain_signal_semaphores = NULL;
VkFence* swapchain_fences = NULL;

VkBuffer vertex_index_buffer = VK_NULL_HANDLE;
VkDeviceMemory vertex_index_memory = VK_NULL_HANDLE;

VkPipelineLayout graphics_pipeline_layout = VK_NULL_HANDLE;
VkPipeline graphics_pipeline = VK_NULL_HANDLE;
VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
size_t descriptor_set_count = 0;

const vec2** game_actors_positions = NULL;
const size_t* game_actor_count = 0;

float background_positions_colors[24] = { -10,-10, 10,-10, 10,10, -10,10, 0.5,0.5,0.5, 0.5,0.5,0.5, 0.5,0.5,0.5, 0.5,0.5,0.5};
size_t background_positions_size = sizeof (float) * 8;
size_t background_colors_size = sizeof (float) * 12;
size_t background_indices[6] = { 0,1,2, 0,2,3 };
size_t background_indices_size = sizeof (background_indices);

float actor_local_positions_colors[15] = { -1,0, 1,0, 0,1,  1,0,0, 0,1,0, 0,0,1 };
size_t actor_local_positions_size = sizeof (float) * 6;
size_t actor_local_colors_size = sizeof (float) * 9;
size_t actor_indices[3] = { 0,1,2 };
size_t actor_indices_size = sizeof (actor_indices);
size_t actor_index_count = 3;

VkResult create_debug_utils_messenger (VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* debug_utils_messenger_create_info,
	const VkAllocationCallbacks* allocation_callbacks,
	VkDebugUtilsMessengerEXT* debug_utils_messenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");

	if (func)
	{
		return func (instance, debug_utils_messenger_create_info, allocation_callbacks, debug_utils_messenger);
	}
	else
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
}

void destroy_debug_utils_messenger (VkInstance instance,
	VkDebugUtilsMessengerEXT debug_utils_messenger,
	const VkAllocationCallbacks* allocation_callbacks)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func)
	{
		func (instance, debug_utils_messenger, allocation_callbacks);
	}
	else
	{
		printf ("Could not destroy debug utils messenger\n");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback (
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* pUserData)
{
	if (callback_data)
	{
		printf ("Debug Callback Message: %s\n", callback_data->pMessage);
	}

	return false;
}

AGE_RESULT graphics_init (HINSTANCE h_instance, HWND h_wnd, const vec2** actor_positions, const size_t* actor_count)
{
#ifdef _DEBUG 
	is_validation_needed = true;
#elif DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

	game_actor_count = actor_count;
	game_actors_positions = actor_positions;

	char** requested_instance_layers = NULL;
	size_t requested_instance_layer_count = 0;

	char** requested_instance_extensions = NULL;
	size_t requested_instance_extension_count = 0;

	if (is_validation_needed)
	{
		size_t layer_count = 0;
		vkEnumerateInstanceLayerProperties (&layer_count, NULL);
		VkLayerProperties* layer_properties = (VkLayerProperties*)utils_calloc (layer_count, sizeof (VkLayerProperties));
		vkEnumerateInstanceLayerProperties (&layer_count, layer_properties);

		for (size_t l = 0; l < layer_count; l++)
		{
			if (strcmp (layer_properties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
			{
				if (requested_instance_layers == NULL)
				{
					requested_instance_layers = (char**)utils_calloc (1, sizeof (char*));
				}
				else
				{
					requested_instance_layers = (char**)utils_realloc_zero (requested_instance_layers, sizeof (char*) * requested_instance_layer_count, sizeof (char*) * (requested_instance_layer_count + 1));
				}

				requested_instance_layers[requested_instance_layer_count] = (char*)utils_calloc (strlen ("VK_LAYER_LUNARG_standard_validation") + 1, sizeof (char));
				strcpy (requested_instance_layers[requested_instance_layer_count], "VK_LAYER_LUNARG_standard_validation");
				++requested_instance_layer_count;

				break;
			}
		}

		utils_free (layer_properties);
	}

	size_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)utils_calloc (extension_count, sizeof (VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties (NULL, &extension_count, extension_properties);

	for (size_t e = 0; e < extension_count; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			if (requested_instance_extensions == NULL)
			{
				requested_instance_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * requested_instance_extension_count, sizeof (char*) * (requested_instance_extension_count + 1));
			}

			requested_instance_extensions[requested_instance_extension_count] = (char*)utils_calloc (strlen (VK_KHR_SURFACE_EXTENSION_NAME) + 1, sizeof (char));
			strcpy (requested_instance_extensions[requested_instance_extension_count], VK_KHR_SURFACE_EXTENSION_NAME);
			++requested_instance_extension_count;
		}
		else if (strcmp (extension_properties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			if (requested_instance_extensions == NULL)
			{
				requested_instance_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * requested_instance_extension_count, sizeof (char*) * (requested_instance_extension_count + 1));
			}

			requested_instance_extensions[requested_instance_extension_count] = (char*)utils_calloc (strlen ("VK_KHR_win32_surface") + 1, sizeof (char));
			strcpy (requested_instance_extensions[requested_instance_extension_count], "VK_KHR_win32_surface");
			++requested_instance_extension_count;
		}

		if (is_validation_needed)
		{
			if (strcmp (extension_properties[e].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				if (requested_instance_extensions == NULL)
				{
					requested_instance_extensions = (char**)utils_calloc (1, sizeof (char*));
				}
				else
				{
					requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * requested_instance_extension_count, sizeof (char*) * (requested_instance_extension_count + 1));
				}

				requested_instance_extensions[requested_instance_extension_count] = (char*)utils_calloc (strlen (VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1, sizeof (char));
				strcpy (requested_instance_extensions[requested_instance_extension_count], VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				++requested_instance_extension_count;
			}
		}
	}

	utils_free (extension_properties);

	VkApplicationInfo application_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "Asteroids", VK_MAKE_VERSION (1, 0, 0), "AGE", VK_MAKE_VERSION (1, 0, 0), VK_API_VERSION_1_2 };
	VkInstanceCreateInfo instance_create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &application_info, requested_instance_layer_count, requested_instance_layers, requested_instance_extension_count, requested_instance_extensions };

	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = vkCreateInstance (&instance_create_info, NULL, &instance);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_INSTANCE;
		goto exit;
	}

	VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		NULL,
		0,
		/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		debug_messenger_callback,
		NULL
	};

	vk_result = create_debug_utils_messenger (instance, &debug_utils_messenger_create_info, NULL, &debug_utils_messenger);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
		goto exit;
	}

	size_t physical_device_count = 0;
	vkEnumeratePhysicalDevices (instance, &physical_device_count, NULL);

	if (physical_device_count == 0)
	{
		age_result = AGE_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
		goto exit;
	}

	VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)utils_calloc (physical_device_count, sizeof (VkPhysicalDevice));
	vkEnumeratePhysicalDevices (instance, &physical_device_count, physical_devices);

	physical_device = physical_devices[0];

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures (physical_device, &device_features);

	size_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)utils_calloc (queue_family_count, sizeof (VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &queue_family_count, queue_family_properties);

	for (size_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	for (size_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && (i != graphics_queue_family_index))
		{
			compute_queue_family_index = i;
			break;
		}
	}

	if (compute_queue_family_index == -1)
	{
		for (size_t i = 0; i < queue_family_count; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute_queue_family_index = i;
				break;
			}
		}
	}

	for (size_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (i != graphics_queue_family_index) && (i != compute_queue_family_index))
		{
			transfer_queue_family_index = i;
			break;
		}
	}

	if (transfer_queue_family_index == -1)
	{
		for (size_t i = 0; i < queue_family_count; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				transfer_queue_family_index = i;
				break;
			}
		}
	}

	vkGetPhysicalDeviceMemoryProperties (physical_device, &physical_device_memory_properties);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties (physical_device, &device_properties);
	physical_device_limits = device_properties.limits;

	VkWin32SurfaceCreateInfoKHR surface_create_info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL, 0, h_instance, h_wnd };

	vk_result = vkCreateWin32SurfaceKHR (instance, &surface_create_info, NULL, &surface);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SURFACE;
		goto exit;
	}

	VkBool32 is_surface_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

	if (!is_surface_supported)
	{
		age_result = AGE_ERROR_GRAPHICS_SURFACE_SUPPORT;
		goto exit;
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilities);

	size_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)utils_calloc (surface_format_count, sizeof (VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &surface_format_count, surface_formats);

	for (size_t s = 0; s < surface_format_count; s++)
	{
		if (surface_formats[s].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			chosen_surface_format = surface_formats[s];
			break;
		}
	}

	size_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, NULL);

	VkPresentModeKHR* present_modes = (VkPresentModeKHR*)utils_calloc (present_mode_count, sizeof (VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &present_mode_count, present_modes);

	for (size_t p = 0; p < present_mode_count; p++)
	{
		if (present_modes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = present_modes[p];
			break;
		}
	}

	if (chosen_present_mode == -1)
	{
		chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	char** requested_device_extensions = NULL;
	size_t requested_device_extension_count = 0;

	extension_count = 0;
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, NULL);

	extension_properties = (VkExtensionProperties*)utils_calloc (extension_count, sizeof (VkExtensionProperties));
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &extension_count, extension_properties);

	for (size_t e = 0; e < extension_count; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			if (requested_device_extensions == NULL)
			{
				requested_device_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_device_extensions = (char**)utils_realloc_zero (requested_device_extensions, sizeof (char*) * requested_device_extension_count, sizeof (char*) * (requested_device_extension_count + 1));
			}

			requested_device_extensions[requested_device_extension_count] = (char*)utils_calloc (strlen (VK_KHR_SWAPCHAIN_EXTENSION_NAME) + 1, sizeof (char));
			strcpy (requested_device_extensions[requested_device_extension_count], VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			++requested_device_extension_count;

			break;
		}
	}

	utils_free (extension_properties);

	float priorities = 1.f;

	VkDeviceQueueCreateInfo queue_create_infos[3] = { 0,0,0 };
	size_t unique_queue_family_indices[3] = { 0,0,0 };
	size_t unique_queue_count[3] = { 1,1,1 };
	size_t unique_queue_family_index_count = 0;

	if (graphics_queue_family_index == compute_queue_family_index)
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		++unique_queue_family_index_count;
		++unique_queue_count[0];
	}
	else
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		unique_queue_family_indices[1] = compute_queue_family_index;
		unique_queue_family_index_count += 2;
	}

	if (graphics_queue_family_index != transfer_queue_family_index)
	{
		unique_queue_family_indices[unique_queue_family_index_count] = transfer_queue_family_index;
		++unique_queue_family_index_count;
	}

	for (size_t ui = 0; ui < unique_queue_family_index_count; ++ui)
	{
		queue_create_infos[ui].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[ui].pNext = NULL;
		queue_create_infos[ui].pQueuePriorities = &priorities;
		queue_create_infos[ui].queueCount = unique_queue_count[ui];
		queue_create_infos[ui].queueFamilyIndex = unique_queue_family_indices[ui];
		queue_create_infos[ui].flags = 0;
	}

	VkDeviceCreateInfo device_create_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		NULL,
		0,
		unique_queue_family_index_count,
		queue_create_infos,
		0,
		NULL,
		requested_device_extension_count,
		requested_device_extensions,
		&device_features
	};

	vk_result = vkCreateDevice (physical_device, &device_create_info, NULL, &graphics_device);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
		goto exit;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		NULL,
		0,
		surface,
		surface_capabilities.minImageCount + 1,
		chosen_surface_format.format,
		chosen_surface_format.colorSpace,
		surface_capabilities.currentExtent,
		1,
		surface_capabilities.supportedUsageFlags,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		surface_capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		chosen_present_mode,
		1,
		VK_NULL_HANDLE
	};

	vk_result = vkCreateSwapchainKHR (graphics_device, &swapchain_create_info, NULL, &swapchain);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SWAPCHAIN;
		goto exit;
	}

	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, NULL);
	swapchain_images = (VkImage*)utils_calloc (swapchain_image_count, sizeof (VkImage));
	vkGetSwapchainImagesKHR (graphics_device, swapchain, &swapchain_image_count, swapchain_images);
	swapchain_image_views = (VkImageView*)utils_calloc (swapchain_image_count, sizeof (VkImageView));

	VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	VkImageViewCreateInfo image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		VK_NULL_HANDLE,
		VK_IMAGE_VIEW_TYPE_2D,
		chosen_surface_format.format,
		{0},
		subresource_range
	};

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		image_view_create_info.image = swapchain_images[i];
		vk_result = vkCreateImageView (graphics_device, &image_view_create_info, NULL, swapchain_image_views + i);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
			goto exit;
		}
	}


	size_t graphics_queue_index = 0;
	size_t compute_queue_index = graphics_queue_family_index == compute_queue_family_index ? 1 : 0;
	size_t transfer_queue_index = transfer_queue_family_index == compute_queue_family_index ? compute_queue_index + 1 : 0;

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, graphics_queue_index, &graphics_queue);
	vkGetDeviceQueue (graphics_device, compute_queue_family_index, compute_queue_index, &compute_queue);
	vkGetDeviceQueue (graphics_device, transfer_queue_family_index, transfer_queue_index, &transfer_queue);

	VkAttachmentDescription color_attachment_description = {
		0,
		chosen_surface_format.format,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference color_attachment_reference = {
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription color_subpass_description = {
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		NULL,
		1,
		&color_attachment_reference,
		NULL,
		NULL,
		0,
		NULL
	};

	VkRenderPassCreateInfo render_pass_create_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		1,
		&color_attachment_description,
		1,
		&color_subpass_description,
		0,
		NULL
	};

	vk_result = vkCreateRenderPass (graphics_device, &render_pass_create_info, NULL, &render_pass);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_RENDER_PASS;
		goto exit;
	}

	swapchain_framebuffers = (VkFramebuffer*)utils_calloc (swapchain_image_count, sizeof (VkFramebuffer));

	VkFramebufferCreateInfo framebuffer_create_info = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		NULL,
		0,
		render_pass,
		1,
		NULL,
		surface_capabilities.currentExtent.width,
		surface_capabilities.currentExtent.height,
		1
	};

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		framebuffer_create_info.pAttachments = swapchain_image_views + i;

		vk_result = vkCreateFramebuffer (graphics_device, &framebuffer_create_info, NULL, swapchain_framebuffers + i);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
			goto exit;
		}
	}

	VkCommandPoolCreateInfo command_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};

	vk_result = vkCreateCommandPool (graphics_device, &command_pool_create_info, NULL, &swapchain_command_pool);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
		goto exit;
	}

	swapchain_command_buffers = (VkCommandBuffer*)utils_calloc (swapchain_image_count, sizeof (VkCommandBuffer));

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		swapchain_command_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		swapchain_image_count
	};

	vk_result = vkAllocateCommandBuffers (graphics_device, &command_buffer_allocate_info, swapchain_command_buffers);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, NULL, 0 };
	vk_result = vkCreateSemaphore (graphics_device, &semaphore_create_info, NULL, &wait_semaphore);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SEMAPHORE;
		goto exit;
	}

	swapchain_signal_semaphores = (VkSemaphore*)utils_calloc (swapchain_image_count, sizeof (VkSemaphore));
	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkCreateSemaphore (graphics_device, &semaphore_create_info, NULL, swapchain_signal_semaphores + i);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_SEMAPHORE;
			goto exit;
		}
	}

	swapchain_fences = (VkFence*)utils_calloc (swapchain_image_count, sizeof (VkFence));
	VkFenceCreateInfo fence_create_info = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		0
	};

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkCreateFence (graphics_device, &fence_create_info, NULL, swapchain_fences + i);
		
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_FENCE;
			goto exit;
		}
	}

	VkBuffer staging_vertex_index_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_vertex_index_memory = VK_NULL_HANDLE;

	VkBufferCreateInfo vertex_index_buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		(VkDeviceSize)actor_local_positions_size + 
		(VkDeviceSize)actor_local_colors_size + 
		(VkDeviceSize)actor_indices_size + 
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_colors_size +
		(VkDeviceSize)background_indices_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL
	};

	vk_result = vkCreateBuffer (graphics_device, &vertex_index_buffer_create_info, NULL, &staging_vertex_index_buffer);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (graphics_device, staging_vertex_index_buffer, &memory_requirements);

	uint32_t required_memory_types = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	uint32_t required_memory_type_index = 0;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_memory_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			required_memory_type_index = i;
			break;
		}
	}

	VkMemoryAllocateInfo memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		required_memory_type_index
	};

	vk_result = vkAllocateMemory (graphics_device, &memory_allocate_info, NULL, &staging_vertex_index_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (graphics_device, staging_vertex_index_buffer, staging_vertex_index_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	void* data = NULL;

	vk_result = vkMapMemory (
		graphics_device, 
		staging_vertex_index_memory, 
		0, 
		actor_local_positions_size + actor_local_colors_size, 
		0, 
		&data
	);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, actor_local_positions_colors, actor_local_positions_size + actor_local_colors_size);
	vkUnmapMemory (graphics_device, staging_vertex_index_memory);

	vk_result = vkMapMemory (
		graphics_device, 
		staging_vertex_index_memory, 
		actor_local_positions_size + actor_local_colors_size, 
		actor_indices_size, 
		0, 
		&data
	);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, actor_indices, actor_indices_size);
	vkUnmapMemory (graphics_device, staging_vertex_index_memory);

	vk_result = vkMapMemory (
		graphics_device, 
		staging_vertex_index_memory, 
		actor_local_positions_size + actor_local_colors_size + actor_indices_size, 
		background_positions_size + background_colors_size, 
		0, 
		&data
	);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, background_positions_colors, background_positions_size + background_colors_size);
	vkUnmapMemory (graphics_device, staging_vertex_index_memory);

	vk_result = vkMapMemory (
		graphics_device,
		staging_vertex_index_memory, 
		actor_local_positions_size + actor_indices_size + background_positions_size + background_colors_size, 
		background_indices_size, 
		0, 
		&data
	);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, background_indices, background_indices_size);
	vkUnmapMemory (graphics_device, staging_vertex_index_memory);
		
	vertex_index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vk_result = vkCreateBuffer (graphics_device, &vertex_index_buffer_create_info, NULL, &vertex_index_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	vkGetBufferMemoryRequirements (graphics_device, vertex_index_buffer, &memory_requirements);

	required_memory_types = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	required_memory_type_index = 0;

	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_memory_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			required_memory_type_index = i;
			break;
		}
	}

	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = required_memory_type_index;

	vk_result = vkAllocateMemory (graphics_device, &memory_allocate_info, NULL, &vertex_index_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (graphics_device, vertex_index_buffer, vertex_index_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	VkCommandPool transfer_command_pool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo transfer_command_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		transfer_queue_family_index
	};
	
	vk_result = vkCreateCommandPool (graphics_device, &transfer_command_pool_create_info, NULL, &transfer_command_pool);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_COMMAND_POOL;
		goto exit;
	}

	VkCommandBuffer copy_command_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo copy_command_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		transfer_command_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	vk_result = vkAllocateCommandBuffers (graphics_device, &copy_command_buffer_allocate_info, &copy_command_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	VkCommandBufferBeginInfo copy_command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};

	vk_result = vkBeginCommandBuffer (copy_command_buffer, &copy_command_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		goto exit;
	}

	VkBufferCopy buffer_copy = { 0, 0, (VkDeviceSize)actor_local_positions_size + (VkDeviceSize)actor_indices_size };
	vkCmdCopyBuffer (copy_command_buffer, staging_vertex_index_buffer, vertex_index_buffer, 1, &buffer_copy);

	vk_result = vkEndCommandBuffer (copy_command_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		goto exit;
	}

	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		0,
		NULL,
		0,
		1,
		&copy_command_buffer,
		0,
		NULL
	};

	vk_result = vkQueueSubmit (transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_QUEUE_SUBMIT;
		goto exit;
	}

	vkQueueWaitIdle (transfer_queue);

	VkDescriptorPoolSize descriptor_pool_size = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		1
	};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		1,
		1,
		&descriptor_pool_size
	};
	vk_result = vkCreateDescriptorPool (graphics_device, &descriptor_pool_create_info, NULL, &descriptor_pool);

	VkDescriptorSetLayoutBinding descriptor_layout_binding = {
		0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		1,
		VK_SHADER_STAGE_VERTEX_BIT,
		NULL
	};

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		&descriptor_layout_binding
	};

	VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;

	vk_result = vkCreateDescriptorSetLayout (graphics_device, &descriptor_set_layout_create_info, NULL, &descriptor_set_layout);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
		goto exit;
	}

	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptor_pool,
		1,
		&descriptor_set_layout
	};
	
	vk_result = vkAllocateDescriptorSets (graphics_device, &descriptor_set_allocate_info, &descriptor_set);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
		goto exit;
	}

	VkDescriptorBufferInfo buffer_info = { 0 };

	VkWriteDescriptorSet descriptor_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		NULL,
		0,
		0,
		0,
		1,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		NULL,
		&buffer_info,
		NULL
	};
	
//	vkUpdateDescriptorSets (graphics_device, 1, &descriptor_write, 0, NULL);

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
		0,
		NULL
	};

	vk_result = vkCreatePipelineLayout (graphics_device, &pipeline_layout_create_info, NULL, &graphics_pipeline_layout);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
		goto exit;
	}

	VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
	VkShaderModuleCreateInfo vertex_shader_module_create_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		sizeof (actor_vert),
		actor_vert
	};
	vk_result = vkCreateShaderModule (graphics_device, &vertex_shader_module_create_info, NULL, &vertex_shader_module);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
		goto exit;
	}

	VkShaderModule fragment_shader_module = VK_NULL_HANDLE;
	VkShaderModuleCreateInfo fragment_shader_module_create_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		sizeof (actor_frag),
		actor_frag
	};

	vk_result = vkCreateShaderModule (graphics_device, &fragment_shader_module_create_info, NULL, &fragment_shader_module);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
		goto exit;
	}

	VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			NULL,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			vertex_shader_module,
			"main",
			NULL
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			NULL,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			fragment_shader_module,
			"main",
			NULL
		}
	};
	VkVertexInputBindingDescription vertex_input_binding_descriptions[2] = {
		{
			0,
			sizeof (float) * 2,
			VK_VERTEX_INPUT_RATE_VERTEX
		},
		{
			1,
			sizeof (float) * 3,
			VK_VERTEX_INPUT_RATE_VERTEX
		}
	};
	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2] = {
		{
			0,
			0,
			VK_FORMAT_R32G32_SFLOAT,
			0
		}, 
		{
			1,
			1,
			VK_FORMAT_R32G32B32_SFLOAT,
			0
		}
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		NULL,
		0,
		2,
		vertex_input_binding_descriptions,
		2,
		vertex_input_attribute_descriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo vertex_input_assembly_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE
	};

	VkViewport viewport = {
		0,
		0,
		(float)surface_capabilities.currentExtent.width,
		(float)surface_capabilities.currentExtent.height,
		0,
		1
	};
	VkRect2D scissor = {
		{0,0},
		surface_capabilities.currentExtent
	};
	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		NULL,
		0,
		1,
		&viewport,
		1,
		&scissor
	};
	
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		NULL,
		0,
		VK_FALSE,
		VK_TRUE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE,
		0,
		0,
		0,
		1
	};
	
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		NULL,
		0,
		VK_SAMPLE_COUNT_1_BIT
	};
	
	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		VK_FALSE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		NULL,
		0,
		VK_FALSE,
		VK_LOGIC_OP_NO_OP,
		1,
		&color_blend_attachment_state,
		{1,1,1,1}
	};

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		0,
		2,
		shader_stage_create_infos,
		&vertex_input_state_create_info,
		&vertex_input_assembly_state_create_info,
		NULL,
		&viewport_state_create_info,
		&rasterization_state_create_info,
		&multisample_state_create_info,
		NULL,
		&color_blend_state_create_info,
		NULL,
		graphics_pipeline_layout,
		render_pass,
		0,
		VK_NULL_HANDLE,
		0
	};

	vk_result = vkCreateGraphicsPipelines (graphics_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &graphics_pipeline);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
		goto exit;
	}

	age_result = graphics_update_command_buffers ();
    if (age_result != AGE_SUCCESS)
    {
        goto exit;
    }

exit: // clear function specific allocations before exit
	for (size_t i = 0; i < requested_instance_layer_count; ++i)
	{
		utils_free (requested_instance_layers[i]);
	}
	utils_free (requested_instance_layers);

	for (size_t i = 0; i < requested_instance_extension_count; ++i)
	{
		utils_free (requested_instance_extensions[i]);
	}
	utils_free (requested_instance_extensions);

	utils_free (physical_devices);
	utils_free (queue_family_properties);

	utils_free (surface_formats);
	utils_free (present_modes);

	for (size_t i = 0; i < requested_device_extension_count; ++i)
	{
		utils_free (requested_device_extensions[i]);
	}
	utils_free (requested_device_extensions);

	if (staging_vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, staging_vertex_index_buffer, NULL);
	}
	
	if (staging_vertex_index_memory != VK_NULL_HANDLE)
	{
		 vkFreeMemory (graphics_device, staging_vertex_index_memory, NULL);
	}

	if (copy_command_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (graphics_device, transfer_command_pool, 1, &copy_command_buffer);
	}

	if (transfer_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (graphics_device, transfer_command_pool, NULL);
	}

	if (descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (graphics_device, descriptor_set_layout, NULL);
	}

	if (vertex_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (graphics_device, vertex_shader_module, NULL);
	}

	if (fragment_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (graphics_device, fragment_shader_module, NULL);
	}

	return age_result;
}

AGE_RESULT graphics_update_command_buffers (void)
{
	printf ("graphics_update_command_buffers\n");
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		NULL
	};

	VkRenderPassBeginInfo render_pass_begin_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		render_pass,
		VK_NULL_HANDLE,
		{ {0,0}, surface_capabilities.currentExtent },
		0,
		NULL
	};

	/*vk_result = vkWaitForFences (graphics_device, swapchain_image_count, swapchain_fences, VK_FALSE, UINT64_MAX);
	if (vk_result != VK_SUCCESS && vk_result != VK_TIMEOUT)
	{
		age_result = AGE_ERROR_GRAPHICS_WAIT_FOR_FENCES;
		goto exit;
	}*/

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		/*vk_result = vkGetFenceStatus (graphics_device, swapchain_fences[i]);
		if (vk_result != VK_SUCCESS)
		{
			continue;
		}

		vk_result = vkResetFences (graphics_device, 1, swapchain_fences + i);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_RESET_FENCES;
			goto exit;
		}*/

		vk_result = vkBeginCommandBuffer (swapchain_command_buffers[i], &command_buffer_begin_info);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
			goto exit;
		}

		render_pass_begin_info.framebuffer = swapchain_framebuffers[i];

		vkCmdBeginRenderPass (swapchain_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
		uint32_t dynamic_offset = 0;
		//vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &descriptor_set, 1, &dynamic_offset);

		VkDeviceSize offsets[6] = {
			0, 
			actor_local_positions_size,
			actor_local_positions_size + actor_local_colors_size, 
			actor_local_positions_size + actor_local_colors_size + actor_indices_size,
			actor_local_positions_size + actor_local_colors_size + actor_indices_size + background_positions_size,
			actor_local_positions_size + actor_local_colors_size + actor_indices_size + background_positions_size + background_colors_size
		};
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &offsets[3]);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &offsets[4]);
		vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, offsets[5], VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (swapchain_command_buffers[i], 6, 1, 0, 0, 0);

		/*for (size_t a = 0; a < *game_actor_count; ++a)
		{

		}*/

		vkCmdEndRenderPass (swapchain_command_buffers[i]);

		vk_result = vkEndCommandBuffer (swapchain_command_buffers[i]);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER;
			goto exit;
		}
	}

exit: // clear function specific allocations before exit
	return age_result;
}

AGE_RESULT graphics_submit_present (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;

	size_t image_index = 0;
	VkResult vk_result = vkAcquireNextImageKHR (graphics_device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &image_index);

	if (vk_result != VK_SUCCESS)
	{
		if (vk_result == VK_SUBOPTIMAL_KHR ||
			vk_result == VK_ERROR_OUT_OF_DATE_KHR ||
			vk_result == VK_TIMEOUT ||
			vk_result == VK_NOT_READY)
		{
			age_result = AGE_SUCCESS;
			goto exit;
		}
		else
		{
			vk_result = AGE_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
			goto exit;
		}
	}

	VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		1,
		&wait_semaphore,
		&wait_stage_mask,
		1,
		swapchain_command_buffers + image_index,
		1,
		swapchain_signal_semaphores + image_index,
	};

	vk_result = vkQueueSubmit (graphics_queue, 1, &submit_info, swapchain_fences[image_index]);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_QUEUE_SUBMIT;
		goto exit;
	}

	VkPresentInfoKHR present_info = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		NULL,
		1,
		swapchain_signal_semaphores + image_index,
		1,
		&swapchain,
		&image_index,
		NULL
	};

	vk_result = vkQueuePresentKHR (graphics_queue, &present_info);

	if (vk_result != VK_SUCCESS)
	{
		if (vk_result == VK_ERROR_OUT_OF_HOST_MEMORY || vk_result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		{
			age_result = AGE_ERROR_GRAPHICS_QUEUE_PRESENT;
			goto exit;
		}
	}

	vk_result = vkWaitForFences (graphics_device, 1, swapchain_fences + image_index, VK_TRUE, UINT64_MAX);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_WAIT_FOR_FENCES;
		goto exit;
	}

	vk_result = vkResetFences (graphics_device, 1, swapchain_fences + image_index);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_RESET_FENCES;
		goto exit;
	}

exit:
	return age_result;
}

void graphics_exit (void)
{
	vkQueueWaitIdle (graphics_queue);

	if (descriptor_set != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (graphics_device, descriptor_pool, 1, &descriptor_set);
	}

	if (descriptor_pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool (graphics_device, descriptor_pool, NULL);
	}

	if (swapchain_fences)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroyFence (graphics_device, swapchain_fences[i], NULL);
		}

		utils_free (swapchain_fences);
	}

	if (graphics_pipeline_layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (graphics_device, graphics_pipeline_layout, NULL);
	}

	if (graphics_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (graphics_device, graphics_pipeline, NULL);
	}

	if (vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, vertex_index_buffer, NULL);
	}

	if (vertex_index_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, vertex_index_memory, NULL);
	}

	if (wait_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (graphics_device, wait_semaphore, NULL);
	}

	if (swapchain_signal_semaphores)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroySemaphore (graphics_device, swapchain_signal_semaphores[i], NULL);
		}
		
		utils_free (swapchain_signal_semaphores);
	}

	if (swapchain_command_buffers)
	{
		vkFreeCommandBuffers (graphics_device, swapchain_command_pool, swapchain_image_count, swapchain_command_buffers);

		utils_free (swapchain_command_buffers);
	}

	if (swapchain_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (graphics_device, swapchain_command_pool, NULL);
	}

	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (graphics_device, render_pass, NULL);
	}

	if (swapchain_framebuffers)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			if (swapchain_framebuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (graphics_device, swapchain_framebuffers[i], NULL);
			}
		}

		utils_free (swapchain_framebuffers);
	}

	if (swapchain_image_views)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			if (swapchain_image_views[i] != VK_NULL_HANDLE)
			{
				vkDestroyImageView (graphics_device, swapchain_image_views[i], NULL);
			}
		}

		utils_free (swapchain_image_views);
	}

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR (graphics_device, swapchain, NULL);
	}

	if (graphics_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice (graphics_device, NULL);
	}

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR (instance, surface, NULL);
	}

	if (is_validation_needed)
	{
		if (debug_utils_messenger != VK_NULL_HANDLE)
		{
			destroy_debug_utils_messenger (instance, debug_utils_messenger, NULL);
		}
	}

    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance (instance, NULL);
    }
}

void graphics_check_data_from_game (void)
{
	printf ("GRAPHICS\n");
	for (size_t i = 0; i < *game_actor_count; ++i)
	{
		printf ("Positions n = %d, x = %f, y = %f\n", i, (*game_actors_positions + i)->x, (*game_actors_positions + i)->y);
	}
}
