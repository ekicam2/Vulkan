#include <Windows.h>

#include <vector>
using namespace std;

// loading SDL and defining that we don't want it to mess with main
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

// defining windows platform for vulkan and telling it we will be loading functions dynamically 
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// helper function
void assert(bool condition, const char* msg) {
    if (!condition) {
        OutputDebugString("ASSERT: ");
        OutputDebugString(msg);
        OutputDebugString("\n");
        int* breaker = 0;
        *breaker = 1;
    }

}

// some global variables
SDL_Window*     window                       = nullptr;
VkInstance      instance                     = nullptr;
VkSurfaceKHR    surface                      = nullptr;
VkDevice        device                       = nullptr;
VkQueue         graphicsQueue                = nullptr;
VkQueue         presentationQueue            = nullptr;
VkSemaphore     imageAvailableSemaphore      = nullptr;
VkSemaphore     renderingFinishedSemaphore   = nullptr;

// space for vulkan dynamic module and functions
HMODULE vulkanModule = VK_NULL_HANDLE;

// GetProcAddr(vulkanModule)
PFN_vkGetInstanceProcAddr                       vkGetInstanceProcAddr                       = nullptr;

// vkGetInstanceProcAddr(nullptr)
PFN_vkCreateInstance                            vkCreateInstance                            = nullptr;
PFN_vkEnumerateInstanceExtensionProperties      vkEnumerateInstanceExtensionProperties      = nullptr;
PFN_vkEnumerateInstanceLayerProperties          vkEnumerateInstanceLayerProperties          = nullptr;

// vkGetInstanceProcAddr(vkInstance)
PFN_vkDestroyInstance                           vkDestroyInstance                           = nullptr;
PFN_vkEnumeratePhysicalDevices                  vkEnumeratePhysicalDevices                  = nullptr;
PFN_vkGetPhysicalDeviceProperties               vkGetPhysicalDeviceProperties               = nullptr;
PFN_vkGetPhysicalDeviceFeatures                 vkGetPhysicalDeviceFeatures                 = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties    vkGetPhysicalDeviceQueueFamilyProperties    = nullptr;
PFN_vkCreateDevice                              vkCreateDevice                              = nullptr;
PFN_vkGetDeviceProcAddr                         vkGetDeviceProcAddr                         = nullptr;
PFN_vkEnumerateDeviceExtensionProperties        vkEnumerateDeviceExtensionProperties        = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR        vkGetPhysicalDeviceSurfaceSupportKHR        = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR   vkGetPhysicalDeviceSurfaceCapabilitiesKHR   = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR        vkGetPhysicalDeviceSurfaceFormatsKHR        = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR   vkGetPhysicalDeviceSurfacePresentModesKHR   = nullptr;
PFN_vkDestroySurfaceKHR                         vkDestroySurfaceKHR                         = nullptr;
PFN_vkCreateWin32SurfaceKHR                     vkCreateWin32SurfaceKHR                     = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties         vkGetPhysicalDeviceMemoryProperties         = nullptr;

// vkGetDeviceProcAddr(vkDevice)
PFN_vkGetDeviceQueue                            vkGetDeviceQueue                            = nullptr;
PFN_vkDestroyDevice                             vkDestroyDevice                             = nullptr;
PFN_vkDeviceWaitIdle                            vkDeviceWaitIdle                            = nullptr;
PFN_vkCreateSemaphore                           vkCreateSemaphore                           = nullptr;
PFN_vkCreateCommandPool                         vkCreateCommandPool                         = nullptr;
PFN_vkAllocateCommandBuffers                    vkAllocateCommandBuffers                    = nullptr;
PFN_vkBeginCommandBuffer                        vkBeginCommandBuffer                        = nullptr;
PFN_vkCmdPipelineBarrier                        vkCmdPipelineBarrier                        = nullptr;
PFN_vkCmdClearColorImage                        vkCmdClearColorImage                        = nullptr;
PFN_vkEndCommandBuffer                          vkEndCommandBuffer                          = nullptr;
PFN_vkQueueSubmit                               vkQueueSubmit                               = nullptr;
PFN_vkFreeCommandBuffers                        vkFreeCommandBuffers                        = nullptr;
PFN_vkDestroyCommandPool                        vkDestroyCommandPool                        = nullptr;
PFN_vkDestroySemaphore                          vkDestroySemaphore                          = nullptr;
PFN_vkCreateSwapchainKHR                        vkCreateSwapchainKHR                        = nullptr;
PFN_vkDestroySwapchainKHR                       vkDestroySwapchainKHR                       = nullptr;
PFN_vkGetSwapchainImagesKHR                     vkGetSwapchainImagesKHR                     = nullptr;
PFN_vkAcquireNextImageKHR                       vkAcquireNextImageKHR                       = nullptr;
PFN_vkQueuePresentKHR                           vkQueuePresentKHR                           = nullptr;


