//
//  CASegmentedControl.cpp
//  CrossApp
//
//  Created by qiaoxin on 15-1-20.
//  Copyright (c) 2015年 CrossApp. All rights reserved.
//

#include "CASegmentedControl.h"
#include "view/CAScale9ImageView.h"
#include "dispatcher/CATouch.h"
#include "basics/CAApplication.h"
#include "view/CAClippingView.h"
#include "draw_nodes/CCDrawNode.h"
#include "view/CARenderImage.h"

using namespace std;
NS_CC_BEGIN

CASegmentedControl::CASegmentedControl(unsigned int itemsCount)
    : CAControl()
    , m_nItemsCount(itemsCount)
    , m_pBackgroundView(NULL)
    , m_fSeparateWidth(1)
    , m_iSelectedIndex(-1)
    , m_iTouchIndex(0)
    , m_cTextColor(ccc4(50, 190, 250, 255))
    , m_cTextSelectedColor(CAColor_white)
    , m_cTintColor(ccc4(50, 190, 250, 255))
    , m_pTarget(NULL)
    , m_pCallFunc(NULL)
    , m_pSegmentItemBackgroundImage(NULL)
{
    m_vTitles.clear();
    m_vNormalImages.clear();
    m_vSelectedImages.clear();
    m_vIsEnabled.clear();
    m_vContentOffset.clear();
    m_vSegments.clear();
    m_vSeparateView.clear();
    m_vSegmentWidth.clear();
    m_vSegmentItemBackground.clear();
}

CASegmentedControl::~CASegmentedControl()
{
    m_vTitles.clear();
    m_vNormalImages.clear();
    m_vSelectedImages.clear();
    m_vIsEnabled.clear();
    m_vContentOffset.clear();
    m_vSegments.clear();
    m_vSeparateView.clear();
    m_vSegmentWidth.clear();
    m_vSegmentItemBackground.clear();
}

void CASegmentedControl::removeAllSeparateImage()
{
    for (std::vector<CAView*>::iterator itr = m_vSeparateView.begin(); itr != m_vSeparateView.end(); ++itr)
    {
        (*itr)->removeFromSuperview();
    }
    m_vSeparateView.clear();
}

void CASegmentedControl::onExitTransitionDidStart()
{
    CAControl::onExitTransitionDidStart();
}

void CASegmentedControl::onEnterTransitionDidFinish()
{
    CAControl::onEnterTransitionDidFinish();
}

CASegmentedControl* CASegmentedControl::createWithFrame(const CCRect& rect, unsigned int itemsCount)
{
    CASegmentedControl* segmentedControl = new CASegmentedControl(itemsCount);
    if (segmentedControl && segmentedControl->initWithFrame(rect))
    {
        segmentedControl->autorelease();
        return segmentedControl;
    }
    CC_SAFE_DELETE(segmentedControl);
    return NULL;
}

CASegmentedControl* CASegmentedControl::createWithCenter(const CCRect& rect, unsigned int itemsCount)
{
    CASegmentedControl* segmentedControl = new CASegmentedControl(itemsCount);
    if (segmentedControl && segmentedControl->initWithCenter(rect))
    {
        segmentedControl->autorelease();
        return segmentedControl;
    }
    CC_SAFE_DELETE(segmentedControl);
    return NULL;
}

bool CASegmentedControl::initWithFrame(const CCRect& rect)
{
    if (!CAControl::initWithFrame(rect))
    {
        return false;
    }
    this->setBackgroundImage( CAImage::create("source_material/btn_rounded_normal.png") );
    this->removeAllSegments();
    const float elemWidth = this->getBounds().size.width / m_nItemsCount;
    CCRect elemFrame = CCRectMake(0, 0, this->getBounds().size.width/m_nItemsCount, this->getBounds().size.height);
    for (int i = 0; i < m_nItemsCount; ++i)
    {
        CAView* segment = this->createDefaultSegment(i);
        if (segment)
        {
            segment->setFrame(elemFrame);
            m_vSegments.pushBack(segment);
            this->insertSubview(segment, 1);
        }
        elemFrame.origin.x += elemWidth;
    }
    setSegmentItemBackgroundImage(CAImage::create("source_material/btn_rounded_highlighted.png"));
    createSeparate();
    return true;
}

