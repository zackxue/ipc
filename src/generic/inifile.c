
/* defines for, or consts and inline functions for C++ */

/* global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>	/* Only needed for the test function */

/* local includes */
#include "inifile.h"

/* Global Variables */

//struct ENTRY *Entry = NULL;
//struct ENTRY *CurEntry = NULL;
//char Result[255] =
//{""};
//FILE *IniFile;

//#define Entry thiz->entry
//#define CurEntry thiz->cur_entry
//#define Result thiz->result
//#define IniFile thiz->inifile

/* Private functions declarations */
void AddpKey (struct ENTRY * Entry, cchr * pKey, cchr * Value);
void FreeMem (void *Ptr);
void FreeAllMem (INIFILE* thiz);
bool FindpKey (INIFILE* thiz, cchr * Section, cchr * pKey, EFIND * List);
bool AddSectionAndpKey (INIFILE* thiz, cchr * Section, cchr * pKey, cchr * Value);
struct ENTRY *MakeNewEntry (INIFILE* thiz);

void delblank(char *lstr)
{
  int i;
  char tmpstr[512];
  for(i=strlen(lstr)-1;i>=0;i--)
    if(*(lstr+i)==' ')
      *(lstr+i)='\0';
    else
      break;
  for(i=0;i<strlen(lstr);i++)
    if(*(lstr+i)!=' ')
      break;
  memset(tmpstr,0,512);
  strcpy(tmpstr,(char *)(lstr+i));
  memset(lstr,0,strlen(lstr));
  strcpy(lstr,tmpstr);
}


#ifdef DONT_HAVE_STRUPR
/* DONT_HAVE_STRUPR is set when INI_REMOVE_CR is defined */
void strupr( char *str )
{
    // We dont check the ptr because the original also dont do it.
	while (*str != 0)
    {
        if ( islower( *str ) )
        {
		     *str = toupper( *str );
        }
        str++;
	}
}
#endif

/*=========================================================================
 *
 * OpenIniFile
 * -------------------------------------------------------------------------
 * Job : Opens an ini file or creates a new one if the requested file
 *         doesnt exists.
 * Att : Be sure to call CloseIniFile to free all mem allocated during
 *         operation!
 *
 *========================================================================*/
static INIFILE* create_inifile();

INIFILE*  
OpenIniFile (cchr * FileName)
{
	char str[255];
	char *pstr;
	struct ENTRY *pEntry;
	int Len;
	INIFILE* thiz = create_inifile();

	//printf("Open File:%s\n",FileName);

	FreeAllMem (thiz);

	if (FileName == NULL)
	{
		printf("FileName is NULL\n");
		return NULL;
	}
	if ((thiz->inifile = fopen (FileName, "r")) == NULL)
	{
		printf("Error open File:%s\n",FileName);
		return NULL;
	}

	while (fgets (str, 255, thiz->inifile) != NULL)
	{
		pstr = strchr (str, '\n');
		if (pstr != NULL)
		{
			*pstr = 0;
		}
		pEntry = MakeNewEntry (thiz);
		if (pEntry == NULL)
		{
			printf("Get Entry Fail!\n");
			return NULL;
		}

#ifdef INI_REMOVE_CR
      	Len = strlen(str);
		if ( Len > 0 )
		{
        	if ( str[Len-1] == '\r' )
        	{
          		str[Len-1] = '\0';
            }
        }
#endif
		
		delblank(str);
		
		pEntry->Text = (char *) malloc (strlen (str) + 1);
		if (pEntry->Text == NULL)
		{
			printf("Get Text Fail!\n");
			FreeAllMem (thiz);
			return NULL;
		}
		//printf("%s\n",str);
		strcpy (pEntry->Text, str);
		pstr = strchr (str, ';');
		if (pstr != NULL)
		{
			*pstr = 0;
		}			/* Cut all comments */
		if ((strstr (str, "[") > 0) && (strstr (str, "]") > 0))	/* Is Section */
		{
			pEntry->Type = tpSECTION;
		}
	    else
		{
			if (strstr (str, "=") > 0)
			{
				pEntry->Type = tpKEYVALUE;
			}
			else
			{
				pEntry->Type = tpCOMMENT;
			}
		}
	    thiz->cur_entry = pEntry;
    }
	fclose (thiz->inifile);
    thiz->inifile = NULL;
  //printf("End Open File!%d\n",true);
	return thiz;
}

