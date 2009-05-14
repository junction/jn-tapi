#ifndef GLOOX_HELPER_H
#define GLOOX_HELPER_H

#include "onsip.h"

class TagHelper
{
public:

	// Retrieve the text of the specified child element (by its name)
	// Returns "" if child element error
	static string getChildText(const Tag *parentTag,const string& childName)
	{
		if ( parentTag == NULL )
			return EmptyString;
		Tag* child = parentTag->findChild(childName);
		if ( child == NULL )
			return EmptyString;
		return child->cdata();
	}
};

#endif