// #HLP_BEGIN
#ifndef _CCB_CCTABLEVIEWLOADER_H_
#define _CCB_CCTABLEVIEWLOADER_H_

#include "CCNodeLoader.h"
#include "../GUI/CCScrollView/CCTableView.h"

NS_CC_EXT_BEGIN

/* Forward declaration. */
class CCBReader;

class CCTableViewLoader : public CCNodeLoader {
    public:
        virtual ~CCTableViewLoader() {};
        CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(CCTableViewLoader, loader);

    protected:
        CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(CCTableView);
		virtual void onHandlePropTypeSize(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCSize pSize, CCBReader * pCCBReader);
        virtual void onHandlePropTypeCCBFile(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCNode * pCCBFileNode, CCBReader * pCCBReader);
        virtual void onHandlePropTypeCheck(CCNode * pNode, CCNode * pParent, const char * pPropertyName, bool pCheck, CCBReader * pCCBReader);
        virtual void onHandlePropTypeFloat(CCNode * pNode, CCNode * pParent, const char * pPropertyName, float pFloat, CCBReader * pCCBReader);
        virtual void onHandlePropTypeIntegerLabeled(CCNode * pNode, CCNode * pParent, const char * pPropertyName, int pIntegerLabeled, CCBReader * pCCBReader);
};

NS_CC_EXT_END

#endif
// #HLP_END