/*
Copyright (C) 2015 Ruslan Kabatsayev <b7.10110111@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DialogEditFPU.h"
#include "Float80Edit.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QRegExp>
#include <QVBoxLayout>
#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "EntryGridKeyUpDownEventFilter.h"

namespace ODbgRegisterView {
namespace {

long double readFloat(const QString &strInput, bool &ok) {
	ok = false;
	const QString str(strInput.toLower().trimmed());

	if(const auto value = util::fullStringToFloat<long double>(str.toStdString())) {
		ok = true;
		return *value;
	}

	// OK, so either it is invalid/unfinished, or it's some special value
	// We still do want the user to be able to enter common special values
	long double value;

	static std::array<std::uint8_t, 10> positiveInf{0, 0, 0, 0, 0, 0, 0, 0x80, 0xff, 0x7f};
	static std::array<std::uint8_t, 10> negativeInf{0, 0, 0, 0, 0, 0, 0, 0x80, 0xff, 0xff};
	static std::array<std::uint8_t, 10> positiveSNaN{0, 0, 0, 0, 0, 0, 0, 0x90, 0xff, 0x7f};
	static std::array<std::uint8_t, 10> negativeSNaN{0, 0, 0, 0, 0, 0, 0, 0x90, 0xff, 0xff};

	// Indefinite values are used for QNaN
	static std::array<std::uint8_t, 10> positiveQNaN{0, 0, 0, 0, 0, 0, 0, 0xc0, 0xff, 0x7f};
	static std::array<std::uint8_t, 10> negativeQNaN{0, 0, 0, 0, 0, 0, 0, 0xc0, 0xff, 0xff};

	if (str == "+snan" || str == "snan")
		std::memcpy(&value, &positiveSNaN, sizeof(value));
	else if (str == "-snan")
		std::memcpy(&value, &negativeSNaN, sizeof(value));
	else if (str == "+qnan" || str == "qnan" || str == "nan")
		std::memcpy(&value, &positiveQNaN, sizeof(value));
	else if (str == "-qnan")
		std::memcpy(&value, &negativeQNaN, sizeof(value));
	else if (str == "+inf" || str == "inf")
		std::memcpy(&value, &positiveInf, sizeof(value));
	else if (str == "-inf")
		std::memcpy(&value, &negativeInf, sizeof(value));
	else
		return 0;

	ok = true;
	return value;
}
}

DialogEditFPU::DialogEditFPU(QWidget *parent) : QDialog(parent), floatEntry(new ODbgRegisterView::Float80Edit(this)), hexEntry(new QLineEdit(this)) {

	setWindowTitle(tr("Modify Register"));
	setModal(true);
	const auto allContentsGrid = new QGridLayout();

	allContentsGrid->addWidget(new QLabel(tr("Float"), this), 0, 0);
	allContentsGrid->addWidget(new QLabel(tr("Hex"), this), 1, 0);

	allContentsGrid->addWidget(floatEntry, 0, 1);
	allContentsGrid->addWidget(hexEntry, 1, 1);

	connect(floatEntry, &Float80Edit::textEdited, this, &DialogEditFPU::onFloatEdited);
	connect(hexEntry,   &QLineEdit::textEdited,   this, &DialogEditFPU::onHexEdited);

	hexEntry->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F ]{,20}"), this));
	connect(floatEntry, &Float80Edit::defocussed, this, &DialogEditFPU::updateFloatEntry);

	hexEntry->installEventFilter(this);
	floatEntry->installEventFilter(this);

	const auto okCancel = new QDialogButtonBox(this);
	okCancel->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	connect(okCancel, &QDialogButtonBox::accepted, this, &DialogEditFPU::accept);
	connect(okCancel, &QDialogButtonBox::rejected, this, &DialogEditFPU::reject);

	const auto dialogLayout = new QVBoxLayout(this);
	dialogLayout->addLayout(allContentsGrid);
	dialogLayout->addWidget(okCancel);

	setTabOrder(floatEntry, hexEntry);
	setTabOrder(hexEntry, okCancel);
}

void DialogEditFPU::updateFloatEntry() {
	floatEntry->setValue(value_);
}

void DialogEditFPU::updateHexEntry() {
	hexEntry->setText(value_.toHexString());
}

bool DialogEditFPU::eventFilter(QObject* obj, QEvent* event) {
	return entryGridKeyUpDownEventFilter(this,obj,event);
}

void DialogEditFPU::set_value(const Register &newReg) {
	reg    = newReg;
	value_ = reg.value<edb::value80>();
	updateFloatEntry();
	updateHexEntry();
	setWindowTitle(tr("Modify %1").arg(reg.name().toUpper()));
	floatEntry->setFocus(Qt::OtherFocusReason);
}

Register DialogEditFPU::value() const {
	Register ret(reg);
	ret.setValueFrom(value_);
	return ret;
}

void DialogEditFPU::onHexEdited(const QString &input) {
	QString readable(input.trimmed());
	readable.replace(' ', "");

	while (readable.size() < 20) {
		readable = '0' + readable;
	}

	const auto byteArray = QByteArray::fromHex(readable.toLatin1());
	auto       source    = byteArray.constData();
	auto       dest      = reinterpret_cast<unsigned char *>(&value_);

	for (std::size_t i = 0; i < sizeof(value_); ++i) {
		dest[i]        = source[sizeof(value_) - i - 1];
	}

	updateFloatEntry();
}

void DialogEditFPU::onFloatEdited(const QString &str) {
	bool       ok;
	const auto value = readFloat(str, ok);

	if (ok) {
		value_ = edb::value80(value);
	}

	updateHexEntry();
}

}
