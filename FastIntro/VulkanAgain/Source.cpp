#include <Windows.h>

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
		OutputDebugStringA(msg);
		OutputDebugStringA("\n");
		int *breaker = 0;
		*breaker = 1;
	}

}

SDL_Window* window;
VkInstance  instance;

// space for vulkan dynamic module and functions
HMODULE vulkanModule = VK_NULL_HANDLE;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

PFN_vkCreateInstance vkCreateInstance			= nullptr;
PFN_vkEnumerateInstanceExtensionProperties 
		 vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties  
			 vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkDestroyInstance vkDestroyInstance			= nullptr;

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

	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.apiVersion			= VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.applicationVersion	= VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.engineVersion			= VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.pApplicationName		= "Vulkan tutorial";
	vkAppInfo.pNext					= nullptr;

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
	vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance, "vkDestroyInstance");
	assert(vkDestroyInstance, "Load vkDestroyInstance");


	SDL_Delay(1000);

	vkDestroyInstance(instance, nullptr);
	SDL_Quit();

	return 0;
}