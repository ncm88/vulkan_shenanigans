/*
Basic Process:

1) Create window

2) Build appInfo struct (optional)

3) Build VkInstanceCreateInfo struct for passing to VkInstance constructor
*/


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string.h>
#include <vector>


const std::vector<const char*> validationLayers = {     //Like extensions, validation layers need to be enabled by specifying their name
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif




const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


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
    VkDebugUtilsLabelEXT debugMessenger;


    void initWindow(){
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // Set window behaviour characteristics
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
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

            VkInstanceCreateInfo createInfo{};                           //Tell Vulkan driver which global extensions and validation layers we want to use
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
                throw std::runtime_error("failed to create instance");
            }
        }          
    }
    

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
    }

    void setupDebugMessenger() {
        if(!enableValidationLayers) return;
    }





    void mainLoop() {
        while(!glfwWindowShouldClose(window)){  //update window until close cmd or error received
            glfwPollEvents();
        }
    }


    void cleanup() {                //Get rid of all redundant objects explicitly
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
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
        VkDebugUtilsMessageTypeFlagBitsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ){
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
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