#include "overlay_text_display.hpp"

#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>

#include <Overlay/OgreOverlay.h>
#include <Overlay/OgreOverlayContainer.h>
#include <Overlay/OgreOverlayElement.h>
#include <Overlay/OgreOverlayManager.h>
#include <Overlay/OgrePanelOverlayElement.h>

#include <rviz_rendering/render_system.hpp>

#include <QColor>
#include <QImage>

#include <iostream>

namespace overlay_text_display_rviz_plugin {
OverlayTextDisplay::OverlayTextDisplay() {
  height_property_ = new rviz_common::properties::IntProperty(
      "Back Window Height", 200, "window height", this, SLOT(renderOverlay()));
  width_property_ = new rviz_common::properties::IntProperty(
      "Back Window Width", 400, "window width", this, SLOT(renderOverlay()));
  size_property_ = new rviz_common::properties::IntProperty(
      "Text Size", 0, "window size", this, SLOT(renderOverlay()));
  text_title_name_property_ = new rviz_common::properties::StringProperty(
      "Text Title", "None", "text name", this, SLOT(renderOverlay()));
  text_title_color_property_ = new rviz_common::properties::ColorProperty(
      "Text Title Color", QColor(255, 30, 227),
      "Text Title color for the Debug.", this, SLOT(updateBackgroundColor()));
  text_value_color_property_ = new rviz_common::properties::ColorProperty(
      "Text Value Color", QColor(255, 255, 255),
      "Text Value color for the Debug.", this, SLOT(updateBackgroundColor()));

  overlay_ = Ogre::OverlayManager::getSingleton().create("RvizLogOverlay");
  panel_ = (Ogre::OverlayContainer *)Ogre::OverlayManager::getSingleton()
               .createOverlayElement("Panel", "TextPanel");
  overlay_->add2D(panel_);

  createMaterial("BackGround");
  mat_element_ = Ogre::OverlayManager::getSingleton().createOverlayElement(
      "Panel", "TexturePanel");
  mat_element_->setMetricsMode(Ogre::GuiMetricsMode::GMM_PIXELS);
  mat_element_->setMaterialName("BackGround");
  overlay_->add2D((Ogre::OverlayContainer *)mat_element_);

  overlay_->show();
}

OverlayTextDisplay::~OverlayTextDisplay() {
  Ogre::OverlayManager::getSingleton().destroy("RvizLogOverlay");
  Ogre::OverlayManager::getSingleton().destroyOverlayElement(panel_);
  for (auto element : text_elements_)
    Ogre::OverlayManager::getSingleton().destroyOverlayElement(element);
  Ogre::OverlayManager::getSingleton().destroyOverlayElement(mat_element_);
  destroyMaterial("BackGround");
}

void OverlayTextDisplay::onInitialize() {
  rviz_rendering::RenderSystem::get()->prepareOverlays(scene_manager_);
  RTDClass::onInitialize();
}

void OverlayTextDisplay::reset() { RTDClass::reset(); }

void OverlayTextDisplay::save(rviz_common::Config config) const {
  rviz_common::Display::save(config);
  config.mapSetValue("Height", height_property_->getInt());
  config.mapSetValue("Width", width_property_->getInt());
  config.mapSetValue("Size", size_property_->getInt());
  config.mapSetValue("Text_name", text_title_name_property_->getValue());
}

void OverlayTextDisplay::load(const rviz_common::Config &config) {
  rviz_common::Display::load(config);
  int tmp_int;
  if (config.mapGetInt("Height", &tmp_int))
    height_property_->setInt(tmp_int);
  if (config.mapGetInt("Width", &tmp_int))
    width_property_->setInt(tmp_int);
  if (config.mapGetInt("Size", &tmp_int))
    size_property_->setInt(tmp_int);
  QString tmp_string;
  if (config.mapGetString("Text Title", &tmp_string))
    size_property_->setValue(tmp_string);
  QVariant tmp_color;
  if (config.mapGetValue("Text Title Color", &tmp_color))
    size_property_->setValue(tmp_color);
  if (config.mapGetValue("Text Value Color", &tmp_color))
    size_property_->setValue(tmp_color);
}

Ogre::TextAreaOverlayElement *OverlayTextDisplay::createTextElement(int index) {
  std::string name = "Text" + std::to_string(index);
  Ogre::TextAreaOverlayElement *text_ptr =
      (Ogre::TextAreaOverlayElement *)Ogre::OverlayManager::getSingleton()
          .createOverlayElement("TextArea", name);
  text_ptr->setMetricsMode(Ogre::GuiMetricsMode::GMM_PIXELS);
  text_ptr->setVerticalAlignment(Ogre::GuiVerticalAlignment::GVA_TOP);
  text_ptr->setHorizontalAlignment(Ogre::GuiHorizontalAlignment::GHA_LEFT);
  text_ptr->setColour(Ogre::ColourValue::White);
  text_ptr->setFontName("Liberation Sans");
  text_ptr->setCaption("text");
  text_ptr->setCharHeight(16);
  text_ptr->setPosition(16, 16 * (index + 1));
  return text_ptr;
}

void OverlayTextDisplay::processMessage(
    std_msgs::msg::String::ConstSharedPtr msg) {

  text_msgs_ = *msg;

  renderOverlay();
}

void OverlayTextDisplay::createMaterial(std::string mat_name) {
  int width = 100;
  int height = 100;
  std::string texture_name = mat_name + "Texture";
  QColor bg_color(0, 0, 0, 70.0);

  Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
      texture_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DEFAULT);

