#ifndef COOPMAT_BENCHMARK_SHADER
#define COOPMAT_BENCHMARK_SHADER

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>


class coopmat_benchmark_shader
{
public:
    struct configuration
    {
        VkDescriptorSetLayoutBinding dslb;
        VkDescriptorSetLayout        dsl;
        VkDescriptorPool             dp;
        VkDescriptorSet              ds;
        VkPipelineLayout             pl;
        std::array<VkSpecializationMapEntry,3> smps;
        VkSpecializationInfo         si;
    };

    coopmat_benchmark_shader(
            VkDevice device,
            std::string_view   code_template,
            VkComponentTypeKHR a_vk_type,
            VkComponentTypeKHR b_vk_type,
            VkComponentTypeKHR c_vk_type,
            std::uint32_t subgroup_size,
            std::uint32_t insts_in_block,
            std::uint32_t blocks_in_kernel
	    );
    coopmat_benchmark_shader(
            const coopmat_benchmark_shader& other)
        :
            device(other.device),
            shader(other.shader), // TODO: check if this gets messed up when we finalize/create pipeline
            pipeline(other.pipeline), // Ehhhh... I kinda only have copying non-finalized shaders in mind
            specialized_code(other.specialized_code),
            subgroup_size(other.subgroup_size)
    {}
    ~coopmat_benchmark_shader()
    {
        // Can't do that here if we'll be sharing the shader modules
        //vkDestroyShaderModule(device, shader, nullptr);
    }

    static configuration create_configuration(VkDevice device);
    static void release_configuration(VkDevice device, configuration config)
    {
        vkDestroyPipelineLayout(device, config.pl, nullptr);
        vkFreeDescriptorSets(device, config.dp, 1, &config.ds);
        vkDestroyDescriptorPool(device, config.dp, nullptr);
        vkDestroyDescriptorSetLayout(device, config.dsl, nullptr);
    }

    void destroy_shared_module()
    {
        vkDestroyShaderModule(device, shader, nullptr);
    }

    void finalize(VkCooperativeMatrixPropertiesKHR cmprops,
                  configuration config);
    void release()
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
    VkPipeline get_pipeline()
    {
        return pipeline;
    }
private:
    VkDevice device;
    VkShaderModule shader;
    VkPipeline pipeline;
    std::string specialized_code;

    std::uint32_t subgroup_size;

};
#endif /* ifndef COOPMAT_BENCHMARK_SHADER */
