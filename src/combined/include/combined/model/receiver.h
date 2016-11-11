#pragma once

#include "combined/model/capsule.h"
#include "combined/model/constrained_point.h"
#include "combined/model/vector.h"

#include "cereal/types/base_class.hpp"

namespace wayverb {
namespace combined {
namespace model {

class receiver final : public owning_member<receiver,
                                            constrained_point,
                                            vector<capsule, 1>> {
    friend class vector<receiver, 1>;
    receiver() = default;

public:
    explicit receiver(core::geo::box bounds);

    void set_name(std::string name);
    std::string get_name() const;

    auto& position() { return get<constrained_point>(); }
    const auto& position() const { return get<constrained_point>(); }

    auto& capsules() { return get<vector<capsule, 1>>(); }
    const auto& capsules() const { return get<vector<capsule, 1>>(); }

    void set_orientation(float azimuth, float elevation);
    core::orientable get_orientation() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(cereal::base_class<type>(this), name_, orientation_);
        notify();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(cereal::base_class<type>(this), name_, orientation_);
    }

private:
    std::string name_ = "new receiver";
    core::orientable orientation_;
};

////////////////////////////////////////////////////////////////////////////////

class receivers final : public owning_member<receivers, vector<receiver, 1>> {
public:
    explicit receivers(const core::geo::box& aabb);

    const receiver& operator[](size_t index) const;
    receiver& operator[](size_t index);

    auto cbegin() const { return data().cbegin(); }
    auto begin() const { return data().begin(); }
    auto begin() { return data().begin(); }

    auto cend() const { return data().cend(); }
    auto end() const { return data().end(); }
    auto end() { return data().end(); }

    template <typename It>
    void insert(It it) {
        data().insert(std::move(it), receiver{aabb_});
    }

    template <typename It>
    void erase(It it) {
        data().erase(std::move(it));
    }

    size_t size() const;
    bool empty() const;

    void clear();

    bool can_erase() const;

private:
    vector<receiver, 1>& data();
    const vector<receiver, 1>& data() const;

    core::geo::box aabb_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
