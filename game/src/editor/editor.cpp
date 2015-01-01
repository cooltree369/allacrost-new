///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    editor.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the map editor main window
*** ***************************************************************************/

#include <QGraphicsView>

#include "script.h"
#include "editor.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_script;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// Editor class -- public functions
///////////////////////////////////////////////////////////////////////////////

Editor::Editor() :
	QMainWindow(),
	_tiles_toolbar(NULL),
	_horizontal_splitter(NULL),
	_right_vertical_splitter(NULL),
	_map_view(NULL),
	_layer_view(NULL),
	_context_view(NULL),
	_tileset_tabs(NULL),
	_undo_stack(NULL),
	_file_menu(NULL),
	_edit_menu(NULL),
	_view_menu(NULL),
	_tools_menu(NULL),
	_help_menu(NULL),
	_new_action(NULL),
	_open_action(NULL),
	_save_action(NULL),
	_save_as_action(NULL),
	_close_action(NULL),
	_quit_action(NULL),
	_undo_action(NULL),
	_redo_action(NULL),
	_cut_action(NULL),
	_copy_action(NULL),
	_paste_action(NULL),
	_tileset_properties_action(NULL),
	_map_properties_action(NULL),
	_view_grid_action(NULL),
	_view_missing_action(NULL),
	_view_inherited_action(NULL),
	_view_collisions_action(NULL),
	_edit_mode_paint_action(NULL),
	_edit_mode_swap_action(NULL),
	_edit_mode_erase_action(NULL),
	_edit_mode_inherit_action(NULL),
	_edit_fill_action(NULL),
	_edit_clear_action(NULL),
	_toggle_select_action(NULL),
	_help_action(NULL),
	_about_action(NULL),
	_about_qt_action(NULL),
	_edit_mode_action_group(NULL)
{
	// Create and initialize the script engine that the editor code uses
	ScriptManager = ScriptEngine::SingletonCreate();
	ScriptManager->SingletonInitialize();

	// Create actions, menus, and toolbars
	_CreateActions();
	_CreateMenus();
	_CreateToolbars();

	_undo_stack = new QUndoStack();
	connect(_undo_stack, SIGNAL(canUndoChanged(bool)), _undo_action, SLOT(setEnabled(bool)));
	connect(_undo_stack, SIGNAL(canRedoChanged(bool)), _redo_action, SLOT(setEnabled(bool)));

	// Create each widget that forms the main window
	_horizontal_splitter = new QSplitter(this);
	_horizontal_splitter->setOrientation(Qt::Horizontal);
	setCentralWidget(_horizontal_splitter);
	_right_vertical_splitter = new QSplitter(_horizontal_splitter);
	_right_vertical_splitter->setOrientation(Qt::Vertical);

    _map_view = new MapView(_horizontal_splitter, &_map_data);
    _layer_view = new LayerView(&_map_data);
    _context_view = new ContextView(&_map_data);
    _tileset_tabs = new QTabWidget();
    _tileset_tabs->setTabPosition(QTabWidget::North);

    _horizontal_splitter->addWidget(_map_view->GetGraphicsView());
    _horizontal_splitter->addWidget(_right_vertical_splitter);
    _right_vertical_splitter->addWidget(_layer_view);
	_right_vertical_splitter->addWidget(_context_view);
    _right_vertical_splitter->addWidget(_tileset_tabs);

	// Size the window and each widget in it appropriately
	resize(1200, 800);
    QList<int> splitter_size;
    splitter_size << 660 << 540;
    _horizontal_splitter->setSizes(splitter_size);
	_horizontal_splitter->show();
    splitter_size.clear();
    splitter_size << 80 << 80 << 640;
    _right_vertical_splitter->setSizes(splitter_size);
	_right_vertical_splitter->show();

	setWindowIcon(QIcon("img/logos/program_icon.ico"));
	_ClearEditorState();
}



