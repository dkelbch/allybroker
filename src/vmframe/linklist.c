/*
	LinkList.c

	Copyright Dosch u. Amand GmbH u. Co KG 1998.

	THIS FILE CONTAINS CONFIDENTIAL INFORMATION.
*/
/**
	@name LinkList.c
	@memo Generic doubly linked list maintenance functions.	
	@doc  Doubly linked lists are based on elements containing pointers to
	      precedessors and successors.
	      A list contains pointers to the head an to the tail of the list.
*/
/*
	$Log: linklist.c $
	Revision 1.9  2004/05/25 07:53:52Z  Krcal
	change definition EXPORTLIB
	Revision 1.8  2004/05/21 07:52:36Z  Kelbch
	change define EXPORT ==> EXPORTLIB
	Revision 1.7  2004/03/01 15:20:46Z  Huber
	DEBUG changed to DEBUGLINKLIST in link list for wCnt
	Revision 1.6  2003/05/22 17:12:10Z  kluwe
	counter for checking no. of list elements in DEBUG version added.
	Revision 1.5  2003/02/10 09:42:48Z  Kelbch
	add pragma automatic, because function are reentrant
	Revision 1.4  2002/03/20 13:19:58Z  crasnic
	completed documentation stuff
	Revision 1.3  2002/03/18 10:55:42Z  Solter
	Added documentation stuff.
	Revision 1.2  2002/02/06 13:06:08Z  Solter
	Prepared for documentation tool evaluation.
	Revision 1.1  2000/01/12 10:48:34Z  crasnic
	Initial revision
 * 
 * 3     6.09.99 15:21 Solter

*/

//#include	<stdtypes.h>
#include	<stdlib.h>
//#include <systools.h>

#if	defined( CR16 ) || defined( _C166 ) || defined( __arm )

#include	<string.h>					// memset etc.

#endif

#include	"linklist.h"

#if defined( _C166 )
#pragma automatic
#endif

//						========== llAttachHead ==========
/**

	@mem 		Adds a link element to the head of a list.
	
	@param		Pointer to a LIST object.
	@param		Pointer to a LINK object.
	
	@return	Pointer to the inserted LINK object or NULL if either the list or the
				link object is NULL.
*/

PLINK EXPORT		llAttachHead( PLIST lpList, PLINK lpLink )
{						
	if( !lpList || !lpLink )
		return	NULL;

	lpLink->Next = lpLink->Prev = NULL;

	if( !lpList->Head )				// empty list?
		lpList->Head = lpList->Tail = lpLink;
	else
	{
		lpLink->Next = lpList->Head;
		lpList->Head->Prev = lpLink;
		lpList->Head = lpLink;
	}

#ifdef DEBUGLINKLIST
	lpList->wCnt++;
#endif
	
	return	lpLink;
}

//						========== llAttachTail ==========
/**

	@name llAttachTail
	@mem	Adds a link element to the end of a list.
	
	@param	lpList Pointer to a LIST object.
	@param	lpLink Pointer to a LINK object.
	
	@return	Pointer to the inserted LINK object or NULL if either the list or the
				link object is NULL.
*/

PLINK EXPORT		llAttachTail( PLIST lpList, PLINK lpLink )
{						
	if( !lpList || !lpLink )
		return	NULL;

	lpLink->Next = lpLink->Prev = NULL;

	if( !lpList->Head )				// empty list?
		lpList->Head = lpList->Tail = lpLink;
	else
	{
		lpLink->Prev = lpList->Tail;
		lpList->Tail->Next = lpLink;
		lpList->Tail = lpLink;
	}

#ifdef DEBUGLINKLIST
	lpList->wCnt++;
#endif
	return	lpLink;
}

//						========== llInsertLink ==========
/**
	@name llInsertLink
	@memo Inserts a link element into a list.
	
	@param	lpList Pointer to a LIST object.
	@param	lpLink Pointer to a LINK object.
	@param	lpNext Pointer to a LINK object the lpLink will be inserted at.
	
	@return	Pointer to the inserted LINK object or NULL if either the list or the
				link object is NULL.
			
	@doc	The element to be inserted is replacing the lpNext member of the list,
			which will become the successor of the inserted element.
			If the list was empty or lpNext specifies the head of a list or is
			NULL, the new element is inserted at the top of the list.
*/

