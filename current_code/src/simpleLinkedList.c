/*
 * simpleLinkedList.c
 * Copyright (C) Clearscene Ltd 2008 <wbooth@essentialcollections.co.uk>
 * 
 * db.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * db.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*      /------------------------\
 *    |/                         |
 *  x1-                          |
 *  | data                       |
 *  | prev [null]                |
 *  | next  -------->  x2-       |
 *  ---                | data    |
 *                     | prev  --/
 *                     | next [null]
 *                     ---
*/

#include "debug.h"
#include "simpleLinkedList.h"
#include <stdlib.h>
#include <string.h>

struct simpleLinkedList *generate_new_element( char *key, void *data ) {
  struct simpleLinkedList *element = malloc( sizeof(struct simpleLinkedList) );
  element->key = key;
  element->data = data;
  element->next = NULL;
  element->prev = NULL;
  return element;
}

struct simpleLinkedList *sll_createNewElement( void *data ) {
  return generate_new_element( NULL, data );
}

struct simpleLinkedList *sll_init() {
  return generate_new_element( NULL, NULL );
}

void sll_append ( struct simpleLinkedList *element, void *data ) {
  if( element ) {
    if( element->data == NULL && element->next == NULL && element->prev == NULL ) {
      element->data = data;
    }
    else {
      struct simpleLinkedList *lastElement = sll_findLastElement( element );
      struct simpleLinkedList *newElement = sll_createNewElement( data );
      lastElement->next = newElement;
      newElement->prev = lastElement;
    }
  }
}

void sll_insert ( struct simpleLinkedList *element, char *key, void *data ) {
  if ( element ) {
    if( element->data == NULL && element->next == NULL && element->prev == NULL ) {
      element->key = key;
      element->data = data;
    }
    else {
      struct simpleLinkedList *lastElement = sll_findLastElement( element );
      struct simpleLinkedList *newElement = generate_new_element( key, data );
      lastElement->next = newElement;
      newElement->prev = lastElement;
    }
  }
}

struct simpleLinkedList *sll_searchKeys( struct simpleLinkedList *element, const char *key ) {
  if( element && ( element != NULL ) ) {
    if( element->key && ( element->key != NULL ) && ( 0 == strcmp(element->key, key) ) ) {
      return element;
    }
    return sll_searchKeys( sll_getNext( element ), key );
  }
  return NULL;
}

struct simpleLinkedList *sll_findLastElement( struct simpleLinkedList *element ) {
  if( element->next == NULL ) {
    return element;
  }
  return sll_findLastElement( element->next );
}

struct simpleLinkedList *sll_findFirstElement( struct simpleLinkedList *element ) {
  if( !element || element->prev == NULL ) {
    return element;
  }
  return sll_findFirstElement( element->prev );
}

struct simpleLinkedList *sll_getNext( struct simpleLinkedList *element ) {
  if( element->next == NULL ) {
    return NULL;
  }
  return element->next;
}

void sll_destroy( struct simpleLinkedList *element ) {
  if( element && ( element != NULL ) ) {
    o_log(SQLDEBUG, "preparing to delete element: %x, with prev=%x, and next=%x", element, element->prev, element->next);
    sll_destroy( sll_getNext( element ) );
    sll_delete( element );
  }
}

void sll_delete( struct simpleLinkedList *element ) {
  struct simpleLinkedList *prev_element = element->prev;
  struct simpleLinkedList *next_element = element->next;
  o_log(SQLDEBUG, "Asked to delete element: %x, with prev=%x, and next=%x", element, element->prev, element->next);
  if( next_element && ( next_element != NULL ) ) {
    next_element->prev = prev_element;
  }
  if( prev_element && ( prev_element != NULL ) ) {
    prev_element->next = next_element;
  }
  free( element );
}

