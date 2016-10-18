#pragma once

#include "raytracer/image_source/finder.h"
#include "raytracer/image_source/reflection_path_builder.h"
#include "raytracer/postprocess.h"
#include "raytracer/reflector.h"

namespace raytracer {
namespace image_source {

//  Has to take voxelised_scene_data<cl_float3, surface> because it uses GPU.
template <typename Func, typename It>
auto run(It b,  /// Iterators over ray directions.
         It e,
         const compute_context& cc,
         const voxelised_scene_data<cl_float3, surface>& voxelised,
         const glm::vec3& source,
         const glm::vec3& receiver,
         bool flip_phase) {
    const auto reflection_depth{raytracer::compute_optimum_reflection_number(
            voxelised.get_scene_data())};

    const scene_buffers buffers{cc.context, voxelised};
    const auto make_ray_iterator{[&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{source, i};
        });
    }};
    raytracer::reflector ref{
            cc, receiver, make_ray_iterator(b), make_ray_iterator(e)};

    //  This will collect the first reflections, to a specified depth,
    //  and use them to find unique image-source paths.
    tree tree{};
    {
        raytracer::image_source::reflection_path_builder builder{
                static_cast<size_t>(std::distance(b, e))};

        //  Run the simulation proper.

        //  Up until the max reflection depth.
        for (auto i{0u}; i != reflection_depth; ++i) {
            //  Get a single step of the reflections.
            const auto reflections{ref.run_step(buffers)};

            //  Find diffuse impulses for these reflections.
            builder.push(begin(reflections), end(reflections));
        }

        for (const auto& path : builder.get_data()) {
            tree.push(path);
        }
    }

    return postprocess<Func>(begin(tree.get_branches()),
                             end(tree.get_branches()),
                             source,
                             receiver,
                             voxelised,
                             flip_phase);
}

}  // namespace image_source
}  // namespace raytracer