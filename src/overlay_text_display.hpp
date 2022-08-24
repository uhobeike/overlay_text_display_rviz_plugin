#ifndef OVERLAY_TEXT_DISPLAY_RVIZ_PLUGIN__OVERLAY_TEXT_DISPLAY_HPP_
#define OVERLAY_TEXT_DISPLAY_RVIZ_PLUGIN__OVERLAY_TEXT_DISPLAY_HPP_

#ifndef Q_MOC_RUN

#include <Overlay/OgreTextAreaOverlayElement.h>
#include <rcl_interfaces/msg/log.hpp>
#include <rviz_common/properties/color_property.hpp>
#include <rviz_common/properties/int_property.hpp>
#include <rviz_common/ros_topic_display.hpp>
#include <std_msgs/msg/string.hpp>

#endif

namespace overlay_text_display_rviz_plugin {
class OverlayTextDisplay
    : public rviz_common::RosTopicDisplay<std_msgs::msg::String> {
  Q_OBJECT
public:
  OverlayTextDisplay();
  virtual ~OverlayTextDisplay();

  virtual void onInitialize();
  virtual void reset();
  virtual void load(const rviz_common::Config &config);
  virtual void save(rviz_common::Config config) const;

private Q_SLOTS:
  void renderOverlay();

private:
  void processMessage(std_msgs::msg::String::ConstSharedPtr msg) override;
  // void processMessage(const rcl_interfaces::msg::Log &msg);
  rviz_common::properties::IntProperty *height_property_;
  rviz_common::properties::IntProperty *width_property_;
  rviz_common::properties::IntProperty *size_property_;
  rviz_common::properties::StringProperty *text_title_name_property_;
  rviz_common::properties::ColorProperty *text_title_color_property_;
  rviz_common::properties::ColorProperty *text_value_color_property_;

  Ogre::TextAreaOverlayElement *createTextElement(int index);
  void createMaterial(std::string mat_name);
  void destroyMaterial(std::string mat_name);

  std_msgs::msg::String text_msgs_;

  Ogre::Overlay *overlay_;
  Ogre::OverlayContainer *panel_;
  std::vector<Ogre::TextAreaOverlayElement *> text_elements_;
  Ogre::OverlayElement *mat_element_;
};

} // namespace overlay_text_display_rviz_plugin

#endif // OVERLAY_TEXT_DISPLAY_RVIZ_PLUGIN__OVERLAY_TEXT_DISPLAY_HPP_