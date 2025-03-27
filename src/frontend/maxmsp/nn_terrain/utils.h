
#include <filesystem>
#include <chrono>
#include <ctime>

#include "c74_min_api.h"


using namespace c74::min;
using namespace c74::min::ui;

number shiftX(const number x, const number r, const number theta) {
    return x - std::sin(theta) * r;
}

number shiftY(const number y, const number r, const number theta) {
	return y - std::cos(theta) * r;
}

number ftoi(const number x, const number f_range, const number int_range){
    float f = static_cast<float>(f_range);
    int i = static_cast<int>(int_range);
    return static_cast<int>(round( i*(x + f)/(2*f)  ));
}

number itof(const number x, const number int_range, const number f_range){
    float f = static_cast<float>(f_range);
    int i = static_cast<int>(int_range);
    return -f + (x/i)*2*f;
}

int itoidx(const int x, const int y, const int width){
    return y*width+x;
}

atoms create_log_and_save(const std::string& save_name, const std::string& base_path, const std::string& input) {

    std::filesystem::create_directory(base_path + "/logs");

//    // Get the current timestamp
//    std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//
//    // Convert the timestamp to a string
//    string timestamp_str = std::ctime(&timestamp);
//
//    // Remove newline character from the timestamp string
//    timestamp_str.erase(timestamp_str.find_last_not_of("\n") + 1);
//
//
//    for (char& c : timestamp_str) {
//        if (c == ':') {
//            c = '-';
//        }
//        if (c== ' ') {
//            c = '_';
//        }
//    }
//    // Use the timestamp as the file name
//    string file_name = save_name + timestamp_str;
//    string src_path_str = base_path + "/logs/" + file_name + ".txt";
    
    char buffer[256];
    struct tm * datetime;
    std::time_t timestamp;
    
    std::time(&timestamp);
    datetime = localtime(&timestamp);
    
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", datetime);
    
    string file_name = save_name + buffer;
    string src_path_str = base_path + "/logs/" + file_name + ".txt";


    string src_content;
    {
        std::ifstream in{ src_path_str };
        src_content = string{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
        //src_content += input + " - " + timestamp_str;
    }
    {
        std::ofstream out{ src_path_str };
        out << input;
        out.close();
    }
    
    return { {src_path_str, file_name} };
}

std::string min_devkit_path() {
#ifdef WIN_VERSION
    char    pathstr[4096];
    HMODULE hm = nullptr;

    if (!GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&min_devkit_path, &hm)) {
        int ret = GetLastError();
        fprintf(stderr, "GetModuleHandle() returned %d\n", ret);
    }
    GetModuleFileNameA(hm, pathstr, sizeof(pathstr));

    // path now is the path to this external's binary, including the binary filename.
    auto filename = strrchr(pathstr, '\\');
    if (filename)
        *filename = 0;
    auto externals = strrchr(pathstr, '\\');
    if (externals)
        *externals = 0;

    path p{ pathstr };    // convert to Max path
    return p;
#endif    // WIN_VERSION

#ifdef MAC_VERSION
    CFBundleRef this_bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.cycling74.nn-terrain"));
    CFURLRef    this_url = CFBundleCopyExecutableURL(this_bundle);
    char        this_path[4096];

    CFURLGetFileSystemRepresentation(this_url, true, reinterpret_cast<UInt8*>(this_path), 4096);

    string this_path_str{ this_path };

    CFRelease(this_url);
    // remember: we don't want to release the bundle because Max is still using it!

    // we now have a path like this:
    // /Users/tim/Materials/min-devkit/externals/min.project.mxo/Contents/MacOS/min.project"
    // so we need to chop off 5 slashes from the end

    auto iter = this_path_str.find("/externals/nn_terrain.mxo/Contents/MacOS/nn_terrain");
    this_path_str.erase(iter, strlen("/externals/nn_terrain.mxo/Contents/MacOS/nn_terrain"));
    return this_path_str;
#endif    // MAC_VERSION
}


unsigned power_ceil(unsigned x) {
  if (x <= 1)
    return 1;
  int power = 2;
  x--;
  while (x >>= 1)
    power <<= 1;
  return power;
}
