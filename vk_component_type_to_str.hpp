#ifndef VK_COMPONENT_TYPE_TO_STR
#define VK_COMPONENT_TYPE_TO_STR
#include <vulkan/vulkan.h>

#include <string_view>

constexpr std::string_view component_type_to_str(VkComponentTypeKHR type)
{
    switch(type)
    {
        case VK_COMPONENT_TYPE_FLOAT16_KHR: return "f16";
        case VK_COMPONENT_TYPE_FLOAT32_KHR: return "f32";
        case VK_COMPONENT_TYPE_FLOAT64_KHR: return "f64";
        case VK_COMPONENT_TYPE_SINT8_KHR: return "s8";
        case VK_COMPONENT_TYPE_SINT16_KHR: return "s16";
        case VK_COMPONENT_TYPE_SINT32_KHR: return "s32";
        case VK_COMPONENT_TYPE_SINT64_KHR: return "s64";
        case VK_COMPONENT_TYPE_UINT8_KHR: return "u8";
        case VK_COMPONENT_TYPE_UINT16_KHR: return "u16";
        case VK_COMPONENT_TYPE_UINT32_KHR: return "u32";
        case VK_COMPONENT_TYPE_UINT64_KHR: return "u64";
        default: return "bad_type";
    }
}
constexpr std::string_view component_type_to_glsl_type_str(VkComponentTypeKHR type)
{
    // GL_EXT_shader_explicit_arithmetic_types
    switch(type)
    {
        case VK_COMPONENT_TYPE_FLOAT16_KHR: return "float16_t";
        case VK_COMPONENT_TYPE_FLOAT32_KHR: return "float32_t";
        case VK_COMPONENT_TYPE_FLOAT64_KHR: return "float64_t";
        case VK_COMPONENT_TYPE_SINT8_KHR: return "int8_t";
        case VK_COMPONENT_TYPE_SINT16_KHR: return "int16_t";
        case VK_COMPONENT_TYPE_SINT32_KHR: return "int32_t";
        case VK_COMPONENT_TYPE_SINT64_KHR: return "int64_t";
        case VK_COMPONENT_TYPE_UINT8_KHR: return "uint8_t";
        case VK_COMPONENT_TYPE_UINT16_KHR: return "uint16_t";
        case VK_COMPONENT_TYPE_UINT32_KHR: return "uint32_t";
        case VK_COMPONENT_TYPE_UINT64_KHR: return "uint64_t";
        default: return "bad_type";
    }
}
#endif /* ifndef VK_COMPONENT_TYPE_TO_STR */
