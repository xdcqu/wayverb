#include "box/img_src.h"

#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

aligned::vector<impulse<8>> run_exact_img_src(const geo::box& box,
                                              float absorption,
                                              const model::parameters& params,
                                              float simulation_time,
                                              bool flip_phase) {
    auto ret{raytracer::image_source::find_impulses<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            box,
            params.source,
            params.receiver,
            make_surface(absorption, 0),
            simulation_time * params.speed_of_sound,
            flip_phase)};
    //  Correct for distance travelled.
    for (auto& it : ret) {
        it.volume *=
                pressure_for_distance(it.distance, params.acoustic_impedance);
    }
    return ret;
}

aligned::vector<impulse<8>> run_fast_img_src(const geo::box& box,
                                             float absorption,
                                             const model::parameters& params,
                                             bool flip_phase) {
    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const auto directions{get_random_directions(1 << 13)};
    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            begin(directions),
            end(directions),
            compute_context{},
            voxelised,
            params.source,
            params.receiver,
            flip_phase)};

    if (const auto direct{raytracer::get_direct(
                params.source, params.receiver, voxelised)}) {
        impulses.emplace_back(*direct);
    }
    //  Correct for distance travelled.
    for (auto& it : impulses) {
        it.volume *=
                pressure_for_distance(it.distance, params.acoustic_impedance);
    }
    return impulses;
}