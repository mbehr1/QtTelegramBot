#include <QThread>
#include <QCoreApplication>
#include "qttelegrambot.h"

using namespace Telegram;

Bot::Bot(const QString &token, bool updates, quint32 updateInterval, quint32 pollingTimeout, QObject *parent) :
    QObject(parent),
    m_net(new Networking(token)),
    m_internalUpdateTimer(new QTimer(this)),
    m_updateInterval(updateInterval),
    m_updateOffset(0),
    m_pollingTimeout(pollingTimeout)
{
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    connect(m_net, SIGNAL(requestFinished(QNetworkReply*)),
            this, SLOT(requestFinished(QNetworkReply*)));

    if (updates) {
        m_internalUpdateTimer->setSingleShot(true);
        connect(m_internalUpdateTimer, &QTimer::timeout, this, &Bot::internalGetUpdates);
        internalGetUpdates();
    }
}

Bot::~Bot()
{
    // stop and delete the timer first
    m_internalUpdateTimer->stop();
    delete m_internalUpdateTimer;
    m_internalUpdateTimer = 0;

    if (_pendingReplies.size()) {
        qWarning() << __PRETTY_FUNCTION__ << "got replies pending" << _pendingReplies.size();
        while (_pendingReplies.size()) {
            QCoreApplication::instance()->processEvents(QEventLoop::AllEvents, 1000);
            QThread::msleep(100);
        }
        qDebug() << __PRETTY_FUNCTION__ << "processed all replies pending" << _pendingReplies.size();
    }
    disconnect(m_net, SIGNAL(requestFinished(QNetworkReply*)),
               this, SLOT(requestFinished(QNetworkReply*)));

    delete m_net;
}

void Bot::requestFinished(QNetworkReply *reply)
{
    if (!reply)
        qWarning() << __PRETTY_FUNCTION__ << "null reply!";
    else {
        // search in map
        auto it = _pendingReplies.find(reply);
        if (it!= _pendingReplies.end()) {
            auto &fn = (*it).second;
            fn(reply);
            _pendingReplies.erase(reply);
        } else {
            qWarning() << __PRETTY_FUNCTION__ << "couldnt find reply in pendingReplies map!" << reply;
        }
        reply->deleteLater();
    }
}

bool Bot::asyncGetMe()
{
    auto reply = m_net->asyncRequest(ENDPOINT_GET_ME, ParameterList(), Networking::GET);
    if (!reply) return false;
    _pendingReplies.insert(std::make_pair(reply,
                                          [this](QNetworkReply *reply) {
                               if (reply->error() != QNetworkReply::NoError) {
                                   qCritical("%s", qPrintable(QString("[%1] %2").arg(reply->error()).arg(reply->errorString())));
                                   return; // todo emit empty/unknown user here!
                               }
                               QByteArray arr = reply->readAll();
                               QJsonObject json = jsonObjectFromByteArray(arr);
                               User ret;
                               ret.id = json.value("id").toInt();
                               ret.firstname = json.value("first_name").toString();
                               ret.lastname = json.value("last_name").toString();
                               ret.username = json.value("username").toString();
                               if (ret.id == 0 || ret.firstname.isEmpty()) {
                                   qCritical("%s", qPrintable("Got invalid user in " + QString(ENDPOINT_GET_ME)));
                                   emit getMe(User());
                               } else
                               emit getMe(ret);
                           }
                               ));
    return true;
}

bool Bot::asyncGetChat(const QVariant &chatId)
{
    ParameterList params;
    params.insert("chat_id", HttpParameter(chatId));

    auto reply = m_net->asyncRequest(ENDPOINT_GET_CHAT, params, Networking::GET);
    if (!reply) return false;
    _pendingReplies.insert(std::make_pair(reply,
                                          [this](QNetworkReply *reply) {
                               if (reply->error() != QNetworkReply::NoError) {
                                   qCritical("%s", qPrintable(QString("[%1] %2").arg(reply->error()).arg(reply->errorString())));
                                   return; // todo emit empty/unknown user here!
                               }
                               QByteArray arr = reply->readAll();
                               QJsonObject json = jsonObjectFromByteArray(arr);
                               qDebug() << __PRETTY_FUNCTION__ << json;
                               emit gotObject(json);
                           }
                               ));
    return true;
}