  Ogre::MaterialPtr mat =
      (Ogre::MaterialPtr)Ogre::MaterialManager::getSingleton().create(
          mat_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          true);

  mat->getTechnique(0)->getPass(0)->createTextureUnitState(texture_name);
  mat->getTechnique(0)->getPass(0)->setSceneBlending(
      Ogre::SBT_TRANSPARENT_ALPHA);

  Ogre::HardwarePixelBufferSharedPtr pixel_buffer = texture->getBuffer();
  pixel_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
  const Ogre::PixelBox &pixelBox = pixel_buffer->getCurrentLock();
  Ogre::uint8 *pDest = static_cast<Ogre::uint8 *>(pixelBox.data);
  memset(pDest, 0, width * height);
  QImage Hud = QImage(pDest, width, height, QImage::Format_ARGB32);
  for (unsigned int i = 0; i < width; i++) {
    for (unsigned int j = 0; j < height; j++) {
      Hud.setPixel(i, j, bg_color.rgba());
    }
  }
  pixel_buffer->unlock();
}

void OverlayTextDisplay::destroyMaterial(std::string mat_name) {
  std::string texture_name = mat_name + "Texture";
  Ogre::TextureManager::getSingleton().destroyResourcePool(texture_name);
  Ogre::MaterialManager::getSingleton().destroyResourcePool(mat_name);
}

void OverlayTextDisplay::renderOverlay() {
  if (!isEnabled()) {
    overlay_->hide();
    return;
  }
  overlay_->show();

  auto view_width = Ogre::OverlayManager::getSingleton().getViewportWidth();
  auto window_width = width_property_->getInt();
  auto window_height = height_property_->getInt();
  auto text_size = size_property_->getInt();
  auto text_title_name = text_title_name_property_->getValue();
  auto text_title_color = text_title_color_property_->getOgreColor();
  auto text_value_color = text_value_color_property_->getOgreColor();

  mat_element_->setDimensions(window_width, window_height);
  mat_element_->setPosition(0, 0);

  static bool once_flag = true;
  if (once_flag) {
    text_elements_.push_back(createTextElement(0));
    text_elements_.push_back(createTextElement(1));
    panel_->addChild(text_elements_[0]);
    panel_->addChild(text_elements_[1]);
  }
  once_flag = false;

  text_elements_[0]->setColour(text_title_color);
  text_elements_[1]->setColour(text_value_color);

  text_elements_[0]->setPosition(window_width * 0.05, window_height * 0.05);
  text_elements_[1]->setPosition(window_width / 4, window_height / 2);

  text_elements_[0]->setCharHeight(50 + text_size);
  text_elements_[1]->setCharHeight(100 + text_size);

  text_elements_[0]->setCaption(text_title_name.toString().toStdString());
  text_elements_[1]->setCaption(text_msgs_.data);
}
} // namespace overlay_text_display_rviz_plugin

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(overlay_text_display_rviz_plugin::OverlayTextDisplay,
                       rviz_common::Display)