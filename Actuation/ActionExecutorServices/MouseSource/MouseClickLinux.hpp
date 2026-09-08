#ifndef MOUSE_CLICK_LINUX_HPP
#define MOUSE_CLICK_LINUX_HPP

#include <unistd.h>

void clickMouseLeft();
void clickMouseRight();
void clickMouseMiddle();
void doubleClick();
void mouseButtonDown(unsigned int button);
void mouseButtonUp(unsigned int button);
void scroll(int direction, int amount = 3);
void mouseButtonHold(unsigned int button);
void mouseButtonRelease(unsigned int button);

#endif