/*=========================================================================
 *
 * CloseIniFile
 * -------------------------------------------------------------------------
 * Job : Frees the memory and closes the ini file without any
 *   modifications. If you want to write the file use
 *    WriteIniFile instead.
 * ========================================================================*/
void 
CloseIniFile (INIFILE* thiz)
{
	FreeAllMem (thiz);
	if (thiz->inifile != NULL)
	{
		fclose (thiz->inifile);
		thiz->inifile = NULL;
	}
}

/*=========================================================================
   WriteIniFile
  -------------------------------------------------------------------------
   Job : Writes the iniFile to the disk and close it. Frees all memory
         allocated by WriteIniFile;
 *========================================================================*/
bool 
WriteIniFile (INIFILE* thiz, const char *FileName)
{
	struct ENTRY *pEntry = thiz->entry;
	thiz->inifile = NULL;
	if (thiz->inifile != NULL)
	{
		fclose (thiz->inifile);
	}
	if ((thiz->inifile = fopen (FileName, "wb")) == NULL)
	{
		FreeAllMem (thiz);
		return FALSE;
	}

	while (pEntry != NULL)
	{
		if (pEntry->Type != tpNULL)
		{

#ifdef INI_REMOVE_CR
			fprintf (thiz->inifile, "%s\n", pEntry->Text);
#else
			fprintf (thiz->inifile, "%s\r\n", pEntry->Text);
#endif
		pEntry = pEntry->pNext;
        }
   	}

	fclose (thiz->inifile);
	thiz->inifile = NULL;
	return TRUE;
}


/*=========================================================================
   Writestring : Writes a string to the ini file
*========================================================================*/
void 
write_string (INIFILE* thiz, cchr * Section, cchr * pKey, cchr * Value)
{
	EFIND List;
	char Str[255];

	if (ArePtrValid (Section, pKey, Value) == FALSE)
	{
		return;
	}
	if (FindpKey (thiz, Section, pKey, &List) == TRUE)
	{
		sprintf (Str, "%s=%s%s", List.KeyText, Value, List.Comment);
		FreeMem (List.pKey->Text);
		List.pKey->Text = (char *) malloc (strlen (Str) + 1);
		strcpy (List.pKey->Text, Str);
	}
	else
	{
		if ((List.pSec != NULL) && (List.pKey == NULL))	/* section exist, pKey not */
		{
			AddpKey (List.pSec, pKey, Value);
		}
		else
		{
			AddSectionAndpKey (thiz, Section, pKey, Value);
		}
	}
}

/*=========================================================================
   write_bool : Writes a boolean to the ini file
*========================================================================*/
void 
write_bool (INIFILE* thiz, cchr * Section, cchr * pKey, bool Value)
{
	char Val[2] = {'0', 0};
	if (Value != 0)
	{
		Val[0] = '1';
	}
	write_string (thiz, Section, pKey, Val);
}

/*=========================================================================
   write_int : Writes an integer to the ini file
*========================================================================*/
void 
write_int (INIFILE* thiz, cchr * Section, cchr * pKey, int Value)
{
	char Val[12];			/* 32bit maximum + sign + \0 */
    sprintf (Val, "%d", Value);
    write_string (thiz, Section, pKey, Val);
}

/*=========================================================================
   write_double : Writes a double to the ini file
*========================================================================*/
void 
write_double (INIFILE* thiz, cchr * Section, cchr * pKey, double Value)
{
	char Val[32];			/* DDDDDDDDDDDDDDD+E308\0 */
	sprintf (Val, "%1.10lE", Value);
	write_string (thiz, Section, pKey, Val);
}


/*=========================================================================
   read_string : Reads a string from the ini file
*========================================================================*/
const char *
read_string (INIFILE* thiz, cchr * Section, cchr * pKey, cchr * Default)
{
	EFIND List;
	if (ArePtrValid (Section, pKey, Default) == FALSE)
	{
		printf("ArePtrValid Fail!\n");
		return Default;
	}
	if (FindpKey (thiz, Section, pKey, &List) == TRUE)
	{
		strcpy (thiz->result, List.ValText);
		return thiz->result;
    }
	return Default;
}

