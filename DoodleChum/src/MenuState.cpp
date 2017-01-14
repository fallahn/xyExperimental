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
#include <MessageIDs.hpp>

#include <xygine/App.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/ui/Button.hpp>
#include <xygine/ui/CheckBox.hpp>
#include <xygine/ui/ComboBox.hpp>
#include <xygine/ui/Slider.hpp>
#include <xygine/ui/Selection.hpp>
#include <xygine/ui/Label.hpp>
#include <xygine/ui/Marquee.hpp>
#include <xygine/util/ConfigFile.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    const float numberPadding = 12.f;
}

MenuState::MenuState(xy::StateStack& stack, Context context, const sf::Font& font, xy::TextureResource& tr, xy::Util::ConfigFile& cf)
    : State             (stack, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_textureResource   (tr),
    m_font              (font),
    m_config            (cf),
    m_helpContainer     (m_messageBus),
    m_optionContainer   (m_messageBus),
    m_creditsContainer  (m_messageBus),
    m_currentContainer  (&m_helpContainer),
    m_in                (true),
    m_scale             (0.f)
{
    auto msg = getContext().appInstance.getMessageBus().post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->stateID = States::ID::Menu;
    msg->type = xy::Message::UIEvent::MenuOpened;

    m_background.setTexture(m_textureResource.get("assets/images/ui/menu_background.png"));
    m_background.setOrigin(xy::DefaultSceneSize / 2.f);
    m_background.setPosition(xy::DefaultSceneSize / 2.f);
    m_background.setScale(0.f, 0.f);

    m_tabs.setColor(sf::Color::Transparent);
    auto& tex = m_textureResource.get("assets/images/ui/menu_tabs.png");
    tex.setSmooth(true);
    m_tabs.setTexture(tex);
    m_tabs.setScale(2.f, 2.f);

    m_versionNumber.setFont(font);
    m_versionNumber.setString("V:1.0.0");
    m_versionNumber.setFillColor(sf::Color::Black);
    m_versionNumber.setPosition(xy::DefaultSceneSize.x - (m_versionNumber.getLocalBounds().width + numberPadding),
                                xy::DefaultSceneSize.y - (m_versionNumber.getLocalBounds().height + numberPadding));

    buildHelp();
    buildOptions();
    buildCredits();

    m_currentContainer->setScale(0.f, 0.f);
}

//public
bool MenuState::handleEvent(const sf::Event& evt)
{ 
    m_currentContainer->handleEvent(evt, getContext().appInstance.getMouseWorldPosition());
    
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
        case sf::Keyboard::BackSpace:
            m_in = false;
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
    m_currentContainer->update(dt);
    
  
    dt *= 2.f;
    if (m_in)
    {
        m_scale = std::min(1.f, m_scale + dt);
        
    }
    else
    {
        m_scale = std::max(0.f, m_scale - dt);
        if (m_scale == 0)
        {
            requestStackPop();

            /*auto msg = getContext().appInstance.getMessageBus().post<xy::Message::UIEvent>(xy::Message::UIMessage);
            msg->stateID = States::ID::Menu;
            msg->type = xy::Message::UIEvent::MenuClosed;*/
        }
    }
    m_background.setScale(m_scale, m_scale);
    m_currentContainer->setScale(m_scale, m_scale);

    sf::Color c = sf::Color::White;
    c.a = static_cast<sf::Uint8>(m_scale * 255.f);
    m_tabs.setColor(c);

    c.r = 0; c.g = 0; c.b = 0;
    m_versionNumber.setFillColor(c);

    return true;
}

void MenuState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);
    rw.draw(m_tabs);
    rw.draw(m_background);
    rw.draw(*m_currentContainer);
    rw.draw(m_versionNumber);
}

