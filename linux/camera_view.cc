#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gst/video/videoframe.h>
#include <gtk/gtk.h>
#include <flutter_linux/flutter_linux.h>

struct _CameraView {
  GtkWidget parent_instance;
  GstElement *pipeline;
  GstElement *videosink;
  GstElement *appsink;
  GstElement *videoconvert;
  GstElement *capsfilter;
  int64_t texture_id;
  FlutterView *flutter_view;
  FlMethodChannel *channel;
};

G_DEFINE_TYPE(CameraView, camera_view, GTK_TYPE_WIDGET)

static void camera_view_dispose(GObject *object) {
  CameraView *self = CAMERA_VIEW(object);
  
  if (self->pipeline) {
    gst_element_set_state(self->pipeline, GST_STATE_NULL);
    gst_object_unref(self->pipeline);
  }
  
  if (self->channel) {
    g_object_unref(self->channel);
  }
  
  G_OBJECT_CLASS(camera_view_parent_class)->dispose(object);
}

static void camera_view_class_init(CameraViewClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = camera_view_dispose;
}

static GstFlowReturn new_sample_callback(GstElement *sink, gpointer data) {
  CameraView *self = CAMERA_VIEW(data);
  GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
  
  if (sample) {
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
      // Create a texture frame
      FlutterTextureFrame *frame = flutter_texture_frame_new(
        self->texture_id,
        map.data,
        map.size,
        gst_buffer_get_width(buffer),
        gst_buffer_get_height(buffer)
      );
      
      // Update the texture
      flutter_view_update_texture(self->flutter_view, frame);
      
      gst_buffer_unmap(buffer, &map);
      flutter_texture_frame_free(frame);
    }
    
    gst_sample_unref(sample);
  }
  
  return GST_FLOW_OK;
}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call, gpointer user_data) {
  CameraView *self = CAMERA_VIEW(user_data);
  const gchar *method = fl_method_call_get_name(method_call);
  FlMethodResponse *response = NULL;
  
  if (strcmp(method, "initialize") == 0) {
    // Create a new texture
    self->texture_id = flutter_view_create_texture(self->flutter_view);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(NULL));
  } else if (strcmp(method, "dispose") == 0) {
    // Clean up texture
    if (self->texture_id != -1) {
      flutter_view_destroy_texture(self->flutter_view, self->texture_id);
      self->texture_id = -1;
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(NULL));
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }
  
  fl_method_call_respond(method_call, response, NULL);
  g_object_unref(response);
}

static void camera_view_init(CameraView *self) {
  // Initialize GStreamer pipeline
  self->pipeline = gst_pipeline_new("camera-pipeline");
  GstElement *v4l2src = gst_element_factory_make("v4l2src", "camera-source");
  self->capsfilter = gst_element_factory_make("capsfilter", "caps-filter");
  self->videoconvert = gst_element_factory_make("videoconvert", "video-convert");
  self->appsink = gst_element_factory_make("appsink", "app-sink");
  
  // Set camera resolution and framerate
  GstCaps *caps = gst_caps_new_simple("video/x-raw",
      "width", G_TYPE_INT, 640,
      "height", G_TYPE_INT, 480,
      "framerate", GST_TYPE_FRACTION, 30, 1,
      NULL);
  g_object_set(G_OBJECT(self->capsfilter), "caps", caps, NULL);
  gst_caps_unref(caps);
  
  // Configure appsink
  g_object_set(G_OBJECT(self->appsink), "emit-signals", TRUE, NULL);
  g_signal_connect(self->appsink, "new-sample", G_CALLBACK(new_sample_callback), self);
  
  // Add elements to pipeline
  gst_bin_add_many(GST_BIN(self->pipeline),
      v4l2src, self->capsfilter, self->videoconvert, self->appsink, NULL);
  
  // Link elements
  gst_element_link_many(v4l2src, self->capsfilter, self->videoconvert, self->appsink, NULL);
  
  // Start pipeline
  gst_element_set_state(self->pipeline, GST_STATE_PLAYING);
}

void camera_view_register_with_registrar(FlPluginRegistrar* registrar) {
  CameraView* view = camera_view_new();
  g_autoptr(FlStandardMessageCodec) codec = fl_standard_message_codec_new();
  g_autoptr(FlMethodChannel) channel = fl_method_channel_new(
      fl_plugin_registrar_get_messenger(registrar),
      "camera_view",
      FL_MESSAGE_CODEC(codec));
  
  fl_method_channel_set_method_call_handler(channel, method_call_cb, g_object_ref(view), g_object_unref);
  
  fl_plugin_registrar_register_view_factory(registrar, "CameraView", FL_VIEW_FACTORY(view));
}

GtkWidget *camera_view_new() {
  return g_object_new(CAMERA_VIEW_TYPE, NULL);
} 