int main(int argc, char* argv[]) {

    // dynamic vulkan loading
    PFN_vkCreateInstance createInstance = NULL;
    vulkanModule = LoadLibrary("vulkan-1.dll");
    assert(vulkanModule, "Load vulkan module");

    // init SDL
    bool sdlInit = SDL_Init(SDL_INIT_VIDEO);
    assert(sdlInit == 0, "SDL_Init");

    // SDL window creation
    window = SDL_CreateWindow("Vulkan tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 786, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    // these functions can be called with nullptr instead pInstance
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkanModule, "vkGetInstanceProcAddr");
    assert(vkGetInstanceProcAddr, "Load vkGetInstanceProcAddr");

    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    assert(vkCreateInstance, "Load vkCreateInstance");

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties");
    assert(vkEnumerateInstanceExtensionProperties, "Load vkEnumerateInstanceExtensionProperties");

    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");
    assert(vkEnumerateInstanceLayerProperties, "Load vkEnumerateInstanceLayerProperties");

    uint32_t instanceExtensionsNum = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsNum, nullptr);
    assert(result == VK_SUCCESS || instanceExtensionsNum == 0, "Call count vkEnumerateInstanceExtensionProperties");

    vector<VkExtensionProperties> instanceExtensionsProperties(instanceExtensionsNum);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsNum, &instanceExtensionsProperties[0]);
    assert(result == VK_SUCCESS, "Call list vkEnumerateInstanceExtensionProperties");

    vector<const char*> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    uint32_t instanceExtCounter = 0;
    for (uint32_t i = 0; i < instanceExtensions.size(); ++i) {
        for (uint32_t j = 0; j < instanceExtensionsProperties.size(); ++j) {
            if (strcmp(instanceExtensions[i], instanceExtensionsProperties[j].extensionName) == 0) {
                ++instanceExtCounter;
            }
        }
    }
    assert(instanceExtCounter == instanceExtensions.size(), "Can not find required instance extensions");


    VkApplicationInfo vkAppInfo = {};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.pApplicationName = "Vulkan tutorial";
    vkAppInfo.pNext = nullptr;

    VkInstanceCreateInfo vkInstanceCreateInfo = {};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.enabledLayerCount = 0;
    vkInstanceCreateInfo.ppEnabledLayerNames = nullptr;
    vkInstanceCreateInfo.enabledExtensionCount = instanceExtCounter;
    vkInstanceCreateInfo.ppEnabledExtensionNames = &instanceExtensions[0];
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceCreateInfo.pNext = nullptr;

    // vulkan Instance creation
    result = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &instance);
    assert(result == VK_SUCCESS, "Call vkCreateInstance");

    // once we have an instance we can load rest of needed functions
#pragma region instanceEntryLevel
    vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance, "vkDestroyInstance");
    assert(vkDestroyInstance, "Load vkDestroyInstance");

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
    assert(vkEnumeratePhysicalDevices, "Load vkEnumeratePhysicalDevices");

    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
    assert(vkGetPhysicalDeviceProperties, "Load vkGetPhysicalDeviceProperties");

    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
    assert(vkGetPhysicalDeviceFeatures, "Load vkGetPhysicalDeviceFeatures");

    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    assert(vkGetPhysicalDeviceQueueFamilyProperties, "Load vkGetPhysicalDeviceQueueFamilyProperties");

    vkCreateDevice = (PFN_vkCreateDevice)vkGetInstanceProcAddr(instance, "vkCreateDevice");
    assert(vkCreateDevice, "Load vkCreateDevice");

    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
    assert(vkGetDeviceProcAddr, "Load vkGetDeviceProcAddr");

    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
    assert(vkEnumerateDeviceExtensionProperties, "Load vkEnumerateDeviceExtensionProperties");

    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    assert(vkGetPhysicalDeviceSurfaceSupportKHR, "Load vkGetPhysicalDeviceSurfaceSupportKHR");

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, "Load vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    assert(vkGetPhysicalDeviceSurfaceFormatsKHR, "Load vkGetPhysicalDeviceSurfaceFormatsKHR");

    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    assert(vkGetPhysicalDeviceSurfacePresentModesKHR, "Load vkGetPhysicalDeviceSurfacePresentModesKHR");

    vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
    assert(vkDestroySurfaceKHR, "Load vkDestroySurfaceKHR");

    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    assert(vkCreateWin32SurfaceKHR, "Load vkCreateWin32SurfaceKHR");

    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
    assert(vkGetPhysicalDeviceMemoryProperties, "Laod vkGetPhysicalDeviceMemoryProperties");
