#include "chat.h"
#include "message.h"
using namespace Telegram;

Chat::Chat(QJsonObject chat)
{
    id = chat.value("id").toDouble();
    QString chatType = chat.value("type").toString();
    if (chatType == "private") type = Private;
    else if (chatType == "group") type = Group;
    else if (chatType == "channel") type = Channel;
    username = chat.value("username").toString();
    firstname = chat.value("first_name").toString();
    lastname = chat.value("last_name").toString();
}

ChatId::ChatId(const Message &msg) {
    if (msg.user.id) {
        _idI = msg.user.id;
    } else {
        _idI = msg.chat.id;
    }
}

bool ChatId::operator <(const ChatId &b) const
{
    if (_idS.length() && b._idS.length())
        return _idS < b._idS;
    if (_idS.length() || b._idS.length()) {
        // mixed. convert both to strings:
        QString sa (toString());
        QString sb (b.toString());
        return sa < sb;
    }
    // both ints
    return _idI < b._idI;
}
