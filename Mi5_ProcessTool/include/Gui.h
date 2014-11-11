#ifndef GUI_H
#define GUI_H

#include <Mi5_ProcessTool/GeneratedFiles/ui_gui_processTool.h>

class Gui : public QMainWindow
{
    Q_OBJECT
public:
    Gui(QWidget* parent = 0);
    ~Gui();

private:
    Ui_MainWindow ui;
};

#endif //GUI_H