#pragma once

#include "vector_list_box.h"

#include "../UtilityComponents/connector.h"
#include "../UtilityComponents/modal_dialog.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

/// Displays a name and a button for further configuration.
template <typename T>
class list_config_item : public updatable_component<std::shared_ptr<T>>,
                         public TextButton::Listener {
public:
    using get_callout_component = std::function<std::unique_ptr<Component>(std::shared_ptr<T>)>;

    list_config_item(std::shared_ptr<T> model, get_callout_component get_callout_component, const String& name)
            : model_{std::move(model)}
            , get_callout_component_{std::move(get_callout_component)} {
        label_.setInterceptsMouseClicks(false, false);

        update(model_);

        this->addAndMakeVisible(label_);
        this->addAndMakeVisible(button_);

        button_.setTooltip("Configure this " + name + ".");
    }

    list_config_item(const list_config_item&) = delete;
    list_config_item(list_config_item&&) noexcept = delete;

    list_config_item& operator=(const list_config_item&) = delete;
    list_config_item& operator=(list_config_item&&) noexcept = delete;

    virtual ~list_config_item() noexcept = default;

    void update(std::shared_ptr<T> model) override {
        model_ = std::move(model);
        set_label(model_->get_name());
    }

    void set_label(const std::string& label) {
        label_.setText(label, dontSendNotification);
    }

    void resized() override {
        const auto button_width = this->getHeight();
        auto bounds = this->getLocalBounds();
        button_.setBounds(bounds.removeFromRight(button_width).reduced(2, 2));
        label_.setBounds(bounds.reduced(2, 2));
    }

    void buttonClicked(Button*) override {
        begin_modal_dialog("",
                           make_done_window_ptr(get_callout_component_(model_)),
                           [](auto) {});
    }

private:
    std::shared_ptr<T> model_;
    Label label_;
    TextButton button_{"..."};
    model::Connector<TextButton> button_connector_{&button_, this};

    get_callout_component get_callout_component_;
};

template <typename T>
auto make_list_config_item_ptr(
        std::shared_ptr<T> model, typename list_config_item<T>::get_callout_component callback, const String& name) {
    return std::make_unique<list_config_item<T>>(std::move(model),
                                                 std::move(callback),
                                                 name);
}

}  // namespace left_bar
