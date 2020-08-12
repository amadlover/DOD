#include "vulkan_graphics.h"
#include "utils.h"
#include "error.h"
#include "actor_vert.h"
#include "actor_frag.h"
#include "vulkan_interface.h"

#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <stb_image.h>


VkRenderPass render_pass = VK_NULL_HANDLE;
VkFramebuffer* swapchain_framebuffers = NULL;

VkCommandPool swapchain_command_pool = VK_NULL_HANDLE;
VkCommandBuffer* swapchain_command_buffers = NULL;

VkSemaphore wait_semaphore = VK_NULL_HANDLE;
VkSemaphore* swapchain_signal_semaphores = NULL;
VkFence* swapchain_fences = NULL;

VkBuffer vertex_index_buffer = VK_NULL_HANDLE;
VkDeviceMemory vertex_index_buffer_memory = VK_NULL_HANDLE;

VkPipelineLayout graphics_pipeline_layout = VK_NULL_HANDLE;
VkPipeline graphics_pipeline = VK_NULL_HANDLE;
VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSetLayout transform_descriptor_set_layout = VK_NULL_HANDLE;
VkDescriptorSet transform_descriptor_set = VK_NULL_HANDLE;
VkDescriptorSetLayout texture_descriptor_set_layout = VK_NULL_HANDLE;
VkDescriptorSet texture_descriptor_set = VK_NULL_HANDLE;
VkSampler common_sampler = VK_NULL_HANDLE;

VkBuffer transforms_buffer = VK_NULL_HANDLE;
VkDeviceMemory transforms_buffer_memory = VK_NULL_HANDLE;

size_t aligned_size_per_transform = 0;
size_t total_transforms_size = 0;

void* transforms_aligned_data = NULL;
void* transforms_mapped_data = NULL;

float screen_aspect_ratio = 1;

float background_positions[12] = { -1,-1,0.9f, 1,-1,0.9f, 1,1,0.9f, -1,1,0.9f };
size_t background_positions_size = sizeof (background_positions);
float background_uvs[8] = { 0,0, 1,0, 1,1, 0,1 };
size_t background_uvs_size = sizeof (background_uvs);
size_t background_indices[6] = { 0,1,2, 0,2,3 };
size_t background_indices_size = sizeof (background_indices);
size_t background_index_count = 6;


float actor_positions[12] = { -0.1f,-0.1f,0.5f, 0.1f,-0.1f,0.5f, 0.1f,0.1f,0.5f, -0.1f,0.1f,0.5f };
size_t actor_positions_size = sizeof (actor_positions);
float actor_uvs[8] = { 1,1, 0,1, 0,0, 1,0 };
size_t actor_uvs_size = sizeof (actor_uvs);
size_t actor_indices[6] = { 0,1,2, 0,2,3 };
size_t actor_indices_size = sizeof (actor_indices);
size_t actor_index_count = 6;


VkImage background_image = VK_NULL_HANDLE;
VkImageView background_image_view = VK_NULL_HANDLE;
size_t background_image_size = 0;
VkImage player_image = VK_NULL_HANDLE;
VkImageView player_image_view = VK_NULL_HANDLE;
size_t player_image_size = 0;
VkImage asteroid_image = VK_NULL_HANDLE;
VkImageView asteroid_image_view = VK_NULL_HANDLE;
size_t asteroid_image_size = 0;
VkImage bullet_image = VK_NULL_HANDLE;
VkImageView bullet_image_view = VK_NULL_HANDLE;
size_t bullet_image_size = 0;

VkDeviceMemory images_memory = VK_NULL_HANDLE;


