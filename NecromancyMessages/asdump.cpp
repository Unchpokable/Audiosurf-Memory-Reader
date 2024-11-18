#include "pch.h"
#include "asdump.h"
#include "messageids.h"

using namespace Necromancy::Messages;

StatusCode ASDump::Serialize(const ASDumpStruct& dump, byte** buffer) {
    if(!buffer) return StatusCode::ErrorCausedWrongData;

    auto localBuffer = new byte[ASDumpMessageSize];
    std::memset(localBuffer, 0, ASDumpMessageSize);

    SerializeDirect(dump, localBuffer, ASDumpMessageSize);

    *buffer = localBuffer;
    return StatusCode::Ok;
}

StatusCode ASDump::SerializeDirect(const ASDumpStruct& dump, byte* buffer, size_t bufferSize) {
    if(bufferSize < ASDumpMessageSize) {
        return StatusCode::ErrorCausedWrongData;
    }

    if(!buffer) {
        return StatusCode::ErrorCausedWrongData;
    }

    auto structPtr = reinterpret_cast<const byte*>(&dump);
    auto serializationPtr = buffer;

    auto messageId(MessageIds::FullDump);

    std::memcpy(serializationPtr, &messageId, sizeof(uint16_t));
    serializationPtr += sizeof(uint16_t);

    std::memcpy(serializationPtr, structPtr + ASDump_ScoreFieldOffset, ASDump_ScoreFieldOffset);
    serializationPtr += ASDump_ScoreFieldSize;

    std::memcpy(serializationPtr, structPtr + ASDump_StatsArraySizeFieldOffset, ASDump_StatsArraySizeFieldSize);
    serializationPtr += ASDump_StatsArraySizeFieldSize;

    std::memcpy(serializationPtr, dump.statsArray, sizeof(float) * dump.statsArraySize);
    serializationPtr += ASDump_StatsArrayFieldSize * dump.statsArraySize;

    std::memcpy(serializationPtr, structPtr + ASDump_GoldThresholdFieldOffset, ASDump_GoldThresholdFieldSize);
    serializationPtr += ASDump_GoldThresholdFieldSize;

    std::memcpy(serializationPtr, structPtr + ASDump_TrafficChainMaxFieldOffset, ASDump_TrafficChainMaxFieldSize);

    return StatusCode::Ok;
}

StatusCode ASDump::Deserialize(const byte* buffer, ASDumpStruct* result) {
    ASDumpStruct data;

    uint16_t messageId;

    std::memcpy(&messageId, buffer, sizeof(uint16_t));

    if(messageId != static_cast<uint16_t>(MessageIds::FullDump)) {
        return StatusCode::ErrorCausedWrongData;
    }

    auto structPtr = reinterpret_cast<byte*>(&data);
    auto serializationPtr = buffer;
    serializationPtr += sizeof(uint16_t);

    std::memcpy(structPtr + ASDump_ScoreFieldOffset, serializationPtr, ASDump_ScoreFieldSize);
    serializationPtr += ASDump_ScoreFieldSize;

    std::memcpy(structPtr + ASDump_StatsArraySizeFieldOffset, serializationPtr, ASDump_StatsArraySizeFieldSize);
    serializationPtr += ASDump_StatsArraySizeFieldSize;

    // copy array
    float* arrayBuffer = new float[data.statsArraySize]; // should be filled already
    std::memcpy(arrayBuffer, serializationPtr, sizeof(float) * data.statsArraySize);
    serializationPtr += ASDump_StatsArrayFieldSize * data.statsArraySize;

    std::memcpy(structPtr + ASDump_GoldThresholdFieldOffset, serializationPtr, ASDump_GoldThresholdFieldSize);
    serializationPtr += ASDump_GoldThresholdFieldSize;

    std::memcpy(structPtr + ASDump_TrafficChainMaxFieldOffset, serializationPtr, ASDump_TrafficChainMaxFieldSize);

    *result = data;
    return StatusCode::Ok;
}

void ASDump::Initialize(ASDumpStruct* dumpStruct, int statsArraySize) {
    dumpStruct->score = 0.0;
    dumpStruct->statsArraySize = statsArraySize;
    dumpStruct->statsArray = new float[statsArraySize];
    dumpStruct->goldThreshold = 0.0;
    dumpStruct->trafficChainMax = 0.0;
}

void ASDump::Free(ASDumpStruct* dumpStruct) {
    delete[] dumpStruct->statsArray;
    dumpStruct->statsArray = nullptr;
}