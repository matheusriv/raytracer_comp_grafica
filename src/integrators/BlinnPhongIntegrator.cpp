#include "integrator.hpp"
#include "../core/surfel.hpp"
#include "../core/blinn_material.hpp"
#include "../core/App.hpp"
#include "../core/light.hpp"

namespace ryt {

std::optional<RGBColor> BlinnPhongIntegrator::Li(const Rayf& ray, const Scene& scene, int depth) const {
  Surfel sf;
  if (!scene.intersect(ray, &sf)) {
    return std::nullopt;
  }

  Vector3f v = normalize(-ray.d); // View vector towards the camera
  Vector3f n = normalize(sf.n);

  // If hitting from behind the face, reverse the normal
  if (dot(n, v) < 0.0f) {
    n = -n;
  }

  const Material* mat = sf.primitive->get_material();
  const BlinnPhongMaterial* blinn_mat = dynamic_cast<const BlinnPhongMaterial*>(mat);

  RGBColor L{0.0f, 0.0f, 0.0f};

  if (blinn_mat) {
    if (App::m_render_options->ambient_light) {
      Vector3f wi;
      RGBColor ambient_I = App::m_render_options->ambient_light->sample_Li(sf, &wi, nullptr);
      L.r += ambient_I.r * blinn_mat->ka.r;
      L.g += ambient_I.g * blinn_mat->ka.g;
      L.b += ambient_I.b * blinn_mat->ka.b;
    }

    for (const auto& light : App::m_render_options->lights) {
      Vector3f l;
      VisibilityTester vis;
      RGBColor I = light->sample_Li(sf, &l, &vis);

      l = normalize(l);
      float ndotl = std::max(0.0f, dot(n, l));

      if (ndotl > 0.0f && vis.unoccluded(scene)) {
        Vector3f h = normalize(v + l); // Half-vector
        float ndoth = std::max(0.0f, dot(n, h));
        float spec_term = 0.0f;
        if (blinn_mat->glossiness > 0.0f) {
          spec_term = std::pow(ndoth, blinn_mat->glossiness);
        }

        L.r += I.r * (blinn_mat->kd.r * ndotl + blinn_mat->ks.r * spec_term);
        L.g += I.g * (blinn_mat->kd.g * ndotl + blinn_mat->ks.g * spec_term);
        L.b += I.b * (blinn_mat->kd.b * ndotl + blinn_mat->ks.b * spec_term);
      }
    }

    if (blinn_mat->mirror != color_black && depth < max_depth) {
      Vector3f reflected_dir = ray.d - 2.0f * dot(ray.d, n) * n;
      reflected_dir = normalize(reflected_dir);
      Rayf reflected_ray(sf.p + n * 0.05f, reflected_dir); // Offset by an epsilon

      auto L_refl = Li(reflected_ray, scene, depth + 1);
      if (L_refl.has_value()) {
        L.r += blinn_mat->mirror.r * L_refl.value().r;
        L.g += blinn_mat->mirror.g * L_refl.value().g;
        L.b += blinn_mat->mirror.b * L_refl.value().b;
      }
    }
  } else if (mat) {
    L = mat->color();
  }

  return L;
}

} // namespace ryt