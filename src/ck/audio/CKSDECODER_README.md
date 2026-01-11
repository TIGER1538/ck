# CKS Decoder Utility Functions

This module provides utility functions to decode Cricket Audio Stream (.cks) files directly to PCM data without requiring the full Cricket Audio system to be initialized.

## Functions

### `CksDecoder::decodeFrames()`

Decodes a specified number of frames from a CKS file starting at a given position.

```cpp
int decodeFrames(
    const char* path,           // Path to the CKS file
    CkPathType pathType,        // Path type (e.g., kCkPathType_Default)
    int offset,                 // Offset in bytes for embedded files (0 for standalone)
    int length,                 // Length in bytes for embedded files (0 for standalone)
    int startFrame,             // Starting frame position
    int frameCount,             // Number of frames to decode
    int16* outBuffer,           // Pre-allocated output buffer
    SampleInfo* outSampleInfo   // Optional: receives sample information
);
```

**Returns:** The actual number of frames decoded (may be less than requested if EOF is reached).

**Example:**
```cpp
#include "ck/audio/cksdecoder.h"
#include "ck/audio/sampleinfo.h"

SampleInfo info;
int16* buffer = new int16[1000 * 2];  // 1000 frames, stereo
int decoded = CksDecoder::decodeFrames(
    "music.cks", 
    kCkPathType_Default,
    0, 0,      // no offset/length
    0,         // start at frame 0
    1000,      // decode 1000 frames
    buffer,
    &info
);

// Use the decoded PCM data...
delete[] buffer;
```

### `CksDecoder::decodeFile()`

Decodes the entire CKS file to PCM data.

```cpp
int16* decodeFile(
    const char* path,           // Path to the CKS file
    CkPathType pathType,        // Path type (e.g., kCkPathType_Default)
    int offset,                 // Offset in bytes for embedded files (0 for standalone)
    int length,                 // Length in bytes for embedded files (0 for standalone)
    int* outFrameCount,         // Receives the total number of frames decoded
    SampleInfo* outSampleInfo   // Optional: receives sample information
);
```

**Returns:** A newly allocated buffer containing all PCM data, or NULL on failure. The caller is responsible for freeing the buffer using `delete[]`.

**Example:**
```cpp
#include "ck/audio/cksdecoder.h"
#include "ck/audio/sampleinfo.h"

int frameCount;
SampleInfo info;
int16* pcmData = CksDecoder::decodeFile(
    "music.cks",
    kCkPathType_Default,
    0, 0,          // no offset/length
    &frameCount,
    &info
);

if (pcmData) {
    // Total samples = frameCount * info.channels
    // Sample rate = info.sampleRate
    // Duration = frameCount / info.sampleRate seconds
    
    // Process the PCM data...
    
    delete[] pcmData;
}
```

## Supported Formats

Both functions support all CKS audio formats:
- PCM 8-bit (signed)
- PCM 16-bit (signed)
- PCM 32-bit float
- ADPCM (IMA ADPCM)

## Output Format

The output is always 16-bit signed integer PCM data (`int16`), interleaved for stereo.

## Sample Information

The `SampleInfo` structure contains:
- `format`: Audio format code
- `channels`: Number of channels (1 = mono, 2 = stereo)
- `sampleRate`: Sample rate in Hz
- `blocks`: Total number of blocks in the file
- `blockBytes`: Bytes per block
- `blockFrames`: Frames per block
- Additional metadata (volume, pan, loop points, etc.)

## Notes

- These functions are independent of the main Cricket Audio system and do not require calling `CkInit()`.
- For embedded files, specify both `offset` and `length`. Otherwise, use 0 for both.
- The `outFrameCount` parameter in `decodeFile()` is required and cannot be NULL.
- Memory allocated by `decodeFile()` must be freed by the caller using `delete[]`.
- For `decodeFrames()`, the output buffer must be pre-allocated with sufficient space: `frameCount * channels * sizeof(int16)` bytes.
