/*
	LinkList.h

	Copyright Dosch u. Amand GmbH u. Co KG 1998-2000.

	THIS FILE CONTAINS CONFIDENTIAL INFORMATION.

*	$Log: linklist.h $
*	Revision 1.8  2004/06/08 14:33:50Z  krcal
*	Revision 1.7  2004/05/21 07:46:55Z  Kelbch
*	change define EXPORT ==> EXPORTLIB
*	Revision 1.6  2004/03/01 15:19:32Z  Huber
*	DEBUG changed to DEBUGLINKLIST in link list for wCnt
*	Revision 1.5  2003/05/15 12:00:00Z  kluwe
*	wCnt for Debug version added.
*	Revision 1.4  2001/03/16 08:58:50Z  Solter
*	Encapsulated EXPORT definition.
*	Revision 1.3  2000/09/20 12:12:48Z  Solter
*	Added __cplusplus conditional compilation statements
*	Revision 1.2  2000/01/12 13:12:57Z  grobbel
*	copyrights inserted
*	Revision 1.1  2000/01/12 10:47:31Z  crasnic
*	Initial revision
*/

#if	!defined( LINKLIST_H )
#define	LINKLIST_H

typedef	struct _LINK * PLINK;
typedef	struct _LINK
{
	PLINK	Next;
	PLINK	Prev;
}	LINK;

typedef	struct _LIST * PLIST;
typedef	struct _LIST
{
	PLINK	Head;
	PLINK	Tail;
#ifdef DBG_LINKLIST
	WORD	wCnt;
#endif
}	LIST;

#define	HEAD(l)			(PLINK)((l)->Head)
#define	CURR(l)			(PLINK)((l)->Curr)
#define	TAIL(l)			(PLINK)((l)->Tail)
#define	NEXT(t)			(PLINK)((t)->Next)
#define	PREV(t)			(PLINK)((t)->Prev)
#define	IS_HEAD(l,t)	(LINK(t) == (l)->Head)
#define	IS_CURR(l,t)	(LINK(t) == (l)->Curr)
#define	IS_TAIL(l,t)	(LINK(t) == (l)->Tail)

#if	!defined( EXPORTLIB )
#define			EXPORTLIB
#endif

#if	!defined( EXPORT )
#define	EXPORT
#endif

#if	defined( __cplusplus )
extern	"C"
{
#endif

PLINK EXPORTLIB		llAttachHead( PLIST list, PLINK link );
PLINK EXPORTLIB		llAttachTail( PLIST list, PLINK link );
PLINK EXPORTLIB		llInsertLink( PLIST list, PLINK link, PLINK next );
PLINK EXPORTLIB		llAppendLink( PLIST list, PLINK link, PLINK prev );
PLINK EXPORTLIB		llDetachHead( PLIST list );
PLINK EXPORTLIB		llDetachTail( PLIST list );
PLINK EXPORTLIB		llDetachLink( PLIST list, PLINK link );

#if	defined( __cplusplus )
}
#endif

#endif

//*/