PLINK EXPORT		llInsertLink( PLIST lpList, PLINK lpLink, PLINK lpNext )
{
	if( !lpList || !lpLink )
		return	NULL;

	lpLink->Next = lpLink->Prev = NULL;

	if( !lpList->Head )				// empty list?
		lpList->Head = lpList->Tail = lpLink;
	else
	{
		if( !lpNext || (lpNext == lpList->Head) )
		{
			lpLink->Next = lpList->Head;
			lpList->Head->Prev = lpLink;
			lpList->Head = lpLink;
		}
		else
		{
			lpLink->Next = lpNext;
			lpLink->Prev = ((PLINK)lpNext)->Prev;
			lpLink->Prev->Next = lpLink;
			lpLink->Next->Prev = lpLink;
		}
	}
#ifdef DEBUGLINKLIST
	lpList->wCnt++;
#endif
	return	lpLink;
}

//						========== llAppendLink ==========
/**

	@name	llAppendLink
	@memo	Inserts a link element into a list.
	
	@param	lpList Pointer to a LIST object.
	@param	lpLink Pointer to a LINK object.
	@param	lpPrev Pointer to a LINK object the lpLink will be inserted at.
	
	@return	Pointer to the inserted LINK object or NULL if either the list or the
				link object is NULL.
			
	@doc	The element is inserted after the lpPrev member of the list.
			If the list was empty or lpNext specifies the tail of a list or is
			NULL, the new element is appended to the end of the list and
			becomes the new tail.
*/

PLINK EXPORT		llAppendLink( PLIST lpList, PLINK lpLink, PLINK lpPrev )
{						
	if( !lpList || !lpLink )
		return	NULL;

	lpLink->Next = lpLink->Prev = NULL;

	if( !lpList->Head )				// empty list?
		lpList->Head = lpList->Tail = lpLink;
	else
	{
		if( !lpPrev || (lpPrev == lpList->Tail) )
		{
			lpLink->Prev = lpList->Tail;
			lpList->Tail->Next = lpLink;
			lpList->Tail = lpLink;
		}
		else
		{
			lpLink->Prev = lpPrev;
			lpLink->Next = ((PLINK)lpPrev)->Next;
			lpLink->Prev->Next = lpLink;
			lpLink->Next->Prev = lpLink;
		}
	}
#ifdef DEBUGLINKLIST
	lpList->wCnt++;
#endif

	return	lpLink;
}

//						========== llDetachHead ==========
/**

	@name	llDetachHead
	
	@memo Detaches the first (head) element from a list.
	
	@param	lpList Pointer to a LIST object.
			
	@return	Pointer to the removed LINK object or NULL if the list is empty.
*/

PLINK EXPORT		llDetachHead( PLIST lpList )
{
	PLINK				lpLink;

	if( !lpList->Head )
		return	NULL;

	lpLink = lpList->Head;

	if( lpList->Head = lpList->Head->Next )
	{
		lpList->Head->Prev = NULL;
	}
	else
		lpList->Tail = NULL;

#ifdef DEBUGLINKLIST
	lpList->wCnt--;
#endif
	return	lpLink;
}

//						========== llDetachTail ==========
/**

	@name llDetachTail
	@memo Detaches the last (tail) element from a list.

	@param	lpList Pointer to a LIST object.
			
	@return	Pointer to the removed LINK object or NULL if the list is empty.
*/

PLINK EXPORT		llDetachTail( PLIST lpList )
{
	PLINK				lpLink;

	if( !lpList->Tail )
		return	NULL;

	lpLink = lpList->Tail;

	if( lpList->Tail = lpList->Tail->Prev )
		lpList->Tail->Next = NULL;
	else
		lpList->Head = NULL;
#ifdef DEBUGLINKLIST
	lpList->wCnt--;
#endif
	return	lpLink;
}

//						========== llDetachLink ==========
/**

	@name llDetachLink
	@memo Detaches an element from a list.

	@param	lpList Pointer to a LIST object.
	@param	lpLink Pointer to a LINK object.
	
	@return	Pointer to the removed LINK object or NULL if the list is empty.
*/

PLINK EXPORT		llDetachLink( PLIST lpList, PLINK lpLink )
{
	if( !lpList || !lpLink )
		return	NULL;				// Leere Liste

	if( lpList->Head == lpList->Tail )
	{
		lpList->Head	=
		lpList->Tail	= NULL;

		return	lpLink;
	}

	if( lpLink == lpList->Head )
	{
		lpList->Head = lpLink->Next;
		lpList->Head->Prev = NULL;
	}
	else if( lpLink == lpList->Tail )
	{
		lpList->Tail = lpLink->Prev;
		lpList->Tail->Next = NULL;
	}
	else
	{
		lpLink->Next->Prev = lpLink->Prev;
		lpLink->Prev->Next = lpLink->Next;
	}

#ifdef DEBUGLINKLIST
	lpList->wCnt--;
#endif
	return	lpLink;
}

//*/