/*=========================================================================
   read_bool : Reads a boolean from the ini file
*========================================================================*/
bool 
read_bool (INIFILE* thiz, cchr * Section, cchr * pKey, bool Default)
{
	const char* pVal = read_string (thiz, Section, pKey, Default ? "yes" : "no");
	if(0 == strncmp(pVal, "yes", 3) || 0 == strncmp(pVal, "1", 1)){
		/* Only 0 or 1 / no or yes allowed */
		return true;
	}
	return false;
}

/*=========================================================================
   read_int : Reads a integer from the ini file
*========================================================================*/
int 
read_int (INIFILE* thiz, cchr * Section, cchr * pKey, int Default)
{
	char Val[12];
	sprintf (Val, "%d", Default);
	return (atoi (read_string (thiz, Section, pKey, Val)));
}

/*=========================================================================
   read_double : Reads a double from the ini file
*========================================================================*/
double 
read_double (INIFILE* thiz, cchr * Section, cchr * pKey, double Default)
{
	double Val;
	sprintf (thiz->result, "%1.10lE", Default);
	sscanf (read_string (thiz, Section, pKey, thiz->result), "%lE", &Val);
	return Val;
}

/*=========================================================================
   delete_key : Deletes a pKey from the ini file.
*========================================================================*/

bool delete_key (INIFILE* thiz, cchr *Section, cchr *pKey)
{
    EFIND         List;
    struct ENTRY *pPrev;
    struct ENTRY *pNext;

    if (FindpKey (thiz, Section, pKey, &List) == TRUE)
    {
        pPrev = List.pKey->pPrev;
        pNext = List.pKey->pNext;
        if (pPrev)
        {
            pPrev->pNext=pNext;
        }
        if (pNext)
        {
            pNext->pPrev=pPrev;
        }
        FreeMem (List.pKey->Text);
        FreeMem (List.pKey);
        return TRUE;
    }
    return FALSE;
}



/* Here we start with our helper functions */
/*=========================================================================
   FreeMem : Frees a pointer. It is set to NULL by Free AllMem
*========================================================================*/
void 
FreeMem (void *Ptr)
{
	if (Ptr != NULL)
	{
		free (Ptr);
	}
}

/*=========================================================================
   FreeAllMem : Frees all allocated memory and set the pointer to NULL.
             	Thats IMO one of the most important issues relating
             	to pointers :

             	A pointer is valid or NULL.
*========================================================================*/
void 
FreeAllMem (INIFILE* thiz)
{
	struct ENTRY *pEntry;
	struct ENTRY *pNextEntry;
	pEntry = thiz->entry;
	while (1)
	{
		if (pEntry == NULL)
		{
			break;
		}
		pNextEntry = pEntry->pNext;
		FreeMem (pEntry->Text);	/* Frees the pointer if not NULL */
		FreeMem (pEntry);
		pEntry = pNextEntry;
	}
	thiz->entry = NULL;
	thiz->cur_entry = NULL;
}

/*=========================================================================
   FindSection : Searches the chained list for a section. The section
                 must be given without the brackets!
   Return Value: NULL at an error or a pointer to the ENTRY structure
                 if succeed.
*========================================================================*/
struct ENTRY *
FindSection (INIFILE* thiz, cchr * Section)
{
	char Sec[130];
	char iSec[130];
	struct ENTRY *pEntry;
	sprintf (Sec, "[%s]", Section);
	strupr (Sec);
	pEntry = thiz->entry;		/* Get a pointer to the first Entry */
	while (pEntry != NULL)
    {
		if (pEntry->Type == tpSECTION)
		{
			strcpy (iSec, pEntry->Text);
			strupr (iSec);
			if (strcmp (Sec, iSec) == 0)
			{
				return pEntry;
			}
		}
		pEntry = pEntry->pNext;
    }
	return NULL;
}

