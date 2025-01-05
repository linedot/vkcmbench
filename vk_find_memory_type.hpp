#include <stdexcept>
#include <cstdint>

#include <vulkan/vulkan.h>

inline std::int32_t vk_find_memory_type(
        const VkPhysicalDeviceMemoryProperties* pMemoryProperties,
        std::uint32_t memoryTypeBitsRequirement,
        VkMemoryPropertyFlags requiredProperties) 
{
    const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
    for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) 
    {
        const uint32_t memoryTypeBits = (1 << memoryIndex);
        const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

        const VkMemoryPropertyFlags properties =
            pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
        const bool hasRequiredProperties =
            (properties & requiredProperties) == requiredProperties;

        if (isRequiredMemoryType && hasRequiredProperties)
            return static_cast<int32_t>(memoryIndex);
    }

    throw std::runtime_error("Failed to find Vulkan memory type");
}
