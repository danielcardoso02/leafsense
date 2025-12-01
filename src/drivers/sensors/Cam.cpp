#include "Cam.h"

// Mock: Returns a dummy path instead of taking a real photo
std::string Cam::takePhoto() {
    return "resources/images/logo_leafsense.png"; // Use existing logo as fake photo
}