#include "hexior.h"
#include <qfile.h>
#include <qdir.h>

Hexior::Hexior(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);


	for(int j = 0; j < 6; ++j) {
		QString foo;
		foo = QString("%1: ").arg(0x9000 + (j * 6 * 4), 8, 16);
		for(int i = 0; i < 6; ++i) {
			foo += QString("%1 ").arg(mAmiga.mM68K.readLong(0x9000 + (i * 4 + (j * 6 * 4))), 8, 16, QLatin1Char('0'));
		}
		ui.textEdit->append(foo);
	}


}

//int validateEA(uint8_t ea, char *valid) {  /* EA = MMMRRR (mode/reg) */
//	uint8_t mode	= (ea >> 3) & 7;
//	uint8_t reg		= (ea) & 7;
//
//	if(mode != 7) {
//		if(valid[mode] == '1') return true;
//	} else {
//		if(reg >= 5) {
//			return false;
//		} else {
//			if(reg == 2)			reg = 3;
//			else if(reg == 3) reg = 4;
//			else if(reg == 4) reg = 2;
//			if(valid[8 + reg] == '1') return true;
//		}
//	}
//
//	return false;
//};

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

	//for(int i = 0; i <= 7; ++i) {
	//	for(int j = 0; j <= 0x3f; ++j) {
	//		if (!validateEA(j, "101111111000")) continue;
	//		ui.textEdit->append(QString("%1").arg((i << 9) + j + 0x140, 4, 16, QLatin1Char('0')));
	//	}
	//}
}

Hexior::~Hexior() {
}
