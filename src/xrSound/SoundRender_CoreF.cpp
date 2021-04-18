#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_CoreF.h"
#include "SoundRender_TargetF.h"

class CSoundRender_EmitterFmod : public CSound_emitter
{
    xrEventInstance* CurrentEvent;
    float starting_delay;
    CSound_params snd_params = {};

public:
    enum State
    {
        stStopped = 0,

        stStartingDelayed,
        stStartingLoopedDelayed,

        stStarting,
        stStartingLooped,

        stPlaying,
        stPlayingLooped,

        stSimulating,
        stSimulatingLooped,

        stFORCEDWORD = u32(-1)
    };

public:
    CSoundRender_EmitterFmod(xrEventInstance* ParentEvent) 
    { 
        CurrentEvent = ParentEvent;
    }

    bool isPlaying()
    { 
        FMOD_STUDIO_PLAYBACK_STATE currentState = {};
        VERIFY(CurrentEvent->getPlaybackState(&currentState) == FMOD_OK);
        return (currentState == FMOD_STUDIO_PLAYBACK_PLAYING);
    }

    bool is_2D() override
    { 
        bool is3D = false;
        FMOD::Studio::EventDescription* pEventDescription = nullptr;
        VERIFY(CurrentEvent->getDescription(&pEventDescription) == FMOD_OK);
        pEventDescription->is3D(&is3D);
        return !is3D;
    }
    void switch_to_2D() override {}
    void switch_to_3D() override {}
    void set_position(const Fvector& pos) override
    {
        FMOD_3D_ATTRIBUTES attributes = {{0}};
        VERIFY(CurrentEvent->get3DAttributes(&attributes) == FMOD_OK);
        attributes.position = XrayVectorToFmodVector(pos);
        VERIFY(CurrentEvent->set3DAttributes(&attributes) == FMOD_OK);
        snd_params.position = pos;
    }

    void set_frequency(float scale) override
    {
        VERIFY(CurrentEvent->setPitch(scale) == FMOD_OK);
        snd_params.freq = scale;
    }

    void set_range(float min, float max) override
    {
        VERIFY(_valid(min) && _valid(max));
        VERIFY(CurrentEvent->setProperty(FMOD_STUDIO_EVENT_PROPERTY_MAXIMUM_DISTANCE, max) == FMOD_OK);
        VERIFY(CurrentEvent->setProperty(FMOD_STUDIO_EVENT_PROPERTY_MINIMUM_DISTANCE, min) == FMOD_OK);
        snd_params.max_distance = max;
        snd_params.min_distance = min;
    }

    void set_volume(float vol) override
    {
        if (!_valid(vol))
            vol = 0.0f;
        
        VERIFY(CurrentEvent->setVolume(vol) == FMOD_OK);
        snd_params.volume;
    }

    void set_priority(float p) override {  }
    const CSound_params* get_params() override { return &snd_params; }
    void fill_block(void* ptr, u32 size)
    {

    }

    void fill_data(u8* ptr, u32 offset, u32 size) {}

    float priority() { return 0.f; }
    void start(ref_sound* _owner, bool _loop, float delay) 
    { 
       /*
        *    
            starting_delay = delay;

             VERIFY(_owner);
             owner_data = _owner->_p;
             VERIFY(owner_data);
             //	source					= (CSoundRender_Source*)owner_data->handle;
             p_source.position.set(0, 0, 0);
             p_source.min_distance = source()->m_fMinDist; // DS3D_DEFAULTMINDISTANCE;
             p_source.max_distance = source()->m_fMaxDist; // 300.f;
             p_source.base_volume = source()->m_fBaseVolume; // 1.f
             p_source.volume = 1.f; // 1.f
             p_source.freq = 1.f;
             p_source.max_ai_distance = source()->m_fMaxAIDist; // 300.f;

             if (fis_zero(delay, EPS_L))
             {
                 m_current_state = _loop ? stStartingLooped : stStarting;
             }
             else
             {
                 m_current_state = _loop ? stStartingLoopedDelayed : stStartingDelayed;
                 fTimeToPropagade = SoundRender->Timer.GetElapsed_sec();
             }
             bStopping = FALSE;
             bRewind = FALSE;
       */

        VERIFY(_owner);
        CurrentEvent = (xrEventInstance*)_owner->sound_state;
        VERIFY(CurrentEvent);
        
        set_position(Fvector({0, 0, 0}));
        FMOD_RESULT outResult = CurrentEvent->start();
        if (outResult != FMOD_OK)
        {
            return;
        }
    }

    void cancel() {} // manager forces out of rendering
    void update(float dt) {}
    bool update_culling(float dt) { return false; }
    void update_environment(float dt) {}
    void rewind() {}
    void stop(bool isDeffered) override {}
    void pause(bool bVal, int id) {}

    u32 play_time() override { return 0; }

