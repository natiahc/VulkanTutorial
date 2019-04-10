// VulkanTutorial.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
//#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan\vulkan.h"

#include <iostream>
#include <vector>
#include <fstream>

#define ASSERT_VULKAN(val)\
		if(val!=VK_SUCCESS) \
		{ \
			__debugbreak();\
		}

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkFramebuffer *framebuffers;
VkSwapchainKHR swapchain;
VkImageView *imageViews;
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;
VkCommandBuffer *commandBuffers;
VkCommandPool commandPool;
uint32_t amountOfImagesInSwapchain = 0;
GLFWwindow *window;

const uint32_t WIDTH = 400;
const uint32_t HEIGHT = 300;
const VkFormat ourFormat = VK_FORMAT_B8G8R8A8_UNORM;

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

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);
	std::cout << "Surface Capabilities:" << std::endl;
	std::cout << "\tminImageCount: " << surfaceCapabilities.minImageCount << std::endl;
	std::cout << "\tmaxImageCount: " << surfaceCapabilities.maxImageCount << std::endl;
	std::cout << "\tcurrentExtent: " << surfaceCapabilities.currentExtent.width << "/" << surfaceCapabilities.currentExtent.height << std::endl;
	std::cout << "\tminImageExtent: " << surfaceCapabilities.minImageExtent.width << "/" << surfaceCapabilities.minImageExtent.height << std::endl;
	std::cout << "\tmaxImageExtent: " << surfaceCapabilities.maxImageExtent.width << "/" << surfaceCapabilities.maxImageExtent.height << std::endl;
	std::cout << "\tmaxImageArrayLayers: " << surfaceCapabilities.maxImageArrayLayers << std::endl;
	std::cout << "\tsupportedTransforms: " << surfaceCapabilities.supportedTransforms << std::endl;
	std::cout << "\tcurrentTransform: " << surfaceCapabilities.currentTransform << std::endl;
	std::cout << "\tsupportedCompositeAlpha: " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
	std::cout << "\tsupportedUsageFlags: " << surfaceCapabilities.supportedUsageFlags << std::endl;

	uint32_t amountOfFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, nullptr);
	VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[amountOfFormats];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, surfaceFormats);

	std::cout << std::endl;

	std::cout << "Amount of Formats: " << amountOfFormats << std::endl;

	for (int i = 0; i < amountOfFormats; i++)
	{
		std::cout << "Format: " << surfaceFormats[i].format << std::endl;
	}

	uint32_t amountOfPresentationModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, nullptr);
	VkPresentModeKHR *presentModes = new VkPresentModeKHR[amountOfPresentationModes];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, presentModes);

	std::cout << std::endl;

	std::cout << "Amount of Presentation Modes: " << amountOfPresentationModes << std::endl;
	for (int i = 0; i < amountOfPresentationModes; i++)
	{
		std::cout << "Supported Presentation Mode: " << presentModes[i] << std::endl;
	}

	std::cout << std::endl;
	delete[] familyProperties;
	delete[] surfaceFormats;
	delete[] presentModes;
}

std::vector<char> readFile(std::string fileName)
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);

	if (file)
	{
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBuffer(fileSize);
		file.seekg(0);
		file.read(fileBuffer.data(), fileSize);
		file.close();
		return fileBuffer;
	}
	else 
	{
		throw std::runtime_error("Failed to open file!!!");
	}
}

void startGlfw()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Tutorial", nullptr, nullptr);
}

