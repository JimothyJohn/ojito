// DO NOT add a / to the end of the URL path!
#define HOST "http://10.0.0.31/predictions"
// #define HOST "https://api.replicate.com/v1/predictions"
#define MODEL_VERSION "0239647164ce56b643213501d63f6116a76a3c620e4194814968e8ba1aa64cb7"

TaskHandle_t InferenceHandle;
unsigned long lastCaptureTime = 0; // Last shooting time
bool camera_sign = false;          // Check camera status
bool sd_sign = false;              // Check sd status
