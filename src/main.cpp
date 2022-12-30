#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace glfw {

struct ExtensionInfo
{
    uint32_t count;
    const char** extensions;
};

struct Instance
{
    Instance ()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // using Vulkan, not OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // disable resizing
    }
    
    ~Instance ()
    {
        glfwTerminate();
    }

    ExtensionInfo extensionInfo() const
    {
        uint32_t count = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);
        return {count, extensions};
    }
};

class Window
{
public:
    Window (int w, int h, const char* title)
    :
        mpWindow {glfwCreateWindow(w, h, title, nullptr, nullptr)}
    {}

    ~Window ()
    {
        glfwDestroyWindow(mpWindow);
    }

    void eventLoop()
    {
        while(!glfwWindowShouldClose(mpWindow)) {  // main loop
            glfwPollEvents();
        }
    }

private:
    GLFWwindow* mpWindow;
};

}  // namespace glfw

namespace vk 
{

namespace {  // namespace

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT             messageType,
    [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,

    [[maybe_unused]] void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
    return VK_FALSE;
}

}  // namespace

template<typename FuncPtr>
FuncPtr getExtensionFunction(VkInstance instance, const char* funcName)
{
    FuncPtr func = reinterpret_cast<FuncPtr>(vkGetInstanceProcAddr(instance, funcName));
    if (func) {
        return func;
    }
    throw std::runtime_error("function not found");
}

const std::vector<const char*> cValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Instance
{
public:
    Instance()
    {
        m_appInfo = {
              .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO
            , .pApplicationName   = "Hello Triangle"
            , .applicationVersion = VK_MAKE_VERSION(1, 0, 0)
            , .pEngineName        = "No Engine"
            , .engineVersion      = VK_MAKE_VERSION(1, 0, 0)
            , .apiVersion         = VK_API_VERSION_1_0
        };

        populateDebugUtilsMessengerCreateInfo();

        if (!CheckValidationLayers(cValidationLayers)) {
            throw std::runtime_error ("no validation layers found");
        }

        m_createInfo = VkInstanceCreateInfo {
              .sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
            , .pNext                 = m_isValidationLayersPresent != 0 ? &m_debugUtilsMessengerCreateInfo : nullptr
            , .pApplicationInfo      = &m_appInfo
            , .enabledLayerCount     = m_isValidationLayersPresent ? Instance::ValidationLayerCount() : 0
            //, .ppenabledLayerNames     = 
            , .enabledExtensionCount = Instance::ExtensionCount() //glfw::Library::extension_info().count
            //, .ppEnabledExtensionNames = glfw::Library::extension_info().extensions
        };

        if (vkCreateInstance(&m_createInfo, nullptr, &m_instance) != VK_SUCCESS) {
                                            // allocator
            throw std::runtime_error ("cannot create instance");
        }

        if (m_isValidationLayersPresent) {
            setupDebugUtilsMessenger();
        }
    }

    ~Instance()
    {
        if (m_isValidationLayersPresent) {
            //destroyDebugUtilsMessenger();
        }
    }
    
    static uint32_t ExtensionCount()
    {
        static uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        return extensionCount;
    }
    
    static uint32_t ValidationLayerCount()
    {
        static uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        return layerCount;
    }

    bool CheckValidationLayers(const std::vector<const char*>& validationLayers) const
    {
        std::vector<VkLayerProperties> availableLayers (Instance::ValidationLayerCount());
        vkEnumerateInstanceLayerProperties(nullptr, availableLayers.data());
        
        bool layerFound = false;
        for (const auto& layer : validationLayers)
        {
            for (const auto& layerProp : availableLayers)
            {
                if (std::string {layerProp.layerName} == layer)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
                return false;
        }
        auto getNames = [&availableLayers]() { 
            std::vector<const char*> names {};
            std::transform(availableLayers.begin(), availableLayers.end(), std::back_inserter(names), [](const auto& l){ return l.layerName; });
            return names;
        };
        m_validationLayerNames = getNames();

        m_createInfo.ppEnabledLayerNames = m_validationLayerNames.data();
        m_isValidationLayersPresent = true;
        return true;
    }

    void populateDebugUtilsMessengerCreateInfo()
    {
        m_debugUtilsMessengerCreateInfo = VkDebugUtilsMessengerCreateInfoEXT {
              .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
            , .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT   // types of errors for which callback will be called
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            , .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            , .pfnUserCallback = debugCallback
            , .pUserData       = nullptr  // Optional
        };
    }

    void setupDebugUtilsMessenger()
    {
        auto vkCreateDebugUtilsMessengerEXT = getExtensionFunction<PFN_vkCreateDebugUtilsMessengerEXT>(m_instance, "vkCreateDebugUtilsMessengerEXT");
        vkCreateDebugUtilsMessengerEXT(m_instance, &m_debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger);
                                                                // allocator
    }

    void destroyDebugUtilsMessenger()
    {
        auto vkDestroyDebugUtilsMessengerEXT = getExtensionFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugUtilsMessenger, nullptr);
                                                                      // allocation callbacks
    }

private:
    /* VkInstance related */
    VkApplicationInfo m_appInfo;
    mutable VkInstanceCreateInfo m_createInfo;
    VkInstance m_instance;

    /* validation layers related */
    mutable bool m_isValidationLayersPresent = false;
    mutable std::vector<const char*> m_validationLayerNames;
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
    VkDebugUtilsMessengerCreateInfoEXT m_debugUtilsMessengerCreateInfo;
};

} // namespace vk

int main() 
{
    glfw::Instance glfw {};
    glfw::Window window {800, 600, "Vulkan"};

    std::cout << vk::Instance::ExtensionCount() << " extensions supported\n";
    std::cout << vk::Instance::ValidationLayerCount() << " validation layers\n";

    vk::Instance instance {};

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    window.eventLoop();

    return 0;
}
