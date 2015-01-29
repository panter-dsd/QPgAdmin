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
 * Project:     QPgAdmin
 * Author:      PanteR
 * Contact:     panter.dsd@gmail.com
 *******************************************************************/

#include <QtGui/QTextDocument>

#include "sqlhighlighter.h"

//! [0]
SQLHighlighter::SQLHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{

}

void SQLHighlighter::setFormatByRegExp(const QRegExp &re, const QString &text, const QColor &color)
{
	for (int i = re.indexIn(text, 0); i != -1; i = re.indexIn(text, i + re.matchedLength())) {
		setFormat(i, re.matchedLength(), color);
	}
}

void SQLHighlighter::highlightBlock(const QString &text)
{
	setFormat(0, text.length(), Qt::gray);

	const QRegExp commandsRegexp("\\b(?:select|from|where|and|case|when|then|else|distinct|all|null|"
								 "is|like|between|not|group|by|having|order|inner|outer|right|left|alter|with|isnull|cast|create|replace|function|"
								 "returns|language|volatile|cost|table|view|or|"
								 "asc|desc|"
								 "join|on|using|union|exists|in|as|intersect|except|coalesce|insert|into|update)\\b",
								 Qt::CaseInsensitive);

	setFormatByRegExp(commandsRegexp, text, Qt::magenta);

	const QRegExp aggregationsRegexp("\\b(?:count|min|max)\\b\\s*\\([^\\)]+\\)",
									 Qt::CaseInsensitive);
	setFormatByRegExp(aggregationsRegexp, text, Qt::darkGreen);

	const QRegExp numbersRegexp("[^\\w]((\\d+)(\\.)?)",
								Qt::CaseInsensitive);
	setFormatByRegExp(numbersRegexp, text, Qt::blue);

	const QRegExp stringsRegexp("'[^']+'",
								Qt::CaseInsensitive);
	setFormatByRegExp(stringsRegexp, text, Qt::red);

	const QRegExp commentRegexp("^\\s*(--)");
	setFormatByRegExp(commentRegexp, text, Qt::blue);
}

