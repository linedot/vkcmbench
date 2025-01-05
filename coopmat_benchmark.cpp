#include "coopmat_benchmark.hpp"
#include "coopmat_benchmark_shader.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>

void base_coopmat_benchmark::create_buffers(std::size_t a_type_size, std::size_t b_type_size, std::size_t c_type_size)
{
    VkBufferCreateInfo bci
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT|
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT|
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT|
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    bci.size  = (num_groups)*a_type_size*cmprops.MSize*cmprops.KSize;
    auto ret = vkCreateBuffer(device, &bci, nullptr, &a_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating a buffer");
    }
    ret = vkCreateBuffer(device, &bci, nullptr, &a_host_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating a buffer");
    }

    bci.size  = (num_groups)*b_type_size*cmprops.KSize*cmprops.NSize;
    ret = vkCreateBuffer(device, &bci, nullptr, &b_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating b buffer");
    }
    ret = vkCreateBuffer(device, &bci, nullptr, &b_host_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating b buffer");
    }

    bci.size  = (num_groups)*c_type_size*cmprops.MSize*cmprops.KSize*insts_in_block;
    ret = vkCreateBuffer(device, &bci, nullptr, &c_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating c buffer");
    }
    ret = vkCreateBuffer(device, &bci, nullptr, &c_host_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating c buffer");
    }

    bci.size = 3*sizeof(VkDeviceAddress);
    ret = vkCreateBuffer(device, &bci, nullptr, &devptr_buffer);
    if (ret != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating dev ptr buffer");
    }

    a_mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    b_mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    c_mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    devptr_mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

    VkBufferMemoryRequirementsInfo2 bmri{};
    bmri.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
    bmri.buffer = a_buffer;
    vkGetBufferMemoryRequirements2(device, &bmri, &a_mem_reqs);
    bmri.buffer = b_buffer;
    vkGetBufferMemoryRequirements2(device, &bmri, &b_mem_reqs);
    bmri.buffer = c_buffer;
    vkGetBufferMemoryRequirements2(device, &bmri, &c_mem_reqs);
    bmri.buffer = devptr_buffer;
    vkGetBufferMemoryRequirements2(device, &bmri, &devptr_mem_reqs);

    VkPhysicalDeviceMemoryProperties2 pdmp{};
    pdmp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(phy_device, &pdmp);

    auto a_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            a_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto a_host_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            a_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto b_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            b_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto b_host_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            b_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto c_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            c_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto c_host_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            c_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto devptr_host_heap_idx = vk_find_memory_type(
            &pdmp.memoryProperties,
            devptr_mem_reqs.memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if(a_heap_idx != b_heap_idx || b_heap_idx != c_heap_idx)
    {
        throw std::runtime_error("Got different heaps for matrix buffers");
    }
    if(a_host_heap_idx != b_host_heap_idx || b_host_heap_idx != c_host_heap_idx)
    {
        throw std::runtime_error("Got different heaps for matrix buffers");
    }

    VkMemoryAllocateFlagsInfo mafi
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
    };

    
    VkMemoryAllocateInfo mai{};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.pNext = &mafi;
    mai.allocationSize = a_mem_reqs.memoryRequirements.size +
                         b_mem_reqs.memoryRequirements.size +
                         c_mem_reqs.memoryRequirements.size;
    mai.memoryTypeIndex = a_heap_idx;
    
    vkAllocateMemory(device, &mai, nullptr, &dev_memory);
    mai.memoryTypeIndex = a_host_heap_idx;
    vkAllocateMemory(device, &mai, nullptr, &host_memory);
    mai.allocationSize = devptr_mem_reqs.memoryRequirements.size;
    mai.memoryTypeIndex = devptr_host_heap_idx;
    vkAllocateMemory(device, &mai, nullptr, &devptr_memory);

    std::vector<VkBindBufferMemoryInfo> bbmis(4);
    for(auto & bbmi : bbmis)
    {
        bbmi.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
        bbmi.memory = dev_memory;
    }
    bbmis[0].buffer = a_buffer;
    bbmis[0].memoryOffset = 0;
    bbmis[1].buffer = b_buffer;
    bbmis[1].memoryOffset = a_mem_reqs.memoryRequirements.size;
    bbmis[2].buffer = c_buffer;
    bbmis[2].memoryOffset = a_mem_reqs.memoryRequirements.size +
                            b_mem_reqs.memoryRequirements.size;
    bbmis[3].memory = devptr_memory;
    bbmis[3].memoryOffset = 0;
    bbmis[3].buffer = devptr_buffer;
    vkBindBufferMemory2(device, bbmis.size(), bbmis.data());
    for(auto& bbmi : bbmis)
    {
        bbmi.memory = host_memory;
    }
    bbmis[0].buffer = a_host_buffer;
    bbmis[1].buffer = b_host_buffer;
    bbmis[2].buffer = c_host_buffer;
    vkBindBufferMemory2(device, bbmis.size()-1, bbmis.data());


    PFN_vkGetBufferDeviceAddress _vkGetBufferDeviceAddress =
        reinterpret_cast<PFN_vkGetBufferDeviceAddress>(
            vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddress"));

    if(nullptr == _vkGetBufferDeviceAddress)
    {
        throw std::runtime_error("got null pointer for vkGetBufferDeviceAddress");
    }

    VkBufferDeviceAddressInfo bdai
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = a_buffer
    };
    auto a_devptr = _vkGetBufferDeviceAddress(device, &bdai);
    bdai.buffer = b_buffer;
    auto b_devptr = _vkGetBufferDeviceAddress(device, &bdai);
    bdai.buffer = c_buffer;
    auto c_devptr = _vkGetBufferDeviceAddress(device, &bdai);

    VkDeviceAddress* devptr_ptr;


    vkMapMemory(device, devptr_memory,
            0, devptr_mem_reqs.memoryRequirements.size,
            0, reinterpret_cast<void**>(&devptr_ptr));

    devptr_ptr[0] = a_devptr;
    devptr_ptr[1] = b_devptr;
    devptr_ptr[2] = c_devptr;

    vkUnmapMemory(device, devptr_memory);
}

