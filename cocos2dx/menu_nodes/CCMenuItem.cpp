/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2011 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCMenuItem.h"
#include "cocoa/CCPointExtension.h"
#include "actions/CCActionInterval.h"
#include "sprite_nodes/CCSprite.h"
#include "label_nodes/CCLabelAtlas.h"
#include "label_nodes/CCLabelTTF.h"
#include "script_support/CCScriptSupport.h"
#include <stdarg.h>
#include <cstring>
#include "support/utils/CCUtils.h"

NS_CC_BEGIN
    
static unsigned int _globalFontSize = kCCItemSize;
static std::string _globalFontName = "Marker Felt";
static bool _globalFontNameRelease = false;

const unsigned int    kCurrentItem = 0xc0c05001;
const unsigned int    kZoomActionTag = 0xc0c05002;

const unsigned int    kNormalTag = 0x1;
const unsigned int    kSelectedTag = 0x2;
const unsigned int    kDisableTag = 0x3;
//
// CCMenuItem
//

CCMenuItem* CCMenuItem::create()
{
    return CCMenuItem::create(NULL, NULL);
}

CCMenuItem* CCMenuItem::create(CCObject *rec, SEL_MenuHandler selector)
{
    CCMenuItem *pRet = new CCMenuItem();
    pRet->initWithTarget(rec, selector);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItem::initWithTarget(CCObject *rec, SEL_MenuHandler selector)
{
    setAnchorPoint(ccp(0.5f, 0.5f));
    m_pListener = rec;
    m_pfnSelector = selector;
    m_bEnabled = true;
    m_bSelected = false;
    return true;
}

CCMenuItem::~CCMenuItem()
{
    unregisterScriptTapHandler();
}

void CCMenuItem::selected()
{
    m_bSelected = true;
}

void CCMenuItem::unselected()
{
    m_bSelected = false;
}

void CCMenuItem::registerScriptTapHandler(ccScriptFunction nHandler)
{
    unregisterScriptTapHandler();
    m_nScriptTapHandler = nHandler;
    LUALOG("[LUA] Add CCMenuItem script handler: %d", m_nScriptTapHandler);
}

void CCMenuItem::unregisterScriptTapHandler(void)
{
    if (m_nScriptTapHandler.handler)
    {
        CCScriptEngineManager::sharedManager()->getScriptEngine()->removeScriptHandler(m_nScriptTapHandler.handler);
        LUALOG("[LUA] Remove CCMenuItem script handler: %d", m_nScriptTapHandler);
        m_nScriptTapHandler.handler = 0;
    }
}

void CCMenuItem::activate()
{
    if (m_bEnabled)
    {
        if (m_pListener && m_pfnSelector)
        {
            (m_pListener->*m_pfnSelector)(this);
        }
        
        if (m_nScriptTapHandler.handler)
        {
            CCArray* args = CCArray::createWithCapacity(1);
            args->addObject(this);
            CCScriptEngineManager::sharedManager()->getScriptEngine()->executeEventWithArgs(m_nScriptTapHandler, args);
        }
    }
}

void CCMenuItem::setEnabled(bool enabled)
{
    m_bEnabled = enabled;
}

bool CCMenuItem::isEnabled()
{
    return m_bEnabled;
}

CCRect CCMenuItem::rect()
{
    return CCRectMake( m_obPosition.x - m_obContentSize.width * m_obAnchorPoint.x,
                      m_obPosition.y - m_obContentSize.height * m_obAnchorPoint.y,
                      m_obContentSize.width, m_obContentSize.height);
}

bool CCMenuItem::isSelected()
{
    return m_bSelected;
}

void CCMenuItem::setTarget(CCObject *rec, SEL_MenuHandler selector)
{
    m_pListener = rec;
    m_pfnSelector = selector;
}

//
//CCMenuItemLabel
//

const ccColor3B& CCMenuItemLabel::getDisabledColor()
{
    return m_tDisabledColor;
}
void CCMenuItemLabel::setDisabledColor(const ccColor3B& var)
{
    m_tDisabledColor = var;
}
CCNode *CCMenuItemLabel::getLabel()
{
    return m_pLabel;
}
void CCMenuItemLabel::setLabel(CCNode* var)
{
    if (var)
    {
        addChild(var);
        var->setAnchorPoint(ccp(0, 0));
        setContentSize(var->getContentSize());
    }
    
    if (m_pLabel)
    {
        removeChild(m_pLabel, true);
    }
    
    m_pLabel = var;
}

CCMenuItemLabel * CCMenuItemLabel::create(CCNode*label, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItemLabel *pRet = new CCMenuItemLabel();
    pRet->initWithLabel(label, target, selector);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

CCMenuItemLabel* CCMenuItemLabel::create(CCNode *label)
{
    CCMenuItemLabel *pRet = new CCMenuItemLabel();
    pRet->initWithLabel(label, NULL, NULL);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemLabel::initWithLabel(CCNode* label, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItem::initWithTarget(target, selector);
    m_fOriginalScale = 1.0f;
    m_tColorBackup = ccWHITE;
    setDisabledColor(ccc3(126,126,126));
    this->setLabel(label);
    
    setCascadeColorEnabled(true);
    setCascadeOpacityEnabled(true);
    
    return true;
}

CCMenuItemLabel::~CCMenuItemLabel()
{
}

void CCMenuItemLabel::setString(const char * label)
{
    dynamic_cast<CCLabelProtocol*>(m_pLabel)->setString(label);
    this->setContentSize(m_pLabel->getContentSize());
}

void CCMenuItemLabel::activate()
{
    if(m_bEnabled)
    {
        this->stopAllActions();
        this->setScale( m_fOriginalScale );
        CCMenuItem::activate();
    }
}

void CCMenuItemLabel::selected()
{
    // subclass to change the default action
    if(m_bEnabled)
    {
        CCMenuItem::selected();
        
        CCAction *action = getActionByTag(kZoomActionTag);
        if (action)
        {
            this->stopAction(action);
        }
        else
        {
            m_fOriginalScale = this->getScale();
        }
        
        CCAction *zoomAction = CCScaleTo::create(0.1f, m_fOriginalScale * 1.2f);
        zoomAction->setTag(kZoomActionTag);
        this->runAction(zoomAction);
    }
}

void CCMenuItemLabel::unselected()
{
    // subclass to change the default action
    if(m_bEnabled)
    {
        CCMenuItem::unselected();
        this->stopActionByTag(kZoomActionTag);
        CCAction *zoomAction = CCScaleTo::create(0.1f, m_fOriginalScale);
        zoomAction->setTag(kZoomActionTag);
        this->runAction(zoomAction);
    }
}

void CCMenuItemLabel::setEnabled(bool enabled)
{
    if( m_bEnabled != enabled ) 
    {
        if(enabled == false)
        {
            m_tColorBackup = dynamic_cast<CCRGBAProtocol*>(m_pLabel)->getColor();
            dynamic_cast<CCRGBAProtocol*>(m_pLabel)->setColor(m_tDisabledColor);
        }
        else
        {
            dynamic_cast<CCRGBAProtocol*>(m_pLabel)->setColor(m_tColorBackup);
        }
    }
    CCMenuItem::setEnabled(enabled);
}

//
//CCMenuItemAtlasFont
//

CCMenuItemAtlasFont * CCMenuItemAtlasFont::create(const char *value, const char *charMapFile, int itemWidth, int itemHeight, char startCharMap)
{
    return CCMenuItemAtlasFont::create(value, charMapFile, itemWidth, itemHeight, startCharMap, NULL, NULL);
}

CCMenuItemAtlasFont * CCMenuItemAtlasFont::create(const char *value, const char *charMapFile, int itemWidth, int itemHeight, char startCharMap, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItemAtlasFont *pRet = new CCMenuItemAtlasFont();
    pRet->initWithString(value, charMapFile, itemWidth, itemHeight, startCharMap, target, selector);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemAtlasFont::initWithString(const char *value, const char *charMapFile, int itemWidth, int itemHeight, char startCharMap, CCObject* target, SEL_MenuHandler selector)
{
    CCAssert( value != NULL && strlen(value) != 0, "value length must be greater than 0");
    CCLabelAtlas *label = new CCLabelAtlas();
    label->initWithString(value, charMapFile, itemWidth, itemHeight, startCharMap);
    CC_SAFE_AUTORELEASE(label);
    if (CCMenuItemLabel::initWithLabel(label, target, selector))
    {
        // do something ?
    }
    return true;
}
//
//CCMenuItemFont
//

void CCMenuItemFont::setFontSize(unsigned int s)
{
    _globalFontSize = s;
}

unsigned int CCMenuItemFont::fontSize()
{
    return _globalFontSize;
}

void CCMenuItemFont::setFontName(const char *name)
{
    if (_globalFontNameRelease)
    {
        _globalFontName.clear();
    }
    _globalFontName = name;
    _globalFontNameRelease = true;
}

const char * CCMenuItemFont::fontName()
{
    return _globalFontName.c_str();
}

CCMenuItemFont * CCMenuItemFont::create(const char *value, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItemFont *pRet = new CCMenuItemFont();
    pRet->initWithString(value, target, selector);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

CCMenuItemFont * CCMenuItemFont::create(const char *value)
{
    CCMenuItemFont *pRet = new CCMenuItemFont();
    pRet->initWithString(value, NULL, NULL);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemFont::initWithString(const char *value, CCObject* target, SEL_MenuHandler selector)
{
    CCAssert( value != NULL && strlen(value) != 0, "Value length must be greater than 0");
    
    m_strFontName = _globalFontName;
    m_uFontSize = _globalFontSize;
    
    CCLabelTTF *label = CCLabelTTF::create(value, m_strFontName.c_str(), (float)m_uFontSize);
    if (CCMenuItemLabel::initWithLabel(label, target, selector))
    {
        // do something ?
    }
    return true;
}

void CCMenuItemFont::recreateLabel()
{
    CCLabelTTF *label = CCLabelTTF::create(dynamic_cast<CCLabelProtocol*>(m_pLabel)->getString(), 
                                                    m_strFontName.c_str(), (float)m_uFontSize);
    this->setLabel(label);
}

void CCMenuItemFont::setFontSizeObj(unsigned int s)
{
    m_uFontSize = s;
    recreateLabel();
}

unsigned int CCMenuItemFont::fontSizeObj()
{
    return m_uFontSize;
}

void CCMenuItemFont::setFontNameObj(const char* name)
{
    m_strFontName = name;
    recreateLabel();
}

const char* CCMenuItemFont::fontNameObj()
{
    return m_strFontName.c_str();
}

//
//CCMenuItemSprite
//

CCNode * CCMenuItemSprite::getNormalImage()
{
    return m_pNormalImage;
}

void CCMenuItemSprite::setNormalImage(CCNode* pImage)
{
    if (pImage != m_pNormalImage)
    {
        if (pImage)
        {
            addChild(pImage, 0, kNormalTag);
            pImage->setAnchorPoint(ccp(0, 0));
        }

        if (m_pNormalImage)
        {
            removeChild(m_pNormalImage, true);
        }

        m_pNormalImage = pImage;
        this->setContentSize(m_pNormalImage->getContentSize());
        this->updateImagesVisibility();
    }
}

CCNode * CCMenuItemSprite::getSelectedImage()
{
    return m_pSelectedImage;
}

void CCMenuItemSprite::setSelectedImage(CCNode* pImage)
{
    if (pImage != m_pNormalImage)
    {
        if (pImage)
        {
            addChild(pImage, 0, kSelectedTag);
            pImage->setAnchorPoint(ccp(0, 0));
        }

        if (m_pSelectedImage)
        {
            removeChild(m_pSelectedImage, true);
        }

        m_pSelectedImage = pImage;
        this->updateImagesVisibility();
    }
}

CCNode * CCMenuItemSprite::getDisabledImage()
{
    return m_pDisabledImage;
}

void CCMenuItemSprite::setDisabledImage(CCNode* pImage)
{
    if (pImage != m_pNormalImage)
    {
        if (pImage)
        {
            addChild(pImage, 0, kDisableTag);
            pImage->setAnchorPoint(ccp(0, 0));
        }

        if (m_pDisabledImage)
        {
            removeChild(m_pDisabledImage, true);
        }

        m_pDisabledImage = pImage;
        this->updateImagesVisibility();
    }
}

//
//CCMenuItemSprite
//

CCMenuItemSprite* CCMenuItemSprite::create(CCNode* sprite, CCObject* target, SEL_MenuHandler selector) {
    CCMenuItemSprite* pRet = new CCMenuItemSprite();
    pRet->initWithNormalSprite(sprite, NULL, NULL, target, selector);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

CCMenuItemSprite * CCMenuItemSprite::create(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCNode* focusSprite)
{
    return CCMenuItemSprite::create(normalSprite, selectedSprite, disabledSprite, focusSprite, NULL, NULL);
}

CCMenuItemSprite * CCMenuItemSprite::create(CCNode* normalSprite, CCNode* selectedSprite, CCObject* target, SEL_MenuHandler selector)
{
    return CCMenuItemSprite::create(normalSprite, selectedSprite, NULL, target, selector);
}

CCMenuItemSprite * CCMenuItemSprite::create(CCNode *normalSprite, CCNode *selectedSprite, CCNode *disabledSprite, CCObject *target, SEL_MenuHandler selector)
{
    CCMenuItemSprite *pRet = new CCMenuItemSprite();
    pRet->initWithNormalSprite(normalSprite, selectedSprite, disabledSprite, target, selector); 
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

CCMenuItemSprite* CCMenuItemSprite::create(CCNode* normalImage, CCNode* selectedImage, CCNode* disabledImage,
                                           CCNode* focusImage, CCObject* target, SEL_MenuHandler selector) {
    CCMenuItemSprite* pRet = new CCMenuItemSprite();
    pRet->initWithNormalSprite(normalImage, selectedImage, disabledImage, target, selector);
    pRet->setFocusImage(focusImage);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemSprite::initWithNormalSprite(CCNode* normalSprite, CCNode* selectedSprite, CCNode* disabledSprite, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItem::initWithTarget(target, selector); 
    setNormalImage(normalSprite);
    setSelectedImage(selectedSprite);
    setDisabledImage(disabledSprite);

    if(m_pNormalImage)
    {
        this->setContentSize(m_pNormalImage->getContentSize());
    }
    
    setCascadeColorEnabled(true);
    setCascadeOpacityEnabled(true);
    
    return true;
}

void CCMenuItemSprite::setSelectedEvent(CCObject* target, SEL_MenuHandler selector) {
    m_selectedEventTarget = target;
    m_selectedEventSelector = selector;
}

void CCMenuItemSprite::setUnselectedEvent(CCObject* target, SEL_MenuHandler selector) {
    m_unselectedEventTarget = target;
    m_unselectedEventSelector = selector;
}

void CCMenuItemSprite::centerAlignImages() {
    CCSize s = CCSizeZero;
    if(m_pNormalImage) {
        CCSize _s = m_pNormalImage->getContentSize();
        s.width = MAX(s.width, _s.width);
        s.height = MAX(s.height, _s.height);
    }
    if(m_pSelectedImage) {
        CCSize _s = m_pSelectedImage->getContentSize();
        s.width = MAX(s.width, _s.width);
        s.height = MAX(s.height, _s.height);
    }
    if(m_focusImage) {
        CCSize _s = m_focusImage->getContentSize();
        s.width = MAX(s.width, _s.width);
        s.height = MAX(s.height, _s.height);
    }
    if(m_pDisabledImage) {
        CCSize _s = m_pDisabledImage->getContentSize();
        s.width = MAX(s.width, _s.width);
        s.height = MAX(s.height, _s.height);
    }
    setContentSize(s);
    
    if(m_pNormalImage) {
        CCSize _s = m_pNormalImage->getContentSize();
        m_pNormalImage->setPosition(ccp((s.width - _s.width) / 2,
                                        (s.height - _s.height) / 2));
    }
    if(m_pSelectedImage) {
        CCSize _s = m_pSelectedImage->getContentSize();
        m_pSelectedImage->setPosition(ccp((s.width - _s.width) / 2,
                                          (s.height - _s.height) / 2));
    }
    if(m_focusImage) {
        CCSize _s = m_focusImage->getContentSize();
        m_focusImage->setPosition(ccp((s.width - _s.width) / 2,
                                      (s.height - _s.height) / 2));
    }
    if(m_pDisabledImage) {
        CCSize _s = m_pDisabledImage->getContentSize();
        m_pDisabledImage->setPosition(ccp((s.width - _s.width) / 2,
                                          (s.height - _s.height) / 2));
    }
}

/**
 @since v0.99.5
 */
void CCMenuItemSprite::selected()
{
    CCMenuItem::selected();
    updateImagesVisibility();
    if (m_pNormalImage)
    {
        if (m_pDisabledImage)
        {
            m_pDisabledImage->setVisible(false);
        }

        if (m_pSelectedImage)
        {
            m_pNormalImage->setVisible(false);
            m_pSelectedImage->setVisible(true);
        }
        else
        {
            m_pNormalImage->setVisible(true);
        }
    }
    
    // set a darker color
    if(!getSelectedImage()) {
        m_oldColor = getColor();
        ccColorHSV hsv = CCUtils::ccc32hsv(m_oldColor);
        hsv.v = MAX(0, hsv.v - 0.2f);
        ccColor3B c = CCUtils::hsv2ccc3(hsv);
        setColor(c);
    }
    
    // event
    if (m_selectedEventTarget && m_selectedEventSelector) {
        (m_selectedEventTarget->*m_selectedEventSelector)(this);
    }
}

void CCMenuItemSprite::unselected()
{
    CCMenuItem::unselected();
    updateImagesVisibility();
    
    // restore old color
    if(!getSelectedImage()) {
        setColor(m_oldColor);
    }
    
    // event
    if (m_unselectedEventTarget && m_unselectedEventSelector) {
        (m_unselectedEventTarget->*m_unselectedEventSelector)(this);
    }
}

void CCMenuItemSprite::setEnabled(bool bEnabled)
{
    if( m_bEnabled != bEnabled ) 
    {
        CCMenuItem::setEnabled(bEnabled);
        this->updateImagesVisibility();
    }
}

// Helper 
void CCMenuItemSprite::updateImagesVisibility()
{
    if (m_pNormalImage)
        m_pNormalImage->setVisible(false);
    if (m_pSelectedImage)
        m_pSelectedImage->setVisible(false);
    if (m_pDisabledImage)
        m_pDisabledImage->setVisible(false);
    if (m_focusImage)
        m_focusImage->setVisible(false);
    
    if(!m_bEnabled) {
        if (m_pDisabledImage) {
            m_pDisabledImage->setVisible(true);
        } else {
            if (m_pNormalImage) {
                m_pNormalImage->setVisible(true);
            }
        }
    } else if(m_focus && m_focusImage) {
        if(m_focusIsAttachment) {
            m_focusImage->setVisible(m_focus || isSelected());
        } else {
            m_focusImage->setVisible(m_focus);
        }
    } else {
        if (m_pNormalImage) {
            m_pNormalImage->setVisible(true);
        }
    }
}

void CCMenuItemSprite::setFocus(bool flag) {
    m_focus = flag;
    updateImagesVisibility();
}

void CCMenuItemSprite::setFocusImage(CCNode* focusImage) {
    if (focusImage != m_focusImage) {
        if (focusImage) {
            addChild(focusImage, 0);
            focusImage->setAnchorPoint(ccp(0, 0));
        }
        
        if (m_focusImage) {
            removeChild(m_focusImage, true);
        }
        
        m_focusImage = focusImage;
        updateImagesVisibility();
    }
}

///
/// CCMenuItemImage
///

CCMenuItemImage* CCMenuItemImage::create()
{
    CCMenuItemImage *pRet = new CCMenuItemImage();
    if (pRet && pRet->init())
    {
        CC_SAFE_AUTORELEASE(pRet);
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

bool CCMenuItemImage::init(void)
{
    return initWithNormalImage(NULL, NULL, NULL, NULL, NULL);
}

CCMenuItemImage * CCMenuItemImage::create(const char *normalImage, const char *selectedImage)
{
    return CCMenuItemImage::create(normalImage, selectedImage, NULL, NULL, NULL);
}

CCMenuItemImage * CCMenuItemImage::create(const char *normalImage, const char *selectedImage, CCObject* target, SEL_MenuHandler selector)
{
    return CCMenuItemImage::create(normalImage, selectedImage, NULL, target, selector);
}

CCMenuItemImage * CCMenuItemImage::create(const char *normalImage, const char *selectedImage, const char *disabledImage, CCObject* target, SEL_MenuHandler selector)
{
    CCMenuItemImage *pRet = new CCMenuItemImage();
    if (pRet && pRet->initWithNormalImage(normalImage, selectedImage, disabledImage, target, selector))
    {
        CC_SAFE_AUTORELEASE(pRet);
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

CCMenuItemImage * CCMenuItemImage::create(const char *normalImage, const char *selectedImage, const char *disabledImage)
{
    CCMenuItemImage *pRet = new CCMenuItemImage();
    if (pRet && pRet->initWithNormalImage(normalImage, selectedImage, disabledImage, NULL, NULL))
    {
        CC_SAFE_AUTORELEASE(pRet);
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

bool CCMenuItemImage::initWithNormalImage(const char *normalImage, const char *selectedImage, const char *disabledImage, CCObject* target, SEL_MenuHandler selector)
{
    CCNode *normalSprite = NULL;
    CCNode *selectedSprite = NULL;
    CCNode *disabledSprite = NULL;

    if (normalImage)
    {
        normalSprite = CCSprite::create(normalImage);
    }

    if (selectedImage)
    {
        selectedSprite = CCSprite::create(selectedImage);
    }
    
    if(disabledImage)
    {
        disabledSprite = CCSprite::create(disabledImage);
    }
    return initWithNormalSprite(normalSprite, selectedSprite, disabledSprite, target, selector);
}
//
// Setter of sprite frames
//
void CCMenuItemImage::setNormalSpriteFrame(CCSpriteFrame * frame)
{
    setNormalImage(CCSprite::createWithSpriteFrame(frame));
}

void CCMenuItemImage::setSelectedSpriteFrame(CCSpriteFrame * frame)
{
    setSelectedImage(CCSprite::createWithSpriteFrame(frame));
}

void CCMenuItemImage::setDisabledSpriteFrame(CCSpriteFrame * frame)
{
    setDisabledImage(CCSprite::createWithSpriteFrame(frame));
}

//
// MenuItemToggle
//

void CCMenuItemToggle::setSubItems(CCArray* var)
{
    CC_SAFE_RETAIN(var);
    CC_SAFE_RELEASE(m_pSubItems);
    m_pSubItems = var;
}

CCArray* CCMenuItemToggle::getSubItems()
{
    return m_pSubItems;
}

CCMenuItemToggle * CCMenuItemToggle::createWithTarget(CCObject* target, SEL_MenuHandler selector, CCArray* menuItems)
{
    CCMenuItemToggle *pRet = new CCMenuItemToggle();
    pRet->CCMenuItem::initWithTarget(target, selector);
    pRet->m_pSubItems = CCArray::create();
    CC_SAFE_RETAIN(pRet->m_pSubItems);
    
    for (unsigned int z=0; z < menuItems->count(); z++)
    {
        CCMenuItem* menuItem = (CCMenuItem*)menuItems->objectAtIndex(z);
        pRet->m_pSubItems->addObject(menuItem);
    }
    
    pRet->m_uSelectedIndex = UINT_MAX;
    pRet->setSelectedIndex(0);
    return pRet;
}

CCMenuItemToggle * CCMenuItemToggle::createWithTarget(CCObject* target, SEL_MenuHandler selector, CCMenuItem* item, ...)
{
    va_list args;
    va_start(args, item);
    CCMenuItemToggle *pRet = new CCMenuItemToggle();
    pRet->initWithTarget(target, selector, item, args);
    CC_SAFE_AUTORELEASE(pRet);
    va_end(args);
    return pRet;
}

CCMenuItemToggle * CCMenuItemToggle::create()
{
    CCMenuItemToggle *pRet = new CCMenuItemToggle();
    pRet->initWithItem(NULL);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemToggle::initWithTarget(CCObject* target, SEL_MenuHandler selector, CCMenuItem* item, va_list args)
{
    CCMenuItem::initWithTarget(target, selector);
    this->m_pSubItems = CCArray::create();
    CC_SAFE_RETAIN(this->m_pSubItems);
    int z = 0;
    CCMenuItem *i = item;
    while(i) 
    {
        z++;
        m_pSubItems->addObject(i);
        i = va_arg(args, CCMenuItem*);
    }
    m_uSelectedIndex = UINT_MAX;
    this->setSelectedIndex(0);
    return true;
}

CCMenuItemToggle* CCMenuItemToggle::create(CCMenuItem *item)
{
    CCMenuItemToggle *pRet = new CCMenuItemToggle();
    pRet->initWithItem(item);
    CC_SAFE_AUTORELEASE(pRet);
    return pRet;
}

bool CCMenuItemToggle::initWithItem(CCMenuItem *item)
{
    CCMenuItem::initWithTarget(NULL, NULL);
    setSubItems(CCArray::create());

    if (item)
    {
        m_pSubItems->addObject(item);
    }
    m_uSelectedIndex = UINT_MAX;
    this->setSelectedIndex(0);
    
    setCascadeColorEnabled(true);
    setCascadeOpacityEnabled(true);
    
    return true;
}

void CCMenuItemToggle::addSubItem(CCMenuItem *item)
{
    m_pSubItems->addObject(item);
}

CCMenuItemToggle::~CCMenuItemToggle()
{
    CC_SAFE_RELEASE(m_pSubItems);
}
void CCMenuItemToggle::setSelectedIndex(unsigned int index)
{
    if( index != m_uSelectedIndex && m_pSubItems->count() > 0 )
    {
        m_uSelectedIndex = index;
        CCMenuItem *currentItem = (CCMenuItem*)getChildByTag(kCurrentItem);
        if( currentItem )
        {
            currentItem->removeFromParentAndCleanup(false);
        }

        CCMenuItem* item = (CCMenuItem*)m_pSubItems->objectAtIndex(m_uSelectedIndex);
        this->addChild(item, 0, kCurrentItem);
        CCSize s = item->getContentSize();
        this->setContentSize(s);
        item->setPosition( ccp( s.width/2, s.height/2 ) );
    }
}
unsigned int CCMenuItemToggle::getSelectedIndex()
{
    return m_uSelectedIndex;
}
void CCMenuItemToggle::selected()
{
    CCMenuItem::selected();
    ((CCMenuItem*)(m_pSubItems->objectAtIndex(m_uSelectedIndex)))->selected();
}
void CCMenuItemToggle::unselected()
{
    CCMenuItem::unselected();
    ((CCMenuItem*)(m_pSubItems->objectAtIndex(m_uSelectedIndex)))->unselected();
}
void CCMenuItemToggle::activate()
{
    // update index
    if( m_bEnabled ) 
    {
        unsigned int newIndex = (m_uSelectedIndex + 1) % m_pSubItems->count();
        this->setSelectedIndex(newIndex);
    }
    CCMenuItem::activate();
}
void CCMenuItemToggle::setEnabled(bool enabled)
{
    if (m_bEnabled != enabled)
    {
        CCMenuItem::setEnabled(enabled);

        if(m_pSubItems && m_pSubItems->count() > 0)
        {
            CCObject* pObj = NULL;
            CCARRAY_FOREACH(m_pSubItems, pObj)
            {
                CCMenuItem* pItem = (CCMenuItem*)pObj;
                pItem->setEnabled(enabled);
            }
        }
    }
}

CCMenuItem* CCMenuItemToggle::selectedItem()
{
    return (CCMenuItem*)m_pSubItems->objectAtIndex(m_uSelectedIndex);
}

NS_CC_END