#pragma endregion instanceEntryLevel

    // after we initialize instance we can create a presentation surface
    SDL_SysWMinfo wmInfo;
    SDL_GetWindowWMInfo(window, &wmInfo);

    VkWin32SurfaceCreateInfoKHR createSurfaceInfo = {};
    createSurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createSurfaceInfo.pNext = nullptr;
    createSurfaceInfo.flags = 0;
    createSurfaceInfo.hinstance = GetModuleHandle(NULL); //untill there is no SDL 2.0.6 released it has to stay as that
    createSurfaceInfo.hwnd = wmInfo.info.win.window;

    result = vkCreateWin32SurfaceKHR(instance, &createSurfaceInfo, nullptr, &surface);
    assert(result == VK_SUCCESS, "Call vkCreateWin32Surface");

    // count all available GPU's
    uint32_t deviceNum = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceNum, nullptr);
    assert(result == VK_SUCCESS, "Call count vkEnumerateDeviceExtensionProperties");

    // and store them into a vector
    vector<VkPhysicalDevice> physicalDevices(deviceNum);
    result = vkEnumeratePhysicalDevices(instance, &deviceNum, &physicalDevices[0]);
    assert(result == VK_SUCCESS, "Call list vkEnumerateDeviceExtensionProperties");
        
    uint32_t  graphicsQueueFamilyIndex    = UINT32_MAX;
    uint32_t  presentQueueFamilyIndex     = UINT32_MAX;
    uint32_t  selectedPhysicalDeviceIndex = UINT32_MAX;

    for (uint32_t i = 0; i < physicalDevices.size(); ++i) {
        // now we are checking if swapchain extension is available
        uint32_t physicalDevExtensionsNum = 0;
        result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &physicalDevExtensionsNum, nullptr);
        assert(result == VK_SUCCESS || physicalDevExtensionsNum > 0, "Call count vkEnumerateDeviceExtensionPRoperties");

        vector<VkExtensionProperties> physicalDevExtensionsProperties(physicalDevExtensionsNum);
        result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &physicalDevExtensionsNum, &physicalDevExtensionsProperties[0]);
        assert(result == VK_SUCCESS, "Call list vkEnumerateDeviceExtensionProperties");

        vector<const char*> physicalDevExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        uint32_t physDevExtCounter = 0;
        for (uint32_t i = 0; i < physicalDevExtensions.size(); ++i) {
            for (uint32_t j = 0; j < physicalDevExtensionsProperties.size(); ++j) {
                if (strcmp(physicalDevExtensions[i], physicalDevExtensionsProperties[j].extensionName) == 0) {
                    ++physDevExtCounter;
                }
            }
        }
        // FIXME: there shouldn't be assert here, if there is more than one GPU one can not support extension while another do!
        assert(physDevExtCounter == physicalDevExtensions.size(), "Can not find required device extensions");

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures   deviceFeatures;

        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

        uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);
        uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

        assert(majorVersion >= 1, "Physical device does not support required parameters!");

        // count all available families
        uint32_t queueFamiliesNum = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamiliesNum, nullptr);
        assert(queueFamiliesNum != 0, "Call count vkGetPhysicalDeviceQueueFamilyProperties");

        vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesNum);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamiliesNum, &queueFamilyProperties[0]);

        // check if device support presentation and graphic extensions
        vector<VkBool32> queuePresentSupprtIndex(queueFamiliesNum);
        for (uint32_t j = 0; j < queueFamiliesNum; ++j)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, surface, &queuePresentSupprtIndex[j]);

            if (queueFamilyProperties[j].queueCount > 0)
            {
                // if current queue supports present it's ok, save it
                if (queuePresentSupprtIndex[j])
                {
                    presentQueueFamilyIndex = j;
                }

                if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    // if current queue supports graphics it's ok, save it
                    graphicsQueueFamilyIndex = j;
                    
                    // if current queue supports both graphics and presnetation. 
                    // there won't be better situtation. break the loop
                    if (queuePresentSupprtIndex[j])
                    {
                        graphicsQueueFamilyIndex = j;
                        presentQueueFamilyIndex = j;
                        selectedPhysicalDeviceIndex = i;
                        break;
                    }
                }
            }
        }

        if(graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1 && selectedPhysicalDeviceIndex != UINT32_MAX)
            selectedPhysicalDeviceIndex = i;
    }

    assert(graphicsQueueFamilyIndex != -1, "Device does not support reuired queue families");
    assert(presentQueueFamilyIndex  != -1, "Device does not support reuired queue families");

    vector<float> queuePriorities = { 1.0f };

    vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

    // as long as we have only one queue which handle graphics and presentation we will need only one queueCreateInfo
    deviceQueueCreateInfos.push_back({
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                nullptr,
                0,
                static_cast<uint32_t>(graphicsQueueFamilyIndex),
                static_cast<uint32_t>(queuePriorities.size()),
                &queuePriorities[0]
    });

    // but it might occurs that our application will have two separete queues one for graphics and one for presentation
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        deviceQueueCreateInfos.push_back({
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(presentQueueFamilyIndex),
            static_cast<uint32_t>(queuePriorities.size()),
            &queuePriorities[0]
        });
    }

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo = {
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        deviceQueueCreateInfos.size(),
        &deviceQueueCreateInfos[0],
        0,
        nullptr,
        deviceExtensions.size(),
        &deviceExtensions[0],
        nullptr
    };
    
    result = vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
    assert(result == VK_SUCCESS, "Call vkCreateDevice");

    // once we have a logical device we can load functions
    // WARNING! functions loaded by vkGetDeviceProcAddr can be used only with device that were called by!
