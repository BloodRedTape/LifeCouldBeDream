#include "simple_bot.hpp"
#include <tgbot/net/TgLongPoll.h>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string.hpp>

KeyboardLayout Keyboard::ToKeyboard(const std::vector<std::string>& texts) {
    return {ToKeyboardRow(texts)};
}

KeyboardLayout Keyboard::ToNiceKeyboard(const std::vector<std::string>& texts, std::size_t row_size, std::function<std::string(std::string)> make_key)
{
    KeyboardLayout layout;
    std::vector<KeyboardButton> row;
    
    std::size_t i = 0;
    for (const auto& text : texts) {
        row.emplace_back(text, make_key(text));
        i++;

        if (i == row_size) {
            i = 0;
            layout.push_back(std::move(row));
        }
    }
    
    if(row.size())
        layout.push_back(std::move(row));

    return layout;
}

std::vector<KeyboardButton> Keyboard::ToKeyboardRow(const std::vector<std::string>& texts) {
    std::vector<KeyboardButton> row;

    for (const auto& text : texts) {
        row.emplace_back(text);
    }
    
    return row;
}

static TgBot::InlineKeyboardMarkup::Ptr ToInlineKeyboardMarkup(const KeyboardLayout &keyboard) {
    if(!keyboard.size())
        return nullptr;

    TgBot::InlineKeyboardMarkup::Ptr keyboard_markup(new TgBot::InlineKeyboardMarkup);

    for (const auto& row : keyboard) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> row_markup;

        for (const auto& button : row) {
            if(!button.Enabled)
                continue;

            TgBot::InlineKeyboardButton::Ptr button_markup(new TgBot::InlineKeyboardButton);
            button_markup->text = button.Text;
            button_markup->callbackData = button.CallbackData;

            row_markup.push_back(button_markup);
        }
        
        if(row_markup.size())
            keyboard_markup->inlineKeyboard.push_back(row_markup);
    }
    
    return keyboard_markup;
}

SimpleBot::SimpleBot(const std::string& token):
	TgBot::Bot(token)
{
    getEvents().onUnknownCommand([this](TgBot::Message::Ptr message) {
        std::string command = ParseCommand(message);

        Println("Command '%'", command);

        if (!command.size()) {
            return;
        }

        BroadcastCommand(command, message);
    });

    m_Username = getApi().getMe()->username;
}

void SimpleBot::LongPoll(){
	TgBot::TgLongPoll long_poll(*this);

    while(true){
        long_poll.start();
    }
}

void SimpleBot::OnLog(LogHandler handler){
	m_Log = handler;
}

void SimpleBot::Log(const std::string& message){
    if(!m_Log)
        return;

	m_Log(message);
}

void SimpleBot::ClearOldUpdates(){
    getApi().getUpdates(-1, 1);
}

TgBot::Message::Ptr SimpleBot::SendMessage(std::int64_t chat, std::int32_t topic, const std::string& message, std::int64_t reply_message) {
    return SendMessage(chat, topic, message, nullptr, reply_message);
}

TgBot::Message::Ptr SimpleBot::SendMessage(std::int64_t chat, std::int32_t topic, const std::string& message, TgBot::GenericReply::Ptr reply, std::int64_t reply_message) {
    if (!message.size()) {
        Log("Can't send empty messages");
        return nullptr;
    }

    TgBot::Message::Ptr result = nullptr;

    try {
        result = getApi().sendMessage(chat, message, DisableWebpagePreview, reply_message, reply, ParseMode, false, {}, false, false, topic);
    }
    catch (const std::exception& exception) {
        auto chat_ptr = getApi().getChat(chat);
        
        std::string chat_name = chat_ptr->username.size() ? chat_ptr->username : chat_ptr->title;

        Log("Failed to send message to chat '%' id % reason %", chat_name, chat, exception.what());
    }

    return result;
}

TgBot::Message::Ptr SimpleBot::EditMessage(TgBot::Message::Ptr message, const std::string& text, const KeyboardLayout& keyboard) {
    return EditMessage(message, text, ToInlineKeyboardMarkup(keyboard));
}

TgBot::Message::Ptr SimpleBot::SendKeyboard(std::int64_t chat, std::int32_t topic, const std::string& message, const KeyboardLayout& keyboard, std::int64_t reply_message) {
    return SendMessage(chat, topic, message, ToInlineKeyboardMarkup(keyboard), reply_message);
}

TgBot::Message::Ptr SimpleBot::SendPhoto(std::int64_t chat, std::int32_t topic, const std::string& text, TgBot::InputFile::Ptr photo, std::int64_t reply_message){
    try {
        return getApi().sendPhoto(chat, photo, text, reply_message, nullptr, ParseMode, false, {}, false, false, topic, false);
    }catch (const std::exception& exception) {
        auto chat_ptr = getApi().getChat(chat);
        std::string chat_name = chat_ptr->username.size() ? chat_ptr->username : chat_ptr->title;

        Log("Failed to send photo in chat '%' id % reason %", chat_name, chat_ptr->id, exception.what());
    }
    return nullptr;
}