/*
User Bot::getMe()
{   
    QJsonObject json = this->jsonObjectFromByteArray(
                m_net->request(ENDPOINT_GET_ME, ParameterList(), Networking::GET));

    User ret;
    ret.id = json.value("id").toInt();
    ret.firstname = json.value("first_name").toString();
    ret.lastname = json.value("last_name").toString();
    ret.username = json.value("username").toString();

    if (ret.id == 0 || ret.firstname.isEmpty()) {
        qCritical("%s", qPrintable("Got invalid user in " + QString(ENDPOINT_GET_ME)));
        return User();
    }

    return ret;
}
*/

bool Bot::sendMessage(const ChatId &chatId, const QString &text, bool markdown, bool disableWebPagePreview, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    if (markdown) params.insert("parse_mode", HttpParameter("Markdown"));
    if (disableWebPagePreview) params.insert("disable_web_page_preview", HttpParameter(disableWebPagePreview));

    return this->_sendPayload(chatId, text, params, replyToMessageId, replyMarkup, "text", ENDPOINT_SEND_MESSAGE);
}

bool Bot::setChatTitle(const ChatId &chatId, const QString &title)
{
    ParameterList params;

    return this->_sendPayload(chatId, title, params, 0, GenericReply(), "title", ENDPOINT_SET_CHAT_TITLE);
}


/*
bool Bot::forwardMessage(QVariant chatId, quint32 fromChatId, quint32 messageId)
{
    if (chatId.type() != QVariant::String && chatId.type() != QVariant::Int) {
        qCritical("Please provide a QString or int as chatId");
        return false;
    }
    ParameterList params;
    params.insert("chat_id", HttpParameter(chatId));
    params.insert("from_chat_id", HttpParameter(fromChatId));
    params.insert("message_id", HttpParameter(messageId));

    bool success = this->responseOk(m_net->request(ENDPOINT_FORWARD_MESSAGE, params, Networking::POST));

    return success;
}*/

bool Bot::sendPhoto(const ChatId &chatId, QFile *file, QString caption, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    if (!caption.isEmpty()) params.insert("caption", HttpParameter(caption));

    return this->_sendPayload(chatId, file, params, replyToMessageId, replyMarkup, "photo", ENDPOINT_SEND_PHOTO);
}

bool Bot::sendPhoto(const ChatId &chatId, QString fileId, QString caption, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    if (!caption.isEmpty()) params.insert("caption", HttpParameter(caption));

    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "photo", ENDPOINT_SEND_PHOTO);
}

bool Bot::sendAudio(const ChatId &chatId, QFile *file, qint64 duration, QString performer, QString title, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    if (duration >= 0) params.insert("duration", HttpParameter(duration));
    if (!performer.isEmpty()) params.insert("performer", HttpParameter(performer));
    if (!title.isEmpty()) params.insert("title", HttpParameter(title));

    return this->_sendPayload(chatId, file, params, replyToMessageId, replyMarkup, "audio", ENDPOINT_SEND_AUDIO);
}

bool Bot::sendAudio(const ChatId &chatId, QString fileId, qint64 duration, QString performer, QString title, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    if (duration >= 0) params.insert("duration", HttpParameter(duration));
    if (!performer.isEmpty()) params.insert("performer", HttpParameter(performer));
    if (!title.isEmpty()) params.insert("title", HttpParameter(title));

    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "audio", ENDPOINT_SEND_AUDIO);
}

bool Bot::sendDocument(const ChatId &chatId, QFile *file, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    return this->_sendPayload(chatId, file, ParameterList(), replyToMessageId, replyMarkup, "document", ENDPOINT_SEND_DOCUMENT);
}

bool Bot::sendDocument(const ChatId &chatId, QString fileId, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "document", ENDPOINT_SEND_DOCUMENT);
}

bool Bot::sendSticker(const ChatId &chatId, QFile *file, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    return this->_sendPayload(chatId, file, ParameterList(), replyToMessageId, replyMarkup, "sticker", ENDPOINT_SEND_STICKER);
}

bool Bot::sendSticker(const ChatId &chatId, QString fileId, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "sticker", ENDPOINT_SEND_STICKER);
}