//private
void MenuState::buildHelp()
{
    m_helpContainer.setPosition(xy::DefaultSceneSize / 2.f);
    m_helpContainer.setBackgroundTexture(m_textureResource.get("assets/images/ui/menu_pic.png"), true);
    auto containerOffset = xy::DefaultSceneSize / 2.f;
    containerOffset.y += 30.f;
    m_helpContainer.setPosition(containerOffset);
    
    auto text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("DoodleBob!");
    text->setCharacterSize(80u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 420.f);
    text->move(-containerOffset);
    text->setColour(sf::Color::Black);
    m_helpContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("(and Bella)");
    text->setCharacterSize(50u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 480.f);
    text->move(-containerOffset);
    text->setColour(sf::Color::Black);
    m_helpContainer.addControl(text);
    
    auto button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/start_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(xy::DefaultSceneSize.x / 2.f, 820.f);
    button->move(-containerOffset);
    button->setString("Continue...");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        m_in = false;

        auto msg = getContext().appInstance.getMessageBus().post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->stateID = States::ID::Menu;
        msg->type = xy::Message::UIEvent::MenuClosed;
    });
    m_helpContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(720.f, 820.f);
    button->move(-containerOffset);
    button->setString("Quit");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        xy::App::quit();
    });
    m_helpContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(1200.f, 820.f);
    button->move(-containerOffset);
    button->setString("Options");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        m_currentContainer = &m_optionContainer;
    });
    m_helpContainer.addControl(button);

    auto marquee = xy::UI::create<xy::UI::Marquee>(m_font, sf::Vector2u(800, 50));
    marquee->setAlignment(xy::UI::Alignment::Centre);
    marquee->setPosition(xy::DefaultSceneSize.x / 2.f, 770.f);
    marquee->move(-containerOffset);
    marquee->setTextColour(sf::Color::Black);
    marquee->setScrollSpeed(100.f);
    marquee->addString("Make sure to check up on Bob regularly!");
    marquee->addString("Tip: you can use the arrow keys or the numpad to open the side tabs");
    marquee->addString("Dont like Bobs music? Place your own in the music folder and Bob will play something you do like!");
    marquee->addString("Tip: You can return to this menu at any time by pressing P, Pause or Escape");
    marquee->addString("Tip: if you dont want to listen to the entire musical reptoire, then disable playing the full track in the options menu");
    m_helpContainer.addControl(marquee);
}

void MenuState::buildOptions()
{   
    m_optionContainer.setOrigin(xy::DefaultSceneSize / 2.f);
    m_optionContainer.setPosition(xy::DefaultSceneSize / 2.f);

    auto text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Options");
    text->setCharacterSize(80u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 420.f);
    text->setColour(sf::Color::Black);
    m_optionContainer.addControl(text);
    
    auto soundSlider = std::make_shared<xy::UI::Slider>(m_font, m_textureResource.get("assets/images/ui/slider_handle.png"), 375.f);
    soundSlider->setPosition(620.f, 554.f);
    soundSlider->setString("Volume");
    soundSlider->setTextColour(sf::Color::Black);
    soundSlider->setBarColour(sf::Color::Black);
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
    muteCheckbox->setPosition(1060.f, 534.f);
    muteCheckbox->setString("Mute");
    muteCheckbox->setTextColour(sf::Color::Black);
    muteCheckbox->addCallback([this, soundSlider](const xy::UI::CheckBox* checkBox)
    {
        auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
        msg->type = (checkBox->checked()) ? xy::Message::UIEvent::RequestAudioMute : xy::Message::UIEvent::RequestAudioUnmute;
        msg->value = soundSlider->getValue();
    }, xy::UI::CheckBox::Event::CheckChanged);
    muteCheckbox->check(getContext().appInstance.getAudioSettings().muted);
    m_optionContainer.addControl(muteCheckbox);

    auto resolutionBox = std::make_shared<xy::UI::Selection>(m_font, m_textureResource.get("assets/images/ui/scroll_arrow.png"), 375.f);
    resolutionBox->setPosition(620.f, 604.f);
    resolutionBox->setTextColour(sf::Color::Black);

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
    fullscreenCheckbox->setPosition(1060.f, 604.f);
    fullscreenCheckbox->setString("Full Screen");
    fullscreenCheckbox->setTextColour(sf::Color::Black);
    fullscreenCheckbox->check((getContext().appInstance.getVideoSettings().WindowStyle & sf::Style::Fullscreen) != 0);
    m_optionContainer.addControl(fullscreenCheckbox);

    auto shadowCheckbox = std::make_shared<xy::UI::CheckBox>(m_font, m_textureResource.get("assets/images/ui/checkbox.png"));
    shadowCheckbox->setPosition(1060.f, 674.f);
    shadowCheckbox->setString("Shadow Maps");
    shadowCheckbox->setTextColour(sf::Color::Black);   
    shadowCheckbox->addCallback([this](const xy::UI::CheckBox* cb)
    {
        auto msg = m_messageBus.post<Message::SystemEvent>(Message::System);
        msg->action = Message::SystemEvent::ToggleShadowMapping;
        msg->value = cb->checked();

        m_config["use_shadowmapping"] = cb->checked();
    }, xy::UI::CheckBox::Event::CheckChanged);
    shadowCheckbox->check(m_config["use_shadowmapping"].asBool());
    m_optionContainer.addControl(shadowCheckbox);

    auto fullTrackCheckbox = xy::UI::create<xy::UI::CheckBox>(m_font, m_textureResource.get("assets/images/ui/checkbox.png"));
    fullTrackCheckbox->setString("Play full audio tracks");
    fullTrackCheckbox->setPosition(620.f, 674.f);
    fullTrackCheckbox->setTextColour(sf::Color::Black);
    fullTrackCheckbox->addCallback([this](const xy::UI::CheckBox* cb)
    {
        auto msg = m_messageBus.post<Message::SystemEvent>(Message::System);
        msg->action = Message::SystemEvent::ToggleFullTrack;
        msg->value = cb->checked();
        
        m_config["play_full_tracks"] = cb->checked();
    }, xy::UI::CheckBox::Event::CheckChanged);
    fullTrackCheckbox->check(m_config["play_full_tracks"].asBool());
    m_optionContainer.addControl(fullTrackCheckbox);

    auto applyButton = std::make_shared<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    applyButton->setString("Apply");
    applyButton->setTextColour(sf::Color::Black);
    applyButton->setAlignment(xy::UI::Alignment::Centre);
    applyButton->setPosition(xy::DefaultSceneSize.x / 2.f, 820.f);
    applyButton->addCallback([fullscreenCheckbox, resolutionBox, this]()
    {
        auto res = resolutionBox->getSelectedValue();

        xy::App::VideoSettings settings;
        settings.VideoMode.width = res >> 16;
        settings.VideoMode.height = res & 0xFFFF;
        settings.WindowStyle = (fullscreenCheckbox->checked()) ? sf::Style::Fullscreen : sf::Style::Close;
        getContext().appInstance.applyVideoSettings(settings);
    });
    m_optionContainer.addControl(applyButton);

    auto button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(720.f, 820.f);
    button->setString("Back");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        m_currentContainer = &m_helpContainer;
    });
    m_optionContainer.addControl(button);

    button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(1200.f, 820.f);
    button->setString("Credits");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        m_currentContainer = &m_creditsContainer;
    });
    m_optionContainer.addControl(button);
}

