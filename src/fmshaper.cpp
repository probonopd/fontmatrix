//
// C++ Implementation: fmshaper
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fmshaper.h"

#include <QList>
#include <QMap>
#include <QDebug>
#include <QVarLengthArray>


HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
{
	return (HB_GraphemeClass) 0;
}
HB_WordClass HB_GetWordClass(HB_UChar32 ch)
{
	return (HB_WordClass) 0;
}
HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
{
	return (HB_SentenceClass) 0;
}

void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
{
	//###
}

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
	FT_Face face = (FT_Face)font;
	FT_ULong ftlen = *length;
	FT_Error error = 0;

	if (!FT_IS_SFNT(face))
		return HB_Err_Invalid_Argument;

	error = FT_Load_Sfnt_Table(face, tableTag, 0, buffer, &ftlen);
	*length = ftlen;
	return (HB_Error)error;
}

/***************** UTILS ************/
namespace 
{	
	HB_Script script2script ( QString script )
	{
		QMap<QString, HB_Script> hbscmap;
		hbscmap["arab"] = HB_Script_Arabic ;
		hbscmap["armn"] = HB_Script_Armenian ;
		hbscmap["beng"] = HB_Script_Bengali ;
		hbscmap["cyrl"] = HB_Script_Cyrillic ;
		hbscmap["deva"] = HB_Script_Devanagari ;
		hbscmap["geor"] = HB_Script_Georgian ;
		hbscmap["grek"] = HB_Script_Greek ;
		hbscmap["gujr"] = HB_Script_Gujarati ;
		hbscmap["guru"] = HB_Script_Gurmukhi ;
		hbscmap["hang"] = HB_Script_Hangul ;
		hbscmap["hebr"] = HB_Script_Hebrew ;
		hbscmap["knda"] = HB_Script_Kannada ;
		hbscmap["khmr"] = HB_Script_Khmer ;
		hbscmap["lao "] = HB_Script_Lao ;
		hbscmap["mlym"] = HB_Script_Malayalam ;
		hbscmap["mymr"] = HB_Script_Myanmar ;
		hbscmap["ogam"] = HB_Script_Ogham ;
		hbscmap["orya"] = HB_Script_Oriya ;
		hbscmap["runr"] = HB_Script_Runic ;
		hbscmap["sinh"] = HB_Script_Sinhala ;
		hbscmap["syrc"] = HB_Script_Syriac ;
		hbscmap["taml"] = HB_Script_Tamil ;
		hbscmap["telu"] = HB_Script_Telugu ;
		hbscmap["thaa"] = HB_Script_Thaana ;
		hbscmap["thai"] = HB_Script_Thai ;
		hbscmap["tibt"] = HB_Script_Tibetan ;
	
		HB_Script ret = hbscmap.contains ( script ) ? hbscmap[script] : HB_Script_Common;
		return   ret;
	}

}

FmShaper::FmShaper(FmOtf *anchor)
	:anchorOTF(anchor)
{
	faceisset = langisset = allocated = false;
	setFont();
	qDebug() << "FmShaper "<< this <<" created";
}

FmShaper::~ FmShaper()
{
	qDebug() << "FmShaper "<< this <<" destructor";
	if (faceisset)
	{
		delete m.font;
		HB_FreeFace(m.face);
	}
	qDebug() << "FmShaper "<< this <<" destroyed";
}

bool FmShaper::setFont (/*FT_Face face, HB_Font font  */)
{
	HB_Face hbFace = HB_NewFace(anchorOTF->_face, hb_getSFntTable);
	HB_Font hbFont = new HB_FontRec;
	
	
	hbFont->klass = anchorOTF->hbFont.klass ;
	hbFont->userData = anchorOTF->hbFont.userData;
	hbFont->x_ppem  = anchorOTF->hbFont.x_ppem;
	hbFont->y_ppem  = anchorOTF->hbFont.y_ppem;
	hbFont->x_scale = anchorOTF->hbFont.x_scale;
	hbFont->y_scale = anchorOTF->hbFont.y_scale;
	m.font = hbFont;
	m.face = hbFace;
	
	anchorFace = anchorOTF->_face;
	faceisset = true;
	return faceisset;
}


bool FmShaper::setScript ( QString script )
{
	HB_Script ret = script2script ( script );
	if ( ret != HB_Script_Common )
	{
		m.item.script = ret;
		return true;
	}
	return false;
}

