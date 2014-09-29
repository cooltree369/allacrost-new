///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    dialog_boxes.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Source file for all of the editor's dialog boxes.
*** **************************************************************************/

#include "dialog_boxes.h"

namespace hoa_editor {

// The limits to the size dimensions of a map, in number of tiles
const int32 MINIMUM_MAP_HEIGHT  = 24;
const int32 MAXIMUM_MAP_HEIGHT  = 1000;
const int32 MINIMUM_MAP_WIDTH   = 32;
const int32 MAXIMUM_MAP_WIDTH   = 1000;

///////////////////////////////////////////////////////////////////////////////
// MapPropertiesDialog class
///////////////////////////////////////////////////////////////////////////////

MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name, bool prop) :
	QDialog(parent, (const char*)name)
{
	setWindowTitle("Map Properties...");

	// Set up the height spinbox
	_height_label = new QLabel("Height (in tiles):", this);
	_height_sbox = new QSpinBox(this);
	_height_sbox->setMinimum(MINIMUM_MAP_HEIGHT);
	_height_sbox->setMaximum(MAXIMUM_MAP_HEIGHT);

	// Set up the width spinbox
	_width_label = new QLabel(" Width (in tiles):", this);
	_width_sbox = new QSpinBox(this);
	_width_sbox->setMinimum(MINIMUM_MAP_WIDTH);
	_width_sbox->setMaximum(MAXIMUM_MAP_WIDTH);

	// Get the existing map's height and width and set the spin boxes to those values
	if (prop) {
		Editor* editor = static_cast<Editor*>(parent);
		_height_sbox->setValue(editor->_ed_scrollview->_map->GetHeight());
		_width_sbox->setValue(editor->_ed_scrollview->_map->GetWidth());
	}

	// Set up the cancel and okay push buttons
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);

	// At construction no tilesets are checked, so disable the ok button
	_ok_pbut->setEnabled(false);
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of selectable tilesets
	QDir tileset_dir("lua/data/tilesets");
	_tileset_tree = new QTreeWidget(this);
	_tileset_tree->setColumnCount(1);
	_tileset_tree->setHeaderLabels(QStringList("Tilesets"));
	connect(_tileset_tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(_EnableOKButton()));

	// Loop through all files found in the tileset definition directory (lua/data/tilesets).
	// Start the loop at 2 to skip over the present and parent working directories ("." and "..")
	QList<QTreeWidgetItem*> tilesets;
	for (uint32 i = 2; i < tileset_dir.count(); i++) {
		// Exclude the file autotiling.lua, as it's not a tileset defition files
		if (tileset_dir[i] == QString("autotiling.lua"))
			continue;

		tilesets.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(tileset_dir[i].remove(".lua"))));
		tilesets.back()->setCheckState(0, Qt::Unchecked); // enables checkboxes

		// Indicates that the user wants to edit the map's existing properties
		if (prop) {
			// Get a reference to the Editor
			Editor* editor = static_cast<Editor*>(parent);

			// Iterate through the names of the tabs to see which ones in the list are already present
			// and set their checkbox appropriately
			for (int j = 0; j < editor->_ed_tabs->count(); j++) {
				// Both tileset names must match to set the checkbox
				if (tilesets.back()->text(0) == editor->_ed_tabs->tabText(j)) {
					tilesets.back()->setCheckState(0, Qt::Checked);
					_ok_pbut->setEnabled(true);
					break;
				}
			}
		}
	}
	_tileset_tree->insertTopLevelItems(0, tilesets);

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_height_label, 0, 0);
	_dia_layout->addWidget(_height_sbox,  1, 0);
	_dia_layout->addWidget(_width_label,  2, 0);
	_dia_layout->addWidget(_width_sbox,   3, 0);
	_dia_layout->addWidget(_tileset_tree, 0, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut,  6, 0);
	_dia_layout->addWidget(_ok_pbut,      6, 1);
} // MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, const QString& name, bool prop)



MapPropertiesDialog::~MapPropertiesDialog() {
	delete _tileset_tree;
	delete _height_label;
	delete _height_sbox;
	delete _width_label;
	delete _width_sbox;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _dia_layout;
}



