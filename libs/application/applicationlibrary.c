/*******************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Affero General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* GNU Affero General Public License for more details.                          *
*                                                                              *
* You should have received a copy of the GNU Affero General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
*                                                                              *
*******************************************************************************/

/*

	ApplicationLibrary code

*/

#include <core/types.h>
#include <core/library.h>
#include "applicationlibrary.h"
#include <util/log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <util/string.h>
#include <openssl/sha.h>
#include <propertieslibrary.h>
#include <user/userlibrary.h>
#include <system/application/application.h>
#include <util/buffered_string.h>
#include <system/json/json_converter.h>

#define LIB_NAME "application.library"
#define LIB_VERSION			1
#define LIB_REVISION		0

//
// init library
//

void *libInit( void *sb )
{
	struct ApplicationLibrary *l;
	DEBUG("Application.library libinit\n");

	if( ( l = calloc( sizeof( struct ApplicationLibrary ), 1 ) ) == NULL )
	{
		return NULL;
	}


	l->l_Name = LIB_NAME;
	l->l_Version = LIB_VERSION;
	//l->libInit//no need
	l->libClose = dlsym ( l->handle, "libClose");
	l->GetVersion = dlsym ( l->handle, "GetVersion");
	l->GetRevision = dlsym( l->handle, "GetRevision");
	l->GetApplicationFromDB  = dlsym( l->handle, "GetApplicationFromDB" );
/*
	// application.library structure
	l->UserExist = dlsym ( l->handle, "UserExist");
	l->Authenticate = dlsym ( l->handle, "Authenticate");
	l->IsSessionValid = dlsym ( l->handle, "IsSessionValid");
	l->UserCreate = dlsym ( l->handle, "UserCreate");
	l->UserFree = dlsym( l->handle, "UserFree");
	l->UserGetBySession = dlsym ( l->handle, "UserGetBySession");
	l->UserGet = dlsym ( l->handle, "UserGet");
	l->SetFullName = dlsym ( l->handle, "SetFullName");
	l->SetEmail = dlsym ( l->handle, "SetEmail");
	l->UserUpdateDb = dlsym( l->handle, "UserUpdateDb");
	*/
	l->sb = sb;

	l->AppWebRequest = dlsym( l->handle, "AppWebRequest" );

	l->al_sqllib = (struct MYSQLLibrary *)LibraryOpen( sb, "mysql.library", 0 );
	
	l->al_zlib = (struct ZLibrary *)LibraryOpen( sb, "z.library", 0 );
	
	l->al_ApplicationList = GetApplicationFromDB( l, NULL );

	
	return l;
}

//
//
//

void libClose( struct ApplicationLibrary *l )
{
	
	DEBUG("Closing connections\n");

	
	Application *ar = l->al_ApplicationList;
	Application *an = ar;
	while( ar != NULL )
	{
		an = (Application *)an->node.mln_Succ;
	}
	
	if( l->al_zlib != NULL )
	{
		LibraryClose( (struct Library *)l->al_zlib );
	}
	
	if( l->al_sqllib != NULL )
	{
		LibraryClose( (struct Library *)l->al_sqllib );
	}
	
	DEBUG("Application.library close\n");
}

//
//
//

long GetVersion(void)
{
	return LIB_VERSION;
}

long GetRevision(void)
{
	return LIB_REVISION;
}


//
// application data free
//

void ApplicationFree( struct ApplicationLibrary *l, Application *app )
{
	
	/*
	if( user->u_Groups != NULL )
	{
		free( user->u_Groups );
	}
	
	DEBUG("User free\n");
	if( user->u_Name != NULL )
	{
		free( user->u_Name ); user->u_Name = NULL;
	}
	DEBUG("User free user\n");

	if( user->u_Password != NULL )
	{
		free( user->u_Password ); user->u_Password = NULL;
	}
	
	DEBUG("User free pass\n");

	if( user->u_SessionID )
	{
		free( user->u_SessionID ); user->u_SessionID = NULL;
	}
	DEBUG("User free END\n");
*/
	free( app );
}