Editor::~Editor() {
	delete _new_action;
	delete _open_action;
	delete _save_action;
	delete _save_as_action;
	delete _close_action;
	delete _quit_action;

	delete _undo_action;
	delete _redo_action;
	delete _cut_action;
	delete _copy_action;
	delete _paste_action;
	delete _tileset_properties_action;
	delete _map_properties_action;

	delete _view_grid_action;
	delete _view_missing_action;
	delete _view_inherited_action;
	delete _view_collisions_action;

	delete _edit_mode_paint_action;
	delete _edit_mode_swap_action;
	delete _edit_mode_erase_action;
	delete _edit_mode_inherit_action;
	delete _edit_fill_action;
	delete _edit_clear_action;
	delete _toggle_select_action;
	delete _edit_mode_action_group;

	delete _help_action;
	delete _about_action;
	delete _about_qt_action;

	delete _map_view;
	delete _tileset_tabs;
	delete _layer_view;
	delete _context_view;

	delete _right_vertical_splitter;
	delete _horizontal_splitter;
	delete _undo_stack;

	ScriptEngine::SingletonDestroy();
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_CreateActions() {
	// Create actions found in the File menu
	_new_action = new QAction(QIcon("img/misc/editor_tools/new.png"), "&New...", this);
	_new_action->setShortcut(tr("Ctrl+N"));
	_new_action->setStatusTip("Create a new map");
	connect(_new_action, SIGNAL(triggered()), this, SLOT(_FileNew()));

	_open_action = new QAction(QIcon("img/misc/editor_tools/open.png"), "&Open...", this);
	_open_action->setShortcut(tr("Ctrl+O"));
	_open_action->setStatusTip("Open an existing map file");
	connect(_open_action, SIGNAL(triggered()), this, SLOT(_FileOpen()));

	_save_action = new QAction(QIcon("img/misc/editor_tools/save.png"), "&Save", this);
	_save_action->setShortcut(tr("Ctrl+S"));
	_save_action->setStatusTip("Save the map file");
	connect(_save_action, SIGNAL(triggered()), this, SLOT(_FileSave()));

	_save_as_action = new QAction(QIcon("img/misc/editor_tools/save_as.png"), "Save &As...", this);
	_save_as_action->setStatusTip("Save the map to a new file");
	connect(_save_as_action, SIGNAL(triggered()), this, SLOT(_FileSaveAs()));

	_close_action = new QAction(QIcon("img/misc/editor_tools/close.png"), "&Close", this);
	_close_action->setShortcut(tr("Ctrl+W"));
	_close_action->setStatusTip("Close the map");
	connect(_close_action, SIGNAL(triggered()), this, SLOT(_FileClose()));

	_quit_action = new QAction(QIcon("img/misc/editor_tools/exit.png"), "&Quit", this);
	_quit_action->setShortcut(tr("Ctrl+Q"));
	_quit_action->setStatusTip("Exit the application");
	connect(_quit_action, SIGNAL(triggered()), this, SLOT(_FileQuit()));

	// Create actions found in the Edit menu
	_undo_action = new QAction(QIcon("img/misc/editor_tools/undo.png"), "&Undo", this);
	_undo_action->setShortcut(tr("Ctrl+Z"));
	_undo_action->setStatusTip("Undo the previous command");
	connect(_undo_action, SIGNAL(triggered()), _undo_stack, SLOT(undo()));

	_redo_action = new QAction(QIcon("img/misc/editor_tools/redo.png"), "&Redo", this);
	_redo_action->setShortcut(tr("Ctrl+Y"));
	_redo_action->setStatusTip("Redo the next command");
	connect(_redo_action, SIGNAL(triggered()), _undo_stack, SLOT(redo()));

	_cut_action = new QAction(QIcon("img/misc/editor_tools/cut.png"), "Cu&t", this);
	_cut_action->setShortcut(tr("Ctrl+X"));
	_cut_action->setStatusTip("Cut the selected area");
	connect(_cut_action, SIGNAL(triggered()), this, SLOT(_CutSelection()));

	_copy_action = new QAction(QIcon("img/misc/editor_tools/copy.png"), "&Copy", this);
	_copy_action->setShortcut(tr("Ctrl+C"));
	_copy_action->setStatusTip("Copy the selected area");
	connect(_copy_action, SIGNAL(triggered()), this, SLOT(_CopySelection()));

	_paste_action = new QAction(QIcon("img/misc/editor_tools/paste.png"), "&Paste", this);
	_paste_action->setShortcut(tr("Ctrl+V"));
	_paste_action->setStatusTip("Paste the copied selection");
	connect(_paste_action, SIGNAL(triggered()), this, SLOT(_PasteSelection()));

	_tileset_properties_action = new QAction("Edit Tile&set Properties...", this);
	_tileset_properties_action->setStatusTip("Edits the properties of a chosen tileset file");
	connect(_tileset_properties_action, SIGNAL(triggered()), this, SLOT(_EditTilesetProperties()));

	_map_properties_action = new QAction("Edit &Map Properties...", this);
	_map_properties_action->setStatusTip("Modify the properties of the active map");
	connect(_map_properties_action, SIGNAL(triggered()), this, SLOT(_EditMapProperties()));

	// Create actions found in the View menu
	_view_grid_action = new QAction("Tile &Grid", this);
	_view_grid_action->setStatusTip("Toggles the display of the tile grid");
	_view_grid_action->setShortcut(tr("G"));
	_view_grid_action->setCheckable(true);
	connect(_view_grid_action, SIGNAL(triggered()), this, SLOT(_ViewTileGrid()));

	_view_missing_action = new QAction("&Missing Tiles", this);
	_view_missing_action->setStatusTip("Toggles the display of an overlay for all missing tiles on the selected tile layer");
	_view_missing_action->setShortcut(tr("M"));
	_view_missing_action->setCheckable(true);
	connect(_view_missing_action, SIGNAL(triggered()), this, SLOT(_ViewMissingTiles()));

	_view_inherited_action = new QAction("&Inherited Tiles", this);
	_view_inherited_action->setStatusTip("Toggles the display of an overlay for all inherited tiles on the selected tile layer");
	_view_inherited_action->setShortcut(tr("I"));
	_view_inherited_action->setCheckable(true);
	connect(_view_inherited_action, SIGNAL(triggered()), this, SLOT(_ViewInheritedTiles()));

	_view_collisions_action = new QAction("&Collision &Data", this);
	_view_collisions_action->setStatusTip("Shows which quadrants on the map have collisions enabled");
	_view_collisions_action->setShortcut(tr("C"));
	_view_collisions_action->setCheckable(true);
	connect(_view_collisions_action, SIGNAL(triggered()), this, SLOT(_ViewCollisionData()));

	// Create actions found in the Tools menu
	_edit_mode_paint_action = new QAction(QIcon("img/misc/editor_tools/pencil.png"), "&Paint Tiles", this);
	_edit_mode_paint_action->setShortcut(tr("P"));
	_edit_mode_paint_action->setStatusTip("Switches the edit mode to allowing painting of tiles to the map");
	_edit_mode_paint_action->setCheckable(true);
	connect(_edit_mode_paint_action, SIGNAL(triggered()), this, SLOT(_SelectPaintMode()));

	_edit_mode_swap_action = new QAction(QIcon("img/misc/editor_tools/arrow.png"), "S&wap Tiles", this);
	_edit_mode_swap_action->setShortcut(tr("W"));
	_edit_mode_swap_action->setStatusTip("Switches the edit mode to allowing swapping of tiles at different positions");
	_edit_mode_swap_action->setCheckable(true);
	connect(_edit_mode_swap_action, SIGNAL(triggered()), this, SLOT(_SelectSwapMode()));

	_edit_mode_erase_action = new QAction(QIcon("img/misc/editor_tools/eraser.png"), "&Erase Tiles", this);
	_edit_mode_erase_action->setShortcut(tr("E"));
	_edit_mode_erase_action->setStatusTip("Switches the edit mode to erase tiles from the map");
	_edit_mode_erase_action->setCheckable(true);
	connect(_edit_mode_erase_action, SIGNAL(triggered()), this, SLOT(_SelectEraseMode()));

	_edit_mode_inherit_action = new QAction(QIcon("img/misc/editor_tools/inherited.png"), "&Inherit Tiles", this);
	_edit_mode_inherit_action->setShortcut(tr("I"));
	_edit_mode_inherit_action->setStatusTip("Switches the edit mode to inherit tiles from the inherited context");
	_edit_mode_inherit_action->setCheckable(true);
	connect(_edit_mode_inherit_action, SIGNAL(triggered()), this, SLOT(_SelectInheritMode()));

	_edit_mode_action_group = new QActionGroup(this);
	_edit_mode_action_group->addAction(_edit_mode_paint_action);
	_edit_mode_action_group->addAction(_edit_mode_swap_action);
	_edit_mode_action_group->addAction(_edit_mode_erase_action);
	_edit_mode_action_group->addAction(_edit_mode_inherit_action);
	_edit_mode_paint_action->setChecked(true);

	_edit_fill_action = new QAction(QIcon("img/misc/editor_tools/fill.png"), "&Fill Area", this);
	_edit_fill_action->setStatusTip("Fills the selection area or tile area with the chosen tile(s)");
	connect(_edit_fill_action, SIGNAL(triggered()), this, SLOT(_FillArea()));

	_edit_clear_action = new QAction(QIcon("img/misc/editor_tools/clear.png"), "&Clear Area", this);
	_edit_clear_action->setStatusTip("Clears all tiles from the selection area or tile area");
	connect(_edit_clear_action, SIGNAL(triggered()), this, SLOT(_ClearArea()));

	_toggle_select_action = new QAction(QIcon("img/misc/editor_tools/selection_rectangle.png"), "&Select Area", this);
	_toggle_select_action->setShortcut(tr("S"));
	_toggle_select_action->setStatusTip("Select an area of tiles on the map");
	_toggle_select_action->setCheckable(true);
	connect(_toggle_select_action, SIGNAL(triggered()), this, SLOT(_ToggleSelectArea()));

	// Create actions found in the Help menu
	_help_action = new QAction("&Help", this);
	_help_action->setShortcut(Qt::Key_F1);
	_help_action->setStatusTip("Brings up help documentation for the editor");
	connect(_help_action, SIGNAL(triggered()), this, SLOT(_HelpHelp()));

	_about_action = new QAction("&About", this);
	_about_action->setStatusTip("Brings up information about the editor");
	connect(_about_action, SIGNAL(triggered()), this, SLOT(_HelpAbout()));

	_about_qt_action = new QAction("About &Qt", this);
	_about_qt_action->setStatusTip("Brings up information about Qt");
	connect(_about_qt_action, SIGNAL(triggered()), this, SLOT(_HelpAboutQt()));
} // void Editor::_CreateActions()



void Editor::_CreateMenus() {
	_file_menu = menuBar()->addMenu("&File");
	_file_menu->addAction(_new_action);
	_file_menu->addAction(_open_action);
	// TODO: add a "Recent Files" action and submenu here
	_file_menu->addSeparator();
	_file_menu->addAction(_save_action);
	_file_menu->addAction(_save_as_action);
	_file_menu->addSeparator();
	_file_menu->addAction(_close_action);
	_file_menu->addAction(_quit_action);
	connect(_file_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckFileActions()));

	_edit_menu = menuBar()->addMenu("&Edit");
	_edit_menu->addAction(_undo_action);
	_edit_menu->addAction(_redo_action);
	_edit_menu->addSeparator();
	_edit_menu->addAction(_cut_action);
	_edit_menu->addAction(_copy_action);
	_edit_menu->addAction(_paste_action);
	_edit_menu->addSeparator();
	_edit_menu->addAction(_tileset_properties_action);
	_edit_menu->addAction(_map_properties_action);
	connect(_edit_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckEditActions()));

	_view_menu = menuBar()->addMenu("&View");
	_view_menu->addAction(_view_grid_action);
	_view_menu->addAction(_view_missing_action);
	_view_menu->addAction(_view_inherited_action);
	_view_menu->addAction(_view_collisions_action);
	connect(_view_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckViewActions()));

	_tools_menu = menuBar()->addMenu("&Tools");
	_tools_menu->addAction(_edit_mode_paint_action);
	_tools_menu->addAction(_edit_mode_swap_action);
	_tools_menu->addAction(_edit_mode_erase_action);
	_tools_menu->addAction(_edit_mode_inherit_action);
	_tools_menu->addSeparator();
	_tools_menu->addAction(_edit_fill_action);
	_tools_menu->addAction(_edit_clear_action);
	_tools_menu->addSeparator();
	_tools_menu->addAction(_toggle_select_action);
	connect(_tools_menu, SIGNAL(aboutToShow()), this, SLOT(_CheckToolsActions()));

	_help_menu = menuBar()->addMenu("&Help");
	_help_menu->addAction(_help_action);
	_help_menu->addAction(_about_action);
	_help_menu->addAction(_about_qt_action);
}



