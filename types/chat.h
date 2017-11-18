#ifndef CHAT_H
#define CHAT_H

#include <QDebug>
#include <QString>
#include <QJsonObject>

namespace Telegram {

class Chat;
class Message;

class ChatId
{
public:
    ChatId(int64_t id) : _idI(id) {}
    ChatId(const QString &id) : _idI(0), _idS(id) {}
    ChatId(const Message &msg); // prefer msg.from.id, then msg.chat.id
    ChatId() = delete;
    operator QVariant () const {
        if (_idS.length())
            return QVariant(_idS);
        else
            return QVariant(QString("%1").arg(_idI));
    }
    QString toString() const { if (_idS.length()) return _idS; else return QString("%1").arg(_idI);}
    bool operator<( const ChatId &b) const;
    bool operator==(const ChatId &b) const;
    bool operator!=(const ChatId &b) const;
protected:
    int64_t _idI;
    QString _idS;
};

class Chat
{
public:
    Chat() : id(0), type(Private) {}
    Chat(QJsonObject chat);

    enum ChatType {
        Private, Group, Channel
    };

    int64_t id;
    ChatType type;
    QString title;
    QString username;
    QString firstname;
    QString lastname;
};

inline QDebug operator<< (QDebug dbg, const Chat &chat)
{
    dbg.nospace() << qUtf8Printable(QString("Telegram::Chat(id=%1; type=%2; title=%3; username=%4; firstname=%5; lastname=%6)")
                                    .arg(chat.id)
                                    .arg(chat.type)
                                    .arg(chat.title)
                                    .arg(chat.username)
                                    .arg(chat.firstname)
                                    .arg(chat.lastname));

    return dbg.maybeSpace();
}

}

#endif // CHAT_H
