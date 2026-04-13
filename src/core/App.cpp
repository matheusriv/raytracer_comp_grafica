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
#include "paramset.hpp"
#include "parser.hpp"
#include "scene.hpp"

namespace ryt{ 

//=== App's static members declaration and initialization.
App::AppState App::m_current_block_state = AppState::Uninitialized;
RunningOptions App::m_current_run_options;
std::unique_ptr<RenderOptions> App::m_render_options;

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
  if (m_current_run_options.verbose) {
    auto type = ps.retrieve<std::string>("type", "unknown");
    std::cout << ">>> film type: " << std::quoted(type) << '\n';
  }
}

void App::camera(const ParamSet& ps) {
  if (not check_in_setup_block_state("App::camera()")) {
    return;
  }
  m_render_options->actors["camera"] = ps;
  if (m_current_run_options.verbose) {
    auto type = ps.retrieve<std::string>("type", "perspective");
    std::cout << ">>> camera type: " << std::quoted(type) << '\n';
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

  auto type = ps.retrieve<std::string>("type", "unknown");
  if (type == "unknown") {
    ERROR("API::background(): Missing \"type\" specificaton for the background.");
  }
  
  Background* bkg{ nullptr };
  if (type == "single_color" or type == "4_colors") {
    bkg = create_color_background(type, ps);
  } else {
    WARNING(std::string{ "API::background(): unknown background type \"" } + type
            + std::string{ "\" provided; assuming colored background." });
    bkg = create_color_background(type, ps);
  }
  // Store current background objec.
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
  if (type == "sphere") {
    Sphere* sph = create_sphere(ps, m_render_options->current_material);
    m_render_options->primitives.push_back(std::shared_ptr<Primitive>(sph));
  }
}

void App::world_begin(const ParamSet& ps) {
  check_in_setup_block_state("App::world_begin()");
  m_current_block_state = AppState::WorldBlock;  // correct machine state.
  hard_engine_reset();
}

/// Erase temporary engine states so that we may render another scene with the same configuration.
void App::hard_engine_reset() {
  // Render options reset
  // TODO: in the future.
}

Camera* App::make_camera(const ParamSet& camera_ps, const ParamSet& film_ps) {
  auto film = std::unique_ptr<Film>(create_film(film_ps));
  if (film == nullptr) {
    return nullptr;
  }
  
  if(m_current_run_options.verbose){
    std::cout << ">>> Parâmetros armazenados em film_ps:\n"
            << "    - type: " << film_ps.retrieve<std::string>("type", "") << "\n"
            << "    - filename: " << film_ps.retrieve<std::string>("filename", "output") << "\n"
            << "    - img_type: " << film_ps.retrieve<std::string>("img_type", "png") << "\n"
            << "    - w_res/x_res: " << film_ps.retrieve<int>("w_res", film_ps.retrieve<int>("x_res", 1280)) << "\n"
            << "    - h_res/y_res: " << film_ps.retrieve<int>("h_res", film_ps.retrieve<int>("y_res", 720)) << "\n\n";
  }

  return create_camera(std::move(film), camera_ps, m_render_options->actors["lookat"]);
}

void App::world_end(const ParamSet& ps) {
  check_in_world_block_state("App::world_end()");

  // ===============================================================
  // 1) Create the integrator.
  // 2) Create the scene (requires the list of objects and background)
  // ===============================================================
  auto camera_type = m_render_options->actors["camera"].retrieve<std::string>("type", "perspective");
  Camera* camera = make_camera(m_render_options->actors["camera"],
                                m_render_options->actors["film"]);

  if (camera == nullptr) {
    ERROR("App::setup_camera(): Unable to create camera.");
    return;  // or handle error
  }
  m_render_options->camera.reset(camera);

  std::ostringstream oss;
  oss << ">>> create_" << camera_type << "_camera()::screen_window:";
  MESSAGE(oss.str());
  oss.str(std::string()); oss.clear();
  oss << "[ " << m_render_options->camera->left() << " "
      << m_render_options->camera->right() << " "
      << m_render_options->camera->bottom() << " "
      << m_render_options->camera->top() << " ]";
  MESSAGE(oss.str());
  MESSAGE("");

  MESSAGE(">>> The Camera frame is:");
  oss.str(std::string()); oss.clear();
  oss << "    u: " << m_render_options->camera->u();
  MESSAGE(oss.str());
  oss.str(std::string()); oss.clear();
  oss << "    v: " << m_render_options->camera->v();
  MESSAGE(oss.str());
  oss.str(std::string()); oss.clear();
  oss << "    w: " << m_render_options->camera->w();
  MESSAGE(oss.str());
  oss.str(std::string()); oss.clear();
  oss << "  eye: " << m_render_options->camera->eye();
  MESSAGE(oss.str());

  // The scene has already been parsed and properly set up. It's time to render the scene.
  
  // Create scene (with background and all collected primitives)
  m_render_options->scene = std::make_unique<Scene>(
      std::move(m_render_options->background),
      std::move(m_render_options->primitives));
  // Create integrator
  m_render_options->integrator = std::unique_ptr<Integrator>(
      create_integrator(m_render_options->actors["integrator"]));

  bool scene_and_integrator_ok = m_render_options->scene && m_render_options->integrator;

  if (scene_and_integrator_ok) {
    MESSAGE("    Parsing scene successfuly done!\n");
    MESSAGE("[3] Starting ray tracing progress.\n");
    MESSAGE("    Ray tracing is usually a slow process, please be patient: \n");
    std::cout << ">>> Rendering started with resolution - width: " 
      << m_render_options->camera->film().get_resolution().x << ", height: " 
      << m_render_options->camera->film().get_resolution().y << "\n";
    //================================================================================
    auto start = std::chrono::steady_clock::now();
    m_render_options->integrator->render(*m_render_options->camera, *m_render_options->scene);
    auto end = std::chrono::steady_clock::now();
    //================================================================================
    auto diff = end - start;  // Store the time difference between start and end
    // Seconds
    auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    MESSAGE("    Time elapsed: " + std::to_string(diff_sec.count()) + " seconds ("
            + std::to_string(std::chrono::duration<double, std::milli>(diff).count()) + " ms) \n");
  }
  // Basic clean up, preparing for new rendering, in case we have
  // several scene setup + world in a single input scene file.
  m_current_block_state = AppState::SetupBlock;  // correct machine state.
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
  // Create a new initial GS
  // m_current_gs = GraphicsState();
  MESSAGE("[1] Rendering engine initiated.\n");
}

void App::run() {
  // Try to load and parse the scene from a file.
  MESSAGE("[2] Beginning scene file parsing...\n");
  // Recall that the file name comes from the running option struct.
  parse_scene_file(m_current_run_options.filename.c_str());
}


} // namespace 