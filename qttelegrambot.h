#ifndef QTTELEGRAMBOT_H
#define QTTELEGRAMBOT_H

#include <map>
#include <functional>
#include <QObject>
#include <QLoggingCategory>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpPart>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMimeDatabase>
#include <QTimer>

#include "networking.h"
#include "types/chat.h"
#include "types/update.h"
#include "types/user.h"
#include "types/file.h"
#include "types/message.h"
#include "types/reply/genericreply.h"
#include "types/reply/replykeyboardmarkup.h"
#include "types/reply/replykeyboardhide.h"
#include "types/reply/forcereply.h"

namespace Telegram {
Q_DECLARE_LOGGING_CATEGORY(CTelBot)

typedef QList<QList<PhotoSize> > UserProfilePhotos;

class Bot : public QObject
{
    Q_OBJECT
public:
    /**mes
     * Bot constructor
     * @param token
     * @param updates - enable automatic update polling
     * @param updateInterval - interval between update polls in msec
     * @param pollingTimeout - timeout in sec
     * @param parent
     */
    explicit Bot(const QString &token, bool updates = false, quint32 updateInterval = 1000, quint32 pollingTimeout = 0, QObject *parent = 0);
    ~Bot();

    enum ChatAction { Typing, UploadingPhoto, RecordingVideo, UploadingVideo, RecordingAudio, UploadingAudio, UploadingDocument, FindingLocation };

    /**
     * Returns basic information about the bot in form of a `User` object.
     * @return User Object
     * @see https://core.telegram.org/bots/api#getme
     */
    User getMe();
    bool asyncGetMe(); // emits signal getMe async
    bool asyncGetChat(const QVariant &chatId); // emits signal gotObject with Chat object

    /**
     * Send text message.
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param text - Text of the message to be sent
     * @param markdown - Use markdown in message display (only Telegram for Android supports this)
     * @param disableWebPagePreview - Disables link previews for links in this message
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendmessage
     */
    bool sendMessage(const ChatId &chatId, const QString &text, bool markdown = false, bool disableWebPagePreview = false, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());


    bool setChatTitle(const ChatId &chatId, const QString &title);

    /**
     * Forward messages of any kind.
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fromChatId - Unique identifier for the chat where the original message was sent
     * @param messageId - Unique message identifier
     * @return success
     * @see https://core.telegram.org/bots/api#forwardmessage
     */
    bool forwardMessage(QVariant chatId, quint32 fromChatId, quint32 messageId);

