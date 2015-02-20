#ifndef HEXIOR_H
#define HEXIOR_H

#include <QtWidgets/QMainWindow>
#include "ui_hexior.h"

class Hexior : public QMainWindow
{
	Q_OBJECT

public:
	Hexior(QWidget *parent = 0);
	~Hexior();

private:
	Ui::HexiorClass ui;
};

#endif // HEXIOR_H
