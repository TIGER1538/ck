#include "ck/audio/cksdecoder.h"
#include "ck/audio/cksaudiostream.h"
#include "ck/audio/audioformat.h"
#include "ck/audio/buffersource.h"
#include "ck/audio/adpcmdecoder.h"
#include "ck/audio/pcmi16decoder.h"
#include "ck/audio/pcmi8decoder.h"
#include "ck/audio/pcmf32decoder.h"
#include "ck/core/bufferstream.h"
#include "ck/core/debug.h"
#include "ck/core/logger.h"
#include "ck/core/math.h"
#include <new>

namespace Cki
{

namespace CksDecoder
{

// Helper class to wrap CksAudioStream as an AudioSource
class CksAudioSource : public AudioSource
{
public:
    CksAudioSource(CksAudioStream& stream) : 
        m_stream(stream),
        m_inited(false)
    {}

    virtual int read(void* buf, int blocks)
    {
        return m_stream.read(buf, blocks);
    }

    virtual int getNumBlocks() const
    {
        return m_stream.getNumBlocks();
    }

    virtual void setBlockPos(int block)
    {
        m_stream.setBlockPos(block);
    }

    virtual int getBlockPos() const
    {
        return m_stream.getBlockPos();
    }

    virtual void reset()
    {
        m_stream.setBlockPos(0);
    }

    virtual const SampleInfo& getSampleInfo() const
    {
        return m_stream.getSampleInfo();
    }

    virtual bool isInited() const
    {
        return m_inited;
    }

    virtual bool isReady() const
    {
        return m_inited && !m_stream.isFailed();
    }

    virtual bool isFailed() const
    {
        return m_stream.isFailed();
    }

    virtual bool isDone() const
    {
        return m_stream.getBlockPos() >= m_stream.getNumBlocks();
    }

    virtual void setLoop(int startFrame, int endFrame) {}
    virtual void getLoop(int& startFrame, int& endFrame) const { startFrame = 0; endFrame = -1; }
    virtual void setLoopCount(int) {}
    virtual int getLoopCount() const { return 0; }
    virtual int getCurrentLoop() const { return 0; }
    virtual void releaseLoop() {}
    virtual bool isLoopReleased() const { return false; }

    void setInited(bool inited) { m_inited = inited; }

private:
    CksAudioStream& m_stream;
    bool m_inited;
};

// Helper function to create appropriate decoder based on format
static Decoder* createDecoder(AudioSource& source, byte* decoderMem)
{
    switch (source.getSampleInfo().format)
    {
        case AudioFormat::k_pcmI16:
            return new (decoderMem) PcmI16Decoder(source);
        case AudioFormat::k_pcmI8:
            return new (decoderMem) PcmI8Decoder(source);
        case AudioFormat::k_pcmF32:
            return new (decoderMem) PcmF32Decoder(source);
        case AudioFormat::k_adpcm:
            return new (decoderMem) AdpcmDecoder(source);
        default:
            CK_LOG_ERROR("Unknown audio format: %d", source.getSampleInfo().format);
            return NULL;
    }
}

int decodeFrames(
    const char* path, 
    CkPathType pathType,
    int offset,
    int length,
    int startFrame,
    int frameCount,
    int16* outBuffer,
    SampleInfo* outSampleInfo)
{
    // Create audio stream
    CksAudioStream stream(path, pathType, offset, length);
    stream.init();
    
    if (stream.isFailed())
    {
        CK_LOG_ERROR("Failed to initialize CKS audio stream from: %s", path);
        return 0;
    }

    // Create audio source wrapper
    CksAudioSource source(stream);
    source.setInited(true);

    if (outSampleInfo)
    {
        *outSampleInfo = stream.getSampleInfo();
    }

    // Seek to start position
    const SampleInfo& sampleInfo = stream.getSampleInfo();
    int startBlock = startFrame / sampleInfo.blockFrames;
    stream.setBlockPos(startBlock);

    // Create appropriate decoder
    byte decoderMem[sizeof(AdpcmDecoder)]; // Large enough for any decoder
    Decoder* decoder = createDecoder(source, decoderMem);
    if (!decoder)
    {
        return 0;
    }

    // Set decoder position to exact frame
    decoder->setFramePos(startFrame);

    // Decode frames
    int framesDecoded = decoder->decode(outBuffer, frameCount);

    // Cleanup
    decoder->~Decoder();

    return framesDecoded;
}

int16* decodeFile(
    const char* path,
    CkPathType pathType,
    int offset,
    int length,
    int* outFrameCount,
    SampleInfo* outSampleInfo)
{
    if (!outFrameCount)
    {
        CK_LOG_ERROR("outFrameCount parameter is required");
        return NULL;
    }

    // Create audio stream
    CksAudioStream stream(path, pathType, offset, length);
    stream.init();
    
    if (stream.isFailed())
    {
        CK_LOG_ERROR("Failed to initialize CKS audio stream from: %s", path);
        *outFrameCount = 0;
        return NULL;
    }

    // Create audio source wrapper
    CksAudioSource source(stream);
    source.setInited(true);

    const SampleInfo& sampleInfo = stream.getSampleInfo();
    if (outSampleInfo)
    {
        *outSampleInfo = sampleInfo;
    }

    // Get total frame count
    int totalFrames = stream.getNumBlocks() * sampleInfo.blockFrames;
    if (totalFrames <= 0)
    {
        CK_LOG_ERROR("Invalid frame count: %d", totalFrames);
        *outFrameCount = 0;
        return NULL;
    }

    // Allocate output buffer
    int totalSamples = totalFrames * sampleInfo.channels;
    int16* outBuffer = new int16[totalSamples];
    if (!outBuffer)
    {
        CK_LOG_ERROR("Failed to allocate output buffer for %d samples", totalSamples);
        *outFrameCount = 0;
        return NULL;
    }

    // Create appropriate decoder
    byte decoderMem[sizeof(AdpcmDecoder)]; // Large enough for any decoder
    Decoder* decoder = createDecoder(source, decoderMem);
    if (!decoder)
    {
        delete[] outBuffer;
        *outFrameCount = 0;
        return NULL;
    }

    // Decode all frames
    decoder->setFramePos(0);
    int framesDecoded = decoder->decode(outBuffer, totalFrames);

    // Cleanup
    decoder->~Decoder();

    *outFrameCount = framesDecoded;
    return outBuffer;
}

} // namespace CksDecoder

}