#pragma region deviceEntryLevel
    vkGetDeviceQueue = (PFN_vkGetDeviceQueue) vkGetDeviceProcAddr(device, "vkGetDeviceQueue");
    assert(vkGetDeviceQueue, "Load vkGetDeviceQueue");

    vkDestroyDevice = (PFN_vkDestroyDevice) vkGetDeviceProcAddr(device, "vkDestroyDevice");
    assert(vkDestroyDevice, "Load vkDestroyDevice");

    vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) vkGetDeviceProcAddr(device, "vkDeviceWaitIdle");
    assert(vkDeviceWaitIdle, "Load vkDeviceWaitIdle");

    vkCreateSemaphore = (PFN_vkCreateSemaphore) vkGetDeviceProcAddr(device, "vkCreateSemaphore");
    assert(vkCreateSemaphore, "Load vkCreateSemaphore");

    vkCreateCommandPool = (PFN_vkCreateCommandPool) vkGetDeviceProcAddr(device, "vkCreateCommandPool");
    assert(vkCreateCommandPool, "Load vkCreateCommandPool");

    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers");
    assert(vkAllocateCommandBuffers, "Load vkAllocateCommandBuffers");

    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) vkGetDeviceProcAddr(device, "vkBeginCommandBuffer");
    assert(vkBeginCommandBuffer, "Load vkBeginCommandBuffer");

    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier");
    assert(vkCmdPipelineBarrier, "Load vkCmdPipelineBarrier");

    vkCmdClearColorImage = (PFN_vkCmdClearColorImage) vkGetDeviceProcAddr(device, "vkCmdClearColorImage");
    assert(vkCmdClearColorImage, "Load vkCmdClearColorImage");

    vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(device, "vkEndCommandBuffer");
    assert(vkEndCommandBuffer, "Load vkEndCommandBuffer");

    vkQueueSubmit = (PFN_vkQueueSubmit)vkGetDeviceProcAddr(device, "vkQueueSubmit");
    assert(vkQueueSubmit, "Load vkQueueSubmit");

    vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) vkGetDeviceProcAddr(device, "vkFreeCommandBuffers");
    assert(vkFreeCommandBuffers, "Load vkFreeCommandBuffers");

    vkDestroyCommandPool = (PFN_vkDestroyCommandPool) vkGetDeviceProcAddr(device, "vkDestroyCommandPool");
    assert(vkDestroyCommandPool, "Load vkDestroyCommandPool");

    vkDestroySemaphore = (PFN_vkDestroySemaphore) vkGetDeviceProcAddr(device, "vkDestroySemaphore");
    assert(vkDestroySemaphore, "Load vkDestroySemaphore");

    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
    assert(vkCreateSwapchainKHR, "Load vkCreateSwapchainKHR");

    vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
    assert(vkDestroySwapchainKHR, "Load vkDestroySwapchainKHR");

    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
    assert(vkGetSwapchainImagesKHR, "Load vkGetSwapchainImagesKHR");

    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
    assert(vkAcquireNextImageKHR, "Load vkAcquireNextImageKHR");

    vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
    assert(vkQueuePresentKHR, "Load vkQueuePresentKHR");
#pragma endregion deviceEntryLevel

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    assert(graphicsQueue != nullptr, "Call vkGetDeviceQueue graphics");
    
    vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentationQueue);
    assert(presentationQueue, "Call vkGetDeviceQueue presentation");

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore);
    assert(result == VK_SUCCESS, "Couldn't create a Semaphore");

    result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderingFinishedSemaphore);
    assert(result == VK_SUCCESS, "Couldn't create a Semaphore");

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[selectedPhysicalDeviceIndex], surface, &surfaceCapabilities);

    SDL_Delay(5000);

    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);

    vkDestroyInstance(instance, nullptr);

    FreeLibrary(vulkanModule);

    SDL_Quit();

    return 0;
}