    ~CSoundRender_EmitterFmod() {}
};


CSoundRender_CoreF* SoundRenderF = nullptr;

CSoundRender_CoreF::CSoundRender_CoreF()
{

}

CSoundRender_CoreF::~CSoundRender_CoreF() {}

void CSoundRender_CoreF::_restart() 
{

}

FMOD_3D_ATTRIBUTES
ConvertXrayListenerToFmodAttributes(SListener& xrayListener)
{
    FMOD_3D_ATTRIBUTES attributes = {{0}};
    attributes.position = XrayVectorToFmodVector(xrayListener.position);
    attributes.up = XrayVectorToFmodVector(xrayListener.orientation[0]);
    attributes.forward = XrayVectorToFmodVector(xrayListener.orientation[1]);
    return attributes;
}

SListener 
CSoundRender_CoreF::ConvertXrayAttributesToFmod(
    const Fvector& P, const Fvector& D, const Fvector& N, float dt)
{
    Fvector tempVector = {}; 
    SListener outListener = {};

    if (dt > 0.00001)
    {
        tempVector = Listener.position;
        tempVector.sub(P).div(dt);
    }

    outListener.position.set(P);
    outListener.orientation[0].set(D.x, D.y, -D.z);
    outListener.orientation[1].set(N.x, N.y, -N.z);
    return outListener;
}

xr_string 
ConvertXrayPathToFmodPath(const char* EventPath)
{
    xr_string returnString(EventPath);
    return returnString;
}

xrEventInstance*
CSoundRender_CoreF::CreateEventFromPath(const char* EventPath)
{
    FMOD::Studio::EventDescription* eventDescription = NULL;
    xr_string TempEventPath = ConvertXrayPathToFmodPath(EventPath);
    VERIFY(SoundSystem->getEvent(TempEventPath.c_str(), &eventDescription) == FMOD_OK);
    if (eventDescription == nullptr)
    {
        Msg("* SOUND: Can't load event with \"%s\" path", EventPath);
        return nullptr;
    }

    xrEventInstance* TempEventInstance = nullptr;
    R_ASSERT(eventDescription->createInstance(&TempEventInstance) == FMOD_OK);
    return TempEventInstance;
}

bool 
CSoundRender_CoreF::PlayEvent(xrEventInstance* eventInstance, const Fvector& P,
    const Fvector& D, const Fvector& N, float dt, bool isLooped)
{
    FMOD_RESULT outResult = eventInstance->start();
    if (outResult != FMOD_OK)
    {
        return false;
    }

    SListener tempListener = ConvertXrayAttributesToFmod(P, D, N, dt);
    const FMOD_3D_ATTRIBUTES fmodAttributes = ConvertXrayListenerToFmodAttributes(tempListener);
    outResult = eventInstance->set3DAttributes(&fmodAttributes);
    if (outResult != FMOD_OK)
    {
        eventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        return false;
    }

    return true;
}

bool
CSoundRender_CoreF::StopEvent(xrEventInstance* eventInstance)
{ return false; }

void CSoundRender_CoreF::_initialize()
{
    FMOD::Studio::Bank* MasterBank = nullptr;

    R_ASSERT(FMOD::Studio::System::create(&SoundSystem) == FMOD_OK);
    R_ASSERT(SoundSystem->getCoreSystem(&CoreSystem) == FMOD_OK);

    R_ASSERT(CoreSystem->setSoftwareFormat(48000, FMOD_SPEAKERMODE_STEREO, 0) == FMOD_OK);

    R_ASSERT(SoundSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, DriverData) == FMOD_OK);

    // Get path to master bank and try to load it
    string_path MasterBankPath = {};
    FS.update_path(MasterBankPath, "$game_sounds$", "$Master.bank");
    R_ASSERT(SoundSystem->loadBankFile(MasterBankPath, FMOD_STUDIO_LOAD_BANK_NORMAL, &MasterBank) == FMOD_OK);
    StudioBanks["Master"] = MasterBank;
}

void CSoundRender_CoreF::set_master_volume(float f)
{
    
}

void CSoundRender_CoreF::_clear()
{

}

void CSoundRender_CoreF::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt)
{
    Listener = ConvertXrayAttributesToFmod(P, D, N, dt);
}

bool CSoundRender_CoreF::i_locked()
{
    return false;
}

bool CSoundRender_CoreF::create(
    ref_sound& S, pcstr fName, esound_type sound_type, int game_type, bool replaceWithNoSound)
{
    S._p = xr_new<ref_sound_data>(fName, sound_type, game_type, replaceWithNoSound);

    if (S._handle() == nullptr && !replaceWithNoSound)
        S._p = nullptr; // no reason to keep it

    S.sound_state = CreateEventFromPath(fName);
    return S._handle() != nullptr;
}

