#include "stdafx.h"

#include "SoundRender_CoreA.h"
#include "SoundRender_CoreF.h"

XRSOUND_API xr_token* snd_devices_token = nullptr;
XRSOUND_API u32 snd_device_id = u32(-1);

void ISoundManager::_create()
{
    if (strstr(Core.Params, "-fmod") != nullptr)
    {
        SoundRenderF = xr_new<CSoundRender_CoreF>();
        SoundRender = SoundRenderF;
        GEnv.Sound = SoundRender;
    }
    else
    {
        SoundRenderA = xr_new<CSoundRender_CoreA>();
        SoundRender = SoundRenderA;
        GEnv.Sound = SoundRender;
        ((CSoundRender_CoreA*)SoundRender)->bPresent = strstr(Core.Params, "-nosound") == nullptr;
        if (!((CSoundRender_CoreA*)SoundRender)->bPresent)
            return;
    }

    GEnv.Sound->_initialize();
}

void ISoundManager::_destroy()
{
    GEnv.Sound->_clear();
    xr_delete(SoundRender);
    GEnv.Sound = nullptr;
}