bool Bot::sendVideo(const ChatId &chatId, QFile *file, qint64 duration, QString caption, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    params.insert("duration", HttpParameter(duration));
    params.insert("caption", HttpParameter(caption));

    return this->_sendPayload(chatId, file, params, replyToMessageId, replyMarkup, "video", ENDPOINT_SEND_VIDEO);
}

bool Bot::sendVideo(const ChatId &chatId, QString fileId, qint64 duration, QString caption, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    params.insert("duration", HttpParameter(duration));
    params.insert("caption", HttpParameter(caption));

    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "video", ENDPOINT_SEND_VIDEO);
}

bool Bot::sendVoice(const ChatId &chatId, QFile *file, qint64 duration, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    params.insert("duration", HttpParameter(duration));

    return this->_sendPayload(chatId, file, params, replyToMessageId, replyMarkup, "voice", ENDPOINT_SEND_VOICE);
}

bool Bot::sendVoice(const ChatId &chatId, QString fileId, qint64 duration, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    ParameterList params;
    params.insert("duration", HttpParameter(duration));

    return this->_sendPayload(chatId, fileId, params, replyToMessageId, replyMarkup, "voice", ENDPOINT_SEND_VOICE);
}

/*
bool Bot::sendLocation(QVariant chatId, float latitude, float longitude, qint32 replyToMessageId, const GenericReply &replyMarkup)
{
    Q_UNUSED(replyMarkup); // TODO
    if (chatId.type() != QVariant::String && chatId.type() != QVariant::Int) {
        qCritical("Please provide a QString or int as chatId");
        return false;
    }
    ParameterList params;
    params.insert("chat_id", HttpParameter(chatId));
    params.insert("latitude", HttpParameter(latitude));
    params.insert("longitude", HttpParameter(longitude));
    if (replyToMessageId >= 0) params.insert("reply_to_message_id", HttpParameter(replyToMessageId));

    bool success = this->responseOk(m_net->request(ENDPOINT_SEND_LOCATION, params, Networking::POST));

    return success;
} */
/*
bool Bot::sendChatAction(QVariant chatId, Bot::ChatAction action)
{
    if (chatId.type() != QVariant::String && chatId.type() != QVariant::Int) {
        qCritical("Please provide a QString or int as chatId");
        return false;
    }

    ParameterList params;
    params.insert("chat_id", HttpParameter(chatId));
    switch (action) {
    case Typing:
        params.insert("action", HttpParameter("typing"));
        break;
    case UploadingPhoto:
        params.insert("action", HttpParameter("upload_photo"));
        break;
    case RecordingVideo:
        params.insert("action", HttpParameter("record_video"));
        break;
    case UploadingVideo:
        params.insert("action", HttpParameter("upload_video"));
        break;
    case RecordingAudio:
        params.insert("action", HttpParameter("record_audio"));
        break;
    case UploadingAudio:
        params.insert("action", HttpParameter("upload_audio"));
        break;
    case UploadingDocument:
        params.insert("action", HttpParameter("upload_document"));
        break;
    case FindingLocation:
        params.insert("action", HttpParameter("find_location"));
        break;
    default:
        return false;
    }

    bool success = this->responseOk(m_net->request(ENDPOINT_SEND_CHAT_ACTION, params, Networking::POST));

    return success;
}
*/
/*
UserProfilePhotos Bot::getUserProfilePhotos(quint32 userId, qint16 offset, qint8 limit)
{
    ParameterList params;
    params.insert("user_id", HttpParameter(userId));
    if (offset > -1) params.insert("offset", HttpParameter(offset));
    if (limit > -1) params.insert("limit", HttpParameter(limit));

    QJsonObject json = this->jsonObjectFromByteArray(m_net->request(ENDPOINT_GET_USER_PROFILE_PHOTOS, params, Networking::GET));

    UserProfilePhotos ret;

    QList<PhotoSize> photo;
    foreach (QJsonValue val, json.value("photos").toArray()) {
        photo = QList<PhotoSize>();

        foreach (QJsonValue p, val.toArray()) {
            PhotoSize ps;
            ps.fileId = p.toObject().value("file_id").toString();
            ps.width = p.toObject().value("width").toInt();
            ps.height = p.toObject().value("height").toInt();
            if (p.toObject().contains("file_size")) ps.fileSize = p.toObject().value("file_size").toInt();
            photo.append(ps);
        }

        ret.append(photo);
    }

    return ret;
}
*/
/*
bool Bot::setWebhook(const QString &url, QFile *certificate)
{
    ParameterList params;
    params.insert("url", HttpParameter(url));

    QMimeDatabase db;
    bool openedFile = false;
    if (!certificate->isOpen()) {
        if (!certificate->open(QFile::ReadOnly)) {
            qCritical("Could not open file %s [%s]", qPrintable(certificate->fileName()), qPrintable(certificate->errorString()));
            return false;
        }
        openedFile = true;
    }
    QByteArray data = certificate->readAll();
    if (openedFile) certificate->close();
    params.insert("certificate", HttpParameter(data, true, db.mimeTypeForData(data).name(), certificate->fileName()));

    bool success = this->responseOk(m_net->request(ENDPOINT_SET_WEBHOOK, params, Networking::UPLOAD));

    return success;
}
*/
/*
File Bot::getFile(const QString &fileId)
{
    ParameterList params;
    params.insert("file_id", HttpParameter(fileId));

    QJsonObject json = this->jsonObjectFromByteArray(m_net->request(ENDPOINT_GET_FILE, params, Networking::GET));

    return File(json.value("file_id").toString(), json.value("file_size").toInt(-1), json.value("file_path").toString());
}
*/