AGE_RESULT graphics_create_geometry_buffers (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkBuffer staging_vertex_index_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_vertex_index_memory = VK_NULL_HANDLE;

	VkBufferCreateInfo vertex_index_buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL
	};

	vk_result = vkCreateBuffer (device, &vertex_index_buffer_create_info, NULL, &staging_vertex_index_buffer);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (device, staging_vertex_index_buffer, &memory_requirements);

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

	vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, &staging_vertex_index_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (device, staging_vertex_index_buffer, staging_vertex_index_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	void* data = NULL;

	vk_result = vkMapMemory (
		device,
		staging_vertex_index_memory,
		0,
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size,
		0,
		&data
	);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, background_positions, background_positions_size);
	memcpy ((char*)data + background_positions_size, background_uvs, background_uvs_size);
	memcpy ((char*)data + background_positions_size + background_uvs_size, background_indices, background_indices_size);
	memcpy ((char*)data + background_positions_size + background_uvs_size + background_indices_size, actor_positions, actor_positions_size);
	memcpy ((char*)data + background_positions_size + background_uvs_size + background_indices_size + actor_positions_size, actor_uvs, actor_uvs_size);
	memcpy ((char*)data + background_positions_size + background_uvs_size + background_indices_size + actor_positions_size + actor_uvs_size, actor_indices, actor_indices_size);

	vkUnmapMemory (device, staging_vertex_index_memory);

	vertex_index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	vk_result = vkCreateBuffer (device, &vertex_index_buffer_create_info, NULL, &vertex_index_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	vkGetBufferMemoryRequirements (device, vertex_index_buffer, &memory_requirements);

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

	vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, &vertex_index_buffer_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (device, vertex_index_buffer, vertex_index_buffer_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	VkCommandBuffer copy_command_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo copy_cmd_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		transfer_command_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	vk_result = vkAllocateCommandBuffers (device, &copy_cmd_buffer_allocate_info, &copy_command_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	VkCommandBufferBeginInfo copy_cmd_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};

	vk_result = vkBeginCommandBuffer (copy_command_buffer, &copy_cmd_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		goto exit;
	}

	VkBufferCopy buffer_copy = {
		0,
		0,
		(VkDeviceSize)background_positions_size +
		(VkDeviceSize)background_uvs_size +
		(VkDeviceSize)background_indices_size +
		(VkDeviceSize)actor_positions_size +
		(VkDeviceSize)actor_uvs_size +
		(VkDeviceSize)actor_indices_size
	};

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

	vk_result = vkQueueSubmit (graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_QUEUE_SUBMIT;
		goto exit;
	}

	vkQueueWaitIdle (graphics_queue);

exit:
	if (staging_vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, staging_vertex_index_buffer, NULL);
	}

	if (staging_vertex_index_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, staging_vertex_index_memory, NULL);
	}

	if (copy_command_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, transfer_command_pool, 1, &copy_command_buffer);
	}

	return age_result;
}