//
//
//

int SetSQLConnection( struct ApplicationLibrary *l, MYSQLLibrary *lib )
{
	l->al_sqllib = lib;
	
	
	
	return 0;
}

//
// get applications from DB
//

Application *GetApplicationFromDB( struct ApplicationLibrary *l, const char *where )
{
	// TODO: This absolutely don't work! Fix it :D
	//if( l->al_sqllib )
	//	return l->al_sqllib->Load( l->al_sqllib, ApplicationDesc, (char *)where );
	return NULL;
}

//
// network handler
//

Http* AppWebRequest( struct ApplicationLibrary *l, char **urlpath, Http* request, Socket_t* sock )
{
	DEBUG("APPLIBRARY WEBREQUEST %s\n", urlpath[ 0 ] );
	Http* response = NULL;
	
	if( strcmp( urlpath[ 0 ], "help" ) == 0 )
	{
		struct TagItem tags[] = {
			{ HTTP_HEADER_CONTENT_TYPE, (ULONG)  StringDuplicate( "text/html" ) },
			{	HTTP_HEADER_CONNECTION, (ULONG)StringDuplicate( "close" ) },
			{TAG_DONE, TAG_DONE}
		};
		
		response = HttpNewSimple( HTTP_200_OK,  tags );
		
		HttpAddTextContent( response, \
			"list - return installed application list\n \
			register - register application in db \
			install - install application for user \
			remove - remove application \
			getPermissions - get permissions for application \
			" );			// out of memory/user not found
		
		//HttpWriteAndFree( response );
	
		//
		// list of all applications avaiable on server
		//
		
	}
	else if( strcmp( urlpath[ 0 ], "list" ) == 0 )
	{
		struct TagItem tags[] = {
			{ HTTP_HEADER_CONTENT_TYPE, (ULONG)  StringDuplicate( "text/html" ) },
			{	HTTP_HEADER_CONNECTION, (ULONG)StringDuplicate( "close" ) },
			{TAG_DONE, TAG_DONE}
		};
		
			response = HttpNewSimple( HTTP_200_OK,  tags );
		
		BufString *bs = BufStringNew();
		if( bs != NULL )
		{
			int pos = 0;
			Application *al = l->al_ApplicationList;
			
			//DEBUG("Application.library LIST\n");
			
			BufStringAdd( bs, " { \"Application\": [" );
			
			while( al != NULL )
			{
				if( pos > 0 )
				{
					BufStringAdd( bs, ", " );
				}
				else
				{
					//BufStringAdd( bs, "{" );
				}
				
				BufString *lbs = GetJSONFromStructure( ApplicationDesc, al );
				
				if( lbs != NULL )
				{
					//DEBUG("Parse application entry %s !\n" , lbs->bs_Buffer );
					
					int msg = BufStringAddSize( bs, lbs->bs_Buffer, lbs->bs_Size );
					//INFO("JSON LBS: %s\n", bs->bs_Buffer );
					//DEBUG("PARSE INFO %d size %d\n", msg, lbs->bs_Size );
				
					BufStringDelete( lbs );
				}
				
				//INFO("JSON INFO: 1 %s\n", bs->bs_Buffer );
				//BufStringAdd( bs, "}" );
				
				al = (Application *)al->node.mln_Succ;
				pos++;
			}
			//INFO("JSON INFO 2: %s\n", bs->bs_Buffer );
			
			BufStringAdd( bs, "  ]}" );
			
			INFO("JSON INFO: %s\n", bs->bs_Buffer );
			
			// test purpose
			
			//Application *app =	GetStructureFromJSON( ApplicationDesc, bs->bs_Buffer );
			
			//DEBUG("----> %s\n" , app->a_Name );
			
			HttpAddTextContent( response, bs->bs_Buffer );
			
			BufStringDelete( bs );
		}
		else
		{
			ERROR("ERROR: Cannot allocate memory for BufferString\n");
		}
		
		//HttpWriteAndFree( response );
		
	}else if( strcmp( urlpath[ 0 ], "install" ) == 0 )
	{
		char *url = NULL;
		
		struct TagItem tags[] = {
			{ HTTP_HEADER_CONTENT_TYPE, (ULONG)  StringDuplicate( "text/html" ) },
			{	HTTP_HEADER_CONNECTION, (ULONG)StringDuplicate( "close" ) },
			{TAG_DONE, TAG_DONE}
		};
		
		response = HttpNewSimple( HTTP_200_OK,  tags );
		
		HashmapElement *el =  HashmapGet( request->uri->query, "url" );
		if( el != NULL )
		{
			url = UrlDecodeToMem( ( char *)el->data );
		}
		
		if( url != NULL )
		{
			
		}
		
		//HttpWriteAndFree( response );
		
		if( url != NULL )
		{
			free( url );
		}
	}
	else
	{
		struct TagItem tags[] = {
			{	HTTP_HEADER_CONNECTION, (ULONG)StringDuplicate( "close" ) },
			{TAG_DONE, TAG_DONE}
		};
		
		response = HttpNewSimple( HTTP_404_NOT_FOUND,  tags );
	
		//HttpWriteAndFree( response );
		return response;
	}
		
/*
						//request->query;
						//
						// PARAMETERS SHOULD BE TAKEN FROM
						// POST NOT GET
						
		if( request->uri->query != NULL )
		{
			char *usr = NULL;
			char *pass = NULL;
							
			HashmapElement_t *el =  HashmapGet( request->uri->query, "username" );
			if( el != NULL )
			{
				usr = (char *)el->data;
			}
							
			el =  HashmapGet( request->uri->query, "password" );
			if( el != NULL )
			{
				pass = (char *)el->data;
			}
							
			if( usr != NULL && pass != NULL )
			{
				User *loggedUser = l->Authenticate( l, usr, pass, NULL );
				if( loggedUser != NULL )
				{
					char tmp[ 20 ];
					sprintf( tmp, "LERR: %d\n", loggedUser->u_Error );	// check user.library to display errors
					HttpAddTextContent( response, tmp );
				}else{
					HttpAddTextContent( response, "LERR: -1" );			// out of memory/user not found
				}
			}
		}
		DEBUG("user login response\n");

		HttpWriteAndFree( response, sock );
		result = 200;
	}else
	{
		Http404( sock );
		return 404;
	}
	*/
	return response;
}

