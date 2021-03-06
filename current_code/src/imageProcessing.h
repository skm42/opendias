/*
 * imageProcessing.h
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * imageProcessing.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * imageProcessing.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMAGEPROCESSING
#define IMAGEPROCESSING

#ifdef CAN_SCAN
#include <FreeImage.h>

extern void FreeImageErrorHandler(FREE_IMAGE_FORMAT, const char *);
extern void reformatImage(FREE_IMAGE_FORMAT, char *, FREE_IMAGE_FORMAT, char *);
#endif // CAN_SCAN //
extern char *getTextFromImage(const unsigned char *, int, int, int, char *);

#endif /* IMAGEPROCESSING */
