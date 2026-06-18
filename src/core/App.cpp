#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string_view>

#include "../../msg_system/error.hpp"

#include "App.hpp"
#include "common.hpp"
#include "camera.hpp"
#include "film.hpp"
#include "background.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "paramset.hpp"
#include "parser.hpp"
#include "scene.hpp"
#include "blinn_material.hpp"
#include "cel_material.hpp"
#include "light.hpp"

namespace ryt{ 

//=== App's static members declaration and initialization.
App::AppState App::m_current_block_state = AppState::Uninitialized;
RunningOptions App::m_current_run_options;
std::unique_ptr<RenderOptions> App::m_render_options;
Transform App::m_current_CTM;
std::stack<Transform> App::m_CTM_stack;
GraphicsState App::m_current_GS;
std::stack<GraphicsState> App::m_GS_stack;
Dictionary<std::string, Transform> App::m_saved_coord_systems;
std::vector<std::unique_ptr<Transform>> App::m_transform_cache;
std::string App::m_current_instance_name;
std::vector<std::shared_ptr<Primitive>> App::m_current_instance_primitives;
Dictionary<std::string, std::shared_ptr<Primitive>> App::m_instances;

/// Check whether the current state has been intialized.
bool App::check_in_initialized_state(std::string_view func_name) {
  if (m_current_block_state == AppState::Uninitialized) {
    std::ostringstream oss;
    oss << "App::init() must be called before " << func_name << ". Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

/// Check whether the current state corresponds to setup section.
bool App::check_in_setup_block_state(std::string_view func_name) {
  check_in_initialized_state(func_name);
  if (m_current_block_state == AppState::WorldBlock) {
    std::ostringstream oss;
    oss << "Rendering setup cannot happen inside World Definition block; ";
    oss << func_name << " not allowed. Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

/// Check whether the current state corresponds to the world section.
bool App::check_in_world_block_state(std::string_view func_name) {
  check_in_initialized_state(func_name);
  if (m_current_block_state == AppState::SetupBlock) {
    std::ostringstream oss;
    oss << "Scene description must happen inside World Definition block; ";
    oss << func_name << " not allowed. Ignoring...";
    ERROR(oss.str());
    return false;
  }
  return true;
}

void App::film(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::film()")) {
    return;
  }
  // Store the parameters associated with the film for later camera creation.
  m_render_options->actors["film"] = ps;
  if(m_current_run_options.verbose){
    std::cout << ">>> Film parameters:\n"
            << "    - type: " << ps.retrieve<std::string>("type", "") << "\n"
            << "    - filename: " << ps.retrieve<std::string>("filename", "output") << "\n"
            << "    - img_type: " << ps.retrieve<std::string>("img_type", "png") << "\n"
            << "    - w_res/x_res: " << ps.retrieve<int>("w_res", ps.retrieve<int>("x_res", 1280)) << "\n"
            << "    - h_res/y_res: " << ps.retrieve<int>("h_res", ps.retrieve<int>("y_res", 720)) << "\n\n";
  }
}

void App::camera(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::camera()")) {
    return;
  }
  m_render_options->actors["camera"] = ps;
  if (m_current_run_options.verbose) {
    auto type = ps.retrieve<std::string>("type", "perspective");
    std::cout << ">>> Camera type: " << std::quoted(type) << "\n\n";
  }
}

void App::look_at(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::look_at()")) {
    return;
  }
  m_render_options->actors["lookat"] = ps;
}

void App::background(const ParamSet& ps) {
  check_in_world_block_state("App::background");

  auto type = ps.retrieve<std::string>("type", "single_color");
  if (m_current_run_options.verbose) {
    std::cout << ">>> Background parameters:\n"
            << "    - type: " << type << "\n";
    if (type == "single_color") {
      auto color = ps.retrieve<RGBColor>("color", color_black);
      std::cout << "    - color: " << color << "\n";
    } else if (type == "4_colors" || type == "colors") {
      auto bl = ps.retrieve<RGBColor>("bl", color_black);
      auto tl = ps.retrieve<RGBColor>("tl", color_black);
      auto tr = ps.retrieve<RGBColor>("tr", color_black);
      auto br = ps.retrieve<RGBColor>("br", color_black);
      std::cout << "    - bl: " << bl << "\n"
                << "    - tl: " << tl << "\n"
                << "    - tr: " << tr << "\n"
                << "    - br: " << br << "\n";
    }
    std::cout << "\n";
  }
  
  Background* bkg = create_color_background(type, ps);
  // Store current background object.
  m_render_options->background.reset(bkg);
}

void App::material(const ParamSet& ps) {
  check_in_world_block_state("App::material()");
  auto type = ps.retrieve<std::string>("type", "flat");
  if (type == "flat") {
    auto color = ps.retrieve<RGBColor>("color", RGBColor{ 0.0f, 0.0f, 1.0f });
    if (color.r > 1.0f || color.g > 1.0f || color.b > 1.0f) {
      color = color / 255.0f;
    }

    m_render_options->current_material = std::make_shared<FlatMaterial>(color);
  } else if (type == "blinn") {
    auto ka = ps.retrieve<RGBColor>("ambient", RGBColor{0.1f, 0.1f, 0.1f});
    auto kd = ps.retrieve<RGBColor>("diffuse", RGBColor{0.5f, 0.5f, 0.5f});
    auto ks = ps.retrieve<RGBColor>("specular", RGBColor{1.0f, 1.0f, 1.0f});
    auto g = ps.retrieve<real_type>("glossiness", 256.0f);
    auto mirror = ps.retrieve<RGBColor>("mirror", color_black);

    if (ka.r > 1.0f || ka.g > 1.0f || ka.b > 1.0f) ka = ka / 255.0f;
    if (kd.r > 1.0f || kd.g > 1.0f || kd.b > 1.0f) kd = kd / 255.0f;
    if (ks.r > 1.0f || ks.g > 1.0f || ks.b > 1.0f) ks = ks / 255.0f;
    if (mirror.r > 1.0f || mirror.g > 1.0f || mirror.b > 1.0f) mirror = mirror / 255.0f;

    m_render_options->current_material = std::make_shared<BlinnPhongMaterial>(ka, kd, ks, g, mirror);
  } else if (type == "cel") {
    auto color_map_vec = ps.retrieve<std::vector<RGBColor>>("color_map", {});
    auto shadow_color = ps.retrieve<RGBColor>("shadow_color", color_black);
    auto silhouette_color = ps.retrieve<RGBColor>("silhouette_color", color_black);
    auto silhouette_angle = ps.retrieve<real_type>("silhouette_angle", 85.0f);

    std::vector<RGBColor> color_map;
    for(auto c : color_map_vec) {
        if (c.r > 1.0f || c.g > 1.0f || c.b > 1.0f) c = c / 255.0f;
        color_map.push_back(c);
    }
    if (shadow_color.r > 1.0f || shadow_color.g > 1.0f || shadow_color.b > 1.0f) shadow_color = shadow_color / 255.0f;
    if (silhouette_color.r > 1.0f || silhouette_color.g > 1.0f || silhouette_color.b > 1.0f) silhouette_color = silhouette_color / 255.0f;

    m_render_options->current_material = std::make_shared<CelMaterial>(color_map, shadow_color, silhouette_color, silhouette_angle);
  }
}

void App::light_source(const ParamSet& ps) {
  check_in_world_block_state("App::light_source()");
  auto type = ps.retrieve<std::string>("type", "point");
  auto I = ps.retrieve<RGBColor>("i", RGBColor{-1.0f, -1.0f, -1.0f});
  if (I.r < 0.0f) {
    I = ps.retrieve<RGBColor>("l", RGBColor{1.0f, 1.0f, 1.0f});
  }
  auto scale = ps.retrieve<Vector3f>("scale", Vector3f{1.0f, 1.0f, 1.0f});
  
  RGBColor final_I = RGBColor{I.r * scale.x, I.g * scale.y, I.b * scale.z};

  if (type == "ambient") {
    m_render_options->ambient_light = std::make_shared<AmbientLight>(final_I);
  } else if (type == "directional") {
    auto from = ps.retrieve<Point3f>("from", Point3f{0.0f, 0.0f, 0.0f});
    auto to = ps.retrieve<Point3f>("to", Point3f{0.0f, 0.0f, -1.0f});
    // Direction mapped backwards towards the light source for lighting ray calculations
    m_render_options->lights.push_back(std::make_shared<DirectionalLight>(normalize(from - to), final_I));
  } else if (type == "point") {
    auto from = ps.retrieve<Point3f>("from", Point3f{0.0f, 0.0f, 0.0f});
    m_render_options->lights.push_back(std::make_shared<PointLight>(from, final_I));
  } else if (type == "spot") {
    auto from = ps.retrieve<Point3f>("from", Point3f{0.0f, 0.0f, 0.0f});
    auto to = ps.retrieve<Point3f>("to", Point3f{0.0f, 0.0f, -1.0f});
    auto cutoff = ps.retrieve<real_type>("cutoff", 30.0f);
    auto falloff = ps.retrieve<real_type>("falloff", 15.0f);
    m_render_options->lights.push_back(std::make_shared<SpotLight>(from, to, final_I, cutoff, falloff));
  }
}

void App::make_named_material(const ParamSet& ps) {
  check_in_world_block_state("App::make_named_material()");
  auto type = ps.retrieve<std::string>("type", "flat");
  auto name = ps.retrieve<std::string>("name", "");
  if (name.empty()) {
    WARNING("make_named_material missing name.");
    return;
  }
  
  if (type == "flat") {
    auto color = ps.retrieve<RGBColor>("color", RGBColor{ 0.0f, 0.0f, 1.0f });
    if (color.r > 1.0f || color.g > 1.0f || color.b > 1.0f) {
      color = color / 255.0f;
    }
    m_render_options->named_materials[name] = std::make_shared<FlatMaterial>(color);
  } else if (type == "blinn") {
    auto ka = ps.retrieve<RGBColor>("ambient", RGBColor{0.1f, 0.1f, 0.1f});
    auto kd = ps.retrieve<RGBColor>("diffuse", RGBColor{0.5f, 0.5f, 0.5f});
    auto ks = ps.retrieve<RGBColor>("specular", RGBColor{1.0f, 1.0f, 1.0f});
    auto g = ps.retrieve<real_type>("glossiness", 256.0f);
    auto mirror = ps.retrieve<RGBColor>("mirror", color_black);

    if (ka.r > 1.0f || ka.g > 1.0f || ka.b > 1.0f) ka = ka / 255.0f;
    if (kd.r > 1.0f || kd.g > 1.0f || kd.b > 1.0f) kd = kd / 255.0f;
    if (ks.r > 1.0f || ks.g > 1.0f || ks.b > 1.0f) ks = ks / 255.0f;
    if (mirror.r > 1.0f || mirror.g > 1.0f || mirror.b > 1.0f) mirror = mirror / 255.0f;

    m_render_options->named_materials[name] = std::make_shared<BlinnPhongMaterial>(ka, kd, ks, g, mirror);
  } else if (type == "cel") {
    auto color_map_vec = ps.retrieve<std::vector<RGBColor>>("color_map", {});
    auto shadow_color = ps.retrieve<RGBColor>("shadow_color", color_black);
    auto silhouette_color = ps.retrieve<RGBColor>("silhouette_color", color_black);
    auto silhouette_angle = ps.retrieve<real_type>("silhouette_angle", 85.0f);

    std::vector<RGBColor> color_map;
    for(auto c : color_map_vec) {
        if (c.r > 1.0f || c.g > 1.0f || c.b > 1.0f) c = c / 255.0f;
        color_map.push_back(c);
    }
    if (shadow_color.r > 1.0f || shadow_color.g > 1.0f || shadow_color.b > 1.0f) shadow_color = shadow_color / 255.0f;
    if (silhouette_color.r > 1.0f || silhouette_color.g > 1.0f || silhouette_color.b > 1.0f) silhouette_color = silhouette_color / 255.0f;

    m_render_options->named_materials[name] = std::make_shared<CelMaterial>(color_map, shadow_color, silhouette_color, silhouette_angle);
  }
}

void App::named_material(const ParamSet& ps) {
  check_in_world_block_state("App::named_material()");
  auto name = ps.retrieve<std::string>("name", "");
  auto it = m_render_options->named_materials.find(name);
  if (it != m_render_options->named_materials.end()) {
    m_render_options->current_material = it->second;
  } else {
    WARNING("named_material not found: " + name);
  }
}

void App::integrator(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::integrator()")) {
    return;
  }
  m_render_options->actors["integrator"] = ps;
}

void App::object(const ParamSet& ps) {
  check_in_world_block_state("App::object()");
  auto type = ps.retrieve<std::string>("type", "");

  std::shared_ptr<Material> mat = m_render_options->current_material;
  auto mat_name = ps.retrieve<std::string>("material", "");
  if (!mat_name.empty()) {
    auto it = m_render_options->named_materials.find(mat_name);
    if (it != m_render_options->named_materials.end()) {
      mat = it->second;
    } else {
      WARNING("Material " + mat_name + " not found for object. Using current material.");
    }
  }

  const Transform* o2w = nullptr;
  const Transform* w2o = nullptr;
  if (m_current_CTM != Transform()) {
    o2w = get_cached_transform(m_current_CTM);
    w2o = get_cached_transform(m_current_CTM.inverse());
  }

  if (type == "sphere") {
    Shape* shape = create_sphere(ps, o2w, w2o);
    GeometricPrimitive* prim = new GeometricPrimitive(std::shared_ptr<Shape>(shape), mat);
    if (!m_current_instance_name.empty()) {
      m_current_instance_primitives.push_back(std::shared_ptr<Primitive>(prim));
    } else {
      m_render_options->primitives.push_back(std::shared_ptr<Primitive>(prim));
    }
  } else if (type == "trianglemesh") {
    auto shapes = create_triangle_mesh_shape(ps, o2w);
    for (auto& shape : shapes) {
      auto prim = std::make_shared<GeometricPrimitive>(shape, mat);
      if (!m_current_instance_name.empty()) {
        m_current_instance_primitives.push_back(prim);
      } else {
        m_render_options->primitives.push_back(prim);
      }
    }
  }
}

void App::aggregator(const ParamSet& ps) {
  check_in_setup_block_state("App::aggregator()");
  m_render_options->actors["aggregator"] = ps;
  if (m_current_run_options.verbose) {
    auto type = ps.retrieve<std::string>("type", "bvh");
    std::cout << ">>> Aggregator type: " << std::quoted(type) << "\n\n";
  }
}

void App::world_begin(const ParamSet& ps) {
  check_in_setup_block_state("App::world_begin()");
  m_current_block_state = AppState::WorldBlock;  // correct machine state.
  hard_engine_reset();
  
  m_current_CTM = Transform();
  while (!m_CTM_stack.empty()) m_CTM_stack.pop();
  
  m_current_GS = GraphicsState();
  m_current_GS.current_material = m_render_options->current_material;
  m_current_GS.named_materials = m_render_options->named_materials;
  while (!m_GS_stack.empty()) m_GS_stack.pop();

  m_saved_coord_systems.clear();
  m_transform_cache.clear();
  m_current_instance_name.clear();
  m_current_instance_primitives.clear();
  m_instances.clear();
}

void App::push_gs(const ParamSet& ps) {
  check_in_world_block_state("App::push_gs()");
  m_current_GS.current_material = m_render_options->current_material;
  m_GS_stack.push(m_current_GS);
  m_CTM_stack.push(m_current_CTM);
}

void App::pop_gs(const ParamSet& ps) {
  check_in_world_block_state("App::pop_gs()");
  if (!m_GS_stack.empty()) {
    m_current_GS = m_GS_stack.top();
    m_GS_stack.pop();
    m_render_options->current_material = m_current_GS.current_material;
  } else {
    WARNING("pop_gs called with empty stack.");
  }

  if (!m_CTM_stack.empty()) {
    m_current_CTM = m_CTM_stack.top();
    m_CTM_stack.pop();
  } else {
    WARNING("pop_gs called with empty CTM stack.");
  }
}

void App::push_ctm(const ParamSet& ps) {
  check_in_world_block_state("App::push_ctm()");
  m_CTM_stack.push(m_current_CTM);
}

void App::pop_ctm(const ParamSet& ps) {
  check_in_world_block_state("App::pop_ctm()");
  if (!m_CTM_stack.empty()) {
    m_current_CTM = m_CTM_stack.top();
    m_CTM_stack.pop();
  } else {
    WARNING("pop_ctm called with empty stack.");
  }
}

void App::identity(const ParamSet& ps) {
  check_in_world_block_state("App::identity()");
  m_current_CTM = Transform();
}

void App::translate(const ParamSet& ps) {
  check_in_world_block_state("App::translate()");
  Vector3f v = ps.retrieve<Vector3f>("value", Vector3f(0,0,0));
  m_current_CTM = m_current_CTM * ryt::translate(v);
}

void App::scale(const ParamSet& ps) {
  check_in_world_block_state("App::scale()");
  Vector3f v = ps.retrieve<Vector3f>("value", Vector3f(1,1,1));
  m_current_CTM = m_current_CTM * ryt::scale(v.x, v.y, v.z);
}

void App::rotate(const ParamSet& ps) {
  check_in_world_block_state("App::rotate()");
  real_type angle = ps.retrieve<real_type>("angle", 0.0f);
  Vector3f axis = ps.retrieve<Vector3f>("axis", Vector3f(0,1,0));
  m_current_CTM = m_current_CTM * ryt::rotate(angle, axis);
}

void App::save_coord_system(const ParamSet& ps) {
  check_in_world_block_state("App::save_coord_system()");
  std::string name = ps.retrieve<std::string>("name", "");
  if (!name.empty()) {
    m_saved_coord_systems[name] = m_current_CTM;
  }
}

void App::restore_coord_system(const ParamSet& ps) {
  check_in_world_block_state("App::restore_coord_system()");
  std::string name = ps.retrieve<std::string>("name", "");
  if (m_saved_coord_systems.count(name) > 0) {
    m_current_CTM = m_saved_coord_systems[name];
  } else {
    WARNING("restore_coord_system: Coordinate system not found: " + name);
  }
}

const Transform* App::get_cached_transform(const Transform& t) {
  for (const auto& ptr : m_transform_cache) {
    if (*ptr == t) return ptr.get();
  }
  m_transform_cache.push_back(std::make_unique<Transform>(t));
  return m_transform_cache.back().get();
}

void App::object_instance_begin(const ParamSet& ps) {
  check_in_world_block_state("App::object_instance_begin()");
  m_current_instance_name = ps.retrieve<std::string>("name", "");
  m_current_instance_primitives.clear();
}

void App::object_instance_end(const ParamSet& ps) {
  check_in_world_block_state("App::object_instance_end()");
  if (!m_current_instance_name.empty()) {
    auto aggregate = std::make_shared<BVHAccel>(std::move(m_current_instance_primitives));
    m_instances[m_current_instance_name] = aggregate;
    m_current_instance_name.clear();
  }
}

void App::object_instance_call(const ParamSet& ps) {
  check_in_world_block_state("App::object_instance_call()");
  auto name = ps.retrieve<std::string>("name", "");
  if (m_instances.count(name) > 0) {
    std::shared_ptr<Primitive> instance_prim = m_instances[name];
    if (m_current_CTM != Transform()) {
      const Transform* o2w = get_cached_transform(m_current_CTM);
      instance_prim = std::make_shared<TransformedPrimitive>(instance_prim, o2w);
    }
    
    if (!m_current_instance_name.empty()) {
      m_current_instance_primitives.push_back(instance_prim);
    } else {
      m_render_options->primitives.push_back(instance_prim);
    }
  } else {
    WARNING("object_instance_call: Instance not found: " + name);
  }
}
/// Erase temporary engine states so that we may render another scene with the same configuration.
void App::hard_engine_reset() {
  if (m_render_options) {
    m_render_options->primitives.clear();
    m_render_options->lights.clear();
    m_render_options->named_materials.clear();
    m_render_options->current_material.reset();
    m_render_options->ambient_light.reset();
  }
}

Camera* App::make_camera(const ParamSet& camera_ps, const ParamSet& film_ps) {
  auto film = std::unique_ptr<Film>(create_film(film_ps));
  if (film == nullptr) {
    return nullptr;
  }

  return create_camera(std::move(film), camera_ps, m_render_options->actors["lookat"]);
}

static void print_camera_info(const Camera* camera, const std::string& camera_type) {
  std::ostringstream oss;
  oss << ">>> create_" << camera_type << "_camera()::screen_window:\n"
      << "[ " << camera->left() << " " << camera->right() << " "
      << camera->bottom() << " " << camera->top() << " ]\n\n"
      << ">>> The Camera frame is:\n"
      << "    u: " << camera->u() << "\n"
      << "    v: " << camera->v() << "\n"
      << "    w: " << camera->w() << "\n"
      << "  eye: " << camera->eye() << "\n";
  MESSAGE(oss.str());
}

static bool build_scene_and_integrator(RenderOptions* options) {
  std::shared_ptr<Primitive> aggregate;
  if (options->actors.count("aggregator")) {
    const auto& ps = options->actors.at("aggregator");
    auto agg_type = ps.retrieve<std::string>("type", "bvh");
    if (agg_type == "bvh") {
      aggregate = std::make_shared<BVHAccel>(
          std::move(options->primitives),
          ps.retrieve<int>("max_prims_per_node", 4),
          ps.retrieve<std::string>("split_method", "middle"));
    } else if (agg_type != "list") {
      WARNING("Unknown aggregator type \"" + std::string(agg_type) + "\". Falling back to \"list\".");
    }
  }
  if (!aggregate) {
    aggregate = std::make_shared<PrimList>(std::move(options->primitives));
  }
  options->scene = std::make_unique<Scene>(
      std::move(options->background),
      aggregate);
      
  // Create integrator
  options->integrator = std::unique_ptr<Integrator>(
      create_integrator(options->camera, options->actors["integrator"]));

  return options->scene != nullptr && options->integrator != nullptr;
}

void App::world_end(const ParamSet& ps) {
  check_in_world_block_state("App::world_end()");

  auto camera_type = m_render_options->actors["camera"].retrieve<std::string>("type", "perspective");
  Camera* camera = make_camera(m_render_options->actors["camera"], m_render_options->actors["film"]);
  if (camera == nullptr) {
    ERROR("App::setup_camera(): Unable to create camera.");
    return;  // or handle error
  }
  m_render_options->camera.reset(camera);

  print_camera_info(m_render_options->camera.get(), camera_type);

  // The scene has already been parsed and properly set up. It's time to render the scene.
  bool scene_and_integrator_ok = build_scene_and_integrator(m_render_options.get());

  if (scene_and_integrator_ok) {
    MESSAGE("    Parsing scene successfuly done!\n"
            "[3] Starting ray tracing progress.\n"
            "    Ray tracing is usually a slow process, please be patient: \n\n"
            ">>> Rendering started with resolution - width: " 
            + std::to_string(m_render_options->camera->film().get_resolution().x) + ", height: " 
            + std::to_string(m_render_options->camera->film().get_resolution().y) + "\n");
    //================================================================================
    auto start = std::chrono::steady_clock::now();
    m_render_options->integrator->render(*m_render_options->scene);
    auto end = std::chrono::steady_clock::now();
    //================================================================================
    auto diff = end - start;  // Store the time difference between start and end
    auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(diff); // Seconds
    MESSAGE("    Time elapsed: " + std::to_string(diff_sec.count()) + " seconds ("
            + std::to_string(std::chrono::duration<double, std::milli>(diff).count()) + " ms) \n");
  }
  // Reset state to allow multiple scenes per file.
  m_current_block_state = AppState::SetupBlock;
}

void App::init_engine(const RunningOptions& run_options) {
  // Save running option sent from the main().
  m_current_run_options = run_options;
  // Check current machine state.
  if (m_current_block_state != AppState::Uninitialized) {
    ERROR("App::init_engine() has already been called! ");
  }
  // Set proper machine state
  m_current_block_state = AppState::SetupBlock;
  // Preprare render infrastructure for a new scene.
  m_render_options = std::make_unique<RenderOptions>();
  MESSAGE("[1] Rendering engine initiated.\n");
}

void App::run() {
  // Try to load and parse the scene from a file.
  MESSAGE("[2] Beginning scene file parsing...\n");
  // Recall that the file name comes from the running option struct.
  parse_scene_file(m_current_run_options.filename.c_str());
}


} // namespace 