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
		VkDeviceMemory stagingBufferMemory;

		createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

		void *data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, getRaw(), imageSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createImage(device, physicalDevice, getWidth(), getHeight(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_image, m_ImageMemory);

		changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		writeBufferToImage(device, commandPool, queue, stagingBuffer);
		changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		createImageView(m_device, m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_imageView);

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

		VkResult result = vkCreateSampler(device, &samplerCreateinfo, nullptr, &m_sampler);
		ASSERT_VULKAN(result);

		m_uploaded = true;
	}

	void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout)
	{
		changeImageLayout(device, commandPool, queue, m_image, VK_FORMAT_R8G8B8A8_UNORM, m_imageLayout, layout);

		m_imageLayout = layout;
	}

	void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer)
	{
		VkCommandBuffer commandBuffer = startSingleTimeCommandBuffer(device, commandPool);

		VkBufferImageCopy bufferImageCopy;
		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.layerCount = 1;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent = { (uint32_t)getWidth(), (uint32_t)getHeight(), 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		endSingleTimeCommandBuffer(device, queue, commandPool, commandBuffer);
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