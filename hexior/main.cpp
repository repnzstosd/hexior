#include "hexior.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Hexior w;
	w.show();
	return a.exec();
}