AGE_RESULT graphics_create_image_buffers (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	int background_image_width = 0;
	int background_image_height = 0;
	int background_image_bpp = 0;
	uint8_t* background_image_pixels = NULL;
	utils_import_texture ("background.png", &background_image_width, &background_image_height, &background_image_bpp, &background_image_pixels);

	background_image_size = background_image_width * background_image_height * background_image_bpp * sizeof (uint8_t);

	int player_image_width = 0;
	int player_image_height = 0;
	int player_image_bpp = 0;
	uint8_t* player_image_pixels = NULL;
	utils_import_texture ("player.png", &player_image_width, &player_image_height, &player_image_bpp, &player_image_pixels);

	player_image_size = player_image_width * player_image_height * player_image_bpp * sizeof (uint8_t);

	int asteroid_image_width = 0;
	int asteroid_image_height = 0;
	int asteroid_image_bpp = 0;
	uint8_t* asteroid_image_pixels = NULL;
	utils_import_texture ("asteroid.png", &asteroid_image_width, &asteroid_image_height, &asteroid_image_bpp, &asteroid_image_pixels);

	asteroid_image_size = asteroid_image_width * asteroid_image_height * asteroid_image_bpp * sizeof (uint8_t);

	int bullet_image_width = 0;
	int bullet_image_height = 0;
	int bullet_image_bpp = 0;
	uint8_t* bullet_image_pixels = NULL;
	utils_import_texture ("bullet.png", &bullet_image_width, &bullet_image_height, &bullet_image_bpp, &bullet_image_pixels);

	bullet_image_size = bullet_image_width * bullet_image_height * bullet_image_bpp * sizeof (uint8_t);

	VkBuffer staging_image_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_image_memory = VK_NULL_HANDLE;

	VkBufferCreateInfo staging_image_buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		(VkDeviceSize)background_image_size +
		(VkDeviceSize)player_image_size +
		(VkDeviceSize)asteroid_image_size +
		(VkDeviceSize)bullet_image_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL
	};

	vk_result = vkCreateBuffer (device, &staging_image_buffer_create_info, NULL, &staging_image_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (device, staging_image_buffer, &memory_requirements);
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

	vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, &staging_image_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (device, staging_image_buffer, staging_image_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	void* data = NULL;

	vk_result = vkMapMemory (
		device,
		staging_image_memory,
		0,
		(VkDeviceSize)background_image_size +
		(VkDeviceSize)player_image_size +
		(VkDeviceSize)asteroid_image_size +
		(VkDeviceSize)bullet_image_size,
		0,
		&data
	);

	if (vk_result != VK_SUCCESS) {
		age_result = AGE_ERROR_GRAPHICS_MAP_MEMORY;
		goto exit;
	}

	memcpy (data, background_image_pixels, background_image_size);
	memcpy ((char*)data + background_image_size, player_image_pixels, player_image_size);
	memcpy ((char*)data + background_image_size + player_image_size, asteroid_image_pixels, asteroid_image_size);
	memcpy ((char*)data + background_image_size + player_image_size + asteroid_image_size, bullet_image_pixels, bullet_image_size);

	vkUnmapMemory (device, staging_image_memory);

	VkImageCreateInfo background_image_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{background_image_width, background_image_height, 1},
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	vk_result = vkCreateImage (device, &background_image_create_info, NULL, &background_image);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE;
		goto exit;
	}

	VkImageCreateInfo player_image_create_info = background_image_create_info;
	player_image_create_info.extent.width = player_image_width;
	player_image_create_info.extent.height = player_image_height;

	vk_result = vkCreateImage (device, &player_image_create_info, NULL, &player_image);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE;
		goto exit;
	}

	VkImageCreateInfo asteroid_image_create_info = player_image_create_info;
	asteroid_image_create_info.extent.width = asteroid_image_width;
	asteroid_image_create_info.extent.height = asteroid_image_height;

	vk_result = vkCreateImage (device, &asteroid_image_create_info, NULL, &asteroid_image);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE;
		goto exit;
	}

	VkImageCreateInfo bullet_image_create_info = player_image_create_info;
	bullet_image_create_info.extent.width = bullet_image_width;
	bullet_image_create_info.extent.height = bullet_image_height;

	vk_result = vkCreateImage (device, &bullet_image_create_info, NULL, &bullet_image);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE;
		goto exit;
	}

	VkDeviceSize total_vk_images_size = 0;
	vkGetImageMemoryRequirements (device, background_image, &memory_requirements);
	total_vk_images_size += memory_requirements.size;
	vkGetImageMemoryRequirements (device, player_image, &memory_requirements);
	total_vk_images_size += memory_requirements.size;
	vkGetImageMemoryRequirements (device, asteroid_image, &memory_requirements);
	total_vk_images_size += memory_requirements.size;
	vkGetImageMemoryRequirements (device, bullet_image, &memory_requirements);
	total_vk_images_size += memory_requirements.size;

	memory_requirements.size = total_vk_images_size;
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

	memory_allocate_info.allocationSize = total_vk_images_size;
	memory_allocate_info.memoryTypeIndex = required_memory_type_index;

	vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, &images_memory);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindImageMemory (device, background_image, images_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		goto exit;
	}

	vk_result = vkBindImageMemory (device, player_image, images_memory, (VkDeviceSize)background_image_size);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		goto exit;
	}

	vk_result = vkBindImageMemory (device, asteroid_image, images_memory, (VkDeviceSize)background_image_size + (VkDeviceSize)player_image_size);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		goto exit;
	}

	vk_result = vkBindImageMemory (device, bullet_image, images_memory, (VkDeviceSize)background_image_size + (VkDeviceSize)player_image_size + (VkDeviceSize)asteroid_image_size);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		goto exit;
	}

	VkImageSubresourceRange subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	};

	VkImageMemoryBarrier background_image_memory_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		graphics_queue_family_index,
		graphics_queue_family_index,
		background_image,
		subresource_range
	};

	VkImageMemoryBarrier player_image_memory_barrier = background_image_memory_barrier;
	player_image_memory_barrier.image = player_image;

	VkImageMemoryBarrier asteroid_image_memory_barrier = player_image_memory_barrier;
	asteroid_image_memory_barrier.image = asteroid_image;

	VkImageMemoryBarrier bullet_image_memory_barrier = asteroid_image_memory_barrier;
	bullet_image_memory_barrier.image = bullet_image;

	VkImageMemoryBarrier image_memory_barriers[] = {
		background_image_memory_barrier,
		player_image_memory_barrier,
		asteroid_image_memory_barrier,
		bullet_image_memory_barrier
	};

	VkCommandBuffer change_image_layout_cmd_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo change_image_layout_cmd_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		transfer_command_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1
	};
	vk_result = vkAllocateCommandBuffers (device, &change_image_layout_cmd_buffer_allocate_info, &change_image_layout_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	VkCommandBufferBeginInfo change_image_layout_cmd_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL
	};;

	vk_result = vkBeginCommandBuffer (change_image_layout_cmd_buffer, &change_image_layout_cmd_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		goto exit;
	}

	vkCmdPipelineBarrier (
		change_image_layout_cmd_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		4,
		image_memory_barriers
	);

	vk_result = vkEndCommandBuffer (change_image_layout_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		goto exit;
	}

	VkSubmitInfo change_image_layout_cmd_buffer_submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		0,
		NULL,
		0,
		1,
		&change_image_layout_cmd_buffer,
		0,
		NULL
	};

	vk_result = vkQueueSubmit (
		graphics_queue,
		1,
		&change_image_layout_cmd_buffer_submit_info,
		VK_NULL_HANDLE
	);
	vkQueueWaitIdle (graphics_queue);

	VkCommandBuffer copy_buffer_to_image_cmd_buffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo copy_buffer_to_image_cmd_buffer_allocate_info = change_image_layout_cmd_buffer_allocate_info;
	vk_result = vkAllocateCommandBuffers (device, &copy_buffer_to_image_cmd_buffer_allocate_info, &copy_buffer_to_image_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	VkCommandBufferBeginInfo copy_buffer_to_image_cmd_buffer_begin_info = change_image_layout_cmd_buffer_begin_info;

	vk_result = vkBeginCommandBuffer (copy_buffer_to_image_cmd_buffer, &copy_buffer_to_image_cmd_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		goto exit;
	}

	VkImageSubresourceLayers subresource_layers = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		0,
		1
	};

	VkOffset3D img_offset = { 0,0,0 };
	VkExtent3D img_extent = { background_image_width, background_image_height, 1 };

	VkBufferImageCopy background_img_copy = {
		0,
		0,
		0,
		subresource_layers,
		img_offset,
		img_extent
	};

	vkCmdCopyBufferToImage (
		copy_buffer_to_image_cmd_buffer,
		staging_image_buffer,
		background_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&background_img_copy
	);

	VkBufferImageCopy player_img_copy = background_img_copy;
	player_img_copy.bufferOffset = (VkDeviceSize)background_image_size;
	player_img_copy.imageExtent.width = player_image_width;
	player_img_copy.imageExtent.height = player_image_height;

	vkCmdCopyBufferToImage (
		copy_buffer_to_image_cmd_buffer,
		staging_image_buffer,
		player_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&player_img_copy
	);

	VkBufferImageCopy asteroid_img_copy = background_img_copy;
	asteroid_img_copy.bufferOffset = (VkDeviceSize)background_image_size + (VkDeviceSize)player_image_size;
	asteroid_img_copy.imageExtent.width = asteroid_image_width;
	asteroid_img_copy.imageExtent.height = asteroid_image_height;

	vkCmdCopyBufferToImage (
		copy_buffer_to_image_cmd_buffer,
		staging_image_buffer,
		asteroid_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&asteroid_img_copy
	);

	VkBufferImageCopy bullet_img_copy = background_img_copy;
	bullet_img_copy.bufferOffset = (VkDeviceSize)background_image_size + (VkDeviceSize)player_image_size + (VkDeviceSize)asteroid_image_size;;
	bullet_img_copy.imageExtent.width = bullet_image_width;
	bullet_img_copy.imageExtent.height = bullet_image_height;

	vkCmdCopyBufferToImage (
		copy_buffer_to_image_cmd_buffer,
		staging_image_buffer,
		bullet_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bullet_img_copy
	);

	vk_result = vkEndCommandBuffer (copy_buffer_to_image_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		goto exit;
	}

	VkSubmitInfo copy_buffer_to_image_cmd_submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		0,
		NULL,
		0,
		1,
		&copy_buffer_to_image_cmd_buffer,
		0,
		NULL
	};

	vk_result = vkQueueSubmit (graphics_queue, 1, &copy_buffer_to_image_cmd_submit_info, VK_NULL_HANDLE);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_QUEUE_SUBMIT;
		goto exit;
	}
	vkQueueWaitIdle (graphics_queue);

	background_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	background_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	background_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	background_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	player_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	player_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	player_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	player_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	asteroid_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	asteroid_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	asteroid_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	asteroid_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	bullet_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	bullet_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bullet_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	bullet_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	image_memory_barriers[0] = background_image_memory_barrier;
	image_memory_barriers[1] = player_image_memory_barrier;
	image_memory_barriers[2] = asteroid_image_memory_barrier;
	image_memory_barriers[3] = bullet_image_memory_barrier;

	vkFreeCommandBuffers (device, transfer_command_pool, 1, &change_image_layout_cmd_buffer);
	vk_result = vkAllocateCommandBuffers (device, &change_image_layout_cmd_buffer_allocate_info, &change_image_layout_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

	vk_result = vkBeginCommandBuffer (change_image_layout_cmd_buffer, &change_image_layout_cmd_buffer_begin_info);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
		goto exit;
	}

	vkCmdPipelineBarrier (
		change_image_layout_cmd_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		4,
		image_memory_barriers
	);

	vk_result = vkEndCommandBuffer (change_image_layout_cmd_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_END_COMMAND_BUFFER;
		goto exit;
	}

	change_image_layout_cmd_buffer_submit_info.pCommandBuffers = &change_image_layout_cmd_buffer;

	vk_result = vkQueueSubmit (
		graphics_queue,
		1,
		&change_image_layout_cmd_buffer_submit_info,
		VK_NULL_HANDLE
	);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_QUEUE_SUBMIT;
		goto exit;
	}

	vkQueueWaitIdle (graphics_queue);

	VkImageViewCreateInfo background_image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		background_image,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		subresource_range
	};

	vk_result = vkCreateImageView (device, &background_image_view_create_info, NULL, &background_image_view);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		goto exit;
	}

	VkImageViewCreateInfo player_image_view_create_info = background_image_view_create_info;
	player_image_view_create_info.image = player_image;

	vk_result = vkCreateImageView (device, &player_image_view_create_info, NULL, &player_image_view);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		goto exit;
	}

	VkImageViewCreateInfo asteroid_image_view_create_info = player_image_view_create_info;
	asteroid_image_view_create_info.image = asteroid_image;

	vk_result = vkCreateImageView (device, &asteroid_image_view_create_info, NULL, &asteroid_image_view);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		goto exit;
	}

	VkImageViewCreateInfo bullet_image_view_create_info = asteroid_image_view_create_info;
	bullet_image_view_create_info.image = bullet_image;

	vk_result = vkCreateImageView (device, &bullet_image_view_create_info, NULL, &bullet_image_view);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
		goto exit;
	}

