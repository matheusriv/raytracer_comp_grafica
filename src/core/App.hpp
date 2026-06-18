#ifndef APP_HPP
#define APP_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <stack>

#include "common.hpp"
#include "paramset.hpp"
#include "background.hpp"
#include "material.hpp"
#include "primitive.hpp"
#include "scene.hpp"
#include "../integrators/integrator.hpp"
#include "light.hpp"
#include "transform.hpp"

// Type of map we want to use.
#define Dictionary std::unordered_map

namespace ryt {
  
class Film;
class Camera;


struct GraphicsState {
  std::shared_ptr<Material> current_material;
  Dictionary<std::string, std::shared_ptr<Material>> named_materials;
  bool flip_normals{false};
  bool mats_lib_cloned{false};
};

struct RenderOptions {
  /// This is a map of attribute names (keys) and paramset (values).
  Dictionary<std::string, ryt::ParamSet> actors;
  /// Background object
  std::unique_ptr<Background> background;
  /// Camera object, owner of the film.
  std::shared_ptr<Camera> camera;
  /// Scene object
  std::unique_ptr<Scene> scene;
  /// Integrator object
  std::unique_ptr<Integrator> integrator;
  /// List of primitives (objects) in the scene.
  std::vector<std::shared_ptr<Primitive>> primitives;
  /// The most recently defined material (applied to subsequent objects).
  std::shared_ptr<Material> current_material;
  /// Named materials library
  Dictionary<std::string, std::shared_ptr<Material>> named_materials;
  /// List of light sources in the scene.
  std::vector<std::shared_ptr<Light>> lights;
  /// Ambient light for the scene.
  std::shared_ptr<Light> ambient_light;
};

/*!
 * Application class that represents our graphics library (GL) or API.
 *
 * This GL works in **retained mode**.
 * In this mode the client calls do not directly cause actual rendering.
 * Instead, it update an abstract internal model (typically a list of objects, camera, lights, etc.)
 * which is maintained within the library's data space. This allows the library to optimize when
 * actual rendering takes place along with the processing of related objects.
 *
 * Therefore, the client calls into the graphics library do not directly cause actual rendering, but
 * make use of extensive indirection to resources, managed – thus retained – by the graphics
 * library.
 *
 * Only after the whole scene file is processed the GL triggers the actual rendering.
 */

class App {
public:
  /// Defines the current state the API may be at a given time
  enum class AppState : std::uint8_t {
    Uninitialized,  //!< Initial state, before parsing.
    SetupBlock,     //!< Parsing render setup.
    WorldBlock
  };  //!< Parsing the world's information.

  // == Public interface
  static void render();
  static void init_engine(const RunningOptions&);
  static void run();
  static void clean_up();
  static void soft_engine_reset();
  static void hard_engine_reset();
  static void reset_engine();

  // == API functions

  static bool check_in_initialized_state(std::string_view func_name);
  static bool check_in_setup_block_state(std::string_view func_name);
  static bool check_in_world_block_state(std::string_view func_name);

  static void camera(const ParamSet& ps);
  static void look_at(const ParamSet& ps);
  static void background(const ParamSet& ps);
  static void material(const ParamSet& ps);
  static void make_named_material(const ParamSet& ps);
  static void named_material(const ParamSet& ps);
  static void integrator(const ParamSet& ps);
  static void object(const ParamSet& ps);
  static void aggregator(const ParamSet& ps);
  static void light_source(const ParamSet& ps);
  static void world_begin(const ParamSet& ps);
  static void world_end(const ParamSet& ps);
  static void film(const ParamSet& ps);
  static Camera* make_camera(const ParamSet& camera_ps, const ParamSet& film_ps);
  static void identity(const ParamSet& ps);
  static void translate(const ParamSet& ps);
  static void scale(const ParamSet& ps);
  static void rotate(const ParamSet& ps);
  static void push_gs(const ParamSet& ps);
  static void pop_gs(const ParamSet& ps);
  static void push_ctm(const ParamSet& ps);
  static void pop_ctm(const ParamSet& ps);
  static void save_coord_system(const ParamSet& ps);
  static void restore_coord_system(const ParamSet& ps);
  static void object_instance_begin(const ParamSet& ps);
  static void object_instance_end(const ParamSet& ps);
  static void object_instance_call(const ParamSet& ps);
  static const Transform* get_cached_transform(const Transform& t);

  /// Stores the running options passed to the main().
  static RunningOptions m_current_run_options;
  /// The infrastructure to render a scene (camera, integrator, etc.).
  static std::unique_ptr<RenderOptions> m_render_options;
    /// Current API state
  static AppState m_current_block_state;
 
  static Transform m_current_CTM;
  static std::stack<Transform> m_CTM_stack;
 
  static GraphicsState m_current_GS;
  static std::stack<GraphicsState> m_GS_stack;
 
  static Dictionary<std::string, Transform> m_saved_coord_systems;
  static std::vector<std::unique_ptr<Transform>> m_transform_cache;

  static std::string m_current_instance_name;
  static std::vector<std::shared_ptr<Primitive>> m_current_instance_primitives;
  static Dictionary<std::string, std::shared_ptr<Primitive>> m_instances;
};

} //namespace

#endif  // APP_HPP