bool Bot::_sendPayload(const ChatId &chatId, QFile *filePayload, ParameterList params, qint32 replyToMessageId, const GenericReply &replyMarkup, QString payloadField, QString endpoint)
{
    params.insert("chat_id", HttpParameter(chatId));

    QMimeDatabase db;
    bool openedFile = false;
    if (!filePayload->isOpen()) {
        if (!filePayload->open(QFile::ReadOnly)) {
            qCritical("Could not open file %s [%s]", qPrintable(filePayload->fileName()), qPrintable(filePayload->errorString()));
            return false;
        }
        openedFile = true;
    }
    QByteArray data = filePayload->readAll();
    if (openedFile) filePayload->close();
    params.insert(payloadField, HttpParameter(data, true, db.mimeTypeForData(data).name(), filePayload->fileName()));

    if (replyToMessageId >= 0) params.insert("reply_to_message_id", HttpParameter(replyToMessageId));
    if (replyMarkup.isValid()) params.insert("reply_markup", HttpParameter(replyMarkup.serialize()));

    //bool success = this->responseOk(m_net->request(endpoint, params, Networking::UPLOAD));
    auto reply = m_net->asyncRequest(endpoint, params, Networking::UPLOAD);
    if (!reply) return false;
    _pendingReplies.insert(std::make_pair(reply,
                                          [this](QNetworkReply *reply) {
                               if (reply->error() != QNetworkReply::NoError) {
                                   qCritical("%s", qPrintable(QString("[%1] %2").arg(reply->error()).arg(reply->errorString())));
                                   return; // todo emit signal here?
                               }
                               QByteArray arr = reply->readAll();
                               bool success = responseOk(arr);
                               if (!success)
                               qWarning() << "_sendPayload no success" << reply;
                               // emit getMe(ret); todo emit a signal here
                           }
                               ));
    return true;
}


bool Bot::_sendPayload(const ChatId &chatId, const QString &textPayload, ParameterList &params, qint32 replyToMessageId, const GenericReply &replyMarkup, const QString &payloadField, const QString &endpoint)
{
    params.insert("chat_id", HttpParameter(chatId));
    params.insert(payloadField, HttpParameter(textPayload));
    if (replyToMessageId >= 0) params.insert("reply_to_message_id", HttpParameter(replyToMessageId));
    if (replyMarkup.isValid()) params.insert("reply_markup", HttpParameter(replyMarkup.serialize()));

    //bool success = this->responseOk(m_net->request(endpoint, params, Networking::POST));
    auto reply = m_net->asyncRequest(endpoint, params, Networking::POST);
    if (!reply) return false;
    _pendingReplies.insert(std::make_pair(reply,
                                          [this](QNetworkReply *reply) {
                               if (reply->error() != QNetworkReply::NoError) {
                                   qCritical("%s", qPrintable(QString("[%1] %2").arg(reply->error()).arg(reply->errorString())));
                                   return; // todo emit signal here?
                               }
                               QByteArray arr = reply->readAll();
                               bool success = responseOk(arr);
                               if (!success)
                               qWarning() << "_sendPayload no success" << reply;
                               // emit getMe(ret); todo emit a signal here
                           }
                               ));
    return true;
}