void Editor::_CreateToolbars() {
	_tiles_toolbar = addToolBar("Tiles");
	_tiles_toolbar->addAction(_undo_action);
	_tiles_toolbar->addAction(_redo_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_edit_mode_paint_action);
	_tiles_toolbar->addAction(_edit_mode_swap_action);
	_tiles_toolbar->addAction(_edit_mode_erase_action);
	_tiles_toolbar->addAction(_edit_mode_inherit_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_edit_fill_action);
	_tiles_toolbar->addAction(_edit_clear_action);
	_tiles_toolbar->addSeparator();
	_tiles_toolbar->addAction(_toggle_select_action);
}



void Editor::_ClearEditorState() {
	_map_view->SetGridVisible(false);
	_map_view->SetSelectionOverlayVisible(false);
	_map_view->SetMissingOverlayVisible(false);
	_map_view->SetInheritedOverlayVisible(false);
	_map_view->SetEditMode(PAINT_MODE);

	_toggle_select_action->setChecked(false);
	_view_grid_action->setChecked(false);
	_view_missing_action->setChecked(false);
	_view_inherited_action->setChecked(false);

	_undo_stack->setClean();

	// Done so that the appropriate icons on the toolbar are enabled or disabled
	_CheckEditActions();
	_CheckToolsActions();

	// Update the visual display of each sub-widget
	_map_view->DrawMap();
	_layer_view->RefreshView();
	_context_view->RefreshView();
}



