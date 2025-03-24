#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include "camera_view.h"
#include "generated_plugin_registrant.h"

int main(int argc, char** argv) {
  g_autoptr(MyApplication) app = my_application_new();
  
  // Initialize GStreamer
  gst_init(&argc, &argv);
  
  // Register camera view plugin
  camera_view_register_with_registrar(flutter_registrar_get_plugin_registrar(FLUTTER_REGISTRAR(app), "CameraView"));
  
  return g_application_run(G_APPLICATION(app), argc, argv);
}