exit:
	
	if (copy_buffer_to_image_cmd_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, transfer_command_pool, 1, &copy_buffer_to_image_cmd_buffer);
	}

	if (change_image_layout_cmd_buffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers (device, transfer_command_pool, 1, &change_image_layout_cmd_buffer);
	}

	if (staging_image_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, staging_image_buffer, NULL);
	}

	if (staging_image_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, staging_image_memory, NULL);
	}

	stbi_image_free (background_image_pixels);
	stbi_image_free (player_image_pixels);
	stbi_image_free (asteroid_image_pixels);
	stbi_image_free (bullet_image_pixels);

	return age_result;
}

AGE_RESULT graphics_create_pipeline (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
	VkShaderModuleCreateInfo vertex_shader_module_create_info = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		sizeof (actor_vert),
		actor_vert
	};

	vk_result = vkCreateShaderModule (device, &vertex_shader_module_create_info, NULL, &vertex_shader_module);

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

	vk_result = vkCreateShaderModule (device, &fragment_shader_module_create_info, NULL, &fragment_shader_module);
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
			sizeof (float) * 3,
			VK_VERTEX_INPUT_RATE_VERTEX
		},
		{
			1,
			sizeof (float) * 2,
			VK_VERTEX_INPUT_RATE_VERTEX
		}
	};
	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2] = {
		{
			0,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0
		},
		{
			1,
			1,
			VK_FORMAT_R32G32_SFLOAT,
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
		(float)current_extent.height,
		(float)current_extent.width,
		-(float)current_extent.height,
		0,
		1
	};
	VkRect2D scissor = {
		{0,0},
		current_extent
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
		VK_FALSE,
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
		VK_TRUE,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		VK_BLEND_OP_ADD,
		VK_BLEND_FACTOR_ONE,
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

	vk_result = vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &graphics_pipeline);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_GRAPHICS_PIPELINE;
		goto exit;
	}