bool CASegmentedControl::initWithCenter(const CCRect& rect)
{
    if (!CAControl::initWithCenter(rect))
    {
        return false;
    }
    this->setBackgroundImage( CAImage::create("source_material/btn_rounded_normal.png") );
    this->removeAllSegments();
    const float elemWidth = this->getBounds().size.width / m_nItemsCount;
    CCRect elemFrame = CCRectMake(0, 0, this->getBounds().size.width/m_nItemsCount, this->getBounds().size.height);
    for (int i = 0; i < m_nItemsCount; ++i)
    {
        CAView *segment = this->createDefaultSegment(i);
        if (segment)
        {
            segment->setFrame(elemFrame);
            m_vSegments.pushBack(segment);
            this->insertSubview(segment, 1);
        }
        elemFrame.origin.x += elemWidth;
    }
    setSegmentItemBackgroundImage(CAImage::create("source_material/btn_rounded_highlighted.png"));
    createSeparate();
    return true;
}

#pragma mark --
void CASegmentedControl::setTitleColor(const CAColor4B& color)
{
    m_cTextColor = color;
    for(int index=0; index<m_vSegments.size(); ++index)
    {
        CALabel* label = m_vTitles.at(index);
        if(label != NULL)
            label->setColor(m_cTextColor);
    }
}

const CAColor4B& CASegmentedControl::getTitleColor()
{
    return m_cTextColor;
}

void CASegmentedControl::setTitleSelectedColor(const CAColor4B& color)
{
    m_cTextSelectedColor = color;
}

void CASegmentedControl::setTintColor(const CAColor4B& color)
{
    m_cTintColor = color;
    cleanAllSeparate();
    createSeparate();
}

void CASegmentedControl::setSelectedAtIndex(int index)
{
    m_iSelectedIndex = index;
    for(int i=0; i<m_vSegments.size(); ++i)
    {
        refreshSegmentItemByIndex(i, CAControlStateNormal);
        setSegmentItemBackgroundVisibleWithIndex(false, i);
    }
    refreshSegmentItemByIndex(index, CAControlStateSelected);
    setSegmentItemBackgroundVisibleWithIndex(true, index);
}

int CASegmentedControl::getSelectedAtIndex()
{
    return m_iSelectedIndex;
}

int CASegmentedControl::getNumberOfSegments()
{
    return m_vSegments.size();
}

