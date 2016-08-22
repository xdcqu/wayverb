#pragma once

#include "FullModel.hpp"

#include "OtherComponents/WorkQueue.hpp"
#include "UtilityComponents/RAIIThread.hpp"

#include "waveguide/mesh/model.h"

class MeshGeneratorFunctor final {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void mesh_generator_finished(waveguide::mesh::model model) = 0;

    protected:
        ~Listener() noexcept = default;
    };

    MeshGeneratorFunctor(Listener& listener,
                         const copyable_scene_data& scene_data,
                         double sample_rate,
                         double speed_of_sound);

    void operator()() const;

private:
    Listener& listener;

    model::Persistent persistent;
    copyable_scene_data scene_data;
    double sample_rate;
    double speed_of_sound;
};

//----------------------------------------------------------------------------//

class MeshGeneratorThread final {
public:
    MeshGeneratorThread(MeshGeneratorFunctor::Listener& listener,
                        const copyable_scene_data& scene_data,
                        double sample_rate,
                        double speed_of_sound);

private:
    RAIIThread thread;
};

//----------------------------------------------------------------------------//

class AsyncMeshGenerator final {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void async_mesh_generator_finished(
                const AsyncMeshGenerator*, waveguide::mesh::model model) = 0;

    protected:
        ~Listener() noexcept = default;
    };

    void run(const copyable_scene_data& scene_data,
             double sample_rate,
             double speed_of_sound);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    mutable std::mutex mut;

    /// Extra level of indirection yo
    class ConcreteListener final : public MeshGeneratorFunctor::Listener {
    public:
        ConcreteListener(AsyncMeshGenerator& mesh_generator);

        void mesh_generator_finished(waveguide::mesh::model model) override;

        void run(const copyable_scene_data& scene_data,
                 double sample_rate,
                 double speed_of_sound);

        void addListener(AsyncMeshGenerator::Listener* l);
        void removeListener(AsyncMeshGenerator::Listener* l);

    private:
        AsyncMeshGenerator& mesh_generator;
        ListenerList<AsyncMeshGenerator::Listener> listener_list;
        AsyncWorkQueue work_queue;

        //  IMPORTANT
        //  The thread relies on the rest of this object. It communicates with
        //  the outside world through the work_queue and listener_list.
        //  It MUST be destroyed before these objects so that it doesn't try to
        //  use them (it must be after them in the class declaration b/c object
        //  members are destroyed in reverse-declaration order).
        std::unique_ptr<MeshGeneratorThread> thread;
    };

    ConcreteListener concrete_listener{*this};
};