exit:
	if (vertex_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (device, vertex_shader_module, NULL);
	}

	if (fragment_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule (device, fragment_shader_module, NULL);
	}

	return age_result;
}

AGE_RESULT graphics_create_swapchain_render_pass_framebuffers (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

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

	vk_result = vkCreateRenderPass (device, &render_pass_create_info, NULL, &render_pass);

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
		current_extent.width,
		current_extent.height,
		1
	};

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		framebuffer_create_info.pAttachments = swapchain_image_views + i;

		vk_result = vkCreateFramebuffer (device, &framebuffer_create_info, NULL, swapchain_framebuffers + i);

		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_FRAMEBUFFER;
			goto exit;
		}
	}

exit:
	return age_result;
}

AGE_RESULT graphics_create_swapchain_command_buffers (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkCommandPoolCreateInfo command_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		0,
		graphics_queue_family_index
	};

	vk_result = vkCreateCommandPool (device, &command_pool_create_info, NULL, &swapchain_command_pool);

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

	vk_result = vkAllocateCommandBuffers (device, &command_buffer_allocate_info, swapchain_command_buffers);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
		goto exit;
	}

exit:
	return age_result;
}

AGE_RESULT graphics_create_swapchain_semaphores_fences (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, NULL, 0 };
	vk_result = vkCreateSemaphore (device, &semaphore_create_info, NULL, &wait_semaphore);

	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_SEMAPHORE;
		goto exit;
	}

	swapchain_signal_semaphores = (VkSemaphore*)utils_calloc (swapchain_image_count, sizeof (VkSemaphore));
	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkCreateSemaphore (device, &semaphore_create_info, NULL, swapchain_signal_semaphores + i);

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
		vk_result = vkCreateFence (device, &fence_create_info, NULL, swapchain_fences + i);
		
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_CREATE_FENCE;
			goto exit;
		}
	}

