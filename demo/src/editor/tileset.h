///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    tileset.h
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Header file for editor's tileset, used for maintaining a visible
***          "list" of tiles to select from for painting on a map.
*** ***************************************************************************/

#ifndef __TILESET_HEADER__
#define __TILESET_HEADER__

//#include <QHeaderView>
#include <QImageReader>
#include <QMessageBox>
#include <QRect>
#include <Q3Table>
//#include <QTableWidgetItem>
#include <QVariant>

#include "defs.h"
#include "script.h"
#include "utils.h"
#include "video.h"

//! All calls to the editor are wrapped in this namespace.
namespace hoa_editor
{

/*	struct AnimatedTileData {
		int tile_id;
		int time;
	};
*/
//! \briefStandard tile dimensions in number of pixels
//@{
const unsigned int TILE_WIDTH  = 32;
const unsigned int TILE_HEIGHT = 32;
//@}

/** ****************************************************************************
*** \brief Represents a tileset and retains the tileset's image and properties
***
*** This is a container of tileset data. The tileset's properties are contained
*** within a Lua file specific to the tileset. The Lua file is located in a
*** separate path from the tileset's image file. Currently this class assumes
*** and only supports the Allacrost standard tileset of 512x512 pixels with
*** 32x32 pixel tiles (256 total tiles in one tileset file).
***
*** \todo Re-examine the need for these odd container choices (maps of vectors)
*** \todo Add support for animated tiles (display, editing)
*** \todo Add support for creating new tilesets (new Lua data files)
*** \todo Add support for saving tileset files
*** ***************************************************************************/
class Tileset
{
public:
	Tileset();

	~Tileset();

	bool IsInitialized() const
		{ return _initialized; }

	/** \brief Creates a new tileset object using only a tileset image
	*** \param img_filename The path + name of the image file to use for the tileset
	*** \return True if the tileset image was loaded successfully
	*** \note A tileset image is required to use this function, but nothing else
	**/
// 	virtual bool New(const QString& img_filename);

	/** \brief Loads the tileset definition file and stores its data in the class containers
	*** \param set_name The unique name that identifies the tileset (not a filename)
	*** \param one_image If true, the tiles vector will contain a single image for the entire tileset
	*** \return True if the tileset was loaded successfully
	*** \note This function will clear the previously loaded contents when it is called
	**/
	virtual bool Load(const QString& set_name, bool one_image = false);

	//void Save();

	//! \brief The name of the tileset this table is representing.
	QString tileset_name;

	//! \brief Contains the StillImage tiles of the tileset, used in grid.cpp.
	std::vector<hoa_video::StillImage> tiles;

	//! \brief Contains walkability information for each tile.
	std::map<int, std::vector<int32> > walkability;

	//! \brief Contains autotiling information for any autotileable tile.
	std::map<int, std::string> autotileability;

protected:
	//! \brief True if the class is holding valid, loaded tileset data
	bool _initialized;

	//std::vector<std::vector<AnimatedTileData> > _animated_tiles;
}; // class Tileset

/** ****************************************************************************
*** \brief Used to visually represent a tileset via a QT table
*** ***************************************************************************/
class TilesetTable : public Tileset {
public:
	TilesetTable();

	~TilesetTable();

	//! \note Inherited methods from Tileset class that need to be overridden
	//@{
// 	bool New(const QString& img_filename);
	bool Load(const QString& set_name);
	//@}

	//! Reference to the table implementation of this tileset
	Q3Table* table;
}; // class TilesetTable : public Tileset

} // namespace hoa_editor

#endif // __TILESET_HEADER__
