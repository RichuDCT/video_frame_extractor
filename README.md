
# Video Frame Extractor

# GStreamer Frame Extractor

## 📝 Overview

A multi-threaded C program that uses GStreamer to extract frames from multiple video files simultaneously. The program creates a directories for all video then save the frame with video name and allows customization of frame extraction parameters.
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

`./frame_extractor <video-file1> [video-file2] ... [options]`

### Command Line Options

| Option | Description          | Default               |
| ------ | -------------------- | --------------------- |
| `-r`   | Frame rate (fps)     | 5                     |
| `-w`   | Output width         | original video width  |
| `-h`   | Output height        | original video  hight |
| `-q`   | JPEG quality (0-100) | 85                    |

### Examples


```
# Process single video with default settings
./frame_extractor video1.mp4

# Process multiple videos with default settings
./frame_extractor video1.mp4 video2.mp4 video3.mp4

# Process videos with custom settings
./frame_extractor *.mp4 -r 10 -w 1280 -h 720 -q 90
```

### Output Structure
* Creates `vid_frame_img` directory in current working directory
* Saves frames as JPEG images:
```
vid_frame_img/
├── video1/
│   ├── frame-0001.jpg
│   ├── frame-0002.jpg
│   └── frame-0003.jpg ....
├── video2/
│   ├── frame-0001.jpg
│   ├── frame-0002.jpg
│   └── frame-0003.jpg ....
....
```
## 🛠️ Technical Details

### GStreamer Pipeline

For original dimensions:

``` bash
`filesrc ! decodebin ! videoconvert ! videorate ! video/x-raw,framerate=FPS/1 ! jpegenc quality=QUALITY ! multifilesink`
```


For custom dimensions:
```bash
`filesrc ! decodebin ! videoconvert ! videoscale ! video/x-raw,width=WIDTH,height=HEIGHT ! videorate ! video/x-raw,framerate=FPS/1 ! jpegenc quality=QUALITY ! multifilesink`
```

Key components:

- Pipeline preserves original video dimensions by default
- `videoscale` element is only used when custom dimensions are specified
- Frame rate and JPEG quality can be customized through command-line options
- Outputs to video-specific subdirectories for better organization

### Threading Model

- One thread per video file
- Concurrent processing using POSIX threads
- Thread-safe resource management
- Independent error handling per thread

## ⚙️ Performance Optimization

### Memory Usage

- Memory consumption scales with:
    - Number of concurrent videos
    - Output frame dimensions
    - Frame rate

### Recommended Limits

- Maximum concurrent videos: Dependent on system RAM and CPU cores
- Suggested starting point: Number of CPU cores - 1


*For more information, visit the [GStreamer documentation](https://gstreamer.freedesktop.org/documentation/).*