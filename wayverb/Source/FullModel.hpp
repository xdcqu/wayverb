#pragma once

#include "common/orientable.h"
#include "common/scene_data.h"

#include "combined/engine.h"
#include "combined/model.h"

#include "DataBinding/collection.hpp"

#include "cereal/types/vector.hpp"

namespace model {

template <>
class ValueWrapper<glm::vec3> : public ModelValue<glm::vec3> {
public:
    ValueWrapper(ModelMember* owner, const glm::vec3& u)
            : ModelValue<glm::vec3>(owner)
            , x(this, u.x)
            , y(this, u.y)
            , z(this, u.z) {}

    glm::vec3 get() const override {
        return glm::vec3{x.get(), y.get(), z.get()};
    }

    void set(const glm::vec3& u, bool do_notify = true) override {
        x.set(u.x, do_notify);
        y.set(u.y, do_notify);
        z.set(u.z, do_notify);
    }

    ValueWrapper<float> x;
    ValueWrapper<float> y;
    ValueWrapper<float> z;
};

template <>
class ValueWrapper<volume_type> : public ModelValue<volume_type> {
public:
    ValueWrapper(ModelMember* owner, const volume_type& u)
            : ModelValue<volume_type>(owner)
            , s0(this, u.s[0])
            , s1(this, u.s[1])
            , s2(this, u.s[2])
            , s3(this, u.s[3])
            , s4(this, u.s[4])
            , s5(this, u.s[5])
            , s6(this, u.s[6])
            , s7(this, u.s[7]) {}

    volume_type get() const override {
        return volume_type{s0.get(),
                           s1.get(),
                           s2.get(),
                           s3.get(),
                           s4.get(),
                           s5.get(),
                           s6.get(),
                           s7.get()};
    }

    void set(const volume_type& u, bool do_notify = true) override {
        s0.set(u.s[0], do_notify);
        s1.set(u.s[1], do_notify);
        s2.set(u.s[2], do_notify);
        s3.set(u.s[3], do_notify);
        s4.set(u.s[4], do_notify);
        s5.set(u.s[5], do_notify);
        s6.set(u.s[6], do_notify);
        s7.set(u.s[7], do_notify);
    }

    ValueWrapper<float> s0;
    ValueWrapper<float> s1;
    ValueWrapper<float> s2;
    ValueWrapper<float> s3;
    ValueWrapper<float> s4;
    ValueWrapper<float> s5;
    ValueWrapper<float> s6;
    ValueWrapper<float> s7;
};

template <>
class ValueWrapper<surface> : public ModelValue<surface> {
public:
    ValueWrapper(ModelMember* owner, const surface& u)
            : ModelValue<surface>(owner)
            , specular(this, u.specular)
            , diffuse(this, u.diffuse) {}

    surface get() const override {
        return surface{specular.get(), diffuse.get()};
    }

    void set(const surface& u, bool do_notify = true) override {
        specular.set(u.specular, do_notify);
        diffuse.set(u.diffuse, do_notify);
    }

    ValueWrapper<volume_type> specular;
    ValueWrapper<volume_type> diffuse;
};

template <>
class ValueWrapper<scene_data::material>
        : public ModelValue<scene_data::material> {
public:
    ValueWrapper(ModelMember* owner, const scene_data::material& u)
            : ModelValue<scene_data::material>(owner)
            , name(this, u.name)
            , surface(this, u.surface) {}

    scene_data::material get() const override {
        return scene_data::material{name.get(), surface.get()};
    }

    void set(const scene_data::material& u, bool do_notify = true) override {
        name.set(u.name, do_notify);
        surface.set(u.surface, do_notify);
    }

    ValueWrapper<std::string> name;
    ValueWrapper<surface> surface;
};

template <>
class ValueWrapper<AzEl> : public ModelValue<AzEl> {
public:
    ValueWrapper(ModelMember* owner, const AzEl& u)
            : ModelValue<AzEl>(owner)
            , azimuth(this, u.azimuth)
            , elevation(this, u.elevation) {}

    AzEl get() const override { return AzEl{azimuth.get(), elevation.get()}; }

    void set(const AzEl& u, bool do_notify = true) override {
        azimuth.set(u.azimuth, do_notify);
        elevation.set(u.elevation, do_notify);
    }

