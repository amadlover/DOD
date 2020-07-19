#include "graphics.h"
#include "utils.h"
#include "error.h"
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
VkPresentModeKHR chosen_present_mode = VK_NULL_HANDLE;
VkDevice graphics_device = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

VkImage* swapchain_images = NULL;
VkImageView* swapchain_image_views = NULL;
size_t num_swapchain_images = 0;

VkQueue graphics_queue = VK_NULL_HANDLE;
VkQueue compute_queue = VK_NULL_HANDLE;
VkQueue transfer_queue = VK_NULL_HANDLE;

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

AGE_RESULT graphics_init (HINSTANCE h_instance, HWND h_wnd)
{
#ifdef _DEBUG 
    is_validation_needed = true;
#elif DEBUG
	is_validation_needed = true;
#else
    is_validation_needed = false;
#endif
	char** requested_instance_layers = NULL;
	size_t num_requested_instance_layer = 0;

	char** requested_instance_extensions = NULL;
	size_t num_requested_instance_extension = 0;

    if (is_validation_needed)
    {
        size_t num_layers = 0;
        vkEnumerateInstanceLayerProperties (&num_layers, NULL);
        VkLayerProperties* layer_properties = (VkLayerProperties*)utils_malloc (sizeof (VkLayerProperties) * num_layers);
        vkEnumerateInstanceLayerProperties (&num_layers, layer_properties);

        for (size_t l = 0; l < num_layers; l++)
        {
            if (strcmp (layer_properties[l].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
            {
                if (requested_instance_layers == NULL)
                {
                    requested_instance_layers = (char**)utils_calloc (1, sizeof (char*));
                }
                else
                {
                    requested_instance_layers = (char**)utils_realloc_zero (requested_instance_layers, sizeof (char*) * num_requested_instance_layer, sizeof (char*) * (num_requested_instance_layer + 1));
                }

                requested_instance_layers[num_requested_instance_layer] = (char*)utils_calloc (strlen ("VK_LAYER_LUNARG_standard_validation") + 1, sizeof (char));
                strcpy (requested_instance_layers[num_requested_instance_layer], "VK_LAYER_LUNARG_standard_validation");
                ++num_requested_instance_layer;

                break;
            }
        }

        utils_free (layer_properties);
    }

	size_t num_extensions = 0;
	vkEnumerateInstanceExtensionProperties (NULL, &num_extensions, NULL);

	VkExtensionProperties* extension_properties = (VkExtensionProperties*)utils_malloc (sizeof (VkExtensionProperties) * num_extensions);
	vkEnumerateInstanceExtensionProperties (NULL, &num_extensions, extension_properties);

	for (size_t e = 0; e < num_extensions; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			if (requested_instance_extensions == NULL)
			{
				requested_instance_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * num_requested_instance_extension, sizeof (char*) * (num_requested_instance_extension + 1));
			}

			requested_instance_extensions[num_requested_instance_extension] = (char*)utils_calloc (strlen (VK_KHR_SURFACE_EXTENSION_NAME) + 1, sizeof (char));
			strcpy (requested_instance_extensions[num_requested_instance_extension], VK_KHR_SURFACE_EXTENSION_NAME);
			++num_requested_instance_extension;
		}
		else if (strcmp (extension_properties[e].extensionName, "VK_KHR_win32_surface") == 0)
		{
			if (requested_instance_extensions == NULL)
			{
				requested_instance_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * num_requested_instance_extension, sizeof (char*) * (num_requested_instance_extension + 1));
			}

			requested_instance_extensions[num_requested_instance_extension] = (char*)utils_calloc (strlen ("VK_KHR_win32_surface") + 1, sizeof (char));
			strcpy (requested_instance_extensions[num_requested_instance_extension], "VK_KHR_win32_surface");
			++num_requested_instance_extension;
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
					requested_instance_extensions = (char**)utils_realloc_zero (requested_instance_extensions, sizeof (char*) * num_requested_instance_extension, sizeof (char*) * (num_requested_instance_extension + 1));
				}

				requested_instance_extensions[num_requested_instance_extension] = (char*)utils_calloc (strlen (VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1, sizeof (char));
				strcpy (requested_instance_extensions[num_requested_instance_extension], VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				++num_requested_instance_extension;
			}
		}
	}

	utils_free (extension_properties);

    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "Asteroids", VK_MAKE_VERSION (1, 0, 0), "AGE", VK_MAKE_VERSION (1, 0, 0), VK_API_VERSION_1_2};
    VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &application_info, num_requested_instance_layer, requested_instance_layers, num_requested_instance_extension, requested_instance_extensions};
    
	AGE_RESULT result = AGE_SUCCESS;
	VkResult res = vkCreateInstance (&instance_create_info, NULL, &instance);
	if (res != VK_SUCCESS)
	{
		result = AGE_ERROR_GRAPHICS_CREATE_INSTANCE;
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

	res = create_debug_utils_messenger (instance, &debug_utils_messenger_create_info, NULL, &debug_utils_messenger);

	if (res != VK_SUCCESS)
	{
		result = AGE_ERROR_GRAPHICS_CREATE_DEBUG_UTILS_MESSENGER;
		goto exit;
	}

	size_t num_physical_devices = 0;
	vkEnumeratePhysicalDevices (instance, &num_physical_devices, NULL);

	if (num_physical_devices == 0)
	{
		result = AGE_ERROR_GRAPHICS_GET_PHYSICAL_DEVICE;
		goto exit;
	}

	VkPhysicalDevice* physical_devices = (VkPhysicalDevice*) utils_malloc (sizeof (VkPhysicalDevice) * num_physical_devices);
	vkEnumeratePhysicalDevices (instance, &num_physical_devices, physical_devices);

	physical_device = physical_devices[0];

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures (physical_device, &device_features);

	size_t num_queue_families = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &num_queue_families, NULL);
	VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)utils_malloc (sizeof (VkQueueFamilyProperties) * num_queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties (physical_device, &num_queue_families, queue_family_properties);

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_queue_family_index = i;
			break;
		}
	}

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && (i != graphics_queue_family_index))
		{
			compute_queue_family_index = i;
			break;
		}
	}

	if (compute_queue_family_index == -1)
	{
		for (size_t i = 0; i < num_queue_families; ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute_queue_family_index = i;
				break;
			}
		}	
	}

	for (size_t i = 0; i < num_queue_families; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (i != graphics_queue_family_index) && (i != compute_queue_family_index))
		{
			transfer_queue_family_index = i;
			break;
		}
	}

	if (transfer_queue_family_index == -1)
	{
		for (size_t i = 0; i < num_queue_families; ++i)
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

	VkWin32SurfaceCreateInfoKHR surface_create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL, 0, h_instance, h_wnd};

	res = vkCreateWin32SurfaceKHR (instance, &surface_create_info, NULL, &surface);

	if (res != VK_SUCCESS)
	{
		result = AGE_ERROR_GRAPHICS_CREATE_SURFACE;
		goto exit;
	}

	VkBool32 is_surface_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR (physical_device, graphics_queue_family_index, surface, &is_surface_supported);

	if (!is_surface_supported)
	{
		result = AGE_ERROR_GRAPHICS_SURFACE_SUPPORT;
		goto exit;
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physical_device, surface, &surface_capabilities);

	size_t num_surface_formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &num_surface_formats, NULL);

	VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)utils_malloc (sizeof (VkSurfaceFormatKHR) * num_surface_formats);
	vkGetPhysicalDeviceSurfaceFormatsKHR (physical_device, surface, &num_surface_formats, surface_formats);

	for (size_t s = 0; s < num_surface_formats; s++)
	{
		if (surface_formats[s].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			chosen_surface_format = surface_formats[s];
			break;
		}
	}

	size_t num_present_modes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &num_present_modes, NULL);

	VkPresentModeKHR* present_modes = (VkPresentModeKHR*)utils_malloc (sizeof (VkPresentModeKHR) * num_present_modes);
	vkGetPhysicalDeviceSurfacePresentModesKHR (physical_device, surface, &num_present_modes, present_modes);

	for (size_t p = 0; p < num_present_modes; p++)
	{
		if (present_modes[p] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosen_present_mode = present_modes[p];
			break;
		}
	}

	char** requested_device_extensions = NULL;
	size_t num_requested_device_extension = 0;

	num_extensions = 0;
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &num_extensions, NULL);

	extension_properties = (VkExtensionProperties*)utils_malloc (sizeof (VkExtensionProperties) * num_extensions);
	vkEnumerateDeviceExtensionProperties (physical_device, NULL, &num_extensions, extension_properties);

	for (size_t e = 0; e < num_extensions; e++)
	{
		if (strcmp (extension_properties[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			if (requested_device_extensions == NULL)
			{
				requested_device_extensions = (char**)utils_calloc (1, sizeof (char*));
			}
			else
			{
				requested_device_extensions = (char**)utils_realloc_zero (requested_device_extensions, sizeof (char*) * num_requested_device_extension, sizeof (char*) * (num_requested_device_extension + 1));
			}

			requested_device_extensions[num_requested_device_extension] = (char*)utils_calloc (strlen (VK_KHR_SWAPCHAIN_EXTENSION_NAME) + 1, sizeof (char));
			strcpy (requested_device_extensions[num_requested_device_extension], VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			++num_requested_device_extension;

			break;
		}
	}

	utils_free (extension_properties);

	float priorities = 1.f;

	VkDeviceQueueCreateInfo queue_create_infos[3] = { 0,0,0 };
	size_t unique_queue_family_indices[3] = { 0,0,0 };
	size_t num_unique_queues[3] = { 1,1,1 };
	size_t num_unique_queue_family_indices = 0;

	if (graphics_queue_family_index == compute_queue_family_index)
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		++num_unique_queue_family_indices;
		++num_unique_queues[0];
	}
	else
	{
		unique_queue_family_indices[0] = graphics_queue_family_index;
		unique_queue_family_indices[1] = compute_queue_family_index;
		num_unique_queue_family_indices += 2;
	}

	if (graphics_queue_family_index != transfer_queue_family_index)
	{
		unique_queue_family_indices[num_unique_queue_family_indices] = transfer_queue_family_index;
		++num_unique_queue_family_indices;
	}

	for (size_t ui = 0; ui < num_unique_queue_family_indices; ++ui)
	{
		queue_create_infos[ui].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[ui].pNext = NULL;
		queue_create_infos[ui].pQueuePriorities = &priorities;
		queue_create_infos[ui].queueCount = num_unique_queues[ui];
		queue_create_infos[ui].queueFamilyIndex = unique_queue_family_indices[ui];
		queue_create_infos[ui].flags = 0;
	}

	VkDeviceCreateInfo device_create_info = {
												VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 
												NULL, 
												0, 
												num_unique_queue_family_indices, 
												queue_create_infos, 
												0,
												NULL,
												num_requested_device_extension,
												requested_device_extensions,
												&device_features
											};
	
	res = vkCreateDevice (physical_device, &device_create_info, NULL, &graphics_device);

	if (res != VK_SUCCESS)
	{
		result = AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_DEVICE;
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
	
	res = vkCreateSwapchainKHR (graphics_device, &swapchain_create_info, NULL, &swapchain);

	if (res != VK_SUCCESS)
	{
		result = AGE_ERROR_GRAPHICS_CREATE_SWAPCHAIN;
		goto exit;
	}
	
	vkGetSwapchainImagesKHR (graphics_device, swapchain, &num_swapchain_images, NULL);
	swapchain_images = (VkImage*) utils_malloc (sizeof (VkImage) * num_swapchain_images);
	vkGetSwapchainImagesKHR (graphics_device, swapchain, &num_swapchain_images, swapchain_images);
	swapchain_image_views = (VkImageView*) utils_malloc (sizeof (VkImageView) * num_swapchain_images);

	VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
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
	
	for (size_t i = 0; i < num_swapchain_images; ++i)
	{
		image_view_create_info.image = swapchain_images[i];
		res = vkCreateImageView (graphics_device, &image_view_create_info, NULL, swapchain_image_views + i);
		
		if (res != VK_SUCCESS)
		{
			result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
			goto exit;
		}
	}

	
	size_t graphics_queue_index = 0;
	size_t compute_queue_index = graphics_queue_family_index == compute_queue_family_index ? 1 : 0;
	size_t transfer_queue_index = transfer_queue_family_index == compute_queue_family_index ? compute_queue_index + 1 : 0;

	vkGetDeviceQueue (graphics_device, graphics_queue_family_index, graphics_queue_index, &graphics_queue);
	vkGetDeviceQueue (graphics_device, compute_queue_family_index, compute_queue_index, &compute_queue);
	vkGetDeviceQueue (graphics_device, transfer_queue_family_index, transfer_queue_index, &transfer_queue);

exit: // clear function specific allocations before exit
	for (size_t i = 0; i < num_requested_instance_layer; ++i)
	{
		utils_free (requested_instance_layers[i]);
	}
	utils_free (requested_instance_layers);

	for (size_t i = 0; i < num_requested_instance_extension; ++i)
	{
		utils_free (requested_instance_extensions[i]);
	}
	utils_free (requested_instance_extensions);

	utils_free (physical_devices);
	utils_free (queue_family_properties);

	utils_free (surface_formats);
	utils_free (present_modes);

	for (size_t i = 0; i < num_requested_device_extension; ++i)
	{
		utils_free (requested_device_extensions[i]);
	}
	utils_free (requested_device_extensions);

    return res;
}

void graphics_exit ()
{
	if (swapchain_image_views)
	{
		for (size_t i = 0; i < num_swapchain_images; i++)
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

	if (graphics_device = VK_NULL_HANDLE)
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