bool Editor::_UnsavedDataPrompt() {
	if (_map_data.IsInitialized() == false || _map_data.IsMapModified() == false)
		return true;

	switch (QMessageBox::warning(this, "Unsaved File", "The document contains unsaved changes.\n"
		"Do you want to save these changes before proceeding?", "&Save", "&Discard", "Cancel", 0, 2))
	{
		case 0: // Selected Save
			_FileSave();
			break;
		case 1: // Selected Discard
			break;
		case 2: // Selected Cancel
		default:
			statusBar()->showMessage("Abandoned save", 5000);
			return false;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Editor class -- private slot functions
///////////////////////////////////////////////////////////////////////////////

void Editor::_CheckFileActions() {
	if (_map_data.IsInitialized() == true) {
		_save_action->setEnabled(_map_data.IsMapModified());
		_save_as_action->setEnabled(true);
		_close_action->setEnabled(true);
	}
	else {
		_save_action->setEnabled(false);
		_save_as_action->setEnabled(false);
		_close_action->setEnabled(false);
	}
}



void Editor::_CheckEditActions() {
	// TODO: Currently tilesets can only be edited when no map is open. This is done because if the tileset data is modified,
	// the editor may be using a stale tileset definition when saving a map. In the future, tilesets should always be able
	// to be edited, and any modified tilesets in use by the map should be reloaded after the tileset file is saved.

	if (_map_data.IsInitialized() == true) {
		// TODO: Undo/Redo feature needs to be reimplemented. This option is disabled until that time
		_undo_action->setEnabled(false);
		_redo_action->setEnabled(false);
// 		_undo_action->setText("Undo " + _undo_stack->undoText());
// 		_redo_action->setText("Redo " + _undo_stack->redoText());
		// TODO: Cut/Copy/Paste feature has not yet been implemented. Options disabled until it becomes available
		_cut_action->setEnabled(false);
		_copy_action->setEnabled(false);
		_paste_action->setEnabled(false);
		_tileset_properties_action->setEnabled(false);
		_map_properties_action->setEnabled(true);
	}
	else {
		_undo_action->setEnabled(false);
		_redo_action->setEnabled(false);
		_cut_action->setEnabled(false);
		_copy_action->setEnabled(false);
		_paste_action->setEnabled(false);
		_tileset_properties_action->setEnabled(true);
		_map_properties_action->setEnabled(false);

	}
}


void Editor::_CheckViewActions() {
	if (_map_data.IsInitialized() == true) {
		_view_grid_action->setEnabled(true);
		_view_missing_action->setEnabled(true);
		_view_inherited_action->setEnabled(true);
		// TODO: View collision grid feature has not yet been implement. Option permanently disabled until it is
		_view_collisions_action->setEnabled(false);
	}
	else {
		_view_grid_action->setEnabled(false);
		_view_missing_action->setEnabled(false);
		_view_inherited_action->setEnabled(false);
		_view_collisions_action->setEnabled(false);
	}
}



void Editor::_CheckToolsActions() {
	if (_map_data.IsInitialized() == true) {
		_edit_mode_paint_action->setEnabled(true);
		_edit_mode_swap_action->setEnabled(true);
		_edit_mode_erase_action->setEnabled(true);
		_edit_mode_inherit_action->setEnabled(true);
		_edit_fill_action->setEnabled(true);
		_edit_clear_action->setEnabled(true);
		_toggle_select_action->setEnabled(true);
	}
	else {
		_edit_mode_paint_action->setEnabled(false);
		_edit_mode_swap_action->setEnabled(false);
		_edit_mode_erase_action->setEnabled(false);
		_edit_mode_inherit_action->setEnabled(false);
		_edit_fill_action->setEnabled(false);
		_edit_clear_action->setEnabled(false);
		_toggle_select_action->setEnabled(false);
	}
}



void Editor::_FileNew() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}

	// ---------- 1) Prompt the user with the dialog for them to enter the map properties
	MapPropertiesDialog* new_dialog = new MapPropertiesDialog(this, "new_dialog", false);
	if (new_dialog->exec() != QDialog::Accepted) {
		delete new_dialog;
		statusBar()->showMessage("New operation cancelled", 5000);
		return;
	}

	// ---------- 2) Initialize the map data and map view widget
	_map_data.DestroyData();
	_map_data.CreateData(new_dialog->GetLength(), new_dialog->GetHeight());

	// ---------- 3) Determine the number of tilesets that will be used by the new map and create a load progress dialog
	QTreeWidget* tilesets = new_dialog->GetTilesetTree();
	int32 num_tileset_items = tilesets->topLevelItemCount();
	int32 num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; ++i) {
		if (tilesets->topLevelItem(i)->checkState(0) == Qt::Checked)
			num_checked_items++;
	}

	// Used to show the progress of tilesets that have been loaded.
	QProgressDialog* load_tileset_progress = new QProgressDialog("Loading tilesets...", NULL, 0, num_checked_items, this,
		Qt::Widget | Qt::FramelessWindowHint | Qt::WindowTitleHint);
	load_tileset_progress->setWindowTitle("Creating Map...");

	// Set the location of the progress dialog and show it
	load_tileset_progress->move(this->pos().x() + this->width() / 2  - load_tileset_progress->width() / 2,
		this->pos().y() + this->height() / 2 - load_tileset_progress->height() / 2);
	load_tileset_progress->show();

	// ---------- 4) Load each tileset object
	num_checked_items = 0;
	for (int32 i = 0; i < num_tileset_items; i++) {
		if (tilesets->topLevelItem(i)->checkState(0) != Qt::Checked) {
			continue;
		}

		// Increment the progress dialog counter
		load_tileset_progress->setValue(num_checked_items++);

		// Load the tileset data from the definition file, add it to the map data, and create the TilesetTable to display it
		Tileset* tileset = new Tileset();
		QString filename = QString("lua/data/tilesets/") + tilesets->topLevelItem(i)->text(0) + (".lua");
		if (tileset->Load(filename) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to load tileset: " + filename);
			delete tileset;
		}

		if (_map_data.AddTileset(tileset) == false) {
			QMessageBox::critical(this, APP_NAME, "Failed to add tileset to map data: " + _map_data.GetErrorMessage());
			delete tileset;
		}

		TilesetTable* tileset_table = new TilesetTable(tileset);
		_tileset_tabs->addTab(tileset_table, tilesets->topLevelItem(i)->text(0));
	}

	// Clean up the editor state and report success
	load_tileset_progress->hide();
	delete load_tileset_progress;
	delete new_dialog;

	_ClearEditorState();
	statusBar()->showMessage("New map created", 5000);
} // void Editor::_FileNew()



