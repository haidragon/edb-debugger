/*
Copyright (C) 2006 - 2015 Evan Teran
                          evan.teran@gmail.com

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

#include "FunctionFinder.h"
#include "edb.h"
#include "DialogFunctions.h"
#include <QMenu>

namespace FunctionFinderPlugin {

//------------------------------------------------------------------------------
// Name: FunctionFinder
// Desc:
//------------------------------------------------------------------------------
FunctionFinder::FunctionFinder(QObject *parent) : QObject(parent) {
}

//------------------------------------------------------------------------------
// Name: ~FunctionFinder
// Desc:
//------------------------------------------------------------------------------
FunctionFinder::~FunctionFinder() {
	delete dialog_;
}

//------------------------------------------------------------------------------
// Name: menu
// Desc:
//------------------------------------------------------------------------------
QMenu *FunctionFinder::menu(QWidget *parent) {

	Q_ASSERT(parent);

	if(!menu_) {
		menu_ = new QMenu(tr("FunctionFinder"), parent);
		menu_->addAction (tr("&Function Finder"), this, SLOT(show_menu()), QKeySequence(tr("Ctrl+Shift+F")));
	}

	return menu_;
}

//------------------------------------------------------------------------------
// Name: show_menu
// Desc:
//------------------------------------------------------------------------------
void FunctionFinder::show_menu() {

	if(!dialog_) {
		dialog_ = new DialogFunctions(edb::v1::debugger_ui);
	}

	dialog_->show();
}

}
