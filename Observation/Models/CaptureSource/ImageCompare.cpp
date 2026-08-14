#include "ImageCompare.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"


namespace fs = std::filesystem;

static constexpr int RESIZE_DIM = 32;
static constexpr int HASH_DIM   = 8;

struct ImgFile {
    fs::path path;
    long number;
};

static bool parseNumber(const std::string& name, long& outNum) {
    static const std::regex re(R"((\d+)\.\w+)");
    std::smatch m;
    if (std::regex_match(name, m, re)) {
        outNum = std::stol(m[1].str());
        return true;
    }
    return false;
}

static void toGray32(const uint8_t* rgb, int w, int h, double out[RESIZE_DIM][RESIZE_DIM]) {
    std::vector<double> gray(w * h);
    for (int i = 0; i < w * h; i++) {
        int r = rgb[i*3], g = rgb[i*3+1], b = rgb[i*3+2];
        gray[i] = 0.299*r + 0.587*g + 0.114*b;
    }
    for (int y = 0; y < RESIZE_DIM; y++) {
        for (int x = 0; x < RESIZE_DIM; x++) {
            int x0 = x * w / RESIZE_DIM, x1 = (x+1) * w / RESIZE_DIM;
            int y0 = y * h / RESIZE_DIM, y1 = (y+1) * h / RESIZE_DIM;
            if (x1 <= x0) x1 = x0 + 1;
            if (y1 <= y0) y1 = y0 + 1;
            double sum = 0; int count = 0;
            for (int yy = y0; yy < y1 && yy < h; yy++)
                for (int xx = x0; xx < x1 && xx < w; xx++) {
                    sum += gray[yy * w + xx];
                    count++;
                }
            out[y][x] = count ? sum / count : 0.0;
        }
    }
}

static void dct1D(const double* in, double* out, int N) {
    for (int k = 0; k < N; k++) {
        double sum = 0.0;
        for (int n = 0; n < N; n++)
            sum += in[n] * std::cos(M_PI / N * (n + 0.5) * k);
        out[k] = sum * (k == 0 ? std::sqrt(1.0 / N) : std::sqrt(2.0 / N));
    }
}

static void dct2D(double in[RESIZE_DIM][RESIZE_DIM], double out[RESIZE_DIM][RESIZE_DIM]) {
    double temp[RESIZE_DIM][RESIZE_DIM];
    double row[RESIZE_DIM], rowOut[RESIZE_DIM];
    for (int y = 0; y < RESIZE_DIM; y++) {
        for (int x = 0; x < RESIZE_DIM; x++) row[x] = in[y][x];
        dct1D(row, rowOut, RESIZE_DIM);
        for (int x = 0; x < RESIZE_DIM; x++) temp[y][x] = rowOut[x];
    }
    double col[RESIZE_DIM], colOut[RESIZE_DIM];
    for (int x = 0; x < RESIZE_DIM; x++) {
        for (int y = 0; y < RESIZE_DIM; y++) col[y] = temp[y][x];
        dct1D(col, colOut, RESIZE_DIM);
        for (int y = 0; y < RESIZE_DIM; y++) out[y][x] = colOut[y];
    }
}

static uint64_t computePHash(const uint8_t* rgb, int w, int h) {
    double gray[RESIZE_DIM][RESIZE_DIM];
    toGray32(rgb, w, h, gray);
    double freq[RESIZE_DIM][RESIZE_DIM];
    dct2D(gray, freq);
    double vals[HASH_DIM * HASH_DIM];
    int idx = 0;
    for (int y = 0; y < HASH_DIM; y++)
        for (int x = 0; x < HASH_DIM; x++)
            vals[idx++] = freq[y][x];
    double sum = 0.0;
    for (int i = 1; i < HASH_DIM * HASH_DIM; i++) sum += vals[i];
    double mean = sum / (HASH_DIM * HASH_DIM - 1);
    uint64_t hash = 0;
    for (int i = 0; i < HASH_DIM * HASH_DIM; i++) {
        hash <<= 1;
        if (vals[i] > mean) hash |= 1;
    }
    return hash;
}

static int hammingDistance(uint64_t a, uint64_t b) {
    return static_cast<int>(std::bitset<64>(a ^ b).count());
}

bool cleanupDuplicateScreenshots(const std::string& dir, double thresholdPercent) {
    std::vector<ImgFile> files;
    for (auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        long num;
        if (parseNumber(entry.path().filename().string(), num))
            files.push_back({entry.path(), num});
    }
    if (files.size() != 2) {
        std::cerr << "Expected exactly 2 screenshots, found " << files.size() << std::endl;
        return false;
    }
    if (files[0].number > files[1].number) std::swap(files[0], files[1]);

    int w1, h1, c1, w2, h2, c2;
    uint8_t* img1 = stbi_load(files[0].path.string().c_str(), &w1, &h1, &c1, 3);
    uint8_t* img2 = stbi_load(files[1].path.string().c_str(), &w2, &h2, &c2, 3);
    if (!img1 || !img2) {
        std::cerr << "Failed to load images." << std::endl;
        if (img1) stbi_image_free(img1);
        if (img2) stbi_image_free(img2);
        return false;
    }

    uint64_t hash1 = computePHash(img1, w1, h1);
    uint64_t hash2 = computePHash(img2, w2, h2);
    stbi_image_free(img1);
    stbi_image_free(img2);

    int distance = hammingDistance(hash1, hash2);
    double diffPercent = (distance / 64.0) * 100.0;
    std::cout << "Hamming distance: " << distance << " (" << diffPercent << "%)" << std::endl;

    if (diffPercent > thresholdPercent) {
        fs::remove(files[0].path);
        std::cout << "Removed old screenshot: " << files[0].path << std::endl;

        fs::path newPath = files[1].path.parent_path() / ("1" + files[1].path.extension().string());
        fs::rename(files[1].path, newPath);
        std::cout << "Renamed remaining screenshot to: " << newPath << std::endl;
        return true;
    } else {
        fs::remove(files[1].path);
        std::cout << "Removed new screenshot (too similar): " << files[1].path << std::endl;

        fs::path newPath = files[0].path.parent_path() / ("1" + files[0].path.extension().string());
        fs::rename(files[0].path, newPath);
        std::cout << "Renamed remaining screenshot to: " << newPath << std::endl;
        return false;
    }
}