void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule)
{
	VkShaderModuleCreateInfo shaderCreatInfo;
	shaderCreatInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreatInfo.pNext = nullptr;
	shaderCreatInfo.flags = 0;
	shaderCreatInfo.codeSize = code.size();
	shaderCreatInfo.pCode = (uint32_t*)code.data();

	VkResult result = vkCreateShaderModule(device, &shaderCreatInfo, nullptr, shaderModule);
	ASSERT_VULKAN(result);
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

	uint32_t amountOfGlfwExtensions = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = validationLayers.size();
	instanceInfo.ppEnabledLayerNames = validationLayers.data();
	instanceInfo.enabledExtensionCount = amountOfGlfwExtensions;// usedExtenesions.size();
	instanceInfo.ppEnabledExtensionNames = glfwExtensions;//usedExtenesions.data();

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
	//vkGetInstanceProcAddr(instance, "");
	ASSERT_VULKAN(result);

	result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
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

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &usedFeature;

	result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
	ASSERT_VULKAN(result);

	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);

	VkBool32 surfaceSupport = false;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport);
	ASSERT_VULKAN(result);

	if (!surfaceSupport)
	{
		std::cerr << "Surface not supported!" << std::endl;
		__debugbreak();
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;
	swapchainCreateInfo.imageFormat = ourFormat;
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = VkExtent2D{ WIDTH, HEIGHT };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	ASSERT_VULKAN(result);

	vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, nullptr);
	VkImage *swapchainImages = new VkImage[amountOfImagesInSwapchain];
	result = vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, swapchainImages);
	ASSERT_VULKAN(result);

	imageViews = new VkImageView[amountOfImagesInSwapchain];
	for (int i = 0; i < amountOfImagesInSwapchain; i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = ourFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]);
		ASSERT_VULKAN(result);
	}

	auto shadercodeVert = readFile("vert.spv");
	auto shadercodeFrag = readFile("frag.spv");

	createShaderModule(shadercodeVert, &shaderModuleVert);
	createShaderModule(shadercodeFrag, &shaderModuleFrag);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
	shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoVert.pNext = nullptr;
	shaderStageCreateInfoVert.flags = 0;
	shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfoVert.module = shaderModuleVert;
	shaderStageCreateInfoVert.pName = "main";
	shaderStageCreateInfoVert.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
	shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoFrag.pNext = nullptr;
	shaderStageCreateInfoFrag.flags = 0;
	shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfoFrag.module = shaderModuleFrag;
	shaderStageCreateInfoFrag.pName = "main";
	shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { shaderStageCreateInfoVert, shaderStageCreateInfoFrag };

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = WIDTH;
	viewport.height = HEIGHT;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { WIDTH, HEIGHT };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationCreatInfo;
	rasterizationCreatInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreatInfo.pNext = nullptr;
	rasterizationCreatInfo.flags = 0;
	rasterizationCreatInfo.depthClampEnable = VK_FALSE;
	rasterizationCreatInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreatInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationCreatInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationCreatInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationCreatInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreatInfo.depthBiasConstantFactor = 0.0f;
	rasterizationCreatInfo.depthBiasClamp = 0.0f;
	rasterizationCreatInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationCreatInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo;
	multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleCreateInfo.pNext = nullptr;
	multiSampleCreateInfo.flags = 0;
	multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multiSampleCreateInfo.minSampleShading = 1.0f;
	multiSampleCreateInfo.pSampleMask = nullptr;
	multiSampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multiSampleCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pNext = nullptr;
	colorBlendCreateInfo.flags = 0;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = 0;

	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	ASSERT_VULKAN(result);

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = ourFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReference;
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPassDescription;
	subPassDescription.flags = 0;
	subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPassDescription.inputAttachmentCount = 0;
	subPassDescription.pInputAttachments = nullptr;
	subPassDescription.colorAttachmentCount = 1;
	subPassDescription.pColorAttachments = &attachmentReference;
	subPassDescription.pResolveAttachments = nullptr;
	subPassDescription.pDepthStencilAttachment = nullptr;
	subPassDescription.preserveAttachmentCount = 0;
	subPassDescription.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subPassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	ASSERT_VULKAN(result);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationCreatInfo;
	pipelineCreateInfo.pMultisampleState = &multiSampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	ASSERT_VULKAN(result);

	framebuffers = new VkFramebuffer[amountOfImagesInSwapchain];
	for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
	{
		VkFramebufferCreateInfo framebufferCreatInfo;
		framebufferCreatInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreatInfo.pNext = nullptr;
		framebufferCreatInfo.flags = 0;
		framebufferCreatInfo.renderPass = renderPass;
		framebufferCreatInfo.attachmentCount = 1;
		framebufferCreatInfo.pAttachments = &(imageViews[i]);
		framebufferCreatInfo.width = WIDTH;
		framebufferCreatInfo.height = HEIGHT;
		framebufferCreatInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferCreatInfo, nullptr, &(framebuffers[i]));
		ASSERT_VULKAN(result);
	}

	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = 0;

	result = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
	ASSERT_VULKAN(result);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = amountOfImagesInSwapchain;

	commandBuffers = new VkCommandBuffer[amountOfImagesInSwapchain];
	result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
	ASSERT_VULKAN(result);

	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
	{
		result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
		ASSERT_VULKAN(result);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { WIDTH, HEIGHT };

		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;
		
		result = vkEndCommandBuffer(commandBuffers[i]);
		ASSERT_VULKAN(result);
	}

	delete[] swapchainImages;
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

	vkFreeCommandBuffers(device, commandPool, amountOfImagesInSwapchain, commandBuffers);
	delete[] commandBuffers;

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}
	delete[] framebuffers;

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (int i = 0; i < amountOfImagesInSwapchain; i++)
	{
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	delete[] imageViews;
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device, shaderModuleVert, nullptr);
	vkDestroyShaderModule(device, shaderModuleFrag, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void shutdownGlfw()
{
	glfwDestroyWindow(window);
}

int main()
{
	startGlfw();
	startVulkan();
	gameLoop();
	shutdownVulkan();
	shutdownGlfw();

    return 0;
}