exit:
	return age_result;
}


AGE_RESULT graphics_create_descriptor_sets (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;
	
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			4
		}
	};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		2,
		2,
		descriptor_pool_sizes
	};
	vk_result = vkCreateDescriptorPool (device, &descriptor_pool_create_info, NULL, &descriptor_pool);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
		goto exit;
	}

	VkDescriptorSetLayoutBinding descriptor_layout_bindings[] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			NULL
		},
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			4,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL
		}
	};

	VkDescriptorSetLayoutCreateInfo transform_descriptor_set_layout_create_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		descriptor_layout_bindings
	};

	vk_result = vkCreateDescriptorSetLayout (device, &transform_descriptor_set_layout_create_info, NULL, &transform_descriptor_set_layout);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
		goto exit;
	}

	VkDescriptorSetLayoutCreateInfo texture_descriptor_set_layout_create_info = transform_descriptor_set_layout_create_info;
	texture_descriptor_set_layout_create_info.pBindings = descriptor_layout_bindings + 1;

	vk_result = vkCreateDescriptorSetLayout (device, &texture_descriptor_set_layout_create_info, NULL, &texture_descriptor_set_layout);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
		goto exit;
	}

	VkDescriptorSetAllocateInfo transform_descriptor_set_allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptor_pool,
		1,
		&transform_descriptor_set_layout
	};

	vk_result = vkAllocateDescriptorSets (device, &transform_descriptor_set_allocate_info, &transform_descriptor_set);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
		goto exit;
	}

	VkDescriptorSetAllocateInfo texture_descriptor_set_allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptor_pool,
		1,
		&texture_descriptor_set_layout
	};

	vk_result = vkAllocateDescriptorSets (device, &texture_descriptor_set_allocate_info, &texture_descriptor_set);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SETS;
		goto exit;
	}

	VkDescriptorSetLayout descriptor_set_layouts[2] = {
		transform_descriptor_set_layout,
		texture_descriptor_set_layout
	};

	VkDescriptorImageInfo image_infos[4] = {
		{
			common_sampler,
			background_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			player_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			asteroid_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		},
		{
			common_sampler,
			bullet_image_view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		}
	};

	VkWriteDescriptorSet texture_descriptor_set_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		NULL,
		texture_descriptor_set,
		0,
		0,
		4,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		image_infos,
		NULL,
		NULL
	};

	vkUpdateDescriptorSets (device, 1, &texture_descriptor_set_write, 0, NULL);

	VkPushConstantRange texture_index_push_constant_range = {
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof (uint32_t)
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		2,
		descriptor_set_layouts,
		1,
		&texture_index_push_constant_range
	};

	vk_result = vkCreatePipelineLayout (device, &pipeline_layout_create_info, NULL, &graphics_pipeline_layout);
	if (vk_result)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_PIPELINE_LAYOUT;
		goto exit;
	}

exit:
	return age_result;
}

AGE_RESULT graphics_create_transforms_buffer (const size_t game_current_max_asteroid_bullet_count)
{
	if (transforms_mapped_data != NULL)
	{
		vkUnmapMemory (device, transforms_buffer_memory);
	}

	if (transforms_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, transforms_buffer, NULL);
	}

	if (transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, transforms_buffer_memory, NULL);
	}

	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	size_t raw_size_per_transform = sizeof (actor_transform_outputs);
	aligned_size_per_transform = (raw_size_per_transform + (size_t)physical_device_limits.minUniformBufferOffsetAlignment - 1) & ~((size_t)physical_device_limits.minUniformBufferOffsetAlignment - 1);

	total_transforms_size = aligned_size_per_transform * (game_current_max_asteroid_bullet_count + 2);
	if (transforms_aligned_data == NULL)
	{
		transforms_aligned_data = utils_malloc_zero (total_transforms_size);
	}
	else 
	{
		transforms_aligned_data = utils_realloc (transforms_aligned_data, total_transforms_size);
	}

	VkBufferCreateInfo transforms_buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		total_transforms_size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL
	};

	vk_result = vkCreateBuffer (device, &transforms_buffer_create_info, NULL, &transforms_buffer);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_CREATE_BUFFER;
		goto exit;
	}

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements (device, transforms_buffer, &memory_requirements);

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

	vk_result = vkAllocateMemory (device, &memory_allocate_info, NULL, &transforms_buffer_memory);
	if (vk_result != VK_SUCCESS) 
	{
		age_result = AGE_ERROR_SYSTEM_ALLOCATE_MEMORY;
		goto exit;
	}

	vk_result = vkBindBufferMemory (device, transforms_buffer, transforms_buffer_memory, 0);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		goto exit;
	}

	vkMapMemory (device, transforms_buffer_memory, 0, memory_requirements.size, 0, &transforms_mapped_data);

	VkDescriptorBufferInfo buffer_info = {
		transforms_buffer,
		0,
		VK_WHOLE_SIZE
	};

	VkWriteDescriptorSet descriptor_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		NULL,
		transform_descriptor_set,
		0,
		0,
		1,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		NULL,
		&buffer_info,
		NULL
	};

	vkUpdateDescriptorSets (device, 1, &descriptor_write, 0, NULL);

