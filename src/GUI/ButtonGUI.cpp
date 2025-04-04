#include "ButtonGUI.h"
#include "binaryttf.h"

M5EPD_Canvas canvas(&M5.EPD);
ButtonGUI::ButtonGUI(int id, int16_t x, int16_t y, int16_t width, int16_t height, String text, String dueDate,Task task, int datum, int offset)
{
    this->id = id;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->text = text;
    this->datum = datum;
    this->offset = offset;
    this->task = task;
    this->dueDate = dueDate;
}

ButtonGUI::~ButtonGUI()
{

}

void ButtonGUI::SetPos(int16_t x, int16_t y)
{
    this->x = x;
    this->y = y;
}

void ButtonGUI::SetSize(int16_t width, int16_t height)
{
    this->width = width;
    this->height = height;
}

void ButtonGUI::SetString(String string)
{
    this->text = text;
}

void ButtonGUI::Draw(M5EPD_Canvas &mainCanvas)
{
    canvas.createCanvas(width, height);

    canvas.fillCanvas(0);

    canvas.createRender(32, 256);

    canvas.setTextSize(32);
    canvas.setTextDatum(datum);
    canvas.setTextColor(15);

    canvas.drawLine(0,height-1,width,height-1,15);
    canvas.drawString(this->text, offset, height/2);
    canvas.drawRightString(this->dueDate, width-offset, height/2,canvas.fontsLoaded());

    canvas.pushToCanvas(x, y, &mainCanvas);
    
    canvas.deleteCanvas();

    log_d("width:%d height:%d x:%d y: %d",width, height,x,y);
}

void ButtonGUI::PressButton()
{
    pressedFunction(&task);
}

void ButtonGUI::SetPressedFunction(void (*function)(Task*))
{
    pressedFunction = function;
}

bool ButtonGUI::isInBox(int16_t x, int16_t y)
{
    if(id == -1)
        return false;

    if(x > this->x && this->x + this->width > x && y > this->y && this->y + this->height > y)
    {
        PressButton();
        return true;
    }
    return false;
}