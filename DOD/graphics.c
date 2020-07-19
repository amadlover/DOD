#include "graphics.h"
#include "utils.h"
#include <stdio.h>
#include <vulkan/vulkan.h>


VkInstance instance = VK_NULL_HANDLE;

bool is_validation_needed = false;

int graphics_init ()
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
    VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &application_info, 0, NULL, 0, NULL};
    if (vkCreateInstance (&instance_create_info, NULL, &instance) != VK_SUCCESS)
    {
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

        return 1;
    }
    else
    {
        printf ("Created Instance\n");
    }

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

    return 0;
}

void graphics_exit ()
{
    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance (instance, NULL);
        printf ("Destroyed Instance\n");
    }
}
