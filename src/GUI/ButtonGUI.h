#pragma once
#include <M5EPD.h>
#include "../ToDoAPI/ToDoAPI.h"

class ButtonGUI
{
public:
    ButtonGUI(int id, int16_t x, int16_t y, int16_t width, int16_t height, String text, String dueDate, Task task, int datum = ML_DATUM, int offset = 16);
    ~ButtonGUI();

    int16_t getX() { return x; }
    int16_t getY() { return y; }
    int16_t getWidth() { return width; }
    int16_t getHeight() { return height; }

    void SetPos(int16_t x, int16_t y);
    void SetSize(int16_t width, int16_t height);
    void SetString(String string);

    void Draw(M5EPD_Canvas &canvas);
    void PressButton();
    void SetPressedFunction(void (*function)(Task *));

    bool isInBox(int16_t x, int16_t y);

private:
    int id;
    int16_t x, y, width, height;
    String text;
    int datum;
    int offset;
    Task task;
    String dueDate;
    void (*pressedFunction)(Task *) = NULL;
};