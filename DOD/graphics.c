#include "graphics.h"

#include <vulkan/vulkan.h>

VkInstance instance;

void graphics_init ()
{
    VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "Asteroids", VK_MAKE_VERSION (1, 0, 0), "AGE", VK_MAKE_VERSION (1, 0, 0), VK_API_VERSION_1_2};
    VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &application_info, 0, NULL, 0, NULL};
    vkCreateInstance (&instance_create_info, NULL, &instance);
}

void graphics_exit ()
{
    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance (instance, NULL);
    }
}
