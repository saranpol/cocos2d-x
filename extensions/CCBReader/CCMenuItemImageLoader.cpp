#include "CCMenuItemImageLoader.h"



#define PROPERTY_NORMALDISPLAYFRAME "normalSpriteFrame"
#define PROPERTY_SELECTEDDISPLAYFRAME "selectedSpriteFrame"
#define PROPERTY_DISABLEDDISPLAYFRAME "disabledSpriteFrame"

NS_CC_EXT_BEGIN

void CCMenuItemImageLoader::onHandlePropTypeSpriteFrame(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCSpriteFrame * pCCSpriteFrame, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_NORMALDISPLAYFRAME) == 0) {
        if(pCCSpriteFrame != NULL) {
            ((CCMenuItemImage *)pNode)->setNormalSpriteFrame(pCCSpriteFrame);
            // #HLP_BEGIN
            // #resource_scale_fix
            CCMenuItemImage *item = (CCMenuItemImage *)pNode;
            item->setNormalSpriteFrame(pCCSpriteFrame);
            if(item->getTag() != TAG_INTERNET_MENU_ITEM_IMAGE){
                item->setScale(item->getScale() * CC_CONTENT_SCALE_FACTOR() / CC_RESOURCE_SCALE_FACTOR());
            }
            // #HLP_END
        }
    } else if(strcmp(pPropertyName, PROPERTY_SELECTEDDISPLAYFRAME) == 0) {
        if(pCCSpriteFrame != NULL) {
            ((CCMenuItemImage *)pNode)->setSelectedSpriteFrame(pCCSpriteFrame);
        }
    } else if(strcmp(pPropertyName, PROPERTY_DISABLEDDISPLAYFRAME) == 0) {
        if(pCCSpriteFrame != NULL) {
            ((CCMenuItemImage *)pNode)->setDisabledSpriteFrame(pCCSpriteFrame);
        }
    } else {
        CCMenuItemLoader::onHandlePropTypeSpriteFrame(pNode, pParent, pPropertyName, pCCSpriteFrame, pCCBReader);
    }
}

NS_CC_EXT_END
