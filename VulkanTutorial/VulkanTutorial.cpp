// VulkanTutorial.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "vulkan\vulkan.h"

#include <iostream>

#define ASSERT_VULKAN(val)\
		if(val!=VK_SUCCESS) \
		{ \
			__debugbreak();\
		}

VkInstance instance;

int main()
{
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Vulkan Tutorial";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "Super Vulkan Engine Turbo Mega";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo;

	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;

	VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
	ASSERT_VULKAN(result);

	uint32_t amountOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, NULL);
	ASSERT_VULKAN(result);

	VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[amountOfPhysicalDevices];
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices);
	ASSERT_VULKAN(result);

    return 0;
}