    /**
     * Send a photo
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param file - A file to send
     * @param caption - Photo caption
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendphoto
     */
    bool sendPhoto(const ChatId &chatId, QFile *file, QString caption = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a photo
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param caption - Photo caption
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendphoto
     */
    bool sendPhoto(const ChatId &chatId, QString fileId, QString caption = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send audio
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param file - A file to send
     * @param duration - Duration of the audio in seconds
     * @param performer - Performer of the audio
     * @param title - Track name of the audio
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendaudio
     */
    bool sendAudio(const ChatId &chatId, QFile *file, qint64 duration = -1, QString performer = QString(), QString title = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send audio
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent audio
     * @param duration - Duration of the audio in seconds
     * @param performer - Performer of the audio
     * @param title - Track name of the audio
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendaudio
     */
    bool sendAudio(const ChatId &chatId, QString fileId, qint64 duration = -1, QString performer = QString(), QString title = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a document
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param file - A file to send
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#senddocument
     */
    bool sendDocument(const ChatId &chatId, QFile *file, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a document
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#senddocument
     */
    bool sendDocument(const ChatId &chatId, QString fileId, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a sticker
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param file - A file to send
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendsticker
     */
    bool sendSticker(const ChatId &chatId, QFile *file, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a sticker
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendsticker
     */
    bool sendSticker(const ChatId &chatId, QString fileId, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a video
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param duration - Duration of sent video in seconds
     * @param caption - Video caption
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendvideo
     */
    bool sendVideo(const ChatId &chatId, QFile *file, qint64 duration = -1, QString caption = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a video
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param duration - Duration of sent video in seconds
     * @param caption - Video caption
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendvideo
     */
    bool sendVideo(const ChatId &chatId, QString fileId, qint64 duration = -1, QString caption = QString(), qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a voice
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param duration - Duration of sent audio in seconds
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendvoice
     */
    bool sendVoice(const ChatId &chatId, QFile *file, qint64 duration = -1, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a voice
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param fileId - Telegram file_id of already sent photo
     * @param duration - Duration of sent audio in seconds
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendvoice
     */
    bool sendVoice(const ChatId &chatId, QString fileId, qint64 duration = -1, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Send a location
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param latitude - latitude of the location
     * @param longitude - longitude of the location
     * @param replyToMessageId - If the message is a reply, ID of the original message
     * @param replyMarkup - Additional interface options
     * @return success
     * @see https://core.telegram.org/bots/api#sendlocation
     */
    bool sendLocation(QVariant chatId, float latitude, float longitude, qint32 replyToMessageId = -1, const GenericReply &replyMarkup = GenericReply());

    /**
     * Use this method when you need to tell the user that something is happening on the bot's side.
     * @param chatId - Unique identifier for the message recipient or @channelname
     * @param action - Type of action to broadcast
     * @return success
     * @see https://core.telegram.org/bots/api#sendchataction
     */
    bool sendChatAction(QVariant chatId, ChatAction action);

    /**
     * Use this method to get a list of profile pictures for a user.
     * @param userId - Unique identifier of the target user
     * @param offset - Sequential number of the first photo to be returned.
     * @param limit - Limits the number of photos to be retrieved. Values between 1—100 are accepted. Defaults to 100.
     * @return UserProfilePhotos list
     * @see Use this method to get a list of profile pictures for a user.
     */
    UserProfilePhotos getUserProfilePhotos(quint32 userId, qint16 offset =  -1, qint8 limit = -1);

    /**
     * Use this method to receive incoming updates using long polling
     * @param timeout - Timeout in seconds for long polling.
     * @param limit - Limits the number of updates to be retrieved.
     * @param offset - Identifier of the first update to be returned.
     * @return List of Update objects
     * @see https://core.telegram.org/bots/api#getupdates
     */
    QList<Update> getUpdates(quint32 timeout, quint32 limit, quint32 offset);

    /**
     * Use this method to specify a url and receive incoming updates via an outgoing webhook.
     * @param url - HTTPS url to send updates to. Use an empty string to remove webhook integration
     * @param certificate - Upload your public key certificate so that the root certificate in use can be checked.
     * @return success
     * @see https://core.telegram.org/bots/api#setwebhook
     */
    bool setWebhook(const QString &url, QFile *certificate);

    /**
     * Use this method to get basic info about a file and prepare it for downloading.
     * @param fileId - File identifier to get info about
     * @return File object
     * @see https://core.telegram.org/bots/api#getfile
     */
    File getFile(const QString &fileId);

private:
    Networking *m_net;

    bool _sendPayload(const ChatId &chatId, QFile *filePayload, ParameterList params, qint32 replyToMessageId, const GenericReply &replyMarkup, QString payloadField, QString endpoint);
    bool _sendPayload(const ChatId &chatId, const QString &textPayload, ParameterList &params, qint32 replyToMessageId, const GenericReply &replyMarkup, const QString &payloadField, const QString &endpoint);

    QJsonObject jsonObjectFromByteArray(QByteArray json);
    QJsonArray jsonArrayFromByteArray(QByteArray json);
    bool responseOk(QByteArray json);

    void internalGetUpdates();
    QTimer *m_internalUpdateTimer;
    quint32 m_updateInterval;
    uint64_t m_updateOffset;
    quint32 m_pollingTimeout;
    //typedef void (*processReplyFunc)(QNetworkReply*);
    std::map<QNetworkReply*, std::function<void(QNetworkReply*)>> _pendingReplies;

private slots:
    void requestFinished(QNetworkReply *reply);

signals:
    void getMe(User user);
    void gotObject(QJsonObject obj);
    void message(uint64_t update_id, Message message);
};

}

#endif // QTTELEGRAMBOT_H