void MapPropertiesDialog::_EnableOKButton() {
	// Disable the ok button if no tilesets are checked, otherwise enable it
	int num_items = _tileset_tree->topLevelItemCount();
	for (int i = 0; i < num_items; i++) {
		// At least one tileset must be checked in order to enable push button
		if (_tileset_tree->topLevelItem(i)->checkState(0) == Qt::Checked) {
			_ok_pbut->setEnabled(true);
			return;
		}
	}

	// If this point is reached, no tilesets are checked.
	_ok_pbut->setEnabled(false);
}

///////////////////////////////////////////////////////////////////////////////
// LayerDialog class
///////////////////////////////////////////////////////////////////////////////

LayerDialog::LayerDialog(QWidget* parent, const QString& name)
    : QDialog(parent)
{
	setWindowTitle("Layer Properties");

	// Set up the push buttons
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_ok_pbut->setDefault(true);
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	_name_label = new QLabel("Layer name: ", this);
	_name_edit = new QLineEdit(this);
	_name_label->setBuddy(_name_edit);

	_type_label = new QLabel("Type: ", this);
	_type_cbox = new QComboBox(this);
	_type_label->setBuddy(_type_cbox);

	// Add the possible layer types
	_type_cbox->addItem("Ground");
	_type_cbox->addItem("Sky");

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dialog_layout = new QGridLayout(this);
	_dialog_layout->addWidget(_name_label,   0, 0);
	_dialog_layout->addWidget(_name_edit,    1, 0);
	_dialog_layout->addWidget(_type_label,   0, 1);
	_dialog_layout->addWidget(_type_cbox,    1, 1);
	_dialog_layout->addWidget(_cancel_pbut,  2, 0);
	_dialog_layout->addWidget(_ok_pbut,      2, 1);
} // LayerDialog::LayerDialog(QWidget* parent, const QString& name)



LayerDialog::~LayerDialog() {
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _name_label;
	delete _type_label;
	delete _name_edit;
	delete _type_cbox;
	delete _dialog_layout;
}



// LayerInfo LayerDialog::_GetLayerInfo() {
// 	LayerInfo layer_info;
//
// 	layer_info.name = _name_edit->text().toStdString();
// 	layer_info.layer_type = getLayerType(_type_cbox->currentText().toStdString());
// 	return layer_info;
// }

////////////////////////////////////////////////////////////////////////////////
// ContextPropertiesDialog class
////////////////////////////////////////////////////////////////////////////////

ContextPropertiesDialog::ContextPropertiesDialog(QWidget* parent, const QString& name) :
	QDialog(parent, (const char*)name)
{
	setWindowTitle("Context Properties");
	_name_label = new QLabel("Context name", this);
	_name_ledit = new QLineEdit(this);
	connect(_name_ledit, SIGNAL(textEdited(const QString&)), this, SLOT(_EnableOKButton()));

	// Set up the cancel and okay push buttons
	_cancel_pbut = new QPushButton("Cancel", this);
	_ok_pbut = new QPushButton("OK", this);
	_cancel_pbut->setDefault(true);
	// At construction nothing has been entered so disable the ok button
	_ok_pbut->setEnabled(false);
	connect(_ok_pbut, SIGNAL(released()), this, SLOT(accept()));
	connect(_cancel_pbut, SIGNAL(released()), this, SLOT(reject()));

	// Set up the list of inheritable contexts
	_context_tree = new QTreeWidget(this);
	_context_tree->setColumnCount(1);
	_context_tree->setHeaderLabels(QStringList("Inherit from context:"));
	QList<QTreeWidgetItem*> contexts;

	// Get a reference to the Editor
	Editor* editor = static_cast<Editor*>(parent);

	// Loop through all files that are present in the tileset directory
	QStringList context_names = editor->_ed_scrollview->_map->context_names;
	for (QStringList::Iterator qit = context_names.begin(); qit != context_names.end(); ++qit)
		contexts.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(*qit)));
	_context_tree->insertTopLevelItems(0, contexts);

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout = new QGridLayout(this);
	_dia_layout->addWidget(_name_label,   0, 0);
	_dia_layout->addWidget(_name_ledit,   0, 1);
	_dia_layout->addWidget(_context_tree, 1, 1, 5, -1);
	_dia_layout->addWidget(_cancel_pbut,  6, 0);
	_dia_layout->addWidget(_ok_pbut,      6, 1);
} // ContextPropertiesDialog constructor



