#include "core/azimuth_elevation.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/scene_data_loader.h"
#include "core/spatial_division/scene_buffers.h"
#include "core/spatial_division/voxel_collection.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/map_to_vector.h"
#include "utilities/string_builder.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

namespace {
template <typename Vertex, typename Surface>
auto get_voxelised(const core::generic_scene_data<Vertex, Surface>& scene) {
    return make_voxelised_scene_data(scene, 5, 0.1f);
}

auto get_test_scenes() {
    return util::aligned::vector<core::scene_data_loader::scene_data>{
            core::geo::get_scene_data(
                    core::geo::box{glm::vec3(0, 0, 0), glm::vec3(4, 3, 6)},
                    core::scene_data_loader::material{
                            "default",
                            core::make_surface<core::simulation_bands>(0, 0)}),
            core::geo::get_scene_data(
                    core::geo::box{glm::vec3(0, 0, 0), glm::vec3(3, 3, 3)},
                    core::scene_data_loader::material{
                            "default",
                            core::make_surface<core::simulation_bands>(0, 0)}),
            core::scene_data_loader{OBJ_PATH}.get_scene_data()};
}

TEST(voxel, walk) {
    for (const auto& scene : get_test_scenes()) {
        const auto voxelised = get_voxelised(scene);
        for (const auto& i : core::get_random_directions(1000)) {
            core::geo::ray ray{glm::vec3{0, 1, 0}, core::to_vec3(i)};
            bool has_triangles{false};
            traverse(voxelised.get_voxels(),
                     ray,
                     [&](const auto&, const auto& items, float, float) {
                         if (!items.empty()) {
                             has_triangles = true;
                         }
                         return false;
                     });
            ASSERT_TRUE(has_triangles);
        }
    }
}

TEST(voxel, flatten) {
    for (const auto& scene : get_test_scenes()) {
        const auto voxelised = get_voxelised(scene);
        const auto f = get_flattened(voxelised.get_voxels());
    }
}

TEST(voxel, surrounded) {
    const glm::vec3 source{1, 2, 1};
    for (const auto& scene : get_test_scenes()) {
        const auto voxelised =
                get_voxelised(scene_with_extracted_surfaces(scene));
        const core::compute_context cc{};
        const auto buffers = make_scene_buffers(cc.context, voxelised);

        const auto directions = core::get_random_directions(1000);

        const auto get_fast_intersections = [&](const auto& directions) {
            return util::map_to_vector(
                    begin(directions), end(directions), [&](const auto& i) {
                        const auto ray = core::geo::ray{source, i};
                        return intersects(voxelised, ray);
                    });
        };
        const auto fast_intersections = get_fast_intersections(directions);

        const auto slow_intersections = [&](const auto& directions) {
            return util::map_to_vector(
                    begin(directions), end(directions), [&](const auto& i) {
                        return core::geo::ray_triangle_intersection(
                                core::geo::ray{source, i},
                                voxelised.get_scene_data().get_triangles(),
                                core::convert(voxelised.get_scene_data()
                                                      .get_vertices()));
                    });
        }(directions);

        util::aligned::vector<glm::vec3> problem_directions{};
        std::set<size_t> problem_surfaces{};
        for (auto i = 0u; i != directions.size(); ++i) {
            if (!static_cast<bool>(fast_intersections[i])) {
                problem_directions.emplace_back(directions[i]);
                problem_surfaces.insert(slow_intersections[i]->index);
            }
        }

        if (!problem_directions.empty()) {
            std::cout << "problem directions:\n";
            for (const auto i : problem_directions) {
                std::cout << '{' << i.x << ", " << i.y << ", " << i.z << "}\n";
            }
        }

        if (!problem_surfaces.empty()) {
            std::cout << "problem surfaces:\n";
            for (const auto i : problem_surfaces) {
                std::cout << i << '\n';
            }
        }

        //  now let's try just the problematic directions
        const auto problem_intersections =
                get_fast_intersections(problem_directions);

        for (auto i = 0u; i != directions.size(); ++i) {
            ASSERT_EQ(static_cast<bool>(fast_intersections[i]),
                      static_cast<bool>(slow_intersections[i]));
            ASSERT_EQ(fast_intersections[i]->index,
                      slow_intersections[i]->index);
        }
    }
}

template <typename Vertex, typename Surface>
void compare(const glm::vec3& source,
             const core::generic_scene_data<Vertex, Surface>& scene) {
    const auto voxelised = get_voxelised(scene_with_extracted_surfaces(scene));
    const core::compute_context cc{};
    const auto buffers = make_scene_buffers(cc.context, voxelised);

    const auto directions = core::get_random_directions(1000);

    const auto get_fast_intersections = [&](const auto& directions) {
        return util::map_to_vector(
                begin(directions), end(directions), [&](const auto& i) {
                    const auto ray{core::geo::ray{source, i}};
                    return intersects(voxelised, ray);
                });
    };
    const auto fast_intersections = get_fast_intersections(directions);

    const auto slow_intersections = [&](const auto& directions) {
        return util::map_to_vector(
                begin(directions), end(directions), [&](const auto& i) {
                    return core::geo::ray_triangle_intersection(
                            core::geo::ray{source, i},
                            voxelised.get_scene_data().get_triangles(),
                            core::convert(
                                    voxelised.get_scene_data().get_vertices()));
                });
    }(directions);

    util::aligned::vector<glm::vec3> problematic{};
    for (auto i = 0u; i != directions.size(); ++i) {
        if (fast_intersections[i] != slow_intersections[i]) {
            problematic.emplace_back(directions[i]);
        }
        ASSERT_EQ(static_cast<bool>(fast_intersections[i]),
                  static_cast<bool>(slow_intersections[i]));
        if (fast_intersections[i]) {
            ASSERT_EQ(fast_intersections[i]->index,
                      slow_intersections[i]->index);
        }
    }

    if (!problematic.empty()) {
        for (const auto& i : problematic) {
            std::cout << util::build_string(i.x, ", ", i.y, ", ", i.z, '\n');
        }
    }

    get_fast_intersections(problematic);

    ASSERT_EQ(problematic.size(), 0);
}

TEST(voxel, compare) {
    for (const auto& source :
         {glm::vec3{-100, -100, -100}, glm::vec3{100, 100, 100}}) {
        for (const auto& scene : get_test_scenes()) {
            compare(source, scene);
        }
    }
}
}  // namespace