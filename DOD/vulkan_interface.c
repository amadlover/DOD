#include "vulkan_interface.h"
#include "utils.h"

#include <stdio.h>
#include <vulkan/vulkan_win32.h>

bool is_validation_needed = false;
VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
VkSurfaceFormatKHR chosen_surface_format;
VkPresentModeKHR chosen_present_mode;

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
		printf ("Debug Callback Message: %s\n\n", callback_data->pMessage);
	}

	return false;
}


AGE_RESULT vulkan_interface_init (HINSTANCE h_instance, HWND h_wnd)
{
    AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

#ifdef _DEBUG 
	is_validation_needed = true;
#elif DEBUG
	is_validation_needed = true;
#else
	is_validation_needed = false;
#endif

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
			if (strcmp (layer_properties[l].layerName, "VK_LAYER_KHRONOS_validation") == 0)
			{
				if (requested_instance_layers == NULL)
				{
					requested_instance_layers = (char**)utils_calloc (1, sizeof (char*));
				}
				else
				{
					requested_instance_layers = (char**)utils_realloc_zero (requested_instance_layers, sizeof (char*) * requested_instance_layer_count, sizeof (char*) * (requested_instance_layer_count + 1));
				}

				requested_instance_layers[requested_instance_layer_count] = (char*)utils_calloc (strlen ("VK_LAYER_KHRONOS_validation") + 1, sizeof (char));
				strcpy (requested_instance_layers[requested_instance_layer_count], "VK_LAYER_KHRONOS_validation");
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

	VkApplicationInfo application_info = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		"Asteroids",
		VK_MAKE_VERSION (1, 0, 0),
		"AGE",
		VK_MAKE_VERSION (1, 0, 0),
		VK_API_VERSION_1_2
	};

	if (is_validation_needed)
	{
		VkValidationFeatureEnableEXT enables[] = {
			VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
		};

		VkValidationFeaturesEXT features = {
			VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
			NULL,
			1,
			enables,
			0,
			NULL
		};

		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			&features,
			0,
			&application_info,
			requested_instance_layer_count,
			requested_instance_layers,
			requested_instance_extension_count,
			requested_instance_extensions
		};

		vk_result = vkCreateInstance (&instance_create_info, NULL, &instance);
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
	}
	else
	{
		VkInstanceCreateInfo instance_create_info = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			NULL,
			0,
			&application_info,
			requested_instance_layer_count,
			requested_instance_layers,
			requested_instance_extension_count,
			requested_instance_extensions
		};

		AGE_RESULT age_result = AGE_SUCCESS;
		VkResult vk_result = vkCreateInstance (&instance_create_info, NULL, &instance);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_INSTANCE;
			goto exit;
		}
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

	float priorities[3] = { 1.f, 1.f, 1.f };

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
		queue_create_infos[ui].pQueuePriorities = priorities;
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

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		&descriptor_set_layout,
		0,
		NULL
	};

	vk_result = vkCreatePipelineLayout (graphics_device, &pipeline_layout_create_info, NULL, &graphics_pipeline_layout);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
		goto exit;
	}

exit: // clean up allocation made within the function

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

	return age_result;

exit:
    return age_result;
}

void vulkan_interface_shutdown (void)
{

}