//
// additional stuff
//

char* StringDuplicate( const char* str )
{
	DEBUG("SD str ptr %p\n", str );
	if( str == NULL )
	{
		return NULL;
	}
	int size = strlen( str );
	
	DEBUG("String duplacate %d\n", size );
	
	char *tmp = calloc( size + 1, sizeof( char ) );
	if( tmp == NULL )
	{
		return NULL;
	}
	
	return strcpy( tmp, str );
}

//
//
//

char *MakeString ( int length )
{
	length++;
	char *c = calloc ( length, sizeof ( char ) );
	if ( c != NULL )
	{
		memset( c, 0, length );
		//bzero ( ( void *)c, length );
		return c;
	}
	return NULL;
	//return calloc ( length + 1, sizeof ( char ) );
}

// Create HASH
// TODO: Use stronger key!
//

void HashedString ( char **str )
{
	printf ( "[HashedString] Hashing\n" );
	unsigned char temp[SHA_DIGEST_LENGTH];
	memset( temp, 0x0, SHA_DIGEST_LENGTH );
	
	char *buf = MakeString ( SHA_DIGEST_LENGTH * 2 );

	SHA1( ( unsigned char *)*str, strlen( *str ), temp);

	int i = 0;
	for ( ; i < SHA_DIGEST_LENGTH; i++ )
		sprintf( (char*)&(buf[i*2]), "%02x", temp[i] );

	if ( *str ) free ( *str );
	*str = buf;
	printf ( "[HashedString] Hashed\n" );
}



