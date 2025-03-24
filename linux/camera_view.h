#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H

#include <gtk/gtk.h>
#include <flutter_linux/flutter_linux.h>

G_BEGIN_DECLS

#define CAMERA_VIEW_TYPE (camera_view_get_type())
G_DECLARE_FINAL_TYPE(CameraView, camera_view, CAMERA, VIEW, GtkWidget)

GtkWidget *camera_view_new(void);
void camera_view_register_with_registrar(FlPluginRegistrar* registrar);

G_END_DECLS

#endif // CAMERA_VIEW_H 