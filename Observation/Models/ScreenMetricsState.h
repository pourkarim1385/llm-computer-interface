#ifndef ACCESSIBILITYSERVICE_SCREENMETRICSSTATE_H
#define ACCESSIBILITYSERVICE_SCREENMETRICSSTATE_H

#include <string>

class ScreenMetricsState {
private:
        int width{0};
        int height{0};
        float scaleFactor{1.0f}; //Managing DPI Scaling
public:
    ScreenMetricsState();
    explicit ScreenMetricsState(int w, int h, float sF) : width(w) , height(h), scaleFactor(sF) {}
    int getWidth() const {return width;}
    int getHeight() const {return height;}
    float getScaleFactor() const {return scaleFactor;}

    std::string resolve();
};


#endif //ACCESSIBILITYSERVICE_SCREENMETRICSSTATE_H