TgBot::Message::Ptr SimpleBot::SendPhoto(TgBot::Message::Ptr source, const std::string& text, TgBot::InputFile::Ptr photo){
    return SendPhoto(source->chat->id, source->isTopicMessage ? source->messageThreadId : 0, text, photo);
}

TgBot::Message::Ptr SimpleBot::ReplyPhoto(TgBot::Message::Ptr source, const std::string& text, TgBot::InputFile::Ptr photo){
    return SendPhoto(source->chat->id, source->isTopicMessage ? source->messageThreadId : 0, text, photo, source->messageId);
}

TgBot::Message::Ptr SimpleBot::EditMessage(TgBot::Message::Ptr message, const std::string& text, TgBot::GenericReply::Ptr reply) {
    try{
        if (text.size() && message->text != text) {
            return getApi().editMessageText(text, message->chat->id, message->messageId, "", ParseMode, DisableWebpagePreview, reply);
        } else {
            return getApi().editMessageReplyMarkup(message->chat->id, message->messageId, "", reply);
        }
    }
    catch (const std::exception& exception) {
        auto chat_ptr = message->chat;
        std::string chat_name = chat_ptr->username.size() ? chat_ptr->username : chat_ptr->title;

        Log("Failed to edit message in chat '%' id % reason %", chat_name, chat_ptr->id, exception.what());
    }
    return nullptr;
}

TgBot::Message::Ptr SimpleBot::EditMessage(TgBot::Message::Ptr message, const KeyboardLayout& keyboard) {
    return EditMessage(message, "", ToInlineKeyboardMarkup(keyboard));
}

TgBot::Message::Ptr SimpleBot::EditMessage(TgBot::Message::Ptr message, const std::string& text) {
    return EditMessage(message, text, nullptr);
}

bool SimpleBot::AnswerCallbackQuery(const std::string& callbackQueryId, const std::string& text) {
    try{
        return getApi().answerCallbackQuery(callbackQueryId, text);
    }
    catch (const std::exception& exception) {
        Log("Failed to answer callback query %", callbackQueryId);
    }
    return false;
}

void SimpleBot::DeleteMessage(TgBot::Message::Ptr message) {
    if (!message)
        return;

    assert(message->chat);

    try{
        getApi().deleteMessage(message->chat->id, message->messageId);
    }
    catch (const std::exception& exception) {
        auto chat = message->chat;

        std::string chat_name = chat->username.size() ? chat->username : chat->title;

        Log("Failed to delete message % from chat %, id %, reason: %", message->messageId, chat_name, chat->id, exception.what());
    }
}

void SimpleBot::RemoveKeyboard(TgBot::Message::Ptr message)
{
    if(!message)
        return;

    if(message->replyMarkup)
        EditMessage(message, message->text);
}

TgBot::Message::Ptr SimpleBot::EnsureMessage(TgBot::Message::Ptr ensurable, std::int64_t chat, std::int32_t topic, const std::string& message, TgBot::GenericReply::Ptr reply)
{
    if(!ensurable)
        return SendMessage(chat, topic, message, reply);
    else
        return EditMessage(ensurable, message, reply);
}

TgBot::Message::Ptr SimpleBot::EnsureKeyboard(TgBot::Message::Ptr ensurable, std::int64_t chat, std::int32_t topic, const std::string& message, const KeyboardLayout& keyboard)
{
    return EnsureMessage(ensurable, chat, topic, message, ToInlineKeyboardMarkup(keyboard));
}

void SimpleBot::OnCommand(const std::string& command, CommandHandler handler){
    m_CommandHandlers.emplace(command, handler);
}

void SimpleBot::BroadcastCommand(const std::string& command, TgBot::Message::Ptr message){
    auto it = m_CommandHandlers.find(command);

    if(it == m_CommandHandlers.end())
        return;

    const auto &handler = it->second;

    handler(message);
}

void SimpleBot::OnNonCommandMessage(MessageHandler handler){
    getEvents().onNonCommandMessage(handler);
}

void SimpleBot::OnCallbackQuery(CallbackQueryHandler handler){
    getEvents().onCallbackQuery(handler);
}

void SimpleBot::OnMyChatMember(ChatMemberStatusHandler chat_member){
    getEvents().onMyChatMember(chat_member);
}

std::string SimpleBot::ParseCommand(TgBot::Message::Ptr message)
{
    const char Space = ' ';
    const char At = '@';

	if (!boost::algorithm::starts_with(message->text, "/"))
		return {};

    std::string command_name = message->text.substr(0, message->text.find(Space));

    std::size_t at = command_name.find(At);
    std::size_t end = std::min(at, command_name.size());

    std::string command = command_name.substr(1, end - 1);

    if (at != std::string::npos) {
        std::string command_bot_name = command_name.substr(at + 1);
        
        if(command_bot_name != m_Username)
            return {};
    }

    if (boost::range::count(command, ' '))
        return {};
    
    return command;
}
