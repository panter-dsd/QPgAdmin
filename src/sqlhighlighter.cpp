/********************************************************************
 * Copyright (C) PanteR
 *-------------------------------------------------------------------
 *
 * QPgAdmin is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * QPgAdmin is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Panther Commander; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *-------------------------------------------------------------------
 * Project:		QPgAdmin
 * Author:		PanteR
 * Contact:		panter.dsd@gmail.com
 *******************************************************************/

#include <QtGui/QTextDocument>

#include "sqlhighlighter.h"

//! [0]
SQLHighlighter::SQLHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{

}

void SQLHighlighter::highlightBlock(const QString &text)
{
	int pos = 0; 	

	setFormat (0, text.length(), Qt::black); 
	
	static const QRegExp commandsRegexp ("\\b(?:select|from|where|and|case|when|then|else|distinct|all|null|"
										 "is|like|between|not|group|by|having|order|inner|outer|right|left|alter|with|isnull|cast|create|replace|function|"
										 "returns|language|volatile|cost|table|view|or|"
										 "join|on|using|union|exists|in|as|intersect|except|coalesce|insert|into|update)\\b", 
										 Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = commandsRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, commandsRegexp.matchedLength(), Qt::magenta);
		pos += commandsRegexp.matchedLength();
	}

	static const QRegExp aggregationsRegexp ("\\b(?:count|min|max)\\b\\s*\\([^\\)]+\\)", 
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = aggregationsRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, aggregationsRegexp.matchedLength(), Qt::darkGreen);
		pos += aggregationsRegexp.matchedLength();
	}

	static const QRegExp numbersRegexp ("[^\\w]((\\d+)(\\.)?)",
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = numbersRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, numbersRegexp.matchedLength(), Qt::blue);
		pos += numbersRegexp.matchedLength();
	}

	static const QRegExp stringsRegexp ("'[^']+'", 
						   Qt::CaseInsensitive);
 	pos = 0;
	while ((pos = stringsRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, stringsRegexp.matchedLength(), Qt::red);
		pos += stringsRegexp.matchedLength();
	}

	pos = 0;
	static const QRegExp commentRegexp ("^\\s*(--)"); 	
	if ((pos = commentRegexp.indexIn (text, pos)) != -1)	{
		setFormat (pos, text.length(), Qt::blue);
		return;	
	} 	
}