QJsonObject Bot::jsonObjectFromByteArray(QByteArray json)
{
    QJsonDocument d = QJsonDocument::fromJson(json);
    QJsonObject obj = d.object();

    if (obj.isEmpty()) {
        qCritical("Got an empty response object");
        return obj;
    }

    if (obj.value("ok").toBool() != true) {
        qWarning("Result is not Ok");
        return obj;
    }

    return obj.value("result").toObject();
}

QJsonArray Bot::jsonArrayFromByteArray(QByteArray json)
{
    QJsonDocument d = QJsonDocument::fromJson(json);
    QJsonObject obj = d.object();

    if (obj.isEmpty()) {
        qCritical("Got an empty response object");
        return QJsonArray();
    }

    if (obj.value("ok").toBool() != true) {
        qWarning("Result is not Ok");
        return QJsonArray();
    }

    return obj.value("result").toArray();
}

bool Bot::responseOk(QByteArray json)
{
    QJsonDocument d = QJsonDocument::fromJson(json);
    QJsonObject obj = d.object();

    return (!obj.isEmpty() && obj.value("ok").toBool() == true);
}
/*
QList<Update> Bot::getUpdates(quint32 timeout, quint32 limit, quint32 offset)
{
    ParameterList params;
    params.insert("offset", HttpParameter(offset));
    params.insert("limit", HttpParameter(limit));
    params.insert("timeout", HttpParameter(timeout));
    QJsonArray json = this->jsonArrayFromByteArray(m_net->request(ENDPOINT_GET_UPDATES, params, Networking::GET));

    QList<Update> ret = QList<Update>();
    foreach (QJsonValue value, json) {
        ret.append(Update(value.toObject()));
    }

    return ret;
}
*/
void Bot::internalGetUpdates()
{
    ParameterList params;
    params.insert("offset", HttpParameter((int)m_updateOffset));
    params.insert("limit", HttpParameter(50));
    params.insert("timeout", HttpParameter(m_pollingTimeout));
    auto reply = m_net->asyncRequest(ENDPOINT_GET_UPDATES, params, Networking::GET);
    if (!reply) {
        qWarning() << __PRETTY_FUNCTION__ << "request failed";
        if (m_internalUpdateTimer)
            m_internalUpdateTimer->start(m_updateInterval);
        return;
    }
    _pendingReplies.insert(std::make_pair(reply,
                                          [this](QNetworkReply *reply) {
                               if (reply->error() != QNetworkReply::NoError) {
                                   qCritical("%s", qPrintable(QString("[%1] %2").arg(reply->error()).arg(reply->errorString())));
                                   if (m_internalUpdateTimer)
                                       m_internalUpdateTimer->start(m_updateInterval);
                                   return;
                               }
                               QByteArray arr = reply->readAll();
                               QJsonArray json = this->jsonArrayFromByteArray(arr);
                               //if (json.count())
                               // qDebug() << __PRETTY_FUNCTION__ << json;
                               foreach (QJsonValue value, json) {
                                   if (value.isObject()) {
                                       const QJsonObject &obj = value.toObject();
                                       uint64_t id = 0;
                                       if (obj.contains("update_id")) {
                                           id = obj["update_id"].toDouble();
                                           if (id >= m_updateOffset)
                                            m_updateOffset = id + 1;
                                       }

                                       if (obj.contains("message")||obj.contains("channel_post")) {
                                           Update u(obj);
                                           emit message(id, u.message);
                                       } else
                                        qDebug() << __PRETTY_FUNCTION__ << "ignored obj:" << obj;
                                   } else
                                    qDebug() << __PRETTY_FUNCTION__ << "ignored:" << value;
                               }
                               if (m_internalUpdateTimer)
                                   m_internalUpdateTimer->start(m_updateInterval);
                           }
                               ));

/*
    QList<Update> updates = getUpdates(m_pollingTimeout, 50, m_updateOffset);
    
    foreach (Update u, updates) {
        // change updateOffset to u.id to avoid duplicate updates
        m_updateOffset = (u.id >= m_updateOffset ? u.id + 1 : m_updateOffset);
        
        emit message(u.message);
    }
    m_internalUpdateTimer->start(m_updateInterval);
    */
}
