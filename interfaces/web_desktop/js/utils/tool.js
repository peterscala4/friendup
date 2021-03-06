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

var friendUP = friendUP || {};
friendUP.tool = friendUP.tool || {};

(function( ns )
{
	ns.getRandomNumber = function( length )
	{
		length = length || 15;
		var numString = '';
		while ( numString.length <= length )
		{
			var part = getNum();
			numString += part;
		}
		
		var slice = numString.slice( 0, length );
		return parseInt( slice, 10 );
		
		function getNum()
		{
			var decimal = Math.random();
			var movedDecimalPoint = decimal * Math.pow( 10, 10 );
			var integer = Math.floor( movedDecimalPoint );
			return integer.toString();
		}
	}
	
	ns.getRandomString = function( length )
	{
		var string = '';
		do
		{
			var part = '';
			var floater = Math.random();
			var base36 = floater.toString( 36 ); // .toString is magic, like friendship
			part =  base36.slice( 2 ); // removing the first two characters, they do not please me ( actually, only the 2nd, wich is a period, but thats what you get for being associated with a period )
			string += part;
		}
		while ( string.length < length )
		
		return string.slice( 0, length );
	}
	
	idCache = {}; // the best solution? possibly not.. >.>
	ns.getId = function( prefix, length )
	{
		var prefix = startWithAlpha( prefix ) || 'id';
		length = length || 36;
		length = Math.max( length, 11 );
		var partLength = 8;
		
		do
		{
			id = prefix;
			id = createId( id );
			id = id.slice(0, length );
			id = removeTrailingHyphen( id );
		}
		while( idCache[ id ])
		
		idCache[ id ] = id;
		return id;
		
		function createId( str )
		{
			do
			{
				var part = ns.getRandomString( partLength );
				str = str + '-' + part;
			}
			while ( str.length < length );
			return str;
		}
		
		function removeTrailingHyphen ( str )
		{
			var lastChar = str[ str.length-1 ];
			if ( lastChar == '-' )
				return str.slice( 0, str.length-1 );
			return str;
		}
		
		function startWithAlpha( prefix )
		{
			if ( typeof( prefix ) !== 'string' )
				return false;
			if ( !( prefix[ 0 ].match( /[a-zA-Z]/ )) )
				return 'id-' + prefix;
			return prefix;
		}
	}
	ns.uid = ns.getId;
	
	ns.stringify = function( obj )
	{
		if ( typeof obj === 'string' )
			return obj;
		
		try
		{
			return JSON.stringify( obj );
		} catch (e)
		{
			console.log( 'tool.Stringify.exception', e );
			console.log( 'tool.Stringifu.exception - returning .toString(): ', obj.toString());
			return obj.toString(); // not an object? probably has .toString() then.. #YOLO #360-NO-SCOPE
		}
	}
	
	ns.parse = function( string )
	{
		if ( typeof string !== 'string' )
			return string;
		
		try
		{
			return JSON.parse( string );
		} catch (e)
		{
			console.log( 'could not objectify:', string );
			return null;
		}
	}
	ns.objectify = ns.parse;
	
	ns.queryString = function( params )
	{
		if ( typeof( params ) === 'string' )
			return params;
			
		var pairs = Object.keys( params ).map( pair )
		function pair( key )
		{
			var value = params[ key ];
			return key + '=' + value;
		}
		
		return pairs.join( '&' );
	}
	
	ns.getCssValue = function( element, style, pseudo )
	{
		pseudo = pseudo || null; // css pesudo selector
		
		if ( element.style[ style ] && !pseudo )
			return element.style[ style ];
		
		if ( element.currentStyle )
			return element.currentStyle( style );
		
		if ( window.getComputedStyle )
		{
			var styles = window.getComputedStyle( element, pseudo );
			return styles[ style ];
		}
	}
	
	ns.ucfirst = function( string ) { // yes, it doesnt handle your prepending whitepsace, fix if you like
		if ( !string.length )
			return string;
		
		var arr = string.split( '' );
		arr[ 0 ] = arr[ 0 ].toUpperCase();
		string = arr.join( '' );
		return string;
	}
	
})( friendUP.tool );