/*=========================================================================
   FindpKey     : Searches the chained list for a pKey under a given section
   Return Value: NULL at an error or a pointer to the ENTRY structure
                 if succeed.
*========================================================================*/
bool 
FindpKey (INIFILE* thiz, cchr * Section, cchr * pKey, EFIND * List)
{
	char Search[130];
	char Found[130];
	char Text[255];
	char *pText;
	struct ENTRY *pEntry;
	List->pSec = NULL;
	List->pKey = NULL;
	pEntry = FindSection (thiz, Section);
	if (pEntry == NULL)
	{
		return FALSE;
    }
	List->pSec = pEntry;
	List->KeyText[0] = 0;
	List->ValText[0] = 0;
	List->Comment[0] = 0;
	pEntry = pEntry->pNext;
	if (pEntry == NULL)
	{
		return FALSE;
    }
  	sprintf (Search, "%s", pKey);
  	//strupr (Search);
  	while (pEntry != NULL)
    {
      	if ((pEntry->Type == tpSECTION) ||	/* Stop after next section or EOF */
	  	(pEntry->Type == tpNULL))
		{
	  		return FALSE;
		}
      	if (pEntry->Type == tpKEYVALUE)
		{
			strcpy (Text, pEntry->Text);
			pText = strchr (Text, ';');
			if (pText != NULL)
			{
				strcpy (List->Comment, Text);
				*pText = 0;
			}
			pText = strchr (Text, '=');
	  		if (pText != NULL)
			{
				*pText = 0;
				strcpy (List->KeyText, Text);
				strcpy (Found, Text);
				*pText = '=';
				//strupr (Found);
				delblank(Found);
				/* printf ("%s,%s\n", Search, Found); */
				if (strcmp (Found, Search) == 0)
				{
					strcpy (List->ValText, pText + 1);
					delblank(List->ValText);
					List->pKey = pEntry;
					return TRUE;
				}
			}
		}
      	pEntry = pEntry->pNext;
    }
  	return FALSE;
}

/*=========================================================================
   AddItem  : Adds an item (pKey or section) to the chaines list
*========================================================================*/
bool 
AddItem (INIFILE* thiz, char Type, cchr * Text)
{
	struct ENTRY *pEntry = MakeNewEntry (thiz);
	if (pEntry == NULL)
	{
		return FALSE;
    }
	pEntry->Type = Type;
	pEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pEntry->Text == NULL)
    {
		free (pEntry);
		return FALSE;
    }
	strcpy (pEntry->Text, Text);
	pEntry->pNext = NULL;
	if (thiz->cur_entry != NULL)
    {
		thiz->cur_entry->pNext = pEntry;
    }
	thiz->cur_entry = pEntry;
	return TRUE;
}

/*=========================================================================
   AddItemAt : Adds an item at a selected position. This means, that the
               chained list will be broken at the selected position and
               that the new item will be Inserted.
               Before : A.Next = &B
               After  : A.Next = &NewItem, NewItem.Next = &B
*========================================================================*/
bool 
AddItemAt (struct ENTRY * EntryAt, char Mode, cchr * Text)
{
	struct ENTRY *pNewEntry;
	if (EntryAt == NULL)
    {
		return FALSE;
    }
	pNewEntry = (struct ENTRY *) malloc (sizeof (ENTRY));
	if (pNewEntry == NULL)
    {
		return FALSE;
    }
	pNewEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pNewEntry->Text == NULL)
    {
		free (pNewEntry);
		return FALSE;
    }
	strcpy (pNewEntry->Text, Text);
	if (EntryAt->pNext == NULL)	/* No following nodes. */
    {
		EntryAt->pNext = pNewEntry;
		pNewEntry->pNext = NULL;
    }
  else
    {
		pNewEntry->pNext = EntryAt->pNext;
		EntryAt->pNext = pNewEntry;
    }
	pNewEntry->pPrev = EntryAt;
	pNewEntry->Type = Mode;
	return TRUE;
}

/*=========================================================================
   AddSectionAndpKey  : Adds a section and then a pKey to the chained list
*========================================================================*/
bool 
AddSectionAndpKey (INIFILE* thiz, cchr * Section, cchr * pKey, cchr * Value)
{
	char Text[255];
	sprintf (Text, "[%s]", Section);
	if (AddItem (thiz, tpSECTION, Text) == FALSE)
    {
		return FALSE;
    }
	sprintf (Text, "%s=%s", pKey, Value);
	return AddItem (thiz, tpKEYVALUE, Text);
}

