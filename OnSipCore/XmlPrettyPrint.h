#ifndef XMLPRETTY_PRINT_H
#define XMLPRETTY_PRINT_H

#include "onsip.h"

class XmlPrettyPrint : TagHandler
{
private:
	auto_ptr<Parser> m_parser;
	std::list<tstring> m_prints;
protected:
	virtual void handleTag(gloox::Tag *tag);
	static tstring prettyPrint(Tag *t,const tstring& indent);

public:
	static tstring prettyPrint(Tag *t);

	tstring prettyPrint(const tstring& xml);
};

#endif