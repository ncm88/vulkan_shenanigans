#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

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
        VkApplicationInfo appInfo;                              //Optional application info handle for diagnostics
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;


        VkInstanceCreateInfo createInfo;                           //Tell Vulkan driver which global extensions and validation layers we want to use
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); 

        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
            throw std::runtime_error("failed to create instance");
        }          
    }


    void initVulkan() {
        createInstance();
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