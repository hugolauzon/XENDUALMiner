#include "processingunit.h"

#include "cudaexception.h"

#include <limits>
#include <iostream>
#ifndef NDEBUG
#include <iostream>
#endif

namespace argon2 {
namespace cuda {

static void setCudaDevice(int deviceIndex)
{
    int currentIndex = -1;
    CudaException::check(cudaGetDevice(&currentIndex));
    if (currentIndex != deviceIndex) {
        CudaException::check(cudaSetDevice(deviceIndex));
    }
}

static bool isPowerOfTwo(std::size_t x)
{
    return (x & (x - 1)) == 0;
}

ProcessingUnit::ProcessingUnit(
        const ProgramContext *programContext, const Argon2Params *params,
        const Device *device, std::size_t batchSize, bool bySegment,
        bool precomputeRefs)
    : programContext(programContext), params(params), device(device),
      krunner(programContext->getArgon2Type(),
             programContext->getArgon2Version(), params->getTimeCost(),
             params->getLanes(), params->getSegmentBlocks(), batchSize,
             bySegment, precomputeRefs),
      bestLanesPerBlock(krunner.getMinLanesPerBlock()),
      bestJobsPerBlock(krunner.getMinJobsPerBlock())
{
    setCudaDevice(device->getDeviceIndex());

    /* pre-fill first blocks with pseudo-random data: */
    for (std::size_t i = 0; i < batchSize; i++) {
        setPassword(i, NULL, 0);
    }

}

void ProcessingUnit::setPassword(std::size_t index, const void *pw,
                                 std::size_t pwSize)
{
    params->fillFirstBlocks(krunner.getInputMemory(index), pw, pwSize,
                            programContext->getArgon2Type(),
                            programContext->getArgon2Version());
    // Expand the storage if needed
    if (passwordStorage.size() <= index) {
        passwordStorage.resize(index + 1);
    }
    
    // Store the password at the specified index
    passwordStorage[index] = std::string(static_cast<const char*>(pw), pwSize);
}

void ProcessingUnit::getHash(std::size_t index, void *hash)
{
    params->finalize(hash, kunner.getOutputMemory(index));
}
std::string ProcessingUnit::getPW(std::size_t index){
    if (index < passwordStorage.size()) {
        return passwordStorage[index];
    }
    return {};  // Return an empty string if the index is out of bounds

}
void ProcessingUnit::beginProcessing()
{
    setCudaDevice(device->getDeviceIndex());
    krunner.run(bestLanesPerBlock, bestJobsPerBlock);
}

void ProcessingUnit::endProcessing()
{
    krunner.finish();
}

} // namespace cuda
} // namespace argon2
