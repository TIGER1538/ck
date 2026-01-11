#pragma once

#include "ck/core/platform.h"
#include "ck/pathtype.h"
#include "ck/audio/sampleinfo.h"

namespace Cki
{

// Utility functions for decoding CKS files to PCM data

namespace CksDecoder
{
    // Decode a specified number of frames from a CKS file to PCM data.
    // Returns the actual number of frames decoded (may be less than requested if end of file is reached).
    // 
    // Parameters:
    //   path: Path to the CKS file
    //   pathType: Path type (default, bundle, document, etc.)
    //   offset: Offset in bytes from the start of the file (for embedded files)
    //   length: Length in bytes of the embedded file (or 0 for entire file)
    //   startFrame: Starting frame position in the file
    //   frameCount: Number of frames to decode
    //   outBuffer: Output buffer for PCM data (must be pre-allocated)
    //   outSampleInfo: Optional output parameter to receive sample info
    //
    // The output buffer should be large enough to hold frameCount * channels samples.
    // PCM data is returned as int16 samples, interleaved for stereo.
    int decodeFrames(
        const char* path, 
        CkPathType pathType,
        int offset,
        int length,
        int startFrame,
        int frameCount,
        int16* outBuffer,
        SampleInfo* outSampleInfo = NULL
    );

    // Decode the entire CKS file to PCM data.
    // Returns a newly allocated buffer containing all PCM data, or NULL on failure.
    // The caller is responsible for freeing the returned buffer using delete[].
    //
    // Parameters:
    //   path: Path to the CKS file
    //   pathType: Path type (default, bundle, document, etc.)
    //   offset: Offset in bytes from the start of the file (for embedded files)
    //   length: Length in bytes of the embedded file (or 0 for entire file)
    //   outFrameCount: Output parameter to receive the total number of frames decoded
    //   outSampleInfo: Optional output parameter to receive sample info
    //
    // The returned buffer contains int16 samples, interleaved for stereo.
    int16* decodeFile(
        const char* path,
        CkPathType pathType,
        int offset,
        int length,
        int* outFrameCount,
        SampleInfo* outSampleInfo = NULL
    );
}


}