    ValueWrapper<float> azimuth;
    ValueWrapper<float> elevation;
};

template <>
class ValueWrapper<Pointer> : public ModelValue<Pointer> {
public:
    ValueWrapper(ModelMember* owner, const Pointer& u)
            : ModelValue<Pointer>(owner)
            , mode(this, u.mode)
            , spherical(this, u.spherical)
            , look_at(this, u.look_at) {}

    Pointer get() const override {
        return Pointer{mode.get(), spherical.get(), look_at.get()};
    }

    void set(const Pointer& u, bool do_notify = true) override {
        mode.set(u.mode, do_notify);
        spherical.set(u.spherical, do_notify);
        look_at.set(u.look_at, do_notify);
    }

    ValueWrapper<Pointer::Mode> mode;
    ValueWrapper<AzEl> spherical;
    ValueWrapper<glm::vec3> look_at;
};

template <>
class ValueWrapper<Microphone> : public ModelValue<Microphone> {
public:
    ValueWrapper(ModelMember* owner, const Microphone& u)
            : ModelValue<Microphone>(owner)
            , pointer(this, u.pointer)
            , shape(this, u.shape) {}

    Microphone get() const override {
        return Microphone{pointer.get(), shape.get()};
    }

    void set(const Microphone& u, bool do_notify = true) override {
        pointer.set(u.pointer, do_notify);
        shape.set(u.shape, do_notify);
    }

    ValueWrapper<Pointer> pointer;
    ValueWrapper<float> shape;
};

template <>
class ValueWrapper<ReceiverSettings> : public ModelValue<ReceiverSettings> {
public:
    ValueWrapper(ModelMember* owner, const ReceiverSettings& u)
            : ModelValue<ReceiverSettings>(owner)
            , position(this, u.position)
            , mode(this, u.mode)
            , microphones(this, u.microphones)
            , hrtf(this, u.hrtf) {}

    ReceiverSettings get() const override {
        return ReceiverSettings{
                position.get(), mode.get(), microphones.get(), hrtf.get()};
    }

    void set(const ReceiverSettings& u, bool do_notify = true) override {
        position.set(u.position, do_notify);
        mode.set(u.mode, do_notify);
        microphones.set(u.microphones, do_notify);
        hrtf.set(u.hrtf, do_notify);
    }

    ValueWrapper<glm::vec3> position;
    ValueWrapper<ReceiverSettings::Mode> mode;
    ValueWrapper<aligned::vector<Microphone>> microphones;
    ValueWrapper<Pointer> hrtf;
};

template <>
class ValueWrapper<SingleShot> : public ModelValue<SingleShot> {
public:
    ValueWrapper(ModelMember* owner, const SingleShot& u)
            : ModelValue<SingleShot>(owner)
            , filter_frequency(this, u.filter_frequency)
            , oversample_ratio(this, u.oversample_ratio)
            , speed_of_sound(this, u.speed_of_sound)
            , rays(this, u.rays)
            , source(this, u.source)
            , receiver_settings(this, u.receiver_settings) {}

    SingleShot get() const override {
        return SingleShot{filter_frequency.get(),
                          oversample_ratio.get(),
                          speed_of_sound.get(),
                          rays.get(),
                          source.get(),
                          receiver_settings.get()};
    }

    void set(const SingleShot& u, bool do_notify = true) override {
        filter_frequency.set(u.filter_frequency, do_notify);
        oversample_ratio.set(u.oversample_ratio, do_notify);
        speed_of_sound.set(u.speed_of_sound, do_notify);
        rays.set(u.rays, do_notify);
        source.set(u.source, do_notify);
        receiver_settings.set(u.receiver_settings, do_notify);
    }

    ValueWrapper<float> filter_frequency;
    ValueWrapper<float> oversample_ratio;
    ValueWrapper<float> speed_of_sound;
    ValueWrapper<size_t> rays;
    ValueWrapper<glm::vec3> source;
    ValueWrapper<ReceiverSettings> receiver_settings;
};

template <>
class ValueWrapper<App> : public ModelValue<App> {
public:
    ValueWrapper(ModelMember* owner, const App& u)
            : ModelValue<App>(owner)
            , filter_frequency(this, u.filter_frequency)
            , oversample_ratio(this, u.oversample_ratio)
            , speed_of_sound(this, u.speed_of_sound)
            , rays(this, u.rays)
            , source(this, u.source)
            , receiver_settings(this, u.receiver_settings) {}

