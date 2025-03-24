#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CAMERA_VIEW_TYPE (camera_view_get_type())
G_DECLARE_FINAL_TYPE(CameraView, camera_view, CAMERA, VIEW, GtkWidget)

GtkWidget *camera_view_new(void);

G_END_DECLS

#endif // CAMERA_VIEW_H 