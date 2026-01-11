/**
 * @file Cam.cpp
 * @brief Implementation of Camera Driver for Raspberry Pi
 */

#include "Cam.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Check if a video device is available and supports capture
 * @param device_path Path to video device (e.g., "/dev/video0")
 * @return true if device is a valid capture device
 */
static bool isValidCaptureDevice(const char* device_path) {
    int fd = open(device_path, O_RDWR);
    if (fd < 0) {
        return false;
    }
    
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        close(fd);
        return false;
    }
    
    close(fd);
    
    // Check if device supports video capture
    return (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0;
}

/**
 * @brief Enhance image quality using computer vision techniques
 * @param input Input image (BGR format)
 * @return Enhanced image
 */
static cv::Mat enhanceImage(const cv::Mat& input) {
    if (input.empty()) return input;
    
    cv::Mat enhanced = input.clone();
    
    // 1. Auto white balance correction
    std::vector<cv::Mat> channels;
    cv::split(enhanced, channels);
    
    for (int i = 0; i < 3; i++) {
        cv::Scalar mean = cv::mean(channels[i]);
        double scale = 128.0 / mean[0]; // Normalize to mid-gray
        channels[i] = channels[i] * scale;
    }
    cv::merge(channels, enhanced);
    
    // 2. Contrast enhancement using CLAHE (Contrast Limited Adaptive Histogram Equalization)
    cv::Mat lab;
    cv::cvtColor(enhanced, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> labChannels;
    cv::split(lab, labChannels);
    
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(labChannels[0], labChannels[0]);
    
    cv::merge(labChannels, lab);
    cv::cvtColor(lab, enhanced, cv::COLOR_Lab2BGR);
    
    // 3. Slight sharpening to enhance plant details
    cv::Mat blurred;
    cv::GaussianBlur(enhanced, blurred, cv::Size(0, 0), 3);
    cv::addWeighted(enhanced, 1.5, blurred, -0.5, 0, enhanced);
    
    // 4. Slight Gaussian blur to reduce noise
    cv::GaussianBlur(enhanced, enhanced, cv::Size(3, 3), 0.5);
    
    return enhanced;
}

/**
 * @brief Try to capture using OpenCV with specified device
 * @param device Device number to try
 * @param filepath Output file path
 * @return true if capture succeeded
 */
static bool tryOpenCVCapture(int device, const std::string& filepath) {
    cv::VideoCapture camera;
    
    // Try different backends in order of preference
    const cv::VideoCaptureAPIs backends[] = {
        cv::CAP_V4L2,
        cv::CAP_GSTREAMER,
        cv::CAP_ANY
    };
    
    for (const auto& backend : backends) {
        camera.open(device, backend);
        if (camera.isOpened()) {
            std::cout << "[Camera] Opened device " << device 
                      << " with backend " << backend << std::endl;
            break;
        }
    }
    
    if (!camera.isOpened()) {
        return false;
    }
    
    // Set camera properties
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    camera.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    // Discard first few frames (camera warmup)
    cv::Mat frame;
    for (int i = 0; i < 10; i++) {
        camera >> frame;
        usleep(100000); // 100ms delay
    }
    
    // Capture final frame
    camera >> frame;
    camera.release();
    
    if (frame.empty()) {
        return false;
    }
    
    // Apply image enhancement
    cv::Mat enhanced = enhanceImage(frame);
    
    // Save enhanced image as JPEG
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(85); // Quality 0-100
    
    return cv::imwrite(filepath, enhanced, compression_params);
}

/* ============================================================================
 * Photo Capture Implementation
 * ============================================================================ */

std::string Cam::takePhoto() 
{
    // Output directory for captured images
    const std::string OUTPUT_DIR = "/opt/leafsense/gallery/";
    
    // Create output directory if it doesn't exist
    struct stat info;
    if (stat(OUTPUT_DIR.c_str(), &info) != 0) {
        // Directory doesn't exist, create it
        if (mkdir(OUTPUT_DIR.c_str(), 0755) != 0) {
            std::cerr << "[Camera] Failed to create gallery directory: " << OUTPUT_DIR << std::endl;
            return ""; // Return empty string on failure
        }
    }
    
    // Generate filename with timestamp
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    std::ostringstream filename;
    filename << OUTPUT_DIR << "plant_"
             << std::put_time(timeinfo, "%Y%m%d_%H%M%S")
             << ".jpg";
    
    std::string filepath = filename.str();
    
    std::cout << "[Camera] Attempting capture to: " << filepath << std::endl;
    
    // Strategy 1: Use libcamera's cam utility FIRST (works with Pi Camera on modern kernels)
    std::cout << "[Camera] Trying libcamera 'cam' utility..." << std::endl;
    
    // Capture using cam utility - saves as PPM, then convert to JPEG
    std::string ppmFile = filepath.substr(0, filepath.length() - 4) + ".ppm";
    std::ostringstream camCmd;
    camCmd << "cam --camera=1 --capture=1 --stream width=640,height=480,pixelformat=BGR888 --file=" << ppmFile << " 2>/dev/null";
    
    int camResult = system(camCmd.str().c_str());
    if (camResult == 0) {
        // Check if PPM file was created
        struct stat st;
        if (stat(ppmFile.c_str(), &st) == 0 && st.st_size > 0) {
            // Convert PPM to JPEG using OpenCV
            cv::Mat ppmImage = cv::imread(ppmFile);
            if (!ppmImage.empty()) {
                cv::Mat enhanced = enhanceImage(ppmImage);
                std::vector<int> params;
                params.push_back(cv::IMWRITE_JPEG_QUALITY);
                params.push_back(85);
                if (cv::imwrite(filepath, enhanced, params)) {
                    std::remove(ppmFile.c_str()); // Delete temp PPM
                    std::cout << "[Camera] Captured via libcamera cam: " << filepath << std::endl;
                    return filepath;
                }
            }
        }
    }
    
    // Strategy 2: Try libcamera-still if available
    std::cout << "[Camera] Trying libcamera-still..." << std::endl;
    std::ostringstream stillCmd;
    stillCmd << "libcamera-still -o " << filepath << " --width 640 --height 480 -t 500 -n 2>/dev/null";
    if (system(stillCmd.str().c_str()) == 0) {
        struct stat st;
        if (stat(filepath.c_str(), &st) == 0 && st.st_size > 0) {
            std::cout << "[Camera] Captured via libcamera-still: " << filepath << std::endl;
            return filepath;
        }
    }
    
    // Strategy 3: Try OpenCV with video devices (USB webcam fallback)
    std::cout << "[Camera] Trying OpenCV V4L2 devices..." << std::endl;
    
    const int devices_to_try[] = {0, 1, 2};
    for (int device : devices_to_try) {
        std::ostringstream dev_path;
        dev_path << "/dev/video" << device;
        
        if (isValidCaptureDevice(dev_path.str().c_str())) {
            std::cout << "[Camera] Trying device " << device << "..." << std::endl;
            if (tryOpenCVCapture(device, filepath)) {
                std::cout << "[Camera] Photo captured successfully: " << filepath << std::endl;
                return filepath;
            }
        }
    }
    
    // Strategy 4: Create realistic test pattern as absolute fallback
    std::cerr << "╔══════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cerr << "║  WARNING: CAMERA HARDWARE NOT DETECTED                           ║" << std::endl;
    std::cerr << "║  Generating TEST PATTERN instead of real camera capture         ║" << std::endl;
    std::cerr << "║                                                                  ║" << std::endl;
    std::cerr << "║  To fix:                                                         ║" << std::endl;
    std::cerr << "║  1. Connect Raspberry Pi Camera Module to CSI port              ║" << std::endl;
    std::cerr << "║  2. Check camera detected: vcgencmd get_camera                  ║" << std::endl;
    std::cerr << "║  3. Enable camera in config.txt if needed                       ║" << std::endl;
    std::cerr << "╚══════════════════════════════════════════════════════════════════╝" << std::endl;
    
    cv::Mat testImage(480, 640, CV_8UC3);
    
    // Create a more realistic plant-like pattern
    cv::RNG rng(std::time(nullptr));
    
    // Background with texture (soil/pot)
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            int noise = rng.uniform(-20, 20);
            testImage.at<cv::Vec3b>(y, x) = cv::Vec3b(
                std::max(0, std::min(255, 60 + noise)),   // B
                std::max(0, std::min(255, 80 + noise)),   // G  
                std::max(0, std::min(255, 70 + noise))    // R
            );
        }
    }
    
    // Draw plant-like structure with varied greens
    for (int i = 0; i < 8; i++) {
        int centerX = 320 + rng.uniform(-100, 100);
        int centerY = 200 + rng.uniform(-80, 80);
        int radius = rng.uniform(30, 60);
        int greenShade = rng.uniform(100, 180);
        cv::circle(testImage, cv::Point(centerX, centerY), radius, 
                   cv::Scalar(rng.uniform(40, 80), greenShade, rng.uniform(50, 90)), -1);
    }
    
    // Add subtle texture and details
    cv::Mat textureNoise(480, 640, CV_8UC3);
    rng.fill(textureNoise, cv::RNG::UNIFORM, -10, 10);
    cv::add(testImage, textureNoise, testImage);
    
    // Add TEST PATTERN warning overlay
    cv::rectangle(testImage, cv::Point(100, 180), cv::Point(540, 280), 
                  cv::Scalar(0, 0, 0), -1);  // Black background
    cv::rectangle(testImage, cv::Point(100, 180), cv::Point(540, 280), 
                  cv::Scalar(0, 0, 255), 3);  // Red border
    
    cv::putText(testImage, "TEST PATTERN", cv::Point(160, 220),
                cv::FONT_HERSHEY_DUPLEX, 1.2, cv::Scalar(0, 0, 255), 3);
    cv::putText(testImage, "Camera Not Detected", cv::Point(150, 260),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    // Add timestamp watermark (small, bottom corner)
    std::ostringstream watermark;
    watermark << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    cv::putText(testImage, watermark.str(), cv::Point(10, 470),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200, 200, 200), 1);
    
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(85);
    
    if (cv::imwrite(filepath, testImage, compression_params)) {
        std::cout << "[Camera] Test image created: " << filepath << std::endl;
        return filepath;
    }
    
    std::cerr << "[Camera] All capture methods failed" << std::endl;
    return "";
}