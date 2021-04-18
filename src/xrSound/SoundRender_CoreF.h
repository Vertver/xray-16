#pragma once
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "SoundRender_Core.h"

inline FMOD_VECTOR XrayVectorToFmodVector(const Fvector& xrayVector)
{
    FMOD_VECTOR returnVector;
    returnVector.x = xrayVector[0];
    returnVector.y = xrayVector[1];
    returnVector.z = xrayVector[2];
    return returnVector;
}

struct SListener
{
    Fvector position;
    Fvector orientation[2];
};

inline SListener XrayAttributesListener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) 
{
    SListener ret = {};
    ret.position.set(P);
    ret.orientation[0].set(D.x, D.y, -D.z);
    ret.orientation[1].set(N.x, N.y, -N.z);
    return ret;
}

FMOD_3D_ATTRIBUTES ConvertXrayListenerToFmodAttributes(SListener& xrayListener);
xr_string ConvertXrayPathToFmodPath(const char* EventPath);

using xrEventInstance = FMOD::Studio::EventInstance;
using xrSoundBank = FMOD::Studio::Bank;

inline bool SetEventPosition(
    xrEventInstance* pEventInstance, const Fvector& P, const Fvector& D, const Fvector& N, float dt)
{
    FMOD_RESULT outResult = {};
    SListener tempListener = XrayAttributesListener(P, D, N, dt);
    const FMOD_3D_ATTRIBUTES fmodAttributes = ConvertXrayListenerToFmodAttributes(tempListener);
    outResult = pEventInstance->set3DAttributes(&fmodAttributes);
    if (outResult != FMOD_OK)
    {
        pEventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        return false;
    }

    return true;
}

class CSoundRender_CoreF : public ISoundManager
{    
    void* DriverData = nullptr;
    
    FMOD::Studio::System* SoundSystem = nullptr;
    FMOD::System* CoreSystem = nullptr;

    xr_unordered_map<xr_string, xrSoundBank*> StudioBanks;
    xr_vector<xrEventInstance*> EventInstances;

    CDB::COLLIDER geom_DB = {};
    CDB::MODEL* geom_SOM = nullptr;
    CDB::MODEL* geom_MODEL = nullptr;
    CDB::MODEL* geom_ENV = nullptr;

    SListener Listener;
    xrEventInstance* CreateEventFromPath(const char* EventPath);
    SListener ConvertXrayAttributesToFmod(const Fvector& P, const Fvector& D, const Fvector& N, float dt);

    bool PlayEvent(
        xrEventInstance* eventInstance, const Fvector& P, const Fvector& D, const Fvector& N, float dt, bool isLooped);
    bool StopEvent(xrEventInstance* eventInstance);

      virtual bool _create_data(
        ref_sound_data& S, pcstr fName, esound_type sound_type, int game_type, bool replaceWithNoSound = true)
    {
        return false;
    };
    virtual void _destroy_data(ref_sound_data& S) {};


public:
    CSoundRender_CoreF();
    virtual ~CSoundRender_CoreF();

    void _initialize() override;
    void _clear() override;
    void _restart() override;

    void set_master_volume(float f) override;

    const Fvector& listener_position() override { return Listener.position; }

    void update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt);

    bool i_locked() override;

    bool create(
        ref_sound& S, pcstr fName, esound_type sound_type, int game_type, bool replaceWithNoSound = true) override;
    void attach_tail(ref_sound& S, pcstr fName) override;
    void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type) override;
    void destroy(ref_sound& S) override;

    void prefetch() override;

    void stop_emitters() override;
    int pause_emitters(bool val) override;

    void play(ref_sound& S, IGameObject* O, u32 flags = 0, float delay = 0.f) override;
    void play_at_pos(ref_sound& S, IGameObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f) override;
    void play_no_feedback(ref_sound& S, IGameObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = nullptr,
        float* vol = nullptr, float* freq = nullptr, Fvector2* range = nullptr) override;

    void set_geometry_env(IReader* I) override;
    void set_geometry_som(IReader* I) override;
    void set_geometry_occ(CDB::MODEL* M) override;
    void set_handler(sound_event* E) override;

    void update(const Fvector& P, const Fvector& D, const Fvector& N) override;
    void statistic(CSound_stats* s0, CSound_stats_ext* s1) override;
    void DumpStatistics(class IGameFont& font, class IPerformanceAlert* alert) override;

    float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f) override;
    float get_occlusion(Fvector& P, float R, Fvector* occ) override;

    void object_relcase(IGameObject* obj) override;

    SoundEnvironment_LIB* get_env_library() override;
    void refresh_env_library() override;
    void set_user_env(CSound_environment* E) override;
    void refresh_sources() override;
    void set_environment(u32 id, CSound_environment** dst_env) override;
    void set_environment_size(CSound_environment* src_env, CSound_environment** dst_env) override;

};
extern CSoundRender_CoreF* SoundRenderF;