void base_coopmat_benchmark::destroy_buffers()
{
    vkDestroyBuffer(device, a_buffer, nullptr);
    vkDestroyBuffer(device, b_buffer, nullptr);
    vkDestroyBuffer(device, c_buffer, nullptr);
    vkDestroyBuffer(device, devptr_buffer, nullptr);
    vkDestroyBuffer(device, a_host_buffer, nullptr);
    vkDestroyBuffer(device, b_host_buffer, nullptr);
    vkDestroyBuffer(device, c_host_buffer, nullptr);

    vkFreeMemory(device, dev_memory, nullptr);
    vkFreeMemory(device, host_memory, nullptr);
    vkFreeMemory(device, devptr_memory, nullptr);
}

void base_coopmat_benchmark::run(
        coopmat_benchmark_shader::configuration config,
        VkQueue queue,
        VkCommandBuffer command_buffer,
	std::uint32_t blocks_in_kernel)
{
    shader->finalize(cmprops, config);

    VkCommandBufferBeginInfo cbbi 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };


    VkDescriptorBufferInfo dbi
    {
        .buffer = devptr_buffer,
        .offset = 0,
        // So this will be larger than the buffer size, which is an error
        //.range = devptr_mem_reqs.memoryRequirements.size,
        .range = 3*sizeof(VkDeviceAddress),
    };

    VkWriteDescriptorSet wds
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = config.ds,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &dbi
    };

    vkUpdateDescriptorSets(device, 1, &wds, 0, nullptr);

    std::vector<std::uint64_t> timestamps(2*outer_iterations);

    VkQueryPoolCreateInfo qpci
    {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = static_cast<std::uint32_t>(timestamps.size()),
    };

    VkQueryPool query_pool;
    vkCreateQueryPool(device, &qpci, nullptr, &query_pool);


    std::uint32_t gpu_n = static_cast<std::uint32_t>(inner_iterations);
    vkBeginCommandBuffer(command_buffer, &cbbi);
    vkCmdResetQueryPool(command_buffer, query_pool, 0, timestamps.size());
    vkCmdPushConstants(command_buffer, config.pl, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(std::uint32_t), &gpu_n);
    vkCmdBindDescriptorSets(command_buffer,
            VK_PIPELINE_BIND_POINT_COMPUTE, config.pl, 
            0u, 1, &config.ds, 0, nullptr);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
            shader->get_pipeline());
    for(std::size_t i = 0; i < outer_iterations; i++)
    {
        vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, query_pool, i*2+0);
        vkCmdDispatch(command_buffer, num_groups, 1, 1);
        vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool, i*2+1);
    }
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo si
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };
    if (VK_SUCCESS != vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE))
    {
        throw std::runtime_error("Failed submitting command buffer to queue");
    }
    VkResult result = vkQueueWaitIdle(queue);
    // AMD windows driver will return VK_TIMEOUT, AMDGPU pro on linux will return VK_NOT_READY
    // I think neither are to spec, as the spec states that you can only either get VK_SUCCESS
    // or some VK_ERROR_* as a return value
    while((result == VK_TIMEOUT) || (result == VK_NOT_READY))
    {
        fmt::print("Timed out, waiting again\n");
        result = vkQueueWaitIdle(queue);
    }
    if (VK_SUCCESS != result)
    {
        fmt::print("Error waiting for queue: {}\n",string_VkResult(result));
        throw std::runtime_error("Failed waiting until queue idle");
    }

    vkGetQueryPoolResults(device, query_pool, 0, timestamps.size(), timestamps.size()*sizeof(std::uint64_t), timestamps.data(), sizeof(std::uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    vkDestroyQueryPool(device, query_pool, nullptr);

    std::uint64_t min_duration = timestamps[1] - timestamps[0];
    std::uint64_t avg_duration = 0;
    for(std::size_t i = 0; i < outer_iterations; i++)
    {
        std::uint64_t duration = timestamps[2*i+1] - timestamps[2*i+0];
        min_duration = std::min(duration, min_duration);
        avg_duration  += duration;
    }
    avg_duration /= outer_iterations;

    // TODO: should have been cached somewhere
    VkPhysicalDeviceProperties phy_dev_props;
    vkGetPhysicalDeviceProperties(phy_device, &phy_dev_props);

    auto min_nanoseconds = min_duration * phy_dev_props.limits.timestampPeriod;
    auto avg_nanoseconds = avg_duration * phy_dev_props.limits.timestampPeriod;

    auto ops = num_groups*inner_iterations*(cmprops.MSize*cmprops.NSize*cmprops.KSize*2)*insts_in_block*blocks_in_kernel;
    auto max_gops_per_sec = static_cast<double>(ops)/static_cast<double>(min_nanoseconds);
    auto avg_gops_per_sec = static_cast<double>(ops)/static_cast<double>(avg_nanoseconds);
    fmt::print("Took Min. {} ns\n", min_nanoseconds);
    fmt::print("Took Avg. {} ns\n", avg_nanoseconds);
    std::string op_prefix = "I";
    if( (cmprops.CType == VK_COMPONENT_TYPE_FLOAT64_KHR ) ||
        (cmprops.CType == VK_COMPONENT_TYPE_FLOAT32_KHR ) ||
        (cmprops.CType == VK_COMPONENT_TYPE_FLOAT16_KHR ))
    {
        op_prefix = "FL";
    }
    fmt::print("Max. {:.2f} G{}OP/s\n", max_gops_per_sec, op_prefix);
    fmt::print("Avg. {:.2f} G{}OP/s\n", avg_gops_per_sec, op_prefix);

}

void base_coopmat_benchmark::cleanup()
{
    shader->release();
}



#define TYPE_MAP(vk_type,cpp_type)\
template<> struct comp_type_map<vk_type>\
{\
    using type = cpp_type;\
};

#define PASTER(a,b,c) a ## b ## c 
#define EV(a,b,c) PASTER(a,b,c)

#define TYPE_MAP_MP(vk_type,cpp_type)\
TYPE_MAP(EV(VK_COMPONENT_TYPE_,vk_type,16_KHR), std::EV(,cpp_type,16_t))\
TYPE_MAP(EV(VK_COMPONENT_TYPE_,vk_type,32_KHR), std::EV(,cpp_type,32_t))\
TYPE_MAP(EV(VK_COMPONENT_TYPE_,vk_type,64_KHR), std::EV(,cpp_type,64_t))\

#define TYPE_MAP_MP8(vk_type,cpp_type)\
TYPE_MAP(EV(VK_COMPONENT_TYPE_,vk_type,8_KHR), std::EV(,cpp_type,8_t))\
TYPE_MAP_MP(vk_type,cpp_type)

TYPE_MAP_MP8(SINT, int)
TYPE_MAP_MP8(UINT, uint)
TYPE_MAP_MP(FLOAT, float)

#undef TYPE_MAP_MP8
#undef TYPE_MAP_MP
#undef TYPE_MAP


#define CCB(a,b,c)\
if(cmprops.AType == EV(VK_COMPONENT_TYPE_, a, _KHR) &&\
   cmprops.BType == EV(VK_COMPONENT_TYPE_, b, _KHR) &&\
   cmprops.CType == EV(VK_COMPONENT_TYPE_, c, _KHR) &&\
   cmprops.CType == cmprops.ResultType)\
{\
    using a_type = typename comp_type_map<EV(VK_COMPONENT_TYPE_, a, _KHR)>::type;\
    using b_type = typename comp_type_map<EV(VK_COMPONENT_TYPE_, b, _KHR)>::type;\
    using c_type = typename comp_type_map<EV(VK_COMPONENT_TYPE_, c, _KHR)>::type;\
    return std::make_unique<coopmat_benchmark<\
        a_type,\
        b_type,\
        c_type>>(\
        phy_device, device, cmprops,\
        insts_in_block, inner_iterations, outer_iterations, num_groups);\
}

std::unique_ptr<base_coopmat_benchmark> create_coop_benchmark(
        VkPhysicalDevice phy_device,
        VkDevice device,
        VkCooperativeMatrixPropertiesKHR cmprops,
        std::size_t insts_in_block,
        std::size_t inner_iterations,
        std::size_t outer_iterations,
        std::size_t num_groups)
{
    CCB(SINT8, SINT8, SINT32)
    CCB(UINT8, UINT8, UINT32)
    CCB(SINT8, SINT8, UINT32)
    CCB(UINT8, UINT8, SINT32)
    CCB(UINT8, SINT8, SINT32)
    CCB(SINT8, UINT8, SINT32)
    CCB(UINT8, SINT8, UINT32)
    CCB(SINT8, UINT8, UINT32)
    CCB(FLOAT16, FLOAT16, FLOAT16)
    CCB(FLOAT16, FLOAT16, FLOAT32)
    CCB(FLOAT32, FLOAT32, FLOAT32)
    CCB(FLOAT64, FLOAT64, FLOAT64)

    throw std::runtime_error("unsupported VkComponentTypeKHR");
}

#undef CCB
#undef EV
#undef PASTER