exit: // clean up allocations made by the function

	return age_result;
}

AGE_RESULT graphics_update_transforms_buffer_data (
	const actor_transform_outputs* game_player_transform_outputs, 
	const actor_transform_outputs* game_asteroids_transform_outputs, 
	const size_t game_asteroid_live_count, 
	const size_t game_asteroid_current_max_count,
	const actor_transform_outputs* game_bullets_transform_outputs, 
	const size_t game_bullet_live_count,
	const size_t game_bullet_current_max_count
)
{
	AGE_RESULT age_result = AGE_SUCCESS;

	memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform), game_player_transform_outputs, sizeof (actor_transform_outputs)); 
	
	for (size_t a = 0; a < game_asteroid_live_count; ++a)
	{
		memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (a + 2)), game_asteroids_transform_outputs + a, sizeof (actor_transform_outputs));
	}

	for (size_t b = 0; b < game_bullet_live_count; ++b)
	{
		memcpy ((char*)transforms_aligned_data + (aligned_size_per_transform * (game_asteroid_live_count + b + 2)), game_bullets_transform_outputs + b, sizeof (actor_transform_outputs));
	}

	memcpy (transforms_mapped_data, transforms_aligned_data, total_transforms_size);

exit:
	return age_result;
}

AGE_RESULT graphics_update_command_buffers (const size_t game_asteroid_live_count, const size_t game_bullet_live_count)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		0,
		NULL
	};

	VkRenderPassBeginInfo render_pass_begin_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		render_pass,
		VK_NULL_HANDLE,
		{ {0,0}, current_extent },
		0,
		NULL
	};

	vk_result = vkResetCommandPool (device, swapchain_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_RESET_COMMAND_POOL;
		goto exit;
	}

	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vk_result = vkBeginCommandBuffer (swapchain_command_buffers[i], &command_buffer_begin_info);
		if (vk_result != VK_SUCCESS)
		{
			age_result = AGE_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
			goto exit;
		}

		render_pass_begin_info.framebuffer = swapchain_framebuffers[i];
		VkDeviceSize vertex_index_buffer_offsets[6] = {
			0, 
			(VkDeviceSize)background_positions_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size + (VkDeviceSize) actor_positions_size,
			(VkDeviceSize)background_positions_size + (VkDeviceSize)background_uvs_size + (VkDeviceSize) background_indices_size + (VkDeviceSize) actor_positions_size + (VkDeviceSize) actor_uvs_size
		};

		vkCmdBeginRenderPass (swapchain_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		uint32_t offset = 0;
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &offset);
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, NULL);
		uint32_t texture_index = 0;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof (uint32_t), &texture_index);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[0]);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[1]);
		vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[2], VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (swapchain_command_buffers[i], background_index_count, 1, 0, 0, 0);

		offset = aligned_size_per_transform;
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &offset);
		vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, NULL);
		texture_index = 1;
		vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof (uint32_t), &texture_index);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
		vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
		vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);

		for (size_t a = 0; a < game_asteroid_live_count; ++a)
		{
			offset = aligned_size_per_transform * (a + 2);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &offset);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, NULL);
			texture_index = 2;
			vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof (uint32_t), &texture_index);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
			vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);
		}

		for (size_t b = 0; b < game_bullet_live_count; ++b)
		{
			offset = aligned_size_per_transform * (game_asteroid_live_count + b + 2);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 0, 1, &transform_descriptor_set, 1, &offset);
			vkCmdBindDescriptorSets (swapchain_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, 1, 1, &texture_descriptor_set, 0, NULL);
			texture_index = 3;
			vkCmdPushConstants (swapchain_command_buffers[i], graphics_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof (uint32_t), &texture_index);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 0, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[3]);
			vkCmdBindVertexBuffers (swapchain_command_buffers[i], 1, 1, &vertex_index_buffer, &vertex_index_buffer_offsets[4]);
			vkCmdBindIndexBuffer (swapchain_command_buffers[i], vertex_index_buffer, vertex_index_buffer_offsets[5], VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed (swapchain_command_buffers[i], actor_index_count, 1, 0, 0, 0);
		}

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


