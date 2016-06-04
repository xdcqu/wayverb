#pragma once

#include "AxesObject.hpp"
#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"
#include "LitSceneShader.hpp"
#include "MeshObject.hpp"
#include "ModelObject.hpp"
#include "ModelSectionObject.hpp"
#include "OctahedronObject.hpp"
#include "PointObject.hpp"

#include "FullModel.hpp"

#define GLM_FORCE_RADIANS
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "combined/config.h"

#include "common/octree.h"
#include "common/scene_data.h"
#include "common/voxel_collection.h"

#include "waveguide/waveguide.h"

#include "raytracer/raytracer.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <future>
#include <mutex>
#include <queue>

template <typename T>
class WorkItemOwner {
public:
    WorkItemOwner(T& obj)
            : obj(obj) {
    }

    struct WorkItem {
        virtual ~WorkItem() noexcept = default;
        virtual void operator()(T& obj) const = 0;
    };

    template <typename Method>
    struct GenericWorkItem : public WorkItem {
        GenericWorkItem(Method&& method)
                : method(std::forward<Method>(method)) {
        }
        void operator()(T& obj) const override {
            method(obj);
        }
        Method method;
    };

    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::make_unique<GenericWorkItem<Method>>(
            std::forward<Method>(method)));
    }

    void pop_one() {
        std::lock_guard<std::mutex> lck(mut);
        pop_impl();
    }

    void pop_all() {
        std::lock_guard<std::mutex> lck(mut);
        while (!work_items.empty()) {
            pop_impl();
        }
    }

    auto size() const {
        std::lock_guard<std::mutex> lck(mut);
        return work_items.size();
    }

private:
    void pop_impl() {
        (*work_items.front())(obj);
        work_items.pop();
    }

    T& obj;
    std::queue<std::unique_ptr<WorkItem>> work_items;
    mutable std::mutex mut;
};

class MultiMaterialObject : public ::Drawable {
public:
    MultiMaterialObject(const GenericShader& generic_shader,
                        const LitSceneShader& lit_scene_shader,
                        const CopyableSceneData& scene_data);

    void draw() const override;

    class SingleMaterialSection : public ::Drawable {
    public:
        SingleMaterialSection(const CopyableSceneData& scene_data,
                              int material_index);

        void draw() const override;

    private:
        static std::vector<GLuint> get_indices(
            const CopyableSceneData& scene_data, int material_index);
        StaticIBO ibo;
        GLuint size;
    };

    void set_highlighted(int material);
    void set_colour(const glm::vec3& c);

private:
    const GenericShader& generic_shader;
    const LitSceneShader& lit_scene_shader;

    VAO wire_vao;
    VAO fill_vao;
    StaticVBO geometry;
    StaticVBO colors;

    int highlighted{-1};

    std::vector<SingleMaterialSection> sections;
};

class DrawableScene final : public ::Drawable {
public:
    DrawableScene(const GenericShader& generic_shader,
                  const MeshShader& mesh_shader,
                  const LitSceneShader& lit_scene_shader,
                  const CopyableSceneData& scene_data);

    void draw() const override;

    void set_receiver(const glm::vec3& u);
    void set_source(const glm::vec3& u);

    void set_rendering(bool b);

    void set_positions(const std::vector<glm::vec3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void set_receiver_pointing(const std::vector<glm::vec3>& directions);

    void set_emphasis(const glm::vec3& c);

private:
    const GenericShader& generic_shader;
    const MeshShader& mesh_shader;
    const LitSceneShader& lit_scene_shader;

    MultiMaterialObject model_object;
    PointObject source_object;
    PointObject receiver_object;

    std::unique_ptr<MeshObject> mesh_object;

    bool rendering{false};
};

class SceneRenderer final : public OpenGLRenderer, public ChangeBroadcaster {
public:
    SceneRenderer(const CopyableSceneData& model);
    virtual ~SceneRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void set_viewport(const glm::ivec2& v);
    void set_scale(float u);
    void set_rotation(const Orientable::AzEl& u);

    void set_rendering(bool b);

    void set_receiver(const glm::vec3& u);
    void set_source(const glm::vec3& u);

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void set_highlighted(int u);

    void set_emphasis(const glm::vec3& c);

    void set_receiver_pointing(const std::vector<glm::vec3>& directions);

    void mouse_down(const glm::vec2& pos);
    void mouse_drag(const glm::vec2& pos);
    void mouse_up(const glm::vec2& pos);
    void mouse_wheel_move(float delta_y);

private:
    //  gl state should be updated inside `update` methods
    //  where we know that stuff is happening on the gl thread
    //  if you (e.g.) delete a buffer on a different thread, your computer will
    //  get mad and maybe freeze
    WorkItemOwner<SceneRenderer> work_queue;
    CopyableSceneData model;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;
};
