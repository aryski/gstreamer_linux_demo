#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gtk/gtk.h>
#include <flutter_linux/flutter_linux.h>

struct _CameraView {
  GtkWidget parent_instance;
  GstElement *pipeline;
  GstElement *videosink;
  GtkWidget *container;
};

G_DEFINE_TYPE(CameraView, camera_view, GTK_TYPE_WIDGET)

static void camera_view_dispose(GObject *object) {
  CameraView *self = CAMERA_VIEW(object);
  
  if (self->pipeline) {
    gst_element_set_state(self->pipeline, GST_STATE_NULL);
    gst_object_unref(self->pipeline);
  }
  
  G_OBJECT_CLASS(camera_view_parent_class)->dispose(object);
}

static void camera_view_class_init(CameraViewClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = camera_view_dispose;
}

static void camera_view_init(CameraView *self) {
  self->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_parent(self->container, GTK_WIDGET(self));
  
  // Create GStreamer pipeline
  self->pipeline = gst_pipeline_new("camera-pipeline");
  GstElement *v4l2src = gst_element_factory_make("v4l2src", "camera-source");
  GstElement *capsfilter = gst_element_factory_make("capsfilter", "caps-filter");
  GstElement *videoconvert = gst_element_factory_make("videoconvert", "video-convert");
  self->videosink = gst_element_factory_make("gtk4paintablesink", "video-sink");
  
  // Set camera resolution and framerate
  GstCaps *caps = gst_caps_new_simple("video/x-raw",
      "width", G_TYPE_INT, 640,
      "height", G_TYPE_INT, 480,
      "framerate", GST_TYPE_FRACTION, 30, 1,
      NULL);
  g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
  gst_caps_unref(caps);
  
  // Add elements to pipeline
  gst_bin_add_many(GST_BIN(self->pipeline),
      v4l2src, capsfilter, videoconvert, self->videosink, NULL);
  
  // Link elements
  gst_element_link_many(v4l2src, capsfilter, videoconvert, self->videosink, NULL);
  
  // Set video sink widget
  g_object_set(G_OBJECT(self->videosink), "widget", self->container, NULL);
  
  // Start pipeline
  gst_element_set_state(self->pipeline, GST_STATE_PLAYING);
}

GtkWidget *camera_view_new() {
  return g_object_new(CAMERA_VIEW_TYPE, NULL);
} 