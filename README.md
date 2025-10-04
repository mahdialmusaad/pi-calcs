## Overview
Portable source files for different methods of calculating pi.<br>
Leverages both CPU and GPU multithreading to get results at speed.
## Usage
Each source file takes in a specific number and format of arguments, which will be shown when executing one erroneously.

Example outputs (results may vary):
<br>
#### [pi_mcarlo.c](pi_mcarlo.c):
```bash
$ ./pi_mcarlo
Usage: ./pi_mcarlo points_per_thread num_threads
$ ./pi_mcarlo 1000000000 12
Points results:
  785397381 inside
  214602619 outside
Pi approximation: 3.141590
Time taken: 1.922358s
```

#### [pi_mcarlo_cuda.cu](pi_mcarlo_cuda.cu):
```bash
$ ./pi_mcarlo_cuda
Usage: ./pi_mcarlo_cuda thread_blocks threads_per_block
$ ./pi_mcarlo_cuda 200000 1024
Using 204800000 threads
Points results:
  160849597 inside
  43950403 outside
Pi approximation: 3.141594
Time taken: 0.836914s
```

## Build
All sources can be built using the provided [CMakeLists.txt](CMakeLists.txt) file using [CMake](https://cmake.org/).<br>
CUDA is also required to build .cu files; see steps to download the toolkit [here](https://developer.nvidia.com/cuda-downloads).<br>
Results are placed into the `execs/` directory.
