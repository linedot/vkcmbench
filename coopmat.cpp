#include "coopmat_benchmark.hpp"
#include "coopmat_benchmark_shader.hpp"
#include "vk_component_type_to_str.hpp"
#include "vk_device_type_to_str.hpp"

#include <fmt/core.h>
#include <vulkan/vulkan.h>

#include <boost/container_hash/hash.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdfloat>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

inline auto ITOP(VkInstance instance) -> std::uint64_t
{
    return reinterpret_cast<std::uint64_t>(instance);
}

template<typename T, typename S>
constexpr void set_vk_stypes(T& container, S stype)
{
    for(auto& element : container)
    {
        element.sType = stype;
        element.pNext = nullptr;
    }
}

// From Vulkan-Tools/vulkaninfo
std::string driver_version_to_str( 
        VkPhysicalDeviceProperties2 device_props,
        VkPhysicalDeviceVulkan12Properties vk12_props)
{
    uint32_t v = device_props.properties.driverVersion;
    if ((vk12_props.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY) || (device_props.properties.deviceID == 4318))
    {
        return std::to_string((v >> 22) & 0x3ff) + "." + std::to_string((v >> 14) & 0x0ff) + "." +
               std::to_string((v >> 6) & 0x0ff) + "." + std::to_string(v & 0x003f);
    } 
    else if ((vk12_props.driverID == VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS)
#if defined(WIN32)
               || (props.deviceID == 0x8086)  // only do the fallback check if running in windows
#endif
    ) 
    {
        return std::to_string(v >> 14) + "." + std::to_string(v & 0x3fff);
    } 
    else 
    {
        // AMD uses the standard vulkan scheme
        
        uint32_t major = VK_API_VERSION_MAJOR(v);
        uint32_t minor = VK_API_VERSION_MINOR(v);
        uint32_t patch = VK_API_VERSION_PATCH(v);
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
}

int main()
{
    // Depending on the implementation, you might need to get all functions with
    // vkGetInstanceProcAddr(). This can also include vkGetInstanceProcAddr (Yes,
    // it's weird) and global commands that should work without an instance

    // Let's check out what kind of layers we have
    std::uint32_t layer_count = 0;
    std::vector<VkLayerProperties> layer_properties;

    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    fmt::print("Number of Layers: {}\n", layer_count);

    layer_properties.resize(layer_count);

    vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());


    std::vector<const char*> wanted_layers;

    // We want khronos validation (VK_LAYER_KHRONOS_validation) and ignore all other stuff
    #if !defined(NDEBUG) // there is an std::bad_alloc during vkCreateComputePipelines somewhere in validation layers on -O3 and higher
    for (std::uint32_t i = 0; i < layer_count; i++)
    {

        if (std::string_view("VK_LAYER_KHRONOS_validation") == std::string_view(static_cast<const char*>(layer_properties[i].layerName)))
        {
            wanted_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
    }
    #endif

    // Let's do the same for extensions
    std::uint32_t extension_count = 0;
    std::vector<VkExtensionProperties> extension_properties;

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    extension_properties.resize(extension_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_properties.data());

    // Ok, now let's create an actual vulkan instance

    VkInstance instance = nullptr;
    std::uint32_t temporary_count = wanted_layers.size();


    // You should provide application info. This is also where you specify the VK API version you request
    VkApplicationInfo app_info =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "vulkan-coopmat-example",
        .applicationVersion = 0x02,
        .pEngineName = "no-engine",
        .engineVersion = 0x01,
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo instance_create_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        // Here we ask for the layers we want
        .enabledLayerCount = temporary_count,
        .ppEnabledLayerNames = wanted_layers.data(),
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr
    };

    auto res = vkCreateInstance(&instance_create_info, nullptr, &instance);

    if(VK_SUCCESS != res)
    {
        fmt::print("Error creating Vulkan Instance: {}\n", std::uint64_t(res));
        return -1;
    }

    fmt::print("Vulkan instance created successfully: {:#08X}\n", ITOP(instance));


    // Let's see what kind of physical devices we have
    std::uint32_t physical_device_count = 0;
    std::vector<VkPhysicalDevice> physical_devices;

    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
    physical_devices.resize(physical_device_count);

    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

    // Let's cycle through the physical device properties and see what these devices are and can do
    for(std::uint32_t i = 0; i < physical_device_count; i++)
    {
        VkPhysicalDeviceVulkan12Properties pdv12p =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
            .pNext = nullptr
        };
        VkPhysicalDeviceProperties2 properties =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &pdv12p
        };
        auto phy_device = physical_devices[i];
        vkGetPhysicalDeviceProperties2(phy_device, &properties);

        fmt::print("Physical device {}:\n", i);
        fmt::print("  Name:        {}\n", properties.properties.deviceName);
        fmt::print("  Device type: {}\n", device_type_to_str(properties.properties.deviceType));
        fmt::print("  Driver Name: {}\n", pdv12p.driverName);
        fmt::print("  Driver Ver.: {}\n", driver_version_to_str(properties, pdv12p));

    }
    

    std::vector<std::size_t> pd_to_use;
    std::vector<std::size_t> pd_to_check_pq;

    std::vector<const char*> device_extensions_to_enable{
        "VK_KHR_cooperative_matrix",
        // Let's look into that later
        //"VK_KHR_pipeline_executable_properties",
        "VK_KHR_buffer_device_address"
    };

    for(std::uint32_t i = 0; i < physical_device_count; i++)
    {
        auto phy_dev = physical_devices[i];
        // Let's query device layers and extensions

        std::uint32_t extension_count = 0;
        std::vector<VkExtensionProperties> extension_props;
        vkEnumerateDeviceExtensionProperties(phy_dev, nullptr, &extension_count, nullptr);
        extension_props.resize(extension_count);
        vkEnumerateDeviceExtensionProperties(phy_dev, nullptr, &extension_count, extension_props.data());

        for (auto& eprops : extension_props)
        {
            // I want to know what kind of instructions are supported in VK_KHR_cooperative_matrix
            if (eprops.extensionName == std::string("VK_KHR_cooperative_matrix"))
            {
                pd_to_use.push_back(i);
            }
            if (eprops.extensionName == std::string("VK_KHR_performance_query"))
            {
                pd_to_check_pq.push_back(i);
            }
        }
    }
    if(pd_to_use.empty())
    {
        fmt::print("No device supports VK_KHR_cooperative_matrix!\n");
    }
    // This is for another time
    //for(auto pd_idx : pd_to_check_pq)
    //{
    //    auto phy_dev = physical_devices[pd_idx];

    //    std::vector<VkQueueFamilyProperties2> qfps;
    //    std::uint32_t queue_count;
    //    vkGetPhysicalDeviceQueueFamilyProperties2(phy_dev, &queue_count, nullptr);
    //    qfps.resize(queue_count);
    //    for(auto& qfp : qfps){qfp.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;}
    //    vkGetPhysicalDeviceQueueFamilyProperties2(phy_dev, &queue_count, qfps.data());


    //    for(std::size_t i = 0; i < qfps.size(); i++)
    //    {

    //        PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
    //            _vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
    //            reinterpret_cast<PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR>(
    //            vkGetInstanceProcAddr(instance,
    //                    "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR"));

    //        std::uint32_t counter_count;
    //        _vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(phy_dev, i, &counter_count, nullptr, nullptr);

    //        fmt::print("PDev {}; queue Family {} has {} performance counters:\n", pd_idx, i, counter_count);
    //        std::vector<VkPerformanceCounterKHR> counters(counter_count);
    //        for(auto & c : counters) c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
    //        std::vector<VkPerformanceCounterDescriptionKHR> descriptions(counter_count);
    //        for(auto & d : descriptions) d.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR;
    //        _vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    //                phy_dev, i,
    //                &counter_count,
    //                counters.data(),
    //                descriptions.data());

    //        for(std::uint32_t j = 0; j < counter_count; j++)
    //        {
    //            auto desc = descriptions[j];
    //            fmt::print("    Counter {}:{} - {}\n", desc.category, desc.name, desc.description);
    //        }
    //    }
    //}

    std::vector<VkDevice> devices;

    std::array<float,1> queue_priorities{{1.0f}};


    std::vector<std::unique_ptr<base_coopmat_benchmark>> benchmarks;

    using dpkey = std::pair<VkDevice,VkCooperativeMatrixPropertiesKHR>;

    auto cm_hash = [](dpkey dev_prop) -> std::size_t
    {
        std::size_t hash = 0;
        boost::hash_combine(hash, std::get<0>(dev_prop));
        boost::hash_combine(hash, std::get<1>(dev_prop).AType);
        boost::hash_combine(hash, std::get<1>(dev_prop).BType);
        boost::hash_combine(hash, std::get<1>(dev_prop).CType);

        return hash;
    };
    auto cm_equal = [](dpkey dev_prop1,
                       dpkey dev_prop2) -> bool
    {
        return (std::get<1>(dev_prop1).AType == std::get<1>(dev_prop2).AType) &&
               (std::get<1>(dev_prop1).BType == std::get<1>(dev_prop2).BType) &&
               (std::get<1>(dev_prop1).CType == std::get<1>(dev_prop2).CType) &&
               (std::get<0>(dev_prop1) == std::get<0>(dev_prop2));
    };
    // TODO: better way of storing this
    std::unordered_map<
        dpkey,
        std::shared_ptr<coopmat_benchmark_shader>,
        decltype(cm_hash),
        decltype(cm_equal)> shaders(10, cm_hash, cm_equal);


    // These numbers go to ~ 95% FLOPS on GH200 (of what you can do with Vulkan - which is somewhat 
    // en par with MMA/WMMA CUDA, but falls short of what you can get with WGMMA).
    // You can get a bit more by making them bigger and can make them somewhat smaller and still get > 90%

    // Determining the optimum for these numbers is a good idea for an auto-tuner
    constexpr std::uint32_t blocks_in_kernel = 4;
    constexpr std::uint32_t insts_in_block = 8;
    // Not sure what to base this number on, I guess it should be something like
    // number_of_compute_units*warps_per_sm, but vulkan doesn't expose functionality
    // to get those numbers
    constexpr std::uint32_t num_groups = 132*8;

    // loop size inside the kernel
    constexpr std::uint32_t inner_iterations = 256;

    // number of measurements to take
    constexpr std::uint32_t num_repetitions = 10;


    std::ifstream spv_template_stream("coopmat.comp.glsl.in");
    std::string code_str;
    std::stringstream code_stream;

    code_stream << spv_template_stream.rdbuf();
    code_str = code_stream.str();

    // TODO: I think I need to make a per-device abstraction of some kind to store this kind of data
    std::unordered_map<VkDevice, coopmat_benchmark_shader::configuration> device_shader_configs;
    std::unordered_map<VkDevice, std::vector<VkDeviceQueueCreateInfo>> device_dqcis;
    for(auto pd_idx : pd_to_use)
    {
        auto phy_dev = physical_devices[pd_idx];



        std::vector<VkQueueFamilyProperties2> qfps;
        std::uint32_t queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties2(phy_dev, &queue_count, nullptr);
        qfps.resize(queue_count);
        for(auto& qfp : qfps){qfp.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;}
        vkGetPhysicalDeviceQueueFamilyProperties2(phy_dev, &queue_count, qfps.data());


        VkDeviceQueueCreateInfo dqci{};
        dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        std::vector<VkDeviceQueueCreateInfo> dqcis;
        for(std::size_t i = 0; i < qfps.size(); i++)
        {
            const auto& qfp = qfps[i];
            if (qfp.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                dqci.queueFamilyIndex = i;
                dqci.queueCount = 1;
                dqci.pQueuePriorities = queue_priorities.data();
                dqcis.push_back(dqci);
            }
        }


        VkPhysicalDeviceCooperativeMatrixFeaturesKHR pdcmf = 
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR,
            .pNext = nullptr,
            .cooperativeMatrix = VK_TRUE,
            .cooperativeMatrixRobustBufferAccess = VK_FALSE,
        };

        VkPhysicalDeviceVulkan11Features pdv11f = 
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &pdcmf,
            .storageBuffer16BitAccess = VK_TRUE,
        };

        VkPhysicalDeviceVulkan12Features pdv12f = 
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &pdv11f,
            .storageBuffer8BitAccess = VK_TRUE,
            .shaderFloat16 = VK_TRUE,
            .shaderInt8 = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE,
            .vulkanMemoryModel = VK_TRUE,
            .vulkanMemoryModelDeviceScope = VK_TRUE,
        };

        VkPhysicalDeviceVulkan13Features pdv13f = 
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &pdv12f,
            .subgroupSizeControl = VK_TRUE,
            .computeFullSubgroups = VK_TRUE,
        };

        VkDeviceCreateInfo dci{};
        dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dci.pNext = &pdv13f;
        dci.enabledExtensionCount = device_extensions_to_enable.size();
        dci.ppEnabledExtensionNames = device_extensions_to_enable.data();
        dci.queueCreateInfoCount = dqcis.size();
        dci.pQueueCreateInfos = dqcis.data();

        VkDevice device;
        vkCreateDevice(phy_dev, &dci, nullptr, &device);

        fmt::print("Device created, handle: {:x}\n", reinterpret_cast<std::size_t>(device));

        device_dqcis[device] = dqcis;


        fmt::print("Physical device {}:\n", pd_idx);

        VkPhysicalDeviceCooperativeMatrixPropertiesKHR pdcmp
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR
        };
        VkPhysicalDeviceVulkan11Properties pdv11p
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
            .pNext = &pdcmp,
        };
        VkPhysicalDeviceSubgroupSizeControlProperties pdsgscp =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES,
            .pNext = &pdv11p
        };
        VkPhysicalDeviceProperties2 properties =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &pdsgscp
        };
        vkGetPhysicalDeviceProperties2(phy_dev, &properties);

        fmt::print("def Subgroup size: {}\n", pdv11p.subgroupSize);
        fmt::print("min Subgroup size: {}\n", pdsgscp.minSubgroupSize);
        fmt::print("max Subgroup size: {}\n", pdsgscp.maxSubgroupSize);

        fmt::print("    Supports Cooperative Matrices in the following shader stages:\n");

        auto check_stage = [](auto bits, auto bit_to_check, auto string)
        {
            if ((bits & bit_to_check) == bit_to_check)
            {
                fmt::print("        {}\n", string);
            }
        };
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_COMPUTE_BIT, "compute");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_VERTEX_BIT, "vertex");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_GEOMETRY_BIT, "geometry");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_FRAGMENT_BIT, "fragment");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tess. control");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tess. evaluation");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_RAYGEN_BIT_KHR, "raygen");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_MISS_BIT_KHR, "miss");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, "any hit");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, "closest hit");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, "intersection");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_CALLABLE_BIT_KHR, "callable");

        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_TASK_BIT_EXT, "task");
        check_stage(pdcmp.cooperativeMatrixSupportedStages, VK_SHADER_STAGE_MESH_BIT_EXT, "mesh");


        PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR _vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = nullptr;

        _vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = 
            reinterpret_cast<PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR"));
        std::vector<VkCooperativeMatrixPropertiesKHR> cmprops;
        std::uint32_t prop_count = 0;
        _vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phy_dev, &prop_count, nullptr);
        cmprops.resize(prop_count);
        for(auto& cmp : cmprops){cmp.sType=VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;}
        _vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phy_dev, &prop_count, cmprops.data());

        auto scope_to_str = [](VkScopeKHR scope)
        {
            switch(scope)
            {
            case VK_SCOPE_DEVICE_KHR: return "device";
            case VK_SCOPE_WORKGROUP_KHR: return "wrkgrp";
            case VK_SCOPE_SUBGROUP_KHR: return "subgrp";
            case VK_SCOPE_QUEUE_FAMILY_KHR: return "qfam";
            default: return "unknwn";
            }
        };

        fmt::print("Supported Cooperative Matrices:\n");
        fmt::print("        M  x  N x  K,   A,   B,   C,   D,  scope, sat\n");
        for(auto& cmprop : cmprops)
        {
            fmt::print("        {:2d} x {:2d} x {:2d}, {:3}, {:3}, {:3}, {:3}, {:6}, {:3}\n",
                    cmprop.MSize, cmprop.NSize, cmprop.KSize,
                    component_type_to_str(cmprop.AType),
                    component_type_to_str(cmprop.BType),
                    component_type_to_str(cmprop.CType),
                    component_type_to_str(cmprop.ResultType),
                    scope_to_str(cmprop.scope),
		    cmprop.saturatingAccumulation);

            if (VK_SCOPE_SUBGROUP_KHR != cmprop.scope)
            {
                fmt::print("Only subgroups supported right now, skipping\n");
                continue;
            }
	        // s8 + u8 exposed by AMD and fails, not sure what's wrong
            if (cmprop.AType != cmprop.BType)
            {
                fmt::print("known buggy type, skipping\n");
                continue;
            }
	        // I don't know what the AMDGPU pro vulkan driver exposes here, but
            // there is no VkComponentTypeKHR with integer value 1000142000,
            // Max value according to current spec is 10
            constexpr std::uint32_t max_ct_value = 10;
            auto type_check = [max_ct_value](auto c, auto t)
            {
                if (t > max_ct_value)
                {
                    fmt::print("unsupported {} type (value={}), skipping\n", c, static_cast<std::uint32_t>(t));
                    return false;
                }
                return true;
            };
            if(!type_check("a", cmprop.AType)) continue;
            if(!type_check("b", cmprop.BType)) continue;
            if(!type_check("c", cmprop.CType)) continue;
            if(!type_check("d", cmprop.ResultType)) continue;

            // Somewhat confused about this.
            // On Windows the AMD driver has emulated cooperative matrices on RDNA2,
            // The driver reports minSubgroupSize=32 and maxSubgroupSize=subgroupSize=64
            // setting it to 32 makes validation layers complain about it being less than
            // subgroupSize, but I think that's a false positive, since minSubgroupSize=32?
            // Using subroupSize for now, as the validation layer just repeats what the spec says
            std::uint32_t subgroup_size = pdv11p.subgroupSize;
            //std::uint32_t subgroup_size = 32;
            //std::uint32_t subgroup_size = pdsgscp.minSubgroupSize;

            auto benchmark = create_coop_benchmark(
                    phy_dev, device, cmprop,
                    insts_in_block, inner_iterations, num_repetitions, num_groups);

            dpkey shader_key = std::make_pair(device, cmprop);

            if(auto findit = shaders.find(shader_key); findit != shaders.end())
            {
                // Idea is to only compile GLSL->SPIR-V once
                // This also means that this shader only exists in the lifetime of the benchmark object
                // (actually this might be good?)
                benchmark->set_shader(std::make_shared<coopmat_benchmark_shader>(*findit->second.get()));
            }
            else
            {
                // TODO: Shader compilation should probably happend independently from 
                //       instantiation in a real-world scenario
                //fmt::print("compiling GLSL->SPIR-V for {:3}, {:3}, {:3}\n", 
                //    component_type_to_str(cmprop.AType),
                //    component_type_to_str(cmprop.BType),
                //    component_type_to_str(cmprop.CType));
                shaders[shader_key] = std::make_shared<coopmat_benchmark_shader>(
                        device, code_str,
                        cmprop.AType, cmprop.BType, cmprop.CType,
                        subgroup_size,
                        insts_in_block,
			blocks_in_kernel);
                if(auto findit = device_shader_configs.find(device); findit == device_shader_configs.end())
                {
                    device_shader_configs[device] = coopmat_benchmark_shader::create_configuration(device);
                }
                benchmark->set_shader(shaders[shader_key]);
            }

            benchmarks.push_back(std::move(benchmark));
        }

        devices.push_back(device);
    }


    // TODO: create a device->benchmark hierarchy
    // This was fine when compiling shaders only, but probably bad for actually running/benchmarking
    //#pragma omp parallel for
    for(std::size_t i = 0; i < benchmarks.size(); i++)
    {
        fmt::print("\n");
        fmt::print("=========================================================");
        fmt::print("\n");
        auto cmprop = benchmarks[i]->get_cmprops();
        fmt::print("Creating buffers for: {:2d} x {:2d} x {:2d}, {:3}, {:3}, {:3}, {:3}\n",
                cmprop.MSize, cmprop.NSize, cmprop.KSize,
                component_type_to_str(cmprop.AType),
                component_type_to_str(cmprop.BType),
                component_type_to_str(cmprop.CType),
                component_type_to_str(cmprop.ResultType));
        benchmarks[i]->create_buffers();
        fmt::print("Running benchmark for: {:2d} x {:2d} x {:2d}, {:3}, {:3}, {:3}, {:3}\n",
                cmprop.MSize, cmprop.NSize, cmprop.KSize,
                component_type_to_str(cmprop.AType),
                component_type_to_str(cmprop.BType),
                component_type_to_str(cmprop.CType),
                component_type_to_str(cmprop.ResultType));
        auto device = benchmarks[i]->get_device();
        auto config = device_shader_configs[device];


        const auto& dqcis = device_dqcis[device];

        // I'm not sure what my idea was with saving queues from multiple families,
        // so just take the first one
        const auto& dqci = dqcis[0];

        //TODO: move queue/command buffer allocations out of the loop and make them per-device
        VkQueue queue;

        vkGetDeviceQueue(device, dqci.queueFamilyIndex, 0, &queue);

        //TODO: neither should this
        VkCommandPool command_pool;
        VkCommandPoolCreateInfo cpci
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            // TODO: Actually with transient buffers, we can leave the allocation in the loop (probably? benchmark idea?)
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = dqci.queueFamilyIndex,
        };
        vkCreateCommandPool(device, &cpci, nullptr, &command_pool);

        VkCommandBuffer command_buffer;
        VkCommandBufferAllocateInfo cbai
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = command_pool,
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        vkAllocateCommandBuffers(device, &cbai, &command_buffer);

        benchmarks[i]->run(config, queue, command_buffer, blocks_in_kernel);
        fmt::print("Cleaning up for: {:2d} x {:2d} x {:2d}, {:3}, {:3}, {:3}, {:3}\n",
                cmprop.MSize, cmprop.NSize, cmprop.KSize,
                component_type_to_str(cmprop.AType),
                component_type_to_str(cmprop.BType),
                component_type_to_str(cmprop.CType),
                component_type_to_str(cmprop.ResultType));
        benchmarks[i]->cleanup();
        benchmarks[i]->destroy_buffers();

        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        vkDestroyCommandPool(device, command_pool, nullptr);
        fmt::print("=========================================================");
        fmt::print("\n");
    }

    // TODO: When adapting this to something more proper,
    //       deal with the lifetime of the 'VkShaderModule's more gracefully
    for (auto& [_,shader] : shaders)
    {
        shader->destroy_shared_module();
    }
    benchmarks.clear();

    for (auto& [device,config] : device_shader_configs)
    {
        coopmat_benchmark_shader::release_configuration(device, config);
    }

    for(std::size_t i = 0; i < devices.size(); i++)
    {
        auto device = devices[i];
        vkDestroyDevice(device, nullptr);
    }


    fmt::print("Destroying vulkan instance {:#08X}\n", ITOP(instance));
    vkDestroyInstance(instance, nullptr);

    fmt::print("Vulkan instance destroyed\n");

    return 0;
}
