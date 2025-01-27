#include <gst/gst.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include <pthread.h>

// Forward declaration of functions
void get_filename_without_ext(const char* filepath, char* output);
void create_directory(const char* path);

typedef struct {
    char *filename;
    int frame_rate;
    int quality;
    gboolean keep_original_size;
    int width;
    int height;
} VideoProcessingParams;

// Function to create directory if it doesn't exist
void create_directory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0777);
    }
}

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

void* process_video(void* params) {
    VideoProcessingParams* vpp = (VideoProcessingParams*)params;
    GstElement *pipeline;
    GError *error = NULL;
    GstBus *bus;
    GstMessage *msg;

    // Create base output directory
    create_directory("vid_frame_img");

    // Get video filename without extension
    char filename[256];
    get_filename_without_ext(vpp->filename, filename);

    // Create video-specific directory path
    char video_dir[512];
    snprintf(video_dir, sizeof(video_dir), "vid_frame_img/%s", filename);
    create_directory(video_dir);

    // Create pipeline string with updated output path
    char pipeline_str[1024];
    if (vpp->keep_original_size) {
        // Pipeline without videoscale when keeping original dimensions
        snprintf(pipeline_str, sizeof(pipeline_str),
            "filesrc location=\"%s\" ! "
            "decodebin ! "
            "videoconvert ! "
            "videorate ! video/x-raw,framerate=%d/1 ! "
            "jpegenc quality=%d ! "
            "multifilesink location=\"%s/%s_%%04d.jpg\"",
            vpp->filename, vpp->frame_rate, vpp->quality, 
            video_dir,filename);
    } else {
        // Original pipeline with scaling
        snprintf(pipeline_str, sizeof(pipeline_str),
            "filesrc location=\"%s\" ! "
            "decodebin ! "
            "videoconvert ! "
            "videoscale ! video/x-raw,width=%d,height=%d ! "
            "videorate ! video/x-raw,framerate=%d/1 ! "
            "jpegenc quality=%d ! "
            "multifilesink location=\"%s/frame-%%04d.jpg\"",
            vpp->filename, vpp->width, vpp->height, 
            vpp->frame_rate, vpp->quality, video_dir);
    }

    // Create pipeline
    pipeline = gst_parse_launch(pipeline_str, &error);
    if (error) {
        g_print("Failed to create pipeline for %s: %s\n", 
                vpp->filename, error->message);
        g_error_free(error);
        return NULL;
    }

    g_print("Processing %s...\n", vpp->filename);
    g_print("Saving frames to: %s\n", video_dir);

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
                g_printerr("Error processing %s: %s\n", 
                          vpp->filename, err->message);
                g_printerr("Debugging information: %s\n", 
                          debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            }
            case GST_MESSAGE_EOS:
                g_print("Finished processing %s\n", vpp->filename);
                break;
            default:
                g_printerr("Unexpected message received for %s\n", 
                          vpp->filename);
                break;
        }
        gst_message_unref(msg);
    }

    // Cleanup
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        g_print("Usage: %s <video-file1> [video-file2] ... [options]\n", argv[0]);
        g_print("Options:\n");
        g_print("  -r <frame_rate>   Frame rate (default: 5)\n");
        g_print("  -w <width>        Output width (default: original)\n");
        g_print("  -h <height>       Output height (default: original)\n");
        g_print("  -q <quality>      JPEG quality 0-100 (default: 85)\n");
        return 1;
    }

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Parse command line arguments
    int frame_rate = 5;
    int width = -1;  // -1 indicates original size should be kept
    int height = -1; // -1 indicates original size should be kept
    int quality = 85;
    int i;

    // Count number of video files
    int num_videos = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            num_videos++;
        }
    }

    // Create threads and params for each video
    pthread_t *threads = malloc(num_videos * sizeof(pthread_t));
    VideoProcessingParams *params = malloc(num_videos * sizeof(VideoProcessingParams));

    // Process command line arguments and start threads
    int video_index = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'r':
                    if (i + 1 < argc) frame_rate = atoi(argv[++i]);
                    break;
                case 'w':
                    if (i + 1 < argc) width = atoi(argv[++i]);
                    break;
                case 'h':
                    if (i + 1 < argc) height = atoi(argv[++i]);
                    break;
                case 'q':
                    if (i + 1 < argc) quality = atoi(argv[++i]);
                    break;
            }
        } else {
            // Set up parameters for this video
            params[video_index].filename = argv[i];
            params[video_index].frame_rate = frame_rate;
            params[video_index].quality = quality;
            params[video_index].keep_original_size = (width == -1 || height == -1);
            params[video_index].width = width;
            params[video_index].height = height;

            // Create thread for processing
            pthread_create(&threads[video_index], NULL, 
                         process_video, &params[video_index]);
            video_index++;
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < num_videos; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup
    free(threads);
    free(params);

    g_print("All videos processed successfully!\n");
    return 0;
}