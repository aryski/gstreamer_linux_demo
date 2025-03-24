#include "generated_plugin_registrant.h"
#include <flutter_linux/flutter_linux.h>
#include <flutter_registrar.h>
#include <url_launcher_linux/url_launcher_plugin.h>
#include "camera_view.h"

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) url_launcher_linux_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "UrlLauncherPlugin");
  url_launcher_plugin_register_with_registrar(url_launcher_linux_registrar);
  
  // Register our camera view plugin
  g_autoptr(FlPluginRegistrar) camera_view_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "CameraView");
  camera_view_register_with_registrar(camera_view_registrar);
} 