///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    dialog_boxes.h
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Header file for all of editor's dialog boxes.
 *****************************************************************************/

#ifndef __DIALOG_BOXES_HEADER__
#define __DIALOG_BOXES_HEADER__

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>

#include "editor.h"

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

class MapPropertiesDialog: public QDialog
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT
	
	public:
		//! \name MapPropertiesDialog constructor
		//! \brief A constructor for the MapPropertiesDialog class. This class is used in 2 instances:
		//!        for presenting a dialog to the user to (1) create a new map or (2) modify the properties
		//!        (such as height, width, or tilesets loaded in the bottom portion of the editor) of an
		//!        already existing map. For case #1, the parameter prop is false, and for case #2, it is true.
		//! \param parent The widget from which this dialog was invoked.
		//! \param name The name of this widget.
		//! \param prop True when accessing an already loaded map's properties, false otherwise.
		MapPropertiesDialog(QWidget* parent, const QString& name, bool prop);
		~MapPropertiesDialog();               // destructor

		//! Public accessor to get the map height from the height spinbox.
		int GetHeight() const { return _height_sbox->value(); }
		//! Public accessor to get the map width from the width spinbox.
		int GetWidth()  const { return  _width_sbox->value(); }
		//! Public accessor to get the tree containing checkable tilesets.
		QTreeWidget* GetTilesetTree() const { return _tileset_tree; }
		
		//! Needed for accessing map properties.
		friend class Editor;
		friend class EditorScrollView;

	private slots:
		//! This slot enables or disables the OK push button of this dialog depending
		//! on whether any tilesets are checked or not.
		void _EnableOKButton();

	private:
		//! A tree for showing all available tilesets.
		QTreeWidget* _tileset_tree;
		//! A spinbox for specifying the map's height.
		QSpinBox* _height_sbox;
		//! A spinbox for specifying the map's width.
		QSpinBox* _width_sbox;
		//! A label used to visually name the height spinbox.
		QLabel* _height_label;
		//! A label used to visually name the width spinbox.
		QLabel* _width_label;
		//! A pushbutton for canceling the new map dialog.
		QPushButton* _cancel_pbut;
		//! A pushbutton for okaying the new map dialog.
		QPushButton* _ok_pbut;
		//! A layout to manage all the labels, spinboxes, and listviews.
		QGridLayout* _dia_layout;
}; // class MapPropertiesDialog

class MusicDialog: public QDialog
{
public:
	MusicDialog(QWidget* parent, const QString& name, const QString& selected_music);
	~MusicDialog();

	QString GetSelectedFile();
private:
	//! A pushbutton for canceling the new map dialog.
	QPushButton* _cancel_pbut;
	//! A pushbutton for okaying the new map dialog.
	QPushButton* _ok_pbut;
	//! Label telling you to select some music.
	QLabel* _select_label;
	//! A layout to manage all the labels, spinboxes, and listviews.
	QGridLayout* _dia_layout;
	//! A tree with all the music files.
	QTreeWidget* _music_list;

	//! Puts music files in the QTreeWidget and selects the specified file.
	//! \param selected_str - this file will be selected
	void _PopulateMusicList(const QString& selected_str);
}; // class MusicDialog

class ContextPropertiesDialog: public QDialog
{
	//! Macro needed to use Qt's slots and signals.
	Q_OBJECT
	
	public:
		//! \name ContextPropertiesDialog constructor
		//! \brief A constructor for the ContextPropertiesDialog class. This class is used when creating
		//!        a new context. The user can give it a name and specify which context to possibly inherit.
		//! \param parent The widget from which this dialog was invoked.
		//! \param name The name of this widget.
		ContextPropertiesDialog(QWidget* parent, const QString& name);
		~ContextPropertiesDialog();               // destructor

		//! Public accessor to get the context's name from the line edit.
		QString GetName() const { return _name_ledit->text(); }
		//! Public accessor to get the tree containing existing contexts.
		QTreeWidget* GetContextTree() const { return _context_tree; }
		
		//! Needed for accessing map properties.
		friend class Editor;
		friend class EditorScrollView;

	private slots:
		//! This slot enables or disables the OK push button of this dialog depending
		//! on whether the line edit is empty.
		void _EnableOKButton();

	private:
		//! A tree for showing all available contexts.
		QTreeWidget* _context_tree;
		//! A label used to visually name the line edit.
		QLabel* _name_label;
		//! A line edit for entering in the context's name.
		QLineEdit* _name_ledit;
		//! A pushbutton for canceling the context dialog.
		QPushButton* _cancel_pbut;
		//! A pushbutton for okaying the context dialog.
		QPushButton* _ok_pbut;
		//! A layout to manage all the labels, buttons, and line edits.
		QGridLayout* _dia_layout;
}; // class ContextPropertiesDialog

} // namespace hoa_editor

#endif
// __DIALOG_BOXES_HEADER__
