#ifndef HEXIOR_H
#define HEXIOR_H

#include <QtWidgets/QMainWindow>
#include "ui_hexior.h"

#include "amiga.h"

class Hexior : public QMainWindow {
	Q_OBJECT

	public:
		Hexior(QWidget *parent = 0);
		~Hexior();

	private:
		Ui::HexiorClass ui;

	public slots:
		void getRegisters();

	private:
		Amiga mAmiga;
};

#endif // HEXIOR_H