void Editor::_FileOpen() {
	if (_UnsavedDataPrompt() == false) {
		statusBar()->showMessage("New operation cancelled due to existing unsaved map data.", 5000);
		return;
	}

	// ---------- 1) Attempt to open the file that the user requested
	QString filename = QFileDialog::getOpenFileName(this, APP_NAME + " -- Open Map File", "lua/data/maps", "Maps (*.lua)");
	if (filename.isEmpty() == true) {
		statusBar()->showMessage("No map file was opened (empty filename)", 5000);
		return;
	}

	// ---------- 2) Clear out any existing map data
	_map_data.DestroyData();
	for (uint32 i = 0; i < static_cast<uint32>(_tileset_tabs->count()); ++i) {
		delete _tileset_tabs->widget(i);
	}
	_tileset_tabs->clear();

	// ---------- 3) Load the map data and setup the TilesetTab widget with the loaded tileset data
	if (_map_data.LoadData(filename) == false) {
		QMessageBox::critical(this, APP_NAME, "Error while opening map file '" + filename + "'. Report errors:\n" + _map_data.GetErrorMessage());
		return;
	}

	vector<Tileset*> tilesets = _map_data.GetTilesets();
	QStringList tileset_names = _map_data.GetTilesetNames();
	for (uint32 i = 0; i < tilesets.size(); ++i) {
		_tileset_tabs->addTab(new TilesetTable(tilesets[i]), tileset_names[i]);
	}

	_ClearEditorState();
	statusBar()->showMessage(QString("Opened map \'%1\'").arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileSave() {
	if (_map_data.IsInitialized() == false) {
		return;
	}

	if (_map_data.SaveData() == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("%1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString("Saved \'%1\' successfully!").arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileSaveAs() {
	// Get the file name to save to from the user
	QString filename = QFileDialog::getSaveFileName(this, "Allacrost Map Editor -- File Save", "lua/data/maps", "Maps (*.lua)");

	if (filename.isEmpty() == true) {
		statusBar()->showMessage("Save abandoned.", 5000);
		return;
	}

	if (_map_data.IsInitialized() == false) {
		return;
	}

	if (_map_data.SaveData(filename) == false) {
		return;
	}

	_undo_stack->setClean();
	setWindowTitle(QString("%1").arg(_map_data.GetMapFilename()));
	statusBar()->showMessage(QString("Saved \'%1\' successfully!").arg(_map_data.GetMapFilename()), 5000);
}



void Editor::_FileClose() {
	if (_UnsavedDataPrompt() == false) {
		return;
	}

	_map_data.DestroyData();
	_ClearEditorState();
	setWindowTitle("Hero of Allacrost Map Editor");
}



void Editor::_FileQuit() {
	if (_UnsavedDataPrompt() == true)
		qApp->exit(0);
}



void Editor::_CutSelection() {
	// TODO: implement this feature
}



void Editor::_CopySelection() {
	// TODO: implement this feature
}



void Editor::_PasteSelection() {
	// TODO: implement this feature
}



void Editor::_EditTilesetProperties() {
	TilesetEditor* tileset_editor = new TilesetEditor(this);
	tileset_editor->exec();
	delete tileset_editor;
}



void Editor::_EditMapProperties() {
	MapPropertiesDialog* props = new MapPropertiesDialog(this, "map_properties", true);
	if (props->exec() != QDialog::Accepted) {
		statusBar()->showMessage("Map properties were not modified", 5000);
		delete props;
		return;
	}

	// TODO: adjust size of map appropriately
	// TODO: add or remove tilesets
	delete props;
}



void Editor::_ViewTileGrid() {
	_view_grid_action->setChecked(_map_view->ToggleGridVisible());
	_map_view->DrawMap();
}



void Editor::_ViewMissingTiles() {
	_view_grid_action->setChecked(_map_view->ToggleMissingOverlayVisible());
	_map_view->DrawMap();
}



void Editor::_ViewInheritedTiles() {
	_view_grid_action->setChecked(_map_view->ToggleInheritedOverlayVisible());
	_map_view->DrawMap();
}



void Editor::_ViewCollisionData() {
	// TODO: toggle this property in the _map_view class similar to _ViewTileGrid above
}



void Editor::_SelectPaintMode() {
	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(PAINT_MODE);
}



void Editor::_SelectSwapMode() {
	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(SWAP_MODE);
}



void Editor::_SelectEraseMode() {
	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(ERASE_MODE);
}



void Editor::_SelectInheritMode() {
	_map_view->ClearSelectionLayer();
	_map_view->SetEditMode(INHERIT_MODE);
}




void Editor::_FillArea() {
	// TODO: if selection area is active, fill to selection. Otherwise fill the layer.

	// TODO: fetch the currently selected tile/tileset and fill the entire layer with that tile for the active tile context

	// TODO: perform autotile randomization for the tile when available

	// TODO: record map data for undo/redo stack

	// Draw the changes.
	_map_data.SetMapModified(true);
	_map_view->DrawMap();
}



void Editor::_ClearArea() {
	// TODO: if selection area is active, clear the selection. Otherwise clear the layer.

	// TODO: fetch the current tile layer and clear it only for the active tile context

	// TODO: record map data for undo/redo stack

	// Draw the changes.
	_map_data.SetMapModified(true);
	_map_view->DrawMap();
}



void Editor::_InheritArea() {
	// TODO: if selection area is active, inherit the selection. Otherwise clear the layer.

	// TODO: fetch the current tile layer and inherit it only for the active tile context

	// TODO: record map data for undo/redo stack

	// Draw the changes.
	_map_data.SetMapModified(true);
	_map_view->DrawMap();
}



void Editor::_ToggleSelectArea() {
	bool selection = _map_view->ToggleSelectionOverlayVisible();
	_toggle_select_action->setChecked(selection);
}



void Editor::_HelpHelp() {
    QMessageBox::about(this, "Hero of Allacrost Map Editor -- Help",
		"<p>In-editor documentation is not yet available. Please visit http://wiki.allacrost.org for available documentation.</p>");
}



void Editor::_HelpAbout() {
    QMessageBox::about(this, "Hero of Allacrost Map Editor -- About",
		"<center><h2>Hero of Allacrost Map Editor</h2></center>"
		"<center><h3>Copyright 2004-2015</h3></center>"
		"<p>A map editor created for the Hero of Allacrost project. See 'http://www.allacrost.org/' for more information</p>");
}



void Editor::_HelpAboutQt() {
    QMessageBox::aboutQt(this, "Hero of Allacrost Map Editor -- About QT");
}

///////////////////////////////////////////////////////////////////////////////
// EditTileCommand class
///////////////////////////////////////////////////////////////////////////////

EditTileCommand::EditTileCommand(const QString& action_text, QUndoCommand* parent) :
	QUndoCommand(action_text, parent)
{}



void EditTileCommand::undo() {
	// TODO: restore tiles / map size and redraw the map view
}



void EditTileCommand::redo() {
	// TODO: restore tiles / map size and redraw the map view
}

} // namespace hoa_editor
