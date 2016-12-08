#include <Windows.h>

#include <vector>
using namespace std;

// loading SDL and defining that we don't want it to mess with main
#define SDL_MAIN_HANDLED
#include <SDL.h>

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
        int *breaker = 0;
        *breaker = 1;
    }

}

SDL_Window* window;
VkInstance  instance;

// space for vulkan dynamic module and functions
HMODULE vulkanModule = VK_NULL_HANDLE;

// vkGetInstanceProcAddr(nullptr)
PFN_vkGetInstanceProcAddr                       vkGetInstanceProcAddr                     = nullptr;
PFN_vkCreateInstance                            vkCreateInstance                          = nullptr;
PFN_vkEnumerateInstanceExtensionProperties      vkEnumerateInstanceExtensionProperties    = nullptr;
PFN_vkEnumerateInstanceLayerProperties          vkEnumerateInstanceLayerProperties        = nullptr;

// vlGetInstanceProcAddr(vkInstance)
PFN_vkDestroyInstance                           vkDestroyInstance                         = nullptr;
PFN_vkEnumeratePhysicalDevices                  vkEnumeratePhysicalDevices                = nullptr;
PFN_vkGetPhysicalDeviceProperties               vkGetPhysicalDeviceProperties             = nullptr;
PFN_vkGetPhysicalDeviceFeatures                 vkGetPhysicalDeviceFeatures               = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties    vkGetPhysicalDeviceQueueFamilyProperties  = nullptr;
PFN_vkCreateDevice                              vkCreateDevice                            = nullptr;
PFN_vkGetDeviceProcAddr                         vkGetDeviceProcAddr                       = nullptr;
PFN_vkEnumerateDeviceExtensionProperties        vkEnumerateDeviceExtensionProperties      = nullptr;


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
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(vulkanModule, "vkGetInstanceProcAddr");
    assert(vkGetInstanceProcAddr, "Load vkGetInstanceProcAddr");

    vkCreateInstance = (PFN_vkCreateInstance) vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    assert(vkCreateInstance, "Load vkCreateInstance");

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties");
    assert(vkEnumerateInstanceExtensionProperties, "Load vkEnumerateInstanceExtensionProperties");

    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");

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
    vkInstanceCreateInfo.enabledExtensionCount = 0;
    vkInstanceCreateInfo.ppEnabledExtensionNames = nullptr;
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceCreateInfo.pNext = nullptr;

    // vulkan Instance creation
    VkResult result = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &instance);
    assert(result == VK_SUCCESS, "Call vkCreateInstance");

    // once we have an instance we can load rest of needed functions
    vkDestroyInstance = (PFN_vkDestroyInstance) vkGetInstanceProcAddr(instance, "vkDestroyInstance");
    assert(vkDestroyInstance, "Load vkDestroyInstance");

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
    assert(vkEnumeratePhysicalDevices, "Load vkEnumeratePhysicalDevices");

    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
    assert(vkGetPhysicalDeviceProperties, "Load vkGetPhysicalDeviceProperties");

    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
    assert(vkGetPhysicalDeviceFeatures, "Load vkGetPhysicalDeviceFeatures");

    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    assert(vkGetPhysicalDeviceQueueFamilyProperties, "Load vkGetPhysicalDeviceQueueFamilyProperties");

    vkCreateDevice = (PFN_vkCreateDevice) vkGetInstanceProcAddr(instance, "vkCreateDevice");
    assert(vkCreateDevice, "Load vkCreateDevice");

    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
    assert(vkGetDeviceProcAddr, "Load vkGetDeviceProcAddr");

    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
    assert(vkEnumerateDeviceExtensionProperties, "Load vkEnumerateDeviceExtensionProperties");


    // count all available GPU's
    uint32_t deviceNum = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceNum, nullptr);
    assert(result == VK_SUCCESS, "Call count vkEnumerateDeviceExtensionProperties");

    // and store them into a vector
    vector<VkPhysicalDevice> physicalDevices(deviceNum);
    result = vkEnumeratePhysicalDevices(instance, &deviceNum, &physicalDevices[0]);
    assert(result == VK_SUCCESS, "Call list vkEnumerateDeviceExtensionProperties");


    // as long as I have only one GPU installed vector will always have size of 1
    // TODO: make it iterate over vector
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures   deviceFeatures;

    vkGetPhysicalDeviceProperties(physicalDevices[0], &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevices[0], &deviceFeatures);

    uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
    uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);
    uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

    assert(majorVersion >= 1 && deviceProperties.limits.maxImageDimension2D >= 4096, "Physical device does not support required parameters!");

    uint32_t queueFamiliesNum = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamiliesNum, nullptr);
    assert(queueFamiliesNum != 0, "Call count vkGetPhysicalDeviceQueueFamilyProperties");

    vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesNum);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamiliesNum, &queueFamilyProperties[0]);

    int32_t queueFamilyIndex = -1;
    for (uint32_t i = 0; i < queueFamiliesNum; ++i)
    {
        if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndex = i;
        }
    }
    assert(queueFamilyIndex != -1, "Device does not support reuired queue families");



    SDL_Delay(5000);

    vkDestroyInstance(instance, nullptr);
    SDL_Quit();

    return 0;
}