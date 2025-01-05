#include "coopmat_benchmark_shader.hpp"
#include "vk_component_type_to_str.hpp"

#include <fmt/format.h>
#include <shaderc/shaderc.h>
#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <numeric>

coopmat_benchmark_shader::coopmat_benchmark_shader(
        VkDevice device,
        std::string_view code_template,
        VkComponentTypeKHR a_vk_type,
        VkComponentTypeKHR b_vk_type,
        VkComponentTypeKHR c_vk_type,
        std::uint32_t subgroup_size,
        std::uint32_t insts_in_block,
	std::uint32_t blocks_in_kernel)
    : device(device),
      subgroup_size(subgroup_size)
{
    specialized_code = code_template;

    using pss = std::pair<std::string, std::string>;

    std::vector<pss> replacements
    {
        pss{"A_TYPE", component_type_to_glsl_type_str(a_vk_type)},
        pss{"B_TYPE", component_type_to_glsl_type_str(b_vk_type)},
        pss{"C_TYPE", component_type_to_glsl_type_str(c_vk_type)},
        pss{"INST_COUNT", fmt::format("{}", insts_in_block)},
        pss{"BLOCKS_IN_KERNEL", fmt::format("{}", blocks_in_kernel)},
        pss{"SUBGRP_SIZE", fmt::format("{}", subgroup_size)},
    };

    // Actually just use shaderc s macro function
    // TODO: move to some snippet repo for reference
    // TODO: if you found this again and want to copy-paste it somewhere else,
    //       remember that for multiple variables it iterates through
    //       the whole string every time. rewrite it to be more efficient
    //auto replace_all = [](auto& s, auto look_for, auto replace_with)
    //{
    //    std::string temporary;

    //    temporary.reserve(s.size());
    //    std::size_t current = 0;
    //    for(std::size_t next=s.find(look_for);
    //        next != std::string::npos;
    //        next=s.find(look_for, current))
    //    {
    //        temporary.append(s, current, next-current);
    //        temporary.append(replace_with, 0, replace_with.size());
    //        current = next + look_for.size();
    //    }
    //    temporary.append(s, current, s.size()-current);

    //    s.swap(temporary);
    //};
    //
    //for (auto const& [var, val] : replacements)
    //{
    //    std::string search_for = fmt::format("${{{}}}", var);
    //    replace_all(specialized_code, search_for, val);
    //}

    auto compiler = shaderc_compiler_initialize();
    auto options = shaderc_compile_options_initialize();

    for(const auto& kv : replacements)
    {
        const auto& key = std::get<0>(kv);
        const auto& value = std::get<1>(kv);
        shaderc_compile_options_add_macro_definition(
                options,
                key.data(), key.size(),
                value.data(), value.size());
    }

    shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
    shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    auto result = shaderc_compile_into_spv(
            compiler, 
            specialized_code.data(), specialized_code.size(), 
            shaderc_compute_shader, "coopmat.comp.glsl", "main",
            options);

    if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
    {
        fmt::print("GLSL->SPIR-V compilation failed with: {}\n", shaderc_result_get_error_message(result));
        throw std::runtime_error("GLSL to SPIR-V compilation failed");
    }


    const std::uint32_t* spv_ptr = reinterpret_cast<const uint32_t*>(shaderc_result_get_bytes(result));


    VkShaderModuleCreateInfo smci{};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.pCode = spv_ptr;
    smci.codeSize = shaderc_result_get_length(result);

    vkCreateShaderModule(device, &smci, nullptr, &shader);

    fmt::print("Shader module {:x} created for device {:x}\n",
		    reinterpret_cast<std::size_t>(shader),
		    reinterpret_cast<std::size_t>(device));

    shaderc_result_release(result);
    shaderc_compile_options_release(options);
    shaderc_compiler_release(compiler);
}

coopmat_benchmark_shader::configuration coopmat_benchmark_shader::create_configuration(VkDevice device)
{
    configuration config;

    config.dslb = VkDescriptorSetLayoutBinding
    {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
    };

    VkDescriptorSetLayoutCreateInfo dslci
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &config.dslb,
    };

    if(VK_SUCCESS != vkCreateDescriptorSetLayout(device, &dslci, nullptr, &config.dsl))
    {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    VkPushConstantRange pcr
    {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(uint32_t),
    };

    VkPipelineLayoutCreateInfo plci
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &config.dsl,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pcr,
    };

    if(VK_SUCCESS != vkCreatePipelineLayout(device, &plci, nullptr, &config.pl))
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    std::array<VkDescriptorPoolSize,1> sizes
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
    };

    VkDescriptorPoolCreateInfo dpci
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = sizes.data(),
    };

    if(VK_SUCCESS != vkCreateDescriptorPool(device, &dpci, nullptr, &config.dp))
    {
        throw std::runtime_error("Failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo dsai
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = config.dp,
        .descriptorSetCount = 1,
        .pSetLayouts = &config.dsl,
    };


    if(VK_SUCCESS != vkAllocateDescriptorSets(device, &dsai, &config.ds))
    {
        throw std::runtime_error("Failed to Allocate Descriptor Sets");
    }


    // Not initializing this was the source of many errors
    // and static analysis tools didn't pick it up!
    config.si.dataSize = 0;
    for (std::size_t i = 0; i < config.smps.size(); i++)
    {
        auto& smp = config.smps[i];
        smp.constantID = i;
        config.si.dataSize += smp.size = sizeof(std::uint32_t);
        smp.offset = i*sizeof(std::uint32_t);
    }
    config.si.mapEntryCount = config.smps.size();
    config.si.pMapEntries = config.smps.data();

    return config;
}

// TODO: This seems wrong, maybe config should be a hidden private singleton? But then when to create/destroy it...
void coopmat_benchmark_shader::finalize(
        VkCooperativeMatrixPropertiesKHR cmprops,
        coopmat_benchmark_shader::configuration config)
{
    using std::bind, std::accumulate, std::bind, std::plus;
    using namespace std::placeholders;

    // TODO: this should be bound tighter to the array of VkSpecializationMapEntry
    std::array<std::uint32_t,3> mnk_values
    {
        cmprops.MSize,
        cmprops.NSize,
        cmprops.KSize
    };
    config.si.pData = mnk_values.data();
    
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfo pssrssc
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO,
        .requiredSubgroupSize = subgroup_size
    };

    VkPipelineShaderStageCreateInfo pssci
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &pssrssc,
        .flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shader,
        .pName = "main",
        .pSpecializationInfo = &config.si,
    };

    VkComputePipelineCreateInfo cpci
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = pssci,
        .layout = config.pl,
    };



    auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipeline);
    if(VK_SUCCESS != result)
    {
        fmt::print("Error: {}\n",string_VkResult(result));
        throw std::runtime_error("Failed to create compute pipeline");
    }

}
