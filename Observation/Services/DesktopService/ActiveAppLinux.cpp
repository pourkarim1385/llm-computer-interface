#include "ActiveAppLinux.hpp"

ActiveWindowInfo getActiveApp(){    
        ActiveWindowInfo info{"", "", 0};
    
        Display* display = XOpenDisplay(nullptr);
        if (!display){
            return info;
        }
    
        Window root = DefaultRootWindow(display);
        Atom activeAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    
        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        unsigned char* data = nullptr;
    
        if (XGetWindowProperty(display, root, activeAtom, 0, 1, False,
                                AnyPropertyType, &actualType, &actualFormat,
                                &nItems, &bytesAfter, &data) == Success && data) {
            Window activeWindow = *(Window*)data;
            XFree(data);
    
            Atom nameAtom = XInternAtom(display, "_NET_WM_NAME", True);
            if (XGetWindowProperty(display, activeWindow, nameAtom, 0, 1024, False,
                                    AnyPropertyType, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data) == Success && data) {
                info.title = std::string((char*)data);
                XFree(data);
            }
    
            Atom pidAtom = XInternAtom(display, "_NET_WM_PID", True);
            if (XGetWindowProperty(display, activeWindow, pidAtom, 0, 1, False,
                                    AnyPropertyType, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data) == Success && data) {
                info.pid = *(unsigned long*)data;
                XFree(data);
    
                std::ifstream commFile("/proc/" + std::to_string(info.pid) + "/comm");
                if (commFile) {
                    std::getline(commFile, info.processName);
                }
            }
        }
    
        XCloseDisplay(display);
        return info;
    }
