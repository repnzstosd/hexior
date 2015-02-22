#include "hexior.h"
#include <qfile.h>
#include <qdir.h>

Hexior::Hexior(QWidget *parent) : QMainWindow(parent) {


	ui.setupUi(this);
	loadFile("Soundtracker-IV", 0x200000);
	mAmiga.mM68K.reset();
	getRegisters();
}

void Hexior::getRegisters() {

	mAmiga.mM68K.step();

	ui.valueA0->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[0], 8, 16, QLatin1Char('0')));
	ui.valueA1->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[1], 8, 16, QLatin1Char('0')));
	ui.valueA2->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[2], 8, 16, QLatin1Char('0')));
	ui.valueA3->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[3], 8, 16, QLatin1Char('0')));
	ui.valueA4->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[4], 8, 16, QLatin1Char('0')));
	ui.valueA5->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[5], 8, 16, QLatin1Char('0')));
	ui.valueA6->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[6], 8, 16, QLatin1Char('0')));
	ui.valueA7->setText(QString("%1").arg(mAmiga.mM68K.mAddressRegister[7], 8, 16, QLatin1Char('0')));

	ui.valueD0->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[0], 8, 16, QLatin1Char('0')));
	ui.valueD1->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[1], 8, 16, QLatin1Char('0')));
	ui.valueD2->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[2], 8, 16, QLatin1Char('0')));
	ui.valueD3->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[3], 8, 16, QLatin1Char('0')));
	ui.valueD4->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[4], 8, 16, QLatin1Char('0')));
	ui.valueD5->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[5], 8, 16, QLatin1Char('0')));
	ui.valueD6->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[6], 8, 16, QLatin1Char('0')));
	ui.valueD7->setText(QString("%1").arg(mAmiga.mM68K.mDataRegister[7], 8, 16, QLatin1Char('0')));

	ui.valueSR->setText(QString("%1").arg(mAmiga.mM68K.mSR.SR, 4, 16, QLatin1Char('0')));
	ui.valueSSP->setText(QString("%1").arg(mAmiga.mM68K.mSSP, 8, 16, QLatin1Char('0')));
	ui.valuePC->setText(QString("%1").arg(mAmiga.mM68K.mPC, 8, 16, QLatin1Char('0')));

	ui.textEdit->append(QString("%1").arg(mAmiga.mM68K.mSR.SR, 0, 16));

	ui.textEdit->append(QString("%1").arg(mAmiga.mM68K.readWord(0x20075e), 0, 16));

	for(int j = 0; j < 6; ++j) {
		QString foo;
		foo = QString("%1: ").arg(mAmiga.mM68K.mAddressRegister[7] + (j * 6 * 4), 8, 16);
		for(int i = 0; i < 6; ++i) {
			foo += QString("%1 ").arg(mAmiga.mM68K.readLong(mAmiga.mM68K.mAddressRegister[7] + (i * 4 + (j * 6 * 4))), 8, 16, QLatin1Char('0'));
		}
		ui.textEdit->append(foo);
	}


}

void Hexior::loadFile(QString filename, uint32_t offset) {
	QByteArray temp;
	QString foo = QDir::currentPath() + QDir::separator() + filename;
	QFile file(foo);
	file.open(QIODevice::ReadOnly);
	temp = file.readAll();
	for(int i = 0 ; i < temp.size() ; ++i) {
		mAmiga.mM68K.writeByte(offset + i, temp.at(i));
	}
	file.close();
}

Hexior::~Hexior() {
}
