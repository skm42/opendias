 /*
 * locaisation.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * localisation.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * localisation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "debug.h"
#include "simpleLinkedList.h"

#include "localisation.h"

struct simpleLinkedList *langList = NULL;

void locale_init( char *lang ) {
  langList = sll_init();
  sll_insert( langList, o_strdup(lang), loadLangList( lang ) );
}

void locale_cleanup() {
  if( langList != NULL ) {
    struct simpleLinkedList *langPack = NULL;
    for( langPack = sll_findFirstElement( langList ) ; langPack != NULL ; langPack = sll_getNext(langPack) ) {
      free(langPack->key); // lang name
      langPack->key = NULL;
      struct simpleLinkedList *localisedValues = (struct simpleLinkedList *)langPack->data;
    
      if( localisedValues != NULL ) {
        struct simpleLinkedList *localisedPhrase = NULL;
        for( localisedPhrase = sll_findFirstElement( localisedValues ) ; localisedPhrase != NULL ; localisedPhrase = sll_getNext(localisedPhrase) ) {
          free(localisedPhrase->key); // lang name
          localisedPhrase->key = NULL;
          if( localisedPhrase->data != NULL ) {
            free(localisedPhrase->data);
            localisedPhrase->data = NULL;
          }
        }
        sll_destroy( sll_findFirstElement( localisedValues ) );
      }

    }
    sll_destroy( sll_findFirstElement( langList ) );
  }
}

const char *getString( const char *phrase, const char *lang) {
  struct simpleLinkedList *tmp;
  struct simpleLinkedList *keysList;

  // fetch or load the list of translations for ths language
	//REMARK: only the main-thread should ever write to the mem area of langList and langs
  tmp = sll_searchKeys( langList, lang );
  if( tmp == NULL || tmp->data == NULL ) {
    keysList = loadLangList( lang );
    sll_insert( langList, o_strdup(lang), keysList );
  }
  else {
    keysList = (struct simpleLinkedList *)tmp->data;
  }

  // return the translation
  tmp = sll_searchKeys( keysList, phrase );
  if( tmp == NULL || tmp->data == NULL ) {
    if( 0 == strcmp (lang, "en") ) {
      o_log(ERROR, "cannot find result for phrase %s ",phrase);
      return phrase; // cannot find ay translation 
    }
    o_log(DEBUGM, "Cannot find key, trying for english instead");
    return getString( phrase, "en" ); // the defaulting lang translation
  }
  return (char *)tmp->data; // the translation we found
}

struct simpleLinkedList *loadLangList( const char *lang ) {
  struct simpleLinkedList *trList = NULL;
  char *resourceFile = o_printf("%s/opendias/language.resource.%s", SHARE_DIR, lang);

  o_log(DEBUGM, "loading translation file: %s", resourceFile);
  FILE *translations = fopen(resourceFile, "r");
  if( translations != NULL) {
    trList = sll_init();
    size_t size = 0;
    char *line = NULL;
    while ( getline( &line, &size, translations ) > 0) {
      // Handle commented and blank lines
      chop(line);
      if( line[0] == '#' || line[0] == 0 ) {
        o_log(SQLDEBUG, "Rejecting line: %s", line);
        continue;
      }
      char *pch = strtok(line, "|");
      if (pch != NULL) {
        o_log(SQLDEBUG, "Adding key=%s, value=%s", pch, pch+strlen(pch)+1);
        sll_insert( trList, o_strdup(pch), o_strdup(pch+strlen(pch)+1) );
      }
    }
    if (line != NULL) {
      free(line);
    }
    fclose( translations );
  }
  free( resourceFile );
  return trList;
}

void localize(char **data, char *lang, size_t *size_ptr) {

	#define MAX_COMMAND_SIZE 255
	const char sep='-';
	char word[MAX_COMMAND_SIZE];

	
	int within=0, complete=0, start=0, end=0,wlen=0,n,nd,words=0,translations=0,msize=0;
	char *rdata,*pageptr,*buf;
	o_log(DEBUGM,"entering localize");

	size_t newsize,size;
	msize=newsize=size=*size_ptr+1;
	
	//size_t newsize=size;

	pageptr=*data;

	if ( pageptr != NULL) {
		o_log(DEBUGM,"localize data of size %d to %s:",(int) size,lang);
		if ( (buf=(char *)malloc(size) ) == NULL ) {
			o_log(ERROR,"localize buffer allocation error.");
			perror("localize-->");
			exit(1);
		}
		

		//REMARK: avoid parsing input elements. could be used malicious.
		for (n=0; n<(int)size; n++) {
			
			if ( pageptr[n] == sep && pageptr[n+1] == sep && pageptr[n+2] == sep) {
				if ( within == 1 ) { 
					complete=1;
					within=0;
					n=n+3;
					end=n;
				} else {
					start=n;
					n=n+3;
					within=1;
				}
			}

			if ( within == 1 ) {

				if ( wlen > MAX_COMMAND_SIZE ) {
					o_log(DEBUGM,"localizer-->localization command end was not reached");
					wlen=0;
					within=0;
				} else {
					word[wlen]=pageptr[n];
					wlen++;
				}
			}
			
			if ( complete == 1 ) {
				word[wlen]='\0';
				words++;
				char *tmp;
				tmp=o_strdup(getString((const char*)word, lang));
				if (tmp == NULL ) {
					o_log(INFORMATION,"localize--> no translation found for %s",word);
				} else {
					translations++;
					nd=strlen(tmp)-strlen(word)-6;
					newsize=size+nd;
					o_log(DEBUGM,"localize--> translation for (%s)(%d) is (%s)(%d)" ,word,strlen(word),tmp,strlen(tmp));
					o_log(DEBUGM,"localize--> size=%d newsize=%d msize=%d nd=%d",size,newsize,msize,nd);
					if ( newsize > msize ) {
								msize=newsize;
							  if ( (rdata=realloc(pageptr,newsize))==NULL ) {
								  perror("localizer--> memory reallocation error");
								  exit(0);
							  } 
							  if(rdata!=pageptr) {
								  o_log(DEBUGM,"localizer-->realloc memory move occured");
							  }
							  pageptr=rdata;
					}
					memmove(&pageptr[end+nd],&pageptr[end],sizeof(char)*(size-end));
					memcpy(&pageptr[start],tmp,sizeof(char)*(strlen(tmp)));	
					n=start+strlen(tmp);
					free(tmp);
					
					size=newsize;
				}
					
				//REMARK: do not react within input fields later
				complete=0;
				wlen=0;
			}
		}
	}
	if (within == 1 ) {
		o_log(DEBUGM,"localizer: ended without reaching end of localization command");
	}
	pageptr[msize-1]='\0';

	//o_log(DEBUGM,"localize-->translation completed (%s)",pageptr);
	o_log(DEBUGM,"localize-->translation done msize=%d, strlen=%d. %d words, %d translations",msize,strlen(pageptr),words,translations);
	*size_ptr=strlen(pageptr);
	*data=pageptr;
	o_log(DEBUGM,"leaving localize");
}
