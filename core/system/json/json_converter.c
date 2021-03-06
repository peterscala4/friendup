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

#include "json_converter.h"
#include <time.h>
#include "json.h"
#include <core/nodes.h>
#include <util/string.h>

//
// DEBUG
//

static void print_depth_shift(int depth)
{
        int j;
        for (j=0; j < depth; j++) {
                printf(" ");
        }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) {
                print_depth_shift(depth);
                DEBUG("object[%d].name = %s\n", x, value->u.object.values[x].name);
                process_value(value->u.object.values[x].value, depth+1);
        }
}

static void process_array(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.array.length;
        DEBUG("array\n");
        for (x = 0; x < length; x++) {
                process_value(value->u.array.values[x], depth);
        }
}

static void process_value(json_value* value, int depth)
{
        int j;
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        
        DEBUG("PROCESSBALUE\n");
        
        switch (value->type) {
                case json_none:
                        DEBUG("none\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_integer:
                        DEBUG("int: %ld\n", value->u.integer);
                        break;
                case json_double:
                        DEBUG("double: %f\n", value->u.dbl);
                        break;
                case json_string:
                        DEBUG("string: %s\n", value->u.string.ptr);
                        break;
                case json_boolean:
                        DEBUG("bool: %d\n", value->u.boolean);
                        break;
				case json_null:
						DEBUG("JSON parse NULL\n");
					break;
        }
}

//
// DEBUG END
//

//
// get JSON structure from data
//

BufString *GetJSONFromStructure( ULONG *descr, void *data )
{
	BufString *bs = BufStringNew();
	if( bs == NULL )
	{
		ERROR("ERROR: bufstring is null\n");
		return NULL;
	}
	
	DEBUG("[GetJSONFromStructure] \n");
	
	if( descr == NULL || data == NULL )
	{
		BufStringDelete( bs );
		ERROR("Data structure or description was not provided!\n");
		return 0;
	}
	
	if( descr[ 0 ] != SQLT_TABNAME )
	{
		BufStringDelete( bs );
		ERROR("SQLT_TABNAME was not provided!\n");
		return 0;
	}
	
	DEBUG("JSONParse\n");

	ULONG *dptr = &descr[ SQL_DATA_STRUCT_START ];		// first 2 entries inform about table, rest information provided is about columns
	unsigned char *strptr = (unsigned char *)data;	// pointer to structure to which will will insert data
	int opt = 0;
	
	BufStringAdd( bs, "{" );
	
	while( dptr[0] != SQLT_END )
	{
		//DEBUG("Found on pos %d tag %d   row %s\n", i, dptr[ 0 ], row[ i ] ); 

		switch( dptr[ 0 ] )
		{
			case SQLT_IDINT:	// primary key
			case SQLT_INT:
				{
					char tmp[ 256 ];
					int tmpint;
					memcpy( &tmpint, strptr + dptr[2], sizeof( int ) );
					
					if( opt == 0 )
					{
						sprintf( tmp, "\"%s\": %d ", (char *)dptr[ 1 ], tmpint );
						BufStringAdd( bs, tmp );
					}
					else
					{
						sprintf( tmp, ", \"%s\": %d ", (char *)dptr[ 1 ], tmpint );
						BufStringAdd( bs, tmp );
					}
					
					opt++;
				}
				break;
				
			case SQLT_STR:
				{
					char tmp[ 512 ];
					char *tmpchar;
					memcpy( &tmpchar, strptr+dptr[2], sizeof( char *) );
					
					if( tmpchar != NULL )
					{
						if( opt == 0 )
						{
							sprintf( tmp, "\"%s\": \"%s\" ", (char *)dptr[ 1 ], tmpchar );
							BufStringAdd( bs, tmp );
						}
						else
						{
							sprintf( tmp, ", \"%s\": \"%s\" ", (char *)dptr[ 1 ], tmpchar );
							BufStringAdd( bs, tmp );
						}
					}
					
					opt++;
				}
				break;
				
			case SQLT_TIMESTAMP:
				{
					// '2015-08-10 16:28:31'
					char date[ 512 ];

					struct tm *tp = (struct tm *)( strptr+dptr[2]);
					if( opt == 0 )
					{
						sprintf( date, "\"%s\": \"%4d-%2d-%2d %2d:%2d:%2d\" ", (char *)dptr[ 1 ], tp->tm_year, tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec );
						BufStringAdd( bs, date );
					}
					else
					{
						sprintf( date, ", \"%s\": \"%4d-%2d-%2d %2d:%2d:%2d\" ", (char *)dptr[ 1 ], tp->tm_year, tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec );
						BufStringAdd( bs, date );
					}
					
					opt++;
				}
			break;
		}
		
		dptr += 3;
	}
	
	BufStringAdd( bs, "}" );
	//DEBUG("Object to JSON parse end %s\n", bs->bs_Buffer );
	
	return bs;
}

//
//
//

void *GetStructureFromJSON( ULONG *descr, const char *jsondata )
{
	char tmpQuery[ 1024 ];
	void *firstObject = NULL;
	void *lastObject = NULL;
	
	DEBUG("[GetStructureFromJSON] Load\n");
	
	if( jsondata == NULL  )
	{
		ERROR("Cannot parse NULL!\n");
		return NULL;
	}
	
	if( descr == NULL  )
	{
		ERROR("Data description was not provided!\n");
		return NULL;
	}
	
	if( descr[ 0 ] != SQLT_TABNAME )
	{
		ERROR("SQLT_TABNAME was not provided!\n");
		return NULL;
	}
	
	INFO("Start\n");
	
	int j = 0;
	json_char* json;
	json_value* value;
	MinNode *node = NULL;
	
		//[{'ID'= '1' , 'Name'= 'testowa' , 'API'= '11' , 'Version'= '1' , 'Author'= 'stefanek' , 'Email'= '1' , 'Description'= 'jacek@placek.pl' , 'PEGI'= '0' , 'DateCreated'= '18' , 'DateInstalled'= '2015' }]
	//json = (json_char*)"{ \"applications\" :  [  {\"ID\": \"1\" , \"Name\": \"testowa\" , \"API\": \"11\" , \"Version\": \"1\" , \"Author\": \"stefanek\" , \"Email\": \"1\" , \"Description\": \"jacek@placek.pl\" , \"PEGI\": \"0\" , \"DateCreated\": \"18\" , \"DateInstalled\": \"2015\" }] }";

	json = (json_char*)jsondata;
	
	//DEBUG("[GetStructureFromJSON] Before parse  -> '%s' \n", json );

	value = json_parse( json, strlen( json ) );

	if (value == NULL) 
	{
		ERROR("Cannot parse string to object\n");
		return NULL;
	}
	
	if( value->type == json_object || value->type == json_array )			// ''main object"
	{
		//DEBUG("OBJECT NAME = %s value array length %d\n", value->u.object.values[0].name, value->array.length );
		
		json_value* arrval;
		
		
		DEBUG("Parse arrval type %d value type %d \n", value->type, value->type );
		
		if( value->type == json_object )
		{
			void *data = calloc( 1, descr[ SQL_DATA_STRUCTURE_SIZE ] );
		
			UBYTE *strptr = (UBYTE *)data;	// pointer to structure to which will will insert data
			ULONG *dptr = &descr[ SQL_DATA_STRUCT_START ];		// first 2 entries inform about table and size, rest information provided is about columns
				
			unsigned int i;
			
			lastObject = data;
			
			for( i=0 ; i < value->u.object.length ; i++ )
			{
				//printf("------%d ---- %s\n", i, value->u.object.values[ i ].name );
			}
			
			//DEBUG("Found obejct\n");
			
			while( dptr[0] != SQLT_END )
			{
				switch( dptr[ 0 ] )
				{
					case SQLT_NODE:
					{
					}
					break;
					
					case SQLT_IDINT:	// primary key
					case SQLT_INT:
					{
						int retPos = -1;
						//DEBUG("FIND INT!\n");
						
						for( i = 0; i <  value->u.object.length; i++) 
						{
							//DEBUG("aaaaaaaaaaobject[%d].name = %s\n", i, locaval->u.object.values[i].name);
							if( strcmp( value->u.object.values[i].name, (char *) dptr[1] ) == 0 )
							{
								retPos = i;
							}
						}
						
						json_value*mval = NULL;
						if( retPos >= 0 )
						{
							mval = value->u.object.values[retPos].value;
						}
						
						if( retPos >= 0 && mval->type == json_integer )
						{
							//DEBUG("ENTRY FOUND %s  int val %d\n",(char *) dptr[ 1 ], mval->u.integer );
							memcpy( strptr + dptr[ 2 ], &(mval->u.integer), sizeof( int ) );
						}
					}
					break;
						
					case SQLT_STR:
					case SQLT_TIMESTAMP:
					{
						int retPos = -1;
						for( i = 0; i <  value->u.object.length; i++) 
						{
							//DEBUG("aaaaaaaaaaobject[%d].name = %s\n", i, value->u.object.values[i].name);
							if( strcmp( value->u.object.values[i].name, (char *)dptr[1] ) == 0 )
							{
								retPos = i;
							}
						}
						
						json_value*mval = NULL;
						
						if( retPos >= 0 )
						{
							mval = value->u.object.values[retPos].value;
						}
						
						if( retPos >= 0  && mval->type == json_string && mval->u.string.ptr != NULL )
						{
							//DEBUG("STRENTRY FOUND %s data %s\n", (char *)dptr[ 1 ], mval->u.string.ptr );
							
							// this is date
							if( strlen( mval->u.string.ptr ) == 19 && mval->u.string.ptr[ 5 ] == '-' && mval->u.string.ptr[ 8 ] == '-' )
							{
								char *ptr = NULL;
								struct tm ltm;
							
								//DEBUG("TIMESTAMP load\n");
							
								//ptr = strptime( row[ i ], "%C%y-%m-%d %H:%M:%S", &ltm );
								if( ptr != NULL )
								{
									// REMEMBER, data fix
								
									ltm.tm_year += 1900;
									ltm.tm_mon ++;
								
									memcpy( strptr+ dptr[ 2 ], &ltm, sizeof( struct tm) );
								
									//DEBUG("Year %d  month %d  day %d\n", ltm.tm_year, ltm.tm_mon, ltm.tm_mday );
								}
							}
							else		// this is string
							{
								char *tmpval = StringDuplicate( mval->u.string.ptr );
								memcpy( strptr+ dptr[ 2 ], &tmpval, sizeof( char *) );
							}
						}
					}
					break;
				}
				i++;
				dptr += 3;
			}
		}
		else if( value->type == json_array )		// object contain our objects
		{
			arrval = value;
			
			int length = value->u.array.length;
			int x;
			for (x = 0; x < length; x++) // get object from array
			{
				json_value*locaval = value->u.array.values[x]; 
				
				void *data = calloc( 1, descr[ SQL_DATA_STRUCTURE_SIZE ] );
				if( firstObject == NULL )
				{
					firstObject = data;
				}
		
				UBYTE *strptr = (UBYTE *)data;	// pointer to structure to which will will insert data
				ULONG *dptr = &descr[ SQL_DATA_STRUCT_START ];		// first 2 entries inform about table and size, rest information provided is about columns
				
				int intlength = locaval->u.object.length;
				int i;
				
				while( dptr[0] != SQLT_END )
				{
					switch( dptr[ 0 ] )
					{
						case SQLT_NODE:
						{
							//DEBUG("Node found\n");
							MinNode *locnode = (MinNode *)(data + dptr[ 2 ]);
							locnode->mln_Succ = (MinNode *)lastObject;
							
							//DEBUG("\n\nlastObject %x currobject %x\n\n", lastObject, data );
						}
						break;
					
						case SQLT_IDINT:	// primary key
						case SQLT_INT:
						{
							int retPos = -1;
							for( i = 0; i < intlength; i++) 
							{
								//DEBUG("aaaaaaaaaaobject[%d].name = %s\n", i, locaval->u.object.values[i].name);
								if( strcmp( locaval->u.object.values[i].name, (char *) dptr[1] ) == 0 )
								{
									retPos = i;
								}
							}
							json_value*mval = locaval->u.object.values[retPos].value;
							
							if( retPos >= 0 && mval->type == json_integer )
							{
								//DEBUG("ENTRY FOUND %s  int val %d\n",(char *) dptr[ 1 ], mval->u.integer );
								memcpy( strptr + dptr[ 2 ], &(mval->u.integer), sizeof( int ) );
							}
						}
						break;
						
						case SQLT_STR:
						case SQLT_TIMESTAMP:
						{
							int retPos = -1;
							for( i = 0; i < intlength; i++) 
							{
								//DEBUG("aaaaaaaaaaobject[%d].name = %s\n", i, locaval->u.object.values[i].name);
								if( strcmp( locaval->u.object.values[i].name, (char *)dptr[1] ) == 0 )
								{
									retPos = i;
								}
							}
							
							json_value*mval = locaval->u.object.values[retPos].value;
							
							if( retPos >= 0  && mval->type == json_string && mval->u.string.ptr != NULL )
							{
								//DEBUG("STRENTRY FOUND %s data %s\n", (char *)dptr[ 1 ], mval->u.string.ptr );
								
								// this is date
								if( strlen( mval->u.string.ptr ) == 19 && mval->u.string.ptr[ 5 ] == '-' && mval->u.string.ptr[ 8 ] == '-' )
								{
									char *ptr = NULL;
									struct tm ltm;
							
									//DEBUG("TIMESTAMP load\n");
							
									//ptr = strptime( row[ i ], "%C%y-%m-%d %H:%M:%S", &ltm );
									if( ptr != NULL )
									{
										// REMEMBER, data fix
								
										ltm.tm_year += 1900;
										ltm.tm_mon ++;
								
										memcpy( strptr+ dptr[ 2 ], &ltm, sizeof( struct tm) );
								
										//DEBUG("Year %d  month %d  day %d\n", ltm.tm_year, ltm.tm_mon, ltm.tm_mday );
									}
								}
								else		// this is string
								{
									char *tmpval = StringDuplicate( mval->u.string.ptr );
									memcpy( strptr+ dptr[ 2 ], &tmpval, sizeof( char *) );
								}
							}
						}
						break;
					}
					i++;
					dptr += 3;
				}
				
				lastObject = data;
			}
		}
	}
	firstObject = lastObject;

	json_value_free( value );
	
	return firstObject;
}