    App get() const override {
        return App{filter_frequency.get(),
                   oversample_ratio.get(),
                   speed_of_sound.get(),
                   rays.get(),
                   source.get(),
                   receiver_settings.get()};
    }

    void set(const App& u, bool do_notify = true) override {
        filter_frequency.set(u.filter_frequency, do_notify);
        oversample_ratio.set(u.oversample_ratio, do_notify);
        speed_of_sound.set(u.speed_of_sound, do_notify);
        rays.set(u.rays, do_notify);
        source.set(u.source, do_notify);
        receiver_settings.set(u.receiver_settings, do_notify);
    }

    ValueWrapper<float> filter_frequency;
    ValueWrapper<float> oversample_ratio;
    ValueWrapper<float> speed_of_sound;
    ValueWrapper<size_t> rays;
    ValueWrapper<aligned::vector<glm::vec3>> source;
    ValueWrapper<aligned::vector<ReceiverSettings>> receiver_settings;
};

class RenderState {
public:
    bool is_rendering{false};
    wayverb::state state{wayverb::state::idle};
    double progress{0};
    bool visualise{true};
};

template <>
class ValueWrapper<RenderState> : public ModelValue<RenderState> {
public:
    ValueWrapper(ModelMember* owner, const RenderState& u)
            : ModelValue<RenderState>(owner)
            , is_rendering(this, u.is_rendering)
            , state(this, u.state)
            , progress(this, u.progress)
            , visualise(this, u.visualise) {}

    RenderState get() const override {
        return RenderState{is_rendering.get(),
                           state.get(),
                           progress.get(),
                           visualise.get()};
    }

    void set(const RenderState& u, bool do_notify = true) override {
        is_rendering.set(u.is_rendering, do_notify);
        state.set(u.state, do_notify);
        progress.set(u.progress, do_notify);
        visualise.set(u.visualise, do_notify);
    }

    void start() { is_rendering.set(true); }

    void stop() {
        is_rendering.set(false);
        state.set(wayverb::state::idle);
        progress.set(0);
    }

    ValueWrapper<bool> is_rendering;
    ValueWrapper<wayverb::state> state;
    ValueWrapper<double> progress;
    ValueWrapper<bool> visualise;
};

class Persistent {
public:
    App app;
    aligned::vector<scene_data::material> materials;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("app", app),
                cereal::make_nvp("materials", materials));
    }
};

template <>
class ValueWrapper<Persistent> : public ModelValue<Persistent> {
public:
    ValueWrapper(ModelMember* owner, const Persistent& u)
            : ModelValue<Persistent>(owner)
            , app(this, u.app)
            , materials(this, u.materials) {}

    Persistent get() const override {
        return Persistent{app.get(), materials.get()};
    }

    void set(const Persistent& u, bool do_notify = true) override {
        app.set(u.app, do_notify);
        materials.set(u.materials, do_notify);
    }

    ValueWrapper<App> app;
    ValueWrapper<aligned::vector<scene_data::material>> materials;
};

class FullModel {
public:
    Persistent persistent;
    aligned::vector<scene_data::material> presets;
    RenderState render_state;
    int shown_surface{-1};
    bool needs_save{false};
};

template <>
class ValueWrapper<FullModel> : public ModelValue<FullModel> {
public:
    ValueWrapper(ModelMember* owner, const FullModel& u)
            : ModelValue<FullModel>(owner)
            , persistent(this, u.persistent)
            , presets(this, u.presets)
            , render_state(this, u.render_state)
            , shown_surface(this, u.shown_surface)
            , needs_save(this, u.needs_save) {}

    FullModel get() const override {
        return FullModel{persistent.get(),
                         presets.get(),
                         render_state.get(),
                         shown_surface.get(),
                         needs_save.get()};
    }

    void set(const FullModel& u, bool do_notify = true) override {
        persistent.set(u.persistent, do_notify);
        presets.set(u.presets, do_notify);
        render_state.set(u.render_state, do_notify);
        shown_surface.set(u.shown_surface, do_notify);
        needs_save.set(u.needs_save, do_notify);
    }

    ValueWrapper<Persistent> persistent;
    ValueWrapper<aligned::vector<scene_data::material>> presets;
    ValueWrapper<RenderState> render_state;
    ValueWrapper<int> shown_surface;
    ValueWrapper<bool> needs_save;
};

}  // namespace model