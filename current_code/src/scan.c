/*
 * scan.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * scan.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * scan.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib.h>       // REMOVE ME (and the g_strconcat)
#include <stdlib.h>
#include <stdio.h>      // printf, file operations
#include <string.h>     // compares
#include <FreeImage.h>  // 
#include <math.h>  // 
#ifdef CAN_SCAN
#include <sane/sane.h>  // Scanner Interface
#include <sane/saneopts.h>  // Scanner Interface
#include <pthread.h>    // Return from this thread
#include "scanner.h"
#endif // CAN_SCAN //

#include "scan.h"
#include "imageProcessing.h"
#include "dbaccess.h"
#include "main.h"
#include "utils.h"
#include "debug.h"

#ifdef CAN_SCAN

extern void doScanningOperation(void *uuid) {

  int request_resolution, testScanner=0;
  char *devName = getScanParam(uuid, SCAN_PARAM_DEVNAME);
  char *docid_s = getScanParam(uuid, SCAN_PARAM_DOCID);

  char *pageCount_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_PAGES);
  int pageCount = atoi(pageCount_s);
  free(pageCount_s);


  // Open the device
  o_log(DEBUGM, "sane_open");
  updateScanProgress(uuid, SCAN_WAITING_ON_SCANNER, 0);
  SANE_Handle *openDeviceHandle;
  SANE_Status status = sane_open ((SANE_String_Const) devName, (SANE_Handle)&openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {
    handleSaneErrors("Cannot open device", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    free(devName);
    return;
  }
  if ( strstr(devName, "test") != 0 ) {
    testScanner=1;
  }
  free(devName);

  int buffer_len = 0; 
  int option=0, paramSetRet=0;
  for (option = 0; option < 9999; option++) {

    const SANE_Option_Descriptor *sod;
    sod = sane_get_option_descriptor (openDeviceHandle, option);

    // No more options    
    if (sod == NULL)
      break;

    // Just a placeholder
    if (sod->type == SANE_TYPE_GROUP
    || sod->name == NULL
    || option == 0)
      continue;

    log_option( option, sod );

    // Validation
    if ( (sod->cap & SANE_CAP_SOFT_SELECT) && (sod->cap & SANE_CAP_HARD_SELECT) ) {
      o_log(DEBUGM, "The backend said that '%s' is both hardward and software settable! Err", sod->name);
      updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, 0);
    }

    // we MUST set this value
    if ( (sod->cap & SANE_CAP_SOFT_DETECT) && ((sod->cap & SANE_CAP_INACTIVE) == 0) ) {

      // A hardware setting
      if ( sod->cap & SANE_CAP_HARD_SELECT ) {
        o_log(DEBUGM, "We've got no way of telling the user to set the hardward %s! Err", sod->name);
        //updateScanProgress(uuid, SCAN_INTERNAL_ERROR, 10001);
      }

      // a software setting
      else {

        // Set scanning resolution
        if ( strcmp(sod->name, SANE_NAME_SCAN_RESOLUTION) == 0) {

          char *request_resolution_s = getScanParam(uuid, SCAN_PARAM_REQUESTED_RESOLUTION);
          request_resolution = atoi(request_resolution_s);
          free(request_resolution_s);

          if (sod->type == SANE_TYPE_FIXED) {
            SANE_Fixed f = SANE_FIX(request_resolution);
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &f, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set resolution (fixed)", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return;
            }
          }
          else {
            int sane_resolution = request_resolution;
            if( sod->constraint.range->quant != 0 ) {
              sane_resolution = sane_resolution * sod->constraint.range->quant;
              buffer_len = sod->constraint.range->quant; // q seam to be a good buffer size to use!
            }
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &sane_resolution, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set resolution (range)", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return;
            }
          }
        }

        // Set scanning Source
        else if ( strcmp(sod->name, SANE_NAME_SCAN_SOURCE) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            int i, j;
            int foundMatch = 0;
            const char *sources[] = { "Auto", SANE_I18N ("Auto"), "Flatbed", SANE_I18N ("Flatbed"), "FlatBed", "Normal", SANE_I18N ("Normal"), NULL };
            for (i = 0; sources[i] != NULL; i++) {
              for (j = 0; sod->constraint.string_list[j]; j++) {
                if (strcmp (sources[i], sod->constraint.string_list[j]) == 0)
                  break;
              }
              if (sod->constraint.string_list[j] != NULL) {
                status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, (void *)sources[i], &paramSetRet);
                if(status != SANE_STATUS_GOOD) {
                  handleSaneErrors("Cannot set source", status, paramSetRet);
                  updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
                  return;
                }
                foundMatch = 1;
                break;
              }
            }
            if( foundMatch == 0 ) {
              o_log(DEBUGM, "Non of the available options are appropriate.");
            }
          }
        }

        // Set scanning depth
        else if ( strcmp(sod->name, SANE_NAME_BIT_DEPTH) == 0 ) {
          SANE_Int v = 8;
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set depth", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return;
            }
          }
        }

        // Set scanning mode
        else if ( strcmp(sod->name, SANE_NAME_SCAN_MODE) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, SANE_VALUE_SCAN_MODE_COLOR, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set mode", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return;
            }
          }
        }

        // Set Preview mode
        else if ( strcmp(sod->name, SANE_NAME_PREVIEW) == 0 ) {
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            SANE_Bool v = SANE_FALSE;
            status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
            if(status != SANE_STATUS_GOOD) {
              handleSaneErrors("Cannot set mode", status, paramSetRet);
              updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
              return;
            }
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_TL_X) == 0 || strcmp(sod->name, SANE_NAME_SCAN_TL_Y) == 0 ) {
          SANE_Fixed v = sod->constraint.range->min;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set TL", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
        }

        else if ( strcmp(sod->name, SANE_NAME_SCAN_BR_X) == 0 || strcmp(sod->name, SANE_NAME_SCAN_BR_Y) == 0 ) {
          SANE_Fixed v = sod->constraint.range->max;
          status = control_option (openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set BR", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
        }

        else if (strcmp (sod->name, "non-blocking") == 0) {
          SANE_Bool v = SANE_FALSE;
          status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set non-blocking", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
        }

        else if (strcmp (sod->name, "custom-gamma") == 0) {
          SANE_Bool v = SANE_FALSE;
          status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set no to custonmer-gamma", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
        }

        // For the test 'virtual scanner'
        else if (testScanner == 1) {
          if (strcmp (sod->name, "hand-scanner") == 0) {
            SANE_Bool v = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "three-pass") == 0) {
            SANE_Bool v = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "three-pass-order") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "RGB", &paramSetRet);
          }
          else if (strcmp (sod->name, "test-picture") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "Color pattern", &paramSetRet);
          }
          else if (strcmp (sod->name, "read-delay") == 0) {
            SANE_Bool v = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "fuzzy-parameters") == 0) {
            SANE_Bool v = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-delay-duration") == 0) {
            SANE_Int v = 1000;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-limit") == 0) {
            SANE_Bool v = SANE_TRUE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-limit-size") == 0) {
            SANE_Int v = sod->constraint.range->max;
            buffer_len = sod->constraint.range->max;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "read-return-value") == 0) {
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, "Default", &paramSetRet);
          }
          else if (strcmp (sod->name, "ppl-loss") == 0) {
            SANE_Int v = 0;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          else if (strcmp (sod->name, "invert-endianess") == 0) {
            SANE_Bool v = SANE_FALSE;
            status = control_option(openDeviceHandle, sod, option, SANE_ACTION_SET_VALUE, &v, &paramSetRet);
          }
          if(status != SANE_STATUS_GOOD) {
            handleSaneErrors("Cannot set option", status, paramSetRet);
            updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
            return;
          }
        }

        // not a 'well known' option
        else {

          // try setting automatically
          if ( !setDefaultScannerOption(openDeviceHandle, sod, option) ) {
            o_log(DEBUGM, "Could not set authmatically", sod->name);
            //updateScanProgress(uuid, SCAN_INTERNAL_ERROR, 10002);
          }

        }
      }
    }
    else {
      o_log(DEBUGM, "The option does not need to be set.");
    }

  }


  o_log(DEBUGM, "sane_start");
  status = sane_start (openDeviceHandle);
  if(status != SANE_STATUS_GOOD) {  
    handleSaneErrors("Cannot start scanning", status, 0);
    updateScanProgress(uuid, SCAN_ERRO_FROM_SCANNER, status);
    return;
  }

  // Get scanning params (from the scanner)
  o_log(DEBUGM, "Get scanning params");
  SANE_Parameters pars;
  status = sane_get_parameters (openDeviceHandle, &pars);
  int getLines = pars.lines;
  char *length_s = getScanParam(uuid, SCAN_PARAM_LENGTH);
  int pagelength = atoi(length_s);
  if(pagelength && pagelength >= 20 && pagelength < 100)
    getLines = (int)(getLines * pagelength / 100);
  free(length_s);

  o_log(INFORMATION, "Scanner Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,get=%d,dep=%d\n",
    sane_strstatus (status),
    pars.format, pars.last_frame,
    pars.bytes_per_line, pars.pixels_per_line,
    pars.lines, getLines, pars.depth);

  int expectFrames = 0;
  switch (pars.format) {
    case SANE_FRAME_GRAY:
      o_log(DEBUGM, "Expecting Gray data (1 channel only).");
      expectFrames = 1;
      break;
    case SANE_FRAME_RGB:
      o_log(DEBUGM, "Expecting RGB data (3 channels).");
      expectFrames = 3;
      break;
    default:
      o_log(DEBUGM, "backend returns three frames speratly. We do not currently support this.");
      updateScanProgress(uuid, SCAN_INTERNAL_ERROR, 10003);
      return;
      break;
  }  

  // Save Record
  //
  int docid, page;
  char *page_s;
  if( docid_s == NULL ) {
    o_log(DEBUGM, "Saving record");
    updateScanProgress(uuid, SCAN_DB_WORKING, 0);
    docid_s = addNewScannedDoc(getLines, pars.pixels_per_line, request_resolution, pageCount); 

    setScanParam(uuid, SCAN_PARAM_DOCID, docid_s);
    page_s = o_strdup("1");
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, page_s);
    page = 1;

    docid = atoi(docid_s);
  }
  else {
    page_s = getScanParam(uuid, SCAN_PARAM_ON_PAGE);
    page = atoi(page_s);
    free(page_s);
    page++;
    page_s = itoa(page, 10);
    setScanParam(uuid, SCAN_PARAM_ON_PAGE, page_s);
    docid = atoi(docid_s);
  }

  // Acquire Image & Save Document
  FILE *scanOutFile;
  if ((scanOutFile = fopen("/tmp/tmp.pnm", "w")) == NULL)
    o_log(ERROR, "could not open file for output");
  fprintf (scanOutFile, "P5\n# SANE data follows\n%d %d\n%d\n", 
    pars.pixels_per_line, getLines,
    (pars.depth <= 8) ? 255 : 65535);
  double totbytes = (pars.bytes_per_line * getLines) / expectFrames;
  double readSoFar = 0;
  int progress = 0;

  buffer_len = pars.bytes_per_line; // Low - but the only way to be sure

  o_log(DEBUGM, "Using a buffer_len of %d", buffer_len);
  SANE_Byte *buffer = malloc(buffer_len);
  SANE_Byte *pic=(unsigned char *)malloc( totbytes+1 );
  int counter;
  int buff_fixed = 0;
  int onlyReadxFromBlockofThree = 0;
  for (counter = 0 ; counter < totbytes ; counter++) pic[counter]=125;

  o_log(DEBUGM, "scan_read - start");
  do {
    int noMoreReads = 0;
    updateScanProgress(uuid, SCAN_SCANNING, progress);
    SANE_Int buff_len;
    status = sane_read (openDeviceHandle, buffer, buffer_len, &buff_len);
    if (status != SANE_STATUS_GOOD) {
      if (status == SANE_STATUS_EOF)
        noMoreReads = 1;
      else
        o_log(ERROR, "something wrong while scanning");
    }

    if(buff_len > 0) {

      if( expectFrames == 3 ) {

        int offset = 0;

        // Do we have to finish the RGB from the last read?
        if( onlyReadxFromBlockofThree ) {

          // A bit of sanity checking!
          if( (buff_len + onlyReadxFromBlockofThree - 3) < 0) {
            o_log(DEBUGM, "Things dont add up. Stopping reading");
            break;
          }

          offset = 3-onlyReadxFromBlockofThree;
          int s;
          for(s = 0; s < offset; s++) {
            pic[(int)readSoFar] = max( pic[(int)readSoFar], (int)buffer[(int)s] );
          }
          readSoFar++;
        }

        // Check we have full blocks of data
        onlyReadxFromBlockofThree = (int)fmod( (buff_len - offset), 3 );
        //if(onlyReadxFromBlockofThree) 
        //  o_log(DEBUGM, "This time the last block only contains %d samples out of 3", onlyReadxFromBlockofThree);


        for(counter = offset; counter < buff_len; counter++) {
          int sample = 0;
          int s;
          int pixelIncrement = 1;
          int bytesInThisBlock = 3;
          if ( (counter+3) > buff_len ) {
            bytesInThisBlock = onlyReadxFromBlockofThree;
            pixelIncrement = 0;
          }
          for(s = 0; s < bytesInThisBlock; s++)
            sample = max(sample, (int)buffer[(int)counter+s]);
          pic[(int)readSoFar] = (int)sample;
          counter += (bytesInThisBlock-1);
          readSoFar += pixelIncrement;
        }
      }

      // Only one frame in "Gray" mode.
      else {
        for(counter = 0; counter < buff_len; counter++)
          pic[(int)(readSoFar + counter)] = 
            (int)buffer[(int)counter];
        readSoFar += buff_len;
      }

      // Update the progress info
      int progress_d = 100 * (readSoFar / totbytes);
      progress = progress_d;
      if(progress > 100)
        progress = 100;

    }

    if(noMoreReads==1)
      break;

    // Update the buffer (based on read feedback
    if(buff_fixed == 0 && buffer_len == buff_len) {
      // Maybe we have scope to increase the buffer length?
      buffer_len += pars.bytes_per_line;
      buffer = realloc(buffer, buffer_len);
      o_log(DEBUGM, "Increasing read buffer to %d bytes.", buffer_len);
    }
    else if( buff_len < buffer_len ) {
      buff_fixed = 1;
      buffer_len = buff_len;
      o_log(DEBUGM, "Reducing read buffer to %d bytes.", buffer_len);
    }

  } while (1);
  o_log(DEBUGM, "scan_read - end");
  free(buffer);

  o_log(DEBUGM, "sane_cancel");
  sane_cancel(openDeviceHandle);

  o_log(DEBUGM, "sane_close");
  sane_close(openDeviceHandle);


  // Fix skew - for this page
  //
  char *skew_s = getScanParam(uuid, SCAN_PARAM_CORRECT_FOR_SKEW);
  int skew = atoi(skew_s);
  free(skew_s);
  if(skew != 0) {
    o_log(DEBUGM, "fixing skew");
    updateScanProgress(uuid, SCAN_FIXING_SKEW, 0);

    deSkew(pic, totbytes, (double)skew, (double)pars.pixels_per_line, getLines);
  }


  // Write the image to disk now
  //
  fwrite (pic, pars.pixels_per_line, getLines, scanOutFile);
  fclose(scanOutFile);


  // Do OCR - on this page
  //
  char *ocrText;
#ifdef CAN_OCR
  char *shoulddoocr_s = getScanParam(uuid, SCAN_PARAM_DO_OCR);
  int shoulddoocr=0;
  if(shoulddoocr_s && 0 == strcmp(shoulddoocr_s, "on") )
    shoulddoocr = 1;
  free(shoulddoocr_s);

  if(shoulddoocr==1) {

    if(request_resolution >= 300 && request_resolution <= 400) {
      o_log(DEBUGM, "attempting OCR");
      updateScanProgress(uuid, SCAN_PERFORMING_OCR, 10);

      char *ocrScanText = getTextFromImage((const unsigned char*)pic, pars.bytes_per_line, pars.pixels_per_line, getLines);

      ocrText = o_strdup("---------------- page ");
      conCat(&ocrText, page_s);
      conCat(&ocrText, " ----------------\n");
      conCat(&ocrText, ocrScanText);
      conCat(&ocrText, "\n");
      free(ocrScanText);
    }
    else {
      o_log(DEBUGM, "OCR was requested, but the specified resolution means it's not safe to be attempted");
      ocrText = o_strdup("---------------- page ");
      conCat(&ocrText, page_s);
      conCat(&ocrText, " ----------------\n");
      conCat(&ocrText, "Resolution set outside safe range to attempt OCR.");
      conCat(&ocrText, "\n");
    }
  }
  else
#endif // CAN_OCR //
    ocrText = o_strdup("");

  free(pic);


  // Convert Raw into JPEG
  //
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 10);
  char *outFilename = g_strconcat(BASE_DIR,"/scans/",docid_s,"_",page_s,".jpg", NULL);
  reformatImage(FIF_PGMRAW, "/tmp/tmp.pnm", FIF_JPEG, outFilename);
  updateScanProgress(uuid, SCAN_CONVERTING_FORMAT, 100);
  free(page_s);


  // update record
  // 
  updateScanProgress(uuid, SCAN_DB_WORKING, 0);
  updateNewScannedPage(docid_s, ocrText, page); // Frees both chars


  // cleaup && What should we do next
  //
  o_log(DEBUGM, "mostly done.");
  if(page >= pageCount)
    updateScanProgress(uuid, SCAN_FINISHED, docid);
  else
    updateScanProgress(uuid, SCAN_WAITING_ON_NEW_PAGE, ++page);

  free(uuid);
  o_log(DEBUGM, "Page scan done.");
  pthread_exit(NULL);

}
#endif // CAN_SCAN //