AGE_RESULT graphics_init (const size_t game_asteroid_current_max_count, const size_t game_asteroid_live_count, const size_t game_bullet_current_max_count, const size_t game_bullet_live_count)
{
	AGE_RESULT age_result = AGE_SUCCESS;
	VkResult vk_result = VK_SUCCESS;

	age_result = graphics_create_geometry_buffers ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_image_buffers ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_descriptor_sets ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_swapchain_render_pass_framebuffers ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_pipeline ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_swapchain_command_buffers ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_swapchain_semaphores_fences ();
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_create_transforms_buffer (game_asteroid_current_max_count + game_bullet_current_max_count);
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

	age_result = graphics_update_command_buffers (game_asteroid_live_count, game_bullet_live_count);
	if (age_result != AGE_SUCCESS)
	{
		goto exit;
	}

exit: // clear function specific allocations before exit
	return age_result;
}

AGE_RESULT graphics_submit_present (void)
{
	AGE_RESULT age_result = AGE_SUCCESS;

	size_t image_index = 0;
	VkResult vk_result = vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &image_index);

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
			age_result = AGE_ERROR_GRAPHICS_ACQUIRE_NEXT_IMAGE;
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

	vk_result = vkWaitForFences (device, 1, swapchain_fences + image_index, VK_TRUE, UINT64_MAX);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_WAIT_FOR_FENCES;
		goto exit;
	}

	vk_result = vkResetFences (device, 1, swapchain_fences + image_index);
	if (vk_result != VK_SUCCESS)
	{
		age_result = AGE_ERROR_GRAPHICS_RESET_FENCES;
		goto exit;
	}

exit:
	return age_result;
}

void graphics_shutdown (void)
{
	vkQueueWaitIdle (graphics_queue);

	if (transforms_mapped_data != NULL)
	{
		vkUnmapMemory (device, transforms_buffer_memory);
	}

	utils_free (transforms_aligned_data);

	if (transform_descriptor_set != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (device, descriptor_pool, 1, &transform_descriptor_set);
	}

	if (texture_descriptor_set != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets (device, descriptor_pool, 1, &texture_descriptor_set);
	}
	
	if (transform_descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (device, transform_descriptor_set_layout, NULL);
	}
	
	if (texture_descriptor_set_layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout (device, texture_descriptor_set_layout, NULL);
	}

	if (descriptor_pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool (device, descriptor_pool, NULL);
	}

	if (swapchain_fences)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroyFence (device, swapchain_fences[i], NULL);
		}

		utils_free (swapchain_fences);
	}

	if (graphics_pipeline_layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout (device, graphics_pipeline_layout, NULL);
	}

	if (graphics_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline (device, graphics_pipeline, NULL);
	}

	if (vertex_index_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (device, vertex_index_buffer, NULL);
	}

	if (vertex_index_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, vertex_index_buffer_memory, NULL);
	}

	if (background_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, background_image, NULL);
	}

	if (background_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, background_image_view, NULL);
	}

	if (player_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, player_image, NULL);
	}

	if (player_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, player_image_view, NULL);
	}

	if (asteroid_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, asteroid_image, NULL);
	}

	if (asteroid_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, asteroid_image_view, NULL);
	}

	if (bullet_image != VK_NULL_HANDLE)
	{
		vkDestroyImage (device, bullet_image, NULL);
	}

	if (bullet_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView (device, bullet_image_view, NULL);
	}

	if (images_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, images_memory, NULL);
	}

	if (transforms_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer (device, transforms_buffer, NULL);
	}

	if (transforms_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (device, transforms_buffer_memory, NULL);
	}

	if (wait_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore (device, wait_semaphore, NULL);
	}

	if (swapchain_signal_semaphores)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			vkDestroySemaphore (device, swapchain_signal_semaphores[i], NULL);
		}
		
		utils_free (swapchain_signal_semaphores);
	}

	if (swapchain_command_buffers)
	{
		vkFreeCommandBuffers (device, swapchain_command_pool, swapchain_image_count, swapchain_command_buffers);

		utils_free (swapchain_command_buffers);
	}

	if (swapchain_command_pool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool (device, swapchain_command_pool, NULL);
	}

	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass (device, render_pass, NULL);
	}
	
	if (swapchain_framebuffers)
	{
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			if (swapchain_framebuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer (device, swapchain_framebuffers[i], NULL);
			}
		}

		utils_free (swapchain_framebuffers);
	}
}