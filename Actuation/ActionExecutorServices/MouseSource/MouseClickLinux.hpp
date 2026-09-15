#ifndef MOUSE_CLICK_LINUX_HPP
#define MOUSE_CLICK_LINUX_HPP

#include <unistd.h>

void clickMouseLeftLinux();
void clickMouseRightLinux();
void clickMouseMiddleLinux();
void doubleClickLinux();
void mouseButtonDownLinux(unsigned int button);
void mouseButtonUpLinux(unsigned int button);
void scrollLinux(int direction, int amount);
void mouseButtonHoldLinux(unsigned int button);
void mouseButtonReleaseLinux(unsigned int button);

#endif
