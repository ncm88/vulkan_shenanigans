/*
Basic Process: 
    1) Initialize window
    2) Create instance
    3) Set up validation layers / debug messenger
    4) Window surface (optional); The window surface needs to be created right after the instance creation, because it can actually influence the physical device selection
    5) Physical device/queue setup
    6) Logical device/queue setup
    End) Cleanup
*/


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <optional>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


const std::vector<const char*> validationLayers = {     //Like extensions, validation layers need to be enabled by specifying their name
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


VkResult CreateDebugUtilsMessengerEXT(  //used because we need to lookup the vkGetInstanceProcAddr function as it is an extension function (this is a proxy function)
    VkInstance vinstance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger
){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vinstance, "vkCreateDebugUtilsMessengerEXT");

    if(func != nullptr){
        return func(vinstance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr){
        func(instance, debugMessenger, pAllocator);
    }
}


class HelloTriangleApplication {

public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }


private:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   //Physical device handle
    VkDevice device;                                    //Logical device handle
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;   //Presentation queue

    VkSurfaceKHR surface;


    void initWindow(){
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // Set window behaviour characteristics
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }


    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
    }


    void mainLoop() {
        while(!glfwWindowShouldClose(window)){  //update window until close cmd or error received
            glfwPollEvents();
        }
    }


    void cleanup() {                //Get rid of all redundant objects explicitly
        vkDestroyDevice(device, nullptr);
        
        if(enableValidationLayers){
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    
    /*
        General vulkan obj creation pattern:
        1) Pointer to struct with creation info
        2) Pointer to custom allocator callbacks, always nullptr in this tutorial
        3) Pointer to the variable that stores the handle to the new object
    */

    void createInstance(){
        if(enableValidationLayers && !checkValidationLayerSupport()){
            throw std::runtime_error("Requested validation layers not available!");
        }
        
        else{
            VkApplicationInfo appInfo{};                              //Optional application info handle for diagnostics
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            uint32_t glfwExtensionCount = 0;            //glfw required extensions are different than vk required extensions
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            VkInstanceCreateInfo createInfo{};                           //Tell Vulkan driver which global extensions and validation layers we want to use; <-extension info struct
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount = 0;


            if(enableValidationLayers){
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            auto extensions = getRequiredExtensions();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
                throw std::runtime_error("Failed to create instance");
            }
        }          
    }


    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }


    void setupDebugMessenger() {
        if(!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
            throw std::runtime_error("Failed to set up debug messenger");
        }
    }


    bool checkValidationLayerSupport(){
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);   //get layerCount number
        
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); //enumerates data members

        for (const char* layerName : validationLayers){ //search for each validation layer inside availableLayers
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers){
                if(strcmp(layerName, layerProperties.layerName) == 0){
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound){
                return false;
            }
        }
        return true;
    }


    std::vector<const char*> getRequiredExtensions(){
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);    //gets  extensionCount first extensions

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if(enableValidationLayers){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData) 
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    ////////////////////////////////////////// Physical device block ///////////////////////////////////////////////////////////////////////////////////////////////////////

    void pickPhysicalDevice(){
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);    //get number of devices

        if(deviceCount == 0){
            throw std::runtime_error("Failed to find GPU's with Vulkan support");
        }
    
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());     //Fill vector with device profiles

        for(const auto& device : devices){
            if(isDeviceSuitable(device)){
                physicalDevice = device;
                break;
            }
        }

        if(physicalDevice == VK_NULL_HANDLE){
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }


    bool isDeviceSuitable(VkPhysicalDevice device){
        QueueFamilyIndices indices = findQueueFamilies(device);
        return indices.graphicsFamily.has_value();
    }


    //////////////////////////////////////////// Queue Block ///////////////////////////////////////////////////////////////////////////////////////////

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        VkBool32 presentSupport = false;

        int i = 0;
        for(const auto& queueFamily : queueFamilies){
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices.graphicsFamily = i; //will always get the last valid device index
            }

            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if(presentSupport){
                indices.presentFamily = i;
            }

            if(indices.isComplete()) break;

            i++;
        }

        return indices;
    }


    void createLogicalDevice(){
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};   //Initialize queue vector

        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamilies){
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;


        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
            throw std::runtime_error("Failed to create logical device");
        }


        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    ///////////////// Window Surface Block ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    void createSurface(){
        if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("Failed to create window surface");
        }
    }


};




int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE; //predefined termination codes form stdexcept
    }

    return EXIT_SUCCESS;
}