QList< RenderedGlyph > FmShaper::doShape(QString string, bool ltr)
{
	qDebug() << "FmShaper::doShape("<<string<<","<<ltr<<")";
	m.kerning_applied = false;
	m.string = reinterpret_cast<const HB_UChar16 *> ( string.constData() );
	m.stringLength = string.length();
	m.item.pos = 0;
	m.item.bidiLevel =  0;
	m.shaperFlags = HB_ShaperFlag_UseDesignMetrics;
	
	m.initialGlyphCount = m.num_glyphs = m.item.length = m.stringLength;
	m.glyphIndicesPresent = false;
	
	
// 	m.glyphs = new HB_Glyph[ m.num_glyphs ];
// 	for(int i = 0; i < m.num_glyphs; ++i)
// 	{
// 		m.glyphs[i] = FT_Get_Char_Index ( anchorFace , string[i].unicode() );
// 		qDebug() << "ADDED "<<string[i]<<" as " << m.glyphs[i];
// 	}
// 	
// 	
/*
	if ( allocated )
	{
		delete  m.glyphs;
		delete  m.attributes;
		delete  m.advances;
		delete  m.offsets;
		delete  m.log_clusters;
	}*/
	int neededspace = m.num_glyphs  ;

	QVarLengthArray<HB_Glyph> hb_glyphs(neededspace);
	QVarLengthArray<HB_GlyphAttributes> hb_attributes(neededspace);
	QVarLengthArray<HB_Fixed> hb_advances(neededspace);
	QVarLengthArray<HB_FixedPoint> hb_offsets(neededspace);
	QVarLengthArray<unsigned short> hb_logClusters(neededspace);

	HB_Bool result = false;
// 	m.glyphs = new HB_Glyph[neededspace];
// 	memset ( m.glyphs, 0, neededspace * sizeof ( HB_Glyph ) );
// 	m.attributes = new HB_GlyphAttributes[neededspace];
// 	memset ( m.attributes, 0, neededspace * sizeof ( HB_GlyphAttributes ) );
// 	m.advances = new HB_Fixed[neededspace];
// 	memset ( m.advances, 0, neededspace * sizeof ( HB_Fixed ) );
// 	m.offsets = new HB_FixedPoint[neededspace];
// 	memset ( m.offsets, 0, neededspace * sizeof ( HB_FixedPoint ) );
// 	m.log_clusters = new unsigned short[neededspace];
// 
// 	allocated = true;
// 
// 	HB_Bool result = HB_ShapeItem ( &m );
// 	qDebug() << "----------------------------------------------ShapeItem run1-----------";
	int iter = 0;
	while ( !result )
	{
		neededspace = m.num_glyphs  ;

// 		delete m.glyphs;
// 		delete m.attributes;
// 		delete m.advances;
// 		delete m.offsets;
// 		delete m.log_clusters;
// 		
// 		qDebug() << "----------------------------------------------item cleaned------------";
// 
// 		m.glyphs = new HB_Glyph[neededspace];
// 		memset ( m.glyphs, 0, neededspace * sizeof ( HB_Glyph ) );
// 		m.attributes = new HB_GlyphAttributes[neededspace];
// 		memset ( m.attributes, 0, neededspace * sizeof ( HB_GlyphAttributes ) );
// 		m.advances = new HB_Fixed[neededspace];
// 		memset ( m.advances, 0, neededspace * sizeof ( HB_Fixed ) );
// 		m.offsets = new HB_FixedPoint[neededspace];
// 		memset ( m.offsets, 0, neededspace * sizeof ( HB_FixedPoint ) );
// 		m.log_clusters = new unsigned short[neededspace];
		
		hb_glyphs.resize(neededspace);
		hb_attributes.resize(neededspace);
		hb_advances.resize(neededspace);
		hb_offsets.resize(neededspace);
		hb_logClusters.resize(neededspace);

		memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(HB_Glyph));
		memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(HB_GlyphAttributes));
		memset(hb_advances.data(), 0, hb_advances.size() * sizeof(HB_Fixed));
		memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(HB_FixedPoint));

		m.glyphs = hb_glyphs.data();
		m.attributes = hb_attributes.data();
		m.advances = hb_advances.data();
		m.offsets = hb_offsets.data();
		m.log_clusters = hb_logClusters.data();
		
		qDebug() << "----------------------------------------------item allocated------------";
		result = HB_ShapeItem ( &m );
		qDebug() << "----------------------------------------------ShapeItem run"<<++iter<<"-"<< result <<"----------";
	}
	
	
	QList<RenderedGlyph> renderedString;
	int base = 0;
	int baseCorrection = 0;
	for(int gIndex = 0; gIndex < m.num_glyphs; ++gIndex)
	{
		HB_GlyphAttributes attr = m.attributes[gIndex];
		qDebug()<< "ATTR("<< m.glyphs[gIndex] 
				<< ") combiningClass = " << attr.combiningClass
				<< "; clusterStart =" << attr.clusterStart
				<< "; mark = "<< attr.mark;
		if(m.attributes[gIndex].clusterStart )
		{
			base = gIndex;
			baseCorrection = 0;
		}
// 		if(m.attributes[gIndex].mark )
		else
		{
			qDebug() << "catch a mark";
// 			for(int b=base; b < gIndex; ++b)
			{
				baseCorrection = renderedString[base].xadvance;
			}
		}
			RenderedGlyph gl;
			gl.glyph = m.glyphs[gIndex];
			gl.xadvance = ltr ? ( double ) ( m.advances[gIndex]) :( double ) ( -m.advances[gIndex]) ;
			gl.yadvance = 0.0;
			gl.xoffset = ltr ? ( m.offsets[gIndex].x  - baseCorrection ) : ( baseCorrection - m.offsets[gIndex].x );
			gl.yoffset = m.offsets[gIndex].y ;
				
			renderedString << gl;
	}
	qDebug() << "EndOf FmShaper::doShape("<<string<<","<<ltr<<")";
	return renderedString;
}



HB_Buffer  FmShaper::out_buffer()
{
	return m.face->buffer;
}




