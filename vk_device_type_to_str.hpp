#include <string_view>

#include <vulkan/vulkan.h>


constexpr std::string_view device_type_to_str(const VkPhysicalDeviceType type)
{
    if(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU == type)
    {
        return "CPU";
    }
    if(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU == type)
    {
        return "Integrated GPU";
    }
    if(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == type)
    {
        return "Discrete GPU";
    }
    if(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU == type)
    {
        return "Virtual GPU";
    }
    if(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_OTHER == type)
    {
        return "Other";
    }

    return "Unknown";
}
