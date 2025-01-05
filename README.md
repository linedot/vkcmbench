# Vulkan cooperative matrix benchmark

WIP Benchmark measuring peak throughput of cooperative matrix instructions on your GPU. See also [https://github.com/jeffbolznv/vk_cooperative_matrix_perf](https://github.com/jeffbolznv/vk_cooperative_matrix_perf) and [https://github.com/nihui/vkpeak](https://github.com/nihui/vkpeak)

## Example NVIDIA Supercomputer GPU: GH200
(NOTE: This falls short of what the GPU can actually do (reaches about 2/3 of peak). I think the Vulkan driver/compiler doesn't use/expose Hoppers [WGMMA instructions](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#asynchronous-warpgroup-level-matrix-multiply-accumulate-instructions). It makes sense as while in CUDA those are exposed, NVIDIA recommends using their libraries instead of using them directly, as they are quite difficult to use, relying on asynchronous memory transfers and complicated tiling (CUTLASS/CUTE takes care of those) and there aren't equivalent libraries for Vulkan. Maybe this functionality will become available with [VK_NV_cooperative_matrix2](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_NV_cooperative_matrix2.html) ? )

(NOTE 2: Also f64 support isn't exposed to Vulkan and tf32/bf16 aren't in the Vulkan spec)

```
Number of Layers: 1
Vulkan instance created successfully: 0XDB3C030
Physical device 0:
  Name:        NVIDIA GH200 480GB
  Device type: Discrete GPU
  Driver Name: NVIDIA
  Driver Ver.: 565.57.1.0
Device created, handle: dbd3680
Physical device 0:
def Subgroup size: 32
min Subgroup size: 32
max Subgroup size: 32
    Supports Cooperative Matrices in the following shader stages:
        compute
Supported Cooperative Matrices:
        M  x  N x  K,   A,   B,   C,   D,  scope, sat
        16 x 16 x 16, f16, f16, f16, f16, subgrp,   0
Shader module de3b560 created for device dbd3680
        16 x  8 x 16, f16, f16, f16, f16, subgrp,   0
        16 x  8 x  8, f16, f16, f16, f16, subgrp,   0
        16 x 16 x 16, f16, f16, f32, f32, subgrp,   0
Shader module db63450 created for device dbd3680
        16 x  8 x 16, f16, f16, f32, f32, subgrp,   0
        16 x  8 x  8, f16, f16, f32, f32, subgrp,   0
        16 x 16 x 32, u8 , u8 , u32, u32, subgrp,   0
Shader module dea7040 created for device dbd3680
        16 x 16 x 32, s8 , s8 , s32, s32, subgrp,   0
Shader module d95ee30 created for device dbd3680
        16 x  8 x 32, u8 , u8 , u32, u32, subgrp,   0
        16 x  8 x 32, s8 , s8 , s32, s32, subgrp,   0

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f16, f16
Running benchmark for: 16 x 16 x 16, f16, f16, f16, f16
Took Min. 113920 ns
Took Avg. 114662 ns
Max. 622076.55 GFLOP/s
Avg. 618050.97 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 16, f16, f16, f16, f16
Running benchmark for: 16 x  8 x 16, f16, f16, f16, f16
Took Min. 58720 ns
Took Avg. 59283 ns
Max. 603431.20 GFLOP/s
Avg. 597700.52 GFLOP/s
Cleaning up for: 16 x  8 x 16, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x  8 x  8, f16, f16, f16, f16
Running benchmark for: 16 x  8 x  8, f16, f16, f16, f16
Took Min. 39936 ns
Took Avg. 40566 ns
Max. 443628.31 GFLOP/s
Avg. 436738.65 GFLOP/s
Cleaning up for: 16 x  8 x  8, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f32, f32
Running benchmark for: 16 x 16 x 16, f16, f16, f32, f32
Took Min. 115776 ns
Took Avg. 116508 ns
Max. 612104.07 GFLOP/s
Avg. 608258.32 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 16, f16, f16, f32, f32
Running benchmark for: 16 x  8 x 16, f16, f16, f32, f32
Took Min. 58560 ns
Took Avg. 59248 ns
Max. 605079.92 GFLOP/s
Avg. 598053.61 GFLOP/s
Cleaning up for: 16 x  8 x 16, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x  8, f16, f16, f32, f32
Running benchmark for: 16 x  8 x  8, f16, f16, f32, f32
Took Min. 40384 ns
Took Avg. 40969 ns
Max. 438706.92 GFLOP/s
Avg. 432442.58 GFLOP/s
Cleaning up for: 16 x  8 x  8, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 32, u8 , u8 , u32, u32
Running benchmark for: 16 x 16 x 32, u8 , u8 , u32, u32
Took Min. 114208 ns
Took Avg. 114851 ns
Max. 1241015.70 GIOP/s
Avg. 1234067.80 GIOP/s
Cleaning up for: 16 x 16 x 32, u8 , u8 , u32, u32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 32, s8 , s8 , s32, s32
Running benchmark for: 16 x 16 x 32, s8 , s8 , s32, s32
Took Min. 114016 ns
Took Avg. 114752 ns
Max. 1243105.54 GIOP/s
Avg. 1235132.47 GIOP/s
Cleaning up for: 16 x 16 x 32, s8 , s8 , s32, s32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 32, u8 , u8 , u32, u32
Running benchmark for: 16 x  8 x 32, u8 , u8 , u32, u32
Took Min. 58304 ns
Took Avg. 58992 ns
Max. 1215473.39 GIOP/s
Avg. 1201297.81 GIOP/s
Cleaning up for: 16 x  8 x 32, u8 , u8 , u32, u32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 32, s8 , s8 , s32, s32
Running benchmark for: 16 x  8 x 32, s8 , s8 , s32, s32
Took Min. 58272 ns
Took Avg. 58953 ns
Max. 1216140.86 GIOP/s
Avg. 1202092.52 GIOP/s
Cleaning up for: 16 x  8 x 32, s8 , s8 , s32, s32
=========================================================
Destroying vulkan instance 0XDB3C030
Vulkan instance destroyed
```

## Example NVIDIA Laptop GPU: RTX 4060

```
Number of Layers: 12
Vulkan instance created successfully: 0X55BCF66EC1F0
Physical device 0:
  Name:        NVIDIA GeForce RTX 4060 Laptop GPU
  Device type: Discrete GPU
  Driver Name: NVIDIA
  Driver Ver.: 565.77.0.0
Physical device 1:
  Name:        AMD Radeon 610M (RADV RAPHAEL_MENDOCINO)
  Device type: Integrated GPU
  Driver Name: radv
  Driver Ver.: 24.3.2
Device created, handle: 55bcf6767680
Physical device 0:
def Subgroup size: 32
min Subgroup size: 32
max Subgroup size: 32
    Supports Cooperative Matrices in the following shader stages:
        compute
Supported Cooperative Matrices:
        M  x  N x  K,   A,   B,   C,   D,  scope, sat
        16 x 16 x 16, f16, f16, f16, f16, subgrp,   0
Shader module 55bcf66e8070 created for device 55bcf6767680
        16 x  8 x 16, f16, f16, f16, f16, subgrp,   0
        16 x  8 x  8, f16, f16, f16, f16, subgrp,   0
        16 x 16 x 16, f16, f16, f32, f32, subgrp,   0
Shader module 55bcf672df10 created for device 55bcf6767680
        16 x  8 x 16, f16, f16, f32, f32, subgrp,   0
        16 x  8 x  8, f16, f16, f32, f32, subgrp,   0
        16 x 16 x 32, u8 , u8 , u32, u32, subgrp,   0
Shader module 55bcf66e7570 created for device 55bcf6767680
        16 x 16 x 32, s8 , s8 , s32, s32, subgrp,   0
Shader module 55bcf66e37d0 created for device 55bcf6767680
        16 x  8 x 32, u8 , u8 , u32, u32, subgrp,   0
        16 x  8 x 32, s8 , s8 , s32, s32, subgrp,   0

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f16, f16
Running benchmark for: 16 x 16 x 16, f16, f16, f16, f16
Took Min. 1136640 ns
Took Avg. 1137734 ns
Max. 62347.76 GFLOP/s
Avg. 62287.81 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 16, f16, f16, f16, f16
Running benchmark for: 16 x  8 x 16, f16, f16, f16, f16
Took Min. 570368 ns
Took Avg. 571811 ns
Max. 62123.89 GFLOP/s
Avg. 61967.12 GFLOP/s
Cleaning up for: 16 x  8 x 16, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x  8 x  8, f16, f16, f16, f16
Running benchmark for: 16 x  8 x  8, f16, f16, f16, f16
Took Min. 288768 ns
Took Avg. 289881 ns
Max. 61352.85 GFLOP/s
Avg. 61117.29 GFLOP/s
Cleaning up for: 16 x  8 x  8, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f32, f32
Running benchmark for: 16 x 16 x 16, f16, f16, f32, f32
Took Min. 2265088 ns
Took Avg. 2265865 ns
Max. 31286.63 GFLOP/s
Avg. 31275.90 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 16, f16, f16, f32, f32
Running benchmark for: 16 x  8 x 16, f16, f16, f32, f32
Took Min. 1134880 ns
Took Avg. 1216483 ns
Max. 31222.23 GFLOP/s
Avg. 29127.81 GFLOP/s
Cleaning up for: 16 x  8 x 16, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x  8, f16, f16, f32, f32
Running benchmark for: 16 x  8 x  8, f16, f16, f32, f32
Took Min. 571392 ns
Took Avg. 582371 ns
Max. 31006.28 GFLOP/s
Avg. 30421.74 GFLOP/s
Cleaning up for: 16 x  8 x  8, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 32, u8 , u8 , u32, u32
Running benchmark for: 16 x 16 x 32, u8 , u8 , u32, u32
Took Min. 1133856 ns
Took Avg. 1135241 ns
Max. 125001.69 GIOP/s
Avg. 124849.19 GIOP/s
Cleaning up for: 16 x 16 x 32, u8 , u8 , u32, u32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 32, s8 , s8 , s32, s32
Running benchmark for: 16 x 16 x 32, s8 , s8 , s32, s32
Took Min. 1133568 ns
Took Avg. 1134947 ns
Max. 125033.45 GIOP/s
Avg. 124881.53 GIOP/s
Cleaning up for: 16 x 16 x 32, s8 , s8 , s32, s32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 32, u8 , u8 , u32, u32
Running benchmark for: 16 x  8 x 32, u8 , u8 , u32, u32
Took Min. 567328 ns
Took Avg. 568787 ns
Max. 124913.56 GIOP/s
Avg. 124593.14 GIOP/s
Cleaning up for: 16 x  8 x 32, u8 , u8 , u32, u32
=========================================================

=========================================================
Creating buffers for: 16 x  8 x 32, s8 , s8 , s32, s32
Running benchmark for: 16 x  8 x 32, s8 , s8 , s32, s32
Took Min. 567296 ns
Took Avg. 568572 ns
Max. 124920.61 GIOP/s
Avg. 124640.26 GIOP/s
Cleaning up for: 16 x  8 x 32, s8 , s8 , s32, s32
=========================================================
Destroying vulkan instance 0X55BCF66EC1F0
Vulkan instance destroyed
```

## Example AMD Desktop GPU without hardware support (Instructions emulated by AMDGPU Pro driver)

(NOTE: Compiler probably overoptimizes, as these numbers are way too high)
```
Number of Layers: 15
Vulkan instance created successfully: 0X5B532CDEB4D0
Physical device 0:
  Name:        AMD Radeon RX 6800 XT
  Device type: Discrete GPU
  Driver Name: AMD proprietary driver
  Driver Ver.: 2.0.325
Device created, handle: 5b532d0b3b10
Physical device 0:
def Subgroup size: 64
min Subgroup size: 32
max Subgroup size: 64
    Supports Cooperative Matrices in the following shader stages:
        compute
Supported Cooperative Matrices:
        M  x  N x  K,   A,   B,   C,   D,  scope, sat
        16 x 16 x 16, f16, f16, f32, f32, subgrp,   0
Shader module 5b532d27f520 created for device 5b532d0b3b10
        16 x 16 x 16, f16, f16, f16, f16, subgrp,   0
Shader module 5b532d27e220 created for device 5b532d0b3b10
        16 x 16 x 16, u8 , u8 , s32, s32, subgrp,   0
Shader module 5b532d27c580 created for device 5b532d0b3b10
        16 x 16 x 16, s8 , s8 , s32, s32, subgrp,   0
Shader module 5b532d09cfb0 created for device 5b532d0b3b10
        16 x 16 x 16, u8 , s8 , s32, s32, subgrp,   0
known buggy type, skipping
        16 x 16 x 16, s8 , u8 , s32, s32, subgrp,   0
known buggy type, skipping
        16 x 16 x 16, bad_type, bad_type, s32, s32, subgrp,   0
unsupported a type (value=1000142000), skipping
        16 x 16 x 16, bad_type, bad_type, s32, s32, subgrp,   0
unsupported a type (value=1000142001), skipping
        16 x 16 x 16, u8 , u8 , s32, s32, subgrp,   1
        16 x 16 x 16, s8 , s8 , s32, s32, subgrp,   1
        16 x 16 x 16, u8 , s8 , s32, s32, subgrp,   1
known buggy type, skipping
        16 x 16 x 16, s8 , u8 , s32, s32, subgrp,   1
known buggy type, skipping

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f32, f32
Running benchmark for: 16 x 16 x 16, f16, f16, f32, f32
Took Min. 428240 ns
Took Avg. 517110 ns
Max. 165484.22 GFLOP/s
Avg. 137044.27 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f32, f32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, f16, f16, f16, f16
Running benchmark for: 16 x 16 x 16, f16, f16, f16, f16
Took Min. 752520 ns
Took Avg. 863720 ns
Max. 94172.86 GFLOP/s
Avg. 82048.53 GFLOP/s
Cleaning up for: 16 x 16 x 16, f16, f16, f16, f16
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, u8 , u8 , s32, s32
Running benchmark for: 16 x 16 x 16, u8 , u8 , s32, s32
Took Min. 362640 ns
Took Avg. 393990 ns
Max. 195419.59 GIOP/s
Avg. 179869.95 GIOP/s
Cleaning up for: 16 x 16 x 16, u8 , u8 , s32, s32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, s8 , s8 , s32, s32
Running benchmark for: 16 x 16 x 16, s8 , s8 , s32, s32
Took Min. 363640 ns
Took Avg. 422020 ns
Max. 194882.19 GIOP/s
Avg. 167923.23 GIOP/s
Cleaning up for: 16 x 16 x 16, s8 , s8 , s32, s32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, u8 , u8 , s32, s32
Running benchmark for: 16 x 16 x 16, u8 , u8 , s32, s32
Took Min. 361320 ns
Took Avg. 383450 ns
Max. 196133.51 GIOP/s
Avg. 184814.08 GIOP/s
Cleaning up for: 16 x 16 x 16, u8 , u8 , s32, s32
=========================================================

=========================================================
Creating buffers for: 16 x 16 x 16, s8 , s8 , s32, s32
Running benchmark for: 16 x 16 x 16, s8 , s8 , s32, s32
Took Min. 363600 ns
Took Avg. 452980 ns
Max. 194903.63 GIOP/s
Avg. 156446.11 GIOP/s
Cleaning up for: 16 x 16 x 16, s8 , s8 , s32, s32
=========================================================
Destroying vulkan instance 0X5B532CDEB4D0
Vulkan instance destroyed
```

## Example AMD Desktop GPU without hardware support (RADV driver):

```
Number of Layers: 15
Vulkan instance created successfully: 0X62354A6D9FB0
Physical device 0:
  Name:        AMD Radeon RX 6800 XT (RADV NAVI21)
  Device type: Discrete GPU
  Driver Name: radv
  Driver Ver.: 24.3.2
No device supports VK_KHR_cooperative_matrix!
Destroying vulkan instance 0X62354A6D9FB0
Vulkan instance destroyed
```

## Example AMD Desktop GPU:

Need RDNA3 GPU