/*=========================================================================
   AddpKey  : Adds a pKey to the chained list
*========================================================================*/
void 
AddpKey (struct ENTRY *SecEntry, cchr * pKey, cchr * Value)
{
	char Text[255];
	sprintf (Text, "%s=%s", pKey, Value);
	AddItemAt (SecEntry, tpKEYVALUE, Text);
}

/*=========================================================================
   MakeNewEntry  : Allocates the memory for a new entry. This is only
                   the new empty structure, that must be filled from
                   function like AddItem etc.
   Info          : This is only a internal function. You dont have to call
                   it from outside.
*==========================================================================*/
struct ENTRY *
MakeNewEntry (INIFILE* thiz)
{
	struct ENTRY *pEntry;
	pEntry = (struct ENTRY *) malloc (sizeof (ENTRY));
	if (pEntry == NULL)
    {
		FreeAllMem (thiz);
		return NULL;
    }
	if (thiz->entry == NULL)
    {
		thiz->entry = pEntry;
    }
	pEntry->Type = tpNULL;
	pEntry->pPrev = thiz->cur_entry;
	pEntry->pNext = NULL;
	pEntry->Text = NULL;
	if (thiz->cur_entry != NULL)
    {
		thiz->cur_entry->pNext = pEntry;
    }
	return pEntry;
}

static INIFILE* create_inifile()
{
	INIFILE* inifile = calloc(sizeof(INIFILE), 1);
	
	inifile->write_string = write_string;
	inifile->write_bool = write_bool;
	inifile->write_int = write_int;
	inifile->write_double = write_double;

	inifile->read_string = read_string;
	inifile->read_bool = read_bool;
	inifile->read_int = read_int;
	inifile->read_double = read_double;

	inifile->delete_key = delete_key;

	return inifile;
}

/*#define INIFILE_TEST_THIS_FILE*/
#ifdef INIFILE_TEST_THIS_FILE
#define INIFILE_TEST_READ_AND_WRITE
int main (void)
{
	printf ("Hello World\n");
	OpenIniFile ("Test.Ini");
#ifdef INIFILE_TEST_READ_AND_WRITE
	write_string  ("Test", "Name", "Value");
	write_string  ("Test", "Name", "OverWrittenValue");
	write_string  ("Test", "Port", "COM1");
	write_string  ("Test", "User", "James Brown jr.");
	write_string  ("Configuration", "eDriver", "MBM2.VXD");
	write_string  ("Configuration", "Wrap", "LPT.VXD");
	write_int 	 ("IO-Port", "Com", 2);
	write_bool 	 ("IO-Port", "IsValid", 0);
	write_double  ("TheMoney", "TheMoney", 67892.00241);
	write_int     ("Test"    , "ToDelete", 1234);
	WriteIniFile ("Test.Ini");
	printf ("Key ToDelete created. Check ini file. Any key to continue");
	while (!kbhit());
	OpenIniFile  ("Test.ini");
	delete_key    ("Test"	  , "ToDelete");
	WriteIniFile ("Test.ini");
#endif
	printf ("[Test] Name = %s\n", read_string ("Test", "Name", "NotFound"));
	printf ("[Test] Port = %s\n", read_string ("Test", "Port", "NotFound"));
	printf ("[Test] User = %s\n", read_string ("Test", "User", "NotFound"));
	printf ("[Configuration] eDriver = %s\n", read_string ("Configuration", "eDriver", "NotFound"));
	printf ("[Configuration] Wrap = %s\n", read_string ("Configuration", "Wrap", "NotFound"));
	printf ("[IO-Port] Com = %d\n", read_int ("IO-Port", "Com", 0));
	printf ("[IO-Port] IsValid = %d\n", read_bool ("IO-Port", "IsValid", 0));
	printf ("[TheMoney] TheMoney = %1.10lf\n", read_double ("TheMoney", "TheMoney", 111));
	CloseIniFile ();
	return 0;
}
#endif