ContextPropertiesDialog::~ContextPropertiesDialog() {
	delete _name_label;
	delete _name_ledit;
	delete _cancel_pbut;
	delete _ok_pbut;
	delete _context_tree;
	delete _dia_layout;
}



void ContextPropertiesDialog::_EnableOKButton() {
	// Disable the ok button if the line edit is empty. The default inheritable context is the base context.
	if (_name_ledit->text() == "")
		_ok_pbut->setEnabled(false);
	else
		_ok_pbut->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
// MusicDialog class
///////////////////////////////////////////////////////////////////////////////

MusicDialog::MusicDialog(QWidget* parent, const QString& name) :
	QDialog(parent, name)
{
	setWindowTitle("Map Music Selector");
	_dia_layout = new QGridLayout(this);


	// Set up the push buttons
	_add_pbut = new QPushButton("Add to map", this);
	_remove_pbut = new QPushButton("Remove from map", this);
	_ok_pbut = new QPushButton("OK", this);
	_ok_pbut->setDefault(true);
	connect(_ok_pbut,     SIGNAL(released()), this, SLOT(accept()));
	connect(_add_pbut,    SIGNAL(released()), this, SLOT(_AddMusic()));
	connect(_remove_pbut, SIGNAL(released()), this, SLOT(_RemoveMusic()));

	_available_label = new QLabel("Available Music", this);
	_used_label = new QLabel("Used by Map", this);

	// Set up the list of already used music files
	_used_music_list = new QListWidget(this);
	_used_music_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Add all files currently used by the map to the use list
	Editor* editor = static_cast<Editor*>(parent);
	QStringList used_music_files = editor->_ed_scrollview->_map->music_files;
	for (QStringList::Iterator qit = used_music_files.begin();  qit != used_music_files.end(); ++qit)
		_used_music_list->addItem(*qit);

	// Set up the list of available, remaining music files
	_available_music_list = new QListWidget(this);
	_available_music_list->setSortingEnabled(true);
	_available_music_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	QDir music_dir("mus");

	// Add music files to the available list
	for (unsigned int i = 0; i < music_dir.count(); i++) {
		// Only look for *.ogg files that are not already used by the map
		if (music_dir[i].contains(".ogg") &&
			_used_music_list->findItems(music_dir[i], Qt::MatchContains).empty())
			_available_music_list->addItem(music_dir[i]);
	}

	// Add all of the aforementioned widgets into a nice-looking grid layout
	_dia_layout->addWidget(_available_label,      0, 0);
	_dia_layout->addWidget(_available_music_list, 1, 0);
	_dia_layout->addWidget(_used_label,           0, 1);
	_dia_layout->addWidget(_used_music_list,      1, 1);
	_dia_layout->addWidget(_add_pbut,             2, 0);
	_dia_layout->addWidget(_remove_pbut,          2, 1);
	_dia_layout->addWidget(_ok_pbut,              3, 1);
} // MusicDialog::MusicDialog(QWidget* parent, const QString& name)



MusicDialog::~MusicDialog() {
	delete _add_pbut;
	delete _remove_pbut;
	delete _ok_pbut;
	delete _available_label;
	delete _used_label;
	delete _available_music_list;
	delete _used_music_list;
	delete _dia_layout;
}



void MusicDialog::_AddMusic() {
	// Add all selected files to the used list
	QList<QListWidgetItem*> files = _available_music_list->selectedItems();
	for (QList<QListWidgetItem*>::Iterator qit = files.begin(); qit != files.end(); ++qit) {
		_used_music_list->addItem((*qit)->text());
		_available_music_list->takeItem(_available_music_list->row(*qit));
	}
}



void MusicDialog::_RemoveMusic() {
	// Remove all selected files from the used list
	QList<QListWidgetItem*> files = _used_music_list->selectedItems();
	for (QList<QListWidgetItem*>::Iterator qit = files.begin(); qit != files.end(); ++qit) {
		_available_music_list->addItem((*qit)->text());
		_used_music_list->takeItem(_used_music_list->row(*qit));
	}
}

} // namespace hoa_editor