void CASegmentedControl::insertSegmentWithTitleAtIndex(const std::string& title, int index)
{
    if(index >= 0 && index <= m_vSegments.size())
    {
        if(index <= m_iSelectedIndex)
        {
            m_iSelectedIndex++;
        }
        CAView *segment = this->createDefaultSegment(index);
        if (segment)
        {
            segment->setFrame(CCRectMake(0,0,this->getBounds().size.width/m_vSegments.size(), this->getBounds().size.height));
            m_vSegments.insert(index, segment);
            this->insertSubview(segment, 1);
        }
        refreshAllSegmentItemBound();
        setTitleForSegmentAtIndex(title, index);
        refreshAllSegmentItemPosition();
        setSegmentItemBackgroundImage(m_pSegmentItemBackgroundImage);
        
        cleanAllSeparate();
        createSeparate();
    }
//    refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

void CASegmentedControl::insertSegmentWithImageAtIndex(CAImage* image, int index, CAControlState controlState)
{
    if(index >= 0 && index <= m_vSegments.size())
    {
        if(index <= m_iSelectedIndex)
        {
            m_iSelectedIndex++;
        }
        CAView *segment = this->createDefaultSegment(index);
        if (segment)
        {
            segment->setFrame(CCRectMake(0,0,this->getBounds().size.width/m_vSegments.size(), this->getBounds().size.height));
            m_vSegments.insert(index, segment);
            this->insertSubview(segment, 1);
        }
        refreshAllSegmentItemBound();
        setImageForSegmentAtIndex(image, index, controlState);
        refreshAllSegmentItemPosition();
        setSegmentItemBackgroundImage(m_pSegmentItemBackgroundImage);
        
        cleanAllSeparate();
        createSeparate();
    }
    refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

void CASegmentedControl::setTitleForSegmentAtIndex(const std::string& title, int index)
{
    if(index < m_vSegments.size())
    {
        CCPoint point = m_vSegments.at(index)->getBounds().origin;
        CCSize size = m_vSegments.at(index)->getBounds().size;
        CALabel* label = CALabel::createWithCenter(CCRect(m_vContentOffset.at(index).width+ size.width, m_vContentOffset.at(index).height + size.height, size.width, size.height));
        label->setTextAlignment(CATextAlignmentCenter);
        label->setVerticalTextAlignmet(CAVerticalTextAlignmentCenter);
        label->setText(title);
        label->setColor(m_cTextColor);
        label->retain();
        CAVector<CALabel*>::iterator itr = m_vTitles.begin()+index;
        if(*itr != NULL)
        {
            (*itr)->removeFromSuperview();
        }
        (*itr) = label;
        m_vSegments.at(index)->addSubview(*itr);
        refreshSegmentItemByIndex(index, CAControlStateNormal);
    }
    if(m_iSelectedIndex>=0 && m_iSelectedIndex<m_vSegments.size())
        refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

const std::string& CASegmentedControl::getTitleForSegmentAtIndex(int index)
{
    std::string str = "";
    if(index >= 0 && index < m_vSegments.size())
    {
        CAVector<CALabel*>::iterator itr = m_vTitles.begin()+index;
        if((*itr) != NULL)
            str = (*itr)->getText();
    }
    return str;
}

void CASegmentedControl::setImageForSegmentAtIndex(CAImage* image, int index, CAControlState controlState)
{
    if(index >= 0 && index < m_vSegments.size())
    {
        CAVector<CALabel*>::iterator itr = m_vTitles.begin()+index;
        if(*itr != NULL)
        {
            (*itr)->removeFromSuperview();
            CC_SAFE_RELEASE_NULL(*itr);
        }
        if( CAControlStateNormal == controlState )
        {
            CAVector<CAImage*>::iterator itr_NImage = m_vNormalImages.begin();
            *(itr_NImage + index) = image;
            
            CAVector<CAImage*>::iterator itr_SImage = m_vSelectedImages.begin();
            if( *(itr_SImage + index) == NULL)
                *(itr_SImage + index) = image;
        }
        else if( CAControlStateSelected == controlState )
        {
            CAVector<CAImage*>::iterator itr_SImage = m_vSelectedImages.begin();
            *(itr_SImage + index) = image;
        }

        refreshSegmentItemByIndex(index, controlState);
        refreshSegmentItemByIndex(index, CAControlStateNormal);
    }
    refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

CAImage* CASegmentedControl::getImageForSegmentAtIndex(int index)
{
    CAImage* image = NULL;
    if(index >= 0 && index < m_vSegments.size())
    {
        CAVector<CAImage*>::iterator itr = m_vNormalImages.begin()+index;
        image = (*itr);
    }
    return image;
}

void CASegmentedControl::setWidthForSegmentAtIndex(float width, int index)
{
    if(index >= 0 && index < m_vSegments.size())
    {
        std::vector<float>::iterator itr_width = m_vSegmentWidth.begin()+index;
        *itr_width = width;

        refreshAllSegmentItemBound();
        for (int i=0; i<m_vSegments.size(); ++i) {
            refreshSegmentItemByIndex(i, CAControlStateNormal);
        }
        refreshAllSegmentItemPosition();
        setSegmentItemBackgroundImage(m_pSegmentItemBackgroundImage);
        
        cleanAllSeparate();
        createSeparate();
    }
    refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

float CASegmentedControl::getWidthForSegmentAtIndex(int index)
{
    float width = 0;
    if(index >= 0 && index < m_vSegments.size())
    {
        CAVector<CAView*>::iterator itr = m_vSegments.begin()+index;
        width = (*itr)->getBounds().size.width;
    }
    return width;
}

void CASegmentedControl::setContentOffsetForSegmentAtIndex(CCSize offset, int index)
{
    if(index >= 0 && index < m_vSegments.size())
    {
        std::vector<CCSize>::iterator itr = m_vContentOffset.begin()+index;
        (*itr) = offset;
        
        CAObject* object = getObjectByIndex(index, CAControlStateNormal);
        
        if(CALabel* label = dynamic_cast<CALabel*>(object)){
            label->setFrameOrigin(CCPointMake(m_vContentOffset.at(index).width, m_vContentOffset.at(index).height));
        }else if(CAImage* image = dynamic_cast<CAImage*>(object)){
            m_vSegments.at(index)->removeAllSubviews();
            CAImageView* imageView = CAImageView::createWithImage(image);
            imageView->setFrameOrigin(CCPointMake(m_vContentOffset.at(index).width, m_vContentOffset.at(index).height));
            m_vSegments.at(index)->addSubview(imageView);
        }
    }
}

CCSize CASegmentedControl::getContentOffsetForSegmentAtIndex(int index)
{
    CCSize offset = CCSizeZero;
    if(index >= 0 && index < m_vSegments.size())
    {
        std::vector<CCSize>::iterator itr = m_vContentOffset.begin()+index;
        offset = (*itr);
    }
    return offset;
}

void CASegmentedControl::setEnabledForSegmentAtIndex(bool isEnable, int index)
{
    if(index >= 0 && index < m_vSegments.size())
    {
        std::vector<bool>::iterator itr = m_vIsEnabled.begin()+index;
        (*itr) = isEnable;
        refreshSegmentItemByIndex(index, CAControlStateNormal);
    }
    refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateSelected);
}

bool CASegmentedControl::isEnabledForSegmentAtIndex(int index)
{
    bool isEnable = true;
    if(index >= 0 && index < m_vSegments.size())
    {
        std::vector<bool>::iterator itr = m_vIsEnabled.begin()+index;
        isEnable = (*itr);
    }
    return isEnable;
}

void CASegmentedControl::removeSegmentAtIndex(int index)
{
    if((index<0 || index >= m_vSegments.size()) && m_vSegments.size() == 1)
        return;
    {
        CAVector<CALabel*>::iterator itr = m_vTitles.begin() + index;
        CC_SAFE_RELEASE(*itr);
        m_vTitles.erase(itr);
    }
    {
        CAVector<CAImage*>::iterator itr = m_vNormalImages.begin() + index;
        CC_SAFE_RELEASE(*itr);
        m_vNormalImages.erase(itr);
    }
    {
        std::vector<bool>::iterator itr = m_vIsEnabled.begin() + index;
        m_vIsEnabled.erase(itr);
    }
    {
        std::vector<CCSize>::iterator itr = m_vContentOffset.begin() + index;
        m_vContentOffset.erase(itr);
    }
    {
        std::vector<float>::iterator itr = m_vSegmentWidth.begin() + index;
        m_vSegmentWidth.erase(itr);
    }
    {
        CAVector<CAView*>::iterator itr = m_vSegmentItemBackground.begin() + index;
        (*itr)->removeFromSuperview();
        m_vSegmentItemBackground.erase(itr);
    }
    {
        CAVector<CAView*>::iterator itr = m_vSegments.begin() + index;
        (*itr)->removeFromSuperview();
        m_vSegments.erase(itr);
    }
    if(index <= m_iSelectedIndex)
    {
        m_iSelectedIndex--;
    }

    refreshAllSegmentItemBound();
    refreshAllSegmentItemPosition();
    setSegmentItemBackgroundImage(m_pSegmentItemBackgroundImage);
    
    cleanAllSeparate();
    createSeparate();
}


void CASegmentedControl::setContentSize(const CrossApp::CCSize &var)
{
    CCSize size = var;
    size.height = MAX(size.height, _px(58));
    size.width = MAX(size.width, size.height * 2);
    CAControl::setContentSize(size);
    if (m_pBackgroundView)
    {
        m_pBackgroundView->setFrame(this->getBounds());
    }
    cleanAllSeparate();
    createSeparate();
}

void CASegmentedControl::setBackgroundView(CrossApp::CAView *view)
{
    CC_SAFE_RETAIN(view);
    if(m_pBackgroundView != NULL)
        m_pBackgroundView->removeFromSuperview();
    m_pBackgroundView = view;
    m_pBackgroundView->setFrame(this->getBounds());
    this->insertSubview(m_pBackgroundView, -2);
}

CAView* CASegmentedControl::getBackgroundView()
{
    return m_pBackgroundView;
}

void CASegmentedControl::setBackgroundImage(CAImage* image)
{
    if(m_pBackgroundView != NULL)
        m_pBackgroundView->removeFromSuperview();
    CAScale9ImageView* backGroundImageView = CAScale9ImageView::createWithFrame(this->getBounds());
    backGroundImageView->setImage(image);
    backGroundImageView->setVisible(true);
    m_pBackgroundView = backGroundImageView;
    this->insertSubview(m_pBackgroundView, -2);
}

void CASegmentedControl::createSeparate()
{
    CCRect frame = CCRectMake(0, 0, m_fSeparateWidth, this->getBounds().size.height);
    float length = 0;
    int count = m_vSegments.size();
    std::vector<CAView *>::iterator itr = m_vSegments.begin();
    for(; itr != m_vSegments.end() && count != 1; ++ itr, --count)
    {
        length += (*itr)->getBounds().size.width;
        frame.origin.x = length;
        CAView* separateImageView = CAView::createWithFrame(frame);
        separateImageView->setColor(m_cTintColor);
        this->insertSubview(separateImageView, 100);
        m_vSeparateView.pushBack(separateImageView);
    }
}

void CASegmentedControl::cleanAllSeparate()
{
    std::vector<CAView *>::iterator itr = m_vSeparateView.begin();
    for(; itr != m_vSeparateView.end(); ++itr)
    {
        (*itr)->removeFromSuperview();
    }
    m_vSeparateView.clear();
}

void CASegmentedControl::removeAllSegments()
{
    for(int i=0; i<m_vSegments.size(); ++i)
    {
        removeSegmentAtIndex(i);
    }
}

void CASegmentedControl::refreshAllSegmentItemBound()
{
    CAVector<CAView*>::iterator itr_segment= m_vSegments.begin();
    for(int index=0; itr_segment!=m_vSegments.end(); ++itr_segment, ++index)
    {
        float setWidth = 0;
        int   setWidthCount = 0;
        std::vector<float>::iterator itr_width = m_vSegmentWidth.begin();
        for(; itr_width!=m_vSegmentWidth.end(); ++itr_width)
        {
            if(*itr_width != -1)
            {
                setWidth+= *itr_width;
                setWidthCount++;
            }
        }
        
        float segmentWidth = m_vSegmentWidth.at(index);
        if(segmentWidth == -1)
        {
            segmentWidth = (this->getBounds().size.width - setWidth)/(m_vSegmentWidth.size()-setWidthCount);
        }
        (*itr_segment)->setBounds(CCRectMake((*itr_segment)->getBounds().origin.x,
                                             (*itr_segment)->getBounds().origin.y,
                                             segmentWidth,
                                             (*itr_segment)->getBounds().size.height));
    }
}

void CASegmentedControl::refreshAllSegmentItemPosition()
{
    float length = 0;
    CAVector<CAView*>::iterator itr = m_vSegments.begin();
    for(; itr != m_vSegments.end(); ++itr)
    {
        CCRect rect = CCRectMake(length, 0, (*itr)->getBounds().size.width, (*itr)->getBounds().size.height);
        length += (*itr)->getBounds().size.width;
        (*itr)->setFrame(rect);
    }
    this->setFrame(CCRectMake(this->getFrameOrigin().x, this->getFrameOrigin().y, length, this->getBounds().size.height));
    /*this->setContentSize(CCSize(length, this->getBounds().size.height));*/
}

void CASegmentedControl::refreshAllSegmentItemBackgroundPosition()
{
    float length = 0;
    CAVector<CAView*>::iterator itr = m_vSegments.begin();
    CAVector<CAView*>::iterator itr_bg = m_vSegmentItemBackground.begin();
    for(; itr != m_vSegments.end(); ++itr, ++itr_bg)
    {
        CCRect rect = CCRectMake(length, 0, (*itr)->getFrame().size.width, (*itr)->getBounds().size.height);
        length += (*itr)->getFrame().size.width;
        (*itr_bg)->setFrame(rect);
    }
}

void CASegmentedControl::refreshSegmentItemByPoint(CCPoint point, CAControlState controlState)
{
    int index =getSegmentItemIndexByPoint(point);
    refreshSegmentItemByIndex(index, controlState);
}

void CASegmentedControl::refreshSegmentItemByIndex(int index, CAControlState controlState)
{
    if(index<0 && index>=m_vSegments.size())
        return;
    CAObject* object = getObjectByIndex(index, controlState);
    CCSize segmentSize = m_vSegments.at(index)->getBounds().size;
    CCSize contentOffset = m_vContentOffset.at(index);
    
    if(CALabel* label = dynamic_cast<CALabel*>(object))
    {
        label->setCenter(CCRectMake(segmentSize.width*0.5f + contentOffset.width,
                                    segmentSize.height*0.5f + contentOffset.height,
                                    segmentSize.width,
                                    segmentSize.height));
    }else if(CAImage* image = dynamic_cast<CAImage*>(object)){
        CCSize imageSize = image->getContentSize();
        CADipSize segmentSizeDip = CADipSize( segmentSize );
        float width = imageSize.width;
        float height = imageSize.height;
        if(imageSize.width > segmentSizeDip.width)
        {
            width = segmentSizeDip.width;
        }
        if(imageSize.height > segmentSizeDip.height)
        {
            height = segmentSizeDip.height;
        }
        m_vSegments.at(index)->removeAllSubviews();
        CAImageView* imageView = CAImageView::createWithImage(image);
        imageView->setCenter(CCRectMake(segmentSize.width*0.5f + contentOffset.width,
                                        segmentSize.height*0.5f + contentOffset.height,
                                        width,
                                        height));
        m_vSegments.at(index)->addSubview(imageView);
    }else{
        m_vSegments.at(index)->removeAllSubviews();
    }
}

int CASegmentedControl::getSegmentItemIndexByPoint(CCPoint point)
{
    if(point.x < 0)
        return -1;
    int index = 0;
    float length = 0;
    CAVector<CAView*>::iterator itr = m_vSegments.begin();
    for(; itr != m_vSegments.end(); ++itr)
    {
        length += (*itr)->getBounds().size.width;
        if(point.x < length)
        {
            break;
        }
        index++;
    }
    
    CCAssert(index < m_vSegments.size(), "index is overStep");
    return index;
}

CAObject* CASegmentedControl::getObjectByIndex(int index, CAControlState controlState)
{
    CAObject* object = NULL;
    if(index < m_vTitles.size() && index < m_vNormalImages.size())
    {
        if(CAControlStateNormal == controlState)
        {
            if(m_vTitles.at(index) != NULL){
                object = m_vTitles.at(index);
                ((CALabel*)object)->setColor(m_cTextColor);
            }else{
                object = m_vNormalImages.at(index);
            }
        }else if(CAControlStateHighlighted == controlState)
        {
            if(m_vTitles.at(index) != NULL){
                object = m_vTitles.at(index);
                ((CALabel*)object)->setColor(m_cTextSelectedColor);
            }else{
                object = m_vNormalImages.at(index);
            }
        }else if(CAControlStateSelected == controlState)
        {
            if(m_vTitles.at(index) != NULL){
                object = m_vTitles.at(index);
                ((CALabel*)object)->setColor(m_cTextSelectedColor);
            }else{
                object = m_vSelectedImages.at(index);
            }
        }
    }
    return object;
}

void CASegmentedControl::addTarget(CAObject* target, SEL_CASegmentedControl selector)
{
    m_pTarget = target;
    m_pCallFunc = selector;
}

void CASegmentedControl::callFunc(CAObject* object, int index)
{
    if(m_pTarget && m_pCallFunc)
    {
        ((CAObject *)m_pTarget->*m_pCallFunc)(this, index);
    }
}

CAView* CASegmentedControl::createDefaultSegment(int index)
{
    CAView* segment = CAView::createWithColor(CAColor_clear);
    if(segment)
    {
        m_vTitles.insert(index, NULL);

        m_vNormalImages.insert(index, NULL);
        
        m_vSelectedImages.insert(index, NULL);
        
        std::vector<bool>::iterator itr = m_vIsEnabled.begin()+index;
        m_vIsEnabled.insert(itr, true);
        
        std::vector<CCSize>::iterator itr_offset = m_vContentOffset.begin()+index;
        m_vContentOffset.insert(itr_offset, CCSizeMake(0, 0));
        
        std::vector<float>::iterator itr_width = m_vSegmentWidth.begin()+index;
        m_vSegmentWidth.insert(itr_width, -1);
        
        CAImageView* imageView = CAImageView::createWithImage(NULL);
        m_vSegmentItemBackground.insert(index, imageView);
        this->insertSubview(imageView, -1);
    }
    return segment;
}

void CASegmentedControl::setSegmentItemBackgroundImage(CAImage* image)
{
    m_pSegmentItemBackgroundImage = image;
    
    if(m_pBackgroundView && m_pBackgroundView->isVisible() && m_pSegmentItemBackgroundImage)
    {
        CAVector<CAView*>::iterator itr = m_vSegmentItemBackground.begin();
        
        for (int i=0; itr != m_vSegmentItemBackground.end() && (*itr)!=NULL; ++itr, ++i) {
            (*itr) -> removeFromSuperview();
            (*itr) = getTailorImageAtIndex(i, image);
            (*itr)->setVisible(false);
            this->insertSubview(*itr, -1);
        }
    }
    else if(m_pBackgroundView && !m_pBackgroundView->isVisible() && m_pSegmentItemBackgroundImage)
    {
        m_pBackgroundView->setVisible(true);
        std::vector<CAView*>::iterator itr = m_vSegmentItemBackground.begin();
        
        for (int i=0; itr != m_vSegmentItemBackground.end() && (*itr)!=NULL; ++itr, ++i) {
            (*itr) -> removeFromSuperview();
            (*itr) = getTailorImageAtIndex(i, image);
            (*itr)->setVisible(false);
            this->insertSubview(*itr, -1);
        }
        m_pBackgroundView->setVisible(false);
    }
    refreshAllSegmentItemBackgroundPosition();
}

CAView* CASegmentedControl::getTailorImageAtIndex(int index, CAImage* image)
{
    CAImageView* imageView = NULL;
    if (NULL == m_pBackgroundView) {
        imageView = CAImageView::createWithImage(CAImage::create(NULL));
        return imageView;
    }
    CAScale9ImageView* clipNode = CAScale9ImageView::createWithFrame(m_pBackgroundView->getBounds());
    clipNode->setImage(image);
    CAClippingView* clip = CAClippingView::create(m_pBackgroundView);
    clip->addSubview(clipNode);
    clip->setInverted(false);
    clip->setAlphaThreshold(0);
    
    float x = 0;
    
    for(int i=0; i<= index-1; ++i)
    {
        x +=m_vSegments.at(i)->getBounds().size.width;
    }
    
    CAView* segment = m_vSegments.at(index);
    if(segment)
    {
        clip->setFrame(this->getBounds());
        this->addSubview(clip);
        CARenderImage* render = CARenderImage::create(m_pBackgroundView->getBounds().size.width, m_pBackgroundView->getBounds().size.height, kCAImagePixelFormat_RGBA8888);
        render->beginWithClear(0, 0, 0, 0);
        clip->visit();
        render->end();
        clip->removeFromSuperview();
        
        CCRect rect = CCRectMake( x, 0, segment->getBounds().size.width, segment->getBounds().size.height);
        imageView = CAImageView::createWithImage(render->getSprite()->getImage());
        imageView->setRotationX(180.0f);
        imageView->setFrame(rect);
        imageView->setImageRect(rect, false, rect.size);
    }
    return imageView;
}

void CASegmentedControl::setSegmentItemBackgroundVisibleWithIndex(bool isVisible, int index)
{
    CAVector<CAView*>::iterator itr = m_vSegmentItemBackground.begin()+index;
    if(*itr!=NULL && index>-1 && index< m_vSegments.size())
        (*itr)->setVisible(isVisible);
}

#pragma mark --

bool CASegmentedControl::ccTouchBegan(CATouch *pTouch, CAEvent *pEvent)
{
    CCPoint point = pTouch->getLocation();
    point = this->convertToNodeSpace(point);
    
    int index = getSegmentItemIndexByPoint(point);
    if(index == m_iSelectedIndex)
    {
        return false;
    }
    m_iTouchIndex = index;
    if(isEnabledForSegmentAtIndex(m_iTouchIndex))
    {
        refreshSegmentItemByPoint(point, CAControlStateHighlighted);
        setSegmentItemBackgroundVisibleWithIndex(true, m_iTouchIndex);
        return true;
    }
    return false;
}

void CASegmentedControl::ccTouchMoved(CrossApp::CATouch *pTouch, CrossApp::CAEvent *pEvent)
{
    CCPoint point = pTouch->getLocation();
    point = this->convertToNodeSpace(point);
    
    if (getBounds().containsPoint(point))
    {
        int index =getSegmentItemIndexByPoint(point);
        if(m_iTouchIndex != index && m_iTouchIndex != -1)
        {
            refreshSegmentItemByIndex(m_iTouchIndex, CAControlStateNormal);
            setSegmentItemBackgroundVisibleWithIndex(false, m_iTouchIndex);
        }
        if(index != m_iSelectedIndex)
        {
            m_iTouchIndex = index;
            refreshSegmentItemByIndex(index, CAControlStateHighlighted);
            setSegmentItemBackgroundVisibleWithIndex(true, index);
        }
    }
    else
    {
        if(m_iTouchIndex == -1)
            return;
        refreshSegmentItemByIndex(m_iTouchIndex, CAControlStateNormal);
        setSegmentItemBackgroundVisibleWithIndex(false, m_iTouchIndex);
        m_iTouchIndex = -1;
    }
}

void CASegmentedControl::ccTouchEnded(CATouch *pTouch, CAEvent *pEvent)
{
    CCPoint point = pTouch->getLocation();
    point = this->convertToNodeSpace(point);
    
    int index =getSegmentItemIndexByPoint(point);
    if(index>-1 && index<m_vSegments.size())
    {
        if( -1 != m_iSelectedIndex )
            refreshSegmentItemByIndex(m_iSelectedIndex, CAControlStateNormal);
        setSegmentItemBackgroundVisibleWithIndex(false, m_iSelectedIndex);
        refreshSegmentItemByIndex(index, CAControlStateSelected);
        setSegmentItemBackgroundVisibleWithIndex(true, index);
        m_iSelectedIndex = index;
        callFunc(this, index);
    }
}

void CASegmentedControl::ccTouchCancelled(CATouch *pTouch, CAEvent *pEvent)
{
    refreshSegmentItemByIndex(m_iTouchIndex, CAControlStateNormal);
    setSegmentItemBackgroundVisibleWithIndex(false, m_iTouchIndex);

}

NS_CC_END
