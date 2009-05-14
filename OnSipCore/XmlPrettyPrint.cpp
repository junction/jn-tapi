
#include "stdafx.h"
#include "xmlprettyprint.h"

#define INDENT "     "

//static
tstring XmlPrettyPrint::prettyPrint(Tag *t)
{
	return prettyPrint(t,"");
}

//static
tstring XmlPrettyPrint::prettyPrint(Tag *t,const tstring& indent)
{
    std::string xml = indent + "<";

	if( !t->prefix().empty() )
    {
      xml += t->prefix();
      xml += ':';
    }

	xml += t->name();
	if( !t->attributes().empty() )
    {
	  Tag::AttributeList::const_iterator it_a = t->attributes().begin();
      for( ; it_a != t->attributes().end(); ++it_a )
      {
        xml += (*it_a)->xml();
      }
    }

	if ( t->children().empty() && t->cdata().empty() )
	{
      xml += "/>\r\n";
	}
    else
    {
      xml += ">\r\n";

	  const TagList tags = t->children();
	  std::list<Tag*>::const_iterator it_n = tags.begin();
      for( ; it_n != tags.end(); ++it_n )
      {
		  Tag* t = (*it_n);
		  tstring xx = prettyPrint(t,indent+INDENT);
		  xml += xx;
      }

	  if ( !t->cdata().empty() )
		  xml += indent + INDENT + t->cdata() + "\r\n";
      xml += indent + "</";
	  if( !t->prefix().empty() )
      {
        xml += t->prefix();
        xml += ':';
      }
      xml += t->name();
      xml += ">\r\n";
    }

    return xml;
}

//virtual
void XmlPrettyPrint::handleTag(gloox::Tag *tag)
{
	m_prints.push_back( prettyPrint(tag,"") );
}

tstring XmlPrettyPrint::prettyPrint(const tstring& xml)
{
	if ( m_parser.get() == NULL )
		m_parser.reset( new Parser(this) );

	// Clear any previous strings
	m_prints.clear();
	tstring temp = xml;
	// Do the parse, this will result in calls to handleTag()
	m_parser->feed(temp);

	// Concatenate each of the xml tags
	tstring ret;
	std::list<tstring>::iterator iter = m_prints.begin();
	while ( iter != m_prints.end() )
	{
		ret += *iter;
		iter++;
	}
	return ret;
}

