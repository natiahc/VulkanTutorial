// VulkanTutorial.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
//#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan\vulkan.h"

#include <iostream>
#include <vector>

#define ASSERT_VULKAN(val)\
		if(val!=VK_SUCCESS) \
		{ \
			__debugbreak();\
		}

VkInstance instance;
VkDevice device;
GLFWwindow *window;

void printStats(VkPhysicalDevice &device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	std::cout << "Name: " << properties.deviceName << std::endl;
	
	uint32_t apiVer = properties.apiVersion;
	std::cout << "API Version: " << VK_VERSION_MAJOR(apiVer) << "." << 
		VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl;

	std::cout << "Driver Version: " << properties.driverVersion << std::endl;
	std::cout << "Vendor ID:      " << properties.vendorID << std::endl;
	std::cout << "Device ID:      " << properties.deviceID << std::endl;
	std::cout << "Device Type:    " << properties.deviceType << std::endl;
	std::cout << "discreteQueuePriorities: " << properties.limits.discreteQueuePriorities << std::endl;

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);
	std::cout << "Geometry Shader: " << features.geometryShader << std::endl;

	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(device, &memProp);

	uint32_t amountOfQueuefamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueuefamilies, nullptr);
	VkQueueFamilyProperties *familyProperties = new VkQueueFamilyProperties[amountOfQueuefamilies];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueuefamilies, familyProperties);

	std::cout << "Amount of Queue Families: " << amountOfQueuefamilies << std::endl;
	for (int i=0; i<amountOfQueuefamilies; i++)
	{
		std::cout << std::endl;
		std::cout << "Queue Family #" << i << std::endl;
		std::cout << "VK_QUEUE_GRAPHCS_BIT " << ((familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_COMPUTE_BIT " << ((familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_TRANSFER_BIT " << ((familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_SPARSE_BINDING_BIT " << ((familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
		std::cout << "Queue Count: " << familyProperties[i].queueCount << std::endl;
		std::cout << "Timestamp Valid Bits: " << familyProperties[i].queueCount << std::endl;

		uint32_t width = familyProperties[i].minImageTransferGranularity.width;
		uint32_t height = familyProperties[i].minImageTransferGranularity.height;
		uint32_t depth = familyProperties[i].minImageTransferGranularity.depth;
		std::cout << "Min Image Timestamp Granularity: " << width << ", " << height << ", " << depth << std::endl;
	}

	std::cout << std::endl;

	delete[] familyProperties;
}

void startGlfw()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(400, 300, "Vulkan Tutorial", nullptr, nullptr);
}

void startVulkan()
{
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Vulkan Tutorial";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "Super Vulkan Engine Turbo Mega";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t amountOfLayers = 0;
	vkEnumerateInstanceLayerProperties(&amountOfLayers, nullptr);
	VkLayerProperties *layers = new VkLayerProperties[amountOfLayers];
	vkEnumerateInstanceLayerProperties(&amountOfLayers, layers);
	std::cout << "Amount of instance Layers: " << amountOfLayers << std::endl;
	for (int i = 0; i < amountOfLayers; i++)
	{
		std::cout << std::endl;
		std::cout << "Name: " << layers[i].layerName << std::endl;
		std::cout << "Spec Verion: " << layers[i].specVersion << std::endl;
		std::cout << "Implementation Version: " << layers[i].implementationVersion << std::endl;
		std::cout << "Description: " << layers[i].description << std::endl;
	}

	std::cout << std::endl;

	uint32_t amountOfExtensions = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, nullptr);
	VkExtensionProperties *extensions = new VkExtensionProperties[amountOfExtensions];
	vkEnumerateInstanceExtensionProperties(nullptr, &amountOfExtensions, extensions);

	std::cout << std::endl;

	std::cout << "Amount of extensions: " << amountOfExtensions << std::endl;

	for (int i = 0; i < amountOfExtensions; i++)
	{
		std::cout << std::endl;
		std::cout << "Name: " << extensions[i].extensionName << std::endl;
		std::cout << "specVersion: " << extensions[i].specVersion << std::endl;
	}

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	//const std::vector<const char*> usedExtenesions = {
	//	"VK_KHR_surface",
	//	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	//};


	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = validationLayers.size();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();
	instanceInfo.enabledExtensionCount = 0;// usedExtenesions.size();
	instanceInfo.ppEnabledExtensionNames = nullptr;//usedExtenesions.data();

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

	//vkGetInstanceProcAddr(instance, "");

	ASSERT_VULKAN(result);

	//VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	//surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	//surfaceCreateInfo.pNext = nullptr;
	//surfaceCreateInfo.flags = 0;
	//surfaceCreateInfo.hinstance = nullptr;
	//surfaceCreateInfo.hwnd = nullptr;

	//VkSurfaceKHR surface;
	//result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
	//ASSERT_VULKAN(result);

	uint32_t amountOfPhysicalDevices = 0;
	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, nullptr);
	ASSERT_VULKAN(result);

	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(amountOfPhysicalDevices);

	result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevices.data());
	ASSERT_VULKAN(result);

	for (int i = 0; i < amountOfPhysicalDevices; i++)
	{
		printStats(physicalDevices[i]);
	}

	float queuePrios[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = nullptr;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = 0;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePrios;

	VkPhysicalDeviceFeatures usedFeature = {};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = &usedFeature;

	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
	ASSERT_VULKAN(result);

	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);

	delete[] layers;
	delete[] extensions;
}

void gameLoop()
{
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void shutdownVulkan()
{
	vkDeviceWaitIdle(device);

	vkDestroyDevice(device, nullptr);
	//	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw()
{}

int main()
{
	startGlfw();
	startVulkan();
	gameLoop();
	shutdownVulkan();
	shutdownGlfw();

    return 0;
}

