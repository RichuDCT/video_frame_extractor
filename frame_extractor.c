#include <gst/gst.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

// Function to extract filename without extension
void get_filename_without_ext(const char* filepath, char* output) {
    char* filename = basename((char*)filepath);
    char* dot = strrchr(filename, '.');
    if (dot) {
        size_t len = dot - filename;
        strncpy(output, filename, len);
        output[len] = '\0';
    } else {
        strcpy(output, filename);
    }
}

int main(int argc, char *argv[]) {
    GstElement *pipeline;
    GError *error = NULL;
    GstBus *bus;
    GstMessage *msg;

    if (argc != 2) {
        g_print("Usage: %s <video-file>\n", argv[0]);
        return 1;
    }

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create output directory
    mkdir("vid_frame_img", 0777);

    // Get video filename without extension
    char filename[256];
    get_filename_without_ext(argv[1], filename);

    // Create pipeline string
    char pipeline_str[1024];
    snprintf(pipeline_str, sizeof(pipeline_str),
        "filesrc location=\"%s\" ! "
        "decodebin ! "
        "videoconvert ! "
        "videoscale ! video/x-raw,width=640,height=480 ! "
        "videorate ! video/x-raw,framerate=5/1 ! "
        "jpegenc quality=85 ! "
        "multifilesink location=\"vid_frame_img/%s_frame-%%04d.jpg\"",
        argv[1], filename);

    // Create pipeline from string
    pipeline = gst_parse_launch(pipeline_str, &error);
    if (error) {
        g_print("Failed to create pipeline: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    // Start playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait until error or EOS
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, 
                                    GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Parse message
    if (msg != NULL) {
        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR: {
                GError *err;
                gchar *debug_info;
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n",
                          GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n",
                          debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            }
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                break;
            default:
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}