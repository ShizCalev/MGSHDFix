#pragma once

/// Checks if the detected GPU meets the minimum requirements.
/// Logs details about vendor, estimated performance, and warnings if below minimum.
///
/// Minimum GPU can be overridden by defining MINIMUM_GPU_NAME before including this header.
/// Default: NVIDIA GeForce GTX 970
void CheckMinimumGPU(const std::string& gpuName, bool logDriver, UINT product, UINT version, UINT subVersion, UINT build);

namespace GPU_Checker
{
    inline bool bForcePerformanceMode = false; //note: if the user has multiple gpus, this will only be set once we hit the first Present() call. Don't rely on it for Initalize() checks.
}

