///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tex_mgmt.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for texture management code
 *
 * We use texture management so that at runtime, we can load many small
 * images (e.g. tiles) and stick them together into larger textures called
 * "texture sheets". This improves performance, because then we don't have to
 * constantly switch textures while rendering.
 *
 * This file contains several classes:
 *
 * TexSheet: physically represents an OpenGL texture in memory, plus a pointer
 *           to a texture management class which keeps track of which images
 *           are in that texture sheet.
 *
 * TexMemMgr: abstract base class, used by TexSheet. The job of a texture
 *            memory manager is to allocate sub-rectangles within the sheet
 *            so we can stuff images into it.
 *
 * FixedTexMemMgr: texture memory manager for fixed-size images, i.e. 32x32.
 *                 This class can do all its operations in constant (O(1)) time
 *                 because it knows in advance that all textures are the same size
 *
 * FixedImageNode: this represents a single allocation within the fixed-size
 *                 texture memory manager
 *
 * VariableTexMemMgr: this manages memory for any size images. This works fairly
 *                    well in practice, but it generally has a fair amount of wasted
 *                    space, because the images don't always "fit together" perfectly
 *                    on the sheet.
 *
 * VariableImageNode: a single alloaction within the variable-size texture memory
 *                    manager
 *****************************************************************************/ 


#ifndef __TEX_MGMT_HEADER__
#define __TEX_MGMT_HEADER__

#include "defs.h"
#include "utils.h"

// OpenGL includes
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

namespace hoa_video
{

namespace private_video
{


/*!***************************************************************************
 *  \brief Represents the different image sizes that a texture sheet can hold
 *****************************************************************************/

enum TexSheetType
{
	VIDEO_TEXSHEET_INVALID = -1,
	
	VIDEO_TEXSHEET_32x32 = 0,
	VIDEO_TEXSHEET_32x64 = 1,
	VIDEO_TEXSHEET_64x64 = 2,
	VIDEO_TEXSHEET_ANY = 3,
	
	VIDEO_TEXSHEET_TOTAL = 4
};


/*!***************************************************************************
 *  \brief base class for texture memory managers. It is used by TextureSheet
 *         to manage which areas of the texture are available and which are used.
 *****************************************************************************/

class TexMemMgr
{
public:
	
	virtual ~TexMemMgr() {}

	/*!******************************************************************
	*  \brief Inserts a new block into the texture
	* \param img the image to insert
	* \return success/failure
	*********************************************************************/
	virtual bool Insert  (Image *img)=0;
	
	/*!******************************************************************
	*  \brief Removes a block from the texture
	* \param img the image to remove
	* \return success/failure
	*********************************************************************/
	virtual bool Remove  (Image *img)=0;
	
	/*!******************************************************************
	*  \brief Marks a block as free
	* \param img the image to free
	* \return success/failure
	*********************************************************************/
	virtual bool Free    (Image *img)=0;
	
	/*!******************************************************************
	*  \brief Marks a block previously freed as used
	* \param img the image to restore
	* \return success/failure
	*********************************************************************/
	virtual bool Restore (Image *img)=0;
	
};


/*!***************************************************************************
 *  \brief An actual OpenGL texture which can be used for storing multiple
 *         smaller images in it, to save on texture switches.
 *
 *  \note  This is called TexSheet instead of Texture, so it's clear that
 *         this doesn't represent an image that you would draw on the screen,
 *         but simply a "container" for smaller images.
 *****************************************************************************/

class TexSheet
{
public:

	TexSheet(int32 w, int32 h, GLuint texID_, TexSheetType type_, bool isStatic_);
	~TexSheet();

	/*!******************************************************************
	*  \brief Adds new image to the tex sheet
	* \param img the image to add
	* \param loadInfo the image loading info
	* \return success/failure
	*********************************************************************/
	bool AddImage
	(
		Image *img,
		ImageLoadInfo & loadInfo
	);
	
	//FIXME Unused
	bool SaveImage(Image *img);
	
