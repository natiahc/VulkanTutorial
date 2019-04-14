#pragma once

#include "VulkanUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class EasyImage 
{
private:
	int m_width;
	int m_height;
	int m_channels;
	stbi_uc *m_ppixels;
	bool m_loaded = false;
	bool m_uploaded = false;
	VkImage m_image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_imageView;
	VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VkDevice m_device;
	VkSampler m_sampler;

public:
	EasyImage()
	{
		m_loaded = false;
	}

	EasyImage(const char* path)
	{
		load(path);
	}

	~EasyImage()
	{
		destroy();
	}

	void load(const char *path)
	{
		if (m_loaded)
		{
			throw std::logic_error("EasyImage was already loaded!");
		}

		m_ppixels = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);

		if (m_ppixels == nullptr)
		{
			throw std::invalid_argument("Could not load image, or image is currupt!");
		}

		m_loaded = true;
	}

	void upload(const VkDevice &device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue)
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		if (m_uploaded)
		{
			throw std::logic_error("EasyImage was already uploaded!");
		}

		m_device = device;

		VkDeviceSize imageSize = getSizeInBytes();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory = {};

		createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);

		void *data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, getRaw(), imageSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width = getWidth();
		imageCreateInfo.extent.height = getHeight();
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = m_imageLayout;

		VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_image);
		ASSERT_VULKAN(result);

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, m_image, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

		result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &m_ImageMemory);
		ASSERT_VULKAN(result);

		vkBindImageMemory(device, m_image, m_ImageMemory, 0);

		changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		writeBufferToImage(device, commandPool, queue, stagingBuffer);
		changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = m_image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_imageView);
		ASSERT_VULKAN(result);

		VkSamplerCreateInfo samplerCreateinfo;
		samplerCreateinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateinfo.pNext = nullptr;
		samplerCreateinfo.flags = 0;
		samplerCreateinfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateinfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateinfo.mipLodBias = 0.0f;
		samplerCreateinfo.anisotropyEnable = VK_TRUE;
		samplerCreateinfo.maxAnisotropy = 16;
		samplerCreateinfo.compareEnable = VK_FALSE;
		samplerCreateinfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateinfo.minLod = 0.0f;
		samplerCreateinfo.maxLod = 0.0f;
		samplerCreateinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateinfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(device, &samplerCreateinfo, nullptr, &m_sampler);
		ASSERT_VULKAN(result);

		m_uploaded = true;
	}

	void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
		ASSERT_VULKAN(result);

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		ASSERT_VULKAN(result);

		VkImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		
		if(m_imageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (m_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else
		{
			throw std::invalid_argument("Layout transition not yet supported!");
		}

		
		imageMemoryBarrier.oldLayout = m_imageLayout;
		imageMemoryBarrier.newLayout = layout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_image;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
			0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

		m_imageLayout = layout;
	}

	void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
		ASSERT_VULKAN(result);

		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		ASSERT_VULKAN(result);

		VkBufferImageCopy bufferImageCopy;
		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.aspectMask = 1;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent = { (uint32_t)getWidth(), (uint32_t)getHeight(), 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	EasyImage(const EasyImage &) = delete;
	EasyImage(const EasyImage &&) = delete;
	EasyImage& operator=(const EasyImage &) = delete;
	EasyImage& operator=(const EasyImage &&) = delete;

	void destroy()
	{
		if (m_loaded)
		{
			stbi_image_free(m_ppixels);
			m_loaded = false;
		}

		if (m_uploaded)
		{
			vkDestroySampler(m_device, m_sampler, nullptr);
			vkDestroyImageView(m_device, m_imageView, nullptr);

			vkDestroyImage(m_device, m_image, nullptr);
			vkFreeMemory(m_device, m_ImageMemory, nullptr);

			m_uploaded = false;
		}
	}

	int getHeight()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return m_height;
	}

	int getWidth()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return m_width;
	}

	int getChannels()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return 4;
	}

	int getSizeInBytes()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return getWidth() * getHeight() * getChannels();
	}

	stbi_uc *getRaw()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return m_ppixels;
	}

	VkSampler getSampler()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return m_sampler;
	}

	VkImageView getImageview()
	{
		if (!m_loaded)
		{
			throw std::logic_error("EasyImage was not loaded!");
		}

		return m_imageView;
	}
};