void MenuState::buildCredits()
{
    m_creditsContainer.setOrigin(xy::DefaultSceneSize / 2.f);
    m_creditsContainer.setPosition(xy::DefaultSceneSize / 2.f);

    auto text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Credits");
    text->setCharacterSize(80u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 420.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Josh Mercier - 3D models & artwork");
    text->setCharacterSize(40u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 500.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Matt Marchant - programming & doodles");
    text->setCharacterSize(40u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 560.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Dorus Abeln - music & doodles");
    text->setCharacterSize(36u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 620.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("SFML Community - join us in #sfml on irc.boxbox.org!");
    text->setCharacterSize(36u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(xy::DefaultSceneSize.x / 2.f, 680.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    text = xy::UI::create<xy::UI::Label>(m_font);
    text->setString("Proudly powered by xygine");
    text->setCharacterSize(30u);
    text->setAlignment(xy::UI::Alignment::Centre);
    text->setPosition(1160.f, 820.f);
    text->setColour(sf::Color::Black);
    m_creditsContainer.addControl(text);

    auto button = xy::UI::create<xy::UI::Button>(m_font, m_textureResource.get("assets/images/ui/small_button.png"));
    button->setAlignment(xy::UI::Alignment::Centre);
    button->setPosition(720.f, 820.f);
    button->setString("Back");
    button->setTextColour(sf::Color::Black);
    button->addCallback([this]()
    {
        m_currentContainer = &m_optionContainer;
    });
    m_creditsContainer.addControl(button);

    auto marquee = xy::UI::create<xy::UI::Marquee>(m_font, sf::Vector2u(800, 50));
    marquee->setAlignment(xy::UI::Alignment::Centre);
    marquee->setPosition(xy::DefaultSceneSize.x / 2.f, 770.f);
    marquee->setTextColour(sf::Color::Black);
    marquee->setScrollSpeed(100.f);
    marquee->addString("Special mention goes to: Jonny for his contributions to xygine, Doris for the exemplary xygine logo, Naomi for all her patience, Jo & Jamz for their creative input");
    marquee->addString("And not to forget Malone and Jon for being just plain wonderful people!");
    m_creditsContainer.addControl(marquee);
}