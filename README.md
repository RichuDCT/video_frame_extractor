
# Video Frame Extractor

## 📝 Overview

A C program that leverages GStreamer to extract frames from video files, converting them into sequences of JPEG images with configurable parameters.

## ✨ Features
* Extract frames from any GStreamer-supported video format
* Configurable frame rate (default: 5 fps)
* Adjustable output resolution (default: 640x480)
* JPEG quality control (default: 85%)
* Automatic output directory creation
* Organized naming pattern: `videoname_frame-XXXX.jpg`

## 🔧 Prerequisites

### System Requirements
* Linux-based operating system (tested on Ubuntu/Debian)
* GCC compiler
* GStreamer 1.0 or later

### Required Libraries
Install the necessary development libraries:
```bash
# Update package list
sudo apt-get update

# Install required packages
sudo apt-get install build-essential
sudo apt-get install libgstreamer1.0-dev
sudo apt-get install libgstreamer-plugins-base1.0-dev
```

## 🚀 Installation


 Compile the program:
 
   ```bash
   gcc -o frame_extractor frame_extractor.c `pkg-config --cflags --libs gstreamer-1.0`
   ```

## 📖 Usage

### Basic Usage
```bash
./frame_extractor <video-file>
```

### Example
```bash
./frame_extractor input.mp4
```

### Output Structure
* Creates `vid_frame_img` directory in current working directory
* Saves frames as JPEG images:
  * `vid_frame_img/video_frame-0001.jpg`
  * `vid_frame_img/video_frame-0002.jpg`
  * etc.

## 🛠️ Technical Details

### GStreamer Pipeline
```bash
filesrc ! decodebin ! videoconvert ! videoscale ! 
video/x-raw,width=640,height=480 ! videorate ! 
video/x-raw,framerate=5/1 ! jpegenc quality=85 ! multifilesink
```

### Pipeline Components
| Element | Purpose |
|---------|----------|
| `filesrc` | Reads input video file |
| `decodebin` | Automatically decodes video |
| `videoconvert` | Converts frames to suitable format |
| `videoscale` | Resizes frames |
| `videorate` | Adjusts frame rate |
| `jpegenc` | Encodes frames as JPEG |
| `multifilesink` | Saves individual frames |

## ⚙️ Customization

### Modifiable Parameters
```c
// Frame rate
framerate=5/1              // Change '5' for different fps

// Resolution
width=640,height=480       // Modify dimensions

// JPEG quality
quality=85                 // Range: 0-100

// Output directory
vid_frame_img              // Change directory name

// Output pattern
location="vid_frame_img/%s_frame-%%04d.jpg"  // Modify pattern
```

## 🐛 Error Handling

The program handles several types of errors:
* Invalid command-line arguments
* Pipeline creation failures
* Runtime errors
* End-of-stream detection

## ⚠️ Limitations
* Single video processing only
* Source code modification needed for parameter changes
* No progress reporting
* Empty output directory required

## 🔍 Troubleshooting

### Common Issues

#### Compilation Errors
* **Solution**: Verify library installation and GStreamer version

#### Runtime Errors
* **Check**:
  * Input file existence
  * Disk space
  * Directory permissions

#### No Frame Output
* **Verify**:
  * Video file validity
  * Codec support in GStreamer

*For more information, visit the [GStreamer documentation](https://gstreamer.freedesktop.org/documentation/).*