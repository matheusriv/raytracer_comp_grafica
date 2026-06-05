#include "integrator.hpp"
#include "../core/surfel.hpp"
#include "../core/cel_material.hpp"
#include "../core/App.hpp"
#include "../core/light.hpp"
#include <algorithm>
#include <cmath>

namespace ryt {

ToonIntegrator::ToonIntegrator(std::shared_ptr<Camera> cam, const std::vector<real_type>& intervals_deg)
    : SamplerIntegrator(std::move(cam)) {
    // Convert angles from degrees to radians and store them
    for (real_type angle_deg : intervals_deg) {
        mapping_intervals_rad.push_back(angle_deg * M_PI / 180.0f);
    }
    // Sort to ensure the intervals are ordered correctly
    std::sort(mapping_intervals_rad.begin(), mapping_intervals_rad.end());
}

std::optional<RGBColor> ToonIntegrator::Li(const Rayf& ray, const Scene& scene, int depth) const {
    Surfel sf;
    if (!scene.intersect(ray, &sf)) {
        return std::nullopt;
    }

    const Material* mat = sf.primitive->get_material();
    const CelMaterial* cel_mat = dynamic_cast<const CelMaterial*>(mat);

    if (!cel_mat) {
        // If it's not a 'cel' material, return the material's base color
        if (mat) return mat->color();
        return color_black;
    }

    Vector3f n = normalize(sf.n);
    Vector3f v = normalize(-ray.d); // View vector towards the camera

    // Silhouette check
    real_type cos_nv = dot(n, v);
    if (cos_nv < 0.0f) {
        n = -n; // Ensure the normal points towards the camera
        cos_nv = dot(n, v);
    }
    
    real_type angle_nv = std::acos(cos_nv);
    if (angle_nv > cel_mat->silhouette_angle_rad) {
        return cel_mat->silhouette_color;
    }

    RGBColor L{0.0f, 0.0f, 0.0f};
    bool is_lit = false;

    // Contribution of each light source
    for (const auto& light : App::m_render_options->lights) {
        Vector3f l;
        VisibilityTester vis;
        RGBColor I = light->sample_Li(sf, &l, &vis);
        l = normalize(l);

        if (vis.unoccluded(scene)) {
            is_lit = true;
            real_type cos_nl = std::max(0.0f, dot(n, l));
            real_type angle_nl = std::acos(cos_nl);

            // Find which interval the angle falls into
            size_t interval_idx = 0;
            for (size_t i = 0; i < mapping_intervals_rad.size(); ++i) {
                if (angle_nl >= mapping_intervals_rad[i]) {
                    interval_idx = i + 1;
                } else {
                    break;
                }
            }

            // Get the corresponding color from the color map
            RGBColor cel_color;
            size_t num_colors = cel_mat->color_map.size();
            if (num_colors == 0) {
                cel_color = color_white; // Default to white if there is no map
            } else {
                size_t num_intervals = mapping_intervals_rad.size() + 1;
                
                // level=0 is the darkest region, level=num_intervals-1 is the lightest
                size_t level = (num_intervals - 1) - interval_idx;
                
                // color_map[0] is the darkest, color_map[num_colors-1] is the lightest
                // Map the level to the color map index, handling mismatch cases
                size_t color_idx = std::min(level, num_colors - 1);
                
                cel_color = cel_mat->color_map[color_idx];
            }

            L = L + RGBColor{I.r * cel_color.r, I.g * cel_color.g, I.b * cel_color.b};
        }
    }

    if (!is_lit && !App::m_render_options->lights.empty()) {
        // If in shadow from all lights, use the shadow color
        L = cel_mat->shadow_color;
    }

    // Ambient light contribution
    if (App::m_render_options->ambient_light) {
        Vector3f wi;
        L = L + App::m_render_options->ambient_light->sample_Li(sf, &wi, nullptr);
    }

    return L;
}

} // namespace ryt