	/*!******************************************************************
	*  \brief copies an image into a sub-rectangle of the texture
	* \param x x coordinate of rectangle to copy image to
	* \param y y coordinate of rectangle to copy image to
	* \param loadInfo the image loading info
	* \return success/failure
	*********************************************************************/
	bool CopyRect(int32 x, int32 y, private_video::ImageLoadInfo & loadInfo);
	
	/*!******************************************************************
	*  \brief removes an image completely
	* \param img the image to remove
	* \return success/failure
	*********************************************************************/
	bool RemoveImage (Image *img);
	
	/*!******************************************************************
	*  \brief marks the image as free
	* \param img the image to mark as free
	* \return success/failure
	*********************************************************************/
	bool FreeImage   (Image *img);
	
	/*!******************************************************************
	*  \brief marks a previously freed image as "used"
	* \param img the image to mark as used
	* \return success/failure
	*********************************************************************/
	bool RestoreImage (Image *img);
	
	/*!******************************************************************
	*  \brief unloads texture memory used by this sheet
	* \return success/failure
	*********************************************************************/
	bool Unload();
	
	/*!******************************************************************
	*  \brief reloads all the images into the sheet
	* \return success/failure
	*********************************************************************/
	bool Reload();

	//! width of the texsheet
	int32 width;
	
	//! height of the texsheet
	int32 height;

	//! if true, images in this sheet that are unlikely to change
	bool isStatic;
	
	//! does it hold 32x32, 32x64, 64x64, or any kind
	TexSheetType type;

	//! manages which areas of the texture are free
	TexMemMgr *texMemManager;

	//! number OpenGL uses to refer to this texture
	GLuint texID;
	
	//! if texsheet is loaded
	bool loaded;		
};


/*!***************************************************************************
 *  \brief used by the fixed-size texture manager to keep track of which blocks
 *         are owned by which images.
 *
 *  \note  The list is doubly linked to allow for O(1) removal
 *****************************************************************************/

class FixedImageNode
{
public:

	//! the image that belongs to the block
	Image          *image;
	
	//! the next node in the list
	FixedImageNode *next;
	
	//! the previous node in the list
	FixedImageNode *prev;
	
	//! the block index
	int32 blockIndex;
};


/*!***************************************************************************
 *  \brief used to manage textures which are designated for fixed image sizes.
 *         For example, a 512x512 sheet that only holds 32x32 tiles.
 *
 *  \note  The texture sheet's size must be divisible by the size of the images
 *         that it holds. For example, you can't create a 256x256 sheet which
 *         holds tiles which are 17x93.
 *****************************************************************************/

class FixedTexMemMgr : public TexMemMgr
{
public:
	FixedTexMemMgr(TexSheet *texSheet, int32 imgW, int32 imgH);
	~FixedTexMemMgr();
	
	/*!******************************************************************
	*  \brief Inserts a new block into the texture
	* \param img the image to insert
	* \return success/failure
	*********************************************************************/
	bool Insert  (Image *img);
	
	/*!******************************************************************
	*  \brief Removes a block from the texture
	* \param img the image to remove
	* \return success/failure
	*********************************************************************/
	bool Remove  (Image *img);
	
	/*!******************************************************************
	*  \brief Marks a block as free
	* \param img the image to free
	* \return success/failure
	*********************************************************************/
	bool Free    (Image *img);
	
	/*!******************************************************************
	*  \brief Marks a block previously freed as used
	* \param img the image to restore
	* \return success/failure
	*********************************************************************/
	bool Restore (Image *img);

private:

	/*!******************************************************************
	*  \brief Grabs the block index based off of the image
	* \param img the image to look for
	* \return the block index for that image
	*********************************************************************/
	int32 _CalculateBlockIndex(Image *img);
	
	/*!******************************************************************
	*  \brief Grabs the block index based off of the image
	* \param blockIndex the node in the list to delete
	*********************************************************************/
	void _DeleteNode(int32 blockIndex);
	
