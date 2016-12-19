/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

DoodleChum - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <MenuState.hpp>

#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/CheckBox.hpp>
#include <xygine/ui/ComboBox.hpp>
#include <xygine/ui/Slider.hpp>
#include <xygine/ui/Selection.hpp>

#include <SFML/Window/Event.hpp>


MenuState::MenuState(xy::StateStack& stack, Context context)
    : State             (stack, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_optionContainer   (m_messageBus)
{
    auto msg = getContext().appInstance.getMessageBus().post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->stateID = States::ID::Menu;
    msg->type = xy::Message::UIEvent::MenuOpened;

    buildOptions();
}

//public
bool MenuState::handleEvent(const sf::Event& evt)
{ 
    m_optionContainer.handleEvent(evt, getContext().appInstance.getMouseWorldPosition());
    
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
        case sf::Keyboard::BackSpace:
            requestStackPop();
            {
                auto msg = getContext().appInstance.getMessageBus().post<xy::Message::UIEvent>(xy::Message::UIMessage);
                msg->stateID = States::ID::Menu;
                msg->type = xy::Message::UIEvent::MenuClosed;
            }
            break;
        }
    }
    return false; 
}

void MenuState::handleMessage(const xy::Message&) 
{
    
}

bool MenuState::update(float dt)
{
    m_optionContainer.update(dt);
    
    return true;
}

void MenuState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);
    rw.draw(m_optionContainer);
}

//private
void MenuState::buildOptions()
{
    m_font.loadFromFile("assets/fonts/FallahnHand.ttf");
    
    auto soundSlider = std::make_shared<xy::UI::Slider>(m_font, m_textureResource.get("assets/images/ui/slider_handle.png"), 375.f);
    soundSlider->setPosition(640.f, 314.f);
    soundSlider->setText("Volume");
    soundSlider->setMaxValue(1.f);
    soundSlider->addCallback([this](const xy::UI::Slider* slider)
    {
        //send volume setting command
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::RequestVolumeChange;
        msg->value = slider->getValue();

    }, xy::UI::Slider::Event::ValueChanged);
    soundSlider->setValue(getContext().appInstance.getAudioSettings().volume); //set this *after* callback is set
    m_optionContainer.addControl(soundSlider);

    auto muteCheckbox = std::make_shared<xy::UI::CheckBox>(m_font, m_textureResource.get("assets/images/ui/checkbox.png"));
    muteCheckbox->setPosition(1110.f, 274.f);
    muteCheckbox->setText("Mute");
    muteCheckbox->addCallback([this](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestAudioMute : xy::Message::UIEvent::RequestAudioUnmute;
    }, xy::UI::CheckBox::Event::CheckChanged);
    muteCheckbox->check(getContext().appInstance.getAudioSettings().muted);
    m_optionContainer.addControl(muteCheckbox);

    auto resolutionBox = std::make_shared<xy::UI::Selection>(m_font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    resolutionBox->setPosition(640.f, 354.f);

    const auto& modes = getContext().appInstance.getVideoSettings().AvailableVideoModes;
    auto i = 0u;
    auto j = 0u;
    for (const auto& m : modes)
    {
        std::string name = std::to_string(m.width) + " x " + std::to_string(m.height);
        sf::Int32 val = (m.width << 16) | m.height;
        resolutionBox->addItem(name, val);
        //select currently active mode
        if (getContext().appInstance.getVideoSettings().VideoMode != m)
            i++;
        else
            j = i;
    }
    if (i < modes.size()) resolutionBox->setSelectedIndex(j);

    m_optionContainer.addControl(resolutionBox);

    auto fullscreenCheckbox = std::make_shared<xy::UI::CheckBox>(m_font, m_textureResource.get("assets/images/ui/checkbox.png"));
    fullscreenCheckbox->setPosition(1110.f, 354.f);
    fullscreenCheckbox->setText("Full Screen");
    fullscreenCheckbox->addCallback([this](const xy::UI::CheckBox*)
    {

    }, xy::UI::CheckBox::Event::CheckChanged);
    fullscreenCheckbox->check((getContext().appInstance.getVideoSettings().WindowStyle & sf::Style::Fullscreen) != 0);
    m_optionContainer.addControl(fullscreenCheckbox);


    auto applyButton = std::make_shared<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/start_button.png"));
    applyButton->setText("Apply");
    applyButton->setAlignment(xy::UI::Alignment::Centre);
    applyButton->setPosition(xy::DefaultSceneSize.x / 2.f, 580.f);
    applyButton->addCallback([fullscreenCheckbox, resolutionBox, this]()
    {
        auto res = resolutionBox->getSelectedValue();

        xy::App::VideoSettings settings;
        settings.VideoMode.width = res >> 16;
        settings.VideoMode.height = res & 0xFFFF;
        settings.WindowStyle = (fullscreenCheckbox->checked()) ? sf::Style::Fullscreen : sf::Style::Close;
        getContext().appInstance.applyVideoSettings(settings);
        xy::App::setMouseCursorVisible(false);

        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = xy::Message::UIEvent::ResizedWindow;
    });
    m_optionContainer.addControl(applyButton);
}