void CSoundRender_CoreF::attach_tail(ref_sound& S, pcstr fName)
{
    string_path fn;
    xr_strcpy(fn, fName);
    if (strext(fn))
        *strext(fn) = 0;
    if (S._p->fn_attached[0].size() && S._p->fn_attached[1].size())
    {
#ifdef DEBUG
        Msg("! 2 file already in queue [%s][%s]", S._p->fn_attached[0].c_str(), S._p->fn_attached[1].c_str());
#endif // #ifdef DEBUG
        return;
    }

    u32 idx = S._p->fn_attached[0].size() ? 1 : 0;

    S._p->fn_attached[idx] = fn;
    //#TODO:
}

void CSoundRender_CoreF::clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type) 
{
    S._p = xr_new<ref_sound_data>();
    S._p->handle = from._p->handle;
    S._p->dwBytesTotal = from._p->dwBytesTotal;
    S._p->fTimeTotal = from._p->fTimeTotal;
    S._p->fn_attached[0] = from._p->fn_attached[0];
    S._p->fn_attached[1] = from._p->fn_attached[1];
    S._p->g_type = (game_type == sg_SourceType) ? S._p->handle->game_type() : game_type;
    S._p->s_type = sound_type;

    xrEventInstance* newInstance = (xrEventInstance*)from.sound_state;
    S.sound_state = from.sound_state;
}

void CSoundRender_CoreF::destroy(ref_sound& S) {}

void CSoundRender_CoreF::prefetch() {}

void CSoundRender_CoreF::stop_emitters() 
{
    
}

int CSoundRender_CoreF::pause_emitters(bool val) { return 0; }

void CSoundRender_CoreF::play(ref_sound& S, IGameObject* O, u32 flags, float delay)
{
    /*
            if (!bPresent || nullptr == S._handle())
                return;
            S._p->g_object = O;
            if (S._feedback())
                ((CSoundRender_Emitter*)S._feedback())->rewind();
            else
                i_play(&S, flags & sm_Looped, delay);

            if (flags & sm_2D || S._handle()->channels_num() == 2)
                S._feedback()->switch_to_2D();
    */

    S._p->g_object = O;
    if (S._feedback() != nullptr)
    {
        ((CSoundRender_EmitterFmod*)S._feedback())->rewind();
    }
    else
    {
        CSoundRender_EmitterFmod* E = xr_new<CSoundRender_EmitterFmod>((xrEventInstance*)S.sound_state);
        S._p->feedback = E;
        E->start(&S, false, delay);
    }
}

void CSoundRender_CoreF::play_at_pos(ref_sound& S, IGameObject* O, const Fvector& pos, u32 flags, float delay)
{
    S._p->g_object = O;
    if (S._feedback() != nullptr)
    {
        ((CSoundRender_EmitterFmod*)S._feedback())->rewind();
    }
    else
    {
        CSoundRender_EmitterFmod* E = xr_new<CSoundRender_EmitterFmod>((xrEventInstance*)S.sound_state);
        S._p->feedback = E;
        E->start(&S, false, delay);
        Fvector tempVector = {};
        SetEventPosition((xrEventInstance*)S.sound_state, pos, tempVector, tempVector, 0.f);
    }
   
}

void CSoundRender_CoreF::play_no_feedback(ref_sound& S, IGameObject* O, u32 flags, float delay,
    Fvector* pos, float* vol, float* freq, Fvector2* range)
{
}

void CSoundRender_CoreF::set_geometry_env(IReader* I) 
{}

void CSoundRender_CoreF::set_geometry_som(IReader* I) {}

void CSoundRender_CoreF::set_geometry_occ(CDB::MODEL* M) {}

void CSoundRender_CoreF::set_handler(sound_event* E) {}

void CSoundRender_CoreF::update(const Fvector& P, const Fvector& D, const Fvector& N) {}

void CSoundRender_CoreF::statistic(CSound_stats* s0, CSound_stats_ext* s1) {}

void CSoundRender_CoreF::DumpStatistics(class IGameFont& font, class IPerformanceAlert* alert) {}

float CSoundRender_CoreF::get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion)
{
    return 0.f;
}

float CSoundRender_CoreF::get_occlusion(Fvector& P, float R, Fvector* occ) { return 0.f; }

void CSoundRender_CoreF::object_relcase(IGameObject* obj) {}

SoundEnvironment_LIB* CSoundRender_CoreF::get_env_library() { return nullptr; }

void CSoundRender_CoreF::refresh_env_library()
{

};

void CSoundRender_CoreF::set_user_env(CSound_environment* E)
{

};
void CSoundRender_CoreF::refresh_sources()
{

};
void CSoundRender_CoreF::set_environment(u32 id, CSound_environment** dst_env)
{

};
void CSoundRender_CoreF::set_environment_size(CSound_environment* src_env, CSound_environment** dst_env)
{

};