	//! store dimensions of both the texture sheet, and the images that
	//! it contains
	
	//! NOTE: the sheet dimensions are not in pixels, but images.
	//!       So, a 512x512 sheet holding 32x32 images would be 16x16
	
	int32 _sheetWidth;
	int32 _sheetHeight;
	int32 _imageWidth;
	int32 _imageHeight;
	
	TexSheet *_texSheet;
	
	//! The open list keeps track of which blocks of memory are
	//! open. Note that we track blocks with BOTH an array and a list.
	//! Although it takes up more memory, this makes ALL operations
	//! dealing with the blocklist O(1) so performance is awesome.
	//! Memory isn't too bad either, since blocklist is fairly small.
	//! The tail pointer is also kept so that we can add newly
	//! freed blocks to the end of the list- that way, essentially
	//! blocks that are freed are given a little bit of time from the
	//! time they're freed to the time they're removed, in case they
	//! are loaded again in the near future
	
	FixedImageNode *_openListHead;
	FixedImageNode *_openListTail;
	
	//! this is our actual array of blocks which is indexed like a 2D
	//! array. For example, blocks[x+y*width]->image would tell us
	//! which image is currently allocated at spot (x,y)
	
	FixedImageNode *_blocks;
};


/*!***************************************************************************
 *  \brief this is how we keep track of which images are used/freed in the
 *         variable texture mem manager
 *****************************************************************************/

class VariableImageNode
{
public:
	VariableImageNode()
	{
		image = NULL;
		free  = true;
	}

	//! the image
	Image *image;
	
	//! is the image freed?
	bool   free;
};


/*!***************************************************************************
 *  \brief used to manage a texture sheet where the size of the images it
 *         will contain are unknown
 *
 *  \note  For the sake of reducing the time it takes to allocate an image,
 *         this class rounds image dimensions up to powers of 16. So although
 *         it's fine to add any-sized images to a variable texture sheet,
 *         some space will be wasted due to rounding.
 *****************************************************************************/

class VariableTexMemMgr : public TexMemMgr
{
public:
	
	VariableTexMemMgr(TexSheet *sheet);
	~VariableTexMemMgr();

	/*!******************************************************************
	*  \brief Inserts a new block into the texture
	* \param img the image to insert
	* \return success/failure
	*********************************************************************/
	bool Insert  (Image *img);
	
	/*!******************************************************************
	*  \brief Removes a block from the texture
	* \param img the image to remove
	* \return success/failure
	*********************************************************************/
	bool Remove  (Image *img);
	
	/*!******************************************************************
	*  \brief Marks a block as free
	* \param img the image to free
	* \return success/failure
	*********************************************************************/
	bool Free    (Image *img);
	
	/*!******************************************************************
	*  \brief Marks a block previously freed as used
	* \param img the image to restore
	* \return success/failure
	*********************************************************************/
	bool Restore (Image *img);

private:

	/*!******************************************************************
	*  \brief goes through all the blocks associated with img, and
	*             updates their "free" and "image" properties if
	*             changeFree and changeImage are true, respectively
	* \param img the image to use for the block
	* \param changeFree the block's free status has changed
	* \param changeImage the block's image has changed
	* \param free the block's free status
	* \param newImage the new image to use if changeImage is true
	*********************************************************************/
	bool SetBlockProperties
	(
		Image *img, 
		bool changeFree, 
		bool changeImage, 
		bool free, 
		Image *newImage
	);

	//! the texhseet it's using
	TexSheet *_texSheet;
	
	//! it's list of blocks
	VariableImageNode *_blocks;
	
	//! Sheet's dimensions
	//! NOTE: these aren't in pixels, but in "blocks" of 16x16. So,
	//!       a 512x512 sheet would be 32x32 in blocks
	
	int32 _sheetWidth;
	int32 _sheetHeight;
};

}  // namespace private_video
}  // namespace hoa_video

#endif   // !__TEX_MGMT_HEADER__
