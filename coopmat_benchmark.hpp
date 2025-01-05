#include <algorithm>
#include <cstdint>
#include <memory>
#include <map>
#include <stdfloat>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <vulkan/vulkan.h>

#include "vk_find_memory_type.hpp"
#include "vk_component_type_to_str.hpp"
#include "coopmat_benchmark_shader.hpp"

template<VkComponentTypeKHR vk_type> struct comp_type_map;

class base_coopmat_benchmark
{
public:
    base_coopmat_benchmark(
            VkPhysicalDevice phy_device, 
            VkDevice device, 
            VkCooperativeMatrixPropertiesKHR cmprops,
            std::size_t insts_in_block,
            std::size_t inner_iterations,
            std::size_t outer_iterations,
            std::size_t num_groups
            ) 
        : phy_device(phy_device),
          device(device),
          cmprops(cmprops),
          insts_in_block(insts_in_block),
          inner_iterations(inner_iterations),
          outer_iterations(outer_iterations),
          num_groups(num_groups)
    {}
    virtual ~base_coopmat_benchmark() = default;

    void set_shader(std::shared_ptr<coopmat_benchmark_shader> shader)
    {
        this->shader = shader;
    }

    void create_descriptors();

    virtual void create_buffers() = 0;
    void destroy_buffers();

    auto get_cmprops() -> auto
    {
        return cmprops;
    }

    auto get_device() -> VkDevice
    {
        return device;
    }

    void run(coopmat_benchmark_shader::configuration config,
            VkQueue queue, VkCommandBuffer command_buffer,
	    std::uint32_t blocks_in_kernel);
    void cleanup();
protected:
    VkPhysicalDevice phy_device;
    VkDevice device;
    VkCooperativeMatrixPropertiesKHR cmprops;

    std::shared_ptr<coopmat_benchmark_shader> shader;

    std::size_t insts_in_block;
    std::size_t inner_iterations;
    std::size_t outer_iterations;
    std::size_t num_groups;

    VkBuffer a_buffer;
    VkBuffer b_buffer;
    VkBuffer c_buffer;
    VkBuffer a_host_buffer;
    VkBuffer b_host_buffer;
    VkBuffer c_host_buffer;

    VkBuffer devptr_buffer;

    VkMemoryRequirements2 a_mem_reqs{}, b_mem_reqs{}, c_mem_reqs{}, devptr_mem_reqs{};
    VkDeviceMemory dev_memory;
    VkDeviceMemory host_memory;
    VkDeviceMemory devptr_memory;

    void create_buffers(std::size_t a_type_size, std::size_t b_type_size, std::size_t c_type_size);
};

template<typename a_type, typename b_type, typename c_type>
class coopmat_benchmark : public base_coopmat_benchmark
{
public:
    coopmat_benchmark(
            VkPhysicalDevice phy_device, 
            VkDevice device, 
            VkCooperativeMatrixPropertiesKHR cmprops,
            std::size_t insts_in_block,
            std::size_t inner_iterations,
            std::size_t outer_iterations,
            std::size_t num_groups
            ) 
        : base_coopmat_benchmark(
                phy_device,
                device,
                cmprops,
                insts_in_block,
                inner_iterations,
                outer_iterations,
                num_groups),
        a_mapped_ptr(nullptr),
        b_mapped_ptr(nullptr),
        c_mapped_ptr(nullptr)
    {}
    virtual ~coopmat_benchmark() = default;


    virtual void create_buffers()
    {
        base_coopmat_benchmark::create_buffers(sizeof(a_type), sizeof(b_type), sizeof(c_type));
    }


private:

    a_type* a_mapped_ptr;
    b_type* b_mapped_ptr;
    c_type* c_mapped_ptr;
};


std::unique_ptr<base_coopmat_benchmark> create_coop_benchmark(
        VkPhysicalDevice phy_device,
        VkDevice device,
        VkCooperativeMatrixPropertiesKHR cmprops,
        std::size_t insts_in_block,
        std::size_t inner_iterations,
        std::size_t outer_iterations,
        std::size_t num_groups);
