#pragma once
#include <A3d_Channels.h>

namespace Necromancy {
namespace Typedefs {

//    void __fastcall HkTrueCallChannel(A3d_Channel* self, DWORD edx);

using TrueCallChannelFn = void(__thiscall*)(A3d_Channel* self);
using Aco_FloatChannel_GetFloat = float(__thiscall*)(void* self);
using Aco_FloatChannel_GetDefaultFloat = float(__thiscall*)(void* self);
using Aco_FloatChannel_SetFloat = void(__thiscall*)(void* self, float value);

using Aco_ArrayTable_GetTable = void*(__thiscall*)(void* self);
using A3d_Channel_TrueCallChannel = void(__thiscall*)(A3d_Channel* self);

}
}
